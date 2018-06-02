/*
4coder_project_commands.h - type header paired with 4coder_project_commands.cpp
*/

// TOP

#if !defined(FCODER_PROJECT_COMMANDS_H)
#define FCODER_PROJECT_COMMANDS_H

#include "4coder_lib/4coder_mem.h"

enum{
    OpenAllFilesFlag_Recursive = 1,
};

///////////////////////////////

struct Project_File_Pattern{
    Absolutes absolutes;
};

struct Project_File_Pattern_Array{
    Project_File_Pattern *patterns;
    int32_t count;
};

struct Project_File_Load_Path{
    String path;
    bool32 recursive;
    bool32 relative;
};

struct Project_File_Load_Path_Array{
    Project_File_Load_Path *paths;
    int32_t count;
};

struct Project_Command{
    String name;
    String cmd;
    String out;
    bool32 footer_panel;
    bool32 save_dirty_files;
    bool32 cursor_at_end;
};

struct Project_Command_Array{
    Project_Command *commands;
    int32_t count;
};

struct Project{
    bool32 loaded;
    
    String dir;
    String name;
    
    Project_File_Pattern_Array pattern_array;
    Project_File_Pattern_Array blacklist_pattern_array;
    Project_File_Load_Path_Array load_path_array;
    Project_Command_Array command_array;
    
    int32_t fkey_commands[16];
};

struct Project_Parse_Result{
    Config *parsed;
    Project *project;
};

///////////////////////////////

struct Project_Setup_Status{
    bool32 bat_exists;
    bool32 sh_exists;
    bool32 project_exists;
    bool32 everything_exists;
};

struct Project_Key_Strings{
    bool32 success;
    String script_file;
    String code_file;
    String output_dir;
    String binary_file;
};

#endif

// BOTTOM

