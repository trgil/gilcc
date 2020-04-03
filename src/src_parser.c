/************************************************************************
 * Copyright (c) 2019, Gil Treibush
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * A copy of the full GNU General Public License is included in this
 * distribution in a file called "COPYING" or "LICENSE".
 ***********************************************************************/

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "src_parser.h"
#include "analysis_print.h"

#define TMP_FILE_NAME           ".gilcc-tmpfile-XXXXXX"
#define TMP_FILE_NAME_SIZE      22

/* Parser file-buffer/stack */
#define PSTACK_BUF_SIZE     2
#define PARSER_BUF_SIZE     200

struct pbuf {
    char pbuf_buf[PARSER_BUF_SIZE];
    int pbuf_content_size;
    int pbuf_indx;
};

#define PBUF_CUR_CHAR(B) (B.pbuf_buf[B.pbuf_indx])
#define PBUF_DATA_SIZE(B) (B.pbuf_content_size - B.pbuf_indx)
#define PBUF_ADVN(B) (B.pbuf_indx++)

static int pbuf_fill(struct pbuf *buf, const int ifd)
{
    int read_size;

    if (buf->pbuf_content_size - buf->pbuf_indx)
        return (buf->pbuf_content_size - buf->pbuf_indx);

    read_size = read(ifd, buf->pbuf_buf, PARSER_BUF_SIZE);
    buf->pbuf_indx = 0;
    buf->pbuf_content_size = read_size;

    return read_size;
}

static inline int pbuf_write_char(struct pbuf *buf, const int ofd)
{
    return write(ofd, &buf->pbuf_buf[buf->pbuf_indx], 1);
}

struct pstack {
    char pstack_buf[PSTACK_BUF_SIZE];
    int pstack_indx;
};

#define PSTACK_DATA_SIZE(S) (S.pstack_indx)
#define PSTACK_PUSH_CHAR(S, C) (S.pstack_buf[S.pstack_indx++] = C)
#define PSTACK_CLEAR(S) (S.pstack_indx = 0)

static inline int pstack_write(struct pstack *stk, const int ofd)
{
    int write_size;

    if (!stk->pstack_indx)
        return 0;

    if ((write_size = write(ofd, stk->pstack_buf, stk->pstack_indx)) > 0)
        stk->pstack_indx = 0;

    return write_size;
}

int *line_reduce_lst = NULL;
static int line_reduce_lst_size;

static inline int line_reduce_add_line(int line_num, int seq_num)
{
    if (!line_reduce_lst)
        return -1;

    if (seq_num >= line_reduce_lst_size)
        return -1;

    line_reduce_lst[seq_num] = line_num;

    return 0;
}

/* TODO: add file tracking and per-file line/char count. */

static void print_file_full(int fd)
{
    char f_buf[PARSER_BUF_SIZE];
    int read_size;

    if (lseek(fd, 0, SEEK_SET)) {
        fprintf(stderr, "**Error: Could not set offset.\n");
        return;
    }

    while ((read_size = read(fd, f_buf, PARSER_BUF_SIZE)) > 0) {
        int read_indx = 0;

        while (read_indx < read_size)
            putchar(f_buf[read_indx++]);
    }
    printf("\n");
}

static inline int write_char(const char c, const int ofd)
{
    return write(ofd, &c, 1);
}

static int src_parser_tstage_1( const int dst_fd,
                                const int src_fd,
                                const bool exp_trigraphs)
{
    struct pbuf buf = {
        .pbuf_indx = 0,
        .pbuf_content_size = 0
    };

    struct pstack stk = {
        .pstack_indx = 0
    };

    int state = 0;

    /* CPP Translation phase 1:
     *  - Map physical characters to source character set.
     *  - Expand trigraphs (if supported).
     *
     * There are several possibilities for end-of-line indicator, which should be
     * replaced each with a single new-line character:
     *  - LF (\n), ASCII: 10.
     *  - CR (\r), ASCII: 13.
     *  - CR + LF (\r\n), ASCII: 13 + 10.
     *  - LF + CR (\n\r), ASCII: 10 + 13.
     *  - RS (Record Separator), ASCII: 30
     *
     * Trigraphs all start with the sequence '??'.
     */

    pbuf_fill(&buf, src_fd);
    while (PBUF_DATA_SIZE(buf) || (pbuf_fill(&buf, src_fd) > 0)) {
        switch(state) {
        case 0:
            switch (PBUF_CUR_CHAR(buf)) {
            case '\r':
                state++;
            case '\n':
                state++;
            case 30:
                write_char('\n', dst_fd);
                PBUF_ADVN(buf);
                break;

            case '?':
                PSTACK_PUSH_CHAR(stk, '?');
                PBUF_ADVN(buf);
                state = 3;
                break;

            default:
                pbuf_write_char(&buf, dst_fd);
                PBUF_ADVN(buf);
            }

            break;

        case 1:
            if (PBUF_CUR_CHAR(buf) == '\r')
                PBUF_ADVN(buf);
            state = 0;
            break;

        case 2:
            if (PBUF_CUR_CHAR(buf) == '\n')
                PBUF_ADVN(buf);
            state = 0;
            break;

        case 3:
            if (PBUF_CUR_CHAR(buf) == '?') {
                PSTACK_PUSH_CHAR(stk, '?');
                PBUF_ADVN(buf);
                state = 4;
            } else {
                pstack_write(&stk, dst_fd);
                state = 0;
            }
            break;

        case 4:
            {
                char c;

                switch (PBUF_CUR_CHAR(buf)) {
                case '=':
                    c = '#';
                    break;
                case '(':
                    c = '[';
                    break;
                case ')':
                    c = ']';
                    break;
                case '/':
                    c = '\\';
                    break;
                case '\'':
                    c = '^';
                    break;
                case '<':
                    c = '{';
                    break;
                case '>':
                    c = '}';
                    break;
                case '!':
                    c = '|';
                    break;
                case '-':
                    c = '~';
                    break;
                default:
                    c = '\0';
                    break;
                }

                if (c && exp_trigraphs) {
                    write_char(c, dst_fd);
                    PSTACK_CLEAR(stk);
                    PBUF_ADVN(buf);
                } else if (c && !exp_trigraphs) {
                    /* TODO: track line/char/file-name & add to error message */
                    analysis_print(APRINT_WARNING, 2, "unsupported trigraph sequence.");
                    pstack_write(&stk, dst_fd);
                } else {
                    pstack_write(&stk, dst_fd);
                }
            }

            state = 0;
            break;

        default:
            return -1;
        }
    }

    return 0;
}

static int src_parser_pre_stage_2(const int src_fd)
{
    struct pbuf buf = {
        .pbuf_indx = 0,
        .pbuf_content_size = 0
    };

    int state = 0;
    int new_line_cnt;
    bool line_empty = true;

    /* Pre-stage 2 processing:
     *
     *  - Find style errors:
     *      - Tabs mixed with spaces.
     *      - Multiple sequential new lines.
     *      - Line containing nothing but white spaces.
     *
     *  - Get number of line splits and allocate split-line record
     *    object.
     */

    /* TODO: search for Sequential split lines. */
    /* TODO: allow for mixing tabs and spaces in comments */

    line_reduce_lst_size = 0;
    pbuf_fill(&buf, src_fd);

    while (PBUF_DATA_SIZE(buf) || (pbuf_fill(&buf, src_fd) > 0)) {
        switch (state) {
        case 0:

            switch (PBUF_CUR_CHAR(buf)) {
            case '\\':
                state++;
            case '\n':
                state++;
            case '\t':
                state++;
            case ' ':
                state++;
                break;

            default:
                break;
            }

            PBUF_ADVN(buf);
            break;

        case 1:
            switch (PBUF_CUR_CHAR(buf)) {
            case '\t':
                analysis_print(APRINT_WARNING, 2, "Mixing spaces and tabs.");
                state++;
            case ' ':
                PBUF_ADVN(buf);
                break;

            case '\\':
                line_empty = false;
                state++;
            case '\n':
                state += 2;
                PBUF_ADVN(buf);
                break;

            default:
                line_empty = false;
                state = 0;
                break;
            }
            break;

        case 2:
            switch (PBUF_CUR_CHAR(buf)) {
            case ' ':
                analysis_print(APRINT_WARNING, 2, "Mixing spaces and tabs.");
                state = 1;
            case '\t':
                PBUF_ADVN(buf);
                break;

            case '\\':
                line_empty = false;
                state++;
            case '\n':
                state++;
                PBUF_ADVN(buf);
                break;

            default:
                line_empty = false;
                state = 0;
                break;
            }
            break;

        case 3:
            if (new_line_cnt < 1 && line_empty)
                analysis_print(APRINT_WARNING, 2, "Line contains only white spaces.");
            else
                line_empty = true;

            switch (PBUF_CUR_CHAR(buf)) {
            case '\n':
                new_line_cnt++;
                if (new_line_cnt > 2)
                    analysis_print(APRINT_WARNING, 2, "Multiple sequential new-lines.");
                PBUF_ADVN(buf);
                break;

            case ' ':
                state--;
            case '\t':
                state--;
                new_line_cnt = 0;
                PBUF_ADVN(buf);
                break;

            case '\\':
                new_line_cnt = 0;
                line_empty = false;
                state++;
                PBUF_ADVN(buf);
                break;

            default:
                new_line_cnt = 0;
                line_empty = false;
                state = 0;
                break;
            }
            break;

        case 4:
            switch (PBUF_CUR_CHAR(buf)) {
            case '\n':
                line_empty = true;
                line_reduce_lst_size++;
                state = 0;
            case '\\':
                PBUF_ADVN(buf);
                break;

            case ' ':
                state--;
            case '\t':
                state -= 2;
                PBUF_ADVN(buf);
                break;
            default:
                state = 0;
                break;
            }
            break;

        default:
            return -1;
        }
    }

    /* Allocate reduced lines record object. */
    if (line_reduce_lst_size) {
        line_reduce_lst = (int *)malloc(sizeof(int) * line_reduce_lst_size);
        if (!line_reduce_lst)
            return -1;
    }

    return 0;
}

static int src_parser_tstage_2( const int dst_fd,
                                const int src_fd)
{
    struct pbuf buf = {
        .pbuf_indx = 0,
        .pbuf_content_size = 0
    };

    int line_split_cntr = 0;
    int line_cntr = 1;
    int state = 0;

    /* CPP Translation phase 2:
     * Join split lines.
     */

    pbuf_fill(&buf, src_fd);
    while (PBUF_DATA_SIZE(buf) || (pbuf_fill(&buf, src_fd) > 0)) {
        switch (state) {
        case 0:
            if (PBUF_CUR_CHAR(buf) == '\\') {
                state = 1;
            } else {

                if (PBUF_CUR_CHAR(buf) == '\n')
                    line_cntr++;

                pbuf_write_char(&buf, dst_fd);
            }

            PBUF_ADVN(buf);
            break;

        case 1:
            if (PBUF_CUR_CHAR(buf) == '\\') {
                write_char('\\', dst_fd);
                PBUF_ADVN(buf);
            } else {
                state = 0;
                if (PBUF_CUR_CHAR(buf) == '\n') {
                    /* TODO: Check return value of this */
                    line_reduce_add_line(line_cntr++, line_split_cntr++);
                    PBUF_ADVN(buf);
                } else {
                    write_char('\\', dst_fd);
                }
            }
            break;

        default:
            return -1;
        }
    }

    /* TODO: check if file ends with '\n', warn if not? */
    return 0;
}

/* TODO: analyze comments (mixed comment sequences, comments inside of strings, etc.) */

static int src_parser_tstage_3( const int dst_fd,
                                const int src_fd,
                                const bool exp_cpp_cmnts)
{
    struct pbuf buf = {
        .pbuf_indx = 0,
        .pbuf_content_size = 0
    };

    int state = 0;

    /* CPP Translation phase 3:
     *  - Replace comments with white spaces.
     *  - Turn horizontal tabs (not in strings) into white spaces
     *      (not required by standard).
     *  - Truncate sequential white spaces.
     *  - Truncate sequential new-lines (not required by standard).
     */

    /* TODO: do not replace comments/spaces inside strings */

    pbuf_fill(&buf, src_fd);
    while (PBUF_DATA_SIZE(buf) || (pbuf_fill(&buf, src_fd) > 0)) {
        switch (state) {
        case 0:
            switch (PBUF_CUR_CHAR(buf)) {
            case '/':
                state = 1;
                PBUF_ADVN(buf);
                break;

            case ' ':
            case '\t':
                write_char(' ', dst_fd);
                state = 5;
                PBUF_ADVN(buf);
                break;

            case '\"':
                write_char('\"', dst_fd);
                state = 6;
                PBUF_ADVN(buf);
                break;

            default:
                pbuf_write_char(&buf, dst_fd);
                PBUF_ADVN(buf);
            }
            break;

        case 1:
            switch (PBUF_CUR_CHAR(buf)) {
            case '*':
                state = 2;
                PBUF_ADVN(buf);
                break;

            case '/':
                if (exp_cpp_cmnts == true) {
                    state = 4;
                    PBUF_ADVN(buf);
                    break;
                }

            default:
                write_char('/', dst_fd);
                state = 0;
            }
            break;

        case 2:
            if (PBUF_CUR_CHAR(buf) == '*')
                state = 3;
            PBUF_ADVN(buf);
            break;

        case 3:
            if (PBUF_CUR_CHAR(buf) == '/')
                state = 0;
            else
                state = 2;
            PBUF_ADVN(buf);
            break;

        case 4:
            if (PBUF_CUR_CHAR(buf) == '\n')
                state = 0;
            PBUF_ADVN(buf);
            break;

        case 5:
            /* TODO: track reduced characters */
            if ((PBUF_CUR_CHAR(buf) == ' ') || (PBUF_CUR_CHAR(buf) == '\t'))
                PBUF_ADVN(buf);
            else
                state = 0;
            break;

        case 6:
            if (PBUF_CUR_CHAR(buf) == '\\')
                state = 7;
            else if (PBUF_CUR_CHAR(buf) == '\"')
                state = 0;

            pbuf_write_char(&buf, dst_fd);
            PBUF_ADVN(buf);
            break;

        case 7:
            pbuf_write_char(&buf, dst_fd);
            PBUF_ADVN(buf);
            state = 6;
            break;

        default:
            return -1;
        }
    }

    if ((state == 2) || (state == 3))
        analysis_print(APRINT_ERROR, 2, "file ends with an unterminated comment.");

    return 0;
}

int src_parser_cpp(const char *src, const struct trans_config *cfg)
{
    int tmp_fd1, tmp_fd2, tmp_fd3;
    int src_fd;
    char fname1[TMP_FILE_NAME_SIZE];
    char fname2[TMP_FILE_NAME_SIZE];
    char fname3[TMP_FILE_NAME_SIZE];
    int ret_val;

    /* Open the source file */
    src_fd = open(src, O_RDONLY);
    if (src_fd == -1) {
        fprintf(stderr, "**Error: Could not open source file: %s.\n", src);
        return -1;
    }

    /* Open temporary work-file for stage 1 */
    strncpy(fname1, TMP_FILE_NAME, TMP_FILE_NAME_SIZE);
    tmp_fd1 = mkstemp(fname1);
    if (tmp_fd1 == -1) {
        fprintf(stderr, "**Error: could not create a working file.\n");
        return -1;
    }

    /* Do stage 1 parsing */
    ret_val = src_parser_tstage_1(tmp_fd1, src_fd, cfg->exp_trigraphs);
    if (ret_val < 0)
        return ret_val;

    /* Source file no longer needed */
    close(src_fd);

    /* Rewind stage 1 output file */
    if (lseek(tmp_fd1, 0, SEEK_SET)) {
        fprintf(stderr, "**Error: Could not set offset.\n");
        return -1;
    }
    /* Count the number of split lines we have */
    /* TODO: check return value */
    src_parser_pre_stage_2(tmp_fd1);

    /* Open temporary work-file for stage 2 */
    strncpy(fname2, TMP_FILE_NAME, TMP_FILE_NAME_SIZE);
    tmp_fd2 = mkstemp(fname2);
    if (tmp_fd2 == -1) {
        fprintf(stderr, "**Error: could not create a working file.\n");
        return -1;
    }

    /* Rewind stage 1 output file */
    if (lseek(tmp_fd1, 0, SEEK_SET)) {
        fprintf(stderr, "**Error: Could not set offset.\n");
        return -1;
    }

    /* Do stage 2 parsing */
    ret_val = src_parser_tstage_2(tmp_fd2, tmp_fd1);
    if (ret_val < 0)
        return ret_val;

    /* Stage 1 file no longer needed */
    unlink(fname1);

    /* Open temporary work-file for stage 3 */
    strncpy(fname3, TMP_FILE_NAME, TMP_FILE_NAME_SIZE);
    tmp_fd3 = mkstemp(fname3);
    if (tmp_fd2 == -1) {
        fprintf(stderr, "**Error: could not create a working file.\n");
        return -1;
    }

    /* Rewind stage 2 output file */
    if (lseek(tmp_fd2, 0, SEEK_SET)) {
        fprintf(stderr, "**Error: Could not set offset.\n");
        return -1;
    }

    /* Do stage 3 parsing */
    ret_val = src_parser_tstage_3(tmp_fd3, tmp_fd2, cfg->exp_cpp_cmnts);
    if (ret_val < 0)
        return ret_val;

    /* Stage 2 file no longer needed */
    unlink(fname2);
    printf("Stage 3 output:\n");
    print_file_full(tmp_fd3);

    /* Stage 3 file no longer needed */
    unlink(fname3);

    return 0;
}

