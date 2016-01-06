/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 21.1.2015
 *
 * Test for CPP lexer & parser layer for project codename "4ed"
 *
 */

// TOP

#include "../4ed_meta.h"

#define Debug(x) x

#include "../4cpp_types.h"
#define FCPP_STRING_IMPLEMENTATION
#include "../4cpp_string.h"
#define FCPP_LEXER_IMPLEMENTATION
#include "../4cpp_lexer.h"
#include "../4cpp_preprocessor.cpp"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

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

internal void
preproc_init(Cpp_PP_State *state, Cpp_PP_Definitions *definitions){
    *state = {};
    state->max = (32 << 10);
    state->base = (byte*)malloc(state->max);
    memset(state->base, 0, state->max);

    *definitions = {};
    definitions->max = 128 << 10;
    definitions->items = (byte*)malloc(definitions->max);
    memset(definitions->items, 0, definitions->max);
    
    definitions->table.count = 0;
    definitions->table.max = 4 << 10;
    definitions->table.entries = (Cpp_PP_Table_Entry*)
        malloc(definitions->table.max*sizeof(Cpp_PP_Table_Entry));
    memset(definitions->table.entries, 0, definitions->table.max*sizeof(Cpp_PP_Table_Entry));

    int str_size = 16 << 10;
    int token_size = 16 << 10;
    void *str_mem = malloc(str_size);
    void *token_mem = malloc(token_size);
    cpp_preproc_set_spare_space(definitions,
                                str_size, str_mem,
                                token_size, token_mem);
}

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
        return 1;
    }
    
    Cpp_Token_Stack target_tokens = {};
    cpp_lex_file(target_file, &target_tokens);

    Cpp_PP_State state = {};
    Cpp_PP_Definitions definitions = {};

    preproc_init(&state, &definitions);
    
    cpp_preproc_target(&state, &definitions, target_file, target_tokens);
    
    for (Cpp_PP_Step step = {};
         step.finished == 0;){
        step = cpp_preproc_step_nonalloc(&state, &definitions);
        
        if (step.error_code){
            printf("error: %s\n", cpp_preproc_error_str(step.error_code));
            if (cpp_preproc_recommend_termination(step.error_code)){
                return 1;
            }
        }

        if (step.mem_size != 0){
            void *memory = malloc(step.mem_size);
            memory = cpp_preproc_provide_memory(&state, &definitions,
                                                step.mem_type, step.mem_size, memory);
            free(memory);
        }
        
        if (step.emit){
            Cpp_File file = *cpp_preproc_get_file(&definitions, step.file_index);
            Cpp_Token token = *cpp_preproc_get_token(&definitions, step.file_index, step.token_index);
            
            printf("TOKEN: %.*s\n", token.size, file.data + token.start);
        }
    }
    
    return 0;
}

// BOTTOM

