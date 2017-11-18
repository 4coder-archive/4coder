/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 25.02.2016
 *
 * Document data structure and generator for 4coder documentation.
 *
 */

// TOP

internal char*
get_null_terminated_version(String str){
    char *ptr = 0;
    if (str.size > 0){
        if (terminate_with_null(&str)){
            ptr = str.str;
        }
        else{
            String b = str_alloc(str.size + 1);
            copy(&b, str);
            terminate_with_null(&b);
            ptr = b.str;
        }
    }
    return(ptr);
}

////////////////////////////////

struct Enriched_Text{
    String fname;
    String source;
};

internal Enriched_Text
load_enriched_text(char *directory, char *filename){
    Enriched_Text result = {0};
    char *fname = fm_str(directory, "/", filename);
    result.fname = str_alloc(str_size(fname) + 1);
    fm_align();
    copy(&result.fname, fname);
    terminate_with_null(&result.fname);
    result.source = file_dump(fname);
    return(result);
}

////////////////////////////////

typedef u32 Mangle_Rule;
enum{
    MangleRule_None,
    MangleRule_MacroSig,
    MangleRule_ToLower,
};

internal Mangle_Rule
get_mangle_rule(String mangle){
    Mangle_Rule result = MangleRule_None;
    if (match(mangle, "macro sig")){
        result = MangleRule_MacroSig;
    }
    else if (match(mangle, "to lower")){
        result = MangleRule_ToLower;
    }
    return(result);
}

internal String
apply_mangle_rule(String name, u32 mangle_rule){
    String result = {0};
    switch (mangle_rule){
        case MangleRule_MacroSig:
        {
            result = str_alloc(name.size + 5);
            fm_align();
            copy(&result, name);
            to_upper(&result);
            append(&result, "_SIG");
            terminate_with_null(&result);
        }break;
        
        case MangleRule_ToLower:
        {
            result = str_alloc(name.size + 1);
            fm_align();
            copy(&result, name);
            to_lower(&result);
            terminate_with_null(&result);
        }break;
        
        default:
        {
            result = name;
        }break;
    }
    
    return(result);
}

////////////////////////////////

enum{
    Doc_Root,
    Doc_Section,
    Doc_Error,
    Doc_Todo,
    Doc_Include,
    Doc_DocList,
    Doc_DocFull,
    Doc_TableOfContents,
    Doc_PlainOldText,
    Doc_Version,
    Doc_Style,
    Doc_DocumentLink,
    Doc_Link,
    Doc_Image,
    Doc_Video,
    Doc_BeginParagraph,
    Doc_EndParagraph,
    Doc_List,
    Doc_Item,
    //
    Doc_COUNT,
};

struct Document_Item{
    Document_Item *next;
    Document_Item *parent;
    i32 type;
    
    struct{
        Document_Item *first_child;
        Document_Item *last_child;
        String name;
        String id;
        b32 show_title;
    } section;
    
    union{
        struct{
            String unit;
            u32 mangle_rule;
        } unit_elements;
        
        struct{
            String string;
            String string2;
        } string;
        
        struct{
            String name;
            struct Abstract_Item *document;
        } include;
    };
};
global Document_Item null_document_item = {0};

////////////////////////////////

internal void
set_item_string(String *out, String text){
    *out = str_alloc(text.size + 1);
    fm_align();
    copy(out, text);
    terminate_with_null(out);
}

////////////////////////////////

struct Basic_Node{
    Basic_Node *next;
};

#define NodeGetData(node, T) ((T*) ((node)+1))

struct Basic_List{
    Basic_Node *head;
    Basic_Node *tail;
    u32 count;
};

internal void
clear_list(Basic_List *list){
    memset(list, 0, sizeof(*list));
}

internal void*
push_item_on_list(Basic_List *list, i32 item_size){
    i32 mem_size = item_size + sizeof(Basic_Node);
    void *mem = fm__push(mem_size);
    Assert(mem != 0);
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
    ++list->count;
    
    void *result = (node + 1);
    return(result);
}

////////////////////////////////

enum{
    ItemType_Document,
    ItemType_Image,
    ItemType_GenericFile,
    ItemType_MetaUnit,
    //
    ItemType_COUNT,
};

struct Abstract_Item{
    i32 item_type;
    char *name;
    
    // Document value members
    Document_Item *root_item;
    
    // Image value members
    char *source_file;
    char *extension;
    float w_h_ratio;
    float h_w_ratio;
    Basic_List img_instantiations;
    
    // Meta parse members
    Meta_Unit *unit;
};
global Abstract_Item null_abstract_item = {0};

internal Abstract_Item*
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

internal Abstract_Item*
create_abstract_item(Basic_List *list, char *name){
    Abstract_Item *result = 0;
    Abstract_Item *lookup = get_item_by_name(*list, make_string_slowly(name));
    if (lookup == 0){
        result = (Abstract_Item*)push_item_on_list(list, sizeof(*result));
    }
    return(result);
}

struct Abstract_Item_Array{
    Abstract_Item **items;
    u32 count;
};

internal Abstract_Item_Array
get_abstract_item_array(Basic_List *list){
    Abstract_Item_Array result = {0};
    
    result.items = (Abstract_Item**)fm_push_array(Abstract_Item*, list->count);
    result.count = list->count;
    
    u32 i = 0;
    for (Basic_Node *node = list->head;
         node != 0;
         node = node->next){
        result.items[i++] = NodeGetData(node, Abstract_Item);
    }
    
    return(result);
}

////////////////////////////////

struct Document_System{
    char *code_dir;
    char *asset_dir;
    char *src_dir;
    
    Basic_List doc_list;
    Basic_List img_list;
    Basic_List file_list;
    Basic_List meta_list;
    
    Basic_List unresolved_includes;
};

internal Document_System
create_document_system(char *code_dir, char *asset_dir, char *src_dir){
    Document_System system = {0};
    system.code_dir = code_dir;
    system.asset_dir = asset_dir;
    system.src_dir = src_dir;
    return(system);
}

internal void
create_unresolved_include(Document_System *doc_system, Document_Item *include_item){
    Document_Item **new_item = (Document_Item**)push_item_on_list(&doc_system->unresolved_includes, sizeof(*new_item));
    *new_item = include_item;
}

////////////////////////////////

enum{
    MetaResult_DidParse,
    MetaResult_AlreadyExists,
    MetaResult_FailedToParse,
};

internal u32
create_meta_unit(Document_System *doc_system, String name_str, String file_str){
    u32 result = MetaResult_DidParse;
    
    char *name = get_null_terminated_version(name_str);
    char *file = get_null_terminated_version(file_str);
    
    Abstract_Item *item = create_abstract_item(&doc_system->meta_list, name);
    
    if (item != 0){
        Meta_Unit *unit = fm_push_array(Meta_Unit, 1);
        *unit = compile_meta_unit(doc_system->code_dir, file, ExpandArray(meta_keywords));
        
        if (unit->count != 0){
            result = true;
            item->item_type = ItemType_MetaUnit;
            item->name = name;
            item->unit = unit;
        }
        else{
            result = MetaResult_FailedToParse;
        }
    }
    else{
        result = MetaResult_AlreadyExists;
    }
    
    return(result);
}

internal Abstract_Item*
add_generic_file(Document_System *system, char *source_file, char *extension, char *name){
    Abstract_Item *item = create_abstract_item(&system->file_list, name);
    if (item){
        char *full_file = fm_str(system->asset_dir, "/", source_file);
        
        item->item_type = ItemType_GenericFile;
        item->extension = extension;
        item->source_file = full_file;
        item->name = name;
    }
    return(item);
}

internal Abstract_Item*
add_image_description(Document_System *system, char *source_file, char *extension, char *name){
    Abstract_Item *item = create_abstract_item(&system->img_list, name);
    if (item != 0){
        char *full_file = fm_str(system->asset_dir, "/", source_file);
        
        item->item_type = ItemType_Image;
        item->name = name;
        item->extension = extension;
        item->source_file = full_file;
        
        i32 w = 0, h = 0, comp = 0;
        i32 stbi_r = stbi_info(full_file, &w, &h, &comp);
        if (!stbi_r){
            fprintf(stdout, "Did not find file %s\n", full_file);
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

////////////////////////////////

struct Image_Instantiation{
    i32 w, h;
};

internal Image_Instantiation*
get_image_instantiation(Basic_List list, i32 w, i32 h){
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

internal void
add_image_instantiation(Basic_List *list, i32 w, i32 h){
    Image_Instantiation *instantiation = (Image_Instantiation*)push_item_on_list(list, sizeof(*instantiation));
    instantiation->w = w;
    instantiation->h = h;
}

////////////////////////////////

struct Document_Builder{
    Abstract_Item *doc;
    Document_Item *item_stack[512];
    i32 item_top;
};

internal Document_Builder
begin_document_description(Document_System *system, char *title, char *name, b32 show_title){
    Document_Builder builder = {0};
    Abstract_Item *doc = create_abstract_item(&system->doc_list, name);
    if (doc != 0){
        builder.doc = doc;
        
        *doc = null_abstract_item;
        doc->item_type = ItemType_Document;
        
        doc->name = name;
        doc->root_item = fm_push_array(Document_Item, 1);
        *doc->root_item = null_document_item;
        
        Document_Item *item = doc->root_item;
        set_item_string(&item->section.name, make_string_slowly(name));
        item->section.show_title = show_title;
        item->type = Doc_Root;
        
        builder.item_stack[builder.item_top] = doc->root_item;
    }
    return(builder);
}

internal void
append_child(Document_Item *parent, Document_Item *item){
    if (parent->section.last_child == 0){
        parent->section.first_child = item;
    }
    else{
        parent->section.last_child->next = item;
    }
    parent->section.last_child = item;
    item->parent = parent;
}

#define PUSH true

internal void
doc_push(Document_Builder *builder, Document_Item *item){
    Assert(builder->item_top + 1 < ArrayCount(builder->item_stack));
    builder->item_stack[++builder->item_top] = item;
}

internal Document_Item*
doc_get_item_top(Document_Builder *builder){
    Assert(builder->item_top < ArrayCount(builder->item_stack));
    Document_Item *parent = builder->item_stack[builder->item_top];
    return(parent);
}

internal Document_Item*
doc_new_item(Document_Builder *builder, u32 type, b32 push = false){
    Document_Item *parent = doc_get_item_top(builder);
    Document_Item *item = fm_push_array(Document_Item, 1);
    *item = null_document_item;
    item->type = type;
    append_child(parent, item);
    if (push){
        doc_push(builder, item);
    }
    return(item);
}

internal Document_Item*
doc_new_item_strings(Document_Builder *builder, u32 type, String s1, String s2, b32 push = false){
    Document_Item *item = doc_new_item(builder, type);
    if (s1.size > 0){
        set_item_string(&item->string.string, s1);
    }
    if (s2.size > 0){
        set_item_string(&item->string.string2, s2);
    }
    if (push){
        doc_push(builder, item);
    }
    return(item);
}

internal Document_Item*
doc_new_item_documentation(Document_Builder *builder, u32 type, String unit, Mangle_Rule mangle_rule, b32 push = false){
    Document_Item *item = doc_new_item(builder, type);
    set_item_string(&item->unit_elements.unit, unit);
    item->unit_elements.mangle_rule = mangle_rule;
    if (push){
        doc_push(builder, item);
    }
    return(item);
}

internal void
begin_section(Document_Builder *builder, char *title, char *id){
    Document_Item *item = doc_new_item(builder, Doc_Section, PUSH);
    set_item_string(&item->section.name, make_string_slowly(title));
    item->section.show_title = true;
    if (id != 0){
        set_item_string(&item->section.id, make_lit_string(id));
    }
}

#define doc_end(b) doc_pop(b)

#define begin_style(b,t)   doc_new_item_strings(b, Doc_Style, t, null_string, PUSH)
#define begin_link(b,t)    doc_new_item_strings(b, Doc_Link, t, null_string, PUSH);
#define begin_list(b)      doc_new_item(b, Doc_List, PUSH)
#define begin_item(b)      doc_new_item(b, Doc_Item, PUSH)

internal void
add_include(Document_System *doc_system, Document_Builder *builder, String text){
    Document_Item *item = doc_new_item(builder, Doc_Include);
    set_item_string(&item->include.name, text);
    create_unresolved_include(doc_system, item);
}

#define add_error(b,t)           doc_new_item_strings(b, Doc_Error, t, null_string)
#define add_todo(b)              doc_new_item(b, Doc_Todo)
#define add_doc_list(b,u,m)      doc_new_item_documentation(b, Doc_DocList, u,  m)
#define add_doc_full(b,u,m)      doc_new_item_documentation(b, Doc_DocFull, u,  m)
#define add_table_of_contents(b) doc_new_item(b, Doc_TableOfContents)
#define add_plain_old_text(b,t)  doc_new_item_strings(b, Doc_PlainOldText, t, null_string);
#define add_version(b)           doc_new_item(b, Doc_Version)
#define add_document_link(b,t)   doc_new_item_strings(b, Doc_DocumentLink, t, null_string)
#define add_image(b,t,e)         doc_new_item_strings(b, Doc_Image, t, e)
#define add_video(b,t)           doc_new_item_strings(b, Doc_Video, t, null_string)
#define add_begin_paragraph(b)   doc_new_item(b, Doc_BeginParagraph)
#define add_end_paragraph(b)     doc_new_item(b, Doc_EndParagraph)

internal void
doc_pop(Document_Builder *builder){
    if (builder->item_top > 0){
        --builder->item_top;
    }
    else{
        add_error(builder, make_lit_string("unbalanced groups -- extra end"));
    }
}

internal void
end_document_description(Document_Builder *builder){
    b32 closing_error = (builder->item_top != 0);
    if (closing_error){
        add_error(builder, make_lit_string("unbalanced groups -- extra begin"));
    }
    
    for (;builder->item_top > 0;){
        doc_end(builder);
    }
}

////////////////////////////////

internal void
report_error_missing_body(Document_Builder *builder, String command_body){
    char space[512];
    String error_string = make_fixed_width_string(space);
    append(&error_string, "missing body for ");
    append(&error_string, command_body);
    add_error(builder, error_string);
}

////////////////////////////////

enum Command_Types{
    Cmd_BackSlash,
    Cmd_End,
    Cmd_Section,
    Cmd_Style,
    Cmd_List,
    Cmd_Item,
    Cmd_Link,
    Cmd_DocumentLink,
    Cmd_Image,
    Cmd_Video,
    Cmd_Version,
    Cmd_TableOfContents,
    Cmd_Todo,
    Cmd_Include,
    Cmd_MetaParse,
    Cmd_DocList,
    Cmd_DocFull,
    // never below this
    Cmd_COUNT,
};

global b32 did_enriched_commands = false;
global String enriched_commands_global_array[Cmd_COUNT];

internal String*
get_enriched_commands(){
    if (!did_enriched_commands){
        did_enriched_commands = true;
        enriched_commands_global_array[Cmd_BackSlash]       = make_lit_string("\\");
        enriched_commands_global_array[Cmd_End]             = make_lit_string("END");
        enriched_commands_global_array[Cmd_Section]         = make_lit_string("SECTION");
        enriched_commands_global_array[Cmd_Style]           = make_lit_string("STYLE");
        enriched_commands_global_array[Cmd_List]            = make_lit_string("LIST");
        enriched_commands_global_array[Cmd_Item]            = make_lit_string("ITEM");
        enriched_commands_global_array[Cmd_Link]            = make_lit_string("LINK");
        enriched_commands_global_array[Cmd_DocumentLink]    = make_lit_string("DOC_LINK");
        enriched_commands_global_array[Cmd_Image]           = make_lit_string("IMAGE");
        enriched_commands_global_array[Cmd_Video]           = make_lit_string("VIDEO");
        enriched_commands_global_array[Cmd_Version]         = make_lit_string("VERSION");
        enriched_commands_global_array[Cmd_TableOfContents] = make_lit_string("TABLE_OF_CONTENTS");
        enriched_commands_global_array[Cmd_Todo]            = make_lit_string("TODO");
        enriched_commands_global_array[Cmd_Include]         = make_lit_string("INCLUDE");
        enriched_commands_global_array[Cmd_MetaParse]       = make_lit_string("META_PARSE");
        enriched_commands_global_array[Cmd_DocList]         = make_lit_string("DOC_LIST");
        enriched_commands_global_array[Cmd_DocFull]         = make_lit_string("DOC_FULL");
    }
    return(enriched_commands_global_array);
}

internal u32
get_enriched_commands_count(){
    return(ArrayCount(enriched_commands_global_array));
}

internal b32
extract_command_body(String l, i32 *i_in_out, String *body_text_out){
    b32 has_body = false;
    i32 i = *i_in_out;
    for (; i < l.size; ++i){
        if (!char_is_whitespace(l.str[i])){
            break;
        }
    }
    
    if (l.str[i] == '{'){
        i32 body_start = i + 1;
        i32 body_end = 0;
        for (++i; i < l.size; ++i){
            if (l.str[i] == '}'){
                has_body = true;
                body_end = i;
                ++i;
                break;
            }
        }
        
        if (has_body){
            *i_in_out = i;
            
            String body_text = substr(l, body_start, body_end - body_start);
            *body_text_out = skip_chop_whitespace(body_text);
        }
    }
    
    return(has_body);
}

internal Abstract_Item*
make_document_from_text(Document_System *doc_system, char *title, char *name, Enriched_Text *text){
    String source = text->source;
    Document_Builder builder = begin_document_description(doc_system, title, name, false);
    
    if (source.str == 0){
        char space[512];
        String str = make_fixed_width_string(space);
        copy(&str, "could not open source file ");
        copy(&str, text->fname);
        add_error(&builder, str);
    }
    else{
        for (String line = get_first_double_line(source);
             line.str;
             line = get_next_double_line(source, line)){
            String l = skip_chop_whitespace(line);
            if (l.size == 0) continue;
            
            add_begin_paragraph(&builder);
            
            i32 start = 0, i = 0;
            for (; i < l.size; ++i){
                char ch = l.str[i];
                if (ch == '\\'){
                    add_plain_old_text(&builder, substr(l, start, i - start));
                    
                    i32 command_start = i + 1;
                    i32 command_end = command_start;
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
                    
                    String *enriched_commands = get_enriched_commands();
                    u32 enriched_commands_count = get_enriched_commands_count();
                    
                    i = command_end;
                    
                    i32 match_index = 0;
                    if (!string_set_match(enriched_commands, enriched_commands_count, command_string, &match_index)){
                        match_index = -1;
                    }
                    
                    switch (match_index){
                        case Cmd_BackSlash:
                        {
                            add_plain_old_text(&builder, make_lit_string("\\"));
                        }break;
                        
                        case Cmd_End:
                        {
                            for (Document_Item *top = doc_get_item_top(&builder);
                                 top->type == Doc_Item;
                                 top = doc_get_item_top(&builder)){
                                doc_end(&builder);
                            }
                            doc_end(&builder);
                        }break;
                        
                        case Cmd_Section:
                        {
                            String body_text = {0};
                            if (extract_command_body(l, &i, &body_text)){
                                String extra_text = {0};
                                extract_command_body(l, &i, &extra_text);
                                
                                char *title = get_null_terminated_version(body_text);
                                char *id = get_null_terminated_version(extra_text);
                                
                                begin_section(&builder, title, id);
                            }
                            else{
                                report_error_missing_body(&builder, command_string);
                            }
                        }break;
                        
                        case Cmd_Style:
                        {
                            String body_text = {0};
                            if (extract_command_body(l, &i, &body_text)){
                                begin_style(&builder, body_text);
                            }
                            else{
                                report_error_missing_body(&builder, command_string);
                            }
                        }break;
                        
                        case Cmd_List:
                        {
                            begin_list(&builder);
                        }break;
                        
                        case Cmd_Item:
                        {
                            Document_Item *top = doc_get_item_top(&builder);
                            if (top->type == Doc_Item){
                                doc_end(&builder);
                            }
                            begin_item(&builder);
                        }break;
                        
                        case Cmd_Link:
                        {
                            String body_text = {0};
                            if (extract_command_body(l, &i, &body_text)){
                                begin_link(&builder, body_text);
                            }
                            else{
                                report_error_missing_body(&builder, command_string);
                            }
                        }break;
                        
                        // TODO(allen): upgrade this bs
                        case Cmd_DocumentLink:
                        {
                            String body_text = {0};
                            if (extract_command_body(l, &i, &body_text)){
                                add_document_link(&builder, body_text);
                            }
                            else{
                                report_error_missing_body(&builder, command_string);
                            }
                        }break;
                        
                        case Cmd_Image:
                        {
                            String body_text = {0};
                            if (extract_command_body(l, &i, &body_text)){
                                String size_parameter = {0};
                                extract_command_body(l, &i, &size_parameter);
                                add_image(&builder, body_text, size_parameter);
                            }
                            else{
                                report_error_missing_body(&builder, command_string);
                            }
                        }break;
                        
                        case Cmd_Video:
                        {
                            String body_text = {0};
                            if (extract_command_body(l, &i, &body_text)){
                                add_video(&builder, body_text);
                            }
                            else{
                                report_error_missing_body(&builder, command_string);
                            }
                        }break;
                        
                        case Cmd_Version:
                        {
                            add_version(&builder);
                        }break;
                        
                        case Cmd_TableOfContents:
                        {
                            add_table_of_contents(&builder);
                        }break;
                        
                        case Cmd_Todo:
                        {
                            add_todo(&builder);
                        }break;
                        
                        case Cmd_Include:
                        {
                            String body_text = {0};
                            if (extract_command_body(l, &i, &body_text)){
                                add_include(doc_system, &builder, body_text);
                            }
                            else{
                                report_error_missing_body(&builder, command_string);
                            }
                        }break;
                        
                        case Cmd_MetaParse:
                        {
                            String name = {0};
                            String file = {0};
                            if (extract_command_body(l, &i, &name)){
                                if (extract_command_body(l, &i, &file)){
                                    u32 result = create_meta_unit(doc_system, name, file);
                                    if (result == MetaResult_FailedToParse){
                                        char space[512];
                                        String str = make_fixed_width_string(space);
                                        append(&str, "parse failed for ");
                                        append(&str, file);
                                        add_error(&builder, str);
                                    }
                                }
                                else{
                                    report_error_missing_body(&builder, command_string);
                                }
                            }
                            else{
                                report_error_missing_body(&builder, command_string);
                            }
                        }break;
                        
                        case Cmd_DocList:
                        case Cmd_DocFull:
                        {
                            String name = {0};
                            if (extract_command_body(l, &i, &name)){
                                String mangle = {0};
                                extract_command_body(l, &i, &mangle);
                                
                                u32 mangle_rule = MangleRule_None;
                                if (match_part(mangle, "mangle:")){
                                    String mangle_name = substr_tail(mangle, sizeof("mangle:")-1);
                                    mangle_name = skip_chop_whitespace(mangle_name);
                                    mangle_rule = get_mangle_rule(mangle_name);
                                }
                                
                                if (match_index == Cmd_DocList){
                                    add_doc_list(&builder, name, mangle_rule);
                                }
                                else{
                                    add_doc_full(&builder, name, mangle_rule);
                                }
                                
                            }
                            else{
                                report_error_missing_body(&builder, command_string);
                            }
                        }break;
                        
                        default:
                        {
                            char space[512];
                            String error = make_fixed_width_string(space);
                            append(&error, "unrecognized command ");
                            append(&error, command_string);
                            add_error(&builder, error);
                        }break;
                    }
                    
                    start = i;
                }
            }
            
            if (start != i){
                add_plain_old_text(&builder, substr(l, start, i - start));
            }
            
            add_end_paragraph(&builder);
        }
    }
    
    end_document_description(&builder);
    return(builder.doc);
}

////////////////////////////////

struct Unresolved_Include_Array{
    Document_Item **items;
    u32 count;
};

internal Unresolved_Include_Array
get_unresolved_includes(Document_System *doc_system){
    Unresolved_Include_Array result = {0};
    
    Basic_List *list = &doc_system->unresolved_includes;
    result.items = (Document_Item**)fm_push_array(Document_Item*, list->count);
    result.count = list->count;
    
    u32 i = 0;
    for (Basic_Node *node = list->head;
         node != 0;
         node = node->next){
        result.items[i++] = *NodeGetData(node, Document_Item*);
    }
    
    return(result);
}

internal void
resolve_all_includes(Document_System *doc_system){
    for (;doc_system->unresolved_includes.count > 0;){
        Unresolved_Include_Array includes = get_unresolved_includes(doc_system);
        clear_list(&doc_system->unresolved_includes);
        
        Document_Item **item_ptr = includes.items;
        for (u32 i = 0; i < includes.count; ++i, ++item_ptr){
            Document_Item *item = *item_ptr;
            Assert(item->include.document == 0);
            Abstract_Item *inc_doc = get_item_by_name(doc_system->doc_list, item->include.name);
            if (inc_doc == 0){
                String source_text = item->include.name;
                Enriched_Text *text = fm_push_array(Enriched_Text, 1);
                *text = load_enriched_text(doc_system->src_dir, source_text.str);
                inc_doc = make_document_from_text(doc_system, source_text.str, source_text.str, text);
            }
            item->include.document = inc_doc;
        }
    }
}

////////////////////////////////

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
    i32 counter[16];
    i32 nest_level;
    i32 list_item_counter;
};

internal b32
doc_get_link_string(Abstract_Item *doc, char *space, i32 capacity){
    String str = make_string_cap(space, 0, capacity);
    append(&str, doc->name);
    append(&str, ".html");
    b32 result = terminate_with_null(&str);
    return(result);
}

internal b32
img_get_link_string(Abstract_Item *img, char *space, i32 capacity, i32 w, i32 h){
    String str = make_string_cap(space, 0, capacity);
    append(&str, img->name);
    
    append(&str, "_");
    append_int_to_str(&str, w);
    append(&str, "_");
    append_int_to_str(&str, h);
    
    append(&str, ".");
    append(&str, img->extension);
    b32 result = terminate_with_null(&str);
    return(result);
}

internal void
append_section_number_reduced(String *out, Section_Counter *section_counter, i32 reduce){
    i32 level = section_counter->nest_level - reduce;
    for (i32 i = 0; i <= level; ++i){
        append_int_to_str(out, section_counter->counter[i]);
        if (i != level){
            append(out, ".");
        }
    }
}

internal void
append_section_number(String *out, Section_Counter *section_counter){
    append_section_number_reduced(out, section_counter, 0);
}

#define ERROR_HTML_START "<span style='color:#F00'>! generator error: "
#define ERROR_HTML_END   " !</span>"

internal void
output_error(String *out, String error){
    append(out, ERROR_HTML_START);
    append(out, error);
    append(out, ERROR_HTML_END);
    fprintf(stdout, "error: %.*s\n", error.size, error.str);
}

internal void
report_error_html_missing_body(String *out, String command_name){
    char space[512];
    String str = make_fixed_width_string(space);
    append(&str, "missing body for ");
    append(&str, command_name);
    output_error(out, str);
}

internal void
html_render_section_header(String *out, String section_name, String section_id, Section_Counter *section_counter){
    if (section_counter->nest_level <= 1){
        if (section_id.size > 0){
            append(out, "\n<h2 id='section_");
            append(out, section_id);
            append(out, "'>&sect;");
        }
        else{
            append(out, "\n<h2>&sect;");
        }
        append_section_number(out, section_counter);
        append(out, " ");
        append(out, section_name);
        append(out, "</h2>");
    }
    else{
        if (section_id.size > 0){
            append(out, "<h3 id='section_");
            append(out, section_id);
            append(out, "'>&sect;");
        }
        else{
            append(out, "<h3>&sect;");
        }
        append_section_number(out, section_counter);
        append(out, " ");
        append(out, section_name);
        append(out, "</h3>");
    }
}

#define HTML_WIDTH 800

internal void
output_plain_old_text(String *out, String l){
    u32 start = 0;
    u32 i = 0;
    for (; i < (u32)l.size; ++i){
        char ch = l.str[i];
        switch (ch){
            case '<':
            {
                append(out, substr(l, start, i - start));
                append(out, "&lt;");
                start = i + 1;
            }break;
            
            case '>':
            {
                append(out, substr(l, start, i - start));
                append(out, "&gt;");
                start = i + 1;
            }break;
        }
    }
    if (start != i){
        append(out, substr(l, start, i - start));
    }
}

internal void
output_begin_style(String *out, String l){
    if (match(l, "code")){
        append(out, "<span style='"HTML_CODE_STYLE"'>");
    }
    else{
        fprintf(stdout, "error: unrecognized style\n");
        append(out, "<span>");
    }
}

internal void
output_end_style(String *out){
    append(out, "</span>");
}

internal void
output_document_link(String *out, String l){
    append(out, "<a href='#");
    append(out, l);
    append(out, "_doc'>");
    append(out, l);
    append(out, "</a>");
}

internal void
output_begin_link(Document_System *doc_system, String *out, String l){
    append(out, "<a ");
    if (l.str[0] == '!'){
        append(out, "target='_blank' ");
        l.str++;
        l.size--;
    }
    append(out, "href='");
    if (match_part(l, "document:")){
        String doc_name = substr_tail(l, sizeof("document:")-1);
        Abstract_Item *doc_lookup = get_item_by_name(doc_system->doc_list, doc_name);
        if (doc_lookup){
            char space[256];
            if (doc_get_link_string(doc_lookup, space, sizeof(space))){
                append(out, space);
            }
            else{
                NotImplemented;
            }
        }
    }
    else{
        append(out, l);
    }
    append(out, "'>");
}

internal void
output_end_link(String *out){
    append(out, "</a>");
}

internal void
output_image(Document_System *doc_system, String *out, String l, String l2){
    i32 pixel_height = 10;
    i32 pixel_width = HTML_WIDTH;
    
    if (l2.size > 0){
        if (match_part(l2, "width:")){
            String width_string = substr_tail(l2, sizeof("width:")-1);
            if (str_is_int(width_string)){
                pixel_width = str_to_int(width_string);
            }
        }
    }
    
    if (match_part(l, "image:")){
        String img_name = substr_tail(l, sizeof("image:")-1);
        Abstract_Item *img_lookup = get_item_by_name(doc_system->img_list, img_name);
        
        if (img_lookup){
            pixel_height = ceil32(pixel_width*img_lookup->h_w_ratio);
            
            append(out, "<img src='");
            
            char space[256];
            if (img_get_link_string(img_lookup, space, sizeof(space), pixel_width, pixel_height)){
                append(out, space);
                add_image_instantiation(&img_lookup->img_instantiations, pixel_width, pixel_height);
            }
            else{
                NotImplemented;
            }
            
            append(out, "' style='width: ");
            append_int_to_str(out, pixel_width);
            append(out, "px; height: ");
            append_int_to_str(out, pixel_height);
            append(out, "px;'>");
        }
    }
}

internal void
output_video(String *out, String l){
    if (match_part(l, "youtube:")){
        i32 pixel_width = HTML_WIDTH;
        i32 pixel_height = (i32)(pixel_width * 0.5625f);
        
        String youtube_str = substr_tail(l, sizeof("youtube:")-1);
        
        append(out, "<iframe  width='");
        append_int_to_str(out, pixel_width);
        append(out, "' height='");
        append_int_to_str(out, pixel_height);
        append(out, "' src='https://www.youtube.com/embed/");
        append(out, youtube_str);
        append(out, "' allowfullscreen> </iframe>");
    }
    else{
        char space[512];
        String str = make_fixed_width_string(space);
        append(&str, "unrecognized video type ");
        append(&str, l);
        output_error(out, str);
    }
}

internal void
output_begin_paragraph(String *out){
    append(out, "<p>");
}

internal void
output_end_paragraph(String *out){
    append(out, "</p>");
}

internal void
output_begin_list(String *out){
    append(out,"<ul style='margin-top: 5mm; margin-left: 1mm;'>");
}

internal void
output_end_list(String *out){
    append(out, "</ul>");
}

internal void
output_begin_item(String *out, Section_Counter *section_counter){
    if (section_counter->list_item_counter == 0){
        append(out, "<li style='font-size: 95%; background: #EFEFDF;'>");
        section_counter->list_item_counter = 1;
    }
    else{
        append(out, "<li style='font-size: 95%;'>");
        section_counter->list_item_counter = 0;
    }
}

internal void
output_end_item(String *out){
    append(out, "</li>");
}

internal void
print_item_in_list(String *out, String name, char *id_postfix){
    append(out, "<li><a href='#");
    append(out, name);
    append(out, id_postfix);
    append(out, "'>");
    append(out, name);
    append(out, "</a></li>");
}

internal void
init_used_links(Used_Links *used, i32 count){
    used->strs = fm_push_array(String, count);
    used->count = 0;
    used->max = count;
}

internal b32
try_to_use_link(Used_Links *used, String str){
    b32 result = true;
    i32 index = 0;
    if (string_set_match(used->strs, used->count, str, &index)){
        result = false;
    }
    else{
        Assert(used->count < used->max);
        used->strs[used->count++] = str;
    }
    return(result);
}

internal void
print_struct_html(String *out, Item_Node *member, i32 hide_children){
    String name = member->name;
    String type = member->type;
    String type_postfix = member->type_postfix;
    
    append     (out, type);
    append_s_char (out, ' ');
    append     (out, name);
    append     (out, type_postfix);
    
    if (match_ss(type, make_lit_string("struct")) ||
        match_ss(type, make_lit_string("union"))){
        
        if (hide_children){
            append(out, " { /* non-public internals */ } ;");
        }
        else{
            append(out, " {<br><div style='margin-left: 8mm;'>");
            
            for (Item_Node *member_iter = member->first_child;
                 member_iter != 0;
                 member_iter = member_iter->next_sibling){
                print_struct_html(out, member_iter, hide_children);
            }
            
            append(out, "</div>};<br>");
        }
    }
    else{
        append(out, ";<br>");
    }
}

internal void
print_function_html(String *out, Used_Links *used, String cpp_name, String ret, char *function_call_head, String name, Argument_Breakdown breakdown){
    
    append     (out, ret);
    append_s_char (out, ' ');
    append     (out, function_call_head);
    append     (out, name);
    
    if (breakdown.count == 0){
        append(out, "()");
    }
    else if (breakdown.count == 1){
        append(out, "(");
        append(out, breakdown.args[0].param_string);
        append(out, ")");
    }
    else{
        append(out, "(<div style='margin-left: 4mm;'>");
        
        for (i32 j = 0; j < breakdown.count; ++j){
            append(out, breakdown.args[j].param_string);
            if (j < breakdown.count - 1){
                append_s_char(out, ',');
            }
            append(out, "<br>");
        }
        
        append(out, "</div>)");
    }
}

internal void
print_macro_html(String *out, String name, Argument_Breakdown breakdown){
    
    append (out, "#define ");
    append (out, name);
    
    if (breakdown.count == 0){
        append(out, "()");
    }
    else if (breakdown.count == 1){
        append_s_char  (out, '(');
        append      (out, breakdown.args[0].param_string);
        append_s_char  (out, ')');
    }
    else{
        append (out, "(<div style='margin-left: 4mm;'>");
        
        for (i32 j = 0; j < breakdown.count; ++j){
            append(out, breakdown.args[j].param_string);
            if (j < breakdown.count - 1){
                append_s_char(out, ',');
            }
            append(out, "<br>");
        }
        
        append(out, ")</div>)");
    }
}

enum Doc_Chunk_Type{
    DocChunk_PlainText,
    DocChunk_CodeExample,
    
    DocChunk_Count
};

global String doc_chunk_headers[] = {
    make_lit_string(""),
    make_lit_string("CODE_EXAMPLE"),
};

internal String
get_next_doc_chunk(String source, String prev_chunk, Doc_Chunk_Type *type){
    String chunk = {0};
    String word = {0};
    i32 pos = source.size;
    i32 word_index = 0;
    Doc_Chunk_Type t = DocChunk_PlainText;
    
    i32 start_pos = (i32)(prev_chunk.str - source.str) + prev_chunk.size;
    String source_tail = substr_tail(source, start_pos);
    
    Assert(DocChunk_Count == ArrayCount(doc_chunk_headers));
    
    for (word = get_first_word(source_tail);
         word.str;
         word = get_next_word(source_tail, word), ++word_index){
        
        for (i32 i = 1; i < DocChunk_Count; ++i){
            if (match_ss(word, doc_chunk_headers[i])){
                pos = (i32)(word.str - source.str);
                t = (Doc_Chunk_Type)i;
                goto doublebreak;
            }
        }
    }
    doublebreak:;
    
    *type = DocChunk_PlainText;
    if (word_index == 0){
        *type = t;
        
        i32 nest_level = 1;
        i32 i = find_s_char(source, pos, '(');
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
    
    i32 is_all_white = 1;
    for (i32 i = 0; i < chunk.size; ++i){
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

internal String
get_first_doc_chunk(String source, Doc_Chunk_Type *type){
    String start_str = make_string(source.str, 0);
    String chunk = get_next_doc_chunk(source, start_str, type);
    return(chunk);
}

internal void
print_doc_description(String *out, String src){
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
                    append(out, line);
                    append(out, "<br><br>");
                }
            }break;
            
            case DocChunk_CodeExample:
            {
                i32 start = 0;
                i32 end = chunk.size-1;
                while (start < end && chunk.str[start] != '(') ++start;
                start += 1;
                while (end > start && chunk.str[end] != ')') --end;
                
                
                append(out, HTML_EXAMPLE_CODE_OPEN);
                
                if (start < end){
                    String code_example = substr(chunk, start, end - start);
                    i32 first_line = 1;
                    
                    for (String line = get_first_line(code_example);
                         line.str;
                         line = get_next_line(code_example, line)){
                        
                        if (!(first_line && line.size == 0)){
                            i32 space_i = 0;
                            for (; space_i < line.size; ++space_i){
                                if (line.str[space_i] == ' '){
                                    append(out, "&nbsp;");
                                }
                                else{
                                    break;
                                }
                            }
                            
                            String line_tail = substr_tail(line, space_i);
                            append(out, line_tail);
                            append(out, "<br>");
                        }
                        first_line = 0;
                    }
                }
                
                append(out, HTML_EXAMPLE_CODE_CLOSE);
            }break;
        }
    }
}

internal void
print_struct_docs(String *out, Item_Node *member){
    for (Item_Node *member_iter = member->first_child;
         member_iter != 0;
         member_iter = member_iter->next_sibling){
        String type = member_iter->type;
        if (match_ss(type, make_lit_string("struct")) ||
            match_ss(type, make_lit_string("union"))){
            print_struct_docs(out, member_iter);
        }
        else{
            Documentation doc = {0};
            perform_doc_parse(member_iter->doc_string, &doc);
            
            append(out, "<div>");
            
            append(out, "<div style='"HTML_CODE_STYLE"'>"HTML_DOC_ITEM_HEAD_INL_OPEN);
            append(out, member_iter->name);
            append(out, HTML_DOC_ITEM_HEAD_INL_CLOSE"</div>");
            
            append(out, "<div style='margin-bottom: 6mm;'>"HTML_DOC_ITEM_OPEN);
            print_doc_description(out, doc.main_doc);
            append(out, HTML_DOC_ITEM_CLOSE"</div>");
            
            append(out, "</div>");
        }
    }
}

internal void
print_see_also(String *out, Documentation *doc){
    i32 doc_see_count = doc->see_also_count;
    if (doc_see_count > 0){
        append(out, HTML_DOC_HEAD_OPEN"See Also"HTML_DOC_HEAD_CLOSE);
        
        for (i32 j = 0; j < doc_see_count; ++j){
            String see_also = doc->see_also[j];
            append(out, HTML_DOC_ITEM_OPEN"<a href='#");
            append(out, see_also);
            append(out, "_doc'>");
            append(out, see_also);
            append(out, "</a>"HTML_DOC_ITEM_CLOSE);
        }
    }
}

internal void
print_function_docs(String *out, String name, String doc_string){
    if (doc_string.size == 0){
        append(out, "No documentation generated for this function.");
        fprintf(stdout, "warning: no documentation string for %.*s\n", name.size, name.str);
    }
    
    Temp temp = fm_begin_temp();
    
    Documentation doc = {0};
    
    perform_doc_parse(doc_string, &doc);
    
    i32 doc_param_count = doc.param_count;
    if (doc_param_count > 0){
        append(out, HTML_DOC_HEAD_OPEN"Parameters"HTML_DOC_HEAD_CLOSE);
        
        for (i32 j = 0; j < doc_param_count; ++j){
            String param_name = doc.param_name[j];
            String param_docs = doc.param_docs[j];
            
            // TODO(allen): check that param_name is actually a parameter to this function!
            
            append(out, "<div>"HTML_DOC_ITEM_HEAD_OPEN);
            append(out, param_name);
            append(out, HTML_DOC_ITEM_HEAD_CLOSE"<div style='margin-bottom: 6mm;'>"HTML_DOC_ITEM_OPEN);
            append(out, param_docs);
            append(out, HTML_DOC_ITEM_CLOSE"</div></div>");
        }
    }
    
    String ret_doc = doc.return_doc;
    if (ret_doc.size != 0){
        append(out, HTML_DOC_HEAD_OPEN"Return"HTML_DOC_HEAD_CLOSE HTML_DOC_ITEM_OPEN);
        append(out, ret_doc);
        append(out, HTML_DOC_ITEM_CLOSE);
    }
    
    String main_doc = doc.main_doc;
    if (main_doc.size != 0){
        append(out, HTML_DOC_HEAD_OPEN"Description"HTML_DOC_HEAD_CLOSE HTML_DOC_ITEM_OPEN);
        print_doc_description(out, main_doc);
        append(out, HTML_DOC_ITEM_CLOSE);
    }
    
    print_see_also(out, &doc);
    
    fm_end_temp(temp);
}

internal void
print_item_html(String *out, Used_Links *used, Item_Node *item, char *id_postfix, char *section, i32 I, u32 mangle_rule){
    Temp temp = fm_begin_temp();
    
    String name = apply_mangle_rule(item->name, mangle_rule);
    
    /* NOTE(allen):
    Open a div for the whole item.
    Put a heading in it with the name and section.
    Open a "descriptive" box for the display of the code interface.
    */
    append(out, "<div id='");
    append(out, name);
    append(out, id_postfix);
    append(out, "' style='margin-bottom: 1cm;'>");
    
    i32 has_cpp_name = 0;
    if (item->cpp_name.str != 0){
        if (try_to_use_link(used, item->cpp_name)){
            append(out, "<div id='");
            append(out, item->cpp_name);
            append(out, id_postfix);
            append(out, "'>");
            has_cpp_name = 1;
        }
    }
    
    append         (out, "<h4>&sect;");
    append         (out, section);
    append_s_char     (out, '.');
    append_int_to_str (out, I);
    append         (out, ": ");
    append         (out, name);
    append         (out, "</h4>");
    
    append(out, "<div style='"HTML_CODE_STYLE" "HTML_DESCRIPT_SECTION_STYLE"'>");
    
    switch (item->t){
        case Item_Function:
        {
            // NOTE(allen): Code box
            print_function_html(out, used, item->cpp_name, item->ret, "", name, item->breakdown);
            
            // NOTE(allen): Close the code box
            append(out, "</div>");
            
            // NOTE(allen): Descriptive section
            print_function_docs(out, name, item->doc_string);
        }break;
        
        case Item_Macro:
        {
            // NOTE(allen): Code box
            print_macro_html(out, name, item->breakdown);
            
            // NOTE(allen): Close the code box
            append(out, "</div>");
            
            // NOTE(allen): Descriptive section
            print_function_docs(out, name, item->doc_string);
        }break;
        
        case Item_Typedef:
        {
            String type = item->type;
            
            // NOTE(allen): Code box
            append     (out, "typedef ");
            append     (out, type);
            append_s_char (out, ' ');
            append     (out, name);
            append_s_char (out, ';');
            
            // NOTE(allen): Close the code box
            append(out, "</div>");
            
            // NOTE(allen): Descriptive section
            String doc_string = item->doc_string;
            Documentation doc = {0};
            perform_doc_parse(doc_string, &doc);
            
            String main_doc = doc.main_doc;
            if (main_doc.size != 0){
                append(out, HTML_DOC_HEAD_OPEN"Description"HTML_DOC_HEAD_CLOSE);
                
                append(out, HTML_DOC_ITEM_OPEN);
                print_doc_description(out, main_doc);
                append(out, HTML_DOC_ITEM_CLOSE);
            }
            else{
                fprintf(stdout, "warning: no documentation string for %.*s\n", name.size, name.str);
            }
            
            print_see_also(out, &doc);
            
        }break;
        
        case Item_Enum:
        {
            // NOTE(allen): Code box
            append     (out, "enum ");
            append     (out, name);
            append_s_char (out, ';');
            
            // NOTE(allen): Close the code box
            append(out, "</div>");
            
            // NOTE(allen): Descriptive section
            String doc_string = item->doc_string;
            Documentation doc = {0};
            perform_doc_parse(doc_string, &doc);
            
            String main_doc = doc.main_doc;
            if (main_doc.size != 0){
                append(out, HTML_DOC_HEAD_OPEN"Description"HTML_DOC_HEAD_CLOSE);
                
                append(out, HTML_DOC_ITEM_OPEN);
                print_doc_description(out, main_doc);
                append(out, HTML_DOC_ITEM_CLOSE);
            }
            else{
                fprintf(stdout, "warning: no documentation string for %.*s\n", name.size, name.str);
            }
            
            if (item->first_child){
                append(out, HTML_DOC_HEAD_OPEN"Values"HTML_DOC_HEAD_CLOSE);
                
                for (Item_Node *member = item->first_child;
                     member;
                     member = member->next_sibling){
                    Documentation doc = {0};
                    perform_doc_parse(member->doc_string, &doc);
                    
                    append(out, "<div>");
                    
                    // NOTE(allen): Dafuq is this all?
                    append(out, "<div><span style='"HTML_CODE_STYLE"'>"HTML_DOC_ITEM_HEAD_INL_OPEN);
                    append(out, member->name);
                    append(out, HTML_DOC_ITEM_HEAD_INL_CLOSE);
                    
                    if (member->value.str){
                        append(out, " = ");
                        append(out, member->value);
                    }
                    
                    append(out, "</span></div>");
                    
                    append(out, "<div style='margin-bottom: 6mm;'>"HTML_DOC_ITEM_OPEN);
                    print_doc_description(out, doc.main_doc);
                    append(out, HTML_DOC_ITEM_CLOSE"</div>");
                    
                    append(out, "</div>");
                }
            }
            
            print_see_also(out, &doc);
            
        }break;
        
        case Item_Struct: case Item_Union:
        {
            String doc_string = item->doc_string;
            
            i32 hide_members = 0;
            
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
            append(out, "</div>");
            
            // NOTE(allen): Descriptive section
            {
                Documentation doc = {0};
                perform_doc_parse(doc_string, &doc);
                
                String main_doc = doc.main_doc;
                if (main_doc.size != 0){
                    append(out, HTML_DOC_HEAD_OPEN"Description"HTML_DOC_HEAD_CLOSE);
                    
                    append(out, HTML_DOC_ITEM_OPEN);
                    print_doc_description(out, main_doc);
                    append(out, HTML_DOC_ITEM_CLOSE);
                }
                else{
                    fprintf(stdout, "warning: no documentation string for %.*s\n", name.size, name.str);
                }
                
                if (!hide_members){
                    if (item->first_child){
                        append(out, HTML_DOC_HEAD_OPEN"Fields"HTML_DOC_HEAD_CLOSE);
                        print_struct_docs(out, item);
                    }
                }
                
                print_see_also(out, &doc);
            }
        }break;
    }
    
    if (has_cpp_name){
        append(out, "</div>");
    }
    
    // NOTE(allen): Close the item box
    append(out, "</div><hr>");
    
    fm_end_temp(temp);
}

global char* html_css =
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
"</style>";

struct Include_Stack{
    Abstract_Item *stack[512];
    u32 top;
};

struct Document_Output_System{
    String *out;
    Document_System *doc_system;
    Used_Links *used_links;
    Section_Counter *section_counter;
    Include_Stack *inc_stack;
};

internal Document_Output_System
make_output_system(String *out, Document_System *doc_system, Used_Links *used_links, Section_Counter *section_counter, Include_Stack *inc_stack){
    Document_Output_System sys = {0};
    sys.out = out;
    sys.doc_system = doc_system;
    sys.used_links = used_links;
    sys.section_counter = section_counter;
    sys.inc_stack = inc_stack;
    return(sys);
}

internal void
doc_item_html(Document_Output_System sys, Document_Item *item, b32 head){
    switch (item->type){
        case Doc_Root:
        {
            if (head){
                append(sys.out,
                       "<html lang=\"en-US\">"
                       "<head>"
                       "<link rel='shortcut icon' type='image/x-icon' href='4coder_icon.ico' />"
                       "<title>");
                append(sys.out, item->section.name);
                append(sys.out, "</title>");
                
                append(sys.out, html_css);
                
                append(sys.out,
                       "</head>\n"
                       "<body>"
                       "<div style='font-family:Arial; margin: 0 auto; "
                       "width: ");
                append_int_to_str(sys.out, HTML_WIDTH);
                append(sys.out, "px; text-align: justify; line-height: 1.25;'>");
                
                if (item->section.show_title){
                    append(sys.out, "<h1 style='margin-top: 5mm; margin-bottom: 5mm;'>");
                    append(sys.out, item->section.name);
                    append(sys.out, "</h1>");
                }
            }
            else{
                append(sys.out, "</div></body></html>");
            }
        }break;
        
        case Doc_Section:
        {
            if (head){
                html_render_section_header(sys.out, item->section.name, item->section.id, sys.section_counter);
            }
        }break;
        
        case Doc_Error:
        {
            if (head){
                output_error(sys.out, item->string.string);
            }
        }break;
        
        case Doc_Todo:
        {
            if (head){
                append(sys.out, "<div><i>Coming Soon</i><div>");
            }
        }break;
        
        case Doc_Include:
        {
            // NOTE(allen): Do nothing.
        }break;
        
        
        case Doc_DocFull:
        case Doc_DocList:
        {
            if (head){
                Mangle_Rule mangle_rule = item->unit_elements.mangle_rule;
                String name = item->unit_elements.unit;
                
                Abstract_Item *unit_item = get_item_by_name(sys.doc_system->meta_list, name);
                Meta_Unit *unit = unit_item->unit;
                i32 count = unit->set.count;
                
                if (item->type == Doc_DocList){
                    append(sys.out, "<ul>");
                    
                    for (i32 i = 0; i < count; ++i){
                        String name = apply_mangle_rule(unit->set.items[i].name, mangle_rule);
                        print_item_in_list(sys.out, name, "_doc");
                    }
                    
                    append(sys.out, "</ul>");
                }
                else{
                    char section_space[32];
                    String section_str = make_fixed_width_string(section_space);
                    append_section_number_reduced(&section_str, sys.section_counter, 1);
                    terminate_with_null(&section_str);
                    
                    i32 I = 1;
                    for (i32 i = 0; i < count; ++i, ++I){
                        print_item_html(sys.out, sys.used_links, &unit->set.items[i], "_doc", section_str.str, I, mangle_rule);
                    }
                }
            }
        }break;
        
        case Doc_TableOfContents:
        {
            if (head){
                append(sys.out, "<h3 style='margin:0;'>Table of Contents</h3><ul>");
                
                i32 i = 1;
                for (Document_Item *toc_item = item->parent->section.first_child;
                     toc_item != 0;
                     toc_item = toc_item->next){
                    if (toc_item->type == Doc_Section){
                        if (toc_item->section.id.size > 0){
                            append(sys.out, "<li><a href='#section_");
                            append(sys.out, toc_item->section.id);
                            append(sys.out, "'>&sect;");
                        }
                        else{
                            append(sys.out, "<li>&sect;");
                        }
                        append_int_to_str (sys.out, i);
                        append_s_char     (sys.out, ' ');
                        append            (sys.out, toc_item->section.name);
                        append            (sys.out, "</a></li>");
                        ++i;
                    }
                }
                
                append(sys.out, "</ul>");
            }
        }break;
        
        case Doc_PlainOldText:
        {
            if (head){
                output_plain_old_text(sys.out, item->string.string);
            }
        }break;
        
        case Doc_Version:
        {
            if (head){
                append(sys.out, VERSION);
            }
        }break;
        
        case Doc_Style:
        {
            if (head){
                output_begin_style(sys.out, item->string.string);
            }
            else{
                output_end_style(sys.out);
            }
        }break;
        
        case Doc_DocumentLink:
        {
            if (head){
                output_document_link(sys.out, item->string.string);
            }
        }break;
        
        case Doc_Link:
        {
            if (head){
                output_begin_link(sys.doc_system, sys.out, item->string.string);
            }
            else{
                output_end_link(sys.out);
            }
        }break;
        
        case Doc_Image:
        {
            if (head){
                output_image(sys.doc_system, sys.out, item->string.string, item->string.string2);
            }
        }break;
        
        case Doc_Video:
        {
            if (head){
                output_video(sys.out, item->string.string);
            }
        }break;
        
        case Doc_BeginParagraph:
        {
            if (head){
                output_begin_paragraph(sys.out);
            }
        }break;
        
        case Doc_EndParagraph:
        {
            if (head){
                output_end_paragraph(sys.out);
            }
        }break;
        
        case Doc_List:
        {
            if (head){
                output_begin_list(sys.out);
            }
            else{
                output_end_list(sys.out);
            }
        }break;
        
        case Doc_Item:
        {
            if (head){
                output_begin_item(sys.out, sys.section_counter);
            }
            else{
                output_end_item(sys.out);
            }
        }break;
    }
}

internal void
generate_document_html_inner(Document_Output_System sys, Abstract_Item *doc);

internal void
generate_item_html(Document_Output_System sys, Document_Item *item){
    doc_item_html(sys, item, true);
    
    if (item->section.first_child != 0){
        if (item->type == Doc_Section){
            i32 level = ++sys.section_counter->nest_level;
            sys.section_counter->counter[level] = 1;
            sys.section_counter->list_item_counter = 0;
        }
        
        for (Document_Item *m = item->section.first_child;
             m != 0;
             m = m->next){
            generate_item_html(sys, m);
        }
        
        if (item->type == Doc_Section){
            --sys.section_counter->nest_level;
            ++sys.section_counter->counter[sys.section_counter->nest_level];
        }
    }
    
    if (item->type == Doc_Include){
        Abstract_Item *new_doc = item->include.document;
        if (new_doc != 0){
            b32 duplicate = false;
            for (u32 i = 0; i < sys.inc_stack->top; ++i){
                if (sys.inc_stack->stack[i] == new_doc){
                    duplicate = true;
                    break;
                }
            }
            
            if (duplicate){
                String error = make_lit_string("recursive inclusion, halted here");
                Document_Item temp_item = {0};
                temp_item.type = Doc_Error;
                set_item_string(&temp_item.string.string, error);
                generate_item_html(sys, &temp_item);
            }
            else{
                generate_document_html_inner(sys, new_doc);
            }
        }
    }
    
    doc_item_html(sys, item, false);
}

internal void
generate_document_html_inner(Document_Output_System sys, Abstract_Item *doc){
    Assert(sys.inc_stack->top < ArrayCount(sys.inc_stack->stack));
    sys.inc_stack->stack[sys.inc_stack->top++] = doc;
    generate_item_html(sys, doc->root_item);
    --sys.inc_stack->top;
}

internal void
generate_document_html(String *out, Document_System *doc_system, Abstract_Item *doc){
    Assert(doc->root_item != 0);
    
    Used_Links used_links = {0};
    init_used_links(&used_links, 4000);
    
    Section_Counter section_counter = {0};
    section_counter.counter[section_counter.nest_level] = 1;
    
    Include_Stack inc_stack = {0};
    
    Document_Output_System sys = make_output_system(out, doc_system, &used_links, &section_counter, &inc_stack);
    
    generate_document_html_inner(sys, doc);
}

// BOTTOM

