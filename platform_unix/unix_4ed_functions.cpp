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

#if defined(USE_LOG)
# include <stdio.h>
#endif

struct Unix_Vars{
    b32 do_logging;
};

static Unix_Vars unixvars;

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

internal
Sys_Get_Binary_Path_Sig(system_get_binary_path){
    ssize_t size = readlink("/proc/self/exe", out->str, out->memory_size - 1);
    if(size != -1 && size < out->memory_size - 1){
        out->size = size;
        remove_last_folder(out);
        terminate_with_null(out);
        size = out->size;
    } else {
        size = 0;
    }
    
    return size;
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

// BOTTOM

