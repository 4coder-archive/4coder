/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 14.12.2019
 *
 * Definition of information contained in 4coder documentation.
 *
 */

// TOP

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
    
    u64 indent_size = call->name.size + 1;
    u8 *buffer = push_array(arena, u8, indent_size);
    for (u64 i = 0; i < indent_size; i += 1){
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

////////////////////////////////

function void
doc_api_check_full_coverage(Arena *arena, Doc_Cluster *cluster, API_Definition *api_def){
    for (API_Call *call = api_def->first_call;
         call != 0;
         call = call->next){
        String_Const_u8 name = call->name;
        Doc_Page *page = doc_get_page(cluster, name);
        if (page == 0){
            doc_errorf(arena, cluster, "missing documentation for %.*s", string_expand(name));
        }
    }
}

// BOTTOM
