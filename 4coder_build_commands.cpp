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
static i32
get_build_directory(Application_Links *app, Buffer_ID buffer, String *dir_out){
    Arena *scratch = context_get_arena(app);
    Temp_Memory_Arena temp = begin_temp_memory(scratch);
    
    i32 result = BuildDir_None;
    
    if (buffer != 0){
        String file_name = buffer_push_file_name(app, buffer, scratch);
        String base_name = buffer_push_base_buffer_name(app, buffer, scratch);
        if (!match(file_name, base_name)){
            remove_last_folder(&file_name);
            append(dir_out, file_name);
            result = BuildDir_AtFile;
        }
    }
    
    if (result == BuildDir_None){
        if (get_hot_directory(app, dir_out, 0)){
            result = BuildDir_AtHot;
        }
    }
    
    end_temp_memory(temp);
    
    return(result);
}

// TODO(allen): Better names for the "standard build search" family.
static i32
standard_build_search(Application_Links *app, View_ID view, String *dir, String *command, b32 perform_backup, b32 use_path_in_command, String filename, String command_name){
    i32 result = false;
    
    for(;;){
        i32 old_size = dir->size;
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
            
            exec_system_command(app, view, buffer_identifier(literal("*compilation*")), dir->str, dir->size, command->str, command->size, CLI_OverlapWithConflict|CLI_SendEndSignal);
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
                exec_system_command(app, view, buffer_identifier(literal("*compilation*")), dir->str, dir->size, backup_command.str, backup_command.size, CLI_OverlapWithConflict|CLI_SendEndSignal);
            }
            break;
        }
    }
    
    return(result);
}

#if defined(IS_WINDOWS)

// NOTE(allen): Build search rule for windows.
static i32
execute_standard_build_search(Application_Links *app, View_ID view, String *dir, String *command, i32 perform_backup){
    i32 result = standard_build_search(app, view, dir, command, perform_backup, true, make_lit_string("build.bat"), make_lit_string("build"));
    return(result);
}

#elif defined(IS_LINUX) || defined(IS_MAC)

// NOTE(allen): Build search rule for linux and mac.
static i32
execute_standard_build_search(Application_Links *app, View_ID view, String *dir, String *command, b32 perform_backup){
    char dir_space[512];
    String dir_copy = make_fixed_width_string(dir_space);
    copy(&dir_copy, *dir);
    i32 result = standard_build_search(app, view, dir, command, 0, 1, make_lit_string("build.sh"), make_lit_string("build.sh"));
    if (!result){
        result = standard_build_search(app, view, &dir_copy, command, perform_backup, 0, make_lit_string("Makefile"), make_lit_string("make"));
    }
    return(result);
}

#else
# error No build search rule for this platform.
#endif

// NOTE(allen): This searches first using the active file's directory,
// then if no build script is found, it searches from 4coders hot directory.
static void
execute_standard_build(Application_Links *app, View_ID view, Buffer_ID active_buffer){
    char dir_space[512];
    String dir = make_fixed_width_string(dir_space);
    char command_str_space[512];
    String command = make_fixed_width_string(command_str_space);
    i32 build_dir_type = get_build_directory(app, active_buffer, &dir);
    if (build_dir_type == BuildDir_AtFile){
        if (!execute_standard_build_search(app, view, &dir, &command, false)){
            dir.size = 0;
            command.size = 0;
            build_dir_type = get_build_directory(app, 0, &dir);
        }
    }
    if (build_dir_type == BuildDir_AtHot){
        execute_standard_build_search(app, view, &dir, &command, true);
    }
}

CUSTOM_COMMAND_SIG(build_search)
CUSTOM_DOC("Looks for a build.bat, build.sh, or makefile in the current and parent directories.  Runs the first that it finds and prints the output to *compilation*.")
{
    View_Summary view = get_active_view(app, AccessAll);
    Buffer_ID buffer = 0;
    view_get_buffer(app, view.view_id, AccessAll, &buffer);
    execute_standard_build(app, view.view_id, buffer);
    memset(&prev_location, 0, sizeof(prev_location));
    lock_jump_buffer(make_lit_string("*compilation*"));
}

#define GET_COMP_BUFFER(app,id) get_buffer_by_name(app, make_lit_string("*compilation*"), AccessAll, (id))

static View_ID
get_or_open_build_panel(Application_Links *app){
    View_ID view = 0;
    Buffer_ID buffer = 0;
    GET_COMP_BUFFER(app, &buffer);
    if (buffer != 0){
        view = get_first_view_with_buffer(app, buffer);
    }
    if (view != 0){
        open_build_footer_panel(app, &view);
    }
    return(view);
}

static void
set_fancy_compilation_buffer_font(Application_Links *app){
    Buffer_ID buffer = 0;
    GET_COMP_BUFFER(app, &buffer);
    set_buffer_face_by_name(app, buffer, literal("Inconsolata"));
}

CUSTOM_COMMAND_SIG(build_in_build_panel)
CUSTOM_DOC("Looks for a build.bat, build.sh, or makefile in the current and parent directories.  Runs the first that it finds and prints the output to *compilation*.  Puts the *compilation* buffer in a panel at the footer of the current view.")
{
    View_Summary view = get_active_view(app, AccessAll);
    Buffer_ID buffer = 0;
    view_get_buffer(app, view.view_id, AccessAll, &buffer);
    
    View_ID build_view = get_or_open_build_panel(app);
    
    execute_standard_build(app, build_view, buffer);
    set_fancy_compilation_buffer_font(app);
    
    memset(&prev_location, 0, sizeof(prev_location));
    lock_jump_buffer(make_lit_string("*compilation*"));
}

CUSTOM_COMMAND_SIG(close_build_panel)
CUSTOM_DOC("If the special build panel is open, closes it.")
{
    close_build_footer_panel(app);
}

CUSTOM_COMMAND_SIG(change_to_build_panel)
CUSTOM_DOC("If the special build panel is open, makes the build panel the active panel.")
{
    View_ID view = get_or_open_build_panel(app);
    if (view != 0){
        view_set_active(app, view);
    }
}

// BOTTOM

