/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 25.02.2016
 *
 * File editing view for 4coder
 *
 */

// TOP

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../4ed_defines.h"
#include "../meta/4ed_meta_defines.h"

#include "../4coder_API/version.h"
#define FSTRING_IMPLEMENTATION
#include "../4coder_lib/4coder_string.h"
#include "../4cpp/4cpp_lexer.h"

#define FTECH_FILE_MOVING_IMPLEMENTATION
#include "../meta/4ed_file_moving.h"
#include "../meta/4ed_meta_parser.cpp"
#include "../meta/4ed_meta_keywords.h"
#include "../meta/4ed_out_context.cpp"
#include "4ed_abstract_document.cpp"

///////////////////////////////////////////////////////////////////////////

//
// Meta Parse Rules
//

static void
print_function_body_code(String *out, Parse_Context *context, i32 start){
    String pstr = {0}, lexeme = {0};
    Cpp_Token *token = 0;
    
    i32 do_print = 0;
    i32 nest_level = 0;
    i32 finish = 0;
    i32 do_whitespace_print = 0;
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

static void
do_html_output(Document_System *doc_system, char *dst_directory, Abstract_Item *doc){
    // NOTE(allen): Output
    i32 out_size = 10 << 20;
    void *mem = malloc(out_size);
    Assert(mem != 0);
    String out = make_string_cap(mem, 0, out_size);
    
    Out_Context context = {0};
    set_context_directory(&context, dst_directory);
    
    // Output Docs
    char space[256];
    if (doc_get_link_string(doc, space, sizeof(space))){
        if (begin_file_out(&context, space, &out)){
            generate_document_html(&out, doc_system, doc);
            end_file_out(context);
        }
        else{
            fprintf(stderr, "Failed to open %s\n", space);
        }
    }
    
    free(mem);
}

static Abstract_Item*
generate_homepage(Document_System *doc_system, char *src_directory){
    Enriched_Text *home = fm_push_array(Enriched_Text, 1);
    *home = load_enriched_text(src_directory, "home.txt");
    
    Abstract_Item *doc = begin_document_description(doc_system, "4coder Home", "home", 0);
    add_enriched_text(doc, home);
    end_document_description(doc);
    
    return(doc);
}

// TODO(allen): replace the documentation declaration system with a straight up enriched text system
static Abstract_Item*
generate_4coder_docs(Document_System *doc_system, char *code_directory, char *src_directory){
#define ExpandArray(a) (a), (ArrayCount(a))
    
    Meta_Unit *custom_types_unit = fm_push_array(Meta_Unit, 1);
    Meta_Unit *lexer_funcs_unit = fm_push_array(Meta_Unit, 1);
    Meta_Unit *lexer_types_unit = fm_push_array(Meta_Unit, 1);
    Meta_Unit *string_unit = fm_push_array(Meta_Unit, 1);
    Meta_Unit *custom_funcs_unit = fm_push_array(Meta_Unit, 1);
    
    Enriched_Text *introduction = fm_push_array(Enriched_Text, 1);
    Enriched_Text *lexer_introduction = fm_push_array(Enriched_Text, 1);
    
    // NOTE(allen): Parse the code.
    *custom_types_unit = compile_meta_unit(code_directory, "4coder_API/types.h", ExpandArray(meta_keywords));
    Assert(custom_types_unit->count != 0);
    
    *lexer_funcs_unit = compile_meta_unit(code_directory, "4cpp/4cpp_lexer.h", ExpandArray(meta_keywords));
    Assert(lexer_funcs_unit->count != 0);
    
    *lexer_types_unit = compile_meta_unit(code_directory, "4cpp/4cpp_lexer_types.h", ExpandArray(meta_keywords));
    Assert(lexer_types_unit->count != 0);
    
    *string_unit = compile_meta_unit(code_directory, "string/internal_4coder_string.cpp", ExpandArray(meta_keywords));
    Assert(string_unit->count != 0);
    
    *custom_funcs_unit = compile_meta_unit(code_directory, "4ed_api_implementation.cpp", ExpandArray(meta_keywords));
    Assert(custom_funcs_unit->count != 0);
    
    // NOTE(allen): Compute and store variations of the custom function names
    Alternate_Names_Array *custom_func_names = fm_push_array(Alternate_Names_Array, 1);
    i32 name_count = custom_funcs_unit->set.count;
    custom_func_names->names = fm_push_array(Alternate_Name, name_count);
    memset(custom_func_names->names, 0, sizeof(*custom_func_names->names)*name_count);
    
    for (i32 i = 0; i < custom_funcs_unit->set.count; ++i){
        String name_string = custom_funcs_unit->set.items[i].name;
        String *macro = &custom_func_names->names[i].macro;
        String *public_name = &custom_func_names->names[i].public_name;
        
        *macro = str_alloc(name_string.size+4);
        to_upper_ss(macro, name_string);
        append_ss(macro, make_lit_string("_SIG"));
        
        *public_name = str_alloc(name_string.size);
        to_lower_ss(public_name, name_string);
        
        fm_align();
    }
    
    // NOTE(allen): Load enriched text materials
    *introduction = load_enriched_text(src_directory, "introduction.txt");
    *lexer_introduction = load_enriched_text(src_directory, "lexer_introduction.txt");
    
    // NOTE(allen): Put together the abstract document
    Abstract_Item *doc = begin_document_description(doc_system, "4coder API Docs", "custom_docs", 1);
    
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

static Abstract_Item*
generate_feature_list(Document_System *doc_system, char *src_directory){
    Enriched_Text *feature_list = fm_push_array(Enriched_Text, 1);
    *feature_list = load_enriched_text(src_directory, "feature_list.txt");
    
    Abstract_Item *doc = begin_document_description(doc_system, "4coder Feature List", "features", 0);
    add_enriched_text(doc, feature_list);
    end_document_description(doc);
    
    return(doc);
}

static Abstract_Item*
generate_binding_list(Document_System *doc_system, char *src_directory){
    Enriched_Text *binding_list = fm_push_array(Enriched_Text, 1);
    *binding_list = load_enriched_text(src_directory, "binding_list.txt");
    
    Abstract_Item *doc = begin_document_description(doc_system, "4coder Binding List", "bindings", 0);
    add_enriched_text(doc, binding_list);
    end_document_description(doc);
    
    return(doc);
}

static Abstract_Item*
generate_roadmap(Document_System *doc_system, char *src_directory){
    Enriched_Text *roadmap = fm_push_array(Enriched_Text, 1);
    *roadmap = load_enriched_text(src_directory, "roadmap.txt");
    
    Abstract_Item *doc = begin_document_description(doc_system, "4coder Roadmap", "roadmap", 0);
    add_enriched_text(doc, roadmap);
    end_document_description(doc);
    
    return(doc);
}

static Abstract_Item*
generate_tutorials(Document_System *doc_system, char *src_directory){
    Enriched_Text *roadmap = fm_push_array(Enriched_Text, 1);
    *roadmap = load_enriched_text(src_directory, "tutorials.txt");
    
    Abstract_Item *doc = begin_document_description(doc_system, "4coder Tutorials", "tutorials", 0);
    add_enriched_text(doc, roadmap);
    end_document_description(doc);
    
    return(doc);
}

static String
push_string(i32 size){
    String str = {0};
    str.memory_size = size;
    str.str = fm_push_array(char, size);
    fm_align();
    return(str);
}

static void
do_image_resize(char *src_file, char *dst_file, char *extension, i32 w, i32 h){
    i32 x = 0, y = 0, channels = 0;
    stbi_uc *image = stbi_load(src_file, &x, &y, &channels, 0);
    
    stbi_uc *resized_image = (stbi_uc*)malloc(w*h*channels);
    stbir_resize_uint8(image, x, y, x*channels, resized_image, w, h, w*channels, channels);
    
    if (match_cc(extension, "png")){
        stbi_write_png(dst_file, w, h, channels, resized_image, w*channels);
    }
    
    free(image);
    free(resized_image);
}

static void
generate_site(char *code_directory, char *asset_directory, char *src_directory, char *dst_directory){
    String str;
    
    Document_System doc_system = create_document_system();
    
    // TODO(allen): code compression here
    struct Site_Asset{
        char *filename;
        char *extension;
        char *name;
        u32 type;
    };
    enum Site_Asset_Type{
        SiteAsset_None,
        SiteAsset_Generic,
        SiteAsset_Image,
    };
    
    Site_Asset asset_list[] = {
        {"4coder_logo_low_green.png", "png", "4coder_logo", SiteAsset_Image},
        {"screen_1.png",              "png", "screen_1",    SiteAsset_Image},
        {"screen_2.png",              "png", "screen_2",    SiteAsset_Image},
        {"screen_3.png",              "png", "screen_3",    SiteAsset_Image},
        {"4coder_icon.ico",           "ico", "4coder_icon", SiteAsset_Generic},
    };
    
    for (u32 i = 0; i < ArrayCount(asset_list); ++i){
        Site_Asset *asset = &asset_list[i];
        
        str = push_string(256);
        append_sc(&str, asset_directory);
        append_sc(&str, "/");
        append_sc(&str, asset->filename);
        terminate_with_null(&str);
        
        switch (asset_list[i].type){
            case SiteAsset_Generic:
            {
                add_generic_file(&doc_system, str.str, asset->extension, asset->name);
            }break;
            
            case SiteAsset_Image:
            {
                add_image_description(&doc_system, str.str, asset->extension, asset->name);
            }break;
            
            default: InvalidCodePath;
        }
    }
    
    generate_homepage(&doc_system, src_directory);
    generate_4coder_docs(&doc_system, code_directory, src_directory);
    generate_feature_list(&doc_system, src_directory);
    generate_binding_list(&doc_system, src_directory);
    generate_roadmap(&doc_system, src_directory);
    generate_tutorials(&doc_system, src_directory);
    
    for (Basic_Node *node = doc_system.doc_list.head;
         node != 0;
         node = node->next){
        Abstract_Item *doc = NodeGetData(node, Abstract_Item);
        Assert(doc->item_type == ItemType_Document);
        do_html_output(&doc_system, dst_directory, doc);
    }
    
    for (Basic_Node *node = doc_system.file_list.head;
         node != 0;
         node = node->next){
        Abstract_Item *file = NodeGetData(node, Abstract_Item);
        Assert(file->item_type == ItemType_GenericFile);
        
        char *file_name = fm_str(file->name, ".", file->extension);
        fm_copy_file(fm_str(file_name), fm_str(dst_directory, "/", file_name));
    }
    
    for (Basic_Node *node = doc_system.img_list.head;
         node != 0;
         node = node->next){
        Abstract_Item *img = NodeGetData(node, Abstract_Item);
        Assert(img->item_type == ItemType_Image);
        
        for (Basic_Node *node = img->img_instantiations.head;
             node != 0;
             node = node->next){
            Image_Instantiation *inst = NodeGetData(node, Image_Instantiation);
            
            char space[256];
            if (img_get_link_string(img, space, sizeof(space), inst->w, inst->h)){
                char space2[256];
                String str = make_fixed_width_string(space2);
                
                append_sc(&str, dst_directory);
                append_sc(&str, "/");
                append_sc(&str, space);
                terminate_with_null(&str);
                
                do_image_resize(img->source_file, space2, img->extension, inst->w, inst->h);
            }
        }
    }
}

int main(int argc, char **argv){
    META_BEGIN();
    fm_init_system();
    
    if (argc == 5){
        generate_site(argv[1], argv[2], argv[3], argv[4]);
    }
    
    META_FINISH();
}

// BOTTOM

