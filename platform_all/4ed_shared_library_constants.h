/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 19.07.2017
 *
 * Cross platform library constants
 *
 */

// TOP

#if !defined(FRED_SHARED_LIBRARY_CONSTANTS_H)
#define FRED_SHARED_LIBRARY_CONSTANTS_H

// Wrapper functions
union Library;

internal b32
system_load_library_direct(Library *library, char *name);

internal void*
system_get_proc(Library *library, char *name);

internal void
system_free_library(Library *library);

// Shared logic
#define LIBRARY_TYPE_SIZE 32

#define AssertLibrarySizes() Assert(sizeof(Library) == LIBRARY_TYPE_SIZE)

typedef u32 Load_Library_Location;
enum{
    LoadLibrary_CurrentDirectory,
    LoadLibrary_BinaryDirectory,
};

internal b32
system_load_library(Library *library, char *name, Load_Library_Location location, char *full_file_out, u32 full_file_max){
    char space[4096];
    String extension = file_extension(make_string_slowly(name));
    if (!match(extension, DLL)){
        String full_name = make_fixed_width_string(space);
        append(&full_name, name);
        append(&full_name, "." DLL);
        if (terminate_with_null(&full_name)){
            name = space;
        }
        else{
            name = 0;
        }
    }
    
    char path_space[4096];
    String path = make_fixed_width_string(path_space);
    switch (location){
        case LoadLibrary_CurrentDirectory:
        {
            path.size = sysfunc.get_current_path(path.str, path.memory_size);
        }break;
        
        case LoadLibrary_BinaryDirectory:
        {
            path.size = sysfunc.get_4ed_path(path.str, path.memory_size);
        }break;
        
        default: LOG("Invalid library location passed.\n"); break;
    }
    
    b32 success = false;
    if (path.size > 0){
        if (path.str[path.size - 1] != SLASH){
            append(&path, SLASH);
        }
        append(&path, name);
        terminate_with_null(&path);
        success = system_load_library_direct(library, path.str);
        if (success && full_file_out != 0){
            String out = make_string_cap(full_file_out, 0, full_file_max);
            copy(&out, path);
            terminate_with_null(&out);
        }
    }
    
    return(success);
}

internal b32
system_load_library(Library *library, char *name, Load_Library_Location location){
    b32 result = system_load_library(library, name, location, 0, 0);
    return(result);
}

#endif

// BOTTOM

