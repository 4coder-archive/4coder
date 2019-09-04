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

TYPEDEF u32 argb_color;

TYPEDEF u32 int_color;

TYPEDEF u16 id_color;

TYPEDEF u32 Child_Process_ID;

TYPEDEF i32 Buffer_ID;

TYPEDEF i32 View_ID;

TYPEDEF i32 Panel_ID;

TYPEDEF u32 Text_Layout_ID;

STRUCT Buffer_Point{
    i64 line_number;
    Vec2 pixel_shift;
};

STRUCT Line_Shift_Vertical{
    i64 line;
    f32 y_delta;
};

STRUCT Line_Shift_Character{
    i64 line;
    i64 character_delta;
};

ENUM(u32, Child_Process_Set_Target_Flags){
    ChildProcessSet_FailIfBufferAlreadyAttachedToAProcess = 1,
    ChildProcessSet_FailIfProcessAlreadyAttachedToABuffer = 2,
    ChildProcessSet_NeverOverrideExistingAttachment = 3,
    ChildProcessSet_CursorAtEnd = 4,
};

ENUM(i32, Key_Modifier_Index){
    MDFR_SHIFT_INDEX,
    MDFR_CONTROL_INDEX,
    MDFR_ALT_INDEX,
    MDFR_COMMAND_INDEX,
    
    MDFR_INDEX_BINDABLE_COUNT,
    
    MDFR_CAPS_INDEX = MDFR_INDEX_BINDABLE_COUNT,
    MDFR_HOLD_INDEX,
    
    MDFR_INDEX_COUNT
};

ENUM(u32, Key_Modifier_Flag){
    MDFR_NONE  = 0x0,
    MDFR_CTRL  = 0x1,
    MDFR_ALT   = 0x2,
    MDFR_CMND  = 0x4,
    MDFR_SHIFT = 0x8,
};

ENUM(u32, Memory_Protect_Flags){
    MemProtect_Read    = 0x1,
    MemProtect_Write   = 0x2,
    MemProtect_Execute = 0x4,
};

ENUM(i32, Wrap_Indicator_Mode){
    WrapIndicator_Hide,
    WrapIndicator_Show_After_Line,
    WrapIndicator_Show_At_Wrap_Edge,
};

ENUM(i32, Global_Setting_ID){
    GlobalSetting_Null,
    GlobalSetting_LAltLCtrlIsAltGr,
};

ENUM(i32, Buffer_Setting_ID){
    BufferSetting_Null,
    BufferSetting_MapID,
    BufferSetting_Eol,
    BufferSetting_Unimportant,
    BufferSetting_ReadOnly,
    BufferSetting_VirtualWhitespace,
    BufferSetting_RecordsHistory,
};

STRUCT Character_Predicate{
    u8 b[32];
};

ENUM(i32, View_Setting_ID){
    ViewSetting_Null,
    ViewSetting_ShowWhitespace,
    ViewSetting_ShowScrollbar,
    ViewSetting_ShowFileBar,
    ViewSetting_UICommandMap
};

ENUM(u32, Buffer_Create_Flag){
    BufferCreate_Background = 0x1,
    BufferCreate_AlwaysNew  = 0x2,
    BufferCreate_NeverNew   = 0x4,
    BufferCreate_JustChangedFile = 0x8,
    BufferCreate_MustAttachToFile = 0x10,
    BufferCreate_NeverAttachToFile = 0x20,
    BufferCreate_SuppressNewFileHook = 0x40,
};

ENUM(u32, Buffer_Save_Flag){
    BufferSave_IgnoreDirtyFlag = 0x1,
};

ENUM(u32, Buffer_Kill_Flag){
    BufferKill_AlwaysKill  = 0x2,
};

ENUM(u32, Buffer_Reopen_Flag){};

ENUM(i32, Buffer_Kill_Result){
    BufferKillResult_Killed = 0,
    BufferKillResult_Dirty = 1,
    BufferKillResult_Unkillable = 2,
    BufferKillResult_DoesNotExist = 3,
};

ENUM(i32, Buffer_Reopen_Result){
    BufferReopenResult_Reopened = 0,
    BufferReopenResult_Failed = 1,
};

ENUM(u32, Access_Flag){
    AccessOpen      = 0x0,
    AccessProtected = 0x1,
    AccessHidden    = 0x2,
    AccessAll       = 0xFF
};

ENUM(u32, Dirty_State){
    DirtyState_UpToDate = 0,
    DirtyState_UnsavedChanges = 1,
    DirtyState_UnloadedChanges = 2,
    DirtyState_UnsavedChangesAndUnloadedChanges = 3,
};

ENUM(u32, Command_Line_Interface_Flag){
    CLI_OverlapWithConflict = 0x1,
    CLI_AlwaysBindToView    = 0x2,
    CLI_CursorAtEnd         = 0x4,
    CLI_SendEndSignal       = 0x8,
};

ENUM(u32, Auto_Indent_Flag){
    AutoIndent_ClearLine = 0x1,
    AutoIndent_UseTab    = 0x2,
    AutoIndent_ExactAlignBlock = 0x4,
    AutoIndent_FullTokens = 0x8,
};

ENUM(u32, Set_Buffer_Flag){
    SetBuffer_KeepOriginalGUI = 0x1
};

ENUM(u32, Input_Type_Flag){
    EventOnAnyKey  = 0x1,
    EventOnEsc     = 0x2,
    EventOnMouseLeftButton  = 0x4,
    EventOnMouseRightButton = 0x8,
    EventOnMouseWheel       = 0x10,
    EventOnMouseMove        = 0x20,
    EventOnAnimate        = 0x40,
    EventOnViewActivation = 0x80,
    EventAll = 0xFFFFFFFF,
};

ENUM(i32, Mouse_Cursor_Show_Type){
    MouseCursorShow_Never,
    MouseCursorShow_Always,
};

ENUM(i32, View_Split_Position){
    ViewSplit_Top,
    ViewSplit_Bottom,
    ViewSplit_Left,
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

TYPEDEF u32 Key_Code;

TYPEDEF u8 Key_Modifier;

STRUCT Key_Event_Data{
    Key_Code keycode;
    Key_Code character;
    Key_Code character_no_caps_lock;
    i8 modifiers[MDFR_INDEX_COUNT];
};

GLOBAL_VAR Key_Event_Data null_key_event_data = {};

STRUCT Mouse_State{
    int8_t l;
    int8_t r;
    int8_t press_l;
    int8_t press_r;
    int8_t release_l;
    int8_t release_r;
    int8_t out_of_window;
    i32 wheel;
    UNION{
        STRUCT{
            i32 x;
            i32 y;
        };
        Vec2_i32 p;
    };
};

STRUCT Parser_String_And_Type{
    char *str;
    u32 length;
    u32 type;
};

TYPEDEF u64 Microsecond_Time_Stamp;

ENUM(u32, File_Attribute_Flag){
    FileAttribute_IsDirectory = 1,
};

STRUCT File_Attributes{
    u64 size;
    u64 last_write_time;
    File_Attribute_Flag flags;
};

STRUCT File_Info{
    File_Info *next;
    String_Const_u8 file_name;
    File_Attributes attributes;
};

STRUCT File_List{
    File_Info **infos;
    u32 count;
};

STRUCT Buffer_Identifier{
    char *name;
    i32 name_len;
    Buffer_ID id;
};

STRUCT Buffer_Scroll{
    Buffer_Point position;
    Buffer_Point target;
};

STRUCT Basic_Scroll{
    Vec2_f32 position;
    Vec2_f32 target;
};

ENUM(i32, Buffer_Seek_Type){
    buffer_seek_pos,
    buffer_seek_line_col,
};

STRUCT Buffer_Seek{
    Buffer_Seek_Type type;
    UNION{
        STRUCT{
            i64 pos;
        };
        STRUCT{
            i64 line;
            i64 col;
        };
    };
};

STRUCT Buffer_Cursor{
    i64 pos;
    i64 line;
    i64 col;
};

STRUCT Range_Cursor{
    struct{
        Buffer_Cursor min;
        Buffer_Cursor max;
    };
    struct{
        Buffer_Cursor begin;
        Buffer_Cursor end;
    };
    struct{
        Buffer_Cursor start;
        Buffer_Cursor end;
    };
    struct{
        Buffer_Cursor first;
        Buffer_Cursor one_past_last;
    };
};

STRUCT Marker{
    i64 pos;
    b32 lean_right;
};


ENUM(i32, Managed_Object_Type)
{
    ManagedObjectType_Error = 0,
    ManagedObjectType_Memory = 1,
    ManagedObjectType_Markers = 2,
    
    ManagedObjectType_COUNT = 4,
};


TYPEDEF u64 Managed_ID;

TYPEDEF u64 Managed_Scope;
TYPEDEF u64 Managed_Object;

static Managed_Scope ManagedScope_NULL = 0;
static Managed_Object ManagedObject_NULL = 0;

static Managed_ID ManagedIndex_ERROR = 0;

STRUCT Marker_Visual{
    Managed_Scope scope;
    u32 slot_id;
    u32 gen_id;
};

ENUM(u32, Glyph_Flag){
    GlyphFlag_None = 0x0,
    GlyphFlag_Rotate90 = 0x1,
};

STRUCT Query_Bar{
    String_Const_u8 prompt;
    String_Const_u8 string;
    umem string_capacity;
};

STRUCT Query_Bar_Ptr_Array{
    Query_Bar **ptrs;
    i32 count;
};

TYPEDEF_FUNC void UI_Quit_Function_Type(struct Application_Links *app, View_ID view);
#define UI_QUIT_FUNCTION(name) void name(struct Application_Links *app, View_ID view)

STRUCT Theme_Color{
    id_color tag;
    argb_color color;
};

//STRUCT Theme{
//int_color colors[Stag_COUNT];
//};

TYPEDEF u32 Face_ID;

STRUCT Font_Load_Location{
    String_Const_u8 file_name;
    b32 in_4coder_font_folder;
};

STRUCT Face_Load_Parameters{
    u32 pt_size;
    b32 bold;
    b32 italic;
    b32 underline;
    b32 hinting;
};

STRUCT Face_Description{
    Font_Load_Location font;
    Face_Load_Parameters parameters;
};

STRUCT Face_Metrics{
    f32 line_height;
    f32 typical_character_width;
};

STRUCT Edit{
    String_Const_u8 text;
    Interval_i64 range;
};

STRUCT Batch_Edit{
    Batch_Edit *next;
    Edit edit;
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
    RecordError_SubIndexOutOfBounds,
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
            String_Const_u8 string_forward;
            String_Const_u8 string_backward;
            i64 first;
        } single;
        struct{
            i32 count;
        } group;
    };
};

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

UNION Generic_Command{
    Custom_Command_Function *command;
};


STRUCT User_Input{
    Key_Event_Data key;
    Generic_Command command;
    b32 abort;
};

STRUCT Frame_Info{
    i32 index;
    f32 literal_dt;
    f32 animation_dt;
};

TYPEDEF_FUNC void Render_Callback(struct Application_Links *app);

ENUM(i32, Hook_ID){
    hook_file_out_of_sync,
    hook_exit,
    hook_buffer_viewer_update,
    // never below this
    hook_type_count
};

ENUM(i32, Special_Hook_ID){
    special_hook_scroll_rule = hook_type_count,
    special_hook_new_file,
    special_hook_open_file,
    special_hook_save_file,
    special_hook_end_file,
    special_hook_file_edit_range,
    special_hook_file_edit_finished,
    special_hook_file_externally_modified,
    special_hook_command_caller,
    special_hook_render_caller,
    special_hook_input_filter,
    special_hook_start,
    special_hook_buffer_name_resolver,
    special_hook_modify_color_table,
    special_hook_clipboard_change,
    special_hook_get_view_buffer_region,
};

TYPEDEF_FUNC i32 Command_Caller_Hook_Function(struct Application_Links *app, Generic_Command cmd);
#define COMMAND_CALLER_HOOK(name) i32 name(struct Application_Links *app, Generic_Command cmd)

TYPEDEF_FUNC void Render_Caller_Function(struct Application_Links *app, Frame_Info frame_info);
#define RENDER_CALLER_SIG(name) void name(struct Application_Links *app, Frame_Info frame_info)

TYPEDEF_FUNC i32 Hook_Function(struct Application_Links *app);
#define HOOK_SIG(name) i32 name(struct Application_Links *app)

TYPEDEF_FUNC i32 Buffer_Hook_Function(struct Application_Links *app, Buffer_ID buffer_id);
#define BUFFER_HOOK_SIG(name) i32 name(struct Application_Links *app, Buffer_ID buffer_id)

TYPEDEF_FUNC i32 File_Edit_Range_Function(struct Application_Links *app, Buffer_ID buffer_id, 
                                          Interval_i64 range, String_Const_u8 text);
#define FILE_EDIT_RANGE_SIG(name) i32 name(struct Application_Links *app, Buffer_ID buffer_id, Interval_i64 range, String_Const_u8 text)

TYPEDEF_FUNC i32 File_Edit_Finished_Function(struct Application_Links *app, Buffer_ID *buffer_ids, i32 buffer_id_count);
#define FILE_EDIT_FINISHED_SIG(name) i32 name(struct Application_Links *app, Buffer_ID *buffer_ids, i32 buffer_id_count)

TYPEDEF_FUNC i32 File_Externally_Modified_Function(struct Application_Links *app, Buffer_ID buffer_id);
#define FILE_EXTERNALLY_MODIFIED_SIG(name) i32 name(struct Application_Links *app, Buffer_ID buffer_id)

TYPEDEF_FUNC void Input_Filter_Function(Mouse_State *mouse);
#define INPUT_FILTER_SIG(name) void name(Mouse_State *mouse)

TYPEDEF_FUNC Vec2_f32 Delta_Rule_Function(Vec2_f32 pending_delta, View_ID view_id, b32 is_new_target, f32 dt);
#define DELTA_RULE_SIG(name) Vec2_f32 name(Vec2_f32 pending_delta, View_ID view_id, b32 is_new_target, f32 dt)

STRUCT Color_Table{
    argb_color *vals;
    u32 count;
};

TYPEDEF_FUNC Color_Table Modify_Color_Table_Function(struct Application_Links *app, Frame_Info frame);
#define MODIFY_COLOR_TABLE_SIG(name) Color_Table name(struct Application_Links *app, Frame_Info frame)

ENUM(u32, Clipboard_Change_Flag){
    ClipboardFlag_FromOS = 0x1,
};
TYPEDEF_FUNC void Clipboard_Change_Hook_Function(struct Application_Links *app, String_Const_u8 contents, Clipboard_Change_Flag  flags);
#define CLIPBOARD_CHANGE_HOOK_SIG(name) void name(struct Application_Links *app, String_Const_u8 contents, Clipboard_Change_Flag flags)

TYPEDEF_FUNC Rect_f32 Get_View_Buffer_Region_Function(struct Application_Links *app, View_ID view_id, Rect_f32 sub_region);
#define GET_VIEW_BUFFER_REGION_SIG(name) Rect_f32 name(struct Application_Links *app, View_ID view_id, Rect_f32 sub_region)

STRUCT Buffer_Name_Conflict_Entry{
    Buffer_ID buffer_id;
    String_Const_u8 file_name;
    String_Const_u8 base_name;
    u8 *unique_name_in_out;
    umem unique_name_len_in_out;
    umem unique_name_capacity;
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


ENUM(i32, Binding_Unit_Type){
    unit_header,
    unit_map_begin,
    unit_callback,
    unit_inherit,
    unit_hook
};

ENUM(i32, Map_ID){
    mapid_global = (1 << 24),
    mapid_file,
    mapid_ui,
    mapid_nomap
};


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

STRUCT Color_Picker{
    String_Const_u8 title;
    argb_color *dest;
    b32 *finished;
};

ENUM(u32, String_Match_Flag){
    StringMatch_CaseSensitive = 1,
    StringMatch_LeftSideSloppy = 2,
    StringMatch_RightSideSloppy = 4,
    StringMatch_Straddled = 8,
};

STRUCT String_Match{
    String_Match *next;
    Buffer_ID buffer;
    i32 string_id;
    String_Match_Flag flags;
    Range_i64 range;
};

STRUCT String_Match_List{
    String_Match *first;
    String_Match *last;
    i32 count;
};

STRUCT Process_State{
    b32 valid;
    b32 is_updating;
    i64 return_code;
};

#endif

