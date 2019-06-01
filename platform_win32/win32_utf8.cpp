/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 23.03.2017
 *
 * UTF8 versions of WIN32 calls.
 *
 */

// TOP

#if !defined(FRED_WIN32_UTF8_CPP)
#define FRED_WIN32_UTF8_CPP

// TODO(allen): rewrite _EVERYTHING_ that does Win32 UTF16 <-> UTF8
internal Win32_UTF16
input_8_to_16(Arena *scratch, u8 *in, u32 in_length){
    Win32_UTF16 r = {};
    u32 utf16_max = (in_length + 1)*2;
    u16 *utf16 = push_array(scratch, u16, utf16_max);
    b32 error = false;
    u32 utf16_len = (u32)utf8_to_utf16_minimal_checking(utf16, utf16_max - 1, in, in_length, &error);
    if (!error && utf16_len < utf16_max){
        utf16[utf16_len] = 0;
        r.success = true;
        r.utf8_len = in_length;
        r.utf16_max = utf16_max;
        r.utf16_len = utf16_len;
        r.utf16 = utf16;
    }
    return(r);
}

internal Win32_UTF16
input_8_to_16(Arena *scratch, u8 *in){
    u32 length = 0;
    for (;in[length];++length);
    return(input_8_to_16(scratch, in, length));
}

internal Win32_UTF16
input_8_to_16(Arena *scratch, String in){
    return(input_8_to_16(scratch, (u8*)in.str, in.size));
}

internal HANDLE
CreateFile_utf8(Arena *scratch, u8 *name, DWORD access, DWORD share, LPSECURITY_ATTRIBUTES security, DWORD creation, DWORD flags, HANDLE template_file){
    HANDLE result = INVALID_HANDLE_VALUE;
    
    Temp_Memory temp = begin_temp(scratch);
    
    Win32_UTF16 name_16 = input_8_to_16(scratch, name);
    if (name_16.success){
        result = CreateFileW((LPWSTR)name_16.utf16, access, share, security, creation, flags, template_file);
    }
    
    end_temp(temp);
    
    return(result);
}

internal DWORD
GetFinalPathNameByHandle_utf8(Arena *scratch, HANDLE file, u8 *file_path_out, DWORD path_max, DWORD flags){
    DWORD result = 0;
    
    if (file_path_out == 0){
        result = GetFinalPathNameByHandleW(file, 0, 0, flags);
        result *= 2;
    }
    else{
        Temp_Memory temp = begin_temp(scratch);
        
        u32 path_16_max = KB(32);
        u16 *path_16 = push_array(scratch, u16, path_16_max);
        
        DWORD length_16 = GetFinalPathNameByHandleW(file, (LPWSTR)path_16, path_16_max, flags);
        
        if (length_16 != 0 && length_16 < path_16_max){
            b32 convert_error = false;
            u32 path_8_len = (u32)utf16_to_utf8_minimal_checking(file_path_out, path_max-1, path_16, length_16, &convert_error);
            if (path_8_len < path_max && !convert_error){
                file_path_out[path_8_len] = 0;
                result = path_8_len;
            }
        }
        
        end_temp(temp);
    }
    
    return(result);
}

internal HANDLE
FindFirstFile_utf8(Arena *scratch, u8 *name, LPWIN32_FIND_DATA find_data){
    HANDLE result = INVALID_HANDLE_VALUE;
    
    Temp_Memory temp = begin_temp(scratch);
    
    Win32_UTF16 name_16 = input_8_to_16(scratch, name);
    if (name_16.success){
        result = FindFirstFileW((LPWSTR)name_16.utf16, find_data);
    }
    
    end_temp(temp);
    
    return(result);
}

internal DWORD
GetFileAttributes_utf8(Arena *scratch, u8 *name){
    DWORD result = 0;
    
    Temp_Memory temp = begin_temp(scratch);
    
    Win32_UTF16 name_16 = input_8_to_16(scratch, name);
    if (name_16.success){
        result = GetFileAttributesW((LPWSTR)name_16.utf16);
    }
    
    end_temp(temp);
    
    return(result);
}

internal DWORD
GetModuleFileName_utf8(Arena *scratch, HMODULE module, u8 *file_out, DWORD max){
    DWORD result = 0;
    Temp_Memory temp = begin_temp(scratch);
    u32 file_16_max = KB(40);
    u16 *file_16 = push_array(scratch, u16, file_16_max);
    DWORD file_16_len = GetModuleFileNameW(module, (LPWSTR)file_16, file_16_max);
    b32 convert_error = false;
    u32 file_8_len = (u32)utf16_to_utf8_minimal_checking(file_out, max - 1, file_16, file_16_len, &convert_error);
    result = file_8_len;
    if (convert_error || file_8_len >= max){
        result = 0;
    }
    end_temp(temp);
    return(result);
}

internal BOOL
CreateProcess_utf8(Arena *scratch, u8 *app_name, u8 *command, LPSECURITY_ATTRIBUTES security, LPSECURITY_ATTRIBUTES thread, BOOL inherit_handles, DWORD creation, LPVOID environment, u8 *curdir, LPSTARTUPINFO startup, LPPROCESS_INFORMATION process){
    BOOL result = false;
    
    Temp_Memory temp = begin_temp(scratch);
    
    Win32_UTF16 app_name_16 = input_8_to_16(scratch, app_name);
    
    if (app_name_16.success){
        Win32_UTF16 command_16 = input_8_to_16(scratch, command);
        if (command_16.success){
            Win32_UTF16 curdir_16 = input_8_to_16(scratch, curdir);
            if (curdir_16.success){
                result = CreateProcessW((LPWSTR)app_name_16.utf16, (LPWSTR)command_16.utf16, security, thread, inherit_handles, creation, environment, (LPWSTR)curdir_16.utf16, startup, process);
            }
        }
    }
    
    end_temp(temp);
    
    return(result);
}

internal DWORD
GetCurrentDirectory_utf8(Arena *scratch, DWORD max, u8 *buffer){
    DWORD result = 0;
    
    Temp_Memory temp = begin_temp(scratch);
    
    u32 buffer_16_max = KB(40);
    u16 *buffer_16 = push_array(scratch, u16, buffer_16_max);
    
    DWORD buffer_16_len = GetCurrentDirectoryW(buffer_16_max, (LPWSTR)buffer_16);
    
    b32 error = false;
    u32 buffer_8_len = (u32)utf16_to_utf8_minimal_checking(buffer, max-1, buffer_16, buffer_16_len, &error);
    
    if (buffer_8_len < max && !error){
        buffer[buffer_8_len] = 0;
        result = buffer_8_len;
    }
    
    end_temp(temp);
    
    return(result);
}

internal int
MessageBox_utf8(Arena *scratch, HWND owner, u8 *text, u8 *caption, UINT type){
    int result = 0;
    Temp_Memory temp = begin_temp(scratch);
    Win32_UTF16 text_16 = input_8_to_16(scratch, text);
    if (text_16.success){
        Win32_UTF16 caption_16 = input_8_to_16(scratch, caption);
        if (caption_16.success){
            result = MessageBoxW(owner, (LPWSTR)text_16.utf16, (LPWSTR)caption_16.utf16, type);
        }
    }
    end_temp(temp);
    return(result);
}

internal BOOL
SetWindowText_utf8(Arena *scratch, HWND window, u8 *string){
    BOOL result = FALSE;
    Temp_Memory temp = begin_temp(scratch);
    Win32_UTF16 string_16 = input_8_to_16(scratch, string);
    if (string_16.success){
        result = SetWindowTextW(window, (LPWSTR)string_16.utf16);
    }
    end_temp(temp);
    return(result);
}

internal BOOL
GetFileAttributesEx_utf8String(Arena *scratch, String_Const_u8 file_name, GET_FILEEX_INFO_LEVELS info_level_id, LPVOID file_info){
    BOOL result = FALSE;
    Temp_Memory temp = begin_temp(scratch);
    Win32_UTF16 string_16 = input_8_to_16(scratch, string_old_from_new(file_name));
    if (string_16.success){
        result = GetFileAttributesExW((LPWSTR)string_16.utf16, info_level_id, file_info);
    }
    end_temp(temp);
    return(result);
}

#endif

// BOTTOM

