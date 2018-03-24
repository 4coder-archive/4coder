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
    char string_space[256];
    char canon_dir_space[256];
    String string;
    String canon_dir;
    File_List file_list;
};

#endif

// BOTTOM

