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
// NOTE(allen): Search List Builders

function void def_search_add_path(Arena *arena, List_String_Const_u8 *list, String_Const_u8 path);
function void def_search_list_add_system_path(Arena *arena, List_String_Const_u8 *list, System_Path_Code path);
function void def_search_normal_load_list(Arena *arena, List_String_Const_u8 *list);

////////////////////////////////
// NOTE(allen): Search List Functions

function FILE *def_search_fopen(Arena *arena, List_String_Const_u8 *list, char *file_name, char *opt);

#endif

// BOTTOM

