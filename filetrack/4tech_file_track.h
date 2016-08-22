/*

Copy Right FourTech LLC, 2016
All Rights Are Reserved

The OS agnostic file tracking API for applications
that want to interact with potentially many files on
the disk that could be changed by other applications.

Created on: 20.07.2016

*/

// TOP

#ifndef FILE_TRACK_4TECH_H
#define FILE_TRACK_4TECH_H

#include <stdint.h>

enum{
    FileTrack_Good,
    FileTrack_MemoryTooSmall,
    FileTrack_OutOfTableMemory,
    FileTrack_OutOfListenerMemory,
    FileTrack_FileNotFound,
    FileTrack_FileAlreadyTracked,
    FileTrack_FileNotTracked,
    FileTrack_NoMoreEvents,
    FileTrack_FileSystemError
};

typedef struct{
    uint8_t opaque[128];
} File_Track_System;

typedef struct{
    uint32_t id[4];
} File_Index;

static File_Index
zero_file_index(){
    File_Index a = {0};
    return(a);
}

static int32_t
file_index_eq(File_Index a, File_Index b){
    return ((a.id[0] == b.id[0]) &&
            (a.id[1] == b.id[1]) &&
            (a.id[2] == b.id[2]) &&
            (a.id[3] == b.id[3]));
}

typedef int32_t  File_Track_Result;
typedef uint64_t File_Time;

File_Track_Result
init_track_system(File_Track_System *system,
                  void *table_memory, int32_t table_memory_size,
                  void *listener_memory, int32_t listener_memory_size);

File_Track_Result
begin_tracking_file(File_Track_System *system, char *name, File_Index *index, File_Time *time);

File_Track_Result
get_tracked_file_index(File_Track_System *system, char *name, File_Index *index);

File_Track_Result
stop_tracking_file(File_Track_System *system, File_Index index);

File_Track_Result
count_tracked_files(File_Track_System *system, int32_t *count);

File_Track_Result
get_tracked_file_time(File_Track_System *system, File_Index index, File_Time *time);

File_Track_Result
get_file_time_now(File_Time *time);

File_Track_Result
move_track_system(File_Track_System *system, void *mem, int32_t size);

File_Track_Result
expand_track_system_listeners(File_Track_System *system, void *mem, int32_t size);

File_Track_Result
get_change_event(File_Track_System *system, File_Index *index);

File_Track_Result
get_tracked_file_size(File_Track_System *system, File_Index index, uint32_t *size);

File_Track_Result
get_tracked_file_data(File_Track_System *system, File_Index index, void *mem, uint32_t size);

File_Track_Result
rewrite_tracked_file(File_Track_System *system, File_Index index,
                     void *data, int32_t size, File_Time *time);

File_Track_Result
shut_down_track_system(File_Track_System *system);

#endif

// BOTTOM
