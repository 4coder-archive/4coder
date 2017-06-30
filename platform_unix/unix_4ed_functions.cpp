/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 30.06.2017
 *
 * General unix functions
 *
 */

// TOP

#include <sys/mman.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <alloca.h>
#include <errno.h>
#include <time.h>

#if defined(USE_LOG)
# include <stdio.h>
#endif

struct Unix_Vars{
    b32 do_logging;
};

static Unix_Vars unixvars;

//
// 4ed Path
//

internal
Sys_Get_4ed_Path_Sig(system_get_4ed_path){
    ssize_t size = readlink("/proc/self/exe", out, capacity - 1);
    if (size != -1 && size < capacity - 1){
        String str = make_string(out, size);
        remove_last_folder(&str);
        terminate_with_null(&str);
        size = str.size;
    }
    else{
        size = 0;
    }
    return(size);
}

//
// Logging
//

internal
Sys_Log_Sig(system_log){
    if (unixvars.do_logging){
        i32 fd = open("4coder_log.txt", O_WRONLY | O_CREAT, 00640);
        if (fd >= 0){
            do{
                ssize_t written = write(fd, message, length);
                if (written != -1){
                    length -= written;
                    message += written;
                }
            } while(length > 0);
            close(fd);
        }
    }
}

//
// Shared system functions (system_shared.h)
//

internal
Sys_File_Can_Be_Made_Sig(system_file_can_be_made){
    b32 result = access((char*)filename, W_OK) == 0;
    LOGF("%s = %d", filename, result);
    return(result);
}

//
// Memory
//

internal
Sys_Memory_Allocate_Sig(system_memory_allocate){
    // NOTE(allen): This must return the exact base of the vpage.
    // We will count on the user to keep track of size themselves.
    void *result = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if(result == MAP_FAILED){
        LOG("mmap failed\n");
        result = NULL;
    }
    return(result);
}

internal 
Sys_Memory_Set_Protection_Sig(system_memory_set_protection){
    bool32 result = true;
    
    int protect = 0;
    switch (flags & 0x7){
        case 0: protect = PROT_NONE; break;
        
        case MemProtect_Read:
        protect = PROT_READ; break;
        
        case MemProtect_Write:
        case MemProtect_Read|MemProtect_Write:
        protect = PROT_READ | PROT_WRITE; break;
        
        case MemProtect_Execute:
        protect = PROT_EXEC; break;
        
        case MemProtect_Execute|MemProtect_Read:
        protect = PROT_READ | PROT_EXEC; break;
        
        // NOTE(inso): some W^X protection things might be unhappy about this one
        case MemProtect_Execute|MemProtect_Write:
        case MemProtect_Execute|MemProtect_Write|MemProtect_Read:
        protect = PROT_READ | PROT_WRITE | PROT_EXEC; break;
    }
    
    if(mprotect(ptr, size, protect) == -1){
        result = 0;
        LOG("mprotect");
    }
    
    return(result);
}

internal
Sys_Memory_Free_Sig(system_memory_free){
    // NOTE(allen): This must take the exact base of the vpage.
    munmap(ptr, size);
}

//
// Files
//

internal
Sys_Set_File_List_Sig(system_set_file_list){
    DIR *d;
    struct dirent *entry;
    char *fname, *cursor, *cursor_start;
    File_Info *info_ptr;
    i32 character_count, file_count, size, required_size, length;
    b32 clear_list = false;
    
    if(directory == 0){
        system_memory_free(file_list->block, file_list->block_size);
        file_list->block = 0;
        file_list->block_size = 0;
        file_list->infos = 0;
        file_list->count = 0;
        return;
    }
    
    LOGF("%s", directory);
    
    d = opendir(directory);
    if (d){
        if (canon_directory_out != 0){
            u32 length = copy_fast_unsafe_cc(canon_directory_out, directory);
            if (canon_directory_out[length-1] != '/'){
                canon_directory_out[length++] = '/';
            }
            canon_directory_out[length] = 0;
            *canon_directory_size_out = length;
        }
        
        character_count = 0;
        file_count = 0;
        for (entry = readdir(d);
             entry != 0;
             entry = readdir(d)){
            fname = entry->d_name;
            if (match_cc(fname, ".") || match_cc(fname, "..")){
                continue;
            }
            ++file_count;            
            for (size = 0; fname[size]; ++size);
            character_count += size + 1;
        }
        
        required_size = character_count + file_count * sizeof(File_Info);
        if (file_list->block_size < required_size){
            system_memory_free(file_list->block, file_list->block_size);
            file_list->block = system_memory_allocate(required_size);
            file_list->block_size = required_size;
        }
        
        file_list->infos = (File_Info*)file_list->block;
        cursor = (char*)(file_list->infos + file_count);
        
        if (file_list->block != 0){
            rewinddir(d);
            info_ptr = file_list->infos;
            for (entry = readdir(d);
                 entry != 0;
                 entry = readdir(d)){
                fname = entry->d_name;
                if (match_cc(fname, ".") || match_cc(fname, "..")){
                    continue;
                }
                cursor_start = cursor;
                length = copy_fast_unsafe_cc(cursor_start, fname);
                cursor += length;
                
                if(entry->d_type == DT_LNK){
                    struct stat st;
                    if(stat(entry->d_name, &st) != -1){
                        info_ptr->folder = S_ISDIR(st.st_mode);
                    } else {
                        info_ptr->folder = 0;
                    }
                } else {
                    info_ptr->folder = (entry->d_type == DT_DIR);
                }
                
                info_ptr->filename = cursor_start;
                info_ptr->filename_len = length;
                *cursor++ = 0;
                ++info_ptr;
            }
        }
        
        file_list->count = file_count;
        
        closedir(d);
    } else {
        system_memory_free(file_list->block, file_list->block_size);
        file_list->block = 0;
        file_list->block_size = 0;
        file_list->infos = 0;
        file_list->count = 0;
    }
}

internal
Sys_Get_Canonical_Sig(system_get_canonical){
    char* path = (char*) alloca(len + 1);
    char* write_p = path;
    const char* read_p = filename;
    
    // return 0 for relative paths (e.g. cmdline args)
    if(len > 0 && filename[0] != '/'){
        return 0;
    }
    
    if (max == 0){
        return 0;
    }
    
    max -= 1;
    
    while(read_p < filename + len){
        if(read_p == filename || read_p[0] == '/'){
            if(read_p[1] == '/'){
                ++read_p;
            } else if(read_p[1] == '.'){
                if(read_p[2] == '/' || !read_p[2]){
                    read_p += 2;
                } else if(read_p[2] == '.' && (read_p[3] == '/' || !read_p[3])){
                    while(write_p > path && *--write_p != '/');
                    read_p += 3;
                } else {
                    *write_p++ = *read_p++;
                }
            } else {
                *write_p++ = *read_p++;
            }
        } else {
            *write_p++ = *read_p++;
        }
    }
    if(write_p == path) *write_p++ = '/';
    
    if(max >= (write_p - path)){
        memcpy(buffer, path, write_p - path);
    } else {
        write_p = path;
    }
    
#if FRED_INTERNAL
    if(len != (write_p - path) || memcmp(filename, path, len) != 0){
        LOGF("[%.*s] -> [%.*s]", len, filename, (int)(write_p - path), path);
    }
#endif
    
    u32 length = (i32)(write_p - path);
    buffer[length] = 0;
    return(length);
}

internal
Sys_Load_Handle_Sig(system_load_handle){
    b32 result = 0;
    
    int fd = open(filename, O_RDONLY);
    if(fd == -1){
        LOG("open");
    } else {
        *(int*)handle_out = fd;
        result = 1;
    }
    
    return result;
}

internal
Sys_Load_Size_Sig(system_load_size){
    u32 result = 0;
    
    int fd = *(int*)&handle;
    struct stat st;
    
    if(fstat(fd, &st) == -1){
        LOG("fstat");
    } else {
        result = st.st_size;
    }
    
    return result;
}

internal
Sys_Load_File_Sig(system_load_file){
    int fd = *(int*)&handle;
    do {
        ssize_t n = read(fd, buffer, size);
        if(n == -1){
            if(errno != EINTR){
                LOG("read");
                break;
            }
        } else {
            size -= n;
            buffer += n;
        }
    } while(size);
    
    return size == 0;
}

internal
Sys_Load_Close_Sig(system_load_close){
    b32 result = 1;
    
    int fd = *(int*)&handle;
    if(close(fd) == -1){
        LOG("close");
        result = 0;
    }
    
    return result;
}

internal
Sys_Save_File_Sig(system_save_file){
    int fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 00640);
    LOGF("%s %d", filename, size);
    if(fd < 0){
        LOGF("system_save_file: open '%s': %s\n", filename, strerror(errno));
    }
    else{
        do {
            ssize_t written = write(fd, buffer, size);
            if(written == -1){
                if(errno != EINTR){
                    LOG("system_save_file: write");
                    break;
                }
            } else {
                size -= written;
                buffer += written;
            }
        } while(size);
        close(fd);
    }
    
    return (size == 0);
}

//
// Time
//

internal
Sys_Now_Time_Sig(system_now_time){
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    u64 result = (spec.tv_sec * UINT64_C(1000000)) + (spec.tv_nsec / UINT64_C(1000));
    return(result);
}

// BOTTOM

