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
            i32 next_block;
            i32 prev_block;
        };
        struct{
            i32 first_child;
            i32 inverse_first_child;
            i32 inverse_child_count;
            i32 special_type;
        };
    };
    i32 child_count;
};

struct Edit_Stack{
    u8 *strings;
    i32 size, max;
    
    Edit_Step *edits;
    i32 edit_count, edit_max;
};

struct Small_Edit_Stack{
    u8 *strings;
    i32 size, max;
    
    Buffer_Edit *edits;
    i32 edit_count, edit_max;
};

struct Undo_Data{
    Edit_Stack undo;
    Edit_Stack redo;
    Edit_Stack history;
    Small_Edit_Stack children;
    
    i32 history_block_count, history_head_block;
    i32 edit_history_cursor;
    b32 current_block_normal;
};

// BOTTOM

