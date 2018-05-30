/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 30.05.2018
 *
 * Generate config parser procedures.
 *
 */

// TOP

#if !defined(FRED_META_GENERATE_PARSER_H)
#define FRED_META_GENERATE_PARSER_H

struct Operation{
    int32_t r_type;
    char *proc_name;
    char *result_type;
    char *output_type;
    char *extra_params;
    char *extra_args;
    char *code_before;
    char *code_after;
};

enum{
    OpClassIterate_Operations = 0,
    OpClassIterate_Types = 1,
    OpClassIterate_COUNT = 2,
};

struct Op_Class{
    int32_t iteration_type;
};

#endif

// BOTTOM

