/*
 * Miscellaneous helpers for common operations.
 */

#if !defined(FCODER_HELPER_H)
#define FCODER_HELPER_H

#define dll_remove(n)        (n)->next->prev=(n)->prev,(n)->prev->next=(n)->next

#define zdll_push_back_(f,l,n) if(f==0){n->next=n->prev=0;f=l=n;}else{n->prev=l;n->next=0;l->next=n;l=n;}
#define zdll_push_back(f,l,n) do{ zdll_push_back_((f),(l),(n)) }while(0)
#define zdll_remove_front_(f,l,n) if(f==l){f=l=0;}else{f=f->next;f->prev=0;}
#define zdll_remove_back_(f,l,n) if(f==l){f=l=0;}else{l=l->prev;l->next=0;}
#define zdll_remove_(f,l,n) if(f==n){zdll_remove_front_(f,l,n);}else if(l==n){zdll_remove_back_(f,l,n);}else{dll_remove(n);}
#define zdll_remove(f,l,n) do{ zdll_remove_((f),(l),(n)) }while(0)

#define Member(S,m) (((S*)0)->m)
#define PtrDif(a,b) ((uint8_t*)(a) - (uint8_t*)(b))
#define PtrAsInt(a) PtrDif(a,0)
#define OffsetOfMember(S,m) PtrAsInt(&Member(S,m))
#define CastFromMember(S,m,ptr) (S*)( (uint8_t*)(ptr) - OffsetOfMember(S,m) )

inline float
hexfloat(uint32_t x){
    union{
        uint32_t x;
        float f;
    } c;
    c.x = x;
    return(c.f);
}

static const float max_f32 = hexfloat(0x7f800000);

#include "4coder_seek_types.h"
#include "4coder_lib/4coder_utf8.h"

static void
exec_command(Application_Links *app, Custom_Command_Function *func){
    func(app);
}

static void
exec_command(Application_Links *app, Generic_Command cmd){
    if (cmd.cmdid < cmdid_count){
        exec_command(app, cmd.cmdid);
    }
    else{
        exec_command(app, cmd.command);
    }
}

static int32_t
key_is_unmodified(Key_Event_Data *key){
    int8_t *mods = key->modifiers;
    int32_t unmodified = (!mods[MDFR_CONTROL_INDEX] && !mods[MDFR_ALT_INDEX]);
    return(unmodified);
}

static uint32_t
to_writable_character(User_Input in, uint8_t *character){
    uint32_t result = 0;
    if (in.type == UserInputKey){
        if (in.key.character != 0){
            u32_to_utf8_unchecked(in.key.character, character, &result);
        }
    }
    return(result);
}

static uint32_t
to_writable_character(Key_Event_Data key, uint8_t *character){
    uint32_t result = 0;
    if (key.character != 0){
        u32_to_utf8_unchecked(key.character, character, &result);
    }
    return(result);
}

static bool32
backspace_utf8(String *str){
    bool32 result = false;
    uint8_t *s = (uint8_t*)str->str;
    if (str->size > 0){
        uint32_t i = str->size-1;
        for (; i > 0; --i){
            if (s[i] <= 0x7F || s[i] >= 0xC0){
                break;
            }
        }
        str->size = i;
        result = true;
    }
    return(result);
}

static bool32
query_user_general(Application_Links *app, Query_Bar *bar, bool32 force_number){
    bool32 success = true;
    
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
        User_Input in = get_user_input(app, EventOnAnyKey, EventOnEsc | EventOnButton);
        
        // NOTE(allen|a3.4.4): The responsible thing to do on abort is to end the command
        // without waiting on get_user_input again.
        if (in.abort){
            success = false;
            break;
        }
        
        uint8_t character[4];
        uint32_t length = 0;
        bool32 good_character = false;
        if (key_is_unmodified(&in.key)){
            if (force_number){
                if (in.key.character >= '0' && in.key.character <= '9'){
                    good_character = true;
                    length = to_writable_character(in, character);
                }
            }
            else{
                length = to_writable_character(in, character);
                if (length != 0){
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
                backspace_utf8(&bar->string);
            }
            else if (good_character){
                append_ss(&bar->string, make_string(character, length));
            }
        }
    }
    
    terminate_with_null(&bar->string);
    
    return(success);
}

static int32_t
query_user_string(Application_Links *app, Query_Bar *bar){
    int32_t success = query_user_general(app, bar, false);
    return(success);
}

static int32_t
query_user_number(Application_Links *app, Query_Bar *bar){
    int32_t success = query_user_general(app, bar, true);
    return(success);
}

static void
init_theme_zero(Theme *theme){
    for (int32_t i = 0; i < Stag_COUNT; ++i){
        theme->colors[i] = 0;
    }
}

static char
buffer_get_char(Application_Links *app, Buffer_Summary *buffer, int32_t pos){
    char result = ' ';
    *buffer = get_buffer(app, buffer->buffer_id, AccessAll);
    if (pos < buffer->size){
        buffer_read_range(app, buffer, pos, pos+1, &result);
    }
    return(result);
}

static Buffer_Identifier
buffer_identifier(char *str, int32_t len){
    Buffer_Identifier identifier;
    identifier.name = str;
    identifier.name_len = len;
    identifier.id = 0;
    return(identifier);
}

static Buffer_Identifier
buffer_identifier(Buffer_ID id){
    Buffer_Identifier identifier;
    identifier.name = 0;
    identifier.name_len = 0;
    identifier.id = id;
    return(identifier);
}

static Range
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

// TODO(allen): Setup buffer seeking to do character_pos and get View_Summary out of this parameter list.
static int32_t
character_pos_to_pos(Application_Links *app, View_Summary *view, Buffer_Summary *buffer, int32_t character_pos){
    int32_t result = 0;
    Full_Cursor cursor = {0};
    if (view_compute_cursor(app, view, seek_character_pos(character_pos), &cursor)){
        result = cursor.pos;
    }
    return(result);
}

struct Buffer_Rect{
    int32_t char0, line0;
    int32_t char1, line1;
};

#ifndef Swap
# define Swap(T,a,b) do{ T t = a; a = b; b = t; } while(0)
#endif

static Buffer_Rect
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

static i32_Rect
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

static View_Summary
get_first_view_with_buffer(Application_Links *app, int32_t buffer_id){
    View_Summary result = {0};
    View_Summary test = {0};
    
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

static bool32
open_file(Application_Links *app, Buffer_Summary *buffer_out, char *filename, int32_t filename_len, bool32 background, bool32 never_new){
    bool32 result = false;
    Buffer_Summary buffer = get_buffer_by_name(app, filename, filename_len, AccessProtected|AccessHidden);
    
    if (buffer.exists){
        if (buffer_out){
            *buffer_out = buffer;
        }
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
            if (buffer_out){
                *buffer_out = buffer;
            }
            result = true;
        }
    }
    
    return(result);
}

static Buffer_ID
buffer_identifier_to_id(Application_Links *app, Buffer_Identifier identifier){
    Buffer_ID id = 0;
    if (identifier.id != 0){
        id = identifier.id;
    }
    else{
        Buffer_Summary buffer = get_buffer_by_name(app, identifier.name, identifier.name_len, AccessAll);
        id = buffer.buffer_id;
        if (id == 0){
            buffer = get_buffer_by_file_name(app, identifier.name, identifier.name_len, AccessAll);
            id = buffer.buffer_id;
        }
    }
    return(id);
}

static bool32
view_open_file(Application_Links *app, View_Summary *view, char *filename, int32_t filename_len, bool32 never_new){
    bool32 result = false;
    
    if (view != 0){
        Buffer_Summary buffer = {0};
        if (open_file(app, &buffer, filename, filename_len, false, never_new)){
            view_set_buffer(app, view, buffer.buffer_id, 0);
            result = true;
        }
    }
    
    return(result);
}

static void
get_view_prev(Application_Links *app, View_Summary *view, uint32_t access){
    if (view->exists){
        View_ID original_id = view->view_id;
        View_ID check_id = original_id;
        
        View_Summary new_view = {0};
        
        for (;;){
            --check_id;
            if (check_id <= 0){
                check_id = 16;
            }
            if (check_id == original_id){
                new_view = *view;
                break;
            }
            new_view = get_view(app, check_id, access);
            if (new_view.exists){
                break;
            }
        }
        
        *view = new_view;
    }
}

static View_Summary
get_view_last(Application_Links *app, uint32_t access){
    View_Summary view = {0};
    view.exists = true;
    get_view_prev(app, &view, access);
    if (view.view_id < 1 || view.view_id > 16){
        view = null_view_summary;
    }
    return(view);
}

static void
get_view_next_looped(Application_Links *app, View_Summary *view, uint32_t access){
    get_view_next(app, view, access);
    if (!view->exists){
        *view = get_view_first(app, access);
    }
}

static void
get_view_prev_looped(Application_Links *app, View_Summary *view, uint32_t access){
    get_view_prev(app, view, access);
    if (!view->exists){
        *view = get_view_last(app, access);
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

static float
get_view_y(View_Summary *view){
    float y = view->cursor.wrapped_y;
    if (view->unwrapped_lines){
        y = view->cursor.unwrapped_y;
    }
    return(y);
}

static float
get_view_x(View_Summary *view){
    float x = view->cursor.wrapped_x;
    if (view->unwrapped_lines){
        x = view->cursor.unwrapped_x;
    }
    return(x);
}

static Range
get_range(View_Summary *view){
    Range range = make_range(view->cursor.pos, view->mark.pos);
    return(range);
}

#if !defined(ArrayCount)
# define ArrayCount(a) (sizeof(a)/sizeof(a[0]))
#endif

#endif
