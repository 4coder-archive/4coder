/*
4coder_experiments.cpp - Supplies extension bindings to the defaults with experimental new features.
*/

// TOP

// TODO(allen): move all out of experimental

#include "4coder_default_include.cpp"
#include "4coder_miblo_numbers.cpp"

#define NO_BINDING
#include "4coder_default_bindings.cpp"

#include <string.h>

// NOTE(allen): An experimental mutli-pasting thing

CUSTOM_COMMAND_SIG(multi_paste){
    Scratch_Block scratch(app);
    
    i32 count = clipboard_count(app, 0);
    if (count > 0){
        View_ID view = get_active_view(app, AccessOpen);
        Managed_Scope scope = view_get_managed_scope(app, view);
        
        Rewrite_Type *rewrite = scope_attachment(app, scope, view_rewrite_loc, Rewrite_Type);
        if (*rewrite == Rewrite_Paste){
            Rewrite_Type *next_rewrite = scope_attachment(app, scope, view_next_rewrite_loc, Rewrite_Type);
            *next_rewrite = Rewrite_Paste;
            i32 *paste_index_ptr = scope_attachment(app, scope, view_paste_index_loc, i32);
            i32 paste_index = (*paste_index_ptr) + 1;
            *paste_index_ptr = paste_index;
            
            String_Const_u8 string = push_clipboard_index(app, scratch, 0, paste_index);
            
            String_Const_u8 insert_string = push_u8_stringf(scratch, "\n%.*s", string_expand(string));
            
            Buffer_ID buffer = view_get_buffer(app, view, AccessOpen);
            Range_i64 range = get_view_range(app, view);
            buffer_replace_range(app, buffer, Ii64(range.max), insert_string);
            view_set_mark(app, view, seek_pos(range.max + 1));
            view_set_cursor_and_preferred_x(app, view, seek_pos(range.max + insert_string.size));
            
            Theme_Color paste = {};
            paste.tag = Stag_Paste;
            get_theme_colors(app, &paste, 1);
            view_post_fade(app, view, 0.667f, Ii64(range.max + 1, range.max + insert_string.size), paste.color);
        }
        else{
            paste(app);
        }
    }
}

static Range_i64
multi_paste_range(Application_Links *app, View_ID view, Range_i64 range, i32 paste_count, b32 old_to_new){
    Scratch_Block scratch(app);
    
    Range_i64 finish_range = range;
    if (paste_count >= 1){
        Buffer_ID buffer = view_get_buffer(app, view, AccessOpen);
        if (buffer != 0){
            i64 total_size = 0;
            for (i32 paste_index = 0; paste_index < paste_count; ++paste_index){
                Temp_Memory temp = begin_temp(scratch);
                String_Const_u8 string = push_clipboard_index(app, scratch, 0, paste_index);
                total_size += string.size + 1;
                end_temp(temp);
            }
            total_size -= 1;
            
            if (total_size <= app->memory_size){
                i32 first = paste_count - 1;
                i32 one_past_last = -1;
                i32 step = -1;
                if (!old_to_new){
                    first = 0;
                    one_past_last = paste_count;
                    step = 1;
                }
                
                List_String_Const_u8 list = {};
                
                for (i32 paste_index = first; paste_index != one_past_last; paste_index += step){
                    if (paste_index != first){
                        string_list_push(scratch, &list, SCu8("\n", 1));
                    }
                    String_Const_u8 string = push_clipboard_index(app, scratch, 0, paste_index);
                    if (string.size > 0){
                        string_list_push(scratch, &list, string);
                    }
                }
                
                String_Const_u8 flattened = string_list_flatten(scratch, list);
                
                buffer_replace_range(app, buffer, range, flattened);
                i64 pos = range.min;
                finish_range.min = pos;
                finish_range.max = pos + total_size;
                view_set_mark(app, view, seek_pos(finish_range.min));
                view_set_cursor_and_preferred_x(app, view, seek_pos(finish_range.max));
                
                // TODO(allen): Send this to all views.
                Theme_Color paste;
                paste.tag = Stag_Paste;
                get_theme_colors(app, &paste, 1);
                view_post_fade(app, view, 0.667f, finish_range, paste.color);
            }
        }
    }
    return(finish_range);
}

static void
multi_paste_interactive_up_down(Application_Links *app, i32 paste_count, i32 clip_count){
    View_ID view = get_active_view(app, AccessOpen);
    i64 pos = view_get_cursor_pos(app, view);
    b32 old_to_new = true;
    Range_i64 range = multi_paste_range(app, view, Ii64(pos), paste_count, old_to_new);
    
    Query_Bar bar = {};
    bar.prompt = string_u8_litexpr("Up and Down to condense and expand paste stages; R to reverse order; Return to finish; Escape to abort.");
    if (start_query_bar(app, &bar, 0) == 0) return;
    
    User_Input in = {};
    for (;;){
        in = get_user_input(app, EventOnAnyKey, EventOnEsc);
        if (in.abort) break;
        
        b32 did_modify = false;
        if (in.key.keycode == key_up){
            if (paste_count > 1){
                --paste_count;
                did_modify = true;
            }
        }
        else if (in.key.keycode == key_down){
            if (paste_count < clip_count){
                ++paste_count;
                did_modify = true;
            }
        }
        else if (in.key.keycode == 'r' || in.key.keycode == 'R'){
            old_to_new = !old_to_new;
            did_modify = true;
        }
        else if (in.key.keycode == '\n'){
            break;
        }
        
        if (did_modify){
            range = multi_paste_range(app, view, range, paste_count, old_to_new);
        }
    }
    
    if (in.abort){
        Buffer_ID buffer = view_get_buffer(app, view, AccessOpen);
        buffer_replace_range(app, buffer, range, SCu8(""));
    }
}

CUSTOM_COMMAND_SIG(multi_paste_interactive){
    i32 clip_count = clipboard_count(app, 0);
    if (clip_count > 0){
        multi_paste_interactive_up_down(app, 1, clip_count);
    }
}

CUSTOM_COMMAND_SIG(multi_paste_interactive_quick){
    i32 clip_count = clipboard_count(app, 0);
    if (clip_count > 0){
        u8 string_space[256];
        Query_Bar bar = {};
        bar.prompt = string_u8_litexpr("How Many Slots To Paste: ");
        bar.string = SCu8(string_space, (umem)0);
        bar.string_capacity = sizeof(string_space);
        query_user_number(app, &bar);
        
        i32 initial_paste_count = (i32)string_to_integer(bar.string, 10);
        initial_paste_count = clamp(1, initial_paste_count, clip_count);
        end_query_bar(app, &bar, 0);
        
        multi_paste_interactive_up_down(app, initial_paste_count, clip_count);
    }
}

extern "C" i32
get_bindings(void *data, i32 size){
    Bind_Helper context_ = begin_bind_helper(data, size);
    Bind_Helper *context = &context_;
    
    set_hook(context, hook_buffer_viewer_update, default_view_adjust);
    
    set_start_hook(context, default_start);
    set_open_file_hook(context, default_file_settings);
    set_new_file_hook(context, default_new_file);
    set_save_file_hook(context, default_file_save);
    set_file_edit_range_hook(context, default_file_edit_range);
    set_file_externally_modified_hook(context, default_file_externally_modified);
    
    set_end_file_hook(context, end_file_close_jump_list);
    
    set_command_caller(context, default_command_caller);
    set_render_caller(context, default_render_caller);
    set_input_filter(context, default_suppress_mouse_filter);
    set_scroll_rule(context, smooth_scroll_rule);
    set_buffer_name_resolver(context, default_buffer_name_resolution);
    set_modify_color_table_hook(context, default_modify_color_table);
    set_get_view_buffer_region_hook(context, default_view_buffer_region);
    
    default_keys(context);
    
    // NOTE(allen|a4.0.6): Command maps can be opened more than
    // once so that you can extend existing maps very easily.
    // You can also use the helper "restart_map" instead of
    // begin_map to clear everything that was in the map and
    // bind new things instead.
    begin_map(context, mapid_file);
    //bind(context, key_back, MDFR_ALT|MDFR_CTRL, kill_rect);
    //bind(context, ' ', MDFR_ALT|MDFR_CTRL, multi_line_edit);
    
    bind(context, key_page_up, MDFR_ALT, miblo_increment_time_stamp);
    bind(context, key_page_down, MDFR_ALT, miblo_decrement_time_stamp);
    
    bind(context, key_home, MDFR_ALT, miblo_increment_time_stamp_minute);
    bind(context, key_end, MDFR_ALT, miblo_decrement_time_stamp_minute);
    
    bind(context, 'b', MDFR_CTRL, multi_paste_interactive_quick);
    bind(context, 'A', MDFR_CTRL, replace_in_all_buffers);
    end_map(context);
    
    return(end_bind_helper(context));
}

// BOTTOM

