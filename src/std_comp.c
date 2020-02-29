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

#include <string.h>

#include "std_comp.h"

int set_std_limits(struct std_trans_lim *lims, unsigned long std)
{
    switch (std) {
    case C_STANDARD_C90_GNU:
        /* TODO: Adjust according to spec. */
    case C_STANDARD_KNR_ORIG:
    case C_STANDARD_C89_ORIG:
    case C_STANDARD_C90_ORIG:
    case C_STANDARD_C95_AMD1:
    {
        struct std_trans_lim lim_init = {
            .cmpnd_statements_nst_lvl = 15,
            .cond_nst_lvl = 8,
            .decl_mod_num = 12,
            .paren_decl_nst_lvl = 31,
            .paren_exp_nst_lvl = 32,
            .init_char_intern_ident_num = 31,
            .init_char_extern_ident_num = 6,
            .extern_ident_num = 511,
            .ident_block_num = 127,
            .ident_macro_num = 1024,
            .param_func_num = 31,
            .arg_func_num = 31,
            .param_macro_num = 31,
            .arg_macro_num = 31,
            .char_src_line_num = 509,
            .char_src_str_num = 509,
            .byte_obj_num = 32767,
            .f_incl_nst_lvl = 8,
            .cslbl_num = 257,
            .membrs_cmpnd_num = 127,
            .enm_cnst_num = 127,
            .cmpnd_nst_lvl = 15,
        };

        memcpy(lims, &lim_init, sizeof(struct std_trans_lim));
    }
    break;

    case C_STANDARD_C99_GNU:
        /* TODO: Adjust according to spec. */
    case C_STANDARD_C11_GNU:
        /* TODO: Adjust according to spec. */
    case C_STANDARD_C99_ORIG:
    case C_STANDARD_C11_ORIG:
    case C_STANDARD_C17_ORIG:
    {
        struct std_trans_lim lim_init = {
            .cmpnd_statements_nst_lvl = 127,
            .cond_nst_lvl = 63,
            .decl_mod_num = 12,
            .paren_decl_nst_lvl = 63,
            .paren_exp_nst_lvl = 63,
            .init_char_intern_ident_num = 63,
            .init_char_extern_ident_num = 31,
            .extern_ident_num = 4095,
            .ident_block_num = 511,
            .ident_macro_num = 4095,
            .param_func_num = 127,
            .arg_func_num = 127,
            .param_macro_num = 127,
            .arg_macro_num = 127,
            .char_src_line_num = 4095,
            .char_src_str_num = 4095,
            .byte_obj_num = 65535,
            .f_incl_nst_lvl = 15,
            .cslbl_num = 1023,
            .membrs_cmpnd_num = 1023,
            .enm_cnst_num = 1023,
            .cmpnd_nst_lvl = 63,
        };

        memcpy(lims, &lim_init, sizeof(struct std_trans_lim));
    }
    break;

    default:
        return -1;
    }

    return 0;
}

