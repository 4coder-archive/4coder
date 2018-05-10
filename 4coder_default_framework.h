/*
4coder_default_framework.cpp - Sets up the basics of the framework that is used for default 4coder behaviour.
*/

// TOP

#if !defined(FCODER_DEFAULT_FRAMEWORK_H)
#define FCODER_DEFAULT_FRAMEWORK_H

enum Default_Maps{
    default_code_map,
    default_maps_count,
};

////////////////////////////////

enum Rewrite_Type{
    RewriteNone,
    RewritePaste,
    RewriteWordComplete
};

struct View_Paste_Index{
    int32_t rewrite;
    int32_t next_rewrite;
    int32_t index;
};

////////////////////////////////

struct ID_Based_Jump_Location{
    int32_t buffer_id;
    int32_t line;
    int32_t column;
};

////////////////////////////////

struct Config_Line{
    Cpp_Token id_token;
    Cpp_Token subscript_token;
    Cpp_Token eq_token;
    Cpp_Token val_token;
    int32_t val_array_start;
    int32_t val_array_end;
    int32_t val_array_count;
    String error_str;
    bool32 read_success;
};

struct Config_Item{
    Config_Line line;
    Cpp_Token_Array array;
    char *mem;
    String id;
    int32_t subscript_index;
    bool32 has_subscript;
};

struct Config_Array_Reader{
    Cpp_Token_Array array;
    char *mem;
    int32_t i;
    int32_t val_array_end;
    bool32 good;
};

////////////////////////////////

struct Named_Mapping{
    String name;
    Custom_Command_Function *remap_command;
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

#endif

// BOTTOM

