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

struct File_Name_Data{
    String_Const_u8 file_name;
    Data data;
};

////////////////////////////////

typedef b8 Character_Predicate_Function(u8 c);

global Character_Predicate character_predicate_alpha = { {
        0,   0,   0,   0,   0,   0,   0,   0, 
        254, 255, 255,   7, 254, 255, 255,   7, 
        0,   0,   0,   0,   0,   0,   0,   0, 
        0,   0,   0,   0,   0,   0,   0,   0, 
    } };

global Character_Predicate character_predicate_alpha_numeric = { {
        0,   0,   0,   0,   0,   0, 255,   3, 
        254, 255, 255,   7, 254, 255, 255,   7, 
        0,   0,   0,   0,   0,   0,   0,   0, 
        0,   0,   0,   0,   0,   0,   0,   0, 
    } };

global Character_Predicate character_predicate_alpha_numeric_underscore = { {
        0,   0,   0,   0,   0,   0, 255,   3, 
        254, 255, 255, 135, 254, 255, 255,   7, 
        0,   0,   0,   0,   0,   0,   0,   0, 
        0,   0,   0,   0,   0,   0,   0,   0, 
    } };

global Character_Predicate character_predicate_uppercase = { {
        0,   0,   0,   0,   0,   0,   0,   0, 
        254, 255, 255,   7,   0,   0,   0,   0, 
        0,   0,   0,   0,   0,   0,   0,   0, 
        0,   0,   0,   0,   0,   0,   0,   0, 
    } };

global Character_Predicate character_predicate_lowercase = { {
        0,   0,   0,   0,   0,   0,   0,   0, 
        0,   0,   0,   0, 254, 255, 255,   7, 
        0,   0,   0,   0,   0,   0,   0,   0, 
        0,   0,   0,   0,   0,   0,   0,   0, 
    } };

global Character_Predicate character_predicate_base10 = { {
        0,   0,   0,   0,   0,   0, 255,   3, 
        0,   0,   0,   0,   0,   0,   0,   0, 
        0,   0,   0,   0,   0,   0,   0,   0, 
        0,   0,   0,   0,   0,   0,   0,   0, 
    } };

global Character_Predicate character_predicate_base16 = { {
        0,   0,   0,   0,   0,   0, 255,   3, 
        126,   0,   0,   0, 126,   0,   0,   0, 
        0,   0,   0,   0,   0,   0,   0,   0, 
        0,   0,   0,   0,   0,   0,   0,   0, 
    } };

global Character_Predicate character_predicate_whitespace = { {
        0,  62,   0,   0,   1,   0,   0,   0, 
        0,   0,   0,   0,   0,   0,   0,   0, 
        0,   0,   0,   0,   0,   0,   0,   0, 
        0,   0,   0,   0,   0,   0,   0,   0, 
    } };

global Character_Predicate character_predicate_non_whitespace = { {
        255, 193, 255, 255, 254, 255, 255, 255, 
        255, 255, 255, 255, 255, 255, 255, 255, 
        255, 255, 255, 255, 255, 255, 255, 255, 
        255, 255, 255, 255, 255, 255, 255, 255, 
    } };

global Character_Predicate character_predicate_utf8_byte = { {
        0,   0,   0,   0,   0,   0,   0,   0, 
        0,   0,   0,   0,   0,   0,   0,   0, 
        255, 255, 255, 255, 255, 255, 255, 255, 
        255, 255, 255, 255, 255, 255, 255, 255, 
    } };

global Character_Predicate character_predicate_alpha_numeric_underscore_utf8 = { {
        0,   0,   0,   0,   0,   0, 255,   3, 
        254, 255, 255, 135, 254, 255, 255,   7, 
        255, 255, 255, 255, 255, 255, 255, 255, 
        255, 255, 255, 255, 255, 255, 255, 255, 
    } };

typedef i32 Boundary_Function(Application_Links *app, Buffer_ID buffer, Side side, Scan_Direction direction, i32 pos);

struct Boundary_Function_Node{
    Boundary_Function_Node *next;
    Boundary_Function *func;
};
struct Boundary_Function_List{
    Boundary_Function_Node *first;
    Boundary_Function_Node *last;
    i32 count;
};

typedef Range Enclose_Function(Application_Links *app, Buffer_ID buffer, Range range);

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

struct History_Group{
    Application_Links *app;
    Buffer_ID buffer;
    History_Record_Index first;
};

#endif

// BOTTOM
