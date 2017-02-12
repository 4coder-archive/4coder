/*
 * Miscellaneous helpers for common operations.
 */

#if !defined(FCODER_HELPER_H)
#define FCODER_HELPER_H

#include "4coder_seek_types.h"

inline void
exec_command(Application_Links *app, Custom_Command_Function *func){
    func(app);
}

inline void
exec_command(Application_Links *app, Generic_Command cmd){
    if (cmd.cmdid < cmdid_count){
        exec_command(app, cmd.cmdid);
    }
    else{
        exec_command(app, cmd.command);
    }
}

inline View_Summary
get_first_view_with_buffer(Application_Links *app, int32_t buffer_id){
    View_Summary result = {};
    View_Summary test = {};
    
    if (buffer_id != 0){
        uint32_t access = AccessAll;
        for(test = get_view_first(app, access);
            test.exists;
            get_view_next(app, &test, access)){
            
            Buffer_Summary buffer = get_buffer(app, test.buffer_id, access);
            
            if(buffer.buffer_id == buffer_id){
                result = test;
                break;
            }
        }
    }
    
    return(result);
}

inline int32_t
key_is_unmodified(Key_Event_Data *key){
    char *mods = key->modifiers;
    int32_t unmodified = !mods[MDFR_CONTROL_INDEX] && !mods[MDFR_ALT_INDEX];
    return(unmodified);
}

static char
to_writable_char(Key_Code long_character){
    char character = 0;
    if (long_character < ' '){
        if (long_character == '\n'){
            character = '\n';
        }
        else if (long_character == '\t'){
            character = '\t';
        }
    }
    else if (long_character >= ' ' && long_character <= 255 && long_character != 127){
        character = (char)long_character;
    }
    return(character);
}

static int32_t
query_user_general(Application_Links *app, Query_Bar *bar, bool32 force_number){
    User_Input in;
    int32_t success = 1;
    
    // NOTE(allen|a3.4.4): It will not cause an *error* if we continue on after failing to.
    // start a query bar, but it will be unusual behavior from the point of view of the
    // user, if this command starts intercepting input even though no prompt is shown.
    // This will only happen if you have a lot of bars open already or if the current view
    // doesn't support query bars.
    if (start_query_bar(app, bar, 0) == 0) return 0;
    
    for (;;){
        // NOTE(allen|a3.4.4): This call will block until the user does one of the input
        // types specified in the flags.  The first set of flags are inputs you'd like to intercept
        // that you don't want to abort on.  The second set are inputs that you'd like to cause
        // the command to abort.  If an event satisfies both flags, it is treated as an abort.
        in = get_user_input(app, EventOnAnyKey, EventOnEsc | EventOnButton);
        
        // NOTE(allen|a3.4.4): The responsible thing to do on abort is to end the command
        // without waiting on get_user_input again.
        if (in.abort){
            success = 0;
            break;
        }
        
        char character = 0;
        bool32 good_character = false;
        if (key_is_unmodified(&in.key)){
            if (force_number){
                if (in.key.character >= '0' && in.key.character <= '9'){
                    good_character = true;
                    character = (char)(in.key.character);
                }
            }
            else{
                character = to_writable_char(in.key.character);
                if (character != 0){
                    good_character = true;
                }
            }
        }
        
        // NOTE(allen|a3.4.4): All we have to do to update the query bar is edit our
        // local Query_Bar struct!  This is handy because it means our Query_Bar
        // is always correct for typical use without extra work updating the bar.
        if (in.type == UserInputKey){
            if (in.key.keycode == '\n' || in.key.keycode == '\t'){
                break;
            }
            else if (in.key.keycode == key_back){
                if (bar->string.size > 0){
                    --bar->string.size;
                }
            }
            else if (good_character){
                append_s_char(&bar->string, character);
            }
        }
    }
    
    terminate_with_null(&bar->string);
    
    return(success);
}

inline int32_t
query_user_string(Application_Links *app, Query_Bar *bar){
    int32_t success = query_user_general(app, bar, false);
    return(success);
}

inline int32_t
query_user_number(Application_Links *app, Query_Bar *bar){
    int32_t success = query_user_general(app, bar, true);
    return(success);
}

inline char
buffer_get_char(Application_Links *app, Buffer_Summary *buffer, int32_t pos){
    char result = ' ';
    *buffer = get_buffer(app, buffer->buffer_id, AccessAll);
    if (pos >= 0 && pos < buffer->size){
        buffer_read_range(app, buffer, pos, pos+1, &result);
    }
    return(result);
}

inline Buffer_Identifier
buffer_identifier(char *str, int32_t len){
    Buffer_Identifier identifier;
    identifier.name = str;
    identifier.name_len = len;
    identifier.id = 0;
    return(identifier);
}

inline Buffer_Identifier
buffer_identifier(int32_t id){
    Buffer_Identifier identifier;
    identifier.name = 0;
    identifier.name_len = 0;
    identifier.id = id;
    return(identifier);
}

static Buffer_Summary
create_buffer(Application_Links *app, char *filename, int32_t filename_len, Buffer_Create_Flag flags){
    Buffer_Summary buffer = {0};
    
    Buffer_Creation_Data data = {0};
    begin_buffer_creation(app, &data, flags);
    buffer_creation_name(app, &data, filename, filename_len, 0);
    buffer = end_buffer_creation(app, &data);
    
    return(buffer);
}

inline Range
make_range(int32_t p1, int32_t p2){
    Range range;
    if (p1 < p2){
        range.min = p1;
        range.max = p2;
    }
    else{
        range.min = p2;
        range.max = p1;
    }
    return(range);
}

static void
adjust_all_buffer_wrap_widths(Application_Links *app, int32_t wrap_width, int32_t min_base_width){
    for (Buffer_Summary buffer = get_buffer_first(app, AccessAll);
         buffer.exists;
         get_buffer_next(app, &buffer, AccessAll)){
        buffer_set_setting(app, &buffer, BufferSetting_WrapPosition, wrap_width);
        buffer_set_setting(app, &buffer, BufferSetting_MinimumBaseWrapPosition, min_base_width);
    }
}

struct Buffer_Rect{
    int32_t char0, line0;
    int32_t char1, line1;
};

#ifndef Swap
#define Swap(T,a,b) do{ T t = a; a = b; b = t; } while(0)
#endif

inline Buffer_Rect
get_rect(View_Summary *view){
    Buffer_Rect rect = {0};
    
    rect.char0 = view->mark.character;
    rect.line0 = view->mark.line;
    
    rect.char1 = view->cursor.character;
    rect.line1 = view->cursor.line;
    
    if (rect.line0 > rect.line1){
        Swap(int32_t, rect.line0, rect.line1);
    }
    if (rect.char0 > rect.char1){
        Swap(int32_t, rect.char0, rect.char1);
    }
    
    return(rect);
}

inline i32_Rect
get_line_x_rect(View_Summary *view){
    i32_Rect rect = {0};
    
    if (view->unwrapped_lines){
        rect.x0 = (int32_t)view->mark.unwrapped_x;
        rect.x1 = (int32_t)view->cursor.unwrapped_x;
    }
    else{
        rect.x0 = (int32_t)view->mark.wrapped_x;
        rect.x1 = (int32_t)view->cursor.wrapped_x;
    }
    rect.y0 = view->mark.line;
    rect.y1 = view->cursor.line;
    
    if (rect.y0 > rect.y1){
        Swap(int32_t, rect.y0, rect.y1);
    }
    if (rect.x0 > rect.x1){
        Swap(int32_t, rect.x0, rect.x1);
    }
    
    return(rect);
}

static bool32
open_file(Application_Links *app, Buffer_Summary *buffer_out, char *filename, int32_t filename_len, bool32 background, bool32 never_new){
    bool32 result = false;
    Buffer_Summary buffer = get_buffer_by_name(app, filename, filename_len, AccessProtected|AccessHidden);
    
    if (buffer.exists){
        if (buffer_out) *buffer_out = buffer;
        result = true;
    }
    else{
        Buffer_Create_Flag flags = 0;
        if (background){
            flags |= BufferCreate_Background;
        }
        if (never_new){
            flags |= BufferCreate_NeverNew;
        }
        buffer = create_buffer(app, filename, filename_len, flags);
        if (buffer.exists){
            if (buffer_out) *buffer_out = buffer;
            result = true;
        }
    }
    
    return(result);
}

static int32_t
view_open_file(Application_Links *app, View_Summary *view, char *filename, int32_t filename_len, int32_t never_new){
    int32_t result = 0;
    
    if (view){
        Buffer_Summary buffer = {0};
        if (open_file(app, &buffer, filename, filename_len, false, never_new)){
            view_set_buffer(app, view, buffer.buffer_id, 0);
            result = 1;
        }
    }
    
    return(result);
}

static void
get_view_next_looped(Application_Links *app, View_Summary *view, uint32_t access){
    get_view_next(app, view, access);
    if (!view->exists){
        *view = get_view_first(app, access);
    }
}

static void
refresh_buffer(Application_Links *app, Buffer_Summary *buffer){
    *buffer = get_buffer(app, buffer->buffer_id, AccessAll);
}

static void
refresh_view(Application_Links *app, View_Summary *view){
    *view = get_view(app, view->view_id, AccessAll);
}

inline float
get_view_y(View_Summary *view){
    float y = view->cursor.wrapped_y;
    if (view->unwrapped_lines){
        y = view->cursor.unwrapped_y;
    }
    return(y);
}

inline float
get_view_x(View_Summary *view){
    float x = view->cursor.wrapped_x;
    if (view->unwrapped_lines){
        x = view->cursor.unwrapped_x;
    }
    return(x);
}

inline Range
get_range(View_Summary *view){
    Range range = make_range(view->cursor.pos, view->mark.pos);
    return(range);
}

#if !defined(ArrayCount)
# define ArrayCount(a) (sizeof(a)/sizeof(a[0]))
#endif

#endif
