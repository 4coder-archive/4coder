/*
* Helpers for setting bindings.
*/

// TOP

#if !defined(FCODER_BIND_HELPER_H)
#define FCODER_BIND_HELPER_H

struct Bind_Helper{
    Binding_Unit *cursor, *start, *end;
    Binding_Unit *header, *group;
    int32_t write_total;
    int32_t error;
};

#define BH_ERR_NONE 0
#define BH_ERR_MISSING_END 1
#define BH_ERR_MISSING_BEGIN 2
#define BH_ERR_OUT_OF_MEMORY 3

struct Bind_Buffer{
    void *data;
    int32_t size;
};

#endif

// BOTTOM

