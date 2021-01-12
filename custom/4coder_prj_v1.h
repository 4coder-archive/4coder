/*
4coder_prj_v1.h - type header paired with 4coder_prj_v1.cpp
*/

// TOP

#ifndef FCODER_PRJ_V1_H
#define FCODER_PRJ_V1_H

///////////////////////////////
// NOTE(allen): Project v0-v1 Structure

struct Prj_V1_File_Load_Path{
    String8 path;
    b32 recursive;
    b32 relative;
};

struct Prj_V1_File_Load_Path_Array{
    Prj_V1_File_Load_Path *paths;
    i32 count;
};

struct Prj_V1_Command{
    String8 name;
    String8 cmd;
    String8 out;
    b32 footer_panel;
    b32 save_dirty_files;
    b32 cursor_at_end;
};

struct Prj_V1_Command_Array{
    Prj_V1_Command *commands;
    i32 count;
};

struct Prj_V1{
    b32 loaded;
    
    String8 dir;
    String8 name;
    
    Prj_Pattern_List pattern_list;
    Prj_Pattern_List blacklist_pattern_list;
    Prj_V1_File_Load_Path_Array load_path_array;
    Prj_V1_Command_Array command_array;
    
    i32 fkey_commands[16];
};

enum Prj_V1_OS_Match_Level{
    PrjV1OSMatchLevel_NoMatch = 0,
    PrjV1OSMatchLevel_PassiveMatch = 1,
    PrjV1OSMatchLevel_ActiveMatch = 2,
};


////////////////////////////////
// NOTE(allen): Project v0-v1 -> v2 Function

function Variable_Handle prj_v1_to_v2(Application_Links *app, String8 dir, Config *parsed);


#endif //4CODER_PRJ_V1_H

// BOTTOM
