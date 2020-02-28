/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 01.10.2019
 *
 * Search list helper.
 *
 */

// TOP

function void
search_list_add_path__inner(Arena *arena, Path_Search_List *list, String_Const_u8 path){
    string_list_push(arena, &list->list, path);
    list->max_member_length = Max(list->max_member_length, path.size);
}

function void
search_list_add_path(Arena *arena, Path_Search_List *list, String_Const_u8 path){
    search_list_add_path__inner(arena, list, push_string_copy(arena, path));
}

function void
search_list_add_system_path(Arena *arena, Path_Search_List *list, System_Path_Code path){
    search_list_add_path__inner(arena, list, system_get_path(arena, path));
}

function String_Const_u8
get_full_path(Arena *arena, Path_Search_List *search_list, String_Const_u8 relative){
    String_Const_u8 result = {};
    Temp_Memory restore_point = begin_temp(arena);
    u64 buffer_cap = search_list->max_member_length + relative.size + 1;
    u8 *buffer = push_array(arena, u8, buffer_cap);
    u8 *opl = buffer + buffer_cap;
    u8 *relative_base = opl - 1 - relative.size;
    block_copy(relative_base, relative.str, relative.size);
    relative_base[relative.size] = 0;
    for (Node_String_Const_u8 *node = search_list->list.first;
         node != 0;
         node = node->next){
        u64 node_size = node->string.size;
        u8 *path_base = relative_base - node_size;
        block_copy(path_base, node->string.str, node_size);
        String_Const_u8 name = SCu8(path_base, opl);
        printf("get_full_path: trying %.*s\n", string_expand(name));
        File_Attributes attribs = system_quick_file_attributes(arena, name);
        if (attribs.size > 0){
            result = name;
            printf("hit\n");
            break;
        }
    }
    if (result.size == 0){
        end_temp(restore_point);
    }
    return(result);
}

// BOTTOM

