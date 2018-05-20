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

struct Fkey_Command{
    char command[128];
    char out[128];
    bool32 use_build_panel;
    bool32 save_dirty_buffers;
};

struct Project{
    char dir_space[256];
    char *dir;
    int32_t dir_len;
    
    Extension_List extension_list;
    Fkey_Command fkey_commands[16];
    
    bool32 open_recursively;
    bool32 loaded;
};

///////////////////////////////

struct Project_Setup_Status{
    bool32 bat_exists;
    bool32 sh_exists;
    bool32 project_exists;
    bool32 everything_exists;
};

#endif

// BOTTOM

