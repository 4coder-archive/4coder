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

struct Application_Links{
    void *cmd_context;
    void *current_thread;
    void *current_coroutine;
    i32 type_coroutine;
};
typedef void Custom_Layer_Init_Type(Application_Links *app);
void custom_layer_init(Application_Links *app);

typedef b32 _Get_Version_Type(i32 maj, i32 min, i32 patch);
typedef Custom_Layer_Init_Type *_Init_APIs_Type(struct API_VTable_custom *custom_vtable,
                                                struct API_VTable_system *system_vtable);

////////////////////////////////

TYPEDEF u32 argb_color;

TYPEDEF u32 int_color;

TYPEDEF u16 id_color;

TYPEDEF u32 Child_Process_ID;

TYPEDEF i32 Buffer_ID;

TYPEDEF i32 View_ID;

TYPEDEF i32 Panel_ID;

TYPEDEF u32 Text_Layout_ID;

typedef i32 UI_Highlight_Level;
enum{
    UIHighlight_None,
    UIHighlight_Hover,
    UIHighlight_Active,
};

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
    BufferSetting_Unimportant,
    BufferSetting_ReadOnly,
    BufferSetting_RecordsHistory,
};

STRUCT Character_Predicate{
    u8 b[32];
};

struct Frame_Info{
    i32 index;
    f32 literal_dt;
    f32 animation_dt;
};

ENUM(i32, View_Setting_ID){
    ViewSetting_Null,
    ViewSetting_ShowWhitespace,
    ViewSetting_ShowScrollbar,
    ViewSetting_ShowFileBar,
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

typedef u32 Access_Flag;
enum{
    Access_Write = 0x1,
    Access_Read = 0x2,
    Access_Visible = 0x4,
};
enum{
    Access_Always = 0,
    Access_ReadWrite = Access_Write|Access_Read,
    Access_ReadVisible = Access_Read|Access_Visible,
    Access_ReadWriteVisible = Access_Write|Access_Read|Access_Visible,
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

ENUM(u32, Set_Buffer_Flag){
    SetBuffer_KeepOriginalGUI = 0x1
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

TYPEDEF u8 Key_Modifier;

STRUCT Mouse_State{
    b8 l;
    b8 r;
    b8 press_l;
    b8 press_r;
    b8 release_l;
    b8 release_r;
    b8 out_of_window;
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

typedef i32 Set_Buffer_Scroll_Rule;
enum{
    SetBufferScroll_NoCursorChange,
    SetBufferScroll_SnapCursorIntoView,
};

struct Buffer_Scroll{
    Buffer_Point position;
    Buffer_Point target;
};

struct Basic_Scroll{
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

struct Query_Bar{
    String_Const_u8 prompt;
    String_Const_u8 string;
    umem string_capacity;
};

struct Query_Bar_Ptr_Array{
    Query_Bar **ptrs;
    i32 count;
};

struct Query_Bar_Group{
    Application_Links *app;
    View_ID view;
    
    Query_Bar_Group(Application_Links *app);
    Query_Bar_Group(Application_Links *app, View_ID view);
    ~Query_Bar_Group();
};

STRUCT Theme_Color{
    id_color tag;
    argb_color color;
};

//STRUCT Theme{
//int_color colors[Stag_COUNT];
//};

typedef  u32 Face_ID;

struct Font_Load_Location{
    String_Const_u8 file_name;
    b32 in_4coder_font_folder;
};

struct Face_Load_Parameters{
    u32 pt_size;
    b32 bold;
    b32 italic;
    b32 underline;
    b32 hinting;
};

struct Face_Description{
    Font_Load_Location font;
    Face_Load_Parameters parameters;
};

struct Face_Metrics{
    f32 text_height;
    f32 line_height;
    f32 max_advance;
    f32 normal_advance;
    f32 space_advance;
    f32 decimal_digit_advance;
    f32 hex_digit_advance;
};

struct Edit{
    String_Const_u8 text;
    Interval_i64 range;
};

struct Batch_Edit{
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

#if defined(CUSTOM_COMMAND_SIG) || defined(CUSTOM_UI_COMMAND_SIG) || defined(CUSTOM_DOC) || defined(CUSTOM_COMMAND)
#error Please do not define CUSTOM_COMMAND_SIG, CUSTOM_DOC, CUSTOM_UI_COMMAND_SIG, or CUSTOM_COMMAND
#endif

#if !defined(META_PASS)
#define CUSTOM_COMMAND_SIG(name) void name(struct Application_Links *app)
#define CUSTOM_UI_COMMAND_SIG(name) void name(struct Application_Links *app)
#define CUSTOM_DOC(str)
#else
#define CUSTOM_COMMAND_SIG(name) CUSTOM_COMMAND(name, __FILE__, __LINE__, Normal)
#define CUSTOM_UI_COMMAND_SIG(name) CUSTOM_COMMAND(name, __FILE__, __LINE__, UI)
#define CUSTOM_DOC(str) CUSTOM_DOC(str)
#endif

// TODO(allen): rename
STRUCT User_Input{
    Input_Event event;
    b32 abort;
};

typedef i32 Hook_ID;
enum{
    HookID_RenderCaller,
    HookID_DeltaRule,
    HookID_BufferViewerUpdate,
    HookID_ViewEventHandler,
    HookID_BufferNameResolver,
    HookID_BeginBuffer,
    HookID_EndBuffer,
    HookID_NewFile,
    HookID_SaveFile,
    HookID_BufferEditRange,
    HookID_BufferRegion,
};

typedef i32 Hook_Function(Application_Links *app);
#define HOOK_SIG(name) i32 name(Application_Links *app)

struct Buffer_Name_Conflict_Entry{
    Buffer_ID buffer_id;
    String_Const_u8 file_name;
    String_Const_u8 base_name;
    u8 *unique_name_in_out;
    umem unique_name_len_in_out;
    umem unique_name_capacity;
};

typedef void Buffer_Name_Resolver_Function(Application_Links *app, Buffer_Name_Conflict_Entry *conflicts, i32 conflict_count);
#define BUFFER_NAME_RESOLVER_SIG(n) \
void n(Application_Links *app, Buffer_Name_Conflict_Entry *conflicts, i32 conflict_count)

typedef i32 Buffer_Hook_Function(Application_Links *app, Buffer_ID buffer_id);
#define BUFFER_HOOK_SIG(name) i32 name(Application_Links *app, Buffer_ID buffer_id)

typedef i32 Buffer_Edit_Range_Function(Application_Links *app, Buffer_ID buffer_id,
                                       Range_i64 range, String_Const_u8 text);
#define BUFFER_EDIT_RANGE_SIG(name) i32 name(Application_Links *app, Buffer_ID buffer_id,\
Interval_i64 range, String_Const_u8 text)

typedef Vec2_f32 Delta_Rule_Function(Vec2_f32 pending, b32 is_new_target, f32 dt, void *data);
#define DELTA_RULE_SIG(name) \
Vec2_f32 name(Vec2_f32 pending, b32 is_new_target, f32 dt, void *data)

typedef Rect_f32 Buffer_Region_Function(Application_Links *app, View_ID view_id, Rect_f32 region);

struct Color_Table{
    argb_color *vals;
    u32 count;
};

typedef void New_Clipboard_Contents_Function(Application_Links *app, String_Const_u8 contents);
#define NEW_CLIPBOARD_CONTENTS_SIG(name) \
void name(Application_Links *app, String_Const_u8 contents)

typedef void Render_Caller_Function(Application_Links *app, Frame_Info frame_info, View_ID view);
#define RENDER_CALLER_SIG(name) void name(Application_Links *app, Frame_Info frame_info, View_ID view)

typedef i64 Command_Map_ID;

struct Command_Binding{
    Custom_Command_Function *custom;
};

struct Command_Modified_Binding{
    Command_Modified_Binding *next;
    SNode order_node;
    Input_Modifier_Set modifiers;
    Command_Binding binding;
};

struct Command_Binding_List{
    Command_Binding_List *next;
    SNode *first;
    SNode *last;
    i32 count;
};

struct Command_Map{
    Command_Map *next;
    Command_Map_ID id;
    Command_Map_ID parent;
    Command_Binding text_input_command;
    Arena node_arena;
    Table_u64_u64 event_code_to_binding_list;
    Command_Modified_Binding *binding_first;
    Command_Modified_Binding *binding_last;
    Command_Binding_List *list_first;
    Command_Binding_List *list_last;
    
    struct Binding_Unit *real_beginning;
};

struct Mapping{
    Arena *node_arena;
    Heap heap;
    Base_Allocator heap_wrapper;
    Table_u64_u64 id_to_map;
    Command_Map_ID id_counter;
    Command_Map *free_maps;
    Command_Modified_Binding *free_bindings;
    Command_Binding_List *free_lists;
};

struct View_Context{
    Render_Caller_Function *render_caller;
    Delta_Rule_Function *delta_rule;
    umem delta_rule_memory_size;
    b32 hides_buffer;
    Mapping *mapping;
    Command_Map_ID map_id;
};

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

struct String_Match{
    String_Match *next;
    Buffer_ID buffer;
    i32 string_id;
    String_Match_Flag flags;
    Range_i64 range;
};

struct String_Match_List{
    String_Match *first;
    String_Match *last;
    i32 count;
};

struct Process_State{
    b32 valid;
    b32 is_updating;
    i64 return_code;
};

#endif

