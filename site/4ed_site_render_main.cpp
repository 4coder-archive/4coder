/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 12.12.2019
 *
 * Render website static pages
 *
 */

// TOP

#include "4coder_base_types.h"
#include "4coder_events.h"
#include "4coder_table.h"
#include "../4ed_api_definition.h"
#include "4coder_doc_content_types.h"
#include "../docs/4ed_doc_helper.h"
#include "4coder_command_map.h"

#include "generated/command_metadata.h"

#include "4coder_base_types.cpp"
#include "4coder_stringf.cpp"
#include "4coder_hash_functions.cpp"
#include "4coder_table.cpp"
#include "4coder_events.cpp"
#include "4coder_malloc_allocator.cpp"
#include "../4ed_api_definition.cpp"
#include "4coder_doc_content_types.cpp"
#include "../docs/4ed_doc_helper.cpp"
#include "4coder_file.cpp"

#define MAP_METADATA_ONLY 1
#include "4coder_command_map.cpp"
#include "4coder_default_map.cpp"
#include "4coder_mac_map.cpp"
#include "generated/custom_api_constructor.cpp"
#include "../docs/4ed_doc_custom_api.cpp"
#include "4coder_doc_commands.cpp"

#include <stdio.h>

////////////////////////////////

char html_header[] = R"HTMLFOO(
<html lang="en-US">

<head>
<meta charset="UTF-8">
<link rel='shortcut icon' type='image/x-icon' href='https://4coder.net/4coder_icon.ico' />
<link href="https://fonts.googleapis.com/css?family=Inconsolata:700&display=swap" rel="stylesheet">
<script src="search.js"></script> 
<title>%.*s</title>
<link rel = "stylesheet" type="text/css" href="styles.css" />
</head>

<body>
)HTMLFOO";

char html_footer[] = R"HTMLFOO(
<div class="bottom_spacer"></div>
    </body>
    </html>
)HTMLFOO";

function void
render_doc_page_to_html__content_list(Arena *scratch, Doc_Content_List *list, FILE *out){
    for (Doc_Content *content = list->first;
         content != 0;
         content = content->next){
        switch (content->emphasis){
            case DocContentEmphasis_Normal:
            {
                // do nothing
            }break;
            
            case DocContentEmphasis_SmallHeader:
            {
                fprintf(out, "<h3>");
            }break;
            
            case DocContentEmphasis_Heavy:
            {
                fprintf(out, "<span class=\"emphasize\">");
            }break;
            
            case DocContentEmphasis_Stylish:
            {
                fprintf(out, "<span class=\"comment\">");
            }break;
            
            case DocContentEmphasis_Code:
            {
                fprintf(out, "<code><pre>");
            }break;
        }
        
        b32 close_link = false;
        if (content->page_link.size > 0){
            fprintf(out, "<a href=\"%.*s.html\">", string_expand(content->page_link));
        }
        else if (content->block_link.size > 0){
            fprintf(out, "<a href=\"#%.*s\">", string_expand(content->block_link));
        }
        fprintf(out, "%.*s", string_expand(content->text));
        if (close_link){
            fprintf(out, "</a>");
        }
        
        switch (content->emphasis){
            case DocContentEmphasis_Normal:
            {
                // do nothing
            }break;
            
            case DocContentEmphasis_SmallHeader:
            {
                fprintf(out, "</h3>");
            }break;
            
            case DocContentEmphasis_Heavy:
            case DocContentEmphasis_Stylish:
            {
                fprintf(out, "</span>");
            }break;
            
            case DocContentEmphasis_Code:
            {
                fprintf(out, "</code></pre>");
            }break;
        }
        fprintf(out, " ");
    }
}

function void
render_doc_page_to_html__content(Arena *scratch, Doc_Content_List *list, FILE *out){
    fprintf(out, "<div class=\"normal\">");
    render_doc_page_to_html__content_list(scratch, list, out);
    fprintf(out, "</div>\n");
}

function void
render_doc_page_to_html__code(Arena *scratch, Doc_Code_Sample_List *code, FILE *out){
    for (Doc_Code_Sample *node = code->first;
         node != 0;
         node = node->next){
        fprintf(out, "<div class=\"comment\">%s</div>", doc_language_name[node->language]);
        fprintf(out, "<div class=\"very_small_spacer\"></div>\n");
        fprintf(out, "<div class=\"sample_code\"><div>"
                "<code><pre>%.*s</pre></code>"
                "</div></div>",
                string_expand(node->contents));
        if (node->next != 0){
            fprintf(out, "<div class=\"small_spacer\"></div>\n");
        }
    }
}

function void
render_doc_page_to_html__table(Arena *scratch, Vec2_i32 dim, Doc_Content_List *vals, FILE *out){
    fprintf(out, "<table class=\"normal\">");
    for (i32 y = 0; y < dim.y; y += 1){
        fprintf(out, "<tr>");
        Doc_Content_List *line = &vals[y*dim.x];
        for (i32 x = 0; x < dim.x; x += 1){
            Doc_Content_List *cont = &line[x];
            fprintf(out, "<td>");
            render_doc_page_to_html__content_list(scratch, cont, out);
            fprintf(out, "</td>");
        }
        fprintf(out, "</tr>");
    }
    fprintf(out, "</table>");
}

function void
render_doc_page_to_html_no_header(Arena *scratch, Doc_Page *page, FILE *file){
    
    Temp_Memory_Block temp(scratch);
    Doc_Cluster *cluster = page->owner;
    
    fprintf(file, "<div class=\"small_spacer\"></div>\n");
    
    fprintf(file, "<div class=\"small\">&gt; <a href=\"https://4coder.net\">home</a> "
            "&gt; <a href=\"%.*s.html\">%.*s</a></div>\n",
            string_expand(cluster->name),
            string_expand(cluster->title));
    
    fprintf(file, "<div class=\"small_spacer\"></div>\n");
    
    fprintf(file, "<h1>%.*s</h1>\n", string_expand(page->name));
    
    fprintf(file, "<div class=\"small\">");
    fprintf(file, "%s %s %d",
            doc_month_names[cluster->gen_date.month],
            doc_day_names[cluster->gen_date.day],
            cluster->gen_date.year);
    fprintf(file, "</div>\n");
    
    fprintf(file, "<div class=\"spacer\"></div>\n");
    
    for (Doc_Block *block = page->first_block;
         block != 0;
         block = block->next){
        if (block->name.size > 0 &&
            !string_match(block->name, string_u8_litexpr("brief"))){
            fprintf(file, "<h2>%.*s</h2>\n", string_expand(block->name));
        }
        
        for (Doc_Paragraph *par = block->first_par;
             par != 0;
             par = par->next){
            switch (par->kind){
                case DocParagraphKind_Text:
                {
                    render_doc_page_to_html__content(scratch, &par->text, file);
                }break;
                
                case DocParagraphKind_Code:
                {
                    render_doc_page_to_html__code(scratch, &par->code, file);
                }break;
                
                case DocParagraphKind_Table:
                {
                    render_doc_page_to_html__table(scratch, par->table.dim, par->table.vals, file);
                }break;
            }
            
            if (par->next != 0){
                fprintf(file, "<div class=\"small_spacer\"></div>\n");
            }
        }
        
        if (block->next != 0){
            fprintf(file, "<div class=\"spacer\"></div>\n");
        }
    }
    
}

function void
render_doc_page_to_html(Arena *scratch, Doc_Page *page, FILE *file){
    fprintf(file, html_header, string_expand(page->title));
    fprintf(file, "<div class=\"api_page\">\n");
    render_doc_page_to_html_no_header(scratch, page, file);
    fprintf(file, "</div>\n");
    fprintf(file, html_footer);
}

function void
render_doc_page_to_html(Arena *scratch, Doc_Page *page, String_Const_u8 docs_root){
    Temp_Memory_Block temp(scratch);
    String_Const_u8 file_name = push_u8_stringf(scratch, "%.*s%.*s.html",
                                                string_expand(docs_root),
                                                string_expand(page->name));
    
    FILE *file = fopen((char*)file_name.str, "wb");
    if (file == 0){
        printf("could not open %s\n", file_name.str);
        return;
    }
    
    render_doc_page_to_html(scratch, page, file);
    fclose(file);
}

function void
sort_doc_page_array(Doc_Page **ptrs, i32 first, i32 one_past_last){
    if (first + 1 < one_past_last){
        i32 pivot_index = one_past_last - 1;
        String_Const_u8 pivot_name = ptrs[pivot_index]->name;
        i32 j = first;
        for (i32 i = first; i < pivot_index; i += 1){
            String_Const_u8 name = ptrs[i]->name;
            if (string_compare(name, pivot_name) < 0){
                Swap(Doc_Page*, ptrs[j], ptrs[i]);
                j += 1;
            }
        }
        Swap(Doc_Page*, ptrs[j], ptrs[pivot_index]);
        sort_doc_page_array(ptrs, first, j);
        sort_doc_page_array(ptrs, j + 1, one_past_last);
    }
}

function void
render_doc_cluster_to_html(Arena *scratch, Doc_Cluster *cluster,
                           FILE *file, FILE *file_index){
    {
        Temp_Memory_Block temp(scratch);
        
        fprintf(file, html_header, string_expand(cluster->title));
        fprintf(file, "<div class=\"api_page\">\n");
        
        fprintf(file, "<div class=\"small_spacer\"></div>\n");
        fprintf(file, "<h1>%.*s</h1>\n", string_expand(cluster->title));
        
        fprintf(file, "<div class=\"small_spacer\"></div>\n");
        fprintf(file, "<div class=\"small\">&gt; <a href=\"https://4coder.net\">home</a></div>\n");
        
        fprintf(file, "<div class=\"spacer\"></div>\n");
        
        // TODO(allen): Cluster description.
        fprintf(file, "<div class=\"normal\">More documentation under construction</div>\n");
        
        fprintf(file, "<div class=\"spacer\"></div>\n");
        
        fprintf(file, "<h2><a href=\"%.*s_index.html\">Index</a></h2>\n",
                string_expand(cluster->name));
        
        fprintf(file, "</div>\n");
        fprintf(file, html_footer);
    }
    
    {
        Temp_Memory_Block temp(scratch);
        
        fprintf(file_index, html_header, string_expand(cluster->title));
        
        Doc_Page **ptrs = push_array(scratch, Doc_Page*, cluster->page_count);
        i32 counter = 0;
        for (Doc_Page *node = cluster->first_page;
             node != 0;
             node = node->next){
            ptrs[counter] = node;
            counter += 1;
        }
        sort_doc_page_array(ptrs, 0, counter);
        
        // NOTE(rjf): List of API pages, with filter box.
        {
            fprintf(file_index, "<div class=\"sidebar\">\n");
            
            fprintf(file_index, "<div class=\"small_spacer\"></div>\n");
            fprintf(file_index, "<div class=\"small\">&gt; <a href=\"https://4coder.net\">home</a></div>\n");
            
            fprintf(file_index, "<div class=\"small_spacer\"></div>\n");
            fprintf(file_index, "<h1>%.*s Index</h1>\n", string_expand(cluster->title));
            fprintf(file_index, "<div class=\"spacer\"></div>\n");
            
            fprintf(file_index, "<input autofocus class=\"filter_box\" type=\"text\" id=\"search_input\" oninput=\"SearchInput(event)\" onkeydown=\"SearchKeyDown(event)\""
                    "placeholder=\"Filter...\" title=\"Filter...\">");
            fprintf(file_index, "<div class=\"spacer\"></div>\n");
            
            fprintf(file_index, "<div class=\"normal\">");
            
            fprintf(file_index, "<ul class=\"docs_menu\" id=\"docs_menu\">\n");
            for (i32 i = 0; i < counter; i += 1){
                Doc_Page *node = ptrs[i];
                fprintf(file_index, "<li><a href=\"#%.*s\">%.*s</a></li>",
                        string_expand(node->name),
                        string_expand(node->name));
            }
            fprintf(file_index, "</ul>\n");
            fprintf(file_index, "</div>\n");
            
            fprintf(file_index, "</div>\n");
        }
        
        // NOTE(rjf): API pages.
        {
            for (i32 i = 0; i < counter; i += 1){
                Doc_Page *node = ptrs[i];
                fprintf(file_index, "<div id=\"%.*s\" class=\"api_page_preview hidden\">\n",
                        string_expand(node->name));
                render_doc_page_to_html_no_header(scratch, node, file_index);
                
                fprintf(file_index, "<div class=\"spacer\"></div>\n");
                fprintf(file_index, "<div class=\"small\"><a href=\"%.*s.html\">Standalone Page</a></div>\n",
                        string_expand(node->name));
                
                fprintf(file_index, "</div>");
            }
        }
        
        fprintf(file_index, html_footer);
    }
}

function void
render_doc_cluster_to_html(Arena *scratch, Doc_Cluster *cluster, String_Const_u8 docs_root){
    Temp_Memory_Block temp(scratch);
    String_Const_u8 file_name = push_u8_stringf(scratch, "%.*s%.*s.html",
                                                string_expand(docs_root),
                                                string_expand(cluster->name));
    
    String_Const_u8 indx_name = push_u8_stringf(scratch, "%.*s%.*s_index.html",
                                                string_expand(docs_root),
                                                string_expand(cluster->name));
    
    FILE *file = fopen((char*)file_name.str, "wb");
    if (file == 0){
        printf("could not open %s\n", file_name.str);
        return;
    }
    
    FILE *file_index = fopen((char*)indx_name.str, "wb");
    if (file_index == 0){
        printf("could not open %s\n", indx_name.str);
        return;
    }
    
    render_doc_cluster_to_html(scratch, cluster, file, file_index);
    printf("%s:1:1\n", indx_name.str);
    
    fclose(file);
}

////////////////////////////////

int main(){
    Arena arena = make_arena_malloc();
    
    Thread_Context tctx_ = {};
    thread_ctx_init(&tctx_, ThreadKind_Main, get_allocator_malloc(), get_allocator_malloc());
    Thread_Context *tctx = &tctx_;
    
    String_Const_u8 self = string_u8_litexpr(__FILE__);
    u64 code_pos = string_find_first(self, string_u8_litexpr("code"));
    String_Const_u8 root = string_prefix(self, code_pos + 5);
    String_Const_u8 outside_root = string_prefix(self, code_pos);
    String_Const_u8 build_root = push_u8_stringf(&arena, "%.*sbuild/",
                                                 string_expand(outside_root));
    String_Const_u8 site_root = push_u8_stringf(&arena, "%.*ssite/",
                                                string_expand(build_root));
    String_Const_u8 docs_root = push_u8_stringf(&arena, "%.*sdocs/",
                                                string_expand(site_root));
    
    (void)root;
    
    i64 global_id = 1;
    i64 file_id = 2;
    i64 code_id = 3;
    
    local_const i32 map_count = 2;
    Mapping mapping_array[map_count] = {};
    char *page_tiles[map_count] = {};
    char *page_names[map_count] = {};
    
    mapping_init(tctx, &mapping_array[0]);
    setup_default_mapping(&mapping_array[0], global_id, file_id, code_id);
    page_tiles[0] = "Default Bindings";
    page_names[0] = "default_bindings";
    
    mapping_init(tctx, &mapping_array[1]);
    setup_mac_mapping(&mapping_array[1], global_id, file_id, code_id);
    page_tiles[1] = "Default Mac Bindings";
    page_names[1] = "default_mac_bindings";
    
    API_Definition *api_def = custom_api_construct(&arena);
    Doc_Cluster *cluster_array[] = {
        doc_custom_api(&arena, api_def),
        doc_commands(&arena),
        doc_default_bindings(&arena, map_count, mapping_array, page_tiles, page_names,
                             global_id, file_id, code_id),
    };
    
    for (i32 i = 0; i < ArrayCount(cluster_array); i += 1){
        Doc_Cluster *cluster = cluster_array[i];
        for (Doc_Page *node = cluster->first_page;
             node != 0;
             node = node->next){
            render_doc_page_to_html(&arena, node, docs_root);
        }
        render_doc_cluster_to_html(&arena, cluster, docs_root);
    }
    
    return(0);
}

// BOTTOM

