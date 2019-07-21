/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 24.03.2018
 *
 * Code wrapping logic
 *
 */

// TOP

#if !defined(FRED_CODE_WRAP_H)
#define FRED_CODE_WRAP_H

struct Code_Wrap_X{
    f32 base_x;
    f32 paren_nesting[32];
    i32 paren_safe_top;
    i32 paren_top;
};
global Code_Wrap_X null_wrap_x  = {};

struct Code_Wrap_State{
    Cpp_Token_Array token_array;
    Cpp_Token *token_ptr;
    Cpp_Token *end_token;
    
    Code_Wrap_X wrap_x;
    
    b32 in_pp_body;
    Code_Wrap_X plane_wrap_x;
    
    i32 *line_starts;
    i32 line_count;
    i32 line_index;
    i32 next_line_start;
    
    f32 x;
    b32 consume_newline;
    
    Gap_Buffer_Stream stream;
    i32 size;
    i32 i;
    
    Face *face;
    f32 tab_indent_amount;
    f32 byte_advance;
    
    Translation_State tran;
    Translation_Emits emits;
    u32 J;
    Buffer_Model_Step step;
    Buffer_Model_Behavior behavior;
};

struct Code_Wrap_Step{
    i32 position_start;
    i32 position_end;
    
    f32 start_x;
    f32 final_x;
    
    Cpp_Token *this_token;
};

struct Wrap_Current_Shift{
    f32 shift;
    b32 adjust_top_to_this;
};

internal void
file_measure_wraps(System_Functions *system, Mem_Options *mem, Editing_File *file, Face *face);

#endif

// BOTTOM

