
#ifndef FCODER_CUSTOM_H
#define FCODER_CUSTOM_H

#include "4coder_version.h"
#include "4coder_keycodes.h"
#include "4coder_style.h"
#include "4coder_rect.h"
#include "4coder_mem.h"
#include "4coder_buffer_types.h"
#include "4coder_gui.h"

#ifndef FRED_STRING_STRUCT
#define FRED_STRING_STRUCT
typedef struct String{
    char *str;
    int size;
    int memory_size;
} String;

typedef struct Offset_String{
    int offset;
    int size;
} Offset_String;
#endif

typedef unsigned char Code;

typedef enum{
	MDFR_SHIFT_INDEX,
	MDFR_CONTROL_INDEX,
	MDFR_ALT_INDEX,
    MDFR_CAPS_INDEX,
    MDFR_HOLD_INDEX,
	// always last
	MDFR_INDEX_COUNT
} Key_Control;

typedef struct Key_Event_Data{
	Code keycode;
	Code character;
	Code character_no_caps_lock;
    
	char modifiers[MDFR_INDEX_COUNT];
} Key_Event_Data;
inline Key_Event_Data
key_event_data_zero(){
    Key_Event_Data data={0};
    return(data);
}

typedef struct Mouse_State{
	char l, r;
    char press_l, press_r;
	char release_l, release_r;
	char wheel;
	char out_of_window;
	int x, y;
} Mouse_State;


typedef union Range{
    struct{
        int min, max;
    };
    struct{
        int start, end;
    };
} Range;

inline Range
make_range(int p1, int p2){
    Range range;
    if (p1 < p2){
        range.min = p1;
        range.max = p2;
    }
    else{
        range.min = p2;
        range.max = p1;
    }
    return(range);
}


typedef enum Dynamic_Type{
    dynamic_type_int,
    dynamic_type_string,
    // never below this
    dynamic_type_count
} Dynamic_Type;

typedef struct Dynamic{
    int type;
    union{
        struct{
            int str_len;
            char *str_value;
        };
        int int_value;
    };
} Dynamic;

inline Dynamic
dynamic_int(int x){
    Dynamic result;
    result.type = dynamic_type_int;
    result.int_value = x;
    return result;
}

inline Dynamic
dynamic_string(const char *string, int len){
    Dynamic result;
    result.type = dynamic_type_string;
    result.str_len = len;
    result.str_value = (char*)(string);
    return result;
}

inline int
dynamic_to_int(Dynamic *dynamic){
    int result = 0;
    if (dynamic->type == dynamic_type_int){
        result = dynamic->int_value;
    }
    return result;
}

inline char*
dynamic_to_string(Dynamic *dynamic, int *len){
    char *result = 0;
    if (dynamic->type == dynamic_type_string){
        result = dynamic->str_value;
        *len = dynamic->str_len;
    }
    return result;
}

inline int
dynamic_to_bool(Dynamic *dynamic){
    int result = 0;
    if (dynamic->type == dynamic_type_int){
        result = (dynamic->int_value != 0);
    }
    else{
        result = 1;
    }
    return result;
}

typedef struct File_Info{
    String filename;
    int folder;
} File_Info;

typedef struct File_List{
    // Ignore this, it's for internal stuff.
    void *block;
    
    // The list of files and folders.
    File_Info *infos;
    int count;
    
    // Ignore this, it's for internal stuff.
    int block_size;
} File_List;

#define MDFR_NONE 0x0
#define MDFR_CTRL 0x1
#define MDFR_ALT 0x2
#define MDFR_SHIFT 0x4

enum Command_ID{
    cmdid_null,
    
    cmdid_seek_left,
    cmdid_seek_right,
    
    cmdid_center_view,
    cmdid_left_adjust_view,
    
    cmdid_word_complete,
    
    cmdid_copy,
    cmdid_cut,
    cmdid_paste,
    cmdid_paste_next,
    
    cmdid_undo,
    cmdid_redo,
    cmdid_history_backward,
    cmdid_history_forward,
    
    cmdid_interactive_new,
    cmdid_interactive_open,
    cmdid_reopen,
    cmdid_save,
    cmdid_interactive_switch_buffer,
    cmdid_interactive_kill_buffer,
    cmdid_kill_buffer,
    
    cmdid_to_uppercase,
    cmdid_to_lowercase,
    
    cmdid_toggle_line_wrap,
    cmdid_toggle_show_whitespace,
    
    cmdid_clean_all_lines,
    cmdid_auto_tab_range,
    cmdid_eol_dosify,
    cmdid_eol_nixify,
    
    cmdid_open_panel_vsplit,
    cmdid_open_panel_hsplit,
    cmdid_close_panel,
    cmdid_change_active_panel,
    
    cmdid_page_up,
    cmdid_page_down,
    
    cmdid_open_color_tweaker,
    cmdid_open_config,
    cmdid_open_menu,
    cmdid_open_debug,
    
    cmdid_hide_scrollbar,
    cmdid_show_scrollbar,
    
    cmdid_set_settings,
    
    cmdid_command_line,
    //
    cmdid_count
};

enum Param_ID{
    par_range_start,
    par_range_end,
    par_name,
    par_buffer_id,
    par_do_in_background,
    par_flags,
    par_lex_as_cpp_file,
    par_wrap_lines,
    par_key_mapid,
    par_show_whitespace,
    par_cli_path,
    par_cli_command,
    par_clear_blank_lines,
    par_use_tabs,
    par_save_update_name,
    
    // never below this
    par_type_count
};

#define CLI_OverlapWithConflict 0x1
#define CLI_AlwaysBindToView 0x2

// These are regular hooks, any of them can be set to any function
// that matches the HOOK_SIG pattern.
enum Hook_ID{
    hook_start,
    hook_open_file,
    hook_new_file,
    hook_file_out_of_sync,
    // never below this
    hook_type_count
};

// These are for special hooks, each must bind to specialized signatures
// that do not necessarily have access to the app pointer.
enum Special_Hook_ID{
    _hook_scroll_rule = hook_type_count,
};

// None of the members of *_Summary structs nor any of the data pointed
// to by the members should be modified, I would have made them all
// const... but that causes a lot problems for C++ reasons.
struct Buffer_Summary{
    int exists;
    int ready;
    int buffer_id;
    
    int size;
    
    int file_name_len;
    int buffer_name_len;
    char *file_name;
    char *buffer_name;
    
    int buffer_cursor_pos;
    int is_lexed;
    int map_id;
};
inline Buffer_Summary
buffer_summary_zero(){
    Buffer_Summary summary={0};
    return(summary);
}

struct View_Summary{
    int exists;
    int view_id;
    int buffer_id;
    int locked_buffer_id;
    int hidden_buffer_id;
    
    Full_Cursor cursor;
    Full_Cursor mark;
    float preferred_x;
    float line_height;
    int unwrapped_lines;
    int show_whitespace;
    
    i32_Rect file_region;
    GUI_Scroll_Vars scroll_vars;
};
inline View_Summary
view_summary_zero(){
    View_Summary summary={0};
    return(summary);
}

enum User_Input_Type{
    UserInputNone,
    UserInputKey,
    UserInputMouse
};
struct User_Input{
    int type;
    int abort;
    union{
        Key_Event_Data key;
        Mouse_State mouse;
    };
    unsigned long long command;
};

#define CommandEqual(c1,c2) ((unsigned long long)(c1) == (unsigned long long)(c2))

struct Query_Bar{
    String prompt;
    String string;
};

enum Event_Message_Type{
    EM_No_Message,
    EM_Open_View,
    EM_Frame,
    EM_Close_View
};
struct Event_Message{
    int type;
};

struct Theme_Color{
    Style_Tag tag;
    unsigned int color;
};


#define VIEW_ROUTINE_SIG(name) void name(struct Application_Links *app, int view_id)
#define GET_BINDING_DATA(name) int name(void *data, int size)
#define CUSTOM_COMMAND_SIG(name) void name(struct Application_Links *app)
#define HOOK_SIG(name) int name(struct Application_Links *app)
#define SCROLL_RULE_SIG(name) int name(float target_x, float target_y, float *scroll_x, float *scroll_y, int view_id, int is_new_target, float dt)

extern "C"{
    typedef VIEW_ROUTINE_SIG(View_Routine_Function);
    typedef CUSTOM_COMMAND_SIG(Custom_Command_Function);
    typedef GET_BINDING_DATA(Get_Binding_Data_Function);
    typedef HOOK_SIG(Hook_Function);
    typedef SCROLL_RULE_SIG(Scroll_Rule_Function);
}

struct Application_Links;
#include "4coder_custom_api.h"


// Boundry type flags
#define BoundryWhitespace 0x1
#define BoundryToken 0x2
#define BoundryAlphanumeric 0x4
#define BoundryCamelCase 0x8



// Input type flags
#define EventOnAnyKey 0x1
#define EventOnEsc 0x2
#define EventOnLeftButton 0x4
#define EventOnRightButton 0x8
#define EventOnWheel 0x10
#define EventOnButton (EventOnLeftButton | EventOnRightButton | EventOnWheel)

// NOTE(allen): These don't work so much, so let's pretend they're not here for now.
#define EventOnMouseMove 0x20
#define EventOnMouse (EventOnButton | EventOnMouseMove)



#define _GET_VERSION_SIG(n) int n(int maj, int min, int patch)
typedef _GET_VERSION_SIG(_Get_Version_Function);

extern "C" _GET_VERSION_SIG(get_alpha_4coder_version){
    int result = (maj == MAJOR && min == MINOR && patch == PATCH);
    return(result);
}

struct Custom_API{
    View_Routine_Function *view_routine;
    Get_Binding_Data_Function *get_bindings;
    _Get_Version_Function *get_alpha_4coder_version;
};

// NOTE(allen): definitions for the buffer that communicates to 4ed.exe

enum Binding_Unit_Type{
    unit_header,
    unit_map_begin,
    unit_binding,
    unit_callback,
    unit_inherit,
    unit_hook
};

enum Map_ID{
    mapid_global = (1 << 24),
    mapid_file,
    
    // NOTE(allen): mapid_nomap will remain empty even if you attempt to fill it
    // it is for setting a map's parent to nothing, in cases where you don't want
    // to inherit from global (which is the default).
    mapid_nomap
};

struct Binding_Unit{
    Binding_Unit_Type type;
    union{
        struct{ int total_size; int user_map_count; int error; } header;
        
        struct{ int mapid; int replace; int bind_count; } map_begin;
        struct{ int mapid; } map_inherit;
        struct{
            short code;
            unsigned char modifiers;
            int command_id;
        } binding;
        struct{
            short code;
            unsigned char modifiers;
            Custom_Command_Function *func;
        } callback;
        struct{
            int hook_id;
            void *func;
        } hook;
    };
};

#endif
