/*
4coder_function_list.cpp - Command for listing all functions in a C/C++ file in a jump list.
*/

// TOP

#if !defined(FCODER_FUNCTION_LIST_H)
#define FCODER_FUNCTION_LIST_H

struct Function_Positions{
    i32 sig_start_index;
    i32 sig_end_index;
    i32 open_paren_pos;
};

struct Get_Positions_Results{
    i32 positions_count;
    i32 next_token_index;
    b32 still_looping;
};

struct Buffered_Write_Stream{
    Buffer_ID output_buffer_id;
    Partition *buffering_arena;
    char *buffer;
};

#endif

// BOTTOM

