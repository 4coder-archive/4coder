/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 20.07.2016
 *
 * File tracking win32.
 *
 */

// TOP

#include "4ed_file_track.h"
#include "4ed_file_track_general.cpp"

#include <Windows.h>

struct Win32_Directory_Listener{
    OVERLAPPED overlapped;
    u8 result[2048];
    HANDLE dir;
    i32 user_count;
};
global_const OVERLAPPED null_overlapped = {};

struct Win32_Directory_Listener_Node{
    DLL_Node node;
    Win32_Directory_Listener listener;
};

struct Win32_File_Track_Vars{
    HANDLE iocp;
    CRITICAL_SECTION table_lock;
    void *tables;
    DLL_Node free_sentinel;
};

struct Win32_File_Track_Entry{
    File_Index hash;
    HANDLE dir;
    Win32_Directory_Listener_Node *listener_node;
};

#define to_vars(s) ((Win32_File_Track_Vars*)(s))
#define to_tables(v) ((File_Track_Tables*)(v->tables))

FILE_TRACK_LINK File_Track_Result
init_track_system(File_Track_System *system, Partition *scratch, void *table_memory, i32 table_memory_size, void *listener_memory, i32 listener_memory_size){
    File_Track_Result result = FileTrack_MemoryTooSmall;
    Win32_File_Track_Vars *vars = to_vars(system);
    
    Assert(sizeof(Win32_File_Track_Entry) <= sizeof(File_Track_Entry));
    
    if (enough_memory_to_init_table(table_memory_size) &&
        sizeof(Win32_Directory_Listener_Node) <= listener_memory_size){
        
        // NOTE(allen): Initialize main data tables
        vars->tables = table_memory;
        File_Track_Tables *tables = to_tables(vars);
        init_table_memory(tables, table_memory_size);
        
        // NOTE(allen): Initialize nodes of directory watching
        {
            init_sentinel_node(&vars->free_sentinel);
            
            Win32_Directory_Listener_Node *listener = (Win32_Directory_Listener_Node*)listener_memory;
            i32 count = listener_memory_size / sizeof(Win32_Directory_Listener_Node);
            for (i32 i = 0; i < count; ++i, ++listener){
                insert_node(&vars->free_sentinel, &listener->node);
            }
        }
        
        // NOTE(allen): Prepare the file tracking synchronization objects.
        {
            InitializeCriticalSection(&vars->table_lock);
            vars->iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 1);
        }
        
        result = FileTrack_Good;
    }
    
    return(result);
}

internal i32
internal_get_parent_name(u8 *out, i32 max, u8 *name){
    u8 *ptr = name;
    for (; *ptr != 0; ++ptr);
    i32 len = (i32)(ptr - name);
    
    // TODO(allen): make this system real
    Assert(len < max);
    
    i32 slash_i = len-1;
    for (;slash_i > 0 && name[slash_i] != '\\' && name[slash_i] != '/';--slash_i);
    
    for (i32 i = 0; i < slash_i; ++i) out[i] = name[i];
    out[slash_i] = 0;
    
    return(slash_i);
}

internal File_Index
internal_get_file_index(BY_HANDLE_FILE_INFORMATION info){
    File_Index hash;
    hash.id[0] = info.nFileIndexLow;
    hash.id[1] = info.nFileIndexHigh;
    hash.id[2] = info.dwVolumeSerialNumber;
    hash.id[3] = 0;
    return(hash);
}

#define FLAGS (                 \
FILE_NOTIFY_CHANGE_FILE_NAME  | \
FILE_NOTIFY_CHANGE_DIR_NAME   | \
FILE_NOTIFY_CHANGE_ATTRIBUTES | \
FILE_NOTIFY_CHANGE_SIZE       | \
FILE_NOTIFY_CHANGE_LAST_WRITE | \
FILE_NOTIFY_CHANGE_SECURITY   | \
FILE_NOTIFY_CHANGE_CREATION   | \
0)

//FILE_NOTIFY_CHANGE_LAST_ACCESS| \

FILE_TRACK_LINK File_Track_Result
add_listener(File_Track_System *system, Partition *scratch, u8 *filename){
    File_Track_Result result = FileTrack_Good;
    Win32_File_Track_Vars *vars = to_vars(system);
    
    EnterCriticalSection(&vars->table_lock);
    {
        File_Track_Tables *tables = to_tables(vars);
        
        u8 dir_name[1024];
        internal_get_parent_name(dir_name, ArrayCount(dir_name), filename);
        
        HANDLE dir = CreateFile_utf8(dir_name, FILE_LIST_DIRECTORY,
                                     FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                     0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, 0);
        
        if (dir != INVALID_HANDLE_VALUE){
            BY_HANDLE_FILE_INFORMATION dir_info = {};
            DWORD getinfo_result = GetFileInformationByHandle(dir, &dir_info);
            
            if (getinfo_result){
                File_Index dir_hash = internal_get_file_index(dir_info);
                File_Track_Entry *dir_lookup = tracking_system_lookup_entry(tables, dir_hash);
                Win32_File_Track_Entry *win32_entry = (Win32_File_Track_Entry*)dir_lookup;
                
                if (entry_is_available(dir_lookup)){
                    if (tracking_system_has_space(tables, 1)){
                        Win32_Directory_Listener_Node *node = (Win32_Directory_Listener_Node*)
                            allocate_node(&vars->free_sentinel);
                        if (node){
                            if (CreateIoCompletionPort(dir, vars->iocp, (ULONG_PTR)node, 1)){
                                node->listener.overlapped = null_overlapped;
                                if (ReadDirectoryChangesW(dir, node->listener.result, sizeof(node->listener.result), 1, FLAGS, 0, &node->listener.overlapped, 0)){
                                    node->listener.dir = dir;
                                    node->listener.user_count = 1;
                                    
                                    win32_entry->hash = dir_hash;
                                    win32_entry->dir = dir;
                                    win32_entry->listener_node = node;
                                    ++tables->tracked_count;
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
                    Win32_Directory_Listener_Node *node = win32_entry->listener_node;
                    ++node->listener.user_count;
                }
            }
            else{
                result = FileTrack_FileSystemError;
            }
        }
        else{
            result = FileTrack_FileSystemError;
        }
        
        if (result != FileTrack_Good && dir != 0 && dir != INVALID_HANDLE_VALUE){
            CloseHandle(dir);
        }
    }
    LeaveCriticalSection(&vars->table_lock);
    
    return(result);
}

FILE_TRACK_LINK File_Track_Result
remove_listener(File_Track_System *system, Partition *scratch, u8 *filename){
    File_Track_Result result = FileTrack_Good;
    Win32_File_Track_Vars *vars = to_vars(system);
    
    EnterCriticalSection(&vars->table_lock);
    
    File_Track_Tables *tables = to_tables(vars);
    
    // TODO(allen): make this real!
    u8 dir_name[1024];
    internal_get_parent_name(dir_name, ArrayCount(dir_name), filename);
    
    HANDLE dir = CreateFile_utf8(dir_name, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, 0);
    
    if (dir != INVALID_HANDLE_VALUE){
        BY_HANDLE_FILE_INFORMATION dir_info = {};
        DWORD getinfo_result = GetFileInformationByHandle(dir, &dir_info);
        
        if (getinfo_result){
            File_Index dir_hash =  internal_get_file_index(dir_info);
            File_Track_Entry *dir_lookup = tracking_system_lookup_entry(tables, dir_hash);
            Win32_File_Track_Entry *win32_dir = (Win32_File_Track_Entry*)dir_lookup;
            
            Assert(!entry_is_available(dir_lookup));
            Win32_Directory_Listener_Node *node = win32_dir->listener_node;
            --node->listener.user_count;
            
            if (node->listener.user_count == 0){
                insert_node(&vars->free_sentinel, &node->node);
                CancelIo(win32_dir->dir);
                CloseHandle(win32_dir->dir);
                internal_free_slot(tables, dir_lookup);
            }
        }
        else{
            result = FileTrack_FileSystemError;
        }
        
        CloseHandle(dir);
    }
    else{
        result = FileTrack_FileSystemError;
    }
    
    LeaveCriticalSection(&vars->table_lock);
    
    return(result);
}

FILE_TRACK_LINK File_Track_Result
move_track_system(File_Track_System *system, Partition *scratch, void *mem, i32 size){
    File_Track_Result result = FileTrack_Good;
    Win32_File_Track_Vars *vars = to_vars(system);
    
    EnterCriticalSection(&vars->table_lock);
    File_Track_Tables *original_tables = to_tables(vars);
    result = move_table_memory(original_tables, mem, size);
    if (result == FileTrack_Good){
        vars->tables = mem;
    }
    LeaveCriticalSection(&vars->table_lock);
    
    return(result);
}

FILE_TRACK_LINK File_Track_Result
expand_track_system_listeners(File_Track_System *system, Partition *scratch, void *mem, i32 size){
    File_Track_Result result = FileTrack_Good;
    Win32_File_Track_Vars *vars = to_vars(system);
    
    EnterCriticalSection(&vars->table_lock);
    
    if (sizeof(Win32_Directory_Listener_Node) <= size){
        Win32_Directory_Listener_Node *listener = (Win32_Directory_Listener_Node*)mem;
        i32 count = size / sizeof(Win32_Directory_Listener_Node);
        for (i32 i = 0; i < count; ++i, ++listener){
            insert_node(&vars->free_sentinel, &listener->node);
        }
    }
    else{
        result = FileTrack_MemoryTooSmall;
    }
    
    LeaveCriticalSection(&vars->table_lock);
    
    return(result);
}

FILE_TRACK_LINK File_Track_Result
get_change_event(File_Track_System *system, Partition *scratch, u8 *buffer, i32 max, i32 *size){
    File_Track_Result result = FileTrack_NoMoreEvents;
    Win32_File_Track_Vars *vars = to_vars(system);
    
    local_persist i32 has_buffered_event = 0;
    local_persist DWORD offset = 0;
    local_persist Win32_Directory_Listener listener = {};
    
    EnterCriticalSection(&vars->table_lock);
    
    OVERLAPPED *overlapped = 0;
    DWORD length = 0;
    ULONG_PTR key = 0;
    
    b32 has_result = 0;
    
    if (has_buffered_event){
        has_buffered_event = 0;
        has_result = 1;
    }
    else{
        if (GetQueuedCompletionStatus(vars->iocp, &length, &key, &overlapped, 0)){
            Win32_Directory_Listener *listener_ptr = (Win32_Directory_Listener*)overlapped;
            
            // NOTE(allen): Get a copy of the state of this node so we can set the node to work listening for changes again right away.
            listener = *listener_ptr;
            
            listener_ptr->overlapped = null_overlapped;
            ReadDirectoryChangesW(listener_ptr->dir, listener_ptr->result, sizeof(listener_ptr->result), 1, FLAGS, 0, &listener_ptr->overlapped, 0);
            
            offset = 0;
            has_result = 1;
        }
    }
    
    if (has_result){
        FILE_NOTIFY_INFORMATION *info = (FILE_NOTIFY_INFORMATION*)(listener.result + offset);
        
        i32 len = info->FileNameLength / 2;
        i32 dir_len = GetFinalPathNameByHandle_utf8(listener.dir, 0, 0, FILE_NAME_NORMALIZED);
        
        i32 req_size = dir_len + (len + 1)*2;
        *size = req_size;
        
        if (req_size < max){
            u32 path_pos = GetFinalPathNameByHandle_utf8(listener.dir, buffer, max, FILE_NAME_NORMALIZED);
            buffer[path_pos++] = '\\';
            
            u32 name_max = max - path_pos;
            u8 *name_buffer = buffer + path_pos;
            
            b32 convert_error = false;
            u32 name_pos = (u32)utf16_to_utf8_minimal_checking(name_buffer, name_max, (u16*)info->FileName, len, &convert_error);
            
            if (name_pos < name_max){
                if (!convert_error){
                    u32 pos = path_pos + name_pos;
                    *size = pos;
                    if (buffer[0] == '\\'){
                        for (u32 i = 0; i+4 < pos; ++i){
                            buffer[i] = buffer[i+4];
                        }
                        *size -= 4;
                    }
                    result = FileTrack_Good;
                }
                else{
                    result = FileTrack_FileSystemError;
                }
            }
            else{
                result = FileTrack_MemoryTooSmall;
            }
            
        }
        else{
            // TODO(allen): Need some way to stash this result so that if the
            // user comes back with more memory we can give them the change
            // notification they missed.
            result = FileTrack_MemoryTooSmall;
        }
        
        if (info->NextEntryOffset != 0){
            // TODO(allen): We're not ready to handle this yet.
            // For now I am breaking.  In the future, if there
            // are more results we should stash them and return
            // them in future calls.
            offset += info->NextEntryOffset;
            has_buffered_event = 1;
        }
    }
    
    LeaveCriticalSection(&vars->table_lock);
    
    return(result);
}

FILE_TRACK_LINK File_Track_Result
shut_down_track_system(File_Track_System *system, Partition *scratch){
    File_Track_Result result = FileTrack_Good;
    Win32_File_Track_Vars *vars = to_vars(system);
    
    DWORD win32_result = 0;
    
    // NOTE(allen): Close all the handles stored in the table.
    {
        File_Track_Tables *tables = to_tables(vars);
        
        File_Track_Entry *entries = (File_Track_Entry*)to_ptr(tables, tables->file_table);
        u32 max = tables->max;
        
        for (u32 index = 0; index < max; ++index){
            File_Track_Entry *entry = entries + index;
            
            if (!entry_is_available(entry)){
                Win32_File_Track_Entry *win32_entry = (Win32_File_Track_Entry*)entry;
                if (!CancelIo(win32_entry->dir)){
                    win32_result = 1;
                }
                if (!CloseHandle(win32_entry->dir)){
                    win32_result = 1;
                }
            }
        }
    }
    
    // NOTE(allen): Close all the global track system resources.
    {
        if (!CloseHandle(vars->iocp)){
            win32_result = 1;
        }
        DeleteCriticalSection(&vars->table_lock);
    }
    
    if (win32_result){
        result = FileTrack_FileSystemError;
    }
    
    return(result);
}

// BOTTOM
