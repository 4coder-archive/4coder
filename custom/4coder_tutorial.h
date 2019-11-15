/*
4coder_tutorial.h - Guided graphical tutorial system.
*/

// TOP

#if !defined(FCODER_TUTORIAL_H)
#define FCODER_TUTORIAL_H

typedef i32 Tutorial_Action;
enum{
    TutorialAction_None,
    TutorialAction_Minimize,
    TutorialAction_Maximize,
    TutorialAction_Prev,
    TutorialAction_Next,
    TutorialAction_Exit,
    TutorialAction_Restart,
};

struct Tutorial_Slide{
    Fancy_Block long_details;
    Fancy_Line short_details;
};

typedef Tutorial_Slide Tutorial_Slide_Function(Application_Links *app, Arena *arena);

struct Tutorial_State{
    b32 in_tutorial;
    View_ID view;
    Face_ID face;
    b32 is_active;
    Tutorial_Action hover_action;
    Tutorial_Action depressed_action;
    
    i32 slide_index;
    Tutorial_Slide_Function **slide_func_ptrs;
    i32 slide_count;
};

#endif

// BOTTOM
