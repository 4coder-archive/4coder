/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 24.01.2018
 *
 * Buffer types
 *
 */

// TOP

#if !defined(FRED_HOT_DIRECTORY_H)
#define FRED_HOT_DIRECTORY_H

struct Hot_Directory{
    u8 string_space[256];
    umem string_size;
    u8 canon_dir_space[256];
    umem canon_dir_size;
    File_List file_list;
};

#endif

// BOTTOM

