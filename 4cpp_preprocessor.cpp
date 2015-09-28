
// TOP

// TODO(allen):
// error check the body of macros as they are first read in
// (as in, check the thing after # is a parameter, and that ## is not on one of the ends)

#define _Assert Assert
#define _TentativeAssert TentativeAssert

enum Cpp_Def_Type{
    CPP_DEFTYPE_ERROR,
    CPP_DEFTYPE_FILE,
    CPP_DEFTYPE_MACRO,
    CPP_DEFTYPE_COUNT
};

struct Table_Entry{
    String name;
    Cpp_Def_Type type;
    fcpp_u32 hash;
    int index;
};

struct Table{
    Table_Entry *table;
    int size, max_size;
};

internal fcpp_u32
get_hash(String name, Cpp_Def_Type type){
    fcpp_u32 x = 5381;
    int i = 0;
    char c;
    while (i < name.size){
        c = name.str[i++];
        x = ((x << 5) + x) + c;
    }
    x += (fcpp_u32)(type)*13;
    return x;
}

internal bool
table_insert(Table *table, Table_Entry info){
    Table_Entry entry;
    int index;
    
    info.hash = get_hash(info.name, info.type);
    index = info.hash % table->max_size;
    while ((entry = table->table[index]).name.str && entry.index != -1){
        if (entry.hash == info.hash && entry.type == info.type && match(entry.name, info.name)){
            return 1;
        }
        index = (index + 1) % table->max_size;
    }
    table->table[index] = info;
    ++table->size;
    return 0;
}

internal bool
table_find_entry(Table *table, String name, Cpp_Def_Type type, int *index_out){
    fcpp_u32 hash = get_hash(name, type);
    int index = hash % table->max_size;
    Table_Entry entry;
    while ((entry = table->table[index]).name.str){
        if (entry.index != -1 && entry.hash == hash && entry.type == type && match(entry.name, name)){
            *index_out = index;
            return 1;
        }
        index = (index + 1) % table->max_size;
    }
    return 0;
}

internal bool
table_find(Table *table, String name, Cpp_Def_Type type, int *index_out){
    bool result;
    int entry_index;
    result = table_find_entry(table, name, type, &entry_index);
    if (result){
        *index_out = table->table[entry_index].index;
    }
    return result;
}

internal bool
table_drop(Table *table, String name, Cpp_Def_Type type){
    bool result;
    int entry_index;
    result = table_find_entry(table, name, type, &entry_index);
    if (result){
        table->table[entry_index].index = -1;
    }
    return result;
}

internal void
table_copy(Table *table_src, Table *table_dst){
    Table_Entry entry;
    int i;
    for (i = 0; i < table_src->max_size; ++i){
        entry = table_src->table[i];
        if (entry.name.str){
            table_insert(table_dst, entry);
        }
    }
}

// TODO(allen): File_Data not Parse_File
struct Cpp_Parse_File{
    Cpp_File file;
    Cpp_Token_Stack tokens;
    String filename;
};

struct Cpp_Macro_Data{
    int file_index;
    int token_index;
    int param_count;
    int first_param_index;
    int body_start_index;
    int body_end_index;
};

union Cpp_Def_Slot{
    Cpp_Parse_File file;
    Cpp_Macro_Data macro;
};

struct Cpp_Loose_Token{
    int file_index;
    int token_index;
    int blocked;
};

struct Cpp_Loose_Token_Stack{
    Cpp_Loose_Token *tokens;
    int count, max;
};

struct Cpp_Parse_Context{
    int preserve_chunk_size;
};

struct Cpp_Parse_Definitions{
    Table table;
    Cpp_Def_Slot *slots;
    int count, max;
    
    int string_file_index;
    int string_write_pos;
    
    Cpp_Loose_Token eof_token;
    Cpp_Loose_Token va_args_token;
};

struct Cpp_Visit{
    int file_index;
    int token_index;
    int blocked;
};

struct Cpp_Expansion{
    int position;
    int end_position;
    int file_index;
    int macro_index;
    int stack_base;
    int stack_pop_pos;
    int out_rule;
    int out_type;
    int param_info_base;
    int invoking_file_index;
    int invoking_token_index;
    
    bool pop_and_drop;
    bool may_have_pp;
    bool is_file_level;
};

struct Cpp_Macro_Reading_Vars{
    int name_index;
    int first_param_index;
    int param_count;
    int body_start_index;
};

struct Cpp_Macro_Invoking_Vars{
    int file_index;
    int token_index;
    int invoking_file_index;
    int invoking_token_index;
    int macro_index;
    int param_count;
    int paren_level;
    int stack_base;
    int param_info_base;
    int variadic;
};

struct Cpp_Token_Range{
    int start, end;
};

struct Cpp_Preproc_State{
    Cpp_Expansion expansions[256];
    Cpp_Token_Range param_info[192];
    int expansion_level;
    int param_info_used;
    
    Cpp_Loose_Token_Stack tokens;
    
    int state;
    
    Cpp_Macro_Reading_Vars mac_read;
    Cpp_Macro_Invoking_Vars mac_inv;
    
    char *spare_string;
    int spare_string_write_pos;
    int spare_string_size;

    bool resizing_slots;
    bool finished;
};

enum Memory_Request_Purpse{
    MEMPURP_NONE,
    MEMPURP_SPARE_STRING,
    MEMPURP_PRESERVE_FILE,
    MEMPURP_TOKEN_STACK,
    MEMPURP_DEFINITION_SLOTS
};

struct Cpp_Preproc_Result{
    int file_index;
    int token_index;
    int blocked;
    
    int invoking_file_index;
    int invoking_token_index;
    
    int error_code;
    int memory_request;
    Memory_Request_Purpse memory_purpose;
    bool emit;
    bool from_macro;
    bool file_request;
};

struct Cpp_Memory_Request{
    Cpp_Preproc_State *state;
    Cpp_Parse_Definitions *definitions;
    int size;
    Memory_Request_Purpse purpose;
};

struct Cpp_File_Request{
    String filename;
};

internal int
cpp_defs_add(Cpp_Parse_Definitions *defs, String name, Cpp_Def_Type type){
    int result;
    _Assert(defs->count < defs->max);
    result = defs->count++;
    defs->slots[result] = {};
    
    if (name.str != 0){
        _Assert(defs->table.size * 7 < defs->table.max_size * 8);
        Table_Entry entry;
        entry.name = name;
        entry.type = type;
        entry.index = result;
        table_insert(&defs->table, entry);
    }
    
    return result;
}

internal Cpp_Memory_Request
cpp_get_memory_request(Cpp_Preproc_State *state, Cpp_Parse_Definitions *definitions,
                       Cpp_Preproc_Result result){
    Cpp_Memory_Request request = {};
    request.state = state;
    request.definitions = definitions;
    request.size = result.memory_request;
    request.purpose = result.memory_purpose;
    return request;
}

internal Cpp_Parse_File*
cpp_get_parse_file(Cpp_Parse_Definitions *definitions, int file_index){
    return &definitions->slots[file_index].file;
}

internal void
cpp_set_parse_file(Cpp_Parse_Definitions *definitions, int file_index, Cpp_Parse_File file){
    definitions->slots[file_index].file = file;
}

internal Cpp_Macro_Data*
cpp_get_macro_data(Cpp_Parse_Definitions *definitions, int macro_index){
    return &definitions->slots[macro_index].macro;
}

internal void
cpp_set_macro_data(Cpp_Parse_Definitions *definitions, int macro_index, Cpp_Macro_Data macro){
    definitions->slots[macro_index].macro = macro;
}

internal void*
cpp_provide_memory(Cpp_Memory_Request request, void *memory){
    void *result = 0;
    switch (request.purpose){
    case MEMPURP_SPARE_STRING:
    {
        Cpp_Preproc_State *state = request.state;
        result = state->spare_string;
        memcpy(memory, result, state->spare_string_size);
        state->spare_string_size = request.size;
        state->spare_string = (char*)memory;
    }break;
    
    case MEMPURP_PRESERVE_FILE:
    {
        persist String string_filename = make_lit_string("~ string space");
        
        Cpp_Parse_Definitions *definitions = request.definitions;
        int size = request.size >> 1;
        Cpp_Parse_File new_file = {};
        new_file.tokens.tokens = (Cpp_Token*)memory;
        new_file.tokens.max_count = size / sizeof(Cpp_Token);
        new_file.file.data = ((char*)memory) + size;
        new_file.file.size = size;
        new_file.filename = string_filename;
        
        int string_index = cpp_defs_add(definitions, {}, CPP_DEFTYPE_FILE);
        definitions->string_write_pos = 0;
        definitions->string_file_index = string_index;
        cpp_set_parse_file(definitions, string_index, new_file); 
    }break;
    
    case MEMPURP_TOKEN_STACK:
    {
        Cpp_Preproc_State *state = request.state;
        result = state->tokens.tokens;
        memcpy(memory, result, sizeof(Cpp_Loose_Token)*state->tokens.count);
        state->tokens.max = request.size / sizeof(Cpp_Loose_Token);
        state->tokens.tokens = (Cpp_Loose_Token*)memory;
    }break;
    
    case MEMPURP_DEFINITION_SLOTS:
    {
        Cpp_Preproc_State *state = request.state;
        Cpp_Parse_Definitions *definitions = request.definitions;
        if (state->resizing_slots){
            result = definitions->slots;
            memcpy(memory, result, sizeof(Cpp_Def_Slot)*(definitions->count));
            definitions->slots = (Cpp_Def_Slot*)memory;
            definitions->max = request.size / sizeof(Cpp_Def_Slot);
        }
        else{
            result = definitions->table.table;
            Table new_table;
            new_table.table = (Table_Entry*)memory;
            new_table.size = 0;
            new_table.max_size = request.size / sizeof(Table_Entry);
            memset(new_table.table, 0, request.size);
            table_copy(&definitions->table, &new_table);
            definitions->table = new_table;
        }
    }break;
    
    default:
    {
        _Assert(!"unrecognized memory request");
    }break;
    }
    return result;
}

internal Cpp_File_Request
cpp_get_file_request(Cpp_Preproc_State *state, Cpp_Preproc_Result result){
    Cpp_File_Request request = {};
    return request;
}

internal bool
cpp_has_more_files(Cpp_File_Request *request){
    return 0;
}

internal void
cpp_get_next_file(Cpp_File_Request *request){
}

internal bool
cpp_try_reuse_file(Cpp_File_Request *request){
    return 0;
}

internal void
cpp_provide_file(Cpp_File_Request *request, Cpp_File new_file, Cpp_Token_Stack new_tokens){
}

enum Preproc_Error_Code{
    PPERR_NO_ERROR,
    PPERR_UNFINISHED_DEFINE,
    PPERR_DEFINE_EXPECTED_IDENTIFIER,
    PPERR_MACRO_NONIDENTIFIER_ARG,
    PPERR_MACRO_INCOMPLETE_PARAMS,
    PPERR_MACRO_EXPECTED_COMMA,
    PPERR_MACRO_MORE_AFTER_ELLIPSIS,
    PPERR_MACROINV_TOO_FEW_PARAMS,
    PPERR_MACROINV_TOO_MANY_PARAMS,
    PPERR_EXPANSION_OVERRUN,
    PPERR_INCOMPLETE_MACROINV
};

internal String
cpp_get_error(int error_code){
    String error = {};
    switch (error_code){
    case PPERR_NO_ERROR:
        error = make_lit_string("no error"); break;
        
    case PPERR_UNFINISHED_DEFINE:
        error = make_lit_string("define directive is unfinished"); break;
        
    case PPERR_DEFINE_EXPECTED_IDENTIFIER:
        error = make_lit_string("define expected identifier for name"); break;
        
    case PPERR_MACRO_NONIDENTIFIER_ARG:
        error = make_lit_string("macro argument must be an identifier"); break;
        
    case PPERR_MACRO_INCOMPLETE_PARAMS:
        error = make_lit_string("macro incomplete parameters"); break;
        
    case PPERR_MACRO_EXPECTED_COMMA:
        error = make_lit_string("macro expected comma between parameters"); break;
        
    case PPERR_MACRO_MORE_AFTER_ELLIPSIS:
        error = make_lit_string("should not have more after the ellipsis"); break;
        
    case PPERR_EXPANSION_OVERRUN:
        error = make_lit_string("nested expansion of files and macros ran too deep"); break;
        
    case PPERR_MACROINV_TOO_FEW_PARAMS:
        error = make_lit_string("not enough arguments to macro invokation"); break;
        
    case PPERR_MACROINV_TOO_MANY_PARAMS:
        error = make_lit_string("too many arguments to macro invokation"); break;

    case PPERR_INCOMPLETE_MACROINV:
        error = make_lit_string("unexpected end of file in macro expansion"); break;
    }
    return error;
}

internal bool
cpp_recommend_termination(int error_code){
    bool result = 0;
    switch (error_code){
    case PPERR_EXPANSION_OVERRUN:
        result = 1;
    }
    return result;
}

struct Spare_String_Checkpoint{
    int write_pos_start;
    int memory_overrun;
};

internal Spare_String_Checkpoint
cpp__checkpoint_spare_string(Cpp_Preproc_State *state){
    Spare_String_Checkpoint result = {};
    result.write_pos_start = state->spare_string_write_pos;
    return result;
}

internal void
cpp__spare_write(Spare_String_Checkpoint *check, Cpp_Preproc_State *state, char c){
    if (state->spare_string_write_pos < state->spare_string_size){
        state->spare_string[state->spare_string_write_pos++] = c;
    }
    else{
        ++check->memory_overrun;
    }
}

internal void
cpp__spare_write(Spare_String_Checkpoint *check, Cpp_Preproc_State *state, String str){
    if (state->spare_string_write_pos + str.size <= state->spare_string_size){
        memcpy(state->spare_string + state->spare_string_write_pos, str.str, str.size);
        state->spare_string_write_pos += str.size;
    }
    else{
        check->memory_overrun += str.size;
    }
}

internal void
cpp__restore_spare_string(Cpp_Preproc_State *state, Spare_String_Checkpoint check){
    state->spare_string_write_pos = check.write_pos_start;
}

struct Preserve_Checkpoint{
    int start_write_pos;
    int start_token_count;
    bool out_of_memory;
};

internal Preserve_Checkpoint
cpp__checkpoint_preserve_write(Cpp_Parse_Definitions *definitions){
    Cpp_Parse_File *file = cpp_get_parse_file(definitions, definitions->string_file_index);
    Preserve_Checkpoint check;
    check.start_write_pos = definitions->string_write_pos;
    check.start_token_count = file->tokens.count;
    check.out_of_memory = 0;
    return check;
}

internal void
cpp__restore_preserve_write(Cpp_Parse_Definitions *definitions, Preserve_Checkpoint check){
    Cpp_Parse_File *file = cpp_get_parse_file(definitions, definitions->string_file_index);
    definitions->string_write_pos = check.start_write_pos;
    file->tokens.count = check.start_token_count;
}

internal void
cpp__preserve_string(Cpp_Parse_Definitions *definitions, String string){
    Cpp_Parse_File *string_file = cpp_get_parse_file(definitions, definitions->string_file_index);
    _Assert(string_file->file.size - definitions->string_write_pos >= string.size);
    copy_fast_unsafe(string_file->file.data + definitions->string_write_pos, string);
    definitions->string_write_pos += string.size;
}

internal Cpp_Loose_Token
cpp__preserve_token(Cpp_Parse_Definitions *definitions, Cpp_Token token){
    Cpp_Parse_File *string_file = cpp_get_parse_file(definitions, definitions->string_file_index);
    _Assert(string_file->tokens.count < string_file->tokens.max_count);
    Cpp_Loose_Token loose;
    loose.file_index = definitions->string_file_index;
    loose.token_index = string_file->tokens.count;
    loose.blocked = 0;
    string_file->tokens.tokens[string_file->tokens.count++] = token;
    return loose;
}

internal void
cpp__preserve_string(Preserve_Checkpoint *check, Cpp_Parse_Definitions *definitions, String string){
    if (!check->out_of_memory){
        Cpp_Parse_File *string_file = cpp_get_parse_file(definitions, definitions->string_file_index);
        if (string_file->file.size - definitions->string_write_pos >= string.size){
            copy_fast_unsafe(string_file->file.data + definitions->string_write_pos, string);
            definitions->string_write_pos += string.size;
        }
        else{
            check->out_of_memory = 1;
        }
    }
}

internal Cpp_Loose_Token
cpp__preserve_token(Preserve_Checkpoint *check, Cpp_Parse_Definitions *definitions, Cpp_Token token){
    Cpp_Loose_Token loose = {};
    if (!check->out_of_memory){
        Cpp_Parse_File *string_file = cpp_get_parse_file(definitions, definitions->string_file_index);
        if (string_file->tokens.count < string_file->tokens.max_count){
            loose.file_index = definitions->string_file_index;
            loose.token_index = string_file->tokens.count;
            loose.blocked = 0;
            string_file->tokens.tokens[string_file->tokens.count++] = token;
        }
        else{
            check->out_of_memory = 1;
        }
    }
    return loose;
}

internal bool
cpp__can_push_loose_token(Cpp_Preproc_State *state, int count){
    return (state->tokens.count + count <= state->tokens.max);
}

internal void
cpp__push_loose_token(Cpp_Preproc_State *state, int file_index, int token_index, int blocked){
    _Assert(state->tokens.count < state->tokens.max);
    state->tokens.tokens[state->tokens.count++] = {file_index, token_index, blocked};
}

struct Token_Stack_Checkpoint{
    int start_count;
    int memory_overrun;
};

internal Token_Stack_Checkpoint
cpp__checkpoint_token_stack(Cpp_Preproc_State *state){
    Token_Stack_Checkpoint result = {};
    result.start_count = state->tokens.count;
    return result;
}

internal void
cpp__restore_token_stack(Cpp_Preproc_State *state, Token_Stack_Checkpoint checkpoint){
    state->tokens.count = checkpoint.start_count;
}

internal void
cpp__push_loose_token(Token_Stack_Checkpoint *checkpoint, Cpp_Preproc_State *state,
                      int file_index, int token_index, int blocked){
    if (state->tokens.count < state->tokens.max){
        cpp__push_loose_token(state, file_index, token_index, blocked);
    }
    else{
        ++checkpoint->memory_overrun;
    }
}

internal void
cpp__finish_macro(Cpp_Parse_Definitions *definitions, String name,
                  int file_index, int token_index,
                  int param_count, int first_param_index,
                  int body_start_index, int body_end_index){
    Cpp_Macro_Data macro;
    macro.file_index = file_index;
    macro.token_index = token_index;
    macro.param_count = param_count;
    macro.first_param_index = first_param_index;
    macro.body_start_index = body_start_index;
    macro.body_end_index = body_end_index;
    
    int macro_index = cpp_defs_add(definitions, name, CPP_DEFTYPE_MACRO);
    cpp_set_macro_data(definitions, macro_index, macro);
}

enum Cpp_Expansion_Type{
    EXPAN_FILE,
    EXPAN_MACRO,
    EXPAN_ARG,
    EXPAN_PMACRO_BODY
};

enum Cpp_Expansion_Out_Type{
    EXPAN_NORMAL,
    EXPAN_BIG_PROCESS_ARGS,
    EXPAN_BIG_PROCESS_BODY,
    EXPAN_BIG_DIRECT_OUT,
    EXPAN_EOF_FLUSH,
    EXPAN_STRFY
};

internal Cpp_Expansion*
cpp__push_expansion_raw(Cpp_Preproc_State *state, Cpp_Expansion *expansion){
    if (state->expansion_level == ArrayCount(state->expansions)){
        return 0;
    }
    
    ++state->expansion_level;
    Cpp_Expansion *result = state->expansions + state->expansion_level;
    *result = *expansion;
    return result;
}

internal void
cpp__set_expansion(Cpp_Preproc_State *state, Cpp_Expansion *expansion, Cpp_Expansion_Type type,
                   int file_index, int start, int end, int out_rule,
                   int invoking_file_index, int invoking_token_index){
    expansion->position = start;
    expansion->end_position = end;
    expansion->file_index = file_index;
    expansion->macro_index = 0;
    expansion->out_rule = out_rule;
    expansion->out_type = EXPAN_NORMAL;
    expansion->invoking_file_index = invoking_file_index;
    expansion->invoking_token_index = invoking_token_index;
    if (state->expansion_level >= 1){
        expansion->stack_base = state->expansions[state->expansion_level-1].stack_base;
    }
    else{
        expansion->stack_base = 0;
    }
    
    switch (type){
    case EXPAN_FILE:
        expansion->may_have_pp = 1;
        expansion->is_file_level = 1;
        expansion->pop_and_drop = 0;
        break;
        
    case EXPAN_MACRO:
        expansion->may_have_pp = 0;
        expansion->is_file_level = 0;
        expansion->pop_and_drop = 0;
        break;
        
    case EXPAN_ARG:
        expansion->may_have_pp = 0;
        expansion->is_file_level = 1;
        expansion->pop_and_drop = 0;
        break;
        
    case EXPAN_PMACRO_BODY:
        expansion->may_have_pp = 0;
        expansion->is_file_level = 0;
        expansion->pop_and_drop = 1;
        expansion->stack_pop_pos = state->tokens.count;
        break;
    }
}

internal Cpp_Expansion*
cpp__push_expansion(Cpp_Preproc_State *state, Cpp_Expansion_Type type,
                    int file_index, int start, int end, int out_rule,
                    int invoking_file_index, int invoking_token_index){
    _Assert(start < end);
    if (state->expansion_level == ArrayCount(state->expansions)){
        return 0;
    }
    
    ++state->expansion_level;
    Cpp_Expansion *result = state->expansions + state->expansion_level;
    cpp__set_expansion(state, result, type, file_index, start, end, out_rule,
                       invoking_file_index, invoking_token_index);
    return result;
}

internal void
cpp__set_strfy_expansion(Cpp_Preproc_State *state, Cpp_Expansion *expansion,
                         int start, int end, int out_rule,
                         int invoking_file_index, int invoking_token_index){
    expansion->position = start;
    expansion->end_position = end;
    expansion->file_index = -1;
    expansion->macro_index = 0;
    expansion->out_rule = out_rule;
    expansion->out_type = EXPAN_STRFY;
    expansion->invoking_file_index = invoking_file_index;
    expansion->invoking_token_index = invoking_token_index;
    
    expansion->may_have_pp = 0;
    expansion->is_file_level = 0;
    expansion->pop_and_drop = 0;
    
    if (state->expansion_level >= 1){
        expansion->stack_base = state->expansions[state->expansion_level-1].stack_base;
    }
    else{
        expansion->stack_base = 0;
    }
}

internal Cpp_Expansion*
cpp__push_strfy_expansion(Cpp_Preproc_State *state, int start, int end, int out_rule,
                          int invoking_file_index, int invoking_token_index){
    _Assert(start < end);
    if (state->expansion_level == ArrayCount(state->expansions)){
        return 0;
    }
    
    ++state->expansion_level;
    Cpp_Expansion *result = state->expansions + state->expansion_level;
    cpp__set_strfy_expansion(state, result, start, end, out_rule,
                             invoking_file_index, invoking_token_index);
    return result;
}

internal void
cpp__set_big_expansion(Cpp_Expansion *expansion, int macro_index, int stack_base,
                       int param_info_base, int out_rule,
                       int invoking_file_index, int invoking_token_index){
    expansion->stack_base = stack_base;
    expansion->file_index = macro_index;
    expansion->macro_index = 0;
    expansion->out_rule = out_rule;
    expansion->out_type = EXPAN_BIG_PROCESS_ARGS;
    expansion->param_info_base = param_info_base;
    expansion->invoking_file_index = invoking_file_index;
    expansion->invoking_token_index = invoking_token_index;
    expansion->pop_and_drop = 0;
    expansion->may_have_pp = 0;
    expansion->is_file_level = 0;
}

internal Cpp_Expansion*
cpp__push_big_expansion(Cpp_Preproc_State *state, int macro_index, int stack_base, int param_info_base,
                        int out_rule, int invoking_file_index, int invoking_token_index){
    if (state->expansion_level == ArrayCount(state->expansions)){
        return 0;
    }
    ++state->expansion_level;
    Cpp_Expansion *result = state->expansions + state->expansion_level;
    cpp__set_big_expansion(result, macro_index, stack_base, param_info_base, out_rule,
                           invoking_file_index, invoking_token_index);
    return result;
}

internal int
cpp__alloc_param_table(Cpp_Preproc_State *state, int param_count){
    _Assert(state->param_info_used + param_count * 3 <= ArrayCount(state->param_info));
    int result = state->param_info_used;
    state->param_info_used += param_count * 3;
    memset(state->param_info + result, 0, sizeof(Cpp_Token_Range)*param_count*3);
    return result;
}

internal void
cpp__free_param_table(Cpp_Preproc_State *state, int param_count){
    _Assert(state->param_info_used >= param_count * 3);
    state->param_info_used -= param_count * 3;
}

enum Param_Info_Position{
    RAW_START,
    RAW_END,
    STRFY_START,
    STRFY_END,
    EXPANDED_START,
    EXPANDED_END
};

enum Param_Info_Range_Position{
    RAW,
    STRFY,
    EXPANDED
};

internal Cpp_Token_Range
cpp__param_info_get(Cpp_Preproc_State *state, int base, int arg_index, Param_Info_Range_Position pos){
    Cpp_Token_Range *range = state->param_info + (base + arg_index*3);
    range += pos;
    return *range;
}

internal void
cpp__param_info_set(Cpp_Preproc_State *state, int base, int arg_index, Param_Info_Position pos, int val){
    Cpp_Token_Range *range = state->param_info + (base + arg_index*3);
    range += (pos >> 1);
    *( ((int*)range) + (pos & 1) ) = val;
}

internal void
cpp_set_target(Cpp_Preproc_State *state, Cpp_Parse_Definitions *definitions,
               Cpp_File file, Cpp_Token_Stack tokens, String filename){
    int target_index;
    target_index = cpp_defs_add(definitions, filename, CPP_DEFTYPE_FILE);
    cpp_set_parse_file(definitions, target_index, {file, tokens, filename});
    state->expansion_level = -1;
    cpp__push_expansion(state, EXPAN_FILE, target_index, 0, tokens.count, 0, -1, -1);
}

internal void
cpp__preproc_pop_expansion(Cpp_Preproc_State *state, Cpp_Expansion *expansion){
    if (expansion->is_file_level){
        _Assert(expansion->out_type != EXPAN_EOF_FLUSH);
        expansion->out_type = EXPAN_EOF_FLUSH;
    }
    
    else{
        if (expansion->pop_and_drop){
            Cpp_Loose_Token *tokens = state->tokens.tokens;
            int count = state->tokens.count - expansion->stack_pop_pos;
            memmove(tokens + expansion->stack_base, tokens + expansion->stack_pop_pos,
                    sizeof(Cpp_Loose_Token)*count);
            state->tokens.count = expansion->stack_base + count;
            state->mac_inv.stack_base -= (expansion->stack_pop_pos - expansion->stack_base);
        }
        
        if (state->expansion_level == 0){
            state->finished = 1;
        }
        else{
            --state->expansion_level;
        }
    }
}

enum Preproc_State{
    PPS_DEFAULT,
    PPS_MACRO_NAME,
    PPS_MACRO_PARAM_OR_CLOSE,
    PPS_MACRO_BODY,
    PPS_MACRO_COMMA_OR_CLOSE,
    PPS_MACRO_PARAM,
    PPS_MACRO_CLOSE,
    PPS_MACRO_BODY_OR_PARAM_OPEN,
    PPS_MACRO_ERROR,
    PPS_MACROINV_OPEN,
    PPS_MACROINV_PARAMS
};

internal Cpp_Preproc_Result
cpp__preproc_normal_step_nonalloc(Cpp_Preproc_State *state, Cpp_Parse_Definitions *definitions,
                                  Cpp_Parse_Context *context){
    Cpp_Preproc_Result result = {};
    Cpp_Expansion to_push_later = {};
    int do_push_later = 0;
    Cpp_Expansion *expansion = state->expansions + state->expansion_level;
    
    if (expansion->position == expansion->end_position && expansion->out_type == EXPAN_NORMAL){
        cpp__preproc_pop_expansion(state, expansion);
        result = {};
        return result;
    }
    
    if (!cpp__can_push_loose_token(state, 1)){
        result.memory_request = sizeof(Cpp_Loose_Token)*((state->tokens.count*2) + 1);
        result.memory_purpose = MEMPURP_TOKEN_STACK;
        return result;
    }
    
    Cpp_Visit visit = {};
    Cpp_Parse_File visit_file;
    Cpp_Token visit_token;
    
    if (expansion->out_type == EXPAN_NORMAL){
        if (expansion->file_index >= 0){
            visit.file_index = expansion->file_index;
            visit.token_index = expansion->position;
            visit.blocked = 0;
        }
        else{
            Cpp_Loose_Token loose_token = state->tokens.tokens[expansion->position];
            visit.file_index = loose_token.file_index;
            visit.token_index = loose_token.token_index;
            visit.blocked = loose_token.blocked;
        }
    }
    else if (expansion->out_type == EXPAN_EOF_FLUSH){
        visit.file_index = definitions->eof_token.file_index;
        visit.token_index = definitions->eof_token.token_index;
        visit.blocked = 0;
    }
    
    visit_file = *cpp_get_parse_file(definitions, visit.file_index);
    visit_token = visit_file.tokens.tokens[visit.token_index];
    
    result.file_index = visit.file_index;
    result.token_index = visit.token_index;
    result.blocked = visit.blocked;
    result.invoking_file_index = expansion->invoking_file_index;
    result.invoking_token_index = expansion->invoking_token_index;
    result.from_macro = !expansion->is_file_level;
    
    bool step_forward = 1;
    switch (state->state){
    case PPS_MACRO_NAME:
    {
        if (visit_token.flags & CPP_TFLAG_PP_BODY){
            switch (visit_token.type){
            case CPP_TOKEN_COMMENT:
            case CPP_TOKEN_JUNK:break;
            
            case CPP_TOKEN_IDENTIFIER:
            {
                state->mac_read.name_index = visit.token_index;
                state->state = PPS_MACRO_BODY_OR_PARAM_OPEN;
            }break;
            
            default:
            {
                result.error_code = PPERR_DEFINE_EXPECTED_IDENTIFIER;
                state->state = PPS_MACRO_ERROR;
            }break;
            }
        }
        else{
            result.error_code = PPERR_UNFINISHED_DEFINE;
            state->state = PPS_DEFAULT;
            step_forward = 0;
        }
    }break;
    
    case PPS_MACRO_BODY_OR_PARAM_OPEN:
    {
        if (visit_token.flags & CPP_TFLAG_PP_BODY){
            switch (visit_token.type){
            case CPP_TOKEN_COMMENT:
            case CPP_TOKEN_JUNK:break;
                
            case CPP_TOKEN_PARENTHESE_OPEN:
            {
                state->state = PPS_MACRO_PARAM_OR_CLOSE;
            }break;
            
            default:
            {
                state->mac_read.param_count = -1;
                state->mac_read.body_start_index = visit.token_index;
                state->state = PPS_MACRO_BODY;
            }break;
            }
        }
        else{
            Cpp_Token token = visit_file.tokens.tokens[state->mac_read.name_index];
            String name = make_string(visit_file.file.data + token.start, token.size);
            cpp__finish_macro(definitions, name, visit.file_index, state->mac_read.name_index, -1, 0, 0, 0);
            state->state = PPS_DEFAULT;
            step_forward = 0;
        }
    }break;
    
    case PPS_MACRO_BODY:
    {
        if (!(visit_token.flags & CPP_TFLAG_PP_BODY)){
            Cpp_Token token = visit_file.tokens.tokens[state->mac_read.name_index];
            String name = make_string(visit_file.file.data + token.start, token.size);
            cpp__finish_macro(definitions, name,
                              visit.file_index, state->mac_read.name_index,
                              state->mac_read.param_count, state->mac_read.first_param_index,
                              state->mac_read.body_start_index, visit.token_index);
            state->state = PPS_DEFAULT;
            step_forward = 0;
        }
    }break;
    
    case PPS_MACRO_PARAM_OR_CLOSE:
    {
        if (visit_token.flags & CPP_TFLAG_PP_BODY){
            switch (visit_token.type){
            case CPP_TOKEN_COMMENT:
            case CPP_TOKEN_JUNK:break;
                
            case CPP_TOKEN_PARENTHESE_CLOSE:
            {
                state->mac_read.body_start_index = visit.token_index + 1;
                state->state = PPS_MACRO_BODY;
            }break;
            
            case CPP_TOKEN_IDENTIFIER:
            {
                state->mac_read.first_param_index = visit.token_index;
                ++state->mac_read.param_count;
                state->state = PPS_MACRO_COMMA_OR_CLOSE;
            }break;
            
            default:
            {
                result.error_code = PPERR_MACRO_NONIDENTIFIER_ARG;
                state->state = PPS_MACRO_ERROR;
            }break;
            }
        }
        else{
            result.error_code = PPERR_MACRO_INCOMPLETE_PARAMS;
            state->state = PPS_DEFAULT;
            step_forward = 0;
        }
    }break;
    
    case PPS_MACRO_COMMA_OR_CLOSE:
    {
        if (visit_token.flags & CPP_TFLAG_PP_BODY){
            switch (visit_token.type){
            case CPP_TOKEN_COMMENT:
            case CPP_TOKEN_JUNK:break;
                
            case CPP_TOKEN_PARENTHESE_CLOSE:
            {
                state->mac_read.body_start_index = visit.token_index + 1;
                state->state = PPS_MACRO_BODY;
            }break;
            
            case CPP_TOKEN_COMMA:
            {
                state->state = PPS_MACRO_PARAM;
            }break;
            
            default:
            {
                result.error_code = PPERR_MACRO_EXPECTED_COMMA;
                state->state = PPS_MACRO_ERROR;
            }break;
            }
        }
        else{
            result.error_code = PPERR_MACRO_INCOMPLETE_PARAMS;
            state->state = PPS_DEFAULT;
            step_forward = 0;
        }
    }break;
    
    case PPS_MACRO_PARAM:
    {
        if (visit_token.flags & CPP_TFLAG_PP_BODY){
            switch (visit_token.type){
            case CPP_TOKEN_COMMENT:
            case CPP_TOKEN_JUNK:break;
                
            case CPP_TOKEN_IDENTIFIER:
            {
                ++state->mac_read.param_count;
                state->state = PPS_MACRO_COMMA_OR_CLOSE;
            }break;
            
            case CPP_TOKEN_ELLIPSIS:
            {
                ++state->mac_read.param_count;
                state->state = PPS_MACRO_CLOSE;
            }break;
            
            default:
            {
                result.error_code = PPERR_MACRO_NONIDENTIFIER_ARG;
                state->state = PPS_MACRO_ERROR;
            }break;
            }
        }
        else{
            result.error_code = PPERR_MACRO_INCOMPLETE_PARAMS;
            state->state = PPS_DEFAULT;
            step_forward = 0;
        }
    }break;
    
    case PPS_MACRO_CLOSE:
    {
        if (visit_token.flags & CPP_TFLAG_PP_BODY){
            switch (visit_token.type){
            case CPP_TOKEN_COMMENT:
            case CPP_TOKEN_JUNK:break;
                
            case CPP_TOKEN_PARENTHESE_CLOSE:
            {
                state->mac_read.body_start_index = visit.token_index + 1;
                state->state = PPS_MACRO_BODY;
            }break;
            
            default:
            {
                result.error_code = PPERR_MACRO_MORE_AFTER_ELLIPSIS;
                state->state = PPS_MACRO_ERROR;
            }break;
            }
        }
        else{
            result.error_code = PPERR_MACRO_INCOMPLETE_PARAMS;
            state->state = PPS_DEFAULT;
            step_forward = 0;
        }
    }break;
    
    case PPS_MACRO_ERROR:
    {
        if (!(visit_token.flags & CPP_TFLAG_PP_BODY)){
            state->state = PPS_DEFAULT;
            step_forward = 0;
        }
    }break;
    
    case PPS_MACROINV_OPEN:
    {
        switch (visit_token.type){
        case CPP_TOKEN_COMMENT:
        case CPP_TOKEN_JUNK:break;
            
        case CPP_TOKEN_PARENTHESE_OPEN:
        {
            Cpp_Macro_Data *macro = cpp_get_macro_data(definitions, state->mac_inv.macro_index);
            state->mac_inv.param_info_base = cpp__alloc_param_table(state, macro->param_count);
            state->state = PPS_MACROINV_PARAMS;
        }break;
        
        default:
        {
            state->tokens.count = state->mac_inv.stack_base;
            
            result.file_index = state->mac_inv.file_index;
            result.token_index = state->mac_inv.token_index;
            result.blocked = 0;
            result.invoking_file_index = state->mac_inv.invoking_file_index;
            result.invoking_token_index = state->mac_inv.invoking_token_index;
            result.from_macro = (result.invoking_file_index != -1);
            result.emit = 1;
            
            step_forward = 0;
            
            state->state = PPS_DEFAULT;
        }break;
        }
    }break;
    
    case PPS_MACROINV_PARAMS:
    {
        enum Macroinv_Params_Action{
            MIA_NONE,
            MIA_EXTEND_PARAM,
            MIA_NEXT_PARAM,
            MIA_FINISH,
            MIA_CLEANUP
        };
        
        Macroinv_Params_Action action = MIA_NONE;
        Cpp_Macro_Data macro = *cpp_get_macro_data(definitions, state->mac_inv.macro_index);
        switch (visit_token.type){
        case CPP_TOKEN_COMMENT:
        case CPP_TOKEN_JUNK:break;
            
        case CPP_TOKEN_PARENTHESE_OPEN:
        {
            action = MIA_EXTEND_PARAM;
            ++state->mac_inv.paren_level;
        }break;
        
        case CPP_TOKEN_PARENTHESE_CLOSE:
        {
            if (state->mac_inv.paren_level == 0){
                action = MIA_FINISH;
            }
            else{
                --state->mac_inv.paren_level;
                action = MIA_EXTEND_PARAM;
            }
        }break;
        
        case CPP_TOKEN_COMMA:
        {
            if (state->mac_inv.paren_level != 0 ||
                (state->mac_inv.variadic &&
                 (state->mac_inv.param_count == macro.param_count ||
                  macro.param_count == 1))){
                action = MIA_EXTEND_PARAM;
            }
            else{
                action = MIA_NEXT_PARAM;
            }
        }break;
        
        case CPP_TOKEN_EOF:
        {
            action = MIA_CLEANUP;
            result.error_code = PPERR_INCOMPLETE_MACROINV;
        }break;
        
        default:
        {
            action = MIA_EXTEND_PARAM;
        }break;
        }
        
        switch (action){
        case MIA_EXTEND_PARAM:
        {
            if (state->mac_inv.param_count == 0){
                state->mac_inv.param_count = 1;
                cpp__param_info_set(state, state->mac_inv.param_info_base,
                                    0, RAW_START, state->tokens.count);
            }
            // NOTE(allen): this function gaurantees at the head end that there is
            // room for at least one token
            cpp__push_loose_token(state, visit.file_index, visit.token_index, 0);
        }break;
        
        case MIA_NEXT_PARAM:
        {
            if (state->mac_inv.param_count == 0){
                state->mac_inv.param_count = 1;
                cpp__param_info_set(state, state->mac_inv.param_info_base,
                                    0, RAW_START, state->tokens.count);
            }
            
            if (state->mac_inv.param_count <= macro.param_count){
                cpp__param_info_set(state, state->mac_inv.param_info_base,
                                    state->mac_inv.param_count - 1, RAW_END, state->tokens.count);
                ++state->mac_inv.param_count;
                cpp__param_info_set(state, state->mac_inv.param_info_base,
                                    state->mac_inv.param_count - 1, RAW_START, state->tokens.count);
            }
            else{
                ++state->mac_inv.param_count;
            }
        }break;
        
        case MIA_FINISH:
        {
            if (state->mac_inv.param_count != 0 && state->mac_inv.param_count <= macro.param_count){
                cpp__param_info_set(state, state->mac_inv.param_info_base,
                                    state->mac_inv.param_count - 1, RAW_END, state->tokens.count);
            }
            if (state->mac_inv.param_count != macro.param_count){
                if (state->mac_inv.param_count < macro.param_count){
                    result.error_code = PPERR_MACROINV_TOO_FEW_PARAMS;
                }
                else{
                    result.error_code = PPERR_MACROINV_TOO_MANY_PARAMS;
                }
            }
            state->state = PPS_DEFAULT;
            
            do_push_later = 2;
            cpp__set_big_expansion(&to_push_later, state->mac_inv.macro_index,
                                   state->mac_inv.stack_base, state->mac_inv.param_info_base,
                                   expansion->out_rule, visit.file_index, visit.token_index);
        }break;
        
        case MIA_CLEANUP:
        {
            state->tokens.count = state->mac_inv.stack_base;
            cpp__free_param_table(state, state->mac_inv.param_count);
        }break;
        }
    }break;
    
    case PPS_DEFAULT:
    {
        switch (visit_token.type){
        case CPP_TOKEN_COMMENT:
        case CPP_TOKEN_JUNK:
        case CPP_TOKEN_EOF:break;
            
        case CPP_PP_DEFINE:
        {
            state->mac_read = {};
            state->state = PPS_MACRO_NAME;
        }break;
        
        case CPP_TOKEN_IDENTIFIER:
        {
            String name = make_string(visit_file.file.data + visit_token.start, visit_token.size);
            int macro_index;
            if (table_find(&definitions->table, name, CPP_DEFTYPE_MACRO, &macro_index)){
                bool blocked = (visit.blocked != 0);
                if (!blocked){
                    for (int i = state->expansion_level; i >= 0; --i){
                        if (macro_index == state->expansions[i].macro_index){
                            blocked = 1;
                            break;
                        }
                    }
                }
                
                if (!blocked){
                    Cpp_Macro_Data *macro = cpp_get_macro_data(definitions, macro_index);
                    if (macro->param_count == -1){
                        if (macro->body_start_index < macro->body_end_index){
                            do_push_later = 1;
                            cpp__set_expansion(state, &to_push_later, EXPAN_MACRO, macro->file_index,
                                               macro->body_start_index, macro->body_end_index,
                                               expansion->out_rule, visit.file_index, visit.token_index);
                            to_push_later.macro_index = macro_index;
                        }
                    }
                    
                    else{
                        int stack_start = state->tokens.count;
                        // NOTE(allen): luckily the number of tokens about to be pushed is known
                        // up front in this case. No states have been changed yet so it's okay
                        // allowing a memory request to be issued from here.
                        if (cpp__can_push_loose_token(state, macro->param_count)){
                            bool variadic = 0;
                            if (macro->param_count != 0){
                                int macro_file_index = macro->file_index;
                                Cpp_Parse_File file = *cpp_get_parse_file(definitions, macro_file_index);
                                int i = macro->first_param_index;
                                Cpp_Token token = file.tokens.tokens[i];
                                for (;;){
                                    if (token.type == CPP_TOKEN_IDENTIFIER){
                                        cpp__push_loose_token(state, macro_file_index, i, 0);
                                    }
                                    if (token.type == CPP_TOKEN_ELLIPSIS){
                                        Cpp_Loose_Token va_args = definitions->va_args_token;
                                        cpp__push_loose_token(state, va_args.file_index,
                                                              va_args.token_index, 0);
                                        variadic = 1;
                                    }
                                    
                                    ++i;
                                    if (i >= file.tokens.count){
                                        break;
                                    }
                                    token = file.tokens.tokens[i];
                                    if (token.type == CPP_TOKEN_PARENTHESE_CLOSE){
                                        break;
                                    }
                                }
                                _Assert(state->tokens.count - stack_start == macro->param_count);
                            }
                            
                            state->mac_inv.file_index = visit.file_index;
                            state->mac_inv.token_index = visit.token_index;
                            
                            state->mac_inv.invoking_file_index = result.invoking_file_index;
                            state->mac_inv.invoking_token_index = result.invoking_token_index;
                            
                            state->mac_inv.macro_index = macro_index;
                            state->mac_inv.param_count = 0;
                            state->mac_inv.paren_level = 0;
                            state->mac_inv.stack_base = stack_start;
                            state->mac_inv.variadic = variadic;
                            
                            state->state = PPS_MACROINV_OPEN;
                        }
                        else{
                            result.memory_request = sizeof(Cpp_Loose_Token)*
                                ((state->tokens.max*2) + macro->param_count);
                            result.memory_purpose = MEMPURP_TOKEN_STACK;
                            step_forward = 0;
                        }
                    }
                }
                else{
                    result.emit = 1;
                    result.blocked = 1;
                }
            }
            else{
                result.emit = 1;
            }
        }break;
        
        default:
        {
            result.emit = 1;
        }break;
        }
    }break;
    
    default:
    {
        _Assert(!"bad state->state");
    }break;
    }
    
    if (expansion->out_rule != 0 && result.emit){
        cpp__push_loose_token(state, result.file_index, result.token_index, result.blocked);
        result.emit = 0;
        if (expansion->out_rule > 0){
            if (state->param_info[expansion->out_rule].start == 0){
                state->param_info[expansion->out_rule].start = state->tokens.count - 1;
            }
            state->param_info[expansion->out_rule].end = state->tokens.count;
        }
    }
    
    if (expansion->out_type == EXPAN_NORMAL){
        if (step_forward){
            ++expansion->position;
            if (expansion->position == expansion->end_position && do_push_later == 0){
                cpp__preproc_pop_expansion(state, expansion);
            }
        }
    }
    else{
        if (state->expansion_level == 0){
            state->finished = 1;
        }
        else{
            --state->expansion_level;
        }
    }
    
    if (do_push_later){
        cpp__push_expansion_raw(state, &to_push_later);
    }
    
    return result;
}

internal int
cpp__get_parameter_i(Cpp_Preproc_State *state, Cpp_Parse_Definitions * definitions,
                     Cpp_Parse_File *macro_file, Cpp_Token token, int param_start, int param_count){
    int param_i = -1;
    if (token.type == CPP_TOKEN_IDENTIFIER){
        String token_str = make_string(macro_file->file.data + token.start, token.size);
        for (int j = 0; j < param_count; ++j){
            Cpp_Loose_Token param_loose = state->tokens.tokens[j + param_start];
            Cpp_Parse_File *file = cpp_get_parse_file(definitions, param_loose.file_index);
            Cpp_Token param_token = file->tokens.tokens[param_loose.token_index];
            String param_str = make_string(file->file.data + param_token.start, param_token.size);
            if (match(token_str, param_str)){
                param_i = j;
                break;
            }
        }
    }
    return param_i;
}

internal Cpp_Preproc_Result
cpp__preproc_big_step_nonalloc(Cpp_Preproc_State *state, Cpp_Parse_Definitions *definitions,
                               Cpp_Parse_Context *context){
    Cpp_Preproc_Result result = {};
    Cpp_Expansion *expansion = state->expansions + state->expansion_level;
    
    Cpp_Macro_Data macro = *cpp_get_macro_data(definitions, expansion->file_index);
    Cpp_Parse_File *macro_file = cpp_get_parse_file(definitions, macro.file_index);
    switch (expansion->out_type){
    case EXPAN_BIG_PROCESS_ARGS:
    {
        bool expand_arg[64];
        bool strfy_arg[64];
        _Assert(macro.param_count < 64);
        memset(expand_arg, 0, macro.param_count);
        memset(strfy_arg, 0, macro.param_count);
        
        bool prev_was_paste = 0;
        bool next_is_paste = 0;
        bool prev_was_strfy = 0;
        Cpp_Token token;
        int param_i;
        for (int i = macro.body_start_index; i < macro.body_end_index; ++i){
            token = macro_file->tokens.tokens[i];
            param_i = cpp__get_parameter_i(state, definitions, macro_file, token,
                                           expansion->stack_base, macro.param_count);
            
            next_is_paste = (i+1 < macro.body_end_index &&
                             macro_file->tokens.tokens[i+1].type == CPP_PP_CONCAT);
            
            if (param_i != -1){
                if (prev_was_strfy){
                    strfy_arg[param_i] = 1;
                }
                else if (!prev_was_paste && !next_is_paste){
                    expand_arg[param_i] = 1;
                }
            }
            
            prev_was_strfy = (token.type == CPP_PP_STRINGIFY);
            prev_was_paste = (token.type == CPP_PP_CONCAT && i != macro.body_start_index);
        }
        
        int param_info_base = expansion->param_info_base;
        for (int i = 0; i < macro.param_count; ++i){
            Cpp_Token_Range range = cpp__param_info_get(state, param_info_base, i, RAW);
            
            if (strfy_arg[i]){
                Cpp_Expansion *new_expansion =
                    cpp__push_strfy_expansion(state, range.start, range.end,
                                              param_info_base + i*3 + 1,
                                              expansion->invoking_file_index,
                                              expansion->invoking_token_index);
                if (new_expansion == 0){
                    result.error_code = PPERR_EXPANSION_OVERRUN;
                    break;
                }
            }
            
            if (expand_arg[i]){
                if (range.start < range.end){
                    Cpp_Expansion *new_expansion =
                        cpp__push_expansion(state, EXPAN_ARG, -1, range.start, range.end,
                                            param_info_base + i*3 + 2,
                                            expansion->invoking_file_index,
                                            expansion->invoking_token_index);
                    if (new_expansion == 0){
                        result.error_code = PPERR_EXPANSION_OVERRUN;
                        break;
                    }
                }
            }
        }
        expansion->out_type = EXPAN_BIG_PROCESS_BODY;
    }break;
    
    case EXPAN_BIG_PROCESS_BODY:
    {
        int start_pos = state->tokens.count;
        
        // TODO(allen): because it is possible that this be used more than once, it is possible that
        // all of this code happens any number of times before it finally runs through with enough
        // memory.  Perhaps loft this checkpoint out and try the whole thing in one big checkpoint?
        // Some sort of way of determining the max size needed in spare string?
        Spare_String_Checkpoint str_checkpoint = {};
        Preserve_Checkpoint pres_checkpoint = cpp__checkpoint_preserve_write(definitions);
        Token_Stack_Checkpoint checkpoint = cpp__checkpoint_token_stack(state);
        bool next_is_paste = 0;
        for (int i = macro.body_start_index; i < macro.body_end_index; ++i){
            Cpp_Token token = macro_file->tokens.tokens[i];
            
            next_is_paste = (i+1 < macro.body_end_index &&
                             macro_file->tokens.tokens[i+1].type == CPP_PP_CONCAT);
            
            enum Body_Builder_Action{
                BBA_NONE,
                BBA_NORMAL,
                BBA_STRFY,
                BBA_PASTE
            };
            
            int action = BBA_NONE;
            int param_i = -1;
            int i2 = -1;
            
            if (token.type == CPP_PP_STRINGIFY){
                i += 1;
                if (i < macro.body_end_index){
                    action = BBA_STRFY;
                }
                else{
                    i -= 1;
                    action = BBA_NONE;
                }
            }
            else if (next_is_paste){
                i2 = i + 2;
                if (i2 < macro.body_end_index){
                    action = BBA_PASTE;
                }
                else{
                    action = BBA_NORMAL;
                }
            }
            else if (token.type == CPP_PP_CONCAT){
                action = BBA_NONE;
            }
            else{
                action = BBA_NORMAL;
            }
            
            // NOTE(allen): loose token pushes here are managed by checkpoint
            switch (action){
            case BBA_NONE:break;
                
            case BBA_NORMAL:
            {
                param_i = cpp__get_parameter_i(state, definitions, macro_file, token,
                                               expansion->stack_base, macro.param_count);
                if (param_i != -1){
                    Cpp_Token_Range range = cpp__param_info_get(state, expansion->param_info_base, param_i, EXPANDED);
                    for (int j = range.start; j < range.end; ++j){
                        Cpp_Loose_Token loose = state->tokens.tokens[j];
                        cpp__push_loose_token(&checkpoint, state, loose.file_index, loose.token_index, loose.blocked);
                    }
                }
                else{
                    cpp__push_loose_token(&checkpoint, state, macro.file_index, i, 0);
                }
            }break;
            
            case BBA_STRFY:
            {
                token = macro_file->tokens.tokens[i];
                param_i = cpp__get_parameter_i(state, definitions, macro_file, token,
                                               expansion->stack_base, macro.param_count);
                if (param_i != -1){
                    Cpp_Token_Range range = cpp__param_info_get(state, expansion->param_info_base, param_i, STRFY);
                    Cpp_Loose_Token loose = state->tokens.tokens[range.start];
                    cpp__push_loose_token(&checkpoint, state, loose.file_index, loose.token_index, loose.blocked);
                }
                else{
                    cpp__push_loose_token(&checkpoint, state, macro.file_index, i, 0);
                }
            }break;
            
            case BBA_PASTE:
            {
                Cpp_Token token2;
                int param_i2;
                
                param_i = cpp__get_parameter_i(state, definitions, macro_file, token,
                                               expansion->stack_base, macro.param_count);
                token2 = macro_file->tokens.tokens[i2];
                param_i2 = cpp__get_parameter_i(state, definitions, macro_file, token2,
                                                expansion->stack_base, macro.param_count);
                
                str_checkpoint = cpp__checkpoint_spare_string(state);
                if (param_i != -1){
                    Cpp_Token_Range range = cpp__param_info_get(state, expansion->param_info_base, param_i, RAW);
                    range.end -= 1;
                    if (range.start <= range.end){
                        int j;
                        for (j = range.start; j < range.end; ++j){
                            Cpp_Loose_Token loose = state->tokens.tokens[j];
                            cpp__push_loose_token(&checkpoint, state, loose.file_index, loose.token_index, loose.blocked);
                        }
                        Cpp_Loose_Token loose = state->tokens.tokens[j];
                        Cpp_Parse_File *end_file = cpp_get_parse_file(definitions, loose.file_index);
                        Cpp_Token end_token = end_file->tokens.tokens[loose.token_index];
                        String end_string = make_string(end_file->file.data + end_token.start, end_token.size);
                        cpp__spare_write(&str_checkpoint, state, end_string);
                    }
                }
                else{
                    String end_string = make_string(macro_file->file.data + token.start, token.size);
                    cpp__spare_write(&str_checkpoint, state, end_string);
                }
                
                if (param_i2 != -1){
                    Cpp_Token_Range range = cpp__param_info_get(state, expansion->param_info_base, param_i2, RAW);
                    if (range.start < range.end){
                        int j = range.start;
                        Cpp_Loose_Token loose = state->tokens.tokens[j];
                        Cpp_Parse_File *start_file = cpp_get_parse_file(definitions, loose.file_index);
                        Cpp_Token start_token = start_file->tokens.tokens[loose.token_index];
                        String start_string = make_string(start_file->file.data + start_token.start, start_token.size);
                        cpp__spare_write(&str_checkpoint, state, start_string);
                    }
                }
                else{
                    String start_string = make_string(macro_file->file.data + token2.start, token2.size);
                    cpp__spare_write(&str_checkpoint, state, start_string);
                }
                
                if (!str_checkpoint.memory_overrun){
                    Cpp_Token _tokens[3];
                    Cpp_Token_Stack tokens;
                    tokens.tokens = _tokens;
                    tokens.max_count = ArrayCount(_tokens);
                    tokens.count = 0;
                    
                    Cpp_File paste_file;
                    paste_file.data = state->spare_string;
                    paste_file.size = state->spare_string_write_pos;
                    
                    cpp_lex_file_nonalloc(paste_file, &tokens);
                    _Assert(tokens.count <= 2);
                    
                    // TODO(allen): protect these preserves
                    String paste_string = make_string(state->spare_string, state->spare_string_write_pos);
                    cpp__preserve_string(&pres_checkpoint, definitions, paste_string);
                    int start_pos = definitions->string_write_pos - paste_string.size;
                    for (int k = 0; k < tokens.count; ++k){
                        tokens.tokens[k].start += start_pos;
                    }
                    for (int k = 0; k < tokens.count; ++k){
                        Cpp_Loose_Token loose = cpp__preserve_token(&pres_checkpoint, definitions, tokens.tokens[k]);
                        cpp__push_loose_token(&checkpoint, state, loose.file_index, loose.token_index, loose.blocked);
                    }
                    
                    i = i2;
                }
                else{
                    i = macro.body_end_index;
                }
                
                state->spare_string_write_pos = 0;
                
                if (param_i2 != -1){
                    Cpp_Token_Range range = cpp__param_info_get(state, expansion->param_info_base, param_i2, RAW);
                    range.start += 1;
                    for (int j = range.start; j < range.end; ++j){
                        Cpp_Loose_Token loose = state->tokens.tokens[j];
                        cpp__push_loose_token(&checkpoint, state, loose.file_index, loose.token_index, loose.blocked);
                    }
                }
            }break;
            
            default:
            {
                _Assert(!"unknown action");
            }break;
            }
        }
        
        if (pres_checkpoint.out_of_memory){
            cpp__restore_token_stack(state, checkpoint);
            cpp__restore_preserve_write(definitions, pres_checkpoint);
            result.memory_request = context->preserve_chunk_size;
            result.memory_purpose = MEMPURP_PRESERVE_FILE;
        }
        else if (str_checkpoint.memory_overrun){
            cpp__restore_token_stack(state, checkpoint);
            cpp__restore_preserve_write(definitions, pres_checkpoint);
            result.memory_request = (state->spare_string_size * 2) + str_checkpoint.memory_overrun;
            result.memory_purpose = MEMPURP_SPARE_STRING;
        }
        else if (checkpoint.memory_overrun){
            cpp__restore_token_stack(state, checkpoint);
            cpp__restore_preserve_write(definitions, pres_checkpoint);
            result.memory_request = sizeof(Cpp_Loose_Token)*((state->tokens.max*2) + checkpoint.memory_overrun);
            result.memory_purpose = MEMPURP_TOKEN_STACK;
        }
        else{
            expansion->position = expansion->stack_base;
            expansion->out_type = EXPAN_BIG_DIRECT_OUT;
            
            if (start_pos < state->tokens.count){
                Cpp_Expansion *new_expansion = 
                    cpp__push_expansion(state, EXPAN_PMACRO_BODY, -1, start_pos, state->tokens.count,
                                        -1, expansion->invoking_file_index,
                                        expansion->invoking_token_index);
                new_expansion->macro_index = expansion->file_index;
                if (new_expansion == 0){
                    result.error_code = PPERR_EXPANSION_OVERRUN;
                }
            }
        }
    }break;
    
    case EXPAN_BIG_DIRECT_OUT:
    {
        if (expansion->out_rule == -1){
            --state->expansion_level;
            _Assert(state->expansion_level != -1);
            cpp__free_param_table(state, macro.param_count);
        }
        else if (expansion->out_rule != 0){
            if (state->param_info[expansion->out_rule].start == 0){
                state->param_info[expansion->out_rule].start = expansion->position;
            }
            state->param_info[expansion->out_rule].end = state->tokens.count;
            --state->expansion_level;
            _Assert(state->expansion_level != -1);
            cpp__free_param_table(state, macro.param_count);
        }
        else{
            Cpp_Visit visit;
            Cpp_Loose_Token loose_token = state->tokens.tokens[expansion->position];
            visit.file_index = loose_token.file_index;
            visit.token_index = loose_token.token_index;
            
            result.file_index = visit.file_index;
            result.token_index = visit.token_index;
            result.invoking_file_index = expansion->invoking_file_index;
            result.invoking_token_index = expansion->invoking_token_index;
            result.from_macro = 1;
            result.emit = 1;
            
            ++expansion->position;
            if (expansion->position == state->tokens.count){
                state->mac_inv.stack_base -= (state->tokens.count - expansion->stack_base);
                state->tokens.count = expansion->stack_base;
                --state->expansion_level;
                _Assert(state->expansion_level != -1);
                cpp__free_param_table(state, macro.param_count);
            }
        }
    }break;
    
    default:
        _Assert(!"only meant to be used with one of the big expan types");
    }
    
    return result;
}

internal Cpp_Preproc_Result
cpp__preproc_strfy_step_nonalloc(Cpp_Preproc_State *state, Cpp_Parse_Definitions *definitions,
                                 Cpp_Parse_Context *context){
    Cpp_Preproc_Result result = {};
    Cpp_Expansion *expansion = state->expansions + state->expansion_level;
    
    if (!cpp__can_push_loose_token(state, 1)){
        result.memory_request = sizeof(Cpp_Loose_Token)*((state->tokens.max*2) + 1);
        result.memory_purpose = MEMPURP_TOKEN_STACK;
        return result;
    }
    
    Cpp_Visit visit = {};

    bool do_body = 1;
    if (expansion->position == expansion->end_position + 1){
        do_body = 0;
        --expansion->position;
    }
    else if (expansion->position == expansion->end_position){
        visit.file_index = definitions->eof_token.file_index;
        visit.token_index = definitions->eof_token.token_index;
    }
    else{
        _Assert(expansion->file_index == -1);
        Cpp_Loose_Token loose = state->tokens.tokens[expansion->position];
        visit.file_index = loose.file_index;
        visit.token_index = loose.token_index;
    }
    
    Spare_String_Checkpoint checkpoint = cpp__checkpoint_spare_string(state);
    
    if (do_body){
        Cpp_Parse_File visit_file;
        Cpp_Token visit_token;
        
        visit_file = *cpp_get_parse_file(definitions, visit.file_index);
        visit_token = visit_file.tokens.tokens[visit.token_index];
        
        result.file_index = visit.file_index;
        result.token_index = visit.token_index;
        result.invoking_file_index = expansion->invoking_file_index;
        result.invoking_token_index = expansion->invoking_token_index;
        result.from_macro = 1;
        result.emit = 0;
        
        if (state->spare_string_write_pos == 0){
            cpp__spare_write(&checkpoint, state, '"');
            expansion->param_info_base = visit_token.start + visit_token.size;
        }
        
        switch (visit_token.type){
        case CPP_TOKEN_COMMENT:
        case CPP_TOKEN_JUNK:break;
            
        case CPP_TOKEN_EOF:
        {
            cpp__spare_write(&checkpoint, state, '"');
        }break;
        
        case CPP_TOKEN_STRING_CONSTANT:
        {
            if (visit_token.start > expansion->param_info_base){
                cpp__spare_write(&checkpoint, state, ' ');
            }
            
            int i = visit_token.start;
            char c = visit_file.file.data[i];
            while (c != '"'){
                cpp__spare_write(&checkpoint, state, c);
                ++i;
                c = visit_file.file.data[i];
            }
            
            cpp__spare_write(&checkpoint, state, '\\');
            cpp__spare_write(&checkpoint, state, '"');
            ++i;
            
            int end_pos = visit_token.start + visit_token.size - 1;
            for (; i < end_pos; ++i){
                c = visit_file.file.data[i];
                switch (c){
                case '\\':
                    cpp__spare_write(&checkpoint, state, '\\');
                    
                default:
                    cpp__spare_write(&checkpoint, state, c);
                }
            }
            
            cpp__spare_write(&checkpoint, state, '\\');
            cpp__spare_write(&checkpoint, state, '"');
            
            expansion->param_info_base = visit_token.start + visit_token.size;
        }break;
        
        case CPP_TOKEN_CHARACTER_CONSTANT:
        {
            if (visit_token.start > expansion->param_info_base){
                cpp__spare_write(&checkpoint, state, ' ');
            }
            
            int i = visit_token.start;
            char c = visit_file.file.data[i];
            while (c != '\''){
                cpp__spare_write(&checkpoint, state, c);
                ++i;
                c = visit_file.file.data[i];
            }
            
            int end_pos = visit_token.start + visit_token.size;
            for (; i < end_pos; ++i){
                c = visit_file.file.data[i];
                switch (c){
                case '\\':
                    cpp__spare_write(&checkpoint, state, '\\');
                    
                default:
                    cpp__spare_write(&checkpoint, state, c);
                }
            }
            
            expansion->param_info_base = visit_token.start + visit_token.size;
        }break;
        
        default:
        {
            if (visit_token.start > expansion->param_info_base){
                cpp__spare_write(&checkpoint, state, ' ');
            }
            
            int end_pos = visit_token.start + visit_token.size;
            for (int i = visit_token.start; i < end_pos; ++i){
                char c = visit_file.file.data[i];
                cpp__spare_write(&checkpoint, state, c);
            }
            
            expansion->param_info_base = visit_token.start + visit_token.size;
        }break;
        }
    }
    
    if (checkpoint.memory_overrun){
        cpp__restore_spare_string(state, checkpoint);
        result.memory_request = (state->spare_string_size*2) + checkpoint.memory_overrun;
        result.memory_purpose = MEMPURP_SPARE_STRING;
    }
    else{
        if (expansion->position == expansion->end_position){
            Preserve_Checkpoint preserve_checkpoint = cpp__checkpoint_preserve_write(definitions);
            
            String string = make_string(state->spare_string, state->spare_string_write_pos);
            Cpp_Token token = {};
            token.type = CPP_TOKEN_STRING_CONSTANT;
            token.start = definitions->string_write_pos;
            token.size = string.size;
            
            cpp__preserve_string(&preserve_checkpoint, definitions, string);
            Cpp_Loose_Token loose = cpp__preserve_token(&preserve_checkpoint, definitions, token);
            
            if (preserve_checkpoint.out_of_memory){
                cpp__restore_preserve_write(definitions, preserve_checkpoint);
                result.memory_request = context->preserve_chunk_size;
                result.memory_purpose = MEMPURP_PRESERVE_FILE;
            }
            else{
                _Assert(expansion->out_rule > 0);
                // NOTE(allen): this function gaurantees there is at least room
                // for one token at the head end
                cpp__push_loose_token(state, loose.file_index, loose.token_index, loose.blocked);
                state->param_info[expansion->out_rule].start = state->tokens.count - 1;
                state->param_info[expansion->out_rule].end = state->tokens.count;
                --state->expansion_level;
                state->spare_string_write_pos = 0;
            }
        }
        ++expansion->position;
    }
    
    return result;
}

internal Cpp_Preproc_Result
cpp_preproc_step_nonalloc(Cpp_Preproc_State *state, Cpp_Parse_Definitions *definitions,
                          Cpp_Parse_Context *context){
    Cpp_Preproc_Result result;
    
    if (state->resizing_slots){
        result = {};
        result.memory_request = sizeof(Table_Entry)*(definitions->table.max_size*2);
        result.memory_purpose = MEMPURP_DEFINITION_SLOTS;
        state->resizing_slots = 0;
        return result;
    }
    if (definitions->count * 8 >= definitions->max * 7){
        result = {};
        result.memory_request = sizeof(Cpp_Def_Slot)*(definitions->max*2);
        result.memory_purpose = MEMPURP_DEFINITION_SLOTS;
        state->resizing_slots = 1;
        return result;
    }
    
    Cpp_Expansion *expansion = state->expansions + state->expansion_level;
    switch (expansion->out_type){
    case EXPAN_NORMAL:
    case EXPAN_EOF_FLUSH:
        result = cpp__preproc_normal_step_nonalloc(state, definitions, context);
        break;
        
    case EXPAN_BIG_PROCESS_ARGS:
    case EXPAN_BIG_PROCESS_BODY:
    case EXPAN_BIG_DIRECT_OUT:
        result = cpp__preproc_big_step_nonalloc(state, definitions, context);
        break;
        
    case EXPAN_STRFY:
        result = cpp__preproc_strfy_step_nonalloc(state, definitions, context);
        break;
        
    default:
        result = {};
        _Assert(!"unknown expansion->out_type");
    }
    
    return result;
}

// BOTTOM

