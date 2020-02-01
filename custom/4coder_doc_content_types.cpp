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

function Doc_Paragraph*
new_doc_par_table(Arena *arena, Doc_Block *block){
    Doc_Paragraph *result = new_doc_par(arena, block);
    result->kind = DocParagraphKind_Table;
    return(result);
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

function Doc_Content*
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
    
    return(doc_content_push(arena, &par->text, SCu8(str)));
}

function Doc_Content*
doc_text(Arena *arena, Doc_Content_List *list, char *string){
    return(doc_content_push(arena, list, SCu8(string)));
}

function void
doc_paragraph(Arena *arena, Doc_Block *block){
    Doc_Paragraph *par = new_doc_par(arena, block);
    par->kind = DocParagraphKind_Text;
}

////////////////////////////////

function Doc_Page*
doc_get_page(Doc_Cluster *cluster, String_Const_u8 name){
    Doc_Page *result = 0;
    for (Doc_Page *page = cluster->first_page;
         page != 0;
         page = page->next){
        if (string_match(name, page->name)){
            result = page;
            break;
        }
    }
    return(result);
}

// BOTTOM

