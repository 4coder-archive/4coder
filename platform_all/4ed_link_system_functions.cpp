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
link_system_code(void){
    SYSLINK(get_canonical);
    SYSLINK(get_file_list);
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
    SYSLINK(signal_step);
    SYSLINK(sleep);
    
    SYSLINK(post_clipboard);
    
    SYSLINK(cli_call);
    SYSLINK(cli_begin_update);
    SYSLINK(cli_update_step);
    SYSLINK(cli_end_update);
    
    SYSLINK(open_color_picker);
    
    SYSLINK(thread_launch);
    SYSLINK(thread_join);
    SYSLINK(thread_free);
    SYSLINK(thread_get_id);
    SYSLINK(mutex_make);
    SYSLINK(mutex_acquire);
    SYSLINK(mutex_release);
    SYSLINK(mutex_free);
    SYSLINK(condition_variable_make);
    SYSLINK(condition_variable_wait);
    SYSLINK(condition_variable_signal);
    SYSLINK(condition_variable_free);
    
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

