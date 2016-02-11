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

#define FCPP_STRING_IMPLEMENTATION
#include "../4coder_string.h"

#include "../4cpp_types.h"
#define FCPP_LEXER_IMPLEMENTATION
#include "../4cpp_lexer.h"
#include "../4cpp_preprocessor.cpp"

#include <stdio.h>
#include <stdlib.h>

Data
file_dump(char *filename){
    Data result;
    FILE *file;
    result = {};
    file = fopen(filename, "rb");
    if (file){
        fseek(file, 0, SEEK_END);
        result.size = ftell(file);
        fseek(file, 0, SEEK_SET);
        result.data = (byte*)malloc(result.size);
        fread(result.data, 1, result.size, file);
        fclose(file);
    }
    return(result);
}

int main(int argc, char **argv){
    Data target_file;
    Cpp_File file;
    Cpp_Token_Stack tokens;
    Cpp_Token *token;
    int i;
    
    if (argc != 2){
        printf("usage: %s <cpp-file>\n", argv[0]);
        exit(1);
    }
    
    target_file = file_dump(argv[1]);
    if (target_file.data == 0){
        printf("couldn't open file %s\n", argv[1]);
        exit(1);
    }
    
    tokens = cpp_make_token_stack(1 << 10);
    
    file = data_as_cpp_file(target_file);
    cpp_lex_file(file, &tokens);
    
    token = tokens.tokens;
    for (i = 0; i < tokens.count; ++i, ++token){
        printf("%.*s\n", token->size, file.data + token->start);
    }
    
    return(0);
}

// BOTTOM

