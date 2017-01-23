
#ifndef FCODER_DEFAULT_INCLUDE
#define FCODER_DEFAULT_INCLUDE

#include "4coder_API/custom.h"


#include "4coder_jump_parsing.cpp"

#include "4coder_default_framework.h"
#include "4coder_base_commands.cpp"
#include "4coder_auto_indent.cpp"
#include "4coder_search.cpp"

#include "4coder_helper/4coder_bind_helper.h"
#include "4coder_helper/4coder_helper.h"
#include "4coder_helper/4coder_streaming.h"
#include "4coder_helper/4coder_long_seek.h"

#define FSTRING_IMPLEMENTATION
#include "4coder_lib/4coder_string.h"
#include "4coder_lib/4coder_table.h"
#include "4coder_lib/4coder_mem.h"

#include "4cpp/4cpp_lexer.h"

#include <assert.h>

//
// Seeks Using Default Framework Memory
//

static int32_t
buffer_boundary_seek(Application_Links *app, Buffer_Summary *buffer, int32_t start_pos, bool32 seek_forward, Seek_Boundary_Flag flags){
    int32_t result = buffer_boundary_seek(app, buffer, &global_part, start_pos, seek_forward, flags);
    return(result);
}

static void
basic_seek(Application_Links *app, int32_t seek_type, uint32_t flags){
    uint32_t access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    int32_t pos = buffer_boundary_seek(app, &buffer, view.cursor.pos, seek_type, flags);
    view_set_cursor(app, &view, seek_pos(pos), true);
}

#define seek_command(n, dir, flags) CUSTOM_COMMAND_SIG(seek_##n##_##dir){ basic_seek(app, dir, flags); }

#define right true
#define left false

seek_command(whitespace,            right, BoundaryWhitespace)
seek_command(whitespace,            left,  BoundaryWhitespace)
seek_command(token,                 right, BoundaryToken)
seek_command(token,                 left,  BoundaryToken)
seek_command(white_or_token,        right, BoundaryToken | BoundaryWhitespace)
seek_command(white_or_token,        left,  BoundaryToken | BoundaryWhitespace)
seek_command(alphanumeric,          right, BoundaryAlphanumeric)
seek_command(alphanumeric,          left,  BoundaryAlphanumeric)
seek_command(alphanumeric_or_camel, right, BoundaryAlphanumeric | BoundaryCamelCase)
seek_command(alphanumeric_or_camel, left,  BoundaryAlphanumeric | BoundaryCamelCase)

#undef right
#undef left

//
// Clipboard
//

static bool32
clipboard_copy(Application_Links *app, int32_t start, int32_t end, Buffer_Summary *buffer_out,
               uint32_t access){
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    bool32 result = 0;
    
    if (buffer.exists){
        if (0 <= start && start <= end && end <= buffer.size){
            int32_t size = (end - start);
            char *str = (char*)app->memory;
            
            if (size <= app->memory_size){
                buffer_read_range(app, &buffer, start, end, str);
                clipboard_post(app, 0, str, size);
                if (buffer_out){*buffer_out = buffer;}
                result = 1;
            }
        }
    }
    
    return(result);
}

static int32_t
clipboard_cut(Application_Links *app, int32_t start, int32_t end, Buffer_Summary *buffer_out, uint32_t access){
    Buffer_Summary buffer = {0};
    int32_t result = false;
    
    if (clipboard_copy(app, start, end, &buffer, access)){
        buffer_replace_range(app, &buffer, start, end, 0, 0);
        if (buffer_out){*buffer_out = buffer;}
    }
    
    return(result);
}

CUSTOM_COMMAND_SIG(copy){
    uint32_t access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    Range range = get_range(&view);
    clipboard_copy(app, range.min, range.max, 0, access);
}

CUSTOM_COMMAND_SIG(cut){
    uint32_t access = AccessOpen;
    View_Summary view = get_active_view(app, access);
    Range range = get_range(&view);
    clipboard_cut(app, range.min, range.max, 0, access);
}

CUSTOM_COMMAND_SIG(paste){
    uint32_t access = AccessOpen;
    int32_t count = clipboard_count(app, 0);
    if (count > 0){
        View_Summary view = get_active_view(app, access);
        
        view_paste_index[view.view_id].next_rewrite = RewritePaste;
        
        int32_t paste_index = 0;
        view_paste_index[view.view_id].index = paste_index;
        
        int32_t len = clipboard_index(app, 0, paste_index, 0, 0);
        char *str = 0;
        
        if (len <= app->memory_size){
            str = (char*)app->memory;
        }
        
        if (str){
            clipboard_index(app, 0, paste_index, str, len);
            
            Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
            int32_t pos = view.cursor.pos;
            buffer_replace_range(app, &buffer, pos, pos, str, len);
            view_set_mark(app, &view, seek_pos(pos));
            view_set_cursor(app, &view, seek_pos(pos + len), true);
            
            // TODO(allen): Send this to all views.
            Theme_Color paste;
            paste.tag = Stag_Paste;
            get_theme_colors(app, &paste, 1);
            view_post_fade(app, &view, 0.667f, pos, pos + len, paste.color);
        }
    }
}

CUSTOM_COMMAND_SIG(paste_next){
    uint32_t access = AccessOpen;
    int32_t count = clipboard_count(app, 0);
    if (count > 0){
        View_Summary view = get_active_view(app, access);
        
        if (view_paste_index[view.view_id].rewrite == RewritePaste){
            view_paste_index[view.view_id].next_rewrite = RewritePaste;
            
            int32_t paste_index = view_paste_index[view.view_id].index + 1;
            view_paste_index[view.view_id].index = paste_index;
            
            int32_t len = clipboard_index(app, 0, paste_index, 0, 0);
            char *str = 0;
            
            if (len <= app->memory_size){
                str = (char*)app->memory;
            }
            
            if (str){
                clipboard_index(app, 0, paste_index, str, len);
                
                Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
                Range range = get_range(&view);
                int32_t pos = range.min;
                
                buffer_replace_range(app, &buffer, range.min, range.max, str, len);
                view_set_cursor(app, &view, seek_pos(pos + len), true);
                
                // TODO(allen): Send this to all views.
                Theme_Color paste;
                paste.tag = Stag_Paste;
                get_theme_colors(app, &paste, 1);
                view_post_fade(app, &view, 0.667f, pos, pos + len, paste.color);
            }
        }
        else{
            exec_command(app, paste);
        }
    }
}

CUSTOM_COMMAND_SIG(paste_and_indent){
    exec_command(app, paste);
    exec_command(app, auto_tab_range);
}

CUSTOM_COMMAND_SIG(paste_next_and_indent){
    exec_command(app, paste_next);
    exec_command(app, auto_tab_range);
}

//////////////////////////

static void
write_string(Application_Links *app, View_Summary *view, Buffer_Summary *buffer, String string){
    buffer_replace_range(app, buffer, view->cursor.pos, view->cursor.pos, string.str, string.size);
    view_set_cursor(app, view, seek_pos(view->cursor.pos + string.size), 1);
}

static void
write_string(Application_Links *app, String string){
    uint32_t access = AccessOpen;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    write_string(app, &view, &buffer, string);
}

static void
long_braces(Application_Links *app, char *text, int32_t size){
    uint32_t access = AccessOpen;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    int32_t pos = view.cursor.pos;
    
    buffer_replace_range(app, &buffer, pos, pos, text, size);
    view_set_cursor(app, &view, seek_pos(pos + 2), true);
    
    buffer_auto_indent(app, &buffer, pos, pos + size, DEF_TAB_WIDTH, DEFAULT_INDENT_FLAGS | AutoIndent_FullTokens);
    move_past_lead_whitespace(app, &view, &buffer);
}

CUSTOM_COMMAND_SIG(open_long_braces){
    char text[] = "{\n\n}";
    int32_t size = sizeof(text) - 1;
    long_braces(app, text, size);
}

CUSTOM_COMMAND_SIG(open_long_braces_semicolon){
    char text[] = "{\n\n};";
    int32_t size = sizeof(text) - 1;
    long_braces(app, text, size);
}

CUSTOM_COMMAND_SIG(open_long_braces_break){
    char text[] = "{\n\n}break;";
    int32_t size = sizeof(text) - 1;
    long_braces(app, text, size);
}

CUSTOM_COMMAND_SIG(if0_off){
    char text1[] = "\n#if 0";
    int32_t size1 = sizeof(text1) - 1;
    
    char text2[] = "#endif\n";
    int32_t size2 = sizeof(text2) - 1;
    
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    
    Range range = get_range(&view);
    
    if (range.min < range.max){
        Buffer_Edit edits[2];
        char *str = 0;
        char *base = (char*)partition_current(&global_part);
        
        str = push_array(&global_part, char, size1);
        memcpy(str, text1, size1);
        edits[0].str_start = (int32_t)(str - base);
        edits[0].len = size1;
        edits[0].start = range.min;
        edits[0].end = range.min;
        
        str = push_array(&global_part, char, size2);
        memcpy(str, text2, size2);
        edits[1].str_start = (int32_t)(str - base);
        edits[1].len = size2;
        edits[1].start = range.max;
        edits[1].end = range.max;
        
        buffer_batch_edit(app, &buffer, base, global_part.pos, edits, ArrayCount(edits), BatchEdit_Normal);
        
        view = get_view(app, view.view_id, AccessAll);
        if (view.cursor.pos > view.mark.pos){
            view_set_cursor(app, &view, seek_line_char(view.cursor.line+1, view.cursor.character), 1);
        }
        else{
            view_set_mark(app, &view, seek_line_char(view.mark.line+1, view.mark.character));
        }
        
        range = get_range(&view);
        buffer_auto_indent(app, &buffer, range.min, range.max, DEF_TAB_WIDTH, DEFAULT_INDENT_FLAGS | AutoIndent_FullTokens);
        move_past_lead_whitespace(app, &view, &buffer);
    }
}


//
// Fast Deletes 
//

CUSTOM_COMMAND_SIG(backspace_word){
    uint32_t access = AccessOpen;
    
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    if (buffer.exists){
        int32_t pos2 = 0, pos1 = 0;
        
        pos2 = view.cursor.pos;
        exec_command(app, seek_alphanumeric_left);
        refresh_view(app, &view);
        pos1 = view.cursor.pos;
        
        buffer_replace_range(app, &buffer, pos1, pos2, 0, 0);
    }
}

CUSTOM_COMMAND_SIG(delete_word){
    uint32_t access = AccessOpen;
    
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    if (buffer.exists){
        int32_t pos2 = 0, pos1 = 0;
        
        pos1 = view.cursor.pos;
        exec_command(app, seek_alphanumeric_right);
        refresh_view(app, &view);
        pos2 = view.cursor.pos;
        
        buffer_replace_range(app, &buffer, pos1, pos2, 0, 0);
    }
}

CUSTOM_COMMAND_SIG(snipe_token_or_word){
    uint32_t access = AccessOpen;
    
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    int32_t pos1 = buffer_boundary_seek(app, &buffer, view.cursor.pos, 0, BoundaryToken | BoundaryWhitespace);
    int32_t pos2 = buffer_boundary_seek(app, &buffer, pos1,            1, BoundaryToken | BoundaryWhitespace);
    
    Range range = make_range(pos1, pos2);
    buffer_replace_range(app, &buffer, range.start, range.end, 0, 0);
}



//
// Open File In Quotes
//

static int32_t
file_name_in_quotes(Application_Links *app, String *file_name){
    int32_t result = false;
    uint32_t access = AccessProtected;
    
    View_Summary view;
    Buffer_Summary buffer;
    char short_file_name[128];
    int32_t pos, start, end, size;
    
    view = get_active_view(app, access);
    buffer = get_buffer(app, view.buffer_id, access);
    pos = view.cursor.pos;
    buffer_seek_delimiter_forward(app, &buffer, pos, '"', &end);
    buffer_seek_delimiter_backward(app, &buffer, pos, '"', &start);
    
    ++start;
    size = end - start;
    
    // NOTE(allen): This check is necessary because buffer_read_range
    // requiers that the output buffer you provide is at least (end - start) bytes long.
    if (size < sizeof(short_file_name)){
        if (buffer_read_range(app, &buffer, start, end, short_file_name)){
            result = true;
            copy_ss(file_name, make_string(buffer.file_name, buffer.file_name_len));
            remove_last_folder(file_name);
            append_ss(file_name, make_string(short_file_name, size));
        }
    }
    
    return(result);
}

CUSTOM_COMMAND_SIG(open_file_in_quotes){
    char file_name_[256];
    String file_name = make_fixed_width_string(file_name_);
    
    if (file_name_in_quotes(app, &file_name)){
        exec_command(app, change_active_panel);
        View_Summary view = get_active_view(app, AccessAll);
        view_open_file(app, &view, expand_str(file_name), true);
    }
}

CUSTOM_COMMAND_SIG(open_in_other){
    exec_command(app, change_active_panel);
    exec_command(app, interactive_open);
}


//
// System Commands
//

static char out_buffer_space[1024];
static char command_space[1024];
static char hot_directory_space[1024];

CUSTOM_COMMAND_SIG(execute_any_cli){
    Query_Bar bar_out = {0};
    Query_Bar bar_cmd = {0};
    
    bar_out.prompt = make_lit_string("Output Buffer: ");
    bar_out.string = make_fixed_width_string(out_buffer_space);
    if (!query_user_string(app, &bar_out)) return;
    
    bar_cmd.prompt = make_lit_string("Command: ");
    bar_cmd.string = make_fixed_width_string(command_space);
    if (!query_user_string(app, &bar_cmd)) return;
    
    String hot_directory = make_fixed_width_string(hot_directory_space);
    hot_directory.size = directory_get_hot(app, hot_directory.str, hot_directory.memory_size);
    
    uint32_t access = AccessAll;
    View_Summary view = get_active_view(app, access);
    
    exec_system_command(app, &view, buffer_identifier(bar_out.string.str, bar_out.string.size), hot_directory.str, hot_directory.size, bar_cmd.string.str, bar_cmd.string.size, CLI_OverlapWithConflict | CLI_CursorAtEnd);
}

CUSTOM_COMMAND_SIG(execute_previous_cli){
    String out_buffer = make_string_slowly(out_buffer_space);
    String cmd = make_string_slowly(command_space);
    String hot_directory = make_string_slowly(hot_directory_space);
    
    if (out_buffer.size > 0 && cmd.size > 0 && hot_directory.size > 0){
        uint32_t access = AccessAll;
        View_Summary view = get_active_view(app, access);
        
        exec_system_command(app, &view, buffer_identifier(out_buffer.str, out_buffer.size), hot_directory.str, hot_directory.size, cmd.str, cmd.size, CLI_OverlapWithConflict | CLI_CursorAtEnd);
    }
}


//
// Default Building Stuff
//

// NOTE(allen|a4.0.9): This is provided to establish a default method of getting
// a "build directory".  This function tries to setup the build directory in the
// directory of the given buffer, if it cannot get that information it get's the
// 4coder hot directory.
//
//  There is no requirement that a custom build system in 4coder actually use the
// directory given by this function.
enum Get_Build_Directory_Result{
    BuildDir_None,
    BuildDir_AtFile,
    BuildDir_AtHot
};

static int32_t
get_build_directory(Application_Links *app, Buffer_Summary *buffer, String *dir_out){
    int32_t result = BuildDir_None;
    
    if (buffer && buffer->file_name){
        if (!match_cc(buffer->file_name, buffer->buffer_name)){
            String dir = make_string_cap(buffer->file_name,
                                         buffer->file_name_len,
                                         buffer->file_name_len+1);
            remove_last_folder(&dir);
            append_ss(dir_out, dir);
            result = BuildDir_AtFile;
        }
    }
    
    if (!result){
        int32_t len = directory_get_hot(app, dir_out->str,
                                        dir_out->memory_size - dir_out->size);
        if (len + dir_out->size < dir_out->memory_size){
            dir_out->size += len;
            result = BuildDir_AtHot;
        }
    }
    
    return(result);
}

// TODO(allen): Better names for the "standard build search" family.
static int32_t
standard_build_search(Application_Links *app, View_Summary *view, Buffer_Summary *active_buffer, String *dir, String *command, int32_t perform_backup, int32_t use_path_in_command, String filename, String commandname){
    int32_t result = false;
    
    for(;;){
        int32_t old_size = dir->size;
        append_ss(dir, filename);
        
        if (file_exists(app, dir->str, dir->size)){
            dir->size = old_size;
            
            if (use_path_in_command){
                append_s_char(command, '"');
                append_ss(command, *dir);
                append_ss(command, commandname);
                append_s_char(command, '"');
            }
            else{
                append_ss(command, commandname);
            }
            
            char space[512];
            String message = make_fixed_width_string(space);
            append_ss(&message, make_lit_string("Building with: "));
            append_ss(&message, *command);
            append_s_char(&message, '\n');
            print_message(app, message.str, message.size);
            
            exec_system_command(app, view, buffer_identifier(literal("*compilation*")), dir->str, dir->size, command->str, command->size, CLI_OverlapWithConflict);
            result = true;
            break;
        }
        dir->size = old_size;
        
        if (directory_cd(app, dir->str, &dir->size, dir->memory_size, literal("..")) == 0){
            if (perform_backup){
                dir->size = directory_get_hot(app, dir->str, dir->memory_size);
                char backup_space[256];
                String backup_command = make_fixed_width_string(backup_space);
                append_ss(&backup_command, make_lit_string("echo could not find "));
                append_ss(&backup_command, filename);
                exec_system_command(app, view, buffer_identifier(literal("*compilation*")), dir->str, dir->size, backup_command.str, backup_command.size, CLI_OverlapWithConflict);
            }
            break;
        }
    }
    
    return(result);
}

#if defined(_WIN32)

// NOTE(allen): Build search rule for windows.
static int32_t
execute_standard_build_search(Application_Links *app, View_Summary *view,
                              Buffer_Summary *active_buffer,
                              String *dir, String *command, int32_t perform_backup){
    int32_t result = standard_build_search(app, view, active_buffer, dir, command, perform_backup, true, make_lit_string("build.bat"), make_lit_string("build"));
    return(result);
}

#elif defined(__linux__)

// NOTE(allen): Build search rule for linux.
static int32_t
execute_standard_build_search(Application_Links *app, View_Summary *view, Buffer_Summary *active_buffer, String *dir, String *command, bool32 perform_backup){
    char dir_space[512];
    String dir_copy = make_fixed_width_string(dir_space);
    copy(&dir_copy, *dir);
    
    int32_t result = standard_build_search(app, view, active_buffer, dir, command, 0, 1, make_lit_string("build.sh"), make_lit_string("build.sh"));
    
    if (!result){
        result = standard_build_search(app, view, active_buffer, &dir_copy, command, perform_backup, 0, make_lit_string("Makefile"), make_lit_string("make"));
    }
    
    return(result);
}

#else
# error No build search rule for this platform.
#endif


// NOTE(allen): This searches first using the active file's directory,
// then if no build script is found, it searches from 4coders hot directory.
static void
execute_standard_build(Application_Links *app, View_Summary *view, Buffer_Summary *active_buffer){
    char dir_space[512];
    String dir = make_fixed_width_string(dir_space);
    
    char command_str_space[512];
    String command = make_fixed_width_string(command_str_space);
    
    int32_t build_dir_type = get_build_directory(app, active_buffer, &dir);
    
    if (build_dir_type == BuildDir_AtFile){
        if (!execute_standard_build_search(app, view, active_buffer, &dir, &command, false)){
            dir.size = 0;
            command.size = 0;
            build_dir_type = get_build_directory(app, 0, &dir);
        }
    }
    
    if (build_dir_type == BuildDir_AtHot){
        execute_standard_build_search(app, view, active_buffer, &dir, &command, true);
    }
}

CUSTOM_COMMAND_SIG(build_search){
    uint32_t access = AccessAll;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    execute_standard_build(app, &view, &buffer);
    prev_location = null_location;
    lock_jump_buffer(literal("*compilation*"));
}

#define GET_COMP_BUFFER(app) get_buffer_by_name(app, literal("*compilation*"), AccessAll)

static View_Summary
get_or_open_build_panel(Application_Links *app){
    View_Summary view = {0};
    
    Buffer_Summary buffer = GET_COMP_BUFFER(app);
    if (buffer.exists){
        view = get_first_view_with_buffer(app, buffer.buffer_id);
    }
    if (!view.exists){
        view = open_special_note_view(app);
    }
    
    return(view);
}

static void
set_fancy_compilation_buffer_font(Application_Links *app){
    Buffer_Summary comp_buffer = get_buffer_by_name(app, literal("*compilation*"), AccessAll);
    buffer_set_font(app, &comp_buffer, literal("Inconsolata"));
}

CUSTOM_COMMAND_SIG(build_in_build_panel){
    uint32_t access = AccessAll;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    View_Summary build_view = get_or_open_build_panel(app);
    
    execute_standard_build(app, &build_view, &buffer);
    set_fancy_compilation_buffer_font(app);
    
    prev_location = null_location;
    lock_jump_buffer(literal("*compilation*"));
}

CUSTOM_COMMAND_SIG(close_build_panel){
    close_special_note_view(app);
}

CUSTOM_COMMAND_SIG(change_to_build_panel){
    View_Summary view = open_special_note_view(app, false);
    
    if (!view.exists){
        Buffer_Summary buffer = GET_COMP_BUFFER(app);
        if (buffer.exists){
            view = open_special_note_view(app);
            view_set_buffer(app, &view, buffer.buffer_id, 0);
        }
    }
    
    if (view.exists){
        set_active_view(app, &view);
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

// NOTE(allen|a4.0.9): All command calls can now go through this hook
// If this hook is not implemented a default behavior of calling the
// command is used.  It is important to note that paste_next does not
// work without this hook.
// NOTE(allen|a4.0.10): As of this version the word_complete command
// also relies on this particular command caller hook.
COMMAND_CALLER_HOOK(default_command_caller){
    View_Summary view = get_active_view(app, AccessAll);
    
    view_paste_index[view.view_id].next_rewrite = 0;
    exec_command(app, cmd);
    view_paste_index[view.view_id].rewrite = view_paste_index[view.view_id].next_rewrite;
    
    return(0);
}

struct Config_Line{
    Cpp_Token id_token;
    Cpp_Token subscript_token;
    Cpp_Token eq_token;
    Cpp_Token val_token;
    int32_t val_array_start;
    int32_t val_array_end;
    int32_t val_array_count;
    bool32 read_success;
};

struct Config_Item{
    Config_Line line;
    Cpp_Token_Array array;
    char *mem;
    String id;
    int32_t subscript_index;
    bool32 has_subscript;
};

struct Config_Array_Reader{
    Cpp_Token_Array array;
    char *mem;
    int32_t i;
    int32_t val_array_end;
    bool32 good;
};

static Cpp_Token
read_config_token(Cpp_Token_Array array, int32_t *i_ptr){
    Cpp_Token token = {0};
    
    int32_t i = *i_ptr;
    
    for (; i < array.count; ++i){
        Cpp_Token comment_token = array.tokens[i];
        if (comment_token.type != CPP_TOKEN_COMMENT){
            break;
        }
    }
    
    if (i < array.count){
        token = array.tokens[i];
    }
    
    *i_ptr = i;
    
    return(token);
}

static Config_Line
read_config_line(Cpp_Token_Array array, int32_t *i_ptr){
    Config_Line config_line = {0};
    
    int32_t i = *i_ptr;
    
    config_line.id_token = read_config_token(array, &i);
    if (config_line.id_token.type == CPP_TOKEN_IDENTIFIER){
        ++i;
        if (i < array.count){
            Cpp_Token token = read_config_token(array, &i);
            
            bool32 subscript_success = 1;
            if (token.type == CPP_TOKEN_BRACKET_OPEN){
                subscript_success = 0;
                ++i;
                if (i < array.count){
                    config_line.subscript_token = read_config_token(array, &i);
                    if (config_line.subscript_token.type == CPP_TOKEN_INTEGER_CONSTANT){
                        ++i;
                        if (i < array.count){
                            token = read_config_token(array, &i);
                            if (token.type == CPP_TOKEN_BRACKET_CLOSE){
                                ++i;
                                if (i < array.count){
                                    token = read_config_token(array, &i);
                                    subscript_success = 1;
                                }
                            }
                        }
                    }
                }
            }
            
            if (subscript_success){
                if (token.type == CPP_TOKEN_EQ){
                    config_line.eq_token = read_config_token(array, &i);
                    ++i;
                    if (i < array.count){
                        Cpp_Token val_token = read_config_token(array, &i);
                        
                        bool32 array_success = 1;
                        if (val_token.type == CPP_TOKEN_BRACE_OPEN){
                            array_success = 0;
                            ++i;
                            if (i < array.count){
                                config_line.val_array_start = i;
                                
                                bool32 expecting_array_item = 1;
                                for (; i < array.count; ++i){
                                    Cpp_Token array_token = read_config_token(array, &i);
                                    if (array_token.size == 0){
                                        break;
                                    }
                                    if (array_token.type == CPP_TOKEN_BRACE_CLOSE){
                                        config_line.val_array_end = i;
                                        array_success = 1;
                                        break;
                                    }
                                    else{
                                        if (array_token.type == CPP_TOKEN_COMMA){
                                            if (!expecting_array_item){
                                                expecting_array_item = 1;
                                            }
                                            else{
                                                break;
                                            }
                                        }
                                        else{
                                            if (expecting_array_item){
                                                expecting_array_item = 0;
                                                ++config_line.val_array_count;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        
                        if (array_success){
                            config_line.val_token = val_token;
                            ++i;
                            if (i < array.count){
                                Cpp_Token semicolon_token = read_config_token(array, &i);
                                if (semicolon_token.type == CPP_TOKEN_SEMICOLON){
                                    config_line.read_success = 1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    if (!config_line.read_success){
        for (; i < array.count; ++i){
            Cpp_Token token = read_config_token(array, &i);
            if (token.type == CPP_TOKEN_SEMICOLON){
                break;
            }
        }
    }
    
    *i_ptr = i;
    
    return(config_line);
}

static Config_Item
get_config_item(Config_Line line, char *mem, Cpp_Token_Array array){
    Config_Item item = {0};
    item.line = line;
    item.array = array;
    item.mem = mem;
    if (line.id_token.size != 0){
        item.id = make_string(mem + line.id_token.start, line.id_token.size);
    }
    
    if (line.subscript_token.size != 0){
        String subscript_str = make_string(mem + line.subscript_token.start,line.subscript_token.size);
        item.subscript_index = str_to_int_s(subscript_str);
        item.has_subscript = 1;
    }
    
    return(item);
}

static bool32
config_var(Config_Item item, char *var_name, int32_t *subscript, uint32_t token_type, void *var_out){
    bool32 result = 0;
    bool32 subscript_succes = 1;
    if (item.line.val_token.type == token_type){
        if ((var_name == 0 && item.id.size == 0) || match(item.id, var_name)){
            if (subscript){
                if (item.has_subscript){
                    *subscript = item.subscript_index;
                }
                else{
                    subscript_succes = 0;
                }
            }
            
            if (subscript_succes){
                if (var_out){
                    switch (token_type){
                        case CPP_TOKEN_BOOLEAN_CONSTANT:
                        {
                            *(bool32*)var_out = (item.mem[item.line.val_token.start] == 't');
                        }break;
                        
                        case CPP_TOKEN_INTEGER_CONSTANT:
                        {
                            String val = make_string(item.mem + item.line.val_token.start, item.line.val_token.size);
                            *(int32_t*)var_out = str_to_int(val);
                        }break;
                        
                        case CPP_TOKEN_STRING_CONSTANT:
                        {
                            *(String*)var_out = make_string(item.mem + item.line.val_token.start + 1,item.line.val_token.size - 2);
                        }break;
                        
                        case CPP_TOKEN_BRACE_OPEN:
                        {
                            Config_Array_Reader *array_reader = (Config_Array_Reader*)var_out;
                            array_reader->array = item.array;
                            array_reader->mem = item.mem;
                            array_reader->i = item.line.val_array_start;
                            array_reader->val_array_end = item.line.val_array_end;
                            array_reader->good = 1;
                        }break;
                    }
                }
                result = 1;
            }
        }
    }
    return(result);
}

static bool32
config_bool_var(Config_Item item, char *var_name, int32_t *subscript, bool32 *var_out){
    bool32 result = config_var(item, var_name, subscript, CPP_TOKEN_BOOLEAN_CONSTANT, var_out);
    return(result);
}

static bool32
config_int_var(Config_Item item, char *var_name, int32_t *subscript, int32_t *var_out){
    bool32 result = config_var(item, var_name, subscript, CPP_TOKEN_INTEGER_CONSTANT, var_out);
    return(result);
}

static bool32
config_string_var(Config_Item item, char *var_name, int32_t *subscript, String *var_out){
    bool32 result = config_var(item, var_name, subscript, CPP_TOKEN_STRING_CONSTANT, var_out);
    return(result);
}

static bool32
config_array_var(Config_Item item, char *var_name, int32_t *subscript, Config_Array_Reader *array_reader){
    bool32 result = config_var(item, var_name, subscript, CPP_TOKEN_BRACE_OPEN, array_reader);
    return(result);
}

static bool32
config_array_next_item(Config_Array_Reader *array_reader, Config_Item *item){
    bool32 result = 0;
    
    for (;array_reader->i < array_reader->val_array_end;
         ++array_reader->i){
        Cpp_Token array_token = read_config_token(array_reader->array, &array_reader->i);
        if (array_token.size == 0 || array_reader->i >= array_reader->val_array_end){
            break;
        }
        
        if (array_token.type == CPP_TOKEN_BRACE_CLOSE){
            break;
        }
        
        switch (array_token.type){
            case CPP_TOKEN_BOOLEAN_CONSTANT:
            case CPP_TOKEN_INTEGER_CONSTANT:
            case CPP_TOKEN_STRING_CONSTANT:
            {
                Config_Line line = {0};
                line.val_token = array_token;
                line.read_success = 1;
                *item = get_config_item(line, array_reader->mem, array_reader->array);
                result = 1;
                ++array_reader->i;
                goto doublebreak;
            }break;
        }
    }
    doublebreak:;
    
    array_reader->good = result;
    return(result);
}

static bool32
config_array_good(Config_Array_Reader *array_reader){
    bool32 result = (array_reader->good);
    return(result);
}

// NOTE(allen|a4.0.12): A primordial config system (actually really hate this but it seems best at least right now... arg)

static bool32 enable_code_wrapping = 1;
static bool32 automatically_adjust_wrapping = 1;
static int32_t default_wrap_width = 672;
static int32_t default_min_base_width = 550;
static bool32 automatically_indent_text_on_save = 1;

static String default_theme_name = make_lit_string("4coder");
static String default_font_name = make_lit_string("Liberation Sans");

#include <stdio.h>

static void
adjust_all_buffer_wrap_widths(Application_Links *app, int32_t wrap_widths, int32_t min_base_width){
    for (Buffer_Summary buffer = get_buffer_first(app, AccessAll);
         buffer.exists;
         get_buffer_next(app, &buffer, AccessAll)){
        buffer_set_setting(app, &buffer, BufferSetting_WrapPosition, wrap_widths);
        buffer_set_setting(app, &buffer, BufferSetting_MinimumBaseWrapPosition, min_base_width);
    }
    default_wrap_width = wrap_widths;
    default_min_base_width = min_base_width;
}

static bool32
file_handle_dump(Partition *part, FILE *file, char **mem_ptr, int32_t *size_ptr){
    bool32 success = 0;
    
    fseek(file, 0, SEEK_END);
    int32_t size = ftell(file);
    char *mem = (char*)push_block(part, size+1);
    fseek(file, 0, SEEK_SET);
    int32_t check_size = (int32_t)fread(mem, 1, size, file);
    if (check_size == size){
        mem[size] = 0;
        success = 1;
    }
    
    *mem_ptr = mem;
    *size_ptr = size;
    
    return(success);
}

static void
process_config_file(Application_Links *app){
    Partition *part = &global_part;
    FILE *file = fopen("config.4coder", "rb");
    
    if (!file){
        char space[256];
        int32_t size = get_4ed_path(app, space, sizeof(space));
        String str = make_string_cap(space, size, sizeof(space));
        append_sc(&str, "/config.4coder");
        terminate_with_null(&str);
        file = fopen(str.str, "rb");
    }
    
    if (file){
        Temp_Memory temp = begin_temp_memory(part);
        
        char *mem = 0;
        int32_t size = 0;
        bool32 file_read_success = file_handle_dump(part, file, &mem, &size);
        
        if (file_read_success){
            fclose(file);
            
            Cpp_Token_Array array;
            array.count = 0;
            array.max_count = (1 << 20)/sizeof(Cpp_Token);
            array.tokens = push_array(&global_part, Cpp_Token, array.max_count);
            
            Cpp_Lex_Data S = cpp_lex_data_init();
            Cpp_Lex_Result result = cpp_lex_step(&S, mem, size+1, HAS_NULL_TERM, &array, NO_OUT_LIMIT);
            
            if (result == LexResult_Finished){
                int32_t new_wrap_width = default_wrap_width;
                int32_t new_min_base_width = default_min_base_width;
                
                for (int32_t i = 0; i < array.count; ++i){
                    Config_Line config_line = read_config_line(array, &i);
                    
                    if (config_line.read_success){
                        Config_Item item = get_config_item(config_line, mem, array);
                        
                        config_bool_var(item, "enable_code_wrapping", 0, &enable_code_wrapping);
                        config_bool_var(item, "automatically_adjust_wrapping", 0, &automatically_adjust_wrapping);
                        config_bool_var(item, "automatically_indent_text_on_save", 0, &automatically_indent_text_on_save);
                        
                        config_int_var(item, "default_wrap_width", 0, &new_wrap_width);
                        config_int_var(item, "default_min_base_width", 0, &new_min_base_width);
                        
                        config_string_var(item, "default_theme_name", 0, &default_theme_name);
                        config_string_var(item, "default_font_name", 0, &default_font_name);
                    }
                }
                adjust_all_buffer_wrap_widths(app, new_wrap_width, new_min_base_width);
            }
        }
        
        end_temp_memory(temp);
    }
    else{
        print_message(app, literal("Did not find config.4coder, using default settings\n"));
    }
}

// NOTE(allen): Project system setup

static char *default_extensions[] = {
    "cpp",
    "hpp",
    "c",
    "h",
    "cc"
};

struct Fkey_Command{
    char command[128];
    char out[128];
    bool32 use_build_panel;
};

struct Project{
    char dir_space[256];
    char *dir;
    int32_t dir_len;
    
    char extension_space[256];
    char *extensions[94];
    int32_t extension_count;
    
    Fkey_Command fkey_commands[16];
    
    bool32 close_all_code_when_this_project_closes;
    bool32 close_all_files_when_project_opens;
};

static Project null_project = {};
static Project current_project = {};

static void
set_project_extensions(Project *project, String src){
    int32_t mode = 0;
    int32_t j = 0, k = 0;
    for (int32_t i = 0; i < src.size; ++i){
        switch (mode){
            case 0:
            {
                if (src.str[i] == '.'){
                    mode = 1;
                    project->extensions[k++] = &project->extension_space[j];
                }
            }break;
            
            case 1:
            {
                if (src.str[i] == '.'){
                    project->extension_space[j++] = 0;
                    project->extensions[k++] = &project->extension_space[j];
                }
                else{
                    project->extension_space[j++] = src.str[i];
                }
            }break;
        }
    }
    project->extension_space[j++] = 0;
    project->extension_count = k;
}

// TODO(allen): make this a string operation or a lexer operation or something
static void
interpret_escaped_string(char *dst, String src){
    int32_t mode = 0;
    int32_t j = 0;
    for (int32_t i = 0; i < src.size; ++i){
        switch (mode){
            case 0:
            {
                if (src.str[i] == '\\'){
                    mode = 1;
                }
                else{
                    dst[j++] = src.str[i];
                }
            }break;
            
            case 1:
            {
                switch (src.str[i]){
                    case '\\':{dst[j++] = '\\'; mode = 0;}break;
                    case 'n': {dst[j++] = '\n'; mode = 0;}break;
                    case 't': {dst[j++] = '\t'; mode = 0;}break;
                    case '"': {dst[j++] = '"';  mode = 0;}break;
                    case '0': {dst[j++] = '\0'; mode = 0;}break;
                }
            }break;
        }
    }
    dst[j] = 0;
}

static void
close_all_files_with_extension(Application_Links *app, Partition *scratch_part, char **extension_list, int32_t extension_count){
    Temp_Memory temp = begin_temp_memory(scratch_part);
    
    int32_t buffers_to_close_max = partition_remaining(scratch_part)/sizeof(int32_t);
    int32_t *buffers_to_close = push_array(scratch_part, int32_t, buffers_to_close_max);
    
    int32_t buffers_to_close_count = 0;
    bool32 do_repeat = 0;
    do{
        buffers_to_close_count = 0;
        do_repeat = 0;
        
        uint32_t access = AccessAll;
        Buffer_Summary buffer = {0};
        for (buffer = get_buffer_first(app, access);
             buffer.exists;
             get_buffer_next(app, &buffer, access)){
            
            bool32 is_match = 1;
            if (extension_count > 0){
                String extension = file_extension(make_string(buffer.file_name, buffer.file_name_len));
                is_match = 0;
                for (int32_t i = 0; i < extension_count; ++i){
                    if (match(extension, extension_list[i])){
                        is_match = 1;
                        break;
                    }
                }
            }
            
            if (is_match){
                if (buffers_to_close_count >= buffers_to_close_max){
                    do_repeat = 1;
                    break;
                }
                buffers_to_close[buffers_to_close_count++] = buffer.buffer_id;
            }
        }
        
        for (int32_t i = 0; i < buffers_to_close_count; ++i){
            kill_buffer(app, buffer_identifier(buffers_to_close[i]), true, 0);
        }
    }
    while(do_repeat);
    
    end_temp_memory(temp);
}

static void
open_all_files_with_extension(Application_Links *app, Partition *scratch_part, char **extension_list, int32_t extension_count){
    Temp_Memory temp = begin_temp_memory(scratch_part);
    
    int32_t max_size = partition_remaining(scratch_part);
    char *memory = push_array(scratch_part, char, max_size);
    
    String dir = make_string_cap(memory, 0, max_size);
    dir.size = directory_get_hot(app, dir.str, dir.memory_size);
    int32_t dir_size = dir.size;
    
    // NOTE(allen|a3.4.4): Here we get the list of files in this directory.
    // Notice that we free_file_list at the end.
    File_List list = get_file_list(app, dir.str, dir.size);
    
    for (int32_t i = 0; i < list.count; ++i){
        File_Info *info = list.infos + i;
        if (!info->folder){
            bool32 is_match = 1;
            
            if (extension_count > 0){
                is_match = 0;
                
                String extension = make_string_cap(info->filename, info->filename_len, info->filename_len+1);
                extension = file_extension(extension);
                for (int32_t j = 0; j < extension_count; ++j){
                    if (match(extension, extension_list[j])){
                        is_match = 1;
                        break;
                    }
                }
                
                if (is_match){
                    // NOTE(allen): There's no way in the 4coder API to use relative
                    // paths at the moment, so everything should be full paths.  Which is
                    // managable.  Here simply set the dir string size back to where it
                    // was originally, so that new appends overwrite old ones.
                    dir.size = dir_size;
                    append_sc(&dir, info->filename);
                    create_buffer(app, dir.str, dir.size, 0);
                }
            }
        }
    }
    
    free_file_list(app, list);
    
    end_temp_memory(temp);
}

static char**
get_standard_code_extensions(int32_t *extension_count_out){
    char **extension_list = default_extensions;
    int32_t extension_count = ArrayCount(default_extensions);
    if (current_project.dir != 0){
        extension_list = current_project.extensions;
        extension_count = current_project.extension_count;
    }
    *extension_count_out = extension_count;
    return(extension_list);
}

// NOTE(allen|a4.0.14): open_all_code and close_all_code now use the extensions set in the loaded project.  If there is no project loaded the extensions ".cpp.hpp.c.h.cc" are used.
CUSTOM_COMMAND_SIG(open_all_code){
    int32_t extension_count = 0;
    char **extension_list = get_standard_code_extensions(&extension_count);
    open_all_files_with_extension(app, &global_part, extension_list, extension_count);
}

CUSTOM_COMMAND_SIG(close_all_code){
    int32_t extension_count = 0;
    char **extension_list = get_standard_code_extensions(&extension_count);
    close_all_files_with_extension(app, &global_part, extension_list, extension_count);
}

CUSTOM_COMMAND_SIG(load_project){
    Partition *part = &global_part;
    
    char project_file_space[512];
    String project_name = make_fixed_width_string(project_file_space);
    project_name.size = directory_get_hot(app, project_name.str, project_name.memory_size);
    if (project_name.size >= project_name.memory_size){
        project_name.size = 0;
    }
    
    if (project_name.size != 0){
        int32_t original_size = project_name.size;
        append_sc(&project_name, "project.4coder");
        terminate_with_null(&project_name);
        
        // TODO(allen): make sure we do nothing when this project is already open
        
        FILE *file = fopen(project_name.str, "rb");
        if (file){
            project_name.size = original_size;
            terminate_with_null(&project_name);
            
            Temp_Memory temp = begin_temp_memory(part);
            
            char *mem = 0;
            int32_t size = 0;
            bool32 file_read_success = file_handle_dump(part, file, &mem, &size);
            if (file_read_success){
                fclose(file);
                
                Cpp_Token_Array array;
                array.count = 0;
                array.max_count = (1 << 20)/sizeof(Cpp_Token);
                array.tokens = push_array(&global_part, Cpp_Token, array.max_count);
                
                Cpp_Lex_Data S = cpp_lex_data_init();
                Cpp_Lex_Result result = cpp_lex_step(&S, mem, size+1, HAS_NULL_TERM, &array, NO_OUT_LIMIT);
                
                if (result == LexResult_Finished){
                    // Clear out current project
                    if (current_project.close_all_code_when_this_project_closes){
                        exec_command(app, close_all_code);
                    }
                    current_project = null_project;
                    
                    // Set new project directory
                    {
                        current_project.dir = current_project.dir_space;
                        String str = make_fixed_width_string(current_project.dir_space);
                        copy(&str, project_name);
                        terminate_with_null(&str);
                        current_project.dir_len = str.size;
                    }
                    
                    // Read the settings from project.4coder
                    for (int32_t i = 0; i < array.count; ++i){
                        Config_Line config_line = read_config_line(array, &i);
                        if (config_line.read_success){
                            Config_Item item = get_config_item(config_line, mem, array);
                            
                            {
                                String str = {0};
                                if (config_string_var(item, "extensions", 0, &str)){
                                    if (str.size < sizeof(current_project.extension_space)){
                                        set_project_extensions(&current_project, str);
                                        print_message(app, str.str, str.size);
                                        print_message(app, "\n", 1);
                                    }
                                    else{
                                        print_message(app, literal("STRING TOO LONG!\n"));
                                    }
                                }
                            }
                            
                            {
#if defined(_WIN32)
#define FKEY_COMMAND "fkey_command_win"
#elif defined(__linux__)
#define FKEY_COMMAND "fkey_command_linux"
#else
#error no project configuration names for this platform
#endif
                                
                                int32_t index = 0;
                                Config_Array_Reader array_reader = {0};
                                if (config_array_var(item, FKEY_COMMAND, &index, &array_reader)){
                                    if (index >= 1 && index <= 16){
                                        Config_Item array_item = {0};
                                        int32_t item_index = 0;
                                        
                                        char space[256];
                                        String msg = make_fixed_width_string(space);
                                        append(&msg, FKEY_COMMAND"[");
                                        append_int_to_str(&msg, index);
                                        append(&msg, "] = {");
                                        
                                        for (config_array_next_item(&array_reader, &array_item);
                                             config_array_good(&array_reader);
                                             config_array_next_item(&array_reader, &array_item)){
                                            
                                            if (item_index >= 3){
                                                break;
                                            }
                                            
                                            append(&msg, "[");
                                            append_int_to_str(&msg, item_index);
                                            append(&msg, "] = ");
                                            
                                            bool32 read_string = 0;
                                            bool32 read_bool = 0;
                                            
                                            char *dest_str = 0;
                                            int32_t dest_str_size = 0;
                                            
                                            bool32 *dest_bool = 0;
                                            
                                            switch (item_index){
                                                case 0:
                                                {
                                                    dest_str = current_project.fkey_commands[index-1].command;
                                                    dest_str_size = sizeof(current_project.fkey_commands[index-1].command);
                                                    read_string = 1;
                                                }break;
                                                
                                                case 1:
                                                {
                                                    dest_str = current_project.fkey_commands[index-1].out;
                                                    dest_str_size = sizeof(current_project.fkey_commands[index-1].out);
                                                    read_string = 1;
                                                }break;
                                                
                                                case 2:
                                                {
                                                    dest_bool = &current_project.fkey_commands[index-1].use_build_panel;
                                                    read_bool = 1;
                                                }break;
                                            }
                                            
                                            if (read_string){
                                                if (config_int_var(array_item, 0, 0, 0)){
                                                    append(&msg, "NULL, ");
                                                    dest_str[0] = 0;
                                                }
                                                
                                                String str = {0};
                                                if (config_string_var(array_item, 0, 0, &str)){
                                                    if (str.size < dest_str_size){
                                                        interpret_escaped_string(dest_str, str);
                                                        append(&msg, dest_str);
                                                        append(&msg, ", ");
                                                    }
                                                    else{
                                                        append(&msg, "STRING TOO LONG!, ");
                                                    }
                                                }
                                            }
                                            
                                            if (read_bool){
                                                if (config_bool_var(array_item, 0, 0, dest_bool)){
                                                    if (dest_bool){
                                                        append(&msg, "true, ");
                                                    }
                                                    else{
                                                        append(&msg, "false, ");
                                                    }
                                                }
                                            }
                                            
                                            item_index++;
                                        }
                                        
                                        append(&msg, "}\n");
                                        print_message(app, msg.str, msg.size);
                                    }
                                }
                            }
                        }
                    }
                    
                    if (current_project.close_all_files_when_project_opens){
                        close_all_files_with_extension(app, &global_part, 0, 0);
                    }
                    
                    // Open all project files
                    exec_command(app, open_all_code);
                }
            }
            
            end_temp_memory(temp);
        }
        else{
            char message_space[512];
            String message = make_fixed_width_string(message_space);
            append_sc(&message, "Did not find project.4coder.  ");
            if (current_project.dir != 0){
                append_sc(&message, "Continuing with: ");
                append_sc(&message, current_project.dir);
            }
            else{
                append_sc(&message, "Continuing without a project");
            }
            print_message(app, message.str, message.size);
        }
    }
    else{
        print_message(app, literal("Failed trying to get project file name"));
    }
}

CUSTOM_COMMAND_SIG(project_fkey_command){
    User_Input input = get_command_input(app);
    if (input.type == UserInputKey){
        if (input.key.keycode >= key_f1 && input.key.keycode <= key_f16){
            int32_t ind = (input.key.keycode - key_f1);
            
            char *command = current_project.fkey_commands[ind].command;
            char *out = current_project.fkey_commands[ind].out;
            bool32 use_build_panel = current_project.fkey_commands[ind].use_build_panel;
            
            if (command[0] != 0){
                int32_t command_len = str_size(command);
                
                View_Summary view_ = {0};
                View_Summary *view = 0;
                Buffer_Identifier buffer_id = {0};
                uint32_t flags = 0;
                
                bool32 set_fancy_font = 0;
                
                if (out[0] != 0){
                    int32_t out_len = str_size(out);
                    buffer_id = buffer_identifier(out, out_len);
                    
                    view = &view_;
                    
                    if (use_build_panel){
                        view_ = get_or_open_build_panel(app);
                        if (match(out, "*compilation*")){
                            set_fancy_font = 1;
                        }
                    }
                    else{
                        view_ = get_active_view(app, AccessAll);
                    }
                    
                    prev_location = null_location;
                    lock_jump_buffer(out, out_len);
                }
                else{
                    // TODO(allen): fix the exec_system_command call so it can take a null buffer_id.
                    buffer_id = buffer_identifier(literal("*dump*"));
                }
                
                exec_system_command(app, view, buffer_id, current_project.dir, current_project.dir_len, command, command_len, flags);
                if (set_fancy_font){
                    set_fancy_compilation_buffer_font(app);
                }
            }
        }
    }
}

//
// Default Framework
//

void
init_memory(Application_Links *app){
    int32_t part_size = (32 << 20);
    int32_t general_size = (4 << 20);
    
    void *part_mem = memory_allocate(app, part_size);
    global_part = make_part(part_mem, part_size);
    
    void *general_mem = memory_allocate(app, general_size);
    general_memory_open(&global_general, general_mem, general_size);
}

static void
default_4coder_initialize(Application_Links *app){
    init_memory(app);
    process_config_file(app);
    change_theme(app, default_theme_name.str, default_theme_name.size);
    change_font(app, default_font_name.str, default_font_name.size, 1);
}

static void
default_4coder_side_by_side_panels(Application_Links *app){
    exec_command(app, open_panel_vsplit);
    exec_command(app, hide_scrollbar);
    exec_command(app, change_active_panel);
    exec_command(app, hide_scrollbar);
}

#endif

