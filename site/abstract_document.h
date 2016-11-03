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

// Document Declaration

struct Enriched_Text{
    int32_t t;
};

enum{
    Doc_Root,
    Doc_Section,
    Doc_Todo,
    Doc_Element_List,
    Doc_Full_Elements,
};

struct Document_Item{
    Document_Item *next;
    int32_t type;
    union{
        struct{
            Document_Item *first_child;
            Document_Item *last_child;
    String name;
    } section;
    
    struct{
        Meta_Unit *unit;
    } unit_elements;
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
begin_document_description(Abstract_Document *doc, Partition *part){
    *doc = null_abstract_document;
    doc->part = part;
    
    doc->root_item = push_struct(doc->part, Document_Item);
    *doc->root_item = null_document_item;
    doc->section_stack[doc->section_top] = doc->root_item;
    doc->root_item->type = Doc_Root;
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
        parent->section.last_child = item;
    }
    else{
        parent->section.last_child->next = item;
        parent->section.last_child = item;
    }
}

static void
begin_section(Abstract_Document *doc, char *title){
    Assert(doc->section_top+1 < ArrayCount(doc->section_stack));
    
    Document_Item *parent = doc->section_stack[doc->section_top];
    Document_Item *section = push_struct(doc->part, Document_Item);
    *section = null_document_item;
    doc->section_stack[++doc->section_top] = section;
    
    section->type = Doc_Section;
    
    int32_t title_len = str_size(title);
    section->section.name = make_string_cap(push_array(doc->part, char, title_len+1), 0, title_len+1);
    partition_align(doc->part, 8);
    append_sc(&section->section.name, title);
    
    append_child(parent, section);
    }

static void
end_section(Abstract_Document *doc){
    Assert(doc->section_top > 0);
    --doc->section_top;
}

static void
add_todo(Abstract_Document *doc){
    Assert(doc->section_top+1 < ArrayCount(doc->section_stack));
    
    Document_Item *parent = doc->section_stack[doc->section_top];
    Document_Item *item = push_struct(doc->part, Document_Item);
    *item = null_document_item;
    item->type = Doc_Todo;
    
    append_child(parent, item);
}

static void
add_element_list(Abstract_Document *doc, Meta_Unit *unit){
    Assert(doc->section_top+1 < ArrayCount(doc->section_stack));
    
    Document_Item *parent = doc->section_stack[doc->section_top];
    Document_Item *item = push_struct(doc->part, Document_Item);
    *item = null_document_item;
    item->type = Doc_Element_List;
    item->unit_elements.unit = unit;
    
    append_child(parent, item);
}

static void
add_full_elements(Abstract_Document *doc, Meta_Unit *unit){
    Assert(doc->section_top+1 < ArrayCount(doc->section_stack));
    
    Document_Item *parent = doc->section_stack[doc->section_top];
    Document_Item *item = push_struct(doc->part, Document_Item);
    *item = null_document_item;
    item->type = Doc_Full_Elements;
    item->unit_elements.unit = unit;
    
    append_child(parent, item);
}

static void
add_enriched_text(Abstract_Document *doc, Enriched_Text *text){
    NotImplemented;
}

// Document Generation

static void
generate_document_html(Out_Context *context, Abstract_Document *doc){
    
}

#endif

// BOTTOM

