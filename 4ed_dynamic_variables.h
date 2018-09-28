/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 22.06.2018
 *
 * Dynamic variable system
 *
 */

// TOP

#if !defined(FRED_DYNAMIC_VARIABLES_H)
#define FRED_DYNAMIC_VARIABLES_H

union Managed_Object_Standard_Header{
    u64 eight_byte_alignment__;
    struct{
        Managed_Object_Type type;
        u32 item_size;
        u32 count;
    };
};

struct Managed_Memory_Header{
    Managed_Object_Standard_Header std_header;
};

struct Managed_Buffer_Markers_Header{
    Managed_Object_Standard_Header std_header;
    Managed_Buffer_Markers_Header *next;
    Managed_Buffer_Markers_Header *prev;
    Buffer_ID buffer_id;
    struct Marker_Visuals_Data *visuals_first;
    struct Marker_Visuals_Data *visuals_last;
    i32 visuals_count;
};

struct Marker_Visuals_Data{
    Marker_Visuals_Data *next;
    Marker_Visuals_Data *prev;
    Managed_Object owner_object;
    u32 slot_id;
    u32 gen_id;
    // "Look"
    Marker_Visuals_Type type;
    u32 color;
    u32 text_color;
    Marker_Visuals_Text_Style text_style;
    // "Take Rule"
    Marker_Visuals_Take_Rule take_rule;
    // "Priority"
    Marker_Visuals_Priority_Level priority;
    // "Key View ID"
    View_ID key_view_id;
};

struct Marker_Visuals_Allocator{
    Marker_Visuals_Data *free_first;
    Marker_Visuals_Data *free_last;
    i32 free_count;
    i32 total_visual_count;
    u32_Ptr_Table id_to_ptr_table;
    u32 slot_id_counter;
};

global_const i32 managed_header_type_sizes[ManagedObjectType_COUNT] = {
    0,
    sizeof(Managed_Memory_Header),
    sizeof(Managed_Buffer_Markers_Header),
};

struct Managed_Buffer_Markers_Header_List{
    Managed_Buffer_Markers_Header *first;
    Managed_Buffer_Markers_Header *last;
    i32 count;
};

////////////////////////////////

struct Dynamic_Variable_Slot{
    Dynamic_Variable_Slot *next;
    Dynamic_Variable_Slot *prev;
    String name;
    u64 default_value;
    i32 location;
};

struct Dynamic_Variable_Layout{
    Dynamic_Variable_Slot sentinel;
    i32 location_counter;
};

struct Dynamic_Variable_Block{
    u64 *val_array;
    i32 count;
    i32 max;
};

////////////////////////////////

struct Dynamic_Memory_Header{
    Dynamic_Memory_Header *next;
};

struct Dynamic_Memory_Bank{
    Heap heap;
    Dynamic_Memory_Header *first;
    Dynamic_Memory_Header *last;
};

////////////////////////////////

struct Dynamic_Workspace{
    Dynamic_Variable_Block var_block;
    Dynamic_Memory_Bank mem_bank;
    Marker_Visuals_Allocator visuals_allocator;
    u32_Ptr_Table object_id_to_object_ptr;
    u32 object_id_counter;
    u32 scope_id;
    i32 user_type;
    void *user_back_ptr;
    Managed_Buffer_Markers_Header_List buffer_markers_list;
};

////////////////////////////////

global_const i32 lifetime_key_reference_per_node = 32;

struct Lifetime_Key_Ref_Node{
    Lifetime_Key_Ref_Node *next;
    Lifetime_Key_Ref_Node *prev;
    struct Lifetime_Key *keys[lifetime_key_reference_per_node];
};

struct Lifetime_Object{
    union{
        struct{
            Lifetime_Object *next;
            Lifetime_Object *prev;
        };
        struct{
            Lifetime_Key_Ref_Node *key_node_first;
            Lifetime_Key_Ref_Node *key_node_last;
            i32 key_count;
            Dynamic_Workspace workspace;
        };
    };
};

struct Lifetime_Key{
    union{
        struct{
            Lifetime_Key *next;
            Lifetime_Key *prev;
        };
        struct{
            struct Lifetime_Object **members;
            i32 count;
            Dynamic_Workspace dynamic_workspace;
        };
    };
};

global_const u64 LifetimeKeyHash_Empty   = 0&(~bit_63);
global_const u64 LifetimeKeyHash_Deleted = max_u64&(~bit_63);

struct Lifetime_Key_Table{
    void *mem_ptr;
    u64 *hashes;
    Lifetime_Key **keys;
    u32 count;
    u32 max;
};

struct Lifetime_Key_Ref_Node_List{
    Lifetime_Key_Ref_Node *first;
    Lifetime_Key_Ref_Node *last;
    i32 count;
};

struct Lifetime_Object_List{
    Lifetime_Object *first;
    Lifetime_Object *last;
    i32 count;
};

struct Lifetime_Key_List{
    Lifetime_Key *first;
    Lifetime_Key *last;
    i32 count;
};

struct Lifetime_Allocator{
    Lifetime_Key_Ref_Node_List free_key_references;
    Lifetime_Object_List free_objects;
    Lifetime_Key_List free_keys;
    Lifetime_Key_Table key_table;
    Ptr_Table key_check_table;
    u32_Ptr_Table scope_id_to_scope_ptr_table;
    u32 scope_id_counter;
};

struct Lifetime_Key_With_Opaque_ID{
    Lifetime_Key *key;
    u64 opaque_id;
};

////////////////////////////////

struct Managed_Object_Ptr_And_Workspace{
    Dynamic_Workspace *workspace;
    Managed_Object_Standard_Header *header;
};

#endif

// BOTTOM

