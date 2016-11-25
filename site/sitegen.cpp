/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 25.02.2016
 *
 * File editing view for 4coder
 *
 */

// TOP

#include "4coder_version.h"
#include "internal_4coder_string.cpp"
#include "4cpp_lexer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "4coder_mem.h"
#include "meta_parser.cpp"
#include "out_context.cpp"
#include "abstract_document.cpp"

#define InvalidPath Assert(!"Invalid path of execution")

///////////////////////////////////////////////////////////////////////////

//
// Meta Parse Rules
//

#define BACK_COLOR   "#FAFAFA"
#define TEXT_COLOR   "#0D0D0D"
#define CODE_BACK    "#DFDFDF"
#define EXAMPLE_BACK "#EFEFDF"

#define POP_COLOR_1  "#309030"
#define POP_BACK_1   "#E0FFD0"
#define VISITED_LINK "#A0C050"

#define POP_COLOR_2  "#005000"

#define CODE_STYLE "font-family: \"Courier New\", Courier, monospace; text-align: left;"

#define CODE_BLOCK_STYLE(back)                             \
"margin-top: 3mm; margin-bottom: 3mm; font-size: .95em; "  \
"background: "back"; padding: 0.25em;"

#define DESCRIPT_SECTION_STYLE CODE_BLOCK_STYLE(CODE_BACK)
#define EXAMPLE_CODE_STYLE CODE_BLOCK_STYLE(EXAMPLE_BACK)

#define DOC_HEAD_OPEN  "<div style='margin-top: 3mm; margin-bottom: 3mm; color: "POP_COLOR_1";'><b><i>"
#define DOC_HEAD_CLOSE "</i></b></div>"

#define DOC_ITEM_HEAD_STYLE "font-weight: 600;"

#define DOC_ITEM_HEAD_INL_OPEN  "<span style='"DOC_ITEM_HEAD_STYLE"'>"
#define DOC_ITEM_HEAD_INL_CLOSE "</span>"

#define DOC_ITEM_HEAD_OPEN  "<div style='"DOC_ITEM_HEAD_STYLE"'>"
#define DOC_ITEM_HEAD_CLOSE "</div>"

#define DOC_ITEM_OPEN  "<div style='margin-left: 5mm; margin-right: 5mm;'>"
#define DOC_ITEM_CLOSE "</div>"

#define EXAMPLE_CODE_OPEN  "<div style='"CODE_STYLE EXAMPLE_CODE_STYLE"'>"
#define EXAMPLE_CODE_CLOSE "</div>"

static void
print_function_body_code(String *out, Parse_Context *context, int32_t start){
    String pstr = {0}, lexeme = {0};
    Cpp_Token *token = 0;
    
    int32_t do_print = 0;
    int32_t nest_level = 0;
    int32_t finish = 0;
    int32_t do_whitespace_print = 0;
    for (; (token = get_token(context)) != 0; get_next_token(context)){
        if (do_whitespace_print){
            pstr = str_start_end(context->data, start, token->start);
            append_ss(out, pstr);
        }
        else{
            do_whitespace_print = 1;
        }
        
        do_print = 1;
        if (token->type == CPP_TOKEN_COMMENT){
            lexeme = get_lexeme(*token, context->data);
            if (check_and_fix_docs(&lexeme)){
                do_print = 0;
            }
        }
        else if (token->type == CPP_TOKEN_BRACE_OPEN){
            ++nest_level;
        }
        else if (token->type == CPP_TOKEN_BRACE_CLOSE){
            --nest_level;
            if (nest_level == 0){
                finish = 1;
            }
        }
        
        if (do_print){
            pstr = get_lexeme(*token, context->data);
            append_ss(out, pstr);
        }
        
        start = token->start + token->size;
        
        if (finish){
            break;
        }
    }
}
                                     
static Alternate_Names_Array
allocate_app_api(Partition *part, int32_t count){
    Alternate_Names_Array app_api = {0};
    app_api.names = push_array(part, Alternate_Name, count);
    memset(app_api.names, 0, sizeof(Alternate_Name)*count);
    return(app_api);
}
                                     
static void
assert_files_are_equal(char *directory, char *filename1, char *filename2){
    char space[256];
    String name = make_fixed_width_string(space);
    append_sc(&name, directory);
    append_sc(&name, "/");
    append_sc(&name, filename1);
    terminate_with_null(&name);
                                     
    String file1 = file_dump(name.str);
                                     
    name.size = 0;
    append_sc(&name, directory);
    append_sc(&name, "/");
    append_sc(&name, filename2);
    terminate_with_null(&name);
                                     
    String file2 = file_dump(name.str);
                                     
    if (!match_ss(file1, file2)){
        fprintf(stderr, "Failed transitional test: %s != %s\n", filename1, filename2);
    }
    else{
        fprintf(stderr, "Passed transitional test: %s == %s\n", filename1, filename2);
    }
    }
                                     
    static void
        do_html_output(Document_System *doc_system, Partition *part, char *dst_directory, Abstract_Document *doc){
            // NOTE(allen): Output
            Temp_Memory temp = begin_temp_memory(part);
            String out = str_alloc(part, 10 << 20);
            Out_Context context = {0};
            set_context_directory(&context, dst_directory);
            
            // Output Docs
            char space[256];
            if (doc_get_link_string(doc, space, sizeof(space))){
            if (begin_file_out(&context, space, &out)){
                generate_document_html(&out, part, doc_system, doc);
                end_file_out(context);
            }
            else{
                fprintf(stderr, "Failed to open %s", space);
            }
        }
            end_temp_memory(temp);
    }
    
    static Abstract_Document*
        generate_homepage(Document_System *doc_system, Partition *part, char *src_directory){
            Enriched_Text *home = push_struct(part, Enriched_Text);
            *home = load_enriched_text(part, src_directory, "home.txt");
            
            Abstract_Document *doc = begin_document_description(doc_system, "4coder Home", "home");
            add_enriched_text(doc, home);
            end_document_description(doc);
            
            return(doc);
    }
    
    static Abstract_Document*
        generate_4coder_docs(Document_System *doc_system, Partition *part, char *code_directory, char *src_directory){
        static Meta_Keywords meta_keywords[] = {
            {make_lit_string("API_EXPORT")        , Item_Function } ,
            {make_lit_string("API_EXPORT_INLINE") , Item_Function } ,
            {make_lit_string("API_EXPORT_MACRO")  , Item_Macro    } ,
            {make_lit_string("CPP_NAME")          , Item_CppName  } ,
            {make_lit_string("TYPEDEF") , Item_Typedef } ,
            {make_lit_string("STRUCT")  , Item_Struct  } ,
            {make_lit_string("UNION")   , Item_Union   } ,
            {make_lit_string("ENUM")    , Item_Enum    } ,
        };
        
#define ExpandArray(a) (a), (ArrayCount(a))
        
        Meta_Unit *custom_types_unit = push_struct(part, Meta_Unit);
        Meta_Unit *lexer_funcs_unit = push_struct(part, Meta_Unit);
        Meta_Unit *lexer_types_unit = push_struct(part, Meta_Unit);
        Meta_Unit *string_unit = push_struct(part, Meta_Unit);
        Meta_Unit *custom_funcs_unit = push_struct(part, Meta_Unit);
        
        Alternate_Names_Array *custom_func_names = push_struct(part, Alternate_Names_Array);
        
        Enriched_Text *introduction = push_struct(part, Enriched_Text);
        Enriched_Text *lexer_introduction = push_struct(part, Enriched_Text);
        
        // NOTE(allen): Parse the important code.
        *custom_types_unit = compile_meta_unit(part, code_directory, "4coder_types.h", ExpandArray(meta_keywords));
        
        *lexer_funcs_unit = compile_meta_unit(part, code_directory, "4cpp_lexer.h", ExpandArray(meta_keywords));
        
        *lexer_types_unit = compile_meta_unit(part, code_directory, "4cpp_lexer_types.h", ExpandArray(meta_keywords));
        
        *string_unit = compile_meta_unit(part, code_directory, "internal_4coder_string.cpp", ExpandArray(meta_keywords));
        
        static char *functions_files[] = {
            "4ed_api_implementation.cpp",
            "win32_api_impl.cpp",
            0
        };
        
        *custom_funcs_unit = compile_meta_unit(part, code_directory, functions_files, ExpandArray(meta_keywords));
        
        
        // NOTE(allen): Compute and store variations of the custom function names
        *custom_func_names = allocate_app_api(part, custom_funcs_unit->set.count);
        
        for (int32_t i = 0; i < custom_funcs_unit->set.count; ++i){
            String name_string = custom_funcs_unit->set.items[i].name;
            String *macro = &custom_func_names->names[i].macro;
            String *public_name = &custom_func_names->names[i].public_name;
            
            *macro = str_alloc(part, name_string.size+4);
            to_upper_ss(macro, name_string);
            append_ss(macro, make_lit_string("_SIG"));
            
            *public_name = str_alloc(part, name_string.size);
            to_lower_ss(public_name, name_string);
            
            partition_align(part, 4);
        }
        
        // NOTE(allen): Load enriched text materials
        *introduction = load_enriched_text(part, src_directory, "introduction.txt");
        *lexer_introduction = load_enriched_text(part, src_directory, "lexer_introduction.txt");
        
        // NOTE(allen): Put together the abstract document
        Abstract_Document *doc = begin_document_description(doc_system, "4coder API Docs", "custom_docs");
        
        add_table_of_contents(doc);
        
        begin_section(doc, "Introduction", "introduction");
        add_enriched_text(doc, introduction);
        end_section(doc);
        
        begin_section(doc, "4coder Systems", "4coder_systems");
        add_todo(doc);
        end_section(doc);
        
        begin_section(doc, "Types and Functions", "types_and_functions");
        {
            begin_section(doc, "Function List", 0);
            add_element_list(doc, custom_funcs_unit, custom_func_names, AltName_Public_Name);
            end_section(doc);
            begin_section(doc, "Type List", 0);
            add_element_list(doc, custom_types_unit);
            end_section(doc);
            begin_section(doc, "Function Descriptions", 0);
            add_full_elements(doc, custom_funcs_unit, custom_func_names, AltName_Public_Name);
            end_section(doc);
            begin_section(doc, "Type Descriptions", 0);
            add_full_elements(doc, custom_types_unit);
            end_section(doc);
        }
        end_section(doc);
        
        begin_section(doc, "String Library", "string_library");
        {
            begin_section(doc, "String Library Intro", 0);
            add_todo(doc);
            end_section(doc);
            begin_section(doc, "String Function List", 0);
            add_element_list(doc, string_unit);
            end_section(doc);
            begin_section(doc, "String Function Descriptions", 0);
            add_full_elements(doc, string_unit);
            end_section(doc);
        }
        end_section(doc);
        
        begin_section(doc, "Lexer Library", "lexer_library");
        {
            begin_section(doc, "Lexer Intro", 0);
            add_enriched_text(doc, lexer_introduction);
            end_section(doc);
            begin_section(doc, "Lexer Function List", 0);
            add_element_list(doc, lexer_funcs_unit);
            end_section(doc);
            begin_section(doc, "Lexer Type List", 0);
            add_element_list(doc, lexer_types_unit);
            end_section(doc);
            begin_section(doc, "Lexer Function Descriptions", 0);
            add_full_elements(doc, lexer_funcs_unit);
            end_section(doc);
            begin_section(doc, "Lexer Type Descriptions", 0);
            add_full_elements(doc, lexer_types_unit);
            end_section(doc);
        }
        end_section(doc);
        
        end_document_description(doc);
        
        return(doc);
    }
    
    static Abstract_Document*
        generate_feature_list(Document_System *doc_system, Partition *part, char *src_directory){
            Enriched_Text *feature_list = push_struct(part, Enriched_Text);
            *feature_list = load_enriched_text(part, src_directory, "feature_list.txt");
            
            Abstract_Document *doc = begin_document_description(doc_system, "4coder Feature List", "features");
            add_enriched_text(doc, feature_list);
            end_document_description(doc);
            
        return(doc);
    }
    
    static Abstract_Document*
        generate_roadmap(Document_System *doc_system, Partition *part, char *src_directory){
            Enriched_Text *roadmap = push_struct(part, Enriched_Text);
            *roadmap = load_enriched_text(part, src_directory, "roadmap.txt");

            Abstract_Document *doc = begin_document_description(doc_system, "4coder Roadmap", "roadmap");
            add_enriched_text(doc, roadmap);
            end_document_description(doc);
            
            return(doc);
    }
    
static void
generate_site(char *code_directory, char *src_directory, char *dst_directory){
    int32_t size = (512 << 20);
    void *mem = malloc(size);
    memset(mem, 0, size);
    
    Partition part_ = make_part(mem, size);
    Partition *part = &part_;
    
    Document_System doc_system = create_document_system(part);
    
    generate_homepage(&doc_system, part, src_directory);
    generate_4coder_docs(&doc_system, part, code_directory, src_directory);
    generate_feature_list(&doc_system, part, src_directory);
    generate_roadmap(&doc_system, part, src_directory);
    
    for (Document_Node *node = doc_system.head;
         node != 0;
         node = node->next){
        do_html_output(&doc_system, part, dst_directory, &node->doc);
    }
}

int main(int argc, char **argv){
    if (argc == 4){
        generate_site(argv[1], argv[2], argv[3]);
    }
}

// BOTTOM

