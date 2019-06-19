/*
 * Helpers for the API transition from 4.0.30 to 4.0.31
 *
 * In order to keep your layer on the old API you don't have to do anything, this provides wrappers
 *  idential to the 4.0.30 API.
 * In order to transition your entire layer over to the 4.0.31 API define 'REMOVE_TRANSITION_HELPER_31' and fix errors.
 * Or you can do it step by step by removing a few wrappers at a time.
 * This transition helper will be removed in a future version so it is recommended to get off sooner or laster.
 *
 * Tips on transitioning:
*
* Wrather than just try to inline this code everywhere, you can simplify things quite a lot by storing references
* to buffers and views and Buffer_ID and View_ID instead of Buffer_Summary and View_Summary.
 * Just get the summaries when you need information in those structures.
 *
 * You will make your code simpler if you stick to String as much as possible, but whenever you want to you can switch
 * to any string type you have to String by calling make_string(char_ptr, length) or make_string_slowly(null_terminated_c_str).
 * To pull the char ptr and length out of a String named "string": string.str and str.size.
 * If you need a null terminated string from a String use get_null_terminated in 4coder_helper.cpp
 *
 */

// TOP

#if !defined(REMOVE_TRANSITION_HELPER_31)

#define BatchEdit_PreserveTokens 0
#define BatchEdit_Normal 0

/* DOC(Buffer_Summary acts as a handle to a buffer and describes the state of the buffer.)
DOC_SEE(Access_Flag)
DOC_SEE(Dirty_State) */
STRUCT Buffer_Summary{
    /* DOC(This field indicates whether the Buffer_Summary describes a buffer that is open in 4coder. When this field is false the summary is referred to as a "null summary".) */
    b32 exists;
    /* DOC(If this is not a null summary, this field indicates whether the buffer has finished loading.) */
    b32 ready;
    /* DOC(If this is not a null summary this field is the id of the associated buffer. If this is a null summary then buffer_id is 0.) */
    Buffer_ID buffer_id;
    /* DOC(If this is not a null summary, this field contains flags describing the protection status of the buffer.) */
    Access_Flag lock_flags;
    
    /* DOC(If this is not a null summary, this field specifies the number of bytes in the buffer.) */
    i32 size;
    /* DOC(If this is not a null summary, this field specifies the number of lines in the buffer.) */
    i32 line_count;
    
    /* DOC(If this is not a null summary, this field specifies the file name associated to this buffer.) */
    char file_name[256];
    /* DOC(This field specifies the length of the file_name string.) */
    i32 file_name_len;
    
    /* DOC(If this is not a null summary, this field specifies the name of the buffer.) */
    char buffer_name[256];
    /* DOC(This field specifies the length of the buffer_name string.) */
    i32 buffer_name_len;
    
    /* DOC(This field indicates the dirty state of the buffer.) */
    Dirty_State dirty;
    
    /* DOC(If this is not a null summary, this field indicates whether the buffer is set to lex tokens.) */
    b32 is_lexed;
    /* DOC(If this is not a null summary, this field indicates whether the buffer has up to date tokens available. If this field is false, it may simply mean the tokens are still being generated in a background task and will be available later.  If that is the case, is_lexed will be true to indicate that the buffer is trying to get it's tokens up to date.) */
    b32 tokens_are_ready;
    /* DOC(If this is not a null summary, this field specifies the id of the command map for this buffer.) */
    i32 map_id;
    /* DOC(If this is not a null summary, this field indicates whether the buffer 'prefers' wrapped lines.) */
    b32 unwrapped_lines;
};

/* DOC(View_Summary acts as a handle to a view and describes the state of the view.)
DOC_SEE(Access_Flag)
DOC_SEE(Full_Cursor)
DOC_SEE(GUI_Scroll_Vars) */
STRUCT View_Summary{
    /* DOC(This field indicates whether the View_Summary describes a view that is open in 4coder. When this field is false the summary is referred to as a "null summary". ) */
    b32 exists;
    /* DOC(This field is the id of the associated view. If this is a null summary then view_id is 0. ) */
    i32 view_id;
    /* DOC(Then this is the id of the buffer this view currently sees.) */
    i32 buffer_id;
    /* DOC(This field contains flags describing the protection status of the view.) */
    Access_Flag lock_flags;
    
    /* DOC(This describes the position of the cursor.) */
    Full_Cursor cursor;
    /* DOC(This describes the position of the mark.) */
    Full_Cursor mark;
    /* DOC(This is the x position that is maintained in vertical navigation.) */
    f32 preferred_x;
    /* DOC(This specifies the height of a line rendered in the view.) */
    f32 line_height;
    /* DOC(This indicates that the view is set to render with unwrapped lines.) */
    b32 unwrapped_lines;
    /* DOC(This indicates that the view is set to highlight white space.) */
    b32 show_whitespace;
    
    /* DOC(This describes the screen position in which this view is displayed.) */
    Rect_f32 view_region;
    /* DOC(TODO) */
    Rect_f32 render_region;
    /* DOC(This describes the scrolling position inside the view.) */
    GUI_Scroll_Vars scroll_vars;
};

/* DOC(A Seek_Boundary_Flag field specifies a set of "boundary" types used in seeks for the beginning or end of different types of words.) */
typedef u32 Seek_Boundary_Flag;
enum{
    BoundaryWhitespace   = 0x1,
    BoundaryToken        = 0x2,
    BoundaryAlphanumeric = 0x4,
    BoundaryCamelCase    = 0x8
};

// NOTE(allen|4.0.31): Stream_Tokens has been deprecated in favor of the Token_Iterator.
// For examples of usage: 4coder_function_list.cpp 4coder_scope_commands.cpp
// If you want to keep your code working easily uncomment the typedef for Stream_Tokens.
struct Stream_Tokens_DEP{
    Application_Links *app;
    Buffer_ID buffer_id;
    
    Cpp_Token *base_tokens;
    Cpp_Token *tokens;
    i32 start;
    i32 end;
    i32 count;
    i32 token_count;
};
//typedef Stream_Tokens_DEP Stream_Tokens;

#endif

// BOTTOM

