/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 18.07.2017
 *
 * Linux library wrapper.
 *
 */

// TOP

struct Library{
    void *lib;
};

internal b32
system_load_library(Library *library, char *name){
    String extension = file_extension(make_string_slowly(name));
    char space[4096];
    if (!match(extension, "so")){
        String full_name = make_fixed_width_string(space);
        append(&full_name, name);
        append(&full_name, ".so");
        terminate_with_null(&full_name);
        name = space;
    }
    
    char path_space[4096];
    i32 size = sysfunc.get_4ed_path(path_space, sizeof(path_space));
    String full_path = make_string(path_space, size, sizeof(path_space));
    append(&full_path, "/");
    append(&full_path, name);
    terminate_with_null(&full_path);
    
    library->lib = dlopen(path, RTLD_LAZY);
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

