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

internal HANDLE
CreateFile_utf8(Arena *scratch, u8 *name, DWORD access, DWORD share, LPSECURITY_ATTRIBUTES security, DWORD creation, DWORD flags, HANDLE template_file){
    Temp_Memory temp = begin_temp(scratch);
    String_u16 name_16 = string_u16_from_string_u8(scratch, SCu8(name), StringFill_NullTerminate);
    HANDLE result = CreateFileW((LPWSTR)name_16.str, access, share, security, creation, flags, template_file);
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
            String_Const_u16 path_16_str = SCu16(path_16, length_16);
            String_u8 path_8 = string_u8_from_string_u16(scratch, path_16_str, StringFill_NullTerminate);
            if (path_8.size + 1 <= path_max && !convert_error){
                block_copy(file_path_out, path_8.str, path_8.size + 1);
                result = (DWORD)path_8.size;
            }
            else{
                result = (DWORD)path_8.size + 1;
            }
        }
        
        end_temp(temp);
    }
    
    return(result);
}

internal HANDLE
FindFirstFile_utf8(Arena *scratch, u8 *name, LPWIN32_FIND_DATA find_data){
    Temp_Memory temp = begin_temp(scratch);
    String_u16 name_16 = string_u16_from_string_u8(scratch, SCu8(name), StringFill_NullTerminate);
    HANDLE result = FindFirstFileW((LPWSTR)name_16.str, find_data);
    end_temp(temp);
    return(result);
}

internal DWORD
GetFileAttributes_utf8(Arena *scratch, u8 *name){
    Temp_Memory temp = begin_temp(scratch);
    String_u16 name_16 = string_u16_from_string_u8(scratch, SCu8(name), StringFill_NullTerminate);
    DWORD result = GetFileAttributesW((LPWSTR)name_16.str);
    end_temp(temp);
    return(result);
}

internal DWORD
GetModuleFileName_utf8(Arena *scratch, HMODULE module, u8 *file_out, DWORD max){
    Temp_Memory temp = begin_temp(scratch);
    u32 file_16_max = KB(40);
    u16 *file_16 = push_array(scratch, u16, file_16_max);
    DWORD file_16_len = GetModuleFileNameW(module, (LPWSTR)file_16, file_16_max);
    String_u8 file_8 = string_u8_from_string_u16(scratch, SCu16(file_16, file_16_len), StringFill_NullTerminate);
    DWORD result = 0;
    if (file_8.size + 1 <= max){
        block_copy(file_out, file_8.str, file_8.size + 1);
        result = (DWORD)file_8.size;
    }
    end_temp(temp);
    return(result);
}

internal BOOL
CreateProcess_utf8(Arena *scratch, u8 *app_name, u8 *command, LPSECURITY_ATTRIBUTES security, LPSECURITY_ATTRIBUTES thread, BOOL inherit_handles, DWORD creation, LPVOID environment, u8 *curdir, LPSTARTUPINFO startup, LPPROCESS_INFORMATION process){
    Temp_Memory temp = begin_temp(scratch);
    String_u16 app_name_16 = string_u16_from_string_u8(scratch, SCu8(app_name), StringFill_NullTerminate);
    String_u16 command_16 = string_u16_from_string_u8(scratch, SCu8(command), StringFill_NullTerminate);
    String_u16 curdir_16 = string_u16_from_string_u8(scratch, SCu8(curdir), StringFill_NullTerminate);
    BOOL result = CreateProcessW((LPWSTR)app_name_16.str, (LPWSTR)command_16.str, security, thread, inherit_handles, creation, environment, (LPWSTR)curdir_16.str, startup, process);
    end_temp(temp);
    return(result);
}

internal DWORD
GetCurrentDirectory_utf8(Arena *scratch, DWORD max, u8 *buffer){
    DWORD result = 0;
    
    if (buffer != 0){
        Temp_Memory temp = begin_temp(scratch);
        u32 buffer_16_max = KB(40);
        u16 *buffer_16 = push_array(scratch, u16, buffer_16_max);
        DWORD buffer_16_len = GetCurrentDirectoryW(buffer_16_max, (LPWSTR)buffer_16);
        String_u8 curdir_8 = string_u8_from_string_u16(scratch, SCu16(buffer_16, buffer_16_len), StringFill_NullTerminate);
        if (curdir_8.size + 1 <= max){
            block_copy(buffer, curdir_8.str, curdir_8.size + 1);
            result = (DWORD)curdir_8.size;
        }
        else{
            result = (DWORD)curdir_8.size + 1;
        }
        end_temp(temp);
    }
    else{
        result = GetCurrentDirectoryW(0, 0);
        result *= 2;
    }
    
    return(result);
}

internal int
MessageBox_utf8(Arena *scratch, HWND owner, u8 *text, u8 *caption, UINT type){
    Temp_Memory temp = begin_temp(scratch);
    String_u16 text_16 = string_u16_from_string_u8(scratch, SCu8(text), StringFill_NullTerminate);
    String_u16 caption_16 = string_u16_from_string_u8(scratch, SCu8(caption), StringFill_NullTerminate);
    int result = MessageBoxW(owner, (LPWSTR)text_16.str, (LPWSTR)caption_16.str, type);
    end_temp(temp);
    return(result);
}

internal BOOL
SetWindowText_utf8(Arena *scratch, HWND window, u8 *string){
    Temp_Memory temp = begin_temp(scratch);
    String_u16 string_16 = string_u16_from_string_u8(scratch, SCu8(string), StringFill_NullTerminate);
    BOOL result = SetWindowTextW(window, (LPWSTR)string_16.str);
    end_temp(temp);
    return(result);
}

internal BOOL
GetFileAttributesEx_utf8String(Arena *scratch, String_Const_u8 file_name, GET_FILEEX_INFO_LEVELS info_level_id, LPVOID file_info){
    Temp_Memory temp = begin_temp(scratch);
    String_u16 string_16 = string_u16_from_string_u8(scratch, file_name, StringFill_NullTerminate);
    BOOL result = GetFileAttributesExW((LPWSTR)string_16.str, info_level_id, file_info);
    end_temp(temp);
    return(result);
}

function HMODULE
LoadLibrary_utf8String(Arena *scratch, String_Const_u8 name){
    Temp_Memory temp = begin_temp(scratch);
    String_u16 string_16 = string_u16_from_string_u8(scratch, name, StringFill_NullTerminate);
    HMODULE result = LoadLibraryW((LPWSTR)string_16.str);
    end_temp(temp);
    return(result);
}

#endif

// BOTTOM

