/*
 * 4coder_profile.cpp - Built in self profiling report.
 */

// TOP

#if !defined(FCODER_PROFILE_H)
#define FCODER_PROFILE_H

struct Profile_Block{
    Thread_Context *tctx;
    b32 is_closed;
    Profile_ID id;
    
    Profile_Block(Thread_Context *tctx, String_Const_u8 name, String_Const_u8 location);
    Profile_Block(Application_Links *app, String_Const_u8 name, String_Const_u8 location);
    ~Profile_Block();
    void close_now();
};

struct Profile_Scope_Block{
    Thread_Context *tctx;
    b32 is_closed;
    Profile_ID id;
    
    Profile_Scope_Block(Application_Links *app, String_Const_u8 name, String_Const_u8 location);
    Profile_Scope_Block(Thread_Context *tctx, String_Const_u8 name, String_Const_u8 location);
    ~Profile_Scope_Block();
    void close_now();
};

#endif

// BOTTOM

