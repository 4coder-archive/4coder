
#if !defined(FCODER_TYPES_H)
#define FCODER_TYPES_H

#if !defined(FCODER_META_TAGS)
#define FCODER_META_TAGS

# define ENUM(type,name) typedef type name; enum name##_
# define TYPEDEF typedef
# define TYPEDEF_FUNC typedef
# define STRUCT struct
# define UNION union
# define GLOBAL_VAR static

#endif

/* DOC(bool32 is an alias name to signal that an integer parameter or field is for true/false values.) */
TYPEDEF int32_t bool32;

/* DOC(int_color is an alias name to signal that an integer parameter or field is for a color value, colors are specified as 32 bit integers (8 bit + 24 bit) with 3 channels: 0x**RRGGBB.) */
TYPEDEF uint32_t int_color;

/* DOC(Parse_Context_ID identifies a parse context, which is a guiding rule for the parser.  Each buffer sets which parse context to use when it is parsed.) */
TYPEDEF uint32_t Parse_Context_ID;

/* DOC(Buffer_ID is used to name a 4coder buffer.  Each buffer has a unique id but when a buffer is closed it's id may be recycled by future, different buffers.) */
TYPEDEF int32_t Buffer_ID;

/* DOC(View_ID is used to name a 4coder view.  Each view has a unique id in the interval [1,16].) */
TYPEDEF int32_t View_ID;

/* DOC(A Key_Modifier_Index acts as an index for specifying modifiers in arrays.) */
ENUM(int32_t, Key_Modifier_Index){
    MDFR_SHIFT_INDEX,
    MDFR_CONTROL_INDEX,
    MDFR_ALT_INDEX,
    MDFR_COMMAND_INDEX,
    
    /* DOC(MDFR_INDEX_BINDABLE_COUNT is used to specify the number of supported modifiers that can be used in key bindings.) */
    MDFR_INDEX_BINDABLE_COUNT,
    
    MDFR_CAPS_INDEX = MDFR_INDEX_BINDABLE_COUNT,
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
    MDFR_CMND  = 0x4,
    MDFR_SHIFT = 0x8,
};

/* DOC(A Command_ID is used as a name for commands implemented internally in 4coder.) */
ENUM(uint64_t, Command_ID){
    /* DOC(cmdid_null is set aside to always be zero and is not associated with any command.) */
    cmdid_null,
    
    /* DOC(cmdid_undo performs a standard undo behavior.) */
    cmdid_undo,
    /* DOC(cmdid_redo reperforms an edit that was undone.) */
    cmdid_redo,
    
    /* DOC(cmdid_interactive_new begins an interactive dialogue to create a new buffer.) */
    cmdid_interactive_new,
    /* DOC(cmdid_interactive_open begins an interactive dialogue to open a file into a buffer.) */
    cmdid_interactive_open,
    /* DOC(cmdid_interactive_open_or_new begins an interactive dialogue to open a file into a buffer, if the name specified does not match any existing buffer, a new buffer is created instead.) */
    cmdid_interactive_open_or_new,
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
    /* DOC(cmdid_open_debug opens the debug information viewer mode.) */
    cmdid_open_debug,
    
    // count
    cmdid_count
};

/* DOC(Flags for describing the memory protection status of pages that come back from memory allocate.  Some combinations may not be available on some platforms, but you are gauranteed to get back a page with at least the permissions you requested.  For example if you request just write permission, you may get back a page with read and write permission, but you will never get back a page that doesn't have write permission.) */
ENUM(uint32_t, Memory_Protect_Flags){
    /* DOC(Allows the page to be read.) */
    MemProtect_Read    = 0x1,
    /* DOC(Allows the page to be written.) */
    MemProtect_Write   = 0x2,
    /* DOC(Allows the page to be executed.) */
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

/* DOC(A Wrap_Indicator_Mode is used in the buffer setting BufferSetting_WrapIndicator to specify how to indicate that line has been wrapped.) */
ENUM(int32_t, Wrap_Indicator_Mode){
    /* DOC(WrapIndicator_Hide tells the buffer rendering system not to put any indicator on wrapped lines.) */
    WrapIndicator_Hide,
    
    /* DOC(WrapIndicator_Show_After_Line tells the buffer rendering system to put a backslash indicator on wrapped lines right after the last character of the line.) */
    WrapIndicator_Show_After_Line,
    
    /* DOC(WrapIndicator_Show_At_Wrap_Edge tells the buffer rendering system to put a backslash indicator on wrapped lines aligned with the wrap position for that line.) */
    WrapIndicator_Show_At_Wrap_Edge,
};

/* DOC(A Global_Setting_ID names a setting.) */
ENUM(int32_t, Global_Setting_ID){
    /* DOC(GlobalSetting_Null is not a valid setting, it is reserved to detect errors.) */
    GlobalSetting_Null,
    
    /* DOC(When GlobalSetting_LAltLCtrlIsAltGr is enabled, keyboard layouts with AltGr will also interpret the left alt and control keys as AltGr.  If you do not use a keyboard with AltGr you should have this turned off.  This setting is only relevant on Windows and has no effect on other systems.) */
    GlobalSetting_LAltLCtrlIsAltGr,
};

/* DOC(A Buffer_Setting_ID names a setting in a buffer.) */
ENUM(int32_t, Buffer_Setting_ID){
    /* DOC(BufferSetting_Null is not a valid setting, it is reserved to detect errors.) */
    BufferSetting_Null,
    
    /* DOC(The BufferSetting_Lex setting is used to determine whether to store C++ tokens from with the buffer.) */
    BufferSetting_Lex,
    
    /* DOC(The BufferSetting_LexWithoutStrings tells the system to treat string and character marks as identifiers instead of strings.  This settings does nothing if the buffer does not have lexing turned on.) */
    BufferSetting_LexWithoutStrings,
    
    /* DOC(The BufferSetting_ParserContext setting determines the parser context that guides the parser for the contents of this buffer.  By default the value is 0, which represents the default C++ context.) */
    BufferSetting_ParserContext,
    
    /* DOC(The BufferSetting_WrapLine setting is used to determine whether a buffer prefers to be viewed with wrapped lines, individual views can be set to override this value after being tied to the buffer.) */
    BufferSetting_WrapLine,
    
    /* DOC(The BufferSetting_WrapPosition setting determines after how many pixels a line will wrap.  A view set's this value from the global default value when the view is created.  This value cannot be set to less than 48, any value passed below 48 will cause the position to be set to 48.  This is a potentially expensive operation because the wrap positions of the file have to be reindexed. For best behavior try to only set this setting once per frame, if possible.) */
    BufferSetting_WrapPosition,
    
    /* DOC(The BufferSetting_MinimumBaseWrapPosition setting is used to increase the with in pixels allotted to a line for wrapping, by setting a minimum position away from the base of the line.  The base of a line is always 0, or the left hand side of the view, in text files.  In code files the base of a line is the amount the line is shifted to the right due to brace nesting.  This setting allows for deeply nested code to remain readable by ensuring lines deep in the nesting get some minimum base width which may be more wrapping space than the non base adjusted wrap position would have allowed.  In any case where the (default wrapping position) is greater than (the base + minimum base position), the larger ) the default will still be used. */
    BufferSetting_MinimumBaseWrapPosition,
    
    /* DOC(The BufferSetting_WrapIndicator setting is used to specify how wrapped lines should be marked so the user can see that they have been wrapped.  The value should be one of the values in the Wrap_Indicator_Mode enum.) DOC_SEE(Wrap_Indicator_Mode) */
    BufferSetting_WrapIndicator,
    
    /* DOC(The BufferSetting_MapID setting specifies the id of the command map that should be active when a buffer is active.) */
    BufferSetting_MapID,
    
    /* DOC(The BufferSetting_Eol setting specifies how line ends should be saved to the backing file.  A 1 indicates dos endings "\r\n" and a 0 indicates nix endings "\n".) */
    BufferSetting_Eol,
    
    /* DOC(The BufferSetting_Unimportant setting marks a buffer so that its dirty state will be forced to stay at DirtyState_UpToDate when the buffer is edited or when the buffer's paired file, if it has one, is edited.) */
    BufferSetting_Unimportant,
    
    /* DOC(The BufferSetting_ReadOnly setting marks a buffer so that it can only be returned from buffer access calls that include an AccessProtected flag.  By convention this means that edit commands that should not be applied to read only buffers will not edit this buffer.) */
    BufferSetting_ReadOnly,
    
    /* DOC(The BufferSetting_VirtualWhitespace setting enables virtual whitespace on a buffer. Text buffers with virtual whitespace will set the indentation of every line to zero. Buffers with lexing enabled will use virtual white space to present the code with appealing indentation.) */
    BufferSetting_VirtualWhitespace,
};

/* DOC(A View_Setting_ID names an adjustable setting in a view.) */
ENUM(int32_t, View_Setting_ID){
    /* DOC(ViewSetting_Null is not a valid setting, it is reserved to detect errors.) */
    ViewSetting_Null,
    
    /* DOC(The ViewSetting_ShowWhitespace setting determines whether the view highlights whitespace in a file.  Whenever the view switches to a new buffer this setting is turned off.) */
    ViewSetting_ShowWhitespace,
    
    /* DOC(The ViewSetting_ShowScrollbar setting determines whether a scroll bar is attached to a view in it's scrollable section.) */
    ViewSetting_ShowScrollbar,
    
    /* DOC(The ViewSetting_ShowFileBar settings determines whether to show the file bar.) */
    ViewSetting_ShowFileBar,
};

/* DOC(A Buffer_Create_Flag field specifies how a buffer should be created.) */
ENUM(uint32_t, Buffer_Create_Flag){
    /* DOC(BufferCreate_Background is not currently implemented.) */
    BufferCreate_Background = 0x1,
    /* DOC(When BufferCreate_AlwaysNew is set it indicates the buffer should be cleared to empty even if it's associated file already has content.) */
    BufferCreate_AlwaysNew  = 0x2,
    /* DOC(When BufferCreate_NeverNew is set it indicates that the buffer should only be created if it is an existing file or if a buffer with the given name is already open.) */
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
    /* DOC(AccessOpen does not include any bits, it indicates that the access should only return objects that have no protection flags set.) */
    AccessOpen      = 0x0,
    /* DOC(AccessProtected is set on buffers and views that are "read only" such as the output from an exec_system_command call into *build*. This is to prevent the user from accidentally editing output that they might prefer to keep in tact.) */
    AccessProtected = 0x1,
    /* DOC(AccessHidden is set on any view that is not currently showing it's file, for instance because it is navigating the file system to open a file.) */
    AccessHidden    = 0x2,
    /* DOC(AccessAll is a catchall access for cases where an access call should always return an object no matter what it's protection flags are.) */
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

/* DOC(Key_Code is the alias for key codes including raw codes and codes translated to textual input that takes modifiers into account.) */
TYPEDEF uint32_t Key_Code;

/* DOC(Key_Modifier is the alias for flags that represent keyboard modifiers, ctrl, alt, shift, etc.)
DOC_SEE(Key_Modifier_Flag) */
TYPEDEF uint8_t Key_Modifier;

/* DOC(Key_Event_Data describes a key event, including the translation to a character, the translation to a character ignoring the state of caps lock, and an array of all the modifiers that were pressed at the time of the event.) */
STRUCT Key_Event_Data{
    /* DOC(This field is the raw keycode which is always non-zero in valid key events.) */
    Key_Code keycode;
    
    /* DOC(This field is the keycode after translation to a character, this is 0 if there is no translation.) */
    Key_Code character;
    
    /* DOC(This field is like the field character, except that the state of caps lock is ignored in the translation.) */
    Key_Code character_no_caps_lock;
    
    /* DOC(This field is an array indicating the state of modifiers at the time of the key press. The array is indexed using the values of Key_Modifier_Index.  1 indicates that the corresponding modifier was held, and a 0 indicates that it was not held.)
    
    DOC_SEE(Key_Modifier)
    */
    int8_t modifiers[MDFR_INDEX_COUNT];
};

// TODO(allen): GLOBAL_VAR meta parsing
GLOBAL_VAR Key_Event_Data null_key_event_data = {0};

/* DOC(Mouse_State describes an entire mouse state complete with the position, left and right button states, the wheel state, and whether or not the mouse if in the window.) */
STRUCT Mouse_State{
    /* DOC(Left button is down.) */
    int8_t l;
    /* DOC(Right button is down.) */
    int8_t r;
    /* DOC(Left button was pressed this frame.) */
    int8_t press_l;
    /* DOC(Right button was pressed this frame.) */
    int8_t press_r;
    /* DOC(Left button was released this frame.) */
    int8_t release_l;
    /* DOC(Right button was released this frame.) */
    int8_t release_r;
    /* DOC(Mouse is outside of the window.) */
    int8_t out_of_window;
    /* DOC(The motion of the wheel. Zero indicates no motion. Positive indicates downard scrolling. Negative indicates upward scrolling. The magnitude corresponds to the number of pixels of scroll will be applied at standard scroll speeds.) */
    int32_t wheel;
    /* DOC(X position of the mouse where the left of the window is x = 0, and x grows to the right.) */
    int32_t x;
    /* DOC(Y position of the mouse where the top of the window is y = 0, and y grows to downward.) */
    int32_t y;
};

GLOBAL_VAR Mouse_State null_mouse_state = {0};

/* DOC(Range describes an integer range typically used for ranges within a buffer. Ranges tend are usually not passed as a Range struct into the API, but this struct is used to return ranges.

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

/* DOC(Parser_String_And_Type contains a string and type integer used to specify information about keywords to the parser.) */
STRUCT Parser_String_And_Type{
    /* DOC(The string specified by the pair, it need not be null terminated.) */
    char *str;
    /* DOC(The number of bytes in the string str.) */
    uint32_t length;
    /* DOC(The type integer.) */
    uint32_t type;
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
    bool32 folder;
};

/* DOC(File_List is a list of File_Info structs.)
DOC_SEE(File_Info) */
STRUCT File_List{
    /* DOC(This field is for inernal use.) */
    void *block;
    /* DOC(This field is an array of File_Info structs.) */
    File_Info *infos;
    /* DOC(This field specifies the number of struts in the info array.) */
    uint32_t count;
    /* DOC(This field is for internal use.) */
    uint32_t block_size;
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

/* DOC(Describes the various coordinate locations associated with the view's scroll position within it's buffer.) */
STRUCT GUI_Scroll_Vars{
    /* DOC(The current actual y position of the view scroll.) */
    float   scroll_y;
    /* DOC(The target y position to which the view is moving.  If scroll_y is not the same value, then it is still sliding to the target by the smooth scroll rule.) */
    int32_t target_y;
    /* DOC(The previous value of target y.  This value should be ignored as it is the "vestigial" remain of a system that will not be around much longer.) */
    int32_t prev_target_y;
    
    /* DOC(The current actual x position of the view scroll.) */
    float   scroll_x;
    /* DOC(The target x position to which the view is moving.  If scroll_x is not the same value, then it is still sliding to the target by the smooth scroll rule.) */
    int32_t target_x;
    /* DOC(The previous value of target x.  This value should be ignored as it is the "vestigial" remain of a system that will not be around much longer.) */
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
    /* DOC(This field indicates whether the Buffer_Summary describes a buffer that is open in 4coder. When this field is false the summary is referred to as a "null summary".) */
    bool32 exists;
    /* DOC(If this is not a null summary, this field indicates whether the buffer has finished loading.) */
    bool32 ready;
    /* DOC(If this is not a null summary this field is the id of the associated buffer. If this is a null summary then buffer_id is 0.) */
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
    /* DOC(If this is not a null summary, this field indicates whether the buffer has up to date tokens available. If this field is false, it may simply mean the tokens are still being generated in a background task and will be available later.  If that is the case, is_lexed will be true to indicate that the buffer is trying to get it's tokens up to date.) */
    bool32 tokens_are_ready;
    /* DOC(If this is not a null summary, this field specifies the id of the command map for this buffer.) */
    int32_t map_id;
    /* DOC(If this is not a null summary, this field indicates whether the buffer 'prefers' wrapped lines.) */
    bool32 unwrapped_lines;
};

GLOBAL_VAR Buffer_Summary null_buffer_summary = {0};

/*
DOC(A markers is a location in a buffer that, once placed, is effected by edits the same way characters are effected.  In particular if an edit occurs in a location in the buffer before a marker, the marker is shifted forward or backward so that it remains on the same character.)
DOC_SEE(buffer_add_markers)
*/
STRUCT Marker{
    /* DOC(The current position of the marker measure in absolute byte positioning coordinates.) */
    int32_t pos;
    /* DOC(When a marker is inside a range that gets edited, by default the marker 'leans_left' which means it goes to the beginning of the edited range.  If the field lean_right is set to true, the marker will lean right with edits and will go to the end of edited range.) */
    bool32 lean_right;
};

/*
DOC(A handle to an internally held array of markers.)
DOC_SEE(Marker)
DOC_SEE(buffer_add_markers)
*/
TYPEDEF void* Marker_Handle;

/*
DOC(A four corner axis aligned rectangle, with integer coordinates.)
*/
STRUCT i32_Rect{
    int32_t x0;
    int32_t y0;
    int32_t x1;
    int32_t y1;
};

GLOBAL_VAR i32_Rect null_i32_rect = {0};

/*
DOC(A four corner axis aligned rectangle, with floating point coordinates.)
*/
STRUCT f32_Rect{
    float x0;
    float y0;
    float x1;
    float y1;
};

GLOBAL_VAR f32_Rect null_f32_rect = {0};

/* DOC(View_Summary acts as a handle to a view and describes the state of the view.)
DOC_SEE(Access_Flag)
DOC_SEE(Full_Cursor) */
STRUCT View_Summary{
    /* DOC(This field indicates whether the View_Summary describes a view that is open in 4coder. When this field is false the summary is referred to as a "null summary". ) */
    bool32 exists;
    /* DOC(This field is the id of the associated view. If this is a null summary then view_id is 0. ) */
    int32_t view_id;
    /* DOC(Then this is the id of the buffer this view currently sees.) */
    int32_t buffer_id;
    /* DOC(This field contains flags describing the protection status of the view.) */
    Access_Flag lock_flags;
    
    /* DOC(This describes the position of the cursor.) */
    Full_Cursor cursor;
    /* DOC(This describes the position of the mark.) */
    Full_Cursor mark;
    /* DOC(This is the x position that is maintained in vertical navigation.) */
    float preferred_x;
    /* DOC(This specifies the height of a line rendered in the view.) */
    float line_height;
    /* DOC(This indicates that the view is set to render with unwrapped lines.) */
    bool32 unwrapped_lines;
    /* DOC(This indicates that the view is set to highlight white space.) */
    bool32 show_whitespace;
    
    /* DOC(This describes the screen position in which this view is displayed.) */
    i32_Rect view_region;
    /* DOC(This describes the screen position in which this view's buffer is displayed.  This is different from view_region, because it does not include any fixed height GUI at the top of the view.) */
    i32_Rect file_region;
    /* DOC(This describes the scrolling position inside the view.) */
    GUI_Scroll_Vars scroll_vars;
};

GLOBAL_VAR View_Summary null_view_summary = {0};

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

/*
DOC(Theme lists every color that makes up a standard color scheme.)
DOC_SEE(int_color)
*/
STRUCT Theme{
    /* DOC(The colors array.  Every style tag, beginning with "Stag", is an index into it's corresponding color in the array.) */
    int_color colors[Stag_COUNT];
};

/*
DOC(Available_Font contains a name for a font was detected at startup either in the local 4coder font folder, or by the system.  An available font is not necessarily loaded yet, and may fail to load for various reasons even though it appearsin the available font list.)
DOC_SEE(get_available_font)
*/
STRUCT Available_Font{
    char name[64];
    bool32 in_local_font_folder;
};

/*
DOC(Every face is assigned a unique and consistent Face_ID for it's life time.  This represents a slot in which a face can exist.  The face in the slot is always valid once it exists, but the face might be changed or released durring it's lifetime.  A Face_ID value of zero is reserved for the meaning "not a valid face".)
*/
TYPEDEF uint32_t Face_ID;

/*
DOC(Face_Description contains all the information unique to a single font face, including the font name, the size, and style of the face.)
DOC_SEE(get_available_font)
*/
STRUCT Face_Description{
    /* DOC(Indicates a face's association with an available font.  This should be an exact copy of an Available_Font returned by get_available_fonts.) DOC_SEE(get_available_font) */
    Available_Font font;
    
    /* DOC(Indicates the size for the face.  Valid values must be greater than 0.  Different fonts with the same pt_size do not necessarily have the same line height.) */
    int32_t pt_size;
    
    /* DOC(Indicates whether the face tries to use a bold style.) */
    bool32 bold;
    
    /* DOC(Indicates whether the face tries to use an italic style.) */
    bool32 italic;
    
    /* DOC(Indicates whether the face tries to underline text.) */
    bool32 underline;
    
    /* DOC(Indicates whether the face tries to apply hinting.) */
    bool32 hinting;
};

/* DOC(A Buffer_Batch_Edit_Type is a type of batch operation.) */
ENUM(int32_t, Buffer_Batch_Edit_Type){
    /* DOC(The BatchEdit_Normal operation is always correct but does the most work if there are tokens to correct.) */
    BatchEdit_Normal,
    /* DOC(The BatchEdit_PreserveTokens operation is one in which none of the edits add, delete, or change any tokens. This usually applies when whitespace is being replaced with whitespace.) */
    BatchEdit_PreserveTokens
};

/*
DOC(This struct is used to bundle the parameters of the buffer_batch_edit function.  It is convenient for a few functions that return a batch edit to the user.)
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


/* DOC(Custom_Command_Function is a function type which matches the signature used for commands.  To declare a command use CUSTOM_COMMAND_SIG.) DOC_SEE(CUSTOM_COMMAND_SIG) */
TYPEDEF void Custom_Command_Function(struct Application_Links *app);

// TODO(allen): Improve meta system so that the system for picking up macros is universal.
#define CUSTOM_COMMAND_SIG(name) void name(struct Application_Links *app)
#define CUSTOM_DOC(str)
#define CUSTOM_ALIAS(x) x

/* DOC(Generic_Command acts as a name for a command, and can name an internal command or a custom command.) */
UNION Generic_Command{
    /*DOC(If this Generic_Command represents an internal command the cmdid field will have a value less than cmdid_count, and this field is the command id for the command.)*/
    Command_ID cmdid;
    /*DOC(If this Generic_Command does not represent an internal command the command
    field is the pointer to the custom command..)*/
    Custom_Command_Function *command;
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


/* DOC(Hook_IDs name the various hooks into 4coder, these hooks use the Hook_Function signature.)
DOC_SEE(Hook_Function) */
ENUM(int32_t, Hook_ID){
    /* DOC(TODO) */
    hook_file_out_of_sync,
    /* DOC(TODO) */
    hook_exit,
    /* DOC(TODO) */
    hook_view_size_change,
    // never below this
    hook_type_count
};

/* DOC(Special_Hook_IDs name special hooks that use specialized signatures.) */
ENUM(int32_t, Special_Hook_ID){
    /* DOC(TODO) */
    special_hook_scroll_rule = hook_type_count,
    /* DOC(TODO) */
    special_hook_new_file,
    /* DOC(TODO) */
    special_hook_open_file,
    /* DOC(TODO) */
    special_hook_save_file,
    /* DOC(TODO) */
    special_hook_end_file,
    /* DOC(TODO) */
    special_hook_command_caller,
    /* DOC(TODO) */
    special_hook_input_filter,
    /* DOC(TODO) */
    special_hook_start,
};

TYPEDEF_FUNC int32_t Command_Caller_Hook_Function(struct Application_Links *app, Generic_Command cmd);
#define COMMAND_CALLER_HOOK(name) int32_t name(struct Application_Links *app, Generic_Command cmd)

TYPEDEF_FUNC int32_t Hook_Function(struct Application_Links *app);
#define HOOK_SIG(name) int32_t name(struct Application_Links *app)

TYPEDEF_FUNC int32_t Open_File_Hook_Function(struct Application_Links *app, int32_t buffer_id);
#define OPEN_FILE_HOOK_SIG(name) int32_t name(struct Application_Links *app, int32_t buffer_id)

TYPEDEF_FUNC void Input_Filter_Function(Mouse_State *mouse);
#define INPUT_FILTER_SIG(name) void name(Mouse_State *mouse)

TYPEDEF_FUNC int32_t Scroll_Rule_Function(float target_x, float target_y, float *scroll_x, float *scroll_y, int32_t view_id, int32_t is_new_target, float dt);
#define SCROLL_RULE_SIG(name) \
int32_t name(float target_x, float target_y, float *scroll_x, float *scroll_y, int32_t view_id, int32_t is_new_target, float dt)

TYPEDEF_FUNC int32_t Start_Hook_Function(struct Application_Links *app, char **files, int32_t file_count, char **flags, int32_t flag_count);
#define START_HOOK_SIG(name) \
int32_t name(struct Application_Links *app, char **files, int32_t file_count, char **flags, int32_t flag_count)

TYPEDEF_FUNC int32_t Get_Binding_Data_Function(void *data, int32_t size);
#define GET_BINDING_DATA(name) int32_t name(void *data, int32_t size)

// NOTE(allen): Definitions for the format that Get_Binding_Data uses to launch 4coder.
// TODO(allen): Transition to a more dynamic Command_Map system.

/*
DOC(Values for the type field in the discriminated union Binding_Unit.)
DOC_SEE(Binding_Unit)
*/
ENUM(int32_t, Binding_Unit_Type){
    unit_header,
    unit_map_begin,
    unit_binding,
    unit_callback,
    unit_inherit,
    unit_hook
};

/*
DOC(Values for built in command maps.)
*/
ENUM(int32_t, Map_ID){
    mapid_global = (1 << 24),
    mapid_file,
    mapid_ui,
    mapid_nomap
};

/*
DOC(Describes a unit of information for setting up key bindings.  A unit can set a key binding, switch the active map, set the inherited map, or set a hook.)
*/
STRUCT Binding_Unit{
    Binding_Unit_Type type;
    UNION{
        STRUCT{ int32_t total_size; int32_t user_map_count; int32_t error; } header;
        STRUCT{ int32_t mapid; int32_t replace; int32_t bind_count; } map_begin;
        STRUCT{ int32_t mapid; } map_inherit;
        STRUCT{ Key_Code code; uint8_t modifiers; Command_ID command_id; } binding;
        STRUCT{ Key_Code code; uint8_t modifiers; Custom_Command_Function *func; } callback;
        STRUCT{ int32_t hook_id; void *func; } hook;
    };
};

typedef int32_t _Get_Version_Function(int32_t maj, int32_t min, int32_t patch);
#define _GET_VERSION_SIG(n) int32_t n(int32_t maj, int32_t min, int32_t patch)

#endif

