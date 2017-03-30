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
CreateFile_utf8(u8 *name, DWORD access, DWORD share, LPSECURITY_ATTRIBUTES security, DWORD creation, DWORD flags, HANDLE template_file){
    HANDLE result = INVALID_HANDLE_VALUE;
    
    Partition *scratch = &shared_vars.scratch;
    Temp_Memory temp = begin_temp_memory(scratch);
    
    u32 len = 0;
    for (;name[len];++len);
    u32 name_16_max = (len+1)*2;
    u16 *name_16 = push_array(scratch, u16, name_16_max);
    
    b32 convert_error = false;
    u32 name_16_len = (u32)utf8_to_utf16_minimal_checking(name_16, name_16_max-1, name, len, &convert_error);
    
    if (!convert_error){
        name_16[name_16_len] = 0;
        result = CreateFileW((LPWSTR)name_16, access, share, security, creation, flags, template_file);
    }
    
    end_temp_memory(temp);
    
    return(result);
}

internal DWORD
GetFinalPathNameByHandle_utf8(HANDLE file, u8 *file_path_out, DWORD path_max, DWORD flags){
    DWORD result = 0;
    
    if (file_path_out == 0){
        result = GetFinalPathNameByHandleW(file, 0, 0, flags);
        result *= 2;
    }
    else{
        Partition *scratch = &shared_vars.scratch;
        Temp_Memory temp = begin_temp_memory(scratch);
        
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
        
        end_temp_memory(temp);
    }
    
    return(result);
}

internal HANDLE
FindFirstFile_utf8(u8 *name, LPWIN32_FIND_DATA find_data){
    HANDLE result = INVALID_HANDLE_VALUE;
    
    Partition *scratch = &shared_vars.scratch;
    Temp_Memory temp = begin_temp_memory(scratch);
    
    u32 len = 0;
    for (;name[len];++len);
    u32 name_16_max = (len+1)*2;
    u16 *name_16 = push_array(scratch, u16, name_16_max);
    
    b32 convert_error = false;
    u32 name_16_len = (u32)utf8_to_utf16_minimal_checking(name_16, name_16_max-1, name, len, &convert_error);
    
    if (name_16_len < name_16_max && !convert_error){
        name_16[name_16_len] = 0;
        result = FindFirstFileW((LPWSTR)name_16, find_data);
    }
    
    end_temp_memory(temp);
    
    return(result);
}

internal DWORD
GetFileAttributes_utf8(u8 *name){
    DWORD result = 0;
    
    Partition *scratch = &shared_vars.scratch;
    Temp_Memory temp = begin_temp_memory(scratch);
    
    u32 len = 0;
    for (;name[len];++len);
    u32 name_16_max = (len+1)*2;
    u16 *name_16 = push_array(scratch, u16, name_16_max);
    
    b32 convert_error = false;
    u32 name_16_len = (u32)utf8_to_utf16_minimal_checking(name_16, name_16_max-1, name, len, &convert_error);
    
    if (name_16_len < name_16_max && !convert_error){
        name_16[name_16_len] = 0;
        result = GetFileAttributesW((LPWSTR)name_16);
    }
    
    end_temp_memory(temp);
    
    return(result);
}

internal DWORD
GetModuleFileName_utf8(HMODULE module, u8 *file_out, DWORD max){
    DWORD result = 0;
    
    Partition *scratch = &shared_vars.scratch;
    Temp_Memory temp = begin_temp_memory(scratch);
    
    u32 file_16_max = KB(40);
    u16 *file_16 = push_array(scratch, u16, file_16_max);
    
    DWORD file_16_len = GetModuleFileNameW(module, (LPWSTR)file_16, file_16_max);
    
    if (max > 0){
        b32 convert_error = false;
        u32 file_8_len = (u32)utf16_to_utf8_minimal_checking(file_out, max-1, file_16, file_16_len, &convert_error);
        result = file_8_len;
        if (convert_error || file_8_len >= max){
            result = 0;
        }
    }
    
    end_temp_memory(temp);
    
    return(result);
}

internal BOOL
CreateProcess_utf8(u8 *app_name, u8 *command, LPSECURITY_ATTRIBUTES security, LPSECURITY_ATTRIBUTES thread, BOOL inherit_handles, DWORD creation, LPVOID environment, u8 *curdir, LPSTARTUPINFO startup, LPPROCESS_INFORMATION process){
    BOOL result = false;
    
    Partition *scratch = &shared_vars.scratch;
    Temp_Memory temp = begin_temp_memory(scratch);
    
    u32 app_name_len = 0;
    for (;app_name[app_name_len];++app_name_len);
    
    u32 command_len = 0;
    for (;command[command_len];++command_len);
    
    u32 curdir_len = 0;
    for (;curdir[curdir_len];++curdir_len);
    
    u32 app_name_16_max = (app_name_len+1)*2;
    u32 command_16_max = (command_len+1)*2;
    u32 curdir_16_max = (curdir_len+1)*2;
    
    u16 *app_name_16 = push_array(scratch, u16, app_name_16_max);
    u16 *command_16 = push_array(scratch, u16, command_16_max);
    u16 *curdir_16 = push_array(scratch, u16, curdir_16_max);
    
    b32 error = false;
    u32 app_name_16_len = (u32)utf8_to_utf16_minimal_checking(app_name_16, app_name_16_max-1, app_name, app_name_len, &error);
    
    if (app_name_16_len < app_name_16_max && !error){
        u32 command_16_len = (u32)utf8_to_utf16_minimal_checking(command_16, command_16_max-1, command, command_len, &error);
        
        if (command_16_len < command_16_max && !error){
            u32 curdir_16_len = (u32)utf8_to_utf16_minimal_checking(curdir_16, curdir_16_max-1, curdir, curdir_len, &error);
            
            app_name_16[app_name_16_len] = 0;
            command_16[command_16_len] = 0;
            curdir_16[curdir_16_len] = 0;
            
            result = CreateProcessW((LPWSTR)app_name_16, (LPWSTR)command_16, security, thread, inherit_handles, creation, environment, (LPWSTR)curdir_16, startup, process);
        }
    }
    
    end_temp_memory(temp);
    
    return(result);
}

internal DWORD
GetCurrentDirectory_utf8(DWORD max, u8 *buffer){
    DWORD result = 0;
    
    Partition *scratch = &shared_vars.scratch;
    Temp_Memory temp = begin_temp_memory(scratch);
    
    u32 buffer_16_max = KB(40);
    u16 *buffer_16 = push_array(scratch, u16, buffer_16_max);
    
    DWORD buffer_16_len = GetCurrentDirectoryW(buffer_16_max, (LPWSTR)buffer_16);
    
    b32 error = false;
    u32 buffer_8_len = (u32)utf16_to_utf8_minimal_checking(buffer, max-1, buffer_16, buffer_16_len, &error);
    
    if (buffer_8_len < max && !error){
        buffer[buffer_8_len] = 0;
        result = buffer_8_len;
    }
    
    end_temp_memory(temp);
    
    return(result);
}

internal int
MessageBox_utf8(HWND owner, u8 *text, u8 *caption, UINT type){
    int result = 0;
    
    Partition *scratch = &shared_vars.scratch;
    Temp_Memory temp = begin_temp_memory(scratch);
    
    u32 text_len = 0;
    for(;text[text_len];++text_len);
    
    u32 caption_len = 0;
    for(;caption[caption_len];++caption_len);
    
    u32 text_16_max = (text_len+1)*2;
    u32 caption_16_max = (caption_len+1)*2;
    
    u16 *text_16 = push_array(scratch, u16, text_16_max);
    u16 *caption_16 = push_array(scratch, u16, caption_16_max);
    
    b32 error = false;
    u32 text_16_len = (u32)utf8_to_utf16_minimal_checking(text_16, text_16_max-1, text, text_len, &error);
    
    if (text_16_len < text_16_max && !error){
        u32 caption_16_len = (u32)utf8_to_utf16_minimal_checking(caption_16, caption_16_max-1, caption, caption_len, &error);
        
        if (text_16_len < text_16_max && !error){
            text_16[text_16_len] = 0;
            caption_16[caption_16_len] = 0;
            MessageBoxW(owner, (LPWSTR)text_16, (LPWSTR)caption_16, type);
        }
    }
    
    end_temp_memory(temp);
    
    return(result);
}

#endif

// BOTTOM

