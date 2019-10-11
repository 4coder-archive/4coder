/*
4coder_config.cpp - Parsing *.4coder files.
*/

// TOP

internal String_Const_u8_Array
parse_extension_line_to_extension_list(Arena *arena, String_Const_u8 str){
    i32 count = 0;
    for (umem i = 0; i < str.size; i += 1){
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
        umem next_period = string_find_first(str, '.');
        String_Const_u8 extension = string_prefix(str, next_period);
        str = string_skip(str, next_period + 1);
        array.strings[i] = push_string_copy(arena, extension);
    }
    push_align(arena, 8);
    
    return(array);
}

////////////////////////////////

internal Error_Location
get_error_location(u8 *base, u8 *pos){
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

internal String_Const_u8
config_stringize_errors(Arena *arena, Config *parsed){
    String_Const_u8 result = {};
    if (parsed->errors.first != 0){
        List_String_Const_u8 list = {};
        for (Config_Error *error = parsed->errors.first;
             error != 0;
             error = error->next){
            Error_Location location = get_error_location(parsed->data.str, error->pos);
            string_list_pushf(arena, &list, "%.*s:%d:%d: %.*s\n",
                              string_expand(error->file_name), location.line_number, location.column_number, string_expand(error->text));
        }
        result = string_list_flatten(arena, list);
    }
    return(result);
}

////////////////////////////////

internal void
config_parser__advance_to_next(Config_Parser *ctx){
    Token *t = ctx->token;
    Token *e = ctx->end;
    for (t += 1;
         t < e && (t->kind == TokenBaseKind_Comment ||
                   t->kind == TokenBaseKind_Whitespace);
         t += 1);
    ctx->token = t;
}

internal Config_Parser
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

internal b32
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

internal b32
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

internal b32
config_parser__recognize_boolean(Config_Parser *ctx){
    b32 result = false;
    Token *token = ctx->token;
    if (ctx->start <= ctx->token && ctx->token < ctx->end){
        result = (token->sub_kind == TokenCppKind_LiteralTrue ||
                  token->sub_kind == TokenCppKind_LiteralFalse);
    }
    return(result);
}

internal String_Const_u8
config_parser__get_lexeme(Config_Parser *ctx){
    String_Const_u8 lexeme = {};
    Token *token = ctx->token;
    if (ctx->start <= token && token < ctx->end){
        lexeme = SCu8(ctx->data.str + token->pos, token->size);
    }
    return(lexeme);
}

internal Config_Integer
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

internal b32
config_parser__get_boolean(Config_Parser *ctx){
    String_Const_u8 str = config_parser__get_lexeme(ctx);
    return(string_match(str, string_u8_litexpr("true")));
}

internal b32
config_parser__recognize_text(Config_Parser *ctx, String_Const_u8 text){
    String_Const_u8 lexeme = config_parser__get_lexeme(ctx);
    return(lexeme.str != 0 && string_match(lexeme, text));
}

internal b32
config_parser__match_token(Config_Parser *ctx, Token_Cpp_Kind kind){
    b32 result = config_parser__recognize_token(ctx, kind);
    if (result){
        config_parser__advance_to_next(ctx);
    }
    return(result);
}

internal b32
config_parser__match_text(Config_Parser *ctx, String_Const_u8 text){
    b32 result = config_parser__recognize_text(ctx, text);
    if (result){
        config_parser__advance_to_next(ctx);
    }
    return(result);
}

#define config_parser__match_text_lit(c,s) config_parser__match_text((c), string_u8_litexpr(s))

internal Config                  *config_parser__config    (Config_Parser *ctx);
internal i32                     *config_parser__version   (Config_Parser *ctx);
internal Config_Assignment       *config_parser__assignment(Config_Parser *ctx);
internal Config_LValue           *config_parser__lvalue    (Config_Parser *ctx);
internal Config_RValue           *config_parser__rvalue    (Config_Parser *ctx);
internal Config_Compound         *config_parser__compound  (Config_Parser *ctx);
internal Config_Compound_Element *config_parser__element   (Config_Parser *ctx);

internal Config*
text_data_and_token_array_to_parse_data(Arena *arena, String_Const_u8 file_name, String_Const_u8 data, Token_Array array){
    Temp_Memory restore_point = begin_temp(arena);
    Config_Parser ctx = make_config_parser(arena, file_name, data, array);
    Config *config = config_parser__config(&ctx);
    if (config == 0){
        end_temp(restore_point);
    }
    return(config);
}

// TODO(allen): Move to string library
internal Config_Error*
config_error_push(Arena *arena, Config_Error_List *list, String_Const_u8 file_name, u8 *pos, char *error_text){
    Config_Error *error = push_array(arena, Config_Error, 1);
    zdll_push_back(list->first, list->last, error);
    list->count += 1;
    error->file_name = file_name;
    error->pos = pos;
    error->text = push_string_copy(arena, SCu8(error_text));
    return(error);
}

internal u8*
config_parser__get_pos(Config_Parser *ctx){
    return(ctx->data.str + ctx->token->pos);
}

internal void
config_parser__log_error_pos(Config_Parser *ctx, u8 *pos, char *error_text){
    config_error_push(ctx->arena, &ctx->errors, ctx->file_name, pos, error_text);
}

internal void
config_parser__log_error(Config_Parser *ctx, char *error_text){
    config_parser__log_error_pos(ctx, config_parser__get_pos(ctx), error_text);
}

internal Config*
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

internal void
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

internal i32*
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

internal Config_Assignment*
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

internal Config_LValue*
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

internal Config_RValue*
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

internal void
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

internal Config_Compound*
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

internal Config_Compound_Element*
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

internal Config_Error*
config_add_error(Arena *arena, Config *config, u8 *pos, char *error_text){
    return(config_error_push(arena, &config->errors, config->file_name, pos, error_text));
}

////////////////////////////////

internal Config_Assignment*
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

internal Config_Get_Result
config_var(Config *config, String_Const_u8 var_name, i32 subscript);

internal Config_Get_Result
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

static Config_Get_Result
config_var(Config *config, String_Const_u8 var_name, i32 subscript){
    Config_Get_Result result = {};
    Config_Assignment *assignment = config_lookup_assignment(config, var_name, subscript);
    if (assignment != 0){
        result = config_evaluate_rvalue(config, assignment, assignment->r);
    }
    return(result);
}

static Config_Get_Result
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

static Config_Iteration_Step_Result
typed_array_iteration_step(Config *parsed, Config_Compound *compound, Config_RValue_Type type, i32 index);

static i32
typed_array_get_count(Config *parsed, Config_Compound *compound, Config_RValue_Type type);

static Config_Get_Result_List
typed_array_reference_list(Arena *arena, Config *parsed, Config_Compound *compound, Config_RValue_Type type);

#define config_fixed_string_var(c,v,s,o,a) config_placed_string_var((c),(v),(s),(o),(a),sizeof(a))

////////////////////////////////

static b32
config_has_var(Config *config, String_Const_u8 var_name, i32 subscript){
    Config_Get_Result result = config_var(config, var_name, subscript);
    return(result.success && result.type == ConfigRValueType_NoType);
}

static b32
config_has_var(Config *config, char *var_name, i32 subscript){
    return(config_has_var(config, SCu8(var_name), subscript));
}

static b32
config_bool_var(Config *config, String_Const_u8 var_name, i32 subscript, b32* var_out){
    Config_Get_Result result = config_var(config, var_name, subscript);
    b32 success = (result.success && result.type == ConfigRValueType_Boolean);
    if (success){
        *var_out = result.boolean;
    }
    return(success);
}
static b32
config_bool_var(Config *config, String_Const_u8 var_name, i32 subscript, b8 *var_out){
    b32 temp = false;
    b32 success = config_bool_var(config, var_name, subscript, &temp);
    *var_out = (temp != false);
    return(success);
}
static b32
config_bool_var(Config *config, char *var_name, i32 subscript, b32* var_out){
    return(config_bool_var(config, SCu8(var_name), subscript, var_out));
}
static b32
config_bool_var(Config *config, char* var_name, i32 subscript, b8 *var_out){
    b32 temp = false;
    b32 success = config_bool_var(config, SCu8(var_name), subscript, &temp);
    *var_out = (temp != false);
    return(success);
}

static b32
config_int_var(Config *config, String_Const_u8 var_name, i32 subscript, i32* var_out){
    Config_Get_Result result = config_var(config, var_name, subscript);
    b32 success = result.success && result.type == ConfigRValueType_Integer;
    if (success){
        *var_out = result.integer;
    }
    return(success);
}

static b32
config_int_var(Config *config, char *var_name, i32 subscript, i32* var_out){
    return(config_int_var(config, SCu8(var_name), subscript, var_out));
}

static b32
config_uint_var(Config *config, String_Const_u8 var_name, i32 subscript, u32* var_out){
    Config_Get_Result result = config_var(config, var_name, subscript);
    b32 success = result.success && result.type == ConfigRValueType_Integer;
    if (success){
        *var_out = result.uinteger;
    }
    return(success);
}

static b32
config_uint_var(Config *config, char *var_name, i32 subscript, u32* var_out){
    return(config_uint_var(config, SCu8(var_name), subscript, var_out));
}

static b32
config_string_var(Config *config, String_Const_u8 var_name, i32 subscript, String_Const_u8* var_out){
    Config_Get_Result result = config_var(config, var_name, subscript);
    b32 success = result.success && result.type == ConfigRValueType_String;
    if (success){
        *var_out = result.string;
    }
    return(success);
}

static b32
config_string_var(Config *config, char *var_name, i32 subscript, String_Const_u8* var_out){
    return(config_string_var(config, SCu8(var_name), subscript, var_out));
}

static b32
config_placed_string_var(Config *config, String_Const_u8 var_name, i32 subscript, String_Const_u8* var_out, u8 *space, umem space_size){
    Config_Get_Result result = config_var(config, var_name, subscript);
    b32 success = (result.success && result.type == ConfigRValueType_String);
    if (success){
        umem size = result.string.size;
        size = clamp_top(size, space_size);
        block_copy(space, result.string.str, size);
        *var_out = SCu8(space, size);
    }
    return(success);
}

static b32
config_placed_string_var(Config *config, char *var_name, i32 subscript, String_Const_u8* var_out, u8 *space, umem space_size){
    return(config_placed_string_var(config, SCu8(var_name), subscript, var_out, space, space_size));
}

static b32
config_char_var(Config *config, String_Const_u8 var_name, i32 subscript, char* var_out){
    Config_Get_Result result = config_var(config, var_name, subscript);
    b32 success = result.success && result.type == ConfigRValueType_Character;
    if (success){
        *var_out = result.character;
    }
    return(success);
}

static b32
config_char_var(Config *config, char *var_name, i32 subscript, char* var_out){
    return(config_char_var(config, SCu8(var_name), subscript, var_out));
}

static b32
config_compound_var(Config *config, String_Const_u8 var_name, i32 subscript, Config_Compound** var_out){
    Config_Get_Result result = config_var(config, var_name, subscript);
    b32 success = result.success && result.type == ConfigRValueType_Compound;
    if (success){
        *var_out = result.compound;
    }
    return(success);
}

static b32
config_compound_var(Config *config, char *var_name, i32 subscript, Config_Compound** var_out){
    return(config_compound_var(config, SCu8(var_name), subscript, var_out));
}

static b32
config_compound_has_member(Config *config, Config_Compound *compound,
                           String_Const_u8 var_name, i32 index){
    Config_Get_Result result = config_compound_member(config, compound, var_name, index);
    b32 success = result.success && result.type == ConfigRValueType_NoType;
    return(success);
}

static b32
config_compound_has_member(Config *config, Config_Compound *compound,
                           char *var_name, i32 index){
    return(config_compound_has_member(config, compound, SCu8(var_name), index));
}

static b32
config_compound_bool_member(Config *config, Config_Compound *compound,
                            String_Const_u8 var_name, i32 index, b32* var_out){
    Config_Get_Result result = config_compound_member(config, compound, var_name, index);
    b32 success = result.success && result.type == ConfigRValueType_Boolean;
    if (success){
        *var_out = result.boolean;
    }
    return(success);
}

static b32
config_compound_bool_member(Config *config, Config_Compound *compound,
                            char *var_name, i32 index, b32* var_out){
    return(config_compound_bool_member(config, compound, SCu8(var_name), index, var_out));
}

static b32
config_compound_int_member(Config *config, Config_Compound *compound,
                           String_Const_u8 var_name, i32 index, i32* var_out){
    Config_Get_Result result = config_compound_member(config, compound, var_name, index);
    b32 success = result.success && result.type == ConfigRValueType_Integer;
    if (success){
        *var_out = result.integer;
    }
    return(success);
}

static b32
config_compound_int_member(Config *config, Config_Compound *compound,
                           char *var_name, i32 index, i32* var_out){
    return(config_compound_int_member(config, compound, SCu8(var_name), index, var_out));
}

static b32
config_compound_uint_member(Config *config, Config_Compound *compound,
                            String_Const_u8 var_name, i32 index, u32* var_out){
    Config_Get_Result result = config_compound_member(config, compound, var_name, index);
    b32 success = result.success && result.type == ConfigRValueType_Integer;
    if (success){
        *var_out = result.uinteger;
    }
    return(success);
}

static b32
config_compound_uint_member(Config *config, Config_Compound *compound,
                            char *var_name, i32 index, u32* var_out){
    return(config_compound_uint_member(config, compound, SCu8(var_name), index, var_out));
}

static b32
config_compound_string_member(Config *config, Config_Compound *compound,
                              String_Const_u8 var_name, i32 index, String_Const_u8* var_out){
    Config_Get_Result result = config_compound_member(config, compound, var_name, index);
    b32 success = (result.success && result.type == ConfigRValueType_String);
    if (success){
        *var_out = result.string;
    }
    return(success);
}

static b32
config_compound_string_member(Config *config, Config_Compound *compound,
                              char *var_name, i32 index, String_Const_u8* var_out){
    return(config_compound_string_member(config, compound, SCu8(var_name), index, var_out));
}

static b32
config_compound_placed_string_member(Config *config, Config_Compound *compound,
                                     String_Const_u8 var_name, i32 index, String_Const_u8* var_out, u8 *space, umem space_size){
    Config_Get_Result result = config_compound_member(config, compound, var_name, index);
    b32 success = (result.success && result.type == ConfigRValueType_String);
    if (success){
        umem size = result.string.size;
        size = clamp_top(size, space_size);
        block_copy(space, result.string.str, size);
        *var_out = SCu8(space, size);
    }
    return(success);
}

static b32
config_compound_placed_string_member(Config *config, Config_Compound *compound,
                                     char *var_name, i32 index, String_Const_u8* var_out, u8 *space, umem space_size){
    return(config_compound_placed_string_member(config, compound, SCu8(var_name), index, var_out, space, space_size));
}

static b32
config_compound_char_member(Config *config, Config_Compound *compound,
                            String_Const_u8 var_name, i32 index, char* var_out){
    Config_Get_Result result = config_compound_member(config, compound, var_name, index);
    b32 success = result.success && result.type == ConfigRValueType_Character;
    if (success){
        *var_out = result.character;
    }
    return(success);
}

static b32
config_compound_char_member(Config *config, Config_Compound *compound,
                            char *var_name, i32 index, char* var_out){
    return(config_compound_char_member(config, compound, SCu8(var_name), index, var_out));
}

static b32
config_compound_compound_member(Config *config, Config_Compound *compound,
                                String_Const_u8 var_name, i32 index, Config_Compound** var_out){
    Config_Get_Result result = config_compound_member(config, compound, var_name, index);
    b32 success = result.success && result.type == ConfigRValueType_Compound;
    if (success){
        *var_out = result.compound;
    }
    return(success);
}

static b32
config_compound_compound_member(Config *config, Config_Compound *compound,
                                char *var_name, i32 index, Config_Compound** var_out){
    return(config_compound_compound_member(config, compound, SCu8(var_name), index, var_out));
}

static Iteration_Step_Result
typed_has_array_iteration_step(Config *config, Config_Compound *compound, i32 index){
    Config_Iteration_Step_Result result = typed_array_iteration_step(config, compound, ConfigRValueType_NoType, index);
    return(result.step);
}

static Iteration_Step_Result
typed_bool_array_iteration_step(Config *config, Config_Compound *compound, i32 index, b32* var_out){
    Config_Iteration_Step_Result result = typed_array_iteration_step(config, compound, ConfigRValueType_Boolean, index);
    b32 success = (result.step == Iteration_Good);
    if (success){
        *var_out = result.get.boolean;
    }
    return(result.step);
}

static Iteration_Step_Result
typed_int_array_iteration_step(Config *config, Config_Compound *compound, i32 index, i32* var_out){
    Config_Iteration_Step_Result result = typed_array_iteration_step(config, compound, ConfigRValueType_Integer, index);
    b32 success = (result.step == Iteration_Good);
    if (success){
        *var_out = result.get.integer;
    }
    return(result.step);
}

static Iteration_Step_Result
typed_uint_array_iteration_step(Config *config, Config_Compound *compound, i32 index, u32* var_out){
    Config_Iteration_Step_Result result = typed_array_iteration_step(config, compound, ConfigRValueType_Integer, index);
    b32 success = (result.step == Iteration_Good);
    if (success){
        *var_out = result.get.uinteger;
    }
    return(result.step);
}

static Iteration_Step_Result
typed_string_array_iteration_step(Config *config, Config_Compound *compound, i32 index, String_Const_u8* var_out){
    Config_Iteration_Step_Result result = typed_array_iteration_step(config, compound, ConfigRValueType_String, index);
    b32 success = (result.step == Iteration_Good);
    if (success){
        *var_out = result.get.string;
    }
    return(result.step);
}

static Iteration_Step_Result
typed_placed_string_array_iteration_step(Config *config, Config_Compound *compound, i32 index, String_Const_u8* var_out, u8 *space, umem space_size){
    Config_Iteration_Step_Result result = typed_array_iteration_step(config, compound, ConfigRValueType_String, index);
    b32 success = (result.step == Iteration_Good);
    if (success){
        umem size = result.get.string.size;
        size = clamp_top(size, space_size);
        block_copy(space, result.get.string.str, size);
        *var_out = SCu8(space, size);
    }
    return(result.step);
}

static Iteration_Step_Result
typed_char_array_iteration_step(Config *config, Config_Compound *compound, i32 index, char* var_out){
    Config_Iteration_Step_Result result = typed_array_iteration_step(config, compound, ConfigRValueType_Character, index);
    b32 success = (result.step == Iteration_Good);
    if (success){
        *var_out = result.get.character;
    }
    return(result.step);
}

static Iteration_Step_Result
typed_compound_array_iteration_step(Config *config, Config_Compound *compound, i32 index, Config_Compound** var_out){
    Config_Iteration_Step_Result result = typed_array_iteration_step(config, compound, ConfigRValueType_Compound, index);
    b32 success = (result.step == Iteration_Good);
    if (success){
        *var_out = result.get.compound;
    }
    return(result.step);
}

static i32
typed_bool_array_get_count(Config *config, Config_Compound *compound){
    i32 count = typed_array_get_count(config, compound, ConfigRValueType_Boolean);
    return(count);
}

static i32
typed_int_array_get_count(Config *config, Config_Compound *compound){
    i32 count = typed_array_get_count(config, compound, ConfigRValueType_Integer);
    return(count);
}

static i32
typed_float_array_get_count(Config *config, Config_Compound *compound){
    i32 count = typed_array_get_count(config, compound, ConfigRValueType_Float);
    return(count);
}

static i32
typed_string_array_get_count(Config *config, Config_Compound *compound){
    i32 count = typed_array_get_count(config, compound, ConfigRValueType_String);
    return(count);
}

static i32
typed_character_array_get_count(Config *config, Config_Compound *compound){
    i32 count = typed_array_get_count(config, compound, ConfigRValueType_Character);
    return(count);
}

static i32
typed_compound_array_get_count(Config *config, Config_Compound *compound){
    i32 count = typed_array_get_count(config, compound, ConfigRValueType_Compound);
    return(count);
}

static i32
typed_no_type_array_get_count(Config *config, Config_Compound *compound){
    i32 count = typed_array_get_count(config, compound, ConfigRValueType_NoType);
    return(count);
}

static Config_Get_Result_List
typed_bool_array_reference_list(Arena *arena, Config *config, Config_Compound *compound){
    Config_Get_Result_List list = typed_array_reference_list(arena, config, compound, ConfigRValueType_Boolean);
    return(list);
}

static Config_Get_Result_List
typed_int_array_reference_list(Arena *arena, Config *config, Config_Compound *compound){
    Config_Get_Result_List list = typed_array_reference_list(arena, config, compound, ConfigRValueType_Integer);
    return(list);
}

static Config_Get_Result_List
typed_float_array_reference_list(Arena *arena, Config *config, Config_Compound *compound){
    Config_Get_Result_List list = typed_array_reference_list(arena, config, compound, ConfigRValueType_Float);
    return(list);
}

static Config_Get_Result_List
typed_string_array_reference_list(Arena *arena, Config *config, Config_Compound *compound){
    Config_Get_Result_List list = typed_array_reference_list(arena, config, compound, ConfigRValueType_String);
    return(list);
}

static Config_Get_Result_List
typed_character_array_reference_list(Arena *arena, Config *config, Config_Compound *compound){
    Config_Get_Result_List list = typed_array_reference_list(arena, config, compound, ConfigRValueType_Character);
    return(list);
}

static Config_Get_Result_List
typed_compound_array_reference_list(Arena *arena, Config *config, Config_Compound *compound){
    Config_Get_Result_List list = typed_array_reference_list(arena, config, compound, ConfigRValueType_Compound);
    return(list);
}

static Config_Get_Result_List
typed_no_type_array_reference_list(Arena *arena, Config *config, Config_Compound *compound){
    Config_Get_Result_List list = typed_array_reference_list(arena, config, compound, ConfigRValueType_NoType);
    return(list);
}

////////////////////////////////

static Config_Iteration_Step_Result
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

static i32
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

static Config_Get_Result_List
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

static void
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

static Token_Array
token_array_from_text(Arena *arena, String_Const_u8 data){
    Token_List list = lex_full_input_cpp(arena, data);
    return(token_array_from_list(arena, &list));
}

static Config*
text_data_to_parsed_data(Arena *arena, String_Const_u8 file_name, String_Const_u8 data){
    Config *parsed = 0;
    Temp_Memory restore_point = begin_temp(arena);
    Token_Array array = token_array_from_text(arena, data);
    if (array.tokens != 0){
        parsed = text_data_and_token_array_to_parse_data(arena, file_name, data, array);
        if (parsed == 0){
            end_temp(restore_point);
        }
    }
    return(parsed);
}

////////////////////////////////

static void
config_init_default(Config_Data *config){
    config->user_name = SCu8(config->user_name_space, (umem)0);
    
    block_zero_struct(&config->code_exts);
    
    config->mode = SCu8(config->mode_space, (umem)0);
    
    config->use_scroll_bars = false;
    config->use_file_bars = true;
    config->use_line_highlight = true;
    config->use_scope_highlight = true;
    config->use_paren_helper = true;
    config->use_comment_keyword = true;
    config->lister_whole_word_backspace_when_modified = false;
    config->show_line_number_margins = false;
    
    config->enable_virtual_whitespace = true;
    config->enable_code_wrapping = true;
    config->automatically_adjust_wrapping = true;
    config->automatically_indent_text_on_save = true;
    config->automatically_save_changes_on_build = true;
    config->automatically_load_project = false;
    
    config->indent_with_tabs = false;
    config->indent_width = 4;
    
    config->default_wrap_width = 672;
    config->default_min_base_width = 550;
    
    config->default_theme_name = SCu8(config->default_theme_name_space, sizeof("4coder") - 1);
    block_copy(config->default_theme_name.str, "4coder", config->default_theme_name.size);
    config->highlight_line_at_cursor = true;
    
    config->default_font_name = SCu8(config->default_font_name_space, (umem)0);
    config->default_font_size = 16;
    config->default_font_hinting = false;
    
    config->default_compiler_bat = SCu8(config->default_compiler_bat_space, 2);
    block_copy(config->default_compiler_bat.str, "cl", 2);
    
    config->default_flags_bat = SCu8(config->default_flags_bat_space, (umem)0);
    
    config->default_compiler_sh = SCu8(config->default_compiler_sh_space, 3);
    block_copy(config->default_compiler_sh.str, "g++", 3);
    
    config->default_flags_sh = SCu8(config->default_flags_sh_space, (umem)0);
    
    config->lalt_lctrl_is_altgr = false;
}

static Config*
config_parse__data(Arena *arena, String_Const_u8 file_name, String_Const_u8 data, Config_Data *config){
    config_init_default(config);
    
    b32 success = false;
    
    Config *parsed = text_data_to_parsed_data(arena, file_name, data);
    if (parsed != 0){
        success = true;
        
        config_fixed_string_var(parsed, "user_name", 0,
                                &config->user_name, config->user_name_space);
        
        String_Const_u8 str = {};
        if (config_string_var(parsed, "treat_as_code", 0, &str)){
            config->code_exts = parse_extension_line_to_extension_list(arena, str);
        }
        
        config_fixed_string_var(parsed, "mode", 0,
                                &config->mode, config->mode_space);
        
        config_bool_var(parsed, "use_scroll_bars", 0, &config->use_scroll_bars);
        config_bool_var(parsed, "use_file_bars", 0, &config->use_file_bars);
        config_bool_var(parsed, "use_line_highlight", 0, &config->use_line_highlight);
        config_bool_var(parsed, "use_scope_highlight", 0, &config->use_scope_highlight);
        config_bool_var(parsed, "use_paren_helper", 0, &config->use_paren_helper);
        config_bool_var(parsed, "use_comment_keyword", 0, &config->use_comment_keyword);
        config_bool_var(parsed, "lister_whole_word_backspace_when_modified", 0, &config->lister_whole_word_backspace_when_modified);
        config_bool_var(parsed, "show_line_number_margins", 0, &config->show_line_number_margins);
        
        
        config_bool_var(parsed, "enable_virtual_whitespace", 0, &config->enable_virtual_whitespace);
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

static Config*
config_parse__file_handle(Arena *arena, String_Const_u8 file_name, FILE *file, Config_Data *config){
    Config *parsed = 0;
    Data data = dump_file_handle(arena, file);
    if (data.data != 0){
        parsed = config_parse__data(arena, file_name, SCu8(data), config);
    }
    else{
        config_init_default(config);
    }
    return(parsed);
}

static Config*
config_parse__file_name(Application_Links *app, Arena *arena, char *file_name, Config_Data *config){
    Config *parsed = 0;
    b32 success = false;
    FILE *file = open_file_try_current_path_then_binary_path(app, file_name);
    if (file != 0){
        Data data = dump_file_handle(arena, file);
        fclose(file);
        if (data.data != 0){
            parsed = config_parse__data(arena, SCu8(file_name), SCu8(data), config);
            success = true; 
        }
    }
    if (!success){
        config_init_default(config);
    }
    return(parsed);
}

#if 0
static void
init_theme_zero(Theme *theme){
    for (i32 i = 0; i < Stag_COUNT; ++i){
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
        
        for (i32 i = 0; i < Stag_COUNT; ++i){
            char *name = style_tag_names[i];
            u32 color = 0;
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
#endif

////////////////////////////////

static void
config_feedback_bool(Arena *arena, List_String_Const_u8 *list, char *name, b32 val){
    string_list_pushf(arena, list, "%s = %s;\n", name, (char*)(val?"true":"false"));
}

static void
config_feedback_string(Arena *arena, List_String_Const_u8 *list, char *name, String_Const_u8 val){
    val.size = clamp_bot(0, val.size);
    string_list_pushf(arena, list, "%s = \"%.*s\";\n", name, string_expand(val));
}

static void
config_feedback_string(Arena *arena, List_String_Const_u8 *list, char *name, char *val){
    string_list_pushf(arena, list, "%s = \"%s\";\n", name, val);
}

static void
config_feedback_extension_list(Arena *arena, List_String_Const_u8 *list, char *name, String_Const_u8_Array *extensions){
    string_list_pushf(arena, list, "%s = \"", name);
    for (i32 i = 0; i < extensions->count; ++i){
        String_Const_u8 ext = extensions->strings[i];
        string_list_pushf(arena, list, ".%.*s", string_expand(ext));
    }
    string_list_push_u8_lit(arena, list, "\";\n");
}

static void
config_feedback_int(Arena *arena, List_String_Const_u8 *list, char *name, i32 val){
    string_list_pushf(arena, list, "%s = %d;\n", name, val);
}

////////////////////////////////

static void
load_config_and_apply(Application_Links *app, Arena *out_arena, Config_Data *config, i32 override_font_size, b32 override_hinting){
    Scratch_Block scratch(app);
    
    linalloc_clear(out_arena);
    Config *parsed = config_parse__file_name(app, out_arena, "config.4coder", config);
    
    if (parsed != 0){
        // Top
        print_message(app, string_u8_litexpr("Loaded config file:\n"));
        
        // Errors
        String_Const_u8 error_text = config_stringize_errors(scratch, parsed);
        if (error_text.str != 0){
            print_message(app, error_text);
        }
        
        // Values
        Temp_Memory temp2 = begin_temp(scratch);
        // TODO(allen): switch to List_String_Const_u8 for the whole config system
        List_String_Const_u8 list = {};
        
        {
            config_feedback_string(scratch, &list, "user_name", config->user_name);
            config_feedback_extension_list(scratch, &list, "treat_as_code", &config->code_exts);
            
            config_feedback_string(scratch, &list, "mode", config->mode);
            
            config_feedback_bool(scratch, &list, "use_scroll_bars", config->use_scroll_bars);
            config_feedback_bool(scratch, &list, "use_file_bars", config->use_file_bars);
            config_feedback_bool(scratch, &list, "use_line_highlight", config->use_line_highlight);
            config_feedback_bool(scratch, &list, "use_scope_highlight", config->use_scope_highlight);
            config_feedback_bool(scratch, &list, "use_paren_helper", config->use_paren_helper);
            config_feedback_bool(scratch, &list, "use_comment_keyword", config->use_comment_keyword);
            config_feedback_bool(scratch, &list, "lister_whole_word_backspace_when_modified", config->lister_whole_word_backspace_when_modified);
            config_feedback_bool(scratch, &list, "show_line_number_margins", config->show_line_number_margins);
            
            config_feedback_bool(scratch, &list, "enable_virtual_whitespace", config->enable_virtual_whitespace);
            config_feedback_bool(scratch, &list, "enable_code_wrapping", config->enable_code_wrapping);
            config_feedback_bool(scratch, &list, "automatically_indent_text_on_save", config->automatically_indent_text_on_save);
            config_feedback_bool(scratch, &list, "automatically_save_changes_on_build", config->automatically_save_changes_on_build);
            config_feedback_bool(scratch, &list, "automatically_adjust_wrapping", config->automatically_adjust_wrapping);
            config_feedback_bool(scratch, &list, "automatically_load_project", config->automatically_load_project);
            
            config_feedback_bool(scratch, &list, "indent_with_tabs", config->indent_with_tabs);
            config_feedback_int(scratch, &list, "indent_width", config->indent_width);
            
            config_feedback_int(scratch, &list, "default_wrap_width", config->default_wrap_width);
            config_feedback_int(scratch, &list, "default_min_base_width", config->default_min_base_width);
            
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
        }
        
        string_list_push_u8_lit(scratch, &list, "\n");
        String_Const_u8 message = string_list_flatten(scratch, list);
        print_message(app, message);
        end_temp(temp2);
        
        // Apply config
        change_mode(app, config->mode);
        highlight_line_at_cursor = config->use_line_highlight;
        do_matching_enclosure_highlight = config->use_scope_highlight;
        do_matching_paren_highlight = config->use_paren_helper;
        do_colored_comment_keywords = config->use_comment_keyword;
        global_set_setting(app, GlobalSetting_LAltLCtrlIsAltGr, config->lalt_lctrl_is_altgr);
        
        //change_theme(app, config->default_theme_name.str, config->default_theme_name.size);
        highlight_line_at_cursor = config->highlight_line_at_cursor;
        
        Face_Description description = {};
        description.font.file_name = config->default_font_name;
        description.font.in_4coder_font_folder = true;
        if (override_font_size != 0){
            description.parameters.pt_size = override_font_size;
        }
        else{
            description.parameters.pt_size = config->default_font_size;
        }
        description.parameters.hinting = config->default_font_hinting || override_hinting;
        if (!modify_global_face_by_description(app, description)){
            description.font.in_4coder_font_folder = !description.font.in_4coder_font_folder;
            modify_global_face_by_description(app, description);
        }
    }
}

#if 0
static void
load_theme_file_into_live_set(Application_Links *app, Partition *scratch, char *file_name){
    Temp_Memory temp = begin_temp_memory(scratch);
    Theme_Data theme = {};
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
        for (u32 i = 0; i < list.count; ++i){
            File_Info *info = &list.infos[i];
            if (info->folder){
                continue;
            }
            String info_file_name = make_string(info->filename, info->filename_len);
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
#endif

// BOTTOM

