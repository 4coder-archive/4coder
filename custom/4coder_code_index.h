/*
4coder_code_index.h - Generic code indexing system for layout, definition jumps, etc.
*/

// TOP

#if !defined(FCODER_CODE_INDEX_H)
#define FCODER_CODE_INDEX_H

struct Code_Index_Nest_List{
    struct Code_Index_Nest *first;
    struct Code_Index_Nest *last;
    i32 count;
};

struct Code_Index_Nest_Ptr_Array{
    struct Code_Index_Nest **ptrs;
    i32 count;
};

typedef i32 Code_Index_Nest_Kind;
enum{
    CodeIndexNest_Scope,
    CodeIndexNest_Paren,
};

struct Code_Index_Nest{
    Code_Index_Nest *next;
    
    Code_Index_Nest_Kind kind;
    b32 is_closed;
    Range_i64 open;
    Range_i64 close;
    
    Code_Index_Nest_List nest_list;
    Code_Index_Nest_Ptr_Array nest_array;
};

struct Code_Index_File{
    Code_Index_Nest_List nest_list;
    Code_Index_Nest_Ptr_Array nest_array;
};

////////////////////////////////

typedef void Generic_Parse_Comment_Function(Application_Links *app, Arena *arena, Code_Index_File *index,
                                            Token *token, String_Const_u8 contents);

struct Generic_Parse_State{
    Application_Links *app;
    Arena *arena;
    String_Const_u8 contents;
    Token_Iterator_Array it;
    Generic_Parse_Comment_Function *handle_comment;
};

#endif

// BOTTOM

