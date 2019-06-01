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
    String_Const_u8 str = SCu8(hot_directory->string_space, hot_directory->string_size);
    if (!character_is_slash(string_get_character(str, str.size - 1))){
        str = string_remove_last_folder(str);
        str.str[str.size] = 0;
    }
}

internal i32
hot_directory_quick_partition(File_Info *infos, i32 start, i32 pivot){
    File_Info *p = infos + pivot;
    File_Info *a = infos + start;
    for (i32 i = start; i < pivot; ++i, ++a){
        i32 comp = p->folder - a->folder;
        if (comp == 0){
            comp = string_compare(SCu8(a->filename, a->filename_len),
                                  SCu8(p->filename, p->filename_len));
        }
        if (comp < 0){
            Swap(File_Info, *a, infos[start]);
            ++start;
        }
    }
    Swap(File_Info, *p, infos[start]);
    return start;
}

internal void
hot_directory_quick_sort(File_Info *infos, i32 start, i32 pivot){
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
hot_directory_set(System_Functions *system, Hot_Directory *hot_directory, String_Const_u8 str){
    b32 success = (str.size < sizeof(hot_directory->string_space));
    if (success){
        block_copy(hot_directory->string_space, str.str, str.size);
        hot_directory->string_space[str.size] = 0;
        hot_directory->string_size = str.size;
        if (str.size > 0){
            u32 canon_max = sizeof(hot_directory->canon_dir_space);
            u8 *canon_str = hot_directory->canon_dir_space;
            u32 canon_length = 0;
            system->set_file_list(&hot_directory->file_list, (char*)hot_directory->string_space, (char*)canon_str, &canon_length, canon_max);
            if (canon_length > 0){
                hot_directory->canon_dir_size = canon_length;
                if (!character_is_slash(hot_directory->canon_dir_space[canon_length - 1])){
                    hot_directory->canon_dir_space[hot_directory->canon_dir_size] = '/';
                    hot_directory->canon_dir_size += 1;
                    hot_directory->canon_dir_space[hot_directory->canon_dir_size] = 0;
                }
            }
            else{
                hot_directory->canon_dir_size = 0;
            }
        }
        else{
            system->set_file_list(&hot_directory->file_list, 0, 0, 0, 0);
            hot_directory->canon_dir_size = 0;
        }
    }
    hot_directory_fixup(hot_directory);
}

internal void
hot_directory_reload(System_Functions *system, Hot_Directory *hot_directory){
    String_Const_u8 string = SCu8(hot_directory->string_space, hot_directory->string_size);
    hot_directory_set(system, hot_directory, string);
}

internal void
hot_directory_init(Hot_Directory *hot_directory, String_Const_u8 dir){
    hot_directory->string_size = 0;
    umem size = clamp_top(dir.size, sizeof(hot_directory->string_space));
    block_copy(hot_directory->string_space, dir.str, size);
    if (dir.size < sizeof(hot_directory->string_space)){
        if (hot_directory->string_space[size - 1] != '/'){
            hot_directory->string_space[size] = '/';
            size = clamp_top(size + 1, sizeof(hot_directory->string_space));
            hot_directory->string_space[size] = 0;
            hot_directory->string_size = size;
        }
    }
}

// BOTTOM

