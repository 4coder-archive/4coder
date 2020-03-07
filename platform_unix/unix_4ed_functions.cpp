/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 30.06.2017
 *
 * General unix functions
 *
 */

// TOP

#error IS THIS STILL REAL? (February 27th 2020)

#if !defined(FD_CHECK)
#define FD_CHECK()
#endif

struct Unix_Vars{
    b32 did_first_log;
};
global Unix_Vars unixvars;

//
// 4ed Path
//

internal
Sys_Get_Current_Path_Sig(system_get_current_path){
    i32 result = 0;
    char *d = getcwd(out, capacity);
    if (d == out){
        result = strlen(out);
    }
    return(result);
}

//
// Shared system functions (system_shared.h)
//

internal
Sys_File_Can_Be_Made_Sig(system_file_can_be_made){
    b32 result = access((char*)filename, W_OK) == 0;
    //LOGF("%s = %d\n", filename, result);
    return(result);
}

//
// Memory
//

internal void*
system_memory_allocate_extended(void *base, u64 size){
    // NOTE(allen): This must return the exact base of the vpage.
    // We will count on the user to keep track of size themselves.
    void *result = mmap(base, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (result == MAP_FAILED){
        result = 0;
    }
    return(result);
}

internal
Sys_Memory_Allocate_Sig(system_memory_allocate){
    void *result = system_memory_allocate_extended(0, size);
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
    if (directory == 0){
        system_memory_free(file_list->block, file_list->block_size);
        file_list->block = 0;
        file_list->block_size = 0;
        file_list->infos = 0;
        file_list->count = 0;
        return;
    }
    
    //LOGF("%s\n", directory);
    
    DIR *d = opendir(directory);
    if (d != 0){
        if (canon_directory_out != 0){
            u32 length = copy_fast_unsafe_cc(canon_directory_out, directory);
            if (canon_directory_out[length-1] != '/'){
                canon_directory_out[length++] = '/';
            }
            canon_directory_out[length] = 0;
            *canon_directory_size_out = length;
        }
        
        i32 character_count = 0;
        i32 file_count = 0;
        for (struct dirent *entry = readdir(d);
             entry != 0;
             entry = readdir(d)){
            char *fname = entry->d_name;
            if (match_cc(fname, ".") || match_cc(fname, "..")){
                continue;
            }
            ++file_count;
            i32 size = 0;
            for (; fname[size]; ++size);
            character_count += size + 1;
        }
        
        i32 required_size = character_count + file_count * sizeof(File_Info);
        if (file_list->block_size < required_size){
            system_memory_free(file_list->block, file_list->block_size);
            file_list->block = system_memory_allocate(required_size);
            file_list->block_size = required_size;
        }
        
        file_list->infos = (File_Info*)file_list->block;
        char *cursor = (char*)(file_list->infos + file_count);
        
        if (file_list->block != 0){
            rewinddir(d);
            File_Info *info_ptr = file_list->infos;
            for (struct dirent *entry = readdir(d);
                 entry != 0;
                 entry = readdir(d)){
                char *fname = entry->d_name;
                if (match(fname, ".") || match(fname, "..")){
                    continue;
                }
                char *cursor_start = cursor;
                i32 length = copy_fast_unsafe_cc(cursor_start, fname);
                cursor += length;
                
                if (entry->d_type == DT_LNK){
                    struct stat st;
                    if (stat(entry->d_name, &st) != -1){
                        info_ptr->folder = S_ISDIR(st.st_mode);
                    }
                    else{
                        info_ptr->folder = false;
                    }
                }
                else{
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
    }
    else{
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
    
    while (read_p < filename + len){
        if (read_p == filename || read_p[0] == '/'){
            if (read_p[1] == '/'){
                ++read_p;
            }
            else if(read_p[1] == '.'){
                if (read_p[2] == '/' || !read_p[2]){
                    read_p += 2;
                } else if(read_p[2] == '.' && (read_p[3] == '/' || !read_p[3])){
                    while(write_p > path && *--write_p != '/');
                    read_p += 3;
                }
                else {
                    *write_p++ = *read_p++;
                }
            }
            else{
                *write_p++ = *read_p++;
            }
        }
        else{
            *write_p++ = *read_p++;
        }
    }
    if (write_p == path) *write_p++ = '/';
    
    if (max >= (write_p - path)){
        memcpy(buffer, path, write_p - path);
    }
    else{
        write_p = path;
    }
    
#if defined(FRED_INTERNAL)
    if (len != (write_p - path) || memcmp(filename, path, len) != 0){
        //LOGF("[%.*s] -> [%.*s]\n", len, filename, (int)(write_p - path), path);
    }
#endif
    
    u32 length = (i32)(write_p - path);
    buffer[length] = 0;
    return(length);
}

internal
Sys_Load_Handle_Sig(system_load_handle){
    b32 result = false;
    
    FD_CHECK();
    
    i32 fd = open(filename, O_RDONLY);
    if (fd == -1 || fd == 0){
        //LOGF("upable to open file descriptor for %s\n", filename);
    }
    else{
        //LOGF("file descriptor (%d) == file %s\n", fd, filename);
        *(i32*)handle_out = fd;
        result = true;
    }
    
    return(result);
}

internal
Sys_Load_Size_Sig(system_load_size){
    u32 result = 0;
    
    i32 fd = *(i32*)&handle;
    struct stat st = {};
    
    if (fstat(fd, &st) == -1){
        //LOGF("unable to stat a file\n");
    }
    else{
        //LOGF("file descriptor (%d) has size %d\n", fd, (i32)st.st_size);
        result = st.st_size;
    }
    
    return(result);
}

internal
Sys_Load_File_Sig(system_load_file){
    i32 fd = *(i32*)&handle;
    
    do{
        ssize_t n = read(fd, buffer, size);
        if (n == -1){
            if (errno != EINTR){
                //LOGF("error reading from file descriptor (%d)\n", fd);
                break;
            }
        }
        else{
            size -= n;
            buffer += n;
        }
    } while(size);
    
    return(size == 0);
}

internal
Sys_Load_Close_Sig(system_load_close){
    b32 result = true;
    
    i32 fd = *(i32*)&handle;
    if (close(fd) == -1){
        //LOGF("error closing file descriptor (%d)\n", fd);
        result = false;
    }
    else{
        //LOGF("file descriptor (%d) closed\n", fd);
    }
    
    FD_CHECK();
    
    return(result);
}

internal
Sys_Save_File_Sig(system_save_file){
    i32 fd = open(filename, O_WRONLY|O_TRUNC|O_CREAT, 00640);
    
    //LOGF("%s %d\n", filename, size);
    if (fd < 0){
        //LOGF("error: open '%s': %s\n", filename, strerror(errno));
    }
    else{
        do{
            ssize_t written = write(fd, buffer, size);
            if (written == -1){
                if (errno != EINTR){
                    break;
                }
            }
            else{
                size -= written;
                buffer += written;
            }
        }while(size);
        close(fd);
    }
    
    return(size == 0);
}

//
// File System
//

internal
Sys_File_Exists_Sig(system_file_exists){
    int result = 0;
    char buff[PATH_MAX] = {};
    
    if (len + 1 > PATH_MAX){
        //LOG("system_directory_has_file: path too long\n");
    }
    else{
        memcpy(buff, filename, len);
        buff[len] = 0;
        struct stat st;
        result = stat(buff, &st) == 0 && S_ISREG(st.st_mode);
    }
    
    //LOGF("%s: %d\n", buff, result);
    
    return(result);
}

// BOTTOM

