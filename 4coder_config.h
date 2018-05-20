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

struct Config_Parser{
    Cpp_Token *start;
    Cpp_Token *token;
Cpp_Token *end;
    
     String file_name;
    String data;
    
    Partition *arena;
    
    Config_Error *first_error;
    Config_Error *last_error;
    int32_t count_error;
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
    ConfigRValueType_COUNT = 7,
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
    
    Config_LValue *l;
    Config_RValue *r;
    
    bool32 visited;
};

struct Config{
    int32_t *version;
    Config_Assignment *first;
    Config_Assignment *last;
    int32_t count;
    
    Config_Error *first_error;
    Config_Error *last_error;
    int32_t count_error;
    
    String data;
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
    int32_t default_wrap_width;
    int32_t default_min_base_width;
    
    bool32 enable_code_wrapping;
    bool32 automatically_adjust_wrapping;
     bool32 automatically_indent_text_on_save;
    bool32 automatically_save_changes_on_build;
    bool32 automatically_load_project;
    bool32 lalt_lctrl_is_altgr;
    
     char default_theme_name_space[256];
     String default_theme_name;
    
     char default_font_name_space[256];
     String default_font_name;
    
     char user_name_space[256];
     String user_name;
    
    char default_compiler_bat_space[256];
    String default_compiler_bat;
    
    char default_flags_bat_space[1024];
    String default_flags_bat;
    
    char default_compiler_sh_space[256];
    String default_compiler_sh;
    
    char default_flags_sh_space[1024];
    String default_flags_sh;
    
    char current_mapping_space[256];
    String current_mapping;
    
    Extension_List code_exts;
    };

struct Theme_Data{
    char space[128];
    String name;
    Theme theme;
};

#endif

// BOTTOM

