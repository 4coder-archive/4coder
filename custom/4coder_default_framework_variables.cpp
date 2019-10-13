/*
4coder_default_framework_variables.cpp - Declares the global variables used by the framework for
the default 4coder behavior.
*/

// TOP

global b32 allow_immediate_close_without_checking_for_changes = false;

global char *default_extensions[] = {
    "cpp",
    "hpp",
    "c",
    "h",
    "cc",
    "cs",
    "java",
    "rs",
    "glsl",
    "m",
};

#if !defined(AUTO_CENTER_AFTER_JUMPS)
#define AUTO_CENTER_AFTER_JUMPS true
#endif
global b32 auto_center_after_jumps = AUTO_CENTER_AFTER_JUMPS;
global u8 locked_buffer_space[256];
global String_Const_u8 locked_buffer = {};


global View_ID build_footer_panel_view_id = 0;


global Managed_ID view_rewrite_loc = 0;
global Managed_ID view_next_rewrite_loc = 0;
global Managed_ID view_paste_index_loc = 0;
global Managed_ID view_is_passive_loc = 0;
global Managed_ID view_snap_mark_to_cursor = 0;
global Managed_ID view_ui_data = 0;
global Managed_ID view_highlight_range = 0;
global Managed_ID view_highlight_buffer = 0;
global Managed_ID view_render_hook = 0;

global Managed_ID buffer_map_id = 0;

global Managed_ID sticky_jump_marker_handle = 0;

global Managed_ID attachment_tokens = 0;

global u8 out_buffer_space[1024];
global u8 command_space[1024];
global char hot_directory_space[1024];


global b32 highlight_line_at_cursor = true;
global b32 do_matching_enclosure_highlight = true;
global b32 do_matching_paren_highlight = true;
global b32 do_colored_comment_keywords = true;
global b32 suppressing_mouse = false;

global b32 show_fps_hud = false;

// TODO(allen): REMOVE THIS!
global Heap global_heap;

enum{
    FCoderMode_Original = 0,
    FCoderMode_NotepadLike = 1,
};
global i32 fcoder_mode = FCoderMode_Original;

global ID_Pos_Jump_Location prev_location = {};


global Arena *global_config_arena = {};
global Config_Data global_config = {};

global char previous_isearch_query[256] = {};

global Mapping framework_mapping = {};

enum{
    mapid_global = 1,
    mapid_file,
    default_code_map,
    default_lister_ui_map,
    default_log_graph_map,
    default_maps_count,
};

// BOTTOM

