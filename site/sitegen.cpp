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

//////////////////////////////////////////////////////////////////////////////////////////////////

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
    int32_t finish = false;
    int32_t do_whitespace_print = false;
    for (; (token = get_token(context)) != 0; get_next_token(context)){
        if (do_whitespace_print){
            pstr = str_start_end(context->data, start, token->start);
            append_ss(out, pstr);
        }
        else{
            do_whitespace_print = true;
        }
        
        do_print = true;
        if (token->type == CPP_TOKEN_COMMENT){
            lexeme = get_lexeme(*token, context->data);
            if (check_and_fix_docs(&lexeme)){
                do_print = false;
            }
        }
        else if (token->type == CPP_TOKEN_BRACE_OPEN){
            ++nest_level;
        }
        else if (token->type == CPP_TOKEN_BRACE_CLOSE){
            --nest_level;
            if (nest_level == 0){
                finish = true;
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
    append_sc(&name, "\\");
    append_sc(&name, filename1);
    terminate_with_null(&name);
                                     
    String file1 = file_dump(name.str);
                                     
    name.size = 0;
    append_sc(&name, directory);
    append_sc(&name, "\\");
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
generate_site(char *code_directory, char *src_directory, char *dst_directory){
#define API_DOC "4coder_API.html"
                                     
    int32_t size = (512 << 20);
    void *mem = malloc(size);
    memset(mem, 0, size);
                                     
    Partition part_ = make_part(mem, size);
    Partition *part = &part_;
                                     
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
                                     
    // NOTE(allen): Parse the important code.
    Meta_Unit custom_types_unit = compile_meta_unit(part, code_directory, "4coder_types.h", ExpandArray(meta_keywords));
                                     
    Meta_Unit lexer_funcs_unit = compile_meta_unit(part, code_directory, "4cpp_lexer.h", ExpandArray(meta_keywords));
                                     
    Meta_Unit lexer_types_unit = compile_meta_unit(part, code_directory, "4cpp_lexer_types.h", ExpandArray(meta_keywords));
                                     
    Meta_Unit string_unit = compile_meta_unit(part, code_directory, "internal_4coder_string.cpp", ExpandArray(meta_keywords));
                                     
    static char *functions_files[] = {
        "4ed_api_implementation.cpp",
        "win32_api_impl.cpp",
        0
    };
                                     
    Meta_Unit custom_funcs_unit = compile_meta_unit(part, code_directory, functions_files, ExpandArray(meta_keywords));
                                     
                                     
    // NOTE(allen): Compute and store variations of the custom function names
     Alternate_Names_Array custom_func_names = allocate_app_api(part, custom_funcs_unit.set.count);
                                     
    for (int32_t i = 0; i < custom_funcs_unit.set.count; ++i){
        String name_string = custom_funcs_unit.set.items[i].name;
        String *macro = &custom_func_names.names[i].macro;
        String *public_name = &custom_func_names.names[i].public_name;
                                     
        *macro = str_alloc(part, name_string.size+4);
        to_upper_ss(macro, name_string);
        append_ss(macro, make_lit_string("_SIG"));
                                     
        *public_name = str_alloc(part, name_string.size);
        to_lower_ss(public_name, name_string);
                                     
        partition_align(part, 4);
    }
                                     
    // NOTE(allen): Load enriched text materials
    Enriched_Text introduction = load_enriched_text(part, src_directory, "introduction.txt");
    Enriched_Text lexer_introduction = load_enriched_text(part, src_directory, "lexer_introduction.txt");
                                     
    // NOTE(allen): Put together the abstract document
    Abstract_Document doc = {0};
    begin_document_description(&doc, part, "4coder API Docs");
                                     
    add_table_of_contents(&doc);
                                     
    begin_section(&doc, "Introduction", "introduction");
    add_enriched_text(&doc, &introduction);
    end_section(&doc);
                                     
    begin_section(&doc, "4coder Systems", "4coder_systems");
    add_todo(&doc);
    end_section(&doc);
                                     
    begin_section(&doc, "Types and Functions", "types_and_functions");
    {
        begin_section(&doc, "Function List", 0);
        add_element_list(&doc, &custom_funcs_unit, &custom_func_names, AltName_Public_Name);
        end_section(&doc);
        begin_section(&doc, "Type List", 0);
        add_element_list(&doc, &custom_types_unit);
        end_section(&doc);
        begin_section(&doc, "Function Descriptions", 0);
        add_full_elements(&doc, &custom_funcs_unit, &custom_func_names, AltName_Public_Name);
        end_section(&doc);
        begin_section(&doc, "Type Descriptions", 0);
        add_full_elements(&doc, &custom_types_unit);
        end_section(&doc);
    }
    end_section(&doc);
                                     
    begin_section(&doc, "String Library", "string_library");
    {
        begin_section(&doc, "String Library Intro", 0);
        add_todo(&doc);
        end_section(&doc);
        begin_section(&doc, "String Function List", 0);
        add_element_list(&doc, &string_unit);
    end_section(&doc);
        begin_section(&doc, "String Function Descriptions", 0);
        add_full_elements(&doc, &string_unit);
        end_section(&doc);
    }
    end_section(&doc);
                                     
    begin_section(&doc, "Lexer Library", "lexer_library");
    {
        begin_section(&doc, "Lexer Intro", 0);
        add_enriched_text(&doc, &lexer_introduction);
        end_section(&doc);
        begin_section(&doc, "Lexer Function List", 0);
        add_element_list(&doc, &lexer_funcs_unit);
        end_section(&doc);
        begin_section(&doc, "Lexer Type List", 0);
        add_element_list(&doc, &lexer_types_unit);
        end_section(&doc);
        begin_section(&doc, "Lexer Function Descriptions", 0);
        add_full_elements(&doc, &lexer_funcs_unit);
        end_section(&doc);
        begin_section(&doc, "Lexer Type Descriptions", 0);
        add_full_elements(&doc, &lexer_types_unit);
        end_section(&doc);
    }
    end_section(&doc);
                                     
    end_document_description(&doc);
                                     
    // NOTE(allen): Output
    String out = str_alloc(part, 10 << 20);
    Out_Context context = {0};
    set_context_directory(&context, dst_directory);
                                     
    // Output Docs - General Document Generator
    if (begin_file_out(&context, "gen-test.html", &out)){
        generate_document_html(&out, part, &doc);
        end_file_out(context);
    }
    else{
        // TODO(allen): warning
    }
    
    // Output Docs - Direct Method
    if (begin_file_out(&context, API_DOC, &out)){
        Used_Links used_links = {0};
        init_used_links(part, &used_links, 4000);
        
        append_sc(&out,
                  "<html lang=\"en-US\">"
                  "<head>"
                  "<title>4coder API Docs</title>"
                  "<style>"
                  
                  "body { "
                  "background: " BACK_COLOR "; "
                  "color: " TEXT_COLOR "; "
                  "}"
                  
                  // H things
                  "h1,h2,h3,h4 { "
                  "color: " POP_COLOR_1 "; "
                  "margin: 0; "
                  "}"
                  
                  "h2 { "
                  "margin-top: 6mm; "
                  "}"
                  
                  "h3 { "
                  "margin-top: 5mm; margin-bottom: 5mm; "
                  "}"
                  
                  "h4 { "
                  "font-size: 1.1em; "
                  "}"
                  
                  // ANCHORS
                  "a { "
                  "color: " POP_COLOR_1 "; "
                  "text-decoration: none; "
                  "}"
                  "a:visited { "
                  "color: " VISITED_LINK "; "
                  "}"
                  "a:hover { "
                  "background: " POP_BACK_1 "; "
                  "}"
                  
                  // LIST
                  "ul { "
                  "list-style: none; "
                  "padding: 0; "
                  "margin: 0; "
                  "}"
                  
                  "</style>"
                  "</head>\n"
                  "<body>"
                  "<div style='font-family:Arial; margin: 0 auto; "
                  "width: 800px; text-align: justify; line-height: 1.25;'>"
                  //"<h1 style='margin-top: 5mm; margin-bottom: 5mm;'>4cpp Lexing Library</h1>");
                  
                  "<h1 style='margin-top: 5mm; margin-bottom: 5mm;'>4coder API Docs</h1>");
        
        struct Section{
            char *id_string;
            char *display_string;
        };
        
        static int32_t msection = -1;
        
        static Section sections[] = {
            {"introduction", "Introduction"},
            {"4coder_systems", "4coder Systems"},
            {"types_and_functions", "Types and Functions"},
            {"string_library", "String Library"},
            {"lexer_library", "Lexer Library"}
        };
        
        append_sc(&out, "<h3 style='margin:0;'>Table of Contents</h3><ul>");
        
        int32_t section_count = ArrayCount(sections);
        for (int32_t i = 0; i < section_count; ++i){
            append_sc         (&out, "<li><a href='#section_");
            append_sc         (&out, sections[i].id_string);
            append_sc         (&out, "'>&sect;");
            append_int_to_str (&out, i+1);
            append_s_char     (&out, ' ');
            append_sc         (&out, sections[i].display_string);
            append_sc         (&out, "</a></li>");
        }
        
        append_sc(&out, "</ul>");
        
#define MAJOR_SECTION "1"
        msection = 0;
        
        append_sc(&out, "\n<h2 id='section_");
        append_sc(&out, sections[msection].id_string);
        append_sc(&out, "'>&sect;"MAJOR_SECTION" ");
        append_sc(&out, sections[msection].display_string);
        append_sc(&out, "</h2>");
        
#if 0
        // NOTE(allen): doc intro for lexer standalone
        append_sc(&out,
                  "<div>"
                  "<p>This is the documentation for the 4cpp lexer version 1.1. "
                  "The documentation is the newest piece of this lexer project "
                  "so it may still have problems.  What is here should be correct "
                  "and mostly complete.</p>"
                  "<p>If you have questions or discover errors please contact "
                  "<span style='"CODE_STYLE"'>editor@4coder.net</span> or "
                  "to get help from community members you can post on the "
                  "4coder forums hosted on handmade.network at "
                  "<span style='"CODE_STYLE"'>4coder.handmade.network</span>.</p>"
                  "</div>");
#endif
        
        append_sc(&out,
                  "<div>"
                  "<p>This is the documentation for " VERSION ". The documentation is still "
                  "under construction so some of the links are linking to sections that "
                  "have not been written yet.  What is here should be correct and I suspect "
                  "useful even without some of the other sections.</p>"
                  "<p>If you have questions or discover errors please contact "
                  "<span style='"CODE_STYLE"'>editor@4coder.net</span> or "
                  "to get help from members of the 4coder and handmade network community you "
                  "can post on the  4coder forums hosted at "
                  "<span style='"CODE_STYLE"'>4coder.handmade.network</span>.</p>"
                  "</div>");
        
#undef MAJOR_SECTION
#define MAJOR_SECTION "2"
        msection = 1;
        
        // TODO(allen): Write the 4coder system descriptions.
        append_sc(&out, "\n<h2 id='section_");
        append_sc(&out, sections[msection].id_string);
        append_sc(&out, "'>&sect;"MAJOR_SECTION" ");
        append_sc(&out, sections[msection].display_string);
        append_sc(&out, "</h2>");
        
        append_sc(&out, "<div><i>Coming Soon</i><div>");
        
#undef MAJOR_SECTION
#define MAJOR_SECTION "3"
        msection = 2;
        
        append_sc(&out, "\n<h2 id='section_");
        append_sc(&out, sections[msection].id_string);
        append_sc(&out, "'>&sect;"MAJOR_SECTION" ");
        append_sc(&out, sections[msection].display_string);
        append_sc(&out, "</h2>");
        
#undef SECTION
#define SECTION MAJOR_SECTION".1"
        
        append_sc(&out, "<h3>&sect;"SECTION" Function List</h3><ul>");
        for (int32_t i = 0; i < custom_funcs_unit.set.count; ++i){
            print_item_in_list(&out, custom_func_names.names[i].public_name, "_doc");
        }
        append_sc(&out, "</ul>");
        
#undef SECTION
#define SECTION MAJOR_SECTION".2"
        
        append_sc(&out, "<h3>&sect;"SECTION" Type List</h3><ul>");
        for (int32_t i = 0; i < custom_types_unit.set.count; ++i){
            print_item_in_list(&out, custom_types_unit.set.items[i].name, "_doc");
        }
        append_sc(&out, "</ul>");
        
#undef SECTION
#define SECTION MAJOR_SECTION".3"
        
        append_sc(&out, "<h3>&sect;"SECTION" Function Descriptions</h3>");
        for (int32_t i = 0; i < custom_funcs_unit.set.count; ++i){
            Item_Node *item = &custom_funcs_unit.set.items[i];
            String name = custom_func_names.names[i].public_name;
            
            append_sc        (&out, "<div id='");
            append_ss        (&out, name);
            append_sc        (&out, "_doc' style='margin-bottom: 1cm;'>");
            append_sc        (&out, "<h4>&sect;"SECTION".");
            append_int_to_str(&out, i+1);
            append_sc        (&out, ": ");
            append_ss        (&out, name);
            append_sc        (&out, "</h4><div style='"CODE_STYLE" "DESCRIPT_SECTION_STYLE"'>");
            
            print_function_html(&out, &used_links, item->cpp_name, item->ret, "", name, item->breakdown);
            append_sc(&out, "</div>");
            
            print_function_docs(&out, part, name, item->doc_string);
            
            append_sc(&out, "</div><hr>");
        }
        
#undef SECTION
#define SECTION MAJOR_SECTION".4"
        
        append_sc(&out, "<h3>&sect;"SECTION" Type Descriptions</h3>");
        
        int32_t I = 1;
        for (int32_t i = 0; i < custom_types_unit.set.count; ++i, ++I){
            print_item_html(&out, part, &used_links, custom_types_unit.set.items + i, "_doc", SECTION, I, 0, 0);
        }
        
#undef MAJOR_SECTION
#define MAJOR_SECTION "4"
        msection = 3;
        
        append_sc(&out, "\n<h2 id='section_");
        append_sc(&out, sections[msection].id_string);
        append_sc(&out, "'>&sect;"MAJOR_SECTION" ");
        append_sc(&out, sections[msection].display_string);
        append_sc(&out, "</h2>");
        
#undef SECTION
#define SECTION MAJOR_SECTION".1"
        
        append_sc(&out, "<h3>&sect;"SECTION" String Library Intro</h3>");
        
        append_sc(&out, "<div><i>Coming Soon</i><div>");
        
#undef SECTION
#define SECTION MAJOR_SECTION".2"
        
        append_sc(&out, "<h3>&sect;"SECTION" String Function List</h3>");
        
        append_sc(&out, "<ul>");
        for (int32_t i = 0; i < string_unit.set.count; ++i){
            print_item_in_list(&out, string_unit.set.items[i].name, "_doc");
        }
        append_sc(&out, "</ul>");
        
#undef SECTION
#define SECTION MAJOR_SECTION".3"
        
        append_sc(&out, "<h3>&sect;"SECTION" String Function Descriptions</h3>");
        
        for (int32_t i = 0; i < string_unit.set.count; ++i){
            print_item_html(&out, part, &used_links, string_unit.set.items+i, "_doc", SECTION, i+1, 0, 0);
        }
        
#undef MAJOR_SECTION
#define MAJOR_SECTION "5"
        msection = 4;
        
        append_sc(&out, "\n<h2 id='section_");
        append_sc(&out, sections[msection].id_string);
        append_sc(&out, "'>&sect;"MAJOR_SECTION" ");
        append_sc(&out, sections[msection].display_string);
        append_sc(&out, "</h2>");
        
#undef SECTION
#define SECTION MAJOR_SECTION".1"
        
        append_sc(&out, "<h3>&sect;"SECTION" Lexer Intro</h3>");
        
        append_sc(&out,
                  "<div>"
                  "<p>The 4cpp lexer system provides a polished, fast, flexible system that "
                  "takes in C/C++ and outputs a tokenization of the text data.  There are "
                  "two API levels. One level is setup to let you easily get a tokenization "
                  "of the file.  This level manages memory for you with malloc to make it "
                  "as fast as possible to start getting your tokens. The second level "
                  "enables deep integration by allowing control over allocation, data "
                  "chunking, and output rate control.</p>"
                  "<p>To use the quick setup API you simply include 4cpp_lexer.h and read the "
                  "documentation at <a href='#cpp_lex_file_doc'>cpp_lex_file</a>.</p>"
                  "<p>To use the the fancier API include 4cpp_lexer.h and read the "
                  "documentation at <a href='#cpp_lex_step_doc'>cpp_lex_step</a>. "
                  "If you want to be absolutely sure you are not including malloc into "
                  "your program you can define FCPP_FORBID_MALLOC before the include and "
                  "the \"step\" API will continue to work.</p>"
                  "<p>There are a few more features in 4cpp that are not documented yet. "
                  "You are free to try to use these, but I am not totally sure they are "
                  "ready yet, and when they are they will be documented.</p>"
                  "</div>");
        
#undef SECTION
#define SECTION MAJOR_SECTION".2"
        
        append_sc(&out, "<h3>&sect;"SECTION" Lexer Function List</h3>");
        
        append_sc(&out, "<ul>");
        for (int32_t i = 0; i < lexer_funcs_unit.set.count; ++i){
            print_item_in_list(&out, lexer_funcs_unit.set.items[i].name, "_doc");
        }
        append_sc(&out, "</ul>");
        
#undef SECTION
#define SECTION MAJOR_SECTION".3"
        
        append_sc(&out, "<h3>&sect;"SECTION" Lexer Type List</h3>");
        
        append_sc(&out, "<ul>");
        for (int32_t i = 0; i < lexer_types_unit.set.count; ++i){
            print_item_in_list(&out, lexer_types_unit.set.items[i].name, "_doc");
        }
        append_sc(&out, "</ul>");
        
#undef SECTION
#define SECTION MAJOR_SECTION".4"
        
        append_sc(&out, "<h3>&sect;"SECTION" Lexer Function Descriptions</h3>");
        for (int32_t i = 0; i < lexer_funcs_unit.set.count; ++i){
            print_item_html(&out, part, &used_links, lexer_funcs_unit.set.items+i, "_doc", SECTION, i+1, 0, 0);
        }
        
#undef SECTION
#define SECTION MAJOR_SECTION".5"
        
        append_sc(&out, "<h3>&sect;"SECTION" Lexer Type Descriptions</h3>");
        for (int32_t i = 0; i < lexer_types_unit.set.count; ++i){
            print_item_html(&out, part, &used_links, lexer_types_unit.set.items+i, "_doc", SECTION, i+1, 0, 0);
        }
        
        append_sc(&out, "</div></body></html>");
        end_file_out(context);
    }
    else{
        // TODO(allen): warning
    }
    
    // Here to test the file equality tester
    // Output Docs - General Document Generator
    if (begin_file_out(&context, "gen-test2.html", &out)){
        generate_document_html(&out, part, &doc);
        end_file_out(context);
    }
    else{
        // TODO(allen): warning
    }
    
    assert_files_are_equal(dst_directory, API_DOC, API_DOC);
    assert_files_are_equal(dst_directory, "gen-test.html", "gen-test2.html");
    assert_files_are_equal(dst_directory, API_DOC, "gen-test.html");
}

int main(int argc, char **argv){
    if (argc == 4){
        generate_site(argv[1], argv[2], argv[3]);
    }
}

// BOTTOM

