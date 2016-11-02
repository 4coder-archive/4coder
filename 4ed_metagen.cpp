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

#if !defined(FSTRING_GUARD)
#include "internal_4coder_string.cpp"
#endif

#include "4cpp_lexer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "4coder_mem.h"
#include "meta_parser.cpp"

#define InvalidPath Assert(!"Invalid path of execution")

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
char *keys_that_need_codes[] = {
    "back",
    "up",
    "down",
    "left",
    "right",
    "del",
    "insert",
    "home",
    "end",
    "page_up",
    "page_down",
    "esc",
    
    "mouse_left",
    "mouse_right",
    "mouse_left_release",
    "mouse_right_release",
    
    "f1",
    "f2",
    "f3",
    "f4",
    "f5",
    "f6",
    "f7",
    "f8",
    
    "f9",
    "f10",
    "f11",
    "f12",
    "f13",
    "f14",
    "f15",
    "f16",
};

void
generate_keycode_enum(){
    char *filename_keycodes = "4coder_keycodes.h";
    Out_Context context = {0};
    int32_t i = 0, count = 0;
    unsigned char code = 1;
    
    String out = make_out_string(10 << 20);
    
    if (begin_file_out(&context, filename_keycodes, &out)){
        count = ArrayCount(keys_that_need_codes);
        
        append_sc(&out, "enum{\n");
        for (i = 0; i < count; i){
            if (strcmp(keys_that_need_codes[i], "f1") == 0 && code < 0x7F){
                code = 0x7F;
            }
            switch (code){
                case '\n': code++; break;
                case '\t': code++; break;
                case 0x20: code = 0x7F; break;
                default:
                append_sc(&out, "key_");
                append_sc(&out, keys_that_need_codes[i++]);
                append_sc(&out, " = ");
                append_int_to_str(&out, code++);
                append_sc(&out, ",\n");
                break;
            }
        }
        append_sc(&out, "};\n");
        
        append_sc(&out,
                  "static char*\n"
                  "global_key_name(int32_t key_code, int32_t *size){\n"
                  "char *result = 0;\n"
                  "switch(key_code){\n");
        
        for (i = 0; i < count; ++i){
            append_sc(&out, "case key_");
            append_sc(&out, keys_that_need_codes[i]);
            append_sc(&out, ": result = \"");
            append_sc(&out, keys_that_need_codes[i]);
            append_sc(&out, "\"; *size = sizeof(\"");
            append_sc(&out, keys_that_need_codes[i]);
            append_sc(&out, "\")-1; break;\n");
        }
        
        append_sc(&out,
                  "}\n"
                  "return(result);\n"
                  "}\n");
        
        end_file_out(context);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
static void
struct_begin(String *str, char *name){
    append_sc(str, "struct ");
    append_sc(str, name);
    append_sc(str, "{\n"); //}
}

static void
enum_begin(String *str, char *name){
    append_sc(str, "enum ");
    append_sc(str, name);
    append_sc(str, "{\n"); //}
}

static void
struct_end(String *str){ //{
    append_sc(str, "};\n\n");
}

static char* bar_style_fields[] = {
    "bar",
    "bar_active",
    "base",
    "pop1",
    "pop2",
};

static char* main_style_fields[] = {
    "back",
    "margin",
    "margin_hover",
    "margin_active",
    "cursor",
    "at_cursor",
    "highlight",
    "at_highlight",
    "mark",
    "default",
    "comment",
    "keyword",
    "str_constant",
    "char_constant",
    "int_constant",
    "float_constant",
    "bool_constant",
    "preproc",
    "include",
    "special_character",
    "ghost_character",
    "highlight_junk",
    "highlight_white",
    "paste",
    "undo",
    "next_undo",
};

static char*
make_style_tag(char *tag){
    char *str;
    int32_t len;
    
    len = (int32_t)strlen(tag);
    str = (char*)malloc(len + 1);
    to_camel_cc(tag, str);
    str[len] = 0;
    
    return(str);
}

static void
generate_style(){
    char filename_4coder[] = "4coder_style.h";
    char filename_4ed[] = "4ed_style.h";
    char *tag = 0;
    int32_t count = 0, i = 0;
    
    String out = make_out_string(10 << 20);
    Out_Context context = {0};
    
    if (begin_file_out(&context, filename_4coder, &out)){
        
        enum_begin(&out, "Style_Tag");
        {
            count = ArrayCount(bar_style_fields);
            for (i = 0; i < count; ++i){
                tag = make_style_tag(bar_style_fields[i]);
                append_sc(&out, "Stag_");
                append_sc(&out, tag);
                append_sc(&out, ",\n");
                free(tag);
            }
            
            count = ArrayCount(main_style_fields);
            for (i = 0; i < count; ++i){
                tag = make_style_tag(main_style_fields[i]);
                append_sc(&out, "Stag_");
                append_sc(&out, tag);
                append_sc(&out, ",\n");
                free(tag);
            }
        }
        struct_end(&out);
        
        end_file_out(context);
    }
    
    if (begin_file_out(&context, filename_4ed, &out)){
        
        struct_begin(&out, "Interactive_Style");
        {
            count = ArrayCount(bar_style_fields);
            for (i = 0; i < count; ++i){
                append_sc(&out, "u32 ");
                append_sc(&out, bar_style_fields[i]);
                append_sc(&out, "_color;\n");
            }
        }
        struct_end(&out);
        
        struct_begin(&out, "Style_Main_Data");
        {
            count = ArrayCount(main_style_fields);
            for (i = 0; i < count; ++i){
                append_sc(&out, "u32 ");
                append_sc(&out, main_style_fields[i]);
                append_sc(&out, "_color;\n");
            }
            append_sc(&out, "Interactive_Style file_info_style;\n");
        }
        struct_end(&out);
        
        {
            append_sc(&out,
                      "inline u32*\n"
                      "style_index_by_tag(Style_Main_Data *s, u32 tag){\n"
                      "u32 *result = 0;\n"
                      "switch (tag){\n");
            
            count = ArrayCount(bar_style_fields);
            for (i = 0; i < count; ++i){
                tag = make_style_tag(bar_style_fields[i]);
                append_sc(&out, "case Stag_");
                append_sc(&out, tag);
                append_sc(&out, ": result = &s->file_info_style.");
                append_sc(&out, bar_style_fields[i]);
                append_sc(&out, "_color; break;\n");
                free(tag);
            }
            
            count = ArrayCount(main_style_fields);
            for (i = 0; i < count; ++i){
                tag = make_style_tag(main_style_fields[i]);
                append_sc(&out, "case Stag_");
                append_sc(&out, tag);
                append_sc(&out, ": result = &s->");
                append_sc(&out, main_style_fields[i]);
                append_sc(&out, "_color; break;\n");
                free(tag);
            }
            
            append_sc(&out,
                      "}\n"
                      "return(result);\n"
                      "}\n\n");
        }
        
        end_file_out(context);
    }
    
    free(out.str);
}

//////////////////////////////////////////////////////////////////////////////////////////////////


//
// Meta Parse Rules
//

static void
print_function_body_code(String *out, Parse_Context *context, int32_t start){
    String pstr = {0}, lexeme = {0};
    Cpp_Token *token = 0;
    
    int32_t do_print = 0;
    int32_t nest_level = 0;
    int32_t finish = false;
    int32_t do_whitespace_print = false;
    int32_t is_first = true;
    
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
        if (is_first){
            do_print = false;
            is_first = false;
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
generate_custom_headers(){
#define API_H "4coder_custom_api.h"
#define OS_API_H "4ed_os_custom_api.h"
#define STRING_H "4coder_string.h"
    
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
    
    // NOTE(allen): Parse the internal string file.
    static char *string_files[] = {
        "internal_4coder_string.cpp",
        0
    };
    
    Meta_Unit string_unit = compile_meta_unit(part, ".", string_files, ExpandArray(meta_keywords));
    
    
    // NOTE(allen): Parse the customization API files
    static char *functions_files[] = {
        "4ed_api_implementation.cpp",
        "win32_api_impl.cpp",
        0
    };
    
    Meta_Unit unit_custom = compile_meta_unit(part, ".", functions_files, ExpandArray(meta_keywords));
    
    
    // NOTE(allen): Compute and store variations of the function names
    App_API func_4ed_names = allocate_app_api(part, unit_custom.set.count);
    
    for (int32_t i = 0; i < unit_custom.set.count; ++i){
        String name_string = unit_custom.set.items[i].name;
        String *macro = &func_4ed_names.names[i].macro;
        String *public_name = &func_4ed_names.names[i].public_name;
        
        *macro = str_alloc(part, name_string.size+4);
        to_upper_ss(macro, name_string);
        append_ss(macro, make_lit_string("_SIG"));
        
        *public_name = str_alloc(part, name_string.size);
        to_lower_ss(public_name, name_string);
        
        partition_align(part, 4);
    }
    
    // NOTE(allen): Output
    String out = str_alloc(part, 10 << 20);
    Out_Context context = {0};
    
    // NOTE(allen): Custom API headers
    if (begin_file_out(&context, OS_API_H, &out)){
        int32_t main_api_count = unit_custom.parse[0].item_count;
        int32_t os_api_count = unit_custom.parse[1].item_count;
        for (int32_t i = main_api_count; i < os_api_count; ++i){
            append_sc(&out, "#define ");
            append_ss(&out, func_4ed_names.names[i].macro);
            append_sc(&out, "(n) ");
            append_ss(&out, unit_custom.set.items[i].ret);
            append_sc(&out, " n");
            append_ss(&out, unit_custom.set.items[i].args);
            append_s_char(&out, '\n');
        }
        
        for (int32_t i = main_api_count; i < os_api_count; ++i){
            append_sc(&out, "typedef ");
            append_ss(&out, func_4ed_names.names[i].macro);
            append_s_char(&out, '(');
            append_ss(&out, unit_custom.set.items[i].name);
            append_sc(&out, "_Function);\n");
        }
        
        end_file_out(context);
    }
    else{
        // TODO(allen): warning
    }
    
    if (begin_file_out(&context, API_H, &out)){
        for (int32_t i = 0; i < unit_custom.set.count; ++i){
            append_sc(&out, "#define ");
            append_ss(&out, func_4ed_names.names[i].macro);
            append_sc(&out, "(n) ");
            append_ss(&out, unit_custom.set.items[i].ret);
            append_sc(&out, " n");
            append_ss(&out, unit_custom.set.items[i].args);
            append_s_char(&out, '\n');
        }
        
        for (int32_t i = 0; i < unit_custom.set.count; ++i){
            append_sc(&out, "typedef ");
            append_ss(&out, func_4ed_names.names[i].macro);
            append_s_char(&out, '(');
            append_ss(&out, unit_custom.set.items[i].name);
            append_sc(&out, "_Function);\n");
        }
        
        append_sc(&out, "struct Application_Links{\n");
        
        
        append_sc(&out, "#if defined(ALLOW_DEP_4CODER)\n");
        for (int32_t i = 0; i < unit_custom.set.count; ++i){
            append_ss(&out, unit_custom.set.items[i].name);
            append_sc(&out, "_Function *");
            append_ss(&out, func_4ed_names.names[i].public_name);
            append_sc(&out, ";\n");
        }
        
        append_sc(&out, "#else\n");
        
        for (int32_t i = 0; i < unit_custom.set.count; ++i){
            append_ss(&out, unit_custom.set.items[i].name);
            append_sc(&out, "_Function *");
            append_ss(&out, func_4ed_names.names[i].public_name);
            append_sc(&out, "_;\n");
        }
        append_sc(&out, "#endif\n");
        
        append_sc(&out,
                  "void *memory;\n"
                  "int32_t memory_size;\n"
                  "void *cmd_context;\n"
                  "void *system_links;\n"
                  "void *current_coroutine;\n"
                  "int32_t type_coroutine;\n"
                  "};\n");
        
        append_sc(&out, "#define FillAppLinksAPI(app_links) do{");
        for (int32_t i = 0; i < unit_custom.set.count; ++i){
            append_sc(&out, "\\\napp_links->");
            append_ss(&out, func_4ed_names.names[i].public_name);
            append_sc(&out, "_ = ");
            append_ss(&out, unit_custom.set.items[i].name);
            append_s_char(&out, ';');
        }
        append_sc(&out, "} while(false)\n");
        
        append_sc(&out, "#if defined(ALLOW_DEP_4CODER)\n");
        for (int32_t use_dep = 1; use_dep >= 0; --use_dep){
            for (int32_t i = 0; i < unit_custom.set.count; ++i){
                Argument_Breakdown breakdown = unit_custom.set.items[i].breakdown;
                String ret = unit_custom.set.items[i].ret;
                String public_name = func_4ed_names.names[i].public_name;
                
                append_sc(&out, "static inline ");
                append_ss(&out, ret);
                append_sc(&out, " ");
                append_ss(&out, public_name);
                
                append_sc(&out, "(");
                for (int32_t j = 0; j < breakdown.count; ++j){
                    append_ss(&out, breakdown.args[j].param_string);
                    if (j+1 != breakdown.count){
                        append_sc(&out, ", ");
                    }
                }
                append_sc(&out, "){");
                
                if (match_ss(ret, make_lit_string("void"))){
                    append_sc(&out, "(");
                }
                else{
                    append_sc(&out, "return(");
                }
                
                append_sc(&out, "app->");
                append_ss(&out, public_name);
                if (!use_dep){
                    append_sc(&out, "_");
                }
                
                append_sc(&out, "(");
                for (int32_t j = 0; j < breakdown.count; ++j){
                    append_ss(&out, breakdown.args[j].param_name);
                    if (j+1 != breakdown.count){
                        append_sc(&out, ", ");
                    }
                }
                append_sc(&out, ")");
                
                append_sc(&out, ");}\n");
            }
            if (use_dep == 1){
                append_sc(&out, "#else\n");
            }
        }
        append_sc(&out, "#endif\n");
        
        end_file_out(context);
    }
    else{
        // TODO(allen): warning
    }
    
    // NOTE(allen): String Library
    if (begin_file_out(&context, STRING_H, &out)){
        Cpp_Token *token = 0;
        int32_t start = 0;
        
        Parse parse = string_unit.parse[0];
        Parse_Context pcontext = setup_parse_context(parse);
        
        for (; (token = get_token(&pcontext)) != 0; get_next_token(&pcontext)){
            if (!(token->flags & CPP_TFLAG_PP_BODY) &&
                token->type == CPP_TOKEN_IDENTIFIER){
                String lexeme = get_lexeme(*token, pcontext.data);
                if (match_ss(lexeme, make_lit_string("FSTRING_BEGIN"))){
                    start = token->start + token->size;
                    break;
                }
            }
        }
        
        String pstr = {0};
        int32_t do_whitespace_print = true;
        
        for(;(token = get_next_token(&pcontext)) != 0;){
            if (do_whitespace_print){
                pstr = str_start_end(pcontext.data, start, token->start);
                append_ss(&out, pstr);
            }
            else{
                do_whitespace_print = true;
            }
            
            String lexeme = get_lexeme(*token, pcontext.data);
            
            int32_t do_print = true;
            if (match_ss(lexeme, make_lit_string("FSTRING_DECLS"))){
                append_sc(&out, "#if !defined(FCODER_STRING_H)\n#define FCODER_STRING_H\n\n");
                do_print = false;
                
                static int32_t SIG_PADDING = 35;
                
                for (int32_t j = 0; j < string_unit.set.count; ++j){
                    char line_[2048];
                    String line = make_fixed_width_string(line_);
                    Item_Node *item = string_unit.set.items + j;
                    
                    if (item->t == Item_Function){
                        //append_ss       (&line, item->marker);
                        //append_padding  (&line, ' ', RETURN_PADDING);
                        append_ss       (&line, item->ret);
                        append_padding  (&line, ' ', SIG_PADDING);
                        append_ss       (&line, item->name);
                        append_ss       (&line, item->args);
                        append_sc       (&line, ";\n");
                    }
                    else if (item->t == Item_Macro){
                        append_ss       (&line, make_lit_string("#ifndef "));
                        append_padding  (&line, ' ', 10);
                        append_ss       (&line, item->name);
                        append_s_char   (&line, '\n');
                        
                        append_ss       (&line, make_lit_string("# define "));
                        append_padding  (&line, ' ', 10);
                        append_ss       (&line, item->name);
                        append_ss       (&line, item->args);
                        append_s_char   (&line, ' ');
                        append_ss       (&line, item->body);
                        append_s_char   (&line, '\n');
                        
                        append_ss       (&line, make_lit_string("#endif"));
                        append_s_char   (&line, '\n');
                    }
                    else{
                        InvalidPath;
                    }
                    
                    append_ss(&out, line);
                }
                
                append_sc(&out, "\n#endif\n");
                
                // NOTE(allen): C++ overload definitions
                append_sc(&out, "\n#if !defined(FSTRING_C) && !defined(FSTRING_GUARD)\n\n");
                
                for (int32_t j = 0; j < string_unit.set.count; ++j){
                    char line_space[2048];
                    String line = make_fixed_width_string(line_space);
                    
                    Item_Node *item = &string_unit.set.items[j];
                    
                    if (item->t == Item_Function){
                        String cpp_name = item->cpp_name;
                        if (cpp_name.str != 0){
                            Argument_Breakdown breakdown = item->breakdown;
                            
                            append_ss     (&line, item->ret);
                            append_padding(&line, ' ', SIG_PADDING);
                            append_ss     (&line, cpp_name);
                            append_ss     (&line, item->args);
                            if (match_ss(item->ret, make_lit_string("void"))){
                                append_ss(&line, make_lit_string("{("));
                            }
                            else{
                                append_ss(&line, make_lit_string("{return("));
                            }
                            append_ss    (&line, item->name);
                            append_s_char(&line, '(');
                            
                            if (breakdown.count > 0){
                                for (int32_t i = 0; i < breakdown.count; ++i){
                                    if (i != 0){
                                        append_s_char(&line, ',');
                                    }
                                    append_ss(&line, breakdown.args[i].param_name);
                                }
                            }
                            else{
                                append_ss(&line, make_lit_string("void"));
                            }
                            
                            append_ss(&line, make_lit_string("));}\n"));
                            
                            append_ss(&out, line);
                        }
                    }
                }
                
                append_sc(&out, "\n#endif\n");
            }
            
            else if (match_ss(lexeme, make_lit_string("API_EXPORT_MACRO"))){
                token = get_next_token(&pcontext);
                if (token && token->type == CPP_TOKEN_COMMENT){
                    token = get_next_token(&pcontext);
                    if (token && token->type == CPP_PP_DEFINE){
                        for (;(token = get_next_token(&pcontext)) != 0;){
                            if (!(token->flags & CPP_TFLAG_PP_BODY)){
                                break;
                            }
                        }
                        if (token != 0){
                            get_prev_token(&pcontext);
                        }
                        do_print = false;
                        do_whitespace_print = false;
                    }
                }
            }
            
            else if (match_ss(lexeme, make_lit_string("API_EXPORT")) ||
                     match_ss(lexeme, make_lit_string("API_EXPORT_INLINE"))){
                if (!(token->flags & CPP_TFLAG_PP_BODY)){
                    if (match_ss(lexeme, make_lit_string("API_EXPORT_INLINE"))){
                        append_sc(&out, "#if !defined(FSTRING_GUARD)\n");
                    }
                    else{
                        append_sc(&out, "#if defined(FSTRING_IMPLEMENTATION)\n");
                    }
                    print_function_body_code(&out, &pcontext, start);
                    append_sc(&out, "\n#endif");
                    do_print = false;
                }
            }
            
            else if (match_ss(lexeme, make_lit_string("CPP_NAME"))){
                Cpp_Token *token_start = token;
                int32_t has_cpp_name = false;
                
                token = get_next_token(&pcontext);
                if (token && token->type == CPP_TOKEN_PARENTHESE_OPEN){
                    token = get_next_token(&pcontext);
                    if (token && token->type == CPP_TOKEN_IDENTIFIER){
                        token = get_next_token(&pcontext);
                        if (token && token->type == CPP_TOKEN_PARENTHESE_CLOSE){
                            has_cpp_name = true;
                            do_print = false;
                        }
                    }
                }
                
                if (!has_cpp_name){
                    token = set_token(&pcontext, token_start);
                }
            }
            
            else if (token->type == CPP_TOKEN_COMMENT){
                if (check_and_fix_docs(&lexeme)){
                    do_print = false;
                }
            }
            
            if ((token = get_token(&pcontext)) != 0){
                if (do_print){
                    pstr = get_lexeme(*token, pcontext.data);
                    append_ss(&out, pstr);
                }
                start = token->start + token->size;
            }
        }
        pstr = str_start_end(pcontext.data, start, parse.code.size);
        append_ss(&out, pstr);
        
        end_file_out(context);
    }
    else{
        // TODO(allen): warning
    }
}

int main(int argc, char **argv){
    generate_keycode_enum();
    generate_style();
    generate_custom_headers();
}

// BOTTOM

