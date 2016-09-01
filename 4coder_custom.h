
#ifndef FCODER_CUSTOM_H
#define FCODER_CUSTOM_H

#include <stdint.h>
// TODO(allen): We need to eliminate our dependence on this soon.
#include <string.h>


#include "4coder_version.h"
#include "4coder_keycodes.h"
#include "4coder_style.h"
#include "4coder_rect.h"
#include "4coder_mem.h"

#ifndef FSTRING_STRUCT
#define FSTRING_STRUCT
typedef struct String{
    char *str;
    int32_t size;
    int32_t memory_size;
} String;

typedef struct Offset_String{
    int32_t offset;
    int32_t size;
} Offset_String;
#endif

// These are regular hooks, any of them can be set to any function
// that matches the HOOK_SIG pattern.
enum Hook_ID{
    hook_start,
    hook_file_out_of_sync,
    // never below this
    hook_type_count
};

// These are for special hooks, each must bind to specialized signatures
// that do not necessarily have access to the app pointer.
enum Special_Hook_ID{
    _hook_scroll_rule = hook_type_count,
    _hook_new_file,
    _hook_open_file,
    _hook_command_caller,
    _hook_input_filter,
};

#define CUSTOM_COMMAND_SIG(name) void name(struct Application_Links *app)
typedef CUSTOM_COMMAND_SIG(Custom_Command_Function);

#include "4coder_types.h"
#include "4coder_seek_types.h"
#include "4coder_gui.h"

#define COMMAND_CALLER_HOOK(name) int32_t name(struct Application_Links *app, Generic_Command cmd)
typedef COMMAND_CALLER_HOOK(Command_Caller_Hook_Function);

static Key_Event_Data null_key_event_data = {0};
static Mouse_State    null_mouse_state    = {0};
static Buffer_Summary null_buffer_summary = {0};
static View_Summary   null_view_summary   = {0};

inline Range
make_range(int32_t p1, int32_t p2){
    Range range;
    if (p1 < p2){
        range.min = p1;
        range.max = p2;
    }
    else{
        range.min = p2;
        range.max = p1;
    }
    return(range);
}

#define HOOK_SIG(name) int32_t name(struct Application_Links *app)
#define OPEN_FILE_HOOK_SIG(name) int32_t name(struct Application_Links *app, int32_t buffer_id)
#define SCROLL_RULE_SIG(name) int32_t name(float target_x, float target_y, float *scroll_x, float *scroll_y, int32_t view_id, int32_t is_new_target, float dt)
#define INPUT_FILTER_SIG(name) void name(Mouse_State *mouse)

typedef HOOK_SIG(Hook_Function);
typedef OPEN_FILE_HOOK_SIG(Open_File_Hook_Function);
typedef SCROLL_RULE_SIG(Scroll_Rule_Function);
typedef INPUT_FILTER_SIG(Input_Filter_Function);




struct Application_Links;
#include "4coder_custom_api.h"



#define VIEW_ROUTINE_SIG(name) void name(struct Application_Links *app, int32_t view_id)
#define GET_BINDING_DATA(name) int32_t name(void *data, int32_t size)
#define _GET_VERSION_SIG(n) int32_t n(int32_t maj, int32_t min, int32_t patch)

typedef VIEW_ROUTINE_SIG(View_Routine_Function);
typedef GET_BINDING_DATA(Get_Binding_Data_Function);
typedef _GET_VERSION_SIG(_Get_Version_Function);

struct Custom_API{
    View_Routine_Function *view_routine;
    Get_Binding_Data_Function *get_bindings;
    _Get_Version_Function *get_alpha_4coder_version;
};

extern "C" _GET_VERSION_SIG(get_alpha_4coder_version){
    int32_t result = (maj == MAJOR && min == MINOR && patch == PATCH);
    return(result);
}

// NOTE(allen): definitions for the buffer that communicates to 4ed.exe

enum Binding_Unit_Type{
    unit_header,
    unit_map_begin,
    unit_binding,
    unit_callback,
    unit_inherit,
    unit_hook
};

enum Map_ID{
    mapid_global = (1 << 24),
    mapid_file,
    
    // NOTE(allen): mapid_nomap will remain empty even if you attempt to fill it
    // it is for setting a map's parent to nothing, in cases where you don't want
    // to inherit from global (which is the default).
    mapid_nomap
};

struct Binding_Unit{
    Binding_Unit_Type type;
    union{
        struct{ int32_t total_size; int32_t user_map_count; int32_t error; } header;
        
        struct{ int32_t mapid; int32_t replace; int32_t bind_count; } map_begin;
        struct{ int32_t mapid; } map_inherit;
        struct{
            int16_t code;
            uint8_t modifiers;
            int32_t command_id;
        } binding;
        struct{
            int16_t code;
            uint8_t modifiers;
            Custom_Command_Function *func;
        } callback;
        struct{
            int32_t hook_id;
            void *func;
        } hook;
    };
};

#endif
