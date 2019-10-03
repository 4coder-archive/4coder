/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 18.07.2017
 *
 * Win32 library wrapper.
 *
 */

// TOP

#error remove this file

#if 0
union Library{
    HMODULE lib;
    FixSize(LIBRARY_TYPE_SIZE);
};

internal b32
system_load_library_direct(Arena *scratch, Library *library, char *name){
    AssertLibrarySizes();
    library->lib = LoadLibraryA(name);
    b32 success = (library->lib != 0);
    if (!success){
        win32_output_error_string(scratch, ErrorString_UseLog);
    }
    return(success);
}

internal void*
system_get_proc(Library *library, char *name){
    return(GetProcAddress(library->lib, name));
}

internal void
system_free_library(Library *library){
    FreeLibrary(library->lib);
    library->lib = 0;
}
#endif

// BOTTOM

