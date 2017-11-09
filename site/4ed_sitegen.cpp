/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 25.02.2016
 *
 * Site generator for 4coder.
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
#include "4ed_abstract_document.cpp"

///////////////////////////////////////////////////////////////////////////

//
// Meta Parse Rules
//

internal Abstract_Item*
generate_page(Document_System *doc_system, char *source_text, char *big_title, char *small_name){
    Enriched_Text *home = fm_push_array(Enriched_Text, 1);
    *home = load_enriched_text(doc_system->src_dir, source_text);
    
    Abstract_Item *doc = make_document_from_text(doc_system, big_title, small_name, home);
    if (doc == 0){
        fprintf(stdout, "warning: could not create document %s from file %s\n", small_name, source_text);
    }
    
    return(doc);
}

internal void
do_image_resize(char *src_file, char *dst_file, char *extension, i32 w, i32 h){
    Temp temp = fm_begin_temp();
    
    i32 x = 0, y = 0, channels = 0;
    stbi_uc *image = stbi_load(src_file, &x, &y, &channels, 0);
    if (image != 0){
        stbi_uc *resized_image = fm_push_array(stbi_uc, w*h*channels);
        stbir_resize_uint8(image, x, y, x*channels, resized_image, w, h, w*channels, channels);
        if (match_cc(extension, "png")){
            stbi_write_png(dst_file, w, h, channels, resized_image, w*channels);
        }
        free(image);
    }
    
    fm_end_temp(temp);
}

internal void
generate_site(char *code_directory, char *asset_directory, char *src_directory, char *dst_directory){
    fm_clear_folder(dst_directory);
    
    Document_System doc_system = create_document_system(code_directory, asset_directory, src_directory);
    Document_System *docs = &doc_system;
    
    // TODO(allen): Declare these in the source files.
    add_image_description(docs, "4coder_logo_low_green.png", "png", "4coder_logo" );
    add_image_description(docs, "screen_1.png",              "png", "screen_1"    );
    add_image_description(docs, "screen_2.png",              "png", "screen_2"    );
    add_image_description(docs, "screen_3.png",              "png", "screen_3"    );
    add_generic_file     (docs, "4coder_icon.ico",           "ico", "4coder_icon" );
    
    // TODO(allen): From the text file get  "Big Title" and        "smallname".
    generate_page(docs, "docs.txt"        , "4coder API Docs"    , "custom_docs" );
    generate_page(docs, "home.txt"        , "4coder Home"        , "home"        );
    generate_page(docs, "feature_list.txt", "4coder Feature List", "features"    );
    generate_page(docs, "binding_list.txt", "4coder Binding List", "bindings"    );
    generate_page(docs, "roadmap.txt"     , "4coder Roadmap"     , "roadmap"     );
    generate_page(docs, "tutorials.txt"   , "4coder Tutorials"   , "tutorials"   );
    
    // NOTE(allen): Create a list of the primary documents to generate.
    Abstract_Item_Array original_documents = get_abstract_item_array(&doc_system.doc_list);
    
    // NOTE(allen): Cross link all the includes and pull in any non-primary documents.
    resolve_all_includes(&doc_system);
    
    // NOTE(allen): Generate the html from the primary documents and publish them.
    String out = make_string_cap(fm__push(10 << 20), 0, 10 << 20);
    Assert(out.str != 0);
    
    Abstract_Item **doc_ptr = original_documents.items;
    for (u32 j = 0; j < original_documents.count; ++j, ++doc_ptr){
        Abstract_Item *doc = *doc_ptr;
        Assert(doc->item_type == ItemType_Document);
        
        char doc_link[256];
        if (doc_get_link_string(doc, doc_link, sizeof(doc_link))){
            generate_document_html(&out, &doc_system, doc);
            
            char *name = fm_str(dst_directory, "/", doc_link);
            fm_write_file(name, out.str, out.size);
            out.size = 0;
        }
    }
    
    // NOTE(allen): Publish files
    for (Basic_Node *node = doc_system.file_list.head;
         node != 0;
         node = node->next){
        Abstract_Item *file = NodeGetData(node, Abstract_Item);
        Assert(file->item_type == ItemType_GenericFile);
        char *file_name = fm_str(file->name, ".", file->extension);
        fm_copy_file(fm_str(file_name), fm_str(dst_directory, "/", file_name));
    }
    
    // NOTE(allen): Publish images
    for (Basic_Node *node = doc_system.img_list.head;
         node != 0;
         node = node->next){
        Abstract_Item *img = NodeGetData(node, Abstract_Item);
        Assert(img->item_type == ItemType_Image);
        
        for (Basic_Node *node = img->img_instantiations.head;
             node != 0;
             node = node->next){
            Image_Instantiation *inst = NodeGetData(node, Image_Instantiation);
            
            char img_link[256];
            if (img_get_link_string(img, img_link, sizeof(img_link), inst->w, inst->h)){
                char *dest_file = fm_str(dst_directory, "/", img_link);
                do_image_resize(img->source_file, dest_file, img->extension, inst->w, inst->h);
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

