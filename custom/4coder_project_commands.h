/*
4coder_project_commands.h - type header paired with 4coder_project_commands.cpp
*/

// TOP

#if !defined(FCODER_PROJECT_COMMANDS_H)
#define FCODER_PROJECT_COMMANDS_H

////////////////////////////////
// NOTE(allen): Match Pattern Types

struct Prj_Pattern{
    String8List absolutes;
};

struct Prj_Pattern_Node{
    Prj_Pattern_Node *next;
    Prj_Pattern pattern;
};

struct Prj_Pattern_List{
    Prj_Pattern_Node *first;
    Prj_Pattern_Node *last;
    i32 count;
};

typedef u32 Prj_Open_File_Flags;
enum{
    PrjOpenFileFlag_Recursive = 1,
};

///////////////////////////////
// NOTE(allen): Project Files

struct Prj_Setup_Status{
    b32 bat_exists;
    b32 sh_exists;
    b32 project_exists;
    b32 everything_exists;
};

struct Prj_Key_Strings{
    b32 success;
    String8 script_file;
    String8 code_file;
    String8 output_dir;
    String8 binary_file;
};

typedef u32 Prj_Setup_Script_Flags;
enum{
    PrjSetupScriptFlag_Project = 0x1,
    PrjSetupScriptFlag_Bat     = 0x2,
    PrjSetupScriptFlag_Sh      = 0x4,
};

////////////////////////////////
// NOTE(allen): File Pattern Operators

function Prj_Pattern_List prj_pattern_list_from_extension_array(Arena *arena, String8Array list);
function Prj_Pattern_List prj_pattern_list_from_var(Arena *arena, Variable_Handle var);
function Prj_Pattern_List prj_get_standard_blacklist(Arena *arena);

function b32  prj_match_in_pattern_list(String8 string, Prj_Pattern_List list);

function void prj_close_files_with_ext(Application_Links *app, String8Array extension_array);
function void prj_open_files_pattern_filter(Application_Links *app, String8 dir, Prj_Pattern_List whitelist, Prj_Pattern_List blacklist, Prj_Open_File_Flags flags);
function void prj_open_all_files_with_ext_in_hot(Application_Links *app, String8Array array, Prj_Open_File_Flags flags);

////////////////////////////////
// NOTE(allen): Project Files

function void prj_stringize_project(Application_Links *app, Arena *arena, Variable_Handle project, String8List *out);

function Prj_Setup_Status prj_file_is_setup(Application_Links *app, String8 script_path, String8 script_file);
function b32 prj_generate_bat(Arena *scratch, String8 opts, String8 compiler, String8 script_path, String8 script_file, String8 code_file, String8 output_dir, String8 binary_file);
function b32 prj_generate_sh(Arena *scratch, String8 opts, String8 compiler, String8 script_path, String8 script_file, String8 code_file, String8 output_dir, String8 binary_file);
function b32 prj_generate_project(Arena *scratch, String8 script_path, String8 script_file, String8 output_dir, String8 binary_file);

function void prj_setup_scripts(Application_Links *app, Prj_Setup_Script_Flags flags);

////////////////////////////////
// NOTE(allen): Project Operations

function void            prj_exec_command(Application_Links *app, Variable_Handle cmd_var);
function Variable_Handle prj_command_from_name(Application_Links *app, String8 cmd_name);
function void            prj_exec_command_name(Application_Links *app, String8 cmd_name);
function void            prj_exec_command_fkey_index(Application_Links *app, i32 fkey_index);

function String8         prj_full_file_path_from_project(Arena *arena, Variable_Handle project);
function String8         prj_path_from_project(Arena *arena, Variable_Handle project);
function Variable_Handle prj_cmd_from_user(Application_Links *app, Variable_Handle prj_var, String8 query);

#endif

// BOTTOM

