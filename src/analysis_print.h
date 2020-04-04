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

#ifndef _ANALYSIS_PRINTER_H__
#define _ANALYSIS_PRINTER_H__

enum analysis_print_type {
    APRINT_INFO,
    APRINT_WARNING,
    APRINT_ERROR,

    APRINT_TYPES_NUM
};

/* Analysis Printer API */

int analysis_print(enum analysis_print_type ap_type, int p_num, char *msg);
int analysis_print_param_1(enum analysis_print_type ap_type, int p_num, char *msg, char *param);

#endif /* _ANALYSIS_PRINTER_H__ */
