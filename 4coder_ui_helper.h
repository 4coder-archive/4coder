/*
 * Helpers for ui data structures.
 */

// TOP

#if !defined(FCODER_UI_HELPER_H)
#define FCODER_UI_HELPER_H

typedef i8 UI_Item_Type;
enum UI_Activation_Level{
    UIActivation_None = 0,
    UIActivation_Hover = 1,
    UIActivation_Active = 2,
};

typedef u8 UI_Coordinate_System;
enum{
    UICoordinates_ViewSpace = 0,
    UICoordinates_PanelSpace = 1,
    UICoordinates_COUNT = 2,
};

struct UI_Item{
    UI_Item *next;
    UI_Item *prev;
    UI_Activation_Level activation_level;
    UI_Coordinate_System coordinates;
    Rect_i32 rect_outer;
    i32 inner_margin;
    Fancy_String_List lines[4];
    i32 line_count;
    void *user_data;
};

struct UI_List{
    UI_Item *first;
    UI_Item *last;
    i32 count;
};

struct UI_Data{
    UI_List list;
    Rect_i32 bounding_box[UICoordinates_COUNT];
};

struct UI_Storage{
    UI_Data *data;
    Arena *arena;
    Managed_Object arena_object;
    Temp_Memory_Arena temp;
};

////////////////////////////////

typedef i32 Lister_Activation_Code;
enum{
    ListerActivation_Finished = 0,
    ListerActivation_Continue = 1,
    ListerActivation_ContinueAndRefresh = 2,
};

typedef void Lister_Activation_Function_Type(Application_Links *app, Partition *scratch, Heap *heap,
                                             View_Summary *view, struct Lister_State *state,
                                             String text_field, void *user_data, b32 activated_by_mouse);

typedef void Lister_Regenerate_List_Function_Type(Application_Links *app, struct Lister *lister);

struct Lister_Node{
    Lister_Node *next;
    Lister_Node *prev;
    String string;
    union{
        String status;
        i32 index;
    };
    void *user_data;
    i32 raw_index;
};

struct Lister_Option_List{
    Lister_Node *first;
    Lister_Node *last;
    i32 count;
};

struct Lister_Node_Ptr_Array{
    Lister_Node **node_ptrs;
    i32 count;
};

struct Lister_Handlers{
    Lister_Activation_Function_Type *activate;
    Lister_Regenerate_List_Function_Type *refresh;
    Custom_Command_Function *write_character;
    Custom_Command_Function *backspace;
    Custom_Command_Function *navigate_up;
    Custom_Command_Function *navigate_down;
};

struct Lister_Data{
    // Event Handlers
    Lister_Handlers handlers;
    
    // List Data
    void *user_data;
    i32 user_data_size;
    char query_space[256];
    String query;
    char text_field_space[256];
    String text_field;
    char key_string_space[256];
    String key_string;
    Lister_Option_List options;
    b32 theme_list;
};

struct Lister{
    Arena arena;
    Lister_Data data;
};

struct Lister_State{
    b32 initialized;
    Lister lister;
    
    // Action defered to next UI update
    b32 set_view_vertical_focus_to_item;
    
    // State set directly by input handlers
    void *hot_user_data;
    i32 item_index;
    
    // State of UI computed during UI update
    i32 raw_item_index;
    i32 item_count_after_filter;
};

struct Lister_Prealloced_String{
    String string;
};

////////////////////////////////

struct Lister_Option{
    String string;
    String status;
    void *user_data;
};

struct Lister_Fixed_Option{
    char *string;
    char *status;
    char *shortcut_chars;
    void *user_data;
};

struct Lister_UI_Option{
    char *string;
    i32 index;
    void *user_data;
};

#endif

// BOTTOM

