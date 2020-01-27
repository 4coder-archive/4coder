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
    Buffer_Scroll scroll;
    i64 cursor_pos;
};

struct Editing_File_Settings{
    Layout_Function *layout_func;
    Face_ID face_id;
    b8 dos_write_mode;
    b8 is_initialized;
    b8 unimportant;
    b8 read_only;
    b8 unkillable;
    b8 never_kill;
};

struct Line_Layout_Key{
    Face_ID face_id;
    i32 face_version_number;
    f32 width;
    i64 line_number;
};

typedef i32 File_Save_State;
enum{
    FileSaveState_Normal,
    FileSaveState_SavedWaitingForNotification,
};

struct Editing_File_State{
    Gap_Buffer buffer;
    
    History history;
    i32 current_record_index;
    
    Dirty_State dirty;
    File_Save_State save_state;
    
    File_Edit_Positions edit_pos_most_recent;
    File_Edit_Positions edit_pos_stack[16];
    i32 edit_pos_stack_top;
    
    Child_Process_ID attached_child_process;
    
    Arena cached_layouts_arena;
    Table_Data_u64 line_layout_table;
};

struct Editing_File_Name{
    u8 name_space[256];
    u64 name_size;
};

struct Editing_File{
    union{
        Editing_File *next;
        Node main_chain_node;
    };
    Node touch_node;
    Node external_mod_node;
    Buffer_ID id;
    Editing_File_Settings settings;
    Editing_File_State state;
    File_Attributes attributes;
    Lifetime_Object *lifetime_object;
    Editing_File_Name base_name;
    Editing_File_Name unique_name;
    Editing_File_Name canon;
};

struct Buffer_Point_Delta{
    Buffer_Point new_point;
    f32 y_shift;
};

#endif

// BOTTOM

