/*
 * Miscellaneous helpers for common operations.
 */

// TOP

#if !defined(FCODER_HELPER_H)
#define FCODER_HELPER_H

struct Bind_Helper{
    Binding_Unit *cursor, *start, *end;
    Binding_Unit *header, *group;
    i32 write_total;
    i32 error;
};

#define BH_ERR_NONE 0
#define BH_ERR_MISSING_END 1
#define BH_ERR_MISSING_BEGIN 2
#define BH_ERR_OUT_OF_MEMORY 3

struct Bind_Buffer{
    void *data;
    i32 size;
};

////////////////////////////////

struct File_Handle_Path{
    FILE *file;
    String path;
};

struct File_Name_Data{
    String file_name;
    String data;
};

struct File_Name_Path_Data{
    String file_name;
    String path;
    String data;
};

////////////////////////////////

struct Buffer_Rect{
    i32 char0;
    i32 line0;
    i32 char1;
    i32 line1;
};

////////////////////////////////

struct Stream_Chunk{
    Application_Links *app;
    Buffer_ID buffer_id;
    
    char *base_data;
    i32 start;
    i32 end;
    i32 min_start;
    i32 max_end;
    b32 add_null;
    u32 data_size;
    
    char *data;
};

// NOTE(allen|4.0.31): Stream_Tokens has been deprecated in favor of the Token_Iterator below.
// For examples of usage: 4coder_function_list.cpp 4coder_scope_commands.cpp
// If you want to keep your code working easily uncomment the typedef for Stream_Tokens.
struct Stream_Tokens_DEP{
    Application_Links *app;
    Buffer_ID buffer_id;
    
    Cpp_Token *base_tokens;
    Cpp_Token *tokens;
    i32 start;
    i32 end;
    i32 count;
    i32 token_count;
};
//typedef Stream_Tokens_DEP Stream_Tokens;

struct Token_Range{
    Cpp_Token *first;
    Cpp_Token *one_past_last;
};

struct Token_Iterator{
    // TODO(allen): source buffer
    Cpp_Token *token;
    Token_Range range;
};

////////////////////////////////

struct Sort_Pair_i32{
    i32 index;
    i32 key;
};

////////////////////////////////

struct Buffer_Insertion
{
    Application_Links *app;
    Buffer_ID buffer;
    i32 at;
};

#endif

// BOTTOM
