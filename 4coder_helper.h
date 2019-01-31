/*
 * Miscellaneous helpers for common operations.
 */

// TOP

#if !defined(FCODER_HELPER_H)
#define FCODER_HELPER_H

// TODO(allen): Stop handling files this way!  My own API should be able to do this!!?!?!?!!?!?!!!!?
// NOTE(allen): Actually need binary buffers for some stuff to work, but not this parsing thing here.
#include <stdio.h>

struct Bind_Helper{
    Binding_Unit *cursor, *start, *end;
    Binding_Unit *header, *group;
    int32_t write_total;
    int32_t error;
};

#define BH_ERR_NONE 0
#define BH_ERR_MISSING_END 1
#define BH_ERR_MISSING_BEGIN 2
#define BH_ERR_OUT_OF_MEMORY 3

struct Bind_Buffer{
    void *data;
    int32_t size;
};

////////////////////////////////

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
#define HandleAsU64(a) (uint64_t)(PtrAsInt(a))
#define OffsetOfMember(S,m) PtrAsInt(&Member(S,m))
#define CastFromMember(S,m,ptr) (S*)( (uint8_t*)(ptr) - OffsetOfMember(S,m) )
#define IntAsPtr(a) (void*)(((uint8_t*)0) + a)

#if !defined(max_f32)
static float
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

#ifndef Swap
# define Swap(T,a,b) do{ T t = a; a = b; b = t; } while(0)
#endif

#define require(c) if (!(c)){ return(0); }

////////////////////////////////

struct File_Handle_Path{
    FILE *file;
    String path;
};

struct File_Name_Data{
    String file_name;
    String data;
};

struct File_Name_Path_Data{
    String file_name;
    String path;
    String data;
};

////////////////////////////////

struct Buffer_Rect{
    int32_t char0;
    int32_t line0;
    int32_t char1;
    int32_t line1;
};

////////////////////////////////

struct Stream_Chunk{
    Application_Links *app;
    Buffer_Summary *buffer;
    
    char *base_data;
    int32_t start, end;
    int32_t min_start, max_end;
    bool32 add_null;
    uint32_t data_size;
    
    char *data;
};

struct Stream_Tokens{
    Application_Links *app;
    Buffer_Summary *buffer;
    
    Cpp_Token *base_tokens;
    Cpp_Token *tokens;
    int32_t start, end;
    int32_t count, token_count;
};

////////////////////////////////

struct Sort_Pair_i32{
    int32_t index;
    int32_t key;
};

#endif

// BOTTOM