/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 03.01.2017
 *
 * Hot_Directory data structure for 4coder
 *
 */

// TOP

internal void
hot_directory_clean_end(Hot_Directory *hot_directory){
    String_Const_u8 str = hot_directory->string;
    if (!character_is_slash(string_get_character(str, str.size - 1))){
        hot_directory->string = string_remove_last_folder(str);
    }
}

internal i32
hot_directory_quick_partition(File_Info **infos, i32 start, i32 pivot){
    File_Info **p = infos + pivot;
    File_Info **a = infos + start;
    for (i32 i = start; i < pivot; ++i, ++a){
        b32 p_folder = (HasFlag((**p).attributes.flags, FileAttribute_IsDirectory));
        b32 a_folder = (HasFlag((**a).attributes.flags, FileAttribute_IsDirectory));
        i32 comp = p_folder - a_folder;
        if (comp == 0){
            comp = string_compare((**a).file_name, (**p).file_name);
        }
        if (comp < 0){
            Swap(File_Info*, *a, infos[start]);
            ++start;
        }
    }
    Swap(File_Info*, *p, infos[start]);
    return(start);
}

internal void
hot_directory_quick_sort(File_Info **infos, i32 start, i32 pivot){
    i32 mid = hot_directory_quick_partition(infos, start, pivot);
    if (start < mid-1) hot_directory_quick_sort(infos, start, mid-1);
    if (mid+1 < pivot) hot_directory_quick_sort(infos, mid+1, pivot);
}

internal void
hot_directory_fixup(Hot_Directory *hot_directory){
    File_List *files = &hot_directory->file_list;
    if (files->count >= 2){
        hot_directory_quick_sort(files->infos, 0, files->count - 1);
    }
}

internal void
hot_directory_set(Hot_Directory *hot_directory, String_Const_u8 str){
    linalloc_clear(&hot_directory->arena);
    hot_directory->string = push_string_copy(&hot_directory->arena, str);
    hot_directory->canonical = system_get_canonical(&hot_directory->arena, str);
    hot_directory->file_list = system_get_file_list(&hot_directory->arena, hot_directory->canonical);
}

internal void
hot_directory_reload(Arena *scratch, Hot_Directory *hot_directory){
    Temp_Memory temp = begin_temp(scratch);
    String_Const_u8 string = push_string_copy(scratch, hot_directory->string);
    hot_directory_set(hot_directory, string);
    end_temp(temp);
}

internal void
hot_directory_init(Arena *scratch, Hot_Directory *hot_directory, String_Const_u8 directory){
    hot_directory->arena = make_arena_system();
    Temp_Memory temp = begin_temp(scratch);
    String_Const_u8 dir = directory;
    if (!character_is_slash(string_get_character(directory, directory.size - 1))){
        dir = push_u8_stringf(scratch, "%.*s/", string_expand(directory));
    }
    hot_directory_set(hot_directory, dir);
    end_temp(temp);
}

// BOTTOM

