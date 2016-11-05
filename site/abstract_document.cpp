/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 25.02.2016
 *
 * File editing view for 4coder
 *
 */

// TOP

#if !defined(ABSTRACT_DOCUMENT_H)
#define ABSTRACT_DOCUMENT_H

#define NotImplemented Assert(!"Not Implemented!")

// Enriched Text

struct Enriched_Text{
     String source;
};

static Enriched_Text
load_enriched_text(Partition *part, char *directory, char *filename){
    Enriched_Text result = {0};
    
    char space[256];
    String fname = make_fixed_width_string(space);
    append_sc(&fname, directory);
    append_sc(&fname, "\\");
    append_sc(&fname, filename);
    terminate_with_null(&fname);
    
     result.source = file_dump(fname.str);
    return(result);
    }

// Document Declaration

enum{
    Doc_Root,
    Doc_Section,
    Doc_Todo,
    Doc_Enriched_Text,
    Doc_Element_List,
    Doc_Full_Elements,
    Doc_Table_Of_Contents
};

struct Document_Item{
    Document_Item *next;
    Document_Item *parent;
    int32_t type;
    union{
        struct{
            Document_Item *first_child;
            Document_Item *last_child;
    String name;
            String id;
    } section;
    
    struct{
        Meta_Unit *unit;
    } unit_elements;
    
    struct{
        Enriched_Text *text;
    } text;
};
};
static Document_Item null_document_item = {0};

struct Abstract_Document{
    // Document value members
    Document_Item *root_item;
    
    // Document building members
    Partition *part;
    Document_Item *section_stack[16];
     int32_t section_top;
};
static Abstract_Document null_abstract_document = {0};

static void
set_section_name(Partition *part, Document_Item *item, char *name){
    int32_t name_len = str_size(name);
    item->section.name = make_string_cap(push_array(part, char, name_len+1), 0, name_len+1);
    partition_align(part, 8);
    append_sc(&item->section.name, name);
}

static void
set_section_id(Partition *part, Document_Item *item, char *id){
    int32_t id_len = str_size(id);
    item->section.id = make_string_cap(push_array(part, char, id_len+1), 0, id_len+1);
    partition_align(part, 8);
    append_sc(&item->section.id, id);
}

static void
begin_document_description(Abstract_Document *doc, Partition *part, char *title){
    *doc = null_abstract_document;
    doc->part = part;
    
    doc->root_item = push_struct(doc->part, Document_Item);
    *doc->root_item = null_document_item;
    doc->section_stack[doc->section_top] = doc->root_item;
    doc->root_item->type = Doc_Root;
    
    set_section_name(doc->part, doc->root_item, title);
}

static void
end_document_description(Abstract_Document *doc){
    Assert(doc->section_top == 0);
}

static void
append_child(Document_Item *parent, Document_Item *item){
    Assert(parent->type == Doc_Root || parent->type == Doc_Section);
    if (parent->section.last_child == 0){
        parent->section.first_child = item;
    }
    else{
        parent->section.last_child->next = item;
    }
    parent->section.last_child = item;
    item->parent = parent;
}

static void
begin_section(Abstract_Document *doc, char *title, char *id){
    Assert(doc->section_top+1 < ArrayCount(doc->section_stack));
    
    Document_Item *parent = doc->section_stack[doc->section_top];
    Document_Item *section = push_struct(doc->part, Document_Item);
    *section = null_document_item;
    doc->section_stack[++doc->section_top] = section;
    
    section->type = Doc_Section;
    
    set_section_name(doc->part, section, title);
    if (id){
    set_section_id(doc->part, section, id);
    }
    
    append_child(parent, section);
    }

static void
end_section(Abstract_Document *doc){
    Assert(doc->section_top > 0);
    --doc->section_top;
}

static void
add_todo(Abstract_Document *doc){
    Document_Item *parent = doc->section_stack[doc->section_top];
    Document_Item *item = push_struct(doc->part, Document_Item);
    *item = null_document_item;
    item->type = Doc_Todo;
    
    append_child(parent, item);
}

static void
add_element_list(Abstract_Document *doc, Meta_Unit *unit){
    Document_Item *parent = doc->section_stack[doc->section_top];
    Document_Item *item = push_struct(doc->part, Document_Item);
    *item = null_document_item;
    item->type = Doc_Element_List;
    item->unit_elements.unit = unit;
    
    append_child(parent, item);
}

static void
add_full_elements(Abstract_Document *doc, Meta_Unit *unit){
    Document_Item *parent = doc->section_stack[doc->section_top];
    Document_Item *item = push_struct(doc->part, Document_Item);
    *item = null_document_item;
    item->type = Doc_Full_Elements;
    item->unit_elements.unit = unit;
    
    append_child(parent, item);
}

static void
add_table_of_contents(Abstract_Document *doc){
    Document_Item *parent = doc->section_stack[doc->section_top];
    Document_Item *item = push_struct(doc->part, Document_Item);
    *item = null_document_item;
    item->type = Doc_Table_Of_Contents;
    
    append_child(parent, item);
}

static void
add_enriched_text(Abstract_Document *doc, Enriched_Text *text){
    Assert(doc->section_top+1 < ArrayCount(doc->section_stack));
    
    Document_Item *parent = doc->section_stack[doc->section_top];
    Document_Item *item = push_struct(doc->part, Document_Item);
    *item = null_document_item;
    item->type = Doc_Enriched_Text;
    item->text.text = text;
    
    append_child(parent, item);
}

// Document Generation

#define HTML_BACK_COLOR   "#FAFAFA"
#define HTML_TEXT_COLOR   "#0D0D0D"
#define HTML_CODE_BACK    "#DFDFDF"
#define HTML_EXAMPLE_BACK "#EFEFDF"

#define HTML_POP_COLOR_1  "#309030"
#define HTML_POP_BACK_1   "#E0FFD0"
#define HTML_VISITED_LINK "#A0C050"

#define HTML_POP_COLOR_2  "#005000"

#define HTML_CODE_STYLE "font-family: \"Courier New\", Courier, monospace; text-align: left;"

#define HTML_CODE_BLOCK_STYLE(back)                             \
"margin-top: 3mm; margin-bottom: 3mm; font-size: .95em; "  \
"background: "back"; padding: 0.25em;"

#define HTML_DESCRIPT_SECTION_STYLE HTML_CODE_BLOCK_STYLE(HTML_CODE_BACK)
#define HTML_EXAMPLE_CODE_STYLE HTML_CODE_BLOCK_STYLE(HTML_EXAMPLE_BACK)

#define HTML_DOC_HEAD_OPEN  "<div style='margin-top: 3mm; margin-bottom: 3mm; color: "HTML_POP_COLOR_1";'><b><i>"
#define HTML_DOC_HEAD_CLOSE "</i></b></div>"

#define HTML_DOC_ITEM_HEAD_STYLE "font-weight: 600;"

#define HTML_DOC_ITEM_HEAD_INL_OPEN  "<span style='"HTML_DOC_ITEM_HEAD_STYLE"'>"
#define HTML_DOC_ITEM_HEAD_INL_CLOSE "</span>"

#define HTML_DOC_ITEM_HEAD_OPEN  "<div style='"HTML_DOC_ITEM_HEAD_STYLE"'>"
#define HTML_DOC_ITEM_HEAD_CLOSE "</div>"

#define HTML_DOC_ITEM_OPEN  "<div style='margin-left: 5mm; margin-right: 5mm;'>"
#define HTML_DOC_ITEM_CLOSE "</div>"

#define HTML_EXAMPLE_CODE_OPEN  "<div style='"HTML_CODE_STYLE HTML_EXAMPLE_CODE_STYLE"'>"
#define HTML_EXAMPLE_CODE_CLOSE "</div>"

struct Section_Counter{
    int32_t counter[16];
    int32_t nest_level;
};

static void
append_section_number(String *out, Section_Counter section_counter){
    for (int32_t i = 1; i <= section_counter.nest_level; ++i){
        append_int_to_str(out, section_counter.counter[i]);
        if (i != section_counter.nest_level){
            append_sc(out, ".");
        }
    }
}

static void
write_enriched_text_html(String *out, Enriched_Text *text){
    String source = text->source;
    
    append_sc(out, "<div>");
    
    for (String line = get_first_double_line(source);
         line.str;
         line = get_next_double_line(source, line)){
        String l  = skip_chop_whitespace(line);
        append_sc(out, "<p>");
        
        //append_ss(out, l);
        int32_t start = 0, i = 0;
        for (; i < l.size; ++i){
            if (l.str[i] == '\\'){
                append_ss(out, substr(l, start, i-start));
                
                int32_t command_start = i+1;
                int32_t command_end = command_start;
                for (; command_end < l.size; ++command_end){
                    if (!char_is_alpha_numeric(l.str[command_end])){
                        break;
                    }
                }
                
                if (command_end == command_start){
                    if (command_end < l.size && l.str[command_end] == '\\'){
                        ++command_end;
                    }
                }
                
                String command_string = substr(l, command_start, command_end - command_start);
                
                static String enriched_commands[] = {
                    make_lit_string("\\"),
                    make_lit_string("VERSION"),
                    make_lit_string("CODE_STYLE"),
                };
                
                int32_t match_index = 0;
                if (string_set_match(enriched_commands, ArrayCount(enriched_commands), command_string, &match_index)){
                    switch (match_index){
                        case 0: append_sc(out, "\\"); break;
                        case 1: append_sc(out, VERSION); break;
                        case 2: append_sc(out, "<span style='"HTML_CODE_STYLE"'> TEST </span>"); break;
                    }
                }
                else{
                    append_sc(out,"<span style='color:#F00'>! Doc generator error: unrecognized command !</span>");
                    fprintf(stderr, "error: Unrecognized command %.*s\n", command_string.size, command_string.str);
                }
                
                i = command_end;
                start = i;
            }
        }
        
        if (start != i){
            append_ss(out, substr(l, start, i-start));
        }
        
        append_sc(out, "</p>");
    }
    
    append_sc(out, "</div>");
    }

static void
doc_item_head_html(String *out, Document_Item *item, Section_Counter section_counter){
    switch (item->type){
        case Doc_Root:
        {
            append_sc(out,
                      "<html lang=\"en-US\">"
                      "<head>"
                      "<title>");
            
            append_ss(out, item->section.name);
            
            append_sc(out,
                      "</title>"
                      "<style>"
                      
                      "body { "
                      "background: " HTML_BACK_COLOR "; "
                      "color: " HTML_TEXT_COLOR "; "
                      "}"
                      
                      // H things
                      "h1,h2,h3,h4 { "
                      "color: " HTML_POP_COLOR_1 "; "
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
                      "color: " HTML_POP_COLOR_1 "; "
                      "text-decoration: none; "
                      "}"
                      "a:visited { "
                      "color: " HTML_VISITED_LINK "; "
                      "}"
                      "a:hover { "
                      "background: " HTML_POP_BACK_1 "; "
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
                      "width: 800px; text-align: justify; line-height: 1.25;'>");
            
            
            append_sc(out, "<h1 style='margin-top: 5mm; margin-bottom: 5mm;'>");
            append_ss(out, item->section.name);
            append_sc(out, "</h1>");
        }break;
        
        case Doc_Section:
        {
            if (section_counter.nest_level <= 1){
            if (item->section.id.size > 0){
            append_sc(out, "\n<h2 id='section_");
            append_ss(out, item->section.id);
            append_sc(out, "'>&sect;");
            }
            else{
                append_sc(out, "\n<h2>&sect;");
            }
            append_section_number(out, section_counter);
                append_sc(out, " ");
            append_ss(out, item->section.name);
            append_sc(out, "</h2>");
        }
        else{
            if (item->section.id.size > 0){
                append_sc(out, "<h3 id='section_");
                append_ss(out, item->section.id);
                append_sc(out, "'>&sect;");
            }
            else{
                append_sc(out, "<h3>&sect;");
            }
            append_section_number(out, section_counter);
            append_sc(out, " ");
            append_ss(out, item->section.name);
            append_sc(out, "</h3>");
        }
        }break;
        
        case Doc_Enriched_Text:
        {
            write_enriched_text_html(out, item->text.text);
        }break;
        
        case Doc_Table_Of_Contents:
        {
            append_sc(out, "<h3 style='margin:0;'>Table of Contents</h3><ul>");
            
            int32_t i = 1;
            for (Document_Item *toc_item = item->parent->section.first_child;
                 toc_item != 0;
                 toc_item = toc_item->next){
                if (toc_item->type == Doc_Section){
                    if (toc_item->section.id.size > 0){
                append_sc(out, "<li><a href='#section_");
                append_ss(out, toc_item->section.id);
                append_sc(out, "'>&sect;");
                    }
                    else{
                        append_sc(out, "<li>&sect;");
                    }
                append_int_to_str (out, i);
                append_s_char     (out, ' ');
                    append_ss         (out, toc_item->section.name);
                    append_sc         (out, "</a></li>");
                    ++i;
                }
            }
            
            append_sc(out, "</ul>");
        }break;
    }
}

static void
doc_item_foot_html(String *out, Document_Item *item, Section_Counter section_counter){
    switch (item->type){
        case Doc_Root:
        {
            append_sc(out, "</div></body></html>");
        }break;
        
        case Doc_Section: break;
        
        case Doc_Table_Of_Contents: break;
    }
}

static void
generate_item_html(String *out, Document_Item *item, Section_Counter *section_counter){
    Section_Counter sc = *section_counter;
    doc_item_head_html(out, item, sc);
    
    if (item->type == Doc_Root || item->type == Doc_Section){
        int32_t level = ++section_counter->nest_level;
        section_counter->counter[level] = 1;
        for (Document_Item *m = item->section.first_child;
             m != 0;
             m = m->next){
            generate_item_html(out, m, section_counter);
        }
        --section_counter->nest_level;
        ++section_counter->counter[section_counter->nest_level];
    }
    
    doc_item_foot_html(out, item, sc);
}

static void
generate_document_html(String *out, Abstract_Document *doc){
    Section_Counter section_counter = {0};
    section_counter.counter[section_counter.nest_level] = 1;
    generate_item_html(out, doc->root_item, &section_counter);
}

#endif

// BOTTOM

