/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 01.10.2019
 *
 * Search list helper.
 *
 */

// TOP

#if !defined(FRED_SEARCH_LIST_H)
#define FRED_SEARCH_LIST_H

////////////////////////////////
// NOTE(allen): Search List Functions

function void def_search_list_add_path(Arena *arena, List_String_Const_u8 *list, String_Const_u8 path);
function void def_search_list_add_system_path(Arena *arena, List_String_Const_u8 *list, System_Path_Code path);

function String_Const_u8 def_get_full_path(Arena *arena, List_String_Const_u8 *list, String_Const_u8 file_name);

#endif

// BOTTOM

