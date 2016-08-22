/*

Copy Right FourTech LLC, 2016
All Rights Are Reserved

The OS agnostic file tracking API for applications
that want to interact with potentially many files on
the disk that could be changed by other applications.

Created on: 20.07.2016

*/

// TOP

#include "4tech_file_track.h"

#include <Windows.h>

#ifndef Assert
# define Assert(c) do { if (!(c)) { *((int*)0) = 0xA11E; } } while (0)
#endif

#ifndef ZeroStruct
# define ZeroStruct(s) for (int32_t i = 0; i < sizeof(s); ++i) { ((char*)(&(s)))[i] = 0; }
#endif

#ifndef NotImplemented
# define NotImplemented Assert(!"not implemented")
#endif

typedef uint32_t rptr32;

typedef struct DLL_Node {
    struct DLL_Node *next;
    struct DLL_Node *prev;
} DLL_Node;

typedef struct {
    OVERLAPPED overlapped;
    HANDLE dir;
    int32_t user_count;
    
    char dir_name[512];
    
    // TODO(allen): I am only ever using one thread
    // for reading results.  So is it possible to
    // have them all go through the same location
    // instead of having a different 2K block
    // for each directory node?
    char result[2048];
} Directory_Listener;

typedef struct {
    DLL_Node node;
    Directory_Listener listener;
} Directory_Listener_Node;

typedef struct {
    HANDLE iocp;
    HANDLE thread;
    CRITICAL_SECTION table_lock;
    
    void *tables;
    DLL_Node free_sentinel;
    
} File_Track_Vars;

typedef struct {
    int32_t size;
    uint32_t tracked_count;
    uint32_t tracked_dir_count;
    uint32_t max;
    uint32_t change_write_pos;
    uint32_t change_read_pos;
    rptr32 change_queue;
    rptr32 file_table;
} File_Track_Tables;

typedef struct {
    HANDLE file, dir;
    File_Index hash;
    union {
        Directory_Listener_Node *listener_node;
        struct {
            int32_t change_pos;
            int32_t skip_change;
        };
    };
} File_Track_Entry;

typedef struct {
    File_Index index;
    int32_t still_active;
} File_Change_Record;

#define FILE_ENTRY_COST (sizeof(File_Change_Record) + sizeof(File_Track_Entry))

#define to_vars_(s) ((File_Track_Vars*)(s))
#define to_tables(v) ((File_Track_Tables*)(v->tables))
#define to_ptr(b,p) ((void*)((char*)b + p))
#define to_rptr32(b,p) ((rptr32)((char*)(p) - (char*)(b)))

static void
insert_node(DLL_Node *pos, DLL_Node *node){
    node->prev = pos;
    node->next = pos->next;
    pos->next = node;
    node->next->prev = node;
}

static void
remove_node(DLL_Node *node){
    node->next->prev = node->prev;
    node->prev->next = node->next;
}

static void
init_sentinel_node(DLL_Node *node){
    node->next = node;
    node->prev = node;
}

static DLL_Node*
allocate_node(DLL_Node *sentinel){
    DLL_Node *result = 0;
    if (sentinel->next != sentinel){
        result = sentinel->next;
        remove_node(result);
    }
    return(result);
}

static int32_t
file_hash_is_zero(File_Index a){
    return ((a.id[0] == 0) &&
            (a.id[1] == 0) &&
            (a.id[2] == 0) &&
            (a.id[3] == 0));
}

static int32_t
file_hash_is_deleted(File_Index a){
    return ((a.id[0] == 0xFFFFFFFF) &&
            (a.id[1] == 0xFFFFFFFF) &&
            (a.id[2] == 0xFFFFFFFF) &&
            (a.id[3] == 0xFFFFFFFF));
}

static int32_t
tracking_system_has_space(File_Track_Tables *tables, int32_t new_count){
    uint32_t count = tables->tracked_count;
    uint32_t max = tables->max;
    int32_t result = ((count + new_count)*8 < max*7);
    return(result);
}

static int32_t
entry_is_available(File_Track_Entry *entry){
    int32_t result = 0;
    if (entry){
        result =
            file_hash_is_zero(entry->hash) ||
            file_hash_is_deleted(entry->hash);
    }
    return (result);
}

typedef struct{
    File_Track_Entry *entry;
} File_Lookup_Result;

static File_Lookup_Result
tracking_system_lookup_entry(File_Track_Tables *tables, File_Index key){
    uint32_t hash = key.id[0];
    uint32_t max = tables->max;
    uint32_t index = (hash) % max;
    uint32_t start = index;
    
    File_Track_Entry *entries = (File_Track_Entry*)to_ptr(tables, tables->file_table);
    
    File_Lookup_Result result = {0};
    
    for (;;){
        File_Track_Entry *entry = entries + index;
        
        if (file_index_eq(entry->hash, key)){
            result.entry = entry;
            break;
        }
        else if (file_hash_is_zero(entry->hash)){
            if (result.entry == 0){
                result.entry = entry;
            }
            break;
        }
        else if (file_hash_is_deleted(entry->hash)){
            if (result.entry == 0){
                result.entry = entry;
            }
        }
        
        ++index;
        if (index == max) index = 0;
        if (index == start) break;
    }
    
    return(result);
}

static File_Track_Entry*
get_file_entry(File_Track_Tables *tables, File_Index index){
    File_Track_Entry *entry = 0;
    
    File_Lookup_Result result = tracking_system_lookup_entry(tables, index);
    if (result.entry && file_index_eq(index, result.entry->hash)){
        entry = result.entry;
    }
    
    return(entry);
}

File_Track_Result
internal_get_tracked_file_index(File_Track_Tables *tables,
                                char *name,
                                File_Index *index,
                                File_Track_Entry **entry);

static DWORD
directory_watching(LPVOID ptr){
    File_Track_Vars *vars = to_vars_(ptr);
    OVERLAPPED *overlapped = 0;
    DWORD length = 0;
    ULONG_PTR key = 0;
    
    for (;;){
        GetQueuedCompletionStatus(
            vars->iocp,
            &length,
            &key,
            &overlapped,
            INFINITE
            );
        
        Directory_Listener *listener_ptr = (Directory_Listener*)overlapped;
        Directory_Listener listener = *listener_ptr;
        
        ZeroStruct(listener_ptr->overlapped);
        ReadDirectoryChangesW(listener_ptr->dir,
                              listener_ptr->result,
                              sizeof(listener_ptr->result),
                              0,
                              FILE_NOTIFY_CHANGE_LAST_WRITE,
                              0,
                              &listener_ptr->overlapped,
                              0);
        
        {
            EnterCriticalSection(&vars->table_lock);
            
            File_Track_Tables *tables = to_tables(vars);
            File_Change_Record *records = (File_Change_Record*)
                to_ptr(tables, tables->change_queue);
            
            char *buffer = listener.result;
            DWORD offset = 0;
            FILE_NOTIFY_INFORMATION *info = 0;
            
            for (;;){
                info = (FILE_NOTIFY_INFORMATION*)(buffer + offset);
                
                // TODO(allen): make this real
                int32_t success = 0;
                char filename[512];
                int32_t len = info->FileNameLength / 2;
                int32_t pos = 0;
                
                char *src = listener.dir_name;
                for (int32_t i = 0; src[i]; ++i, ++pos){
                    filename[pos] = src[i];
                }
                
                if (len + pos + 1 < sizeof(filename)){
                    filename[pos++] = '/';
                    
                    for (int32_t i = 0; i < len; ++i, ++pos){
                        filename[pos] = (char)info->FileName[i];
                    }
                    filename[pos] = 0;
                    
                    success = 1;
                }
                
                if (success){
                    File_Index change_index = zero_file_index();
                    File_Track_Entry *entry = 0;
                    File_Track_Result result =
                        internal_get_tracked_file_index(tables, filename, &change_index, &entry);
                    
                    if (result == FileTrack_Good){
                        BY_HANDLE_FILE_INFORMATION info = {0};
                        
                        if (GetFileInformationByHandle(entry->file, &info)){
                            if (entry->skip_change){
                                entry->skip_change = 0;
                            }
                            else{
                                File_Change_Record *record = 0;
                                
                                if (entry->change_pos == -1){
                                    int32_t write_pos = tables->change_write_pos;
                                    if (tables->change_write_pos + 1 == tables->change_read_pos){
                                        break;
                                    }
                                    
                                    tables->change_write_pos += 1;
                                    entry->change_pos = write_pos;
                                    
                                    record = records + write_pos;
                                }
                                else{
                                    record = records + entry->change_pos;
                                }
                                
                                record->index = entry->hash;
                                record->still_active = 1;
                            }
                        }
                    }
                }
                
                if (info->NextEntryOffset != 0){
                    offset += info->NextEntryOffset;
                }
                else{
                    break;
                }
            }
            
            LeaveCriticalSection(&vars->table_lock);
        }
    }
}

File_Track_Result
init_track_system(File_Track_System *system,
                  void *table_memory, int32_t table_memory_size,
                  void *listener_memory, int32_t listener_memory_size){
    File_Track_Result result = FileTrack_MemoryTooSmall;
    File_Track_Vars *vars = to_vars_(system);
    
    if (sizeof(File_Track_Tables) + FILE_ENTRY_COST*8 <= table_memory_size &&
        sizeof(Directory_Listener_Node) <= listener_memory_size){
        vars->tables = table_memory;
        
        File_Track_Tables *tables = to_tables(vars);
        
        // NOTE(allen): Initialize main data tables
        {
            tables->size = table_memory_size;
            tables->tracked_count = 0;
            tables->tracked_dir_count = 0;
            
            int32_t likely_entry_size = FILE_ENTRY_COST;
            int32_t max_number_of_entries = (table_memory_size - sizeof(*tables)) / likely_entry_size;
            
            tables->change_queue = sizeof(*tables);
            tables->file_table = tables->change_queue +
                sizeof(File_Change_Record)*max_number_of_entries;
            tables->max = max_number_of_entries;
        }
        
        // NOTE(allen): Initialize nodes of directory watching
        {
            init_sentinel_node(&vars->free_sentinel);
            
            Directory_Listener_Node *listener = (Directory_Listener_Node*)listener_memory;
            int32_t count = listener_memory_size / sizeof(Directory_Listener_Node);
            for (int32_t i = 0; i < count; ++i, ++listener){
                insert_node(&vars->free_sentinel, &listener->node);
            }
        }
        
        // NOTE(allen): Launch the file watching thread
        {
            InitializeCriticalSection(&vars->table_lock);
            
            vars->iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 1);
            
            vars->thread = CreateThread(0, 0,
                                        directory_watching,
                                        system,
                                        0, 0);
        }
        
        result = FileTrack_Good;
    }
    
    return(result);
}

static int32_t
internal_get_parent_name(char *out, int32_t max, char *name){
    int32_t len, slash_i;
    
    char *ptr = name;
    for (; *ptr != 0; ++ptr);
    len = (int32_t)(ptr - name);
    
    // TODO(allen): make this system real
    Assert(len < max);
    
    for (slash_i = len-1;
         slash_i > 0 && name[slash_i] != '\\' && name[slash_i] != '/';
         --slash_i);
    
    for (int32_t i = 0; i < slash_i; ++i){
        out[i] = name[i];
    }
    out[slash_i] = 0;
    
    return(slash_i);
}

static File_Index
internal_get_file_index(BY_HANDLE_FILE_INFORMATION info){
    File_Index hash;
    hash.id[0] = info.nFileIndexLow;
    hash.id[1] = info.nFileIndexHigh;
    hash.id[2] = info.dwVolumeSerialNumber;
    hash.id[3] = 0;
    return(hash);
}

static void
internal_set_time(File_Time *time, BY_HANDLE_FILE_INFORMATION info){
    *time = (((uint64_t)info.ftLastWriteTime.dwHighDateTime << 32) |
             (info.ftLastWriteTime.dwLowDateTime));
}

static void
internal_free_slot(File_Track_Tables *tables, File_Track_Entry *entry){
    Assert(!entry_is_available(entry));
    
    ZeroStruct(*entry);
    entry->hash.id[0] = 0xFFFFFFFF;
    entry->hash.id[1] = 0xFFFFFFFF;
    entry->hash.id[2] = 0xFFFFFFFF;
    entry->hash.id[3] = 0xFFFFFFFF;
    
    --tables->tracked_count;
}

File_Track_Result
begin_tracking_file(File_Track_System *system,
                    char *name,
                    File_Index *index,
                    File_Time *time){
    File_Track_Result result = FileTrack_Good;
    File_Track_Vars *vars = to_vars_(system);
    
    EnterCriticalSection(&vars->table_lock);
    
    File_Track_Tables *tables = to_tables(vars);
    
    // TODO(allen): Try multiple ways to open the
    // file and get as many privledges as possible.
    // Expand the API to be capable of reporting
    // what privledges are available on a file.
    HANDLE file = CreateFile(
        name,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        0,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        0);
    
    if (file != INVALID_HANDLE_VALUE){
        char dir_name[1024];
        int32_t dir_name_len =
            internal_get_parent_name(dir_name, sizeof(dir_name), name);
        
        HANDLE dir = CreateFile(
            dir_name,
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            0,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
            0);
        
        if (dir != INVALID_HANDLE_VALUE){
            BY_HANDLE_FILE_INFORMATION info = {0};
            BY_HANDLE_FILE_INFORMATION dir_info = {0};
            
            if (GetFileInformationByHandle(dir, &dir_info)){
                File_Index dir_hash = internal_get_file_index(dir_info);
                
                File_Lookup_Result dir_lookup = tracking_system_lookup_entry(tables, dir_hash);
                
                if (GetFileInformationByHandle(file, &info)){
                    File_Index hash = internal_get_file_index(info);
                    
                    if (entry_is_available(dir_lookup.entry)){
                        if (tracking_system_has_space(tables, 2)){
                            Directory_Listener_Node *node = (Directory_Listener_Node*)
                                allocate_node(&vars->free_sentinel);
                            if (node){
                                if (CreateIoCompletionPort(dir, vars->iocp, (ULONG_PTR)node, 1)){
                                    ZeroStruct(node->listener.overlapped);
                                    if (ReadDirectoryChangesW(dir,
                                                              node->listener.result,
                                                              sizeof(node->listener.result),
                                                              0,
                                                              FILE_NOTIFY_CHANGE_LAST_WRITE,
                                                              0,
                                                              &node->listener.overlapped,
                                                              0)){
                                        node->listener.dir = dir;
                                        node->listener.user_count = 1;
                                        
                                        // TODO(allen): make this real!
                                        Assert(dir_name_len < sizeof(node->listener.dir_name));
                                        for (int32_t i = 0; i < dir_name_len; ++i){
                                            node->listener.dir_name[i] = dir_name[i];
                                        }
                                        node->listener.dir_name[dir_name_len] = 0;
                                        
                                        dir_lookup.entry->hash = dir_hash;
                                        dir_lookup.entry->file = dir;
                                        dir_lookup.entry->dir = dir;
                                        dir_lookup.entry->listener_node = node;
                                        ++tables->tracked_count;
                                        ++tables->tracked_dir_count;
                                        
                                        File_Lookup_Result lookup =
                                            tracking_system_lookup_entry(tables, hash);
                                        Assert(entry_is_available(lookup.entry));
                                        
                                        lookup.entry->hash = hash;
                                        lookup.entry->file = file;
                                        lookup.entry->dir = node->listener.dir;
                                        lookup.entry->change_pos = -1;
                                        ++tables->tracked_count;
                                        
                                        *index = hash;
                                        internal_set_time(time, info);
                                    }
                                    else{
                                        result = FileTrack_FileSystemError;
                                    }
                                }
                                else{
                                    result = FileTrack_FileSystemError;
                                }
                                
                                if (result != FileTrack_Good){
                                    insert_node(&vars->free_sentinel, &node->node);
                                }
                            }
                            else{
                                result = FileTrack_OutOfListenerMemory;
                            }
                        }
                        else{
                            result = FileTrack_OutOfTableMemory;
                        }
                    }
                    else{
                        if (tracking_system_has_space(tables, 1)){
                            File_Lookup_Result lookup = tracking_system_lookup_entry(tables, hash);
                            if (entry_is_available(lookup.entry)){
                                Directory_Listener_Node *node = dir_lookup.entry->listener_node;
                                ++node->listener.user_count;
                                
                                lookup.entry->hash = hash;
                                lookup.entry->file = file;
                                lookup.entry->dir = node->listener.dir;
                                lookup.entry->change_pos = -1;
                                ++tables->tracked_count;
                                
                                *index = hash;
                                internal_set_time(time, info);
                            }
                            else{
                                result = FileTrack_FileAlreadyTracked;
                            }
                        }
                        else{
                            result = FileTrack_OutOfTableMemory;
                        }
                    }
                }
                else{
                    result = FileTrack_FileSystemError;
                }
            }
            else{
                result = FileTrack_FileSystemError;
            }
            
            if (result != FileTrack_Good && dir != 0){
                CloseHandle(dir);
            }
        }
        else{
            result = FileTrack_FileSystemError;
        }
        
        if (result != FileTrack_Good){
            CloseHandle(file);
        }
    }
    else{
        result = FileTrack_FileNotFound;
    }
    
    LeaveCriticalSection(&vars->table_lock);
    
    return(result);
}

File_Track_Result
internal_get_tracked_file_index(File_Track_Tables *tables,
                                char *name,
                                File_Index *index,
                                File_Track_Entry **entry){
    File_Track_Result result = FileTrack_Good;
    
    HANDLE file = CreateFile(
        name,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        0,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        0);
    
    if (file != INVALID_HANDLE_VALUE){
        BY_HANDLE_FILE_INFORMATION info = {0};
        if (GetFileInformationByHandle(file, &info)){
            File_Index hash;
            hash.id[0] = info.nFileIndexLow;
            hash.id[1] = info.nFileIndexHigh;
            hash.id[2] = info.dwVolumeSerialNumber;
            hash.id[3] = 0;
            
            File_Lookup_Result lookup = tracking_system_lookup_entry(tables, hash);
            if (!entry_is_available(lookup.entry)){
                *index = hash;
                *entry = lookup.entry;
            }
            else{
                result = FileTrack_FileNotTracked;
            }
        }
        else{
            result = FileTrack_FileSystemError;
        }
        CloseHandle(file);
    }
    else{
        result = FileTrack_FileNotFound;
    }
    
    return(result);
}

File_Track_Result
get_tracked_file_index(File_Track_System *system,
                       char *name,
                       File_Index *index){
    File_Track_Result result = FileTrack_Good;
    File_Track_Vars *vars = to_vars_(system);
    File_Track_Tables *tables = 0;
    File_Track_Entry *entry = 0;
    
    EnterCriticalSection(&vars->table_lock);
    tables = to_tables(vars);
    result = internal_get_tracked_file_index(tables, name, index, &entry);
    LeaveCriticalSection(&vars->table_lock);
    
    return(result);
}

File_Track_Result
stop_tracking_file(File_Track_System *system, File_Index index){
    File_Track_Result result = FileTrack_Good;
    File_Track_Vars *vars = to_vars_(system);
    
    EnterCriticalSection(&vars->table_lock);
    
    File_Track_Tables *tables = to_tables(vars);
    
    File_Track_Entry *entry = get_file_entry(tables, index);
    
    if (entry){
        HANDLE dir = entry->dir;
        if (dir != entry->file){
            BY_HANDLE_FILE_INFORMATION dir_info = {0};
            if (GetFileInformationByHandle(dir, &dir_info)){
                File_Index dir_hash =  internal_get_file_index(dir_info);
                
                File_Lookup_Result dir_lookup = tracking_system_lookup_entry(tables, dir_hash);
                
                Assert(!entry_is_available(dir_lookup.entry));
                Directory_Listener_Node *node = dir_lookup.entry->listener_node;
                --node->listener.user_count;
                
                if (node->listener.user_count == 0){
                    insert_node(&vars->free_sentinel, &node->node);
                    CloseHandle(entry->dir);
                    internal_free_slot(tables, dir_lookup.entry);
                    --tables->tracked_dir_count;
                }
                
                CloseHandle(entry->file);
                internal_free_slot(tables, entry);
            }
            else{
                result = FileTrack_FileSystemError;
            }
        }
        else{
            result = FileTrack_FileNotTracked;
        }
    }
    else{
        result = FileTrack_FileNotTracked;
    }
    
    LeaveCriticalSection(&vars->table_lock);
    
    return(result);
}

File_Track_Result
count_tracked_files(File_Track_System *system, int32_t *count){
    File_Track_Result result = FileTrack_Good;
    File_Track_Tables *tables = to_tables(to_vars_(system));
    *count = tables->tracked_count - tables->tracked_dir_count;
    return(result);
}

File_Track_Result
get_tracked_file_time(File_Track_System *system, File_Index index, File_Time *time){
    File_Track_Result result = FileTrack_Good;
    File_Track_Vars *vars = to_vars_(system);
    
    EnterCriticalSection(&vars->table_lock);
    
    File_Track_Tables *tables = to_tables(vars);
    
    File_Track_Entry *entry = get_file_entry(tables, index);
    
    if (entry){
        BY_HANDLE_FILE_INFORMATION info = {0};
        if (GetFileInformationByHandle(entry->file, &info)){
            internal_set_time(time, info);
        }
        else{
            result = FileTrack_FileSystemError;
        }
    }
    else{
        result = FileTrack_FileNotTracked;
    }
    
    LeaveCriticalSection(&vars->table_lock);
    
    return(result);
}

File_Track_Result
get_file_time_now(File_Time *time){
    File_Track_Result result = FileTrack_Good;
    
    FILETIME file_time;
    SYSTEMTIME system_time;
    
    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    *time = (((uint64_t)file_time.dwHighDateTime << 32) |
             (file_time.dwLowDateTime));
    
    return(result);
}

File_Track_Result
move_track_system(File_Track_System *system, void *mem, int32_t size){
    File_Track_Result result = FileTrack_Good;
    File_Track_Vars *vars = to_vars_(system);
    
    EnterCriticalSection(&vars->table_lock);
    
    File_Track_Tables *original_tables = to_tables(vars);
    
    if (original_tables->size < size){
        File_Track_Tables *tables = (File_Track_Tables*)mem;
        
        // NOTE(allen): Initialize main data tables
        {
            tables->size = size;
            
            int32_t likely_entry_size = FILE_ENTRY_COST;
            int32_t max_number_of_entries = (size - sizeof(*tables)) / likely_entry_size;
            
            tables->change_queue = sizeof(*tables);
            tables->file_table = tables->change_queue +
                sizeof(File_Change_Record)*max_number_of_entries;
            tables->max = max_number_of_entries;
        }
        
        if (tables->max > original_tables->max){
            uint32_t original_max = original_tables->max;
            
            uint32_t change_shift = 0;
            uint32_t change_modulo = original_max;
            
            // NOTE(allen): Copy the original change queue
            {
                uint32_t start_pos = original_tables->change_read_pos;
                uint32_t end_pos = original_tables->change_write_pos;
                
                uint32_t changes_count = 0;
                if (original_tables->change_write_pos < original_tables->change_read_pos){
                    changes_count = original_max + end_pos - start_pos;
                }
                else{
                    changes_count = end_pos - start_pos;
                }
                
                change_shift = original_max - start_pos;
                
                File_Change_Record *src = (File_Change_Record*)
                    to_ptr(original_tables, original_tables->change_queue);
                
                File_Change_Record *dst = (File_Change_Record*)
                    to_ptr(tables, tables->change_queue);
                
                for (uint32_t i_dst = 0, i_src = start_pos;
                     i_dst < changes_count;
                     ++i_dst, i_src = (i_src+1)%original_max){
                    dst[i_dst] = src[i_src];
                }
                
                tables->change_read_pos = 0;
                tables->change_write_pos = changes_count;
            }
            
            // NOTE(allen): Rehash the tracking table
            {
                File_Track_Entry *entries = (File_Track_Entry*)
                    to_ptr(original_tables, original_tables->file_table);
                
                for (uint32_t index = 0;
                     index < original_max;
                     ++index){
                    File_Track_Entry *entry = entries + index;
                    if (!entry_is_available(entry)){
                        File_Index hash = entry->hash;
                        File_Lookup_Result lookup =
                            tracking_system_lookup_entry(tables, hash);
                        Assert(entry_is_available(lookup.entry));
                        
                        *lookup.entry = *entry;
                        
                        if (lookup.entry->file != lookup.entry->dir &&
                            lookup.entry->change_pos != -1){
                            lookup.entry->change_pos =
                                (lookup.entry->change_pos+change_shift)%change_modulo;
                        }
                    }
                }
                
                tables->tracked_count = original_tables->tracked_count;
                tables->tracked_dir_count = original_tables->tracked_dir_count;
            }
            
            // NOTE(allen): Update to the new table
            vars->tables = mem;
        }
        else{
            result = FileTrack_MemoryTooSmall;
        }
    }
    else{
        result = FileTrack_MemoryTooSmall;
    }
    
    LeaveCriticalSection(&vars->table_lock);
    
    return(result);
}

File_Track_Result
expand_track_system_listeners(File_Track_System *system, void *mem, int32_t size){
    File_Track_Result result = FileTrack_Good;
    File_Track_Vars *vars = to_vars_(system);
    
    EnterCriticalSection(&vars->table_lock);
    
    if (sizeof(Directory_Listener_Node) <= size){
        Directory_Listener_Node *listener = (Directory_Listener_Node*)mem;
        int32_t count = size / sizeof(Directory_Listener_Node);
        for (int32_t i = 0; i < count; ++i, ++listener){
            insert_node(&vars->free_sentinel, &listener->node);
        }
    }
    else{
        result = FileTrack_MemoryTooSmall;
    }
    
    LeaveCriticalSection(&vars->table_lock);
    
    return(result);
}

File_Track_Result
get_change_event(File_Track_System *system, File_Index *index){
    File_Track_Result result = FileTrack_NoMoreEvents;
    File_Track_Vars *vars = to_vars_(system);
    
    EnterCriticalSection(&vars->table_lock);
    
    File_Track_Tables *tables = to_tables(vars);
    
    uint32_t write_pos = tables->change_write_pos;
    uint32_t read_pos = tables->change_read_pos;
    uint32_t max = tables->max;
    
    File_Change_Record *change_queue =
        (File_Change_Record*)to_ptr(tables, tables->change_queue);
    
    while (read_pos != write_pos){
        File_Change_Record *record = change_queue + read_pos;
        
        read_pos = (read_pos + 1) % max;
        
        if (record->still_active){
            File_Lookup_Result lookup = tracking_system_lookup_entry(tables, record->index);
            if (!entry_is_available(lookup.entry)){
                lookup.entry->change_pos = -1;
                *index = record->index;
                result = FileTrack_Good;
            }
            break;
        }
    }
    
    tables->change_write_pos = write_pos;
    tables->change_read_pos = read_pos;
    
    LeaveCriticalSection(&vars->table_lock);
    
    return(result);
}

File_Track_Result
get_tracked_file_size(File_Track_System *system, File_Index index, uint32_t *size){
    File_Track_Result result = FileTrack_Good;
    File_Track_Vars *vars = to_vars_(system);
    
    EnterCriticalSection(&vars->table_lock);
    
    File_Track_Tables *tables = to_tables(vars);
    
    File_Lookup_Result lookup = tracking_system_lookup_entry(tables, index);
    if (!entry_is_available(lookup.entry)){
        DWORD lo, hi;
        lo = GetFileSize(lookup.entry->file, &hi);
        Assert(hi == 0);
        *size = lo;
    }
    else{
        result = FileTrack_FileNotTracked;
    }
    
    LeaveCriticalSection(&vars->table_lock);
    
    return(result);
}

File_Track_Result
get_tracked_file_data(File_Track_System *system, File_Index index, void *mem, uint32_t size){
    File_Track_Result result = FileTrack_Good;
    File_Track_Vars *vars = to_vars_(system);
    
    EnterCriticalSection(&vars->table_lock);
    
    File_Track_Tables *tables = to_tables(vars);
    
    File_Lookup_Result lookup = tracking_system_lookup_entry(tables, index);
    if (!entry_is_available(lookup.entry)){
        DWORD read_size = 0;
        if (ReadFile(lookup.entry->file, mem, size, &read_size, 0)){
            if (read_size != size){
                result = FileTrack_FileSystemError;
            }
        }
        else{
            result = FileTrack_FileSystemError;
        }
    }
    else{
        result = FileTrack_FileNotTracked;
    }
    
    LeaveCriticalSection(&vars->table_lock);
    
    return(result);
}

File_Track_Result
rewrite_tracked_file(File_Track_System *system, File_Index index,
                     void *data, int32_t size, File_Time *time){
    File_Track_Result result = FileTrack_Good;
    File_Track_Vars *vars = to_vars_(system);
    
    EnterCriticalSection(&vars->table_lock);
    
    File_Track_Tables *tables = to_tables(vars);
    
    File_Lookup_Result lookup = tracking_system_lookup_entry(tables, index);
    if (!entry_is_available(lookup.entry)){
        DWORD written = 0;
        
        lookup.entry->skip_change = 1;
        SetFilePointer(lookup.entry->file, 0, 0, FILE_BEGIN);
        if (WriteFile(lookup.entry->file, data, size, &written, 0)){
            FILETIME file_time;
            SYSTEMTIME system_time;
            
            GetSystemTime(&system_time);
            SystemTimeToFileTime(&system_time, &file_time);
            SetFileTime(lookup.entry->file, 0, 0, &file_time);
            
            FlushFileBuffers(lookup.entry->file);
        }
        else{
            result = FileTrack_FileSystemError;
        }
    }
    else{
        result = FileTrack_FileNotTracked;
    }
    
    LeaveCriticalSection(&vars->table_lock);
    
    return(result);
}

File_Track_Result
shut_down_track_system(File_Track_System *system){
    File_Track_Result result = FileTrack_Good;
    File_Track_Vars *vars = to_vars_(system);
    
    File_Track_Tables *tables = to_tables(vars);
    
    File_Track_Entry *entries = (File_Track_Entry*)to_ptr(tables, tables->file_table);
    
    uint32_t index = 0;
    uint32_t max = tables->max;
    
    DWORD win32_result = 0;
    
    for (; index < max; ++index){
        File_Track_Entry *entry = entries + index;
        if (!entry_is_available(entry)){
            if (!CloseHandle(entry->file)){
                win32_result = 1;
            }
        }
    }
    
    if (!CloseHandle(vars->iocp)){
        win32_result = 1;
    }
    TerminateThread(vars->thread, 0);
    if (!CloseHandle(vars->thread)){
        win32_result = 1;
    }
    
    if (win32_result){
        result = FileTrack_FileSystemError;
    }
    
    DeleteCriticalSection(&vars->table_lock);
    
    return(result);
}

// BOTTOM
