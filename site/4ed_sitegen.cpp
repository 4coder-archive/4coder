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

#include "../4coder_API/4coder_version.h"
#include "4coder_lib/4coder_arena.h"
#include "4coder_lib/4coder_arena.cpp"
#define FSTRING_IMPLEMENTATION
#include "../4coder_lib/4coder_string.h"
#include "../4coder_lib/4cpp_lexer.h"

#define FTECH_FILE_MOVING_IMPLEMENTATION
#include "../meta/4ed_file_moving.h"
#include "../meta/4ed_meta_parser.cpp"
#include "../meta/4ed_meta_keywords.h"
#include "4ed_abstract_document.cpp"

#include "../4coder_generated/command_metadata.h"
#include "../4coder_generated/remapping.h"
#include "../4coder_API/4coder_keycodes.h"

///////////////////////////////////////////////////////////////////////////

internal void
copy_and_fix_name(char *src, char *dst, int32_t cap){
    String s = make_string_cap(dst, 0, cap);
    copy(&s, src);
    replace_char(&s, '_', '-');
    terminate_with_null(&s);
}

enum{
    MDFR_NONE  = 0x0,
    MDFR_CTRL  = 0x1,
    MDFR_ALT   = 0x2,
    MDFR_CMND  = 0x4,
    MDFR_SHIFT = 0x8,
};

internal void
generate_binding_list(char *code_directory, char *src_directory){
    char full_path[512];
    String s = make_fixed_width_string(full_path);
    append(&s, src_directory);
    if (s.size == 0 || !char_is_slash(s.str[s.size - 1])){
        append(&s, "/");
    }
    append(&s, "binding_list.txt");
    terminate_with_null(&s);
    
    FILE *out = fopen(full_path, "wb");
    if (out == 0){
        fprintf(stdout, "could not open binding_list.txt for auto synchronized binding info\n");
        return;
    }
    
    fprintf(out, "\n\\INCLUDE{site_header.txt}\n");
    fprintf(out, "\n4coder version \\VERSION\n");
    
    fprintf(out, "\n\\SECTION{Built in Bindings}\n");
    for (i32 i = 0; i < ArrayCount(fcoder_meta_maps); ++i){
        Meta_Mapping *mapping = &fcoder_meta_maps[i];
        
        char s[512];
        copy_and_fix_name(mapping->name, s, sizeof(s));
        fprintf(out, "\\ITEM \\STYLE{code} \"%s\" \\END %s\n", s, mapping->description);
    }
    fprintf(out, "\\END\n");
    
    for (i32 i = 0; i < ArrayCount(fcoder_meta_maps); ++i){
        Meta_Mapping *mapping = &fcoder_meta_maps[i];
        
        char s[512];
        copy_and_fix_name(mapping->name, s, sizeof(s));
        fprintf(out, "\\SECTION{Map: %s}\n", s);
        
        for (i32 j = 0; j < mapping->sub_map_count; ++j){
            Meta_Sub_Map *sub_map = &mapping->sub_maps[j];
            
            char sub_s[512];
            copy_and_fix_name(sub_map->name, sub_s, sizeof(sub_s));
            fprintf(out, "\\SECTION{%s}\n", sub_s);
            fprintf(out, "%s\n", sub_map->description);
            fprintf(out, "\\LIST\n", sub_map->description);
            
            for (i32 k = 0; k < sub_map->bind_count; ++k){
                Meta_Key_Bind *bind = &sub_map->binds[k];
                
                // Get modifier key string
                char mdfr_str[256];
                String m = make_fixed_width_string(mdfr_str);
                b32 has_base = false;
                
                if (bind->modifiers & MDFR_CTRL){
                    if (has_base){
                        append(&m, "+");
                    }
                    append(&m, "ctrl");
                }
                if (bind->modifiers & MDFR_ALT){
                    if (has_base){
                        append(&m, "+");
                    }
                    append(&m, "alt");
                }
                if (bind->modifiers & MDFR_CMND){
                    if (has_base){
                        append(&m, "+");
                    }
                    append(&m, "cmnd");
                }
                if (bind->modifiers & MDFR_SHIFT){
                    if (has_base){
                        append(&m, "+");
                    }
                    append(&m, "shift");
                }
                if (bind->modifiers != 0){
                    append(&m, " ");
                }
                terminate_with_null(&m);
                
                // Get printable key string
                char key_str_space[2];
                char *key_str = 0;
                
                if (bind->vanilla){
                    key_str = "any character";
                }
                else{
                    switch (bind->keycode){
                        case key_back: key_str = "backspace"; break;
                        case key_up: key_str = "up"; break; 
                        case key_down: key_str = "down"; break; 
                        case key_left: key_str = "left"; break; 
                        case key_right: key_str = "right"; break; 
                        case key_del: key_str = "delete"; break; 
                        case key_insert: key_str = "insert"; break; 
                        case key_home: key_str = "home"; break; 
                        case key_end: key_str = "end"; break; 
                        case key_page_up: key_str = "page up"; break; 
                        case key_page_down: key_str = "page down"; break; 
                        case key_esc: key_str = "escape"; break; 
                        case key_mouse_left: key_str = "left click"; break; 
                        case key_mouse_right: key_str = "right click"; break; 
                        case key_mouse_left_release: key_str = "left release"; break; 
                        case key_mouse_right_release: key_str = "right release"; break; 
                        case key_mouse_wheel: key_str = "mouse wheel"; break;
                        case key_mouse_move: key_str = "mouse move"; break;
                        case key_animate: key_str = "animate"; break;
                        case key_click_activate_view: key_str = "click_activate_view"; break;
                        case key_click_deactivate_view: key_str = "click_deactivate_view"; break;
                        case key_f1: key_str = "f1"; break; 
                        case key_f2: key_str = "f2"; break; 
                        case key_f3: key_str = "f3"; break; 
                        case key_f4: key_str = "f4"; break; 
                        case key_f5: key_str = "f5"; break; 
                        case key_f6: key_str = "f6"; break; 
                        case key_f7: key_str = "f7"; break; 
                        case key_f8: key_str = "f8"; break; 
                        case key_f9: key_str = "f9"; break; 
                        case key_f10: key_str = "f10"; break; 
                        case key_f11: key_str = "f11"; break; 
                        case key_f12: key_str = "f12"; break; 
                        case key_f13: key_str = "f13"; break; 
                        case key_f14: key_str = "f14"; break; 
                        case key_f15: key_str = "f15"; break; 
                        case key_f16: key_str = "f16"; break;
                        
                        default:
                        {
                            if (bind->keycode == '\n'){
                                key_str = "return";
                            }
                            else if (bind->keycode == '\t'){
                                key_str = "tab";
                            }
                            else if (bind->keycode == ' '){
                                key_str = "space";
                            }
                            else{
                                Assert(bind->keycode <= 127);
                                key_str = key_str_space;
                                key_str_space[0] = (char)bind->keycode;
                                key_str_space[1] = 0;
                            }
                        }break;
                    }
                }
                
                // Get description from doc string
                char *description = "description missing";
                char *command = bind->command;
                for (i32 i = 0; i < command_one_past_last_id; ++i){
                    Command_Metadata *metadata = command_metadata_by_id(i);
                    if (match(metadata->name, command)){
                        description = metadata->description;
                        break;
                    }
                }
                
                fprintf(out, "\\ITEM \\STYLE{code} <%s%s> \\END %s\n",
                        mdfr_str, key_str,
                        description);
            }
            
            fprintf(out, "\\END\n");
            fprintf(out, "\\END\n");
        }
        
        fprintf(out, "\\END\n");
    }
    
    fclose(out);
}

//
// Meta Parse Rules
//

internal Abstract_Item*
generate_page(Partition *arena, Document_System *doc_system, char *source_text, char *big_title, char *small_name){
    Enriched_Text *home = push_array(arena, Enriched_Text, 1);
    *home = load_enriched_text(arena, doc_system->src_dir, source_text);
    
    Abstract_Item *doc = make_document_from_text(arena, doc_system, big_title, small_name, home);
    if (doc == 0){
        fprintf(stdout, "warning: could not create document %s from file %s\n", small_name, source_text);
    }
    
    return(doc);
}

internal void
do_image_resize(Partition *arena, char *src_file, char *dst_file, char *extension, i32 w, i32 h){
    Temp_Memory temp = begin_temp_memory(arena);
    
    i32 x = 0, y = 0, channels = 0;
    stbi_uc *image = stbi_load(src_file, &x, &y, &channels, 0);
    if (image != 0){
        stbi_uc *resized_image = push_array(arena, stbi_uc, w*h*channels);
        stbir_resize_uint8(image, x, y, x*channels, resized_image, w, h, w*channels, channels);
        if (match_cc(extension, "png")){
            stbi_write_png(dst_file, w, h, channels, resized_image, w*channels);
        }
        free(image);
    }
    
    end_temp_memory(temp);
}

internal void
generate_site(Partition *arena, char *code_directory, char *asset_directory, char *src_directory, char *dst_directory){
    fm_clear_folder(dst_directory);
    
    Document_System doc_system = create_document_system(code_directory, asset_directory, src_directory);
    Document_System *docs = &doc_system;
    
    // TODO(allen): Declare these in the source files.
    add_image_description(arena, docs, "4coder_logo_low_green.png", "png", "4coder_logo" );
    add_image_description(arena, docs, "screen_1.png",              "png", "screen_1"    );
    add_image_description(arena, docs, "screen_2.png",              "png", "screen_2"    );
    add_image_description(arena, docs, "screen_3.png",              "png", "screen_3"    );
    add_generic_file     (arena, docs, "4coder_icon.ico",           "ico", "4coder_icon" );
    
    // TODO(allen): From the text file get  "Big Title" and        "smallname".
    generate_page(arena, docs, "docs.txt"        , "4coder API Docs"    , "custom_docs" );
    generate_page(arena, docs, "home.txt"        , "4coder Home"        , "home"        );
    generate_page(arena, docs, "feature_list.txt", "4coder Feature List", "features"    );
    generate_page(arena, docs, "binding_list.txt", "4coder Binding List", "bindings"    );
    generate_page(arena, docs, "roadmap.txt"     , "4coder Roadmap"     , "roadmap"     );
    generate_page(arena, docs, "tutorials.txt"   , "4coder Tutorials"   , "tutorials"   );
    
    // NOTE(allen): Create a list of the primary documents to generate.
    Abstract_Item_Array original_documents = get_abstract_item_array(arena, &doc_system.doc_list);
    
    // NOTE(allen): Cross link all the includes and pull in any non-primary documents.
    resolve_all_includes(arena, &doc_system);
    
    // NOTE(allen): Generate the html from the primary documents and publish them.
    String out = make_string_cap(push_array(arena, char, 10 << 20), 0, 10 << 20);
    Assert(out.str != 0);
    
    Abstract_Item **doc_ptr = original_documents.items;
    for (u32 j = 0; j < original_documents.count; ++j, ++doc_ptr){
        Abstract_Item *doc = *doc_ptr;
        Assert(doc->item_type == ItemType_Document);
        
        char doc_link[256];
        if (doc_get_link_string(doc, doc_link, sizeof(doc_link))){
            generate_document_html(arena, &out, &doc_system, doc);
            
            char *name = fm_str(arena, dst_directory, "/", doc_link);
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
        char *file_name = fm_str(arena, file->name, ".", file->extension);
        fm_copy_file(fm_str(arena, file_name), fm_str(arena, dst_directory, "/", file_name));
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
                char *dest_file = fm_str(arena, dst_directory, "/", img_link);
                do_image_resize(arena, img->source_file, dest_file, img->extension, inst->w, inst->h);
            }
        }
    }
}

int main(int argc, char **argv){
    META_BEGIN();
    
    if (argc == 5){
        Partition arena = fm_init_system();
        
        char *code_directory = argv[1];
        char *asset_directory = argv[2];
        char *src_directory = argv[3];
        char *dst_directory = argv[4];
        generate_binding_list(code_directory, src_directory);
        generate_site(&arena, code_directory, asset_directory, src_directory, dst_directory);
    }
    
    META_FINISH();
}

// BOTTOM

