/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 07.12.2019
 *
 * Documentation of the custom layer's primary api : buffer-related calls
 *
 */

// TOP

function void
doc_custom_api__buffer(Arena *arena, API_Definition *api_def, Doc_Cluster *cluster){
    Doc_Function func = {};
    
    if (begin_doc_call(arena, cluster, api_def, "get_buffer_count", &func)){
        doc_function_brief(arena, &func, "Retrieve the number of buffers loaded in the core");
        
        // params
        doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the number of buffers loaded in the core");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "get_buffer_next", &func)){
        doc_function_brief(arena, &func, "Iterate to the next buffer loaded in the core");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to iterate from, or zero to get the first buffer");
        
        doc_function_param(arena, &func, "access");
        doc_text(arena, params, "the type of access requirements used to filter on the buffers that are returned");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the id of the next buffer, or zero if there is no next buffer");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "get_buffer_by_name", &func)){
        doc_function_brief(arena, &func, "Retrieve a buffer querying by name");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "name");
        doc_text(arena, params, "the name to query against buffer names");
        
        doc_function_param(arena, &func, "access");
        doc_text(arena, params, "the type of access requirements used to determine whether or not to return a match");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the id of the buffer who's name agrees with name if it exists and it has the required access state, zero otherwise");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "get_buffer_by_file_name", &func)){
        doc_function_brief(arena, &func, "Retrieve a buffer querying by file name");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "file_name");
        doc_text(arena, params, "the name to query against buffer file names");
        
        doc_function_param(arena, &func, "access");
        doc_text(arena, params, "the type of access requirements used to determine whether or not to return a match");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the id of the buffer who's attached file name agrees with file_name if it exists and it has the required access state, zero otherwise");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_read_range", &func)){
        doc_function_brief(arena, &func, "Read a range of text out of a buffer");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to be read");
        
        doc_function_param(arena, &func, "range");
        doc_text(arena, params, "byte range in the buffer that will be read - the range is left inclusive right exclusive, for example the range [10,20) reads ten bytes, with the first byte read being the one with index 10.");
        
        doc_function_param(arena, &func, "out");
        doc_text(arena, params, "the buffer that will get the copy of the new text which should have at least range.max - range.min available bytes");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero on success, when the buffer exists and the range is completely contained within the buffer's contents, otherwise zero");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_replace_range", &func)){
        doc_function_brief(arena, &func, "Replace a range of text with a specific string");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to modify");
        
        doc_function_param(arena, &func, "range");
        doc_text(arena, params, "the range of bytes to replace - the range is left inclusive right exclusive, for example the range [10,20) replaces ten bytes and the byte at index 20 remains in the buffer, possibly shifted if the string is not ten bytes");
        
        doc_function_param(arena, &func, "string");
        doc_text(arena, params, "the new text to be placed in the given range");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero on success, when the buffer eixsts and the range is contained within the buffer, zero otherwise");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "This operation is used to implement inserts by setting the range to be empty, [x,x) inserts the string at x, and it implements deletion by setting the string to be empty.");
        
        doc_paragraph(arena, det);
        
        doc_text(arena, det, "All modifications made by this call are simultaneously saved onto the buffer's history if is enabled.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_batch_edit", &func)){
        doc_function_brief(arena, &func, "Replace a sorted sequence of ranges with specific strings");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to modify");
        
        doc_function_param(arena, &func, "batch");
        doc_text(arena, params, "the first batch edit in a linked list of edits to apply in one atomic modification");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero on success, when the buffer exists and the batch is correctly sorted and contained within the buffer, zero otherwise");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "The ranges of the batch edit should all refer to the range they would have edited in the original state of the buffer. Put another way, the user should make no assumption about what order the individual edist are applied, and instead should treat the operation as applying all replacements atomically.");
        
        doc_paragraph(arena, det);
        
        doc_text(arena, det, "All modifications made by this call are saved into the history as a single edit record, thus undoing this edit reversed the entire batch.");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "buffer_replace_range");
        doc_function_add_related(arena, rel, "Batch_Edit");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_seek_string", &func)){
        doc_function_brief(arena, &func, "Scan a buffer from a point to the first occurence of a string");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer");
        doc_text(arena, params, "the id of the buffer who's contents to seek");
        
        doc_function_param(arena, &func, "needle");
        doc_text(arena, params, "the string to match against the contents of the buffer");
        
        doc_function_param(arena, &func, "direction");
        doc_text(arena, params, "the scan direction of the scan from the start point");
        
        doc_function_param(arena, &func, "start_pos");
        doc_text(arena, params, "the start point of the scan");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "a single string match containing the range of the match and 'match type' flags on success, when the buffer exists and a match is found, otherwise cleared to zero; one can easily determine whether there was a match by the buffer member of the result");
        
        doc_paragraph(arena, ret);
        
        doc_text(arena, ret, "The returned range is left inclusive right exclusive, so that range.max - range.min is the size of the match, and range.min is the first index of the matching string.");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "The match is never permitted to be found at start_pos, thus to search the whole buffer from the beginning start_pos should be -1 and to search backward from the end start_pos should be the size of the buffer.");
        
        doc_paragraph(arena, ret);
        
        doc_text(arena, ret, "Non-case sensitive matches are reported, but if the match that was found is case sensitive the StringMatch_CaseSensitive flag is set on the result.");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "Scan_Direction");
        doc_function_add_related(arena, rel, "String_Match");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_seek_character_class", &func)){
        doc_function_brief(arena, &func, "Scan a buffer from a point to the first character in a specified set of characters");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer");
        doc_text(arena, params, "the id of the buffer who's contents to seek");
        
        doc_function_param(arena, &func, "predicate");
        doc_text(arena, params, "specifies the set of bytes that will end the scan");
        
        doc_function_param(arena, &func, "direction");
        doc_text(arena, params, "the scan direction of the scan from the start point");
        
        doc_function_param(arena, &func, "start_pos");
        doc_text(arena, params, "the start point of the scan");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "a single string match containing a range of a single character on success, when the buffer exists and a match is found, otherwise a cleared to zero; one can easily determine whether there was a match by the buffer member of the result");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "Character_Predicate");
        doc_function_add_related(arena, rel, "String_Match");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_line_y_difference", &func)){
        doc_function_brief(arena, &func, "Compute the signed vertical pixel distance between the top of two lines");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer who's layout will be measured");
        
        doc_function_param(arena, &func, "width");
        doc_text(arena, params, "the width parameter of the layout, passed to layout rules as a recommended wrap point");
        
        doc_function_param(arena, &func, "face_id");
        doc_text(arena, params, "the face parameter of the layout, passed to layout rules as a recommended face");
        
        doc_function_param(arena, &func, "line_a");
        doc_text(arena, params, "the line number of the line 'A' in the subtraction top(A) - top(B)");
        
        doc_function_param(arena, &func, "line_b");
        doc_text(arena, params, "the line number of the line 'B' in the subtraction top(A) - top(B)");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the signed distance between the lines in pixels on success, when the buffer exists and contains both given line numbers");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "Line numbers are 1 based.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_line_shift_y", &func)){
        doc_function_brief(arena, &func, "Compute a new line number and pixel shift relative to a given line number and pixel shift, guaranteeing that the new line number is the one closest to containing the new point");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer who's layout will be measured");
        
        doc_function_param(arena, &func, "width");
        doc_text(arena, params, "the width parameter of the layout, passed to layout rules as a recommended wrap point");
        
        doc_function_param(arena, &func, "face_id");
        doc_text(arena, params, "the face parameter of the layout, passed to layout rules as a recommended face");
        
        doc_function_param(arena, &func, "line");
        doc_text(arena, params, "the line number of the line that serves as the relative starting point of the measurement");
        
        doc_function_param(arena, &func, "y_shift");
        doc_text(arena, params, "the y shift, in pixels, from the top of the specified line to be measured");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the best match line number and the remaining y shift that is not accounted for by the change in line number on success, when the buffer exists and contains the line, cleared to zero otherwise");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "Line numbers are 1 based.");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "Line_Shift_Vertical");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_pos_at_relative_xy", &func)){
        doc_function_brief(arena, &func, "Compute a byte position at a particular point relative to the top left corner of a particular line");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer who's layout will be measured");
        
        doc_function_param(arena, &func, "width");
        doc_text(arena, params, "the width parameter of the layout, passed to layout rules as a recommended wrap point");
        
        doc_function_param(arena, &func, "face_id");
        doc_text(arena, params, "the face parameter of the layout, passed to layout rules as a recommended face");
        
        doc_function_param(arena, &func, "base_line");
        doc_text(arena, params, "the line number of the line that serves as the relative starting point of the measurement");
        
        doc_function_param(arena, &func, "relative_xy");
        doc_text(arena, params, "the point, in pixels, interpreted relative to the line's top left corner, that will serve as the query point");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the byte index associated as the first byte of a character in the layout that is the closest to containing the query point on success, when the buffer exists and contains the line, zero otherwise");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "Line numbers are 1 based.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_relative_box_of_pos", &func)){
        doc_function_brief(arena, &func, "Compute the box of a character that spans a particular byte position, relative to the top left corner of a given line");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer who's layout will be measured");
        
        doc_function_param(arena, &func, "width");
        doc_text(arena, params, "the width parameter of the layout, passed to layout rules as a recommended wrap point");
        
        doc_function_param(arena, &func, "face_id");
        doc_text(arena, params, "the face parameter of the layout, passed to layout rules as a recommended face");
        
        doc_function_param(arena, &func, "base_line");
        doc_text(arena, params, "the line number of the line that serves as the relative starting point of the measurement");
        
        doc_function_param(arena, &func, "pos");
        doc_text(arena, params, "the absolute byte index of the position to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the rectangle of a character in the layout that is closest to including the given query position in it's span, with coordinates set relative to the top left corner of the base line, on success, when the buffer exists and contains the base line and query position, cleared to zero otherwise");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "Line numbers are 1 based.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_padded_box_of_pos", &func)){
        doc_function_brief(arena, &func, "Compute the rectangle around a character at a particular byte position, relative to the top left corner of a given line");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer who's layout will be measured");
        
        doc_function_param(arena, &func, "width");
        doc_text(arena, params, "the width parameter of the layout, passed to layout rules as a recommended wrap point");
        
        doc_function_param(arena, &func, "face_id");
        doc_text(arena, params, "the face parameter of the layout, passed to layout rules as a recommended face");
        
        doc_function_param(arena, &func, "base_line");
        doc_text(arena, params, "the line number of the line that serves as the relative starting point of the measurement");
        
        doc_function_param(arena, &func, "pos");
        doc_text(arena, params, "the absolute byte index of the position to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the rectangle around a character in the layout that is closest to including the given query position in it's span, with coordinates set relative to the top left corner of the base line, on success, when the buffer exists and contains the base line and query position, cleared to zero otherwise");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "Line numbers are 1 based.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_relative_character_from_pos", &func)){
        doc_function_brief(arena, &func, "Compute a character index relative to a particular lines first character");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer who's layout will be measured");
        
        doc_function_param(arena, &func, "width");
        doc_text(arena, params, "the width parameter of the layout, passed to layout rules as a recommended wrap point");
        
        doc_function_param(arena, &func, "face_id");
        doc_text(arena, params, "the face parameter of the layout, passed to layout rules as a recommended face");
        
        doc_function_param(arena, &func, "base_line");
        doc_text(arena, params, "the line number of the line that serves as the relative starting point of the measurement");
        
        doc_function_param(arena, &func, "pos");
        doc_text(arena, params, "the absolute byte index of the position to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the relative index, based at the first character of the base line, of the character that is closest to spanning the query position on success, when the buffer exists and contains the base line and query position, zero otherwise");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "Line numbers are 1 based.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_pos_from_relative_character", &func)){
        doc_function_brief(arena, &func, "Compute the byte position associated with the start of a particular character specified relative to the first character of a particular line");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer who's layout will be measured");
        
        doc_function_param(arena, &func, "width");
        doc_text(arena, params, "the width parameter of the layout, passed to layout rules as a recommended wrap point");
        
        doc_function_param(arena, &func, "face_id");
        doc_text(arena, params, "the face parameter of the layout, passed to layout rules as a recommended face");
        
        doc_function_param(arena, &func, "base_line");
        doc_text(arena, params, "the line number of the line that serves as the relative starting point of the measurement");
        
        doc_function_param(arena, &func, "relative_character");
        doc_text(arena, params, "the relative character index of the query character, based at the first character of base line");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the byte index associated with the start of the character specified by the the base_line and relative_character parameters on success, when the buffer exists and contains the base line, zero otherwise");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "Line numbers are 1 based.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_exists", &func)){
        doc_function_brief(arena, &func, "Check that a buffer id represents a real buffer");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the buffer id to check");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero if the buffer exists, zero otherwise");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_get_access_flags", &func)){
        doc_function_brief(arena, &func, "Retrieve the access flags of a buffer");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the access flag fied of the buffer on success, when the buffer exists, otherwise zero");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "Access_Flag");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_get_size", &func)){
        doc_function_brief(arena, &func, "Retrieve the size of a buffer in bytes");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the size in bytes of the buffer on success, when the buffer exists, otherwise zero");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_get_line_count", &func)){
        doc_function_brief(arena, &func, "Retrieve the line count of a buffer");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the number of lines in the buffer on success, when the buffer exists, otherwise zero");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "push_buffer_base_name", &func)){
        doc_function_brief(arena, &func, "Get a copy of the base name of a buffer");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "arena");
        doc_text(arena, params, "the arena on which the returned string will be allocated");
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the name assigned to the buffer when it was created on success, when the buffer exists, otherwise an empty string");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "push_buffer_unique_name", &func)){
        doc_function_brief(arena, &func, "Get a copy of the unique name of a buffer");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "out");
        doc_text(arena, params, "the arena on which the returned string will be allocated");
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the name assigned to the buffer by resolving duplicate names on success, when the buffer exists, otherwise an empty string");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "push_buffer_file_name", &func)){
        doc_function_brief(arena, &func, "Get a copy of the file name of a buffer");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "arena");
        doc_text(arena, params, "the arena on which the returned string will be allocated");
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the name of the file attached to the buffer when it was created, when the buffer exists and has an attached file, otherwise an empty string");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_get_dirty_state", &func)){
        doc_function_brief(arena, &func, "Retrieve the dirty state flags of a buffer");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the dirty state flags of the buffer on success, when it exists, otherwise zero");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "Dirty_State");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_set_dirty_state", &func)){
        doc_function_brief(arena, &func, "Set the dirty state of a buffer");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to be modified");
        
        doc_function_param(arena, &func, "dirty_state");
        doc_text(arena, params, "the new value for the buffer's dirty state");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero on success, when the buffer exists, otherwise zero");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "Dirty_State");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_set_layout", &func)){
        doc_function_brief(arena, &func, "Set the layout function of a buffer");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to be modified");
        
        doc_function_param(arena, &func, "layout_func");
        doc_text(arena, params, "the new layout function for the buffer's layout");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero on success, when the buffer exists, otherwise zero");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_clear_layout_cache", &func)){
        doc_function_brief(arena, &func, "Clear all the layout information cached in the buffer");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to be modified");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_get_layout", &func)){
        doc_function_brief(arena, &func, "Retrieve the layout rule of a buffer");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "a pointer to the function specifying the buffer's layout rule on success, when the buffer exists, zero otherwise");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_get_setting", &func)){
        doc_function_brief(arena, &func, "Retrieve a core setting of a buffer");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to query");
        
        doc_function_param(arena, &func, "setting");
        doc_text(arena, params, "the id of the setting to query");
        
        doc_function_param(arena, &func, "value_out");
        doc_text(arena, params, "the output destination of the setting's value");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero on success, when the buffer and setting exist, zero otherwise");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "Buffer_Setting_ID");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_set_setting", &func)){
        doc_function_brief(arena, &func, "Retrieve a core setting of a buffer");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to modify");
        
        doc_function_param(arena, &func, "setting");
        doc_text(arena, params, "the id of the setting to modify");
        
        doc_function_param(arena, &func, "value");
        doc_text(arena, params, "the new value of the specified setting");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero on success, when the buffer and setting exist, zero otherwise");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "Buffer_Setting_ID");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_get_managed_scope", &func)){
        doc_function_brief(arena, &func, "Retrieve the managed scope tied to the lifetime of a buffer");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the id of the managed scope tied to the buffer on success, when the buffer exists, zero otherwise");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "Managed_Scope");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_send_end_signal", &func)){
        doc_function_brief(arena, &func, "Cause the buffer to reset it's lifetime");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to receive the signal");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero on success, when the buffer exists, zero otherwise");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "When a buffer's end signal is sent, it's managed scope is cleared, as if it had been destroyed and recreated, and the buffer end hook is run on the buffer. The upshot of this is that it is as if the buffer were closed and re-opened except that it still has the same id.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "create_buffer", &func)){
        doc_function_brief(arena, &func, "Create a new buffer");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "file_name");
        doc_text(arena, params, "if there exists a file with the given name, and no buffer for that file, a new buffer is created, attached to the file, and named after the file; if no such file exists, or the buffer is forbidden from attaching to a file by the flags, then this is the name of a newly created detached buffer");
        
        doc_function_param(arena, &func, "flags");
        doc_text(arena, params, "flags controlling behavior of the buffer creation");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "if a buffer matching the file name already exists, it's id is returned, otherwise if a new buffer can be created according to the flags, a buffer is created and it's id is returned, if no matching buffer exists and no buffer can be created from it, zero is returned");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "Buffer_Create_Flag");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_save", &func)){
        doc_function_brief(arena, &func, "Save the contents of a buffer to a file");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer who's contents to write to disk");
        
        doc_function_param(arena, &func, "file_name");
        doc_text(arena, params, "the name of the file to be written, if empty and the buffer has an attached file, that is used instead, using this does not alter the attachment of the buffer either way");
        
        doc_function_param(arena, &func, "flags");
        doc_text(arena, params, "flags controlling the behavior of the save");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero when the buffer exists and the file was successfully written, zero otherwise");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "Buffer_Save_Flag");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_kill", &func)){
        doc_function_brief(arena, &func, "Close a buffer");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to close");
        
        doc_function_param(arena, &func, "flags");
        doc_text(arena, params, "flags controlling the buffer closing behavior");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "a code indicating the result of the attempt to kill a buffer, if the buffer does not exist this will be a failure, if the buffer exists but has unsaved changes, this will indicate as much unless flag was used to override the dirty state check, if the buffer is successfully closed that will be indicated by the code");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "When a buffer's dirty state includes unsaved changes, this call will not close the buffer unless it is forced to by the flags.");
        doc_paragraph(arena, det);
        doc_text(arena, det, "Certain buffers critical to the operation of the core cannot be closed, and attempts to close them will always result in failure to close.");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "Buffer_Kill_Result");
        doc_function_add_related(arena, rel, "Buffer_Kill_Flag");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_reopen", &func)){
        doc_function_brief(arena, &func, "Reload the content of a buffer from the the attached file");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to reload");
        
        doc_function_param(arena, &func, "flags");
        doc_text(arena, params, "flags controlling the behavior of the reload");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero if the buffer exists and has an attached file that can be read, zero otherwise");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "Buffer_Reopen_Flag");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_get_file_attributes", &func)){
        doc_function_brief(arena, &func, "Get file attributes of a buffer");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the file attributes of the buffer as set by the last time it synced with it's attached file on disk when the buffer exists, cleared to zero otherwise");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "File_Attributes");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_compute_cursor", &func)){
        doc_function_brief(arena, &func, "Compute a buffer cursor from a buffer and a seek target");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer");
        doc_text(arena, params, "the id of the buffer to query");
        
        doc_function_param(arena, &func, "seek");
        doc_text(arena, params, "the seek target to use in a query for full cursor information");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the full cursor information for the position specified by the seek, if the buffer exists, otherwise cleared to zero");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "Buffer_Seek");
        doc_function_add_related(arena, rel, "Buffer_Cursor");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_history_get_max_record_index", &func)){
        doc_function_brief(arena, &func, "Get the largest record index in the buffer history");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "if the buffer exists and has history enabled, the maximum index of the records in the history, otherwise zero");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_history_get_record_info", &func)){
        doc_function_brief(arena, &func, "Get record information out of a buffer's history");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to query");
        
        doc_function_param(arena, &func, "index");
        doc_text(arena, params, "the index of the record to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "on success, when the buffer exists and index is within the history index range, the record information at the given index, otherwise zero");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "Record_Info");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_history_get_group_sub_record", &func)){
        doc_function_brief(arena, &func, "Get a sub-record of a group record from a buffer history");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to query");
        
        doc_function_param(arena, &func, "index");
        doc_text(arena, params, "the index of the record to query");
        
        doc_function_param(arena, &func, "sub_index");
        doc_text(arena, params, "the sub-index of the record to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "on success, when the buffer exists, it's history contains a group record at the given index, and the group record contains a record at the sub-index, the record information contained there, otherwise zero");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "Record_Info");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_history_get_current_state_index", &func)){
        doc_function_brief(arena, &func, "Get the current state index of the history");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "if the buffer exists and has history enabled, the current state index, otherwise zero");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "The current state index indicates how far backward the user has undone. If the index is the most recent record, then nothing has been undone. This way undo-redo operations do not have to modify the history stack, and instead just move the current state index through the stack. Normal modifications to the buffer cause the history to discard everything after the current state index, before putting the new record on top of the stack.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_history_set_current_state_index", &func)){
        doc_function_brief(arena, &func, "Modify the current state index and update the buffer contents to reflect the contents of the buffer as it was at that index.");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to modify");
        
        doc_function_param(arena, &func, "index");
        doc_text(arena, params, "the new current state index value");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero if the buffer exists, has history enabled, and contains a record at the given index, otherwise zero");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "This call simultaneously changes the state index and modifies the buffer to reflect undoing the necessary records from the top of the stack, or redoing them if the state index is being moved forward.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_history_merge_record_range", &func)){
        doc_function_brief(arena, &func, "Merge a range of records into a single group record");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to modify");
        
        doc_function_param(arena, &func, "first_index");
        doc_text(arena, params, "the first index in the range to merge");
        
        doc_function_param(arena, &func, "last_index");
        doc_text(arena, params, "the last index in the range to merge, forming an inclusive-inclusive range");
        
        doc_function_param(arena, &func, "flags");
        doc_text(arena, params, "flags controlling the behavior of the operation");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero on success, when the buffer exists, and the index range is contained within the buffer history, otherwise zero");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "Group records contain all the same information that the range of individual records previously contained.  A group is treated as a single unit in undo-redo operations.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_history_clear_after_current_state", &func)){
        doc_function_brief(arena, &func, "Forget the portion of the buffer history on the redo side");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to modify");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero on success, when the buffer exists, otherwise zero");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_set_face", &func)){
        doc_function_brief(arena, &func, "Change the face of a buffer");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to modify");
        
        doc_function_param(arena, &func, "id");
        doc_text(arena, params, "the id of the face to set on the buffer");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero on success, when the buffer and face exist, otherwise zero");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "buffer_find_all_matches", &func)){
        doc_function_brief(arena, &func, "Find all matches for a search pattern in a buffer");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "arena");
        doc_text(arena, params, "the arena on which the returned string will be allocated");
        
        doc_function_param(arena, &func, "buffer");
        doc_text(arena, params, "the id of the buffer to search");
        
        doc_function_param(arena, &func, "string_id");
        doc_text(arena, params, "the id to store into the resulting string matches");
        
        doc_function_param(arena, &func, "range");
        doc_text(arena, params, "the range in byte positions where all matches must be contained");
        
        doc_function_param(arena, &func, "needle");
        doc_text(arena, params, "the string to search for in the buffer range");
        
        doc_function_param(arena, &func, "predicate");
        doc_text(arena, params, "a character predicate used to check the left and right side of the match to add left sloppy and right sloppy match flags.");
        
        doc_function_param(arena, &func, "direction");
        doc_text(arena, params, "the direction of the scan through the buffer range determining the order of the generated matches");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "a linked list of matches to the search pattern");
    }
}

// BOTTOM

