/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 27.01.2016
 *
 * Global app level settings definition
 *
 */

// TOP

struct App_Settings{
    char *user_file;
    b32 user_file_is_strict;
    
    char *init_files[4];
    i32 init_files_count;
    i32 init_files_max;

    i32 initial_line;
    b32 lctrl_lalt_is_altgr;
};

// BOTTOM

