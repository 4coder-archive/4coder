/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 10.12.2019
 *
 * Documentation of the custom layer's primary api.
 *
 */

// TOP

#include "4coder_base_types.h"
#include "4coder_token.h"
#include "generated/lexer_cpp.h"
#include "../4ed_api_definition.h"
#include "4coder_doc_content_types.h"
#include "4ed_doc_helper.h"

#include "4coder_base_types.cpp"
#include "4coder_stringf.cpp"
#include "4coder_malloc_allocator.cpp"
#include "4coder_token.cpp"
#include "generated/lexer_cpp.cpp"
#include "../4ed_api_definition.cpp"
#include "../4ed_api_parser.cpp"
#include "4coder_doc_content_types.cpp"
#include "4ed_doc_helper.cpp"
#include "4coder_file.cpp"

////////////////////////////////

#include "4ed_doc_custom_api.cpp"

////////////////////////////////

#include <stdio.h>

int main(void){
    Arena arena = make_arena_malloc();
    
    String_Const_u8 me = string_u8_litexpr(__FILE__);
    String_Const_u8 docs_folder = string_remove_last_folder(me);
    String_Const_u8 root = string_remove_last_folder(docs_folder);
    String_Const_u8 file_name = push_u8_stringf(&arena, "%.*scustom/generated/custom_api_master_list.h",
                                                string_expand(root));
    
    FILE *file = fopen((char*)file_name.str, "rb");
    if (file == 0){
        printf("could not load %s\n", file_name.str);
        return(1);
    }
    
    printf("documenting %s\n", file_name.str);
    String_Const_u8 text = data_from_file(&arena, file);
    fclose(file);
    
    API_Definition_List def_list = {};
    api_parse_source_add_to_list(&arena, file_name, text, &def_list);
    
    API_Definition *api_def = api_get_api(&def_list, string_u8_litexpr("custom"));
    Doc_Cluster *cluster = doc_custom_api(&arena, api_def);
    
    doc_api_check_full_coverage(&arena, cluster, api_def);
    
    for (Doc_Log *node = cluster->first_log;
         node != 0;
         node = node->next){
        printf("%.*s\n", string_expand(node->content));
    }
    
    return(0);
}

// BOTTOM

