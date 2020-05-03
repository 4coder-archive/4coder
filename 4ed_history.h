/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 24.03.2018
 *
 * History
 *
 */

// TOP

#if !defined(FRED_HISTORY_H)
#define FRED_HISTORY_H

struct Record_Batch_Slot{
    i64 length_forward;
    i64 length_backward;
    i32 first;
};

struct Record{
    Node node;
    Temp_Memory restore_point;
    i64 pos_before_edit;
    i32 edit_number;
    Record_Kind kind;
    union{
        struct{
            String_Const_u8 forward_text;
            String_Const_u8 backward_text;
            i64 first;
        } single;
        struct{
            Node children;
            i32 count;
        } group;
    };
};

struct Record_Ptr_Lookup_Table{
    Record **records;
    i32 count;
    i32 max;
};

struct History{
    b32 activated;
    Arena arena;
    Heap heap;
    Base_Allocator heap_wrapper;
    Node free_records;
    Node records;
    i32 record_count;
    Record_Ptr_Lookup_Table record_lookup;
};

struct Global_History{
    i32 edit_number_counter;
    i32 edit_grouping_counter;
};

#endif

// BOTTOM
