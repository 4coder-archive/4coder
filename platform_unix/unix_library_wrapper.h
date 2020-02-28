/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 18.07.2017
 *
 * Linux library wrapper.
 *
 */

// TOP

#error IS THIS STILL REAL? (February 27th 2020)

union Library{
    void *lib;
    FixSize(LIBRARY_TYPE_SIZE);
};

internal b32
system_load_library_direct(Library *library, char *name){
    AssertLibrarySizes();
    library->lib = dlopen(name, RTLD_LAZY);
    b32 success = (library->lib != 0);
    return(success);
}

internal void*
system_get_proc(Library *library, char *name){
    void *result = dlsym(library->lib, name);
    return(result);
}

internal void
system_free_library(Library *library){
    dlclose(library->lib);
    library->lib = 0;
}

// BOTTOM

