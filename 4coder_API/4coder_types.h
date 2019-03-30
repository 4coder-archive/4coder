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

/* DOC(argb_color is an alias name to signal that an integer parameter or field is for a color value,
colors are specified as 32 bit integers with 4 channels: 0xAARRGGBB.) */
TYPEDEF u32 argb_color;

TYPEDEF u32 int_color;

TYPEDEF u16 id_color;

/* DOC(Parse_Context_ID identifies a parse context, which is a guiding rule for the parser.  Each buffer sets which parse context to use when it is parsed.) */
TYPEDEF u32 Parse_Context_ID;

TYPEDEF u32 Child_Process_ID;

/* DOC(Buffer_ID is used to name a 4coder buffer.  Each buffer has a unique id but when a buffer is closed it's id may be recycled by future, different buffers.) */
TYPEDEF i32 Buffer_ID;

/* DOC(View_ID is used to name a 4coder view.  Each view has a unique id in the interval [1,16].) */
TYPEDEF i32 View_ID;

TYPEDEF i32 Panel_ID;

ENUM(u32, Child_Process_Set_Target_Flags){
    ChildProcessSet_FailIfBufferAlreadyAttachedToAProcess = 1,
    ChildProcessSet_FailIfProcessAlreadyAttachedToABuffer = 2,
    ChildProcessSet_NeverOverrideExistingAttachment = 3,
    ChildProcessSet_CursorAtEnd = 4,
};

/* DOC(A Key_Modifier_Index acts as an index for specifying modifiers in arrays.) */
ENUM(i32, Key_Modifier_Index){
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
ENUM(u32, Key_Modifier_Flag){
    /* DOC(MDFR_NONE specifies that no modifiers are pressed.) */
    MDFR_NONE  = 0x0,
    MDFR_CTRL  = 0x1,
    MDFR_ALT   = 0x2,
    MDFR_CMND  = 0x4,
    MDFR_SHIFT = 0x8,
};

/* DOC(Flags for describing the memory protection status of pages that come back from memory allocate.  Some combinations may not be available on some platforms, but you are gauranteed to get back a page with at least the permissions you requested.  For example if you request just write permission, you may get back a page with read and write permission, but you will never get back a page that doesn't have write permission.) */
ENUM(u32, Memory_Protect_Flags){
    /* DOC(Allows the page to be read.) */
    MemProtect_Read    = 0x1,
    /* DOC(Allows the page to be written.) */
    MemProtect_Write   = 0x2,
    /* DOC(Allows the page to be executed.) */
    MemProtect_Execute = 0x4,
};

/* DOC(A Wrap_Indicator_Mode is used in the buffer setting BufferSetting_WrapIndicator to specify how to indicate that line has been wrapped.) */
ENUM(i32, Wrap_Indicator_Mode){
    /* DOC(WrapIndicator_Hide tells the buffer rendering system not to put any indicator on wrapped lines.) */
    WrapIndicator_Hide,
    
    /* DOC(WrapIndicator_Show_After_Line tells the buffer rendering system to put a backslash indicator on wrapped lines right after the last character of the line.) */
    WrapIndicator_Show_After_Line,
    
    /* DOC(WrapIndicator_Show_At_Wrap_Edge tells the buffer rendering system to put a backslash indicator on wrapped lines aligned with the wrap position for that line.) */
    WrapIndicator_Show_At_Wrap_Edge,
};

/* DOC(A Global_Setting_ID names a setting.) */
ENUM(i32, Global_Setting_ID){
    /* DOC(GlobalSetting_Null is not a valid setting, it is reserved to detect errors.) */
    GlobalSetting_Null,
    
    /* DOC(When GlobalSetting_LAltLCtrlIsAltGr is enabled, keyboard layouts with AltGr will also interpret the left alt and control keys as AltGr.  If you do not use a keyboard with AltGr you should have this turned off.  This setting is only relevant on Windows and has no effect on other systems.) */
    GlobalSetting_LAltLCtrlIsAltGr,
};

/* DOC(A Buffer_Setting_ID names a setting in a buffer.) */
ENUM(i32, Buffer_Setting_ID){
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
    
    /* DOC(TODO) */
    BufferSetting_RecordsHistory,
};

/* DOC(A View_Setting_ID names an adjustable setting in a view.) */
ENUM(i32, View_Setting_ID){
    /* DOC(ViewSetting_Null is not a valid setting, it is reserved to detect errors.) */
    ViewSetting_Null,
    
    /* DOC(Determines whether the view highlights whitespace in a file.  Whenever the view switches to a new buffer this setting is turned off.) */
    ViewSetting_ShowWhitespace,
    
    /* DOC(Determines whether a scroll bar is attached to a view in it's scrollable section.) */
    ViewSetting_ShowScrollbar,
    
    /* DOC(Determines whether to show the file bar.) */
    ViewSetting_ShowFileBar,
    
    /* DOC(Determines what command map the view uses when it is in ui mode.) */
    ViewSetting_UICommandMap
};

/* DOC(A Buffer_Create_Flag field specifies how a buffer should be created.) */
ENUM(u32, Buffer_Create_Flag){
    /* DOC(BufferCreate_Background is not currently implemented.) */
    BufferCreate_Background = 0x1,
    /* DOC(When BufferCreate_AlwaysNew is set it indicates the buffer should be cleared to empty even if it's associated file already has content.) */
    BufferCreate_AlwaysNew  = 0x2,
    /* DOC(When BufferCreate_NeverNew is set it indicates that the buffer should only be created if it is an existing file or if a buffer with the given name is already open.) */
    BufferCreate_NeverNew   = 0x4,
    /* DOC(When BufferCreate_JustChangedFile is set it indicates that the file to load has just been saved in the same frame and a change notification for the file should be ignored.) */
    BufferCreate_JustChangedFile = 0x8,
    /* DOC(Indicates that when create_buffer searches for already existing buffers that match the name parameter, it should only search by file name, and that if it cannot create the buffer with the file attached, it should not create the buffer at all.) */
    BufferCreate_MustAttachToFile = 0x10,
    /* DOC(Indicates that when create_buffer searches for already existing buffers that match the name parameter, it should only search by buffer name, and that it should not attach a file to the buffer even if it can.  Caution! Buffers that don't have attached files cannot be saved.) */
    BufferCreate_NeverAttachToFile = 0x20,
    /* DOC(Normally the new file hook is called on any created buffer.  Passing this flag will prevent the new file hook from being called automatically by the core.) */
    BufferCreate_SuppressNewFileHook = 0x40,
};

/* DOC(Buffer_Creation_Data is a struct used as a local handle for the creation of a buffer. )
HIDE_MEMBERS() */
STRUCT Buffer_Creation_Data{
    Buffer_Create_Flag flags;
    char fname_space [256];
    i32 fname_len;
};

/* DOC(A Buffer_Save_Flag field specifies buffer saving behavior.) */
ENUM(u32, Buffer_Save_Flag){
    /* DOC(BufferSave_IgnoreDirtyFlag tells the save procedure not to check the dirty flag of the buffer. Usually buffers not marked dirty will not be saved, but sometimes it is useful to force it to save anyway. ) */
    BufferSave_IgnoreDirtyFlag = 0x1,
};

/* DOC(A Buffer_Kill_Flag field specifies how a buffer should be killed.) */
ENUM(u32, Buffer_Kill_Flag){
    /* DOC(When BufferKill_AlwaysKill is set it indicates the buffer should be killed
    without asking, even when the buffer is dirty.) */
    BufferKill_AlwaysKill  = 0x2,
};

/* DOC(A Buffer_Reopen_Flag field specifies how a buffer should be reopened -- currently no flags are provided.) */
ENUM(u32, Buffer_Reopen_Flag){};

/* DOC(A status enumeration returned by kill_buffer.)
DOC_SEE(kill_buffer) */
ENUM(i32, Buffer_Kill_Result){
    /* DOC(The buffer was successfully killed.) */
    BufferKillResult_Killed = 0,
    /* DOC(The buffer was not killed because it is dirty.  This can be overriden with the flag BufferKill_AlwaysKill.  This result is usually followed up by launching a "sure to kill" dialogue, but that is entirely optional.) */
    BufferKillResult_Dirty = 1,
    /* DOC(The buffer was not killed because it is unkillable.  Unkillable buffers are the buffers that must be open for the core's purposes.  *messages* and *scratch* are unkillable buffers.) */
    BufferKillResult_Unkillable = 2,
    /* DOC(The specified buffer does not exist.) */
    BufferKillResult_DoesNotExist = 3,
};

/* DOC(A status enumeration returned by reopen_buffer.)
DOC_SEE(kill_buffer) */
ENUM(i32, Buffer_Reopen_Result){
    /* DOC(The buffer was successfully reopened.) */
    BufferReopenResult_Reopened = 0,
    /* DOC(The buffer was not reopened, because either the buffer has no attached file, or the attached file could not be loaded.) */
    BufferReopenResult_Failed = 1,
};

/* DOC(An Access_Flag field specifies what sort of permission you grant to an access call.  An access call is usually one the returns a summary struct.  If a 4coder object has a particular protection flag set and the corresponding bit is not set in the access field, that 4coder object is hidden.  On the other hand if a protection flag is set in the access parameter and the object does not have that protection flag, the object is still returned from the access call.) */
ENUM(u32, Access_Flag){
    /* DOC(AccessOpen does not include any bits, it indicates that the access should only return objects that have no protection flags set.) */
    AccessOpen      = 0x0,
    /* DOC(AccessProtected is set on buffers and views that are "read only" such as the output from an exec_system_command call into *build*. This is to prevent the user from accidentally editing output that they might prefer to keep in tact.) */
    AccessProtected = 0x1,
    /* DOC(AccessHidden is set on any view that is not currently showing it's file, for instance because it is navigating the file system to open a file.) */
    AccessHidden    = 0x2,
    /* DOC(AccessAll is a catchall access for cases where an access call should always return an object no matter what it's protection flags are.) */
    AccessAll       = 0xFF
};

/* DOC(A Dirty_State value describes whether changes have been made to a buffer or to an underlying
file since the last sync time between the two.  Saving a buffer to it's file or loading the buffer
from the file both act as sync points.) */
ENUM(u32, Dirty_State){
    /* DOC(DirtyState_UpToDate indicates that there are no unsaved changes and the underlying
system file still agrees with the buffer's state.) */
    DirtyState_UpToDate = 0,
    
    /* DOC(DirtyState_UnsavedChanges indicates that there have been changes in the buffer since
the last sync point.) */
    DirtyState_UnsavedChanges = 1,
    
    /* DOC(DirtyStateUnloadedChanges indicates that the underlying file has been edited since
the last sync point .) */
    DirtyState_UnloadedChanges = 2,
    
    /* DOC(TODO) */
    DirtyState_UnsavedChangesAndUnloadedChanges = 3,
};

/* DOC(A Seek_Boundary_Flag field specifies a set of "boundary" types used in seeks for the beginning or end of different types of words.) */
ENUM(u32, Seek_Boundary_Flag){
    BoundaryWhitespace   = 0x1,
    BoundaryToken        = 0x2,
    BoundaryAlphanumeric = 0x4,
    BoundaryCamelCase    = 0x8
};

/* DOC(A Command_Line_Interface_Flag field specifies the behavior of a call to a command line interface.) */
ENUM(u32, Command_Line_Interface_Flag){
    /* DOC(If CLI_OverlapWithConflict is set if output buffer of the new command is already in use by another command which is still executing, the older command relinquishes control of the buffer and both operate simultaneously with only the newer command outputting to the buffer.) */
    CLI_OverlapWithConflict = 0x1,
    /* DOC(If CLI_AlwaysBindToView is set the output buffer will always be set in the active view even if it is already set in another open view.) */
    CLI_AlwaysBindToView    = 0x2,
    /* DOC(If CLI_CursorAtEnd is set the cursor will be kept at the end of the output buffer, otherwise the cursor is kept at the beginning.) */
    CLI_CursorAtEnd         = 0x4,
    /* DOC(TODO) */
    CLI_SendEndSignal       = 0x8,
};

/* DOC(An Auto_Indent_Flag field specifies the behavior of an auto indentation operation.) */
ENUM(u32, Auto_Indent_Flag){
    /* DOC(If AutoIndent_ClearLine is set, then any line that is only whitespace will be cleared to contain nothing at all, otherwise the line is filled with whitespace to match the nearby indentation.) */
    AutoIndent_ClearLine = 0x1,
    /* DOC(If AutoIndent_UseTab is set, then when putting in leading whitespace to align code, as many tabs will be used as possible until the fine grained control of spaces is needed to finish the alignment.) */
    AutoIndent_UseTab    = 0x2,
    /* DOC(If AutoIndent_ExactAlignBlock is set, then block comments are indented by putting the first non-whitespace character of the line in line with the beginning of the comment.) */
    AutoIndent_ExactAlignBlock = 0x4,
    /* DOC(If AutoIndent_FullTokens is set, then the set of lines that are indented is automatically expanded so that any token spanning multiple lines gets entirely indented.) */
    AutoIndent_FullTokens = 0x8,
};

/* DOC(A Set_Buffer_Flag field specifies the behavior of an operation that sets the buffer of a view.) */
ENUM(u32, Set_Buffer_Flag){
    /* DOC(If SetBuffer_KeepOriginalGUI then when the file is set, the view will not switch to it
    if some other GUI was currently up, otherwise any GUI that is up is closed and the view
    switches to the file.) */
    SetBuffer_KeepOriginalGUI = 0x1
};

/* DOC(A Input_Type_Flag field specifies a set of input event types.) */
ENUM(u32, Input_Type_Flag){
    /* DOC(If EventOnAnyKey is set, all keyboard events are included in the set.) */
    EventOnAnyKey  = 0x1,
    /* DOC(If EventOnEsc is set, any press of the escape key is included in the set.) */
    EventOnEsc     = 0x2,
    
    /* DOC(If EventOnLeftButton is set, left clicks and releases are included in the set.) */
    EventOnMouseLeftButton  = 0x4,
    /* DOC(If EventOnRightButton is set, right clicks and releases are included in the set.) */
    EventOnMouseRightButton = 0x8,
    /* DOC(If EventOnWheel is set, any wheel movement is included in the set.) */
    EventOnMouseWheel       = 0x10,
    /* DOC(If EventOnMouseMove is set, mouse movement events are included in the set.) */
    EventOnMouseMove        = 0x20,
    
    /* DOC(If EventOnAnimate is set, animate events are included in the set.) */
    EventOnAnimate        = 0x40,
    /* DOC(If EventOnViewActivation is set, view activation and deactivation events are included in the set.) */
    EventOnViewActivation = 0x80,
    
    /* DOC(EventAll is a catch all name for including all possible events in the set.) */
    EventAll = 0xFFFFFFFF,
};

/* DOC(A Mouse_Cursor_Show_Type value specifes a mode for 4coder to handle the mouse cursor.) */
ENUM(i32, Mouse_Cursor_Show_Type){
    /* DOC(The MouseCursorShow_Never mode never shows the cursor.) */
    MouseCursorShow_Never,
    /* DOC(The MouseCursorShow_Never mode always shows the cursor.) */
    MouseCursorShow_Always,
    // TODO(allen): coming soon
    //    MouseCursorShow_WhenActive,
};

/* DOC(A View_Split_Position specifies where a new view should be placed as a result of a view split operation.) */
ENUM(i32, View_Split_Position){
    /* DOC(This value indicates that the new view should be above the existing view.) */
    ViewSplit_Top,
    /* DOC(This value indicates that the new view should be below the existing view.) */
    ViewSplit_Bottom,
    /* DOC(This value indicates that the new view should be left of the existing view.) */
    ViewSplit_Left,
    /* DOC(This value indicates that the new view should be right of the existing view.) */
    ViewSplit_Right,
};

ENUM(i32, Panel_Split_Kind){
    PanelSplitKind_Ratio_Min = 0,
    PanelSplitKind_Ratio_Max = 1,
    PanelSplitKind_FixedPixels_Min = 2,
    PanelSplitKind_FixedPixels_Max = 3,
};

ENUM(i32, Panel_Split_Orientation){
    PanelSplit_LeftAndRight = 0,
    PanelSplit_TopAndBottom = 1,
};

ENUM(i32, Panel_Child){
    PanelChild_Min = 0,
    PanelChild_Max = 1,
};

/* DOC(Key_Code is the alias for key codes including raw codes and codes translated to textual input that takes modifiers into account.) */
TYPEDEF u32 Key_Code;

/* DOC(Key_Modifier is the alias for flags that represent keyboard modifiers, ctrl, alt, shift, etc.)
DOC_SEE(Key_Modifier_Flag) */
TYPEDEF uint8_t Key_Modifier;

/* DOC(Key_Event_Data describes a key event, including the translation to a character, the translation to a character ignoring the state of caps lock, and an array of all the modifiers that were pressed at the time of the event.)
DOC_SEE(Key_Modifier_Index)
*/
STRUCT Key_Event_Data{
    /* DOC(This field is the raw keycode which is always non-zero in valid key events.) */
    Key_Code keycode;
    
    /* DOC(This field is the keycode after translation to a character, this is 0 if there is no translation.) */
    Key_Code character;
    
    /* DOC(This field is like the field character, except that the state of caps lock is ignored in the translation.) */
    Key_Code character_no_caps_lock;
    
    /* DOC(This field is an array indicating the state of modifiers at the time of the key press. The array is indexed using the values of Key_Modifier_Index.
1 indicates that the corresponding modifier was held, and a 0 indicates that it was not held.)
DOC_SEE(Key_Modifier) */
    int8_t modifiers[MDFR_INDEX_COUNT];
};

// TODO(allen): GLOBAL_VAR meta parsing
GLOBAL_VAR Key_Event_Data null_key_event_data = {};

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
    /* DOC(The motion of the wheel. Zero indicates no motion. Positive indicates downard scrolling. Negative indicates upward scrolling. The magnitude corresponds to the number of pixels of scroll that would be applied at standard scroll speeds.) */
    i32 wheel;
    UNION{
        STRUCT{
            /* DOC(X position of the mouse where the left of the window is x = 0, and x grows to the right.) */
            i32 x;
            /* DOC(Y position of the mouse where the top of the window is y = 0, and y grows to downward.) */
            i32 y;
        };
        /* DOC(TODO) */
        Vec2_i32 p;
    };
};

/* DOC(Range describes an integer range typically used for ranges within a buffer. Ranges are not used to pass into the API, but this struct is used for returns.

Throughout the API ranges are thought of in the form [min,max) where max is "one past the end" of the range that is actually read/edited/modified.) */
UNION Range{
    STRUCT{
        /* DOC(This is the smaller value in the range.) */
        i32 min;
        /* DOC(This is the larger value in the range.) */
        i32 max;
    };
    STRUCT{
        /* DOC(This is the start of the range, unioned with min.) */
        i32 start;
        /* DOC(This is the end of the range, unioned with max.) */
        i32 end;
    };
    STRUCT{
        /* DOC(This is the first value in the range, unioned with min.) */
        i32 first;
        /* DOC(This is one_past_the_last value in the range, unioned with max.) */
        i32 one_past_last;
    };
};

/* DOC(An array of ranges.  This is just a plain pointer bundled with a count, no additional special structure.) */
STRUCT Range_Array{
    /* DOC(A pointer to the array of ranges.) */
    Range *ranges;
    /* DOC(The number of ranges in the array.) */
    i32 count;
};

/* DOC(Parser_String_And_Type contains a string and type integer used to specify information about keywords to the parser.) */
STRUCT Parser_String_And_Type{
    /* DOC(The string specified by the pair, it need not be null terminated.) */
    char *str;
    /* DOC(The number of bytes in the string str.) */
    u32 length;
    /* DOC(The type integer.) */
    u32 type;
};

/* DOC(Microsecond_Time_Stamp is a typedef of an unsigned 64 bit integer used to signify that the value is an arbitrary for a moment in time.) */
TYPEDEF u64 Microsecond_Time_Stamp;

/*
DOC(File_Info describes the name and type of a file.)
DOC_SEE(File_List)
*/
STRUCT File_Info{
    /* DOC(This field is a null terminated string specifying the name of the file.) */
    char *filename;
    /* DOC(This field specifies the length of the filename string not counting the null terminator.) */
    i32 filename_len;
    /* DOC(This field indicates that the description is for a folder not a file.) */
    b32 folder;
};

/* DOC(File_List is a list of File_Info structs.)
DOC_SEE(File_Info) */
STRUCT File_List{
    /* DOC(This field is for inernal use.) */
    void *block;
    /* DOC(This field is an array of File_Info structs.) */
    File_Info *infos;
    /* DOC(This field specifies the number of struts in the info array.) */
    u32 count;
    /* DOC(This field is for internal use.) */
    u32 block_size;
};

/* DOC(Buffer_Identifier acts as a loosely typed description of a buffer that can either be a name or an id.) */
STRUCT Buffer_Identifier{
    /* DOC(This field is the name of the buffer; it need not be null terminated. If id is specified this pointer should be NULL.) */
    char *name;
    /* DOC(This field specifies the length of the name string.) */
    i32 name_len;
    /* DOC(This field is the id of the buffer.  If name is specified this should be 0.) */
    Buffer_ID id;
};

/* DOC(Describes the various coordinate locations associated with the view's scroll position within it's buffer.) */
STRUCT GUI_Scroll_Vars{
    UNION{
        STRUCT{
            /* DOC(The current actual x position of the view scroll.) */
            f32 scroll_x;
            /* DOC(The current actual y position of the view scroll.) */
            f32 scroll_y;
        };
        /* DOC(TODO) */
        Vec2 scroll_p;
    };
    UNION{
        STRUCT{
            /* DOC(The target x position to which the view is moving.  If scroll_x is not the same value, then it is still sliding to the target by the smooth scroll rule.) */
            i32 target_x;
            /* DOC(The target y position to which the view is moving.  If scroll_y is not the same value, then it is still sliding to the target by the smooth scroll rule.) */
            i32 target_y;
        };
        /* DOC(TODO) */
        Vec2_i32 target_p;
    };
};

/* DOC(The Buffer_Seek_Type is is used in a Buffer_Seek to identify which coordinates are suppose to be used for the seek.)
DOC_SEE(Buffer_Seek)
DOC_SEE(4coder_Buffer_Positioning_System)
*/
ENUM(i32, Buffer_Seek_Type){
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
            i32 pos;
        };
        STRUCT {
            /* DOC(For xy coordinate seeks, rounding down means that any x in the box of the character lands on that character. For instance when clicking rounding down is the user's expected behavior.  Not rounding down means that the right hand portion of the character's box, which is closer to the next character, will land on that next character.  The unrounded behavior is the expected behavior when moving vertically and keeping the preferred x.) */
            b32 round_down;
            /* DOC(The x coordinate for xy type seeks.) */
            float x;
            /* DOC(The y coordinate for xy type seeks.) */
            float y;
        };
        STRUCT {
            /* DOC(The line number of a line-character type seek.) */
            i32 line;
            /* DOC(The character number of a line-character type seek.) */
            i32 character;
        };
    };
};

/* DOC(Full_Cursor describes the position of a cursor in every buffer coordinate system supported by 4coder. This cursor type requires that the buffer is associated with a view to give the x/y values meaning.)
DOC_SEE(4coder_Buffer_Positioning_System) */
STRUCT Full_Cursor{
    /* DOC(This field contains the cursor's position in absolute byte index positioning.) */
    i32 pos;
    /* DOC(This field contains the cursor's position in apparent character index positioning.) */
    i32 character_pos;
    /* DOC(This field contains the number of the line where the cursor is located. This field is one based.) */
    i32 line;
    /* DOC(This field contains the number of the character from the beginninf of the line where the cursor is located. This field is one based.) */
    i32 character;
    /* DOC(This field contains the number of the line where the cursor is located, taking the line wrapping into account.  This field is one based.) */
    i32 wrap_line;
    union{
        struct{
            /* DOC(This field contains the x position measured with unwrapped lines.) */
            f32 unwrapped_x;
            /* DOC(This field contains the y position measured with unwrapped lines.) */
            f32 unwrapped_y;
        };
        /* DOC(TODO) */
        Vec2 unwrapped_p;
    };
    union{
        struct{
            /* DOC(This field contains the x position measured with wrapped lines.) */
            f32 wrapped_x;
            /* DOC(This field contains the y position measured with wrapped lines.) */
            f32 wrapped_y;
        };
        /* DOC(TODO) */
        Vec2 wrapped_p;
    };
};

/* DOC(Partial_Cursor describes the position of a cursor in all of the coordinate systems that a invariant to the View.  In other words the coordinate systems available here can be used on a buffer that is not currently associated with a View.)
DOC_SEE(4coder_Buffer_Positioning_System) */
STRUCT Partial_Cursor{
    /* DOC(This field contains the cursor's position in absolute byte index positioning.) */
    i32 pos;
    /* DOC(This field contains the number of the character from the beginninf of the line
    where the cursor is located. This field is one based.) */
    i32 line;
    /* DOC(This field contains the number of the column where the cursor is located. This field is one based.) */
    i32 character;
};

TYPEDEF_FUNC b32 Buffer_Edit_Handler(struct Application_Links *app, Buffer_ID buffer_id, i32 start, i32 one_past_last, String str);
// TODO(allen): what to do with batches???

STRUCT File_Attributes{
    u64 size;
    u64 last_write_time;
};

/* DOC(Buffer_Summary acts as a handle to a buffer and describes the state of the buffer.)
DOC_SEE(Access_Flag)
DOC_SEE(Dirty_State) */
STRUCT Buffer_Summary{
    /* DOC(This field indicates whether the Buffer_Summary describes a buffer that is open in 4coder. When this field is false the summary is referred to as a "null summary".) */
    b32 exists;
    /* DOC(If this is not a null summary, this field indicates whether the buffer has finished loading.) */
    b32 ready;
    /* DOC(If this is not a null summary this field is the id of the associated buffer. If this is a null summary then buffer_id is 0.) */
    Buffer_ID buffer_id;
    /* DOC(If this is not a null summary, this field contains flags describing the protection status of the buffer.) */
    Access_Flag lock_flags;
    
    /* DOC(TODO) */
    Buffer_Edit_Handler *edit_handler;
    
    /* DOC(If this is not a null summary, this field specifies the number of bytes in the buffer.) */
    i32 size;
    /* DOC(If this is not a null summary, this field specifies the number of lines in the buffer.) */
    i32 line_count;
    
    /* DOC(If this is not a null summary, this field specifies the file name associated to this buffer.) */
    char *file_name;
    /* DOC(This field specifies the length of the file_name string.) */
    i32 file_name_len;
    
    /* DOC(If this is not a null summary, this field specifies the name of the buffer.) */
    char *buffer_name;
    /* DOC(This field specifies the length of the buffer_name string.) */
    i32 buffer_name_len;
    
    /* DOC(This field indicates the dirty state of the buffer.) */
    Dirty_State dirty;
    
    /* DOC(If this is not a null summary, this field indicates whether the buffer is set to lex tokens.) */
    b32 is_lexed;
    /* DOC(If this is not a null summary, this field indicates whether the buffer has up to date tokens available. If this field is false, it may simply mean the tokens are still being generated in a background task and will be available later.  If that is the case, is_lexed will be true to indicate that the buffer is trying to get it's tokens up to date.) */
    b32 tokens_are_ready;
    /* DOC(If this is not a null summary, this field specifies the id of the command map for this buffer.) */
    i32 map_id;
    /* DOC(If this is not a null summary, this field indicates whether the buffer 'prefers' wrapped lines.) */
    b32 unwrapped_lines;
};

/*
DOC(A markers is a location in a buffer that, once placed, is effected by edits the same way characters are effected.  In particular if an edit occurs in a location in the buffer before a marker, the marker is shifted forward or backward so that it remains on the same character.)
DOC_SEE(buffer_add_markers)
*/
STRUCT Marker{
    /* DOC(The current position of the marker measure in absolute byte positioning coordinates.) */
    i32 pos;
    /* DOC(When a marker is inside a range that gets edited, by default the marker 'leans_left' which means it goes to the beginning of the edited range.  If the field lean_right is set to true, the marker will lean right with edits and will go to the end of edited range.) */
    b32 lean_right;
};


/* DOC(View_Summary acts as a handle to a view and describes the state of the view.)
DOC_SEE(Access_Flag)
DOC_SEE(Full_Cursor)
DOC_SEE(GUI_Scroll_Vars) */
STRUCT View_Summary{
    /* DOC(This field indicates whether the View_Summary describes a view that is open in 4coder. When this field is false the summary is referred to as a "null summary". ) */
    b32 exists;
    /* DOC(This field is the id of the associated view. If this is a null summary then view_id is 0. ) */
    i32 view_id;
    /* DOC(Then this is the id of the buffer this view currently sees.) */
    i32 buffer_id;
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
    b32 unwrapped_lines;
    /* DOC(This indicates that the view is set to highlight white space.) */
    b32 show_whitespace;
    
    /* DOC(This describes the screen position in which this view is displayed.) */
    i32_Rect view_region;
    /* DOC(TODO) */
    i32_Rect render_region;
    /* DOC(This describes the scrolling position inside the view.) */
    GUI_Scroll_Vars scroll_vars;
};

/* DOC(The enumeration of types of managed objects.) */
ENUM(i32, Managed_Object_Type)
{
    /* DOC(This type value indicates that the specified object for an operation is not a valid managed object.
A handle for a managed object can "go bad" when it's scope is cleared, or when it is freed, so it is possible to store handles and check them for validity on use.  However, all managed object APIs do the necessary checks on their parameters and return appropriate failure results on bad handles.) */
    ManagedObjectType_Error = 0,
    /* DOC(A memory object is used for straightforward memory allocation in a managed scope.  These objects have no special cleanup, no extra operations, and their memory storage is never touched by the core.) */
    ManagedObjectType_Memory = 1,
    /* DOC(A marker object is used to place markers into buffers that move along with the text upon which they are placed.  A marker object has a specific buffer to which it is attached, and must be allocated in a scope dependent upon the lifetime of that buffer.  Marker objects always use the Marker type for their items, and their item size is always sizeof(Marker).  When a marker object is freed, all of the marker visuals attached to it are also freed and the specific buffer of the object no longer adjusts the marker data when edits occur.) */
    ManagedObjectType_Markers = 2,
    /* DOC(TODO) */
    ManagedObjectType_Arena = 3,
    
    
    ManagedObjectType_COUNT = 4,
};

/* DOC(A handle to a managed scope.  A managed scope contains variables and objects all of which can be freed and reset in optimized bulk operations.  Many managed scopes are created and destroyed by the core to track the lifetime of entities like buffers and views.  Because a managed scope contains it's own copy of the managed variables, managed scopes can also be used as a keying mechanism to store and retrieve special information related to entities like buffers and views.) */
TYPEDEF u64 Managed_Scope;

/* DOC(An id refering to a managed variable.  Managed variables are created globally, but each managed scope has it's own set of values for the managed variables.  Managed variables are created via a unique string.  Attempting to create a variable with the same name as an existing variable fails.  When naming variables it is recommended that you place a 'module name' followed by a '.' and then a descriptive variable name to distinguish your variables from variables written by other 4coder users that might someday need to work together in the same configuration.  Example: "MyUniqueCustomization.variable_name").  The variable's id is used to set and get the value from managed scopes. */
TYPEDEF i32 Managed_Variable_ID;

/* DOC(A handle to a managed object.  Managed objects have various behaviors and special uses depending on their type.  All managed objects share the property of being tied to a managed scope, so that they are cleaned up and freed when that scope's contents are cleared or when the scope is destroyed, they all support store and load operations, although not all with the exact same meanings and implications, and they can all be freed individually instead of with the entire scope.) */
TYPEDEF u64 Managed_Object;

static Managed_Scope ManagedScope_NULL = 0;
static Managed_Variable_ID ManagedVariableIndex_ERROR = -1;
static Managed_Object ManagedObject_NULL = 0;

/* DOC(A multi-member identifier for a marker visual.  A marker visual is attached to a marker object (Marker_Object), it is freed when the marker object is freed or when it is specifically destroyed.  Multiple marker visuals can be placed on a single marker object.)
DOC_SEE(Marker_Visual_Type)
DOC_SEE(Marker_Visual_Symbolic_Color)
DOC_SEE(Marker_Visual_Text_Style)
DOC_SEE(Marker_Visual_Take_Rule)
DOC_SEE(Marker_Visual_Priority_Level)
*/
STRUCT Marker_Visual{
    Managed_Scope scope;
    u32 slot_id;
    u32 gen_id;
};

/*
DOC(The enumeration of visual effects that a marker visual can create.  All effects have a color aspect and text_color aspect.  The exact meaning of these aspects depends upon the effect's type.

There are several effect styles, two effect styles never conflict with one another, each style can be applied to each character.  If two effects of the same style are applied to the same character, then the effect with the higher priority is rendered, and the lower priority effect is ignored.  The render order for effects is:

Highlight Style

Wire Frame Style

I-Bar Style

Some effects are character oriented, meaning they have an effect on the individual character/characters that the markers specify.  Other effects are line oriented, meaning that they effect the entire line on which their markers are placed.  Line oriented highlight style effects are always overriden by character oriented highlight style effects of the same style, regardless of relative priority levels.

Single marked effects use each marker to specify a single point at which an effect is applied.  Range marked effects take pairs of markers to specify a left inclusive right eclusive range to effect.

In range marked effects, two conflicting effects with the same priority level are further resolved by prefering the effect with the higher-index starting position.  This means that if range A completely contains range B and they have the same priority level, then range B will be visible, and range A will be visible wherever range B is not.
)
*/
ENUM(i32, Marker_Visual_Type)
{
    /* DOC(No visual effect, with this type it is as if the marker visual does not exist.) */
    VisualType_Invisible = 0,
    /* DOC(Shows a block around the background of each marked character.  The color aspect determines the color of the block behind the characters, the text_color aspect determines the color of the characters.
    
This is a character oriented highlight style single marked effect.) */
    VisualType_CharacterBlocks = 1,
    /* DOC(Shows a rectangle outline around each marked character.  The color aspect determines the color of the rectangle, the text_color aspect is ignored.
    
This is a character oriented wire frame style single marked effect.) */
    VisualType_CharacterWireFrames = 2,
    /* DOC(Shows a single pixel line on the left side of each marked character.  The color aspect determines the color of the line, the text_color aspect is ignored.
    
This is a character oriented wire frame style single marked effect.) */
    VisualType_CharacterIBars = 3,
    /* DOC(Shows a block in the background of the entire line on which the marker is placed.  The color aspect determines the color of the highlight, the text_color aspect is ignored.
    
This is a line oriented highlight style single marked effect.) */
    VisualType_LineHighlights = 4,
    /* DOC(Shows a block around the background of each character between the range pairs.  The color aspect determines the color of the highlight, the text_color aspect is ignored.
    
This is a character oriented highlight style range marked effect.) */
    VisualType_CharacterHighlightRanges = 5,
    /* DOC(Shows a block in the background of the entire line on each line within the range.  The color aspect determines the color of the highlight, the text_color aspect is ignored.
    
This is a line oriented highlight style range marked effect.) */
    VisualType_LineHighlightRanges = 6,
    VisualType_COUNT = 7,
};

/* DOC(Special codes that can be used as the color aspects of marker visual effects.  These special codes are for convenience and in some cases effects that could not be expressed as 32-bit colors.) */
ENUM(u32, Marker_Visual_Symbolic_Color)
{
    /* DOC(When default is used for text_color aspect, the text is unchanged from the coloring the core would apply to it.  For all effects, the default value of the color aspect for all effects is the same as transparent.  For convenience it is guaranteed this will always be the zero value, so that users may simply pass 0 to color aspects they do not wish to set.) */
    SymbolicColor_Default = 0,
    /* DOC(This flag bit-ored with a style tag will be reevaluated at render time to the color of the specific tag in the currently active palette.  The macro SymbolicColorFromPalette(Stag_CODE) applies the bit-or to Stag_CODE.  For example SymbolicColorFromPalette(Stag_Cursor) will always evaluate to the color of the current cursor.  Note that this evaluation happens at render time, so that if the palette changes, the evaluated color will also change.) */
    SymbolicColor__StagColorFlag = 0x00800000,
};

/* DOC(Not implemented, but reserved for future use.  Where this type is used in the API the value passed should always be zero for now.) */
ENUM(i32, Marker_Visual_Text_Style)
{
    MARKER_TEXT_STYLE_NOT_YET_IMPLEMENTED,
};

/* DOC(The take rule of a marker visual specifies how to iterate the markers in the marker object when applying the visual effect.  For range marked effects, which use a pair of markers to specify a left inclusive right exclusive range, it is not necessary that two markers be adjacent in the marker object when they are taken.  The first taken marker is paired to the second taken marker, the third to the fourth, etc, regardless of any gaps between the consecutively taken markers.  If the take rule overflows the marker object, the effect is still applied, but the iteration is cut short as soon as it would overflow.) */
STRUCT Marker_Visual_Take_Rule{
    /* DOC(The index of the first marker to take.  Indices are zero based.  The default value is zero.) */
    i32 first_index;
    /* DOC(From the start of a "step" take_count_per_step markers.  Markers taken in the same step have consectuive indices.  For example, if the first marker taken has index = 0, and the take_count_per_step = 2, then the second marker taken will have index = 1. The default value is 1.) */
    i32 take_count_per_step;
    /* DOC(The stride between each "step".  After taking take_count_per_step markers from the current step, the iteation advances to the next step counting from the start of current step.  So if 2 markers are taken per step, and the stride is 3, then the pattern of taken markers will be **.**.**.**. where * is a tken marker.  The core automatically adjusts this field to be at least equal to take_count_per_step before using it to iterate. The default value is 1.) */
    i32 step_stride_in_marker_count;
    /* DOC(The maximum number of markers to be taken durring iteration.  Since overflow does not cause the visual effect to fail, and is prevented internally, this field can be set to the max value of a signed 32-bit integer and will take as many markers as possible until it hits the end of the marker object.  If the maximum count is reached mid-step, the iteration stops without completeing the step.) */
    i32 maximum_number_of_markers;
};

/* DOC(A helper enumeration for common priority levels.  Although any unsigned integer is a valid priority level, the following five levels are used to establish a standard convention.  Highest priority is given to effects immediately at the cursor, mark, and highlighted range.  Default priority is given to actively attached effects like compilation error highlights.  Passive effects like scope highlighting go to Lowest priority.

This system is considered a temporary measure and will eventually be replaced with a priority level brokering system to enable cooperation between modules [note made 4.0.29].) */
ENUM(u32, Marker_Visual_Priority_Level){
    VisualPriority_Lowest = 0,
    VisualPriority_Low = 1000,
    VisualPriority_Default = 2000,
    VisualPriority_High = 3000,
    VisualPriority_Highest = 4000,
};

/* DOC(Flags that control how font glyphs are modified before drawing.) */
ENUM(u32, Glyph_Flag){
    GlyphFlag_None = 0x0,
    GlyphFlag_Rotate90 = 0x1,
};

/* DOC(Query_Bar is a struct used to store information in the user's control that will be displayed as a drop down bar durring an interactive command.) */
STRUCT Query_Bar{
    /* DOC(This specifies the prompt portion of the drop down bar.) */
    String prompt;
    /* DOC(This specifies the main string portion of the drop down bar.) */
    String string;
};

STRUCT Query_Bar_Ptr_Array{
    Query_Bar **ptrs;
    i32 count;
};

TYPEDEF_FUNC void UI_Quit_Function_Type(struct Application_Links *app, View_Summary view);
#define UI_QUIT_FUNCTION(name) void name(struct Application_Links *app, View_Summary view)

/*
DOC(Theme_Color stores a style tag/color pair, for the purpose of setting and getting colors in the theme.)
DOC_SEE(Style_Tag)
DOC_SEE(int_color)
*/
STRUCT Theme_Color{
    /* DOC(The style slot in the style palette.) */
    id_color tag;
    /* DOC(The color in the slot.) */
    argb_color color;
};

/*
DOC(Theme lists every color that makes up a standard color scheme.)
DOC_SEE(int_color)
*/
//STRUCT Theme{
/* DOC(The colors array.  Every style tag, beginning with "Stag", is an index into it's corresponding color in the array.) */
//int_color colors[Stag_COUNT];
//};

/*
DOC(Available_Font contains a name for a font was detected at startup either in the local 4coder font folder, or by the system.  An available font is not necessarily loaded yet, and may fail to load for various reasons even though it appearsin the available font list.)
DOC_SEE(get_available_font)
*/
STRUCT Available_Font{
    char name[64];
    b32 in_local_font_folder;
};

/*
DOC(Every face is assigned a unique and consistent Face_ID for it's life time.  This represents a slot in which a face can exist.  The face in the slot is always valid once it exists, but the face might be changed or released durring it's lifetime.  A Face_ID value of zero is reserved for the meaning "not a valid face".)
*/
TYPEDEF u32 Face_ID;

/*
DOC(Face_Description contains all the information unique to a single font face, including the font name, the size, and style of the face.)
DOC_SEE(get_available_font)
*/
STRUCT Face_Description{
    /* DOC(Indicates a face's association with an available font.  This should be an exact copy of an Available_Font returned by get_available_fonts.) DOC_SEE(get_available_font) */
    Available_Font font;
    
    /* DOC(Indicates the size for the face.  Valid values must be greater than 0.  Different fonts with the same pt_size do not necessarily have the same line height.) */
    i32 pt_size;
    
    /* DOC(Indicates whether the face tries to use a bold style.) */
    b32 bold;
    
    /* DOC(Indicates whether the face tries to use an italic style.) */
    b32 italic;
    
    /* DOC(Indicates whether the face tries to underline text.) */
    b32 underline;
    
    /* DOC(Indicates whether the face tries to apply hinting.) */
    b32 hinting;
};

/* DOC(A Buffer_Batch_Edit_Type is a type of batch operation.) */
ENUM(i32, Buffer_Batch_Edit_Type){
    /* DOC(The BatchEdit_Normal operation is always correct but does the most work if there are tokens to correct.) */
    BatchEdit_Normal,
    /* DOC(The BatchEdit_PreserveTokens operation is one in which none of the edits add, delete, or change any tokens. This usually applies when whitespace is being replaced with whitespace.) */
    BatchEdit_PreserveTokens
};

/* DOC(Buffer_Edit describes a range of a buffer and string to replace that range. A Buffer_Edit has to be paired with a string that contains the actual
text that will be replaced into the buffer.) */
STRUCT Buffer_Edit{
    /* DOC(The str_start field specifies the first character in the accompanying string that corresponds with this edit.) */
    i32 str_start;
    /* DOC(The len field specifies the length of the string being written into the buffer.) */
    i32 len;
    /* DOC(The start field specifies the start of the range in the buffer to replace in absolute position.) */
    i32 start;
    /* DOC(The end field specifies one past the end of the range in the buffer to replace in absolute position.) */
    i32 end;
};

/*
DOC(This struct is used to bundle the parameters of the buffer_batch_edit function.  It is convenient for a few functions that return a batch edit to the user.)
DOC_SEE(Buffer_Edit)
DOC_SEE(buffer_batch_edit)
*/
STRUCT Buffer_Batch_Edit{
    /* DOC(The pointer to the edit string buffer.) */
    char *str;
    /* DOC(The length of the edit string buffer.) */
    i32 str_len;
    
    /* DOC(The array of edits to be applied.) */
    Buffer_Edit *edits;
    /* DOC(The number of edits in the array.) */
    i32 edit_count;
};

ENUM(i32, Record_Kind){
    RecordKind_Single,
    RecordKind_Group,
};

ENUM(i32, Record_Error){
    RecordError_NoError,
    RecordError_InvalidBuffer,
    RecordError_NoHistoryAttached,
    RecordError_IndexOutOfBounds,
    RecordError_InitialStateDummyRecord,
    RecordError_WrongRecordTypeAtIndex,
};

ENUM(u32, Record_Merge_Flag){
    RecordMergeFlag_StateInRange_MoveStateForward = 0x0,
    RecordMergeFlag_StateInRange_MoveStateBackward = 0x1,
    RecordMergeFlag_StateInRange_ErrorOut = 0x2,
};

TYPEDEF i32 History_Record_Index;

STRUCT Record_Info{
    Record_Error error;
    Record_Kind kind;
    i32 edit_number;
    union{
        struct{
            String string_forward;
            String string_backward;
            i32 first;
        } single;
        struct{
            i32 count;
        } group;
    };
};

/* DOC(Custom_Command_Function is a function type which matches the signature used for commands.  To declare a command use CUSTOM_COMMAND_SIG.) DOC_SEE(CUSTOM_COMMAND_SIG) */
TYPEDEF void Custom_Command_Function(struct Application_Links *app);

#if defined(CUSTOM_COMMAND_SIG) || defined(CUSTOM_DOC) || defined(CUSTOM_ALIAS)
#error Please do not define CUSTOM_COMMAND_SIG, CUSTOM_DOC, or CUSTOM_ALIAS
#endif

#if !defined(META_PASS)
#define CUSTOM_COMMAND_SIG(name) void name(struct Application_Links *app)
#define CUSTOM_DOC(str)
#define CUSTOM_ALIAS(x) x
#else
#define CUSTOM_COMMAND_SIG(name) CUSTOM_COMMAND_SIG(name, __FILE__, __LINE__)
#define CUSTOM_DOC(str) CUSTOM_DOC(str)
#define CUSTOM_ALIAS(x) CUSTOM_ALIAS(x)
#endif

/* DOC(Generic_Command acts as a name for a command, and can name an internal command or a custom command.) */
UNION Generic_Command{
    /* DOC(If this Generic_Command does not represent an internal command the command field is the pointer to the custom command..) */
    Custom_Command_Function *command;
};


/*
DOC(User_Input describes an event, such as key presses, mouse button presses, mouse moves,
and also non-input related events like animation frames, and view activation changes.)
DOC_SEE(Generic_Command)
DOC_SEE(Key_Event_Data)
*/
STRUCT User_Input{
    /* DOC(The description of the event.) */
    Key_Event_Data key;
    /* DOC(If this event would trigger a command, this field specifies what the command would be.) */
    Generic_Command command;
    /* DOC(This field indicates that an abort event has occurred and the command needs to shut down.
This can be set even if key and command are also set, in which case the command still needs to abort, and the key and command simply reflect
what event triggered the abort event.) */
    b32 abort;
};

/*
DOC(Data is used for passing and returing pointer size pairs.)
*/
STRUCT Data{
    /* DOC(A pointer to the data.) */
    u8 *data;
    /* DOC(The size of the data in bytes.) */
    u64 size;
};

STRUCT Frame_Info{
    i32 index;
    f32 literal_dt;
    f32 animation_dt;
};

TYPEDEF_FUNC void Render_Callback(struct Application_Links *app);

STRUCT Render_Parameters{
    Frame_Info frame;
    View_ID view_id;
    Range on_screen_range;
    Rect_i32 buffer_region;
    Render_Callback *do_core_render;
};

/* DOC(Hook_IDs name the various hooks in 4coder, these hooks use the Hook_Function signature.)
DOC_SEE(Hook_Function) */
ENUM(i32, Hook_ID){
    /* DOC(TODO) */
    hook_file_out_of_sync,
    /* DOC(TODO) */
    hook_exit,
    /* DOC(TODO) */
    hook_buffer_viewer_update,
    // never below this
    hook_type_count
};

/* DOC(Special_Hook_IDs name special hooks that use specialized signatures.) */
ENUM(i32, Special_Hook_ID){
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
    special_hook_file_edit_range,
    /* DOC(TODO) */
    special_hook_file_edit_finished,
    /* DOC(TODO) */
    special_hook_command_caller,
    /* DOC(TODO) */
    special_hook_render_caller,
    /* DOC(TODO) */
    special_hook_input_filter,
    /* DOC(TODO) */
    special_hook_start,
    /* DOC(TODO) */
    special_hook_buffer_name_resolver,
    /* DOC(TODO) */
    special_hook_modify_color_table,
    /* DOC(TODO) */
    special_hook_clipboard_change,
    /* DOC(TODO) */
    special_hook_get_view_buffer_region,
};

TYPEDEF_FUNC i32 Command_Caller_Hook_Function(struct Application_Links *app, Generic_Command cmd);
#define COMMAND_CALLER_HOOK(name) i32 name(struct Application_Links *app, Generic_Command cmd)

TYPEDEF_FUNC void Render_Caller_Function(struct Application_Links *app, Render_Parameters render_params);
#define RENDER_CALLER_SIG(name) void name(struct Application_Links *app, Render_Parameters render_params)

TYPEDEF_FUNC i32 Hook_Function(struct Application_Links *app);
#define HOOK_SIG(name) i32 name(struct Application_Links *app)

TYPEDEF_FUNC i32 Open_File_Hook_Function(struct Application_Links *app, Buffer_ID buffer_id);
#define OPEN_FILE_HOOK_SIG(name) i32 name(struct Application_Links *app, Buffer_ID buffer_id)

TYPEDEF_FUNC i32 File_Edit_Range_Function(struct Application_Links *app, Buffer_ID buffer_id, Range range, String text);
#define FILE_EDIT_RANGE_SIG(name) i32 name(struct Application_Links *app, Buffer_ID buffer_id, Range range, String text)

TYPEDEF_FUNC i32 File_Edit_Finished_Function(struct Application_Links *app, Buffer_ID *buffer_ids, i32 buffer_id_count);
#define FILE_EDIT_FINISHED_SIG(name) i32 name(struct Application_Links *app, Buffer_ID *buffer_ids, i32 buffer_id_count)

TYPEDEF_FUNC void Input_Filter_Function(Mouse_State *mouse);
#define INPUT_FILTER_SIG(name) void name(Mouse_State *mouse)

TYPEDEF_FUNC i32 Scroll_Rule_Function(float target_x, float target_y, float *scroll_x, float *scroll_y, i32 view_id, i32 is_new_target, float dt);
#define SCROLL_RULE_SIG(name) \
i32 name(float target_x, float target_y, float *scroll_x, float *scroll_y, i32 view_id, i32 is_new_target, float dt)

STRUCT Color_Table{
    argb_color *vals;
    u32 count;
};

TYPEDEF_FUNC Color_Table Modify_Color_Table_Function(struct Application_Links *app, Frame_Info frame);
#define MODIFY_COLOR_TABLE_SIG(name) Color_Table name(struct Application_Links *app, Frame_Info frame)

ENUM(u32, Clipboard_Change_Flag){
    ClipboardFlag_FromOS = 0x1,
};
TYPEDEF_FUNC void Clipboard_Change_Hook_Function(struct Application_Links *app, String contents, Clipboard_Change_Flag  flags);
#define CLIPBOARD_CHANGE_HOOK_SIG(name) void name(struct Application_Links *app, String contents, Clipboard_Change_Flag flags)

TYPEDEF_FUNC Rect_i32 Get_View_Buffer_Region_Function(struct Application_Links *app, View_ID view_id, Rect_i32 sub_region);
#define GET_VIEW_BUFFER_REGION_SIG(name) Rect_i32 name(struct Application_Links *app, View_ID view_id, Rect_i32 sub_region)

STRUCT Buffer_Name_Conflict_Entry{
    Buffer_ID buffer_id;
    char *file_name;
    i32 file_name_len;
    char *base_name;
    i32 base_name_len;
    char *unique_name_in_out;
    i32 unique_name_len_in_out;
    i32 unique_name_capacity;
};

TYPEDEF_FUNC void Buffer_Name_Resolver_Function(struct Application_Links *app, Buffer_Name_Conflict_Entry *conflicts, i32 conflict_count);
#define BUFFER_NAME_RESOLVER_SIG(n) \
void n(struct Application_Links *app, Buffer_Name_Conflict_Entry *conflicts, i32 conflict_count)

TYPEDEF_FUNC i32 Start_Hook_Function(struct Application_Links *app, char **files, i32 file_count, char **flags, i32 flag_count);
#define START_HOOK_SIG(name) \
i32 name(struct Application_Links *app, char **files, i32 file_count, char **flags, i32 flag_count)

TYPEDEF_FUNC i32 Get_Binding_Data_Function(void *data, i32 size);
#define GET_BINDING_DATA(name) i32 name(void *data, i32 size)

// NOTE(allen): Definitions for the format that Get_Binding_Data uses to launch 4coder.
// TODO(allen): Transition to a more dynamic Command_Map system.

/*
DOC(Values for the type field in the discriminated union Binding_Unit.)
DOC_SEE(Binding_Unit)
*/
ENUM(i32, Binding_Unit_Type){
    unit_header,
    unit_map_begin,
    unit_callback,
    unit_inherit,
    unit_hook
};

/* DOC(Values for built in command maps.) */
ENUM(i32, Map_ID){
    mapid_global = (1 << 24),
    mapid_file,
    mapid_ui,
    mapid_nomap
};


/*
DOC(Describes a unit of information for setting up key bindings.  A unit can set a key binding, switch the active map, set the inherited map, or set a hook.) */
STRUCT Binding_Unit{
    Binding_Unit_Type type;
    UNION{
        STRUCT{ i32 total_size; i32 user_map_count; i32 error; } header;
        STRUCT{ i32 mapid; i32 replace; i32 bind_count; } map_begin;
        STRUCT{ i32 mapid; } map_inherit;
        STRUCT{ Key_Code code; uint8_t modifiers; Custom_Command_Function *func; } callback;
        STRUCT{ i32 hook_id; void *func; } hook;
    };
};

typedef i32 _Get_Version_Function(i32 maj, i32 min, i32 patch);
#define _GET_VERSION_SIG(n) i32 n(i32 maj, i32 min, i32 patch)

STRUCT color_picker{
    String title;
    argb_color *dest;
    b32 *finished;
};

enum Found_String_Flag{
    FoundString_Sensitive = 0x1,
    FoundString_Insensitive = 0x2,
    FoundString_CleanEdges = 0x4,
    
    FoundString_Straddled = 0x10,
};

STRUCT Found_String{
    Found_String *next;
    Buffer_ID buffer_id;
    
    u32 flags;
    i32 string_id;
    
    Range location;
};

// TODO(casey): If this sticks around, there should be a way to export add/remove/merge as inlines that are shared
STRUCT Found_String_List{
    Found_String *first;
    Found_String *last;
    i32 count;
};

STRUCT Process_State {
    b32 is_updating;
    i64 return_code;
};

#endif

