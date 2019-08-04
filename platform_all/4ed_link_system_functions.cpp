/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 18.07.2017
 *
 * Code to link system functions using a name convention
 *
 */

// TOP

// TODO(allen): Should auto-gen this!

#define SYSLINK(name) sysfunc.name = system_##name

internal void
link_system_code(){
    SYSLINK(get_canonical);
    SYSLINK(get_file_list);
    SYSLINK(add_listener);
    SYSLINK(remove_listener);
    SYSLINK(get_file_change);
    SYSLINK(quick_file_attributes);
    SYSLINK(load_handle);
    SYSLINK(load_attributes);
    SYSLINK(load_file);
    SYSLINK(load_close);
    SYSLINK(save_file);
    
    SYSLINK(now_time);
    SYSLINK(wake_up_timer_create);
    SYSLINK(wake_up_timer_release);
    SYSLINK(wake_up_timer_set);
    
    SYSLINK(post_clipboard);
    
    SYSLINK(create_coroutine);
    SYSLINK(launch_coroutine);
    SYSLINK(resume_coroutine);
    SYSLINK(yield_coroutine);
    
    SYSLINK(cli_call);
    SYSLINK(cli_begin_update);
    SYSLINK(cli_update_step);
    SYSLINK(cli_end_update);
    
    SYSLINK(open_color_picker);
    
    SYSLINK(post_job);
    SYSLINK(cancel_job);
    SYSLINK(check_cancel);
    SYSLINK(grow_thread_memory);
    SYSLINK(acquire_lock);
    SYSLINK(release_lock);
    
    SYSLINK(memory_allocate);
    SYSLINK(memory_set_protection);
    SYSLINK(memory_free);
    
    SYSLINK(get_current_path);
    SYSLINK(get_4ed_path);
    
    SYSLINK(set_fullscreen);
    SYSLINK(is_fullscreen);
    SYSLINK(show_mouse_cursor);
}

// BOTTOM

