/*
4coder_examples.cpp - Commands that are included mainly to serve as example code for
customization writers.
*/

// TOP

// example: History_Group
// example: history_group_begin
// example: history_group_end
CUSTOM_COMMAND_SIG(double_backspace)
CUSTOM_DOC("Example of history group helpers")
{
    View_ID view = get_active_view(app, Access_ReadWriteVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
    History_Group group = history_group_begin(app, buffer);
    backspace_char(app);
    backspace_char(app);
    history_group_end(group);
}

// BOTTOM

