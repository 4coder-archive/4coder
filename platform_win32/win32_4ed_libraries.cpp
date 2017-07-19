/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 18.07.2017
 *
 * Win32 library wrapper.
 *
 */

// TOP

struct Library{
    HMODULE lib;
};

internal b32
system_load_library(Library *library, char *name){
    String extension = file_extension(make_string_slowly(name));
    char space[4096];
    if (!match(extension, "dll")){
        String full_name = make_fixed_width_string(space);
        append(&full_name, name);
        append(&full_name, ".dll");
        terminate_with_null(&full_name);
        name = space;
    }
    
    library->lib = LoadLibraryA(name);
    b32 success = (library->lib != 0);
    return(success);
}

internal void*
system_get_proc(Library *library, char *name){
    void *result = GetProcAddress(library->lib, name);
    return(result);
}

internal void
system_free_library(Library *library){
    FreeLibrary(library->lib);
    library->lib = 0;
}

// BOTTOM

