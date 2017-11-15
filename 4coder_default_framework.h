/*
4coder_default_framework.cpp - Sets up the basics of the framework that is used for default 4coder behaviour.

TYPE: 'internal-for-default-system'
*/

// TOP

#if !defined(FCODER_DEFAULT_FRAMEWORK_H)
#define FCODER_DEFAULT_FRAMEWORK_H

#include "4coder_base_commands.cpp"

#include "4coder_helper/4coder_helper.h"
#include "4coder_lib/4coder_mem.h"
#include "4cpp/4cpp_lexer.h"

//
// Command Maps
//

enum Default_Maps{
    default_code_map,
    default_maps_count
};

//
// Global Memory
//

static Partition global_part;
static General_Memory global_general;

//
// Jump Buffer Locking
//

#if !defined(AUTO_CENTER_AFTER_JUMPS)
static bool32 auto_center_after_jumps = true;
#else
static bool32 auto_center_after_jumps = AUTO_CENTER_AFTER_JUMPS;
#endif

static char locked_buffer_space[256];
static String locked_buffer = make_fixed_width_string(locked_buffer_space);

static void
unlock_jump_buffer(){
    locked_buffer.size = 0;
}

static void
lock_jump_buffer(char *name, int32_t size){
    if (size <= locked_buffer.memory_size){
        copy(&locked_buffer, make_string(name, size));
    }
}

static void
lock_jump_buffer(Buffer_Summary buffer){
    lock_jump_buffer(buffer.buffer_name, buffer.buffer_name_len);
}

static View_Summary
get_view_for_locked_jump_buffer(Application_Links *app){
    View_Summary view = {0};
    
    if (locked_buffer.size > 0){
        Buffer_Summary buffer = get_buffer_by_name(app, locked_buffer.str, locked_buffer.size, AccessAll);
        if (buffer.exists){
            view = get_first_view_with_buffer(app, buffer.buffer_id);
        }
        else{
            unlock_jump_buffer();
        }
    }
    
    return(view);
}

//
// Panel Management
//

static View_ID special_note_view_id = 0;

static bool32 default_use_scrollbars = false;
static bool32 default_use_file_bars = true;

static void
new_view_settings(Application_Links *app, View_Summary *view){
    if (!default_use_scrollbars){
        view_set_setting(app, view, ViewSetting_ShowScrollbar, false);
    }
    if (!default_use_file_bars){
        view_set_setting(app, view, ViewSetting_ShowFileBar, false);
    }
}

static void
close_special_note_view(Application_Links *app){
    View_Summary special_view = get_view(app, special_note_view_id, AccessAll);
    if (special_view.exists){
        close_view(app, &special_view);
    }
    special_note_view_id = 0;
}

static View_Summary
open_special_note_view(Application_Links *app, bool32 create_if_not_exist = true){
    View_Summary special_view = get_view(app, special_note_view_id, AccessAll);
    
    if (create_if_not_exist && !special_view.exists){
        View_Summary view = get_active_view(app, AccessAll);
        special_view = open_view(app, &view, ViewSplit_Bottom);
        new_view_settings(app, &special_view);
        view_set_split_proportion(app, &special_view, .2f);
        set_active_view(app, &view);
        special_note_view_id = special_view.view_id;
    }
    
    return(special_view);
}

CUSTOM_COMMAND_SIG(change_active_panel){
    View_Summary view = get_active_view(app, AccessAll);
    View_ID original_view_id = view.view_id;
    
    do{
        get_view_next_looped(app, &view, AccessAll);
        if (view.view_id != special_note_view_id){
            break;
        }
    }while(view.view_id != original_view_id);
    
    if (view.exists){
        set_active_view(app, &view);
    }
}

CUSTOM_COMMAND_SIG(change_active_panel_backwards){
    View_Summary view = get_active_view(app, AccessAll);
    View_ID original_view_id = view.view_id;
    
    do{
        get_view_prev_looped(app, &view, AccessAll);
        if (view.view_id != special_note_view_id){
            break;
        }
    }while(view.view_id != original_view_id);
    
    if (view.exists){
        set_active_view(app, &view);
    }
}

CUSTOM_COMMAND_SIG(open_panel_vsplit){
    View_Summary view = get_active_view(app, AccessAll);
    View_Summary new_view = open_view(app, &view, ViewSplit_Right);
    new_view_settings(app, &new_view);
}

CUSTOM_COMMAND_SIG(open_panel_hsplit){
    View_Summary view = get_active_view(app, AccessAll);
    View_Summary new_view = open_view(app, &view, ViewSplit_Bottom);
    new_view_settings(app, &new_view);
}

//
// View Variabls
//

enum Rewrite_Type{
    RewriteNone,
    RewritePaste,
    RewriteWordComplete
};

struct View_Paste_Index{
    int32_t rewrite;
    int32_t next_rewrite;
    int32_t index;
};

View_Paste_Index view_paste_index_[16];
View_Paste_Index *view_paste_index = view_paste_index_ - 1;


//
// System Buffer Names
//

static char out_buffer_space[1024];
static char command_space[1024];
static char hot_directory_space[1024];


//
// Mouse Suppression
//

static bool32 suppressing_mouse = false;

static void
set_mouse_suppression(Application_Links *app, int32_t suppress){
    if (suppress){
        suppressing_mouse = 1;
        show_mouse_cursor(app, MouseCursorShow_Never);
    }
    else{
        suppressing_mouse = 0;
        show_mouse_cursor(app, MouseCursorShow_Always);
    }
}

CUSTOM_COMMAND_SIG(suppress_mouse){
    set_mouse_suppression(app, true);
}

CUSTOM_COMMAND_SIG(allow_mouse){
    set_mouse_suppression(app, false);
}

CUSTOM_COMMAND_SIG(toggle_mouse){
    set_mouse_suppression(app, !suppressing_mouse);
}

CUSTOM_COMMAND_SIG(toggle_fullscreen){
    set_fullscreen(app, !is_fullscreen(app));
}


//
// Projects
//

static char *default_extensions[] = {
    "cpp",
    "hpp",
    "c",
    "h",
    "cc",
    "cs"
};

struct Extension_List{
    char extension_space[256];
    char *extensions[94];
    int32_t extension_count;
};

static void
set_extensions(Extension_List *extension_list, String src){
    int32_t mode = 0;
    int32_t j = 0, k = 0;
    for (int32_t i = 0; i < src.size; ++i){
        switch (mode){
            case 0:
            {
                if (src.str[i] == '.'){
                    mode = 1;
                    extension_list->extensions[k++] = &extension_list->extension_space[j];
                }
            }break;
            
            case 1:
            {
                if (src.str[i] == '.'){
                    extension_list->extension_space[j++] = 0;
                    extension_list->extensions[k++] = &extension_list->extension_space[j];
                }
                else{
                    extension_list->extension_space[j++] = src.str[i];
                }
            }break;
        }
    }
    extension_list->extension_space[j++] = 0;
    extension_list->extension_count = k;
}

struct Fkey_Command{
    char command[128];
    char out[128];
    bool32 use_build_panel;
    bool32 save_dirty_buffers;
};

struct Project{
    char dir_space[256];
    char *dir;
    int32_t dir_len;
    
    Extension_List extension_list;
    Fkey_Command fkey_commands[16];
    
    bool32 close_all_code_when_this_project_closes;
    bool32 close_all_files_when_project_opens;
    
    bool32 open_recursively;
    
    bool32 loaded;
};

static Project null_project = {0};
static Project current_project = {0};

static char**
get_current_project_extensions(int32_t *extension_count_out){
    char **extension_list = default_extensions;
    int32_t extension_count = ArrayCount(default_extensions);
    if (current_project.dir != 0){
        extension_list = current_project.extension_list.extensions;
        extension_count = current_project.extension_list.extension_count;
    }
    *extension_count_out = extension_count;
    return(extension_list);
}


//
// Location Jumping State
//

struct ID_Based_Jump_Location{
    int32_t buffer_id;
    int32_t line;
    int32_t column;
};

static ID_Based_Jump_Location null_location = {0};
static ID_Based_Jump_Location prev_location = {0};


//
// Config File Parsing
//

struct Config_Line{
    Cpp_Token id_token;
    Cpp_Token subscript_token;
    Cpp_Token eq_token;
    Cpp_Token val_token;
    int32_t val_array_start;
    int32_t val_array_end;
    int32_t val_array_count;
    String error_str;
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
read_config_line(Cpp_Token_Array array, int32_t *i_ptr, char *text){
    Config_Line config_line = {0};
    
    int32_t i = *i_ptr;
    
    config_line.id_token = read_config_token(array, &i);
    int32_t text_index_start = config_line.id_token.start;
    if (config_line.id_token.type == CPP_TOKEN_IDENTIFIER){
        ++i;
        if (i < array.count){
            Cpp_Token token = read_config_token(array, &i);
            
            bool32 lvalue_success = true;
            if (token.type == CPP_TOKEN_BRACKET_OPEN){
                lvalue_success = false;
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
                                    lvalue_success = true;
                                }
                            }
                        }
                    }
                }
            }
            
            if (lvalue_success){
                if (token.type == CPP_TOKEN_EQ){
                    config_line.eq_token = read_config_token(array, &i);
                    ++i;
                    if (i < array.count){
                        Cpp_Token val_token = read_config_token(array, &i);
                        
                        bool32 rvalue_success = true;
                        if (val_token.type == CPP_TOKEN_BRACE_OPEN){
                            rvalue_success = false;
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
                                        rvalue_success = true;
                                        break;
                                    }
                                    else{
                                        if (array_token.type == CPP_TOKEN_COMMA){
                                            if (!expecting_array_item){
                                                expecting_array_item = true;
                                            }
                                            else{
                                                break;
                                            }
                                        }
                                        else{
                                            if (expecting_array_item){
                                                expecting_array_item = false;
                                                ++config_line.val_array_count;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        
                        if (rvalue_success){
                            config_line.val_token = val_token;
                            ++i;
                            if (i < array.count){
                                Cpp_Token semicolon_token = read_config_token(array, &i);
                                if (semicolon_token.type == CPP_TOKEN_SEMICOLON){
                                    config_line.read_success = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    if (!config_line.read_success){
        Cpp_Token token = {0};
        if (i < array.count){
            token = array.tokens[i];
        }
        int32_t text_index_current = token.start + token.size;
        if (text_index_current <= text_index_start){
            if (array.count > 0){
                token = array.tokens[array.count - 1];
                text_index_current = token.start + token.size;
            }
        }
        
        if (text_index_current > text_index_start){
            config_line.error_str = make_string(text + text_index_start, text_index_current - text_index_start);
        }
        
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
    bool32 result = false;
    bool32 subscript_success = true;
    if (item.line.val_token.type == token_type){
        if ((var_name == 0 && item.id.size == 0) || match(item.id, var_name)){
            if (subscript){
                if (item.has_subscript){
                    *subscript = item.subscript_index;
                }
                else{
                    subscript_success = false;
                }
            }
            
            if (subscript_success){
                if (var_out){
                    switch (token_type){
                        case CPP_TOKEN_BOOLEAN_CONSTANT:
                        {
                            *(bool32*)var_out = (item.mem[item.line.val_token.start] == 't');
                        }break;
                        
                        case CPP_TOKEN_INTEGER_CONSTANT:
                        {
                            if (match(make_string(item.mem + item.line.val_token.start, 2), "0x")){
                                // Hex Integer
                                String val = make_string(item.mem + item.line.val_token.start + 2, item.line.val_token.size - 2);
                                *(uint32_t*)var_out = hexstr_to_int(val);
                            }
                            else{
                                // Integer
                                String val = make_string(item.mem + item.line.val_token.start, item.line.val_token.size);
                                *(int32_t*)var_out = str_to_int(val);
                            }
                        }break;
                        
                        case CPP_TOKEN_STRING_CONSTANT:
                        {
                            String str = make_string(item.mem + item.line.val_token.start + 1,item.line.val_token.size - 2);
                            copy((String*)var_out, str);
                        }break;
                        
                        case CPP_TOKEN_IDENTIFIER:
                        {
                            String str = make_string(item.mem + item.line.val_token.start,item.line.val_token.size);
                            copy((String*)var_out, str);
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
                result = true;
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
config_uint_var(Config_Item item, char *var_name, int32_t *subscript, uint32_t *var_out){
    bool32 result = config_var(item, var_name, subscript, CPP_TOKEN_INTEGER_CONSTANT, var_out);
    return(result);
}

static bool32
config_string_var(Config_Item item, char *var_name, int32_t *subscript, String *var_out){
    bool32 result = config_var(item, var_name, subscript, CPP_TOKEN_STRING_CONSTANT, var_out);
    return(result);
}

static bool32
config_identifier_var(Config_Item item, char *var_name, int32_t *subscript, String *var_out){
    bool32 result = config_var(item, var_name, subscript, CPP_TOKEN_IDENTIFIER, var_out);
    return(result);
}

static bool32
config_array_var(Config_Item item, char *var_name, int32_t *subscript, Config_Array_Reader *array_reader){
    bool32 result = config_var(item, var_name, subscript, CPP_TOKEN_BRACE_OPEN, array_reader);
    return(result);
}

static bool32
config_array_next_item(Config_Array_Reader *array_reader, Config_Item *item){
    bool32 result = false;
    
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
                result = true;
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


//
// Lexer Helper
//

static void
lexer_keywords_default_init(Partition *part, Cpp_Keyword_Table *kw_out, Cpp_Keyword_Table *pp_out){
    umem_4tech kw_size = cpp_get_table_memory_size_default(CPP_TABLE_KEYWORDS);
    umem_4tech pp_size = cpp_get_table_memory_size_default(CPP_TABLE_PREPROCESSOR_DIRECTIVES);
    
    void *kw_mem = push_block(part, (i32_4tech)kw_size);
    void *pp_mem = push_block(part, (i32_4tech)pp_size);
    
    *kw_out = cpp_make_table_default(CPP_TABLE_KEYWORDS, kw_mem, kw_size);
    *pp_out = cpp_make_table_default(CPP_TABLE_PREPROCESSOR_DIRECTIVES, pp_mem, pp_size);
}


//
// Dynamic Mapping Changes
//

struct Named_Mapping{
    String name;
    Custom_Command_Function *remap_command;
};

static Named_Mapping *named_maps = 0;
static int32_t named_map_count = 0;

static void
change_mapping(Application_Links *app, String mapping){
    bool32 did_remap = false;
    for (int32_t i = 0; i < named_map_count; ++i){
        if (match(mapping, named_maps[i].name)){
            did_remap = true;
            exec_command(app, named_maps[i].remap_command);
            break;
        }
    }
    if (!did_remap){
        print_message(app, literal("Leaving bindings unaltered.\n"));
    }
}

CUSTOM_COMMAND_SIG(remap_interactive){
    Query_Bar bar = {0};
    char space[1024];
    bar.prompt = make_lit_string("Map Name: ");
    bar.string = make_fixed_width_string(space);
    
    if (!query_user_string(app, &bar)) return;
    
    change_mapping(app, bar.string);
}


//
// Configuration
//

static bool32 enable_code_wrapping = true;

static bool32 automatically_adjust_wrapping = true;
static bool32 automatically_indent_text_on_save = true;
static bool32 automatically_save_changes_on_build = true;

static int32_t default_wrap_width = 672;
static int32_t default_min_base_width = 550;

static char default_theme_name_space[256] = {0};
static String default_theme_name = make_fixed_width_string(default_theme_name_space);

static char default_font_name_space[256] = {0};
static String default_font_name = make_fixed_width_string(default_font_name_space);

static char user_name_space[256] = {0};
static String user_name = make_fixed_width_string(user_name_space);

static Extension_List treat_as_code_exts = {0};

static bool32 automatically_load_project = false;

static char default_compiler_bat_space[256];
static String default_compiler_bat = make_fixed_width_string(default_compiler_bat_space);

static char default_flags_bat_space[1024];
static String default_flags_bat = make_fixed_width_string(default_flags_bat_space);

static char default_compiler_sh_space[256];
static String default_compiler_sh = make_fixed_width_string(default_compiler_sh_space);

static char default_flags_sh_space[1024];
static String default_flags_sh = make_fixed_width_string(default_flags_sh_space);

static bool32
get_current_name(char **name_out, int32_t *len_out){
    bool32 result = false;
    *name_out = 0;
    if (user_name.str[0] != 0){
        *name_out = user_name.str;
        *len_out = user_name.size;
        result = true;
    }
    return(result);
}

static String
get_default_theme_name(){
    String str = default_theme_name;
    if (str.size == 0){
        str = make_lit_string("4coder");
    }
    return(str);
}

static String
get_default_font_name(){
    String str = default_font_name;
    if (str.size == 0){
        str = make_lit_string("Liberation Mono");
    }
    return(str);
}

static char**
get_current_code_extensions(int32_t *extension_count_out){
    char **extension_list = default_extensions;
    int32_t extension_count = ArrayCount(default_extensions);
    if (treat_as_code_exts.extension_count != 0){
        extension_list = treat_as_code_exts.extensions;
        extension_count = treat_as_code_exts.extension_count;
    }
    *extension_count_out = extension_count;
    return(extension_list);
}

// TODO(allen): Stop handling files this way!  My own API should be able to do this!!?!?!?!!?!?!!!!?
// NOTE(allen): Actually need binary buffers for some stuff to work, but not this parsing thing here.
#include <stdio.h>

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
    
    static bool32 has_initialized = false;
    if (!has_initialized){
        has_initialized = true;
        copy(&default_compiler_bat, "cl");
        copy(&default_flags_bat, "");
        copy(&default_compiler_sh, "g++");
        copy(&default_flags_bat, "");
    }
    
    if (file == 0){
        char space[256];
        int32_t size = get_4ed_path(app, space, sizeof(space));
        String str = make_string_cap(space, size, sizeof(space));
        append_sc(&str, "/config.4coder");
        terminate_with_null(&str);
        file = fopen(str.str, "rb");
    }
    
    if (file != 0){
        Temp_Memory temp = begin_temp_memory(part);
        
        char *mem = 0;
        int32_t size = 0;
        bool32 file_read_success = file_handle_dump(part, file, &mem, &size);
        
        if (file_read_success){
            fclose(file);
            
            Cpp_Token_Array array = {0};
            array.count = 0;
            array.max_count = (1 << 20)/sizeof(Cpp_Token);
            array.tokens = push_array(part, Cpp_Token, array.max_count);
            
            if (array.tokens != 0){
                Cpp_Keyword_Table kw_table = {0};
                Cpp_Keyword_Table pp_table = {0};
                lexer_keywords_default_init(part, &kw_table, &pp_table);
                
                Cpp_Lex_Data S = cpp_lex_data_init(false, kw_table, pp_table);
                Cpp_Lex_Result result = cpp_lex_step(&S, mem, size + 1, HAS_NULL_TERM, &array, NO_OUT_LIMIT);
                
                if (result == LexResult_Finished){
                    int32_t new_wrap_width = default_wrap_width;
                    int32_t new_min_base_width = default_min_base_width;
                    bool32 lalt_lctrl_is_altgr = false;
                    
                    for (int32_t i = 0; i < array.count; ++i){
                        Config_Line config_line = read_config_line(array, &i, mem);
                        
                        if (config_line.read_success){
                            Config_Item item = get_config_item(config_line, mem, array);
                            
                            config_bool_var(item, "enable_code_wrapping", 0, &enable_code_wrapping);
                            config_bool_var(item, "automatically_adjust_wrapping", 0, &automatically_adjust_wrapping);
                            config_bool_var(item, "automatically_indent_text_on_save", 0, &automatically_indent_text_on_save);
                            config_bool_var(item, "automatically_save_changes_on_build", 0, &automatically_save_changes_on_build);
                            
                            config_int_var(item, "default_wrap_width", 0, &new_wrap_width);
                            config_int_var(item, "default_min_base_width", 0, &new_min_base_width);
                            
                            config_string_var(item, "default_theme_name", 0, &default_theme_name);
                            config_string_var(item, "default_font_name", 0, &default_font_name);
                            config_string_var(item, "user_name", 0, &user_name);
                            
                            config_string_var(item, "default_compiler_bat", 0, &default_compiler_bat);
                            config_string_var(item, "default_flags_bat", 0, &default_flags_bat);
                            config_string_var(item, "default_compiler_sh", 0, &default_compiler_sh);
                            config_string_var(item, "default_flags_sh", 0, &default_flags_sh);
                            
                            char str_space[512];
                            String str = make_fixed_width_string(str_space);
                            if (config_string_var(item, "mapping", 0, &str)){
                                change_mapping(app, str);
                            }
                            
                            if (config_string_var(item, "treat_as_code", 0, &str)){
                                set_extensions(&treat_as_code_exts, str);
                            }
                            
                            config_bool_var(item, "automatically_load_project", 0, &automatically_load_project);
                            
                            config_bool_var(item, "lalt_lctrl_is_altgr", 0, &lalt_lctrl_is_altgr);
                        }
                        else if (config_line.error_str.str != 0){
                            char space[2048];
                            String str = make_fixed_width_string(space);
                            copy(&str, "WARNING: bad syntax in 4coder.config at ");
                            append(&str, config_line.error_str);
                            append(&str, "\n");
                            print_message(app, str.str, str.size);
                        }
                    }
                    
                    adjust_all_buffer_wrap_widths(app, new_wrap_width, new_min_base_width);
                    default_wrap_width = new_wrap_width;
                    default_min_base_width = new_min_base_width;
                    global_set_setting(app, GlobalSetting_LAltLCtrlIsAltGr, lalt_lctrl_is_altgr);
                }
            }
            else{
                print_message(app, literal("Ran out of memory processing config.4coder\n"));
            }
        }
        
        end_temp_memory(temp);
    }
    else{
        print_message(app, literal("Did not find config.4coder, using default settings\n"));
    }
}

//
// Color Theme
//

static void
load_color_theme_file(Application_Links *app, char *file_name){
    Partition *part = &global_part;
    FILE *file = fopen(file_name, "rb");
    
    if (!file){
        char space[256];
        int32_t size = get_4ed_path(app, space, sizeof(space));
        String str = make_string_cap(space, size, sizeof(space));
        append_sc(&str, "/");
        append_sc(&str, file_name);
        terminate_with_null(&str);
        file = fopen(str.str, "rb");
    }
    
    if (file != 0){
        Temp_Memory temp = begin_temp_memory(part);
        
        char *mem = 0;
        int32_t size = 0;
        bool32 file_read_success = file_handle_dump(part, file, &mem, &size);
        fclose(file);
        bool32 success = false;
        
        if (file_read_success){
            Cpp_Token_Array array;
            array.count = 0;
            array.max_count = (1 << 20)/sizeof(Cpp_Token);
            array.tokens = push_array(&global_part, Cpp_Token, array.max_count);
            
            Cpp_Keyword_Table kw_table = {0};
            Cpp_Keyword_Table pp_table = {0};
            lexer_keywords_default_init(part, &kw_table, &pp_table);
            
            Cpp_Lex_Data S = cpp_lex_data_init(false, kw_table, pp_table);
            Cpp_Lex_Result result = cpp_lex_step(&S, mem, size + 1, HAS_NULL_TERM, &array, NO_OUT_LIMIT);
            
            if (result == LexResult_Finished){
                success = true;
                
                char name_space[512];
                String name_str = make_fixed_width_string(name_space);
                Theme theme;
                init_theme_zero(&theme);
                
                for (int32_t i = 0; i < array.count; ++i){
                    Config_Line config_line = read_config_line(array, &i, mem);
                    if (config_line.read_success){
                        Config_Item item = get_config_item(config_line, mem, array);
                        config_string_var(item, "name", 0, &name_str);
                        
                        for (int32_t tag = 0; tag < ArrayCount(style_tag_names); ++tag){
                            char *name = style_tag_names[tag];
                            int_color color = 0;
                            if (config_uint_var(item, name, 0, &color)){
                                int_color *color_slot = &theme.colors[tag];
                                *color_slot = color;
                            }
                            else{
                                char var_space[512];
                                String var_str = make_fixed_width_string(var_space);
                                if (config_identifier_var(item, name, 0, &var_str)){
                                    for (int32_t eq_tag = 0; eq_tag < ArrayCount(style_tag_names); ++eq_tag){
                                        if (match(var_str, style_tag_names[eq_tag])){
                                            int_color *color_slot = &theme.colors[tag];
                                            *color_slot = theme.colors[eq_tag];
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else if (config_line.error_str.str != 0){
                        char space[2048];
                        String str = make_fixed_width_string(space);
                        copy(&str, "WARNING: bad syntax in 4coder.config at ");
                        append(&str, config_line.error_str);
                        append(&str, "\n");
                        print_message(app, str.str, str.size);
                    }
                }
                
                if (name_str.size == 0){
                    copy(&name_str, file_name);
                }
                
                create_theme(app, &theme, name_str.str, name_str.size);
            }
        }
        end_temp_memory(temp);
        
        if (!success){
            char space[256];
            String str = make_fixed_width_string(space);
            append_sc(&str, "Could not parse ");
            append_sc(&str, file_name);
            append_sc(&str, ", color scheme not loaded");
            print_message(app, str.str, str.size);
        }
    }
    else{
        char space[256];
        String str = make_fixed_width_string(space);
        append_sc(&str, "Did not find ");
        append_sc(&str, file_name);
        append_sc(&str, ", color scheme not loaded");
        print_message(app, str.str, str.size);
    }
}

static void
load_themes_folder(Application_Links *app){
    char folder_name_space[512];
    String folder_name = make_fixed_width_string(folder_name_space);
    folder_name.size = get_4ed_path(app, folder_name_space, sizeof(folder_name_space));
    append(&folder_name, "themes");
    
    if (folder_name.size < folder_name.memory_size){
        File_List list = get_file_list(app, folder_name.str, folder_name.size);
        for (uint32_t i = 0; i < list.count; ++i){
            File_Info *info = &list.infos[i];
            if (!info->folder){
                char file_name_space[512];
                String file_name = make_fixed_width_string(file_name_space);
                copy(&file_name, folder_name);
                append(&file_name, "/");
                append(&file_name, make_string(info->filename, info->filename_len));
                if (file_name.size < file_name.memory_size){
                    terminate_with_null(&file_name);
                    load_color_theme_file(app, file_name.str);
                }
            }
        }
        free_file_list(app, list);
    }
}

//
// Framework Init Functions
//

static void
init_memory(Application_Links *app){
    int32_t part_size = (32 << 20);
    int32_t general_size = (4 << 20);
    
    void *part_mem = memory_allocate(app, part_size);
    global_part = make_part(part_mem, part_size);
    
    void *general_mem = memory_allocate(app, general_size);
    general_memory_open(&global_general, general_mem, general_size);
}

static void
default_4coder_initialize(Application_Links *app, bool32 use_scrollbars, bool32 use_file_bars){
    init_memory(app);
    process_config_file(app);
    load_themes_folder(app);
    
    String theme = get_default_theme_name();
    String font = get_default_font_name();
    
    change_theme(app, theme.str, theme.size);
    change_font(app, font.str, font.size, 1);
    
    default_use_scrollbars = use_scrollbars;
    default_use_file_bars = use_file_bars;
}

static void
default_4coder_initialize(Application_Links *app){
    default_4coder_initialize(app, false, true);
}

static void
default_4coder_side_by_side_panels(Application_Links *app, Buffer_Identifier left_buffer, Buffer_Identifier right_buffer){
    Buffer_ID left_id = buffer_identifier_to_id(app, left_buffer);
    Buffer_ID right_id = buffer_identifier_to_id(app, right_buffer);
    
    // Left Panel
    View_Summary view = get_active_view(app, AccessAll);
    new_view_settings(app, &view);
    view_set_buffer(app, &view, left_id, 0);
    
    // Right Panel
    open_panel_vsplit(app);
    View_Summary right_view = get_active_view(app, AccessAll);
    view_set_buffer(app, &right_view, right_id, 0);
    
    // Restore Active to Left
    set_active_view(app, &view);
}

static void
default_4coder_side_by_side_panels(Application_Links *app, char **command_line_files, int32_t file_count){
    Buffer_Identifier left = buffer_identifier(literal("*scratch*"));
    Buffer_Identifier right = buffer_identifier(literal("*messages*"));
    
    if (file_count > 0){
        char *left_name = command_line_files[0];
        int32_t left_len = str_size(left_name);
        left = buffer_identifier(left_name, left_len);
        
        if (file_count > 1){
            char *right_name = command_line_files[1];
            int32_t right_len = str_size(right_name);
            right = buffer_identifier(right_name, right_len);
        }
    }
    
    default_4coder_side_by_side_panels(app, left, right);
}

static void
default_4coder_side_by_side_panels(Application_Links *app){
    default_4coder_side_by_side_panels(app, 0, 0);
}

static void
default_4coder_one_panel(Application_Links *app, Buffer_Identifier buffer){
    Buffer_ID id = buffer_identifier_to_id(app, buffer);
    
    View_Summary view = get_active_view(app, AccessAll);
    new_view_settings(app, &view);
    view_set_buffer(app, &view, id, 0);
}

static void
default_4coder_one_panel(Application_Links *app, char **command_line_files, int32_t file_count){
    Buffer_Identifier buffer = buffer_identifier(literal("*messages*"));
    
    if (file_count > 0){
        char *name = command_line_files[0];
        int32_t len = str_size(name);
        buffer = buffer_identifier(name, len);
    }
    
    default_4coder_one_panel(app, buffer);
}

static void
default_4coder_one_panel(Application_Links *app){
    default_4coder_one_panel(app, 0, 0);
}

#endif

// BOTTOM

