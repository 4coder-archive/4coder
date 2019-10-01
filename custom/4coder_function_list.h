/*
4coder_function_list.cpp - Command for listing all functions in a C/C++ file in a jump list.
*/

// TOP

#if !defined(FCODER_FUNCTION_LIST_H)
#define FCODER_FUNCTION_LIST_H

struct Function_Positions{
    i64 sig_start_index;
    i64 sig_end_index;
    i64 open_paren_pos;
};

struct Get_Positions_Results{
    i64 positions_count;
    i64 next_token_index;
    b32 still_looping;
};

#endif

// BOTTOM

