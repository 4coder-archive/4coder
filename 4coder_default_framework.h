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


//
// Projects
//

static char *default_extensions[] = {
    "cpp",
    "hpp",
    "c",
    "h",
    "cc"
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
            
            bool32 subscript_success = true;
            if (token.type == CPP_TOKEN_BRACKET_OPEN){
                subscript_success = false;
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
                                    subscript_success = true;
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
                        
                        bool32 array_success = true;
                        if (val_token.type == CPP_TOKEN_BRACE_OPEN){
                            array_success = false;
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
                                        array_success = true;
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
                            String str = make_string(item.mem + item.line.val_token.start + 1,item.line.val_token.size - 2);
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
            
            Cpp_Lex_Data S = cpp_lex_data_init(false);
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
                        config_bool_var(item, "automatically_save_changes_on_build", 0, &automatically_save_changes_on_build);
                        
                        config_int_var(item, "default_wrap_width", 0, &new_wrap_width);
                        config_int_var(item, "default_min_base_width", 0, &new_min_base_width);
                        
                        config_string_var(item, "default_theme_name", 0, &default_theme_name);
                        config_string_var(item, "default_font_name", 0, &default_font_name);
                        config_string_var(item, "user_name", 0, &user_name);
                        
                        {
                            char str_space[512];
                            String str = make_fixed_width_string(str_space);
                            if (config_string_var(item, "treat_as_code", 0, &str)){
                                set_extensions(&treat_as_code_exts, str);
                            }
                        }
                        
                        config_bool_var(item, "automatically_load_project", 0, &automatically_load_project);
                    }
                }
                adjust_all_buffer_wrap_widths(app, new_wrap_width, new_min_base_width);
                default_wrap_width = new_wrap_width;
                default_min_base_width = new_min_base_width;
            }
        }
        
        end_temp_memory(temp);
    }
    else{
        print_message(app, literal("Did not find config.4coder, using default settings\n"));
    }
}


//
// Framework Init Functions
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
default_4coder_initialize(Application_Links *app, bool32 use_scrollbars, bool32 use_file_bars){
    init_memory(app);
    process_config_file(app);
    
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
default_4coder_side_by_side_panels(Application_Links *app){
    View_Summary view = get_active_view(app, AccessAll);
    new_view_settings(app, &view);
    open_panel_vsplit(app);
    set_active_view(app, &view);
}

static void
default_4coder_one_panel(Application_Links *app){
    View_Summary view = get_active_view(app, AccessAll);
    new_view_settings(app, &view);
}

static void
default_4coder_full_width_bottom_side_by_side_panels(Application_Links *app){
    open_special_note_view(app);
    default_4coder_side_by_side_panels(app);
}

#endif

// BOTTOM

