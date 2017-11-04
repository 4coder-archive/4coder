/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 28.06.2017
 *
 * Mac file tracking C++ wrapper.
 *
 */

// TOP

File_Track_Result
init_track_system(File_Track_System *system, Partition *scratch, void *table_memory, i32 table_memory_size, void *listener_memory, i32 listener_memory_size){
    File_Track_Result result = FileTrack_Good;
    // NOTE(allen): Do nothing???
    return(result);
}

File_Track_Result
add_listener(File_Track_System *system, Partition *scratch, u8 *filename){
    File_Track_Result result = FileTrack_Good;
    osx_add_file_listener((char*)filename);
    return(result);
}

File_Track_Result
remove_listener(File_Track_System *system, Partition *scratch, u8 *filename){
    File_Track_Result result = FileTrack_Good;
    osx_remove_file_listener((char*)filename);
    return(result);
}

File_Track_Result
move_track_system(File_Track_System *system, Partition *scratch, void *mem, i32 size){
    File_Track_Result result = FileTrack_Good;
    // NOTE(allen): Do nothing???
    return(result);
}

File_Track_Result
expand_track_system_listeners(File_Track_System *system, Partition *scratch, void *mem, i32 size){
    File_Track_Result result = FileTrack_Good;
    // NOTE(allen): Do nothing???
    return(result);
}

File_Track_Result
get_change_event(File_Track_System *system, Partition *scratch, u8 *buffer, i32 max, i32 *size){
    File_Track_Result result = FileTrack_Good;
    i32 status = osx_get_file_change_event((char*)buffer, max, size);
    if (status == 0){
        result = FileTrack_NoMoreEvents;
    }
    if (status == -1){
        result = FileTrack_MemoryTooSmall;
    }
    return(result);
}

File_Track_Result
shut_down_track_system(File_Track_System *system, Partition *scratch){
    File_Track_Result result = FileTrack_Good;
    // NOTE(allen): Do nothing???
    return(result);
}

// BOTTOM

