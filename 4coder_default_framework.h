/*
4coder_default_framework.cpp - Sets up the basics of the framework that is used 
for default 4coder behaviour.

TYPE: 'internal-for-default-system'
*/

// TOP

#if !defined(FCODER_DEFAULT_FRAMEWORK_H)
#define FCODER_DEFAULT_FRAMEWORK_H

#include "4coder_helper/4coder_helper.h"
#include "4coder_lib/4coder_mem.h"

//
// Global Memory
//

static Partition global_part;
static General_Memory global_general;

//
// Jump Buffer Locking
//

static char locked_buffer_space[256];
static String locked_buffer = make_fixed_width_string(locked_buffer_space);

static void
unlock_jump_buffer(){
    locked_buffer.size = 0;
}

static void
lock_jump_buffer(char *name, int32_t size){
    copy(&locked_buffer, make_string(name, size));
}

static void
lock_jump_buffer(Buffer_Summary buffer){
    copy(&locked_buffer, make_string(buffer.buffer_name, buffer.buffer_name_len));
}

static View_Summary
get_view_for_locked_jump_buffer(Application_Links *app){
    View_Summary view = {0};
    
    if (locked_buffer.size > 0){
        Buffer_Summary buffer = get_buffer_by_name(app, locked_buffer.str, locked_buffer.size, AccessAll);
        if (buffer.exists){
            view = get_first_view_with_buffer(app, buffer.buffer_id);
        }
        else{
            unlock_jump_buffer();
        }
    }
    
    return(view);
}

//
// Panel Management
//

static View_ID special_note_view_id = 0;

static void
close_special_note_view(Application_Links *app){
    View_Summary special_view = get_view(app, special_note_view_id, AccessAll);
    if (special_view.exists){
        close_view(app, &special_view);
    }
    special_note_view_id = 0;
}

static View_Summary
open_special_note_view(Application_Links *app, bool32 create_if_not_exist = true){
    View_Summary special_view = get_view(app, special_note_view_id, AccessAll);
    
    if (create_if_not_exist && !special_view.exists){
        View_Summary view = get_active_view(app, AccessAll);
        special_view = open_view(app, &view, ViewSplit_Bottom);
        view_set_setting(app, &special_view, ViewSetting_ShowScrollbar, false);
        view_set_split_proportion(app, &special_view, .2f);
        set_active_view(app, &view);
        special_note_view_id = special_view.view_id;
    }
    
    return(special_view);
}

CUSTOM_COMMAND_SIG(change_active_panel){
    View_Summary view = get_active_view(app, AccessAll);
    View_ID original_view_id = view.view_id;
    
    do{
        get_view_next_looped(app, &view, AccessAll);
        if (view.view_id != special_note_view_id){
            break;
        }
    }while(view.view_id != original_view_id);
    
    if (view.exists){
        set_active_view(app, &view);
    }
}

//
// View Variabls
//

enum Rewrite_Type{
    RewriteNone,
    RewritePaste,
    RewriteWordComplete
};

struct View_Paste_Index{
    int32_t rewrite;
    int32_t next_rewrite;
    int32_t index;
};

View_Paste_Index view_paste_index_[16];
View_Paste_Index *view_paste_index = view_paste_index_ - 1;

#endif

// BOTTOM

