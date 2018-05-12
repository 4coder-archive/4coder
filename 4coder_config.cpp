/*
4coder_config.cpp - Parsing *.4coder files.
*/

// TOP

// TODO(allen): Stop handling files this way!  My own API should be able to do this!!?!?!?!!?!?!!!!?
// NOTE(allen): Actually need binary buffers for some stuff to work, but not this parsing thing here.
#include <stdio.h>

////////////////////////////////

static CString_Array
get_code_extensions(Extension_List *list){
    CString_Array array = {0};
    array.strings = default_extensions;
    array.count = ArrayCount(default_extensions);
    if (list->count != 0){
        array.strings = list->exts;
        array.count = list->count;
    }
    return(array);
}

static void
parse_extension_line_to_extension_list(String str, Extension_List *list){
    int32_t mode = 0;
    int32_t j = 0, k = 0;
    for (int32_t i = 0; i < str.size; ++i){
        switch (mode){
            case 0:
            {
                if (str.str[i] == '.'){
                    mode = 1;
                    list->exts[k++] = &list->space[j];
                }
            }break;
            
            case 1:
            {
                if (str.str[i] == '.'){
                    list->space[j++] = 0;
                    list->exts[k++] = &list->space[j];
                }
                else{
                    list->space[j++] = str.str[i];
                }
            }break;
        }
    }
    list->space[j++] = 0;
    list->count = k;
}

////////////////////////////////

static void
config_parser__advance_to_next(Config_Parser *ctx){
    Cpp_Token *t = ctx->token;
    Cpp_Token *e = ctx->end;
    for (t += 1; t < e && t->type == CPP_TOKEN_COMMENT; t += 1);
    ctx->token = t;
}

static Config_Parser
make_config_parser(Partition *arena, char *file_name, String data, Cpp_Token_Array array){
    Config_Parser ctx = {0};
    ctx.start = array.tokens;
    ctx.token = ctx.start;
    ctx.end = ctx.start + array.count;
    ctx.file_name = file_name;
    ctx.data = data;
    ctx.arena = arena;
    config_parser__advance_to_next(&ctx);
    return(ctx);
}

static bool32
config_parser__recognize_token(Config_Parser *ctx, Cpp_Token_Type type){
    bool32 result = false;
    if (ctx->start <= ctx->token && ctx->token < ctx->end){
        result = (ctx->token->type == type);
    }
    else if (type == CPP_TOKEN_EOF){
        result = true;
    }
    return(result);
}

static String
config_parser__get_lexeme(Config_Parser *ctx){
    String lexeme = {0};
    if (ctx->start <= ctx->token && ctx->token < ctx->end){
         lexeme = make_string(ctx->data.str + ctx->token->start, ctx->token->size);
    }
    return(lexeme);
}

static int32_t
config_parser__get_integer(Config_Parser *ctx){
    String str = config_parser__get_lexeme(ctx);
    return(str_to_int(str));
}

static bool32
config_parser__get_boolean(Config_Parser *ctx){
    String str = config_parser__get_lexeme(ctx);
    return(match(str, "true"));
}

static bool32
config_parser__recognize_text(Config_Parser *ctx, String text){
    bool32 result = false;
    String lexeme = config_parser__get_lexeme(ctx);
    if (lexeme.str != 0 && match(lexeme, text)){
            result = true;
        }
    return(result);
}

static bool32
config_parser__match_token(Config_Parser *ctx, Cpp_Token_Type type){
    bool32 result = config_parser__recognize_token(ctx, type);
    if (result){
        config_parser__advance_to_next(ctx);
    }
    return(result);
    }

static bool32
config_parser__match_text(Config_Parser *ctx, String text){
    bool32 result = config_parser__recognize_text(ctx, text);
    if (result){
        config_parser__advance_to_next(ctx);
    }
    return(result);
}

static Config                  *config_parser__config(Config_Parser *ctx);
static int32_t                 *config_parser__version(Config_Parser *ctx);
static Config_Assignment       *config_parser__assignment(Config_Parser *ctx);
static Config_LValue           *config_parser__lvalue(Config_Parser *ctx);
static Config_RValue           *config_parser__rvalue(Config_Parser *ctx);
static Config_Compound         *config_parser__compound(Config_Parser *ctx);
static Config_Compound_Element *config_parser__element(Config_Parser *ctx);

static Config*
config_parser__config(Config_Parser *ctx){
    int32_t *version = config_parser__version(ctx);
    
    Config_Assignment *first = 0;
    Config_Assignment *last = 0;
    int32_t count = 0;
    for (;!config_parser__recognize_token(ctx, CPP_TOKEN_EOF);){
        Config_Assignment *assignment = config_parser__assignment(ctx);
        require(assignment != 0);
        zdll_push_back(first, last, assignment);
        count += 1;
    }
    
    Config *config = push_array(ctx->arena, Config, 1);
    config->version = version;
    config->first = first;
    config->last = last;
    config->count = count;
    return(config);
}

static int32_t*
config_parser__version(Config_Parser *ctx){
    require(config_parser__match_text(ctx, make_lit_string("version")));
    require(config_parser__match_token(ctx, CPP_TOKEN_PARENTHESE_OPEN));
    require(config_parser__recognize_token(ctx, CPP_TOKEN_INTEGER_CONSTANT));
    int32_t value = config_parser__get_integer(ctx);
    config_parser__advance_to_next(ctx);
    require(config_parser__match_token(ctx, CPP_TOKEN_PARENTHESE_CLOSE));
    int32_t *ptr = push_array(ctx->arena, int32_t, 1);
    *ptr = value;
    return(ptr);
}

static Config_Assignment*
config_parser__assignment(Config_Parser *ctx){
    Config_LValue *l = config_parser__lvalue(ctx);
    require(l != 0);
    Config_RValue *r = config_parser__rvalue(ctx);
require(r != 0);
    
    Config_Assignment *assignment = push_array(ctx->arena, Config_Assignment, 1);
    assignment->l = l;
    assignment->r = r;
    return(assignment);
    }

static Config_LValue*
config_parser__lvalue(Config_Parser *ctx){
    require(config_parser__recognize_token(ctx, CPP_TOKEN_IDENTIFIER));
    String identifier = config_parser__get_lexeme(ctx);
    config_parser__advance_to_next(ctx);
    
    int32_t index = 0;
    if (config_parser__match_token(ctx, CPP_TOKEN_BRACKET_OPEN)){
        require(config_parser__recognize_token(ctx, CPP_TOKEN_INTEGER_CONSTANT));
        index = config_parser__get_integer(ctx);
        config_parser__advance_to_next(ctx);
        require(config_parser__match_token(ctx, CPP_TOKEN_BRACKET_CLOSE));
    }
    
    Config_LValue *lvalue = push_array(ctx->arena, Config_LValue, 1);
    lvalue->identifier = identifier;
    lvalue->index = index;
    return(lvalue);
}

static Config_RValue*
config_parser__rvalue(Config_Parser *ctx){
    if (config_parser__recognize_token(ctx, CPP_TOKEN_IDENTIFIER)){
        Config_LValue *l = config_parser__lvalue(ctx);
        require(l != 0);
        Config_RValue *rvalue = push_array(ctx->arena, Config_RValue, 1);
        rvalue->type = ConfigRValueType_LValue;
        rvalue->lvalue = l;
        return(rvalue);
    }
    else if (config_parser__recognize_token(ctx, CPP_TOKEN_BRACE_OPEN)){
        config_parser__advance_to_next(ctx);
        Config_Compound *compound = config_parser__compound(ctx);
        require(compound != 0);
        Config_RValue *rvalue = push_array(ctx->arena, Config_RValue, 1);
        rvalue->type = ConfigRValueType_Compound;
        rvalue->compound = compound;
        return(rvalue);
    }
    else if (config_parser__recognize_token(ctx, CPP_TOKEN_BOOLEAN_CONSTANT)){
        bool32 b = config_parser__get_boolean(ctx);
        config_parser__advance_to_next(ctx);
        Config_RValue *rvalue = push_array(ctx->arena, Config_RValue, 1);
        rvalue->type = ConfigRValueType_Boolean;
        rvalue->boolean = b;
        return(rvalue);
    }
    else if (config_parser__recognize_token(ctx, CPP_TOKEN_INTEGER_CONSTANT)){
        int32_t v = config_parser__get_integer(ctx);
        config_parser__advance_to_next(ctx);
        Config_RValue *rvalue = push_array(ctx->arena, Config_RValue, 1);
        rvalue->type = ConfigRValueType_Integer;
        rvalue->integer = v;
        return(rvalue);
    }
    else if (config_parser__recognize_token(ctx, CPP_TOKEN_STRING_CONSTANT)){
        String s = config_parser__get_lexeme(ctx);
        config_parser__advance_to_next(ctx);
        char *space = push_array(ctx->arena, char, s.size + 1);
        push_align(ctx->arena, 8);
        s = substr(s, 1, s.size - 2);
        string_interpret_escapes(s, space);
        Config_RValue *rvalue = push_array(ctx->arena, Config_RValue, 1);
        rvalue->type = ConfigRValueType_String;
        rvalue->string = make_string_slowly(space);
        return(rvalue);
    }
    else if (config_parser__recognize_token(ctx, CPP_TOKEN_CHARACTER_CONSTANT)){
        String s = config_parser__get_lexeme(ctx);
        config_parser__advance_to_next(ctx);
        char *space = push_array(ctx->arena, char, s.size + 1);
        push_align(ctx->arena, 8);
        s = substr(s, 1, s.size - 2);
        string_interpret_escapes(s, space);
        Config_RValue *rvalue = push_array(ctx->arena, Config_RValue, 1);
        rvalue->type = ConfigRValueType_Character;
        rvalue->character = space[0];
        return(rvalue);
    }
    return(0);
}

static Config_Compound*
config_parser__compound(Config_Parser *ctx){
    Config_Compound_Element *first = 0;
    Config_Compound_Element *last = 0;
    int32_t count = 0;
    
    Config_Compound_Element *element = config_parser__element(ctx);
    require(element != 0);
    zdll_push_back(first, last, element);
    count += 1;
    
    for (;config_parser__match_token(ctx, CPP_TOKEN_COMMA);){
        if (config_parser__recognize_token(ctx, CPP_TOKEN_BRACE_CLOSE)){
            break;
        }
        element = config_parser__element(ctx);
        require(element != 0);
        zdll_push_back(first, last, element);
        count += 1;
    }
    
    require(config_parser__match_token(ctx, CPP_TOKEN_BRACE_CLOSE));
    
    Config_Compound *compound = push_array(ctx->arena, Config_Compound, 1);
    compound->first = first;
    compound->last = last;
    compound->count = count;
    return(compound);
    }

static Config_Compound_Element*
config_parser__element(Config_Parser *ctx){
    Config_Layout layout = {0};
    if (config_parser__match_token(ctx, CPP_TOKEN_DOT)){
        if (config_parser__recognize_token(ctx, CPP_TOKEN_INTEGER_CONSTANT)){
            layout.type = ConfigLayoutType_Identifier;
            layout.identifier = config_parser__get_lexeme(ctx);
            config_parser__advance_to_next(ctx);
            }
        else if (config_parser__recognize_token(ctx, CPP_TOKEN_IDENTIFIER)){
            layout.type = ConfigLayoutType_Integer;
            layout.integer = config_parser__get_integer(ctx);
        config_parser__advance_to_next(ctx);
        }
        else{
            return(0);
        }
        require(config_parser__match_token(ctx, CPP_TOKEN_EQ));
    }
    Config_RValue *rvalue = config_parser__rvalue(ctx);
    require(rvalue);
    Config_Compound_Element *element = push_array(ctx->arena, Config_Compound_Element, 1);
    element->l = layout;
    element->r = rvalue;
    return(element);
}

////////////////////////////////

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
            Cpp_Token skip_token = read_config_token(array, &i);
            if (skip_token.type == CPP_TOKEN_SEMICOLON){
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
    return(config_var(item, var_name, subscript, CPP_TOKEN_BOOLEAN_CONSTANT, var_out));
}

static bool32
config_int_var(Config_Item item, char *var_name, int32_t *subscript, int32_t *var_out){
    return(config_var(item, var_name, subscript, CPP_TOKEN_INTEGER_CONSTANT, var_out));
}

static bool32
config_uint_var(Config_Item item, char *var_name, int32_t *subscript, uint32_t *var_out){
    return(config_var(item, var_name, subscript, CPP_TOKEN_INTEGER_CONSTANT, var_out));
}

static bool32
config_string_var(Config_Item item, char *var_name, int32_t *subscript, String *var_out){
    return(config_var(item, var_name, subscript, CPP_TOKEN_STRING_CONSTANT, var_out));
}

static bool32
config_identifier_var(Config_Item item, char *var_name, int32_t *subscript, String *var_out){
    return(config_var(item, var_name, subscript, CPP_TOKEN_IDENTIFIER, var_out));
}

static bool32
config_array_var(Config_Item item, char *var_name, int32_t *subscript, Config_Array_Reader *array_reader){
    return(config_var(item, var_name, subscript, CPP_TOKEN_BRACE_OPEN, array_reader));
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
    return(array_reader->good);
}

////////////////////////////////

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

////////////////////////////////

static void
config_init_default(Config_Data *config){
    config->default_wrap_width = 672;
    config->default_min_base_width = 550;
    
    config->enable_code_wrapping = true;
    config->automatically_adjust_wrapping = true;
    config->automatically_indent_text_on_save = true;
    config->automatically_save_changes_on_build = true;
    config->automatically_load_project = false;
    config->lalt_lctrl_is_altgr = false;
    
    config->default_theme_name = make_fixed_width_string(config->default_theme_name_space);
    copy(&config->default_theme_name, "4coder");
    
    config->default_font_name = make_fixed_width_string(config->default_font_name_space);
    copy(&config->default_font_name, "");
    
    config->user_name = make_fixed_width_string(config->user_name_space);
    copy(&config->user_name, "");
    
    config->default_compiler_bat = make_fixed_width_string(config->default_compiler_bat_space);
    copy(&config->default_compiler_bat, "cl");
    
    config->default_flags_bat = make_fixed_width_string(config->default_flags_bat_space);
    copy(&config->default_flags_bat, "");
    
    config->default_compiler_sh = make_fixed_width_string(config->default_compiler_sh_space);
    copy(&config->default_compiler_sh, "g++");
    
    config->default_flags_sh = make_fixed_width_string(config->default_flags_sh_space);
    copy(&config->default_flags_sh, "");
    
    config->current_mapping = make_fixed_width_string(config->current_mapping_space);
    copy(&config->current_mapping, "");
    
    memset(&config->code_exts, 0, sizeof(config->code_exts));
}

static void
config_parse__data(Partition *scratch,
                    String data, Config_Data *config){
    config_init_default(config);
    
    bool32 success = false;
    Temp_Memory temp = begin_temp_memory(scratch);
    
    Cpp_Token_Array array = {0};
    array.count = 0;
    array.max_count = (1 << 20)/sizeof(Cpp_Token);
    array.tokens = push_array(scratch, Cpp_Token, array.max_count);
    
    if (array.tokens != 0){
        Cpp_Keyword_Table kw_table = {0};
        Cpp_Keyword_Table pp_table = {0};
        lexer_keywords_default_init(scratch, &kw_table, &pp_table);
        
        Cpp_Lex_Data S = cpp_lex_data_init(false, kw_table, pp_table);
        Cpp_Lex_Result result = cpp_lex_step(&S, data.str, data.size + 1, HAS_NULL_TERM, &array, NO_OUT_LIMIT);
        
        if (result == LexResult_Finished){
            success = true;
            
            for (int32_t i = 0; i < array.count; ++i){
                Config_Line config_line = read_config_line(array, &i, data.str);
                
                if (config_line.read_success){
                    Config_Item item = get_config_item(config_line, data.str, array);
                    
                    config_bool_var(item, "enable_code_wrapping", 0,
                                    &config->enable_code_wrapping);
                    config_bool_var(item, "automatically_adjust_wrapping", 0,
                                    &config->automatically_adjust_wrapping);
                    config_bool_var(item, "automatically_indent_text_on_save", 0,
                                    &config->automatically_indent_text_on_save);
                    config_bool_var(item, "automatically_save_changes_on_build", 0,
                                    &config->automatically_save_changes_on_build);
                    
                    config_int_var(item, "default_wrap_width", 0,
                                   &config->default_wrap_width);
                    config_int_var(item, "default_min_base_width", 0,
                                   &config->default_min_base_width);
                    
                    config_string_var(item, "default_theme_name", 0,
                                      &config->default_theme_name);
                    config_string_var(item, "default_font_name", 0,
                                      &config->default_font_name);
                    config_string_var(item, "user_name", 0,
                                      &config->user_name);
                    
                    config_string_var(item, "default_compiler_bat", 0,
                                      &config->default_compiler_bat);
                    config_string_var(item, "default_flags_bat", 0,
                                      &config->default_flags_bat);
                    config_string_var(item, "default_compiler_sh", 0,
                                      &config->default_compiler_sh);
                    config_string_var(item, "default_flags_sh", 0,
                                      &config->default_flags_sh);
                    
                    config_string_var(item, "mapping", 0,
                                      &config->current_mapping);
                    
                    char space[512];
                    String str = make_fixed_width_string(space);
                    if (config_string_var(item, "treat_as_code", 0, &str)){
                        parse_extension_line_to_extension_list(str, &config->code_exts);
                    }
                    
                    config_bool_var(item, "automatically_load_project", 0,
                                    &config->automatically_load_project);
                    
                    config_bool_var(item, "lalt_lctrl_is_altgr", 0,
                                    &config->lalt_lctrl_is_altgr);
                }
                }
        }
    }
    
    end_temp_memory(temp);
    
    if (!success){
        config_init_default(config);
    }
}

static void
config_parse__file_handle(Partition *scratch,
                          FILE *file, Config_Data *config){
    Temp_Memory temp = begin_temp_memory(scratch);
    String data = dump_file_handle(scratch, file);
        if (data.str != 0){
          config_parse__data(scratch, data, config);
        }
    else{
        config_init_default(config);
    }
        end_temp_memory(temp);
        }

static void
config_parse__file_name(Application_Links *app, Partition *scratch,
                        char *file_name, Config_Data *config){
    FILE *file = open_file_try_current_path_then_binary_path(app, file_name);
    if (file != 0){
        config_parse__file_handle(scratch, file, config);
        fclose(file);
    }
    else{
        print_message(app, literal("Did not find config file, using default settings\n"));
        config_init_default(config);
    }
}

static bool32
theme_parse__data(Partition *scratch, String data, Theme_Data *theme){
    bool32 success = false;
    
    Cpp_Token_Array array;
    array.count = 0;
    array.max_count = (1 << 20)/sizeof(Cpp_Token);
    array.tokens = push_array(scratch, Cpp_Token, array.max_count);
    
    Cpp_Keyword_Table kw_table = {0};
    Cpp_Keyword_Table pp_table = {0};
    lexer_keywords_default_init(scratch, &kw_table, &pp_table);
    
    Cpp_Lex_Data S = cpp_lex_data_init(false, kw_table, pp_table);
    Cpp_Lex_Result result = cpp_lex_step(&S, data.str, data.size + 1, HAS_NULL_TERM, &array, NO_OUT_LIMIT);
    
    if (result == LexResult_Finished){
        success = true;
        
        theme->name = make_fixed_width_string(theme->space);
        copy(&theme->name, "unnamed");
        init_theme_zero(&theme->theme);
        
        for (int32_t i = 0; i < array.count; ++i){
            Config_Line config_line = read_config_line(array, &i, data.str);
            if (config_line.read_success){
                Config_Item item = get_config_item(config_line, data.str, array);
                config_string_var(item, "name", 0, &theme->name);
                
                for (int32_t tag = 0; tag < ArrayCount(style_tag_names); ++tag){
                    char *name = style_tag_names[tag];
                    int_color color = 0;
                    if (config_uint_var(item, name, 0, &color)){
                        int_color *color_slot = &theme->theme.colors[tag];
                        *color_slot = color;
                    }
                    else{
                        char var_space[512];
                        String var_str = make_fixed_width_string(var_space);
                        if (config_identifier_var(item, name, 0, &var_str)){
                            for (int32_t eq_tag = 0; eq_tag < ArrayCount(style_tag_names); ++eq_tag){
                                if (match(var_str, style_tag_names[eq_tag])){
                                    int_color *color_slot = &theme->theme.colors[tag];
                                    *color_slot = theme->theme.colors[eq_tag];
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    return(success);
}

static bool32
theme_parse__file_handle(Partition *scratch, FILE *file, Theme_Data *theme){
     Temp_Memory temp = begin_temp_memory(scratch);
        String data = dump_file_handle(scratch, file);
        bool32 success = false;
        if (data.str != 0){
            success = theme_parse__data(scratch, data, theme);
        }
        end_temp_memory(temp);
        return(success);
}

static bool32
theme_parse__file_name(Application_Links *app, Partition *scratch,
                       char *file_name, Theme_Data *theme){
    bool32 success = false;
    FILE *file = open_file_try_current_path_then_binary_path(app, file_name);
    if (file != 0){
        success = theme_parse__file_handle(scratch, file, theme);
        fclose(file);
    }
    else{
        char space[256];
        String str = make_fixed_width_string(space);
        append(&str, "Did not find ");
        append(&str, file_name);
        append(&str, ", color scheme not loaded");
        print_message(app, str.str, str.size);
    }
    
    return(success);
}

////////////////////////////////

static void
load_config_and_apply(Application_Links *app, Partition *scratch, Config_Data *config){
    config_parse__file_name(app, scratch, "config.4coder", config);
    change_mapping(app, config->current_mapping);
    adjust_all_buffer_wrap_widths(app, config->default_wrap_width, config->default_min_base_width);
    global_set_setting(app, GlobalSetting_LAltLCtrlIsAltGr, config->lalt_lctrl_is_altgr);
}

static void
load_theme_file_into_live_set(Application_Links *app, Partition *scratch, char *file_name){
    Theme_Data theme = {0};
    theme_parse__file_name(app, scratch, file_name, &theme);
    create_theme(app, &theme.theme, theme.name.str, theme.name.size);
}

static void
load_folder_of_themes_into_live_set(Application_Links *app, Partition *scratch,
                   char *folder_name){
    char path_space[512];
    String path = make_fixed_width_string(path_space);
    path.size = get_4ed_path(app, path_space, sizeof(path_space));
    append(&path, folder_name);
    
    if (path.size < path.memory_size){
        File_List list = get_file_list(app, path.str, path.size);
        for (uint32_t i = 0; i < list.count; ++i){
            File_Info *info = &list.infos[i];
            if (info->folder) continue;
            String info_file_name = make_string(info->filename, info->filename_len);
            if (!match(file_extension(info_file_name), "4coder")) continue;
            char file_name_space[512];
            String file_name = make_fixed_width_string(file_name_space);
            copy(&file_name, path);
            append(&file_name, "/");
            append(&file_name, info_file_name);
            if (terminate_with_null(&file_name)){
                load_theme_file_into_live_set(app, scratch, file_name.str);
            }
            }
        free_file_list(app, list);
    }
}

// BOTTOM

