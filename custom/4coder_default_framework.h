/*
4coder_default_framework.cpp - Sets up the basics of the framework that is used for default 4coder behaviour.
*/

// TOP

#if !defined(FCODER_DEFAULT_FRAMEWORK_H)
#define FCODER_DEFAULT_FRAMEWORK_H

////////////////////////////////

typedef i64 Rewrite_Type;
enum{
    Rewrite_None,
    Rewrite_Paste,
    Rewrite_WordComplete
};

////////////////////////////////

struct ID_Line_Column_Jump_Location{
    Buffer_ID buffer_id;
    i32 line;
    i32 column;
};
typedef ID_Line_Column_Jump_Location ID_Based_Jump_Location;

struct ID_Pos_Jump_Location{
    Buffer_ID buffer_id;
    i64 pos;
};

struct Name_Line_Column_Location{
    String_Const_u8 file;
    i32 line;
    i32 column;
};

struct Parsed_Jump{
    b32 success;
    Name_Line_Column_Location location;
    i32 colon_position;
    b32 is_sub_jump;
    b32 sub_jump_indented;
    b32 sub_jump_note;
    b32 is_ms_style;
    b32 has_rust_arrow;
};

struct ID_Pos_Jump_Location_Array{
    struct ID_Pos_Jump_Location *jumps;
    i32 count;
};

////////////////////////////////

typedef i32 Fallback_Dispatch_Result_Code;
enum{
    FallbackDispatch_Unhandled,
    FallbackDispatch_DidCall,
    FallbackDispatch_DelayedUICall,
};

struct Fallback_Dispatch_Result{
    Fallback_Dispatch_Result_Code code;
    Custom_Command_Function *func;
};

////////////////////////////////

typedef void View_Render_Hook(Application_Links *app, View_ID view, Frame_Info frame_info, Rect_f32 inner);

////////////////////////////////

function b32
do_buffer_kill_user_check(Application_Links *app, Buffer_ID buffer, View_ID view);

function b32
do_4coder_close_user_check(Application_Links *app, View_ID view);

////////////////////////////////

struct Buffer_Modified_Node{
    Buffer_Modified_Node *next;
    Buffer_Modified_Node *prev;
    Buffer_ID buffer;
};

struct Buffer_Modified_Set{
    Arena arena;
    Buffer_Modified_Node *free;
    Buffer_Modified_Node *first;
    Buffer_Modified_Node *last;
    Table_u64_u64 id_to_node;
};

////////////////////////////////

struct Fade_Range{
    Fade_Range *next;
    union{
        Buffer_ID buffer_id;
        View_ID view_id;
    };
    f32 t;
    f32 full_t;
    Range_i64 range;
    ARGB_Color color;
};

struct Fade_Range_List{
    Fade_Range *first;
    Fade_Range *last;
    i32 count;
};

////////////////////////////////

function void
setup_built_in_mapping(Application_Links *app, String_Const_u8 name, Mapping *mapping, i64 global_id, i64 file_id, i64 code_id);

#endif

// BOTTOM

