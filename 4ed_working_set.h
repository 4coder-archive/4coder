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
};

#endif

// BOTTOM

