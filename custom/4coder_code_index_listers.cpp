/*
4coder_code_index_listers.cpp - Listers for exploring the contents of the code index.
*/

// TOP

struct Tiny_Jump{
    Buffer_ID buffer;
    i64 pos;
};

CUSTOM_UI_COMMAND_SIG(jump_to_type_definition)
CUSTOM_DOC("List all types in the code index and jump to one chosen by the user.")
{
    char *query = "Type:";
    
    Scratch_Block scratch(app, Scratch_Share);
    Lister *lister = begin_lister(app, scratch);
    lister_set_query(lister, query);
    lister->handlers = lister_get_default_handlers();
    
    code_index_lock();
    for (Buffer_ID buffer = get_buffer_next(app, 0, Access_Always);
         buffer != 0;
         buffer = get_buffer_next(app, buffer, Access_Always)){
        Code_Index_File *file = code_index_get_file(buffer);
        if (file != 0){
            for (i32 i = 0; i < file->note_array.count; i += 1){
                Code_Index_Note *note = file->note_array.ptrs[i];
                Tiny_Jump *jump = push_array(scratch, Tiny_Jump, 1);
                jump->buffer = buffer;
                jump->pos = note->pos.first;
                lister_add_item(lister, note->text, string_u8_litexpr("type"), jump, 0);
            }
        }
    }
    code_index_unlock();
    
    Lister_Result l_result = run_lister(app, lister);
    Tiny_Jump result = {};
    if (!l_result.canceled && l_result.user_data != 0){
        block_copy_struct(&result, (Tiny_Jump*)l_result.user_data);
    }
    
    if (result.buffer != 0){
        View_ID view = get_this_ctx_view(app, Access_Always);
        jump_to_location(app, view, result.buffer, result.pos);
    }
}

// BOTTOM

