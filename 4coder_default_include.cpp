/*
4coder_default_include.cpp - Default set of commands and setup used in 4coder.

TYPE: 'major-system-include'
*/

// TOP

#if !defined(FCODER_DEFAULT_INCLUDE_CPP)
#define FCODER_DEFAULT_INCLUDE_CPP

#include "4coder_API/custom.h"

#include "4coder_helper/4coder_jump_parsing.h"

#include "4coder_default_framework.h"
#include "4coder_base_commands.cpp"
#include "4coder_auto_indent.cpp"
#include "4coder_search.cpp"
#include "4coder_jump_parsing.cpp"
#include "4coder_clipboard.cpp"
#include "4coder_system_command.cpp"
#include "4coder_build_commands.cpp"
#include "4coder_project_commands.cpp"
#include "4coder_default_hooks.cpp"
#include "4coder_function_list.cpp"

#include "4coder_helper/4coder_bind_helper.h"
#include "4coder_helper/4coder_helper.h"
#include "4coder_helper/4coder_streaming.h"
#include "4coder_helper/4coder_long_seek.h"

#define FSTRING_IMPLEMENTATION
#include "4coder_lib/4coder_string.h"
#include "4coder_lib/4coder_table.h"
#include "4coder_lib/4coder_mem.h"
#include "4coder_lib/4coder_utf8.h"

#include "4cpp/4cpp_lexer.h"

//
// Seeks Using Default Framework Memory
//

static int32_t
buffer_boundary_seek(Application_Links *app, Buffer_Summary *buffer, int32_t start_pos, bool32 seek_forward, Seek_Boundary_Flag flags){
    int32_t result = buffer_boundary_seek(app, buffer, &global_part, start_pos, seek_forward, flags);
    return(result);
}

static void
basic_seek(Application_Links *app, bool32 seek_forward, uint32_t flags){
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    int32_t pos = buffer_boundary_seek(app, &buffer, view.cursor.pos, seek_forward, flags);
    view_set_cursor(app, &view, seek_pos(pos), true);
}

#define right true
#define left false

CUSTOM_COMMAND_SIG(seek_whitespace_right){ basic_seek(app, right, BoundaryWhitespace); }
CUSTOM_COMMAND_SIG(seek_whitespace_left){ basic_seek(app, left, BoundaryWhitespace); }
CUSTOM_COMMAND_SIG(seek_token_right){ basic_seek(app, right, BoundaryToken); }
CUSTOM_COMMAND_SIG(seek_token_left){ basic_seek(app, left, BoundaryToken); }
CUSTOM_COMMAND_SIG(seek_white_or_token_right){basic_seek(app, right, BoundaryToken | BoundaryWhitespace);}
CUSTOM_COMMAND_SIG(seek_white_or_token_left){basic_seek(app, left, BoundaryToken | BoundaryWhitespace);}
CUSTOM_COMMAND_SIG(seek_alphanumeric_right){ basic_seek(app, right, BoundaryAlphanumeric); }
CUSTOM_COMMAND_SIG(seek_alphanumeric_left){ basic_seek(app, left, BoundaryAlphanumeric); }
CUSTOM_COMMAND_SIG(seek_alphanumeric_or_camel_right){ basic_seek(app, right, BoundaryAlphanumeric | BoundaryCamelCase); }
CUSTOM_COMMAND_SIG(seek_alphanumeric_or_camel_left){ basic_seek(app, left, BoundaryAlphanumeric | BoundaryCamelCase); }

#undef right
#undef left

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
// Line Manipulation
//

CUSTOM_COMMAND_SIG(duplicate_line){
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    
    Partition *part = &global_part;
    
    Temp_Memory temp = begin_temp_memory(part);
    String line_string = {0};
    read_line(app, part, &buffer, view.cursor.line, &line_string);
    
    push_array(part, char, 1);
    ++line_string.memory_size;
    append_s_char(&line_string, '\n');
    
    int32_t pos = buffer_get_line_end(app, &buffer, view.cursor.line) + 1;
    buffer_replace_range(app, &buffer, pos, pos, line_string.str, line_string.size);
    
    end_temp_memory(temp);
}

CUSTOM_COMMAND_SIG(delete_line){
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    
    Partition *part = &global_part;
    
    Temp_Memory temp = begin_temp_memory(part);
    int32_t start = buffer_get_line_start(app, &buffer, view.cursor.line);
    int32_t end = buffer_get_line_end(app, &buffer, view.cursor.line) + 1;
    
    buffer_replace_range(app, &buffer, start, end, 0, 0);
    
    end_temp_memory(temp);
}


//
// Clipboard + Indent Combo Command
//

CUSTOM_COMMAND_SIG(paste_and_indent){
    exec_command(app, paste);
    exec_command(app, auto_tab_range);
}

CUSTOM_COMMAND_SIG(paste_next_and_indent){
    exec_command(app, paste_next);
    exec_command(app, auto_tab_range);
}


//
// Combined Write Commands
//

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

CUSTOM_COMMAND_SIG(write_todo){
    char space[512];
    String str = make_fixed_width_string(space);
    
    char *name = 0;
    int32_t name_len = 0;
    if (get_current_name(&name, &name_len)){
        append(&str, "// TODO(");
        append(&str, make_string(name, name_len));
        append(&str, "): ");
    }
    else{
        append(&str, "// TODO: ");
    }
    
    write_string(app, str);
}

CUSTOM_COMMAND_SIG(write_note){
    char space[512];
    String str = make_fixed_width_string(space);
    
    char *name = 0;
    int32_t name_len = 0;
    if (get_current_name(&name, &name_len)){
        append(&str, "// NOTE(");
        append(&str, make_string(name, name_len));
        append(&str, "): ");
    }
    else{
        append(&str, "// NOTE: ");
    }
    
    write_string(app, str);
}

CUSTOM_COMMAND_SIG(write_block){
    write_string(app, make_lit_string("/*  */"));
}

CUSTOM_COMMAND_SIG(write_zero_struct){
    write_string(app, make_lit_string(" = {0};"));
}


//
// Open File In Quotes
//

static bool32
file_name_in_quotes(Application_Links *app, String *file_name){
    bool32 result = false;
    uint32_t access = AccessProtected;
    
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    if (buffer.file_name != 0){
        int32_t pos = view.cursor.pos;
        int32_t start = 0, end = 0;
        buffer_seek_delimiter_forward(app, &buffer, pos, '"', &end);
        buffer_seek_delimiter_backward(app, &buffer, pos, '"', &start);
        ++start;
        
        int32_t size = end - start;
        
        char short_file_name[128];
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

CUSTOM_COMMAND_SIG(new_in_other){
    exec_command(app, change_active_panel);
    exec_command(app, interactive_new);
}


//
// File Navigating
//

static bool32
get_cpp_matching_file(Application_Links *app, Buffer_Summary buffer, Buffer_Summary *buffer_out){
    bool32 result = false;
    
    if (buffer.file_name != 0){
        char space[512];
        String file_name = make_string_cap(space, 0, sizeof(space));
        append(&file_name, make_string(buffer.file_name, buffer.file_name_len));
        
        String extension = file_extension(file_name);
        String new_extensions[2] = {0};
        int32_t new_extensions_count = 0;
        
        if (match(extension, "cpp") || match(extension, "cc")){
            new_extensions[0] = make_lit_string("h");
            new_extensions[1] = make_lit_string("hpp");
            new_extensions_count = 2;
        }
        else if (match(extension, "c")){
            new_extensions[0] = make_lit_string("h");
            new_extensions_count = 1;
        }
        else if (match(extension, "h")){
            new_extensions[0] = make_lit_string("c");
            new_extensions[1] = make_lit_string("cpp");
            new_extensions_count = 2;
        }
        else if (match(extension, "hpp")){
            new_extensions[0] = make_lit_string("cpp");
            new_extensions_count = 1;
        }
        
        remove_extension(&file_name);
        int32_t base_pos = file_name.size;
        for (int32_t i = 0; i < new_extensions_count; ++i){
            String ext = new_extensions[i];
            file_name.size = base_pos;
            append(&file_name, ext);
            
            if (open_file(app, buffer_out, file_name.str, file_name.size, false, true)){
                result = true;
                break;
            }
        }
    }
    
    return(result);
}

CUSTOM_COMMAND_SIG(open_matching_file_cpp){
    View_Summary view = get_active_view(app, AccessAll);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessAll);
    
    Buffer_Summary new_buffer = {0};
    if (get_cpp_matching_file(app, buffer, &new_buffer)){
        get_view_next_looped(app, &view, AccessAll);
        view_set_buffer(app, &view, new_buffer.buffer_id, 0);
        set_active_view(app, &view);
    }
}


//
// Execute Arbitrary Command
//

CUSTOM_COMMAND_SIG(execute_arbitrary_command){
    // NOTE(allen): This isn't a super powerful version of this command, I will expand
    // upon it so that it has all the cmdid_* commands by default.  However, with this
    // as an example you have everything you need to make it work already. You could
    // even use app->memory to create a hash table in the start hook.
    Query_Bar bar;
    char space[1024];
    bar.prompt = make_lit_string("Command: ");
    bar.string = make_fixed_width_string(space);
    
    if (!query_user_string(app, &bar)) return;
    
    // NOTE(allen): Here I chose to end this query bar because when I call another
    // command it might ALSO have query bars and I don't want this one hanging
    // around at that point.  Since the bar exists on my stack the result of the query
    // is still available in bar.string though.
    end_query_bar(app, &bar, 0);
    
    if (match_ss(bar.string, make_lit_string("load project"))){
        exec_command(app, load_project);
    }
    else if (match_ss(bar.string, make_lit_string("open all code"))){
        exec_command(app, open_all_code);
    }
    else if (match_ss(bar.string, make_lit_string("open all code recursive"))){
        exec_command(app, open_all_code_recursive);
    }
    else if(match_ss(bar.string, make_lit_string("close all code"))){
        exec_command(app, close_all_code);
    }
    else if (match_ss(bar.string, make_lit_string("open menu"))){
        exec_command(app, cmdid_open_menu);
    }
    else if (match_ss(bar.string, make_lit_string("dos lines")) ||
             match_ss(bar.string, make_lit_string("dosify"))){
        exec_command(app, eol_dosify);
    }
    else if (match_ss(bar.string, make_lit_string("nix lines")) ||
             match_ss(bar.string, make_lit_string("nixify"))){
        exec_command(app, eol_nixify);
    }
    else{
        print_message(app, literal("unrecognized command\n"));
    }
}

#endif

// BOTTOM

