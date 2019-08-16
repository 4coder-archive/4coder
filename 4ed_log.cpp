/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 14.08.2019
 *
 * Core logging implementation.
 *
 */

// TOP

global Log global_log = {};

internal void
log_init(System_Functions *system){
    global_log.mutex = system->mutex_make();
    global_log.mutex_acquire = system->mutex_acquire;
    global_log.mutex_release = system->mutex_release;
    global_log.thread_get_id = system->thread_get_id;
    global_log.arena = make_arena_system(system);
}

internal b32
log_string(String_Const_u8 str){
    b32 result = false;
    i32 thread_id = global_log.thread_get_id();
    if (global_log.disabled_thread_id != thread_id){
        global_log.mutex_acquire(global_log.mutex);
        string_list_push(&global_log.arena, &global_log.list, push_string_copy(&global_log.arena, str));
        global_log.mutex_release(global_log.mutex);
        result = true;
    }
    return(result);
}

internal void
output_file_append(Models *models, Editing_File *file, String_Const_u8 value);

internal b32
log_flush(Models *models){
    b32 result = false;
    
    global_log.mutex_acquire(global_log.mutex);
    global_log.disabled_thread_id = global_log.thread_get_id();
    
    if (global_log.list.total_size > 0){
        String_Const_u8 text = string_list_flatten(&global_log.arena, global_log.list);
        output_file_append(models, models->log_buffer, text);
        result = true;
    }
    linalloc_clear(&global_log.arena);
    block_zero_struct(&global_log.list);
    
    global_log.disabled_thread_id = 0;
    global_log.mutex_release(global_log.mutex);
    
    return(result);
}

// BOTTOM
