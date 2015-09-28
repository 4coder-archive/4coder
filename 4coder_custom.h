
#define MDFR_NONE 0
#define MDFR_CTRL 1
#define MDFR_ALT 2
#define MDFR_SHIFT 4

typedef unsigned short Code;

struct Key_Codes{
	Code back;
	Code up;
	Code down;
	Code left;
	Code right;
	Code del;
	Code insert;
	Code home;
	Code end;
	Code page_up;
	Code page_down;
	Code esc;
    
#if 0 // TODO(allen): Get these working sometime
	union{
		struct{
			Code f1;
			Code f2;
			Code f3;
			Code f4;
			Code f5;
			Code f6;
			Code f7;
			Code f8;
			
			Code f9;
			Code f10;
			Code f11;
			Code f12;
			Code f13;
			Code f14;
			Code f15;
			Code f16;
		};
		Code f[16];
	};
#endif
};

enum Command_ID{
    cmdid_null,
    cmdid_write_character,
    cmdid_seek_whitespace_right,
    cmdid_seek_whitespace_left,
    cmdid_seek_whitespace_up,
    cmdid_seek_whitespace_down,
    cmdid_seek_token_left,
    cmdid_seek_token_right,
    cmdid_seek_white_or_token_left,
    cmdid_seek_white_or_token_right,
    cmdid_seek_alphanumeric_left,
    cmdid_seek_alphanumeric_right,
    cmdid_seek_alphanumeric_or_camel_left,
    cmdid_seek_alphanumeric_or_camel_right,
    cmdid_search,
    cmdid_rsearch,
    cmdid_goto_line,
    cmdid_set_mark,
    cmdid_copy,
    cmdid_cut,
    cmdid_paste,
    cmdid_paste_next,
    cmdid_delete_chunk,
    cmdid_interactive_new,
    cmdid_interactive_open,
    cmdid_reopen,
    cmdid_save,
    cmdid_interactive_save_as,
    cmdid_change_active_panel,
    cmdid_interactive_switch_file,
    cmdid_interactive_kill_file,
    cmdid_kill_file,
    cmdid_toggle_line_wrap,
    cmdid_toggle_endline_mode,
    cmdid_to_uppercase,
    cmdid_to_lowercase,
    cmdid_toggle_show_whitespace,
    cmdid_clean_line,
    cmdid_clean_all_lines,
    cmdid_eol_dosify,
    cmdid_eol_nixify,
    cmdid_auto_tab,
    cmdid_open_panel_vsplit,
    cmdid_open_panel_hsplit,
    cmdid_close_panel,
    cmdid_move_left,
    cmdid_move_right,
    cmdid_delete,
    cmdid_backspace,
    cmdid_move_up,
    cmdid_move_down,
    cmdid_seek_end_of_line,
    cmdid_seek_beginning_of_line,
    cmdid_page_up,
    cmdid_page_down,
    cmdid_open_color_tweaker,
    cmdid_close_minor_view,
    cmdid_cursor_mark_swap,
    cmdid_open_menu,
    //
    cmdid_count
};

struct Extra_Font{
    char file_name[256];
    char font_name[24];
    int size;
};

#define GET_BINDING_DATA(name) int name(void *data, int size, Key_Codes *codes)
#define SET_EXTRA_FONT_SIG(name) void name(Extra_Font *font_out)
#define CUSTOM_COMMAND_SIG(name) void name(void *cmd_context, struct Application_Links app)
#define START_HOOK_SIG(name) void name(void *cmd_context, struct Application_Links app)

extern "C"{
    typedef CUSTOM_COMMAND_SIG(Custom_Command_Function);
    typedef GET_BINDING_DATA(Get_Binding_Data_Function);
    typedef SET_EXTRA_FONT_SIG(Set_Extra_Font_Function);
    typedef START_HOOK_SIG(Start_Hook_Function);
}

#define EXECUTE_COMMAND_SIG(name) void name(void *cmd_context, int command_id)
#define FULFILL_INTERACTION_SIG(name) void name(void *cmd_context, char *data, bool full_set)

extern "C"{
    typedef EXECUTE_COMMAND_SIG(Exec_Command_Function);
    typedef FULFILL_INTERACTION_SIG(Fulfill_Interaction_Function);
}

struct Application_Links{
    Exec_Command_Function *exec_command;
    Fulfill_Interaction_Function *fulfill_interaction;
};

enum Binding_Unit_Type{
    UNIT_HEADER,
    UNIT_MAP_BEGIN,
    UNIT_BINDING,
    UNIT_CALLBACK
};

enum Map_ID{
    MAPID_GLOBAL,
    MAPID_FILE
};

struct Binding_Unit{
    Binding_Unit_Type type;
    union{
        struct{ int total_size; int error; } header;
        
        struct{ int mapid; } map_begin;
        
        struct{
            int command_id;
            short code;
            unsigned char modifiers;
        } binding;
        
        struct{
            Custom_Command_Function *func;
            short code;
            unsigned char modifiers;
        } callback;
    };
};

