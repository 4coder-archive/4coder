/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 24.01.2018
 *
 * Buffer types
 *
 */

// TOP

#if !defined(FRED_BUFFER_H)
#define FRED_BUFFER_H

struct String_Array{
    String *vals;
    i32 count;
};

struct Cursor_With_Index{
    i32 pos;
    i32 index;
};

struct Gap_Buffer{
    char *data;
    i32 size1;
    i32 gap_size;
    i32 size2;
    i32 max;
    
    i32 *line_starts;
    i32 line_count;
    i32 line_max;
};

struct Gap_Buffer_Init{
    Gap_Buffer *buffer;
    char *data;
    i32 size;
};

struct Gap_Buffer_Stream{
    Gap_Buffer *buffer;
    char *data;
    i32 end;
    i32 separated;
    i32 absolute_end;
    
    b32 use_termination_character;
    char terminator;
};
global Gap_Buffer_Stream null_buffer_stream = {};

struct Buffer_Batch_State{
    i32 i;
    i32 shift_total;
};

struct Buffer_Measure_Starts{
    i32 i;
    i32 count;
    i32 start;
};

enum{
    BLStatus_Finished,
    BLStatus_NeedWrapLineShift,
    BLStatus_NeedLineShift,
    BLStatus_NeedWrapDetermination,
};

struct Buffer_Layout_Stop{
    u32 status;
    i32 line_index;
    i32 wrap_line_index;
    i32 pos;
    i32 next_line_pos;
    f32 x;
};

struct Buffer_Measure_Wrap_Params{
    Gap_Buffer *buffer;
    i32 *wrap_line_index;
    System_Functions *system;
    Font_Pointers font;
    b32 virtual_white;
};

struct Buffer_Measure_Wrap_State{
    Gap_Buffer_Stream stream;
    i32 i;
    i32 size;
    b32 still_looping;
    
    i32 line_index;
    
    i32 current_wrap_index;
    f32 current_adv;
    f32 x;
    
    b32 skipping_whitespace;
    i32 wrap_unit_end;
    b32 did_wrap;
    b32 first_of_the_line;
    
    Translation_State tran;
    Translation_Emits emits;
    u32 J;
    Buffer_Model_Step step;
    Buffer_Model_Behavior behavior;
    
    i32 __pc__;
};

struct Buffer_Cursor_Seek_Params{
    Gap_Buffer *buffer;
    Buffer_Seek seek;
    System_Functions *system;
    Font_Pointers font;
    i32 *wrap_line_index;
    i32 *character_starts;
    b32 virtual_white;
    b32 return_hint;
    Full_Cursor *cursor_out;
};

struct Buffer_Cursor_Seek_State{
    Full_Cursor next_cursor;
    Full_Cursor this_cursor;
    Full_Cursor prev_cursor;
    
    Gap_Buffer_Stream stream;
    b32 still_looping;
    i32 i;
    i32 size;
    i32 wrap_unit_end;
    
    b32 first_of_the_line;
    b32 xy_seek;
    f32 ch_width;
    
    i32 font_height;
    
    Translation_State tran;
    Translation_Emits emits;
    u32 J;
    Buffer_Model_Step step;
    Buffer_Model_Behavior behavior;
    
    
    i32 __pc__;
};

struct Buffer_Invert_Batch{
    i32 i;
    i32 shift_amount;
    i32 len;
};

enum Buffer_Render_Flag{
    BRFlag_Special_Character = (1 << 0),
    BRFlag_Ghost_Character = (1 << 1)
};

struct Buffer_Render_Item{
    i32 index;
    u32 codepoint;
    u32 flags;
    f32 x0, y0;
    f32 x1, y1;
};

struct Render_Item_Write{
    Buffer_Render_Item *item;
    Buffer_Render_Item *item_end;
    f32 x;
    f32 y;
    System_Functions *system;
    Font_Pointers font;
    i32 font_height;
    f32 x_min;
    f32 x_max;
};

struct Buffer_Render_Params{
    Gap_Buffer *buffer;
    Buffer_Render_Item *items;
    i32 max;
    i32 *count;
    f32 port_x;
    f32 port_y;
    f32 clip_w;
    f32 scroll_x;
    f32 scroll_y;
    f32 width;
    f32 height;
    Full_Cursor start_cursor;
    i32 wrapped;
    System_Functions *system;
    Font_Pointers font;
    b32 virtual_white;
    i32 wrap_slashes;
};

struct Buffer_Render_State{
    Gap_Buffer_Stream stream;
    b32 still_looping;
    i32 i;
    i32 size;
    
    f32 shift_x;
    f32 shift_y;
    
    f32 ch_width;
    
    Render_Item_Write write;
    f32 byte_advance;
    
    i32 line;
    i32 wrap_line;
    b8 skipping_whitespace;
    b8 first_of_the_line;
    b8 first_of_the_wrap;
    i32 wrap_unit_end;
    
    Translation_State tran;
    Translation_Emits emits;
    u32 J;
    Buffer_Model_Step step;
    Buffer_Model_Behavior behavior;
    
    i32 __pc__;
};

#endif

// BOTTOM
