/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 09.02.2016
 *
 * Shared system functions
 *
 */

// TOP

// NOTE(allen): This serves as a list of functions to implement
// in addition to those in 4ed_system.h  These are not exposed to
// the application code, but system_shared.cpp and 4ed_rendering.cpp
// rely on the functions listed here.

struct Font_Load_Parameters{
    Font_Load_Parameters *next;
    Font_Load_Parameters *prev;
    
    Render_Font *font_out;
    char *filename;
    i32 pt_size;
    i32 tab_width;
};

struct Font_Load_System{
    Font_Load_Parameters *params;
    Font_Load_Parameters used_param;
    Font_Load_Parameters free_param;
    Partition part;
    i32 max;
};

struct File_Data{
    Data data;
    b32 got_file;
};
inline File_Data
file_data_zero(){
    File_Data result = {0};
    return(result);
}

#define Sys_Get_Memory_Sig(name) void* name(i32 size, i32 line_number, char *file_name)
#define Sys_Free_Memory_Sig(name) void name(void *block)
#define Sys_File_Can_Be_Made(name) b32 name(char *filename)
#define Sys_Load_File_Sig(name) File_Data name(char *filename)
#define Sys_Save_File_Sig(name) b32 name(char *filename, char *data, i32 size)
#define Sys_To_Binary_Path(name) b32 name(String *out_filename, char *filename)

internal Sys_Get_Memory_Sig(system_get_memory_);
internal Sys_Free_Memory_Sig(system_free_memory);
internal Sys_File_Can_Be_Made(system_file_can_be_made);
internal Sys_Load_File_Sig(system_load_file);
internal Sys_Save_File_Sig(system_save_file);
internal Sys_To_Binary_Path(system_to_binary_path);

#define system_get_memory(size) system_get_memory_((size), __LINE__, __FILE__)

// BOTTOM

