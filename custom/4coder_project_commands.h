/*
4coder_project_commands.h - type header paired with 4coder_project_commands.cpp
*/

// TOP

#if !defined(FCODER_PROJECT_COMMANDS_H)
#define FCODER_PROJECT_COMMANDS_H

enum{
    OpenAllFilesFlag_Recursive = 1,
};

///////////////////////////////

struct Match_Pattern{
    List_String_Const_u8 absolutes;
};

struct Match_Pattern_Node{
    Match_Pattern_Node *next;
    Match_Pattern pattern;
};

struct Match_Pattern_List{
    Match_Pattern_Node *first;
    Match_Pattern_Node *last;
    i32 count;
};

struct Project_File_Load_Path{
    String_Const_u8 path;
    b32 recursive;
    b32 relative;
};

struct Project_File_Load_Path_Array{
    Project_File_Load_Path *paths;
    i32 count;
};

struct Project_Command{
    String_Const_u8 name;
    String_Const_u8 cmd;
    String_Const_u8 out;
    b32 footer_panel;
    b32 save_dirty_files;
    b32 cursor_at_end;
};

struct Project_Command_Array{
    Project_Command *commands;
    i32 count;
};

struct Project{
    b32 loaded;
    
    String_Const_u8 dir;
    String_Const_u8 name;
    
    Match_Pattern_List pattern_list;
    Match_Pattern_List blacklist_pattern_list;
    Project_File_Load_Path_Array load_path_array;
    Project_Command_Array command_array;
    
    // NOTE(allen): Only used for conversion from 1 -> 2
    i32 fkey_commands[16];
};

typedef i32 Project_OS_Match_Level;
enum{
    ProjectOSMatchLevel_NoMatch = 0,
    ProjectOSMatchLevel_PassiveMatch = 1,
    ProjectOSMatchLevel_ActiveMatch = 2,
};

///////////////////////////////

struct Project_Setup_Status{
    b32 bat_exists;
    b32 sh_exists;
    b32 project_exists;
    b32 everything_exists;
};

struct Project_Key_Strings{
    b32 success;
    String_Const_u8 script_file;
    String_Const_u8 code_file;
    String_Const_u8 output_dir;
    String_Const_u8 binary_file;
};

#endif

// BOTTOM

