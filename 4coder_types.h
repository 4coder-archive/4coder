

#ifndef ENUM
# define ENUM(type,name) typedef type name; enum name##_
#endif

#ifndef TYPEDEF
# define TYPEDEF typedef
#endif

#ifndef STRUCT
# define STRUCT struct
#endif

#ifndef UNION
# define UNION union
#endif


/* DOC(bool32 is an alias name to signal that an integer parameter or field is for
true/false values.) */
TYPEDEF int32_t bool32;

/* DOC(int_color is an alias name to signal that an integer parameter or field is for
a color value, colors are specified as 24 bit integers in 3 channels: 0xRRGGBB.) */
TYPEDEF uint32_t int_color;

/* DOC(Key_Code is the alias for key codes including raw codes and codes translated
to textual input that takes modifiers into account.) */
TYPEDEF unsigned char Key_Code;

/* DOC(Buffer_ID is used to name a 4coder buffer.  Each buffer has a unique id but
when a buffer is closed it's id may be recycled by future, different buffers.) */
TYPEDEF int32_t Buffer_ID;

/* DOC(View_ID is used to name a 4coder view.  Each view has a unique id in
the interval [1,16].) */
TYPEDEF int32_t View_ID;

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
ENUM(uint32_t, Key_Modifier_Flag){
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
    
    /* DOC(cmdid_undo performs a standard undo behavior.) */
    cmdid_undo,
    /* DOC(cmdid_redo reperforms an edit that was undone.) */
    cmdid_redo,
    /* DOC(cmdid_history_backward performs a step backwards through the file history, which includes previously lost redo branches.) */
    cmdid_history_backward,
    /* DOC(cmdid_history_forward unperforms the previous cmdid_history_backward step if possible.) */
    cmdid_history_forward,
    
    /* DOC(cmdid_interactive_new begins an interactive dialogue to create a new buffer.) */
    cmdid_interactive_new,
    /* DOC(cmdid_interactive_open begins an interactive dialogue to open a file into a buffer.) */
    cmdid_interactive_open,
    /* DOC(cmdid_save_as does not currently work and is likely to be removed rather that fixed.) */
    cmdid_save_as,
    /* DOC(cmdid_interactive_switch_buffer begins an interactive dialogue to choose an open buffer to swap into the active view.) */
    cmdid_interactive_switch_buffer,
    /* DOC(cmdid_interactive_kill_buffer begins an interactive dialogue to choose an open buffer to kill.) */
    cmdid_interactive_kill_buffer,
    
    /* DOC(cmdid_reopen reloads the active buffer's associated file and discards the old buffer contents for the reloaded file.) */
    cmdid_reopen,
    /* DOC(cmdid_save saves the buffer's contents into the associated file.) */
    cmdid_save,
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
    
    // count
    cmdid_count
};

/* DOC(TODO) */
ENUM(uint32_t, Memory_Protect_Flags){
    /* DOC(TODO) */
    MemProtect_Read    = 0x1,
    /* DOC(TODO) */
    MemProtect_Write   = 0x2,
    /* DOC(TODO) */
    MemProtect_Execute = 0x4,
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

/* DOC(A Wrap_Indicator_Mode is used in the buffer setting BufferSetting_WrapIndicator to specify how to indicate that line has been wrapped.) */
ENUM(int32_t, Wrap_Indicator_Mode){
    /* DOC(WrapIndicator_Hide tells the buffer rendering system not to put any indicator on wrapped lines.) */
    WrapIndicator_Hide,
    
    /* DOC(WrapIndicator_Show_After_Line tells the buffer rendering system to put a backslash indicator on wrapped lines right after the last character of the line.) */
    WrapIndicator_Show_After_Line,
    
    /* DOC(WrapIndicator_Show_At_Wrap_Edge tells the buffer rendering system to put a backslash indicator on wrapped lines aligned with the wrap position for that line.) */
    WrapIndicator_Show_At_Wrap_Edge,
};

/* DOC(A Buffer_Setting_ID names a setting in a buffer.) */
ENUM(int32_t, Buffer_Setting_ID){
    /* DOC(BufferSetting_Null is not a valid setting, it is reserved to detect errors.) */
    BufferSetting_Null,
    
    /* DOC(The BufferSetting_Lex setting is used to determine whether to store C++ tokens
    from with the buffer.) */
    BufferSetting_Lex,
    
    /* DOC(The BufferSetting_WrapLine setting is used to determine whether a buffer prefers
    to be viewed with wrapped lines, individual views can be set to override this value after
    being tied to the buffer.) */
    BufferSetting_WrapLine,
    
    /* DOC(The BufferSetting_WrapPosition setting determines after how many pixels
    a line will wrap.  A view set's this value from the global default value
    when the view is created.  This value cannot be set to less than 48, any value passed
    below 48 will cause the position to be set to 48.  This is a potentially expensive
    operation because the wrap positions of the file have to be reindexed. For
    best behavior try to only set this setting once per frame, if possible.) */
    BufferSetting_WrapPosition,
    
    /* DOC(The BufferSetting_MinimumBaseWrapPosition setting is used to increase the with in pixels allotted to a line for wrapping, by setting a minimum position away from the base of the line.  The base of a line is always 0, or the left hand side of the view, in text files.  In code files the base of a line is the amount the line is shifted to the right due to brace nesting.  This setting allows for deeply nested code to remain readable by ensuring lines deep in the nesting get some minimum base width which may be more wrapping space than the non base adjusted wrap position would have allowed.  In any case where the (default wrapping position) is greater than (the base + minimum base position), the larger ) the default will still be used. */
    BufferSetting_MinimumBaseWrapPosition,
    
    /* DOC(The BufferSetting_WrapIndicator setting is used to specify how wrapped lines should be marked so the user can see that they have been wrapped.  The value should be one of the values in the Wrap_Indicator_Mode enum.) DOC_SEE(Wrap_Indicator_Mode) */
    BufferSetting_WrapIndicator,
    
    /* DOC(The BufferSetting_MapID setting specifies the id of the command map that should be
    active when a buffer is active.) */
    BufferSetting_MapID,
    
    /* DOC(The BufferSetting_Eol setting specifies how line ends should be saved to the backing file. 
    A 1 indicates dos endings "\r\n" and a 0 indicates nix endings "\n".) */
    BufferSetting_Eol,
    
    /* DOC(The BufferSetting_Unimportant setting marks a buffer so that it's dirty state will be completely
    ignored.  This means the "dirty" star is hidden and the buffer can be closed without presenting an
    "are you sure" dialogue screen.) */
    BufferSetting_Unimportant,
    
    /* DOC(The BufferSetting_ReadOnly setting marks a buffer so that it can only be returned from buffer
    access calls that include an AccessProtected flag.  By convention this means that edit commands that
    should not be applied to read only buffers will not edit this buffer.) */
    BufferSetting_ReadOnly,
    
    /* DOC(The BufferSetting_VirtualWhitespace settings enables virtual whitespace on a buffer.
    Text buffers with virtual whitespace will set the indentation of every line to zero.
    Buffers with lexing enabled will use virtual white space to present the code with appealing indentation.) */
    BufferSetting_VirtualWhitespace,
};

/* DOC(A View_Setting_ID names an adjustable setting in a view.) */
ENUM(int32_t, View_Setting_ID){
    /* DOC(ViewSetting_Null is not a valid setting, it is reserved to detect errors.) */
    ViewSetting_Null,
    
    /* DOC(The ViewSetting_ShowWhitespace setting determines whether the view highlights
    whitespace in a file.  Whenever the view switches to a new buffer this setting is turned off.) */
    ViewSetting_ShowWhitespace,
    
    /* DOC(The ViewSetting_ShowScrollbar setting determines whether a scroll bar is
    attached to a view in it's scrollable section.) */
    ViewSetting_ShowScrollbar,
    
};

/* DOC(A Buffer_Create_Flag field specifies how a buffer should be created.) */
ENUM(uint32_t, Buffer_Create_Flag){
    /* DOC(BufferCreate_Background is not currently implemented.) */
    BufferCreate_Background = 0x1,
    /* DOC(When BufferCreate_AlwaysNew is set it indicates the buffer should be
    cleared to empty even if it's associated file already has content.) */
    BufferCreate_AlwaysNew  = 0x2,
    /* DOC(When BufferCreate_NeverNew is set it indicates that the buffer should
    only be created if it is an existing file or if a buffer with the given name
    is already open.) */
    BufferCreate_NeverNew   = 0x4,
};


/* DOC(Buffer_Creation_Data is a struct used as a local handle for the creation of a buffer. )
HIDE_MEMBERS() */
STRUCT Buffer_Creation_Data{
    Buffer_Create_Flag flags;
    char fname_space [256];
    int32_t fname_len;
};

/* DOC(A Buffer_Kill_Flag field specifies how a buffer should be killed.) */
ENUM(uint32_t, Buffer_Kill_Flag){
    /* DOC(BufferKill_Background is not currently implemented.) */
    BufferKill_Background  = 0x1,
    /* DOC(When BufferKill_AlwaysKill is set it indicates the buffer should be killed
    without asking, even when the buffer is dirty.) */
    BufferKill_AlwaysKill  = 0x2,
};

/* DOC(An Access_Flag field specifies what sort of permission you grant to an access call.  An access call is usually one the returns a summary struct.  If a 4coder object has a particular protection flag set and the corresponding bit is not set in the access field, that 4coder object is hidden.  On the other hand if a protection flag is set in the access parameter and the object does not have that protection flag, the object is still returned from the access call.) */
ENUM(uint32_t, Access_Flag){
    /* DOC(AccessOpen does not include any bits, it indicates that the access should
    only return objects that have no protection flags set.) */
    AccessOpen      = 0x0,
    /* DOC(AccessProtected is set on buffers and views that are "read only" such as
    the output from an app->exec_system_command call into *build*. This is to prevent
    the user from accidentally editing output that they might prefer to keep in tact.) */
    AccessProtected = 0x1,
    /* DOC(AccessHidden is set on any view that is not currently showing it's file, for
    instance because it is navigating the file system to open a file.) */
    AccessHidden    = 0x2,
    /* DOC(AccessAll is a catchall access for cases where an access call should always
    return an object no matter what it's protection flags are.) */
    AccessAll       = 0xFF
};

/* DOC(A Dirty_State value describes whether changes have been made to a buffer or to an underlying file since the last sync time between the two.  Saving a buffer to it's file or loading the buffer from the file both act as sync points.) */
ENUM(uint32_t, Dirty_State){
    /* DOC(DirtyState_UpToDate indicates that there are no unsaved changes and the underlying system file still agrees with the buffer's state.) */
    DirtyState_UpToDate = 0,
    
    /* DOC(DirtyState_UnsavedChanges indicates that there have been changes in the buffer since the last sync point.) */
    DirtyState_UnsavedChanges = 1,
    
    /* DOC(DirtyState_UnsavedChanges indicates that the underlying file has been edited since the last sync point with the buffer.) */
    DirtyState_UnloadedChanges = 2
};

/* DOC(A Seek_Boundary_Flag field specifies a set of "boundary" types used in seeks for the beginning or end of different types of words.) */
ENUM(uint32_t, Seek_Boundary_Flag){
    BoundaryWhitespace   = 0x1,
    BoundaryToken        = 0x2,
    BoundaryAlphanumeric = 0x4,
    BoundaryCamelCase    = 0x8
};

/* DOC(A Command_Line_Interface_Flag field specifies the behavior of a call to a command line interface.) */
ENUM(uint32_t, Command_Line_Interface_Flag){
    /* DOC(If CLI_OverlapWithConflict is set if output buffer of the new command is already in use by another command which is still executing, the older command relinquishes control of the buffer and both operate simultaneously with only the newer command outputting to the buffer.) */
    CLI_OverlapWithConflict = 0x1,
    /* DOC(If CLI_AlwaysBindToView is set the output buffer will always be set in the active view even if it is already set in another open view.) */
    CLI_AlwaysBindToView    = 0x2,
    /* DOC(If CLI_CursorAtEnd is set the cursor will be kept at the end of the output buffer, otherwise the cursor is kept at the beginning.) */
    CLI_CursorAtEnd         = 0x4,
};

/* DOC(An Auto_Indent_Flag field specifies the behavior of an auto indentation operation.) */
ENUM(uint32_t, Auto_Indent_Flag){
    /* DOC(If AutoIndent_ClearLine is set, then any line that is only whitespace will be cleared to contain nothing at all. otherwise the line is filled with whitespace to match the nearby indentation.) */
    AutoIndent_ClearLine = 0x1,
    /* DOC(If AutoIndent_UseTab is set, then when putting in leading whitespace to align code, as many tabs will be used as possible until the fine grained control of spaces is needed to finish the alignment.) */
    AutoIndent_UseTab    = 0x2,
    /* DOC(If AutoIndent_ExactAlignBlock is set, then block comments are indented by putting the first non-whitespace character of the line in line with the beginning of the comment.) */
    AutoIndent_ExactAlignBlock = 0x4,
    /* DOC(If AutoIndent_FullTokens is set, then the set of lines that are indented is automatically expanded so that any token spanning multiple lines gets entirely indented.) */
    AutoIndent_FullTokens = 0x8,
};

/* DOC(A Set_Buffer_Flag field specifies the behavior of an operation that sets the buffer of a view.) */
ENUM(uint32_t, Set_Buffer_Flag){
    /* DOC(If SetBuffer_KeepOriginalGUI then when the file is set, the view will not switch to it
    if some other GUI was currently up, otherwise any GUI that is up is closed and the view
    switches to the file.) */
    SetBuffer_KeepOriginalGUI = 0x1
};

/* DOC(A Input_Type_Flag field specifies a set of input event types.) */
ENUM(uint32_t, Input_Type_Flag){
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
    /* DOC(This is not totally implemented yet.) */
    EventOnMouseMove   = 0x20,
    
    /* DOC(If EventOnButton is set, all mouse button events are included in the set.) */
    EventOnButton      = (EventOnLeftButton | EventOnRightButton | EventOnWheel),
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

/* DOC(A View_Split_Position specifies where a new view should be placed as a result of a view split operation.) */
ENUM(int32_t, View_Split_Position){
    /* DOC(This value indicates that the new view should be above the existing view.) */
    ViewSplit_Top,
    /* DOC(This value indicates that the new view should be below the existing view.) */
    ViewSplit_Bottom,
    /* DOC(This value indicates that the new view should be left of the existing view.) */
    ViewSplit_Left,
    /* DOC(This value indicates that the new view should be right of the existing view.) */
    ViewSplit_Right
};

/* DOC(Generic_Command acts as a name for a command, and can name an internal command or a custom command.) */
UNION Generic_Command{
    /*DOC(If this Generic_Command represents an internal command the cmdid field will have a value less than cmdid_count, and this field is the command id for the command.)*/
    Command_ID cmdid;
    /*DOC(If this Generic_Command does not represent an internal command the command
    field is the pointer to the custom command..)*/
    Custom_Command_Function *command;
};

/* DOC(Key_Event_Data describes a key event, including the translation to a character, the translation to a character ignoring the state of caps lock, and an array of all the modifiers that were pressed at the time of the event.) */
STRUCT Key_Event_Data{
    /* DOC(This field is the raw keycode which is always non-zero in valid key events.) */
    Key_Code keycode;
    
    /* DOC(This field is the keycode after translation to a character, this is 0 if there is no translation.) */
    Key_Code character;
    
    /* DOC(This field is like the field character, except that the state of caps lock is ignored in the translation.) */
    Key_Code character_no_caps_lock;
    
    /* DOC(This field is an array indicating the state of modifiers at the time of the key press. The array is indexed using the values of Key_Modifier.  A 1 indicates that the corresponding modifier was held, and a 0 indicates that it was not held.)
    
    DOC_SEE(Key_Modifier)
    */
    char modifiers[MDFR_INDEX_COUNT];
};

/* DOC(Mouse_State describes an entire mouse state complete with the position, left and right button states, the wheel state, and whether or not the mouse if in the window.) */
STRUCT Mouse_State{
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
    int32_t x;
    /* DOC(This field contains the y position of the mouse relative to the window where the top side is 0.) */
    int32_t y;
};

/* DOC(
Range describes an integer range typically used for ranges within a buffer. Ranges tend are usually not passed as a Range struct into the API, but this struct is used to return ranges.

Throughout the API ranges are thought of in the form [min,max) where max is "one past the end" of the range that is actually read/edited/modified.) */
UNION Range{
    STRUCT{
        /* DOC(This is the smaller value in the range, it is also the 'start'.) */
        int32_t min;
        /* DOC(This is the larger value in the range, it is also the 'end'.) */
        int32_t max;
    };
    STRUCT{
        /* DOC(This is the start of the range, it is also the 'min'.) */
        int32_t start;
        /* DOC(This is the end of the range, it is also the 'max'.) */
        int32_t end;
    };
};

/*
DOC(File_Info describes the name and type of a file.)
DOC_SEE(File_List)
*/
STRUCT File_Info{
    /* DOC(This field is a null terminated string specifying the name of the file.) */
    char *filename;
    /* DOC(This field specifies the length of the filename string not counting the null terminator.) */
    int32_t filename_len;
    /* DOC(This field indicates that the description is for a folder not a file.) */
    int32_t folder;
};

/* DOC(File_List is a list of File_Info structs.) */
STRUCT File_List{
    /* DOC(This field is for inernal use.) */
    void *block;
    /* DOC(This field is an array of File_Info structs.) */
    File_Info *infos;
    /* DOC(This field specifies the number of struts in the info array.) */
    int32_t count;
    /* DOC(This field is for internal use.) */
    int32_t block_size;
};

/* DOC(Buffer_Identifier acts as a loosely typed description of a buffer that can either be a name or an id.) */
STRUCT Buffer_Identifier{
    /* DOC(This field is the name of the buffer; it need not be null terminated. If id is specified this pointer should be NULL.) */
    char *name;
    
    /* DOC(This field specifies the length of the name string.) */
    int32_t name_len;
    
    /* DOC(This field is the id of the buffer.  If name is specified this should be 0.) */
    Buffer_ID id;
};

/* DOC(This struct is a part of an incomplete feature.) */
STRUCT GUI_Scroll_Vars{
    /* DOC(TODO) */
    float   scroll_y;
    /* DOC(TODO) */
    int32_t target_y;
    /* DOC(TODO) */
    int32_t prev_target_y;
    
    /* DOC(TODO) */
    float   scroll_x;
    /* DOC(TODO) */
    int32_t target_x;
    /* DOC(TODO) */
    int32_t prev_target_x;
};

/* DOC(The Buffer_Seek_Type is is used in a Buffer_Seek to identify which coordinates are suppose to be used for the seek.)
DOC_SEE(Buffer_Seek)
DOC_SEE(4coder_Buffer_Positioning_System)
*/
ENUM(int32_t, Buffer_Seek_Type){
    /* DOC(This value indicates absolute byte index positioning
    where positions are measured as the number of bytes from the start of the file.) */
    buffer_seek_pos,
    /* DOC(This value indicates apparent character index positioning 
    where positions are measured as the number of apparent characters from the starts of the file.) */
    buffer_seek_character_pos,
    /* DOC(This value indicates xy positioning with wrapped lines where the x and y values are in pixels.) */
    buffer_seek_wrapped_xy,
    /* DOC(This value indicates xy positioning with unwrapped lines where the x and y values are in pixels.) */
    buffer_seek_unwrapped_xy,
    /* DOC(This value indicates line-character positioning.
    These coordinates are 1 based to match standard line numbering.) */
    buffer_seek_line_char
};

/* DOC(Buffer_Seek describes the destination of a seek operation.  There are helpers for concisely creating Buffer_Seek structs.  They can be found in 4coder_buffer_types.h.)
DOC_SEE(Buffer_Seek_Type)
DOC_SEE(4coder_Buffer_Positioning_System) */
STRUCT Buffer_Seek{
    /* DOC(The type field determines the coordinate system of the seek operation.) */
    Buffer_Seek_Type type;
    UNION{
        STRUCT {
            /* DOC(The pos field specified the pos when the seek is in absolute position.) */
            int32_t pos;
        };
        STRUCT {
            /* DOC(For xy coordinate seeks, rounding down means that any x in the box of the character lands on that character. For instance when clicking rounding down is the user's expected behavior.  Not rounding down means that the right hand portion of the character's box, which is closer to the next character, will land on that next character.  The unrounded behavior is the expected behavior when moving vertically and keeping the preferred x.) */
            bool32 round_down;
            /* DOC(The x coordinate for xy type seeks.) */
            float x;
            /* DOC(The y coordinate for xy type seeks.) */
            float y;
        };
        STRUCT {
            /* DOC(The line number of a line-character type seek.) */
            int32_t line;
            /* DOC(The character number of a line-character type seek.) */
            int32_t character;
        };
    };
};

/* DOC(Full_Cursor describes the position of a cursor in every buffer coordinate system supported by 4coder. This cursor type requires that the buffer is associated with a view to give the x/y values meaning.)
DOC_SEE(4coder_Buffer_Positioning_System) */
STRUCT Full_Cursor{
    /* DOC(This field contains the cursor's position in absolute byte index positioning.) */
    int32_t pos;
    /* DOC(This field contains the cursor's position in apparent character index positioning.) */
    int32_t character_pos;
    /* DOC(This field contains the number of the line where the cursor is located. This field is one based.) */
    int32_t line;
    /* DOC(This field contains the number of the character from the beginninf of the line where the cursor is located. This field is one based.) */
    int32_t character;
    /* DOC(This field contains the number of the line where the cursor is located, taking the line wrapping into account.  This field is one based.) */
    int32_t wrap_line;
    /* DOC(This field contains the x position measured with unwrapped lines.) */
    float unwrapped_x;
    /* DOC(This field contains the y position measured with unwrapped lines.) */
    float unwrapped_y;
    /* DOC(This field contains the x position measured with wrapped lines.) */
    float wrapped_x;
    /* DOC(This field contains the y position measured with wrapped lines.) */
    float wrapped_y;
};

/* DOC(Partial_Cursor describes the position of a cursor in all of the coordinate systems that a invariant to the View.  In other words the coordinate systems available here can be used on a buffer that is not currently associated with a View.)
DOC_SEE(4coder_Buffer_Positioning_System) */
STRUCT Partial_Cursor{
    /* DOC(This field contains the cursor's position in absolute byte index positioning.) */
    int32_t pos;
    /* DOC(This field contains the number of the character from the beginninf of the line
    where the cursor is located. This field is one based.) */
    int32_t line;
    /* DOC(This field contains the number of the column where the cursor is located. This field is one based.) */
    int32_t character;
};

/* DOC(Buffer_Edit describes a range of a buffer and string to replace that range. A Buffer_Edit has to be paired with a string that contains the actual text that will be replaced into the buffer.) */
STRUCT Buffer_Edit{
    /* DOC(The str_start field specifies the first character in the accompanying string that corresponds with this edit.) */
    int32_t str_start;
    /* DOC(The len field specifies the length of the string being written into the buffer.) */
    int32_t len;
    /* DOC(The start field specifies the start of the range in the buffer to replace in absolute position.) */
    int32_t start;
    /* DOC(The end field specifies one past the end of the range in the buffer to replace in absolute position.) */
    int32_t end;
};

/* DOC(Buffer_Summary acts as a handle to a buffer and describes the state of the buffer.)
DOC_SEE(Access_Flag)
DOC_SEE(Dirty_State) */
STRUCT Buffer_Summary{
    /* DOC(This field indicates whether the Buffer_Summary describes a buffer that is open in 4coder.
    When this field is false the summary is referred to as a "null summary".) */
    bool32 exists;
    /* DOC(If this is not a null summary, this field indicates whether the buffer has finished loading.) */
    bool32 ready;
    /* DOC(If this is not a null summary this field is the id of the associated buffer.
    If this is a null summary then buffer_id is 0.) */
    int32_t buffer_id;
    /*DOC(If this is not a null summary, this field contains flags describing the protection status of the buffer.)*/
    Access_Flag lock_flags;
    
    /* DOC(If this is not a null summary, this field specifies the size of the text in the buffer.) */
    int32_t size;
    /* DOC(If this is not a null summary, this field specifies the number of lines in the buffer.) */
    int32_t line_count;
    
    /* DOC(If this is not a null summary, this field specifies the file name associated to this buffer.) */
    char *file_name;
    /* DOC(This field specifies the length of the file_name string.) */
    int32_t file_name_len;
    
    /* DOC(If this is not a null summary, this field specifies the name of the buffer.) */
    char *buffer_name;
    /* DOC(This field specifies the length of the buffer_name string.) */
    int32_t buffer_name_len;
    
    /* DOC(This field indicates the dirty state of the buffer.) */
    Dirty_State dirty;
    
    /* DOC(If this is not a null summary, this field indicates whether the buffer is set to lex tokens.) */
    bool32 is_lexed;
    /* DOC(If this is not a null summary, this field indicates whether the buffer has up to date tokens available.
    If this field is false, it may simply mean the tokens are still being generated in a background task and will
    be available later.  If that is the case, is_lexed will be true to indicate that the buffer is trying to get
    it's tokens up to date.) */
    bool32 tokens_are_ready;
    /* DOC(If this is not a null summary, this field specifies the id of the command map for this buffer.) */
    int32_t map_id;
    /* DOC(If this is not a null summary, this field indicates whether the buffer 'prefers' wrapped lines.) */
    bool32 unwrapped_lines;
};

/* DOC(View_Summary acts as a handle to a view and describes the state of the view.)
DOC_SEE(Access_Flag)
DOC_SEE(Full_Cursor) */
STRUCT View_Summary{
    /* DOC( This field indicates whether the View_Summary describes a view that is open in 4coder. When this field is false the summary is referred to as a "null summary". ) */
    bool32 exists;
    /* DOC(If this is not a null summary, this field is the id of the associated view. If this is a null summary then view_id is 0. ) */
    int32_t view_id;
    /* DOC(If this is not a null summary, then this is the id of the buffer this view currently sees.) */
    int32_t buffer_id;
    /* DOC(If this is not a null summary, this field contains flags describing the protection status of the view.) */
    Access_Flag lock_flags;
    
    /* DOC(If this is not a null summary, this describes the position of the cursor.) */
    Full_Cursor cursor;
    /* DOC(If this is not a null summary, this describes the position of the mark.) */
    Full_Cursor mark;
    /* DOC(If this is not a null summary, this is the x position that is maintained in vertical navigation.) */
    float preferred_x;
    /* DOC(If this is not a null summary, this specifies the height of a line rendered in the view.) */
    float line_height;
    /* DOC(If this is not a null summary, this indicates that the view is set to render with unwrapped lines.) */
    bool32 unwrapped_lines;
    /* DOC(If this is not a null summary, this indicates that the view is set to highlight white space.) */
    bool32 show_whitespace;
    
    /* DOC(If this is not a null summary, this describes the screen position in which this view's is displayed.) */
    i32_Rect view_region;
    /* DOC(If this is not a null summary, this describes the screen position in which this view's buffer is displayed.  This is different from view_region, because it does not include any fixed height GUI at the top of the view.) */
    i32_Rect file_region;
    /* DOC(If this is not a null summary, this describes the scrolling position inside the view.) */
    GUI_Scroll_Vars scroll_vars;
};

/*
DOC(User_Input describes a user input event which can be either a key press or mouse event.)
DOC_SEE(User_Input_Type_ID)
DOC_SEE(Generic_Command)
*/
STRUCT User_Input{
    /* DOC(This field specifies whether the event was a key press or mouse event.) */
    User_Input_Type_ID type;
    /* DOC(This field indicates that an abort event has occurred and the command needs to shut down.) */
    bool32 abort;
    UNION{
        /* DOC(This field describes a key press event.) */
        Key_Event_Data key;
        /* DOC(This field describes a mouse input event.) */
        Mouse_State mouse;
    };
    /* DOC(If this event would trigger a command, this field specifies what the command would be.) */
    Generic_Command command;
};

/* DOC(Query_Bar is a struct used to store information in the user's control
that will be displayed as a drop down bar durring an interactive command.) */
STRUCT Query_Bar{
    /* DOC(This specifies the prompt portion of the drop down bar.) */
    String prompt;
    /* DOC(This specifies the main string portion of the drop down bar.) */
    String string;
};

/* DOC(This feature is not implemented.) */
STRUCT Event_Message{
    /* DOC(This feature is not implemented.) */
    int32_t type;
};

/* 
DOC(Theme_Color stores a style tag/color pair, for the purpose of setting and getting colors in the theme.)
DOC_SEE(Style_Tag)
DOC_SEE(int_color)
*/
STRUCT Theme_Color{
    /* DOC(The style slot in the style palette.) */
    Style_Tag tag;
    /* DOC(The color in the slot.) */
    int_color color;
};

/* DOC(A Buffer_Batch_Edit_Type is a type of batch operation.) */
ENUM(int32_t, Buffer_Batch_Edit_Type){
    /* DOC(The BatchEdit_Normal operation is always correct but does the most work if there are tokens to correct.) */
    BatchEdit_Normal,
    /* DOC(The BatchEdit_PreserveTokens operation is one in which none of the edits add, delete, or change any tokens.
    This usually applies when whitespace is being replaced with whitespace.) */
    BatchEdit_PreserveTokens
};

/*
DOC(This struct is used to bundle the parameters of the buffer_batch_edit function.  It is convenient
for a few functions that return a batch edit to the user.)
DOC_SEE(buffer_batch_edit)
*/
STRUCT Buffer_Batch_Edit{
    /* DOC(The pointer to the edit string buffer.) */
    char *str;
    /* DOC(The length of the edit string buffer.) */
    int32_t str_len;
    
    /* DOC(The array of edits to be applied.) */
    Buffer_Edit *edits;
    /* DOC(The number of edits in the array.) */
    int32_t edit_count;
};


