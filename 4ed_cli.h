/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 24.03.2018
 *
 * CLI handling code.
 *
 */

// TOP

#if !defined(FRED_CLI_H)
#define FRED_CLI_H

struct CLI_Process{
    CLI_Handles cli;
    Editing_File *out_file;
    b32 cursor_at_end;
};

struct CLI_List{
    CLI_Process *procs;
    u32 count;
    u32 max;
};

#endif

// BOTTOM

