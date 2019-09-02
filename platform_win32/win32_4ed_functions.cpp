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
    HANDLE file = CreateFile_utf8(scratch, filename, FILE_APPEND_DATA, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
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
    b32 result = false;
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
    DWORD size = GetCurrentDirectory_utf8(arena, 0, 0);
    u8 *out = push_array(arena, u8, size);
    GetCurrentDirectory_utf8(arena, size, out);
    return(SCu8(out, size - 1));
}

internal
Sys_Get_4ed_Path_Sig(system_get_4ed_path){
    local_persist b32 has_stashed_4ed_path = false;
    if (!has_stashed_4ed_path){
        has_stashed_4ed_path = true;
        local_const i32 binary_path_capacity = KB(32);
        u8 *memory = (u8*)system_memory_allocate(binary_path_capacity);
        i32 size = GetModuleFileName_utf8(arena, 0, memory, binary_path_capacity);
        Assert(size <= binary_path_capacity - 1);
        win32vars.binary_path = SCu8(memory, size);
        win32vars.binary_path = string_remove_last_folder(win32vars.binary_path);
        win32vars.binary_path.str[win32vars.binary_path.size] = 0;
    }
    return(push_string_copy(arena, win32vars.binary_path));
}

//
// Files
//

internal String_Const_u8
win32_remove_unc_prefix_characters(String_Const_u8 path){
    if (string_match(string_prefix(path, 7), string_u8_litexpr("\\\\?\\UNC"))){
#if 0
        // TODO(allen): Why no just do
        path = string_skip(path, 7);
        path.str[0] = '\\';
        // ?
#endif
        path.size -= 7;
        memmove(path.str, path.str + 7, path.size);
        path.str[0] = '\\';
    }
    else if (string_match(string_prefix(path, 4), string_u8_litexpr("\\\\?\\"))){
        // TODO(allen): Same questions essentially.
        path.size -= 4;
        memmove(path.str, path.str + 4, path.size);
    }
    return(path);
}

internal
Sys_Get_Canonical_Sig(system_get_canonical){
    String_Const_u8 result = {};
    if ((character_is_alpha(string_get_character(name, 0)) &&
         string_get_character(name, 1) == ':') ||
        string_match(string_prefix(name, 2), string_u8_litexpr("\\\\"))){
        
        u8 *c_name = push_array(arena, u8, name.size + 1);
        block_copy(c_name, name.str, name.size);
        c_name[name.size] = 0;
        HANDLE file = CreateFile_utf8(arena, c_name, GENERIC_READ, 0, 0, OPEN_EXISTING,
                                      FILE_ATTRIBUTE_NORMAL, 0);
        
        if (file != INVALID_HANDLE_VALUE){
            DWORD capacity = GetFinalPathNameByHandle_utf8(arena, file, 0, 0, 0);
            u8 *buffer = push_array(arena, u8, capacity);
            DWORD length = GetFinalPathNameByHandle_utf8(arena, file, buffer, capacity, 0);
            if (length > 0 && buffer[length - 1] == 0){
                length -= 1;
            }
            result = SCu8(buffer, length);
            result = win32_remove_unc_prefix_characters(result);
            CloseHandle(file);
        }
        else{
            String_Const_u8 path = string_remove_front_of_path(name);
            String_Const_u8 front = string_front_of_path(name);
            
            u8 *c_path = push_array(arena, u8, path.size + 1);
            block_copy(c_path, path.str, path.size);
            c_path[path.size] = 0;
            
            HANDLE dir = CreateFile_utf8(arena, c_path, FILE_LIST_DIRECTORY,
                                         FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0,
                                         OPEN_EXISTING,
                                         FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, 0);
            
            if (dir != INVALID_HANDLE_VALUE){
                DWORD capacity = GetFinalPathNameByHandle_utf8(arena, dir, 0, 0, 0);
                u8 *buffer = push_array(arena, u8, capacity + front.size + 1);
                DWORD length = GetFinalPathNameByHandle_utf8(arena, dir, buffer, capacity, 0);
                if (length > 0 && buffer[length - 1] == 0){
                    length -= 1;
                }
                buffer[length] = '\\';
                length += 1;
                block_copy(buffer + length, front.str, front.size);
                length += (DWORD)front.size;
                result = SCu8(buffer, length);
                result = win32_remove_unc_prefix_characters(result);
                CloseHandle(dir);
            }
        }
    }
    return(result);
}

internal File_Attribute_Flag
win32_convert_file_attribute_flags(DWORD dwFileAttributes){
    File_Attribute_Flag result = {};
    MovFlag(dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY, result, FileAttribute_IsDirectory);
    return(result);
}

internal u64
win32_u64_from_u32_u32(u32 hi, u32 lo){
    return( (((u64)hi) << 32) | ((u64)lo) );
}

internal u64
win32_u64_from_filetime(FILETIME time){
    return(win32_u64_from_u32_u32(time.dwHighDateTime, time.dwLowDateTime));
}

internal File_Attributes
win32_file_attributes_from_HANDLE(HANDLE file){
    BY_HANDLE_FILE_INFORMATION info = {};
    GetFileInformationByHandle(file, &info);
    File_Attributes result = {};
    result.size = win32_u64_from_u32_u32(info.nFileSizeHigh, info.nFileSizeLow);
    result.last_write_time = win32_u64_from_filetime(info.ftLastWriteTime);
    result.flags = win32_convert_file_attribute_flags(info.dwFileAttributes);
    return(result);
}

internal
Sys_Get_File_List_Sig(system_get_file_list){
    File_List result = {};
    String_Const_u8 search_pattern = {};
    if (character_is_slash(string_get_character(directory, directory.size - 1))){
        search_pattern = push_u8_stringf(arena, "%.*s*", string_expand(directory));
    }
    else{
        search_pattern = push_u8_stringf(arena, "%.*s\\*", string_expand(directory));
    }
    
    WIN32_FIND_DATA find_data = {};
    HANDLE search = FindFirstFile_utf8(arena, search_pattern.str, &find_data);
    if (search != INVALID_HANDLE_VALUE){
        File_Info *first = 0;
        File_Info *last = 0;
        i32 count = 0;
        
        for (;;){
            String_Const_u16 file_name_utf16 = SCu16(find_data.cFileName);
            if (!(string_match(file_name_utf16, string_u16_litexpr(L".")) ||
                  string_match(file_name_utf16, string_u16_litexpr(L"..")))){
                String_Const_u8 file_name = string_u8_from_string_u16(arena, file_name_utf16,
                                                                      StringFill_NullTerminate).string;
                
                File_Info *info = push_array(arena, File_Info, 1);
                sll_queue_push(first, last, info);
                count += 1;
                
                info->file_name = file_name;
                info->attributes.size = win32_u64_from_u32_u32(find_data.nFileSizeHigh,
                                                               find_data.nFileSizeLow);
                info->attributes.last_write_time = win32_u64_from_filetime(find_data.ftLastWriteTime);
                info->attributes.flags = win32_convert_file_attribute_flags(find_data.dwFileAttributes);
            }
            if (!FindNextFile(search, &find_data)){
                break;
            }
        }
        
        result.infos = push_array(arena, File_Info*, count);
        result.count = count;
        
        i32 counter = 0;
        for (File_Info *node = first;
             node != 0;
             node = node->next){
            result.infos[counter] = node;
            counter += 1;
        }
    }
    
    return(result);
}

internal
Sys_Quick_File_Attributes_Sig(system_quick_file_attributes){
    WIN32_FILE_ATTRIBUTE_DATA info = {};
    File_Attributes result = {};
    if (GetFileAttributesEx_utf8String(scratch, file_name, GetFileExInfoStandard, &info)){
        result.size = ((u64)info.nFileSizeHigh << 32LL) | ((u64)info.nFileSizeLow);
        result.last_write_time = ((u64)info.ftLastWriteTime.dwHighDateTime << 32LL) | ((u64)info.ftLastWriteTime.dwLowDateTime);
        result.flags = win32_convert_file_attribute_flags(info.dwFileAttributes);
    }
    return(result);
}

internal
Sys_Load_Handle_Sig(system_load_handle){
    b32 result = false;
    HANDLE file = CreateFile_utf8(scratch, (u8*)filename, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
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
    
    HANDLE file = CreateFile_utf8(scratch, (u8*)filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    
    if (file != INVALID_HANDLE_VALUE){
        u64 written_total = 0;
        
        b32 success = true;
        for (;written_total < data.size;){
            DWORD read_size = max_u32;
            DWORD write_size = 0;
            if ((data.size - written_total) < max_u32){
                read_size = (DWORD)data.size;
            }
            if (!WriteFile(file, data.str + written_total, read_size, &write_size, 0)){
                success = false;
                break;
            }
            written_total += write_size;
        }
        
        if (success){
            result = win32_file_attributes_from_HANDLE(file);
        }
        
        CloseHandle(file);
    }
    
    return(result);
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
system_schedule_step(u32 code);

internal UINT_PTR CALLBACK
color_picker_hook(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam){
    UINT_PTR result = 0;
    switch(Message)
    {
        // TODO(allen): review
        case WM_INITDIALOG:
        {
            CHOOSECOLORW *win32_params = (CHOOSECOLORW *)LParam;
            Color_Picker *picker = (Color_Picker*)win32_params->lCustData;
            SetWindowLongPtr(Window, GWLP_USERDATA, (LONG_PTR)LParam);
            
            u16 Temp[256];
            Temp[ArrayCount(Temp) - 1] = 0;
            
            b32 ignored;
            utf8_to_utf16_minimal_checking(Temp, ArrayCount(Temp), (u8 *)picker->title.str, picker->title.size, &ignored);
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
                    Color_Picker *picker = (Color_Picker*)win32_params->lCustData;
                    
                    RECT rect;
                    GetClientRect(swatch_window, &rect);
                    HDC swatch_dc = (HDC)WParam;
                    COLORREF Probe = GetPixel(swatch_dc, (rect.left + rect.right) / 4, (rect.top + rect.bottom) / 2);
                    int_color new_color = int_color_from_colorref(Probe, *picker->dest);
                    
                    if(*picker->dest != new_color)
                    {
                        *picker->dest = new_color;
                        system_schedule_step(0);
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

// TODO(allen): review
internal DWORD WINAPI
color_picker_thread(LPVOID Param)
{
    Color_Picker *picker = (Color_Picker*)Param;
    
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
    // TODO(allen): review
    // NOTE(casey): Because this is going to be used by a semi-permanent thread, we need to copy
    // it to system memory where it can live as long as it wants, no matter what we do over here
    // on the 4coder threads.
    Color_Picker *perm = (Color_Picker*)system_memory_allocate_extended(0, sizeof(Color_Picker));
    *perm = *picker;
    
    HANDLE ThreadHandle = CreateThread(0, 0, color_picker_thread, perm, 0, 0);
    CloseHandle(ThreadHandle);
}

// BOTTOM

