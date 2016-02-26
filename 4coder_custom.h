
#include "4coder_keycodes.h"
#include "4coder_buffer_types.h"

#define MDFR_NONE 0
#define MDFR_CTRL 1
#define MDFR_ALT 2
#define MDFR_SHIFT 4

typedef unsigned char Code;

enum Command_ID{
    cmdid_null,
    cmdid_write_character,
    cmdid_seek_whitespace_right,
    cmdid_seek_whitespace_left,
    cmdid_seek_whitespace_up,
    cmdid_seek_whitespace_down,
    cmdid_seek_token_left,
    cmdid_seek_token_right,
    cmdid_seek_white_or_token_left,
    cmdid_seek_white_or_token_right,
    cmdid_seek_alphanumeric_left,
    cmdid_seek_alphanumeric_right,
    cmdid_seek_alphanumeric_or_camel_left,
    cmdid_seek_alphanumeric_or_camel_right,
    cmdid_search,
    cmdid_reverse_search,
    cmdid_word_complete,
    cmdid_goto_line,
    cmdid_set_mark,
    cmdid_copy,
    cmdid_cut,
    cmdid_paste,
    cmdid_paste_next,
    cmdid_delete_range,
    cmdid_timeline_scrub,
    cmdid_undo,
    cmdid_redo,
    cmdid_increase_rewind_speed,
    cmdid_increase_fastforward_speed,
    cmdid_stop_rewind_fastforward,
    cmdid_history_backward,
    cmdid_history_forward,
    cmdid_interactive_new,
    cmdid_interactive_open,
    cmdid_reopen,
    cmdid_save,
    cmdid_interactive_save_as,
    cmdid_change_active_panel,
    cmdid_interactive_switch_buffer,
    cmdid_interactive_kill_buffer,
    cmdid_kill_buffer,
    cmdid_toggle_line_wrap,
    cmdid_toggle_endline_mode,
    cmdid_to_uppercase,
    cmdid_to_lowercase,
    cmdid_toggle_show_whitespace,
    cmdid_clean_all_lines,
    cmdid_eol_dosify,
    cmdid_eol_nixify,
    cmdid_auto_tab_range,
    cmdid_auto_tab_line_at_cursor,
    cmdid_auto_tab_whole_file,
    cmdid_open_panel_vsplit,
    cmdid_open_panel_hsplit,
    cmdid_close_panel,
    cmdid_move_left,
    cmdid_move_right,
    cmdid_delete,
    cmdid_backspace,
    cmdid_move_up,
    cmdid_move_down,
    cmdid_seek_end_of_line,
    cmdid_seek_beginning_of_line,
    cmdid_page_up,
    cmdid_page_down,
    cmdid_open_color_tweaker,
    cmdid_close_minor_view,
    cmdid_cursor_mark_swap,
    cmdid_open_menu,
    cmdid_set_settings,
    cmdid_build,
    //
    cmdid_count
};

enum Param_ID{
    par_range_start,
    par_range_end,
    par_name,
    par_lex_as_cpp_file,
    par_wrap_lines,
    par_key_mapid,
    par_cli_path,
    par_cli_command,
    par_cli_overlap_with_conflict,
    par_cli_always_bind_to_view,
    par_clear_blank_lines,
    // never below this
    par_type_count
};

enum Hook_ID{
    hook_start,
    hook_open_file,
    // never below this
    hook_type_count
};

enum Dynamic_Type{
    dynamic_type_int,
    dynamic_type_string,
    // never below this
    dynamic_type_count
};

struct Dynamic{
    int type;
    union{
        struct{
            int str_len;
            char *str_value;
        };
        int int_value;
    };
};

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

// NOTE(allen): None of the members of *_Summary structs nor any of the
// data pointed to by the members should be modified, I would have made
// them all const... but that causes a lot problems for C++ reasons.
struct Buffer_Summary{
    int exists;
    int ready;
    int file_id;
    
    int size;
    
    int file_name_len;
    int buffer_name_len;
    char *file_name;
    char *buffer_name;
    
    int file_cursor_pos;
    int is_lexed;
    int map_id;
};

struct File_View_Summary{
    int exists;
    int view_id;
    int file_id;
    
    Full_Cursor cursor;
    Full_Cursor mark;
    float preferred_x;
    int line_height;
    int unwrapped_lines;
};

#ifndef FRED_STRING_STRUCT
#define FRED_STRING_STRUCT
struct String{
    char *str;
    int size;
    int memory_size;
};
#endif

#define GET_BINDING_DATA(name) int name(void *data, int size)
#define SET_EXTRA_FONT_SIG(name) void name(Extra_Font *font_out)
#define CUSTOM_COMMAND_SIG(name) void name(struct Application_Links *app)
#define HOOK_SIG(name) void name(struct Application_Links *app)

extern "C"{
    typedef CUSTOM_COMMAND_SIG(Custom_Command_Function);
    typedef GET_BINDING_DATA(Get_Binding_Data_Function);
    typedef HOOK_SIG(Hook_Function);
}

struct Application_Links;

// Command exectuion
#define PUSH_PARAMETER_SIG(name) void name(Application_Links *context, Dynamic param, Dynamic value)
#define PUSH_MEMORY_SIG(name) char* name(Application_Links *context, int len)
#define EXECUTE_COMMAND_SIG(name) void name(Application_Links *context, int command_id)
#define CLEAR_PARAMETERS_SIG(name) void name(Application_Links *context)

// File system navigation
#define DIRECTORY_GET_HOT_SIG(name) int name(Application_Links *context, char *buffer, int max)
#define DIRECTORY_HAS_FILE_SIG(name) int name(Application_Links *context, String dir, char *filename)
#define DIRECTORY_CD_SIG(name) int name(Application_Links *context, String *dir, char *rel_path)

// Direct buffer manipulation
#define GET_BUFFER_MAX_INDEX_SIG(name) int name(Application_Links *context)
#define GET_BUFFER_SIG(name) Buffer_Summary name(Application_Links *context, int index)
#define GET_ACTIVE_BUFFER_SIG(name) Buffer_Summary name(Application_Links *context)
#define GET_BUFFER_BY_NAME(name) Buffer_Summary name(Application_Links *context, String filename)

#define REFRESH_BUFFER_SIG(name) int name(Application_Links *context, Buffer_Summary *buffer)
#define BUFFER_SEEK_DELIMITER_SIG(name) int name(Application_Links *context, Buffer_Summary *buffer, int start, char delim, int seek_forward, int *out)
#define BUFFER_READ_RANGE_SIG(name) int name(Application_Links *context, Buffer_Summary *buffer, int start, int end, char *out)
#define BUFFER_REPLACE_RANGE_SIG(name) int name(Application_Links *context, Buffer_Summary *buffer, int start, int end, char *str, int len)
// TODO(allen): buffer save

// File view manipulation
#define GET_VIEW_MAX_INDEX_SIG(name) int name(Application_Links *context)
#define GET_FILE_VIEW_SIG(name) File_View_Summary name(Application_Links *context, int index)
#define GET_ACTIVE_FILE_VIEW_SIG(name) File_View_Summary name(Application_Links *context)

#define REFRESH_FILE_VIEW_SIG(name) int name(Application_Links *context, File_View_Summary *view)
#define VIEW_SET_CURSOR_SIG(name) int name(Application_Links *context, File_View_Summary *view, Buffer_Seek seek, int set_preferred_x)
#define VIEW_SET_MARK_SIG(name) int name(Application_Links *context, File_View_Summary *view, Buffer_Seek seek)
#define VIEW_SET_FILE_SIG(name) int name(Application_Links *context, File_View_Summary *view, int file_id)

extern "C"{
    // Command exectuion
    typedef EXECUTE_COMMAND_SIG(Exec_Command_Function);
    typedef PUSH_PARAMETER_SIG(Push_Parameter_Function);
    typedef PUSH_MEMORY_SIG(Push_Memory_Function);
    typedef CLEAR_PARAMETERS_SIG(Clear_Parameters_Function);
    
    // File system navigation
    typedef DIRECTORY_GET_HOT_SIG(Directory_Get_Hot);
    typedef DIRECTORY_HAS_FILE_SIG(Directory_Has_File);
    typedef DIRECTORY_CD_SIG(Directory_CD);
    
    // Buffer manipulation
    typedef GET_BUFFER_MAX_INDEX_SIG(Get_Buffer_Max_Index_Function);
    typedef GET_BUFFER_SIG(Get_Buffer_Function);
    typedef GET_ACTIVE_BUFFER_SIG(Get_Active_Buffer_Function);
    typedef GET_BUFFER_BY_NAME(Get_Buffer_By_Name_Function);
    
    typedef REFRESH_BUFFER_SIG(Refresh_Buffer_Function);
    typedef BUFFER_SEEK_DELIMITER_SIG(Buffer_Seek_Delimiter_Function);
    typedef BUFFER_READ_RANGE_SIG(Buffer_Read_Range_Function);
    typedef BUFFER_REPLACE_RANGE_SIG(Buffer_Replace_Range_Function);
    
    // View manipulation
    typedef GET_VIEW_MAX_INDEX_SIG(Get_View_Max_Index_Function);
    typedef GET_FILE_VIEW_SIG(Get_File_View_Function);
    typedef GET_ACTIVE_FILE_VIEW_SIG(Get_Active_File_View_Function);
    
    typedef REFRESH_FILE_VIEW_SIG(Refresh_File_View_Function);
    typedef VIEW_SET_CURSOR_SIG(View_Set_Cursor_Function);
    typedef VIEW_SET_MARK_SIG(View_Set_Mark_Function);
    typedef VIEW_SET_FILE_SIG(View_Set_File_Function);
}

struct Application_Links{
    // Command exectuion
    Exec_Command_Function *exec_command_keep_stack;
    Push_Parameter_Function *push_parameter;
    Push_Memory_Function *push_memory;
    Clear_Parameters_Function *clear_parameters;
    
    // File system navigation
    Directory_Get_Hot *directory_get_hot;
    Directory_Has_File *directory_has_file;
    Directory_CD *directory_cd;
    
    // Buffer manipulation
    Get_Buffer_Max_Index_Function *get_buffer_max_index;
    Get_Buffer_Function *get_buffer;
    Get_Active_Buffer_Function *get_active_buffer;
    Get_Buffer_By_Name_Function *get_buffer_by_name;
    
    Refresh_Buffer_Function *refresh_buffer;
    Buffer_Seek_Delimiter_Function *buffer_seek_delimiter;
    Buffer_Read_Range_Function *buffer_read_range;
    Buffer_Replace_Range_Function *buffer_replace_range;
    
    // View manipulation
    Get_View_Max_Index_Function *get_view_max_index;
    Get_File_View_Function *get_file_view;
    Get_Active_File_View_Function *get_active_file_view;
    
    Refresh_File_View_Function *refresh_file_view;
    View_Set_Cursor_Function *view_set_cursor;
    View_Set_Mark_Function *view_set_mark;
    View_Set_File_Function *view_set_file;
    
    // Application data ptr
    void *data;
};

struct Custom_API{
    Get_Binding_Data_Function *get_bindings;
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
        
        struct{ int mapid; int bind_count; } map_begin;
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
            Custom_Command_Function *func;
        } hook;
    };
};


