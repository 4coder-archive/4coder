/*
4coder_function_list.cpp - Command for listing all functions in a C/C++ file in a jump list.
*/

// TOP

#if !defined(FCODER_FUNCTION_LIST_H)
#define FCODER_FUNCTION_LIST_H

struct Function_Positions{
    int32_t sig_start_index;
    int32_t sig_end_index;
    int32_t open_paren_pos;
};

struct Get_Positions_Results{
    int32_t positions_count;
    int32_t next_token_index;
    bool32 still_looping;
};

struct Buffered_Write_Stream{
    Buffer_ID output_buffer_id;
    Partition *buffering_arena;
    char *buffer;
};

#endif

// BOTTOM

