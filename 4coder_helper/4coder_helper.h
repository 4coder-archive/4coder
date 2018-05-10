/*
 * Miscellaneous helpers for common operations.
 */

// TOP

#if !defined(FCODER_HELPER_H)
#define FCODER_HELPER_H

#define dll_remove(n)        (n)->next->prev=(n)->prev,(n)->prev->next=(n)->next

#define zdll_push_back_(f,l,n) if(f==0){n->next=n->prev=0;f=l=n;}else{n->prev=l;n->next=0;l->next=n;l=n;}
#define zdll_push_back(f,l,n) do{ zdll_push_back_((f),(l),(n)) }while(0)
#define zdll_remove_front_(f,l,n) if(f==l){f=l=0;}else{f=f->next;f->prev=0;}
#define zdll_remove_back_(f,l,n) if(f==l){f=l=0;}else{l=l->prev;l->next=0;}
#define zdll_remove_(f,l,n) if(f==n){zdll_remove_front_(f,l,n);}else if(l==n){zdll_remove_back_(f,l,n);}else{dll_remove(n);}
#define zdll_remove(f,l,n) do{ zdll_remove_((f),(l),(n)) }while(0)

#if !defined(Member)
# define Member(S,m) (((S*)0)->m)
#endif
#define PtrDif(a,b) ((uint8_t*)(a) - (uint8_t*)(b))
#define PtrAsInt(a) PtrDif(a,0)
#define OffsetOfMember(S,m) PtrAsInt(&Member(S,m))
#define CastFromMember(S,m,ptr) (S*)( (uint8_t*)(ptr) - OffsetOfMember(S,m) )

#if !defined(max_f32)
inline float
max_f32_proc(void){
    union{
        uint32_t x;
        float f;
    } c;
    c.x = 0x7f800000;
    return(c.f);
}

#define max_f32 max_f32_proc()
#endif

#if !defined(ArrayCount)
# define ArrayCount(a) (sizeof(a)/sizeof(a[0]))
#endif

#include "4coder_seek_types.h"
#include "4coder_lib/4coder_utf8.h"

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

#endif

// BOTTOM