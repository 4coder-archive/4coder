/*
4coder_jump_lister.h - Lister for jump buffers.
*/

// TOP

typedef i32 Jump_Lister_Activation_Rule;
enum{
    JumpListerActivation_OpenInUIView = 0,
    JumpListerActivation_OpenInTargetViewKeepUI = 1,
    JumpListerActivation_OpenInTargetViewCloseUI = 2,
    JumpListerActivation_OpenInNextViewKeepUI = 3,
    JumpListerActivation_OpenInNextViewCloseUI = 4,
};

struct Jump_Lister_Parameters{
    Buffer_ID list_buffer_id;
    Jump_Lister_Activation_Rule activation_rule;
    View_ID target_view_id;
};

// BOTTOM