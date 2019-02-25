/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 18.07.2017
 *
 * General win32 functions
 *
 */

// TOP

//
// Shared system functions (system_shared.h)
//

internal
Sys_File_Can_Be_Made_Sig(system_file_can_be_made){
    HANDLE file = CreateFile_utf8(&shared_vars.scratch, filename, FILE_APPEND_DATA, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    b32 result = false;
    if (file != INVALID_HANDLE_VALUE){
        CloseHandle(file);
        result = true;
    }
    return(result);
}

//
// Memory
//

internal void*
system_memory_allocate_extended(void *base, umem size){
    void *result = VirtualAlloc(base, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    return(result);
}

internal
Sys_Memory_Allocate_Sig(system_memory_allocate){
    return(system_memory_allocate_extended(0, size));
}

internal
Sys_Memory_Set_Protection_Sig(system_memory_set_protection){
    bool32 result = false;
    DWORD old_protect = 0;
    DWORD protect = 0;
    
    switch (flags & 0x7){
        case 0:                                                   protect = PAGE_NOACCESS; break;
        case MemProtect_Read:                                     protect = PAGE_READONLY; break;
        case MemProtect_Write:                                    /* below */
        case MemProtect_Write|MemProtect_Read:                    protect = PAGE_READWRITE; break;
        case MemProtect_Execute:                                  protect = PAGE_EXECUTE; break;
        case MemProtect_Execute|MemProtect_Read:                  protect = PAGE_EXECUTE_READ; break;
        case MemProtect_Execute|MemProtect_Write:                 /* below */
        case MemProtect_Execute|MemProtect_Write|MemProtect_Read: protect = PAGE_EXECUTE_READWRITE; break;
    }
    
    VirtualProtect(ptr, size, protect, &old_protect);
    return(result);
}

internal
Sys_Memory_Free_Sig(system_memory_free){
    VirtualFree(ptr, 0, MEM_RELEASE);
}

//
// 4ed path
//

internal
Sys_Get_Current_Path_Sig(system_get_current_path){
    i32 result = GetCurrentDirectory_utf8(&shared_vars.scratch, capacity, (u8*)out);
    return(result);
}

internal
Sys_Get_4ed_Path_Sig(system_get_4ed_path){
    local_persist b32 has_stashed_4ed_path = false;
    if (!has_stashed_4ed_path){
        has_stashed_4ed_path = true;
        local_const i32 binary_path_capacity = KB(32);
        u8 *memory = (u8*)system_memory_allocate(binary_path_capacity);
        i32 size = GetModuleFileName_utf8(&shared_vars.scratch, 0, memory, binary_path_capacity);
        Assert(size <= binary_path_capacity - 1);
        win32vars.binary_path = make_string(memory, size);
        remove_last_folder(&win32vars.binary_path);
        terminate_with_null(&win32vars.binary_path);
    }
    i32 copy_size = Min(win32vars.binary_path.size, capacity);
    block_copy(out, win32vars.binary_path.str, copy_size);
    return(win32vars.binary_path.size);
}

//
// Logging
//

internal
Sys_Log_Sig(system_log){
    if (plat_settings.use_log){
        u8 space[4096];
        String str = make_fixed_width_string(space);
        str.size = system_get_4ed_path(str.str, str.memory_size);
        append_sc(&str, "4coder_log.txt");
        terminate_with_null(&str);
        HANDLE file = CreateFile_utf8(&shared_vars.scratch, space, GENERIC_WRITE, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
        if (file != INVALID_HANDLE_VALUE){
            SetFilePointer(file, win32vars.log_position, 0, FILE_BEGIN);
            win32vars.log_position += length;
            DWORD written = 0;
            DWORD total_written = 0;
            do{
                WriteFile(file, message + total_written, length - total_written, &written, 0);
                total_written += written;
            }while (total_written < length);
            CloseHandle(file);
        }
    }
}

//
// Files
//

internal String
win32_remove_unc_prefix_characters(String path){
    if (match_part(path, make_lit_string("\\\\?\\UNC"))){
        path.size -= 6;
        memmove(path.str, path.str + 6, path.size);
        path.str[0] = '\\';
    }
    else if (match_part(path, make_lit_string("\\\\?\\"))){
        path.size -= 4;
        memmove(path.str, path.str + 4, path.size);
    }
    return(path);
}

internal
Sys_Set_File_List_Sig(system_set_file_list){
    b32 clear_list = true;
    if (directory != 0){
        char dir_space[MAX_PATH + 32];
        String dir = make_string_cap(dir_space, 0, MAX_PATH + 32);
        append_sc(&dir, directory);
        terminate_with_null(&dir);
        
        HANDLE dir_handle = CreateFile_utf8(&shared_vars.scratch, (u8*)dir.str, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, 0);
        
        if (dir_handle != INVALID_HANDLE_VALUE){
            DWORD final_length = GetFinalPathNameByHandle_utf8(&shared_vars.scratch, dir_handle, (u8*)dir_space, sizeof(dir_space), 0);
            CloseHandle(dir_handle);
            
            if (final_length + 3 < sizeof(dir_space)){
                u8 *c_str_dir = (u8*)dir_space;
                if (c_str_dir[final_length - 1] == 0){
                    --final_length;
                }
                String str_dir = make_string(c_str_dir, final_length);
                String adjusted_str_dir = win32_remove_unc_prefix_characters(str_dir);
                
                c_str_dir = (u8*)adjusted_str_dir.str;
                final_length = adjusted_str_dir.size;
                c_str_dir[final_length] = '\\';
                c_str_dir[final_length + 1] = '*';
                c_str_dir[final_length + 2] = 0;
                
                if (canon_directory_out != 0){
                    if (final_length+1 < canon_directory_max){
                        memcpy(canon_directory_out, c_str_dir, final_length);
                        if (canon_directory_out[final_length-1] != '\\'){
                            canon_directory_out[final_length++] = '\\';
                        }
                        canon_directory_out[final_length] = 0;
                        *canon_directory_size_out = final_length;
                    }
                    else{
                        u32 length = copy_fast_unsafe_cc(canon_directory_out, directory);
                        canon_directory_out[length] = 0;
                        *canon_directory_size_out = length;
                    }
                }
                
                WIN32_FIND_DATA find_data;
                HANDLE search = FindFirstFile_utf8(&shared_vars.scratch, c_str_dir, &find_data);
                
                if (search != INVALID_HANDLE_VALUE){            
                    u32 character_count = 0;
                    u32 file_count = 0;
                    BOOL more_files = true;
                    do{
                        b32 nav_dir =
                            (find_data.cFileName[0] == '.' && find_data.cFileName[1] == 0) ||(find_data.cFileName[0] == '.' && find_data.cFileName[1] == '.' && find_data.cFileName[2] == 0);
                        if (!nav_dir){
                            ++file_count;
                            u32 size = 0;
                            for(;find_data.cFileName[size];++size);
                            character_count += size + 1;
                        }
                        more_files = FindNextFile(search, &find_data);
                    }while(more_files);
                    FindClose(search);
                    
                    u32 remaining_size = character_count*2;
                    u32 required_size = remaining_size + file_count*sizeof(File_Info);
                    if (file_list->block_size < required_size){
                        system_memory_free(file_list->block, 0);
                        file_list->block = system_memory_allocate(required_size);
                        file_list->block_size = required_size;
                    }
                    
                    file_list->infos = (File_Info*)file_list->block;
                    u8 *name = (u8*)(file_list->infos + file_count);
                    u32 corrected_file_count = 0;
                    if (file_list->block != 0){
                        search = FindFirstFile_utf8(&shared_vars.scratch, c_str_dir, &find_data);
                        
                        if (search != INVALID_HANDLE_VALUE){
                            File_Info *info = file_list->infos;
                            more_files = true;
                            do{
                                b32 nav_dir =
                                    (find_data.cFileName[0] == '.' && find_data.cFileName[1] == 0) ||(find_data.cFileName[0] == '.' && find_data.cFileName[1] == '.' && find_data.cFileName[2] == 0);
                                
                                if (!nav_dir){
                                    u32 attribs = find_data.dwFileAttributes;
                                    info->folder = (attribs & FILE_ATTRIBUTE_DIRECTORY) != 0;
                                    info->filename = (char*)name;
                                    
                                    u16 *src = (u16*)find_data.cFileName;
                                    u32 src_len = 0;
                                    for (;src[src_len];++src_len);
                                    
                                    u8 *dst = name;
                                    u32 max = remaining_size-1;
                                    
                                    b32 error = false;
                                    u32 length = (u32)utf16_to_utf8_minimal_checking(dst, max, src, src_len, &error);
                                    
                                    if (length <= max && !error){
                                        name += length;
                                        
                                        info->filename_len = length;
                                        *name++ = 0;
                                        String fname = make_string_cap(info->filename, length, length+1);
                                        replace_char(&fname, '\\', '/');
                                        ++info;
                                        ++corrected_file_count;
                                    }
                                }
                                more_files = FindNextFile(search, &find_data);
                            }while(more_files);
                            FindClose(search);
                            
                            file_list->count = corrected_file_count;
                            clear_list = false;
                        }
                    }
                }
            }
        }
    }
    
    if (clear_list){
        system_memory_free(file_list->block, 0);
        file_list->block = 0;
        file_list->block_size = 0;
        file_list->infos = 0;
        file_list->count = 0;
    }
}

internal
Sys_Get_Canonical_Sig(system_get_canonical){
    u32 result = 0;
    
    char src_space[MAX_PATH + 32];
    if (len < sizeof(src_space) &&
        (len >= 2 &&
         ((filename[0] >= 'a' && filename[0] <= 'z') ||
          (filename[0] >= 'A' && filename[0] <= 'Z')) && filename[1] == ':') ||
        (filename[0] == '\\' && filename[1] == '\\')){
        memcpy(src_space, filename, len);
        src_space[len] = 0;
        
        HANDLE file = CreateFile_utf8(&shared_vars.scratch, (u8*)src_space, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        
        if (file != INVALID_HANDLE_VALUE){
            DWORD final_length = GetFinalPathNameByHandle_utf8(&shared_vars.scratch, file, (u8*)buffer, max, 0);
            
            if (final_length + 3 < max){
                if (buffer[final_length - 1] == 0){
                    --final_length;
                }
                String str_dir = make_string(buffer, final_length);
                String adjusted_str_dir = win32_remove_unc_prefix_characters(str_dir);
                buffer = adjusted_str_dir.str;
                final_length = adjusted_str_dir.size;
                buffer[final_length] = 0;
                result = final_length;
            }
            
            CloseHandle(file);
        }
        else{
            String src_str = make_string(filename, len);
            String path_str = path_of_directory(src_str);
            String front_str = front_of_directory(src_str);
            
            memcpy(src_space, path_str.str, path_str.size);
            src_space[path_str.size] = 0;
            
            HANDLE dir = CreateFile_utf8(&shared_vars.scratch, (u8*)src_space, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, 0);
            
            if (dir != INVALID_HANDLE_VALUE){
                DWORD final_length = GetFinalPathNameByHandle_utf8(&shared_vars.scratch, dir, (u8*)buffer, max, 0);
                
                if (final_length + 3 < max){
                    if (buffer[final_length-1] == 0){
                        --final_length;
                    }
                    String str_dir = make_string(buffer, final_length);
                    String adjusted_str_dir = win32_remove_unc_prefix_characters(str_dir);
                    buffer = adjusted_str_dir.str;
                    final_length = adjusted_str_dir.size;
                    buffer[final_length++] = '\\';
                    memcpy(buffer + final_length, front_str.str, front_str.size);
                    final_length += front_str.size;
                    buffer[final_length] = 0;
                    result = final_length;
                }
                
                CloseHandle(dir);
            }
        }
    }
    
    return(result);
}

internal File_Attributes
win32_file_attributes_from_HANDLE(HANDLE file){
    BY_HANDLE_FILE_INFORMATION info = {};
    GetFileInformationByHandle(file, &info);
    File_Attributes result = {};
    result.size = ((u64)info.nFileSizeHigh << 32LL) | ((u64)info.nFileSizeLow);
    result.last_write_time = ((u64)info.ftLastWriteTime.dwHighDateTime << 32LL) | ((u64)info.ftLastWriteTime.dwLowDateTime);
    return(result);
}

internal
Sys_Quick_File_Attributes_Sig(system_quick_file_attributes){
    WIN32_FILE_ATTRIBUTE_DATA info = {};
    File_Attributes result = {};
    if (GetFileAttributesEx_utf8String(&shared_vars.scratch, file_name, GetFileExInfoStandard, &info)){
        result.size = ((u64)info.nFileSizeHigh << 32LL) | ((u64)info.nFileSizeLow);
        result.last_write_time = ((u64)info.ftLastWriteTime.dwHighDateTime << 32LL) | ((u64)info.ftLastWriteTime.dwLowDateTime);
    }
    return(result);
}

internal
Sys_Load_Handle_Sig(system_load_handle){
    b32 result = false;
    HANDLE file = CreateFile_utf8(&shared_vars.scratch, (u8*)filename, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (file != INVALID_HANDLE_VALUE){
        *(HANDLE*)handle_out = file;
        result = true;
    }
    return(result);
}

internal
Sys_Load_Attributes_Sig(system_load_attributes){
    HANDLE file = *(HANDLE*)(&handle);
    return(win32_file_attributes_from_HANDLE(file));
}

internal
Sys_Load_File_Sig(system_load_file){
    HANDLE file = *(HANDLE*)(&handle);
    DWORD read_size = 0;
    b32 result = false;
    if (ReadFile(file, buffer, size, &read_size, 0)){
        if (read_size == size){
            result = true;
        }
    }
    return(result);
}

internal
Sys_Load_Close_Sig(system_load_close){
    b32 result = false;
    HANDLE file = *(HANDLE*)(&handle);
    if (CloseHandle(file)){
        result = true;
    }
    return(result);
}

internal
Sys_Save_File_Sig(system_save_file){
    File_Attributes result = {};
    HANDLE file = CreateFile_utf8(&shared_vars.scratch, (u8*)filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    
    if (file != INVALID_HANDLE_VALUE){
        DWORD written_total = 0;
        DWORD written_size = 0;
        
        b32 success = true;
        for (;written_total < size;){
            if (!WriteFile(file, buffer + written_total, size - written_total, &written_size, 0)){
                success = false;
                break;
            }
            written_total += written_size;
        }
        
        if (success){
            result = win32_file_attributes_from_HANDLE(file);
        }
        
        CloseHandle(file);
    }
    
    return(result);
}

//
// File System
//

internal b32
system_directory_exists(char *path){
    DWORD attrib = GetFileAttributes_utf8(&shared_vars.scratch, (u8*)path);
    return(attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY));
}

//
// Color picker
//

internal int_color
swap_r_and_b(int_color a){
    int_color result = a & 0xff00ff00;
    result |= ((a >> 16) & 0xff);
    result |= ((a & 0xff) << 16);
    return(result);
}

internal int_color
int_color_from_colorref(COLORREF ref, int_color alpha_from){
    int_color rgb = swap_r_and_b(ref & 0xffffff);
    
    int_color result = ((0xff000000 & alpha_from) | rgb);
    return(result);
}

internal void
system_schedule_step();

internal UINT_PTR CALLBACK
color_picker_hook(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam){
    UINT_PTR result = 0;
    switch(Message)
    {
        case WM_INITDIALOG:
        {
            CHOOSECOLORW *win32_params = (CHOOSECOLORW *)LParam;
            color_picker *picker = (color_picker *)win32_params->lCustData;
            SetWindowLongPtr(Window, GWLP_USERDATA, (LONG_PTR)LParam);
            
            u16_4tech Temp[256];
            Temp[ArrayCount(Temp) - 1] = 0;
            
            b32_4tech ignored;
            utf8_to_utf16_minimal_checking(Temp, ArrayCount(Temp), (u8_4tech *)picker->title.str, picker->title.size, &ignored);
            if(picker->title.size < ArrayCount(Temp))
            {
                Temp[picker->title.size] = 0;
            }
            
            SetWindowTextW(Window, (LPCWSTR)Temp);
        } break;
        
        case WM_CTLCOLORSTATIC:
        {
            // NOTE(casey): I can't believe I'm 42 years old and I still have to do this fucking crap.
            // Microsoft is so fucking fired every god damn day.  Would it have killed you to update rgbResult
            // continuously, or at least provide a GetCurrentColor() call???
            //
            // Anyway, since the color picker doesn't tell us when the color is changed, what we do is watch for messages
            // that repaint the color swatch, which is dialog id 0x2c5, and then we sample it to see what color it is.
            // No, I'm not fucking kidding, that's what we do.
            HWND swatch_window = (HWND)LParam;
            if(GetDlgCtrlID(swatch_window) == 0x2c5)
            {
                CHOOSECOLORW *win32_params = (CHOOSECOLORW *)GetWindowLongPtr(Window, GWLP_USERDATA);
                if(win32_params)
                {
                    color_picker *picker = (color_picker *)win32_params->lCustData;
                    
                    RECT rect;
                    GetClientRect(swatch_window, &rect);
                    HDC swatch_dc = (HDC)WParam;
                    COLORREF Probe = GetPixel(swatch_dc, (rect.left + rect.right) / 4, (rect.top + rect.bottom) / 2);
                    int_color new_color = int_color_from_colorref(Probe, *picker->dest);
                    
                    if(*picker->dest != new_color)
                    {
                        *picker->dest = new_color;
                        system_schedule_step();
                    }
                }
            }
        } break;
        
        default:
        {
#if 0
            // NOTE(casey): Enable this if you want to dump the color edit dialog messages to the debug log
            short Temp[256];
            wsprintf((LPWSTR)Temp, L"%u 0x%x 0x%x\n", Message, WParam, LParam);
            OutputDebugStringW((LPWSTR)Temp);
#endif
        } break;
    }
    
    return(result);
}

internal DWORD WINAPI
color_picker_thread(LPVOID Param)
{
    color_picker *picker = (color_picker *)Param;
    
    int_color color = 0;
    if(picker->dest)
    {
        color = *picker->dest;
    }
    
    COLORREF custom_colors[16] = {};
    
    CHOOSECOLORW win32_params = {};
    win32_params.lStructSize = sizeof(win32_params);
    //win32_params.hwndOwner = win32vars.window_handle;
    win32_params.hInstance = win32vars.window_handle;
    win32_params.rgbResult = swap_r_and_b(color) & 0xffffff;
    win32_params.lpCustColors = custom_colors;
    win32_params.Flags = CC_RGBINIT | CC_FULLOPEN | CC_ANYCOLOR | CC_ENABLEHOOK;
    win32_params.lCustData = (LPARAM)picker;
    win32_params.lpfnHook = color_picker_hook;
    
    if(ChooseColorW(&win32_params))
    {
        color = int_color_from_colorref(win32_params.rgbResult, color);
    }
    
    if(picker->dest)
    {
        *picker->dest = color;
    }
    
    if(picker->finished)
    {
        *picker->finished = true;
    }
    
    system_memory_free(picker, sizeof(*picker));
    
    return(0);
}

internal
Sys_Open_Color_Picker_Sig(system_open_color_picker){
    
    // NOTE(casey): Because this is going to be used by a semi-permanent thread, we need to copy
    // it to system memory where it can live as long as it wants, no matter what we do over here
    // on the 4coder threads.
    color_picker *perm = (color_picker *)system_memory_allocate_extended(0, sizeof(color_picker));
    *perm = *picker;
    
    HANDLE ThreadHandle = CreateThread(0, 0, color_picker_thread, perm, 0, 0);
    CloseHandle(ThreadHandle);
}

internal
Sys_Animate_Sig(system_animate){
    system_schedule_step();
};

// BOTTOM

