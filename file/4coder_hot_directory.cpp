/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 03.01.2017
 *
 * Hot_Directory data structure for 4coder
 *
 */

// TOP

struct Hot_Directory{
    String string;
    File_List file_list;
};

internal void
hot_directory_clean_end(Hot_Directory *hot_directory){
    String *str = &hot_directory->string;
    if (str->size != 0 && str->str[str->size-1] != '/'){
        str->size = reverse_seek_slash(*str) + 1;
        str->str[str->size] = 0;
    }
}

internal i32
hot_directory_quick_partition(File_Info *infos, i32 start, i32 pivot){
    File_Info *p = infos + pivot;
    File_Info *a = infos + start;
    for (i32 i = start; i < pivot; ++i, ++a){
        i32 comp = 0;
        comp = p->folder - a->folder;
        if (comp == 0){
            comp = compare_cc(a->filename, p->filename);
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

inline void
hot_directory_fixup(Hot_Directory *hot_directory){
    File_List *files = &hot_directory->file_list;
    if (files->count >= 2){
        hot_directory_quick_sort(files->infos, 0, files->count - 1);
    }
}

inline void
hot_directory_set(System_Functions *system, Hot_Directory *hot_directory, String str){
    copy_checked_ss(&hot_directory->string, str);
    b32 success = terminate_with_null(&hot_directory->string);
    if (success){
        if (str.size > 0){
            system->set_file_list(&hot_directory->file_list, hot_directory->string.str);
        }
        else{
            system->set_file_list(&hot_directory->file_list, 0);
        }
    }
    hot_directory_fixup(hot_directory);
}

inline void
hot_directory_reload(System_Functions *system, Hot_Directory *hot_directory){
    hot_directory_set(system, hot_directory, hot_directory->string);
}

internal void
hot_directory_init(Hot_Directory *hot_directory, String base, String dir){
    hot_directory->string = base;
    hot_directory->string.str[255] = 0;
    hot_directory->string.size = 0;
    copy_ss(&hot_directory->string, dir);
    if (hot_directory->string.str[hot_directory->string.size-1] != '/'){
        append_s_char(&hot_directory->string, '/');
    }
}

// BOTTOM

