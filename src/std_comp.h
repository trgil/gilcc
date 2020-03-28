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

#ifndef _STD_COMP_H__
#define _STD_COMP_H__

#include <stdbool.h>

/* Standard complience defines */
#define C_STANDARD_KNR_ORIG 0x0001UL
#define C_STANDARD_C89_ORIG 0x0002UL
#define C_STANDARD_C90_ORIG 0x0004UL
#define C_STANDARD_C90_GNU  0x0008UL
#define C_STANDARD_C95_AMD1 0x0010UL
#define C_STANDARD_C99_ORIG 0x0020UL
#define C_STANDARD_C99_GNU  0x0040UL
#define C_STANDARD_C11_ORIG 0x0080UL
#define C_STANDARD_C11_GNU  0x0100UL
#define C_STANDARD_C17_ORIG 0x0200UL

/* Standards complience options */
#define STD_COMPL_OPT(STD) (1-(STD<<1))

/* Standard dependent limits */
struct std_trans_lim {

    /* Nesting level for: compound statements, iteration control,
     * selection control structures.
     */
    unsigned int cmpnd_statements_nst_lvl;

    /* Nesting level for conditional inclusion. */
    unsigned int cond_nst_lvl;

    /* Number of declarators in sequence modifying a type in declaration. */
    unsigned int decl_mod_num;

    /* Nesting level for parenthesized declarators in a declarator. */
    unsigned int paren_decl_nst_lvl;

    /* Nesting level for parenthesized expressions in an expression. */
    unsigned int paren_exp_nst_lvl;

    /* Number of significant initial characters in an internal identifier. */
    unsigned int init_char_intern_ident_num;

    /* Number of significant initial characters in an external identifier. */
    unsigned int init_char_extern_ident_num;

    /* Number of external identifiers in a single translation unit. */
    unsigned int extern_ident_num;

    /* Number of identifiers in a block scope. */
    unsigned int ident_block_num;

    /* Number of macro identifiers in a single translation unit. */
    unsigned int ident_macro_num;

    /* Number of parameters in a function definition. */
    unsigned int param_func_num;

    /* Number of arguments in a function call. */
    unsigned int arg_func_num;

    /* Number of parameters in a macro definition. */
    unsigned int param_macro_num;

    /* Number of arguments in a macro call. */
    unsigned int arg_macro_num;

    /* Number of characters in a line of source code. */
    unsigned int char_src_line_num;

    /* Number of characters in a string literal. */
    unsigned int char_src_str_num;

    /* Number of bytes in a single object. */
    unsigned int byte_obj_num;

    /* Nesting level for included files. */
    unsigned int f_incl_nst_lvl;

    /* Number of case labels in a switch statement. */
    unsigned int cslbl_num;

    /* Number of members in a struct or union. */
    unsigned int membrs_cmpnd_num;

    /* Number of enumeration constants in an enumeration. */
    unsigned int enm_cnst_num;

    /* Nesting level for structs and unions. */
    unsigned int cmpnd_nst_lvl;
};

struct std_config {
    unsigned long std;
    char *name;
    char *cli_flags[4];

    /* Default configurations */
    bool exp_trigraphs;
    bool exp_cpp_cmnts;
};

#define STD_SUPPORTED_NUM 8

extern struct std_config std_configs[STD_SUPPORTED_NUM];

/* Translation configurations */
struct trans_config {

    /* TODO: add pointer to std struct in std_configs*/

    /* Standard complience */
    unsigned long std;

    /* Standard based configurations */
    struct std_trans_lim lim;

    /* Other common translation configurations */
    bool exp_trigraphs;
    bool exp_cpp_cmnts;
};

/* std_comp API Functions */
int set_std_limits(struct std_trans_lim *lims, unsigned long std);

#endif /* _STD_COMP_H__ */
