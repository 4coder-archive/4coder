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
    append_sc(&fname, "/");
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

struct Alternate_Name{
    String macro;
    String public_name;
};

struct Alternate_Names_Array{
    Alternate_Name *names;
};

enum{
    AltName_Standard,
    AltName_Macro,
    AltName_Public_Name,
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
            int32_t show_title;
        } section;
        
        struct{
            Meta_Unit *unit;
            Alternate_Names_Array *alt_names;
            int32_t alt_name_type;
        } unit_elements;
        
        struct{
            Enriched_Text *text;
        } text;
    };
};
static Document_Item null_document_item = {0};

enum{
    ItemType_Document,
    ItemType_Image,
    ItemType_GenericFile,
    // never below this
    ItemType_COUNT,
};

struct Basic_Node{
    Basic_Node *next;
};

#define NodeGetData(node, T) ((T*) ((node)+1))

struct Basic_List{
    Basic_Node *head;
    Basic_Node *tail;
};

struct Abstract_Item{
    int32_t item_type;
    char *name;
    
    // Document value members
    Document_Item *root_item;
    
    // TODO(allen): make these external
    // Document building members
    Partition *part;
    Document_Item *section_stack[16];
    int32_t section_top;
    
    // Image value members
    char *source_file;
    char *extension;
    float w_h_ratio;
    float h_w_ratio;
    Basic_List img_instantiations;
};
static Abstract_Item null_abstract_item = {0};

struct Image_Instantiation{
    int32_t w, h;
};

struct Document_System{
    Basic_List doc_list;
    Basic_List img_list;
    Basic_List file_list;
    Partition *part;
};

static Document_System
create_document_system(Partition *part){
    Document_System system = {0};
    system.part = part;
    return(system);
}

static void*
push_item_on_list(Partition *part, Basic_List *list, int32_t item_size){
    int32_t mem_size = item_size + sizeof(Basic_Node);
    void *mem = push_block(part, mem_size);
    assert(mem != 0);
    memset(mem, 0, mem_size);
    
    Basic_Node *node = (Basic_Node*)mem;
    if (list->head == 0){
        list->head = node;
        list->tail = node;
    }
    else{
        list->tail->next = node;
        list->tail = node;
    }
    
    void *result = (node+1);
    return(result);
}

static Abstract_Item*
get_item_by_name(Basic_List list, String name){
    Abstract_Item *result = 0;
    
    for (Basic_Node *node = list.head;
         node != 0;
         node = node->next){
        Abstract_Item *item = NodeGetData(node, Abstract_Item);
        if (match(item->name, name)){
            result = item;
            break;
        }
    }
    
    return(result);
}

static Abstract_Item*
get_item_by_name(Basic_List list, char *name){
    Abstract_Item *result = 0;
    
    for (Basic_Node *node = list.head;
         node != 0;
         node = node->next){
        Abstract_Item *item = NodeGetData(node, Abstract_Item);
        if (match(item->name, name)){
            result = item;
            break;
        }
    }
    
    return(result);
}

static Image_Instantiation*
get_image_instantiation(Basic_List list, int32_t w, int32_t h){
    Image_Instantiation *result = 0;
    
    for (Basic_Node *node = list.head;
         node != 0;
         node = node->next){
        Image_Instantiation *instantiation = NodeGetData(node, Image_Instantiation);
        if (instantiation->w == w && instantiation->h == h){
            result = instantiation;
            break;
        }
    }
    
    return(result);
}

static Abstract_Item*
create_item(Partition *part, Basic_List *list, char *name){
    Abstract_Item *lookup = get_item_by_name(*list, name);
    
    Abstract_Item *result = 0;
    if (lookup == 0){
        result = (Abstract_Item*)push_item_on_list(part, list, sizeof(Abstract_Item));
    }
    
    return(result);
}

static void
add_image_instantiation(Partition *part, Basic_List *list, int32_t w, int32_t h){
    Image_Instantiation *instantiation = (Image_Instantiation*)push_item_on_list(part, list, sizeof(Image_Instantiation));
    instantiation->w = w;
    instantiation->h = h;
}

static void
set_section_name(Partition *part, Document_Item *item, char *name, int32_t show_title){
    int32_t name_len = str_size(name);
    item->section.name = make_string_cap(push_array(part, char, name_len+1), 0, name_len+1);
    partition_align(part, 8);
    append_sc(&item->section.name, name);
    item->section.show_title = show_title;
}

static void
set_section_id(Partition *part, Document_Item *item, char *id){
    int32_t id_len = str_size(id);
    item->section.id = make_string_cap(push_array(part, char, id_len+1), 0, id_len+1);
    partition_align(part, 8);
    append_sc(&item->section.id, id);
}

static void
begin_document_description(Abstract_Item *doc, Partition *part, char *title, int32_t show_title){
    *doc = null_abstract_item;
    doc->item_type = ItemType_Document;
    doc->part = part;
    
    doc->root_item = push_struct(doc->part, Document_Item);
    *doc->root_item = null_document_item;
    doc->section_stack[doc->section_top] = doc->root_item;
    doc->root_item->type = Doc_Root;
    
    set_section_name(doc->part, doc->root_item, title, show_title);
}

static Abstract_Item*
add_generic_file(Document_System *system, char *source_file, char *extension, char *name){
    Abstract_Item *item = create_item(system->part, &system->file_list, name);
    if (item){
        item->item_type = ItemType_GenericFile;
        item->extension = extension;
        item->source_file = source_file;
        item->name = name;
    }
    return(item);
}

static Abstract_Item*
add_image_description(Document_System *system, char *source_file, char *extension, char *name){
    Abstract_Item *item = create_item(system->part, &system->img_list, name);
    if (item){
        item->item_type = ItemType_Image;
        item->extension = extension;
        item->source_file = source_file;
        item->name = name;
        
        int32_t w = 0, h = 0, comp = 0;
        int32_t stbi_r = stbi_info(source_file, &w, &h, &comp);
        if (!stbi_r){
            fprintf(stderr, "Did not find file %s\n", source_file);
            item->w_h_ratio = 1.f;
            item->h_w_ratio = 1.f;
        }
        else{
        item->w_h_ratio = ((float)w/(float)h);
        item->h_w_ratio = ((float)h/(float)w);
        }
    }
    return(item);
}

static Abstract_Item*
begin_document_description(Document_System *system, char *title, char *name, int32_t show_title){
    Abstract_Item *item = create_item(system->part, &system->doc_list, name);
    if (item){
        begin_document_description(item, system->part, title, show_title);
        item->name = name;
    }
    return(item);
}

static void
end_document_description(Abstract_Item *item){
    Assert(item->section_top == 0);
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
begin_section(Abstract_Item *item, char *title, char *id){
    Assert(item->section_top+1 < ArrayCount(item->section_stack));
    
    Document_Item *parent = item->section_stack[item->section_top];
    Document_Item *section = push_struct(item->part, Document_Item);
    *section = null_document_item;
    item->section_stack[++item->section_top] = section;
    
    section->type = Doc_Section;
    
    set_section_name(item->part, section, title, 1);
    if (id){
        set_section_id(item->part, section, id);
    }
    
    append_child(parent, section);
}

static void
end_section(Abstract_Item *doc){
    Assert(doc->section_top > 0);
    --doc->section_top;
}

static void
add_todo(Abstract_Item *doc){
    Document_Item *parent = doc->section_stack[doc->section_top];
    Document_Item *item = push_struct(doc->part, Document_Item);
    *item = null_document_item;
    item->type = Doc_Todo;
    
    append_child(parent, item);
}

static void
add_element_list(Abstract_Item *doc, Meta_Unit *unit){
    Document_Item *parent = doc->section_stack[doc->section_top];
    Document_Item *item = push_struct(doc->part, Document_Item);
    *item = null_document_item;
    item->type = Doc_Element_List;
    item->unit_elements.unit = unit;
    
    append_child(parent, item);
}

static void
add_element_list(Abstract_Item *doc, Meta_Unit *unit, Alternate_Names_Array *alt_names, int32_t alt_name_type){
    Document_Item *parent = doc->section_stack[doc->section_top];
    Document_Item *item = push_struct(doc->part, Document_Item);
    *item = null_document_item;
    item->type = Doc_Element_List;
    item->unit_elements.unit = unit;
    item->unit_elements.alt_names = alt_names;
    item->unit_elements.alt_name_type = alt_name_type;
    
    append_child(parent, item);
}

static void
add_full_elements(Abstract_Item *doc, Meta_Unit *unit){
    Document_Item *parent = doc->section_stack[doc->section_top];
    Document_Item *item = push_struct(doc->part, Document_Item);
    *item = null_document_item;
    item->type = Doc_Full_Elements;
    item->unit_elements.unit = unit;
    
    append_child(parent, item);
}

static void
add_full_elements(Abstract_Item *doc, Meta_Unit *unit, Alternate_Names_Array *alt_names, int32_t alt_name_type){
    Document_Item *parent = doc->section_stack[doc->section_top];
    Document_Item *item = push_struct(doc->part, Document_Item);
    *item = null_document_item;
    item->type = Doc_Full_Elements;
    item->unit_elements.unit = unit;
    item->unit_elements.alt_names = alt_names;
    item->unit_elements.alt_name_type = alt_name_type;
    
    append_child(parent, item);
}

static void
add_table_of_contents(Abstract_Item *doc){
    Document_Item *parent = doc->section_stack[doc->section_top];
    Document_Item *item = push_struct(doc->part, Document_Item);
    *item = null_document_item;
    item->type = Doc_Table_Of_Contents;
    
    append_child(parent, item);
}

static void
add_enriched_text(Abstract_Item *doc, Enriched_Text *text){
    Assert(doc->section_top+1 < ArrayCount(doc->section_stack));
    
    Document_Item *parent = doc->section_stack[doc->section_top];
    Document_Item *item = push_struct(doc->part, Document_Item);
    *item = null_document_item;
    item->type = Doc_Enriched_Text;
    item->text.text = text;
    
    append_child(parent, item);
}

// HTML Document Generation

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

static int32_t
doc_get_link_string(Abstract_Item *doc, char *space, int32_t capacity){
    String str = make_string_cap(space, 0, capacity);
    append_sc(&str, doc->name);
    append_sc(&str, ".html");
    int32_t result = terminate_with_null(&str);
    return(result);
}

static int32_t
img_get_link_string(Abstract_Item *img, char *space, int32_t capacity, int32_t w, int32_t h){
    String str = make_string_cap(space, 0, capacity);
    append_sc(&str, img->name);
    
    append_sc(&str, "_");
    append_int_to_str(&str, w);
    append_sc(&str, "_");
    append_int_to_str(&str, h);
    
    append_sc(&str, ".");
    append_sc(&str, img->extension);
    int32_t result = terminate_with_null(&str);
    return(result);
}

static void
append_section_number_reduced(String *out, Section_Counter *section_counter, int32_t reduce){
    int32_t level = section_counter->nest_level-reduce;
    for (int32_t i = 1; i <= level; ++i){
        append_int_to_str(out, section_counter->counter[i]);
        if (i != level){
            append_sc(out, ".");
        }
    }
}

static void
append_section_number(String *out, Section_Counter *section_counter){
    append_section_number_reduced(out, section_counter, 0);
}

static int32_t
extract_command_body(String *out, String l, int32_t *i_in_out, int32_t *body_start_out, int32_t *body_end_out, String command_name, int32_t require_body){
    int32_t result = 0;
    
    int32_t i = *i_in_out;
    
    for (; i < l.size; ++i){
        if (!char_is_whitespace(l.str[i])){
            break;
        }
    }
    
    int32_t found_command_body = 0;
    int32_t body_start = 0, body_end = 0;
    if (l.str[i] == '{'){
        body_start = i+1;
        
        for (++i; i < l.size; ++i){
            if (l.str[i] == '}'){
                found_command_body = 1;
                body_end = i;
                ++i;
                break;
            }
        }
    }
    
    if (found_command_body){
        result = 1;
    }
    else{
        if (require_body){
#define STR_START "<span style='color:#F00'>! Doc generator error: missing body for "
#define STR_SLOW " !</span>"
        append_sc(out, STR_START);
        append_ss(out, command_name);
        append_sc(out, STR_SLOW);
#undef STR
        
        fprintf(stderr, "error: missing body for %.*s\n", command_name.size, command_name.str);
        }
    }
    
    *i_in_out = i;
    *body_start_out = body_start;
    *body_end_out = body_end;
    
    return(result);
}

static void
html_render_section_header(String *out, String section_name, String section_id, Section_Counter *section_counter){
    if (section_counter->nest_level <= 1){
        if (section_id.size > 0){
            append_sc(out, "\n<h2 id='section_");
            append_ss(out, section_id);
            append_sc(out, "'>&sect;");
        }
        else{
            append_sc(out, "\n<h2>&sect;");
        }
        append_section_number(out, section_counter);
        append_sc(out, " ");
        append_ss(out, section_name);
        append_sc(out, "</h2>");
    }
    else{
        if (section_id.size > 0){
            append_sc(out, "<h3 id='section_");
            append_ss(out, section_id);
            append_sc(out, "'>&sect;");
        }
        else{
            append_sc(out, "<h3>&sect;");
        }
        append_section_number(out, section_counter);
        append_sc(out, " ");
        append_ss(out, section_name);
        append_sc(out, "</h3>");
    }
}

#define HTML_WIDTH 800

static void
write_enriched_text_html(String *out, Partition *part, Enriched_Text *text, Document_System *doc_system,  Section_Counter *section_counter){
    String source = text->source;
    
    append_sc(out, "<div>");
    
    int32_t item_counter = 0;
    
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
                
                enum Command_Types{
                    Cmd_BackSlash,
                    Cmd_Version,
                    Cmd_BeginStyle,
                    Cmd_EndStyle,
                    Cmd_DocLink,
                    Cmd_BeginList,
                    Cmd_EndList,
                    Cmd_BeginItem,
                    Cmd_EndItem,
                    Cmd_BoldFace,
                    Cmd_Section,
                    Cmd_BeginLink,
                    Cmd_EndLink,
                    Cmd_Image,
                    Cmd_Video,
                    // never below this
                    Cmd_COUNT,
                };
                
                static String enriched_commands[Cmd_COUNT];
                
                enriched_commands[Cmd_BackSlash]  = make_lit_string("\\");
                enriched_commands[Cmd_Version]    = make_lit_string("VERSION");
                enriched_commands[Cmd_BeginStyle] = make_lit_string("BEGIN_STYLE");
                enriched_commands[Cmd_EndStyle]   = make_lit_string("END_STYLE");
                enriched_commands[Cmd_DocLink]    = make_lit_string("DOC_LINK");
                enriched_commands[Cmd_BeginList]  = make_lit_string("BEGIN_LIST");
                enriched_commands[Cmd_EndList]    = make_lit_string("END_LIST");
                enriched_commands[Cmd_BeginItem]  = make_lit_string("BEGIN_ITEM");
                enriched_commands[Cmd_EndItem]    = make_lit_string("END_ITEM");
                enriched_commands[Cmd_BoldFace]   = make_lit_string("BOLD_FACE");
                enriched_commands[Cmd_Section]    = make_lit_string("SECTION");
                enriched_commands[Cmd_BeginLink]  = make_lit_string("BEGIN_LINK");
                enriched_commands[Cmd_EndLink]    = make_lit_string("END_LINK");
                enriched_commands[Cmd_Image]      = make_lit_string("IMAGE");
                enriched_commands[Cmd_Video]      = make_lit_string("VIDEO");
                
                i = command_end;
                
                int32_t match_index = 0;
                if (string_set_match(enriched_commands, ArrayCount(enriched_commands), command_string, &match_index)){
                    switch (match_index){
                        case Cmd_BackSlash: append_sc(out, "\\"); break;
                        case Cmd_Version: append_sc(out, VERSION); break;
                        
                        case Cmd_BeginStyle:
                        {
                            int32_t body_start = 0, body_end = 0;
                            int32_t has_body = extract_command_body(out, l, &i, &body_start, &body_end, command_string, 1);
                            if (has_body){
                                String body_text = substr(l, body_start, body_end - body_start);
                                body_text = skip_chop_whitespace(body_text);
                                if (match_sc(body_text, "code")){
                                    append_sc(out, "<span style='"HTML_CODE_STYLE"'>");
                                }
                            }
                        }break;
                        
                        case Cmd_EndStyle:
                        {
                            append_sc(out, "</span>");
                        }break;
                        
                        // TODO(allen): upgrade this bs
                        case Cmd_DocLink:
                        {
                            int32_t body_start = 0, body_end = 0;
                            int32_t has_body = extract_command_body(out, l, &i, &body_start, &body_end, command_string, 1);
                            if (has_body){
                                String body_text = substr(l, body_start, body_end - body_start);
                                body_text = skip_chop_whitespace(body_text);
                                append_sc(out, "<a href='#");
                                append_ss(out, body_text);
                                append_sc(out, "_doc'>");
                                append_ss(out, body_text);
                                append_sc(out, "</a>");
                            }
                        }break;
                        
                        case Cmd_BeginList:
                        {
                            append_sc(out,"<ul style='margin-top: 5mm; margin-left: 1mm;'>");
                        }break;
                        
                        case Cmd_EndList:
                        {
                            append_sc(out, "</ul>");
                        }break;
                        
                        case Cmd_BeginItem:
                        {
                            if (item_counter == 0){
                                append_sc(out, "<li style='font-size: 95%; background: #EFEFDF;'>");
                                ++item_counter;
                            }
                            else{
                                append_sc(out, "<li style='font-size: 95%;'>");
                                item_counter = 0;
                            }
                        }break;
                        
                        case Cmd_EndItem:
                        {
                            append_sc(out, "</li>");
                        }break;
                        
                        case Cmd_Section:
                        {
                            // TODO(allen): undo the duplication of this body extraction code.
                            int32_t body_start = 0, body_end = 0;
                            int32_t has_body = extract_command_body(out, l, &i, &body_start, &body_end, command_string, 1);
                            if (has_body){
                                String body_text = substr(l, body_start, body_end - body_start);
                                body_text = skip_chop_whitespace(body_text);
                                
                                html_render_section_header(out, body_text, null_string, section_counter);
                                ++section_counter->counter[section_counter->nest_level];
                                item_counter = 0;
                            }
                        }break;
                        
                        case Cmd_BeginLink:
                        {
                            int32_t body_start = 0, body_end = 0;
                            int32_t has_body = extract_command_body(out, l, &i, &body_start, &body_end, command_string, 1);
                            if (has_body){
                                String body_text = substr(l, body_start, body_end - body_start);
                                body_text = skip_chop_whitespace(body_text);
                                
                                append_sc(out, "<a ");
                                if (body_text.str[0] == '!'){
                                    append_sc(out, "target='_blank' ");
                                    body_text.str++;
                                    body_text.size--;
                                }
                                append_sc(out, "href='");
                                if (match_part_sc(body_text, "document:")){
                                    String doc_name = substr_tail(body_text, sizeof("document:")-1);
                                    Abstract_Item *doc_lookup = get_item_by_name(doc_system->doc_list, doc_name);
                                    if (doc_lookup){
                                        char space[256];
                                        if (doc_get_link_string(doc_lookup, space, sizeof(space))){
                                            append_sc(out, space);
                                        }
                                        else{
                                            NotImplemented;
                                        }
                                    }
                                }
                                else{
                                    append_ss(out, body_text);
                                }
                                append_sc(out, "'>");
                            }
                        }break;
                        
                        case Cmd_EndLink:
                        {
                            append_sc(out, "</a>");
                        }break;
                        
                        case Cmd_Image:
                        {
                            // TODO(allen): generalize this
                            int32_t body_start = 0, body_end = 0;
                            int32_t has_body = extract_command_body(out, l, &i, &body_start, &body_end, command_string, 1);
                            
                            if (has_body){
                                String body_text = substr(l, body_start, body_end - body_start);
                                body_text = skip_chop_whitespace(body_text);
                                
                                int32_t pixel_height = 10;
                                int32_t pixel_width = HTML_WIDTH;
                                
                                body_start = 0, body_end = 0;
                                has_body = extract_command_body(out, l, &i, &body_start, &body_end, command_string, 0);
                                if (has_body){
                                    String size_parameter = substr(l, body_start, body_end - body_start);
                                    if (match_part_sc(size_parameter, "width:")){
                                        String width_string = substr_tail(size_parameter, sizeof("width:")-1);
                                        if (str_is_int_s(width_string)){
                                            pixel_width = str_to_int_s(width_string);
                                        }
                                    }
                                }
                                
                                if (match_part_sc(body_text, "image:")){
                                    String img_name = substr_tail(body_text, sizeof("image:")-1);
                                    Abstract_Item *img_lookup = get_item_by_name(doc_system->img_list, img_name);
                                    
                                    if (img_lookup){
                                        pixel_height = CEIL32(pixel_width*img_lookup->h_w_ratio);
                                        
                                        append_sc(out, "<img src='");
                                        
                                        char space[256];
                                        if (img_get_link_string(img_lookup, space, sizeof(space), pixel_width, pixel_height)){
                                            append_sc(out, space);
                                            add_image_instantiation(part, &img_lookup->img_instantiations, pixel_width, pixel_height);
                                        }
                                        else{
                                            NotImplemented;
                                        }
                                        
                                        append_sc(out, "' style='width: ");
                                        append_int_to_str(out, pixel_width);
                                        append_sc(out, "px; height: ");
                                        append_int_to_str(out, pixel_height);
                                        append_sc(out, "px;'>");
                                    }
                                }
                            }
                        }break;
                        
                        case Cmd_Video:
                        {
                            // TODO(allen): generalize this
                            int32_t body_start = 0, body_end = 0;
                            int32_t has_body = extract_command_body(out, l, &i, &body_start, &body_end, command_string, 1);
                            
                            int32_t pixel_width = HTML_WIDTH;
                            int32_t pixel_height = (int32_t)(pixel_width * 0.5625);
                            
                            if (has_body){
                                String body_text = substr(l, body_start, body_end - body_start);
                                body_text = skip_chop_whitespace(body_text);
                                
                                if (match_part_sc(body_text, "youtube:")){
                                    String youtube_str = substr_tail(body_text, sizeof("youtube:")-1);
                                    
                                    append_sc(out, "<iframe  width='");
                                    append_int_to_str(out, pixel_width);
                                    append_sc(out, "' height='");
                                    append_int_to_str(out, pixel_height);
                                append_sc(out, "' src='");
                                    append_ss(out, youtube_str);
                                    append_sc(out, "' allowfullscreen> </iframe>");
                                }
                            }
                        }break;
                    }
                }
                else{
                    append_sc(out, "<span style='color:#F00'>! Doc generator error: unrecognized command !</span>");
                    fprintf(stderr, "error: unrecognized command %.*s\n", command_string.size, command_string.str);
                }
                
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
print_item_in_list(String *out, String name, char *id_postfix){
    append_sc(out, "<li><a href='#");
    append_ss(out, name);
    append_sc(out, id_postfix);
    append_sc(out, "'>");
    append_ss(out, name);
    append_sc(out, "</a></li>");
}

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
                
                
                append_sc(out, HTML_EXAMPLE_CODE_OPEN);
                
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
                
                append_sc(out, HTML_EXAMPLE_CODE_CLOSE);
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
            
            append_sc(out, "<div style='"HTML_CODE_STYLE"'>"HTML_DOC_ITEM_HEAD_INL_OPEN);
            append_ss(out, member_iter->name);
            append_sc(out, HTML_DOC_ITEM_HEAD_INL_CLOSE"</div>");
            
            append_sc(out, "<div style='margin-bottom: 6mm;'>"HTML_DOC_ITEM_OPEN);
            print_doc_description(out, part, doc.main_doc);
            append_sc(out, HTML_DOC_ITEM_CLOSE"</div>");
            
            append_sc(out, "</div>");
        }
    }
}

static void
print_see_also(String *out, Documentation *doc){
    int32_t doc_see_count = doc->see_also_count;
    if (doc_see_count > 0){
        append_sc(out, HTML_DOC_HEAD_OPEN"See Also"HTML_DOC_HEAD_CLOSE);
        
        for (int32_t j = 0; j < doc_see_count; ++j){
            String see_also = doc->see_also[j];
            append_sc(out, HTML_DOC_ITEM_OPEN"<a href='#");
            append_ss(out, see_also);
            append_sc(out, "_doc'>");
            append_ss(out, see_also);
            append_sc(out, "</a>"HTML_DOC_ITEM_CLOSE);
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
        append_sc(out, HTML_DOC_HEAD_OPEN"Parameters"HTML_DOC_HEAD_CLOSE);
        
        for (int32_t j = 0; j < doc_param_count; ++j){
            String param_name = doc.param_name[j];
            String param_docs = doc.param_docs[j];
            
            // TODO(allen): check that param_name is actually
            // a parameter to this function!
            
            append_sc(out, "<div>"HTML_DOC_ITEM_HEAD_OPEN);
            append_ss(out, param_name);
            append_sc(out, HTML_DOC_ITEM_HEAD_CLOSE"<div style='margin-bottom: 6mm;'>"HTML_DOC_ITEM_OPEN);
            append_ss(out, param_docs);
            append_sc(out, HTML_DOC_ITEM_CLOSE"</div></div>");
        }
    }
    
    String ret_doc = doc.return_doc;
    if (ret_doc.size != 0){
        append_sc(out, HTML_DOC_HEAD_OPEN"Return"HTML_DOC_HEAD_CLOSE HTML_DOC_ITEM_OPEN);
        append_ss(out, ret_doc);
        append_sc(out, HTML_DOC_ITEM_CLOSE);
    }
    
    String main_doc = doc.main_doc;
    if (main_doc.size != 0){
        append_sc(out, HTML_DOC_HEAD_OPEN"Description"HTML_DOC_HEAD_CLOSE HTML_DOC_ITEM_OPEN);
        print_doc_description(out, part, main_doc);
        append_sc(out, HTML_DOC_ITEM_CLOSE);
    }
    
    print_see_also(out, &doc);
    
    end_temp_memory(temp);
}

static void
print_item_html(String *out, Partition *part, Used_Links *used, Item_Node *item, char *id_postfix, char *section, int32_t I, Alternate_Name *alt_name, int32_t alt_name_type){
    Temp_Memory temp = begin_temp_memory(part);
    
    String name = item->name;
    
    switch (alt_name_type){
        case AltName_Macro:
        {
            name = alt_name->macro;
        }break;
        
        case AltName_Public_Name:
        {
            name = alt_name->public_name;
        }break;
    }
    
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
    
    append_sc(out, "<div style='"HTML_CODE_STYLE" "HTML_DESCRIPT_SECTION_STYLE"'>");
    
    switch (item->t){
        case Item_Function:
        {
            // NOTE(allen): Code box
            print_function_html(out, used, item->cpp_name, item->ret, "", name, item->breakdown);
            
            // NOTE(allen): Close the code box
            append_sc(out, "</div>");
            
            // NOTE(allen): Descriptive section
            print_function_docs(out, part, name, item->doc_string);
        }break;
        
        case Item_Macro:
        {
            // NOTE(allen): Code box
            print_macro_html(out, name, item->breakdown);
            
            // NOTE(allen): Close the code box
            append_sc(out, "</div>");
            
            // NOTE(allen): Descriptive section
            print_function_docs(out, part, name, item->doc_string);
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
                append_sc(out, HTML_DOC_HEAD_OPEN"Description"HTML_DOC_HEAD_CLOSE);
                
                append_sc(out, HTML_DOC_ITEM_OPEN);
                print_doc_description(out, part, main_doc);
                append_sc(out, HTML_DOC_ITEM_CLOSE);
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
                append_sc(out, HTML_DOC_HEAD_OPEN"Description"HTML_DOC_HEAD_CLOSE);
                
                append_sc(out, HTML_DOC_ITEM_OPEN);
                print_doc_description(out, part, main_doc);
                append_sc(out, HTML_DOC_ITEM_CLOSE);
            }
            else{
                fprintf(stderr, "warning: no documentation string for %.*s\n", name.size, name.str);
            }
            
            if (item->first_child){
                append_sc(out, HTML_DOC_HEAD_OPEN"Values"HTML_DOC_HEAD_CLOSE);
                
                for (Item_Node *member = item->first_child;
                     member;
                     member = member->next_sibling){
                    Documentation doc = {0};
                    perform_doc_parse(part, member->doc_string, &doc);
                    
                    append_sc(out, "<div>");
                    
                    // NOTE(allen): Dafuq is this all?
                    append_sc(out, "<div><span style='"HTML_CODE_STYLE"'>"HTML_DOC_ITEM_HEAD_INL_OPEN);
                    append_ss(out, member->name);
                    append_sc(out, HTML_DOC_ITEM_HEAD_INL_CLOSE);
                    
                    if (member->value.str){
                        append_sc(out, " = ");
                        append_ss(out, member->value);
                    }
                    
                    append_sc(out, "</span></div>");
                    
                    append_sc(out, "<div style='margin-bottom: 6mm;'>"HTML_DOC_ITEM_OPEN);
                    print_doc_description(out, part, doc.main_doc);
                    append_sc(out, HTML_DOC_ITEM_CLOSE"</div>");
                    
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
                    append_sc(out, HTML_DOC_HEAD_OPEN"Description"HTML_DOC_HEAD_CLOSE);
                    
                    append_sc(out, HTML_DOC_ITEM_OPEN);
                    print_doc_description(out, part, main_doc);
                    append_sc(out, HTML_DOC_ITEM_CLOSE);
                }
                else{
                    fprintf(stderr, "warning: no documentation string for %.*s\n", name.size, name.str);
                }
                
                if (!hide_members){
                    if (item->first_child){
                        append_sc(out, HTML_DOC_HEAD_OPEN"Fields"HTML_DOC_HEAD_CLOSE);
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

static void
doc_item_head_html(String *out, Partition *part, Document_System *doc_system, Used_Links *used_links, Document_Item *item, Section_Counter *section_counter){
    switch (item->type){
        case Doc_Root:
        {
            append_sc(out,
                      "<html lang=\"en-US\">"
                      "<head>"
                      "<link rel='shortcut icon' type='image/x-icon' href='4coder_icon.ico' />"
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
                      "width: ");
            append_int_to_str(out, HTML_WIDTH);
            append_sc(out, "px; text-align: justify; line-height: 1.25;'>");
            
            if (item->section.show_title){
                append_sc(out, "<h1 style='margin-top: 5mm; margin-bottom: 5mm;'>");
                append_ss(out, item->section.name);
                append_sc(out, "</h1>");
            }
        }break;
        
        case Doc_Section:
        {
            html_render_section_header(out, item->section.name, item->section.id, section_counter);
        }break;
        
        case Doc_Todo:
        {
            append_sc(out, "<div><i>Coming Soon</i><div>");
        }break;
        
        case Doc_Enriched_Text:
        {
            write_enriched_text_html(out, part, item->text.text, doc_system, section_counter);
        }break;
        
        case Doc_Element_List:
        {
            append_sc(out, "<ul>");
            
            Meta_Unit *unit = item->unit_elements.unit;
            Alternate_Names_Array *alt_names = item->unit_elements.alt_names;
            int32_t count = unit->set.count;
            
            switch (item->unit_elements.alt_name_type){
                case AltName_Standard:
                {
                    for (int32_t i = 0; i < count; ++i){
                        print_item_in_list(out, unit->set.items[i].name, "_doc");
                    }
                }break;
                
                case AltName_Macro:
                {
                    for (int32_t i = 0; i < count; ++i){
                        print_item_in_list(out, alt_names->names[i].macro, "_doc");
                    }
                }break;
                
                case AltName_Public_Name:
                {
                    for (int32_t i = 0; i < count; ++i){
                        print_item_in_list(out, alt_names->names[i].public_name, "_doc");
                    }
                }break;
            }
            
            append_sc(out, "</ul>");
        }break;
        
        case Doc_Full_Elements:
        {
            Meta_Unit *unit = item->unit_elements.unit;
            Alternate_Names_Array *alt_names = item->unit_elements.alt_names;
            int32_t count = unit->set.count;
            
            char section_space[32];
            String section_str = make_fixed_width_string(section_space);
            append_section_number_reduced(&section_str, section_counter, 1);
            terminate_with_null(&section_str);
            
            if (alt_names){
                int32_t I = 1;
                for (int32_t i = 0; i < count; ++i, ++I){
                    print_item_html(out, part, used_links, &unit->set.items[i], "_doc", section_str.str, I, &alt_names->names[i], item->unit_elements.alt_name_type);
                }
            }
            else{
                int32_t I = 1;
                for (int32_t i = 0; i < count; ++i, ++I){
                    print_item_html(out, part, used_links, &unit->set.items[i], "_doc", section_str.str, I, 0, 0);
                }
            }
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
doc_item_foot_html(String *out, Partition *part, Document_System *doc_system, Used_Links *used_links, Document_Item *item, Section_Counter *section_counter){
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
generate_item_html(String *out, Partition *part, Document_System *doc_system, Used_Links *used_links, Document_Item *item, Section_Counter *section_counter){
    doc_item_head_html(out, part, doc_system, used_links, item, section_counter);
    
    if (item->type == Doc_Root || item->type == Doc_Section){
        int32_t level = ++section_counter->nest_level;
        section_counter->counter[level] = 1;
        for (Document_Item *m = item->section.first_child;
             m != 0;
             m = m->next){
            generate_item_html(out, part, doc_system, used_links, m, section_counter);
        }
        --section_counter->nest_level;
        ++section_counter->counter[section_counter->nest_level];
    }
    
    doc_item_foot_html(out, part, doc_system, used_links, item, section_counter);
}

static void
generate_document_html(String *out, Partition *part, Document_System *doc_system, Abstract_Item *doc){
    assert(doc->root_item != 0);
    
    Used_Links used_links = {0};
    init_used_links(part, &used_links, 4000);
    
    Section_Counter section_counter = {0};
    section_counter.counter[section_counter.nest_level] = 1;
    generate_item_html(out, part, doc_system, &used_links, doc->root_item, &section_counter);
}

#endif

// BOTTOM

