/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 18.07.2017
 *
 * Win32 library wrapper.
 *
 */

// TOP

union Library{
    HMODULE lib;
    FixSize(LIBRARY_TYPE_SIZE);
};

internal b32
system_load_library_direct(Library *library, char *name){
    AssertLibrarySizes();
    library->lib = LoadLibraryA(name);
    b32 success = (library->lib != 0);
    if (!success){
        win32_output_error_string(false);
    }
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

