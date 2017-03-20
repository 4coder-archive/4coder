/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 06.01.2017
 *
 * Undo subsystem for 4coder
 *
 */

// TOP

//
// Undo Basics
//

enum Edit_Type{
    ED_NORMAL,
    ED_REVERSE_NORMAL,
    ED_UNDO,
    ED_REDO,
};

struct Edit_Step{
    Edit_Type type;
    union{
        struct{
            b32 can_merge;
            Buffer_Edit edit;
            u32 next_block;
            u32 prev_block;
        };
        struct{
            u32 first_child;
            u32 inverse_first_child;
            u32 inverse_child_count;
            u32 special_type;
        };
    };
    u32 child_count;
};

struct Edit_Stack{
    u8 *strings;
    u32 size, max;
    
    Edit_Step *edits;
    u32 edit_count, edit_max;
};

struct Small_Edit_Stack{
    u8 *strings;
    u32 size, max;
    
    Buffer_Edit *edits;
    u32 edit_count, edit_max;
};

struct Undo_Data{
    Edit_Stack undo;
    Edit_Stack redo;
    Edit_Stack history;
    Small_Edit_Stack children;
    
    u32 history_block_count;
    u32 history_head_block;
    u32 edit_history_cursor;
    b32 current_block_normal;
};

// BOTTOM

