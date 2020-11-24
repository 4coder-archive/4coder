/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 01.10.2019
 *
 * Search list helper.
 *
 */

// TOP

////////////////////////////////
// NOTE(allen): Search List Builders

function void
def_search_list_add_path(Arena *arena, List_String_Const_u8 *list, String_Const_u8 path){
    String_Const_u8 path_copy = push_string_copy(arena, path);
    string_list_push(arena, list, path_copy);
}

function void
def_search_list_add_system_path(Arena *arena, List_String_Const_u8 *list, System_Path_Code path){
    String_Const_u8 path_string = system_get_path(arena, path);
    string_list_push(arena, list, path_string);
}

function void
def_search_normal_load_list(Arena *arena, List_String_Const_u8 *list){
    if (def_search_project_path.size > 0){
        def_search_list_add_path(arena, list, def_search_project_path);
    }
    def_search_list_add_system_path(arena, list, SystemPath_Binary);
}

////////////////////////////////
// NOTE(allen): Search List Functions

function String_Const_u8
def_get_full_path(Arena *arena, List_String_Const_u8 *list, String_Const_u8 relative){
    String_Const_u8 result = {};
    
    Temp_Memory temp = begin_temp(arena);
    
    u8 slash = '/';
    
    for (Node_String_Const_u8 *node = list->first;
         node != 0;
         node = node->next){
        String_Const_u8 full_name = {};
        full_name.size = node->string.size + 1 + relative.size;
        full_name.str = push_array(arena, u8, full_name.size + 1);
        block_copy(full_name.str, node->string.str, node->string.size);
        full_name.str[node->string.size] = slash;
        block_copy(full_name.str + node->string.size + 1, relative.str, relative.size);
        full_name.str[full_name.size] = 0;
        
        File_Attributes attribs = system_quick_file_attributes(arena, full_name);
        if (attribs.last_write_time > 0){
            result = full_name;
            break;
        }
        
        end_temp(temp);
    }
    
    return(result);
}

function FILE*
def_search_fopen(Arena *arena, List_String_Const_u8 *list, char *file_name, char *opt){
    Temp_Memory_Block block(arena);
    String_Const_u8 full_path = def_get_full_path(arena, list, SCu8(file_name));
    FILE *file = 0;
    if (full_path.size > 0){
        file = fopen((char*)full_path.str, opt);
    }
    return(file);
}

function FILE*
def_search_normal_fopen(Arena *arena, char *file_name, char *opt){
    Temp_Memory_Block block(arena);
    List_String_Const_u8 list = {};
    def_search_normal_load_list(arena, &list);
    FILE *file = def_search_fopen(arena, &list, file_name, opt);
    return(file);
}

// BOTTOM

