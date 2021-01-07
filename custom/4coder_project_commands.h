/*
4coder_project_commands.h - type header paired with 4coder_project_commands.cpp
*/

// TOP

#if !defined(FCODER_PROJECT_COMMANDS_H)
#define FCODER_PROJECT_COMMANDS_H

// TODO(allen): finish sorting

////////////////////////////////
// NOTE(allen): * Type

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

////////////////////////////////
// NOTE(allen): * Functions

function Match_Pattern_List prj_pattern_list_from_extension_array(Arena *arena, String_Const_u8_Array list);
function Match_Pattern_List prj_pattern_list_from_var(Arena *arena, Variable_Handle var);
function Match_Pattern_List prj_get_standard_blacklist(Arena *arena);

function b32  prj_match_in_pattern_list(String_Const_u8 string, Match_Pattern_List list);

function void prj_close_files_with_ext(Application_Links *app, String_Const_u8_Array extension_array);
function void prj_open_files_with_ext(Application_Links *app, String_Const_u8 dir, String_Const_u8_Array array, u32 flags);
function void prj_open_files_pattern_filter(Application_Links *app, String_Const_u8 dir, Match_Pattern_List whitelist, Match_Pattern_List blacklist, u32 flags);
function void prj_open_all_files_with_ext_in_hot(Application_Links *app, String_Const_u8_Array array, u32 flags);

////////////////////////////////
// NOTE(allen): Project Parse

function void prj_parse_pattern_list(Arena *arena, Config * parsed, char* root_variable_name, Match_Pattern_List *list_out);
function Project_OS_Match_Level prj_parse_v1_os_match(String_Const_u8 str, String_Const_u8 this_os_str);
function Project *prj_parse_from_v1_config_data(Application_Links *app, Arena *arena, String_Const_u8 root_dir, Config * parsed);

function String_Const_u8 prj_join_pattern_string(Arena *arena, List_String_Const_u8 list);
function String_Const_u8 prj_sanitize_string(Arena *arena, String_Const_u8 string);
function Variable_Handle prj_version_1_to_version_2(Application_Links *app, Config * parsed, Project *project);
function String_Const_u8 prj_path_from_project(Arena *arena, Variable_Handle project);
function void            prj_exec_command(Application_Links *app, Variable_Handle cmd_var);
function Variable_Handle prj_command_from_name(Application_Links *app, String_Const_u8 cmd_name);
function void            prj_exec_command_name(Application_Links *app, String_Const_u8 cmd_name);
function void            prj_exec_command_fkey_index(Application_Links *app, i32 fkey_index);

function Project_Setup_Status project_is_setup(Application_Links *app, String_Const_u8 script_path, String_Const_u8 script_file);
function Project_Key_Strings project_key_strings_query_user(Application_Links *app, b32 get_script_file, b32 get_code_file, u8* script_file_space, i32 script_file_cap, u8* code_file_space, i32 code_file_cap, u8* output_dir_space, i32 output_dir_cap, u8* binary_file_space, i32 binary_file_cap);
function b32 project_generate_bat_script(Arena *scratch, String_Const_u8 opts, String_Const_u8 compiler, String_Const_u8 script_path, String_Const_u8 script_file, String_Const_u8 code_file, String_Const_u8 output_dir, String_Const_u8 binary_file);
function b32 project_generate_sh_script(Arena *scratch, String_Const_u8 opts, String_Const_u8 compiler, String_Const_u8 script_path, String_Const_u8 script_file, String_Const_u8 code_file, String_Const_u8 output_dir, String_Const_u8 binary_file);
function b32 project_generate_project_4coder_file(Arena *scratch, String_Const_u8 script_path, String_Const_u8 script_file, String_Const_u8 output_dir, String_Const_u8 binary_file);
function void project_setup_scripts__generic(Application_Links *app, b32 do_project_file, b32 do_bat_script, b32 do_sh_script);

function Variable_Handle prj_cmd_from_user(Application_Links *app, Variable_Handle prj_var, String_Const_u8 query);

function Variable_Handle get_project_command_from_user(Application_Links *app, Project *project, String_Const_u8 query);

#endif

// BOTTOM

