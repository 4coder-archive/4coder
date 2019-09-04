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

struct Child_Process{
    Node node;
    Child_Process_ID id;
    CLI_Handles cli;
    Editing_File *out_file;
    b32 cursor_at_end;
};

struct Child_Process_Container{
    Arena arena;
    Node child_process_active_list;
    Node child_process_free_list;
    i32 active_child_process_count;
    u32 child_process_id_counter;
    Table_u64_u64 id_to_ptr_table;
    Table_u64_u64 id_to_return_code_table;
};

struct Child_Process_And_ID{
    Child_Process *process;
    Child_Process_ID id;
};

#endif

// BOTTOM

