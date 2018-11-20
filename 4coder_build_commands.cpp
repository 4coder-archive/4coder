/*
4coder_build_commands.cpp - Commands for building.
*/

// TOP

// NOTE(allen|a4.0.9): This is provided to establish a default method of getting
// a "build directory".  This function tries to setup the build directory in the
// directory of the given buffer, if it cannot get that information it get's the
// 4coder hot directory.
//
//  There is no requirement that a custom build system in 4coder actually use the
// directory given by this function.
static int32_t
get_build_directory(Application_Links *app, Buffer_Summary *buffer, String *dir_out){
    int32_t result = BuildDir_None;
    
    if (buffer != 0 && buffer->file_name != 0){
        if (!match_cc(buffer->file_name, buffer->buffer_name)){
            char *file_name = buffer->file_name;
            int32_t file_name_len = buffer->file_name_len;
            String dir = make_string_cap(file_name, file_name_len, file_name_len+1);
            remove_last_folder(&dir);
            append_ss(dir_out, dir);
            result = BuildDir_AtFile;
        }
    }
    
    if (!result){
        int32_t len = directory_get_hot(app, dir_out->str,
                                        dir_out->memory_size - dir_out->size);
        if (dir_out->size + len < dir_out->memory_size){
            dir_out->size += len;
            result = BuildDir_AtHot;
        }
    }
    
    return(result);
}

// TODO(allen): Better names for the "standard build search" family.
static int32_t
standard_build_search(Application_Links *app, View_Summary *view, Buffer_Summary *active_buffer,
                      String *dir, String *command, bool32 perform_backup, bool32 use_path_in_command, String filename, String command_name){
    int32_t result = false;
    
    for(;;){
        int32_t old_size = dir->size;
        append_ss(dir, filename);
        
        if (file_exists(app, dir->str, dir->size)){
            dir->size = old_size;
            
            if (use_path_in_command){
                append(command, '"');
                append(command, *dir);
                append(command, command_name);
                append(command, '"');
            }
            else{
                append_ss(command, command_name);
            }
            
            char space[512];
            String message = make_fixed_width_string(space);
            append_ss(&message, make_lit_string("Building with: "));
            append_ss(&message, *command);
            append_s_char(&message, '\n');
            print_message(app, message.str, message.size);
            
            if (global_config.automatically_save_changes_on_build){
                save_all_dirty_buffers(app);
            }
            
            exec_system_command(app, view, buffer_identifier(literal("*compilation*")), dir->str, dir->size, command->str, command->size, CLI_OverlapWithConflict);
            result = true;
            break;
        }
        dir->size = old_size;
        
        if (directory_cd(app, dir->str, &dir->size, dir->memory_size, literal("..")) == 0){
            if (perform_backup){
                dir->size = directory_get_hot(app, dir->str, dir->memory_size);
                char backup_space[256];
                String backup_command = make_fixed_width_string(backup_space);
                append_ss(&backup_command, make_lit_string("echo could not find "));
                append_ss(&backup_command, filename);
                exec_system_command(app, view, buffer_identifier(literal("*compilation*")), dir->str, dir->size, backup_command.str, backup_command.size, CLI_OverlapWithConflict);
            }
            break;
        }
    }
    
    return(result);
}

#if defined(IS_WINDOWS)

// NOTE(allen): Build search rule for windows.
static int32_t
execute_standard_build_search(Application_Links *app, View_Summary *view,
                              Buffer_Summary *active_buffer,
                              String *dir, String *command, int32_t perform_backup){
    int32_t result = standard_build_search(app, view, active_buffer, dir, command, perform_backup, true, make_lit_string("build.bat"), make_lit_string("build"));
    return(result);
}

#elif defined(IS_LINUX) || defined(IS_MAC)

// NOTE(allen): Build search rule for linux and mac.
static int32_t
execute_standard_build_search(Application_Links *app, View_Summary *view, Buffer_Summary *active_buffer, String *dir, String *command, bool32 perform_backup){
    char dir_space[512];
    String dir_copy = make_fixed_width_string(dir_space);
    copy(&dir_copy, *dir);
    
    int32_t result = standard_build_search(app, view, active_buffer, dir, command, 0, 1, make_lit_string("build.sh"), make_lit_string("build.sh"));
    
    if (!result){
        result = standard_build_search(app, view, active_buffer, &dir_copy, command, perform_backup, 0, make_lit_string("Makefile"), make_lit_string("make"));
    }
    
    return(result);
}

#else
# error No build search rule for this platform.
#endif

// NOTE(allen): This searches first using the active file's directory,
// then if no build script is found, it searches from 4coders hot directory.
static void
execute_standard_build(Application_Links *app, View_Summary *view, Buffer_Summary *active_buffer){
    char dir_space[512];
    String dir = make_fixed_width_string(dir_space);
    
    char command_str_space[512];
    String command = make_fixed_width_string(command_str_space);
    
    int32_t build_dir_type = get_build_directory(app, active_buffer, &dir);
    
    if (build_dir_type == BuildDir_AtFile){
        if (!execute_standard_build_search(app, view, active_buffer, &dir, &command, false)){
            dir.size = 0;
            command.size = 0;
            build_dir_type = get_build_directory(app, 0, &dir);
        }
    }
    
    if (build_dir_type == BuildDir_AtHot){
        execute_standard_build_search(app, view, active_buffer, &dir, &command, true);
    }
}

CUSTOM_COMMAND_SIG(build_search)
CUSTOM_DOC("Looks for a build.bat, build.sh, or makefile in the current and parent directories.  Runs the first that it finds and prints the output to *compilation*.")
{
    uint32_t access = AccessAll;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    execute_standard_build(app, &view, &buffer);
    memset(&prev_location, 0, sizeof(prev_location));
    lock_jump_buffer(literal("*compilation*"));
}

#define GET_COMP_BUFFER(app) get_buffer_by_name(app, literal("*compilation*"), AccessAll)

static View_Summary
get_or_open_build_panel(Application_Links *app){
    View_Summary view = {};
    Buffer_Summary buffer = GET_COMP_BUFFER(app);
    if (buffer.exists){
        view = get_first_view_with_buffer(app, buffer.buffer_id);
    }
    if (!view.exists){
        view = open_build_footer_panel(app);
    }
    return(view);
}

static void
set_fancy_compilation_buffer_font(Application_Links *app){
    Buffer_Summary comp_buffer = GET_COMP_BUFFER(app);
    set_buffer_face_by_name(app, &comp_buffer, literal("Inconsolata"));
}

CUSTOM_COMMAND_SIG(build_in_build_panel)
CUSTOM_DOC("Looks for a build.bat, build.sh, or makefile in the current and parent directories.  Runs the first that it finds and prints the output to *compilation*.  Puts the *compilation* buffer in a panel at the footer of the current view.")
{
    uint32_t access = AccessAll;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    View_Summary build_view = get_or_open_build_panel(app);
    
    execute_standard_build(app, &build_view, &buffer);
    set_fancy_compilation_buffer_font(app);
    
    memset(&prev_location, 0, sizeof(prev_location));
    lock_jump_buffer(literal("*compilation*"));
}

CUSTOM_COMMAND_SIG(close_build_panel)
CUSTOM_DOC("If the special build panel is open, closes it.")
{
    close_build_footer_panel(app);
}

CUSTOM_COMMAND_SIG(change_to_build_panel)
CUSTOM_DOC("If the special build panel is open, makes the build panel the active panel.")
{
    View_Summary view = get_or_open_build_panel(app);
    if (view.exists){
        set_active_view(app, &view);
    }
}

// BOTTOM

