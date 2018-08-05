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

struct ID_Based_Jump_Location{
    int32_t buffer_id;
    int32_t line;
    int32_t column;
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

