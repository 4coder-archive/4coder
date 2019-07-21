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

#endif

// BOTTOM

