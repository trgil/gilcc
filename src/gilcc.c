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

static void cli_flags_analysis_print_2(int f_indx_1, int f_indx_2, char *flg_1, char *flg_2, char *msg)
{
    char p_msg[120];

    sprintf(p_msg, "CLI flag: (%d, %s) %s [previous: (%d, %s)]", f_indx_1, flg_1, msg, f_indx_2, flg_2);
    analysis_print(APRINT_WARNING, 2, p_msg);
}

static void cli_flag_analysis_print(int f_indx, char *flg, char *msg)
{
    char p_msg[120];

    sprintf(p_msg, "CLI flag: (%d, %s) %s", f_indx, flg, msg);
    analysis_print(APRINT_WARNING, 2, p_msg);
}

static void cli_param_analysis_print(char *param, char *msg)
{
    char p_msg[120];

    sprintf(p_msg, "CLI parameter: (%s) %s", param, msg);
    analysis_print(APRINT_WARNING, 2, p_msg);
}

static int pre_parse_cmd(int argc, char** argv, struct trans_config *cfg)
{
    char *cmd;
    int f_indx = 1;
    char *std_name = NULL;
    int std_indx = -1;

    /* We count the number of include-path and macro definition parameters,
     * and allocate a buffer for storring them in proper order.
     * We also determin the standard to be used, before we parse all the other
     * flags.
     */

    while (argc) {
        cmd = argv[0];

        if (cmd[0] == '-') {

            if (!strncmp(cmd, "-D", 2)) {
                defs_num++;

            } else if (!strncmp(cmd, "-I", 2)) {
                ipaths_num++;

            } else if ( !strcmp(cmd, "-ansi") ||
                        !strncmp(cmd, "-std=", 5)) {

                int i;

                for (i = 0; i < STD_SUPPORTED_NUM; i++) {

                    int j = 0;

                    while (std_configs[i].cli_flags[j]) {
                        if (!strcmp(cmd, std_configs[i].cli_flags[j])) {
                            if (cfg->std)
                                cli_flags_analysis_print_2(f_indx, std_indx, cmd, std_name,
                                        "multiple standard declarations");

                            if (cfg->std < std_configs[i].std) {
                                cfg->std = std_configs[i].std;
                                cfg->exp_trigraphs = std_configs[i].exp_trigraphs;
                                cfg->exp_cpp_cmnts = std_configs[i].exp_cpp_cmnts;
                            }

                            std_name = cmd;
                            std_indx = f_indx;

                            i = STD_SUPPORTED_NUM;
                            break;
                        }

                        j++;
                    }
                }
            }
        }

        argc--;
        argv++;
        f_indx++;
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

    /* This is the defautl standard if none is provided */
    if (!cfg->std) {
        cfg->std = C_STANDARD_C11_GNU;
        cfg->exp_cpp_cmnts = true;
        cfg->exp_trigraphs = false;
    }

    /* TODO: print working standard by referencing std struct */

    return 0;
}

static int parse_cmd(int argc, char** argv, struct trans_config *cfg)
{
    char *cmd;
    int ipath_cntr = 0;
    int defs_cntr = 0;
    int f_indx = 1;
    int trigraphs_flg = 0;

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

            } else if (!strcmp(cmd, "-trigraphs")) {

                if (cfg->exp_trigraphs)
                    if (trigraphs_flg)
                        cli_flag_analysis_print(f_indx, cmd, "flag duplicate");
                    else
                        cli_flag_analysis_print(f_indx, cmd, "trigraphs are already allowed by the standard");
                else
                    cfg->exp_trigraphs = true;

                trigraphs_flg++;

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
        f_indx++;
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

    pre_parse_cmd(--argc, ++argv, &cfg);

    if(parse_cmd(argc, argv, &cfg) < 0)
        /* Something went wrong during CLI command parsing. */
        return 1;

    /* Check duplications in command-line arguments */
    if (ipaths_num) {
        for (i = 0; i < (ipaths_num - 1); i++) {
            for (j = i + 1; j < ipaths_num; j++) {
                if (!strcmp(ipaths[i], ipaths[j]))
                    cli_param_analysis_print(ipaths[i], "duplicate inclusiong path parameter");
            }
        }
    }

    if (defs_num) {
        for (i = 0; i < (defs_num - 1); i++) {
            for (j = i + 1; j < defs_num; j++) {
                if (!strcmp(defs[i], defs[j]))
                    cli_param_analysis_print(defs[i], "duplicate definition parameter");
            }
        }
    }

    /* TODO: check environment variables (relevant to compiler) */
    /* TODO: verify missing files check */

    if (srcs_num == 0) {
        if (argc > 2)
            /* We have multiple flags with no input files. */
            return 2;

        /* We have a single flag, no input files (probably a -v or -h). */
        /* TODO: verify this ^ */
        return 0;
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

        analysis_print_param_1(APRINT_INFO, 2, "processing source file", srcs[srcs_num]);
        if (src_parser_cpp(srcs[srcs_num], &cfg) < 0)
            return 1;
    }

    if (ipaths)
        free(ipaths);

    if (defs)
        free(defs);

    return 0;
}

