/*
4coder_combined_write_commands.cpp - Commands for writing text specialized for particular contexts.
*/

// TOP

static void
write_string(Application_Links *app, View_ID view, Buffer_ID buffer, String_Const_u8 string){
    i64 pos = view_get_cursor_pos(app, view);
    buffer_replace_range(app, buffer, Ii64(pos), string);
    view_set_cursor_and_preferred_x(app, view, seek_pos(pos + string.size));
}

static void
write_string(Application_Links *app, String_Const_u8 string){
    View_ID view = get_active_view(app, AccessOpen);
    Buffer_ID buffer = view_get_buffer(app, view, AccessOpen);
    write_string(app, view, buffer, string);
}

static void
write_named_comment_string(Application_Links *app, char *type_string){
    Scratch_Block scratch(app);
    String_Const_u8 name = global_config.user_name;
    String_Const_u8 str = {};
    if (name.size > 0){
        str = push_u8_stringf(scratch, "// %s(%.*s): ", type_string, string_expand(name));
    }
    else{
        str = push_u8_stringf(scratch, "// %s: ", type_string);
    }
    write_string(app, str);
}

static void
long_braces(Application_Links *app, char *text, i32 size){
    View_ID view = get_active_view(app, AccessOpen);
    Buffer_ID buffer = view_get_buffer(app, view, AccessOpen);
    i64 pos = view_get_cursor_pos(app, view);
    buffer_replace_range(app, buffer, Ii64(pos), SCu8(text, size));
    view_set_cursor_and_preferred_x(app, view, seek_pos(pos + 2));
    buffer_auto_indent(app, buffer, pos, pos + size, DEF_TAB_WIDTH, DEFAULT_INDENT_FLAGS | AutoIndent_FullTokens);
    move_past_lead_whitespace(app, view, buffer);
}

CUSTOM_COMMAND_SIG(open_long_braces)
CUSTOM_DOC("At the cursor, insert a '{' and '}' separated by a blank line.")
{
    char text[] = "{\n\n}";
    i32 size = sizeof(text) - 1;
    long_braces(app, text, size);
}

CUSTOM_COMMAND_SIG(open_long_braces_semicolon)
CUSTOM_DOC("At the cursor, insert a '{' and '};' separated by a blank line.")
{
    char text[] = "{\n\n};";
    i32 size = sizeof(text) - 1;
    long_braces(app, text, size);
}

CUSTOM_COMMAND_SIG(open_long_braces_break)
CUSTOM_DOC("At the cursor, insert a '{' and '}break;' separated by a blank line.")
{
    char text[] = "{\n\n}break;";
    i32 size = sizeof(text) - 1;
    long_braces(app, text, size);
}

CUSTOM_COMMAND_SIG(if0_off)
CUSTOM_DOC("Surround the range between the cursor and mark with an '#if 0' and an '#endif'")
{
    place_begin_and_end_on_own_lines(app, "#if 0", "#endif");
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
    write_string(app, string_u8_litexpr("/*  */"));
}

CUSTOM_COMMAND_SIG(write_zero_struct)
CUSTOM_DOC("At the cursor, insert a ' = {};'.")
{
    write_string(app, string_u8_litexpr(" = {};"));
}

static i64
get_start_of_line_at_cursor(Application_Links *app, View_ID view, Buffer_ID buffer){
    i64 pos = view_get_cursor_pos(app, view);
    i64 line = get_line_number_from_pos(app, buffer, pos);
    return(get_pos_past_lead_whitespace_from_line_number(app, buffer, line));
}

static b32
c_line_comment_starts_at_position(Application_Links *app, Buffer_ID buffer, i64 pos){
    b32 alread_has_comment = false;
    char check_buffer[2];
    if (buffer_read_range(app, buffer, Ii64(pos, pos + 2), check_buffer)){
        if (check_buffer[0] == '/' && check_buffer[1] == '/'){
            alread_has_comment = true;
        }
    }
    return(alread_has_comment);
}

CUSTOM_COMMAND_SIG(comment_line)
CUSTOM_DOC("Insert '//' at the beginning of the line after leading whitespace.")
{
    View_ID view = get_active_view(app, AccessOpen);
    Buffer_ID buffer = view_get_buffer(app, view, AccessOpen);
    i64 pos = get_start_of_line_at_cursor(app, view, buffer);
    b32 alread_has_comment = c_line_comment_starts_at_position(app, buffer, pos);
    if (!alread_has_comment){
        buffer_replace_range(app, buffer, Ii64(pos), string_u8_litexpr("//"));
    }
}

CUSTOM_COMMAND_SIG(uncomment_line)
CUSTOM_DOC("If present, delete '//' at the beginning of the line after leading whitespace.")
{
    View_ID view = get_active_view(app, AccessOpen);
    Buffer_ID buffer = view_get_buffer(app, view, AccessOpen);
    i64 pos = get_start_of_line_at_cursor(app, view, buffer);
    b32 alread_has_comment = c_line_comment_starts_at_position(app, buffer, pos);
    if (alread_has_comment){
        buffer_replace_range(app, buffer, Ii64(pos, pos + 2), string_u8_empty);
    }
}

CUSTOM_COMMAND_SIG(comment_line_toggle)
CUSTOM_DOC("Turns uncommented lines into commented lines and vice versa for comments starting with '//'.")
{
    View_ID view = get_active_view(app, AccessOpen);
    Buffer_ID buffer = view_get_buffer(app, view, AccessOpen);
    i64 pos = get_start_of_line_at_cursor(app, view, buffer);
    b32 alread_has_comment = c_line_comment_starts_at_position(app, buffer, pos);
    if (alread_has_comment){
        buffer_replace_range(app, buffer, Ii64(pos, pos + 2), string_u8_empty);
    }
    else{
        buffer_replace_range(app, buffer, Ii64(pos), string_u8_litexpr("//"));
    }
}

////////////////////////////////

static Snippet default_snippets[] = {
    // general (for Allen's style)
    {"if",     "if (){\n\n}\n", 4, 7},
    {"ifelse", "if (){\n\n}\nelse{\n\n}", 4, 7},
    {"forn",   "for (node = ;\nnode != 0;\nnode = node->next){\n\n}\n", 5, 38},
    {"fori",   "for (i = 0; i < ; i += 1){\n\n}\n", 5, 16},
    {"forj",   "for (j = 0; j < ; j += 1){\n\n}\n", 5, 16},
    {"fork",   "for (k = 0; k < ; k += 1){\n\n}\n", 5, 16},
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
activate_snippet(Application_Links *app, Heap *heap, View_ID view, struct Lister_State *state, String_Const_u8 text_field, void *user_data, b32 activated_by_mouse){
    i32 index = (i32)PtrAsInt(user_data);
    Snippet_Array snippets = *(Snippet_Array*)state->lister.data.user_data;
    if (0 <= index && index < snippets.count){
        Snippet snippet = snippets.snippets[index];
        lister_default(app, heap, view, state, ListerActivation_Finished);
        Buffer_ID buffer = view_get_buffer(app, view, AccessOpen);
        i64 pos = view_get_cursor_pos(app, view);
        buffer_replace_range(app, buffer, Ii64(pos), SCu8(snippet.text));
        view_set_cursor_and_preferred_x(app, view, seek_pos(pos + snippet.cursor_offset));
        view_set_mark(app, view, seek_pos(pos + snippet.mark_offset));
    }
    else{
        lister_default(app, heap, view, state, ListerActivation_Finished);
    }
    
}

static void
snippet_lister__parameterized(Application_Links *app, Snippet_Array snippet_array){
    
    View_ID view = get_active_view(app, AccessAll);
    view_end_ui_mode(app, view);
    Arena *scratch = context_get_arena(app);
    Temp_Memory temp = begin_temp(scratch);
    i32 option_count = snippet_array.count;
    Lister_Option *options = push_array(scratch, Lister_Option, option_count);
    for (i32 i = 0; i < snippet_array.count; i += 1){
        options[i].string = SCu8(snippet_array.snippets[i].name);
        options[i].status = SCu8(snippet_array.snippets[i].text);
        options[i].user_data = IntAsPtr(i);
    }
    begin_integrated_lister__basic_list(app, "Snippet:", activate_snippet, &snippet_array, sizeof(snippet_array), options, option_count, 0, view);
    end_temp(temp);
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

