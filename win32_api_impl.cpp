/*
Implementation of system level functions that get exposed straight
into the 4coder custom API.  This file need not be split on other platforms,
as this is the only one that will be used for generating headers and docs.
-Allen

27.06.2016 (dd.mm.yyyy)
*/

// TOP

#define API_EXPORT

API_EXPORT void*
Memory_Allocate(Application_Links *app, int32_t size)/*
DOC_PARAM(size, The size in bytes of the block that should be returned.)
DOC(This calls to a low level OS allocator which means it is best used
for infrequent, large allocations.  The size of the block must be remembered
if it will be freed or if it's mem protection status will be changed.)
DOC_SEE(memory_free)
*/{
    void *result = VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    return(result);
}

API_EXPORT bool32
Memory_Set_Protection(Application_Links *app, void *ptr, int32_t size, Memory_Protect_Flags flags)/*
DOC_PARAM(ptr, The base of the block on which to set memory protection flags.)
DOC_PARAM(size, The size that was originally used to allocate this block.)
DOC_PARAM(flags, The new memory protection flags.)
DOC(This call sets the memory protection flags of a block of memory that was previously
allocate by memory_allocate.)
DOC_SEE(memory_allocate)
DOC_SEE(Memory_Protect_Flags)
*/{
    bool32 result = false;
    DWORD old_protect = 0;
    DWORD protect = 0;
    
    flags = flags & 0x7;
    
    switch (flags){
        case 0:
        protect = PAGE_NOACCESS; break;
        
        case MemProtect_Read:
        protect = PAGE_READONLY; break;
        
        case MemProtect_Write:
        case MemProtect_Read|MemProtect_Write:
        protect = PAGE_READWRITE; break;
        
        case MemProtect_Execute:
        protect = PAGE_EXECUTE; break;
        
        case MemProtect_Execute|MemProtect_Read:
        protect = PAGE_EXECUTE_READ; break;
        
        case MemProtect_Execute|MemProtect_Write:
        case MemProtect_Execute|MemProtect_Write|MemProtect_Read:
        protect = PAGE_EXECUTE_READWRITE; break;
    }
    
    VirtualProtect(ptr, size, protect, &old_protect);
    return(result);
}

API_EXPORT void
Memory_Free(Application_Links *app, void *ptr, int32_t size)/*
DOC_PARAM(mem, The base of a block to free.)
DOC_PARAM(size, The size that was originally used to allocate this block.)
DOC(This call frees a block of memory that was previously allocated by
memory_allocate.)
DOC_SEE(memory_allocate)
*/{
    VirtualFree(ptr, 0, MEM_RELEASE);
}

API_EXPORT bool32
File_Exists(Application_Links *app, char *filename, int32_t len)/*
DOC_PARAM(filename, This parameter specifies the full path to a file; it need not be null terminated.)
DOC_PARAM(len, This parameter specifies the length of the filename string.)
DOC_RETURN(This call returns non-zero if and only if the file exists.)
*/{
    char full_filename_space[1024];
    String full_filename;
    HANDLE file;
    b32 result = 0;
    
    if (len < sizeof(full_filename_space)){
        full_filename = make_fixed_width_string(full_filename_space);
        copy_ss(&full_filename, make_string(filename, len));
        terminate_with_null(&full_filename);
        
        file = CreateFile(full_filename.str, GENERIC_READ, 0, 0,
                          OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        
        if (file != INVALID_HANDLE_VALUE){
            CloseHandle(file);
            result = 1;
        }
    }
    
    return(result);
}

API_EXPORT bool32
Directory_CD(Application_Links *app, char *dir, int32_t *len, int32_t capacity, char *rel_path, int32_t rel_len)/*
DOC_PARAM(dir, This parameter provides a character buffer that stores a directory; it need not be null terminated.)
DOC_PARAM(len, This parameter specifies the length of the dir string.)
DOC_PARAM(capacity, This parameter specifies the maximum size of the dir string.)
DOC_PARAM(rel_path, This parameter specifies the path to change to, may include '.' or '..'; it need not be null terminated.)
DOC_PARAM(rel_len, This parameter specifies the length of the rel_path string.)
DOC_RETURN(This call returns non-zero if the call succeeds.)
DOC
(
This call succeeds if the new directory exists and the it fits inside the
dir buffer. If the call succeeds the dir buffer is filled with the new
directory and len is overwritten with the length of the new string in the buffer.

For instance if dir contains "C:/Users/MySelf" and rel is "Documents" the buffer
will contain "C:/Users/MySelf/Documents" and len will contain the length of that
string.  This call can also be used with rel set to ".." to traverse to parent
folders.
)
*/{
    String directory = make_string_cap(dir, *len, capacity);
    b32 result = 0;
    i32 old_size;
    
    char rel_path_space[1024];
    String rel_path_string = make_fixed_width_string(rel_path_space);
    copy_ss(&rel_path_string, make_string(rel_path, rel_len));
    terminate_with_null(&rel_path_string);
    
    if (rel_path[0] != 0){
        if (rel_path[0] == '.' && rel_path[1] == 0){
            result = 1;
        }
        else if (rel_path[0] == '.' && rel_path[1] == '.' && rel_path[2] == 0){
            result = remove_last_folder(&directory);
            terminate_with_null(&directory);
        }
        else{
            if (directory.size + rel_len + 1 > directory.memory_size){
                old_size = directory.size;
                append_partial_sc(&directory, rel_path);
                append_s_char(&directory, '\\');
                if (Win32DirectoryExists(directory.str)){
                    result = 1;
                }
                else{
                    directory.size = old_size;
                }
            }
        }
    }
    
    *len = directory.size;
    
    return(result);
}

API_EXPORT bool32
Get_4ed_Path(Application_Links *app, char *out, int32_t capacity)/*
DOC_PARAM(out, This parameter provides a character buffer that receives the path to the 4ed executable file.)
DOC_PARAM(capacity, This parameter specifies the maximum capacity of the out buffer.)
DOC_RETURN(This call returns non-zero on success.)
*/{
    String str = make_string_cap(out, 0, capacity);
    return(system_get_binary_path(&str));
}

// TODO(allen): add a "shown but auto-hides on timer" setting here.
API_EXPORT void
Show_Mouse_Cursor(Application_Links *app, Mouse_Cursor_Show_Type show)/*
DOC_PARAM(show, This parameter specifies the new state of the mouse cursor.)
DOC_SEE(Mouse_Cursor_Show_Type)
*/{
    switch (show){
        case MouseCursorShow_Never:
        ShowCursor(false);
        break;
        
        case MouseCursorShow_Always:
        ShowCursor(true);
        break;
        
        // TODO(allen): MouseCursor_HideWhenStill
    }
}

API_EXPORT void
Toggle_Fullscreen(Application_Links *app)/*
DOC(This call tells 4coder to switch into or out of full screen mode.
The changes of full screen mode do not take effect until the end of the current frame.
On Windows this call will not work unless 4coder was started in "stream mode".
Stream mode can be enabled with -S or -F flags on the command line to 4ed.)
*/{
    /* NOTE(allen): Don't actually change window size now!
    Tell the platform layer to do the toggle (or to cancel the toggle)
    later when the app.step function isn't running. If the size changes
    mid step, it messes up the rendering rules and stuff. */
    
    // NOTE(allen): On windows we must be in stream mode to go fullscreen.
    if (win32vars.settings.stream_mode){
        win32vars.do_toggle = !win32vars.do_toggle;
    }
    else{
        app->print_message(app, literal("WARNING: Cannot go full screen unless 4coder is in stream mode\n Use the flag -S to put 4coder in stream mode.\n"));
    }
}

API_EXPORT bool32
Is_Fullscreen(Application_Links *app)/*
DOC_SEE(This call returns true if the 4coder is in full screen mode.  This call
takes toggles that have already occured this frame into account.  So it may return
true even though the frame has not ended and actually put 4coder into full screen. If
it returns true though, 4coder will definitely be full screen by the beginning of the next
frame if the state is not changed.)
*/{
    /* NOTE(allen): This is a fancy way to say 'full_screen XOR do_toggle'
    This way this function can always report the state the fullscreen
    will have when the next frame runs, given the number of toggles
    that have occurred this frame and the original value. */
    bool32 result = (win32vars.full_screen + win32vars.do_toggle) & 1;
    return(result);
}

API_EXPORT void
Send_Exit_Signal(Application_Links *app)/*
DOC_SEE(This call sends a signal to 4coder to attempt to exit.  If there are unsaved
files this triggers a dialogue ensuring you're okay with closing.)
*/{
    win32vars.send_exit_signal = 1;
}

// BOTTOM

