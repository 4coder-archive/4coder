/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 07.02.2019
 *
 * Types used for edit operations
 *
 */

// TOP

#if !defined(FRED_EDIT_H)
#define FRED_EDIT_H

struct Edit{
    char *str;
    i32 length;
    Range range;
};

struct Edit_Array{
    Edit *vals;
    i32 count;
};

struct Edit_Behaviors{
    b32 do_not_post_to_history;
};

#endif

// BOTTOM

