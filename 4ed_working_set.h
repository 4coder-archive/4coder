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
    // NOTE(allen): After initialization of file_change_thread
    // the members of this struct should only be accessed by a thread
    // who owns the mutex member.
    
    Arena arena;
    
    Editing_File *free_files;
    Buffer_ID id_counter;
    
    Node active_file_sentinel;
    Node touch_order_sentinel;
    i32 active_file_count;
    
    Table_u64_u64 id_to_ptr_table;
    Table_Data_u64 canon_table;
    Table_Data_u64 name_table;
    
    Node *sync_check_iterator;
    Node has_external_mod_sentinel;
    System_Mutex mutex;
    System_Thread file_change_thread;
    
    // TODO(allen): do(update clipboard system to exist fully in the custom layer)
    // NOTE(allen): These members have nothing to do with the working set or
    // the mutex that gaurds the other members.
    String_Const_u8 clipboards[64];
    i32 clipboard_size;
    i32 clipboard_max_size;
    i32 clipboard_current;
    i32 clipboard_rolling;
};

#endif

// BOTTOM

