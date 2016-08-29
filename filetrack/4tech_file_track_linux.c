/*

The OS agnostic file tracking API for applications
that want to interact with potentially many files on
the disk that could be changed by other applications.

Created on: 29.08.2016

*/

// TOP

#include "4tech_file_track.h"
#include "4tech_file_track_general.c"

// NOTE(allen):
// There are a number of places where the "table" is built into this system.
// If a table is not actually needed we will leave it in the API, but pull
// out all of the following internal refrences to the table:
// In Linux_File_Track_Vars :
//   + void *tables; and all refrences to it
// The entire Linux_File_Track_Entry struct and all refrences to it.
// In init_track_system :
//   + enough_memory_to_init_table(table_memory_size)
//   + everything under the note "Initialize main data tables"
// Leave move_track_system unimplemented and assert when it is called
// 

typedef struct {
    void *tables;
    // NOTE(allen):
    // This struct must fit inside the opaque File_Track_System struct.
    // If it needs more room we can make File_Track_System bigger.
    //
    // Here we need:
    // 1. A mutex of some kind to make the main operations thread safe.
    //    If it turns out the operations will be thread safe without a mutex
    //    then it is not required.
    // 2. Any synchronization primatives that are needed for dequeueing
    //    change events.
    // 3. Any data structures needed for handling the listeners.
    //    This system can expect to be provided with memory from the user.
    //    Right now the way the API works, it assumes that the user will
    //    only ever add memory and never free it.  If freeing becomes necessary
    //    on Linux the API can be extended to support that.
} Linux_File_Track_Vars;

typedef struct {
    File_Index hash;
    // NOTE(allen):
    // This struct must fit inside the opaque File_Track_Entry struct.
    // The table is meant to help keep track of what is already being
    // tracked.  It may turn out that this isn't needed on Linux which
    // would be fine.  If it is used hash should be the first element
    // of this struct and it should be used to uniquely identify each
    // entry because it is used as the key for the table.
} Linux_File_Track_Entry;

#define to_vars(s) ((Linux_File_Track_Vars*)(s))
#define to_tables(v) ((File_Track_Tables*)(v->tables))

FILE_TRACK_LINK File_Track_Result
init_track_system(File_Track_System *system,
                  void *table_memory, int32_t table_memory_size,
                  void *listener_memory, int32_t listener_memory_size){
    File_Track_Result result = FileTrack_MemoryTooSmall;
    Linux_File_Track_Vars *vars = to_vars(system);
    
    Assert(sizeof(Linux_File_Track_Entry) <= sizeof(File_Track_Entry));
    
    if (enough_memory_to_init_table(table_memory_size) &&
        /*if listener memory is important check it's size here*/ 1){
        
        // NOTE(allen): Initialize main data tables
        vars->tables = table_memory;
        File_Track_Tables *tables = to_tables(vars);
        init_table_memory(tables, table_memory_size);
        
        // NOTE(allen):
        // Here prepare the data structure and synchronization primatives
        // in the vars struct.
        
        result = FileTrack_Good;
    }
    
    return(result);
}

FILE_TRACK_LINK File_Track_Result
add_listener(File_Track_System *system, char *filename){
    File_Track_Result result = FileTrack_Good;
    Linux_File_Track_Vars *vars = to_vars(system);
    
    // ACQUIRE LOCK (only keep this if either of the data structures is kept)
    
    // NOTE(allen):
    // Here do something to begin listening to changes to the file named filename.
    // On Windows it listens to the parent directory if no other file in that
    // directory is already being listened to.  We will assume that the user
    // never passes the same filename in twice, although they may pass in two
    // different names that both refer to the same file.  In this case we should
    // treat them as two separate files so that if one of the listeners is removed
    // the other on the same file keeps working.
    
    // RELEASE LOCK
    
    return(result);
}

FILE_TRACK_LINK File_Track_Result
remove_listener(File_Track_System *system, char *filename){
    File_Track_Result result = FileTrack_Good;
    Linux_File_Track_Vars *vars = to_vars(system);
    
    // ACQUIRE LOCK
    
    // NOTE(allen):
    // Here do something to stop listening to changes to the file named filename.
    // We assume this filename has been passed into add_listener already and that
    // if it has not it is a bug in the user's code not in this code.
    
    // RELEASE LOCK
    
    return(result);
}

FILE_TRACK_LINK File_Track_Result
move_track_system(File_Track_System *system, void *mem, int32_t size){
    File_Track_Result result = FileTrack_Good;
    Linux_File_Track_Vars *vars = to_vars(system);
    
    // ACQUIRE LOCK
    
    {
        File_Track_Tables *original_tables = to_tables(vars);
        result = move_table_memory(original_tables, mem, size);
        if (result == FileTrack_Good){
            vars->tables = mem;
        }
    }
    
    // RELEASE LOCK
    
    return(result);
}

FILE_TRACK_LINK File_Track_Result
expand_track_system_listeners(File_Track_System *system, void *mem, int32_t size){
    File_Track_Result result = FileTrack_Good;
    Linux_File_Track_Vars *vars = to_vars(system);
    
    // ACQUIRE LOCK
    
    // NOTE(allen): If there is a data structure for the listeners
    // this call adds more memory to that system.  If this system
    // wants to free old memory the API does not currently support
    // that but it can.
    
    // RELEASE LOCK
    
    return(result);
}

FILE_TRACK_LINK File_Track_Result
get_change_event(File_Track_System *system, char *buffer, int32_t max, int32_t *size){
    File_Track_Result result = FileTrack_NoMoreEvents;
    Linux_File_Track_Vars *vars = to_vars(system);
    
    // ACQUIRE LOCK
    
    // NOTE(allen): If there are any new file changes report them
    // by copying the name of the file to the buffer and returning
    // FileTrack_Good.  It is allowed for this system to report
    // changes to files that were not requested to be tracked, it
    // is up to the user to check the filename and see if it cares
    // about that file.  If there are no new changes the return
    // FileTrack_NoMoreEvents.
    
    // RELEASE LOCK
    
    return(result);
}

FILE_TRACK_LINK File_Track_Result
shut_down_track_system(File_Track_System *system){
    File_Track_Result result = FileTrack_Good;
    Linux_File_Track_Vars *vars = to_vars(system);
    
    
    // NOTE(allen): Close all the handles stored in the table.
    {
        File_Track_Tables *tables = to_tables(vars);
        
        File_Track_Entry *entries = (File_Track_Entry*)to_ptr(tables, tables->file_table);
        uint32_t max = tables->max;
        
        for (uint32_t index = 0; index < max; ++index){
            File_Track_Entry *entry = entries + index;
            
            if (!entry_is_available(entry)){
                // NOTE(allen): If table entries contain anything
                // that need cleanup, that cleanup should happen here.
            }
        }
    }
    
    // NOTE(allen): Close all the global track system resources.
    
    return(result);
}



// BOTTOM



