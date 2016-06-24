
#ifndef FCODER_CUSTOM_H
#define FCODER_CUSTOM_H

#include "4coder_version.h"
#include "4coder_keycodes.h"
#include "4coder_style.h"
#include "4coder_rect.h"
#include "4coder_mem.h"
#include "4coder_buffer_types.h"
#include "4coder_gui.h"

#define MDFR_NONE 0x0
#define MDFR_CTRL 0x1
#define MDFR_ALT 0x2
#define MDFR_SHIFT 0x4

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

// NOTE(allen|a4.0.8): This is used to identify which buffer
// an operation should work on when you might want to
// identify it by id or by name.
typedef struct Buffer_Identifier{
    char *name;
    int name_len;
    int id;
} Buffer_Identifier;

enum Command_ID{
    cmdid_null,
    
    cmdid_center_view,
    cmdid_left_adjust_view,
    cmdid_page_up,
    cmdid_page_down,
    
    cmdid_word_complete,
    
    cmdid_undo,
    cmdid_redo,
    cmdid_history_backward,
    cmdid_history_forward,
    
    cmdid_to_uppercase,
    cmdid_to_lowercase,
    
    cmdid_toggle_line_wrap,
    cmdid_toggle_show_whitespace,
    cmdid_clean_all_lines,
    cmdid_eol_dosify,
    cmdid_eol_nixify,
    
    cmdid_hide_scrollbar,
    cmdid_show_scrollbar,
    
    cmdid_interactive_new,
    cmdid_interactive_open,
    cmdid_reopen,
    cmdid_save,
    cmdid_save_as,
    cmdid_interactive_switch_buffer,
    cmdid_interactive_kill_buffer,
    cmdid_kill_buffer,
    
    cmdid_open_color_tweaker,
    cmdid_open_config,
    cmdid_open_menu,
    cmdid_open_debug,
    
    cmdid_open_panel_vsplit,
    cmdid_open_panel_hsplit,
    cmdid_close_panel,
    cmdid_change_active_panel,
    
    //
    cmdid_count
};

// These are regular hooks, any of them can be set to any function
// that matches the HOOK_SIG pattern.
enum Hook_ID{
    hook_start,
    hook_file_out_of_sync,
    // never below this
    hook_type_count
};

// These are for special hooks, each must bind to specialized signatures
// that do not necessarily have access to the app pointer.
enum Special_Hook_ID{
    _hook_scroll_rule = hook_type_count,
    _hook_new_file,
    _hook_open_file,
};

// None of the members of *_Summary structs nor any of the data pointed
// to by the members should be modified, I would have made them all
// const... but that causes a lot problems for C++ reasons.
struct Buffer_Summary{
    int exists;
    int ready;
    int buffer_id;
    unsigned int lock_flags;
    
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
    unsigned int lock_flags;
    
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

struct Event_Message{
    int type;
};

struct Theme_Color{
    Style_Tag tag;
    unsigned int color;
};

// Flags and enum values
// TODO(allen): auto generate this and the docs for it
enum User_Input_Type_ID{
    UserInputNone,
    UserInputKey,
    UserInputMouse
};

enum Event_Message_Type_ID{
    EventMessage_NoMessage,
    EventMessage_OpenView,
    EventMessage_Frame,
    EventMessage_CloseView
};

enum Buffer_Setting_ID{
    BufferSetting_Null,
    BufferSetting_Lex,
    BufferSetting_WrapLine,
    BufferSetting_MapID,
};

enum Buffer_Kill_Flag{
    BufferKill_Background  = 0x1,
    BufferKill_AlwaysKill  = 0x2,
};

enum Buffer_Create_Flag{
    BufferCreate_Background = 0x1,
    BufferCreate_AlwaysNew  = 0x2,
};

enum Access_Flag{
    AccessOpen      = 0x0,
    AccessProtected = 0x1,
    AccessHidden    = 0x2,
    AccessAll       = 0xFF
};

enum Seek_Boundry_Flag{
    BoundryWhitespace   = 0x1,
    BoundryToken        = 0x2,
    BoundryAlphanumeric = 0x4,
    BoundryCamelCase    = 0x8
};

enum Command_Line_Input_Flag{
    CLI_OverlapWithConflict = 0x1,
    CLI_AlwaysBindToView    = 0x2,
    CLI_CursorAtEnd         = 0x4,
};

enum Auto_Indent_Flag{
    AutoIndent_ClearLine = 0x1,
    AutoIndent_UseTab    = 0x2
};

enum Set_Buffer_Flag{
    SetBuffer_KeepOriginalGUI = 0x1
};

enum Input_Type_Flag{
    EventOnAnyKey      = 0x1,
    EventOnEsc         = 0x2,
    EventOnLeftButton  = 0x4,
    EventOnRightButton = 0x8,
    EventOnWheel       = 0x10,
    EventOnButton      = (EventOnLeftButton | EventOnRightButton | EventOnWheel),
    
    // NOTE(allen): These don't work so much, so let's pretend they're not here for now.
    EventOnMouseMove   = 0x20,
    EventOnMouse       = (EventOnButton | EventOnMouseMove),
    
    EventAll           = 0xFF
};

#define VIEW_ROUTINE_SIG(name) void name(struct Application_Links *app, int view_id)
#define GET_BINDING_DATA(name) int name(void *data, int size)
#define CUSTOM_COMMAND_SIG(name) void name(struct Application_Links *app)
#define HOOK_SIG(name) int name(struct Application_Links *app)
#define OPEN_FILE_HOOK_SIG(name) int name(struct Application_Links *app, int buffer_id)
#define SCROLL_RULE_SIG(name) int name(float target_x, float target_y, float *scroll_x, float *scroll_y, int view_id, int is_new_target, float dt)

extern "C"{
    typedef VIEW_ROUTINE_SIG(View_Routine_Function);
    typedef CUSTOM_COMMAND_SIG(Custom_Command_Function);
    typedef GET_BINDING_DATA(Get_Binding_Data_Function);
    typedef HOOK_SIG(Hook_Function);
    
    typedef OPEN_FILE_HOOK_SIG(Open_File_Hook_Function);
    typedef SCROLL_RULE_SIG(Scroll_Rule_Function);
}

struct Application_Links;
#include "4coder_custom_api.h"



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
