/*
 4coder_file.h - File enumeration and reading procedures for each platform.
 */

// TOP

#if !defined(FCODER_FILE_ENUMERATOR_CPP)
#define FCODER_FILE_ENUMERATOR_CPP

#include "4coder_base_types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if OS_WINDOWS

//// WINDOWS BEGIN ////
#undef function
#define UNICODE
#include <Windows.h>
typedef TCHAR Filename_Character;
#define SLASH '\\'
#define function static
//// WINDOWS END ////

#else
#include <dirent.h>
#include <sys/stat.h>
#define SLASH '/'
typedef char Filename_Character;
#endif

struct Cross_Platform_File_Info{
    Filename_Character *name;
    int32_t len;
    b32 is_folder;
};

struct Cross_Platform_File_List{
    Cross_Platform_File_Info *info;
    int32_t count;
    int32_t path_length;
    Filename_Character path_name[4096];
};

typedef b32 File_Filter(Filename_Character *name, int32_t len);

static Cross_Platform_File_List
get_file_list(Arena *arena, Filename_Character *dir, File_Filter *filter);

static Filename_Character*
encode(Arena *arena, char *str){
    int32_t size = 0;
    for (;str[size]!=0;++size);
    
    Filename_Character *out = push_array(arena, Filename_Character, size + 1);
    
    if (out == 0){
        fprintf(stdout, "fatal error: ran out of memory encoding string to filename\n");
        exit(1);
    }
    
    int32_t j = 0;
    for (int32_t i = 0; i <= size; ++i){
        if (str[i] != '"'){
            out[j++] = str[i];
        }
    }
    
    return(out);
}

static char*
unencode(Arena *arena, Filename_Character *str, int32_t len){
    Temp_Memory temp = begin_temp(arena);
    char *out = push_array(arena, char, len + 1);
    
    if (out == 0){
        fprintf(stdout, "fatal error: ran out of memory unencoding string to filename\n");
        exit(1);
    }
    
    for (int32_t i = 0; i <= len; ++i){
        if (str[i] <= 127){
            out[i] = (char)str[i];
        }
        else{
            out = 0;
            end_temp(temp);
            break;
        }
    }
    
    return(out);
}

static b32
filter_all(Filename_Character *name, int32_t len){
    return(true);
}

static b32
filter_is_code_file(Filename_Character *name, int32_t len){
    b32 is_code = false;
    if (len >= 5){
        Filename_Character *ext = &name[len - 4];
        if (ext[0] == '.' && ext[1] == 'c' && ext[2] == 'p' && ext[3] == 'p'){
            is_code = true;
        }
        else if (ext[0] == '.' && ext[1] == 'h' && ext[2] == 'p' && ext[3] == 'p'){
            is_code = true;
        }
    }
    if (len >= 4){
        Filename_Character *ext = &name[len - 3];
        if (ext[0] == '.' && ext[1] == 'c' && ext[2] == 'c'){
            is_code = true;
        }
    }
    if (len >= 3){
        Filename_Character *ext = &name[len - 2];
        if (ext[0] == '.' && ext[1] == 'h'){
            is_code = true;
        }
        else if (ext[0] == '.' && ext[1] == 'c'){
            is_code = true;
        }
    }
    return(is_code);
}

#if OS_WINDOWS

//// WINDOWS BEGIN ////
static Cross_Platform_File_List
get_file_list(Arena *arena, Filename_Character *pattern, File_Filter *filter){
    if (arena == 0){
        fprintf(stdout, "fatal error: NULL part passed to %s\n", __FUNCTION__);
        exit(1);
    }
    if (pattern == 0){
        fprintf(stdout, "fatal error: NULL pattern passed to %s\n", __FUNCTION__);
        exit(1);
    }
    
    int32_t pattern_length = 0;
    for (; pattern[pattern_length] != 0; ++pattern_length);
    int32_t last_slash = pattern_length;
    for (; last_slash >= 0 && pattern[last_slash] != SLASH; --last_slash);
    if (last_slash < 0){
        fprintf(stdout, "fatal error: invalid file pattern\n");
        exit(1);
    }
    pattern[last_slash] = 0;
    
    HANDLE dir_handle =
        CreateFile(pattern,
                   FILE_LIST_DIRECTORY,
                   FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                   0,
                   OPEN_EXISTING,
                   FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                   0);
    pattern[last_slash] = SLASH;
    
    if (dir_handle == INVALID_HANDLE_VALUE){
        fprintf(stdout, "fatal error: could not open directory handle\n");
        exit(1);
    }
    
    Filename_Character path_name[4096];
    DWORD path_length = GetFinalPathNameByHandle(dir_handle, path_name, sizeof(path_name), 0);
    if (path_length > sizeof(path_name)){
        fprintf(stdout, "fatal error: path name too long for local buffer\n");
        exit(1);
    }
    CloseHandle(dir_handle);
    
    path_length -= 4;
    memmove(path_name, path_name + 4, path_length*sizeof(*path_name));
    path_name[path_length] = 0;
    
    // TODO(allen): // TODO(allen): // TODO(allen): // TODO(allen): // TODO(allen): 
    // TODO(allen): // TODO(allen): // TODO(allen): // TODO(allen): // TODO(allen): 
    // Get this working with the new search by pattern structure!!!
    
    WIN32_FIND_DATA find_data = {};
    HANDLE search = FindFirstFile(pattern, &find_data);
    if (search == INVALID_HANDLE_VALUE){
        fprintf(stdout, "fatal error: could not begin a file search\n");
        exit(1);
    }
    
    int32_t character_count = 0;
    int32_t file_count = 0;
    BOOL more_files = true;
    do{
        Filename_Character *name = &find_data.cFileName[0];
        
        int32_t size = 0;
        for(;name[size];++size);
        
        uint32_t attribs = find_data.dwFileAttributes;
        b32 is_folder = ((attribs & FILE_ATTRIBUTE_DIRECTORY) != 0);
        b32 is_hidden = ((attribs & FILE_ATTRIBUTE_HIDDEN) != 0);
        
        if (!is_hidden){
            if (name[0] != '.' && (is_folder || filter(name, size))){
                ++file_count;
                character_count += size + 1;
            }
        }
        
        more_files = FindNextFile(search, &find_data);
    }while(more_files);
    FindClose(search);
    
    Cross_Platform_File_List list = {};
    Temp_Memory part_reset = begin_temp(arena);
    
    int32_t rounded_char_size = (character_count*sizeof(Filename_Character) + 7)&(~7);
    int32_t memsize = rounded_char_size + file_count*sizeof(Cross_Platform_File_Info);
    void *mem = push_array(arena, u8, memsize);
    if (mem == 0){
        fprintf(stdout, "fatal error: not enough memory on the partition for a file list.\n");
        exit(1);
    }
    
    Filename_Character *char_ptr = (Filename_Character*)mem;
    Cross_Platform_File_Info *info_ptr = (Cross_Platform_File_Info*)((uint8_t*)mem + rounded_char_size);
    
    Filename_Character *char_ptr_end = (Filename_Character*)info_ptr;
    Cross_Platform_File_Info *info_ptr_end = info_ptr + file_count;
    
    Cross_Platform_File_Info *info_ptr_base = info_ptr;
    
    search = FindFirstFile(pattern, &find_data);
    if (search == INVALID_HANDLE_VALUE){
        fprintf(stdout, "fatal error: could not restart a file search\n");
        exit(1);
    }
    
    int32_t adjusted_file_count = 0;
    more_files = true;
    do{
        Filename_Character *name = &find_data.cFileName[0];
        
        int32_t size = 0;
        for(;name[size]!=0;++size);
        
        uint32_t attribs = find_data.dwFileAttributes;
        b32 is_folder = ((attribs & FILE_ATTRIBUTE_DIRECTORY) != 0);
        b32 is_hidden = ((attribs & FILE_ATTRIBUTE_HIDDEN) != 0);
        
        if (!is_hidden){
            if (name[0] != '.' && (is_folder || filter(name, size))){
                if (info_ptr + 1 > info_ptr_end || char_ptr + size + 1 > char_ptr_end){
                    memset(&list, 0, sizeof(list));
                    end_temp(part_reset);
                    FindClose(search);
                    return(list);
                }
                
                info_ptr->name = char_ptr;
                info_ptr->len = size;
                info_ptr->is_folder = is_folder;
                
                memmove(char_ptr, name, size*sizeof(*name));
                char_ptr[size] = 0;
                
                char_ptr += size + 1;
                ++info_ptr;
                ++adjusted_file_count;
            }
        }
        
        more_files = FindNextFile(search, &find_data);
    }while(more_files);
    FindClose(search);
    
    list.info = info_ptr_base;
    list.count = adjusted_file_count;
    list.path_length = path_length;
    memcpy(list.path_name, path_name, list.path_length*sizeof(*path_name));
    list.path_name[list.path_length] = 0;
    
    return(list);
}
//// WINDOWS END ////

#elif OS_LINUX || OS_MAC

//// UNIX BEGIN ////
static b32
match_pattern(Filename_Character *name, Filename_Character *pattern){
    b32 match = false;
    if (sizeof(*name) == 1){
        String_Const_u8 name_str = SCu8(name);
        String_Const_u8 pattern_str = SCu8(pattern);
        List_String_Const_u8 list = {};
        Node_String_Const_u8 node = { NULL, name_str };
        string_list_push(&list, &node);
        match = string_wildcard_match(list, pattern_str, StringMatch_Exact);
    }
    else{
        fprintf(stdout, "fatal error: wide characters not supported!\n");
        exit(1);
    }
    return(match);
}

static Cross_Platform_File_List
get_file_list(Arena *arena, Filename_Character *pattern, File_Filter *filter){
    if (arena == 0){
        fprintf(stdout, "fatal error: NULL arena passed to %s\n", __FUNCTION__);
        exit(1);
    }
    if (pattern == 0){
        fprintf(stdout, "fatal error: NULL dir passed to %s\n", __FUNCTION__);
        exit(1);
    }
    
    int32_t pattern_length = 0;
    for (; pattern[pattern_length] != 0; ++pattern_length);
    int32_t last_slash = pattern_length;
    for (; last_slash > 0 && pattern[last_slash] != SLASH; --last_slash);
    if (last_slash < 0){
        fprintf(stdout, "fatal error: invalid file pattern\n");
        exit(1);
    }
    pattern[last_slash] = 0;
    
    DIR *dir_handle = opendir(pattern);
    
    if (dir_handle == 0){
        fprintf(stdout, "fatal error: could not open directory handle\n");
        if (sizeof(*pattern) == 2){
            fprintf(stdout, "%ls\n", (wchar_t*)pattern);
        }
        else{
            fprintf(stdout, "%s\n", (char*)pattern);
        }
        exit(1);
    }
    
    Filename_Character path_name[4096];
    int32_t path_length = cstring_length(pattern);
    if (path_length + 1 > sizeof(path_name)){
        fprintf(stdout, "fatal error: path name too long for local buffer\n");
        exit(1);
    }
    memcpy(path_name, pattern, path_length + 1);
    
    char *file_pattern = pattern + last_slash + 1;
    
    int32_t character_count = 0;
    int32_t file_count = 0;
    for (struct dirent *entry = readdir(dir_handle);
         entry != 0;
         entry = readdir(dir_handle)){
        Filename_Character *name = entry->d_name;
        if (!match_pattern(name, file_pattern)){
            continue;
        }
        
        int32_t size = 0;
        for(;name[size];++size);
        
        b32 is_folder = false;
        if (entry->d_type == DT_LNK){
            struct stat st;
            if (stat(entry->d_name, &st) != -1){
                is_folder = S_ISDIR(st.st_mode);
            }
        }
        else{
            is_folder = (entry->d_type == DT_DIR);
        }
        
        if (name[0] != '.' && (is_folder || filter(name, size))){
            ++file_count;
            character_count += size + 1;
        }
    }
    
    Cross_Platform_File_List list = {};
    Temp_Memory part_reset = begin_temp(arena);
    
    int32_t rounded_char_size = (character_count*sizeof(Filename_Character) + 7)&(~7);
    int32_t memsize = rounded_char_size + file_count*sizeof(Cross_Platform_File_Info);
    void *mem = push_array(arena, uint8_t, memsize);
    if (mem == 0){
        fprintf(stdout, "fatal error: not enough memory on the partition for a file list.\n");
        exit(1);
    }
    
    Filename_Character *char_ptr = (Filename_Character*)mem;
    Cross_Platform_File_Info *info_ptr = (Cross_Platform_File_Info*)((uint8_t*)mem + rounded_char_size);
    
    Filename_Character *char_ptr_end = (Filename_Character*)info_ptr;
    Cross_Platform_File_Info *info_ptr_end = info_ptr + file_count;
    
    Cross_Platform_File_Info *info_ptr_base = info_ptr;
    
    rewinddir(dir_handle);
    
    int32_t adjusted_file_count = 0;
    for (struct dirent *entry = readdir(dir_handle);
         entry != 0;
         entry = readdir(dir_handle)){
        Filename_Character *name = entry->d_name;
        if (!match_pattern(name, file_pattern)){
            continue;
        }
        
        int32_t size = 0;
        for(;name[size];++size);
        
        b32 is_folder = false;
        if (entry->d_type == DT_LNK){
            struct stat st;
            if (stat(entry->d_name, &st) != -1){
                is_folder = S_ISDIR(st.st_mode);
            }
        }
        else{
            is_folder = (entry->d_type == DT_DIR);
        }
        
        if (name[0] != '.' && (is_folder || filter(name, size))){
            if (info_ptr + 1 > info_ptr_end || char_ptr + size + 1 > char_ptr_end){
                memset(&list, 0, sizeof(list));
                end_temp(part_reset);
                closedir(dir_handle);
                return(list);
            }
            
            info_ptr->name = char_ptr;
            info_ptr->len = size;
            info_ptr->is_folder = is_folder;
            
            memmove(char_ptr, name, size*sizeof(*name));
            char_ptr[size] = 0;
            
            char_ptr += size + 1;
            ++info_ptr;
            ++adjusted_file_count;
        }
    }
    closedir(dir_handle);
    
    list.info = info_ptr_base;
    list.count = adjusted_file_count;
    list.path_length = path_length;
    memcpy(list.path_name, path_name, list.path_length*sizeof(*path_name));
    list.path_name[list.path_length] = 0;
    
    return(list);
}
//// UNIX END ////

#else
# error metdata generator not supported on this platform
#endif

static String_Const_u8
file_dump(Arena *arena, char *name){
    String_Const_u8 text = {};
    FILE *file = fopen(name, "rb");
    if (file != 0){
        fseek(file, 0, SEEK_END);
        text.size = ftell(file);
        fseek(file, 0, SEEK_SET);
        text.str = push_array(arena, u8, text.size + 1);
        if (text.str == 0){
            fprintf(stdout, "fatal error: not enough memory in partition for file dumping");
            exit(1);
        }
        fread(text.str, 1, (size_t)text.size, file);
        text.str[text.size] = 0;
        fclose(file);
    }
    return(text);
}

#endif

// BOTTOM

