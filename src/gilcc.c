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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "gilcc.h"
#include "std_comp.h"
#include "src_parser.h"
#include "analysis_print.h"

static char *srcs[GILCC_SRCS_MAX_NUM];
static int srcs_num;

static char **ipaths;
static int ipaths_num;

static char **defs;
static int defs_num;

static void print_usage(void)
{
    printf( "usage: gilcc [OPTIONS] [Input files]\n"
            "GilCC options:\n"
            "\t-h, --help           - Print this help menu and quit.\n"
            "\t-v, --version        - Print program version and quit.\n"
            "GCC compatible options:\n"
            "\tMost GCC compatible flags, which influence the way source files\n"
            "\tare parsed by GCC.\n"
            "*Unknown flags will be ignored.\n");
}

static void print_version(void)
{
    printf("gilcc - Gil's Code Cleanup, version %.1f\n", GILCC_VERSION);
}

static inline int std_error(void)
{
    analysis_print(APRINT_ERROR, 1, "multiple standard declarations.");
    return -1;
}

static int pre_parse_cmd(int argc, char** argv)
{
    char *cmd;

    while (argc) {
        cmd = argv[0];

        if (cmd[0] == '-') {
            /* Probably a flag. */

            if (!strncmp(cmd, "-D", 2))
                defs_num++;
            else if (!strncmp(cmd, "-I", 2))
                ipaths_num++;
        }

        argc--;
        argv++;
    }

    if (ipaths_num) {
        ipaths = (char **)malloc(sizeof(char *) * ipaths_num);
        if (!ipaths)
            return -1;
    }

    if (defs_num) {
        defs = (char **)malloc(sizeof(char *) * defs_num);
        if (!defs) {
            free(ipaths);
            return -1;
        }
    }

    return 0;
}

static int parse_cmd(int argc, char** argv, struct trans_config *cfg)
{
    char *cmd;
    int ipath_cntr = 0;
    int defs_cntr = 0;

    if (!cfg)
        return -1;

    while (argc) {
        cmd = argv[0];

        if (cmd[0] == '-') {
            /* Probably a flag. */

            if (!strcmp(cmd, "-h") || !strcmp(cmd, "--help")) {
                print_usage();
                srcs_num = 0;
                return 0;

            } else if (!strcmp(cmd, "-v") || !strcmp(cmd, "--version")) {
                print_version();
                srcs_num = 0;
                return 0;

            } else if ( !strcmp(cmd, "-ansi") ||
                        !strcmp(cmd, "-std=c90") ||
                        !strcmp(cmd, "-std=iso9899:1990")) {

                if (cfg->std)
                    return std_error();

                if (cfg->exp_trigraphs)
                    analysis_print(APRINT_WARNING, 1, "trigraphs are already allowed by the standard.");
                else
                    cfg->exp_trigraphs = true;

                cfg->std = C_STANDARD_C90_ORIG;

            } else if (!strcmp(cmd, "-std=gnu90")) {

                if (cfg->std)
                    return std_error();

                cfg->std = C_STANDARD_C90_GNU;
                cfg->exp_cpp_cmnts = true;

            } else if (!strcmp(cmd, "-std=iso9899:199409")) {

                if (cfg->std)
                    return std_error();

                cfg->std = C_STANDARD_C95_AMD1;
                cfg->exp_trigraphs = true;

            } else if ( !strcmp(cmd, "-std=c99") ||
                        !strcmp(cmd, "-std=iso9899:1999")) {

                if (cfg->std)
                    return std_error();

                if (cfg->exp_trigraphs)
                    analysis_print(APRINT_WARNING, 1, "trigraphs are already allowed by the standard.");
                else
                    cfg->exp_trigraphs = true;

                cfg->std = C_STANDARD_C99_ORIG;
                cfg->exp_cpp_cmnts = true;

            } else if (!strcmp(cmd, "-std=gnu99")) {

                if (cfg->std)
                    return std_error();

                cfg->std = C_STANDARD_C99_GNU;
                cfg->exp_cpp_cmnts = true;

            } else if ( !strcmp(cmd, "-std=c11") ||
                        !strcmp(cmd, "-std=iso9899:2011")) {

                if (cfg->std)
                    return std_error();

                if (cfg->exp_trigraphs)
                    analysis_print(APRINT_WARNING, 1, "trigraphs are already allowed by the standard.");
                else
                    cfg->exp_trigraphs = true;

                cfg->std = C_STANDARD_C11_ORIG;
                cfg->exp_cpp_cmnts = true;

            } else if (!strcmp(cmd, "-std=gnu11")) {

                if (cfg->std)
                    return std_error();

                cfg->std = C_STANDARD_C11_GNU;
                cfg->exp_cpp_cmnts = true;

            } else if ( !strcmp(cmd, "-std=c17") ||
                        !strcmp(cmd, "-std=iso9899:2017")) {

                if (cfg->std)
                    return std_error();

                if (cfg->exp_trigraphs)
                    analysis_print(APRINT_WARNING, 1, "trigraphs are already allowed by the standard.");
                else
                    cfg->exp_trigraphs = true;

                cfg->std = C_STANDARD_C17_ORIG;
                cfg->exp_cpp_cmnts = true;

            } else if ( !strcmp(cmd, "-trigraphs")) {

                if (cfg->exp_trigraphs)
                    analysis_print(APRINT_WARNING, 1, "trigraphs are already allowed by the standard.");
                else
                    cfg->exp_trigraphs = true;

            } else if (!strncmp(cmd, "-D", 2)) {
                if (defs_cntr >= defs_num) {
                    fprintf(stderr, "**Error: too many defines.\n");
                    return -1;
                }

                if (strlen(cmd) > 2) {
                    defs[defs_cntr++] = (cmd + 2);
                } else {
                    if (argc == 1) {
                        fprintf(stderr, "**Error: missing define parameter.\n");
                        return -1;
                    }

                    argc--;
                    argv++;
                    defs[defs_cntr++] = argv[0];
                }

            } else if (!strncmp(cmd, "-I", 2)) {
                if (ipath_cntr >= ipaths_num) {
                    fprintf(stderr, "**Error: too many defines.\n");
                    return -1;
                }

                if (strlen(cmd) > 2) {
                    ipaths[ipath_cntr++] = (cmd + 2);

                } else {
                    if (argc == 1) {
                        fprintf(stderr, "**Error: missing include path parameter.\n");
                        return -1;
                    }

                    argc--;
                    argv++;
                    ipaths[ipath_cntr++] = argv[0];
                }
            }

            /* Unmatched flags will be ignored. */

        } else {
            /* Probably a file. */

            if (srcs_num >= GILCC_SRCS_MAX_NUM) {
                fprintf(stderr, "**Error: too many input files.\n");
                return -1;
            }

            srcs[srcs_num++] = cmd;
        }

        argc--;
        argv++;
    }
    return 0;
}

int main(int argc, char** argv)
{
    struct trans_config cfg = {
        .std = 0,
        .exp_trigraphs = false,
        .exp_cpp_cmnts = false,
    };

    int i,j;
    
    pre_parse_cmd(--argc, ++argv);

    if(parse_cmd(argc, argv, &cfg) < 0)
        /* Something went wrong during CLI command parsing. */
        return 1;

    /* Check duplications in command-line arguments */
    if (ipaths_num) {
        for (i = 0; i < (ipaths_num - 1); i++) {
            for (j = i + 1; j < ipaths_num; j++) {
                if (!strcmp(ipaths[i], ipaths[j]))
                    analysis_print(APRINT_WARNING, 1, "duplicate path via command-line.");
            }
        }
    }

    if (defs_num) {
        for (i = 0; i < (defs_num - 1); i++) {
            for (j = i + 1; j < defs_num; j++) {
                if (!strcmp(defs[i], defs[j]))
                    analysis_print(APRINT_WARNING, 1, "duplicate definition via command-line.");
            }
        }
    }

    /* TODO: verify missing files check */

    if (srcs_num == 0) {
        if (argc > 2)
            /* We have multiple flags with no input files. */
            return 2;

        /* We have a single flag, no input files (probably a -v or -h). */
        return 0;
    }

    /* TODO: configure policy for multiple standard flags */

    if (!cfg.std) {
        cfg.std = C_STANDARD_C11_GNU;
        cfg.exp_cpp_cmnts = true;
    }

    if (set_std_limits(&cfg.lim, cfg.std)) {
        fprintf(stderr, "**Error: Could Not configure standard limits\n");
        return 1;
    }

    while (srcs_num--) {
        if (access(srcs[srcs_num], R_OK)) {
            fprintf(stderr, "**Error: Could Not access file: %s\n", srcs[srcs_num]);
            continue;
        }

        printf("Processing file [ %s ]:\n", srcs[srcs_num]);
        if (src_parser_cpp(srcs[srcs_num], &cfg) < 0)
            return 1;
    }

    if (ipaths)
        free(ipaths);

    if (defs)
        free(defs);

    return 0;
}

