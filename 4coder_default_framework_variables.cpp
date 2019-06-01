/*
4coder_default_framework_variables.cpp - Declares the global variables used by the framework for
the default 4coder behavior.
*/

// TOP

static Named_Mapping *named_maps = 0;
static i32 named_map_count = 0;

static b32 allow_immediate_close_without_checking_for_changes = false;

static char *default_extensions[] = {
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
static b32 auto_center_after_jumps = AUTO_CENTER_AFTER_JUMPS;
static u8 locked_buffer_space[256];
static String_Const_u8 locked_buffer = {};


static View_ID build_footer_panel_view_id = 0;


static Managed_Variable_ID view_rewrite_loc = 0;
static Managed_Variable_ID view_next_rewrite_loc = 0;
static Managed_Variable_ID view_paste_index_loc = 0;
static Managed_Variable_ID view_is_passive_loc = 0;
static Managed_Variable_ID view_snap_mark_to_cursor = 0;
static Managed_Variable_ID view_ui_data = 0;

static u8 out_buffer_space[1024];
static u8 command_space[1024];
static char hot_directory_space[1024];


static b32 highlight_line_at_cursor = true;
static b32 do_matching_enclosure_highlight = true;
static b32 do_matching_paren_highlight = true;
static b32 do_colored_comment_keywords = true;
static b32 suppressing_mouse = false;

static b32 cursor_is_hidden = false;

static b32 show_fps_hud = false;

static Heap global_heap;

enum{
    FCoderMode_Original = 0,
    FCoderMode_NotepadLike = 1,
};
static i32 fcoder_mode = FCoderMode_Original;

static ID_Line_Column_Jump_Location prev_location = {};


static Arena global_config_arena = {};
static Config_Data global_config = {};

static char previous_isearch_query[256] = {};

// BOTTOM

