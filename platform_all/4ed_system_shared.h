/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 09.02.2016
 *
 * Shared system functions
 *
 */

// TOP

// NOTE(allen): This serves as a list of functions to implement
// in addition to those in 4ed_system.h  These are not exposed to
// the application code, but system_shared.cpp
// rely on the functions listed here.

#if !defined(FRED_SYSTEM_SHARED_H)
#define FRED_SYSTEM_SHARED_H

struct File_Data{
    char *data;
    u32 size;
    b32 got_file;
};
global File_Data null_file_data = {0};

#define Sys_File_Can_Be_Made_Sig(name) b32 name(u8 *filename)
internal Sys_File_Can_Be_Made_Sig(system_file_can_be_made);

struct Shared_Vars{
    File_Track_System track;
    void *track_table;
    u32 track_table_size;
    u32 track_node_size;
    
    Partition scratch;
    Partition font_scratch;
    Partition pixel_scratch;
};
global Shared_Vars shared_vars;

#endif

// BOTTOM

