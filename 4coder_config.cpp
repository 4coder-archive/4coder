/*
4coder_config.cpp - Parsing *.4coder files.
*/

// TOP

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

static Error_Location
get_error_location(char *base, char *pos){
    Error_Location location = {0};
    location.line_number = 1;
    location.column_number = 1;
    for (char *ptr = base;
         ptr < pos;
         ptr += 1){
        if (*ptr == '\n'){
            location.line_number += 1;
            location.column_number = 1;
        }
        else{
            location.column_number += 1;
        }
    }
    return(location);
}

static String
config_stringize_errors(Partition *arena, Config *parsed){
    String result = {0};
    if (parsed->errors.first != 0){
        result.str = push_array(arena, char, 0);
        result.memory_size = partition_remaining(arena);
        for (Config_Error *error = parsed->errors.first;
             error != 0;
             error = error->next){
            Error_Location location = get_error_location(parsed->data.str, error->pos);
            append(&result, error->file_name);
            append(&result, ":");
            append_int_to_str(&result, location.line_number);
            append(&result, ":");
            append_int_to_str(&result, location.column_number);
            append(&result, ": ");
            append(&result, error->text);
            append(&result, "\n");
        }
        result.memory_size = result.size;
        push_array(arena, char, result.size);
    }
    return(result);
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
make_config_parser(Partition *arena, String file_name, String data, Cpp_Token_Array array){
    Config_Parser ctx = {0};
    ctx.start = array.tokens;
    ctx.token = ctx.start - 1;
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

static Config_Integer
config_parser__get_int(Config_Parser *ctx){
    Config_Integer config_integer = {0};
    String str = config_parser__get_lexeme(ctx);
    if (match(substr(str, 0, 2), "0x")){
        config_integer.is_signed = false;
        config_integer.uinteger = hexstr_to_int(substr_tail(str, 2));
    }
    else{
        config_integer.is_signed = true;
        config_integer.integer = str_to_int(str);
    }
    return(config_integer);
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

static Config                  *config_parser__config    (Config_Parser *ctx);
static int32_t                 *config_parser__version   (Config_Parser *ctx);
static Config_Assignment       *config_parser__assignment(Config_Parser *ctx);
static Config_LValue           *config_parser__lvalue    (Config_Parser *ctx);
static Config_RValue           *config_parser__rvalue    (Config_Parser *ctx);
static Config_Compound         *config_parser__compound  (Config_Parser *ctx);
static Config_Compound_Element *config_parser__element   (Config_Parser *ctx);

static Config*
text_data_and_token_array_to_parse_data(Partition *arena, String file_name, String data, Cpp_Token_Array array){
    Temp_Memory restore_point = begin_temp_memory(arena);
    Config_Parser ctx = make_config_parser(arena, file_name, data, array);
    Config *config = config_parser__config(&ctx);
    if (config == 0){
        end_temp_memory(restore_point);
    }
    return(config);
}

// TODO(allen): Move to string library
static String
config_begin_string(Partition *arena){
    String str;
    str.str = push_array(arena, char, 0);
    str.size = 0;
    str.memory_size = arena->max - arena->pos;
    return(str);
}

static void
config_end_string(Partition *arena, String *str){
    str->memory_size = str->size;
    push_array(arena, char, str->size);
}

static Config_Error*
config_error_push(Partition *arena, Config_Error_List *list, String file_name, char *pos, char *error_text){
    Config_Error *error = push_array(arena, Config_Error, 1);
    zdll_push_back(list->first, list->last, error);
    list->count += 1;
    error->file_name = file_name;
    error->pos = pos;
    error->text = config_begin_string(arena);
    append(&error->text, error_text);
    config_end_string(arena, &error->text);
    return(error);
}

static char*
config_parser__get_pos(Config_Parser *ctx){
    return(ctx->data.str + ctx->token->start);
}

static void
config_parser__log_error_pos(Config_Parser *ctx, char *pos, char *error_text){
    config_error_push(ctx->arena, &ctx->errors, ctx->file_name, pos, error_text);
}

static void
config_parser__log_error(Config_Parser *ctx, char *error_text){
    config_parser__log_error_pos(ctx, config_parser__get_pos(ctx), error_text);
}

static Config*
config_parser__config(Config_Parser *ctx){
    int32_t *version = config_parser__version(ctx);
    
    Config_Assignment *first = 0;
    Config_Assignment *last = 0;
    int32_t count = 0;
    for (;!config_parser__recognize_token(ctx, CPP_TOKEN_EOF);){
        Config_Assignment *assignment = config_parser__assignment(ctx);
        if (assignment != 0){
            zdll_push_back(first, last, assignment);
            count += 1;
        }
    }
    
    Config *config = push_array(ctx->arena, Config, 1);
    memset(config, 0, sizeof(*config));
    config->version = version;
    config->first = first;
    config->last = last;
    config->count = count;
    config->errors = ctx->errors;
    config->file_name = ctx->file_name;
    config->data = ctx->data;
    return(config);
}

static void
config_parser__recover_parse(Config_Parser *ctx){
    for (;;){
        if (config_parser__match_token(ctx, CPP_TOKEN_SEMICOLON)){
            break;
        }
        if (config_parser__recognize_token(ctx, CPP_TOKEN_EOF)){
            break;
        }
        config_parser__advance_to_next(ctx);
    }
}

static int32_t*
config_parser__version(Config_Parser *ctx){
    require(config_parser__match_text(ctx, make_lit_string("version")));
    
    if (!config_parser__match_token(ctx, CPP_TOKEN_PARENTHESE_OPEN)){
        config_parser__log_error(ctx, "expected token '(' for version specifier: 'version(#)'");
        config_parser__recover_parse(ctx);
        return(0);
    }
    
    if (!config_parser__recognize_token(ctx, CPP_TOKEN_INTEGER_CONSTANT)){
        config_parser__log_error(ctx, "expected an integer constant for version specifier: 'version(#)'");
        config_parser__recover_parse(ctx);
        return(0);
    }
    
    Config_Integer value = config_parser__get_int(ctx);
    config_parser__advance_to_next(ctx);
    
    if (!config_parser__match_token(ctx, CPP_TOKEN_PARENTHESE_CLOSE)){
        config_parser__log_error(ctx, "expected token ')' for version specifier: 'version(#)'");
        config_parser__recover_parse(ctx);
        return(0);
    }
    
    if (!config_parser__match_token(ctx, CPP_TOKEN_SEMICOLON)){
        config_parser__log_error(ctx, "expected token ';' for version specifier: 'version(#)'");
        config_parser__recover_parse(ctx);
        return(0);
    }
    
    int32_t *ptr = push_array(ctx->arena, int32_t, 1);
    *ptr = value.integer;
    return(ptr);
}

static Config_Assignment*
config_parser__assignment(Config_Parser *ctx){
    char *pos = config_parser__get_pos(ctx);
    
    Config_LValue *l = config_parser__lvalue(ctx);
    if (l == 0){
        config_parser__log_error(ctx, "expected an l-value; l-value formats: 'identifier', 'identifier[#]'");
        config_parser__recover_parse(ctx);
        return(0);
    }
    
    if (!config_parser__match_token(ctx, CPP_TOKEN_EQ)){
        config_parser__log_error(ctx, "expected token '=' for assignment: 'l-value = r-value;'");
        config_parser__recover_parse(ctx);
        return(0);
    }
    
    Config_RValue *r = config_parser__rvalue(ctx);
    if (r == 0){
        config_parser__log_error(ctx, "expected an r-value; r-value formats:\n"
                                 "\tconstants (true, false, integers, hexadecimal integers, strings, characters)\n"
                                 "\tany l-value that is set in the file\n"
                                 "\tcompound: '{ compound-element, compound-element, compound-element ...}'\n"
                                 "\ta compound-element is an r-value, and can have a layout specifier\n"
                                 "\tcompound-element with layout specifier: .name = r-value, .integer = r-value");
        config_parser__recover_parse(ctx);
        return(0);
    }
    
    if (!config_parser__match_token(ctx, CPP_TOKEN_SEMICOLON)){
        config_parser__log_error(ctx, "expected token ';' for assignment: 'l-value = r-value;'");
        config_parser__recover_parse(ctx);
        return(0);
    }
    
    Config_Assignment *assignment = push_array(ctx->arena, Config_Assignment, 1);
    memset(assignment, 0, sizeof(*assignment));
    assignment->pos = pos;
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
        Config_Integer value = config_parser__get_int(ctx);
        index = value.integer;
        config_parser__advance_to_next(ctx);
        require(config_parser__match_token(ctx, CPP_TOKEN_BRACKET_CLOSE));
    }
    
    Config_LValue *lvalue = push_array(ctx->arena, Config_LValue, 1);
    memset(lvalue, 0, sizeof(*lvalue));
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
        memset(rvalue, 0, sizeof(*rvalue));
        rvalue->type = ConfigRValueType_LValue;
        rvalue->lvalue = l;
        return(rvalue);
    }
    else if (config_parser__recognize_token(ctx, CPP_TOKEN_BRACE_OPEN)){
        config_parser__advance_to_next(ctx);
        Config_Compound *compound = config_parser__compound(ctx);
        require(compound != 0);
        Config_RValue *rvalue = push_array(ctx->arena, Config_RValue, 1);
        memset(rvalue, 0, sizeof(*rvalue));
        rvalue->type = ConfigRValueType_Compound;
        rvalue->compound = compound;
        return(rvalue);
    }
    else if (config_parser__recognize_token(ctx, CPP_TOKEN_BOOLEAN_CONSTANT)){
        bool32 b = config_parser__get_boolean(ctx);
        config_parser__advance_to_next(ctx);
        Config_RValue *rvalue = push_array(ctx->arena, Config_RValue, 1);
        memset(rvalue, 0, sizeof(*rvalue));
        rvalue->type = ConfigRValueType_Boolean;
        rvalue->boolean = b;
        return(rvalue);
    }
    else if (config_parser__recognize_token(ctx, CPP_TOKEN_INTEGER_CONSTANT)){
        Config_Integer value = config_parser__get_int(ctx);
        config_parser__advance_to_next(ctx);
        Config_RValue *rvalue = push_array(ctx->arena, Config_RValue, 1);
        memset(rvalue, 0, sizeof(*rvalue));
        rvalue->type = ConfigRValueType_Integer;
        if (value.is_signed){
            rvalue->integer = value.integer;
        }
        else{
            rvalue->uinteger = value.uinteger;
        }
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
        memset(rvalue, 0, sizeof(*rvalue));
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
        memset(rvalue, 0, sizeof(*rvalue));
        rvalue->type = ConfigRValueType_Character;
        rvalue->character = space[0];
        return(rvalue);
    }
    return(0);
}

static void
config_parser__compound__check(Config_Parser *ctx, Config_Compound *compound){
    bool32 implicit_index_allowed = true;
    for (Config_Compound_Element *node = compound->first;
         node != 0;
         node = node->next){
        if (node->l.type != ConfigLayoutType_Unset){
            implicit_index_allowed = false;
        }
        else if (!implicit_index_allowed){
            config_parser__log_error_pos(ctx, node->l.pos,
                                         "encountered unlabeled member after one or more labeled members");
        }
    }
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
    memset(compound, 0, sizeof(*compound));
    compound->first = first;
    compound->last = last;
    compound->count = count;
    config_parser__compound__check(ctx, compound);
    return(compound);
}

static Config_Compound_Element*
config_parser__element(Config_Parser *ctx){
    Config_Layout layout = {0};
    layout.pos = config_parser__get_pos(ctx);
    if (config_parser__match_token(ctx, CPP_TOKEN_DOT)){
        if (config_parser__recognize_token(ctx, CPP_TOKEN_IDENTIFIER)){
            layout.type = ConfigLayoutType_Identifier;
            layout.identifier = config_parser__get_lexeme(ctx);
            config_parser__advance_to_next(ctx);
        }
        else if (config_parser__recognize_token(ctx, CPP_TOKEN_INTEGER_CONSTANT)){
            layout.type = ConfigLayoutType_Integer;
            Config_Integer value = config_parser__get_int(ctx);
            layout.integer = value.integer;
            config_parser__advance_to_next(ctx);
        }
        else{
            return(0);
        }
        require(config_parser__match_token(ctx, CPP_TOKEN_EQ));
    }
    Config_RValue *rvalue = config_parser__rvalue(ctx);
    require(rvalue != 0);
    Config_Compound_Element *element = push_array(ctx->arena, Config_Compound_Element, 1);
    memset(element, 0, sizeof(*element));
    element->l = layout;
    element->r = rvalue;
    return(element);
}

////////////////////////////////

static Config_Error*
config_add_error(Partition *arena, Config *config, char *pos, char *error_text){
    return(config_error_push(arena, &config->errors, config->file_name, pos, error_text));
}

////////////////////////////////

static Config_Assignment*
config_lookup_assignment(Config *config, String var_name, int32_t subscript){
    Config_Assignment *assignment;
    for (assignment = config->first;
         assignment != 0;
         assignment = assignment->next){
        Config_LValue *l = assignment->l;
        if (l != 0 && match(l->identifier, var_name) && l->index == subscript){
            break;
        }
    }
    return(assignment);
}

static Config_Get_Result
config_var(Config *config, String var_name, int32_t subscript);

static Config_Get_Result
config_evaluate_rvalue(Config *config, Config_Assignment *assignment, Config_RValue *r){
    Config_Get_Result result = {0};
    if (r != 0 && !assignment->visited){
        if (r->type == ConfigRValueType_LValue){
            assignment->visited = true;
            Config_LValue *l = r->lvalue;
            result = config_var(config, l->identifier, l->index);
            assignment->visited = false;
        }
        else{
            result.success = true;
            result.pos = assignment->pos;
            result.type = r->type;
            switch (r->type){
                case ConfigRValueType_Boolean:
                {
                    result.boolean = r->boolean;
                }break;
                
                case ConfigRValueType_Integer:
                {
                    result.integer = r->integer;
                }break;
                
                case ConfigRValueType_String:
                {
                    result.string = r->string;
                }break;
                
                case ConfigRValueType_Character:
                {
                    result.character = r->character;
                }break;
                
                case ConfigRValueType_Compound:
                {
                    result.compound = r->compound;
                }break;
            }
        }
    }
    return(result);
}

static Config_Get_Result
config_var(Config *config, String var_name, int32_t subscript){
    Config_Get_Result result = {0};
    Config_Assignment *assignment = config_lookup_assignment(config, var_name, subscript);
    if (assignment != 0){
        result = config_evaluate_rvalue(config, assignment, assignment->r);
    }
    return(result);
}

static Config_Get_Result
config_compound_member(Config *config, Config_Compound *compound, String var_name, int32_t index){
    Config_Get_Result result = {0};
    int32_t implicit_index = 0;
    bool32 implicit_index_is_valid = true;
    for (Config_Compound_Element *element = compound->first;
         element != 0;
         element = element->next, implicit_index += 1){
        bool32 element_matches_query = false;
        switch (element->l.type){
            case ConfigLayoutType_Unset:
            {
                if (implicit_index_is_valid && index == implicit_index){
                    element_matches_query = true;
                }
            }break;
            
            case ConfigLayoutType_Identifier:
            {
                implicit_index_is_valid = false;
                if (match(element->l.identifier, var_name)){
                    element_matches_query = true;
                }
            }break;
            
            case ConfigLayoutType_Integer:
            {
                implicit_index_is_valid = false;
                if (element->l.integer == index){
                    element_matches_query = true;
                }
            }break;
        }
        if (element_matches_query){
            Config_Assignment dummy_assignment = {0};
            dummy_assignment.pos = element->l.pos;
            result = config_evaluate_rvalue(config, &dummy_assignment, element->r);
            break;
        }
    }
    return(result);
}

static Config_Iteration_Step_Result
typed_array_iteration_step(Config *parsed, Config_Compound *compound, Config_RValue_Type type, int32_t index);

static int32_t
typed_array_get_count(Config *parsed, Config_Compound *compound, Config_RValue_Type type);

static Config_Get_Result_List
typed_array_reference_list(Partition *arena, Config *parsed, Config_Compound *compound, Config_RValue_Type type);

#define config_fixed_string_var(c,v,s,o,a) config_placed_string_var((c),(v),(s),(o),(a),sizeof(a))

////////////////////////////////

static bool32
config_has_var(Config *config, String var_name, int32_t subscript){
    Config_Get_Result result = config_var(config, var_name, subscript);
    return(result.success);
}

static bool32
config_has_var(Config *config, char *var_name, int32_t subscript){
    String var_name_str = make_string_slowly(var_name);
    Config_Get_Result result = config_var(config, var_name_str, subscript);
    return(result.success);
}

static bool32
config_bool_var(Config *config, String var_name, int32_t subscript, bool32* var_out){
    Config_Get_Result result = config_var(config, var_name, subscript);
    if (result.success){
        *var_out = result.boolean;
    }
    return(result.success);
}

static bool32
config_bool_var(Config *config, char *var_name, int32_t subscript, bool32* var_out){
    String var_name_str = make_string_slowly(var_name);
    Config_Get_Result result = config_var(config, var_name_str, subscript);
    if (result.success){
        *var_out = result.boolean;
    }
    return(result.success);
}

static bool32
config_int_var(Config *config, String var_name, int32_t subscript, int32_t* var_out){
    Config_Get_Result result = config_var(config, var_name, subscript);
    if (result.success){
        *var_out = result.integer;
    }
    return(result.success);
}

static bool32
config_int_var(Config *config, char *var_name, int32_t subscript, int32_t* var_out){
    String var_name_str = make_string_slowly(var_name);
    Config_Get_Result result = config_var(config, var_name_str, subscript);
    if (result.success){
        *var_out = result.integer;
    }
    return(result.success);
}

static bool32
config_uint_var(Config *config, String var_name, int32_t subscript, uint32_t* var_out){
    Config_Get_Result result = config_var(config, var_name, subscript);
    if (result.success){
        *var_out = result.uinteger;
    }
    return(result.success);
}

static bool32
config_uint_var(Config *config, char *var_name, int32_t subscript, uint32_t* var_out){
    String var_name_str = make_string_slowly(var_name);
    Config_Get_Result result = config_var(config, var_name_str, subscript);
    if (result.success){
        *var_out = result.uinteger;
    }
    return(result.success);
}

static bool32
config_string_var(Config *config, String var_name, int32_t subscript, String* var_out){
    Config_Get_Result result = config_var(config, var_name, subscript);
    if (result.success){
        *var_out = result.string;
    }
    return(result.success);
}

static bool32
config_string_var(Config *config, char *var_name, int32_t subscript, String* var_out){
    String var_name_str = make_string_slowly(var_name);
    Config_Get_Result result = config_var(config, var_name_str, subscript);
    if (result.success){
        *var_out = result.string;
    }
    return(result.success);
}

static bool32
config_placed_string_var(Config *config, String var_name, int32_t subscript, String* var_out, char *space, int32_t space_size){
    Config_Get_Result result = config_var(config, var_name, subscript);
    if (result.success){
        *var_out = result.string;
    }
    bool32 success = result.success;
    if (success){
        String str = *var_out;
        *var_out = make_string_cap(space, 0, space_size);
        copy(var_out, str);
    }
    return(result.success);
}

static bool32
config_placed_string_var(Config *config, char *var_name, int32_t subscript, String* var_out, char *space, int32_t space_size){
    String var_name_str = make_string_slowly(var_name);
    Config_Get_Result result = config_var(config, var_name_str, subscript);
    if (result.success){
        *var_out = result.string;
    }
    bool32 success = result.success;
    if (success){
        String str = *var_out;
        *var_out = make_string_cap(space, 0, space_size);
        copy(var_out, str);
    }
    return(result.success);
}

static bool32
config_char_var(Config *config, String var_name, int32_t subscript, char* var_out){
    Config_Get_Result result = config_var(config, var_name, subscript);
    if (result.success){
        *var_out = result.character;
    }
    return(result.success);
}

static bool32
config_char_var(Config *config, char *var_name, int32_t subscript, char* var_out){
    String var_name_str = make_string_slowly(var_name);
    Config_Get_Result result = config_var(config, var_name_str, subscript);
    if (result.success){
        *var_out = result.character;
    }
    return(result.success);
}

static bool32
config_compound_var(Config *config, String var_name, int32_t subscript, Config_Compound** var_out){
    Config_Get_Result result = config_var(config, var_name, subscript);
    if (result.success){
        *var_out = result.compound;
    }
    return(result.success);
}

static bool32
config_compound_var(Config *config, char *var_name, int32_t subscript, Config_Compound** var_out){
    String var_name_str = make_string_slowly(var_name);
    Config_Get_Result result = config_var(config, var_name_str, subscript);
    if (result.success){
        *var_out = result.compound;
    }
    return(result.success);
}

static bool32
config_compound_has_member(Config *config, Config_Compound *compound,
                           String var_name, int32_t index){
    Config_Get_Result result = config_compound_member(config, compound, var_name, index);
    return(result.success);
}

static bool32
config_compound_has_member(Config *config, Config_Compound *compound,
                           char *var_name, int32_t index){
    String var_name_str = make_string_slowly(var_name);
    Config_Get_Result result = config_compound_member(config, compound, var_name_str, index);
    return(result.success);
}

static bool32
config_compound_bool_member(Config *config, Config_Compound *compound,
                            String var_name, int32_t index, bool32* var_out){
    Config_Get_Result result = config_compound_member(config, compound, var_name, index);
    if (result.success){
        *var_out = result.boolean;
    }
    return(result.success);
}

static bool32
config_compound_bool_member(Config *config, Config_Compound *compound,
                            char *var_name, int32_t index, bool32* var_out){
    String var_name_str = make_string_slowly(var_name);
    Config_Get_Result result = config_compound_member(config, compound, var_name_str, index);
    if (result.success){
        *var_out = result.boolean;
    }
    return(result.success);
}

static bool32
config_compound_int_member(Config *config, Config_Compound *compound,
                           String var_name, int32_t index, int32_t* var_out){
    Config_Get_Result result = config_compound_member(config, compound, var_name, index);
    if (result.success){
        *var_out = result.integer;
    }
    return(result.success);
}

static bool32
config_compound_int_member(Config *config, Config_Compound *compound,
                           char *var_name, int32_t index, int32_t* var_out){
    String var_name_str = make_string_slowly(var_name);
    Config_Get_Result result = config_compound_member(config, compound, var_name_str, index);
    if (result.success){
        *var_out = result.integer;
    }
    return(result.success);
}

static bool32
config_compound_uint_member(Config *config, Config_Compound *compound,
                            String var_name, int32_t index, uint32_t* var_out){
    Config_Get_Result result = config_compound_member(config, compound, var_name, index);
    if (result.success){
        *var_out = result.uinteger;
    }
    return(result.success);
}

static bool32
config_compound_uint_member(Config *config, Config_Compound *compound,
                            char *var_name, int32_t index, uint32_t* var_out){
    String var_name_str = make_string_slowly(var_name);
    Config_Get_Result result = config_compound_member(config, compound, var_name_str, index);
    if (result.success){
        *var_out = result.uinteger;
    }
    return(result.success);
}

static bool32
config_compound_string_member(Config *config, Config_Compound *compound,
                              String var_name, int32_t index, String* var_out){
    Config_Get_Result result = config_compound_member(config, compound, var_name, index);
    if (result.success){
        *var_out = result.string;
    }
    return(result.success);
}

static bool32
config_compound_string_member(Config *config, Config_Compound *compound,
                              char *var_name, int32_t index, String* var_out){
    String var_name_str = make_string_slowly(var_name);
    Config_Get_Result result = config_compound_member(config, compound, var_name_str, index);
    if (result.success){
        *var_out = result.string;
    }
    return(result.success);
}

static bool32
config_compound_placed_string_member(Config *config, Config_Compound *compound,
                                     String var_name, int32_t index, String* var_out, char *space, int32_t space_size){
    Config_Get_Result result = config_compound_member(config, compound, var_name, index);
    if (result.success){
        *var_out = result.string;
    }
    bool32 success = result.success;
    if (success){
        String str = *var_out;
        *var_out = make_string_cap(space, 0, space_size);
        copy(var_out, str);
    }
    return(result.success);
}

static bool32
config_compound_placed_string_member(Config *config, Config_Compound *compound,
                                     char *var_name, int32_t index, String* var_out, char *space, int32_t space_size){
    String var_name_str = make_string_slowly(var_name);
    Config_Get_Result result = config_compound_member(config, compound, var_name_str, index);
    if (result.success){
        *var_out = result.string;
    }
    bool32 success = result.success;
    if (success){
        String str = *var_out;
        *var_out = make_string_cap(space, 0, space_size);
        copy(var_out, str);
    }
    return(result.success);
}

static bool32
config_compound_char_member(Config *config, Config_Compound *compound,
                            String var_name, int32_t index, char* var_out){
    Config_Get_Result result = config_compound_member(config, compound, var_name, index);
    if (result.success){
        *var_out = result.character;
    }
    return(result.success);
}

static bool32
config_compound_char_member(Config *config, Config_Compound *compound,
                            char *var_name, int32_t index, char* var_out){
    String var_name_str = make_string_slowly(var_name);
    Config_Get_Result result = config_compound_member(config, compound, var_name_str, index);
    if (result.success){
        *var_out = result.character;
    }
    return(result.success);
}

static bool32
config_compound_compound_member(Config *config, Config_Compound *compound,
                                String var_name, int32_t index, Config_Compound** var_out){
    Config_Get_Result result = config_compound_member(config, compound, var_name, index);
    if (result.success){
        *var_out = result.compound;
    }
    return(result.success);
}

static bool32
config_compound_compound_member(Config *config, Config_Compound *compound,
                                char *var_name, int32_t index, Config_Compound** var_out){
    String var_name_str = make_string_slowly(var_name);
    Config_Get_Result result = config_compound_member(config, compound, var_name_str, index);
    if (result.success){
        *var_out = result.compound;
    }
    return(result.success);
}

static Iteration_Step_Result
typed_has_array_iteration_step(Config *config, Config_Compound *compound, int32_t index){
    Config_Iteration_Step_Result result = typed_array_iteration_step(config, compound, ConfigRValueType_NoType, index);
    return(result.step);
}

static Iteration_Step_Result
typed_bool_array_iteration_step(Config *config, Config_Compound *compound, int32_t index, bool32* var_out){
    Config_Iteration_Step_Result result = typed_array_iteration_step(config, compound, ConfigRValueType_Boolean, index);
    bool32 success = (result.step == Iteration_Good);
    if (success){
        *var_out = result.get.boolean;
    }
    return(result.step);
}

static Iteration_Step_Result
typed_int_array_iteration_step(Config *config, Config_Compound *compound, int32_t index, int32_t* var_out){
    Config_Iteration_Step_Result result = typed_array_iteration_step(config, compound, ConfigRValueType_Integer, index);
    bool32 success = (result.step == Iteration_Good);
    if (success){
        *var_out = result.get.integer;
    }
    return(result.step);
}

static Iteration_Step_Result
typed_uint_array_iteration_step(Config *config, Config_Compound *compound, int32_t index, uint32_t* var_out){
    Config_Iteration_Step_Result result = typed_array_iteration_step(config, compound, ConfigRValueType_Integer, index);
    bool32 success = (result.step == Iteration_Good);
    if (success){
        *var_out = result.get.uinteger;
    }
    return(result.step);
}

static Iteration_Step_Result
typed_string_array_iteration_step(Config *config, Config_Compound *compound, int32_t index, String* var_out){
    Config_Iteration_Step_Result result = typed_array_iteration_step(config, compound, ConfigRValueType_String, index);
    bool32 success = (result.step == Iteration_Good);
    if (success){
        *var_out = result.get.string;
    }
    return(result.step);
}

static Iteration_Step_Result
typed_placed_string_array_iteration_step(Config *config, Config_Compound *compound, int32_t index, String* var_out
                                         , char *space, int32_t space_size){
    Config_Iteration_Step_Result result = typed_array_iteration_step(config, compound, ConfigRValueType_String, index);
    bool32 success = (result.step == Iteration_Good);
    if (success){
        *var_out = result.get.string;
    }
    if (success){
        String str = *var_out;
        *var_out = make_string_cap(space, 0, space_size);
        copy(var_out, str);
    }
    return(result.step);
}

static Iteration_Step_Result
typed_char_array_iteration_step(Config *config, Config_Compound *compound, int32_t index, char* var_out){
    Config_Iteration_Step_Result result = typed_array_iteration_step(config, compound, ConfigRValueType_Character, index);
    bool32 success = (result.step == Iteration_Good);
    if (success){
        *var_out = result.get.character;
    }
    return(result.step);
}

static Iteration_Step_Result
typed_compound_array_iteration_step(Config *config, Config_Compound *compound, int32_t index, Config_Compound** var_out){
    Config_Iteration_Step_Result result = typed_array_iteration_step(config, compound, ConfigRValueType_Compound, index);
    bool32 success = (result.step == Iteration_Good);
    if (success){
        *var_out = result.get.compound;
    }
    return(result.step);
}

static int32_t
typed_bool_array_get_count(Config *config, Config_Compound *compound){
    int32_t count = typed_array_get_count(config, compound, ConfigRValueType_Boolean);
    return(count);
}

static int32_t
typed_int_array_get_count(Config *config, Config_Compound *compound){
    int32_t count = typed_array_get_count(config, compound, ConfigRValueType_Integer);
    return(count);
}

static int32_t
typed_float_array_get_count(Config *config, Config_Compound *compound){
    int32_t count = typed_array_get_count(config, compound, ConfigRValueType_Float);
    return(count);
}

static int32_t
typed_string_array_get_count(Config *config, Config_Compound *compound){
    int32_t count = typed_array_get_count(config, compound, ConfigRValueType_String);
    return(count);
}

static int32_t
typed_character_array_get_count(Config *config, Config_Compound *compound){
    int32_t count = typed_array_get_count(config, compound, ConfigRValueType_Character);
    return(count);
}

static int32_t
typed_compound_array_get_count(Config *config, Config_Compound *compound){
    int32_t count = typed_array_get_count(config, compound, ConfigRValueType_Compound);
    return(count);
}

static int32_t
typed_no_type_array_get_count(Config *config, Config_Compound *compound){
    int32_t count = typed_array_get_count(config, compound, ConfigRValueType_NoType);
    return(count);
}

static Config_Get_Result_List
typed_bool_array_reference_list(Partition *arena, Config *config, Config_Compound *compound){
    Config_Get_Result_List list = typed_array_reference_list(arena, config, compound, ConfigRValueType_Boolean);
    return(list);
}

static Config_Get_Result_List
typed_int_array_reference_list(Partition *arena, Config *config, Config_Compound *compound){
    Config_Get_Result_List list = typed_array_reference_list(arena, config, compound, ConfigRValueType_Integer);
    return(list);
}

static Config_Get_Result_List
typed_float_array_reference_list(Partition *arena, Config *config, Config_Compound *compound){
    Config_Get_Result_List list = typed_array_reference_list(arena, config, compound, ConfigRValueType_Float);
    return(list);
}

static Config_Get_Result_List
typed_string_array_reference_list(Partition *arena, Config *config, Config_Compound *compound){
    Config_Get_Result_List list = typed_array_reference_list(arena, config, compound, ConfigRValueType_String);
    return(list);
}

static Config_Get_Result_List
typed_character_array_reference_list(Partition *arena, Config *config, Config_Compound *compound){
    Config_Get_Result_List list = typed_array_reference_list(arena, config, compound, ConfigRValueType_Character);
    return(list);
}

static Config_Get_Result_List
typed_compound_array_reference_list(Partition *arena, Config *config, Config_Compound *compound){
    Config_Get_Result_List list = typed_array_reference_list(arena, config, compound, ConfigRValueType_Compound);
    return(list);
}

static Config_Get_Result_List
typed_no_type_array_reference_list(Partition *arena, Config *config, Config_Compound *compound){
    Config_Get_Result_List list = typed_array_reference_list(arena, config, compound, ConfigRValueType_NoType);
    return(list);
}

////////////////////////////////

static Config_Iteration_Step_Result
typed_array_iteration_step(Config *parsed, Config_Compound *compound, Config_RValue_Type type, int32_t index){
    Config_Iteration_Step_Result result = {0};
    result.step = Iteration_Quit;
    Config_Get_Result get_result = config_compound_member(parsed, compound, make_lit_string("~"), index);
    if (get_result.success){
        if (get_result.type == type || type == ConfigRValueType_NoType){
            result.step = Iteration_Good;
            result.get = get_result;
        }
        else{
            result.step = Iteration_Skip;
        }
    }
    return(result);
}

static int32_t
typed_array_get_count(Config *parsed, Config_Compound *compound, Config_RValue_Type type){
    int32_t count = 0;
    for (int32_t i = 0;; ++i){
        Config_Iteration_Step_Result result = typed_array_iteration_step(parsed, compound, type, i);
        if (result.step == Iteration_Skip){
            continue;
        }
        else if (result.step == Iteration_Quit){
            break;
        }
        count += 1;
    }
    return(count);
}

static Config_Get_Result_List
typed_array_reference_list(Partition *arena, Config *parsed, Config_Compound *compound, Config_RValue_Type type){
    Config_Get_Result_List list = {0};
    for (int32_t i = 0;; ++i){
        Config_Iteration_Step_Result result = typed_array_iteration_step(parsed, compound, type, i);
        if (result.step == Iteration_Skip){
            continue;
        }
        else if (result.step == Iteration_Quit){
            break;
        }
        Config_Get_Result_Node *node = push_array(arena, Config_Get_Result_Node, 1);
        node->result = result.get;
        zdll_push_back(list.first, list.last, node);
        list.count += 1;
    }
    return(list);
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

static Cpp_Token_Array
text_data_to_token_array(Partition *arena, String data){
    bool32 success = false;
    int32_t max_count = (1 << 20)/sizeof(Cpp_Token);
    Temp_Memory restore_point = begin_temp_memory(arena);
    Cpp_Token_Array array = {0};
    array.tokens = push_array(arena, Cpp_Token, max_count);
    if (array.tokens != 0){
        array.max_count = max_count;
        Cpp_Keyword_Table kw_table = {0};
        Cpp_Keyword_Table pp_table = {0};
        if (lexer_keywords_default_init(arena, &kw_table, &pp_table)){
            Cpp_Lex_Data S = cpp_lex_data_init(false, kw_table, pp_table);
            Cpp_Lex_Result result = cpp_lex_step(&S, data.str, data.size + 1, HAS_NULL_TERM, &array, NO_OUT_LIMIT);
            if (result == LexResult_Finished){
                success = true;
            }
        }
    }
    if (!success){
        memset(&array, 0, sizeof(array));
        end_temp_memory(restore_point);
    }
    return(array);
}

static Config*
text_data_to_parsed_data(Partition *arena, String file_name, String data){
    Config *parsed = 0;
    Temp_Memory restore_point = begin_temp_memory(arena);
    Cpp_Token_Array array = text_data_to_token_array(arena, data);
    if (array.tokens != 0){
        parsed = text_data_and_token_array_to_parse_data(arena, file_name, data, array);
        if (parsed == 0){
            end_temp_memory(restore_point);
        }
    }
    return(parsed);
}

////////////////////////////////

static void
config_init_default(Config_Data *config){
    config->user_name = make_fixed_width_string(config->user_name_space);
    copy(&config->user_name, "");
    
    memset(&config->code_exts, 0, sizeof(config->code_exts));
    
    config->current_mapping = make_fixed_width_string(config->current_mapping_space);
    copy(&config->current_mapping, "");
    
    config->use_scroll_bars = false;
    config->use_file_bars = true;
    config->enable_code_wrapping = true;
    config->automatically_adjust_wrapping = true;
    config->automatically_indent_text_on_save = true;
    config->automatically_save_changes_on_build = true;
    config->automatically_load_project = false;
    
    config->indent_with_tabs = false;
    config->indent_width = 4;
    
    config->default_wrap_width = 672;
    config->default_min_base_width = 550;
    
    config->default_theme_name = make_fixed_width_string(config->default_theme_name_space);
    copy(&config->default_theme_name, "4coder");
    
    config->default_font_name = make_fixed_width_string(config->default_font_name_space);
    copy(&config->default_font_name, "");
    config->default_font_size = 16;
    config->default_font_hinting = false;
    
    config->default_compiler_bat = make_fixed_width_string(config->default_compiler_bat_space);
    copy(&config->default_compiler_bat, "cl");
    
    config->default_flags_bat = make_fixed_width_string(config->default_flags_bat_space);
    copy(&config->default_flags_bat, "");
    
    config->default_compiler_sh = make_fixed_width_string(config->default_compiler_sh_space);
    copy(&config->default_compiler_sh, "g++");
    
    config->default_flags_sh = make_fixed_width_string(config->default_flags_sh_space);
    copy(&config->default_flags_sh, "");
    
    config->lalt_lctrl_is_altgr = false;
}

static Config*
config_parse__data(Partition *arena, String file_name, String data, Config_Data *config){
    config_init_default(config);
    
    bool32 success = false;
    
    Config *parsed = text_data_to_parsed_data(arena, file_name, data);
    if (parsed != 0){
        success = true;
        
        config_fixed_string_var(parsed, "user_name", 0,
                                &config->user_name, config->user_name_space);
        
        String str;
        if (config_string_var(parsed, "treat_as_code", 0, &str)){
            parse_extension_line_to_extension_list(str, &config->code_exts);
        }
        
        config_fixed_string_var(parsed, "mapping", 0,
                                &config->current_mapping, config->current_mapping_space);
        
        config_bool_var(parsed, "use_scroll_bars", 0, &config->use_scroll_bars);
        config_bool_var(parsed, "use_file_bars", 0, &config->use_file_bars);
        config_bool_var(parsed, "enable_code_wrapping", 0, &config->enable_code_wrapping);
        config_bool_var(parsed, "automatically_adjust_wrapping", 0, &config->automatically_adjust_wrapping);
        config_bool_var(parsed, "automatically_indent_text_on_save", 0, &config->automatically_indent_text_on_save);
        config_bool_var(parsed, "automatically_save_changes_on_build", 0, &config->automatically_save_changes_on_build);
        config_bool_var(parsed, "automatically_load_project", 0, &config->automatically_load_project);
        
        config_bool_var(parsed, "indent_with_tabs", 0, &config->indent_with_tabs);
        config_int_var(parsed, "indent_width", 0, &config->indent_width);
        
        config_int_var(parsed, "default_wrap_width", 0, &config->default_wrap_width);
        config_int_var(parsed, "default_min_base_width", 0, &config->default_min_base_width);
        
        config_fixed_string_var(parsed, "default_theme_name", 0,
                                &config->default_theme_name, config->default_theme_name_space);
        
        config_fixed_string_var(parsed, "default_font_name", 0,
                                &config->default_font_name, config->default_font_name_space);
        config_int_var(parsed, "default_font_size", 0, &config->default_font_size);
        config_bool_var(parsed, "default_font_hinting", 0, &config->default_font_hinting);
        
        config_fixed_string_var(parsed, "default_compiler_bat", 0,
                                &config->default_compiler_bat, config->default_compiler_bat_space);
        config_fixed_string_var(parsed, "default_flags_bat", 0,
                                &config->default_flags_bat, config->default_flags_bat_space);
        config_fixed_string_var(parsed, "default_compiler_sh", 0,
                                &config->default_compiler_sh, config->default_compiler_sh_space);
        config_fixed_string_var(parsed, "default_flags_sh", 0,
                                &config->default_flags_sh, config->default_flags_sh_space);
        
        config_bool_var(parsed, "lalt_lctrl_is_altgr", 0, &config->lalt_lctrl_is_altgr);
    }
    
    if (!success){
        config_init_default(config);
    }
    
    return(parsed);
}

static Config*
config_parse__file_handle(Partition *arena,
                          String file_name, FILE *file, Config_Data *config){
    Config *parsed = 0;
    String data = dump_file_handle(arena, file);
    if (data.str != 0){
        parsed = config_parse__data(arena, file_name, data, config);
    }
    else{
        config_init_default(config);
    }
    return(parsed);
}

static Config*
config_parse__file_name(Application_Links *app, Partition *arena,
                        char *file_name, Config_Data *config){
    Config *parsed = 0;
    bool32 success = false;
    FILE *file = open_file_try_current_path_then_binary_path(app, file_name);
    if (file != 0){
        String data = dump_file_handle(arena, file);
        fclose(file);
        if (data.str != 0){
            parsed = config_parse__data(arena, make_string_slowly(file_name), data, config);
            success = true; 
        }
    }
    if (!success){
        config_init_default(config);
    }
    return(parsed);
}

static void
init_theme_zero(Theme *theme){
    for (int32_t i = 0; i < Stag_COUNT; ++i){
        theme->colors[i] = 0;
    }
}

static Config*
theme_parse__data(Partition *arena, String file_name, String data, Theme_Data *theme){
    theme->name = make_fixed_width_string(theme->space);
    copy(&theme->name, "unnamed");
    init_theme_zero(&theme->theme);
    
    Config *parsed = text_data_to_parsed_data(arena, file_name, data);
    if (parsed != 0){
        config_fixed_string_var(parsed, "name", 0, &theme->name, theme->space);
        
        for (int32_t i = 0; i < Stag_COUNT; ++i){
            char *name = style_tag_names[i];
            uint32_t color = 0;
            if (!config_uint_var(parsed, name, 0, &color)){
                color = 0xFFFF00FF;
            }
            theme->theme.colors[i] = color;
        }
    }
    
    return(parsed);
}

static Config*
theme_parse__file_handle(Partition *arena, String file_name, FILE *file, Theme_Data *theme){
    String data = dump_file_handle(arena, file);
    Config *parsed = 0;
    if (data.str != 0){
        parsed = theme_parse__data(arena, file_name, data, theme);
    }
    return(parsed);
}

static Config*
theme_parse__file_name(Application_Links *app, Partition *arena,
                       char *file_name, Theme_Data *theme){
    Config *parsed = 0;
    FILE *file = open_file_try_current_path_then_binary_path(app, file_name);
    if (file != 0){
        String data = dump_file_handle(arena, file);
        fclose(file);
        parsed = theme_parse__data(arena, make_string_slowly(file_name), data, theme);
    }
    if (parsed == 0){
        char space[256];
        String str = make_fixed_width_string(space);
        append(&str, "Did not find ");
        append(&str, file_name);
        append(&str, ", color scheme not loaded");
        print_message(app, str.str, str.size);
    }
    return(parsed);
}

////////////////////////////////

static void
config_feedback_bool(String *space, char *name, bool32 val){
    append(space, name);
    append(space, " = ");
    append(space, (char*)(val?"true":"false"));
    append(space, ";\n");
}

static void
config_feedback_string(String *space, char *name, String val){
    if (val.size > 0){
        append(space, name);
        append(space, " = \"");
        append(space, val);
        append(space, "\";\n");
    }
}

static void
config_feedback_string(String *space, char *name, char *val){
    config_feedback_string(space, name, make_string_slowly(val));
}

static void
config_feedback_extension_list(String *space, char *name, Extension_List *list){
    if (list->count > 0){
        append(space, name);
        append(space, " = \"");
        for (int32_t i = 0; i < list->count; ++i){
            append(space, ".");
            append(space, list->exts[i]);
        }
        append(space, "\";\n");
    }
}

static void
config_feedback_int(String *space, char *name, int32_t val){
    append(space, name);
    append(space, " = ");
    append_int_to_str(space, val);
    append(space, ";\n");
}

////////////////////////////////

static void
load_config_and_apply(Application_Links *app, Partition *scratch, Config_Data *config,
                      int32_t override_font_size, bool32 override_hinting){
    Temp_Memory temp = begin_temp_memory(scratch);
    Config *parsed = config_parse__file_name(app, scratch, "config.4coder", config);
    
    if (parsed != 0){
        // Top
        print_message(app, literal("Loaded config file:\n"));
        
        // Errors
        String error_text = config_stringize_errors(scratch, parsed);
        if (error_text.str != 0){
            print_message(app, error_text.str, error_text.size);
        }
        
        // Values
        Temp_Memory temp2 = begin_temp_memory(scratch);
        String space = push_string(scratch, partition_remaining(scratch));
        
        {
            config_feedback_string(&space, "user_name", config->user_name);
            config_feedback_extension_list(&space, "treat_as_code", &config->code_exts);
            config_feedback_string(&space, "current_mapping", config->current_mapping);
            
            config_feedback_bool(&space, "use_scroll_bars", config->use_scroll_bars);
            config_feedback_bool(&space, "use_file_bars", config->use_file_bars);
            config_feedback_bool(&space, "enable_code_wrapping", config->enable_code_wrapping);
            config_feedback_bool(&space, "automatically_indent_text_on_save", config->automatically_indent_text_on_save);
            config_feedback_bool(&space, "automatically_save_changes_on_build", config->automatically_save_changes_on_build);
            config_feedback_bool(&space, "automatically_adjust_wrapping", config->automatically_adjust_wrapping);
            config_feedback_bool(&space, "automatically_load_project", config->automatically_load_project);
            
            config_feedback_bool(&space, "indent_with_tabs", config->indent_with_tabs);
            config_feedback_int(&space, "indent_width", config->indent_width);
            
            config_feedback_int(&space, "default_wrap_width", config->default_wrap_width);
            config_feedback_int(&space, "default_min_base_width", config->default_min_base_width);
            
            config_feedback_string(&space, "default_theme_name", config->default_theme_name);
            
            config_feedback_string(&space, "default_font_name", config->default_font_name);
            config_feedback_int(&space, "default_font_size", config->default_font_size);
            config_feedback_bool(&space, "default_font_hinting", config->default_font_hinting);
            
            config_feedback_string(&space, "default_compiler_bat", config->default_compiler_bat);
            config_feedback_string(&space, "default_flags_bat", config->default_flags_bat);
            config_feedback_string(&space, "default_compiler_sh", config->default_compiler_sh);
            config_feedback_string(&space, "default_flags_sh", config->default_flags_sh);
            
            config_feedback_bool(&space, "lalt_lctrl_is_altgr", config->lalt_lctrl_is_altgr);
        }
        
        append(&space, "\n");
        print_message(app, space.str, space.size);
        end_temp_memory(temp2);
        
        // Apply config
        change_mapping(app, config->current_mapping);
        adjust_all_buffer_wrap_widths(app, config->default_wrap_width, config->default_min_base_width);
        global_set_setting(app, GlobalSetting_LAltLCtrlIsAltGr, config->lalt_lctrl_is_altgr);
        
        change_theme(app, config->default_theme_name.str, config->default_theme_name.size);
        
        Face_Description description = {0};
        int32_t len = config->default_font_name.size;
        char *name_ptr = config->default_font_name.str;
        if (len > sizeof(description.font.name) - 1){
            len = sizeof(description.font.name) - 1;
        }
        memcpy(description.font.name, name_ptr, len);
        description.font.name[len] = 0;
        if (override_font_size != 0){
            description.pt_size = override_font_size;
        }
        else{
            description.pt_size = config->default_font_size;
        }
        description.hinting = config->default_font_hinting || override_hinting;
        change_global_face_by_description(app, description, true);
    }
    
    end_temp_memory(temp);
}

static void
load_theme_file_into_live_set(Application_Links *app, Partition *scratch, char *file_name){
    Temp_Memory temp = begin_temp_memory(scratch);
    Theme_Data theme = {0};
    Config *config = theme_parse__file_name(app, scratch, file_name, &theme);
    String error_text = config_stringize_errors(scratch, config);
    print_message(app, error_text.str, error_text.size);
    end_temp_memory(temp);
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

