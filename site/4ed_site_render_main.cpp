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
#include "../4ed_api_definition.h"
#include "4coder_doc_content_types.h"
#include "../docs/4ed_doc_helper.h"

#include "generated/command_metadata.h"

#include "4coder_base_types.cpp"
#include "4coder_stringf.cpp"
#include "4coder_malloc_allocator.cpp"
#include "../4ed_api_definition.cpp"
#include "4coder_doc_content_types.cpp"
#include "../docs/4ed_doc_helper.cpp"
#include "4coder_file.cpp"

#include "generated/custom_api_constructor.cpp"
#include "../docs/4ed_doc_custom_api.cpp"
#include "4coder_doc_commands.cpp"

#include <stdio.h>

////////////////////////////////

char html_header[] = R"HTMLFOO(
<html lang="en-US">

<head>
<link rel='shortcut icon' type='image/x-icon' href='4coder_icon.ico' />
<link href="https://fonts.googleapis.com/css?family=Inconsolata:700&display=swap" rel="stylesheet">
<script src="search.js"></script> 
<title>%.*s</title>
<style>
body {
  font-family: 'Inconsolata', monospace;
  background: #0C0C0C;
  color: #FF00FF;
}

h1 {
  margin-top: 0;
  margin-bottom: .4em;
font-size: 1.9em;
  color: #00A000;
}

h2 {
  margin-top: 0;
  margin-bottom: .666em;
  font-size: 1.5em;
  color: #00A000;
}

h3 {
  margin-top: 0;
  margin-bottom: 0;
  font-size: 1.17em;
  color: #90B080;
background: #001300;
padding: 0 .25em 0 .25em;
display: inline;
}

ul {
  margin-top: 0;
  margin-bottom: 0;
}

li {
  margin-top: 5px;
}

li.firstli {
  margin-top: 0px;
}

pre {
margin: 0;
}

.root {
  width: 50%%;
  margin: auto;
}

@media only screen and (max-width: 1000px) {
  .root {
    width: 70%%;
  }
}
@media only screen and (max-width: 800px) {
  .root {
    width: 90%%;
  }
}

.center {
  text-align: center;
}

.normal {
  font-size: 1em;
  color: #90B080;
  text-align: justify;
}

.normal li {
  text-align: left;
}

.small {
  font-size: .8em;
  color: #90B080;
  text-align: justify;
}

.small li {
  text-align: left;
}

.normal a:link, .small a:link,
h1 a:link, h2 a:link, a:link {
  color: #D08F20;
}
.normal a:visited, .small a:visited,
h1 a:visited, h2 a:visited, a:visited {
  color: #744F14;
}
.normal a:hover, .small a:hover,
h1 a:hover, h2 a:hover, a:hover {
  color: #E0AF60;
}

.sample_code {
font-size: 1em;
    color: #90B080;
padding: 3px;
background: #323232;
}

.sample_code div{
padding: .5em;
background: #181818;
}

.comment {
  margin-left: 14px;
  margin-right: 14px;
  font-size: 1.25em;
  color: #2090F0;
}

.emphasize {
  color: #00A000;
}

.spacer {
  height: 2.5em;
}

.small_spacer {
  height: 1em;
}

.very_small_spacer {
  height: .5em;
}

.bottom_spacer {
  height: 20em;
}

.docs_menu {
  list-style-type:none;
  padding: 0;
}

.filter_box {
  border: 1px solid #90B080;
  font-size: 1.5em;
  color: #90B080;
  background: none;
  text-align: justify;
  font-family: 'Inconsolata', monospace;
  width: 15em;
}

</style>

</head>

<body>
<div class="root">
)HTMLFOO";

char html_footer[] = R"HTMLFOO(
<div class="bottom_spacer"></div>
    </div>
    </body>
    </html>
)HTMLFOO";

function void
render_doc_page_to_html__content(Arena *scratch, Doc_Content_List *list, FILE *out){
    fprintf(out, "<div class=\"normal\">");
    
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
    
}

function void
render_doc_page_to_html(Arena *scratch, Doc_Page *page, FILE *file){
    Temp_Memory_Block temp(scratch);
    
    Doc_Cluster *cluster = page->owner;
    
    fprintf(file, html_header, string_expand(page->title));
    
    fprintf(file, "<div class=\"small_spacer\"></div>\n");
    
    fprintf(file, "<div class=\"small\">&gt; <a href=\"%.*s.html\">%.*s</a></div>\n",
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
        
        
        fprintf(file, "<div class=\"small_spacer\"></div>\n");
        fprintf(file, "<h1>%.*s</h1>\n", string_expand(cluster->title));
        fprintf(file, "<div class=\"spacer\"></div>\n");
        
        // TODO(allen): Cluster description.
        fprintf(file, "<div class=\"normal\">More documentation under construction</div>\n");
        
        fprintf(file, "<div class=\"spacer\"></div>\n");
        
        fprintf(file, "<h2><a href=\"%.*s_index.html\">Index</a></h2>\n",
                string_expand(cluster->name));
        
        fprintf(file, html_footer);
    }
    
{
        Temp_Memory_Block temp(scratch);
        
        fprintf(file_index, html_header, string_expand(cluster->title));
    
    fprintf(file_index, "<div class=\"small_spacer\"></div>\n");
    fprintf(file_index, "<h1>%.*s Index</h1>\n", string_expand(cluster->title));
    fprintf(file_index, "<div class=\"spacer\"></div>\n");
    fprintf(file_index, "<input class=\"filter_box\" type=\"text\" id=\"search_input\" onkeyup=\"SearchKeyUp(event)\" onkeydown=\"SearchKeyDown(event)\""
                        "placeholder=\"Filter...\" title=\"Filter...\">");
    fprintf(file_index, "<div class=\"spacer\"></div>\n");
    
    Doc_Page **ptrs = push_array(scratch, Doc_Page*, cluster->page_count);
    i32 counter = 0;
    for (Doc_Page *node = cluster->first_page;
         node != 0;
         node = node->next){
        ptrs[counter] = node;
        counter += 1;
    }
    
    sort_doc_page_array(ptrs, 0, counter);
    
    fprintf(file_index, "<div class=\"normal\">");

    fprintf(file_index, "<ul class=\"docs_menu\" id=\"docs_menu\">\n");
    for (i32 i = 0; i < counter; i += 1){
        Doc_Page *node = ptrs[i];
        fprintf(file_index, "<li><a href=\"%.*s.html\">%.*s</a></li>",
                string_expand(node->name),
                string_expand(node->name));
    }
    fprintf(file_index, "</ul>\n");
    fprintf(file_index, "</div>\n");
    
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
    
    String_Const_u8 self = string_u8_litexpr(__FILE__);
    umem code_pos = string_find_first(self, string_u8_litexpr("code"));
    String_Const_u8 root = string_prefix(self, code_pos + 5);
    String_Const_u8 outside_root = string_prefix(self, code_pos);
    String_Const_u8 build_root = push_u8_stringf(&arena, "%.*sbuild/",
                                             string_expand(outside_root));
    String_Const_u8 site_root = push_u8_stringf(&arena, "%.*ssite/",
                                             string_expand(build_root));
    String_Const_u8 docs_root = push_u8_stringf(&arena, "%.*sdocs/",
                                                string_expand(site_root));
    
    (void)root;
    
    API_Definition *api_def = custom_api_construct(&arena);
    Doc_Cluster *cluster_array[] = {
        doc_custom_api(&arena, api_def),
        doc_commands(&arena),
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

