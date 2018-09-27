/*
4coder_default_framework_variables.cpp - Declares the global variables used by the framework for
the default 4coder behavior.
*/

// TOP

static Named_Mapping *named_maps = 0;
static int32_t named_map_count = 0;

static bool32 allow_immediate_close_without_checking_for_changes = false;

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
static bool32 auto_center_after_jumps = AUTO_CENTER_AFTER_JUMPS;
static char locked_buffer_space[256];
static String locked_buffer = make_fixed_width_string(locked_buffer_space);


static View_ID build_footer_panel_view_id = 0;


static Managed_Variable_ID view_rewrite_loc = 0;
static Managed_Variable_ID view_next_rewrite_loc = 0;
static Managed_Variable_ID view_paste_index_loc = 0;
static Managed_Variable_ID view_is_passive_loc = 0;


static char out_buffer_space[1024];
static char command_space[1024];
static char hot_directory_space[1024];


static bool32 highlight_line_at_cursor = false;
static bool32 suppressing_mouse = false;

enum{
    CursorRenderMode_Hidden = 0,
    CursorRenderMode_BlockCursorAndWireMark = 1,
    CursorRenderMode_IBarOrHighlightRange = 2,
};
static int32_t cursor_render_mode = CursorRenderMode_IBarOrHighlightRange;


static ID_Line_Column_Jump_Location prev_location = {0};


static Config_Data global_config = {0};

static char previous_isearch_query[256] = {0};

// BOTTOM

