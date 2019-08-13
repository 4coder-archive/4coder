/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 24.01.2018
 *
 * Buffer types
 *
 */

// TOP

#if !defined(FRED_FILE_H)
#define FRED_FILE_H

typedef i32 Edit_Pos_Set_Type;
enum{
    EditPos_None,
    EditPos_CursorSet,
    EditPos_ScrollSet
};
struct File_Edit_Positions{
    Edit_Pos_Set_Type last_set_type;
    GUI_Scroll_Vars scroll;
    i64 cursor_pos;
};

// TODO(allen): do(replace Text_Effect with markers over time)
struct Text_Effect{
    i32 start;
    i32 end;
    u32 color;
    f32 seconds_down;
    f32 seconds_max;
};

struct Editing_File_Settings{
    i32 base_map_id;
    i32 display_width;
    i32 minimum_base_display_width;
    i32 wrap_indicator;
    Parse_Context_ID parse_context_id;
    b32 dos_write_mode;
    Face_ID font_id;
    b8 unwrapped_lines;
    b8 tokens_exist;
    b8 tokens_without_strings;
    b8 is_initialized;
    b8 unimportant;
    b8 read_only;
    b8 never_kill;
    b8 virtual_white;
};

struct Editing_File_State{
    Gap_Buffer buffer;
    
    i32 *wrap_line_index;
    i32 wrap_max;
    
    i32 *character_starts;
    i32 character_start_max;
    
    f32 *line_indents;
    i32 line_indent_max;
    
    i32 wrap_line_count;
    
    i32 *wrap_positions;
    i32 wrap_position_count;
    i32 wrap_position_max;
    
    History history;
    i32 current_record_index;
    
    Cpp_Token_Array token_array;
    b32 in_edit_handler;
    
    Text_Effect paste_effect;
    
    Dirty_State dirty;
    u32 ignore_behind_os;
    
    File_Edit_Positions edit_pos_most_recent;
    File_Edit_Positions edit_pos_stack[16];
    i32 edit_pos_stack_top;
    
    Child_Process_ID attached_child_process;
};

struct Editing_File_Name{
    u8 name_space[256];
    umem name_size;
};

struct Editing_File{
    union{
        Editing_File *next;
        Node main_chain_node;
    };
    Node touch_node;
    Node reloaded_node;
    Node edit_finished_mark_node;
    b32 edit_finished_marked;
    b32 is_loading;
    Buffer_ID id;
    Editing_File_Settings settings;
    Editing_File_State state;
    File_Attributes attributes;
    Lifetime_Object *lifetime_object;
    Editing_File_Name base_name;
    Editing_File_Name unique_name;
    Editing_File_Name canon;
};

#endif

// BOTTOM

