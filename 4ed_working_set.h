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

struct Non_File_Table_Entry{
    String name;
    Buffer_Slot_ID id;
};

struct File_Array{
    Editing_File *files;
    i32 size;
};

struct Working_Set{
    File_Array *file_arrays;
    i32 file_count, file_max;
    i16 array_count, array_max;
    
    File_Node free_sentinel;
    File_Node used_sentinel;
    
    Table canon_table;
    Table name_table;
    
    // TODO(allen): WTF?
    String clipboards[64];
    i32 clipboard_size, clipboard_max_size;
    i32 clipboard_current, clipboard_rolling;
    
    //u64 unique_file_counter;
    
    File_Node *sync_check_iter;
    
    i32 default_display_width;
    i32 default_minimum_base_display_width;
};

struct File_Name_Entry{
    String name;
    Buffer_Slot_ID id;
};

#endif

// BOTTOM

