/*
* Fancy string - immediate mode renderer for colored strings
*/

// TOP

#if !defined(FCODER_FANCY_H)
#define FCODER_FANCY_H

struct Fancy_Color{
    union{
        struct{
            id_color index_a;
            id_color index_b;
        };
        u32 rgba;
    };
    
    union{
        struct{
            u8 table_a;
            u8 table_b;
            u8 c_a;
            u8 c_b;
        };
        u32 code;
    };
};

struct Fancy_String{
    Fancy_String *next;
    String_Const_u8 value;
    
    Face_ID font_id;
    Fancy_Color fore;
    Fancy_Color back;
    
    f32 pre_margin;
    f32 post_margin;
};

struct Fancy_String_List{
    Fancy_String *first;
    Fancy_String *last;
};

#endif

// BOTTOM

