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

struct Ptr_Check_Table{
    void **keys;
    u32 count;
    u32 max;
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
            i32 user_type;
            void *user_back_ptr;
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
            Dynamic_Variable_Block dynamic_vars;
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
    Lifetime_Ptr_Check_Table key_check_table;
};

struct Lifetime_Key_With_Opaque_ID{
    Lifetime_Key *key;
    u64 opaque_id;
};

#endif

// BOTTOM

