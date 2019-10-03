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

function HANDLE
CreateFile_utf8(Arena *scratch, u8 *name, DWORD access, DWORD share, LPSECURITY_ATTRIBUTES security, DWORD creation, DWORD flags, HANDLE template_file);

function DWORD
GetFinalPathNameByHandle_utf8(Arena *scratch, HANDLE file, u8 *file_path_out, DWORD path_max, DWORD flags);

function HANDLE
FindFirstFile_utf8(Arena *scratch, u8 *name, LPWIN32_FIND_DATA find_data);

function DWORD
GetFileAttributes_utf8(Arena *scratch, u8 *name);

function DWORD
GetModuleFileName_utf8(Arena *scratch, HMODULE module, u8 *file_out, DWORD max);

function BOOL
CreateProcess_utf8(Arena *scratch, u8 *app_name, u8 *command, LPSECURITY_ATTRIBUTES security, LPSECURITY_ATTRIBUTES thread, BOOL inherit_handles, DWORD creation, LPVOID environment, u8 *curdir, LPSTARTUPINFO startup, LPPROCESS_INFORMATION process);

function DWORD
GetCurrentDirectory_utf8(Arena *scratch, DWORD max, u8 *buffer);

function int
MessageBox_utf8(Arena *scratch, HWND owner, u8 *text, u8 *caption, UINT type);

function BOOL
SetWindowText_utf8(Arena *scratch, HWND window, u8 *string);

function BOOL
GetFileAttributesEx_utf8String(Arena *scratch, String_Const_u8 file_name, GET_FILEEX_INFO_LEVELS info_level_id, LPVOID file_info);

function HMODULE
LoadLibrary_utf8String(Arena *scratch, String_Const_u8 file_name);

#endif

// BOTTOM

