/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 1.21.2015
 *
 * Test for CPP lexer & parser layer for project codename "4ed"
 *
 */

// TOP

#include "../4ed_meta.h"

#include "../4cpp_types.h"
#define FCPP_STRING_IMPLEMENTATION
#include "../4cpp_string.h"
#define FCPP_LEXER_IMPLEMENTATION
#include "../4cpp_lexer.h"
#include "../4cpp_preprocessor.cpp"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define FCPP_PREPROCESSOR_DBG_LEVEL 1

internal bool
system_is_absoute_path(char *path){
    bool is_absolute = 0;
    char c = 1;
    while (c){
        c = *path++;
        if (c == ':'){
            is_absolute = 1;
            break;
        }
    }
    return is_absolute;
}

#undef Assert
#undef TentativeAssert

#define Assert assert
#define TentativeAssert assert

Cpp_File
quickie_file(char *filename){
    Cpp_File result = {};
    
    FILE *file = fopen(filename, "rb");
    if (file){
        fseek(file, 0, SEEK_END);
        result.size = ftell(file);
        if (result.size > 0){
            fseek(file, 0, SEEK_SET);
            result.data = (char*)malloc(result.size);
            fread(result.data, 1, result.size, file);
        }
        fclose(file);
    }
    
    return result;
}

inline Cpp_File
quickie_file(String filename){
    assert(filename.size < 511);
    char buffer[512];
    memcpy(buffer, filename.str, filename.size);
    buffer[filename.size] = 0;
    return quickie_file(buffer);
}

#define STRICT_MEM_TEST 1

int main(int argc, char **argv){
    if (argc < 2){
        printf("usage: %s <file>\n", argv[0]);
        return 1;
    }
    char *TEST_FILE = argv[1];
    
    Cpp_File target_file;
    target_file = quickie_file(TEST_FILE);

    if (target_file.data == 0){
        printf("could not open file %s\n", TEST_FILE);
        exit(1);
    }
    
    Cpp_Token_Stack target_tokens = {};
    cpp_lex_file(target_file, &target_tokens);
    
    Cpp_Parse_Context context = {};
    Cpp_Parse_Definitions definitions = {};
    Cpp_Preproc_State state = {};
    
    context.preserve_chunk_size = (200 << 10);
    
    definitions.table.size = 0;
#if STRICT_MEM_TEST
    definitions.table.max_size = 16;
#else
    definitions.table.max_size = 100;
#endif
    definitions.table.table = (Table_Entry*)malloc(sizeof(Table_Entry)*definitions.table.max_size);
    memset(definitions.table.table, 0, sizeof(Table_Entry)*definitions.table.max_size);
    
    definitions.count = 0;
#if STRICT_MEM_TEST
    definitions.max = 16;
#else
    definitions.max = 100;
#endif
    definitions.slots = (Cpp_Def_Slot*)malloc(sizeof(Cpp_Def_Slot)*definitions.max);
    
    {
        String string_filename = make_lit_string("~ string space");
        Cpp_File string_file;
#if STRICT_MEM_TEST
        string_file.size = 100;
#else
        string_file.size = (128 << 10);
#endif
        string_file.data = (char*)malloc(string_file.size);
        Cpp_Token_Stack string_tokens;
        string_tokens.count = 0;
#if STRICT_MEM_TEST
        string_tokens.max_count = 2;
#else
        string_tokens.max_count = (128 << 10)/sizeof(Cpp_Token);
#endif
        string_tokens.tokens = (Cpp_Token*)malloc(sizeof(Cpp_Token)*string_tokens.max_count);
        
        Cpp_File_Data string_parse_file;
        string_parse_file.file = string_file;
        string_parse_file.tokens = string_tokens;
        string_parse_file.filename = string_filename;
        
        int string_index = cpp_defs_add(&definitions, {}, CPP_DEFTYPE_FILE);
        cpp_set_parse_file(&definitions, string_index, string_parse_file);
        
        definitions.string_file_index = string_index;
        definitions.string_write_pos = 0;
        
        {
            Cpp_Token eof_token = {};
            eof_token.type = CPP_TOKEN_EOF;
            definitions.eof_token = cpp__preserve_token(&definitions, eof_token);
        }
        
        {
            String string_va_args = make_lit_string("__VA_ARGS__");
            Cpp_Token va_args_token;
            va_args_token.type = CPP_TOKEN_IDENTIFIER;
            va_args_token.start = definitions.string_write_pos;
            va_args_token.size = string_va_args.size;
            cpp__preserve_string(&definitions, string_va_args);
            definitions.va_args_token = cpp__preserve_token(&definitions, va_args_token);
        }
    }
    
    state.tokens.count = 0;
#if STRICT_MEM_TEST
    state.tokens.max = 5;
#else
    state.tokens.max = 100;
#endif
    state.tokens.tokens = (Cpp_Loose_Token*)malloc(sizeof(Cpp_Loose_Token)*state.tokens.max);
    
    state.spare_string_write_pos = 0;
#if STRICT_MEM_TEST
    state.spare_string_size = 1;
#else
    state.spare_string_size = (10 << 10);
#endif
    state.spare_string = (char*)malloc(state.spare_string_size);
    
    String target_filename = make_lit_string(TEST_FILE);
    cpp_set_target(&state, &definitions, target_file, target_tokens, target_filename);
    
    while (!state.finished){
        Cpp_Preproc_Result result;
        result = cpp_preproc_step_nonalloc(&state, &definitions, &context);
        
        if (result.memory_request){
            Cpp_Memory_Request request = cpp_get_memory_request(&state, &definitions, result);
            void *memory = malloc(request.size);
            void *old_memory = cpp_provide_memory(request, memory);
            free(old_memory);
        }
        
        if (result.file_request){
            Cpp_File_Request request = cpp_get_file_request(&state, result);
            for (; cpp_has_more_files(&request); cpp_get_next_file(&request)){
                if (!cpp_try_reuse_file(&request)){
                    Cpp_File new_file = quickie_file(request.filename);
                    if (new_file.data == 0){
                        printf("could not open file %s\n", request.filename);
                        exit(1);
                    }
                    Cpp_Token_Stack new_tokens = {};
                    cpp_lex_file(new_file, &new_tokens);
                    cpp_provide_file(&request, new_file, new_tokens);
                }
            }
        }
        
        if (result.error_code){
            String error_message = cpp_get_error(result.error_code);
            Cpp_File_Data file = *cpp_get_parse_file(&definitions, result.file_index);
            Cpp_Token token = file.tokens.tokens[result.token_index];
            bool terminate = cpp_recommend_termination(result.error_code);
            
            if (terminate){
                printf("FATAL ");
            }
            
            printf("ERROR IN %.*s AT %.*s\n%.*s\n",
                   file.filename.size, file.filename.str,
                   token.size, file.file.data + token.start,
                   error_message.size, error_message.str);
            
            if (terminate){
                break;
            }
        }
        
        if (result.emit){
            Cpp_File_Data file = *cpp_get_parse_file(&definitions, result.file_index);
            Cpp_Token token = file.tokens.tokens[result.token_index];
            
            if (result.from_macro){
                Cpp_File_Data file = *cpp_get_parse_file(&definitions, result.invoking_file_index);
                Cpp_Token token = file.tokens.tokens[result.invoking_token_index];
                
                printf("EXPANDING %.*s => ", token.size, file.file.data + token.start);
            }
            
            printf("TOKEN %.*s\n", token.size, file.file.data + token.start);
        }
    }
    
    assert(state.finished == 0 || state.expansion_level == 0);
    assert(state.finished == 0 || state.param_info_used == 0);
    assert(state.finished == 0 || state.state == 0);
    
    return 0;
}

// BOTTOM

