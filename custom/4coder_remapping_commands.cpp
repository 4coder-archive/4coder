/*
4coder_remapping_commands.cpp - Commands that remap all of the keys to one of the maps
in the set of default maps.
*/

// TOP

#if !defined(FCODER_REMAPPING_COMMANDS_CPP)
#define FCODER_REMAPPING_COMMANDS_CPP

//
// Buffer Filling Helpers
//

#include "generated/remapping.h"

void
default_keys(Bind_Helper *context){
    fill_keys_default(context);
    fill_log_graph_command_map(context);
}

void
mac_default_keys(Bind_Helper *context){
    fill_keys_mac_default(context);
    fill_log_graph_command_map(context);
}


//
// Remapping Commands
//

static Bind_Helper
get_context_on_arena(Arena *arena){
    Bind_Helper result = {};
    result = begin_bind_helper(push_array(arena, char, MB(1)), MB(1));
    return(result);
}

CUSTOM_COMMAND_SIG(set_bindings_choose)
CUSTOM_DOC("Remap keybindings using the 'choose' mapping rule.")
{
#if OS_WINDOWS || OS_LINUX
    set_bindings_default(app);
#elif OS_MAC
    set_bindings_mac_default(app);
#endif
}

CUSTOM_COMMAND_SIG(set_bindings_default)
CUSTOM_DOC("Remap keybindings using the 'default' mapping rule.")
{
    Scratch_Block scratch(app, Scratch_Share);
    Bind_Helper context = get_context_on_arena(scratch);
    set_all_default_hooks(&context);
    default_keys(&context);
    Bind_Buffer result = end_bind_helper_get_buffer(&context);
    global_set_mapping(app, result.data, result.size);
}

CUSTOM_COMMAND_SIG(set_bindings_mac_default)
CUSTOM_DOC("Remap keybindings using the 'mac-default' mapping rule.")
{
    Scratch_Block scratch(app, Scratch_Share);
    Bind_Helper context = get_context_on_arena(scratch);
    set_all_default_hooks(&context);
    mac_default_keys(&context);
    Bind_Buffer result = end_bind_helper_get_buffer(&context);
    global_set_mapping(app, result.data, result.size);
}

#endif

// BOTTOM
