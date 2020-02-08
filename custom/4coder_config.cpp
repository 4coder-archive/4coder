/*
4coder_config.cpp - Parsing *.4coder files.
*/

// TOP

function String_Const_u8_Array
parse_extension_line_to_extension_list(Application_Links *app,
                                       Arena *arena, String_Const_u8 str){
    ProfileScope(app, "parse extension line to extension list");
    i32 count = 0;
    for (u64 i = 0; i < str.size; i += 1){
        if (str.str[i] == '.'){
            count += 1;
        }
    }
    
    String_Const_u8_Array array = {};
    array.count = count;
    array.strings = push_array(arena, String_Const_u8, count);
    
    push_align(arena, 1);
    str = string_skip(str, string_find_first(str, '.') + 1);
    for (i32 i = 0; i < count; i += 1){
        u64 next_period = string_find_first(str, '.');
        String_Const_u8 extension = string_prefix(str, next_period);
        str = string_skip(str, next_period + 1);
        array.strings[i] = push_string_copy(arena, extension);
    }
    push_align(arena, 8);
    
    return(array);
}

////////////////////////////////

function void
setup_built_in_mapping(Application_Links *app, String_Const_u8 name, Mapping *mapping, i64 global_id, i64 file_id, i64 code_id){
    Thread_Context *tctx = get_thread_context(app);
    if (string_match(name, string_u8_litexpr("default"))){
        mapping_release(tctx, mapping);
        mapping_init(tctx, mapping);
        setup_default_mapping(mapping, global_id, file_id, code_id);
    }
    else if (string_match(name, string_u8_litexpr("mac-default"))){
        mapping_release(tctx, mapping);
        mapping_init(tctx, mapping);
        setup_mac_mapping(mapping, global_id, file_id, code_id);
    }
    else if (string_match(name, string_u8_litexpr("choose"))){
        mapping_release(tctx, mapping);
        mapping_init(tctx, mapping);
#if OS_MAC
        setup_mac_mapping(mapping, global_id, file_id, code_id);
#else
        setup_default_mapping(mapping, global_id, file_id, code_id);
#endif
    }
}

////////////////////////////////

function Error_Location
get_error_location(Application_Links *app, u8 *base, u8 *pos){
    ProfileScope(app, "get error location");
    Error_Location location = {};
    location.line_number = 1;
    location.column_number = 1;
    for (u8 *ptr = base;
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

function String_Const_u8
config_stringize_errors(Application_Links *app, Arena *arena, Config *parsed){
    ProfileScope(app, "stringize errors");
    String_Const_u8 result = {};
    if (parsed->errors.first != 0){
        List_String_Const_u8 list = {};
        for (Config_Error *error = parsed->errors.first;
             error != 0;
             error = error->next){
            Error_Location location = get_error_location(app, parsed->data.str, error->pos);
            string_list_pushf(arena, &list, "%.*s:%d:%d: %.*s\n",
                              string_expand(error->file_name), location.line_number, location.column_number, string_expand(error->text));
        }
        result = string_list_flatten(arena, list);
    }
    return(result);
}

////////////////////////////////

function void
config_parser__advance_to_next(Config_Parser *ctx){
    Token *t = ctx->token;
    Token *e = ctx->end;
    for (t += 1;
         t < e && (t->kind == TokenBaseKind_Comment ||
                   t->kind == TokenBaseKind_Whitespace);
         t += 1);
    ctx->token = t;
}

function Config_Parser
make_config_parser(Arena *arena, String_Const_u8 file_name, String_Const_u8 data, Token_Array array){
    Config_Parser ctx = {};
    ctx.start = array.tokens;
    ctx.token = ctx.start - 1;
    ctx.end = ctx.start + array.count;
    ctx.file_name = file_name;
    ctx.data = data;
    ctx.arena = arena;
    config_parser__advance_to_next(&ctx);
    return(ctx);
}

function b32
config_parser__recognize_base_token(Config_Parser *ctx, Token_Base_Kind kind){
    b32 result = false;
    if (ctx->start <= ctx->token && ctx->token < ctx->end){
        result = (ctx->token->kind == kind);
    }
    else if (kind == TokenBaseKind_EOF){
        result = true;
    }
    return(result);
}

function b32
config_parser__recognize_token(Config_Parser *ctx, Token_Cpp_Kind kind){
    b32 result = false;
    if (ctx->start <= ctx->token && ctx->token < ctx->end){
        result = (ctx->token->sub_kind == kind);
    }
    else if (kind == TokenCppKind_EOF){
        result = true;
    }
    return(result);
}

function b32
config_parser__recognize_boolean(Config_Parser *ctx){
    b32 result = false;
    Token *token = ctx->token;
    if (ctx->start <= ctx->token && ctx->token < ctx->end){
        result = (token->sub_kind == TokenCppKind_LiteralTrue ||
                  token->sub_kind == TokenCppKind_LiteralFalse);
    }
    return(result);
}

function String_Const_u8
config_parser__get_lexeme(Config_Parser *ctx){
    String_Const_u8 lexeme = {};
    Token *token = ctx->token;
    if (ctx->start <= token && token < ctx->end){
        lexeme = SCu8(ctx->data.str + token->pos, token->size);
    }
    return(lexeme);
}

function Config_Integer
config_parser__get_int(Config_Parser *ctx){
    Config_Integer config_integer = {};
    String_Const_u8 str = config_parser__get_lexeme(ctx);
    if (string_match(string_prefix(str, 2), string_u8_litexpr("0x"))){
        config_integer.is_signed = false;
        config_integer.uinteger = (u32)(string_to_integer(string_skip(str, 2), 16));
    }
    else{
        b32 is_negative = (string_get_character(str, 0) == '-');
        if (is_negative){
            str = string_skip(str, 1);
        }
        config_integer.is_signed = true;
        config_integer.integer = (i32)(string_to_integer(str, 10));
        if (is_negative){
            config_integer.integer *= -1;
        }
    }
    return(config_integer);
}

function b32
config_parser__get_boolean(Config_Parser *ctx){
    String_Const_u8 str = config_parser__get_lexeme(ctx);
    return(string_match(str, string_u8_litexpr("true")));
}

function b32
config_parser__recognize_text(Config_Parser *ctx, String_Const_u8 text){
    String_Const_u8 lexeme = config_parser__get_lexeme(ctx);
    return(lexeme.str != 0 && string_match(lexeme, text));
}

function b32
config_parser__match_token(Config_Parser *ctx, Token_Cpp_Kind kind){
    b32 result = config_parser__recognize_token(ctx, kind);
    if (result){
        config_parser__advance_to_next(ctx);
    }
    return(result);
}

function b32
config_parser__match_text(Config_Parser *ctx, String_Const_u8 text){
    b32 result = config_parser__recognize_text(ctx, text);
    if (result){
        config_parser__advance_to_next(ctx);
    }
    return(result);
}

#define config_parser__match_text_lit(c,s) config_parser__match_text((c), string_u8_litexpr(s))

function Config                  *config_parser__config    (Config_Parser *ctx);
function i32                     *config_parser__version   (Config_Parser *ctx);
function Config_Assignment       *config_parser__assignment(Config_Parser *ctx);
function Config_LValue           *config_parser__lvalue    (Config_Parser *ctx);
function Config_RValue           *config_parser__rvalue    (Config_Parser *ctx);
function Config_Compound         *config_parser__compound  (Config_Parser *ctx);
function Config_Compound_Element *config_parser__element   (Config_Parser *ctx);

function Config*
config_parse(Application_Links *app, Arena *arena, String_Const_u8 file_name,
             String_Const_u8 data, Token_Array array){
    ProfileScope(app, "config parse");
    Temp_Memory restore_point = begin_temp(arena);
    Config_Parser ctx = make_config_parser(arena, file_name, data, array);
    Config *config = config_parser__config(&ctx);
    if (config == 0){
        end_temp(restore_point);
    }
    return(config);
}

// TODO(allen): Move to string library
function Config_Error*
config_error_push(Arena *arena, Config_Error_List *list, String_Const_u8 file_name,
                  u8 *pos, char *error_text){
    Config_Error *error = push_array(arena, Config_Error, 1);
    zdll_push_back(list->first, list->last, error);
    list->count += 1;
    error->file_name = file_name;
    error->pos = pos;
    error->text = push_string_copy(arena, SCu8(error_text));
    return(error);
}

function u8*
config_parser__get_pos(Config_Parser *ctx){
    return(ctx->data.str + ctx->token->pos);
}

function void
config_parser__log_error_pos(Config_Parser *ctx, u8 *pos, char *error_text){
    config_error_push(ctx->arena, &ctx->errors, ctx->file_name, pos, error_text);
}

function void
config_parser__log_error(Config_Parser *ctx, char *error_text){
    config_parser__log_error_pos(ctx, config_parser__get_pos(ctx), error_text);
}

function Config*
config_parser__config(Config_Parser *ctx){
    i32 *version = config_parser__version(ctx);
    
    Config_Assignment *first = 0;
    Config_Assignment *last = 0;
    i32 count = 0;
    for (;!config_parser__recognize_token(ctx, TokenCppKind_EOF);){
        Config_Assignment *assignment = config_parser__assignment(ctx);
        if (assignment != 0){
            zdll_push_back(first, last, assignment);
            count += 1;
        }
    }
    
    Config *config = push_array(ctx->arena, Config, 1);
    block_zero_struct(config);
    config->version = version;
    config->first = first;
    config->last = last;
    config->count = count;
    config->errors = ctx->errors;
    config->file_name = ctx->file_name;
    config->data = ctx->data;
    return(config);
}

function void
config_parser__recover_parse(Config_Parser *ctx){
    for (;;){
        if (config_parser__match_token(ctx, TokenCppKind_Semicolon)){
            break;
        }
        if (config_parser__recognize_token(ctx, TokenCppKind_EOF)){
            break;
        }
        config_parser__advance_to_next(ctx);
    }
}

function i32*
config_parser__version(Config_Parser *ctx){
    require(config_parser__match_text_lit(ctx, "version"));
    
    if (!config_parser__match_token(ctx, TokenCppKind_ParenOp)){
        config_parser__log_error(ctx, "expected token '(' for version specifier: 'version(#)'");
        config_parser__recover_parse(ctx);
        return(0);
    }
    
    if (!config_parser__recognize_base_token(ctx, TokenBaseKind_LiteralInteger)){
        config_parser__log_error(ctx, "expected an integer constant for version specifier: 'version(#)'");
        config_parser__recover_parse(ctx);
        return(0);
    }
    
    Config_Integer value = config_parser__get_int(ctx);
    config_parser__advance_to_next(ctx);
    
    if (!config_parser__match_token(ctx, TokenCppKind_ParenCl)){
        config_parser__log_error(ctx, "expected token ')' for version specifier: 'version(#)'");
        config_parser__recover_parse(ctx);
        return(0);
    }
    
    if (!config_parser__match_token(ctx, TokenCppKind_Semicolon)){
        config_parser__log_error(ctx, "expected token ';' for version specifier: 'version(#)'");
        config_parser__recover_parse(ctx);
        return(0);
    }
    
    i32 *ptr = push_array(ctx->arena, i32, 1);
    *ptr = value.integer;
    return(ptr);
}

function Config_Assignment*
config_parser__assignment(Config_Parser *ctx){
    u8 *pos = config_parser__get_pos(ctx);
    
    Config_LValue *l = config_parser__lvalue(ctx);
    if (l == 0){
        config_parser__log_error(ctx, "expected an l-value; l-value formats: 'identifier', 'identifier[#]'");
        config_parser__recover_parse(ctx);
        return(0);
    }
    
    if (!config_parser__match_token(ctx, TokenCppKind_Eq)){
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
    
    if (!config_parser__match_token(ctx, TokenCppKind_Semicolon)){
        config_parser__log_error(ctx, "expected token ';' for assignment: 'l-value = r-value;'");
        config_parser__recover_parse(ctx);
        return(0);
    }
    
    Config_Assignment *assignment = push_array_zero(ctx->arena, Config_Assignment, 1);
    assignment->pos = pos;
    assignment->l = l;
    assignment->r = r;
    return(assignment);
}

function Config_LValue*
config_parser__lvalue(Config_Parser *ctx){
    require(config_parser__recognize_token(ctx, TokenCppKind_Identifier));
    String_Const_u8 identifier = config_parser__get_lexeme(ctx);
    config_parser__advance_to_next(ctx);
    
    i32 index = 0;
    if (config_parser__match_token(ctx, TokenCppKind_BrackOp)){
        require(config_parser__recognize_base_token(ctx, TokenBaseKind_LiteralInteger));
        Config_Integer value = config_parser__get_int(ctx);
        index = value.integer;
        config_parser__advance_to_next(ctx);
        require(config_parser__match_token(ctx, TokenCppKind_BrackCl));
    }
    
    Config_LValue *lvalue = push_array_zero(ctx->arena, Config_LValue, 1);
    lvalue->identifier = identifier;
    lvalue->index = index;
    return(lvalue);
}

function Config_RValue*
config_parser__rvalue(Config_Parser *ctx){
    Config_RValue *rvalue = 0;
    if (config_parser__recognize_token(ctx, TokenCppKind_Identifier)){
        Config_LValue *l = config_parser__lvalue(ctx);
        require(l != 0);
        rvalue = push_array_zero(ctx->arena, Config_RValue, 1);
        rvalue->type = ConfigRValueType_LValue;
        rvalue->lvalue = l;
    }
    else if (config_parser__recognize_token(ctx, TokenCppKind_BraceOp)){
        config_parser__advance_to_next(ctx);
        Config_Compound *compound = config_parser__compound(ctx);
        require(compound != 0);
        rvalue = push_array_zero(ctx->arena, Config_RValue, 1);
        rvalue->type = ConfigRValueType_Compound;
        rvalue->compound = compound;
    }
    else if (config_parser__recognize_boolean(ctx)){
        b32 b = config_parser__get_boolean(ctx);
        config_parser__advance_to_next(ctx);
        rvalue = push_array_zero(ctx->arena, Config_RValue, 1);
        rvalue->type = ConfigRValueType_Boolean;
        rvalue->boolean = b;
    }
    else if (config_parser__recognize_base_token(ctx, TokenBaseKind_LiteralInteger)){
        Config_Integer value = config_parser__get_int(ctx);
        config_parser__advance_to_next(ctx);
        rvalue = push_array_zero(ctx->arena, Config_RValue, 1);
        rvalue->type = ConfigRValueType_Integer;
        if (value.is_signed){
            rvalue->integer = value.integer;
        }
        else{
            rvalue->uinteger = value.uinteger;
        }
    }
    else if (config_parser__recognize_token(ctx, TokenCppKind_LiteralString)){
        String_Const_u8 s = config_parser__get_lexeme(ctx);
        config_parser__advance_to_next(ctx);
        s = string_chop(string_skip(s, 1), 1);
        String_Const_u8 interpreted = string_interpret_escapes(ctx->arena, s);
        rvalue = push_array_zero(ctx->arena, Config_RValue, 1);
        rvalue->type = ConfigRValueType_String;
        rvalue->string = interpreted;
    }
    else if (config_parser__recognize_token(ctx, TokenCppKind_LiteralCharacter)){
        String_Const_u8 s = config_parser__get_lexeme(ctx);
        config_parser__advance_to_next(ctx);
        s = string_chop(string_skip(s, 1), 1);
        String_Const_u8 interpreted = string_interpret_escapes(ctx->arena, s);
        rvalue = push_array_zero(ctx->arena, Config_RValue, 1);
        rvalue->type = ConfigRValueType_Character;
        rvalue->character = string_get_character(interpreted, 0);
    }
    return(rvalue);
}

function void
config_parser__compound__check(Config_Parser *ctx, Config_Compound *compound){
    b32 implicit_index_allowed = true;
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

function Config_Compound*
config_parser__compound(Config_Parser *ctx){
    Config_Compound_Element *first = 0;
    Config_Compound_Element *last = 0;
    i32 count = 0;
    
    Config_Compound_Element *element = config_parser__element(ctx);
    require(element != 0);
    zdll_push_back(first, last, element);
    count += 1;
    
    for (;config_parser__match_token(ctx, TokenCppKind_Comma);){
        if (config_parser__recognize_token(ctx, TokenCppKind_BraceCl)){
            break;
        }
        element = config_parser__element(ctx);
        require(element != 0);
        zdll_push_back(first, last, element);
        count += 1;
    }
    
    require(config_parser__match_token(ctx, TokenCppKind_BraceCl));
    
    Config_Compound *compound = push_array(ctx->arena, Config_Compound, 1);
    block_zero_struct(compound);
    compound->first = first;
    compound->last = last;
    compound->count = count;
    config_parser__compound__check(ctx, compound);
    return(compound);
}

function Config_Compound_Element*
config_parser__element(Config_Parser *ctx){
    Config_Layout layout = {};
    layout.pos = config_parser__get_pos(ctx);
    if (config_parser__match_token(ctx, TokenCppKind_Dot)){
        if (config_parser__recognize_token(ctx, TokenCppKind_Identifier)){
            layout.type = ConfigLayoutType_Identifier;
            layout.identifier = config_parser__get_lexeme(ctx);
            config_parser__advance_to_next(ctx);
        }
        else if (config_parser__recognize_base_token(ctx, TokenBaseKind_LiteralInteger)){
            layout.type = ConfigLayoutType_Integer;
            Config_Integer value = config_parser__get_int(ctx);
            layout.integer = value.integer;
            config_parser__advance_to_next(ctx);
        }
        else{
            return(0);
        }
        require(config_parser__match_token(ctx, TokenCppKind_Eq));
    }
    Config_RValue *rvalue = config_parser__rvalue(ctx);
    require(rvalue != 0);
    Config_Compound_Element *element = push_array(ctx->arena, Config_Compound_Element, 1);
    block_zero_struct(element);
    element->l = layout;
    element->r = rvalue;
    return(element);
}

////////////////////////////////

function Config_Error*
config_add_error(Arena *arena, Config *config, u8 *pos, char *error_text){
    return(config_error_push(arena, &config->errors, config->file_name, pos,
                             error_text));
}

////////////////////////////////

function Config_Assignment*
config_lookup_assignment(Config *config, String_Const_u8 var_name, i32 subscript){
    Config_Assignment *assignment = 0;
    for (assignment = config->first;
         assignment != 0;
         assignment = assignment->next){
        Config_LValue *l = assignment->l;
        if (l != 0 && string_match(l->identifier, var_name) && l->index == subscript){
            break;
        }
    }
    return(assignment);
}

function Config_Get_Result
config_var(Config *config, String_Const_u8 var_name, i32 subscript);

function Config_Get_Result
config_evaluate_rvalue(Config *config, Config_Assignment *assignment, Config_RValue *r){
    Config_Get_Result result = {};
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

function Config_Get_Result
config_var(Config *config, String_Const_u8 var_name, i32 subscript){
    Config_Get_Result result = {};
    Config_Assignment *assignment = config_lookup_assignment(config, var_name, subscript);
    if (assignment != 0){
        result = config_evaluate_rvalue(config, assignment, assignment->r);
    }
    return(result);
}

function Config_Get_Result
config_compound_member(Config *config, Config_Compound *compound, String_Const_u8 var_name, i32 index){
    Config_Get_Result result = {};
    i32 implicit_index = 0;
    b32 implicit_index_is_valid = true;
    for (Config_Compound_Element *element = compound->first;
         element != 0;
         element = element->next, implicit_index += 1){
        b32 element_matches_query = false;
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
                if (string_match(element->l.identifier, var_name)){
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
            Config_Assignment dummy_assignment = {};
            dummy_assignment.pos = element->l.pos;
            result = config_evaluate_rvalue(config, &dummy_assignment, element->r);
            break;
        }
    }
    return(result);
}

function Config_Iteration_Step_Result
typed_array_iteration_step(Config *parsed, Config_Compound *compound, Config_RValue_Type type, i32 index);

function i32
typed_array_get_count(Config *parsed, Config_Compound *compound, Config_RValue_Type type);

function Config_Get_Result_List
typed_array_reference_list(Arena *arena, Config *parsed, Config_Compound *compound, Config_RValue_Type type);

#define config_fixed_string_var(c,v,s,o,a) config_placed_string_var((c),(v),(s),(o),(a),sizeof(a))

////////////////////////////////

function b32
config_has_var(Config *config, String_Const_u8 var_name, i32 subscript){
    Config_Get_Result result = config_var(config, var_name, subscript);
    return(result.success && result.type == ConfigRValueType_NoType);
}

function b32
config_has_var(Config *config, char *var_name, i32 subscript){
    return(config_has_var(config, SCu8(var_name), subscript));
}

function b32
config_bool_var(Config *config, String_Const_u8 var_name, i32 subscript, b32* var_out){
    Config_Get_Result result = config_var(config, var_name, subscript);
    b32 success = (result.success && result.type == ConfigRValueType_Boolean);
    if (success){
        *var_out = result.boolean;
    }
    return(success);
}
function b32
config_bool_var(Config *config, String_Const_u8 var_name, i32 subscript, b8 *var_out){
    b32 temp = false;
    b32 success = config_bool_var(config, var_name, subscript, &temp);
    if (success){
        *var_out = (temp != false);
    }
    return(success);
}
function b32
config_bool_var(Config *config, char *var_name, i32 subscript, b32* var_out){
    return(config_bool_var(config, SCu8(var_name), subscript, var_out));
}
function b32
config_bool_var(Config *config, char* var_name, i32 subscript, b8 *var_out){
    b32 temp = false;
    b32 success = config_bool_var(config, SCu8(var_name), subscript, &temp);
    if (success){
        *var_out = (temp != false);
    }
    return(success);
}

function b32
config_int_var(Config *config, String_Const_u8 var_name, i32 subscript, i32* var_out){
    Config_Get_Result result = config_var(config, var_name, subscript);
    b32 success = result.success && result.type == ConfigRValueType_Integer;
    if (success){
        *var_out = result.integer;
    }
    return(success);
}

function b32
config_int_var(Config *config, char *var_name, i32 subscript, i32* var_out){
    return(config_int_var(config, SCu8(var_name), subscript, var_out));
}

function b32
config_uint_var(Config *config, String_Const_u8 var_name, i32 subscript, u32* var_out){
    Config_Get_Result result = config_var(config, var_name, subscript);
    b32 success = result.success && result.type == ConfigRValueType_Integer;
    if (success){
        *var_out = result.uinteger;
    }
    return(success);
}

function b32
config_uint_var(Config *config, char *var_name, i32 subscript, u32* var_out){
    return(config_uint_var(config, SCu8(var_name), subscript, var_out));
}

function b32
config_string_var(Config *config, String_Const_u8 var_name, i32 subscript, String_Const_u8* var_out){
    Config_Get_Result result = config_var(config, var_name, subscript);
    b32 success = result.success && result.type == ConfigRValueType_String;
    if (success){
        *var_out = result.string;
    }
    return(success);
}

function b32
config_string_var(Config *config, char *var_name, i32 subscript, String_Const_u8* var_out){
    return(config_string_var(config, SCu8(var_name), subscript, var_out));
}

function b32
config_placed_string_var(Config *config, String_Const_u8 var_name, i32 subscript, String_Const_u8* var_out, u8 *space, u64 space_size){
    Config_Get_Result result = config_var(config, var_name, subscript);
    b32 success = (result.success && result.type == ConfigRValueType_String);
    if (success){
        u64 size = result.string.size;
        size = clamp_top(size, space_size);
        block_copy(space, result.string.str, size);
        *var_out = SCu8(space, size);
    }
    return(success);
}

function b32
config_placed_string_var(Config *config, char *var_name, i32 subscript, String_Const_u8* var_out, u8 *space, u64 space_size){
    return(config_placed_string_var(config, SCu8(var_name), subscript, var_out, space, space_size));
}

function b32
config_char_var(Config *config, String_Const_u8 var_name, i32 subscript, char* var_out){
    Config_Get_Result result = config_var(config, var_name, subscript);
    b32 success = result.success && result.type == ConfigRValueType_Character;
    if (success){
        *var_out = result.character;
    }
    return(success);
}

function b32
config_char_var(Config *config, char *var_name, i32 subscript, char* var_out){
    return(config_char_var(config, SCu8(var_name), subscript, var_out));
}

function b32
config_compound_var(Config *config, String_Const_u8 var_name, i32 subscript, Config_Compound** var_out){
    Config_Get_Result result = config_var(config, var_name, subscript);
    b32 success = (result.success && result.type == ConfigRValueType_Compound);
    if (success){
        *var_out = result.compound;
    }
    return(success);
}

function b32
config_compound_var(Config *config, char *var_name, i32 subscript, Config_Compound** var_out){
    return(config_compound_var(config, SCu8(var_name), subscript, var_out));
}

function b32
config_compound_has_member(Config *config, Config_Compound *compound,
                           String_Const_u8 var_name, i32 index){
    Config_Get_Result result = config_compound_member(config, compound, var_name, index);
    b32 success = result.success && result.type == ConfigRValueType_NoType;
    return(success);
}

function b32
config_compound_has_member(Config *config, Config_Compound *compound,
                           char *var_name, i32 index){
    return(config_compound_has_member(config, compound, SCu8(var_name), index));
}

function b32
config_compound_bool_member(Config *config, Config_Compound *compound,
                            String_Const_u8 var_name, i32 index, b32* var_out){
    Config_Get_Result result = config_compound_member(config, compound, var_name, index);
    b32 success = result.success && result.type == ConfigRValueType_Boolean;
    if (success){
        *var_out = result.boolean;
    }
    return(success);
}

function b32
config_compound_bool_member(Config *config, Config_Compound *compound,
                            char *var_name, i32 index, b32* var_out){
    return(config_compound_bool_member(config, compound, SCu8(var_name), index, var_out));
}

function b32
config_compound_int_member(Config *config, Config_Compound *compound,
                           String_Const_u8 var_name, i32 index, i32* var_out){
    Config_Get_Result result = config_compound_member(config, compound, var_name, index);
    b32 success = result.success && result.type == ConfigRValueType_Integer;
    if (success){
        *var_out = result.integer;
    }
    return(success);
}

function b32
config_compound_int_member(Config *config, Config_Compound *compound,
                           char *var_name, i32 index, i32* var_out){
    return(config_compound_int_member(config, compound, SCu8(var_name), index, var_out));
}

function b32
config_compound_uint_member(Config *config, Config_Compound *compound,
                            String_Const_u8 var_name, i32 index, u32* var_out){
    Config_Get_Result result = config_compound_member(config, compound, var_name, index);
    b32 success = result.success && result.type == ConfigRValueType_Integer;
    if (success){
        *var_out = result.uinteger;
    }
    return(success);
}

function b32
config_compound_uint_member(Config *config, Config_Compound *compound,
                            char *var_name, i32 index, u32* var_out){
    return(config_compound_uint_member(config, compound, SCu8(var_name), index, var_out));
}

function b32
config_compound_string_member(Config *config, Config_Compound *compound,
                              String_Const_u8 var_name, i32 index, String_Const_u8* var_out){
    Config_Get_Result result = config_compound_member(config, compound, var_name, index);
    b32 success = (result.success && result.type == ConfigRValueType_String);
    if (success){
        *var_out = result.string;
    }
    return(success);
}

function b32
config_compound_string_member(Config *config, Config_Compound *compound,
                              char *var_name, i32 index, String_Const_u8* var_out){
    return(config_compound_string_member(config, compound, SCu8(var_name), index, var_out));
}

function b32
config_compound_placed_string_member(Config *config, Config_Compound *compound,
                                     String_Const_u8 var_name, i32 index, String_Const_u8* var_out, u8 *space, u64 space_size){
    Config_Get_Result result = config_compound_member(config, compound, var_name, index);
    b32 success = (result.success && result.type == ConfigRValueType_String);
    if (success){
        u64 size = result.string.size;
        size = clamp_top(size, space_size);
        block_copy(space, result.string.str, size);
        *var_out = SCu8(space, size);
    }
    return(success);
}

function b32
config_compound_placed_string_member(Config *config, Config_Compound *compound,
                                     char *var_name, i32 index, String_Const_u8* var_out, u8 *space, u64 space_size){
    return(config_compound_placed_string_member(config, compound, SCu8(var_name), index, var_out, space, space_size));
}

function b32
config_compound_char_member(Config *config, Config_Compound *compound,
                            String_Const_u8 var_name, i32 index, char* var_out){
    Config_Get_Result result = config_compound_member(config, compound, var_name, index);
    b32 success = result.success && result.type == ConfigRValueType_Character;
    if (success){
        *var_out = result.character;
    }
    return(success);
}

function b32
config_compound_char_member(Config *config, Config_Compound *compound,
                            char *var_name, i32 index, char* var_out){
    return(config_compound_char_member(config, compound, SCu8(var_name), index, var_out));
}

function b32
config_compound_compound_member(Config *config, Config_Compound *compound,
                                String_Const_u8 var_name, i32 index, Config_Compound** var_out){
    Config_Get_Result result = config_compound_member(config, compound, var_name, index);
    b32 success = result.success && result.type == ConfigRValueType_Compound;
    if (success){
        *var_out = result.compound;
    }
    return(success);
}

function b32
config_compound_compound_member(Config *config, Config_Compound *compound,
                                char *var_name, i32 index, Config_Compound** var_out){
    return(config_compound_compound_member(config, compound, SCu8(var_name), index, var_out));
}

function Iteration_Step_Result
typed_has_array_iteration_step(Config *config, Config_Compound *compound, i32 index){
    Config_Iteration_Step_Result result = typed_array_iteration_step(config, compound, ConfigRValueType_NoType, index);
    return(result.step);
}

function Iteration_Step_Result
typed_bool_array_iteration_step(Config *config, Config_Compound *compound, i32 index, b32* var_out){
    Config_Iteration_Step_Result result = typed_array_iteration_step(config, compound, ConfigRValueType_Boolean, index);
    b32 success = (result.step == Iteration_Good);
    if (success){
        *var_out = result.get.boolean;
    }
    return(result.step);
}

function Iteration_Step_Result
typed_int_array_iteration_step(Config *config, Config_Compound *compound, i32 index, i32* var_out){
    Config_Iteration_Step_Result result = typed_array_iteration_step(config, compound, ConfigRValueType_Integer, index);
    b32 success = (result.step == Iteration_Good);
    if (success){
        *var_out = result.get.integer;
    }
    return(result.step);
}

function Iteration_Step_Result
typed_uint_array_iteration_step(Config *config, Config_Compound *compound, i32 index, u32* var_out){
    Config_Iteration_Step_Result result = typed_array_iteration_step(config, compound, ConfigRValueType_Integer, index);
    b32 success = (result.step == Iteration_Good);
    if (success){
        *var_out = result.get.uinteger;
    }
    return(result.step);
}

function Iteration_Step_Result
typed_string_array_iteration_step(Config *config, Config_Compound *compound, i32 index, String_Const_u8* var_out){
    Config_Iteration_Step_Result result = typed_array_iteration_step(config, compound, ConfigRValueType_String, index);
    b32 success = (result.step == Iteration_Good);
    if (success){
        *var_out = result.get.string;
    }
    return(result.step);
}

function Iteration_Step_Result
typed_placed_string_array_iteration_step(Config *config, Config_Compound *compound, i32 index, String_Const_u8* var_out, u8 *space, u64 space_size){
    Config_Iteration_Step_Result result = typed_array_iteration_step(config, compound, ConfigRValueType_String, index);
    b32 success = (result.step == Iteration_Good);
    if (success){
        u64 size = result.get.string.size;
        size = clamp_top(size, space_size);
        block_copy(space, result.get.string.str, size);
        *var_out = SCu8(space, size);
    }
    return(result.step);
}

function Iteration_Step_Result
typed_char_array_iteration_step(Config *config, Config_Compound *compound, i32 index, char* var_out){
    Config_Iteration_Step_Result result = typed_array_iteration_step(config, compound, ConfigRValueType_Character, index);
    b32 success = (result.step == Iteration_Good);
    if (success){
        *var_out = result.get.character;
    }
    return(result.step);
}

function Iteration_Step_Result
typed_compound_array_iteration_step(Config *config, Config_Compound *compound, i32 index, Config_Compound** var_out){
    Config_Iteration_Step_Result result = typed_array_iteration_step(config, compound, ConfigRValueType_Compound, index);
    b32 success = (result.step == Iteration_Good);
    if (success){
        *var_out = result.get.compound;
    }
    return(result.step);
}

function i32
typed_bool_array_get_count(Config *config, Config_Compound *compound){
    i32 count = typed_array_get_count(config, compound, ConfigRValueType_Boolean);
    return(count);
}

function i32
typed_int_array_get_count(Config *config, Config_Compound *compound){
    i32 count = typed_array_get_count(config, compound, ConfigRValueType_Integer);
    return(count);
}

function i32
typed_float_array_get_count(Config *config, Config_Compound *compound){
    i32 count = typed_array_get_count(config, compound, ConfigRValueType_Float);
    return(count);
}

function i32
typed_string_array_get_count(Config *config, Config_Compound *compound){
    i32 count = typed_array_get_count(config, compound, ConfigRValueType_String);
    return(count);
}

function i32
typed_character_array_get_count(Config *config, Config_Compound *compound){
    i32 count = typed_array_get_count(config, compound, ConfigRValueType_Character);
    return(count);
}

function i32
typed_compound_array_get_count(Config *config, Config_Compound *compound){
    i32 count = typed_array_get_count(config, compound, ConfigRValueType_Compound);
    return(count);
}

function i32
typed_no_type_array_get_count(Config *config, Config_Compound *compound){
    i32 count = typed_array_get_count(config, compound, ConfigRValueType_NoType);
    return(count);
}

function Config_Get_Result_List
typed_bool_array_reference_list(Arena *arena, Config *config, Config_Compound *compound){
    Config_Get_Result_List list = typed_array_reference_list(arena, config, compound, ConfigRValueType_Boolean);
    return(list);
}

function Config_Get_Result_List
typed_int_array_reference_list(Arena *arena, Config *config, Config_Compound *compound){
    Config_Get_Result_List list = typed_array_reference_list(arena, config, compound, ConfigRValueType_Integer);
    return(list);
}

function Config_Get_Result_List
typed_float_array_reference_list(Arena *arena, Config *config, Config_Compound *compound){
    Config_Get_Result_List list = typed_array_reference_list(arena, config, compound, ConfigRValueType_Float);
    return(list);
}

function Config_Get_Result_List
typed_string_array_reference_list(Arena *arena, Config *config, Config_Compound *compound){
    Config_Get_Result_List list = typed_array_reference_list(arena, config, compound, ConfigRValueType_String);
    return(list);
}

function Config_Get_Result_List
typed_character_array_reference_list(Arena *arena, Config *config, Config_Compound *compound){
    Config_Get_Result_List list = typed_array_reference_list(arena, config, compound, ConfigRValueType_Character);
    return(list);
}

function Config_Get_Result_List
typed_compound_array_reference_list(Arena *arena, Config *config, Config_Compound *compound){
    Config_Get_Result_List list = typed_array_reference_list(arena, config, compound, ConfigRValueType_Compound);
    return(list);
}

function Config_Get_Result_List
typed_no_type_array_reference_list(Arena *arena, Config *config, Config_Compound *compound){
    Config_Get_Result_List list = typed_array_reference_list(arena, config, compound, ConfigRValueType_NoType);
    return(list);
}

////////////////////////////////

function Config_Iteration_Step_Result
typed_array_iteration_step(Config *parsed, Config_Compound *compound, Config_RValue_Type type, i32 index){
    Config_Iteration_Step_Result result = {};
    result.step = Iteration_Quit;
    Config_Get_Result get_result = config_compound_member(parsed, compound, string_u8_litexpr("~"), index);
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

function i32
typed_array_get_count(Config *parsed, Config_Compound *compound, Config_RValue_Type type){
    i32 count = 0;
    for (i32 i = 0;; ++i){
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

function Config_Get_Result_List
typed_array_reference_list(Arena *arena, Config *parsed, Config_Compound *compound, Config_RValue_Type type){
    Config_Get_Result_List list = {};
    for (i32 i = 0;; ++i){
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

function void
change_mode(Application_Links *app, String_Const_u8 mode){
    fcoder_mode = FCoderMode_Original;
    if (string_match(mode, string_u8_litexpr("4coder"))){
        fcoder_mode = FCoderMode_Original;
    }
    else if (string_match(mode, string_u8_litexpr("notepad-like"))){
        begin_notepad_mode(app);
    }
    else{
        print_message(app, string_u8_litexpr("Unknown mode.\n"));
    }
}

////////////////////////////////

function Token_Array
token_array_from_text(Application_Links *app, Arena *arena, String_Const_u8 data){
    ProfileScope(app, "token array from text");
    Token_List list = lex_full_input_cpp(arena, data);
    return(token_array_from_list(arena, &list));
}

function Config*
config_from_text(Application_Links *app, Arena *arena, String_Const_u8 file_name,
                 String_Const_u8 data){
    Config *parsed = 0;
    Temp_Memory restore_point = begin_temp(arena);
    Token_Array array = token_array_from_text(app, arena, data);
    if (array.tokens != 0){
        parsed = config_parse(app, arena, file_name, data, array);
        if (parsed == 0){
            end_temp(restore_point);
        }
    }
    return(parsed);
}

////////////////////////////////

function void
config_init_default(Config_Data *config){
    config->user_name = SCu8(config->user_name_space, (u64)0);
    
    block_zero_struct(&config->code_exts);
    
    config->mapping = SCu8(config->mapping_space, (u64)0);
    config->mode = SCu8(config->mode_space, (u64)0);
    
    config->use_scroll_bars = false;
    config->use_file_bars = true;
    config->hide_file_bar_in_ui = true;
    config->use_error_highlight = true;
    config->use_jump_highlight = true;
    config->use_scope_highlight = true;
    config->use_paren_helper = true;
    config->use_comment_keyword = true;
    config->lister_whole_word_backspace_when_modified = false;
    config->show_line_number_margins = false;
    
    config->enable_virtual_whitespace = true;
    config->enable_code_wrapping = true;
    config->automatically_indent_text_on_save = true;
    config->automatically_save_changes_on_build = true;
    config->automatically_load_project = false;
    
    config->virtual_whitespace_regular_indent = 4;
    
    config->indent_with_tabs = false;
    config->indent_width = 4;
    
    config->default_theme_name = SCu8(config->default_theme_name_space, sizeof("4coder") - 1);
    block_copy(config->default_theme_name.str, "4coder", config->default_theme_name.size);
    config->highlight_line_at_cursor = true;
    
    config->default_font_name = SCu8(config->default_font_name_space, (u64)0);
    config->default_font_size = 16;
    config->default_font_hinting = false;
    
    config->default_compiler_bat = SCu8(config->default_compiler_bat_space, 2);
    block_copy(config->default_compiler_bat.str, "cl", 2);
    
    config->default_flags_bat = SCu8(config->default_flags_bat_space, (u64)0);
    
    config->default_compiler_sh = SCu8(config->default_compiler_sh_space, 3);
    block_copy(config->default_compiler_sh.str, "g++", 3);
    
    config->default_flags_sh = SCu8(config->default_flags_sh_space, (u64)0);
    
    config->lalt_lctrl_is_altgr = false;
}

function Config*
config_parse__data(Application_Links *app, Arena *arena, String_Const_u8 file_name,
                   String_Const_u8 data, Config_Data *config){
    config_init_default(config);
    
    b32 success = false;
    
    Config *parsed = config_from_text(app, arena, file_name, data);
    if (parsed != 0){
        success = true;
        
        config_fixed_string_var(parsed, "user_name", 0,
                                &config->user_name, config->user_name_space);
        
        String_Const_u8 str = {};
        if (config_string_var(parsed, "treat_as_code", 0, &str)){
            config->code_exts =
                parse_extension_line_to_extension_list(app, arena, str);
        }
        
        config_fixed_string_var(parsed, "mapping", 0, &config->mapping, config->mapping_space);
        config_fixed_string_var(parsed, "mode", 0, &config->mode, config->mode_space);
        
        config_bool_var(parsed, "use_scroll_bars", 0, &config->use_scroll_bars);
        config_bool_var(parsed, "use_file_bars", 0, &config->use_file_bars);
        config_bool_var(parsed, "hide_file_bar_in_ui", 0, &config->hide_file_bar_in_ui);
        config_bool_var(parsed, "use_error_highlight", 0, &config->use_error_highlight);
        config_bool_var(parsed, "use_jump_highlight", 0, &config->use_jump_highlight);
        config_bool_var(parsed, "use_scope_highlight", 0, &config->use_scope_highlight);
        config_bool_var(parsed, "use_paren_helper", 0, &config->use_paren_helper);
        config_bool_var(parsed, "use_comment_keyword", 0, &config->use_comment_keyword);
        config_bool_var(parsed, "lister_whole_word_backspace_when_modified", 0, &config->lister_whole_word_backspace_when_modified);
        config_bool_var(parsed, "show_line_number_margins", 0, &config->show_line_number_margins);
        
        
        config_bool_var(parsed, "enable_virtual_whitespace", 0, &config->enable_virtual_whitespace);
        config_bool_var(parsed, "enable_code_wrapping", 0, &config->enable_code_wrapping);
        config_bool_var(parsed, "automatically_indent_text_on_save", 0, &config->automatically_indent_text_on_save);
        config_bool_var(parsed, "automatically_save_changes_on_build", 0, &config->automatically_save_changes_on_build);
        config_bool_var(parsed, "automatically_load_project", 0, &config->automatically_load_project);
        
        config_int_var(parsed, "virtual_whitespace_regular_indent", 0, &config->virtual_whitespace_regular_indent);
        
        config_bool_var(parsed, "indent_with_tabs", 0, &config->indent_with_tabs);
        config_int_var(parsed, "indent_width", 0, &config->indent_width);
        
        config_fixed_string_var(parsed, "default_theme_name", 0,
                                &config->default_theme_name, config->default_theme_name_space);
        config_bool_var(parsed, "highlight_line_at_cursor", 0, &config->highlight_line_at_cursor);
        
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

function Config*
config_parse__file_handle(Application_Links *app, Arena *arena,
                          String_Const_u8 file_name, FILE *file, Config_Data *config){
    Config *parsed = 0;
    Data data = dump_file_handle(arena, file);
    if (data.data != 0){
        parsed = config_parse__data(app, arena, file_name, SCu8(data), config);
    }
    else{
        config_init_default(config);
    }
    return(parsed);
}

function Config*
config_parse__file_name(Application_Links *app, Arena *arena, char *file_name, Config_Data *config){
    Config *parsed = 0;
    b32 success = false;
    FILE *file = open_file_try_current_path_then_binary_path(app, file_name);
    if (file != 0){
        Data data = dump_file_handle(arena, file);
        fclose(file);
        if (data.data != 0){
            parsed = config_parse__data(app, arena, SCu8(file_name), SCu8(data),
                                        config);
            success = true; 
        }
    }
    if (!success){
        config_init_default(config);
    }
    return(parsed);
}

function Config*
theme_parse__data(Application_Links *app, Arena *arena, String_Const_u8 file_name, String_Const_u8 data, Arena *color_arena, Color_Table *color_table){
    Config *parsed = config_from_text(app, arena, file_name, data);
    if (parsed != 0){
        for (Config_Assignment *node = parsed->first;
             node != 0;
             node = node->next){
            Scratch_Block scratch(app);
            Config_LValue *l = node->l;
            String_Const_u8 l_name = push_string_copy(scratch, l->identifier);
            Managed_ID id = managed_id_get(app, string_u8_litexpr("colors"), l_name);
            if (id != 0){
                u32 color = 0;
                if (config_uint_var(parsed, l_name, 0, &color)){
                    color_table->arrays[id%color_table->count] = make_colors(color_arena, color);
                }
                else{
                    Config_Compound *compound = 0;
                    if (config_compound_var(parsed, l_name, 0, &compound)){
                        local_persist u32 color_array[256];
                        i32 counter = 0;
                        for (i32 i = 0;; i += 1){
                            Config_Iteration_Step_Result result = typed_array_iteration_step(parsed, compound, ConfigRValueType_Integer, i);
                            if (result.step == Iteration_Skip){
                                continue;
                            }
                            else if (result.step == Iteration_Quit){
                                break;
                            }
                            
                            color_array[counter] = result.get.uinteger;
                            counter += 1;
                            if (counter == 256){
                                break;
                            }
                        }
                        
                        color_table->arrays[id%color_table->count] = make_colors(color_arena, color_array, counter);
                    }
                }
            }
            
        }
    }
    return(parsed);
}

function Config*
theme_parse__buffer(Application_Links *app, Arena *arena, Buffer_ID buffer, Arena *color_arena, Color_Table *color_table){
    String_Const_u8 contents = push_whole_buffer(app, arena, buffer);
    Config *parsed = 0;
    if (contents.str != 0){
        String_Const_u8 file_name = push_buffer_file_name(app, arena, buffer);
        parsed = theme_parse__data(app, arena, file_name, contents, color_arena, color_table);
    }
    return(parsed);
}

function Config*
theme_parse__file_handle(Application_Links *app, Arena *arena, String_Const_u8 file_name, FILE *file, Arena *color_arena, Color_Table *color_table){
    Data data = dump_file_handle(arena, file);
    Config *parsed = 0;
    if (data.data != 0){
        parsed = theme_parse__data(app, arena, file_name, SCu8(data), color_arena, color_table);
    }
    return(parsed);
}

function Config*
theme_parse__file_name(Application_Links *app, Arena *arena, char *file_name, Arena *color_arena, Color_Table *color_table){
    Config *parsed = 0;
    FILE *file = open_file_try_current_path_then_binary_path(app, file_name);
    if (file != 0){
        Data data = dump_file_handle(arena, file);
        fclose(file);
        parsed = theme_parse__data(app, arena, SCu8(file_name), SCu8(data), color_arena, color_table);
    }
    if (parsed == 0){
        Scratch_Block scratch(app);
        String_Const_u8 str = push_u8_stringf(arena, "Did not find %s, theme not loaded", file_name);
        print_message(app, str);
    }
    return(parsed);
}

////////////////////////////////

function void
config_feedback_bool(Arena *arena, List_String_Const_u8 *list, char *name, b32 val){
    string_list_pushf(arena, list, "%s = %s;\n", name, (char*)(val?"true":"false"));
}

function void
config_feedback_string(Arena *arena, List_String_Const_u8 *list, char *name, String_Const_u8 val){
    val.size = clamp_bot(0, val.size);
    string_list_pushf(arena, list, "%s = \"%.*s\";\n", name, string_expand(val));
}

function void
config_feedback_string(Arena *arena, List_String_Const_u8 *list, char *name, char *val){
    string_list_pushf(arena, list, "%s = \"%s\";\n", name, val);
}

function void
config_feedback_extension_list(Arena *arena, List_String_Const_u8 *list, char *name, String_Const_u8_Array *extensions){
    string_list_pushf(arena, list, "%s = \"", name);
    for (i32 i = 0; i < extensions->count; ++i){
        String_Const_u8 ext = extensions->strings[i];
        string_list_pushf(arena, list, ".%.*s", string_expand(ext));
    }
    string_list_push_u8_lit(arena, list, "\";\n");
}

function void
config_feedback_int(Arena *arena, List_String_Const_u8 *list, char *name, i32 val){
    string_list_pushf(arena, list, "%s = %d;\n", name, val);
}

////////////////////////////////

function void
load_config_and_apply(Application_Links *app, Arena *out_arena, Config_Data *config,
                      i32 override_font_size, b32 override_hinting){
    Scratch_Block scratch(app);
    
    linalloc_clear(out_arena);
    Config *parsed = config_parse__file_name(app, out_arena, "config.4coder", config);
    
    if (parsed != 0){
        // Top
        print_message(app, string_u8_litexpr("Loaded config file:\n"));
        
        // Errors
        String_Const_u8 error_text = config_stringize_errors(app, scratch, parsed);
        if (error_text.str != 0){
            print_message(app, error_text);
        }
    }
    else{
        print_message(app, string_u8_litexpr("Using default config:\n"));
        Face_Description description = get_face_description(app, 0);
        if (description.font.file_name.str != 0){
            u64 size = Min(description.font.file_name.size, sizeof(config->default_font_name_space));
            block_copy(config->default_font_name_space, description.font.file_name.str, size);
            config->default_font_name.size = size;
        }
    }
    
    if (config->default_font_name.size == 0){
#define M "liberation-mono.ttf"
        block_copy(config->default_font_name_space, M, sizeof(M) - 1);
        config->default_font_name.size = sizeof(M) - 1;
#undef M
    }
    
    {
        // Values
        Temp_Memory temp2 = begin_temp(scratch);
        List_String_Const_u8 list = {};
        
        config_feedback_string(scratch, &list, "user_name", config->user_name);
        config_feedback_extension_list(scratch, &list, "treat_as_code", &config->code_exts);
        
        config_feedback_string(scratch, &list, "mapping", config->mapping);
        config_feedback_string(scratch, &list, "mode", config->mode);
        
        config_feedback_bool(scratch, &list, "use_scroll_bars", config->use_scroll_bars);
        config_feedback_bool(scratch, &list, "use_file_bars", config->use_file_bars);
        config_feedback_bool(scratch, &list, "hide_file_bar_in_ui", config->hide_file_bar_in_ui);
        config_feedback_bool(scratch, &list, "use_error_highlight", config->use_error_highlight);
        config_feedback_bool(scratch, &list, "use_jump_highlight", config->use_jump_highlight);
        config_feedback_bool(scratch, &list, "use_scope_highlight", config->use_scope_highlight);
        config_feedback_bool(scratch, &list, "use_paren_helper", config->use_paren_helper);
        config_feedback_bool(scratch, &list, "use_comment_keyword", config->use_comment_keyword);
        config_feedback_bool(scratch, &list, "lister_whole_word_backspace_when_modified", config->lister_whole_word_backspace_when_modified);
        config_feedback_bool(scratch, &list, "show_line_number_margins", config->show_line_number_margins);
        
        config_feedback_bool(scratch, &list, "enable_virtual_whitespace", config->enable_virtual_whitespace);
        config_feedback_int(scratch, &list, "virtual_whitespace_regular_indent", config->virtual_whitespace_regular_indent);
        config_feedback_bool(scratch, &list, "enable_code_wrapping", config->enable_code_wrapping);
        config_feedback_bool(scratch, &list, "automatically_indent_text_on_save", config->automatically_indent_text_on_save);
        config_feedback_bool(scratch, &list, "automatically_save_changes_on_build", config->automatically_save_changes_on_build);
        config_feedback_bool(scratch, &list, "automatically_load_project", config->automatically_load_project);
        
        config_feedback_bool(scratch, &list, "indent_with_tabs", config->indent_with_tabs);
        config_feedback_int(scratch, &list, "indent_width", config->indent_width);
        
        config_feedback_string(scratch, &list, "default_theme_name", config->default_theme_name);
        config_feedback_bool(scratch, &list, "highlight_line_at_cursor", config->highlight_line_at_cursor);
        
        config_feedback_string(scratch, &list, "default_font_name", config->default_font_name);
        config_feedback_int(scratch, &list, "default_font_size", config->default_font_size);
        config_feedback_bool(scratch, &list, "default_font_hinting", config->default_font_hinting);
        
        config_feedback_string(scratch, &list, "default_compiler_bat", config->default_compiler_bat);
        config_feedback_string(scratch, &list, "default_flags_bat", config->default_flags_bat);
        config_feedback_string(scratch, &list, "default_compiler_sh", config->default_compiler_sh);
        config_feedback_string(scratch, &list, "default_flags_sh", config->default_flags_sh);
        
        config_feedback_bool(scratch, &list, "lalt_lctrl_is_altgr", config->lalt_lctrl_is_altgr);
        
        string_list_push_u8_lit(scratch, &list, "\n");
        String_Const_u8 message = string_list_flatten(scratch, list);
        print_message(app, message);
        end_temp(temp2);
    }
    
    // Apply config
    setup_built_in_mapping(app, config->mapping, &framework_mapping, mapid_global, mapid_file, mapid_code);
    change_mode(app, config->mode);
    global_set_setting(app, GlobalSetting_LAltLCtrlIsAltGr, config->lalt_lctrl_is_altgr);
    
    Color_Table *colors = get_color_table_by_name(config->default_theme_name);
    set_active_color(colors);
    
    Face_Description description = {};
    if (override_font_size != 0){
        description.parameters.pt_size = override_font_size;
    }
    else{
        description.parameters.pt_size = config->default_font_size;
    }
    description.parameters.hinting = config->default_font_hinting || override_hinting;
    
    description.font.file_name = config->default_font_name;
    if (!modify_global_face_by_description(app, description)){
        description.font.file_name = get_file_path_in_fonts_folder(scratch, config->default_font_name);
        modify_global_face_by_description(app, description);
    }
}

function void
load_theme_file_into_live_set(Application_Links *app, char *file_name){
    Arena *arena = &global_theme_arena;
    Color_Table color_table = make_color_table(app, arena);
    Scratch_Block scratch(app);
    Config *config = theme_parse__file_name(app, scratch, file_name, arena, &color_table);
    String_Const_u8 error_text = config_stringize_errors(app, scratch, config);
    print_message(app, error_text);
    
    String_Const_u8 name = SCu8(file_name);
    name = string_front_of_path(name);
    if (string_match(string_postfix(name, 7), string_u8_litexpr(".4coder"))){
        name = string_chop(name, 7);
    }
    save_theme(color_table, name);
}

CUSTOM_COMMAND_SIG(load_theme_current_buffer)
CUSTOM_DOC("Parse the current buffer as a theme file and add the theme to the theme list. If the buffer has a .4coder postfix in it's name, it is removed when the name is saved.")
{
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
    
    Scratch_Block scratch(app);
    String_Const_u8 file_name = push_buffer_file_name(app, scratch, buffer);
    if (file_name.size > 0){
        Arena *arena = &global_theme_arena;
        Color_Table color_table = make_color_table(app, arena);
        Config *config = theme_parse__buffer(app, scratch, buffer, arena, &color_table);
        String_Const_u8 error_text = config_stringize_errors(app, scratch, config);
        print_message(app, error_text);
        
        String_Const_u8 name = string_front_of_path(file_name);
        if (string_match(string_postfix(name, 7), string_u8_litexpr(".4coder"))){
            name = string_chop(name, 7);
        }
        save_theme(color_table, name);
        
        Color_Table_Node *node = global_theme_list.last;
        if (node != 0 && string_match(node->name, name)){
            active_color_table = node->table;
        }
    }
}

function void
load_folder_of_themes_into_live_set(Application_Links *app, String_Const_u8 path){
    Scratch_Block scratch(app, Scratch_Share);
    
    File_List list = system_get_file_list(scratch, path);
    for (File_Info **ptr = list.infos, **end = list.infos + list.count;
         ptr < end;
         ptr += 1){
        File_Info *info = *ptr;
        if (!HasFlag(info->attributes.flags, FileAttribute_IsDirectory)){
            String_Const_u8 name = info->file_name;
            Temp_Memory_Block temp(scratch);
            String_Const_u8 full_name = push_u8_stringf(scratch, "%.*s/%.*s",
                                                        string_expand(path),
                                                        string_expand(name));
            load_theme_file_into_live_set(app, (char*)full_name.str);
        }
    }
}

// BOTTOM

