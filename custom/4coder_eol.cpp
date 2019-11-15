/*
 * 4coder_eol.cpp - Commands and routines for controling the end-of-line encoding
 * of files.
 */

// TOP

function void
rewrite_lines_to_crlf(Application_Links *app, Buffer_ID buffer){
    ProfileScope(app, "rewrite lines to crlf");
    Scratch_Block scratch(app);
    i64 size = buffer_get_size(app, buffer);
    
    Batch_Edit *first = 0;
    Batch_Edit *last = 0;
    
    ProfileBlockNamed(app, "build batch edit", profile_batch);
    i64 pos = -1;
    Character_Predicate pred_cr = character_predicate_from_character('\r');
    Character_Predicate pred_lf = character_predicate_from_character('\n');
    Character_Predicate pred = character_predicate_or(&pred_cr, &pred_lf);
    for (;;){
        String_Match match = buffer_seek_character_class(app, buffer, &pred,
                                                         Scan_Forward, pos);
        if (match.range.min == match.range.max){
            break;
        }
        pos = match.range.min;
        
        u8 c1 = buffer_get_char(app, buffer, pos);
        u8 c2 = buffer_get_char(app, buffer, pos + 1);
        if (c1 == '\r'){
            if (pos + 1 == size || c2 != '\n'){
                Batch_Edit *edit = push_array(scratch, Batch_Edit, 1);
                sll_queue_push(first, last, edit);
                edit->edit.text = string_u8_litexpr("");
                edit->edit.range = match.range;
            }
            else{
                pos += 1;
            }
        }
        else{
            Batch_Edit *edit = push_array(scratch, Batch_Edit, 1);
            sll_queue_push(first, last, edit);
            edit->edit.text = string_u8_litexpr("\r");
            edit->edit.range = Ii64(pos);
        }
    }
    ProfileCloseNow(profile_batch);
    
    buffer_batch_edit(app, buffer, first);
}

function void
rewrite_lines_to_lf(Application_Links *app, Buffer_ID buffer){
    ProfileScope(app, "rewrite lines to lf");
    Scratch_Block scratch(app);
    
    Batch_Edit *first = 0;
    Batch_Edit *last = 0;
    
    ProfileBlockNamed(app, "build batch edit", profile_batch);
    i64 pos = -1;
    Character_Predicate pred = character_predicate_from_character('\r');
    for (;;){
        String_Match match = buffer_seek_character_class(app, buffer, &pred,
                                                         Scan_Forward, pos);
        if (match.range.min == match.range.max){
            break;
        }
        pos = match.range.min;
        
        Batch_Edit *edit = push_array(scratch, Batch_Edit, 1);
        sll_queue_push(first, last, edit);
        edit->edit.text = string_u8_litexpr("");
        edit->edit.range = match.range;
    }
    ProfileCloseNow(profile_batch);
    
	buffer_batch_edit(app, buffer, first);
}

////////////////////////////////

CUSTOM_COMMAND_SIG(set_eol_mode_to_crlf)
CUSTOM_DOC("Puts the buffer in crlf line ending mode.")
{
    View_ID view = get_active_view(app, Access_ReadWriteVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
    Managed_Scope scope = buffer_get_managed_scope(app, buffer);
    Line_Ending_Kind *eol_setting = scope_attachment(app, scope, buffer_eol_setting,
                                                     Line_Ending_Kind);
    if (eol_setting != 0){
    *eol_setting = LineEndingKind_CRLF;
    }
}

CUSTOM_COMMAND_SIG(set_eol_mode_to_lf)
CUSTOM_DOC("Puts the buffer in lf line ending mode.")
{
    View_ID view = get_active_view(app, Access_ReadWriteVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
    Managed_Scope scope = buffer_get_managed_scope(app, buffer);
    Line_Ending_Kind *eol_setting = scope_attachment(app, scope, buffer_eol_setting,
                                                     Line_Ending_Kind);
    if (eol_setting != 0){
    *eol_setting = LineEndingKind_LF;
    }
}

CUSTOM_COMMAND_SIG(set_eol_mode_to_binary)
CUSTOM_DOC("Puts the buffer in bin line ending mode.")
{
    View_ID view = get_active_view(app, Access_ReadWriteVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
    Managed_Scope scope = buffer_get_managed_scope(app, buffer);
    Line_Ending_Kind *eol_setting = scope_attachment(app, scope, buffer_eol_setting,
                                                     Line_Ending_Kind);
    if (eol_setting != 0){
        *eol_setting = LineEndingKind_Binary;
    }
}

CUSTOM_COMMAND_SIG(set_eol_mode_from_contents)
CUSTOM_DOC("Sets the buffer's line ending mode to match the contents of the buffer.")
{
    View_ID view = get_active_view(app, Access_ReadWriteVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
    Line_Ending_Kind setting = guess_line_ending_kind_from_buffer(app, buffer);
    Managed_Scope scope = buffer_get_managed_scope(app, buffer);
    Line_Ending_Kind *eol_setting = scope_attachment(app, scope, buffer_eol_setting,
                                                     Line_Ending_Kind);
    if (eol_setting != 0){
        *eol_setting = setting;
    }
}

// BOTTOM

