/*
4coder_default_hooks.cpp - Sets up the hooks for the default framework.

TYPE: 'internal-for-default-system'
*/

// TOP

#if !defined(FCODER_DEFAULT_HOOKS_CPP)
#define FCODER_DEFAULT_HOOKS_CPP

#include "4coder_default_framework.h"
#include "4coder_helper/4coder_bind_helper.h"
#include "4coder_project_commands.cpp"

#include "languages/4coder_language_cpp.h"
#include "languages/4coder_language_rust.h"
#include "languages/4coder_language_cs.h"
#include "languages/4coder_language_java.h"

CUSTOM_COMMAND_SIG(set_bindings_choose);
CUSTOM_COMMAND_SIG(set_bindings_default);
CUSTOM_COMMAND_SIG(set_bindings_mac_4coder_like);
CUSTOM_COMMAND_SIG(set_bindings_mac_default);

static Named_Mapping named_maps_values[] = {
    {make_lit_string("choose")         , set_bindings_choose         },
    {make_lit_string("default")        , set_bindings_default        },
    {make_lit_string("mac-4coder-like"), set_bindings_mac_4coder_like},
    {make_lit_string("mac-default")    , set_bindings_mac_default    },
};

START_HOOK_SIG(default_start){
    named_maps = named_maps_values;
    named_map_count = ArrayCount(named_maps_values);
    
    default_4coder_initialize(app);
    default_4coder_side_by_side_panels(app, files, file_count);
    
    if (automatically_load_project){
        load_project(app);
    }
    
    // no meaning for return
    return(0);
}

// NOTE(allen|a4.0.9): All command calls can now go through this hook
// If this hook is not implemented a default behavior of calling the
// command is used.  It is important to note that paste_next does not
// work without this hook.
// NOTE(allen|a4.0.10): As of this version the word_complete command
// also relies on this particular command caller hook.
COMMAND_CALLER_HOOK(default_command_caller){
    View_Summary view = get_active_view(app, AccessAll);
    
#if 0
    // NOTE(allen): Debugging input.
#include <stdio.h>
    User_Input in = get_command_input(app);
    if (in.type == UserInputKey){
        Key_Event_Data key = in.key;
        fprintf(stdout, "keycode %u/'%c'\n", key.keycode, (char)key.keycode);
        fprintf(stdout, "character %u/'%c'\n", key.character, (char)key.character);
        fprintf(stdout, "character_no_caps_lock %u/'%c'\n", key.character_no_caps_lock, (char)key.character_no_caps_lock);
        fprintf(stdout, "ctrl: %s ", key.modifiers[MDFR_CONTROL_INDEX]?" true":"false");
        fprintf(stdout, "alt: %s " , key.modifiers[MDFR_ALT_INDEX]    ?" true":"false");
        fprintf(stdout, "cmnd: %s ", key.modifiers[MDFR_COMMAND_INDEX]?" true":"false");
        fprintf(stdout, "shift: %s", key.modifiers[MDFR_SHIFT_INDEX]  ?" true":"false");
        fprintf(stdout, "\n");
    }
#endif
    
    view_paste_index[view.view_id].next_rewrite = 0;
    exec_command(app, cmd);
    view_paste_index[view.view_id].rewrite = view_paste_index[view.view_id].next_rewrite;
    
    return(0);
}

HOOK_SIG(default_exit){
    // if this returns zero it cancels the exit.
    return(1);
}

HOOK_SIG(default_view_adjust){
    int32_t count = 0;
    int32_t new_wrap_width = 0;
    for (View_Summary view = get_view_first(app, AccessAll);
         view.exists;
         get_view_next(app, &view, AccessAll)){
        new_wrap_width += view.view_region.x1 - view.view_region.x0;
        ++count;
    }
    
    new_wrap_width /= count;
    new_wrap_width = (int32_t)(new_wrap_width * .9f);
    
    int32_t new_min_base_width = (int32_t)(new_wrap_width * .77f);
    if (automatically_adjust_wrapping){
        adjust_all_buffer_wrap_widths(app, new_wrap_width, new_min_base_width);
        default_wrap_width = new_wrap_width;
        default_min_base_width = new_min_base_width;
    }
    
    // no meaning for return
    return(0);
}

// TODO(allen): Eliminate this hook if you can.
OPEN_FILE_HOOK_SIG(default_file_settings){
    // NOTE(allen|a4.0.8): The get_parameter_buffer was eliminated
    // and instead the buffer is passed as an explicit parameter through
    // the function call.  That is where buffer_id comes from here.
    Buffer_Summary buffer = get_buffer(app, buffer_id, AccessAll);
    Assert(buffer.exists);
    
    bool32 treat_as_code = false;
    bool32 treat_as_todo = false;
    bool32 wrap_lines = true;
    
    int32_t extension_count = 0;
    char **extension_list = get_current_code_extensions(&extension_count);
    
    Parse_Context_ID parse_context_id = 0;
    
    if (buffer.file_name != 0 && buffer.size < (16 << 20)){
        String name = make_string(buffer.file_name, buffer.file_name_len);
        String ext = file_extension(name);
        for (int32_t i = 0; i < extension_count; ++i){
            if (match(ext, extension_list[i])){
                treat_as_code = true;
                
                if (match(ext, "cs")){
                    if (parse_context_language_cs == 0){
                        init_language_cs(app);
                    }
                    parse_context_id = parse_context_language_cs;
                }
                
                if (match(ext, "java")){
                    if (parse_context_language_java == 0){
                        init_language_java(app);
                    }
                    parse_context_id = parse_context_language_java;
                }
                
                if (match(ext, "rs")){
                    if (parse_context_language_rust == 0){
                        init_language_rust(app);
                    }
                    parse_context_id = parse_context_language_rust;
                }
                
                if (match(ext, "cpp") || match(ext, "h") || match(ext, "c") || match(ext, "hpp") || match(ext, "cc")){
                    if (parse_context_language_cpp == 0){
                        init_language_cpp(app);
                    }
                    parse_context_id = parse_context_language_cpp;
                }
                
                // TODO(NAME): Real GLSL highlighting
                if (match(ext, "glsl")){
                    if (parse_context_language_cpp == 0){
                        init_language_cpp(app);
                    }
                    parse_context_id = parse_context_language_cpp;
                }
                
                // TODO(NAME): Real Objective-C highlighting
                if (match(ext, "m")){
                    if (parse_context_language_cpp == 0){
                        init_language_cpp(app);
                    }
                    parse_context_id = parse_context_language_cpp;
                }
                
                break;
            }
        }
        
        if (!treat_as_code){
            String lead_name = front_of_directory(name);
            if (match_insensitive(lead_name, "todo.txt")){
                treat_as_todo = true;
            }
        }
    }
    
    if (treat_as_code){
        wrap_lines = false;
    }
    if (buffer.file_name == 0){
        wrap_lines = false;
    }
    
    int32_t map_id = (treat_as_code)?((int32_t)default_code_map):((int32_t)mapid_file);
    
    buffer_set_setting(app, &buffer, BufferSetting_WrapPosition, default_wrap_width);
    buffer_set_setting(app, &buffer, BufferSetting_MinimumBaseWrapPosition, default_min_base_width);
    buffer_set_setting(app, &buffer, BufferSetting_MapID, map_id);
    buffer_set_setting(app, &buffer, BufferSetting_ParserContext, parse_context_id);
    
    if (treat_as_todo){
        buffer_set_setting(app, &buffer, BufferSetting_WrapLine, true);
        buffer_set_setting(app, &buffer, BufferSetting_LexWithoutStrings, true);
        buffer_set_setting(app, &buffer, BufferSetting_VirtualWhitespace, true);
    }
    else if (treat_as_code && enable_code_wrapping && buffer.size < (1 << 18)){
        // NOTE(allen|a4.0.12): There is a little bit of grossness going on here.
        // If we set BufferSetting_Lex to true, it will launch a lexing job.
        // If a lexing job is active when we set BufferSetting_VirtualWhitespace, the call can fail.
        // Unfortunantely without tokens virtual whitespace doesn't really make sense.
        // So for now I have it automatically turning on lexing when virtual whitespace is turned on.
        // Cleaning some of that up is a goal for future versions.
        buffer_set_setting(app, &buffer, BufferSetting_WrapLine, true);
        buffer_set_setting(app, &buffer, BufferSetting_VirtualWhitespace, true);
    }
    else{
        buffer_set_setting(app, &buffer, BufferSetting_WrapLine, wrap_lines);
        buffer_set_setting(app, &buffer, BufferSetting_Lex, treat_as_code);
    }
    
    // no meaning for return
    return(0);
}

OPEN_FILE_HOOK_SIG(default_new_file){
    Buffer_Summary buffer = get_buffer(app, buffer_id, AccessOpen);
    char str[] = "/*\nNew File\n*/\n\n\n";
    buffer_replace_range(app, &buffer, 0, 0, str, sizeof(str)-1);
    
    // no meaning for return
    return(0);
}

OPEN_FILE_HOOK_SIG(default_file_save){
    Buffer_Summary buffer = get_buffer(app, buffer_id, AccessAll);
    Assert(buffer.exists);
    
#if defined(FCODER_AUTO_INDENT_CPP)
    int32_t is_virtual = 0;
    if (automatically_indent_text_on_save && buffer_get_setting(app, &buffer, BufferSetting_VirtualWhitespace, &is_virtual)){ 
        if (is_virtual){
            auto_tab_whole_file_by_summary(app, &buffer);
        }
    }
#endif
    
    // no meaning for return
    return(0);
}

OPEN_FILE_HOOK_SIG(default_end_file){
    Buffer_Summary buffer = get_buffer(app, buffer_id, AccessAll);
    Assert(buffer.exists);
    
    char space[1024];
    String str = make_fixed_width_string(space);
    append(&str, "Ending file: ");
    append(&str, make_string(buffer.buffer_name, buffer.buffer_name_len));
    append(&str, "\n");
    
    print_message(app, str.str, str.size);
    
    // no meaning for return
    return(0);
}

// NOTE(allen|a4.0.9): The input filter allows you to modify the input
// to a frame before 4coder starts processing it at all.
//
// Right now it only has access to the mouse state, but it will be
// extended to have access to the key presses soon.
INPUT_FILTER_SIG(default_suppress_mouse_filter){
    if (suppressing_mouse){
        *mouse = null_mouse_state;
        mouse->x = -100;
        mouse->y = -100;
    }
}

// NOTE(allen|a4): scroll rule information
//
// The parameters:
// target_x, target_y
//  This is where the view would like to be for the purpose of
// following the cursor, doing mouse wheel work, etc.
//
// scroll_x, scroll_y
//  These are pointers to where the scrolling actually is. If you bind
// the scroll rule it is you have to update these in some way to move
// the actual location of the scrolling.
//
// view_id
//  This corresponds to which view is computing it's new scrolling position.
// This id DOES correspond to the views that View_Summary contains.
// This will always be between 1 and 16 (0 is a null id).
// See below for an example of having state that carries across scroll udpates.
//
// is_new_target
//  If the target of the view is different from the last target in either x or y
// this is true, otherwise it is false.
//
// The return:
//  Should be true if and only if scroll_x or scroll_y are changed.
//
// Don't try to use the app pointer in a scroll rule, you're asking for trouble.
//
// If you don't bind scroll_rule, nothing bad will happen, yo will get default
// 4coder scrolling behavior.
//

struct Scroll_Velocity{
    float x, y;
};

Scroll_Velocity scroll_velocity_[16] = {0};
Scroll_Velocity *scroll_velocity = scroll_velocity_ - 1;

static int32_t
smooth_camera_step(float target, float *current, float *vel, float S, float T){
    int32_t result = 0;
    float curr = *current;
    float v = *vel;
    if (curr != target){
        if (curr > target - .1f && curr < target + .1f){
            curr = target;
            v = 1.f;
        }
        else{
            float L = curr + T*(target - curr);
            
            int32_t sign = (target > curr) - (target < curr);
            float V = curr + sign*v;
            
            if (sign > 0) curr = (L<V)?(L):(V);
            else curr = (L>V)?(L):(V);
            
            if (curr == V){
                v *= S;
            }
        }
        
        *current = curr;
        *vel = v;
        result = 1;
    }
    return(result);
}

SCROLL_RULE_SIG(smooth_scroll_rule){
    Scroll_Velocity *velocity = scroll_velocity + view_id;
    int32_t result = 0;
    if (velocity->x == 0.f){
        velocity->x = 1.f;
        velocity->y = 1.f;
    }
    
    if (smooth_camera_step(target_y, scroll_y, &velocity->y, 80.f, 1.f/2.f)){
        result = 1;
    }
    if (smooth_camera_step(target_x, scroll_x, &velocity->x, 80.f, 1.f/2.f)){
        result = 1;
    }
    
    return(result);
}

static void
set_all_default_hooks(Bind_Helper *context){
    set_hook(context, hook_exit, default_exit);
    set_hook(context, hook_view_size_change, default_view_adjust);
    
    set_start_hook(context, default_start);
    set_open_file_hook(context, default_file_settings);
    set_new_file_hook(context, default_new_file);
    set_save_file_hook(context, default_file_save);
    
#if defined(FCODER_STICKY_JUMP)
    set_end_file_hook(context, end_file_close_jump_list);
#else
    set_end_file_hook(context, default_end_file);
#endif
    
    set_command_caller(context, default_command_caller);
    set_input_filter(context, default_suppress_mouse_filter);
    set_scroll_rule(context, smooth_scroll_rule);
}

#endif

// BOTTOM

