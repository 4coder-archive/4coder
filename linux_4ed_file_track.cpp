/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 20.07.2016
 *
 * File tracking linux.
 *
 */

// TOP

#include "4ed_file_track.h"
#include "4ed_file_track_general.cpp"

#include <libgen.h> // dirname

typedef struct {
    void *tables;
    int inotify;
    pthread_mutex_t lock;
    char *string_mem_begin;
    char *string_mem_end;
} Linux_File_Track_Vars;

typedef struct {
    File_Index hash;
    char* dir;
    int ref_count;
} Linux_File_Track_Entry;

#define to_vars(s) ((Linux_File_Track_Vars*)(s))
#define to_tables(v) ((File_Track_Tables*)(v->tables))

FILE_TRACK_LINK File_Track_Result
init_track_system(File_Track_System *system, Partition *scratch, void *table_memory, int32_t table_memory_size, void *listener_memory, int32_t listener_memory_size){
    File_Track_Result result = FileTrack_MemoryTooSmall;
    
    Assert(sizeof(Linux_File_Track_Vars) <= sizeof(File_Track_System));
    
    Linux_File_Track_Vars *vars = to_vars(system);
    
    Assert(sizeof(Linux_File_Track_Entry) <= sizeof(File_Track_Entry));
    
    if (enough_memory_to_init_table(table_memory_size)){
        
        // NOTE(allen): Initialize main data tables
        vars->tables = (File_Track_Tables*) table_memory;
        File_Track_Tables *tables = to_tables(vars);
        init_table_memory(tables, table_memory_size);
        
        vars->inotify = inotify_init1(IN_NONBLOCK);
        
        pthread_mutex_init(&vars->lock, NULL);
        
        vars->string_mem_begin = (char*)listener_memory;
        vars->string_mem_end   = (char*)listener_memory + listener_memory_size;
        
        result = FileTrack_Good;
    }
    
    LINUX_FN_DEBUG("result: %d", result);
    
    return(result);
}

FILE_TRACK_LINK File_Track_Result
add_listener(File_Track_System *system, Partition *scratch, char *filename){
    File_Track_Result result = FileTrack_Good;
    Linux_File_Track_Vars *vars = to_vars(system);
    File_Track_Tables *tables = to_tables(vars);
    
    pthread_mutex_lock(&vars->lock);
    
    if(tracking_system_has_space(tables, 1)){
        char *dir = dirname(strdupa(filename));
        size_t dir_len = strlen(dir) + 1;
        
        if(vars->string_mem_end - vars->string_mem_begin >= dir_len){
            int wd = inotify_add_watch(vars->inotify, dir, IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVE);
            
            if(wd != -1){
                File_Index key = { wd, 1 };
                File_Track_Entry *entry = tracking_system_lookup_entry(tables, key);
                Linux_File_Track_Entry *linux_entry = (Linux_File_Track_Entry*) entry;
                
                LINUX_FN_DEBUG("map %s to wd %d", filename, wd);
                
                if(entry_is_available(entry)){
                    linux_entry->hash = key;
                    linux_entry->dir = vars->string_mem_begin;
                    linux_entry->ref_count = 1;
                    memcpy(vars->string_mem_begin, dir, dir_len);
                    vars->string_mem_begin += dir_len;
                    ++tables->tracked_count;
                } else {
                    LINUX_FN_DEBUG("dir already tracked, adding ref");
                    ++linux_entry->ref_count;
                }
            } else {
                result = FileTrack_FileSystemError;
            }
        } else {
            result = FileTrack_OutOfListenerMemory;
        }
    } else {
        result = FileTrack_OutOfTableMemory;
    }
    
    pthread_mutex_unlock(&vars->lock);
    
    LINUX_FN_DEBUG("result: %d", result);
    
    return(result);
}

FILE_TRACK_LINK File_Track_Result
remove_listener(File_Track_System *system, Partition *scratch, char *filename){
    File_Track_Result result = FileTrack_Good;
    Linux_File_Track_Vars *vars = to_vars(system);
    File_Track_Tables *tables = to_tables(vars);
    File_Track_Entry *entries = (File_Track_Entry*) to_ptr(tables, tables->file_table);
    
    pthread_mutex_lock(&vars->lock);
    
    char *dir = dirname(strdupa(filename));
    // NOTE(inso): this assumes the filename was previously added
    
    for(uint32_t i = 0; i < tables->max; ++i){
        Linux_File_Track_Entry *e = (Linux_File_Track_Entry*)(entries + i);
        if(e->hash.id[1] != 1) continue;
        if(strcmp(e->dir, dir) == 0){
            LINUX_FN_DEBUG("%s found as wd %d", filename, e->hash.id[0]);
            if(--e->ref_count == 0){
                LINUX_FN_DEBUG("refcount == 0, calling rm_watch");
                if(inotify_rm_watch(vars->inotify, e->hash.id[0]) == -1){
                    perror("inotify_rm_watch");
                    result = FileTrack_FileSystemError;
                }
                internal_free_slot(tables, (File_Track_Entry*)e);
            }
            // NOTE(inso): associated string memory in listeners would be freed here
            break;
        }
    }
    
    pthread_mutex_unlock(&vars->lock);
    
    LINUX_FN_DEBUG("result: %d", result);
    
    return(result);
}

FILE_TRACK_LINK File_Track_Result
move_track_system(File_Track_System *system, Partition *scratch, void *mem, int32_t size){
    File_Track_Result result = FileTrack_Good;
    Linux_File_Track_Vars *vars = to_vars(system);
    
    pthread_mutex_lock(&vars->lock);
    
    {
        File_Track_Tables *original_tables = to_tables(vars);
        result = move_table_memory(original_tables, mem, size);
        if (result == FileTrack_Good){
            vars->tables = mem;
        }
    }
    
    pthread_mutex_unlock(&vars->lock);
    
    LINUX_FN_DEBUG("size: %d, %d", size, result);
    
    return(result);
}

FILE_TRACK_LINK File_Track_Result
expand_track_system_listeners(File_Track_System *system, Partition *scratch, void *mem, int32_t size){
    File_Track_Result result = FileTrack_Good;
    Linux_File_Track_Vars *vars = to_vars(system);
    
    pthread_mutex_lock(&vars->lock);
    
    // NOTE(inso): pointer to old string mem is lost here.
    // would need to keep it around if we want to free in the future
    
    // NOTE(inso): assuming PATH_MAX is a reasonable lower bound of extra memory to get
    
    if(size < PATH_MAX){
        result = FileTrack_MemoryTooSmall;
    } else {
        vars->string_mem_begin = (char*) mem;
        vars->string_mem_end   = (char*) mem + size;;
    }
    
    pthread_mutex_unlock(&vars->lock);
    
    LINUX_FN_DEBUG("size: %d, result: %d", size, result);
    
    return(result);
}

FILE_TRACK_LINK File_Track_Result
get_change_event(File_Track_System *system, Partition *scratch, char *buffer, int32_t max, int32_t *size){
    File_Track_Result result = FileTrack_NoMoreEvents;
    Linux_File_Track_Vars *vars = to_vars(system);
    File_Track_Tables *tables = to_tables(vars);
    
    pthread_mutex_lock(&vars->lock);
    
    struct inotify_event *ev;
    char buff[sizeof(*ev) + NAME_MAX + 1] __attribute__((aligned(__alignof__(*ev))));
    
    // NOTE(inso): make sure we only read one event
    size_t read_count = sizeof(*ev);
    ssize_t n;
    
    do {
        n = read(vars->inotify, buff, read_count);
        read_count++;
    } while(n == -1 && errno == EINVAL);
    
    if(n == -1 && errno != EAGAIN){
        perror("inotify read");
    } else if(n > 0){
        ev = (struct inotify_event*) buff;
        
        File_Index key = { ev->wd, 1 };
        File_Track_Entry *entry = tracking_system_lookup_entry(tables, key);
        Linux_File_Track_Entry *linux_entry = (Linux_File_Track_Entry*) entry;
        
        LINUX_FN_DEBUG("mask: %#x", ev->mask);
        
        if(!entry_is_available(entry)){
            
            char* full_name = (char*) alloca(strlen(linux_entry->dir) + ev->len + 1);
            strcpy(full_name, linux_entry->dir);
            strcat(full_name, "/");
            strcat(full_name, ev->name);
            
            LINUX_FN_DEBUG("event from wd %d (%s)", ev->wd, full_name);
            
            size_t full_name_size = strlen(full_name);
            if(max < full_name_size){
                result = FileTrack_MemoryTooSmall;
                // NOTE(inso): this event will be dropped, needs to be stashed.
                LINUX_FN_DEBUG("max too small, event dropped");
            } else {
                memcpy(buffer, full_name, full_name_size);
                *size = full_name_size;
                result = FileTrack_Good;
            }
        } else {
            LINUX_FN_DEBUG("dead event from wd %d", ev->wd);
        }
    }
    
    pthread_mutex_unlock(&vars->lock);
    
    return(result);
}

FILE_TRACK_LINK File_Track_Result
shut_down_track_system(File_Track_System *system, Partition *scratch){
    File_Track_Result result = FileTrack_Good;
    Linux_File_Track_Vars *vars = to_vars(system);
    
    // NOTE(allen): Close all the global track system resources.
    if(close(vars->inotify) == -1){
        result = FileTrack_FileSystemError;
    }
    
    pthread_mutex_destroy(&vars->lock);
    
    LINUX_FN_DEBUG("result: %d", result);
    
    return(result);
}



// BOTTOM



