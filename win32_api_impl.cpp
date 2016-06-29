/*
Implementation of system level functions that get exposed straight
into the 4coder custom API.  This file need not be split on other platforms,
as this is the only one that will be used for generating headers and docs.
-Allen

27.06.2016 (dd.mm.yyyy)
*/

// TOP

#define API_EXPORT

API_EXPORT int
File_Exists(Application_Links *app, char *filename, int len)/*
DOC_PARAM(filename, the full path to a file)
DOC_PARAM(len, the number of characters in the filename string)
DOC_RETURN(returns non-zero if the file exists, returns zero if the file does not exist)
*/{
    char full_filename_space[1024];
    String full_filename;
    HANDLE file;
    b32 result;
    
    result = 0;
    
    if (len < sizeof(full_filename_space)){
        full_filename = make_fixed_width_string(full_filename_space);
        copy(&full_filename, make_string(filename, len));
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

API_EXPORT int
Directory_CD(Application_Links *app, char *dir, int *len, int capacity, char *rel_path, int rel_len)/*
DOC_PARAM(dir, a string buffer containing a directory)
DOC_PARAM(len, the length of the string in the string buffer)
DOC_PARAM(capacity, the maximum size of the string buffer)
DOC_PARAM(rel_path, the path to change to, may include '.' or '..')
DOC_PARAM(rel_len, the length of the rel_path string)
DOC_RETURN(returns non-zero if the call succeeds, returns zero otherwise)
DOC
(
This call succeeds if the directory exists and the new directory fits inside the dir buffer.
If the call succeeds the dir buffer is filled with the new directory and len contains the
length of the string in the buffer.

For instance if dir contains "C:/Users/MySelf" and rel is "Documents" the buffer will contain
"C:/Users/MySelf/Documents" and len will contain the length of that string.  This call can
also be used with rel set to ".." to traverse to parent folders.
)
*/{
    String directory = make_string(dir, *len, capacity);
    b32 result = 0;
    i32 old_size;
    
    char rel_path_space[1024];
    String rel_path_string = make_fixed_width_string(rel_path_space);
    copy(&rel_path_string, make_string(rel_path, rel_len));
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
                append_partial(&directory, rel_path);
                append_partial(&directory, "\\");
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

API_EXPORT int
Get_4ed_Path(Application_Links *app, char *out, int capacity)/*
DOC_PARAM(out, a buffer that receives the path to the 4ed executable file)
DOC_PARAM(capacity, the maximum capacity of the output buffer)
DOC_RETURN(returns non-zero on success, returns zero on failure)
*/{
    String str = make_string(out, 0, capacity);
    return(system_get_binary_path(&str));
}

// TODO(allen): add a "shown but auto-hides on timer" setting here.
API_EXPORT void
Show_Mouse_Cursor(Application_Links *app, int show)/*
DOC_PARAM(show, The show state to put the mouse cursor into.  If this is 1 the mouse cursor is shown.  If it is 0 the mouse cursor is hidden.)
*/{
    ShowCursor(show);
}

// BOTTOM

