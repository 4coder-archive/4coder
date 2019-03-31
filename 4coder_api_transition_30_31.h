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

#endif

// BOTTOM

