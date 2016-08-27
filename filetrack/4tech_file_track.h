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
    FileTrack_NoMoreEvents,
    FileTrack_FileSystemError
};

typedef struct{
    uint8_t opaque[128];
} File_Track_System;

typedef int32_t File_Track_Result;

File_Track_Result
init_track_system(File_Track_System *system,
                  void *table_memory, int32_t table_memory_size,
                  void *listener_memory, int32_t listener_memory_size);

File_Track_Result
add_listener(File_Track_System *system, char *filename);

File_Track_Result
remove_listener(File_Track_System *system, char *filename);

File_Track_Result
move_track_system(File_Track_System *system, void *mem, int32_t size);

File_Track_Result
expand_track_system_listeners(File_Track_System *system, void *mem, int32_t size);

File_Track_Result
get_change_event(File_Track_System *system, char *buffer, int32_t max);

File_Track_Result
shut_down_track_system(File_Track_System *system);

#endif

// BOTTOM
