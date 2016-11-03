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
#include "abstract_document.h"

#define InvalidPath Assert(!"Invalid path of execution")

// TODO(allen): Move the Out_Context into it's own file.

typedef struct Out_Context{
    char out_directory_space[256];
    String out_directory;
    FILE *file;
    String *str;
} Out_Context;

static void
set_context_directory(Out_Context *context, char *dst_directory){
    context->out_directory = make_fixed_width_string(context->out_directory_space);
    copy_sc(&context->out_directory, dst_directory);
}

static int32_t
begin_file_out(Out_Context *out_context, char *filename, String *out){
    char str_space[512];
    String name = make_fixed_width_string(str_space);
    if (out_context->out_directory.size > 0){
        append_ss(&name, out_context->out_directory);
        append_sc(&name, "\\");
    }
    append_sc(&name, filename);
    terminate_with_null(&name);
    
    int32_t r = 0;
    out_context->file = fopen(name.str, "wb");
    out_context->str = out;
    out->size = 0;
    if (out_context->file){
        r = 1;
    }
    
    return(r);
}

static void
dump_file_out(Out_Context out_context){
    fwrite(out_context.str->str, 1, out_context.str->size, out_context.file);
    out_context.str->size = 0;
}

static void
end_file_out(Out_Context out_context){
    dump_file_out(out_context);
    fclose(out_context.file);
}

static String
make_out_string(int32_t x){
    String str;
    str.size = 0;
    str.memory_size = x;
    str.str = (char*)malloc(x);
    return(str);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

//
// Meta Parse Rules
//

static void
init_used_links(Partition *part, Used_Links *used, int32_t count){
    used->strs = push_array(part, String, count);
    used->count = 0;
    used->max = count;
}

static int32_t
try_to_use(Used_Links *used, String str){
    int32_t result = 1;
    int32_t index = 0;
    
    if (string_set_match(used->strs, used->count, str, &index)){
        result = 0;
    }
    else{
        used->strs[used->count++] = str;
    }
    
    return(result);
}

static void
print_struct_html(String *out, Item_Node *member, int32_t hide_children){
    String name = member->name;
    String type = member->type;
    String type_postfix = member->type_postfix;
    
    append_ss     (out, type);
    append_s_char (out, ' ');
    append_ss     (out, name);
    append_ss     (out, type_postfix);
    
    if (match_ss(type, make_lit_string("struct")) ||
        match_ss(type, make_lit_string("union"))){
        
        if (hide_children){
            append_sc(out, " { /* non-public internals */ } ;");
        }
        else{
            append_sc(out, " {<br><div style='margin-left: 8mm;'>");
            
            for (Item_Node *member_iter = member->first_child;
                 member_iter != 0;
                 member_iter = member_iter->next_sibling){
                print_struct_html(out, member_iter, hide_children);
            }
            
            append_sc(out, "</div>};<br>");
        }
    }
    else{
        append_sc(out, ";<br>");
    }
}

static void
print_function_html(String *out, Used_Links *used, String cpp_name, String ret, char *function_call_head, String name, Argument_Breakdown breakdown){
    
    append_ss     (out, ret);
    append_s_char (out, ' ');
    append_sc     (out, function_call_head);
    append_ss     (out, name);
    
    if (breakdown.count == 0){
        append_sc(out, "()");
}
else if (breakdown.count == 1){
    append_sc(out, "(");
        append_ss(out, breakdown.args[0].param_string);
    append_sc(out, ")");
}
else{
    append_sc(out, "(<div style='margin-left: 4mm;'>");
    
    for (int32_t j = 0; j < breakdown.count; ++j){
        append_ss(out, breakdown.args[j].param_string);
        if (j < breakdown.count - 1){
            append_s_char(out, ',');
        }
        append_sc(out, "<br>");
    }
    
    append_sc(out, "</div>)");
}
}

static void
print_macro_html(String *out, String name, Argument_Breakdown breakdown){
    
    append_sc (out, "#define ");
    append_ss (out, name);
    
    if (breakdown.count == 0){
        append_sc(out, "()");
    }
    else if (breakdown.count == 1){
        append_s_char  (out, '(');
        append_ss      (out, breakdown.args[0].param_string);
        append_s_char  (out, ')');
    }
    else{
        append_sc (out, "(<div style='margin-left: 4mm;'>");
        
        for (int32_t j = 0; j < breakdown.count; ++j){
            append_ss(out, breakdown.args[j].param_string);
            if (j < breakdown.count - 1){
                append_s_char(out, ',');
            }
            append_sc(out, "<br>");
        }
        
        append_sc(out, ")</div>)");
    }
}

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

static String
get_first_double_line(String source){
    String line = {0};
    int32_t pos0 = find_substr_s(source, 0, make_lit_string("\n\n"));
    int32_t pos1 = find_substr_s(source, 0, make_lit_string("\r\n\r\n"));
    if (pos1 < pos0){
        pos0 = pos1;
    }
    line = substr(source, 0, pos0);
    return(line);
}

static String
get_next_double_line(String source, String line){
    String next = {0};
    int32_t pos = (int32_t)(line.str - source.str) + line.size;
    int32_t start = 0, pos0 = 0, pos1 = 0;
    
    if (pos < source.size){
        assert(source.str[pos] == '\n' || source.str[pos] == '\r');
        start = pos + 1;
        
        if (start < source.size){
            pos0 = find_substr_s(source, start, make_lit_string("\n\n"));
            pos1 = find_substr_s(source, start, make_lit_string("\r\n\r\n"));
            if (pos1 < pos0){
                pos0 = pos1;
            }
            next = substr(source, start, pos0 - start);
        }
    }
    
    return(next);
}

static String
get_next_word(String source, String prev_word){
    String word = {0};
    int32_t pos0 = (int32_t)(prev_word.str - source.str) + prev_word.size;
    int32_t pos1 = 0;
    char c = 0;
    
    for (; pos0 < source.size; ++pos0){
        c = source.str[pos0];
        if (!(char_is_whitespace(c) || c == '(' || c == ')')){
            break;
        }
    }
    
    if (pos0 < source.size){
        for (pos1 = pos0; pos1 < source.size; ++pos1){
            c = source.str[pos1];
            if (char_is_whitespace(c) || c == '(' || c == ')'){
                break;
            }
        }
        
        word = substr(source, pos0, pos1 - pos0);
    }
    
    return(word);
}

static String
get_first_word(String source){
    String start_str = make_string(source.str, 0);
    String word = get_next_word(source, start_str);
    return(word);
}

enum Doc_Chunk_Type{
    DocChunk_PlainText,
    DocChunk_CodeExample,
    
    DocChunk_Count
};

static String doc_chunk_headers[] = {
    make_lit_string(""),
    make_lit_string("CODE_EXAMPLE"),
};

static String
get_next_doc_chunk(String source, String prev_chunk, Doc_Chunk_Type *type){
    String chunk = {0};
    String word = {0};
    int32_t pos = source.size;
    int32_t word_index = 0;
    Doc_Chunk_Type t = DocChunk_PlainText;
    
    int32_t start_pos = (int32_t)(prev_chunk.str - source.str) + prev_chunk.size;
    String source_tail = substr_tail(source, start_pos);
    
    Assert(DocChunk_Count == ArrayCount(doc_chunk_headers));
    
    for (word = get_first_word(source_tail);
         word.str;
         word = get_next_word(source_tail, word), ++word_index){
        
        for (int32_t i = 1; i < DocChunk_Count; ++i){
            if (match_ss(word, doc_chunk_headers[i])){
                pos = (int32_t)(word.str - source.str);
                t = (Doc_Chunk_Type)i;
                goto doublebreak;
            }
        }
    }
    doublebreak:;
    
    *type = DocChunk_PlainText;
    if (word_index == 0){
        *type = t;
        
        int32_t nest_level = 1;
        int32_t i = find_s_char(source, pos, '(');
        for (++i; i < source.size; ++i){
            if (source.str[i] == '('){
                ++nest_level;
            }
            else if (source.str[i] == ')'){
                --nest_level;
                if (nest_level == 0){
                    break;
                }
            }
        }
        
        pos = i+1;
    }
    
    chunk = substr(source, start_pos, pos - start_pos);
    
    int32_t is_all_white = 1;
    for (int32_t i = 0; i < chunk.size; ++i){
        if (!char_is_whitespace(chunk.str[i])){
            is_all_white = 0;
            break;
        }
    }
    
    if (is_all_white){
        chunk = null_string;
    }
    
    return(chunk);
}

static String
get_first_doc_chunk(String source, Doc_Chunk_Type *type){
    String start_str = make_string(source.str, 0);
    String chunk = get_next_doc_chunk(source, start_str, type);
    return(chunk);
}

static void
print_doc_description(String *out, Partition *part, String src){
    Doc_Chunk_Type type;
    
    for (String chunk = get_first_doc_chunk(src, &type);
         chunk.str;
         chunk = get_next_doc_chunk(src, chunk, &type)){
        
        switch (type){
            case DocChunk_PlainText:
            {
                for (String line = get_first_double_line(chunk);
                     line.str;
                     line = get_next_double_line(chunk, line)){
                    append_ss(out, line);
                    append_sc(out, "<br><br>");
                }
            }break;
            
            case DocChunk_CodeExample:
            {
                int32_t start = 0;
                int32_t end = chunk.size-1;
                while (start < end && chunk.str[start] != '(') ++start;
                start += 1;
                while (end > start && chunk.str[end] != ')') --end;
                
                
                append_sc(out, EXAMPLE_CODE_OPEN);
                
                if (start < end){
                    String code_example = substr(chunk, start, end - start);
                    int32_t first_line = 1;
                    
                    for (String line = get_first_line(code_example);
                         line.str;
                         line = get_next_line(code_example, line)){
                        
                        if (!(first_line && line.size == 0)){
                            int32_t space_i = 0;
                            for (; space_i < line.size; ++space_i){
                                if (line.str[space_i] == ' '){
                                    append_sc(out, "&nbsp;");
                                }
                                else{
                                    break;
                                }
                            }
                            
                            String line_tail = substr_tail(line, space_i);
                            append_ss(out, line_tail);
                            append_sc(out, "<br>");
                        }
                        first_line = 0;
                    }
                }
                
                append_sc(out, EXAMPLE_CODE_CLOSE);
            }break;
        }
    }
}

static void
print_struct_docs(String *out, Partition *part, Item_Node *member){
    for (Item_Node *member_iter = member->first_child;
         member_iter != 0;
         member_iter = member_iter->next_sibling){
        String type = member_iter->type;
        if (match_ss(type, make_lit_string("struct")) ||
            match_ss(type, make_lit_string("union"))){
            print_struct_docs(out, part, member_iter);
        }
        else{
            Documentation doc = {0};
            perform_doc_parse(part, member_iter->doc_string, &doc);
            
            append_sc(out, "<div>");
            
            append_sc(out, "<div style='"CODE_STYLE"'>"DOC_ITEM_HEAD_INL_OPEN);
            append_ss(out, member_iter->name);
            append_sc(out, DOC_ITEM_HEAD_INL_CLOSE"</div>");
            
            append_sc(out, "<div style='margin-bottom: 6mm;'>"DOC_ITEM_OPEN);
            print_doc_description(out, part, doc.main_doc);
            append_sc(out, DOC_ITEM_CLOSE"</div>");
            
            append_sc(out, "</div>");
        }
    }
}

static void
print_see_also(String *out, Documentation *doc){
    int32_t doc_see_count = doc->see_also_count;
    if (doc_see_count > 0){
        append_sc(out, DOC_HEAD_OPEN"See Also"DOC_HEAD_CLOSE);
        
        for (int32_t j = 0; j < doc_see_count; ++j){
            String see_also = doc->see_also[j];
            append_sc(out, DOC_ITEM_OPEN"<a href='#");
            append_ss(out, see_also);
            append_sc(out, "_doc'>");
            append_ss(out, see_also);
            append_sc(out, "</a>"DOC_ITEM_CLOSE);
        }
    }
}

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

static void
print_function_docs(String *out, Partition *part, String name, String doc_string){
    if (doc_string.size == 0){
        append_sc(out, "No documentation generated for this function.");
        fprintf(stderr, "warning: no documentation string for %.*s\n", name.size, name.str);
    }
    
    Temp_Memory temp = begin_temp_memory(part);
    
    Documentation doc = {0};
    
    perform_doc_parse(part, doc_string, &doc);
    
    int32_t doc_param_count = doc.param_count;
    if (doc_param_count > 0){
        append_sc(out, DOC_HEAD_OPEN"Parameters"DOC_HEAD_CLOSE);
        
        for (int32_t j = 0; j < doc_param_count; ++j){
            String param_name = doc.param_name[j];
            String param_docs = doc.param_docs[j];
            
            // TODO(allen): check that param_name is actually
            // a parameter to this function!
            
            append_sc(out, "<div>"DOC_ITEM_HEAD_OPEN);
            append_ss(out, param_name);
            append_sc(out, DOC_ITEM_HEAD_CLOSE"<div style='margin-bottom: 6mm;'>"DOC_ITEM_OPEN);
            append_ss(out, param_docs);
            append_sc(out, DOC_ITEM_CLOSE"</div></div>");
        }
    }
    
    String ret_doc = doc.return_doc;
    if (ret_doc.size != 0){
        append_sc(out, DOC_HEAD_OPEN"Return"DOC_HEAD_CLOSE DOC_ITEM_OPEN);
        append_ss(out, ret_doc);
        append_sc(out, DOC_ITEM_CLOSE);
    }
    
    String main_doc = doc.main_doc;
    if (main_doc.size != 0){
        append_sc(out, DOC_HEAD_OPEN"Description"DOC_HEAD_CLOSE DOC_ITEM_OPEN);
        print_doc_description(out, part, main_doc);
        append_sc(out, DOC_ITEM_CLOSE);
    }
    
    print_see_also(out, &doc);
    
    end_temp_memory(temp);
}

static void
print_item_in_list(String *out, String name, char *id_postfix){
    append_sc(out, "<li><a href='#");
    append_ss(out, name);
    append_sc(out, id_postfix);
    append_sc(out, "'>");
    append_ss(out, name);
    append_sc(out, "</a></li>");
}

static void
print_item(String *out, Partition *part, Used_Links *used,
           Item_Node *item, char *id_postfix, char *function_prefix,
           char *section, int32_t I){
    Temp_Memory temp = begin_temp_memory(part);
    
    String name = item->name;
    /* NOTE(allen):
    Open a div for the whole item.
    Put a heading in it with the name and section.
    Open a "descriptive" box for the display of the code interface.
    */
    append_sc(out, "<div id='");
    append_ss(out, name);
    append_sc(out, id_postfix);
    append_sc(out, "' style='margin-bottom: 1cm;'>");
    
    int32_t has_cpp_name = 0;
    if (item->cpp_name.str != 0){
        if (try_to_use(used, item->cpp_name)){
            append_sc(out, "<div id='");
            append_ss(out, item->cpp_name);
            append_sc(out, id_postfix);
            append_sc(out, "'>");
            has_cpp_name = 1;
        }
    }
    
    append_sc         (out, "<h4>&sect;");
    append_sc         (out, section);
    append_s_char     (out, '.');
    append_int_to_str (out, I);
    append_sc         (out, ": ");
    append_ss         (out, name);
    append_sc         (out, "</h4>");
    
    append_sc(out, "<div style='"CODE_STYLE" "DESCRIPT_SECTION_STYLE"'>");
    
    switch (item->t){
        case Item_Function:
        {
            // NOTE(allen): Code box
            Assert(function_prefix != 0);
            print_function_html(out, used, item->cpp_name, item->ret, function_prefix, item->name, item->breakdown);
            
            // NOTE(allen): Close the code box
            append_sc(out, "</div>");
            
            // NOTE(allen): Descriptive section
            print_function_docs(out, part, item->name, item->doc_string);
        }break;
        
        case Item_Macro:
        {
            // NOTE(allen): Code box
            print_macro_html(out, item->name, item->breakdown);
            
            // NOTE(allen): Close the code box
            append_sc(out, "</div>");
            
            // NOTE(allen): Descriptive section
            print_function_docs(out, part, item->name, item->doc_string);
        }break;
        
        case Item_Typedef:
        {
            String type = item->type;
            
            // NOTE(allen): Code box
            append_sc     (out, "typedef ");
            append_ss     (out, type);
            append_s_char (out, ' ');
            append_ss     (out, name);
            append_s_char (out, ';');
            
            // NOTE(allen): Close the code box
            append_sc(out, "</div>");
            
            // NOTE(allen): Descriptive section
            String doc_string = item->doc_string;
            Documentation doc = {0};
            perform_doc_parse(part, doc_string, &doc);
            
            String main_doc = doc.main_doc;
            if (main_doc.size != 0){
                append_sc(out, DOC_HEAD_OPEN"Description"DOC_HEAD_CLOSE);
                
                append_sc(out, DOC_ITEM_OPEN);
                print_doc_description(out, part, main_doc);
                append_sc(out, DOC_ITEM_CLOSE);
            }
            else{
                fprintf(stderr, "warning: no documentation string for %.*s\n", name.size, name.str);
            }
            
            print_see_also(out, &doc);
            
        }break;
        
        case Item_Enum:
        {
            // NOTE(allen): Code box
            append_sc     (out, "enum ");
            append_ss     (out, name);
            append_s_char (out, ';');
            
            // NOTE(allen): Close the code box
            append_sc(out, "</div>");
            
            // NOTE(allen): Descriptive section
            String doc_string = item->doc_string;
            Documentation doc = {0};
            perform_doc_parse(part, doc_string, &doc);
            
            String main_doc = doc.main_doc;
            if (main_doc.size != 0){
                append_sc(out, DOC_HEAD_OPEN"Description"DOC_HEAD_CLOSE);
                
                append_sc(out, DOC_ITEM_OPEN);
                print_doc_description(out, part, main_doc);
                append_sc(out, DOC_ITEM_CLOSE);
            }
            else{
                fprintf(stderr, "warning: no documentation string for %.*s\n", name.size, name.str);
            }
            
            if (item->first_child){
                append_sc(out, DOC_HEAD_OPEN"Values"DOC_HEAD_CLOSE);
                
                for (Item_Node *member = item->first_child;
                     member;
                     member = member->next_sibling){
                    Documentation doc = {0};
                    perform_doc_parse(part, member->doc_string, &doc);
                    
                    append_sc(out, "<div>");
                    
                    // NOTE(allen): Dafuq is this all?
                    append_sc(out, "<div><span style='"CODE_STYLE"'>"DOC_ITEM_HEAD_INL_OPEN);
                    append_ss(out, member->name);
                    append_sc(out, DOC_ITEM_HEAD_INL_CLOSE);
                    
                    if (member->value.str){
                        append_sc(out, " = ");
                        append_ss(out, member->value);
                    }
                    
                    append_sc(out, "</span></div>");
                    
                    append_sc(out, "<div style='margin-bottom: 6mm;'>"DOC_ITEM_OPEN);
                    print_doc_description(out, part, doc.main_doc);
                    append_sc(out, DOC_ITEM_CLOSE"</div>");
                    
                    append_sc(out, "</div>");
                }
            }
            
            print_see_also(out, &doc);
            
        }break;
        
        case Item_Struct: case Item_Union:
        {
            String doc_string = item->doc_string;
            
            int32_t hide_members = 0;
            
            if (doc_string.size == 0){
                hide_members = 1;
            }
            else{
                for (String word = get_first_word(doc_string);
                     word.str;
                     word = get_next_word(doc_string, word)){
                    if (match_ss(word, make_lit_string("HIDE_MEMBERS"))){
                        hide_members = 1;
                        break;
                    }
                }
            }
            
            // NOTE(allen): Code box
            print_struct_html(out, item, hide_members);
            
            // NOTE(allen): Close the code box
            append_sc(out, "</div>");
            
            // NOTE(allen): Descriptive section
            {
                Documentation doc = {0};
                perform_doc_parse(part, doc_string, &doc);
                
                String main_doc = doc.main_doc;
                if (main_doc.size != 0){
                    append_sc(out, DOC_HEAD_OPEN"Description"DOC_HEAD_CLOSE);
                    
                    append_sc(out, DOC_ITEM_OPEN);
                    print_doc_description(out, part, main_doc);
                    append_sc(out, DOC_ITEM_CLOSE);
                }
                else{
                    fprintf(stderr, "warning: no documentation string for %.*s\n", name.size, name.str);
                }
                
                if (!hide_members){
                    if (item->first_child){
                        append_sc(out, DOC_HEAD_OPEN"Fields"DOC_HEAD_CLOSE);
                        print_struct_docs(out, part, item);
                    }
                }
                
                print_see_also(out, &doc);
            }
        }break;
    }
    
    if (has_cpp_name){
        append_sc(out, "</div>");
    }
    
    // NOTE(allen): Close the item box
    append_sc(out, "</div><hr>");
    
    end_temp_memory(temp);
}

typedef struct App_API_Name{
    String macro;
    String public_name;
} App_API_Name;

typedef struct App_API{
    App_API_Name *names;
} App_API;

static App_API
allocate_app_api(Partition *part, int32_t count){
    App_API app_api = {0};
    app_api.names = push_array(part, App_API_Name, count);
    memset(app_api.names, 0, sizeof(App_API_Name)*count);
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
    
    if (!match(file1, file2)){
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
    App_API func_4ed_names = allocate_app_api(part, custom_funcs_unit.set.count);
    
    for (int32_t i = 0; i < custom_funcs_unit.set.count; ++i){
        String name_string = custom_funcs_unit.set.items[i].name;
        String *macro = &func_4ed_names.names[i].macro;
        String *public_name = &func_4ed_names.names[i].public_name;
        
        *macro = str_alloc(part, name_string.size+4);
        to_upper_ss(macro, name_string);
        append_ss(macro, make_lit_string("_SIG"));
        
        *public_name = str_alloc(part, name_string.size);
        to_lower_ss(public_name, name_string);
        
        partition_align(part, 4);
    }
    
    
    // NOTE(allen): Put together the abstract document
    Abstract_Document doc = {0};
    begin_document_description(&doc, part);
    
    begin_section(&doc, "Intro");
    add_todo(&doc);
    end_section(&doc);
    
    begin_section(&doc, "4coder Systems Overview");
    add_todo(&doc);
    end_section(&doc);
    
    begin_section(&doc, "Types and Functions");
    {
        begin_section(&doc, "Function List");
        add_element_list(&doc, &custom_funcs_unit);
        end_section(&doc);
        begin_section(&doc, "Type List");
        add_element_list(&doc, &custom_types_unit);
        end_section(&doc);
        begin_section(&doc, "Function Descriptions");
        add_full_elements(&doc, &custom_funcs_unit);
        end_section(&doc);
        begin_section(&doc, "Type Descriptions");
        add_full_elements(&doc, &custom_types_unit);
        end_section(&doc);
    }
    end_section(&doc);
    
    begin_section(&doc, "String Library");
    {
        begin_section(&doc, "String Library Intro");
        add_todo(&doc);
        end_section(&doc);
    begin_section(&doc, "String Function List");
        add_element_list(&doc, &string_unit);
    end_section(&doc);
        begin_section(&doc, "String Function Descriptions");
        add_full_elements(&doc, &string_unit);
        end_section(&doc);
    }
    end_section(&doc);
    
    begin_section(&doc, "Lexer Library");
    {
        begin_section(&doc, "Lexer Intro");
        add_todo(&doc);
        end_section(&doc);
        begin_section(&doc, "Lexer Function List");
        add_element_list(&doc, &lexer_funcs_unit);
        end_section(&doc);
        begin_section(&doc, "Lexer Type List");
        add_element_list(&doc, &lexer_types_unit);
        end_section(&doc);
        begin_section(&doc, "Lexer Function Descriptions");
        add_full_elements(&doc, &lexer_funcs_unit);
        end_section(&doc);
        begin_section(&doc, "Lexer Type Descriptions");
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
        generate_document_html(&context, &doc);
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
                  //                  "<h1 style='margin-top: 5mm; margin-bottom: 5mm;'>4cpp Lexing Library</h1>");
                  
                  "<h1 style='margin-top: 5mm; margin-bottom: 5mm;'>4coder API</h1>");
        
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
        
        append_sc(&out, "<h3 style='margin:0;'>Table of Contents</h3>""<ul>");
        
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
                  "<span style='"CODE_STYLE"'>4coder.handmade.network</span></p>"
                  "</div>");
#endif
        
        append_sc(&out,
                  "<div>"
                  "<p>This is the documentation for " VERSION " The documentation is still "
                  "under construction so some of the links are linking to sections that "
                  "have not been written yet.  What is here should be correct and I suspect "
                  "useful even without some of the other sections.</p>"
                  "<p>If you have questions or discover errors please contact "
                  "<span style='"CODE_STYLE"'>editor@4coder.net</span> or "
                  "to get help from community members you can post on the "
                  "4coder forums hosted on handmade.network at "
                  "<span style='"CODE_STYLE"'>4coder.handmade.network</span></p>"
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
            print_item_in_list(&out, func_4ed_names.names[i].public_name, "_doc");
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
            String name = func_4ed_names.names[i].public_name;
            
            append_sc        (&out, "<div id='");
            append_ss        (&out, name);
            append_sc        (&out, "_doc' style='margin-bottom: 1cm;'><h4>&sect;"SECTION".");
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
            print_item(&out, part, &used_links, custom_types_unit.set.items + i, "_doc", 0, SECTION, I);
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
        
        append_sc(&out, "<h3>&sect;"SECTION" String Intro</h3>");
        
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
            print_item(&out, part, &used_links, string_unit.set.items+i, "_doc", "", SECTION, i+1);
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
                  "The 4cpp lexer system provides a polished, fast, flexible system that "
                  "takes in C/C++ and outputs a tokenization of the text data.  There are "
                  "two API levels. One level is setup to let you easily get a tokenization "
                  "of the file.  This level manages memory for you with malloc to make it "
                  "as fast as possible to start getting your tokens. The second level "
                  "enables deep integration by allowing control over allocation, data "
                  "chunking, and output rate control.<br><br>"
                  "To use the quick setup API you simply include 4cpp_lexer.h and read the "
                  "documentation at <a href='#cpp_lex_file_doc'>cpp_lex_file</a>.<br><br>"
                  "To use the the fancier API include 4cpp_lexer.h and read the "
                  "documentation at <a href='#cpp_lex_step_doc'>cpp_lex_step</a>. "
                  "If you want to be absolutely sure you are not including malloc into "
                  "your program you can define FCPP_FORBID_MALLOC before the include and "
                  "the \"step\" API will continue to work.<br><br>"
                  "There are a few more features in 4cpp that are not documented yet. "
                  "You are free to try to use these, but I am not totally sure they are "
                  "ready yet, and when they are they will be documented."
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
        
        append_sc(&out, "<h3>&sect;"SECTION" Lexer Types List</h3>");
        
        append_sc(&out, "<ul>");
        for (int32_t i = 0; i < lexer_types_unit.set.count; ++i){
            print_item_in_list(&out, lexer_types_unit.set.items[i].name, "_doc");
        }
        append_sc(&out, "</ul>");
        
#undef SECTION
#define SECTION MAJOR_SECTION".4"
        
        append_sc(&out, "<h3>&sect;"SECTION" Lexer Function Descriptions</h3>");
        for (int32_t i = 0; i < lexer_funcs_unit.set.count; ++i){
            print_item(&out, part, &used_links, lexer_funcs_unit.set.items+i, "_doc", "", SECTION, i+1);
        }
        
#undef SECTION
#define SECTION MAJOR_SECTION".5"
        
        append_sc(&out, "<h3>&sect;"SECTION" Lexer Type Descriptions</h3>");
        for (int32_t i = 0; i < lexer_types_unit.set.count; ++i){
            print_item(&out, part, &used_links, lexer_types_unit.set.items+i, "_doc", "", SECTION, i+1);
        }
        
        append_sc(&out, "</div></body></html>");
        end_file_out(context);
    }
    else{
        // TODO(allen): warning
    }
    
    assert_files_are_equal(dst_directory, API_DOC, "gen-test.html");
}

int main(int argc, char **argv){
    if (argc == 4){
        generate_site(argv[1], argv[2], argv[3]);
    }
}

// BOTTOM

