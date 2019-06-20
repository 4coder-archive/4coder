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

typedef i32 Position_Within_Line;
enum{
    PositionWithinLine_Start,
    PositionWithinLine_SkipLeadingWhitespace,
    PositionWithinLine_End,
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

typedef i64 Boundary_Function(Application_Links *app, Buffer_ID buffer, Side side, Scan_Direction direction, i64 pos);

struct Boundary_Function_Node{
    Boundary_Function_Node *next;
    Boundary_Function *func;
};
struct Boundary_Function_List{
    Boundary_Function_Node *first;
    Boundary_Function_Node *last;
    i32 count;
};

typedef Range_i64 Enclose_Function(Application_Links *app, Buffer_ID buffer, Range_i64 range);

struct Indent_Info{
    i64 first_char_pos;
    i32 indent_pos;
    b32 is_blank;
    b32 all_space;
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

////////////////////////////////

typedef i32 View_Split_Kind;
enum{
    ViewSplitKind_Ratio,
    ViewSplitKind_FixedPixels,
};

#endif

// BOTTOM
