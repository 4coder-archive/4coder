

/* DOC(Key_Code is the type alias for key codes.) */
typedef unsigned char Key_Code;

#define ENUM(type,name) typedef type name; enum name##_
#define FLAGENUM(name) typedef uint32_t name; enum name##_

/* DOC(A Key_Modifier acts as an index for specifying modifiers in arrays.) */
ENUM(int32_t, Key_Modifier){
	MDFR_SHIFT_INDEX,
	MDFR_CONTROL_INDEX,
	MDFR_ALT_INDEX,
    MDFR_CAPS_INDEX,
    MDFR_HOLD_INDEX,
    
    /* DOC(MDFR_INDEX_COUNT is used to specify the number of modifiers supported.) */
	MDFR_INDEX_COUNT
};

/* DOC(A Key_Modifier_Flag field is used to specify a specific state of modifiers.
Flags can be combined with bit or to specify a state with multiple modifiers.) */
FLAGENUM(Key_Modifier_Flag){
    /* DOC(MDFR_NONE specifies that no modifiers are pressed.) */
    MDFR_NONE  = 0x0,
    MDFR_CTRL  = 0x1,
    MDFR_ALT   = 0x2,
    MDFR_SHIFT = 0x4,
};

/* DOC(A Command_ID is used as a name for commands implemented internally in 4coder.) */
ENUM(uint64_t, Command_ID){
    /* DOC(cmdid_null is set aside to always be zero and is not associated with any command.) */
    cmdid_null,
    
    /* DOC(cmdid_center_view centers the view vertically on the cursor.) */
    cmdid_center_view,
    /* DOC(cmdid_left_adjust_view adjusts the view to be just left of the cursor's position.) */
    cmdid_left_adjust_view,
    /* DOC(cmdid_page_up moves the view up one whole screen height and centers the cursor there.) */
    cmdid_page_up,
    /* DOC(cmdid_page_down moves the view down one whole screen height and centers the cursor there.) */
    cmdid_page_down,
    
    /* DOC(cmdid_word_complete begins or continues cycling through completions for a partial word.) */
    cmdid_word_complete,
    
    /* DOC(cmdid_undo performs a standard undo behavior.) */
    cmdid_undo,
    /* DOC(cmdid_redo reperforms an edit that was undone.) */
    cmdid_redo,
    /* DOC(cmdid_history_backward performs a step backwards through the file history, which includes previously lost redo branches.) */
    cmdid_history_backward,
    /* DOC(cmdid_history_forward unperforms the previous cmdid_history_backward step if possib.e) */
    cmdid_history_forward,
    
    /* DOC(cmdid_to_uppercase makes all the alphabetic characters in the cursor/mark range uppercase.) */
    cmdid_to_uppercase,
    /* DOC(cmdid_to_uppercase makes all the alphabetic characters in the cursor/mark range lowercase.) */
    cmdid_to_lowercase,
    
    /* DOC(cmdid_toggle_line_wrap toggles the line wrap setting of the active view. ) */
    cmdid_toggle_line_wrap,
    /* DOC(cmdid_toggle_line_wrap toggles the show whitespace setting of the active view.) */
    cmdid_toggle_show_whitespace,
    /* DOC(cmdid_clean_all_lines deletes extra whitespace out the currently active buffer.) */
    cmdid_clean_all_lines,
    /* DOC(cmdid_eol_dosify sets the currently active buffer to dos save mode where the end of a line is "\r\n") */
    cmdid_eol_dosify,
    /* DOC(cmdid_eol_nixify sets the currently active buffer to nix save mode where the end of a line is "\n") */
    cmdid_eol_nixify,
    
    /* DOC(cmdid_interactive_new begins an interactive dialogue to create a new buffer.) */
    cmdid_interactive_new,
    /* DOC(cmdid_interactive_open begins an interactive dialogue to open a file into a buffer.) */
    cmdid_interactive_open,
    /* DOC(cmdid_reopen reloads the active buffer's associated file and discards the old buffer contents for the reloaded file.) */
    cmdid_reopen,
    /* DOC(cmdid_save saves the buffer's contents into the associated file.) */
    cmdid_save,
    /* DOC(cmdid_save_as does not currently work and is likely to be removed rather that fixed.) */
    cmdid_save_as,
    /* DOC(cmdid_interactive_switch_buffer begins an interactive dialogue to choose an open buffer to swap into the active view.) */
    cmdid_interactive_switch_buffer,
    /* DOC(cmdid_interactive_kill_buffer begins an interactive dialogue to choose an open buffer to kill.) */
    cmdid_interactive_kill_buffer,
    /* DOC(cmdid_kill_buffer tries to kill the active buffer.) */
    cmdid_kill_buffer,
    
    /* DOC(cmdid_open_color_tweaker opens the theme editing GUI.) */
    cmdid_open_color_tweaker,
    /* DOC(cmdid_open_config opens the configuration menu.) */
    cmdid_open_config,
    /* DOC(cmdid_open_menu opens the top level menu. ) */
    cmdid_open_menu,
    /* DOC(cmdid_open_debug opens the debug information viewer mode.) */
    cmdid_open_debug,
    
    /* DOC(cmdid_open_panel_vsplit splits the current panel into two with a vertical divider.) */
    cmdid_open_panel_vsplit,
    /* DOC(cmdid_open_panel_hsplit splits the current panel into two with a horizontal divider.) */
    cmdid_open_panel_hsplit,
    /* DOC(cmdid_close_panel closes the active panel.) */
    cmdid_close_panel,
    /* DOC(cmdid_change_active_panel cycles to the next open panel.) */
    cmdid_change_active_panel,
    
    // count
    cmdid_count
};

/* DOC(User_Input_Type_ID specifies a type of user input event.) */
ENUM(int32_t, User_Input_Type_ID){
    /* DOC(UserInputNone indicates that no event has occurred.) */
    UserInputNone,
    /* DOC(UserInputKey indicates an event which can be described by a Key_Event_Data struct.) */
    UserInputKey,
    /* DOC(UserInputMouse indicates an event which can be described by a Mouse_State struct.) */
    UserInputMouse
};

/* DOC(Event_Message_Type_ID is a part of an unfinished feature.) */
ENUM(int32_t, Event_Message_Type_ID){
    /* DOC( TODO. ) */
    EventMessage_NoMessage,
    /* DOC( TODO. ) */
    EventMessage_OpenView,
    /* DOC( TODO. ) */
    EventMessage_Frame,
    /* DOC( TODO. ) */
    EventMessage_CloseView
};

/* DOC(A Buffer_Setting_ID names a setting in a buffer.) */
ENUM(int32_t, Buffer_Setting_ID){
    /* DOC(BufferSetting_Null is not a valid setting, it is reserved to detect errors.) */
    BufferSetting_Null,
    /* DOC(The BufferSetting_Lex setting is used to determine whether to store C++ tokens from with the buffer.) */
    BufferSetting_Lex,
    /* DOC(The BufferSetting_WrapLine setting is used to determine whether a buffer prefers to be viewed with wrapped lines,
    individual views can be set to override this value after being tied to the buffer.) */
    BufferSetting_WrapLine,
    /* DOC(The BufferSetting_MapID setting specifies the id of the command map that should be active when a buffer is active.) */
    BufferSetting_MapID,
};

/* DOC(A View_Setting_ID names a setting in a view.) */
ENUM(int32_t, View_Setting_ID){
    /* DOC(ViewSetting_Null is not a valid setting, it is reserved to detect errors.) */
    ViewSetting_Null,
    /* DOC(The ViewSetting_ShowScrollbar setting determines whether a scroll bar is attached to a view in it's scrollable section.) */
    ViewSetting_ShowScrollbar,
};

/* DOC(A Buffer_Create_Flag field specifies how a buffer should be created.) */
FLAGENUM(Buffer_Create_Flag){
    /* DOC(BufferCreate_Background is not currently implemented.) */
    BufferCreate_Background = 0x1,
    /* DOC(When BufferCreate_AlwaysNew is et it indicates the buffer should be
    cleared to empty even if it's associated file already has content.) */
    BufferCreate_AlwaysNew  = 0x2,
};

/* DOC(A Buffer_Kill_Flag field specifies how a buffer should be killed.) */
FLAGENUM(Buffer_Kill_Flag){
    /* DOC(BufferKill_Background is not currently implemented.) */
    BufferKill_Background  = 0x1,
    /* DOC(When BufferKill_AlwaysKill is set it indicates the buffer should be killed
    without asking, even when the buffer is dirty.) */
    BufferKill_AlwaysKill  = 0x2,
};

/* DOC(An Access_Flag field specifies what sort of permission you grant to an
access call.  An access call is usually one the returns a summary struct.  If a
4coder object has a particular protection flag set and the corresponding bit is
not set in the access field, that 4coder object is hidden.  On the other hand if
a protection flag is set in the access parameter and the object does not have
that protection flag, the object is still returned from the access call.) TODO */
FLAGENUM(Access_Flag){
    /* DOC(AccessOpen does not include any bits, it indicates that the access should
    only return objects that have no protection flags set.) */
    AccessOpen      = 0x0,
    /* DOC(AccessProtected is set on buffers and views that are "read only" such as
    the output from an app->exec_system_command call such as *build*. This is to prevent
    the user from accidentally editing output that they might prefer to keep in tact.) */
    AccessProtected = 0x1,
    /* DOC(AccessHidden is set on any view that is not currently showing it's file, for
    instance because it is navigating the file system to open a file.) */
    AccessHidden    = 0x2,
    /* DOC(AccessAll is a catchall access for cases where an access call should always
    return an object no matter what it's protection flags are.) */
    AccessAll       = 0xFF
};

/* DOC(A Seek_Boundary_Flag field specifies a set of "boundary" types used in seeks for the
beginning or end of different types of words.) */
FLAGENUM(Seek_Boundary_Flag){
    BoundaryWhitespace   = 0x1,
    BoundaryToken        = 0x2,
    BoundaryAlphanumeric = 0x4,
    BoundaryCamelCase    = 0x8
};

/* DOC(A Command_Line_Input_Flag field specifies the behavior of a call to a command line interface.) */
FLAGENUM(Command_Line_Input_Flag){
    /* DOC(If CLI_OverlapWithConflict is set if output buffer of the new command is already
    in use by another command which is still executing, the older command relinquishes control
    of the buffer and both operate simultaneously with only the newer command outputting to
    the buffer.) */
    CLI_OverlapWithConflict = 0x1,
    /* DOC(If CLI_AlwaysBindToView is set the output buffer will always be set in the active
    view even if it is already set in another open view.) */
    CLI_AlwaysBindToView    = 0x2,
    /* DOC(If CLI_CursorAtEnd is set the cursor will be kept at the end of the output buffer,
    otherwise the cursor is kept at the beginning.) */
    CLI_CursorAtEnd         = 0x4,
};

/* DOC(An Auto_Indent_Flag field specifies the behavior of an auto indentation operation.) */
FLAGENUM(Auto_Indent_Flag){
    /* DOC(If AutoIndent_ClearLine is set, then any line that is only whitespace will
    be cleared to contain nothing at all. otherwise the line is filled with whitespace
    to match the nearby indentation.) */
    AutoIndent_ClearLine = 0x1,
    /* DOC(If AutoIndent_UseTab is set, then when putting in leading whitespace to align
    code, as many tabs will be used as possible until the fine grained control of spaces
    is needed to finish the alignment.) */
    AutoIndent_UseTab    = 0x2
};

/* DOC(A Set_Buffer_Flag field specifies the behavior of an operation that sets the buffer of a view.) */
FLAGENUM(Set_Buffer_Flag){
    /* DOC(If SetBuffer_KeepOriginalGUI then when the file is set, the view will not switch to it
    if some other GUI was currently up, otherwise any GUI that is up is closed and the view
    switches to the file.) */
    SetBuffer_KeepOriginalGUI = 0x1
};

/* DOC(A Input_Type_Flag field specifies a set of input event types.) */
FLAGENUM(Input_Type_Flag){
    /* DOC(If EventOnAnyKey is set, all keyboard events are included in the set.) */
    EventOnAnyKey      = 0x1,
    /* DOC(If EventOnEsc is set, any press of the escape key is included in the set.) */
    EventOnEsc         = 0x2,
    /* DOC(If EventOnLeftButton is set, left clicks are included in the set.) */
    EventOnLeftButton  = 0x4,
    /* DOC(If EventOnRightButton is set, right clicks are included in the set.) */
    EventOnRightButton = 0x8,
    /* DOC(If EventOnWheel is set, any wheel movement is included in the set.) */
    EventOnWheel       = 0x10,
    /* DOC(If EventOnButton is set, all mouse button events are included in the set.) */
    EventOnButton      = (EventOnLeftButton | EventOnRightButton | EventOnWheel),
    
    /* DOC(This is not totally implemented yet.) */
    EventOnMouseMove   = 0x20,
    /* DOC(This is not totally implemented yet.) */
    EventOnMouse       = (EventOnButton | EventOnMouseMove),
    
    /* DOC(EventAll is a catch all name for including all possible events in the set.) */
    EventAll           = 0xFF
};

/* DOC(A Mouse_Cursor_Show_Type value specifes a mode for 4coder to handle the mouse cursor.) */
ENUM(int32_t, Mouse_Cursor_Show_Type){
    /* DOC(The MouseCursorShow_Never mode never shows the cursor.) */
    MouseCursorShow_Never,
    /* DOC(The MouseCursorShow_Never mode always shows the cursor.) */
    MouseCursorShow_Always,
//    MouseCursorShow_WhenActive,// TODO(allen): coming soon
};

/* DOC(
Key_Event_Data describes a key event, including the
translation to a character, the translation to
a character ignoring the state of caps lock, and
an array of all the modifiers that were pressed
at the time of the event.
)
*/
struct Key_Event_Data{
    /* DOC(This field is the raw keycode which is always non-zero in valid key events.) */
	Key_Code keycode;
    
    /* DOC(This field is the keycode after translation to a character, this is 0 if there is no translation.) */
	Key_Code character;
    
    /* DOC(
    This field is like the field character, except that the state of caps lock is ignored in the translation.
    ) */
	Key_Code character_no_caps_lock;
    
    /* DOC(
    This field is an array indicating the state of modifiers at the time of the key press.
    The array is indexed using the values of Key_Modifier.  A 1 indicates that the corresponding
    modifier was held, and a 0 indicates that it was not held.
    )
    DOC_SEE(Key_Modifier)
    */
	char modifiers[MDFR_INDEX_COUNT];
};

/* DOC(
Mouse_State describes an entire mouse state complete with the position,
left and right button states, the wheel state, and whether or not the
mouse if in the window.
) */
struct Mouse_State{
    /* DOC(This field indicates that the left button is held.) */
    char l;
    /* DOC(This field indicates that the right button is held.) */
    char r;
    /* DOC(This field indicates that the left button was pressed this frame.) */
    char press_l;
    /* DOC(This field indicates that the right button was pressed this frame.) */
    char press_r;
    /* DOC(This field indicates that the left button was released this frame.) */
    char release_l;
    /* DOC(This field indicates that the right button was released this frame.) */
    char release_r;
    /* DOC(
    This field is 0 when the wheel has not moved, it is 1 for a downward motion and -1 for an upward motion.
    ) */
	char wheel;
    /* DOC(This field indicates that the mouse is outside of the window.) */
	char out_of_window;
    /* DOC(This field contains the x position of the mouse relative to the window where the left side is 0.) */
    int x;
    /* DOC(This field contains the y position of the mouse relative to the window where the top side is 0.) */
    int y;
};

/* DOC(
Range describes an integer range typically used for ranges within a buffer.
Ranges tend are usually not passed as a Range struct into the API, but this
struct is used to return ranges.

Throughout the API ranges are thought of in the form [min,max) where max is
"one past the end" of the range that is actually read/edited/modified.
) */
union Range{
    struct{
        /* DOC(This is the smaller value in the range, it is also the 'start'.) */
        int min;
        /* DOC(This is the larger value in the range, it is also the 'end'.) */
        int max;
    };
    struct{
        /* DOC(This is the start of the range, it is also the 'min'.) */
        int start;
        /* DOC(This is the end of the range, it is also the 'max'.) */
        int end;
    };
};

/*
DOC(File_Info describes the name and type of a file.)
DOC_SEE(File_List)
*/
struct File_Info{
    /* DOC(This field is a null terminated string specifying the name of the file.) */
    char *filename;
    
    /* DOC(This field specifies the length of the filename string not counting the null terminator.) */
    int filename_len;
    
    /* DOC(This field indicates that the description is for a folder not a file.) */
    int folder;
};

/* DOC(File_List is a list of File_Info structs.) */
struct File_List{
    /* DOC(This field is for inernal use.) */
    void *block;
    /* DOC(This field is an array of File_Info structs.) */
    File_Info *infos;
    /* DOC(This field specifies the number of struts in the info array.) */
    int count;
    /* DOC(This field is for internal use.) */
    int block_size;
};

/* DOC(
Buffer_Identifier acts as a loosely typed description of a buffer that
can either be a name or an id.  If the
) */
struct Buffer_Identifier{
    /* DOC(
    This field is the name of the buffer, need not be null terminated.
    If id is specified this should be NULL.
    ) */
    char *name;
    
    /* DOC(This field is specifies the length of the name string.) */
    int name_len;
    
    /* DOC(This field is the id of the buffer.  If name is specified this should be 0.) */
    int id;
};

/* DOC(
Buffer_Summary acts as a handle to a buffer and describes the state of the buffer.
) */
struct Buffer_Summary{
    /* DOC(
    This field indicates whether the Buffer_Summary describes a buffer that is open in 4coder.
    When this field is false the summary is referred to as a "null summary".
    ) */
    int exists;
    /* DOC(If this is not a null summary, this field indicates whether the buffer has finished loading.) */
    int ready;
    /* DOC(
    If this is not a null summary this field is the id of the associated buffer.
    If this is a null summary then buffer_id is 0.
    ) */
    int buffer_id;
    /*
    DOC(If this is not a null summary, this field contains flags describing the protection status of the buffer.)
    DOC_SEE(Access_Flag)
    */
    unsigned int lock_flags;
    
    /* DOC(If this is not a null summary, this field specifies the size of the text in the buffer.) */
    int size;
    
    /* DOC(If this is not a null summary, this field specifies the file name associated to this buffer.) */
    char *file_name;
    /* DOC(This field specifies the length of the file_name string.) */
    int file_name_len;
    
    /* DOC(If this is not a null summary, this field specifies the name of the buffer.) */
    char *buffer_name;
    /* DOC(This field specifies the length of the buffer_name string.) */
    int buffer_name_len;
    
    /* DOC(This is a hold over from an old system, consider it deprecated.) */
    int buffer_cursor_pos;
    /* DOC(If this is not a null summary, this field indicates whether the buffer is set to lex tokens.) */
    int is_lexed;
    /* DOC(If this is not a null summary, this field specifies the id of the command map for this buffer.) */
    int map_id;
};

/* DOC(
View_Summary acts as a handle to a view and describes the state of the view.
) */
struct View_Summary{
    /* DOC(
    This field indicates whether the View_Summary describes a view that is open in 4coder.
    When this field is false the summary is referred to as a "null summary".
    ) */
    int exists;
    /* DOC(
    If this is not a null summary, this field is the id of the associated view.
    If this is a null summary then view_id is 0.
    ) */
    int view_id;
    /* DOC(If this is not a null summary, and this view looks at a buffer, this is the id of the buffer.) */
    int buffer_id;
    /*
    DOC(If this is not a null summary, this field contains flags describing the protection status of the view.)
    DOC_SEE(Access_Flag)
    */
    unsigned int lock_flags;
    
    /*
    DOC(If this is not a null summary, this describes the position of the cursor.)
    DOC_SEE(Full_Cursor)
    */
    Full_Cursor cursor;
    /*
    DOC(If this is not a null summary, this describes the position of the mark.)
    DOC_SEE(Full_Cursor)
    */
    Full_Cursor mark;
    /* DOC(If this is not a null summary, this is the x position that is maintained in vertical navigation.) */
    float preferred_x;
    /* DOC(If this is not a null summary, this specifies the height of a line rendered in the view.) */
    float line_height;
    /* DOC(If this is not a null summary, this indicates that the view is set to render with unwrapped lines.) */
    int unwrapped_lines;
    /* DOC(If this is not a null summary, this indicates that the view is set to highlight white space.) */
    int show_whitespace;
    
    /* DOC(This feature is not fully implemented yet.) */
    i32_Rect file_region;
    /* DOC(This feature is not fully implemented yet.) */
    GUI_Scroll_Vars scroll_vars;
};

/* DOC(User_Input describes a user input event which can be either a key press or mouse event.) */
struct User_Input{
    /*
    DOC(This field specifies whether the event was a key press or mouse event.)
    DOC_SEE(User_Input_Type_ID)
    */
    int type;
    /* DOC(This field indicates that an abort event has occurred and the command needs to shut down.) */
    int abort;
    union{
        /* DOC(This field describes a key press event.) */
        Key_Event_Data key;
        /* DOC(This field describes a mouse input event.) */
        Mouse_State mouse;
    };
    /*
    DOC(If this event would trigger a command, this field specifies what the command would be.)
    TODO
    */
    unsigned long long command;
};

/* DOC(Query_Bar is a struct used to store information in the user's control
that will be displayed as a drop down bar durring an interactive command.) */
struct Query_Bar{
    /* DOC(This specifies the prompt portion of the drop down bar.) */
    String prompt;
    /* DOC(This specifies the main string portion of the drop down bar.) */
    String string;
};

/* DOC(This feature is not implemented.) */
struct Event_Message{
    /* DOC(This feature is not implemented.) */
    int type;
};

/* 
DOC(Theme_Color stores a style tag/color pair, for the purpose of setting and getting colors in the theme .)
DOC_SEE(Style_Tag)
*/
struct Theme_Color{
    Style_Tag tag;
    /* DOC(This field specifies a color in a 24, bit 3 channel RGB integer.) */
    uint32_t color;
};


