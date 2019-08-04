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
    Arena arena;
    String_Const_u8 string;
    String_Const_u8 canonical;
    File_List file_list;
};

#endif

// BOTTOM

