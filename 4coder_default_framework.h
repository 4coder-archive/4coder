/*
4coder_default_framework.cpp - Sets up the basics of the framework that is used for default 4coder behaviour.
*/

// TOP

#if !defined(FCODER_DEFAULT_FRAMEWORK_H)
#define FCODER_DEFAULT_FRAMEWORK_H

enum Default_Maps{
    default_code_map,
    default_lister_ui_map,
    default_maps_count,
};

////////////////////////////////

enum Rewrite_Type{
    RewriteNone,
    RewritePaste,
    RewriteWordComplete
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
    i32 pos;
};

struct Name_Line_Column_Location{
    String file;
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

struct Named_Mapping{
    String name;
    Custom_Command_Function *remap_command;
};

////////////////////////////////

static void
do_gui_sure_to_kill(Application_Links *app, Buffer_Summary *buffer, View_Summary *view);

static void
do_gui_sure_to_close_4coder(Application_Links *app, View_Summary *view);

#endif

// BOTTOM

