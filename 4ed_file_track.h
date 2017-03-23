/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 20.07.2016
 *
 * File tracking API.
 *
 */

// TOP

#if !defined(FILE_TRACK_4TECH_H)
#define FILE_TRACK_4TECH_H

#if !defined(FILE_TRACK_LINK)
# define FILE_TRACK_LINK static
#endif

typedef struct{
    u8 opaque[128];
} File_Track_System;

typedef i32 File_Track_Result;
enum{
    FileTrack_Good,
    FileTrack_MemoryTooSmall,
    FileTrack_OutOfTableMemory,
    FileTrack_OutOfListenerMemory,
    FileTrack_NoMoreEvents,
    FileTrack_FileSystemError
};

FILE_TRACK_LINK File_Track_Result
init_track_system(File_Track_System *system, Partition *scratch, void *table_memory, umem table_memory_size, void *listener_memory, umem listener_memory_size);

FILE_TRACK_LINK File_Track_Result
add_listener(File_Track_System *system, Partition *scratch, u8 *filename);

FILE_TRACK_LINK File_Track_Result
remove_listener(File_Track_System *system, Partition *scratch, u8 *filename);

FILE_TRACK_LINK File_Track_Result
move_track_system(File_Track_System *system, Partition *scratch, void *mem, umem size);

FILE_TRACK_LINK File_Track_Result
expand_track_system_listeners(File_Track_System *system, Partition *scratch, void *mem, umem size);

FILE_TRACK_LINK File_Track_Result
get_change_event(File_Track_System *system, Partition *scratch, u8 *buffer, umem max, umem *size);

FILE_TRACK_LINK File_Track_Result
shut_down_track_system(File_Track_System *system, Partition *scratch);

#endif

// BOTTOM
