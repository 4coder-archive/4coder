/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 25.02.2016
 *
 * File editing view for 4coder
 *
 */

// TOP

#define API_H "4coder_generated/app_functions.h"
#define REMAPPING_FILE "4coder_generated/remapping.h"

#include "4coder_base_types.h"
#include "4ed_meta_defines.h"
#include "4coder_API/4coder_version.h"
#include "4coder_API/4coder_keycodes.h"

#include "4coder_base_types.cpp"
#include "4coder_stringf.cpp"
# define FSTRING_IMPLEMENTATION
#include "4coder_lib/4cpp_lexer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define FTECH_FILE_MOVING_IMPLEMENTATION
#include "4ed_file_moving.h"
#include "4ed_meta_parser.cpp"
#include "4ed_meta_keywords.h"

//
// Meta Parse Rules
//

struct App_API_Name{
    String_Const_char macro;
    String_Const_char public_name;
};

struct App_API{
    App_API_Name *names;
};

internal App_API
allocate_app_api(Arena *arena, i32 count){
    App_API app_api = {};
    app_api.names = push_array(arena, App_API_Name, count);
    memset(app_api.names, 0, sizeof(App_API_Name)*count);
    return(app_api);
}

internal void
generate_custom_headers(Arena *arena){
    Temp_Memory temp = begin_temp(arena);
    
    // NOTE(allen): Parse the customization API files
    static char *functions_files[] = { "4ed_api_implementation.cpp", 0 };
    Meta_Unit unit_custom = compile_meta_unit(arena, ".", functions_files, ExpandArray(meta_keywords));
    if (unit_custom.parse == 0){
        Assert(!"Missing one or more input files!");
    }
    
    // NOTE(allen): Compute and store variations of the function names
    App_API func_4ed_names = allocate_app_api(arena, unit_custom.set.count);
    
    for (i32 i = 0; i < unit_custom.set.count; ++i){
        String_Const_char name_string = unit_custom.set.items[i].name;
        
        List_String_Const_char macro_list = {};
        string_list_push(arena, &macro_list, string_mod_upper(push_string_copy(arena, name_string)));
        string_list_push_lit(arena, &macro_list, "_SIG");
        func_4ed_names.names[i].macro = string_list_flatten(arena, macro_list);
        
        func_4ed_names.names[i].public_name = string_mod_lower(push_string_copy(arena, name_string));
    }
    
    // NOTE(allen): Output
    List_String_Const_char out_list = {};
    
    // NOTE(allen): Custom API headers
    string_list_push_lit(arena, &out_list, "struct Application_Links;\n");
    
    for (i32 i = 0; i < unit_custom.set.count; ++i){
        string_list_pushf(arena, &out_list, "#define %.*s(n) %.*s n%.*s\n",
                          string_expand(func_4ed_names.names[i].macro),
                          string_expand(unit_custom.set.items[i].ret),
                          string_expand(unit_custom.set.items[i].args));
#if 0
        append(&out, "#define ");
        append(&out, func_4ed_names.names[i].macro);
        append(&out, "(n) ");
        append(&out, unit_custom.set.items[i].ret);
        append(&out, " n");
        append(&out, unit_custom.set.items[i].args);
        append_s_char(&out, '\n');
#endif
    }
    
    for (i32 i = 0; i < unit_custom.set.count; ++i){
        string_list_pushf(arena, &out_list, "typedef %.*s(%.*s_Function);\n",
                          string_expand(func_4ed_names.names[i].macro),
                          string_expand(unit_custom.set.items[i].name));
#if 0
        append(&out, "typedef ");
        append(&out, func_4ed_names.names[i].macro);
        append_s_char(&out, '(');
        append(&out, unit_custom.set.items[i].name);
        append(&out, "_Function);\n");
#endif
    }
    
    string_list_push_lit(arena, &out_list, "struct Application_Links{\n");
    string_list_push_lit(arena, &out_list, "#if defined(ALLOW_DEP_4CODER)\n");
    for (i32 i = 0; i < unit_custom.set.count; ++i){
        string_list_pushf(arena, &out_list, "%.*s_Function *%.*s;\n",
                          string_expand(unit_custom.set.items[i].name),
                          string_expand(func_4ed_names.names[i].public_name));
#if 0
        append(&out, unit_custom.set.items[i].name);
        append(&out, "_Function *");
        append(&out, func_4ed_names.names[i].public_name);
        append(&out, ";\n");
#endif
    }
    
    string_list_push_lit(arena, &out_list, "#else\n");
    
    for (i32 i = 0; i < unit_custom.set.count; ++i){
        string_list_pushf(arena, &out_list, "%.*s_Function *%.*s_;\n",
                          string_expand(unit_custom.set.items[i].name),
                          string_expand(func_4ed_names.names[i].public_name));
#if 0
        append(&out, unit_custom.set.items[i].name);
        append(&out, "_Function *");
        append(&out, func_4ed_names.names[i].public_name);
        append(&out, "_;\n");
#endif
    }
    
    string_list_push_lit(arena, &out_list, "#endif\n");
    
    string_list_push_lit(arena, &out_list,
                         "void *memory;\n"
                         "int32_t memory_size;\n"
                         "void *cmd_context;\n"
                         "void *system_links;\n"
                         "void *current_coroutine;\n"
                         "int32_t type_coroutine;\n"
                         "};\n");
    
    string_list_push_lit(arena, &out_list, "#define FillAppLinksAPI(app_links) do{");
    
    for (i32 i = 0; i < unit_custom.set.count; ++i){
        string_list_pushf(arena, &out_list, "\\\napp_links->%.*s_ = %.*s;",
                          string_expand(func_4ed_names.names[i].public_name),
                          string_expand(unit_custom.set.items[i].name));
    }
    
    string_list_push_lit(arena, &out_list, "} while(false)\n");
    
    string_list_push_lit(arena, &out_list, "#if defined(ALLOW_DEP_4CODER)\n");
    for (i32 use_dep = 1; use_dep >= 0; --use_dep){
        for (i32 i = 0; i < unit_custom.set.count; ++i){
            Argument_Breakdown breakdown = unit_custom.set.items[i].breakdown;
            String_Const_char ret = unit_custom.set.items[i].ret;
            String_Const_char public_name = func_4ed_names.names[i].public_name;
            
            string_list_pushf(arena, &out_list, "static %.*s %.*s(",
                              string_expand(ret), string_expand(public_name));
            for (i32 j = 0; j < breakdown.count; j += 1){
                if (j + 1 != breakdown.count){
                    string_list_pushf(arena, &out_list, "%.*s, ", 
                                      string_expand(breakdown.args[j].param_string));
                }
                else{
                    string_list_push(arena, &out_list, breakdown.args[j].param_string);
                }
            }
            string_list_push_lit(arena, &out_list, "){");
            
            if (string_match(ret, string_litexpr("void"))){
                string_list_push_lit(arena, &out_list, "(");
            }
            else{
                string_list_push_lit(arena, &out_list, "return(");
            }
            
            string_list_pushf(arena, &out_list, "app->%.*s", string_expand(public_name));
            if (!use_dep){
                string_list_push_lit(arena, &out_list, "_");
            }
            
            string_list_push_lit(arena, &out_list, "(");
            for (i32 j = 0; j < breakdown.count; ++j){
                if (j + 1 != breakdown.count){
                    string_list_pushf(arena, &out_list, "%.*s, ",
                                      string_expand(breakdown.args[j].param_name));
                }
                else{
                    string_list_push(arena, &out_list, breakdown.args[j].param_name);
                }
            }
            string_list_push_lit(arena, &out_list, "));}\n");
        }
        if (use_dep == 1){
            string_list_push_lit(arena, &out_list, "#else\n");
        }
    }
    string_list_push_lit(arena, &out_list, "#endif\n");
    
    String_Const_char out = string_list_flatten(arena, out_list);
    fm_write_file(API_H, out.str, (i32)out.size);
    
    end_temp(temp);
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
emit_begin_mapping(Arena *arena, Mapping_Array *array, char *name, char *description){
    Assert(array->current_mapping == 0);
    Mapping *mapping = push_array(arena, Mapping, 1);
    mapping->name = fm_basic_str(arena, name);
    mapping->name_len = (i32)cstring_length(name);
    mapping->description = fm_basic_str(arena, description);
    mapping->description_len = (i32)cstring_length(description);
    mapping->first_sub_map = 0;
    mapping->last_sub_map = 0;
    mapping->sub_map_count = 0;
    sll_queue_push(array->first_mapping, array->last_mapping, mapping);
    ++array->mapping_count;
    array->current_mapping = mapping;
}

internal void
emit_end_mapping(Mapping_Array *array){
    Assert(array->current_mapping != 0);
    array->current_mapping = 0;
}

internal void
emit_begin_map(Arena *arena, Mapping_Array *array, char *mapid, char *description){
    Assert(array->current_mapping != 0);
    Assert(array->current_sub_map == 0);
    
    Sub_Map *sub_map = push_array(arena, Sub_Map, 1);
    sub_map->name = fm_basic_str(arena, mapid);
    sub_map->name_len = (i32)cstring_length(mapid);
    sub_map->description = fm_basic_str(arena, description);
    sub_map->description_len = (i32)cstring_length(description);
    sub_map->parent = 0;
    sub_map->parent_len = 0;
    sub_map->first_key_bind = 0;
    sub_map->last_key_bind = 0;
    sub_map->key_bind_count = 0;
    
    Mapping *mapping = array->current_mapping;
    sll_queue_push(mapping->first_sub_map, mapping->last_sub_map, sub_map);
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
emit_inherit_map(Arena *arena, Mapping_Array *array, char *mapid){
    Assert(array->current_mapping != 0);
    Assert(array->current_sub_map != 0);
    
    Sub_Map *sub_map = array->current_sub_map;
    Assert(sub_map->parent == 0);
    
    sub_map->parent = fm_basic_str(arena, mapid);
    sub_map->parent_len = (i32)cstring_length(mapid);
}

internal void
emit_bind(Arena *arena, Mapping_Array *array, u32 keycode, u32 modifiers, char *command){
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
        Key_Bind *bind = push_array(arena, Key_Bind, 1);
        bind->vanilla = false;
        bind->keycode = keycode;
        bind->modifiers = modifiers;
        bind->command = fm_basic_str(arena, command);
        bind->command_len = (i32)cstring_length(command);
        sll_queue_push(sub_map->first_key_bind, sub_map->last_key_bind, bind);
        ++sub_map->key_bind_count;
    }
}

internal void
emit_bind_all_modifiers(Arena *arena, Mapping_Array *mappings, u32 code, char *command){
    emit_bind(arena, mappings, code, MDFR_NONE, command);
    emit_bind(arena, mappings, code, MDFR_CTRL, command);
    emit_bind(arena, mappings, code, MDFR_ALT , command);
    emit_bind(arena, mappings, code, MDFR_CMND, command);
    emit_bind(arena, mappings, code, MDFR_CTRL|MDFR_ALT , command);
    emit_bind(arena, mappings, code, MDFR_ALT |MDFR_CMND, command);
    emit_bind(arena, mappings, code, MDFR_CTRL|MDFR_CMND, command);
    emit_bind(arena, mappings, code, MDFR_CTRL|MDFR_ALT|MDFR_CMND, command);
}

internal void
emit_bind_vanilla_keys(Arena *arena, Mapping_Array *array, u32 modifiers, char *command){
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
        Key_Bind *bind = push_array(arena, Key_Bind, 1);
        bind->vanilla = true;
        bind->keycode = 0;
        bind->modifiers = modifiers;
        bind->command = fm_basic_str(arena, command);
        bind->command_len = (i32)cstring_length(command);
        sll_queue_push(sub_map->first_key_bind, sub_map->last_key_bind, bind);
        ++sub_map->key_bind_count;
    }
}

#define begin_mapping(pa,mp,n,d) emit_begin_mapping(pa,mp, #n, d)
#define end_mapping(mp)          emit_end_mapping(mp)
#define begin_map(pa,mp,mapid,d) emit_begin_map(pa,mp, #mapid, d)
#define end_map(mp)              emit_end_map(mp)
#define inherit_map(pa,mp,mapid)      emit_inherit_map(pa,mp, #mapid)
#define bind(pa,mp,k,md,c)            emit_bind(pa,mp, k, md, #c)
#define bind_all_modifiers(pa,mp,k,c) emit_bind_all_modifiers(pa,mp, k, #c)
#define bind_vanilla_keys(pa,mp,md,c) emit_bind_vanilla_keys(pa,mp, md, #c)

//////////////////////////////////////////////////////////////////////////////////////////////////

internal void
generate_remapping_code_and_data(Arena *arena){
    Temp_Memory temp = begin_temp(arena);
    
    // Generate mapping array data structure
    Mapping_Array mappings_ = {};
    Mapping_Array *mappings = &mappings_;
    
    begin_mapping(arena, mappings, default, "The default 4coder bindings - typically good for Windows and Linux");
    {
        // NOTE(allen): GLOBAL
        begin_map(arena, mappings, mapid_global, "The following bindings apply in all situations.");
        
        bind(arena, mappings, ',', MDFR_CTRL, change_active_panel);
        bind(arena, mappings, '<', MDFR_CTRL, change_active_panel_backwards);
        
        bind(arena, mappings, 'n', MDFR_CTRL, interactive_new);
        bind(arena, mappings, 'o', MDFR_CTRL, interactive_open_or_new);
        bind(arena, mappings, 'o', MDFR_ALT , open_in_other);
        bind(arena, mappings, 'k', MDFR_CTRL, interactive_kill_buffer);
        bind(arena, mappings, 'i', MDFR_CTRL, interactive_switch_buffer);
        bind(arena, mappings, 'h', MDFR_CTRL, project_go_to_root_directory);
        bind(arena, mappings, 'S', MDFR_CTRL, save_all_dirty_buffers);
        
        bind(arena, mappings, key_scroll_lock, MDFR_NONE, toggle_filebar);
        bind(arena, mappings, key_pause, MDFR_NONE, toggle_filebar);
        bind(arena, mappings, key_caps, MDFR_NONE, toggle_filebar);
        
        bind(arena, mappings, '.', MDFR_ALT, change_to_build_panel);
        bind(arena, mappings, ',', MDFR_ALT, close_build_panel);
        bind(arena, mappings, 'n', MDFR_ALT, goto_next_jump);
        bind(arena, mappings, 'N', MDFR_ALT, goto_prev_jump);
        bind(arena, mappings, 'M', MDFR_ALT, goto_first_jump);
        bind(arena, mappings, 'm', MDFR_ALT, build_in_build_panel);
        bind(arena, mappings, 'b', MDFR_ALT, toggle_filebar);
        
        bind(arena, mappings, 'z', MDFR_ALT, execute_any_cli);
        bind(arena, mappings, 'Z', MDFR_ALT, execute_previous_cli);
        
        bind(arena, mappings, 'x', MDFR_ALT, command_lister);
        bind(arena, mappings, 'X', MDFR_ALT, project_command_lister);
        
        bind(arena, mappings, 'I', MDFR_CTRL, list_all_functions_all_buffers_lister);
        bind(arena, mappings, 'E', MDFR_ALT, exit_4coder);
        
        bind(arena, mappings, key_f1, MDFR_NONE, project_fkey_command);
        bind(arena, mappings, key_f2, MDFR_NONE, project_fkey_command);
        bind(arena, mappings, key_f3, MDFR_NONE, project_fkey_command);
        bind(arena, mappings, key_f4, MDFR_NONE, project_fkey_command);
        
        bind(arena, mappings, key_f5, MDFR_NONE, project_fkey_command);
        bind(arena, mappings, key_f6, MDFR_NONE, project_fkey_command);
        bind(arena, mappings, key_f7, MDFR_NONE, project_fkey_command);
        bind(arena, mappings, key_f8, MDFR_NONE, project_fkey_command);
        
        bind(arena, mappings, key_f9, MDFR_NONE, project_fkey_command);
        bind(arena, mappings, key_f10, MDFR_NONE, project_fkey_command);
        bind(arena, mappings, key_f11, MDFR_NONE, project_fkey_command);
        bind(arena, mappings, key_f12, MDFR_NONE, project_fkey_command);
        
        bind(arena, mappings, key_f13, MDFR_NONE, project_fkey_command);
        bind(arena, mappings, key_f14, MDFR_NONE, project_fkey_command);
        bind(arena, mappings, key_f15, MDFR_NONE, project_fkey_command);
        bind(arena, mappings, key_f16, MDFR_NONE, project_fkey_command);
        
        bind(arena, mappings, key_mouse_wheel, MDFR_NONE, mouse_wheel_scroll);
        bind(arena, mappings, key_mouse_wheel, MDFR_CTRL, mouse_wheel_change_face_size);
        
        end_map(mappings);
        
        // NOTE(allen): FILE
        begin_map(arena, mappings, mapid_file, "The following bindings apply in general text files and most apply in code files, but some are overriden by other commands specific to code files.");
        
        bind_vanilla_keys(arena, mappings, MDFR_NONE, write_character);
        
        bind(arena, mappings, key_mouse_left, MDFR_NONE, click_set_cursor_and_mark);
        bind(arena, mappings, key_click_activate_view, MDFR_NONE, click_set_cursor_and_mark);
        bind(arena, mappings, key_mouse_left_release, MDFR_NONE, click_set_cursor);
        bind(arena, mappings, key_mouse_move, MDFR_NONE, click_set_cursor_if_lbutton);
        
        bind(arena, mappings, key_del,  MDFR_NONE, delete_char);
        bind(arena, mappings, key_del,  MDFR_SHIFT, delete_char);
        bind(arena, mappings, key_back, MDFR_NONE, backspace_char);
        bind(arena, mappings, key_back, MDFR_SHIFT, backspace_char);
        
        bind(arena, mappings, key_up,    MDFR_NONE, move_up);
        bind(arena, mappings, key_down,  MDFR_NONE, move_down);
        bind(arena, mappings, key_left,  MDFR_NONE, move_left);
        bind(arena, mappings, key_right, MDFR_NONE, move_right);
        bind(arena, mappings, key_up,    MDFR_SHIFT, move_up);
        bind(arena, mappings, key_down,  MDFR_SHIFT, move_down);
        bind(arena, mappings, key_left,  MDFR_SHIFT, move_left);
        bind(arena, mappings, key_right, MDFR_SHIFT, move_right);
        
        bind(arena, mappings, key_end,       MDFR_NONE, seek_end_of_line);
        bind(arena, mappings, key_home,      MDFR_NONE, seek_beginning_of_line);
        bind(arena, mappings, key_page_up,   MDFR_CTRL, goto_beginning_of_file);
        bind(arena, mappings, key_page_down, MDFR_CTRL, goto_end_of_file);
        bind(arena, mappings, key_page_up,   MDFR_NONE, page_up);
        bind(arena, mappings, key_page_down, MDFR_NONE, page_down);
        bind(arena, mappings, key_end,       MDFR_SHIFT, seek_end_of_line);
        bind(arena, mappings, key_home,      MDFR_SHIFT, seek_beginning_of_line);
        bind(arena, mappings, key_page_up,   MDFR_CTRL|MDFR_SHIFT, goto_beginning_of_file);
        bind(arena, mappings, key_page_down, MDFR_CTRL|MDFR_SHIFT, goto_end_of_file);
        bind(arena, mappings, key_page_up,   MDFR_SHIFT, page_up);
        bind(arena, mappings, key_page_down, MDFR_SHIFT, page_down);
        
        bind(arena, mappings, key_up,    MDFR_CTRL, move_up_to_blank_line_skip_whitespace);
        bind(arena, mappings, key_down,  MDFR_CTRL, move_down_to_blank_line_end);
        bind(arena, mappings, key_left,  MDFR_CTRL, move_left_whitespace_boundary);
        bind(arena, mappings, key_right, MDFR_CTRL, move_right_whitespace_boundary);
        bind(arena, mappings, key_up,    MDFR_CTRL|MDFR_SHIFT, move_up_to_blank_line_skip_whitespace);
        bind(arena, mappings, key_down,  MDFR_CTRL|MDFR_SHIFT, move_down_to_blank_line_end);
        bind(arena, mappings, key_left,  MDFR_CTRL|MDFR_SHIFT, move_left_whitespace_boundary);
        bind(arena, mappings, key_right, MDFR_CTRL|MDFR_SHIFT, move_right_whitespace_boundary);
        
        bind(arena, mappings, key_up,   MDFR_ALT, move_line_up);
        bind(arena, mappings, key_down, MDFR_ALT, move_line_down);
        
        bind(arena, mappings, key_back, MDFR_CTRL, backspace_alpha_numeric_boundary);
        bind(arena, mappings, key_del,  MDFR_CTRL, delete_alpha_numeric_boundary);
        bind(arena, mappings, key_back, MDFR_ALT, snipe_backward_whitespace_or_token_boundary);
        bind(arena, mappings, key_del,  MDFR_ALT, snipe_forward_whitespace_or_token_boundary);
        
        bind(arena, mappings, ' ', MDFR_CTRL, set_mark);
        bind(arena, mappings, 'a', MDFR_CTRL, replace_in_range);
        bind(arena, mappings, 'c', MDFR_CTRL, copy);
        bind(arena, mappings, 'd', MDFR_CTRL, delete_range);
        bind(arena, mappings, 'D', MDFR_CTRL, delete_line);
        bind(arena, mappings, 'e', MDFR_CTRL, center_view);
        bind(arena, mappings, 'E', MDFR_CTRL, left_adjust_view);
        bind(arena, mappings, 'f', MDFR_CTRL, search);
        bind(arena, mappings, 'F', MDFR_CTRL, list_all_locations);
        bind(arena, mappings, 'F', MDFR_ALT , list_all_substring_locations_case_insensitive);
        bind(arena, mappings, 'g', MDFR_CTRL, goto_line);
        bind(arena, mappings, 'G', MDFR_CTRL, list_all_locations_of_selection);
        bind(arena, mappings, 'j', MDFR_CTRL, snippet_lister);
        bind(arena, mappings, 'K', MDFR_CTRL, kill_buffer);
        bind(arena, mappings, 'L', MDFR_CTRL, duplicate_line);
        bind(arena, mappings, 'm', MDFR_CTRL, cursor_mark_swap);
        bind(arena, mappings, 'O', MDFR_CTRL, reopen);
        bind(arena, mappings, 'q', MDFR_CTRL, query_replace);
        bind(arena, mappings, 'Q', MDFR_CTRL, query_replace_identifier);
        bind(arena, mappings, 'q', MDFR_ALT , query_replace_selection);
        bind(arena, mappings, 'r', MDFR_CTRL, reverse_search);
        bind(arena, mappings, 's', MDFR_CTRL, save);
        bind(arena, mappings, 't', MDFR_CTRL, search_identifier);
        bind(arena, mappings, 'T', MDFR_CTRL, list_all_locations_of_identifier);
        bind(arena, mappings, 'v', MDFR_CTRL, paste_and_indent);
        bind(arena, mappings, 'V', MDFR_CTRL, paste_next_and_indent);
        bind(arena, mappings, 'x', MDFR_CTRL, cut);
        bind(arena, mappings, 'y', MDFR_CTRL, redo);
        bind(arena, mappings, 'z', MDFR_CTRL, undo);
        
        bind(arena, mappings, '1', MDFR_CTRL, view_buffer_other_panel);
        bind(arena, mappings, '2', MDFR_CTRL, swap_buffers_between_panels);
        
        bind(arena, mappings, '\n', MDFR_NONE, newline_or_goto_position);
        bind(arena, mappings, '\n', MDFR_SHIFT, newline_or_goto_position_same_panel);
        bind(arena, mappings, '>', MDFR_CTRL, view_jump_list_with_lister);
        bind(arena, mappings, ' ', MDFR_SHIFT, write_character);
        
        end_map(mappings);
        
        // NOTE(allen): CODE
        begin_map(arena, mappings, default_code_map, "The following commands only apply in files where the lexer (syntax highlighting) is turned on.");
        
        inherit_map(arena, mappings, mapid_file);
        
        bind(arena, mappings, key_left , MDFR_CTRL, move_left_alpha_numeric_or_camel_boundary);
        bind(arena, mappings, key_right, MDFR_CTRL, move_right_alpha_numeric_or_camel_boundary);
        
        bind(arena, mappings, key_left , MDFR_ALT, move_left_alpha_numeric_boundary);
        bind(arena, mappings, key_right, MDFR_ALT, move_right_alpha_numeric_boundary);
        
        bind(arena, mappings, '\n', MDFR_NONE, write_and_auto_tab);
        bind(arena, mappings, '\n', MDFR_SHIFT, write_and_auto_tab);
        bind(arena, mappings, '}', MDFR_NONE, write_and_auto_tab);
        bind(arena, mappings, ')', MDFR_NONE, write_and_auto_tab);
        bind(arena, mappings, ']', MDFR_NONE, write_and_auto_tab);
        bind(arena, mappings, ';', MDFR_NONE, write_and_auto_tab);
        bind(arena, mappings, '#', MDFR_NONE, write_and_auto_tab);
        
        bind(arena, mappings, ';', MDFR_CTRL, comment_line_toggle);
        
        bind(arena, mappings, '\t', MDFR_NONE, word_complete);
        bind(arena, mappings, '\t', MDFR_CTRL, auto_tab_range);
        bind(arena, mappings, '\t', MDFR_SHIFT, auto_tab_line_at_cursor);
        
        bind(arena, mappings, 'r', MDFR_ALT, write_block);
        bind(arena, mappings, 't', MDFR_ALT, write_todo);
        bind(arena, mappings, 'y', MDFR_ALT, write_note);
        
        bind(arena, mappings, 'D', MDFR_ALT, list_all_locations_of_type_definition);
        bind(arena, mappings, 'T', MDFR_ALT, list_all_locations_of_type_definition_of_identifier);
        
        bind(arena, mappings, '[', MDFR_CTRL, open_long_braces);
        bind(arena, mappings, '{', MDFR_CTRL, open_long_braces_semicolon);
        bind(arena, mappings, '}', MDFR_CTRL, open_long_braces_break);
        
        bind(arena, mappings, '[', MDFR_ALT, select_surrounding_scope);
        bind(arena, mappings, ']', MDFR_ALT, select_prev_scope_absolute);
        bind(arena, mappings, '\'', MDFR_ALT, select_next_scope_absolute);
        bind(arena, mappings, '/', MDFR_ALT, place_in_scope);
        bind(arena, mappings, '-', MDFR_ALT, delete_current_scope);
        bind(arena, mappings, 'j', MDFR_ALT, scope_absorb_down);
        
        bind(arena, mappings, 'i', MDFR_ALT, if0_off);
        
        bind(arena, mappings, '1', MDFR_ALT, open_file_in_quotes);
        bind(arena, mappings, '2', MDFR_ALT, open_matching_file_cpp);
        bind(arena, mappings, '0', MDFR_CTRL, write_zero_struct);
        
        end_map(mappings);
        
        // NOTE(allen): LISTER
        begin_map(arena, mappings, default_lister_ui_map,
                  "These commands apply in 'lister mode' such as when you open a file.");
        
        bind_vanilla_keys(arena, mappings, MDFR_NONE, lister__write_character);
        bind(arena, mappings, key_esc, MDFR_NONE, lister__quit);
        bind(arena, mappings, '\n', MDFR_NONE, lister__activate);
        bind(arena, mappings, '\t', MDFR_NONE, lister__activate);
        bind_all_modifiers(arena, mappings, key_back, lister__backspace_text_field);
        bind(arena, mappings, key_up       , MDFR_NONE, lister__move_up);
        bind(arena, mappings, 'k'          , MDFR_ALT , lister__move_up);
        bind(arena, mappings, key_page_up  , MDFR_NONE, lister__move_up);
        bind(arena, mappings, key_down     , MDFR_NONE, lister__move_down);
        bind(arena, mappings, 'j'          , MDFR_ALT , lister__move_down);
        bind(arena, mappings, key_page_down, MDFR_NONE, lister__move_down);
        bind(arena, mappings, key_mouse_wheel       , MDFR_NONE, lister__wheel_scroll);
        bind(arena, mappings, key_mouse_left        , MDFR_NONE, lister__mouse_press);
        bind(arena, mappings, key_mouse_left_release, MDFR_NONE, lister__mouse_release);
        bind(arena, mappings, key_mouse_move, MDFR_NONE, lister__repaint);
        bind(arena, mappings, key_animate   , MDFR_NONE, lister__repaint);
        
        end_map(mappings);
    }
    end_mapping(mappings);
    
    begin_mapping(arena, mappings, mac_default, "Default 4coder bindings on a Mac keyboard");
    {
        // NOTE(allen): GLOBAL
        begin_map(arena, mappings, mapid_global, "The following bindings apply in all situations.");
        
        bind(arena, mappings, ',', MDFR_CMND, change_active_panel);
        bind(arena, mappings, '<', MDFR_CMND, change_active_panel_backwards);
        
        bind(arena, mappings, 'n', MDFR_CMND, interactive_new);
        bind(arena, mappings, 'o', MDFR_CMND, interactive_open_or_new);
        bind(arena, mappings, 'o', MDFR_CTRL, open_in_other);
        bind(arena, mappings, 'k', MDFR_CMND, interactive_kill_buffer);
        bind(arena, mappings, 'i', MDFR_CMND, interactive_switch_buffer);
        bind(arena, mappings, 'h', MDFR_CMND, project_go_to_root_directory);
        bind(arena, mappings, 'S', MDFR_CMND, save_all_dirty_buffers);
        
        bind(arena, mappings, '.', MDFR_CTRL, change_to_build_panel);
        bind(arena, mappings, ',', MDFR_CTRL, close_build_panel);
        bind(arena, mappings, 'n', MDFR_CTRL, goto_next_jump);
        bind(arena, mappings, 'N', MDFR_CTRL, goto_prev_jump);
        bind(arena, mappings, 'M', MDFR_CTRL, goto_first_jump);
        bind(arena, mappings, 'm', MDFR_CTRL, build_in_build_panel);
        bind(arena, mappings, 'b', MDFR_CTRL, toggle_filebar);
        
        bind(arena, mappings, 'z', MDFR_CTRL, execute_any_cli);
        bind(arena, mappings, 'Z', MDFR_CTRL, execute_previous_cli);
        
        bind(arena, mappings, 'x', MDFR_CTRL, command_lister);
        bind(arena, mappings, 'X', MDFR_CTRL, project_command_lister);
        
        bind(arena, mappings, 'I', MDFR_CMND, list_all_functions_all_buffers_lister);
        bind(arena, mappings, 'E', MDFR_CTRL, exit_4coder);
        
        bind(arena, mappings, key_f1, MDFR_NONE, project_fkey_command);
        bind(arena, mappings, key_f2, MDFR_NONE, project_fkey_command);
        bind(arena, mappings, key_f3, MDFR_NONE, project_fkey_command);
        bind(arena, mappings, key_f4, MDFR_NONE, project_fkey_command);
        
        bind(arena, mappings, key_f5, MDFR_NONE, project_fkey_command);
        bind(arena, mappings, key_f6, MDFR_NONE, project_fkey_command);
        bind(arena, mappings, key_f7, MDFR_NONE, project_fkey_command);
        bind(arena, mappings, key_f8, MDFR_NONE, project_fkey_command);
        
        bind(arena, mappings, key_f9, MDFR_NONE, project_fkey_command);
        bind(arena, mappings, key_f10, MDFR_NONE, project_fkey_command);
        bind(arena, mappings, key_f11, MDFR_NONE, project_fkey_command);
        bind(arena, mappings, key_f12, MDFR_NONE, project_fkey_command);
        
        bind(arena, mappings, key_f13, MDFR_NONE, project_fkey_command);
        bind(arena, mappings, key_f14, MDFR_NONE, project_fkey_command);
        bind(arena, mappings, key_f15, MDFR_NONE, project_fkey_command);
        bind(arena, mappings, key_f16, MDFR_NONE, project_fkey_command);
        
        bind(arena, mappings, key_mouse_wheel, MDFR_NONE, mouse_wheel_scroll);
        bind(arena, mappings, key_mouse_wheel, MDFR_CMND, mouse_wheel_change_face_size);
        
        end_map(mappings);
        
        // NOTE(allen): FILE
        begin_map(arena, mappings, mapid_file, "The following bindings apply in general text files and most apply in code files, but some are overriden by other commands specific to code files.");
        
        bind_vanilla_keys(arena, mappings, MDFR_NONE, write_character);
        bind_vanilla_keys(arena, mappings, MDFR_ALT, write_character);
        
        bind(arena, mappings, key_mouse_left, MDFR_NONE, click_set_cursor_and_mark);
        bind(arena, mappings, key_click_activate_view, MDFR_NONE, click_set_cursor_and_mark);
        bind(arena, mappings, key_mouse_left_release, MDFR_NONE, click_set_cursor);
        bind(arena, mappings, key_mouse_move, MDFR_NONE, click_set_cursor_if_lbutton);
        
        bind(arena, mappings, key_del, MDFR_NONE, delete_char);
        bind(arena, mappings, key_del, MDFR_SHIFT, delete_char);
        bind(arena, mappings, key_back, MDFR_NONE, backspace_char);
        bind(arena, mappings, key_back, MDFR_SHIFT, backspace_char);
        
        bind(arena, mappings, key_up,    MDFR_NONE, move_up);
        bind(arena, mappings, key_down,  MDFR_NONE, move_down);
        bind(arena, mappings, key_left,  MDFR_NONE, move_left);
        bind(arena, mappings, key_right, MDFR_NONE, move_right);
        bind(arena, mappings, key_up,    MDFR_SHIFT, move_up);
        bind(arena, mappings, key_down,  MDFR_SHIFT, move_down);
        bind(arena, mappings, key_left,  MDFR_SHIFT, move_left);
        bind(arena, mappings, key_right, MDFR_SHIFT, move_right);
        
        bind(arena, mappings, key_end,       MDFR_NONE, seek_end_of_line);
        bind(arena, mappings, key_home,      MDFR_NONE, seek_beginning_of_line);
        bind(arena, mappings, key_page_up,   MDFR_CTRL, goto_beginning_of_file);
        bind(arena, mappings, key_page_down, MDFR_CTRL, goto_end_of_file);
        bind(arena, mappings, key_page_up,   MDFR_NONE, page_up);
        bind(arena, mappings, key_page_down, MDFR_NONE, page_down);
        bind(arena, mappings, key_end,       MDFR_SHIFT, seek_end_of_line);
        bind(arena, mappings, key_home,      MDFR_SHIFT, seek_beginning_of_line);
        bind(arena, mappings, key_page_up,   MDFR_CTRL|MDFR_SHIFT, goto_beginning_of_file);
        bind(arena, mappings, key_page_down, MDFR_CTRL|MDFR_SHIFT, goto_end_of_file);
        bind(arena, mappings, key_page_up,   MDFR_SHIFT, page_up);
        bind(arena, mappings, key_page_down, MDFR_SHIFT, page_down);
        
        bind(arena, mappings, key_up,    MDFR_CMND, move_up_to_blank_line_skip_whitespace);
        bind(arena, mappings, key_down,  MDFR_CMND, move_down_to_blank_line_end);
        bind(arena, mappings, key_left,  MDFR_CMND, move_left_whitespace_boundary);
        bind(arena, mappings, key_right, MDFR_CMND, move_right_whitespace_boundary);
        bind(arena, mappings, key_up,    MDFR_CMND|MDFR_SHIFT, move_up_to_blank_line_skip_whitespace);
        bind(arena, mappings, key_down,  MDFR_CMND|MDFR_SHIFT, move_down_to_blank_line_end);
        bind(arena, mappings, key_left,  MDFR_CMND|MDFR_SHIFT, move_left_whitespace_boundary);
        bind(arena, mappings, key_right, MDFR_CMND|MDFR_SHIFT, move_right_whitespace_boundary);
        
        bind(arena, mappings, key_up,   MDFR_ALT, move_line_up);
        bind(arena, mappings, key_down, MDFR_ALT, move_line_down);
        
        bind(arena, mappings, key_back, MDFR_CMND, backspace_alpha_numeric_boundary);
        bind(arena, mappings, key_del, MDFR_CMND, delete_alpha_numeric_boundary);
        bind(arena, mappings, key_back, MDFR_CTRL, snipe_backward_whitespace_or_token_boundary);
        bind(arena, mappings, key_del, MDFR_CTRL, snipe_forward_whitespace_or_token_boundary);
        
        bind(arena, mappings, '/', MDFR_CMND, set_mark);
        bind(arena, mappings, 'a', MDFR_CMND, replace_in_range);
        bind(arena, mappings, 'c', MDFR_CMND, copy);
        bind(arena, mappings, 'd', MDFR_CMND, delete_range);
        bind(arena, mappings, 'D', MDFR_CMND, delete_line);
        bind(arena, mappings, 'e', MDFR_CMND, center_view);
        bind(arena, mappings, 'E', MDFR_CMND, left_adjust_view);
        bind(arena, mappings, 'f', MDFR_CMND, search);
        bind(arena, mappings, 'F', MDFR_CMND, list_all_locations);
        bind(arena, mappings, 'F', MDFR_CTRL, list_all_substring_locations_case_insensitive);
        bind(arena, mappings, 'g', MDFR_CMND, goto_line);
        bind(arena, mappings, 'G', MDFR_CMND, list_all_locations_of_selection);
        bind(arena, mappings, 'K', MDFR_CMND, kill_buffer);
        bind(arena, mappings, 'L', MDFR_CMND, duplicate_line);
        bind(arena, mappings, 'm', MDFR_CMND, cursor_mark_swap);
        bind(arena, mappings, 'O', MDFR_CMND, reopen);
        bind(arena, mappings, 'q', MDFR_CMND, query_replace);
        bind(arena, mappings, 'Q', MDFR_CMND, query_replace_identifier);
        bind(arena, mappings, 'r', MDFR_CMND, reverse_search);
        bind(arena, mappings, 's', MDFR_CMND, save);
        bind(arena, mappings, 't', MDFR_CMND, search_identifier);
        bind(arena, mappings, 'T', MDFR_CMND, list_all_locations_of_identifier);
        bind(arena, mappings, 'v', MDFR_CMND, paste_and_indent);
        bind(arena, mappings, 'V', MDFR_CMND, paste_next_and_indent);
        bind(arena, mappings, 'x', MDFR_CMND, cut);
        bind(arena, mappings, 'y', MDFR_CMND, redo);
        bind(arena, mappings, 'z', MDFR_CMND, undo);
        
        bind(arena, mappings, '1', MDFR_CMND, view_buffer_other_panel);
        bind(arena, mappings, '2', MDFR_CMND, swap_buffers_between_panels);
        
        bind(arena, mappings, '\n', MDFR_NONE, newline_or_goto_position);
        bind(arena, mappings, '\n', MDFR_SHIFT, newline_or_goto_position_same_panel);
        bind(arena, mappings, '>', MDFR_CMND, view_jump_list_with_lister);
        bind(arena, mappings, ' ', MDFR_SHIFT, write_character);
        
        end_map(mappings);
        
        // NOTE(allen): CODE
        begin_map(arena, mappings, default_code_map, "The following commands only apply in files where the lexer (syntax highlighting) is turned on.");
        
        inherit_map(arena, mappings, mapid_file);
        
        bind(arena, mappings, key_left , MDFR_CMND, move_left_alpha_numeric_or_camel_boundary);
        bind(arena, mappings, key_right, MDFR_CMND, move_right_alpha_numeric_or_camel_boundary);
        
        bind(arena, mappings, key_left , MDFR_CTRL, move_left_alpha_numeric_boundary);
        bind(arena, mappings, key_right, MDFR_CTRL, move_right_alpha_numeric_boundary);
        
        bind(arena, mappings, '\n', MDFR_NONE, write_and_auto_tab);
        bind(arena, mappings, '\n', MDFR_SHIFT, write_and_auto_tab);
        bind(arena, mappings, '}', MDFR_NONE, write_and_auto_tab);
        bind(arena, mappings, ')', MDFR_NONE, write_and_auto_tab);
        bind(arena, mappings, ']', MDFR_NONE, write_and_auto_tab);
        bind(arena, mappings, ';', MDFR_NONE, write_and_auto_tab);
        bind(arena, mappings, '#', MDFR_NONE, write_and_auto_tab);
        
        bind(arena, mappings, ';', MDFR_CTRL, comment_line_toggle);
        
        bind(arena, mappings, '\t', MDFR_NONE, word_complete);
        bind(arena, mappings, '\t', MDFR_CMND, auto_tab_range);
        bind(arena, mappings, '\t', MDFR_SHIFT, auto_tab_line_at_cursor);
        
        bind(arena, mappings, 'r', MDFR_CTRL, write_block);
        bind(arena, mappings, 't', MDFR_CTRL, write_todo);
        bind(arena, mappings, 'y', MDFR_CTRL, write_note);
        
        bind(arena, mappings, 'D', MDFR_CTRL, list_all_locations_of_type_definition);
        bind(arena, mappings, 'T', MDFR_CTRL, list_all_locations_of_type_definition_of_identifier);
        
        bind(arena, mappings, '[', MDFR_CMND, open_long_braces);
        bind(arena, mappings, '{', MDFR_CMND, open_long_braces_semicolon);
        bind(arena, mappings, '}', MDFR_CMND, open_long_braces_break);
        
        bind(arena, mappings, '[', MDFR_CTRL, select_surrounding_scope);
        bind(arena, mappings, ']', MDFR_CTRL, select_prev_scope_absolute);
        bind(arena, mappings, '\'', MDFR_CTRL, select_next_scope_absolute);
        bind(arena, mappings, '/', MDFR_CTRL, place_in_scope);
        bind(arena, mappings, '-', MDFR_CTRL, delete_current_scope);
        bind(arena, mappings, 'j', MDFR_CTRL, scope_absorb_down);
        
        bind(arena, mappings, 'i', MDFR_CTRL, if0_off);
        
        bind(arena, mappings, '1', MDFR_CTRL, open_file_in_quotes);
        bind(arena, mappings, '2', MDFR_CTRL, open_matching_file_cpp);
        bind(arena, mappings, '0', MDFR_CMND, write_zero_struct);
        
        end_map(mappings);
        
        // NOTE(allen): LISTER
        begin_map(arena, mappings, default_lister_ui_map,
                  "These commands apply in 'lister mode' such as when you open a file.");
        
        bind_vanilla_keys(arena, mappings, MDFR_NONE, lister__write_character);
        bind(arena, mappings, key_esc, MDFR_NONE, lister__quit);
        bind(arena, mappings, '\n', MDFR_NONE, lister__activate);
        bind(arena, mappings, '\t', MDFR_NONE, lister__activate);
        bind_all_modifiers(arena, mappings, key_back, lister__backspace_text_field);
        bind(arena, mappings, key_up       , MDFR_NONE, lister__move_up);
        bind(arena, mappings, key_page_up  , MDFR_NONE, lister__move_up);
        bind(arena, mappings, key_down     , MDFR_NONE, lister__move_down);
        bind(arena, mappings, key_page_down, MDFR_NONE, lister__move_down);
        bind(arena, mappings, key_mouse_wheel       , MDFR_NONE, lister__wheel_scroll);
        bind(arena, mappings, key_mouse_left        , MDFR_NONE, lister__mouse_press);
        bind(arena, mappings, key_mouse_left_release, MDFR_NONE, lister__mouse_release);
        bind(arena, mappings, key_mouse_move, MDFR_NONE, lister__repaint);
        bind(arena, mappings, key_animate   , MDFR_NONE, lister__repaint);
        
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
                    Temp_Memory bind_temp = begin_temp(arena);
                    
                    List_String_Const_char mdfr_list = {};
                    b32 has_base = false;
                    
                    if (bind->modifiers & MDFR_CTRL){
                        if (has_base){
                            string_list_push_lit(arena, &mdfr_list, "|");
                        }
                        string_list_push_lit(arena, &mdfr_list, "MDFR_CTRL");
                        has_base = true;
                    }
                    if (bind->modifiers & MDFR_ALT){
                        if (has_base){
                            string_list_push_lit(arena, &mdfr_list, "|");
                        }
                        string_list_push_lit(arena, &mdfr_list, "MDFR_ALT");
                        has_base = true;
                    }
                    if (bind->modifiers & MDFR_CMND){
                        if (has_base){
                            string_list_push_lit(arena, &mdfr_list, "|");
                        }
                        string_list_push_lit(arena, &mdfr_list, "MDFR_CMND");
                        has_base = true;
                    }
                    if (bind->modifiers & MDFR_SHIFT){
                        if (has_base){
                            string_list_push_lit(arena, &mdfr_list, "|");
                        }
                        string_list_push_lit(arena, &mdfr_list, "MDFR_SHIFT");
                        has_base = true;
                    }
                    if (bind->modifiers == 0){
                        string_list_push_lit(arena, &mdfr_list, "MDFR_NONE");
                        has_base = true;
                    }
                    String_Const_char mdfr = string_list_flatten(arena, mdfr_list, StringFill_NullTerminate);
                    
                    if (bind->vanilla){
                        if (bind->modifiers == 0){
                            fprintf(out, "bind_vanilla_keys(context, %s);\n", bind->command);
                        }
                        else{
                            fprintf(out, "bind_vanilla_keys(context, %s, %s);\n", mdfr.str, bind->command);
                        }
                    }
                    else{
                        char key_str_space[16];
                        i32 size = 0;
                        char *key_str = global_key_name(bind->keycode, &size);
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
                        
                        fprintf(out, "bind(context, %s, %s, %s);\n", key_str, mdfr.str, bind->command);
                    }
                    
                    end_temp(bind_temp);
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
    
    end_temp(temp);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv){
    META_BEGIN();
    Arena arena = fm_init_system();
    generate_custom_headers(&arena);
    generate_remapping_code_and_data(&arena);
    META_FINISH();
}

// BOTTOM

