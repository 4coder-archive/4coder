/*
4coder_config.h - Configuration structs.
*/

// TOP

#if !defined(FCODER_CONFIG_H)
#define FCODER_CONFIG_H

// TODO(allen): Stop handling files this way!  My own API should be able to do this!!?!?!?!!?!?!!!!?
// NOTE(allen): Actually need binary buffers for some stuff to work, but not this parsing thing here.
#include <stdio.h>

struct Error_Location{
    int32_t line_number;
    int32_t column_number;
};

struct Config_Error{
    Config_Error *next;
    Config_Error *prev;
    String file_name;
    char *pos;
    String text;
};

struct Config_Error_List{
    Config_Error *first;
    Config_Error *last;
    int32_t count;
};

struct Config_Parser{
    Cpp_Token *start;
    Cpp_Token *token;
    Cpp_Token *end;
    
    String file_name;
    String data;
    
    Partition *arena;
    
    Config_Error_List errors;
};

struct Config_LValue{
    String identifier;
    int32_t index;
};

typedef int32_t Config_RValue_Type;
enum{
    ConfigRValueType_LValue = 0,
    ConfigRValueType_Boolean = 1,
    ConfigRValueType_Integer = 2,
    ConfigRValueType_Float = 3,
    ConfigRValueType_String = 4,
    ConfigRValueType_Character = 5,
    ConfigRValueType_Compound = 6,
    ConfigRValueType_NoType = 7,
};
enum{
    ConfigRValueType_COUNT = ConfigRValueType_NoType,
};

struct Config_Compound{
    struct Config_Compound_Element *first;
    struct Config_Compound_Element *last;
    int32_t count;
};

struct Config_RValue{
    Config_RValue_Type type;
    union{
        Config_LValue *lvalue;
        bool32 boolean;
        int32_t integer;
        uint32_t uinteger;
        String string;
        char character;
        Config_Compound *compound;
    };
};

struct Config_Integer{
    bool32 is_signed;
    union{
        int32_t integer;
        uint32_t uinteger;
    };
};

typedef int32_t Config_Layout_Type;
enum{
    ConfigLayoutType_Unset = 0,
    ConfigLayoutType_Identifier = 1,
    ConfigLayoutType_Integer = 2,
    ConfigLayoutType_COUNT = 3,
};
struct Config_Layout{
    Config_Layout_Type type;
    char *pos;
    union{
        String identifier;
        int32_t integer;
    };
};

struct Config_Compound_Element{
    Config_Compound_Element *next;
    Config_Compound_Element *prev;
    
    Config_Layout l;
    Config_RValue *r;
};

struct Config_Assignment{
    Config_Assignment *next;
    Config_Assignment *prev;
    
    char *pos;
    Config_LValue *l;
    Config_RValue *r;
    
    bool32 visited;
};

struct Config{
    int32_t *version;
    Config_Assignment *first;
    Config_Assignment *last;
    int32_t count;
    
    Config_Error_List errors;
    
    String file_name;
    String data;
};

////////////////////////////////

typedef int32_t Iteration_Step_Result;
enum{
    Iteration_Good = 0,
    Iteration_Skip = 1,
    Iteration_Quit = 2,
};

struct Config_Get_Result{
    bool32 success;
    Config_RValue_Type type;
    char *pos;
    union{
        bool32 boolean;
        int32_t integer;
        uint32_t uinteger;
        String string;
        char character;
        Config_Compound *compound;
    };
};

struct Config_Iteration_Step_Result{
    Iteration_Step_Result step;
    Config_Get_Result get;
};

struct Config_Get_Result_Node{
    Config_Get_Result_Node *next;
    Config_Get_Result_Node *prev;
    Config_Get_Result result;
};

struct Config_Get_Result_List{
    Config_Get_Result_Node *first;
    Config_Get_Result_Node *last;
    int32_t count;
};

////////////////////////////////

struct Extension_List{
    char space[256];
    char *exts[94];
    int32_t count;
};

struct CString_Array{
    char **strings;
    int32_t count;
};

struct Config_Data{
    char user_name_space[256];
    String user_name;
    
    Extension_List code_exts;
    
    char current_mapping_space[256];
    String current_mapping;
    
    char mode_space[64];
    String mode;
    
    bool32 use_scroll_bars;
    bool32 use_file_bars;
    bool32 use_line_highlight;
    bool32 use_scope_highlight;
    bool32 use_paren_helper;
    bool32 use_comment_keyword;
    
    bool32 enable_virtual_whitespace;
    bool32 enable_code_wrapping;
    bool32 automatically_indent_text_on_save;
    bool32 automatically_save_changes_on_build;
    bool32 automatically_adjust_wrapping;
    bool32 automatically_load_project;
    
    bool32 indent_with_tabs;
    int32_t indent_width;
    
    int32_t default_wrap_width;
    int32_t default_min_base_width;
    
    char default_theme_name_space[256];
    String default_theme_name;
    
    bool32 highlight_line_at_cursor;
    
    char default_font_name_space[256];
    String default_font_name;
    int32_t default_font_size;
    bool32 default_font_hinting;
    
    char default_compiler_bat_space[256];
    String default_compiler_bat;
    
    char default_flags_bat_space[1024];
    String default_flags_bat;
    
    char default_compiler_sh_space[256];
    String default_compiler_sh;
    
    char default_flags_sh_space[1024];
    String default_flags_sh;
    
    bool32 lalt_lctrl_is_altgr;
};

struct Theme_Data{
    char space[128];
    String name;
    Theme theme;
};

#endif

// BOTTOM

