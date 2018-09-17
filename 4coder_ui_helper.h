/*
 * Helpers for ui data structures.
 */

// TOP

#if !defined(FCODER_UI_HELPER_H)
#define FCODER_UI_HELPER_H

typedef int32_t Lister_Activation_Code;
enum{
    ListerActivation_Finished = 0,
    ListerActivation_Continue = 1,
    ListerActivation_ContinueAndRefresh = 2,
};

typedef void Lister_Activation_Function_Type(Application_Links *app, Partition *scratch, Heap *heap,
                                             View_Summary *view, struct Lister_State *state,
                                             String text_field, void *user_data, bool32 activated_by_mouse);

typedef void Lister_Regenerate_List_Function_Type(Application_Links *app, Partition *arena, struct Lister *lister);

struct Lister_Node{
    Lister_Node *next;
    Lister_Node *prev;
    String string;
    union{
        String status;
        int32_t index;
    };
    void *user_data;
    int32_t raw_index;
};

struct Lister_Option_List{
    Lister_Node *first;
    Lister_Node *last;
    int32_t count;
};

struct Lister_Node_Ptr_Array{
    Lister_Node **node_ptrs;
    int32_t count;
};

struct Lister_Handlers{
    Lister_Activation_Function_Type *activate;
    Lister_Regenerate_List_Function_Type *refresh;
    Custom_Command_Function *write_character;
    Custom_Command_Function *backspace;
    Custom_Command_Function *navigate_up;
    Custom_Command_Function *navigate_down;
};

struct Lister{
    // Event Handlers
    Lister_Handlers handlers;
    
    // List Data
    void *user_data;
    char query_space[256];
    String query;
    char text_field_space[256];
    String text_field;
    char key_string_space[256];
    String key_string;
    Lister_Option_List options;
    bool32 theme_list;
};

struct Lister_Prealloced_String{
    String string;
};

struct Lister_State{
    bool32 initialized;
    void *hot_user_data;
    int32_t item_index;
    int32_t raw_item_index;
    bool32 set_view_vertical_focus_to_item;
    int32_t option_item_count;
    Partition arena;
    Lister lister;
};

////////////////////////////////

struct Lister_Option{
    char *string;
    char *status;
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
    int32_t index;
    void *user_data;
};

#endif

// BOTTOM

