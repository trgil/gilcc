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

#include "analysis_print.h"

static char *ap_msgs[APRINT_TYPES_NUM] = {
    [APRINT_INFO] = "INFO",
    [APRINT_WARNING] = "WARNING",
    [APRINT_ERROR] = "ERROR"
};

int analysis_print(enum analysis_print_type ap_type, int p_num, char *msg)
{
    if ((ap_type >= APRINT_TYPES_NUM) || (ap_type < 0))
        return -1;

    printf("%s:%4d:%s\n", ap_msgs[ap_type], p_num, msg);
    return 0;
}
