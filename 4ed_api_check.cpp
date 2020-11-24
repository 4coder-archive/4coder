/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 06.10.2019
 *
 * Type checker that lists errors between two api parses.
 *
 */

// TOP

#include "4coder_base_types.h"
#include "4coder_token.h"
#include "generated/lexer_cpp.h"
#include "4ed_api_definition.h"

#include "4coder_base_types.cpp"
#include "4coder_stringf.cpp"
#include "4coder_malloc_allocator.cpp"
#include "4coder_token.cpp"
#include "generated/lexer_cpp.cpp"
#include "4coder_file.cpp"
#include "4ed_api_definition.cpp"
#include "4ed_api_parser.cpp"

#include <stdio.h>

////////////////////////////////

function void
print_usage(void){
    printf("usage: <script> <source-1> {<source-1>} : <source-2> {<source-2>}\n"
           " source-1 : the authoritative/master api source file(s)\n"
           " source-2 : the 'remote' api source file(s) to check against the master\n");
    exit(1);
}

int
main(int argc, char **argv){
    Arena arena = make_arena_malloc();
    
    if (argc < 4){
        print_usage();
    }
    
    API_Definition_List master_list = {};
    API_Definition_List remote_list = {};
    
    {
        i32 i = 1;
        for (;i < argc; i += 1){
            char *file_name = argv[i];
            if (string_match(SCu8(file_name), string_u8_litexpr(":"))){
                i += 1;
                break;
            }
            FILE *file = fopen(file_name, "rb");
            if (file == 0){
                printf("error: could not open input file: '%s'\n", file_name);
                continue;
            }
            String_Const_u8 text = data_from_file(&arena, file);
            fclose(file);
            if (text.size > 0){
                api_parse_source_add_to_list(&arena, SCu8(file_name), text, &master_list);
            }
        }
        for (;i < argc; i += 1){
            char *file_name = argv[i];
            FILE *file = fopen(file_name, "rb");
            if (file == 0){
                printf("error: could not open input file: '%s'\n", file_name);
                continue;
            }
            String_Const_u8 text = data_from_file(&arena, file);
            fclose(file);
            if (text.size > 0){
                api_parse_source_add_to_list(&arena, SCu8(file_name), text, &remote_list);
            }
        }
    }
    
    if (master_list.count == 0){
        printf("error: no apis in master list\n");
        exit(1);
    }
    
    if (remote_list.count == 0){
        printf("error: no apis in remote list\n");
        exit(1);
    }
    
    List_String_Const_u8 errors = {};
    api_list_check(&arena, &master_list, &remote_list, APICheck_ReportAll, &errors);
    String_Const_u8 string = string_list_flatten(&arena, errors, StringFill_NullTerminate);
    printf("%.*s", string_expand(string));
    if (string.size > 0){
        exit(1);
    }
    
    return(0);
}

// BOTTOM
