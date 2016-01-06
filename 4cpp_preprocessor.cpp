/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 30.11.2015
 *
 * CPP preprocessor
 *
 */

// TOP

#define byte unsigned char

struct Cpp_PP_Step{
    int error_code;
    int finished;
    int early_out;
    
    int mem_size;
    int mem_type;
    
    int emit;
    int file_index;
    int token_index;
};

enum Cpp_PP_Expansion_Type{
    PPExp_Normal,
    // never below this
    PPExp_Count
};
struct Cpp_PP_Expansion_Header{
    int prev_frame;
    int type;
};
struct Cpp_PP_Expansion_Normal{
    int file_offset;
    int current_token;
    int end_token;
    int file_level;
};

struct Cpp_PP_State{
    byte *base;
    int top, frame, max;
    
    int substep;

    struct{
        union{
            int name_index;
            int param_count;
            int macro_offset;
        } define;
    } vars;
    
    Debug(int did_advance);
};

internal int
cpp__push_expansion_normal(Cpp_PP_State *state, int file, int start, int end, int file_level){
    Cpp_PP_Expansion_Header *header;
    Cpp_PP_Expansion_Normal *expansion;
    int added_size;
    int prev_frame;
    int result;

    added_size = sizeof(Cpp_PP_Expansion_Header) + sizeof(Cpp_PP_Expansion_Normal);

    if (state->top + added_size <= state->max){
        result = 1;

        prev_frame = state->frame;
        state->frame = state->top;
        state->top += added_size;

        header = (Cpp_PP_Expansion_Header*)(state->base + state->frame);
        expansion = (Cpp_PP_Expansion_Normal*)(header + 1);

        header->prev_frame = prev_frame;
        header->type = PPExp_Normal;
        expansion->file_offset = file;
        expansion->current_token = start;
        expansion->end_token = end;
        expansion->file_level = file_level;
    }
    else{
        result = 0;
    }

    return(result);
}

internal int
cpp__pop_expansion(Cpp_PP_State *state){
    Cpp_PP_Expansion_Header *header;
    int result;

    if (state->top >= sizeof(Cpp_PP_Expansion_Header)){
        Assert(state->top - state->frame >= sizeof(Cpp_PP_Expansion_Header));
        result = 1;
        
        header = (Cpp_PP_Expansion_Header*)(state->base + state->frame);
        state->top = state->frame;
        state->frame = header->prev_frame;
    }
    else{
        result = 0;
    }

    return(result);
}

enum Cpp_PP_Memory_Type{
    PPMem_None,
    PPMem_Definition_Items,
    PPMem_Definition_Table,
    PPMem_Spare_File_String,
    PPMem_Spare_File_Tokens,
    // never below this
    PPMem_Count
};

internal void
cpp__pp_memory_request(Cpp_PP_Step *step, int amount, int type){
    Assert(!step->early_out);
    step->early_out = 1;
    step->mem_size = amount;
    step->mem_type = type;
}

enum Cpp_PP_Table_Entry_Type{
    PPItem_Unset,
    PPItem_File,
    PPItem_Macro,
    // never below this
    PPItem_Count
};
struct Cpp_PP_Table_Entry{
    int name_start, name_size;
    int type;
    unsigned int hash;
    int item_offset;
};
struct Cpp_PP_Table{
    Cpp_PP_Table_Entry *entries;
    unsigned int count, max;
};

struct Cpp_PP_File{
    Cpp_File file;
    Cpp_Token_Stack tokens;
};
struct Cpp_PP_Macro{
    int self_start;
    int name_start;
    int name_size;
    int param_count;
    int variadic;
    int body_start;
    int body_end;
};

struct Cpp_PP_Definitions{
    byte *items;
    int pos, restore_pos, max;
    Cpp_PP_File *spare_file;
    int spare_pos;

    Cpp_PP_Table table;
};

internal unsigned int
cpp__hash(char *name, int size, int type){
    unsigned int x;
    int i;
    
    x = 5381;
    x = ((x << 5) + x) ^ ('A' + type);
    for (i = 0; i < size; ++i, ++name){
        x = ((x << 5) + x) ^ *name;
    }
    
    return(x);
}

internal unsigned int
cpp__hash(Cpp_PP_Definitions *definitions, int name_start, int name_size, int type){
    unsigned int x;
    char *s;
    s = definitions->spare_file->file.data + name_start;
    x = cpp__hash(s, name_size, type);
    return(x);
}

internal int
cpp__table_can_add(Cpp_PP_Table *table){
    int result;
    result = 0;
    if (table->count*8 < table->max*7) result = 1;
    return(result);
}

internal void
cpp__table_add_direct(Cpp_PP_Table *table, Cpp_PP_Table_Entry entry){
    unsigned int i, j;
    i = entry.hash % table->max;
    j = i - 1;
    if (i <= 1) j += table->max;
    
    for (; i != j; ++i){
        if (i == table->max) i = 0;
        if (table->entries[i].type == PPItem_Unset){
            table->entries[i] = entry;
            break;
        }
    }

    Assert(i != j);
}

internal void
cpp__table_add(Cpp_PP_Definitions *definitions, Cpp_PP_Table *table,
               int name_start, int name_size, int type, int offset){
    Cpp_PP_Table_Entry entry;
    unsigned int hash;

    Assert(cpp__table_can_add(table));

    hash = cpp__hash(definitions, name_start, name_size, type);
    entry.name_start = name_start;
    entry.name_size = name_size;
    entry.type = type;
    entry.hash = hash;
    entry.item_offset = offset;

    cpp__table_add_direct(table, entry);
}

internal int
cpp__str_match(char *a, char *b, int size){
    char *e;
    int result;
    result = 0;
    for(e = a + size; a < e && *a == *b; ++a, ++b);
    if (a == e) result = 1;
    return(result);
}

internal int
cpp__table_find_hash_pos(Cpp_PP_Definitions *definitions, Cpp_PP_Table *table,
                         char *name, int name_size, int typ e, unsigned int *hash_index){
    Cpp_PP_Table_Entry *entry;
    int result;
    unsigned int hash;
    unsigned int i, j;

    result = 0;
    hash = cpp__hash(name, name_size, type);
    i = hash % table->max;
    j = i - 1;
    if (i <= 1) j += table->max;
    
    for (; i != j; ++i){
        if (i == table->max) i = 0;
        entry = table->entries + i;
        if (entry->type == PPItem_Unset) break;
        if (entry->hash == hash && entry->type == type && entry->name_size == name_size &&
            cpp__str_match(name, definitions->spare_file->file.data + entry->name_start, name_size)){
            result = 1;
            *hash_index = i;
            break;
        }
    }
    
    return(result);
}

internal int
cpp__table_find(Cpp_PP_Definitions *definitions, Cpp_PP_Table *table,
                char *name, int name_size, int type, int *offset){
    int result;
    unsigned int hash_index;

    result = 0;
    if (cpp__table_find_hash_pos(definitions, table, name, name_size, type, &hash_index)){
        result = 1;
        *offset = table->entries[hash_index].item_offset;
    }

    return(result);
}

internal void
cpp__table_cpy(Cpp_PP_Table *dest, Cpp_PP_Table *src){
    Cpp_PP_Table_Entry *entry;
    unsigned int i;
    
    Assert(dest->max >= src->max);
    for (i = 0, entry = src->entries; i < src->max; ++i, ++entry){
        if (entry->type != PPItem_Unset){
            cpp__table_add_direct(dest, *entry);
        }   
    }
}

internal void
cpp__def_begin_uncertain(Cpp_PP_Definitions *definitions){
    definitions->restore_pos = definitions->pos;
}
internal void
cpp__def_success_uncertain(Cpp_PP_Definitions *definitions){
    definitions->restore_pos = 0;
}
internal void
cpp__def_fail_uncertain(Cpp_PP_Definitions *definitions){
    if (definitions->restore_pos != 0){
        definitions->pos = definitions->restore_pos;
    }
}

internal int
cpp__def_can_save_strings(Cpp_PP_Definitions *definitions, int total_size){
    int result;
    Cpp_PP_File *file;
    file = definitions->spare_file;
    
    if (definitions->spare_pos + total_size <= file->file.size) result = 1;
    else result = 0;
    
    return(result);
}

internal int
cpp__def_can_save_item(Cpp_PP_Definitions *definitions, int size){
    int result;
    
    if (definitions->pos + size <= definitions->max) result = 1;
    else result = 0;
    
    return(result);
}

internal int
cpp__def_save_string(Cpp_PP_Step *step, Cpp_PP_Definitions *definitions,
                     char *str, int size){
    int result;
    Cpp_PP_File *file;
    file = definitions->spare_file;

    Assert(definitions->spare_pos + size <= file->file.size);
    memcpy(file->file.data + definitions->spare_pos, str, size);
    result = definitions->spare_pos;
    definitions->spare_pos += size;

    return(result);
}

internal int
cpp__mem_up(int x){
    int xx;
    xx = x / 1024;
    if (xx*1024 < x){
        xx += 1;
    }
    xx = xx << 10;
    return(xx);
}

internal int
cpp__def_begin_macro(Cpp_PP_Step *step, Cpp_PP_Definitions *definitions,
                     char *str, int size){
    int result, str_pos;
    Cpp_PP_Macro *macro_item;

    result = 0;
    if (cpp__def_can_save_strings(definitions, size)){
        if (cpp__def_can_save_item(definitions, sizeof(Cpp_PP_Macro))){
            str_pos = cpp__def_save_string(step, definitions, str, size);

            Assert(definitions->pos + sizeof(Cpp_PP_Macro) <= definitions->max);
            result = definitions->pos;
            definitions->pos += sizeof(Cpp_PP_Macro);

            macro_item = (Cpp_PP_Macro*)(definitions->items + result);
            *macro_item = {};
            macro_item->self_start = result;
            macro_item->name_start = str_pos;
            macro_item->name_size = size;
        }
        else{
            cpp__pp_memory_request(step, cpp__mem_up(definitions->max + 1), PPMem_Definition_Items);
        }
    }
    else{
        cpp__pp_memory_request(step, cpp__mem_up(definitions->spare_file->file.size + size),
                               PPMem_Spare_File_String);
    }

    return(result);
}

internal void
cpp__def_set_macro_params(Cpp_PP_Definitions *definitions, int macro_offset, int param_count){
    Cpp_PP_Macro *macro_item;
    macro_item = (Cpp_PP_Macro*)(definitions->items + macro_offset);
    macro_item->param_count = param_count;
}

internal void
cpp__def_set_macro_variadic(Cpp_PP_Definitions *definitions, int macro_offset, int variadic){
    Cpp_PP_Macro *macro_item;
    macro_item = (Cpp_PP_Macro*)(definitions->items + macro_offset);
    macro_item->variadic = variadic;
}

internal void
cpp__def_set_macro_body_start(Cpp_PP_Definitions *definitions, int macro_offset, int body_start){
    Cpp_PP_Macro *macro_item;
    macro_item = (Cpp_PP_Macro*)(definitions->items + macro_offset);
    macro_item->body_start = body_start;
}

internal void
cpp__def_set_macro_body_end(Cpp_PP_Definitions *definitions, int macro_offset, int body_end){
    Cpp_PP_Macro *macro_item;
    macro_item = (Cpp_PP_Macro*)(definitions->items + macro_offset);
    macro_item->body_end = body_end;
}

internal void
cpp__def_end_macro(Cpp_PP_Step *step, Cpp_PP_Definitions *definitions){
    if (cpp__table_can_add(&definitions->table)){
        Cpp_PP_Macro *macro_item;
        macro_item = (Cpp_PP_Macro*)(definitions->items + definitions->restore_pos);
        
        cpp__table_add(definitions, &definitions->table,
                       macro_item->name_start, macro_item->name_size,
                       PPItem_Macro, macro_item->self_start);
    }
    else{
        cpp__pp_memory_request(step,
                               cpp__mem_up(definitions->table.max*sizeof(Cpp_PP_Table_Entry)+1),
                               PPMem_Definition_Table);
    }
}

internal Cpp_PP_File*
cpp_preproc_get_pp_file(Cpp_PP_Definitions *definitions, int file_offset){
    Cpp_PP_File *result;
    result = (Cpp_PP_File*)(definitions->items+file_offset);
    return(result);
}

internal Cpp_File*
cpp_preproc_get_file(Cpp_PP_Definitions *definitions, int file_offset){
    Cpp_PP_File *pp_file;
    Cpp_File *result;
    pp_file = cpp_preproc_get_pp_file(definitions, file_offset);
    result = &pp_file->file;
    return(result);
}

internal Cpp_Token*
cpp_preproc_get_token(Cpp_PP_Definitions *definitions, int file_offset, int token_index){
    Cpp_PP_File *pp_file;
    Cpp_Token *result;
    pp_file = cpp_preproc_get_pp_file(definitions, file_offset);
    result = &pp_file->tokens.tokens[token_index];
    return(result);
}

internal int
cpp__define_file(Cpp_PP_Definitions *definitions, Cpp_File file, Cpp_Token_Stack tokens){
    int result;
    Cpp_PP_File *file_item;

    Assert(definitions->pos + sizeof(Cpp_PP_File) <= definitions->max);
    result = definitions->pos;
    definitions->pos += sizeof(Cpp_PP_File);

    file_item = (Cpp_PP_File*)(definitions->items + result);
    file_item->file = file;
    file_item->tokens = tokens;
    
    return(result);
}

internal void
cpp_preproc_set_spare_space(Cpp_PP_Definitions *definitions,
                            int str_size, void *str_mem,
                            int token_size, void *token_mem){
    Cpp_PP_File pp_file;
    pp_file.file.size = str_size;
    pp_file.file.data = (char*)str_mem;
    pp_file.tokens.max_count = token_size;
    pp_file.tokens.count = 0;
    pp_file.tokens.tokens = (Cpp_Token*)token_mem;

    int pos = cpp__define_file(definitions, pp_file.file, pp_file.tokens);
    Cpp_PP_File *spare_file = cpp_preproc_get_pp_file(definitions, pos);
    definitions->spare_file = spare_file;
    definitions->spare_pos = 0;
}

internal void
cpp_preproc_target(Cpp_PP_State *state, Cpp_PP_Definitions *definitions,
                   Cpp_File file, Cpp_Token_Stack tokens){
    int file_index;
    
    Assert(state->top == 0);
    
    file_index = cpp__define_file(definitions, file, tokens);
    cpp__push_expansion_normal(state, file_index, 0, tokens.count, 1);
}

internal void*
cpp_preproc_provide_memory(Cpp_PP_State *state, Cpp_PP_Definitions *definitions,
                           int type, int size, void *memory){
    Cpp_PP_Table new_table;
    Cpp_PP_File *spare;
    void *result;
    result = 0;
    
    
    
    switch (type){
    case PPMem_None: Assert(0); break;
        
    case PPMem_Definition_Items:
        Assert(size > definitions->max);
        memcpy(memory, definitions->items, definitions->pos);
        result = definitions->items;
        definitions->items = (byte*)memory;
        break;
        
    case PPMem_Definition_Table:
        Assert(size > definitions->table.max * sizeof(*new_table.entries));
        new_table.entries = (Cpp_PP_Table_Entry*)memory;
        new_table.max = size/sizeof(*new_table.entries);
        new_table.count = 0;
        cpp__table_cpy(&new_table, &definitions->table);
        result = definitions->table.entries;
        definitions->table = new_table;
        break;
        
    case PPMem_Spare_File_String:
        spare = definitions->spare_file;
        Assert(size > spare->file.size);
        memcpy(memory, spare->file.data, definitions->spare_pos);
        result = spare->file.data;
        spare->file.data = (char*)memory;
        break;
        
    case PPMem_Spare_File_Tokens:
        spare = definitions->spare_file;
        Assert(size > spare->tokens.count*sizeof(Cpp_Token));
        memcpy(memory, spare->tokens.tokens, spare->tokens.count*sizeof(Cpp_Token));
        result = spare->tokens.tokens;
        spare->tokens.tokens = (Cpp_Token*)memory;
        break;
        
    default: Assert(0); break;
    }

    return(result);
}

enum Cpp_PP_Error{
    PPErr_None,
    PPErr_Define_Identifier_Missing,
    PPErr_Parameter_Identifier_Missing,
    PPErr_Preprocessor_Directive_In_Expansion,
    PPErr_Expected_Comma_Or_Parenthese,
    PPErr_Unfinished_Parameters,
    // never below this
    PPErr_Count
};

internal char*
cpp_preproc_error_str(int error_code){
    char *result = 0;
    
    switch (error_code){
    case PPErr_None:
        result = "no error"; break;

    case PPErr_Define_Identifier_Missing:
        result = "define identifier missing"; break;

    case PPErr_Parameter_Identifier_Missing:
        result = "parameter identifier missing"; break;
        
    case PPErr_Preprocessor_Directive_In_Expansion:
        result = "preprocessor directive in expansion"; break;
        
    case PPErr_Expected_Comma_Or_Parenthese:
        result = "expected comma or parenthese"; break;

    case PPErr_Unfinished_Parameters:
        result = "unfinished parameter list"; break;
        
    default:
        result = "unknown error code";
    }

    return(result);
}

internal int
cpp_preproc_recommend_termination(int error_code){
    int result = 0;
    
    return(result);
}

struct Cpp__PP_Visit{
    int file_offset;
    int token_index;
};

internal void
cpp__pp_next_token(Cpp_PP_State *state, Cpp_PP_Expansion_Normal *expansion){
    ++expansion->current_token;
    Debug(state->did_advance = 1);
}

enum Cpp_PP_State_Step{
    PPState_None,
    PPState_Define,
    PPState_Name,
    PPState_Define_Body,
    PPState_Define_Parameter,
    PPState_Define_Parameter_Comma,
    PPState_Error_Recovery,
    // never below this
    PPState_Count
};

internal int
cpp__pp_require_identifier(Cpp_PP_State *state, Cpp_Token token){
    int result;
    result = PPErr_None;
    if ((token.flags & CPP_TFLAG_PP_BODY) == 0){
        result = PPErr_Define_Identifier_Missing;
        state->substep = PPState_None;
        Debug(state->did_advance = 1);
    }
    else if (token.type != CPP_TOKEN_IDENTIFIER &&
             (token.flags & CPP_TFLAG_IS_KEYWORD) == 0){
        result = PPErr_Define_Identifier_Missing;
        state->substep = PPState_Error_Recovery;
        Debug(state->did_advance = 1);
    }
    return(result);
}

internal Cpp_PP_Step
cpp__pp_step_normal(Cpp_PP_State *state, Cpp_PP_Definitions *definitions,
                    Cpp_PP_Expansion_Normal *expansion){
    Cpp__PP_Visit visit;
    Cpp_PP_Step result;
    Cpp_Token token;
    Cpp_PP_File pp_file;
    int void_step;

    visit = {};
    result = {};
    void_step = 0;
    
    if (expansion->current_token < expansion->end_token){
        visit.file_offset = expansion->file_offset;
        visit.token_index = expansion->current_token;
    }
    else{
        void_step = 1;
        cpp__pop_expansion(state);
    }
    
    if (!void_step){
        pp_file = *cpp_preproc_get_pp_file(definitions, visit.file_offset);
        Assert(visit.token_index >= 0 && visit.token_index < pp_file.tokens.count);
        token = pp_file.tokens.tokens[visit.token_index];


        
        Debug(state->did_advance = 0);
        if (token.type == CPP_TOKEN_COMMENT || token.type == CPP_TOKEN_JUNK){
            cpp__pp_next_token(state, expansion);
        }
        else{
            switch (state->substep){
            case PPState_None:
            {
                if (expansion->file_level == 0 && (token.flags & CPP_TFLAG_PP_DIRECTIVE)){
                    result.error_code = PPErr_Preprocessor_Directive_In_Expansion;
                    cpp__pp_next_token(state, expansion);
                }
                else{
                    switch (token.type){
                    case CPP_PP_DEFINE:
                    {
                        state->substep = PPState_Define;
                        state->vars = {};
                        cpp__pp_next_token(state, expansion);
                    }break;
            
                    default:
                    {
                        result.emit = 1;
                        result.file_index = visit.file_offset;
                        result.token_index = visit.token_index;
                        cpp__pp_next_token(state, expansion);
                    }break;
            
                    }
                }
            }break;
        
            case PPState_Define:
            {
                result.error_code = cpp__pp_require_identifier(state, token);
                if (result.error_code == PPErr_None){

                    cpp__def_begin_uncertain(definitions);
                    state->vars.define.macro_offset =
                        cpp__def_begin_macro(&result, definitions,
                                             pp_file.file.data + token.start,
                                             token.size);
                    if (result.early_out) goto cpp__pp_step_early_out;

                    state->substep = PPState_Name;
                    state->vars.define.name_index = visit.token_index;
                    cpp__pp_next_token(state, expansion);
                }
            }break;
        
            case PPState_Name:
            {
                if ((token.flags & CPP_TFLAG_PP_BODY) == 0){
                    cpp__def_end_macro(&result, definitions);
                    if (result.early_out) goto cpp__pp_step_early_out;
                    cpp__def_success_uncertain(definitions);
                    state->substep = PPState_None;
                    Debug(state->did_advance = 1);
                }
                else if (token.type == CPP_TOKEN_PARENTHESE_OPEN){
                    state->substep = PPState_Define_Parameter;
                    cpp__pp_next_token(state, expansion);
                }
                else{
                    state->substep = PPState_Define_Body;
                }
            }break;

            case PPState_Define_Parameter: 
            {
                if (token.type == CPP_TOKEN_PARENTHESE_CLOSE &&
                    (token.flags & CPP_TFLAG_PP_BODY) &&
                    state->vars.define.param_count == 0){
                    state->substep = PPState_Define_Body;
                    cpp__pp_next_token(state, expansion);
                }
                else{
                    result.error_code = cpp__pp_require_identifier(state, token);
                    if (result.error_code == PPErr_None){
                        state->substep = PPState_Define_Parameter_Comma;
                        cpp__pp_next_token(state, expansion);
                        ++state->vars.define.param_count;
                    }
                    else{
                        result.error_code = PPErr_Parameter_Identifier_Missing;
                        // NOTE(allen): Here I do not do the work I normally do when
                        // setting an error, because that work was done earlier, this is
                        // just translating the error code.
                    }
                }
            }break;
            
            case PPState_Define_Parameter_Comma:
            {
                if ((token.flags & CPP_TFLAG_PP_BODY) == 0){
                    result.error_code = PPErr_Unfinished_Parameters;
                    state->substep = PPState_None;
                    Debug(state->did_advance = 1);
                }
                else{
                    if (token.type == CPP_TOKEN_COMMA){
                        state->substep = PPState_Define_Parameter;
                        cpp__pp_next_token(state, expansion);
                    }
                    else if (token.type == CPP_TOKEN_PARENTHESE_CLOSE){
                        state->substep = PPState_Define_Body;
                        cpp__pp_next_token(state, expansion);
                    }
                    else{
                        state->substep = PPState_Error_Recovery;
                        result.error_code = PPErr_Expected_Comma_Or_Parenthese;
                        Debug(state->did_advance = 1);
                    }
                }
            }break;
            
            case PPState_Define_Body:
            {
                if ((token.flags & CPP_TFLAG_PP_BODY) == 0){
                    // TODO(allen): define macro
                    state->substep = PPState_None;
                    Debug(state->did_advance = 1);
                }
                else{
                    cpp__pp_next_token(state, expansion);
                }
            }break;
            
            case PPState_Error_Recovery:
            {
                if ((token.flags & CPP_TFLAG_PP_BODY) == 0){
                    state->substep = PPState_None;
                    Debug(state->did_advance = 1);
                }
                else{
                    cpp__pp_next_token(state, expansion);
                }
            }break;
            
            }
        }
        
        Assert(state->did_advance == 1);
    }

cpp__pp_step_early_out:
    return(result);
}

internal Cpp_PP_Step
cpp_preproc_step_nonalloc(Cpp_PP_State *state, Cpp_PP_Definitions *definitions){
    Cpp_PP_Step result;
    Cpp_PP_Expansion_Header *header;

    Assert(state->top >= sizeof(Cpp_PP_Expansion_Header));

    result = {};    
    header = (Cpp_PP_Expansion_Header*)(state->base + state->frame);
    switch (header->type){
    case PPExp_Normal:
        result = cpp__pp_step_normal(state, definitions,
                                     (Cpp_PP_Expansion_Normal*)(header + 1));
        break;
    default: Assert(0);
    }
    
    if (state->top == 0) result.finished = 1;
    
    return(result);
}

#undef byte

// BOTTOM

