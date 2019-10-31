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
    
    i64 interior_indentation;
    i64 close_indentation;
    
    struct Code_Index_File *file;
    Code_Index_Nest *parent;
    
    Code_Index_Nest_List nest_list;
    Code_Index_Nest_Ptr_Array nest_array;
};

struct Code_Index_File{
    Code_Index_Nest_List nest_list;
    Code_Index_Nest_Ptr_Array nest_array;
    Buffer_ID buffer;
};

struct Code_Index_File_Storage{
    Code_Index_File_Storage *next;
    Code_Index_File_Storage *prev;
    Arena arena;
    Code_Index_File *file;
};

struct Code_Index{
    System_Mutex mutex;
    Arena node_arena;
    Table_u64_u64 buffer_to_index_file;
    Code_Index_File_Storage *free_storage;
    Code_Index_File_Storage *storage_first;
    Code_Index_File_Storage *storage_last;
    i32 storage_count;
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
    u8 *prev_line_start;
    b32 finished;
};

#endif

// BOTTOM

