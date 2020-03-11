/*
4coder_docs.cpp - Documentation explorers
*/

// TOP

function Doc_Page*
get_doc_page_from_user(Application_Links *app, Doc_Cluster *doc, String_Const_u8 query){
    Scratch_Block scratch(app);
    Lister_Block lister(app, scratch);
    lister_set_query(lister, query);
    lister_set_default_handlers(lister);
    
    for (Doc_Page *page = doc->first_page;
         page != 0;
         page = page->next){
        lister_add_item(lister, page->name, SCu8(""), page, 0);
    }
    Lister_Result l_result = run_lister(app, lister);
    Doc_Page *result = 0;
    if (!l_result.canceled){
        result = (Doc_Page*)l_result.user_data;
    }
    return(result);
}

function Doc_Page*
get_doc_page_from_user(Application_Links *app, Doc_Cluster *doc, char *query){
    return(get_doc_page_from_user(app, doc, SCu8(query)));
}

function void
render_doc_page__content(Application_Links *app, Buffer_Insertion *insert, Doc_Content_List *list){
    for (Doc_Content *content = list->first;
         content != 0;
         content = content->next){
        // TODO(allen): actually implement links
        
        if (content->emphasis == DocContentEmphasis_SmallHeader){
            insertf(insert, "\n");
        }
        if (content->emphasis == DocContentEmphasis_Heavy){
            insertf(insert, "_");
        }
        if (content->emphasis == DocContentEmphasis_Stylish){
            insertf(insert, "*");
        }
        
        insertf(insert, "%.*s", string_expand(content->text));
        if (content->page_link.size > 0){
            insertf(insert, " (link page %.*s)", string_expand(content->page_link));
        }
        else if (content->block_link.size > 0){
            insertf(insert, " (link block %.*s)", string_expand(content->block_link));
        }
        
        if (content->emphasis == DocContentEmphasis_Heavy){
            insertf(insert, "_");
        }
        if (content->emphasis == DocContentEmphasis_Stylish){
            insertf(insert, "*");
        }
        if (content->emphasis == DocContentEmphasis_SmallHeader){
            insertf(insert, "\n");
        }
        else{
            if (content->next != 0){
                insertf(insert, " ");
            }
        }
    }
}

function void
render_doc_page__code(Application_Links *app, Buffer_Insertion *insert, Doc_Code_Sample_List *code){
    for (Doc_Code_Sample *sample = code->first;
         sample != 0;
         sample = sample->next){
        insertf(insert, "language: ");
        switch (sample->language){
            case DocCodeLanguage_Cpp:
            {
                insertf(insert, "C++\n");
            }break;
            case DocCodeLanguage_Bat:
            {
                insertf(insert, "Batch\n\n");
            }break;
        }
        insertf(insert, "\n%.*s\n", string_expand(sample->contents));
    }
}

function void
render_doc_page__table(Application_Links *app, Buffer_Insertion *insert, Vec2_i32 dim, Doc_Content_List *vals){
    // TODO(allen): align better or something
    Doc_Content_List *val = vals;
    for (i32 y = 0; y < dim.y; y += 1){
        for (i32 x = 0; x < dim.x; x += 1){
            render_doc_page__content(app, insert, val);
            insertf(insert, "; ");
            val += 1;
        }
        insertf(insert, "\n");
    }
}

function Buffer_ID
render_doc_page(Application_Links *app, Doc_Page *page){
    Scratch_Block scratch(app);
    
    String_Const_u8 doc_buffer_name = push_u8_stringf(scratch, "*doc: %.*s*",
                                                      string_expand(page->name));
    
    Buffer_Create_Flag flags = BufferCreate_NeverAttachToFile;
    Buffer_ID buffer = create_buffer(app, doc_buffer_name, flags);
    if (buffer != 0){
        buffer_set_setting(app, buffer, BufferSetting_RecordsHistory, false);
        buffer_set_setting(app, buffer, BufferSetting_ReadOnly, true);
        buffer_set_setting(app, buffer, BufferSetting_Unimportant, true);
        
        i64 size = buffer_get_size(app, buffer);
        if (size != 0){
            buffer_replace_range(app, buffer, Ii64(0, size), SCu8(""));
        }
        Buffer_Insertion insert = begin_buffer_insertion_at_buffered(app, buffer, 0, scratch, KB(16));
        
        char dashes[] =
            "----------------------------------------------------------------"
            "----------------------------------------------------------------"
            "----------------------------------------------------------------"
            "----------------------------------------------------------------";
        
        insertf(&insert, "%.*s\n%.*s\n",
                string_expand(page->title),
                page->title.size, dashes);
        
        for (Doc_Block *block = page->first_block;
             block != 0;
             block = block->next){
            insertf(&insert, "%.*s\n\n", string_expand(block->name));
            
            for (Doc_Paragraph *par = block->first_par;
                 par != 0;
                 par = par->next){
                switch (par->kind){
                    case DocParagraphKind_Text:
                    {
                        render_doc_page__content(app, &insert, &par->text);
                    }break;
                    
                    case DocParagraphKind_Code:
                    {
                        render_doc_page__code(app, &insert, &par->code);
                    }break;
                    
                    case DocParagraphKind_Table:
                    {
                        render_doc_page__table(app, &insert, par->table.dim, par->table.vals);
                    }break;
                }
                
                insert_string(&insert, string_u8_litexpr("\n"));
            }
            
            insertf(&insert, "%.*s\n", page->title.size, dashes);
        }
        
        end_buffer_insertion(&insert);
    }
    
    return(buffer);
}

CUSTOM_UI_COMMAND_SIG(custom_api_documentation)
CUSTOM_DOC("Prompts the user to select a Custom API item then loads a doc buffer for that item")
{
    View_ID view = get_this_ctx_view(app, Access_ReadWrite);
    if (view != 0){
        Scratch_Block scratch(app);
        Doc_Cluster *docs = get_custom_layer_boundary_docs(app, scratch);
        Doc_Page *page = get_doc_page_from_user(app, docs, "Doc Page:");
		if (page != 0){
            Buffer_ID buffer = render_doc_page(app, page);
            view_set_buffer(app, view, buffer, 0);
        }
    }
}

CUSTOM_UI_COMMAND_SIG(command_documentation)
CUSTOM_DOC("Prompts the user to select a command then loads a doc buffer for that item")
{
    View_ID view = get_this_ctx_view(app, Access_Always);
    if (view != 0){
        Scratch_Block scratch(app);
        Doc_Cluster *docs = doc_commands(scratch);
        Doc_Page *page = get_doc_page_from_user(app, docs, "Doc Page:");
        if (page != 0){
            Buffer_ID buffer = render_doc_page(app, page);
            view_set_buffer(app, view, buffer, 0);
        }
    }
}

// BOTTOM

