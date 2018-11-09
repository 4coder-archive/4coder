/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 25.02.2016
 *
 * File editing view for 4coder
 *
 */

// TOP

#define KEYCODES_FILE "4coder_generated/keycodes.h"
#define STYLE_FILE "4coder_generated/style.h"
#define API_H "4coder_generated/app_functions.h"
#define REMAPPING_FILE "4coder_generated/remapping.h"

#include "../4ed_defines.h"
#include "4ed_meta_defines.h"
#include "../4coder_API/version.h"

#define FSTRING_IMPLEMENTATION
#include "../4coder_lib/4coder_string.h"

#include "../4coder_lib/4cpp_lexer.h"
#include "../4ed_linked_node_macros.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define FTECH_FILE_MOVING_IMPLEMENTATION
#include "4ed_file_moving.h"
#include "4ed_meta_parser.cpp"
#include "4ed_meta_keywords.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
#define KEY_LIST(M)\
M(back) \
M(up) \
M(down) \
M(left) \
M(right) \
M(del) \
M(insert) \
M(home) \
M(end) \
M(page_up) \
M(page_down) \
M(esc) \
M(mouse_left) \
M(mouse_right) \
M(mouse_left_release) \
M(mouse_right_release) \
M(mouse_wheel) \
M(mouse_move) \
M(animate) \
M(view_activate) \
M(f1) \
M(f2) \
M(f3) \
M(f4) \
M(f5) \
M(f6) \
M(f7) \
M(f8) \
M(f9) \
M(f10) \
M(f11) \
M(f12) \
M(f13) \
M(f14) \
M(f15) \
M(f16)


enum{
    key_enum_kicker_offer = 0xD800 - 1,
#define DefKeyEnum(n) key_##n,
    KEY_LIST(DefKeyEnum)
#undef DefKeyEnum
};

internal void
generate_keycode_enum(){
    Temp temp = fm_begin_temp();
    
    char *filename_keycodes = KEYCODES_FILE;
    
    String out = str_alloc(10 << 20);
    
    append(&out, "enum{\n");
#define DefKeyEnum(n) append(&out, "key_" #n " = "); append_int_to_str(&out, key_##n); append(&out, ",\n");
    KEY_LIST(DefKeyEnum);
#undef DefKeyEnum
    append(&out, "};\n");
    
    append(&out,
           "static char*\n"
           "global_key_name(uint32_t key_code, int32_t *size){\n"
           "char *result = 0;\n"
           "switch(key_code){\n");
    
#define KeyCase(n) append(&out, "case key_" #n ": result = \"key_" #n "\"; *size = sizeof(\"key_" #n "\")-1; break;\n");
    KEY_LIST(KeyCase);
#undef KeyCase
    
    append(&out,
           "}\n"
           "return(result);\n"
           "}\n");
    
    fm_write_file(filename_keycodes, out.str, out.size);
    out.size = 0;
    
    fm_end_temp(temp);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
internal void
struct_begin(String *str, char *name){
    append(str, "struct ");
    append(str, name);
    append(str, "{\n");
}

internal void
enum_begin(String *str, char *name){
    append(str, "enum ");
    append(str, name);
    append(str, "{\n");
}

internal void
struct_end(String *str){
    append(str, "};\n\n");
}

static char* bar_style_fields[] = {
    "bar",
    "bar_active",
    "base",
    "pop1",
    "pop2",
};

static char* main_style_fields[] = {
    "back",
    "margin",
    "margin_hover",
    "margin_active",
    "list_item",
    "list_item_hover",
    "list_item_active",
    "cursor",
    "at_cursor",
    "highlight_cursor_line",
    "highlight",
    "at_highlight",
    "mark",
    "default",
    "comment",
    "keyword",
    "str_constant",
    "char_constant",
    "int_constant",
    "float_constant",
    "bool_constant",
    "preproc",
    "include",
    "special_character",
    "ghost_character",
    "highlight_junk",
    "highlight_white",
    "paste",
    "undo",
    "next_undo",
    "back_cycle_1",
    "back_cycle_2",
    "back_cycle_3",
    "back_cycle_4",
    "text_cycle_1",
    "text_cycle_2",
    "text_cycle_3",
    "text_cycle_4",
};

internal char*
make_style_tag(char *tag){
    i32 len = (i32)strlen(tag);
    char *str = fm_push_array(char, len + 1);
    to_camel(tag, str);
    str[len] = 0;
    return(str);
}

internal void
generate_style(){
    Temp temp = fm_begin_temp();
    
    char filename_4coder[] = STYLE_FILE;
    char filename_4ed[] = "4ed_generated_style.h";
    
    String out = str_alloc(10 << 20);
    
    enum_begin(&out, "Style_Tag");
    {
        i32 count = ArrayCount(bar_style_fields);
        for (i32 i = 0; i < count; ++i){
            char *tag = make_style_tag(bar_style_fields[i]);
            append(&out, "Stag_");
            append(&out, tag);
            append(&out, ",\n");
        }
        
        count = ArrayCount(main_style_fields);
        for (i32 i = 0; i < count; ++i){
            char *tag = make_style_tag(main_style_fields[i]);
            append(&out, "Stag_");
            append(&out, tag);
            append(&out, ",\n");
        }
        
        append(&out, "Stag_COUNT\n");
    }
    struct_end(&out);
    
    append(&out, "static char *style_tag_names[] = {\n");
    {
        i32 count = ArrayCount(bar_style_fields);
        for (i32 i = 0; i < count; ++i){
            char *tag = make_style_tag(bar_style_fields[i]);
            append(&out, "\"");
            append(&out, tag);
            append(&out, "\",\n");
        }
        
        count = ArrayCount(main_style_fields);
        for (i32 i = 0; i < count; ++i){
            char *tag = make_style_tag(main_style_fields[i]);
            append(&out, "\"");
            append(&out, tag);
            append(&out, "\",\n");
        }
    }
    append(&out, "};\n");
    
    fm_write_file(filename_4coder, out.str, out.size);
    out.size = 0;
    
    struct_begin(&out, "Interactive_Style");
    {
        i32 count = ArrayCount(bar_style_fields);
        for (i32 i = 0; i < count; ++i){
            append(&out, "u32 ");
            append(&out, bar_style_fields[i]);
            append(&out, "_color;\n");
        }
    }
    struct_end(&out);
    
    struct_begin(&out, "Style_Main_Data");
    {
        i32 count = ArrayCount(main_style_fields);
        for (i32 i = 0; i < count; ++i){
            append(&out, "u32 ");
            append(&out, main_style_fields[i]);
            append(&out, "_color;\n");
        }
        append(&out, "Interactive_Style file_info_style;\n");
    }
    struct_end(&out);
    
    {
        append(&out,
               "inline u32*\n"
               "style_index_by_tag(Style_Main_Data *s, u32 tag){\n"
               "u32 *result = 0;\n"
               "switch (tag){\n");
        
        i32 count = ArrayCount(bar_style_fields);
        for (i32 i = 0; i < count; ++i){
            char *tag = make_style_tag(bar_style_fields[i]);
            append(&out, "case Stag_");
            append(&out, tag);
            append(&out, ": result = &s->file_info_style.");
            append(&out, bar_style_fields[i]);
            append(&out, "_color; break;\n");
        }
        
        count = ArrayCount(main_style_fields);
        for (i32 i = 0; i < count; ++i){
            char *tag = make_style_tag(main_style_fields[i]);
            append(&out, "case Stag_");
            append(&out, tag);
            append(&out, ": result = &s->");
            append(&out, main_style_fields[i]);
            append(&out, "_color; break;\n");
        }
        
        append(&out,
               "}\n"
               "return(result);\n"
               "}\n\n");
    }
    
    fm_write_file(filename_4ed, out.str, out.size);
    out.size = 0;
    
    fm_end_temp(temp);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

//
// Meta Parse Rules
//

struct App_API_Name{
    String macro;
    String public_name;
};

struct App_API{
    App_API_Name *names;
};

internal App_API
allocate_app_api(i32 count){
    App_API app_api = {0};
    app_api.names = fm_push_array(App_API_Name, count);
    memset(app_api.names, 0, sizeof(App_API_Name)*count);
    return(app_api);
}

internal void
generate_custom_headers(){
    Temp temp = fm_begin_temp();
    
    // NOTE(allen): Parse the customization API files
    static char *functions_files[] = { "4ed_api_implementation.cpp", 0 };
    Meta_Unit unit_custom = compile_meta_unit(".", functions_files, ExpandArray(meta_keywords));
    if (unit_custom.parse == 0){
        Assert(!"Missing one or more input files!");
    }
    
    // NOTE(allen): Compute and store variations of the function names
    App_API func_4ed_names = allocate_app_api(unit_custom.set.count);
    
    for (i32 i = 0; i < unit_custom.set.count; ++i){
        String name_string = unit_custom.set.items[i].name;
        String *macro = &func_4ed_names.names[i].macro;
        String *public_name = &func_4ed_names.names[i].public_name;
        
        *macro = str_alloc(name_string.size+4);
        to_upper(macro, name_string);
        append(macro, make_lit_string("_SIG"));
        
        *public_name = str_alloc(name_string.size);
        to_lower(public_name, name_string);
        
        fm_align();
    }
    
    // NOTE(allen): Output
    String out = str_alloc(10 << 20);
    
    // NOTE(allen): Custom API headers
    append(&out, "struct Application_Links;\n");
    
    for (i32 i = 0; i < unit_custom.set.count; ++i){
        append(&out, "#define ");
        append(&out, func_4ed_names.names[i].macro);
        append(&out, "(n) ");
        append(&out, unit_custom.set.items[i].ret);
        append(&out, " n");
        append(&out, unit_custom.set.items[i].args);
        append_s_char(&out, '\n');
    }
    
    for (i32 i = 0; i < unit_custom.set.count; ++i){
        append(&out, "typedef ");
        append(&out, func_4ed_names.names[i].macro);
        append_s_char(&out, '(');
        append(&out, unit_custom.set.items[i].name);
        append(&out, "_Function);\n");
    }
    
    append(&out, "struct Application_Links{\n");
    
    append(&out, "#if defined(ALLOW_DEP_4CODER)\n");
    for (i32 i = 0; i < unit_custom.set.count; ++i){
        append(&out, unit_custom.set.items[i].name);
        append(&out, "_Function *");
        append(&out, func_4ed_names.names[i].public_name);
        append(&out, ";\n");
    }
    
    append(&out, "#else\n");
    
    for (i32 i = 0; i < unit_custom.set.count; ++i){
        append(&out, unit_custom.set.items[i].name);
        append(&out, "_Function *");
        append(&out, func_4ed_names.names[i].public_name);
        append(&out, "_;\n");
    }
    append(&out, "#endif\n");
    
    append(&out,
           "void *memory;\n"
           "int32_t memory_size;\n"
           "void *cmd_context;\n"
           "void *system_links;\n"
           "void *current_coroutine;\n"
           "int32_t type_coroutine;\n"
           "};\n");
    
    append(&out, "#define FillAppLinksAPI(app_links) do{");
    for (i32 i = 0; i < unit_custom.set.count; ++i){
        append(&out, "\\\napp_links->");
        append(&out, func_4ed_names.names[i].public_name);
        append(&out, "_ = ");
        append(&out, unit_custom.set.items[i].name);
        append_s_char(&out, ';');
    }
    append(&out, "} while(false)\n");
    
    append(&out, "#if defined(ALLOW_DEP_4CODER)\n");
    for (i32 use_dep = 1; use_dep >= 0; --use_dep){
        for (i32 i = 0; i < unit_custom.set.count; ++i){
            Argument_Breakdown breakdown = unit_custom.set.items[i].breakdown;
            String ret = unit_custom.set.items[i].ret;
            String public_name = func_4ed_names.names[i].public_name;
            
            append(&out, "static inline ");
            append(&out, ret);
            append(&out, " ");
            append(&out, public_name);
            
            append(&out, "(");
            for (i32 j = 0; j < breakdown.count; ++j){
                append(&out, breakdown.args[j].param_string);
                if (j+1 != breakdown.count){
                    append(&out, ", ");
                }
            }
            append(&out, "){");
            
            if (match(ret, make_lit_string("void"))){
                append(&out, "(");
            }
            else{
                append(&out, "return(");
            }
            
            append(&out, "app->");
            append(&out, public_name);
            if (!use_dep){
                append(&out, "_");
            }
            
            append(&out, "(");
            for (i32 j = 0; j < breakdown.count; ++j){
                append(&out, breakdown.args[j].param_name);
                if (j+1 != breakdown.count){
                    append(&out, ", ");
                }
            }
            append(&out, ")");
            
            append(&out, ");}\n");
        }
        if (use_dep == 1){
            append(&out, "#else\n");
        }
    }
    append(&out, "#endif\n");
    
    fm_write_file(API_H, out.str, out.size);
    out.size = 0;
    
    fm_end_temp(temp);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

struct Key_Bind{
    Key_Bind *next;
    
    b32 vanilla;
    u32 keycode;
    u32 modifiers;
    
    char *command;
    i32 command_len;
};

struct Sub_Map{
    Sub_Map *next;
    
    char *name;
    i32 name_len;
    char *description;
    i32 description_len;
    char *parent;
    i32 parent_len;
    b32 has_vanilla;
    Key_Bind *first_key_bind;
    Key_Bind *last_key_bind;
    i32 key_bind_count;
};

struct Mapping{
    Mapping *next;
    
    char *name;
    i32 name_len;
    char *description;
    i32 description_len;
    Sub_Map *first_sub_map;
    Sub_Map *last_sub_map;
    i32 sub_map_count;
};

struct Mapping_Array{
    Mapping *first_mapping;
    Mapping *last_mapping;
    i32 mapping_count;
    
    Mapping *current_mapping;
    Sub_Map *current_sub_map;
};

enum{
    MDFR_NONE = 0x0,
    MDFR_CTRL  = 0x1,
    MDFR_ALT   = 0x2,
    MDFR_CMND  = 0x4,
    MDFR_SHIFT = 0x8,
};

//////////////////////////////////////////////////////////////////////////////////////////////////

internal void
emit_begin_mapping(Mapping_Array *array, char *name, char *description){
    Assert(array->current_mapping == 0);
    
    Mapping *mapping = fm_push_array(Mapping, 1);
    mapping->name = fm_basic_str(name);
    mapping->name_len = str_size(name);
    mapping->description = fm_basic_str(description);
    mapping->description_len = str_size(description);
    mapping->first_sub_map = 0;
    mapping->last_sub_map = 0;
    mapping->sub_map_count = 0;
    sll_push(array->first_mapping, array->last_mapping, mapping);
    ++array->mapping_count;
    array->current_mapping = mapping;
}

internal void
emit_end_mapping(Mapping_Array *array){
    Assert(array->current_mapping != 0);
    array->current_mapping = 0;
}

internal void
emit_begin_map(Mapping_Array *array, char *mapid, char *description){
    Assert(array->current_mapping != 0);
    Assert(array->current_sub_map == 0);
    
    Sub_Map *sub_map = fm_push_array(Sub_Map, 1);
    sub_map->name = fm_basic_str(mapid);
    sub_map->name_len = str_size(mapid);
    sub_map->description = fm_basic_str(description);
    sub_map->description_len = str_size(description);
    sub_map->parent = 0;
    sub_map->parent_len = 0;
    sub_map->first_key_bind = 0;
    sub_map->last_key_bind = 0;
    sub_map->key_bind_count = 0;
    
    Mapping *mapping = array->current_mapping;
    sll_push(mapping->first_sub_map, mapping->last_sub_map, sub_map);
    ++mapping->sub_map_count;
    
    array->current_sub_map = sub_map;
}

internal void
emit_end_map(Mapping_Array *array){
    Assert(array->current_mapping != 0);
    Assert(array->current_sub_map != 0);
    array->current_sub_map = 0;
}

internal void
emit_inherit_map(Mapping_Array *array, char *mapid){
    Assert(array->current_mapping != 0);
    Assert(array->current_sub_map != 0);
    
    Sub_Map *sub_map = array->current_sub_map;
    Assert(sub_map->parent == 0);
    
    sub_map->parent = fm_basic_str(mapid);
    sub_map->parent_len = str_size(mapid);
}

internal void
emit_bind(Mapping_Array *array, u32 keycode, u32 modifiers, char *command){
    Assert(array->current_mapping != 0);
    Assert(array->current_sub_map != 0);
    
    b32 is_duplicate = false;
    Sub_Map *sub_map = array->current_sub_map;
    for (Key_Bind *bind = sub_map->first_key_bind;
         bind != 0;
         bind = bind->next){
        if (!bind->vanilla && keycode == bind->keycode && modifiers == bind->modifiers){
            fprintf(stdout, "duplicate binding for %u %u\n", keycode, modifiers);
            is_duplicate = true;
            break;
        }
    }
    
    if (!is_duplicate){
        Key_Bind *bind = fm_push_array(Key_Bind, 1);
        bind->vanilla = false;
        bind->keycode = keycode;
        bind->modifiers = modifiers;
        bind->command = fm_basic_str(command);
        bind->command_len = str_size(command);
        sll_push(sub_map->first_key_bind, sub_map->last_key_bind, bind);
        ++sub_map->key_bind_count;
    }
}

internal void
emit_bind_vanilla_keys(Mapping_Array *array, u32 modifiers, char *command){
    Assert(array->current_mapping != 0);
    Assert(array->current_sub_map != 0);
    
    b32 is_duplicate = false;
    Sub_Map *sub_map = array->current_sub_map;
    for (Key_Bind *bind = sub_map->first_key_bind;
         bind != 0;
         bind = bind->next){
        if (bind->vanilla && modifiers == bind->modifiers){
            fprintf(stdout, "duplicate vanilla binding %u\n", modifiers);
            is_duplicate = true;
            break;
        }
    }
    
    if (!is_duplicate){
        Key_Bind *bind = fm_push_array(Key_Bind, 1);
        bind->vanilla = true;
        bind->keycode = 0;
        bind->modifiers = modifiers;
        bind->command = fm_basic_str(command);
        bind->command_len = str_size(command);
        sll_push(sub_map->first_key_bind, sub_map->last_key_bind, bind);
        ++sub_map->key_bind_count;
    }
}

#define begin_mapping(mp,n,d) emit_begin_mapping(mp, #n, d)
#define end_mapping(mp)       emit_end_mapping(mp)
#define begin_map(mp,mapid,d) emit_begin_map(mp, #mapid, d)
#define end_map(mp)           emit_end_map(mp)
#define inherit_map(mp,mapid)      emit_inherit_map(mp, #mapid)
#define bind(mp,k,md,c)            emit_bind(mp, k, md, #c)
#define bind_vanilla_keys(mp,md,c) emit_bind_vanilla_keys(mp, md, #c)

//////////////////////////////////////////////////////////////////////////////////////////////////

internal void
generate_remapping_code_and_data(){
    Temp temp = fm_begin_temp();
    
    // Generate mapping array data structure
    Mapping_Array mappings_ = {0};
    Mapping_Array *mappings = &mappings_;
    
    begin_mapping(mappings, default, "The default 4coder bindings - typically good for Windows and Linux");
    {
        // NOTE(allen): GLOBAL
        begin_map(mappings, mapid_global, "The following bindings apply in all situations.");
        
        bind(mappings, ',', MDFR_CTRL, change_active_panel);
        bind(mappings, '<', MDFR_CTRL, change_active_panel_backwards);
        
        bind(mappings, 'n', MDFR_CTRL, interactive_new);
        bind(mappings, 'o', MDFR_CTRL, interactive_open_or_new);
        bind(mappings, 'o', MDFR_ALT, open_in_other);
        bind(mappings, 'k', MDFR_CTRL, interactive_kill_buffer);
        bind(mappings, 'i', MDFR_CTRL, interactive_switch_buffer);
        bind(mappings, 'h', MDFR_CTRL, project_go_to_root_directory);
        bind(mappings, 'S', MDFR_CTRL, save_all_dirty_buffers);
        
        bind(mappings, '.', MDFR_ALT, change_to_build_panel);
        bind(mappings, ',', MDFR_ALT, close_build_panel);
        bind(mappings, 'n', MDFR_ALT, goto_next_jump_no_skips_sticky);
        bind(mappings, 'N', MDFR_ALT, goto_prev_jump_no_skips_sticky);
        bind(mappings, 'M', MDFR_ALT, goto_first_jump_sticky);
        bind(mappings, 'm', MDFR_ALT, build_in_build_panel);
        bind(mappings, 'b', MDFR_ALT, toggle_filebar);
        
        bind(mappings, 'z', MDFR_ALT, execute_any_cli);
        bind(mappings, 'Z', MDFR_ALT, execute_previous_cli);
        
        bind(mappings, 'x', MDFR_ALT, command_lister);
        bind(mappings, 'X', MDFR_ALT, project_command_lister);
        
        bind(mappings, 'I', MDFR_CTRL, list_all_functions_current_buffer_lister);
        bind(mappings, 'E', MDFR_ALT, exit_4coder);
        
        bind(mappings, key_f1, MDFR_NONE, project_fkey_command);
        bind(mappings, key_f2, MDFR_NONE, project_fkey_command);
        bind(mappings, key_f3, MDFR_NONE, project_fkey_command);
        bind(mappings, key_f4, MDFR_NONE, project_fkey_command);
        
        bind(mappings, key_f5, MDFR_NONE, project_fkey_command);
        bind(mappings, key_f6, MDFR_NONE, project_fkey_command);
        bind(mappings, key_f7, MDFR_NONE, project_fkey_command);
        bind(mappings, key_f8, MDFR_NONE, project_fkey_command);
        
        bind(mappings, key_f9, MDFR_NONE, project_fkey_command);
        bind(mappings, key_f10, MDFR_NONE, project_fkey_command);
        bind(mappings, key_f11, MDFR_NONE, project_fkey_command);
        bind(mappings, key_f12, MDFR_NONE, project_fkey_command);
        
        bind(mappings, key_f13, MDFR_NONE, project_fkey_command);
        bind(mappings, key_f14, MDFR_NONE, project_fkey_command);
        bind(mappings, key_f15, MDFR_NONE, project_fkey_command);
        bind(mappings, key_f16, MDFR_NONE, project_fkey_command);
        
        bind(mappings, key_mouse_wheel, MDFR_NONE, mouse_wheel_scroll);
        
        end_map(mappings);
        
        // NOTE(allen): FILE
        begin_map(mappings, mapid_file, "The following bindings apply in general text files and most apply in code files, but some are overriden by other commands specific to code files.");
        
        bind_vanilla_keys(mappings, MDFR_NONE, write_character);
        
        bind(mappings, key_mouse_left, MDFR_NONE, click_set_cursor_and_mark);
        bind(mappings, key_view_activate, MDFR_NONE, click_set_cursor_and_mark);
        bind(mappings, key_mouse_left_release, MDFR_NONE, click_set_cursor);
        bind(mappings, key_mouse_move, MDFR_NONE, click_set_cursor_if_lbutton);
        
        bind(mappings, key_del,  MDFR_NONE, delete_char);
        bind(mappings, key_del,  MDFR_SHIFT, delete_char);
        bind(mappings, key_back, MDFR_NONE, backspace_char);
        bind(mappings, key_back, MDFR_SHIFT, backspace_char);
        
        bind(mappings, key_up,    MDFR_NONE, move_up);
        bind(mappings, key_down,  MDFR_NONE, move_down);
        bind(mappings, key_left,  MDFR_NONE, move_left);
        bind(mappings, key_right, MDFR_NONE, move_right);
        bind(mappings, key_up,    MDFR_SHIFT, move_up);
        bind(mappings, key_down,  MDFR_SHIFT, move_down);
        bind(mappings, key_left,  MDFR_SHIFT, move_left);
        bind(mappings, key_right, MDFR_SHIFT, move_right);
        
        bind(mappings, key_end,       MDFR_NONE, seek_end_of_line);
        bind(mappings, key_home,      MDFR_NONE, seek_beginning_of_line);
        bind(mappings, key_page_up,   MDFR_CTRL, goto_beginning_of_file);
        bind(mappings, key_page_down, MDFR_CTRL, goto_end_of_file);
        bind(mappings, key_page_up,   MDFR_NONE, page_up);
        bind(mappings, key_page_down, MDFR_NONE, page_down);
        bind(mappings, key_end,       MDFR_SHIFT, seek_end_of_line);
        bind(mappings, key_home,      MDFR_SHIFT, seek_beginning_of_line);
        bind(mappings, key_page_up,   MDFR_CTRL|MDFR_SHIFT, goto_beginning_of_file);
        bind(mappings, key_page_down, MDFR_CTRL|MDFR_SHIFT, goto_end_of_file);
        bind(mappings, key_page_up,   MDFR_SHIFT, page_up);
        bind(mappings, key_page_down, MDFR_SHIFT, page_down);
        
        bind(mappings, key_up,    MDFR_CTRL, seek_whitespace_up_end_line);
        bind(mappings, key_down,  MDFR_CTRL, seek_whitespace_down_end_line);
        bind(mappings, key_right, MDFR_CTRL, seek_whitespace_right);
        bind(mappings, key_left,  MDFR_CTRL, seek_whitespace_left);
        bind(mappings, key_up,    MDFR_CTRL|MDFR_SHIFT, seek_whitespace_up_end_line);
        bind(mappings, key_down,  MDFR_CTRL|MDFR_SHIFT, seek_whitespace_down_end_line);
        bind(mappings, key_right, MDFR_CTRL|MDFR_SHIFT, seek_whitespace_right);
        bind(mappings, key_left,  MDFR_CTRL|MDFR_SHIFT, seek_whitespace_left);
        
        bind(mappings, key_up,   MDFR_ALT, move_line_up);
        bind(mappings, key_down, MDFR_ALT, move_line_down);
        
        bind(mappings, key_back, MDFR_CTRL, backspace_word);
        bind(mappings, key_del,  MDFR_CTRL, delete_word);
        bind(mappings, key_back, MDFR_ALT, snipe_token_or_word);
        bind(mappings, key_del,  MDFR_ALT, snipe_token_or_word_right);
        
        bind(mappings, ' ', MDFR_CTRL, set_mark);
        bind(mappings, 'a', MDFR_CTRL, replace_in_range);
        bind(mappings, 'c', MDFR_CTRL, copy);
        bind(mappings, 'd', MDFR_CTRL, delete_range);
        bind(mappings, 'D', MDFR_CTRL, delete_line);
        bind(mappings, 'e', MDFR_CTRL, center_view);
        bind(mappings, 'E', MDFR_CTRL, left_adjust_view);
        bind(mappings, 'f', MDFR_CTRL, search);
        bind(mappings, 'F', MDFR_CTRL, list_all_locations);
        bind(mappings, 'F', MDFR_ALT , list_all_substring_locations_case_insensitive);
        bind(mappings, 'g', MDFR_CTRL, goto_line);
        bind(mappings, 'G', MDFR_CTRL, list_all_locations_of_selection);
        bind(mappings, 'j', MDFR_CTRL, snippet_lister);
        bind(mappings, 'K', MDFR_CTRL, kill_buffer);
        bind(mappings, 'L', MDFR_CTRL, duplicate_line);
        bind(mappings, 'm', MDFR_CTRL, cursor_mark_swap);
        bind(mappings, 'O', MDFR_CTRL, reopen);
        bind(mappings, 'q', MDFR_CTRL, query_replace);
        bind(mappings, 'Q', MDFR_CTRL, query_replace_identifier);
        bind(mappings, 'q', MDFR_ALT , query_replace_selection);
        bind(mappings, 'r', MDFR_CTRL, reverse_search);
        bind(mappings, 's', MDFR_CTRL, save);
        bind(mappings, 't', MDFR_CTRL, search_identifier);
        bind(mappings, 'T', MDFR_CTRL, list_all_locations_of_identifier);
        bind(mappings, 'v', MDFR_CTRL, paste_and_indent);
        bind(mappings, 'V', MDFR_CTRL, paste_next_and_indent);
        bind(mappings, 'x', MDFR_CTRL, cut);
        bind(mappings, 'y', MDFR_CTRL, redo);
        bind(mappings, 'z', MDFR_CTRL, undo);
        
        bind(mappings, '1', MDFR_CTRL, view_buffer_other_panel);
        bind(mappings, '2', MDFR_CTRL, swap_buffers_between_panels);
        
        bind(mappings, '\n', MDFR_NONE, newline_or_goto_position_sticky);
        bind(mappings, '\n', MDFR_SHIFT, newline_or_goto_position_same_panel_sticky);
        bind(mappings, '>', MDFR_CTRL, view_jump_list_with_lister);
        bind(mappings, ' ', MDFR_SHIFT, write_character);
        
        end_map(mappings);
        
        // NOTE(allen): CODE
        begin_map(mappings, default_code_map, "The following commands only apply in files where the lexer (syntax highlighting) is turned on.");
        
        inherit_map(mappings, mapid_file);
        
        bind(mappings, key_right, MDFR_CTRL, seek_alphanumeric_or_camel_right);
        bind(mappings, key_left, MDFR_CTRL, seek_alphanumeric_or_camel_left);
        
        bind(mappings, '\n', MDFR_NONE, write_and_auto_tab);
        bind(mappings, '\n', MDFR_SHIFT, write_and_auto_tab);
        bind(mappings, '}', MDFR_NONE, write_and_auto_tab);
        bind(mappings, ')', MDFR_NONE, write_and_auto_tab);
        bind(mappings, ']', MDFR_NONE, write_and_auto_tab);
        bind(mappings, ';', MDFR_NONE, write_and_auto_tab);
        bind(mappings, '#', MDFR_NONE, write_and_auto_tab);
        
        bind(mappings, '\t', MDFR_NONE, word_complete);
        bind(mappings, '\t', MDFR_CTRL, auto_tab_range);
        bind(mappings, '\t', MDFR_SHIFT, auto_tab_line_at_cursor);
        
        bind(mappings, 'h', MDFR_ALT, write_hack);
        bind(mappings, 'r', MDFR_ALT, write_block);
        bind(mappings, 't', MDFR_ALT, write_todo);
        bind(mappings, 'y', MDFR_ALT, write_note);
        
        bind(mappings, 'D', MDFR_ALT, list_all_locations_of_type_definition);
        bind(mappings, 'T', MDFR_ALT, list_all_locations_of_type_definition_of_identifier);
        
        bind(mappings, '[', MDFR_CTRL, open_long_braces);
        bind(mappings, '{', MDFR_CTRL, open_long_braces_semicolon);
        bind(mappings, '}', MDFR_CTRL, open_long_braces_break);
        
        bind(mappings, '[', MDFR_ALT, select_surrounding_scope);
        bind(mappings, ']', MDFR_ALT, select_prev_scope_absolute);
        bind(mappings, '\'', MDFR_ALT, select_next_scope_absolute);
        bind(mappings, '/', MDFR_ALT, place_in_scope);
        bind(mappings, '-', MDFR_ALT, delete_current_scope);
        bind(mappings, 'j', MDFR_ALT, scope_absorb_down);
        
        bind(mappings, 'i', MDFR_ALT, if0_off);
        
        bind(mappings, '1', MDFR_ALT, open_file_in_quotes);
        bind(mappings, '2', MDFR_ALT, open_matching_file_cpp);
        bind(mappings, '0', MDFR_CTRL, write_zero_struct);
        
        end_map(mappings);
        
        // NOTE(allen): LISTER
        begin_map(mappings, default_lister_ui_map,
                  "These commands apply in 'lister mode' such as when you open a file.");
        
        bind_vanilla_keys(mappings, MDFR_NONE, lister__write_character);
        bind(mappings, key_esc, MDFR_NONE, lister__quit);
        bind(mappings, '\n', MDFR_NONE, lister__activate);
        bind(mappings, '\t', MDFR_NONE, lister__activate);
        bind(mappings, key_back     , MDFR_NONE, lister__backspace_text_field);
        bind(mappings, key_up       , MDFR_NONE, lister__move_up);
        bind(mappings, key_page_up  , MDFR_NONE, lister__move_up);
        bind(mappings, key_down     , MDFR_NONE, lister__move_down);
        bind(mappings, key_page_down, MDFR_NONE, lister__move_down);
        bind(mappings, key_mouse_wheel       , MDFR_NONE, lister__wheel_scroll);
        bind(mappings, key_mouse_left        , MDFR_NONE, lister__mouse_press);
        bind(mappings, key_view_activate     , MDFR_NONE, lister__mouse_press);
        bind(mappings, key_mouse_left_release, MDFR_NONE, lister__mouse_release);
        bind(mappings, key_mouse_move, MDFR_NONE, lister__repaint);
        bind(mappings, key_animate   , MDFR_NONE, lister__repaint);
        
        end_map(mappings);
    }
    end_mapping(mappings);
    
    begin_mapping(mappings, mac_default, "Default 4coder bindings on a Mac keyboard");
    {
        // NOTE(allen): GLOBAL
        begin_map(mappings, mapid_global, "The following bindings apply in all situations.");
        
        bind(mappings, ',', MDFR_CMND, change_active_panel);
        bind(mappings, '<', MDFR_CMND, change_active_panel_backwards);
        
        bind(mappings, 'n', MDFR_CMND, interactive_new);
        bind(mappings, 'o', MDFR_CMND, interactive_open_or_new);
        bind(mappings, 'o', MDFR_CTRL, open_in_other);
        bind(mappings, 'k', MDFR_CMND, interactive_kill_buffer);
        bind(mappings, 'i', MDFR_CMND, interactive_switch_buffer);
        bind(mappings, 'h', MDFR_CMND, project_go_to_root_directory);
        bind(mappings, 'S', MDFR_CMND, save_all_dirty_buffers);
        
        bind(mappings, '.', MDFR_CTRL, change_to_build_panel);
        bind(mappings, ',', MDFR_CTRL, close_build_panel);
        bind(mappings, 'n', MDFR_CTRL, goto_next_jump_sticky);
        bind(mappings, 'N', MDFR_CTRL, goto_prev_jump_sticky);
        bind(mappings, 'M', MDFR_CTRL, goto_first_jump_sticky);
        bind(mappings, 'm', MDFR_CTRL, build_in_build_panel);
        bind(mappings, 'b', MDFR_CTRL, toggle_filebar);
        
        bind(mappings, 'z', MDFR_CTRL, execute_any_cli);
        bind(mappings, 'Z', MDFR_CTRL, execute_previous_cli);
        
        bind(mappings, 'x', MDFR_CTRL, command_lister);
        bind(mappings, 'X', MDFR_CTRL, project_command_lister);
        
        bind(mappings, 'I', MDFR_CMND, list_all_functions_current_buffer_lister);
        bind(mappings, 'E', MDFR_CTRL, exit_4coder);
        
        bind(mappings, key_f1, MDFR_NONE, project_fkey_command);
        bind(mappings, key_f2, MDFR_NONE, project_fkey_command);
        bind(mappings, key_f3, MDFR_NONE, project_fkey_command);
        bind(mappings, key_f4, MDFR_NONE, project_fkey_command);
        
        bind(mappings, key_f5, MDFR_NONE, project_fkey_command);
        bind(mappings, key_f6, MDFR_NONE, project_fkey_command);
        bind(mappings, key_f7, MDFR_NONE, project_fkey_command);
        bind(mappings, key_f8, MDFR_NONE, project_fkey_command);
        
        bind(mappings, key_f9, MDFR_NONE, project_fkey_command);
        bind(mappings, key_f10, MDFR_NONE, project_fkey_command);
        bind(mappings, key_f11, MDFR_NONE, project_fkey_command);
        bind(mappings, key_f12, MDFR_NONE, project_fkey_command);
        
        bind(mappings, key_f13, MDFR_NONE, project_fkey_command);
        bind(mappings, key_f14, MDFR_NONE, project_fkey_command);
        bind(mappings, key_f15, MDFR_NONE, project_fkey_command);
        bind(mappings, key_f16, MDFR_NONE, project_fkey_command);
        
        bind(mappings, key_mouse_wheel, MDFR_NONE, mouse_wheel_scroll);
        
        end_map(mappings);
        
        // NOTE(allen): FILE
        begin_map(mappings, mapid_file, "The following bindings apply in general text files and most apply in code files, but some are overriden by other commands specific to code files.");
        
        bind_vanilla_keys(mappings, MDFR_NONE, write_character);
        bind_vanilla_keys(mappings, MDFR_ALT, write_character);
        
        bind(mappings, key_mouse_left, MDFR_NONE, click_set_cursor_and_mark);
        bind(mappings, key_view_activate, MDFR_NONE, click_set_cursor_and_mark);
        bind(mappings, key_mouse_left_release, MDFR_NONE, click_set_cursor);
        bind(mappings, key_mouse_move, MDFR_NONE, click_set_cursor_if_lbutton);
        
        bind(mappings, key_del, MDFR_NONE, delete_char);
        bind(mappings, key_del, MDFR_SHIFT, delete_char);
        bind(mappings, key_back, MDFR_NONE, backspace_char);
        bind(mappings, key_back, MDFR_SHIFT, backspace_char);
        
        bind(mappings, key_up,    MDFR_NONE, move_up);
        bind(mappings, key_down,  MDFR_NONE, move_down);
        bind(mappings, key_left,  MDFR_NONE, move_left);
        bind(mappings, key_right, MDFR_NONE, move_right);
        bind(mappings, key_up,    MDFR_SHIFT, move_up);
        bind(mappings, key_down,  MDFR_SHIFT, move_down);
        bind(mappings, key_left,  MDFR_SHIFT, move_left);
        bind(mappings, key_right, MDFR_SHIFT, move_right);
        
        bind(mappings, key_end,       MDFR_NONE, seek_end_of_line);
        bind(mappings, key_home,      MDFR_NONE, seek_beginning_of_line);
        bind(mappings, key_page_up,   MDFR_CTRL, goto_beginning_of_file);
        bind(mappings, key_page_down, MDFR_CTRL, goto_end_of_file);
        bind(mappings, key_page_up,   MDFR_NONE, page_up);
        bind(mappings, key_page_down, MDFR_NONE, page_down);
        bind(mappings, key_end,       MDFR_SHIFT, seek_end_of_line);
        bind(mappings, key_home,      MDFR_SHIFT, seek_beginning_of_line);
        bind(mappings, key_page_up,   MDFR_CTRL|MDFR_SHIFT, goto_beginning_of_file);
        bind(mappings, key_page_down, MDFR_CTRL|MDFR_SHIFT, goto_end_of_file);
        bind(mappings, key_page_up,   MDFR_SHIFT, page_up);
        bind(mappings, key_page_down, MDFR_SHIFT, page_down);
        
        bind(mappings, key_up,    MDFR_CMND, seek_whitespace_up_end_line);
        bind(mappings, key_down,  MDFR_CMND, seek_whitespace_down_end_line);
        bind(mappings, key_right, MDFR_CMND, seek_whitespace_right);
        bind(mappings, key_left,  MDFR_CMND, seek_whitespace_left);
        bind(mappings, key_up,    MDFR_CMND|MDFR_SHIFT, seek_whitespace_up_end_line);
        bind(mappings, key_down,  MDFR_CMND|MDFR_SHIFT, seek_whitespace_down_end_line);
        bind(mappings, key_right, MDFR_CMND|MDFR_SHIFT, seek_whitespace_right);
        bind(mappings, key_left,  MDFR_CMND|MDFR_SHIFT, seek_whitespace_left);
        
        bind(mappings, key_up,   MDFR_ALT, move_line_up);
        bind(mappings, key_down, MDFR_ALT, move_line_down);
        
        bind(mappings, key_back, MDFR_CMND, backspace_word);
        bind(mappings, key_del, MDFR_CMND, delete_word);
        bind(mappings, key_back, MDFR_CTRL, snipe_token_or_word);
        bind(mappings, key_del, MDFR_CTRL, snipe_token_or_word_right);
        
        bind(mappings, '/', MDFR_CMND, set_mark);
        bind(mappings, 'a', MDFR_CMND, replace_in_range);
        bind(mappings, 'c', MDFR_CMND, copy);
        bind(mappings, 'd', MDFR_CMND, delete_range);
        bind(mappings, 'D', MDFR_CMND, delete_line);
        bind(mappings, 'e', MDFR_CMND, center_view);
        bind(mappings, 'E', MDFR_CMND, left_adjust_view);
        bind(mappings, 'f', MDFR_CMND, search);
        bind(mappings, 'F', MDFR_CMND, list_all_locations);
        bind(mappings, 'F', MDFR_CTRL, list_all_substring_locations_case_insensitive);
        bind(mappings, 'g', MDFR_CMND, goto_line);
        bind(mappings, 'G', MDFR_CMND, list_all_locations_of_selection);
        bind(mappings, 'K', MDFR_CMND, kill_buffer);
        bind(mappings, 'L', MDFR_CMND, duplicate_line);
        bind(mappings, 'm', MDFR_CMND, cursor_mark_swap);
        bind(mappings, 'O', MDFR_CMND, reopen);
        bind(mappings, 'q', MDFR_CMND, query_replace);
        bind(mappings, 'Q', MDFR_CMND, query_replace_identifier);
        bind(mappings, 'r', MDFR_CMND, reverse_search);
        bind(mappings, 's', MDFR_CMND, save);
        bind(mappings, 't', MDFR_CMND, search_identifier);
        bind(mappings, 'T', MDFR_CMND, list_all_locations_of_identifier);
        bind(mappings, 'v', MDFR_CMND, paste_and_indent);
        bind(mappings, 'V', MDFR_CMND, paste_next_and_indent);
        bind(mappings, 'x', MDFR_CMND, cut);
        bind(mappings, 'y', MDFR_CMND, redo);
        bind(mappings, 'z', MDFR_CMND, undo);
        
        bind(mappings, '1', MDFR_CMND, view_buffer_other_panel);
        bind(mappings, '2', MDFR_CMND, swap_buffers_between_panels);
        
        bind(mappings, '\n', MDFR_NONE, newline_or_goto_position_sticky);
        bind(mappings, '\n', MDFR_SHIFT, newline_or_goto_position_same_panel_sticky);
        bind(mappings, '>', MDFR_CMND, view_jump_list_with_lister);
        bind(mappings, ' ', MDFR_SHIFT, write_character);
        
        end_map(mappings);
        
        // NOTE(allen): CODE
        begin_map(mappings, default_code_map, "The following commands only apply in files where the lexer (syntax highlighting) is turned on.");
        
        inherit_map(mappings, mapid_file);
        
        bind(mappings, key_right, MDFR_CMND, seek_alphanumeric_or_camel_right);
        bind(mappings, key_left, MDFR_CMND, seek_alphanumeric_or_camel_left);
        
        bind(mappings, '\n', MDFR_NONE, write_and_auto_tab);
        bind(mappings, '\n', MDFR_SHIFT, write_and_auto_tab);
        bind(mappings, '}', MDFR_NONE, write_and_auto_tab);
        bind(mappings, ')', MDFR_NONE, write_and_auto_tab);
        bind(mappings, ']', MDFR_NONE, write_and_auto_tab);
        bind(mappings, ';', MDFR_NONE, write_and_auto_tab);
        bind(mappings, '#', MDFR_NONE, write_and_auto_tab);
        
        bind(mappings, '\t', MDFR_NONE, word_complete);
        bind(mappings, '\t', MDFR_CMND, auto_tab_range);
        bind(mappings, '\t', MDFR_SHIFT, auto_tab_line_at_cursor);
        
        bind(mappings, 'h', MDFR_CTRL, write_hack);
        bind(mappings, 'r', MDFR_CTRL, write_block);
        bind(mappings, 't', MDFR_CTRL, write_todo);
        bind(mappings, 'y', MDFR_CTRL, write_note);
        
        bind(mappings, 'D', MDFR_CTRL, list_all_locations_of_type_definition);
        bind(mappings, 'T', MDFR_CTRL, list_all_locations_of_type_definition_of_identifier);
        
        bind(mappings, '[', MDFR_CMND, open_long_braces);
        bind(mappings, '{', MDFR_CMND, open_long_braces_semicolon);
        bind(mappings, '}', MDFR_CMND, open_long_braces_break);
        
        bind(mappings, '[', MDFR_CTRL, select_surrounding_scope);
        bind(mappings, ']', MDFR_CTRL, select_prev_scope_absolute);
        bind(mappings, '\'', MDFR_CTRL, select_next_scope_absolute);
        bind(mappings, '/', MDFR_CTRL, place_in_scope);
        bind(mappings, '-', MDFR_CTRL, delete_current_scope);
        bind(mappings, 'j', MDFR_CTRL, scope_absorb_down);
        
        bind(mappings, 'i', MDFR_CTRL, if0_off);
        
        bind(mappings, '1', MDFR_CTRL, open_file_in_quotes);
        bind(mappings, '2', MDFR_CTRL, open_matching_file_cpp);
        bind(mappings, '0', MDFR_CMND, write_zero_struct);
        
        end_map(mappings);
        
        // NOTE(allen): LISTER
        begin_map(mappings, default_lister_ui_map,
                  "These commands apply in 'lister mode' such as when you open a file.");
        
        bind_vanilla_keys(mappings, MDFR_NONE, lister__write_character);
        bind(mappings, key_esc, MDFR_NONE, lister__quit);
        bind(mappings, '\n', MDFR_NONE, lister__activate);
        bind(mappings, '\t', MDFR_NONE, lister__activate);
        bind(mappings, key_back     , MDFR_NONE, lister__backspace_text_field);
        bind(mappings, key_up       , MDFR_NONE, lister__move_up);
        bind(mappings, key_page_up  , MDFR_NONE, lister__move_up);
        bind(mappings, key_down     , MDFR_NONE, lister__move_down);
        bind(mappings, key_page_down, MDFR_NONE, lister__move_down);
        bind(mappings, key_mouse_wheel       , MDFR_NONE, lister__wheel_scroll);
        bind(mappings, key_mouse_left        , MDFR_NONE, lister__mouse_press);
        bind(mappings, key_view_activate     , MDFR_NONE, click_set_cursor_and_mark);
        bind(mappings, key_mouse_left_release, MDFR_NONE, lister__mouse_release);
        bind(mappings, key_mouse_move, MDFR_NONE, lister__repaint);
        bind(mappings, key_animate   , MDFR_NONE, lister__repaint);
        
        end_map(mappings);
    }
    end_mapping(mappings);
    
    // Generate remapping from mapping array
    FILE *out = fopen(REMAPPING_FILE, "wb");
    if (out != 0){
        
        fprintf(out, "#if defined(CUSTOM_COMMAND_SIG)\n");
        for (Mapping *mapping = mappings->first_mapping;
             mapping != 0;
             mapping = mapping->next){
            fprintf(out, "void fill_keys_%s(Bind_Helper *context){\n", mapping->name);
            
            for (Sub_Map *sub_map = mapping->first_sub_map;
                 sub_map != 0;
                 sub_map = sub_map->next){
                fprintf(out, "begin_map(context, %s);\n", sub_map->name);
                
                if (sub_map->parent != 0){
                    fprintf(out, "inherit_map(context, %s);\n", sub_map->parent);
                }
                
                for (Key_Bind *bind = sub_map->first_key_bind;
                     bind != 0;
                     bind = bind->next){
                    char mdfr_str[256];
                    String m = make_fixed_width_string(mdfr_str);
                    b32 has_base = false;
                    
                    if (bind->modifiers & MDFR_CTRL){
                        if (has_base){
                            append(&m, "|");
                        }
                        append(&m, "MDFR_CTRL");
                        has_base = true;
                    }
                    if (bind->modifiers & MDFR_ALT){
                        if (has_base){
                            append(&m, "|");
                        }
                        append(&m, "MDFR_ALT");
                        has_base = true;
                    }
                    if (bind->modifiers & MDFR_CMND){
                        if (has_base){
                            append(&m, "|");
                        }
                        append(&m, "MDFR_CMND");
                        has_base = true;
                    }
                    if (bind->modifiers & MDFR_SHIFT){
                        if (has_base){
                            append(&m, "|");
                        }
                        append(&m, "MDFR_SHIFT");
                        has_base = true;
                    }
                    if (bind->modifiers == 0){
                        append(&m, "MDFR_NONE");
                        has_base = true;
                    }
                    terminate_with_null(&m);
                    
                    if (bind->vanilla){
                        if (bind->modifiers == 0){
                            fprintf(out, "bind_vanilla_keys(context, %s);\n", bind->command);
                        }
                        else{
                            fprintf(out, "bind_vanilla_keys(context, %s, %s);\n", mdfr_str, bind->command);
                        }
                    }
                    else{
                        char key_str_space[16];
                        char *key_str = 0;
                        switch (bind->keycode){
#define KeyCase(n) case key_##n: key_str = "key_" #n; break;
                            KEY_LIST(KeyCase)
#undef KeyCase
                        }
                        
                        if (key_str == 0){
                            key_str = key_str_space;
                            if (bind->keycode == '\n'){
                                memcpy(key_str_space, "'\\n'", 5);
                            }
                            else if (bind->keycode == '\t'){
                                memcpy(key_str_space, "'\\t'", 5);
                            }
                            else if (bind->keycode == '\''){
                                memcpy(key_str_space, "'\\''", 5);
                            }
                            else if (bind->keycode == '\\'){
                                memcpy(key_str_space, "'\\\\'", 5);
                            }
                            else{
                                Assert(bind->keycode <= 127);
                                key_str_space[0] = '\'';
                                key_str_space[1] = (char)bind->keycode;
                                key_str_space[2] = '\'';
                                key_str_space[3] = 0;
                            }
                        }
                        
                        fprintf(out, "bind(context, %s, %s, %s);\n",
                                key_str,
                                mdfr_str,
                                bind->command);
                    }
                }
                
                fprintf(out, "end_map(context);\n");
            }
            
            fprintf(out, "}\n");
        }
        fprintf(out, "#endif\n");
        
        fprintf(out,
                "#if defined(CUSTOM_COMMAND_SIG)\n"
                "#define LINK_PROCS(x) x\n"
                "#else\n"
                "#define LINK_PROCS(x)\n"
                "#endif\n");
        
        fprintf(out,
                "struct Meta_Key_Bind{\n"
                "int32_t vanilla;\n"
                "uint32_t keycode;\n"
                "uint32_t modifiers;\n"
                "char *command;\n"
                "int32_t command_len;\n"
                "LINK_PROCS(Custom_Command_Function *proc;)\n"
                "};\n"
                "struct Meta_Sub_Map{\n"
                "char *name;\n"
                "int32_t name_len;\n"
                "char *description;\n"
                "int32_t description_len;\n"
                "char *parent;\n"
                "int32_t parent_len;\n"
                "Meta_Key_Bind *binds;\n"
                "int32_t bind_count;\n"
                "};\n"
                "struct Meta_Mapping{\n"
                "char *name;\n"
                "int32_t name_len;\n"
                "char *description;\n"
                "int32_t description_len;\n"
                "Meta_Sub_Map *sub_maps;\n"
                "int32_t sub_map_count;\n"
                "LINK_PROCS(void (*fill_keys_proc)(Bind_Helper *context);)\n"
                "};\n");
        
        for (Mapping *mapping = mappings->first_mapping;
             mapping != 0;
             mapping = mapping->next){
            for (Sub_Map *sub_map = mapping->first_sub_map;
                 sub_map != 0;
                 sub_map = sub_map->next){
                if (sub_map->key_bind_count > 0){
                    fprintf(out, "static Meta_Key_Bind fcoder_binds_for_%s_%s[%d] = {\n",
                            mapping->name, sub_map->name, sub_map->key_bind_count);
                    for (Key_Bind *bind = sub_map->first_key_bind;
                         bind != 0;
                         bind = bind->next){
                        fprintf(out,
                                "{%d, %u, %u, \"%s\", %d, LINK_PROCS(%s)},\n",
                                bind->vanilla, bind->keycode, bind->modifiers,
                                bind->command, bind->command_len,
                                bind->command);
                    }
                    fprintf(out, "};\n");
                }
            }
            
            fprintf(out, "static Meta_Sub_Map fcoder_submaps_for_%s[%d] = {\n",
                    mapping->name, mapping->sub_map_count);
            for (Sub_Map *sub_map = mapping->first_sub_map;
                 sub_map != 0;
                 sub_map = sub_map->next){
                if (sub_map->parent != 0){
                    fprintf(out, "{\"%s\", %d, \"%s\", %d, \"%s\", %d, fcoder_binds_for_%s_%s, %d},\n",
                            sub_map->name, sub_map->name_len,
                            sub_map->description, sub_map->description_len,
                            sub_map->parent, sub_map->parent_len,
                            mapping->name, sub_map->name,
                            sub_map->key_bind_count);
                }
                else{
                    fprintf(out, "{\"%s\", %d, \"%s\", %d, 0, 0, fcoder_binds_for_%s_%s, %d},\n",
                            sub_map->name, sub_map->name_len,
                            sub_map->description, sub_map->description_len,
                            mapping->name, sub_map->name,
                            sub_map->key_bind_count);
                }
            }
            fprintf(out, "};\n");
        }
        
        fprintf(out, "static Meta_Mapping fcoder_meta_maps[%d] = {\n",
                mappings->mapping_count);
        for (Mapping *mapping = mappings->first_mapping;
             mapping != 0;
             mapping = mapping->next){
            fprintf(out, "{\"%s\", %d, \"%s\", %d, fcoder_submaps_for_%s, %d, LINK_PROCS(fill_keys_%s)},\n",
                    mapping->name, mapping->name_len,
                    mapping->description, mapping->description_len,
                    mapping->name,
                    mapping->sub_map_count,
                    mapping->name);
        }
        fprintf(out, "};\n");
        
        fclose(out);
    }
    
    fm_end_temp(temp);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv){
    META_BEGIN();
    
    fm_init_system();
    generate_keycode_enum();
    generate_style();
    generate_custom_headers();
    generate_remapping_code_and_data();
    
    META_FINISH();
}

// BOTTOM

