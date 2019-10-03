/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 19.07.2017
 *
 * Cross platform library constants
 *
 */

// TOP

#error remove this file

#if 0
#if !defined(FRED_SHARED_LIBRARY_CONSTANTS_H)
#define FRED_SHARED_LIBRARY_CONSTANTS_H

// Wrapper functions
union Library;

internal b32
system_load_library_direct(Arena *scratch, Library *library, char *name);

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
system_load_library(Arena *scratch, Library *library, char *name_cstr, Load_Library_Location location, char *full_file_out, u32 full_file_max){
    Temp_Memory temp = begin_temp(scratch);
    
    String_Const_char name = SCchar(name_cstr);
    String_Const_char extension = string_file_extension(name);
    if (!string_match(extension, string_litexpr( DLL ))){
        String_Const_char full_name = push_stringf(scratch, "%.*s." DLL, string_expand(name));
        name_cstr = full_name.str;
    }
    
    String_Const_u8 path = {};
    switch (location){
        case LoadLibrary_CurrentDirectory:
        {
            path = sysfunc.get_current_path(scratch);
        }break;
        
        case LoadLibrary_BinaryDirectory:
        {
            path = sysfunc.get_4ed_path(scratch);
        }break;
        
        //default: LOG("Invalid library location passed.\n"); break;
    }
    
    b32 success = false;
    if (path.size > 0){
        if (path.str[path.size - 1] != SLASH){
            path = push_u8_stringf(scratch, "%.*s%c%.*s", string_expand(path), SLASH, string_expand(name));
        }
        else{
            path = push_u8_stringf(scratch, "%.*s%.*s", string_expand(path), string_expand(name));
        }
        success = system_load_library_direct(scratch, library, (char*)path.str);
    }
    if (success && full_file_out != 0 && full_file_out > 0){
        u32 fill_size = clamp_top((u32)(path.size), (u32)(full_file_max - 1));
        block_copy(full_file_out, path.str, fill_size);
        full_file_out[fill_size] = 0;
    }
    
    end_temp(temp);
    
    return(success);
}

internal b32
system_load_library(Arena *scratch, Library *library, char *name, Load_Library_Location location){
    return(system_load_library(scratch, library, name, location, 0, 0));
}

#endif
#endif

// BOTTOM

