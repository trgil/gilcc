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

#include "gilcc.h"

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
    printf("gilcc - Gil's Code Clean, version %.1f\n", GILCC_VERSION);
}

static int parse_cmd(int argc, char** argv)
{
    char *cmd;
    int srcs = 0;

    while (argc) {
        cmd = argv[0];

        if (cmd[0] == '-') {
            /* Probably a flag. */
            if (!strcmp(cmd, "-h") || !strcmp(cmd, "--help")) {
                print_usage();
                return 0;

            } else if (!strcmp(cmd, "-v") || !strcmp(cmd, "--version")) {
                print_version();
                return 0;

            }

            /* Unmatched flags will be ignored. */

        } else {
            /* Probably a file. */

            srcs++;
        }

        argc--;
        argv++;
    }

    return srcs;
}

int main(int argc, char** argv)
{
    int srcs;

    srcs = parse_cmd(argc--, argv++);

    if (srcs == 0) {
        if (argc > 2)
            /* We have multiple flags with no input files. */
            return 2;

        /* We have a single flag. */
        return 0;
    }

    if (srcs < 0)
        return 1;

    while (--srcs) {
        /* Process each input file here. */
    }

    return 0;
}

