/*
* Fancy string - immediate mode renderer for colored strings
*/

// TOP

#if !defined(FCODER_FANCY_H)
#define FCODER_FANCY_H

/* TODO(casey): This warrants a lot of thought.

   Since you want to be able to edit colors after they have already been stored away in
   internal structures, you want to capture as much as possible where the colors came
   from.  In the current set-up, you can blend any two ids, but that's it.  If you
   go beyond that, it collapses down to just RGBA.  Maybe there should be more than
   that.  It's hard to say.  I don't know.
*/

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

