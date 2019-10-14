/*
 * Helpers for ui data structures.
 */

// TOP

#if !defined(FCODER_UI_HELPER_H)
#define FCODER_UI_HELPER_H

typedef i32 Lister_Activation_Code;
enum{
    ListerActivation_Finished = 0,
    ListerActivation_Continue = 1,
    ListerActivation_ContinueAndRefresh = 2,
};

typedef Lister_Activation_Code Lister_Activation_Type(Application_Links *app,
                                                      View_ID view, struct Lister *lister,
                                                      String_Const_u8 text_field, void *user_data, b32 activated_by_mouse);

typedef void Lister_Regenerate_List_Function_Type(Application_Links *app, struct Lister *lister);

struct Lister_Node{
    Lister_Node *next;
    Lister_Node *prev;
    String_Const_u8 string;
    union{
        String_Const_u8 status;
        i32 index;
    };
    void *user_data;
    i32 raw_index;
};

struct Lister_Node_List{
    Lister_Node *first;
    Lister_Node *last;
    i32 count;
};

struct Lister_Node_Ptr_Array{
    Lister_Node **node_ptrs;
    i32 count;
};

typedef Lister_Activation_Code Lister_Key_Stroke_Function(Application_Links *app);

struct Lister_Handlers{
    Lister_Activation_Type *activate;
    Lister_Regenerate_List_Function_Type *refresh;
    Custom_Command_Function *write_character;
    Custom_Command_Function *backspace;
    Custom_Command_Function *navigate_up;
    Custom_Command_Function *navigate_down;
    Lister_Key_Stroke_Function *key_stroke;
};

struct Lister{
    Arena *arena;
    Temp_Memory restore_all_point;
    
    Lister_Handlers handlers;
    
    Mapping *mapping;
    Command_Map *map;
    
    void *user_data;
    umem user_data_size;
    
    u8 query_space[256];
    u8 text_field_space[256];
    u8 key_string_space[256];
    String_u8 query;
    String_u8 text_field;
    String_u8 key_string;
    
    Lister_Node_List options;
    Temp_Memory filter_restore_point;
    Lister_Node_Ptr_Array filtered;
    
    b32 set_view_vertical_focus_to_item;
    Lister_Node *highlighted_node;
    void *hot_user_data;
    i32 item_index;
    i32 raw_item_index;
    
    Basic_Scroll scroll;
};

struct Lister_Prealloced_String{
    String_Const_u8 string;
};

struct Lister_Filtered{
    Lister_Node_Ptr_Array exact_matches;
    Lister_Node_Ptr_Array before_extension_matches;
    Lister_Node_Ptr_Array substring_matches;
};

struct Lister_Top_Level_Layout{
    Rect_f32 text_field_rect;
    Rect_f32 list_rect;
};

////////////////////////////////

struct Lister_Option{
    String_Const_u8 string;
    String_Const_u8 status;
    void *user_data;
};

struct Lister_Fixed_Option{
    char *string;
    char *status;
    Key_Code key_code;
    void *user_data;
};

#endif

// BOTTOM

