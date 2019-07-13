/*
 * 4coder tables
 */

// TOP

#if !defined(FCODER_TABLES_H)
#define FCODER_TABLES_H

struct Table_Lookup{
    u64 hash;
    u32 index;
    b8 found_match;
    b8 found_empty_slot;
    b8 found_erased_slot;
};

struct Table_u64_u64{
    Base_Allocator *allocator;
    void *memory;
    u64 *keys;
    u64 *vals;
    u32 slot_count;
    u32 used_count;
    u32 dirty_count;
};

struct Table_Data_u64{
    Base_Allocator *allocator;
    void *memory;
    u64 *hashes;
    Data *keys;
    u64 *vals;
    u32 slot_count;
    u32 used_count;
    u32 dirty_count;
};

struct Table_u64_Data{
    Base_Allocator *allocator;
    void *memory;
    u64 *keys;
    Data *vals;
    u32 slot_count;
    u32 used_count;
    u32 dirty_count;
};

struct Table_Data_Data{
    Base_Allocator *allocator;
    void *memory;
    u64 *hashes;
    Data *keys;
    Data *vals;
    u32 slot_count;
    u32 used_count;
    u32 dirty_count;
};

#endif

// BOTTOM
