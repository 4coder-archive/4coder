/*
* Mr. 4th Dimention - Allen Webster
*
* 23.07.2019
*
* Type for organizating the set of all loaded font faces.
*
*/

// TOP

#if !defined(FRED_FONT_SET_H)
#define FRED_FONT_SET_H

struct Font_Face_ID_Node{
    Font_Face_ID_Node *next;
    Face_ID id;
};

union Font_Face_Slot{
    struct{
        Font_Face_Slot *next;
    };
    struct{
        Arena arena;
        Face *face;
    };
};

struct Font_Set{
    Arena arena;
    Face_ID next_id_counter;
    Font_Face_ID_Node *free_ids;
    Font_Face_ID_Node *free_id_nodes;
    Font_Face_Slot *free_face_slots;
    Table_u64_u64 id_to_slot_table;
    f32 scale_factor;
};

#endif

// BOTTOM

