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
#include "../4coder_lib/4coder_mem.h"
#include "../4cpp/4cpp_lexer.h"

#include "../meta/4ed_meta_parser.cpp"
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

static Alternate_Names_Array
allocate_app_api(Partition *part, i32 count){
    Alternate_Names_Array app_api = {0};
    app_api.names = push_array(part, Alternate_Name, count);
    memset(app_api.names, 0, sizeof(Alternate_Name)*count);
    return(app_api);
}

static void
do_html_output(Document_System *doc_system, Partition *part, char *dst_directory, Abstract_Item *doc){
    // NOTE(allen): Output
    i32 out_size = 10 << 20;
    Tail_Temp_Partition temp = begin_tail_part(part, out_size);
    
    String out = str_alloc(&temp.part, out_size);
    assert(out.str);
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
            fprintf(stderr, "Failed to open %s\n", space);
        }
    }
    
    end_tail_part(temp);
}

static Abstract_Item*
generate_homepage(Document_System *doc_system, Partition *part, char *src_directory){
    Enriched_Text *home = push_struct(part, Enriched_Text);
    *home = load_enriched_text(part, src_directory, "home.txt");
    
    Abstract_Item *doc = begin_document_description(doc_system, "4coder Home", "home", 0);
    add_enriched_text(doc, home);
    end_document_description(doc);
    
    return(doc);
}

// TODO(allen): replace the documentation declaration system with a straight up enriched text system
static Abstract_Item*
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
    
    // NOTE(allen): Parse the code.
    *custom_types_unit = compile_meta_unit(part, code_directory, "4coder_API/types.h", ExpandArray(meta_keywords));
    Assert(custom_types_unit->count != 0);
    
    *lexer_funcs_unit = compile_meta_unit(part, code_directory, "4cpp/4cpp_lexer.h", ExpandArray(meta_keywords));
    Assert(lexer_funcs_unit->count != 0);
    
    *lexer_types_unit = compile_meta_unit(part, code_directory, "4cpp/4cpp_lexer_types.h", ExpandArray(meta_keywords));
    Assert(lexer_types_unit->count != 0);
    
    *string_unit = compile_meta_unit(part, code_directory, "string/internal_4coder_string.cpp", ExpandArray(meta_keywords));
    Assert(string_unit->count != 0);
    
    *custom_funcs_unit = compile_meta_unit(part, code_directory, "4ed_api_implementation.cpp", ExpandArray(meta_keywords));
    Assert(custom_funcs_unit->count != 0);
    
    
    // NOTE(allen): Compute and store variations of the custom function names
    *custom_func_names = allocate_app_api(part, custom_funcs_unit->set.count);
    
    for (i32 i = 0; i < custom_funcs_unit->set.count; ++i){
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
generate_feature_list(Document_System *doc_system, Partition *part, char *src_directory){
    Enriched_Text *feature_list = push_struct(part, Enriched_Text);
    *feature_list = load_enriched_text(part, src_directory, "feature_list.txt");
    
    Abstract_Item *doc = begin_document_description(doc_system, "4coder Feature List", "features", 0);
    add_enriched_text(doc, feature_list);
    end_document_description(doc);
    
    return(doc);
}

static Abstract_Item*
generate_binding_list(Document_System *doc_system, Partition *part, char *src_directory){
    Enriched_Text *binding_list = push_struct(part, Enriched_Text);
    *binding_list = load_enriched_text(part, src_directory, "binding_list.txt");
    
    Abstract_Item *doc = begin_document_description(doc_system, "4coder Binding List", "bindings", 0);
    add_enriched_text(doc, binding_list);
    end_document_description(doc);
    
    return(doc);
}

static Abstract_Item*
generate_roadmap(Document_System *doc_system, Partition *part, char *src_directory){
    Enriched_Text *roadmap = push_struct(part, Enriched_Text);
    *roadmap = load_enriched_text(part, src_directory, "roadmap.txt");
    
    Abstract_Item *doc = begin_document_description(doc_system, "4coder Roadmap", "roadmap", 0);
    add_enriched_text(doc, roadmap);
    end_document_description(doc);
    
    return(doc);
}

static Abstract_Item*
generate_tutorials(Document_System *doc_system, Partition *part, char *src_directory){
    Enriched_Text *roadmap = push_struct(part, Enriched_Text);
    *roadmap = load_enriched_text(part, src_directory, "tutorials.txt");
    
    Abstract_Item *doc = begin_document_description(doc_system, "4coder Tutorials", "tutorials", 0);
    add_enriched_text(doc, roadmap);
    end_document_description(doc);
    
    return(doc);
}

static String
push_string(Partition *part, i32 size){
    String str = {0};
    str.memory_size = size;
    str.str = push_array(part, char, size);
    partition_align(part, 4);
    return(str);
}

static void
do_image_resize(Partition *part, char *src_file, char *dst_file, char *extension, i32 w, i32 h){
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
    i32 size = (512 << 20);
    void *mem = malloc(size);
    memset(mem, 0, size);
    
    Partition part_ = make_part(mem, size);
    Partition *part = &part_;
    String str;
    
    Document_System doc_system = create_document_system(part);
    
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
        
        str = push_string(part, 256);
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
    
    generate_homepage(&doc_system, part, src_directory);
    generate_4coder_docs(&doc_system, part, code_directory, src_directory);
    generate_feature_list(&doc_system, part, src_directory);
    generate_binding_list(&doc_system, part, src_directory);
    generate_roadmap(&doc_system, part, src_directory);
    generate_tutorials(&doc_system, part, src_directory);
    
    for (Basic_Node *node = doc_system.doc_list.head;
         node != 0;
         node = node->next){
        Abstract_Item *doc = NodeGetData(node, Abstract_Item);
        assert(doc->item_type == ItemType_Document);
        do_html_output(&doc_system, part, dst_directory, doc);
    }
    
    for (Basic_Node *node = doc_system.file_list.head;
         node != 0;
         node = node->next){
        Abstract_Item *file = NodeGetData(node, Abstract_Item);
        assert(file->item_type == ItemType_GenericFile);
        
        char space[256];
        String str = make_fixed_width_string(space);
        append_sc(&str, file->name);
        append_sc(&str, ".");
        append_sc(&str, file->extension);
        terminate_with_null(&str);
        
        do_file_copy(part, file->source_file, dst_directory, space);
    }
    
    for (Basic_Node *node = doc_system.img_list.head;
         node != 0;
         node = node->next){
        Abstract_Item *img = NodeGetData(node, Abstract_Item);
        assert(img->item_type == ItemType_Image);
        
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
                
                do_image_resize(part, img->source_file, space2, img->extension, inst->w, inst->h);
            }
        }
    }
}

int main(int argc, char **argv){
    META_BEGIN();
    
    if (argc == 5){
        generate_site(argv[1], argv[2], argv[3], argv[4]);
    }
    
    META_FINISH();
}

// BOTTOM

