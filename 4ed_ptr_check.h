/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 26.08.2018
 *
 * Pointer check table
 *
 */

// TOP

#if !defined(FRED_POINTER_CHECK_H)
#define FRED_POINTER_CHECK_H

struct Ptr_Table{
    void *mem;
    u64 *hashes;
    i32 count;
    i32 dirty_slot_count;
    i32 max;
};

struct u32_Ptr_Lookup_Result{
    b32 success;
    void**val;
};
struct u32_Ptr_Table{
    void *mem;
    u64 *hashes;
    void**vals;
    i32 count;
    i32 dirty_slot_count;
    i32 max;
};

#endif

// BOTTOM

