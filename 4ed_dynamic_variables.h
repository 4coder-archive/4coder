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
};

struct Managed_Arena_Header{
    Managed_Object_Standard_Header std_header;
    Managed_Arena_Header *next;
    Managed_Arena_Header *prev;
    Arena arena;
};

global_const i32 managed_header_type_sizes[ManagedObjectType_COUNT] = {
    0,
    sizeof(Managed_Memory_Header),
    sizeof(Managed_Buffer_Markers_Header),
    sizeof(Managed_Arena_Header),
};

struct Managed_Buffer_Markers_Header_List{
    Managed_Buffer_Markers_Header *first;
    Managed_Buffer_Markers_Header *last;
    i32 count;
};

struct Managed_Arena_Header_List{
    Managed_Arena_Header *first;
    Managed_Arena_Header *last;
    i32 count;
};

////////////////////////////////

struct Managed_ID_Group{
    Table_Data_u64 name_to_id_table;
    Managed_ID id_counter;
};

struct Managed_ID_Set{
    Arena arena;
    Table_Data_u64 name_to_group_table;
};

struct Dynamic_Variable_Block{
    Arena arena;
    Table_u64_Data id_to_data_table;
};

////////////////////////////////

struct Dynamic_Workspace{
    Dynamic_Variable_Block var_block;
    Heap heap;
    Base_Allocator heap_wrapper;
    Table_u64_u64 object_id_to_object_ptr;
    u32 object_id_counter;
    u32 visual_id_counter;
    u32 scope_id;
    i32 user_type;
    void *user_back_ptr;
    Managed_Buffer_Markers_Header_List buffer_markers_list;
    Managed_Arena_Header_List arena_list;
    i32 total_marker_count;
};

////////////////////////////////

global_const i32 lifetime_key_reference_per_node = 32;

struct Lifetime_Key_Ref_Node{
    Lifetime_Key_Ref_Node *next;
    Lifetime_Key_Ref_Node *prev;
    struct Lifetime_Key *keys[lifetime_key_reference_per_node];
};

union Lifetime_Object{
    Lifetime_Object *next;
    struct{
        Lifetime_Key_Ref_Node *key_node_first;
        Lifetime_Key_Ref_Node *key_node_last;
        i32 key_count;
        Dynamic_Workspace workspace;
    };
};

struct Lifetime_Key{
    union{
        struct{
            Lifetime_Key *next;
            Lifetime_Key *prev;
        };
        struct{
            Lifetime_Object **members;
            i32 count;
            Dynamic_Workspace dynamic_workspace;
        };
    };
};

global_const u64 LifetimeKeyHash_Empty   = 0&(~bit_63);
global_const u64 LifetimeKeyHash_Deleted = max_u64&(~bit_63);

struct Lifetime_Allocator{
    Base_Allocator *allocator;
    Arena node_arena;
    Lifetime_Key_Ref_Node *free_key_references;
    Lifetime_Key* free_keys;
    Lifetime_Object *free_objects;
    Table_Data_u64 key_table;
    Table_u64_u64 key_check_table;
    Table_u64_u64 scope_id_to_scope_ptr_table;
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

