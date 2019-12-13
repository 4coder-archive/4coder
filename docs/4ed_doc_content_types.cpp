/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 04.12.2019
 *
 * Definition of information contained in 4coder documentation.
 *
 */

// TOP

#include <time.h>

function Doc_Date
doc_date_now(void){
    time_t t = time(0);
    tm *time = localtime(&t);
    Doc_Date date = {};
    date.day = time->tm_mday;
    date.month = time->tm_mon + 1;
    date.year = 1900 + time->tm_year;
    return(date);
             }

////////////////////////////////

function Doc_Content*
doc_content_push(Arena *arena, Doc_Content_List *list, String_Const_u8 text, Doc_Content_Emphasis emphasis){
    Doc_Content *content = push_array_zero(arena, Doc_Content, 1);
    sll_queue_push(list->first, list->last, content);
    list->total_size += text.size;
    list->node_count += 1;
    content->text = text;
    content->emphasis = emphasis;
    return(content);
}

function Doc_Content*
doc_content_push(Arena *arena, Doc_Content_List *list, String_Const_u8 text){
    return(doc_content_push(arena, list, text, DocContentEmphasis_Normal));
}

function void
doc_code_list_push(Arena *arena, Doc_Code_Sample_List *list, String_Const_u8 contents, Doc_Code_Language language){
    Doc_Code_Sample *sample = push_array_zero(arena, Doc_Code_Sample, 1);
    sll_queue_push(list->first, list->last, sample);
    list->count += 1;
    sample->contents = contents;
    sample->language = language;
}

////////////////////////////////

function Doc_Cluster*
new_doc_cluster(Arena *arena, char *title, char *name, Doc_Date *date){
    Doc_Cluster *result = push_array_zero(arena, Doc_Cluster, 1);
    result->title = SCu8(title);
    result->name = SCu8(name);
    result->gen_date = *date;
    return(result);
}

function Doc_Cluster*
new_doc_cluster(Arena *arena, char *title, char *name){
    Doc_Date date = doc_date_now();
    return(new_doc_cluster(arena, title, name, &date));
}

function Doc_Page*
new_doc_page(Arena *arena, Doc_Cluster *cluster, char *title, char *name){
    Doc_Page *result = push_array_zero(arena, Doc_Page, 1);
    
    result->owner = cluster;
    sll_queue_push(cluster->first_page, cluster->last_page, result);
    cluster->page_count += 1;
    
    result->title = SCu8(title);
    result->name = SCu8(name);
    
    return(result);
}

function Doc_Page*
new_doc_page_normal_title(Arena *arena, Doc_Cluster *cluster, char *title, char *name){
    String_Const_u8 full_title = push_u8_stringf(arena, "%s - %.*s", title, string_expand(cluster->title));
    return(new_doc_page(arena, cluster, (char*)full_title.str, name));
}

function Doc_Page*
new_doc_page_function(Arena *arena, Doc_Cluster *cluster, char *name){
    return(new_doc_page_normal_title(arena, cluster, name, name));
}

function Doc_Page*
new_doc_page_function(Arena *arena, Doc_Cluster *cluster, String_Const_u8 name){
    name = push_string_copy(arena, name);
    char *c_name = (char*)name.str;
    return(new_doc_page_function(arena, cluster, c_name));
}

 function Doc_Block*
new_doc_block(Arena *arena, Doc_Page *page, char *name){
    Doc_Block *result = push_array_zero(arena, Doc_Block, 1);
    result->owner = page;
    sll_queue_push(page->first_block, page->last_block, result);
    page->block_count += 1;
    result->name = SCu8(name);
    return(result);
}

function void
new_doc_block_jump(Arena *arena, Doc_Page *page, Doc_Block *block){
    Doc_Block_Ptr *node = push_array_zero(arena, Doc_Block_Ptr, 1);
    sll_queue_push(page->quick_jumps.first, page->quick_jumps.last, node);
    page->quick_jumps.count += 1;
    node->block = block;
}

function Doc_Paragraph*
new_doc_par(Arena *arena, Doc_Block *block){
    Doc_Paragraph *result = push_array_zero(arena, Doc_Paragraph, 1);
    sll_queue_push(block->first_par, block->last_par, result);
    block->par_count += 1;
    return(result);
}

function void
new_doc_par_single_code(Arena *arena, Doc_Block *block, String_Const_u8 contents, Doc_Code_Language language){
    Doc_Paragraph *paragraph = new_doc_par(arena, block);
    paragraph->kind = DocParagraphKind_Code;
    doc_code_list_push(arena, &paragraph->code, contents, language);
}

////////////////////////////////

function void
doc_log(Arena *arena, Doc_Cluster *cluster, String_Const_u8 string){
    Doc_Log *log = push_array_zero(arena, Doc_Log, 1);
    sll_queue_push(cluster->first_log, cluster->last_log, log);
    log->content = string;
}

function void
doc_log(Arena *arena, Doc_Cluster *cluster, char *str){
    doc_log(arena, cluster, SCu8(str));
}

function void
doc_logfv(Arena *arena, Doc_Cluster *cluster, char *format, va_list args){
    String_Const_u8 str = push_u8_stringfv(arena, format, args);
    doc_log(arena, cluster, str);
}

function void
doc_logf(Arena *arena, Doc_Cluster *cluster, char *format, ...){
    va_list args;
    va_start(args, format);
    doc_logfv(arena, cluster, format, args);
    va_end(args);
}

#define doc_warning(a,c) doc_log(a),(c), "warning: ")
#define doc_error(a,c) doc_log(a),(c), "error: ")
#define doc_note(a,c) doc_log(a),(c), "note: ")

#define doc_warningf(a,c,f,...) doc_logf((a),(c), "warning: " f, __VA_ARGS__)
#define doc_errorf(a,c,f,...) doc_logf((a),(c), "error: " f, __VA_ARGS__)
#define doc_notef(a,c,f,...) doc_logf((a),(c), "note: " f, __VA_ARGS__)

////////////////////////////////

function void
doc_text(Arena *arena, Doc_Block *block, char *str){
    Doc_Paragraph *par = block->last_par;
    if (par != 0){
        if (par->kind != DocParagraphKind_Text){
            par = 0;
        }
    }
    
    if (par == 0){
        par = new_doc_par(arena, block);
        par->kind = DocParagraphKind_Text;
    }
    
    doc_content_push(arena, &par->text, SCu8(str));
}

function void
doc_paragraph(Arena *arena, Doc_Block *block){
    Doc_Paragraph *par = new_doc_par(arena, block);
    par->kind = DocParagraphKind_Text;
}

////////////////////////////////

function Doc_Function
make_doc_function(Arena *arena, Doc_Cluster *cluster, API_Call *call){
    Doc_Function result = {};
    result.call = call;
    result.page = new_doc_page_function(arena, cluster, call->name);
    result.brief = new_doc_block(arena, result.page, "brief");
    result.sig = new_doc_block(arena, result.page, "Signature");
    new_doc_block_jump(arena, result.page, result.sig);
    
       String_Const_u8 opener = push_u8_stringf(arena, "%.*s\n%.*s(",
                                             string_expand(call->return_type),
                                             string_expand(call->name));
    
    umem indent_size = call->name.size + 1;
    u8 *buffer = push_array(arena, u8, indent_size);
    for (umem i = 0; i < indent_size; i += 1){
        buffer[i] = ' ';
    }
    String_Const_u8 indent = SCu8(buffer, indent_size);
    
    List_String_Const_u8 list = {};
    string_list_push(arena, &list, opener);
    for (API_Param *node = call->params.first;
         node != 0;
         node = node->next){
        string_list_pushf(arena, &list, "%.*s %.*s",
                          string_expand(node->type_name),
                          string_expand(node->name));
        if (node->next != 0){
            string_list_pushf(arena, &list, ",\n%.*s",
                              string_expand(indent));
        }
    }
    string_list_push(arena, &list, string_u8_litexpr(");"));
    
    String_Const_u8 contents = string_list_flatten(arena, list);
     new_doc_par_single_code(arena, result.sig, contents, DocCodeLanguage_Cpp);
    
    return(result);
}

function b32
begin_doc_call(Arena *arena, Doc_Cluster *cluster, API_Definition *api_def, char *name, Doc_Function *func){
    API_Call *call = api_get_call(api_def, SCu8(name));
    b32 result = (call != 0);
    if (result){
        *func = make_doc_function(arena, cluster, call);
    }
    else{
        doc_warningf(arena, cluster, "dead call documentation %s", name);
    }
    return(result);
}

function Doc_Block*
doc_function_brief(Arena *arena, Doc_Function *func, char *text){
    if (text != 0){
        doc_text(arena, func->brief, text);
    }
    return(func->brief);
}

function Doc_Block*
doc_function_begin_params(Arena *arena, Doc_Function *func){
    func->params = new_doc_block(arena, func->page, "Parameters");
    new_doc_block_jump(arena, func->page, func->params);
    return(func->params);
}

function void
doc_function_param(Arena *arena, Doc_Function *func, char *name){
    String_Const_u8 name_str = SCu8(name);
    
    API_Call *call = func->call;
    API_Param *param = 0;
    for (API_Param *node = call->params.first;
         node != 0;
         node = node->next){
        if (string_match(name_str, node->name)){
            param = node;
            break;
        }
    }
    
    if (param == 0){
        doc_errorf(arena, func->page->owner, "documentation for non-existant parameter %s in call %.*s", name, string_expand(call->name));
        return;
    }
    
    API_Param *iter = func->param_iter;
    if (iter != 0){
        for (iter = iter->next;
             iter != 0 && iter != param;
         iter = iter->next);
    if (iter == 0){
            doc_warningf(arena, func->page->owner, "parameters out of order in documentation for call %.*s", string_expand(call->name));
    }
    }
    func->param_iter = param;
    
    // parameter header
    Doc_Paragraph *par = new_doc_par(arena, func->params);
    par->kind = DocParagraphKind_Text;
    doc_content_push(arena, &par->text, name_str, DocContentEmphasis_SmallHeader);
    
    // empty paragraph to start filling after
    par = new_doc_par(arena, func->params);
    par->kind = DocParagraphKind_Text;
}

function Doc_Block*
doc_function_return(Arena *arena, Doc_Function *func){
    func->ret = new_doc_block(arena, func->page, "Return");
    new_doc_block_jump(arena, func->page, func->ret);
    return(func->ret);
}

function Doc_Block*
doc_function_details(Arena *arena, Doc_Function *func){
    func->det = new_doc_block(arena, func->page, "Details");
    new_doc_block_jump(arena, func->page, func->det);
    return(func->det);
}

function Doc_Block*
doc_function_examples(Arena *arena, Doc_Function *func){
    func->examples = new_doc_block(arena, func->page, "Examples");
    new_doc_block_jump(arena, func->page, func->examples);
    return(func->examples);
}

function Doc_Block*
doc_function_begin_related(Arena *arena, Doc_Function *func){
    func->rel = new_doc_block(arena, func->page, "Related");
    new_doc_block_jump(arena, func->page, func->rel);
    return(func->rel);
}

function void
doc_function_add_related(Arena *arena, Doc_Block *rel, char *name){
    Doc_Paragraph *par = new_doc_par(arena, rel);
    par->kind = DocParagraphKind_Text;
    Doc_Content *content = doc_content_push(arena, &par->text, SCu8(name));
    content->page_link = SCu8(name);
}

// BOTTOM

