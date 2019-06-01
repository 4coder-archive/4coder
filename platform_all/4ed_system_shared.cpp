/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 09.02.2016
 *
 * Shared system functions
 *
 */

// TOP

#if !defined(FCODER_SYSTEM_SHARED_CPP)
#define FCODER_SYSTEM_SHARED_CPP

//
// Standard implementation of file system stuff based on the file track layer.
//

internal i32
system_get_binary_path_string(String *out){
    out->size = system_get_4ed_path(out->str, out->memory_size);
    return(out->size);
}

internal void
init_shared_vars(){
    shared_vars.scratch = make_arena_system(&sysfunc);
    shared_vars.font_scratch = make_arena_system(&sysfunc);
    shared_vars.pixel_scratch = make_arena_system(&sysfunc);
}

//
// General shared pieces
//

internal void
sysshared_filter_real_files(char **files, i32 *file_count){
    i32 end = *file_count;
    i32 i = 0, j = 0;
    for (; i < end; ++i){
        if (system_file_can_be_made((u8*)files[i])){
            files[j] = files[i];
            ++j;
        }
    }
    *file_count = j;
}

internal b32
sysshared_to_binary_path(String *out_filename, char *filename){
    b32 translate_success = 0;
    i32 max = out_filename->memory_size;
    i32 size = out_filename->size = system_get_4ed_path(out_filename->str, out_filename->memory_size);
    if (size > 0 && size < max-1){
        out_filename->size = size;
        if (append_sc(out_filename, filename) && terminate_with_null(out_filename)){
            translate_success = 1;
        }
    }
    return(translate_success);
}

#endif

// BOTTOM

