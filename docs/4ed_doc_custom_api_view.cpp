/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 07.12.2019
 *
 * Documentation of the custom layer's primary api : view-related calls
 *
 */

// TOP

function void
doc_custom_api__view(Arena *arena, API_Definition *api_def, Doc_Cluster *cluster){
    Doc_Function func = {};
    
    if (begin_doc_call(arena, cluster, api_def, "view_line_y_difference", &func)){
        doc_function_brief(arena, &func, "Compute the signed vertical pixel distance between the top of two lines");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of the view who's layout will be measured");
        
        doc_function_param(arena, &func, "line_a");
        doc_text(arena, params, "the line number of the line 'A' in the subtraction top(A) - top(B)");
        
        doc_function_param(arena, &func, "line_b");
        doc_text(arena, params, "the line number of the line 'B' in the subtraction top(A) - top(B)");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the signed distance between the lines in pixels on success, when the view exists and contains both given line numbers");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "Line numbers are 1 based.");
        doc_paragraph(arena, det);
        doc_text(arena, det, "Equivalent to calling the buffer related function of the same name by deriving the buffer, width, and face from the view.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "view_line_shift_y", &func)){
        doc_function_brief(arena, &func, "Compute a new line number and pixel shift relative to a given line number and pixel shift, guaranteeing that the new line number is the one closest to containing the new point");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of the view who's layout will be measured");
        
        doc_function_param(arena, &func, "line");
        doc_text(arena, params, "the line number of the line that serves as the relative starting point of the measurement");
        
        doc_function_param(arena, &func, "y_shift");
        doc_text(arena, params, "the y shift, in pixels, from the top of the specified line to be measured");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the best match line number and the remaining y shift that is not accounted for by the change in line number on success, when the view exists and contains the line, cleared to zero otherwise");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "Line numbers are 1 based.");
        doc_paragraph(arena, det);
        doc_text(arena, det, "Equivalent to calling the buffer related function of the same name by deriving the buffer, width, and face from the view.");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "Line_Shift_Vertical");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "view_pos_at_relative_xy", &func)){
        doc_function_brief(arena, &func, "Compute a byte position at a particular point relative to the top left corner of a particular line");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of the view who's layout will be measured");
        
        doc_function_param(arena, &func, "base_line");
        doc_text(arena, params, "the line number of the line that serves as the relative starting point of the measurement");
        
        doc_function_param(arena, &func, "relative_xy");
        doc_text(arena, params, "the point, in pixels, interpreted relative to the line's top left corner, that will serve as the query point");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the byte index associated as the first byte of a character in the layout that is the closest to containing the query point on success, when the view exists and contains the line, zero otherwise");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "Line numbers are 1 based.");
        doc_paragraph(arena, det);
        doc_text(arena, det, "Equivalent to calling the buffer related function of the same name by deriving the buffer, width, and face from the view.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "view_relative_box_of_pos", &func)){
        doc_function_brief(arena, &func, "Compute the box of a character that spans a particular byte position, relative to the top left corner of a given line");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of the view who's layout will be measured");
        
        doc_function_param(arena, &func, "base_line");
        doc_text(arena, params, "the line number of the line that serves as the relative starting point of the measurement");
        
        doc_function_param(arena, &func, "pos");
        doc_text(arena, params, "the absolute byte index of the position to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the rectangle of a character in the layout that is closest to including the given query position in it's span, with coordinates set relative to the top left corner of the base line, on success, when the view exists and contains the base line and query position, cleared to zero otherwise");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "Line numbers are 1 based.");
        doc_paragraph(arena, det);
        doc_text(arena, det, "Equivalent to calling the buffer related function of the same name by deriving the buffer, width, and face from the view.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "view_padded_box_of_pos", &func)){
        doc_function_brief(arena, &func, "Compute the rectangle around a character at a particular byte position, relative to the top left corner of a given line");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of the view who's layout will be measured");
        
        doc_function_param(arena, &func, "base_line");
        doc_text(arena, params, "the line number of the line that serves as the relative starting point of the measurement");
        
        doc_function_param(arena, &func, "pos");
        doc_text(arena, params, "the absolute byte index of the position to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the rectangle around a character in the layout that is closest to including the given query position in it's span, with coordinates set relative to the top left corner of the base line, on success, when the view exists and contains the base line and query position, cleared to zero otherwise");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "Line numbers are 1 based.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "view_relative_character_from_pos", &func)){
        doc_function_brief(arena, &func, "Compute a character index relative to a particular lines first character");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of the view who's layout will be measured");
        
        doc_function_param(arena, &func, "base_line");
        doc_text(arena, params, "the line number of the line that serves as the relative starting point of the measurement");
        
        doc_function_param(arena, &func, "pos");
        doc_text(arena, params, "the absolute byte index of the position to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the relative index, based at the first character of the base line, of the character that is closest to spanning the query position on success, when the view exists and contains the base line and query position, zero otherwise");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "Line numbers are 1 based.");
        doc_paragraph(arena, det);
        doc_text(arena, det, "Equivalent to calling the buffer related function of the same name by deriving the buffer, width, and face from the view.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "view_pos_from_relative_character", &func)){
        doc_function_brief(arena, &func, "Compute the byte position associated with the start of a particular character specified relative to the first character of a particular line");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of the view who's layout will be measured");
        
        doc_function_param(arena, &func, "base_line");
        doc_text(arena, params, "the line number of the line that serves as the relative starting point of the measurement");
        
        doc_function_param(arena, &func, "character");
        doc_text(arena, params, "the relative character index of the query character, based at the first character of base line");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the byte index associated with the start of the character specified by the the base_line and relative_character parameters on success, when the view exists and contains the base line, zero otherwise");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "Line numbers are 1 based.");
        doc_paragraph(arena, det);
        doc_text(arena, det, "Equivalent to calling the buffer related function of the same name by deriving the buffer, width, and face from the view.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "get_view_next", &func)){
        doc_function_brief(arena, &func, "Iterate to the next view");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of the view to iterate from, or zero to get the first view");
        
        doc_function_param(arena, &func, "access");
        doc_text(arena, params, "the type of access requirements used to filter on the views that are returned");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the id of the next view, or zero if there is no next view");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "get_view_prev", &func)){
        doc_function_brief(arena, &func, "Iterate to the previous view");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of the view to iterate from, or zero to get the first view");
        
        doc_function_param(arena, &func, "access");
        doc_text(arena, params, "the type of access requirements used to filter on the views that are returned");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the id of the previous view, or zero if there is no previous view");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "get_this_ctx_view", &func)){
        doc_function_brief(arena, &func, "Get the view attached to the thread context");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "access");
        doc_text(arena, params, "the type of access requirements that the view must satisfy");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the id of the view attached to this thread, if the calling thread is a view context, and if it can satisfy the access requirements, zero otherwise");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "get_active_view", &func)){
        doc_function_brief(arena, &func, "Get the id of the active view");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "access");
        doc_text(arena, params, "the type of access requirements that the view must satisfy");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the id of the active view, if it can satisfy the access requirements, zero otherwise");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "view_exists", &func)){
        doc_function_brief(arena, &func, "Check that a view id represents a real view");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of the view to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero if the view exists, zero otherwise");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "view_get_buffer", &func)){
        doc_function_brief(arena, &func, "Get the buffer attached to a view");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of the view to query");
        
        doc_function_param(arena, &func, "access");
        doc_text(arena, params, "required access flags for the buffer, in this call the flag Access_Visible indicates whether the view is currently displaying it's buffer, by adding it to the access flags you can require that the buffer is visible");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the id of the buffer if the view exists and the buffer satisfies the access requirements, zero otherwise");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "view_get_cursor_pos", &func)){
        doc_function_brief(arena, &func, "Get the position of the core tracked cursor on a view");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of the view to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the position of the view's cursor in bytes, if the view exists, otherwise zero");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "view_get_mark_pos", &func)){
        doc_function_brief(arena, &func, "Get the position of the core tracked mark on a view");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of the view to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the position of the view's mark in bytes, if the view exists, otherwise zero");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "view_get_preferred_x", &func)){
        doc_function_brief(arena, &func, "Get the x coordinate of the core tracked preferred x on a view");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of the view to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the preferred x of the view in pixels from the left edge of buffer layout space, if the view exists, otherwise zero");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "view_set_preferred_x", &func)){
        doc_function_brief(arena, &func, "Set the preferred x on a view");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of the view to modify");
        
        doc_function_param(arena, &func, "x");
        doc_text(arena, params, "the new preferred x coordinate");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero if the view exists, zero otherwise");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "the preferred x is used to keep a cursor aligned as closely as possible to a particular x-coordinate durring vertical movements");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "view_get_screen_rect", &func)){
        doc_function_brief(arena, &func, "Get the rectangle on screen covered by a view");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of the view to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the rectangle covered by the view in pixels relative to the screen rectangle's top left corner, if the view exists, zero otherwise");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "view_get_panel", &func)){
        doc_function_brief(arena, &func, "Get the id of a panel associated with a view");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of the view to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the id of the panel associated with the view, if the view exists, zero otherwise");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "Associations between panels and views are not 'stable'. They often change whenever the layout of panels is adjusted. The results of this query should only be taken as correct for as long as no panels or views are opened or closed.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "panel_get_view", &func)){
        doc_function_brief(arena, &func, "Get the id of a view associated with a panel");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "panel_id");
        doc_text(arena, params, "the id of the panel to query");
        
        doc_function_param(arena, &func, "access");
        doc_text(arena, params, "the type of access requirements that the view must satisfy");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the id of the view associated with the panel, if the panel exists and is a 'leaf' panel in the panel layout, zero otherwise");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "Associations between panels and views are not 'stable'. They often change whenever the layout of panels is adjusted. The results of this query should only be taken as correct for as long as no panels or views are opened or closed.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "panel_is_split", &func)){
        doc_function_brief(arena, &func, "Check if a panel is an internal split panel");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "panel_id");
        doc_text(arena, params, "the id of the panel to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero if the panel exists and is an internal panel in the panel layout tree");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "The panel layout tree is a binary tree with internal panels having exactly two children and a split rule, and leaf panels having an associated view.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "panel_is_leaf", &func)){
        doc_function_brief(arena, &func, "Check if a panel is a leaf panel with an associated view");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "panel_id");
        doc_text(arena, params, "the id of the panel to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero if the panel exists and is a leaf panel in the panel layout tree");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "The panel layout tree is a binary tree with internal panels having exactly two children and a split rule, and leaf panels having an associated view.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "panel_split", &func)){
        doc_function_brief(arena, &func, "Introduce a split at a given panel");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "panel_id");
        doc_text(arena, params, "the id of the panel to split");
        
        doc_function_param(arena, &func, "split_dim");
        doc_text(arena, params, "the dimension along which the split is placed - x splits split the horizontal axis with a vertical divider - y splits split the vertical axis with a horizontal divider");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero on success, when the panel exists and splitting it is possible, zero otherwise");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "New splits are created as 50/50 proportional splits.");
        doc_paragraph(arena, det);
        doc_text(arena, det, "The only limit on splits is the number of views, which is 16. When a leaf panel is split, the view it was associated with is placed in the 'min' child of the split and a panel with a new view is placed in the 'max' child of the split. When an internal panel is split, it's 'min' child adopts the old children and split settings of the panel and the 'max' child gets a panel with a new view.");
        doc_paragraph(arena, det);
        doc_text(arena, det, "In either case, a successful split puts a new leaf panel in the 'max' child, puts the old contents of the panel into the 'min' child. The id of the panel that was split becomes the id of the parent of the split. View panel associations are modified by a split.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "panel_set_split", &func)){
        doc_function_brief(arena, &func, "Set the split properties of a split panel");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "panel_id");
        doc_text(arena, params, "the id of the panel to modify");
        
        doc_function_param(arena, &func, "kind");
        doc_text(arena, params, "the kind of split rule used to determine the position of the split");
        
        doc_function_param(arena, &func, "t");
        doc_text(arena, params, "the value parameter of the split rule");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero if the panel exists and is a split panel, zero otherwise");
        
        // TODO(allen): write this allen!
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "panel_swap_children", &func)){
        doc_function_brief(arena, &func, "Swap the min and max children of a split panel");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "panel_id");
        doc_text(arena, params, "the id of the panel to modify");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero if the panel exists and is a split panel");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "panel_get_root", &func)){
        doc_function_brief(arena, &func, "Get the root panel of the panel layout");
        
        // params
        doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the id of the root panel");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "panel_get_parent", &func)){
        doc_function_brief(arena, &func, "Get the parent panel of a panel");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "panel_id");
        doc_text(arena, params, "the id of the panel to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the id of the panel that is the parent of the given panel, if the given panel exists and is not root, otherwise zero");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "panel_get_child", &func)){
        doc_function_brief(arena, &func, "Get one of the children of a split panel");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "panel_id");
        doc_text(arena, params, "the id of the panel to query");
        
        doc_function_param(arena, &func, "which_child");
        doc_text(arena, params, "the selector for which of the children to acquire, 'min' children are the children on the top or left, and 'max' children are the children on the bottom or right, depending on the dimension of the split");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the id of the requested child of the given panel, if the given panel exists and is a split, otherwise zero");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "view_close", &func)){
        doc_function_brief(arena, &func, "Close a view");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of the view to close");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero if the view exists, zero otherwise");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "When the view is closed, it's associated panel is also closed and the layout is adjusted to compensate, possibly changing the associations of of panels and views.");
    }
    
    ////////////////////////////////
    
    // TODO(allen): get rid of view_get_buffer_region
    if (begin_doc_call(arena, cluster, api_def, "view_get_buffer_region", &func)){
        doc_function_brief(arena, &func, "Plans to deprecate - do not rely on this call!");
    }
    
    ////////////////////////////////
    
    // TODO(allen): get rid of view_get_buffer_region
    if (begin_doc_call(arena, cluster, api_def, "view_get_buffer_scroll", &func)){
        doc_function_brief(arena, &func, "Plans to deprecate - do not rely on this call!");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "view_set_active", &func)){
        doc_function_brief(arena, &func, "Set the active view");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of the view to set as active");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero if the view exists, zero otherwise");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "view_enqueue_command_function", &func)){
        doc_function_brief(arena, &func, "Push an command to the core to be processed in the context of a particular view");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of the view who's context thread will be sent the command");
        
        doc_function_param(arena, &func, "custom_func");
        doc_text(arena, params, "the function pointer to the command to be processed by the view");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero if the view exists, zero otherwise");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "Directly enqueued commands are triggered in the order they were enqueued before any additional events, virtual o real, are processed, and before the current frame renders.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "view_get_setting", &func)){
        doc_function_brief(arena, &func, "Retrieve a core setting of a view");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of the view to query");
        
        doc_function_param(arena, &func, "setting");
        doc_text(arena, params, "the id of the setting to query");
        
        doc_function_param(arena, &func, "value_out");
        doc_text(arena, params, "the output destination of the setting's value");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero on success, when the view and setting exist, zero otherwise");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "View_Setting_ID");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "view_set_setting", &func)){
        doc_function_brief(arena, &func, "Retrieve a core setting of a view");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of the view to modify");
        
        doc_function_param(arena, &func, "setting");
        doc_text(arena, params, "the id of the setting to modify");
        
        doc_function_param(arena, &func, "value");
        doc_text(arena, params, "the new value of the specified setting");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero on success, when the view and setting exist, zero otherwise");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "View_Setting_ID");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "view_get_managed_scope", &func)){
        doc_function_brief(arena, &func, "Retrieve the managed scope tied to the lifetime of a view");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of the view to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the id of the managed scope tied to the view on success, when the view exists, zero otherwise");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "Managed_Scope");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "view_compute_cursor", &func)){
        doc_function_brief(arena, &func, "Compute a view cursor from a view and a seek target");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of the view to query");
        
        doc_function_param(arena, &func, "seek");
        doc_text(arena, params, "the seek target to use in a query for full cursor information");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the full cursor information for the position specified by the seek, if the view exists, otherwise cleared to zero");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "Equivalent to calling the buffer related function with the same name using view to derive the buffer parameter.");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "Buffer_Seek");
        doc_function_add_related(arena, rel, "Buffer_Cursor");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "view_set_cursor", &func)){
        doc_function_brief(arena, &func, "Set a view's internal cursor position");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of the view to modify");
        
        doc_function_param(arena, &func, "seek");
        doc_text(arena, params, "the seek target to use to derive the new position");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero if the view exists, zero otherwise");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "Buffer_Seek");
    }
    
    ////////////////////////////////
    
    // TODO(allen): remove view_set_buffer_scroll
    if (begin_doc_call(arena, cluster, api_def, "view_set_buffer_scroll", &func)){
        doc_function_brief(arena, &func, "Plans to deprecate - do not rely on this call!");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "view_set_mark", &func)){
        doc_function_brief(arena, &func, "Set a view's internal mark position");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of the view to modify");
        
        doc_function_param(arena, &func, "seek");
        doc_text(arena, params, "the seek target to use to derive the new position");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero if the view exists, zero otherwise");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "Buffer_Seek");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "view_quit_ui", &func)){
        doc_function_brief(arena, &func, "Try to force a view to exit a UI loop and return to the default buffer display behavior");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of the view to modify");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero if the view exists, zero otherwise");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "A view is considered to be in a UI loop when it's context details have set their 'hides_buffer' field to true. This call attemps to get the view to a state with this field set to false by sending abort events repeatedly. This can fail due to buggy or non-compliant implementations of views. It can either fail because the UI loop refuses to respond to the abort after repeated attempts to close it, or because the view shuts down completely rather than returning to a buffer viewing state. The core ensures that if this happens to the final view, a new root panel and empty view will be initialized.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "view_set_buffer", &func)){
        doc_function_brief(arena, &func, "Set a view's associated buffer");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of the view to modify");
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to associate to this view");
        
        doc_function_param(arena, &func, "flags");
        doc_text(arena, params, "flags controlling the behavior of the view as it changes to the new buffer");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero if the view and buffer both exist, otherwise zero");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "Set_Buffer_Flag");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "view_push_context", &func)){
        doc_function_brief(arena, &func, "Push a view's stack of context details with a pointer to the new values for the context");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of the view to modify");
        
        doc_function_param(arena, &func, "ctx");
        doc_text(arena, params, "the new field values of the view context details to be copied onto the top of the stack");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero if the view exists, otherwise zero");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "View_Context");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "view_pop_context", &func)){
        doc_function_brief(arena, &func, "Pop a view's stack of context details");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of the view to modify");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero on success, when the view exists and the details stack has more than one entry, zero otherwise");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "View_Context");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "view_alter_context", &func)){
        doc_function_brief(arena, &func, "Modify the context details at the top of a view's context details stack");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of the view to modify");
        
        doc_function_param(arena, &func, "ctx");
        doc_text(arena, params, "the new field values of the view context details to be copied onto the top of the stack");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero if the view exists, zero otherwise");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "View_Context");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "view_current_context", &func)){
        doc_function_brief(arena, &func, "Get the current context details at the top of a view's context details stack");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of thew view to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "a copy of the context details for the given view if it exists, zero otherwise");
        
        // related
        Doc_Block *rel = doc_function_begin_related(arena, &func);
        doc_function_add_related(arena, rel, "View_Context");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "view_current_context_hook_memory", &func)){
        doc_function_brief(arena, &func, "Get the memory allocated for a specific hook tied to the current details context of a view");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "view_id");
        doc_text(arena, params, "the id of the view to query");
        
        doc_function_param(arena, &func, "hook_id");
        doc_text(arena, params, "the id of the hook to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the data struct pointing to the memory set aside for the context on success, when the view and hook exit and have associated memory for the hook, otherwise cleared to zero");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "Some hooks controlled by the view context details stack have their own requirements for unique, stable, context memory, which needs a variable sized allocation. The size of the memory available to such hooks is determined by the context details, and each instance of the context on the stack gets it's own allocation for these hooks. For example, smooth scrolling rules often require some state that wants to be locally tied to the scrolling for a particular UI loop.");
    }
}

// BOTTOM

