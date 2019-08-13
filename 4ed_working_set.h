/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 24.03.2018
 *
 * Working_Set data structure
 *
 */

// TOP

#if !defined(FRED_WORKING_SET_H)
#define FRED_WORKING_SET_H

struct Working_Set{
    Arena arena;
    
    Editing_File *free_files;
    Buffer_ID id_counter;
    
    Node active_file_sentinel;
    Node touch_order_sentinel;
    i32 active_file_count;
    
    Node edit_finished_sentinel;
    i32 edit_finished_count;
    u64 time_of_next_edit_finished_signal;
    Plat_Handle edit_finished_timer;
    b32 do_not_mark_edits;
    
    Table_u64_u64 id_to_ptr_table;
    Table_Data_u64 canon_table;
    Table_Data_u64 name_table;
    
    // TODO(allen): do(update clipboard system to exist fully in the custom layer)
    String_Const_u8 clipboards[64];
    i32 clipboard_size;
    i32 clipboard_max_size;
    i32 clipboard_current;
    i32 clipboard_rolling;
    
    i32 default_display_width;
    i32 default_minimum_base_display_width;
};

internal void
file_mark_edit_finished(Working_Set *working_set, Editing_File *file);

internal b32
file_unmark_edit_finished(Working_Set *working_set, Editing_File *file);

#endif

// BOTTOM

