/*
4coder_default_include.cpp - Default set of commands and setup used in 4coder.

TYPE: 'major-system-include'
*/

// TOP

#if !defined(FCODER_DEFAULT_INCLUDE_CPP)
#define FCODER_DEFAULT_INCLUDE_CPP

#include "4coder_API/custom.h"

#include "4coder_os_comp_cracking.h"

#include "4coder_default_framework.h"
#include "4coder_base_commands.cpp"
#include "4coder_auto_indent.cpp"
#include "4coder_search.cpp"
#include "4coder_jump_direct.cpp"
#include "4coder_jump_sticky.cpp"
#include "4coder_clipboard.cpp"
#include "4coder_system_command.cpp"
#include "4coder_build_commands.cpp"
#include "4coder_project_commands.cpp"
#include "4coder_function_list.cpp"
#include "4coder_scope_commands.cpp"

// NOTE(allen): Define USE_OLD_STYLE_JUMPS before 4coder_default_include.cpp to get
// the direct jumps (instead of sticky jumps).
#if defined(USE_OLD_STYLE_JUMPS)

#define goto_jump_at_cursor                 CUSTOM_ALIAS(goto_jump_at_cursor_direct)
#define goto_jump_at_cursor_same_panel      CUSTOM_ALIAS(goto_jump_at_cursor_same_panel_direct)
#define goto_next_jump                      CUSTOM_ALIAS(goto_next_jump_direct)
#define goto_prev_jump                      CUSTOM_ALIAS(goto_prev_jump_direct)
#define goto_next_jump_no_skips             CUSTOM_ALIAS(goto_next_jump_no_skips_direct)
#define goto_prev_jump_no_skips             CUSTOM_ALIAS(goto_prev_jump_no_skips_direct)
#define goto_first_jump                     CUSTOM_ALIAS(goto_first_jump_direct)
#define newline_or_goto_position            CUSTOM_ALIAS(newline_or_goto_position_direct)
#define newline_or_goto_position_same_panel CUSTOM_ALIAS(newline_or_goto_position_same_panel_direct)

#else

#define goto_jump_at_cursor                 CUSTOM_ALIAS(goto_jump_at_cursor_sticky)
#define goto_jump_at_cursor_same_panel      CUSTOM_ALIAS(goto_jump_at_cursor_same_panel_sticky)
#define goto_next_jump                      CUSTOM_ALIAS(goto_next_jump_sticky)
#define goto_prev_jump                      CUSTOM_ALIAS(goto_prev_jump_sticky)
#define goto_next_jump_no_skips             CUSTOM_ALIAS(goto_next_jump_no_skips_sticky)
#define goto_prev_jump_no_skips             CUSTOM_ALIAS(goto_prev_jump_no_skips_sticky)
#define goto_first_jump                     CUSTOM_ALIAS(goto_first_jump_sticky)
#define newline_or_goto_position            CUSTOM_ALIAS(newline_or_goto_position_sticky)
#define newline_or_goto_position_same_panel CUSTOM_ALIAS(newline_or_goto_position_same_panel_sticky)

#endif

#define seek_error               CUSTOM_ALIAS(seek_jump)
#define goto_next_error          CUSTOM_ALIAS(goto_next_jump)
#define goto_prev_error          CUSTOM_ALIAS(goto_prev_jump)
#define goto_next_error_no_skips CUSTOM_ALIAS(goto_next_jump_no_skips)
#define goto_prev_error_no_skips CUSTOM_ALIAS(goto_prev_jump_no_skips)
#define goto_first_error         CUSTOM_ALIAS(goto_first_jump)

#include "4coder_default_hooks.cpp"

#include "4coder_helper/4coder_bind_helper.h"
#include "4coder_helper/4coder_helper.h"
#include "4coder_helper/4coder_streaming.h"
#include "4coder_helper/4coder_long_seek.h"

#define FSTRING_IMPLEMENTATION
#include "4coder_lib/4coder_string.h"
#include "4coder_lib/4coder_table.h"
#include "4coder_lib/4coder_mem.h"
#include "4coder_lib/4coder_utf8.h"

#include "4coder_lib/4cpp_lexer.h"

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

CUSTOM_COMMAND_SIG(seek_whitespace_right)
CUSTOM_DOC("Seek right for the next boundary between whitespace and non-whitespace.")
{ basic_seek(app, right, BoundaryWhitespace); }

CUSTOM_COMMAND_SIG(seek_whitespace_left)
CUSTOM_DOC("Seek left for the next boundary between whitespace and non-whitespace.")
{ basic_seek(app, left, BoundaryWhitespace); }

CUSTOM_COMMAND_SIG(seek_token_right)
CUSTOM_DOC("Seek right for the next end of a token.")
{ basic_seek(app, right, BoundaryToken); }

CUSTOM_COMMAND_SIG(seek_token_left)
CUSTOM_DOC("Seek left for the next beginning of a token.")
{ basic_seek(app, left, BoundaryToken); }

CUSTOM_COMMAND_SIG(seek_white_or_token_right)
CUSTOM_DOC("Seek right for the next end of a token or boundary between whitespace and non-whitespace.")
{basic_seek(app, right, BoundaryToken | BoundaryWhitespace);}

CUSTOM_COMMAND_SIG(seek_white_or_token_left)
CUSTOM_DOC("Seek left for the next end of a token or boundary between whitespace and non-whitespace.")
{basic_seek(app, left, BoundaryToken | BoundaryWhitespace);}

CUSTOM_COMMAND_SIG(seek_alphanumeric_right)
CUSTOM_DOC("Seek right for boundary between alphanumeric characters and non-alphanumeric characters.")
{ basic_seek(app, right, BoundaryAlphanumeric); }

CUSTOM_COMMAND_SIG(seek_alphanumeric_left)
CUSTOM_DOC("Seek left for boundary between alphanumeric characters and non-alphanumeric characters.")
{ basic_seek(app, left, BoundaryAlphanumeric); }

CUSTOM_COMMAND_SIG(seek_alphanumeric_or_camel_right)
CUSTOM_DOC("Seek right for boundary between alphanumeric characters or camel case word and non-alphanumeric characters.")
{ basic_seek(app, right, BoundaryAlphanumeric | BoundaryCamelCase); }

CUSTOM_COMMAND_SIG(seek_alphanumeric_or_camel_left)
CUSTOM_DOC("Seek left for boundary between alphanumeric characters or camel case word and non-alphanumeric characters.")
{ basic_seek(app, left, BoundaryAlphanumeric | BoundaryCamelCase); }

#undef right
#undef left

//
// Fast Deletes 
//

CUSTOM_COMMAND_SIG(backspace_word)
CUSTOM_DOC("Delete characters between the cursor position and the first alphanumeric boundary to the left.")
{
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

CUSTOM_COMMAND_SIG(delete_word)
CUSTOM_DOC("Delete characters between the cursor position and the first alphanumeric boundary to the right.")
{
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

CUSTOM_COMMAND_SIG(snipe_token_or_word)
CUSTOM_DOC("Delete a single, whole token on or to the left of the cursor and post it to the clipboard.")
{
    uint32_t access = AccessOpen;
    
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    int32_t pos1 = buffer_boundary_seek(app, &buffer, view.cursor.pos, false, BoundaryToken|BoundaryWhitespace);
    int32_t pos2 = buffer_boundary_seek(app, &buffer, pos1,            true,  BoundaryToken|BoundaryWhitespace);
    
    Range range = make_range(pos1, pos2);
    
    Partition *part = &global_part;
    Temp_Memory temp = begin_temp_memory(part);
    int32_t len = range.end - range.start;
    char *space = push_array(part, char, len);
    buffer_read_range(app, &buffer, range.start, range.end, space);
    clipboard_post(app, 0, space, len);
    end_temp_memory(temp);
    
    buffer_replace_range(app, &buffer, range.start, range.end, 0, 0);
}

CUSTOM_COMMAND_SIG(snipe_token_or_word_right)
CUSTOM_DOC("Delete a single, whole token on or to the right of the cursor and post it to the clipboard.")
{
    uint32_t access = AccessOpen;
    
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    int32_t pos2 = buffer_boundary_seek(app, &buffer, view.cursor.pos, true,  BoundaryToken|BoundaryWhitespace);
    int32_t pos1 = buffer_boundary_seek(app, &buffer, pos2,            false, BoundaryToken|BoundaryWhitespace);
    
    Range range = make_range(pos1, pos2);
    
    Partition *part = &global_part;
    Temp_Memory temp = begin_temp_memory(part);
    int32_t len = range.end - range.start;
    char *space = push_array(part, char, len);
    buffer_read_range(app, &buffer, range.start, range.end, space);
    clipboard_post(app, 0, space, len);
    end_temp_memory(temp);
    
    buffer_replace_range(app, &buffer, range.start, range.end, 0, 0);
}


//
// Query Replace Selection
//

CUSTOM_COMMAND_SIG(query_replace_selection)
CUSTOM_DOC("Queries the user for a string, and incrementally replace every occurence of the string found in the selected range with the specified string.")
{
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    
    if (!buffer.exists){
        return;
    }
    
    Partition *part = &global_part;
    Temp_Memory temp = begin_temp_memory(part);
    
    Range range = get_range(&view);
    int32_t replace_length = range.max - range.min;
    if (replace_length != 0){
        char *replace_space = push_array(part, char, replace_length);
        if (buffer_read_range(app, &buffer, range.min, range.max, replace_space)){
            String replace = make_string(replace_space, replace_length);
            query_replace_parameter(app, replace, range.min, true);
        }
    }
    
    end_temp_memory(temp);
}


//
// Line Manipulation
//

CUSTOM_COMMAND_SIG(move_line_up)
CUSTOM_DOC("Swaps the line under the cursor with the line above it, and moves the cursor up with it.")
{
    View_Summary view = get_active_view(app, AccessOpen);
    
    if (view.cursor.line <= 1){
        return;
    }
    
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    if (!buffer.exists){
        return;
    }
    
    Full_Cursor prev_line_cursor = {0};
    Full_Cursor this_line_cursor = {0};
    Full_Cursor next_line_cursor = {0};
    
    int32_t this_line = view.cursor.line;
    int32_t prev_line = this_line - 1;
    int32_t next_line = this_line +1;
    
    if (view_compute_cursor(app, &view, seek_line_char(prev_line, 1), &prev_line_cursor) &&
        view_compute_cursor(app, &view, seek_line_char(this_line, 1), &this_line_cursor) &&
        view_compute_cursor(app, &view, seek_line_char(next_line, 1), &next_line_cursor)){
        
        int32_t prev_line_pos = prev_line_cursor.pos;
        int32_t this_line_pos = this_line_cursor.pos;
        int32_t next_line_pos = next_line_cursor.pos;
        
        Partition *part = &global_part;
        Temp_Memory temp = begin_temp_memory(part);
        
        int32_t length = next_line_pos - prev_line_pos;
        char *swap = push_array(part, char, length + 1);
        int32_t first_len = next_line_pos - this_line_pos;
        
        if (buffer_read_range(app, &buffer, this_line_pos, next_line_pos, swap)){
            bool32 second_line_didnt_have_newline = true;
            for (int32_t i = first_len - 1; i >= 0; --i){
                if (swap[i] == '\n'){
                    second_line_didnt_have_newline = false;
                    break;
                }
            }
            
            if (second_line_didnt_have_newline){
                swap[first_len] = '\n';
                first_len += 1;
                // NOTE(allen): Don't increase "length" because then we will be including
                // the original newline and addignt this new one, making the file longer
                // which shouldn't be possible for this command!
            }
            
            if (buffer_read_range(app, &buffer, prev_line_pos, this_line_pos, swap + first_len)){
                buffer_replace_range(app, &buffer, prev_line_pos, next_line_pos, swap, length);
                view_set_cursor(app, &view, seek_line_char(prev_line, 1), true);
            }
        }
        
        end_temp_memory(temp);
    }
}

CUSTOM_COMMAND_SIG(move_down_textual)
CUSTOM_DOC("Moves down to the next line of actual text, regardless of line wrapping.")
{
    View_Summary view = get_active_view(app, AccessOpen);
    if (!view.exists){
        return;
    }
    int32_t next_line = view.cursor.line + 1;
    view_set_cursor(app, &view, seek_line_char(next_line, 1), true);
}

CUSTOM_COMMAND_SIG(move_line_down)
CUSTOM_DOC("Swaps the line under the cursor with the line below it, and moves the cursor down with it.")
{
    View_Summary view = get_active_view(app, AccessOpen);
    if (!view.exists){
        return;
    }
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    if (!buffer.exists){
        return;
    }
    
    int32_t next_line = view.cursor.line + 1;
    Full_Cursor new_cursor = {0};
    if (view_compute_cursor(app, &view, seek_line_char(next_line, 1), &new_cursor)){
        if (new_cursor.line == next_line){
            view_set_cursor(app, &view, seek_pos(new_cursor.pos), true);
            move_line_up(app);
            move_down_textual(app);
        }
    }
}

CUSTOM_COMMAND_SIG(duplicate_line)
CUSTOM_DOC("Create a copy of the line on which the cursor sits.")
{
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    
    Partition *part = &global_part;
    
    Temp_Memory temp = begin_temp_memory(part);
    String line_string = {0};
    char *before_line = push_array(part, char, 1);
    if (read_line(app, part, &buffer, view.cursor.line, &line_string)){
        *before_line = '\n';
        line_string.str = before_line;
        line_string.size += 1;
        
        int32_t pos = buffer_get_line_end(app, &buffer, view.cursor.line);
        buffer_replace_range(app, &buffer, pos, pos, line_string.str, line_string.size);
    }
    end_temp_memory(temp);
}

CUSTOM_COMMAND_SIG(delete_line)
CUSTOM_DOC("Delete the line the on which the cursor sits.")
{
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    
    Partition *part = &global_part;
    
    Temp_Memory temp = begin_temp_memory(part);
    int32_t start = buffer_get_line_start(app, &buffer, view.cursor.line);
    int32_t end = buffer_get_line_end(app, &buffer, view.cursor.line) + 1;
    if (end > buffer.size){
        end = buffer.size;
    }
    if (start == end || buffer_get_char(app, &buffer, end - 1) != '\n'){
        start -= 1;
        if (start < 0){
            start = 0;
        }
    }
    
    buffer_replace_range(app, &buffer, start, end, 0, 0);
    
    end_temp_memory(temp);
}


//
// Clipboard + Indent Combo Command
//

CUSTOM_COMMAND_SIG(paste_and_indent)
CUSTOM_DOC("Paste from the top of clipboard and run auto-indent on the newly pasted text.")
{
    exec_command(app, paste);
    exec_command(app, auto_tab_range);
}

CUSTOM_COMMAND_SIG(paste_next_and_indent)
CUSTOM_DOC("Paste the next item on the clipboard and run auto-indent on the newly pasted text.")
{
    exec_command(app, paste_next);
    exec_command(app, auto_tab_range);
}


//
// Approximate Definition Search
//

static void
get_search_definition(Application_Links *app, Query_Bar *bar, char *string_space, int32_t space_size){
    bar->prompt = make_lit_string("List Definitions For: ");
    bar->string = make_string_cap(string_space, 0, space_size);
    
    if (!query_user_string(app, bar)){
        bar->string.size = 0;
    }
}

static String
build_string(Partition *part, char *s1, char *s2, char *s3){
    String sr = {0};
    sr.memory_size = str_size(s1) + str_size(s2) + str_size(s3);
    sr.str = push_array(part, char, sr.memory_size);
    append(&sr, s1);
    append(&sr, s2);
    append(&sr, s3);
    return(sr);
}

static void
list_all_locations_of_type_definition_parameters(Application_Links *app, char *str){
    Partition *part = &global_part;
    Temp_Memory temp = begin_temp_memory(part);
    
    String match_strings[6];
    match_strings[0] = build_string(part, "struct ", str, "{");
    match_strings[1] = build_string(part, "struct ", str, "\n{");
    match_strings[2] = build_string(part, "union " , str, "{");
    match_strings[3] = build_string(part, "union " , str, "\n{");
    match_strings[4] = build_string(part, "enum "  , str, "{");
    match_strings[5] = build_string(part, "enum "  , str, "\n{");
    
    list_all_locations_parameters(app, &global_general, part, match_strings, ArrayCount(match_strings), 0);
    
    end_temp_memory(temp);
    
    Buffer_Summary buffer = get_buffer_by_name(app, literal("*search*"), AccessAll);
    if (buffer.line_count == 2){
        goto_first_jump_same_panel_sticky(app);
    }
}

CUSTOM_COMMAND_SIG(list_all_locations_of_type_definition)
CUSTOM_DOC("Queries user for string, lists all locations of strings that appear to define a type whose name matches the input string.")
{
    char string_space[1024];
    Query_Bar bar;
    get_search_definition(app, &bar, string_space, sizeof(string_space));
    if (bar.string.size == 0) return;
    if (!terminate_with_null(&bar.string)) return;
    
    list_all_locations_of_type_definition_parameters(app, bar.string.str);
}

CUSTOM_COMMAND_SIG(list_all_locations_of_type_definition_of_identifier)
CUSTOM_DOC("Reads a token or word under the cursor and lists all locations of strings that appear to define a type whose name matches it.")
{
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    
    char space[512];
    String str = get_token_or_word_under_pos(app, &buffer, view.cursor.pos, space, sizeof(space) - 1);
    if (str.size > 0){
        str.str[str.size] = 0;
        
        change_active_panel(app);
        list_all_locations_of_type_definition_parameters(app, str.str);
    }
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
    
    buffer_auto_indent(app, &global_part, &buffer, pos, pos + size, DEF_TAB_WIDTH, DEFAULT_INDENT_FLAGS | AutoIndent_FullTokens);
    move_past_lead_whitespace(app, &view, &buffer);
}

CUSTOM_COMMAND_SIG(open_long_braces)
CUSTOM_DOC("At the cursor, insert a '{' and '}' separated by a blank line.")
{
    char text[] = "{\n\n}";
    int32_t size = sizeof(text) - 1;
    long_braces(app, text, size);
}

CUSTOM_COMMAND_SIG(open_long_braces_semicolon)
CUSTOM_DOC("At the cursor, insert a '{' and '};' separated by a blank line.")
{
    char text[] = "{\n\n};";
    int32_t size = sizeof(text) - 1;
    long_braces(app, text, size);
}

CUSTOM_COMMAND_SIG(open_long_braces_break)
CUSTOM_DOC("At the cursor, insert a '{' and '}break;' separated by a blank line.")
{
    char text[] = "{\n\n}break;";
    int32_t size = sizeof(text) - 1;
    long_braces(app, text, size);
}

CUSTOM_COMMAND_SIG(if0_off)
CUSTOM_DOC("Surround the range between the cursor and mark with an '#if 0' and an '#endif'")
{
    place_begin_and_end_on_own_lines(app, &global_part, "#if 0", "#endif");
}

static void
write_named_comment_string(Application_Links *app, char *type_string){
    char space[512];
    String str = make_fixed_width_string(space);
    
    char *name = 0;
    int32_t name_len = 0;
    if (get_current_name(&name, &name_len)){
        append(&str, "// ");
        append(&str, type_string);
        append(&str, "(");
        append(&str, make_string(name, name_len));
        append(&str, "): ");
    }
    else{
        append(&str, "// ");
        append(&str, type_string);
        append(&str, ": ");
    }
    
    write_string(app, str);
}

CUSTOM_COMMAND_SIG(write_todo)
CUSTOM_DOC("At the cursor, insert a '// TODO' comment, includes user name if it was specified in config.4coder.")
{
    write_named_comment_string(app, "TODO");
}

CUSTOM_COMMAND_SIG(write_hack)
CUSTOM_DOC("At the cursor, insert a '// HACK' comment, includes user name if it was specified in config.4coder.")
{
    write_named_comment_string(app, "HACK");
}

CUSTOM_COMMAND_SIG(write_note)
CUSTOM_DOC("At the cursor, insert a '// NOTE' comment, includes user name if it was specified in config.4coder.")
{
    write_named_comment_string(app, "NOTE");
}

CUSTOM_COMMAND_SIG(write_block)
CUSTOM_DOC("At the cursor, insert a block comment.")
{
    write_string(app, make_lit_string("/*  */"));
}

CUSTOM_COMMAND_SIG(write_zero_struct)
CUSTOM_DOC("At the cursor, insert a ' = {0};'.")
{
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

CUSTOM_COMMAND_SIG(open_file_in_quotes)
CUSTOM_DOC("Reads a filename from surrounding '\"' characters and attempts to open the corresponding file.")
{
    char file_name_[256];
    String file_name = make_fixed_width_string(file_name_);
    
    if (file_name_in_quotes(app, &file_name)){
        exec_command(app, change_active_panel);
        View_Summary view = get_active_view(app, AccessAll);
        view_open_file(app, &view, expand_str(file_name), true);
    }
}

//
// File Navigating
//

CUSTOM_COMMAND_SIG(open_in_other)
CUSTOM_DOC("Reads a filename from surrounding '\"' characters and attempts to open the corresponding file, displaying it in the other view.")
{
    exec_command(app, change_active_panel);
    exec_command(app, interactive_open_or_new);
}

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

CUSTOM_COMMAND_SIG(open_matching_file_cpp)
CUSTOM_DOC("If the current file is a *.cpp or *.h, attempts to open the corresponding *.h or *.cpp file in the other view.")
{
    View_Summary view = get_active_view(app, AccessAll);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessAll);
    
    Buffer_Summary new_buffer = {0};
    if (get_cpp_matching_file(app, buffer, &new_buffer)){
        get_view_next_looped(app, &view, AccessAll);
        view_set_buffer(app, &view, new_buffer.buffer_id, 0);
        set_active_view(app, &view);
    }
}

CUSTOM_COMMAND_SIG(view_buffer_other_panel)
CUSTOM_DOC("Set the other non-active panel to view the buffer that the active panel views, and switch to that panel.")
{
    View_Summary view = get_active_view(app, AccessAll);
    int32_t buffer_id = view.buffer_id;
    change_active_panel(app);
    view = get_active_view(app, AccessAll);
    view_set_buffer(app, &view, buffer_id, 0);
}

CUSTOM_COMMAND_SIG(swap_buffers_between_panels)
CUSTOM_DOC("Set the other non-active panel to view the buffer that the active panel views, and switch to that panel.")
{
    View_Summary view1 = get_active_view(app, AccessAll);
    change_active_panel(app);
    View_Summary view2 = get_active_view(app, AccessAll);
    
    if (view1.view_id != view2.view_id){
        int32_t buffer_id1 = view1.buffer_id;
        int32_t buffer_id2 = view2.buffer_id;
        view_set_buffer(app, &view1, buffer_id2, 0);
        view_set_buffer(app, &view2, buffer_id1, 0);
    }
}

//
// Execute Arbitrary Command
//

CUSTOM_COMMAND_SIG(execute_arbitrary_command)
CUSTOM_DOC("Execute a 'long form' command.")
{
    // NOTE(allen): This isn't a super powerful version of this command, I will expand
    // upon it so that it has all the cmdid_* commands by default.  However, with this
    // as an example you have everything you need to make it work already. You could
    // even use app->memory to create a hash table in the start hook.
    Query_Bar bar = {0};
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
        load_project(app);
    }
    else if (match_ss(bar.string, make_lit_string("open all code"))){
        open_all_code(app);
    }
    else if (match_ss(bar.string, make_lit_string("open all code recursive"))){
        open_all_code_recursive(app);
    }
    else if(match_ss(bar.string, make_lit_string("close all code"))){
        close_all_code(app);
    }
    else if (match_ss(bar.string, make_lit_string("dos lines")) ||
             match_ss(bar.string, make_lit_string("dosify"))){
        eol_dosify(app);
    }
    else if (match_ss(bar.string, make_lit_string("nix lines")) ||
             match_ss(bar.string, make_lit_string("nixify"))){
        eol_nixify(app);
    }
    else if (match_ss(bar.string, make_lit_string("remap"))){
        remap_interactive(app);
    }
    else if (match_ss(bar.string, make_lit_string("new project"))){
        setup_new_project(app);
    }
    else if (match_ss(bar.string, make_lit_string("delete file"))){
        delete_file_query(app);
    }
    else if (match_ss(bar.string, make_lit_string("rename file"))){
        rename_file_query(app);
    }
    else if (match_ss(bar.string, make_lit_string("mkdir"))){
        make_directory_query(app);
    }
    else{
        print_message(app, literal("unrecognized command\n"));
    }
}

#include "4coder_remapping_commands.cpp"

#endif

// BOTTOM

