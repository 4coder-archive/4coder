/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 23.03.2017
 *
 * UTF8 versions of WIN32 calls.
 *
 */

// TOP

#if !defined(FRED_WIN32_UTF8_H)
#define FRED_WIN32_UTF8_H

internal HANDLE
CreateFile_utf8(u8 *name, DWORD access, DWORD share, LPSECURITY_ATTRIBUTES security, DWORD creation, DWORD flags, HANDLE template_file);

internal DWORD
GetFinalPathNameByHandle_utf8(HANDLE file, u8 *file_path_out, DWORD path_max, DWORD flags);

internal HANDLE
FindFirstFile_utf8(u8 *name, LPWIN32_FIND_DATA find_data);

internal DWORD
GetFileAttributes_utf8(u8 *name);

internal DWORD
GetModuleFileName_utf8(HMODULE module, u8 *file_out, DWORD max);

internal BOOL
CreateProcess_utf8(u8 *app_name, u8 *command, LPSECURITY_ATTRIBUTES security, LPSECURITY_ATTRIBUTES thread, BOOL inherit_handles, DWORD creation, LPVOID environment, u8 *curdir, LPSTARTUPINFO startup, LPPROCESS_INFORMATION process);

internal DWORD
GetCurrentDirectory_utf8(DWORD max, u8 *buffer);

internal int
MessageBox_utf8(HWND owner, u8 *text, u8 *caption, UINT type);

internal BOOL
SetWindowText_utf8(HWND window, u8 *string);

// For implementation

struct Win32_UTF16{
    b32 success;
    u32 utf8_len;
    u32 utf16_max;
    u32 utf16_len;
    u16 *utf16;
};

#endif

// BOTTOM

