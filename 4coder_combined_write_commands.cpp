/*
4coder_combined_write_commands.cpp - Commands for writing text specialized for particular contexts.
*/

// TOP

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
write_named_comment_string(Application_Links *app, char *type_string){
    char space[512];
    String str = make_fixed_width_string(space);
    
    String name = global_config.user_name;
    if (name.size > 0){
        append(&str, "// ");
        append(&str, type_string);
        append(&str, "(");
        append(&str, name);
        append(&str, "): ");
    }
    else{
        append(&str, "// ");
        append(&str, type_string);
        append(&str, ": ");
    }
    
    write_string(app, str);
}

static void
long_braces(Application_Links *app, char *text, int32_t size){
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
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
CUSTOM_DOC("At the cursor, insert a ' = {};'.")
{
    write_string(app, make_lit_string(" = {};"));
}

static int32_t
get_start_of_line_at_cursor(Application_Links *app, View_Summary *view, Buffer_Summary *buffer){
    Full_Cursor cursor = {};
    view_compute_cursor(app, view, seek_line_char(view->cursor.line, 1), &cursor);
    Hard_Start_Result hard_start = buffer_find_hard_start(app, buffer, cursor.pos, DEF_TAB_WIDTH);
    return(hard_start.char_pos);
}

static bool32
c_line_comment_starts_at_position(Application_Links *app, Buffer_Summary *buffer, int32_t pos){
    bool32 alread_has_comment = false;
    char check_buffer[2];
    if (buffer_read_range(app, buffer, pos, pos + 2, check_buffer)){
        if (check_buffer[0] == '/' && check_buffer[1] == '/'){
            alread_has_comment = true;
        }
    }
    return(alread_has_comment);
}

CUSTOM_COMMAND_SIG(comment_line)
CUSTOM_DOC("Insert '//' at the beginning of the line after leading whitespace.")
{
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    int32_t pos = get_start_of_line_at_cursor(app, &view, &buffer);
    bool32 alread_has_comment = c_line_comment_starts_at_position(app, &buffer, pos);
    if (!alread_has_comment){
        buffer_replace_range(app, &buffer, pos, pos, "//", 2);
    }
}

CUSTOM_COMMAND_SIG(uncomment_line)
CUSTOM_DOC("If present, delete '//' at the beginning of the line after leading whitespace.")
{
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    int32_t pos = get_start_of_line_at_cursor(app, &view, &buffer);
    bool32 alread_has_comment = c_line_comment_starts_at_position(app, &buffer, pos);
    if (alread_has_comment){
        buffer_replace_range(app, &buffer, pos, pos + 2, 0, 0);
    }
}

CUSTOM_COMMAND_SIG(comment_line_toggle)
CUSTOM_DOC("Turns uncommented lines into commented lines and vice versa for comments starting with '//'.")
{
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    int32_t pos = get_start_of_line_at_cursor(app, &view, &buffer);
    bool32 alread_has_comment = c_line_comment_starts_at_position(app, &buffer, pos);
    if (alread_has_comment){
        buffer_replace_range(app, &buffer, pos, pos + 2, 0, 0);
    }
    else{
        buffer_replace_range(app, &buffer, pos, pos, "//", 2);
    }
}

////////////////////////////////

static Snippet default_snippets[] = {
    // general (for Allen's style)
    {"if",     "if (){\n\n}\n", 4, 7},
    {"ifelse", "if (){\n\n}\nelse{\n\n}", 4, 7},
    {"forn",   "for (;\nnode != 0;\nnode = node->next){\n\n}\n", 5, 38},
    {"fori",   "for (i = 0; i < ; i += 1){\n\n}\n", 5, 16},
    {"for",    "for (;;){\n\n}\n", 5, 10},
    {"case",   "case :\n{\n\n}break;\n", 5, 9},
    {"///",    "////////////////////////////////", 32, 32},
    {"#guard", "#if !defined(Z)\n#define Z\n#endif\n", 0, 26},
    {"space",  "char space[256];", 0, 14},
    
    {"op+",  "Z\noperator+(Z a, Z b){\n,\n}\n", 0, 23},
    {"op-",  "Z\noperator-(Z a, Z b){\n,\n}\n", 0, 23},
    {"op*",  "Z\noperator*(Z a, Z b){\n,\n}\n", 0, 23},
    {"op/",  "Z\noperator/(Z a, Z b){\n,\n}\n", 0, 23},
    {"op+=", "Z&\noperator+=(Z &a, Z b){\n,\n}\n", 0, 26},
    {"op-=", "Z&\noperator-=(Z &a, Z b){\n,\n}\n", 0, 26},
    {"op*=", "Z&\noperator*=(Z &a, Z b){\n,\n}\n", 0, 26},
    {"op/=", "Z&\noperator/=(Z &a, Z b){\n,\n}\n", 0, 26},
    
    // for 4coder development
    {"4command", "CUSTOM_COMMAND_SIG()\nCUSTOM_DOC()\n{\n\n}\n", 19, 32},
    {"4app", "Application_Links *app", 22, 22},
    
#if defined(SNIPPET_EXPANSION)
#include SNIPPET_EXPANSION
#endif
};

static void
activate_snippet(Application_Links *app, Partition *scratch, Heap *heap,
                 View_Summary *view, struct Lister_State *state,
                 String text_field, void *user_data, bool32 activated_by_mouse){
    int32_t index = (int32_t)PtrAsInt(user_data);
    Snippet_Array snippets = *(Snippet_Array*)state->lister.data.user_data;
    if (0 <= index && index < snippets.count){
        Snippet snippet = snippets.snippets[index];
        lister_default(app, scratch, heap, view, state, ListerActivation_Finished);
        Buffer_Summary buffer = get_buffer(app, view->buffer_id, AccessOpen);
        int32_t pos = view->cursor.pos;
        int32_t len = str_size(snippet.text);
        buffer_replace_range(app, &buffer, pos, pos, snippet.text, len);
        view_set_cursor(app, view, seek_pos(pos + snippet.cursor_offset), true);
        view_set_mark(app, view, seek_pos(pos + snippet.mark_offset));
    }
    else{
        lister_default(app, scratch, heap, view, state, ListerActivation_Finished);
    }
    
}

static void
snippet_lister__parameterized(Application_Links *app, Snippet_Array snippet_array){
    Partition *arena = &global_part;
    
    View_Summary view = get_active_view(app, AccessAll);
    view_end_ui_mode(app, &view);
    Temp_Memory temp = begin_temp_memory(arena);
    int32_t option_count = snippet_array.count;
    Lister_Option *options = push_array(arena, Lister_Option, option_count);
    for (int32_t i = 0; i < snippet_array.count; i += 1){
        options[i].string = make_string_slowly(snippet_array.snippets[i].name);
        options[i].status = make_string_slowly(snippet_array.snippets[i].text);
        options[i].user_data = IntAsPtr(i);
    }
    begin_integrated_lister__basic_list(app, "Snippet:", activate_snippet,
                                        &snippet_array, sizeof(snippet_array),
                                        options, option_count, 0, &view);
    end_temp_memory(temp);
}

CUSTOM_COMMAND_SIG(snippet_lister)
CUSTOM_DOC("Opens a snippet lister for inserting whole pre-written snippets of text.")
{
    Snippet_Array snippet_array = {};
    snippet_array.snippets = default_snippets;
    snippet_array.count = ArrayCount(default_snippets);
    snippet_lister__parameterized(app, snippet_array);
}

// BOTTOM

