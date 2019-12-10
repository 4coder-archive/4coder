/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 09.12.2019
 *
 * Documentation of the custom layer's primary api : buffer-related calls
 *
 */

// TOP

function void
doc_custom_api__draw(Arena *arena, API_Definition *api_def, Doc_Cluster *cluster){
    Doc_Function func = {};
    
    if (begin_doc_call(arena, cluster, api_def, "draw_string_oriented", &func)){
        doc_function_brief(arena, &func, "Directly render text");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "font_id");
        doc_text(arena, params, "the id of the face to use rendering the text");
        
        doc_function_param(arena, &func, "color");
        doc_text(arena, params, "the color of the text");
        
        doc_function_param(arena, &func, "str");
        doc_text(arena, params, "the text to be rendered");
        
        doc_function_param(arena, &func, "point");
        doc_text(arena, params, "the position of the text, in pixels relative to the screen's top left corner");
        
        doc_function_param(arena, &func, "flags");
        doc_text(arena, params, "flags determining the orientation of the string to render");
        
        doc_function_param(arena, &func, "delta");
        doc_text(arena, params, "a unit vector specifying the direction along which the text advances");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the new point after advancing the point while laying out the text");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "As a render function, this only takes effect during a call to the render hook.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "get_string_advance", &func)){
        doc_function_brief(arena, &func, "Compute the total advance of a string rendered in a particular font");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "font_id");
        doc_text(arena, params, "the id of the face to use rendering the text");
        
        doc_function_param(arena, &func, "str");
        doc_text(arena, params, "the text to be rendered");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the total advance of the string");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "draw_rectangle", &func)){
        doc_function_brief(arena, &func, "Directly render a rectangle");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "rect");
        doc_text(arena, params, "the rectangle coordinates, in pixels relative to the top left corner of the screen");
        
        doc_function_param(arena, &func, "roundness");
        doc_text(arena, params, "the roundness of the pixels, set as a radius of a circular curve measured in pixels");
        
        doc_function_param(arena, &func, "color");
        doc_text(arena, params, "the color of the rectangle");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "As a render function, this only takes effect during a call to the render hook.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "draw_rectangle_outline", &func)){
        doc_function_brief(arena, &func, "Directly render a rectangle outline");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "rect");
        doc_text(arena, params, "the rectangle coordinates, in pixels relative to the top left corner of the screen");
        
        doc_function_param(arena, &func, "roundness");
        doc_text(arena, params, "the roundness of the pixels, set as a radius of a circular curve measured in pixels");
        
        doc_function_param(arena, &func, "thickness");
        doc_text(arena, params, "the thickness, in pixels, of the rectangle outline");
        
        doc_function_param(arena, &func, "color");
        doc_text(arena, params, "the color of the rectangle");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "As a render function, this only takes effect during a call to the render hook.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "draw_set_clip", &func)){
        doc_function_brief(arena, &func, "Set the clip rectangle for future render functions");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "new_clip");
        doc_text(arena, params, "the new rectangular coordinates of the clip rectangle, in pixels relative to the top left corner of the screen");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the previous value of clip rectangle");
        
        // details
        Doc_Block *det = doc_function_details(arena, &func);
        doc_text(arena, det, "As a render function, this only takes effect during a call to the render hook.");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "text_layout_create", &func)){
        doc_function_brief(arena, &func, "Create a text layout object");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "buffer_id");
        doc_text(arena, params, "the id of the buffer to read for layout");
        
        doc_function_param(arena, &func, "rect");
        doc_text(arena, params, "the region on the screen where the layout will be placed");
        
        doc_function_param(arena, &func, "buffer_point");
        doc_text(arena, params, "the point in the buffer that will be placed in the top left corner of the rectangle");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "the id of the new text layout object, when the buffer exists and contains the buffer point, otherwise zero");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "text_layout_region", &func)){
        doc_function_brief(arena, &func, "Get the on screen rectangular region of a layout object");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "text_layout_id");
        doc_text(arena, params, "the id of the text layout object");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "on success, when the text layout object exists, the rectangular region used to create the object, otherwise zero");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "text_layout_get_buffer", &func)){
        doc_function_brief(arena, &func, "Get the buffer of a layout object");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "text_layout_id");
        doc_text(arena, params, "the id of the text layout object");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "on success, when the text layout object exists, the id of the buffer used to create the object, otherwise zero");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "text_layout_get_visible_range", &func)){
        doc_function_brief(arena, &func, "Get the range in byte positions within the buffer that are visible in a text layout");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "text_layout_id");
        doc_text(arena, params, "the id of the text layout object");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "on success, when the text layout object exists, the range of positions in the buffer that are visible in the object, otherwise cleared to zero");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "text_layout_line_on_screen", &func)){
        doc_function_brief(arena, &func, "Get the range of the y-axis spanned by a line layout");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "layout_id");
        doc_text(arena, params, "the id of the text layout object");
        
        doc_function_param(arena, &func, "line_number");
        doc_text(arena, params, "the line number of the line to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "on success, when the object exists and the line is visible in the layout, the range in pixels from the top to the bottom of a particular line, cleared to zero otherwise");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "text_layout_character_on_screen", &func)){
        doc_function_brief(arena, &func, "Get the rectangle covered by a character in the layout");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "layout_id");
        doc_text(arena, params, "the id of the text layout object");
        
        doc_function_param(arena, &func, "pos");
        doc_text(arena, params, "the byte position to query");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "on success, when the object exists and the position is visible in the layout, the rectangle in pixels covered by the character, cleared to zero otherwise");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "paint_text_color", &func)){
        doc_function_brief(arena, &func, "Set the color of a range of characters in a text layout");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "layout_id");
        doc_text(arena, params, "the id of the text layout object");
        
        doc_function_param(arena, &func, "range");
        doc_text(arena, params, "the range in byte positions to modify");
        
        doc_function_param(arena, &func, "color");
        doc_text(arena, params, "the new color of the text in the range");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "text_layout_free", &func)){
        doc_function_brief(arena, &func, "Release the resources associated with a text layout object");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "text_layout_id");
        doc_text(arena, params, "the id of the text layout object");
        
        // return
        Doc_Block *ret = doc_function_return(arena, &func);
        doc_text(arena, ret, "non-zero on success, when the text layout object exists, otherwise zero");
    }
    
    ////////////////////////////////
    
    if (begin_doc_call(arena, cluster, api_def, "draw_text_layout", &func)){
        doc_function_brief(arena, &func, "Directly render a text layout object");
        
        // params
        Doc_Block *params = doc_function_begin_params(arena, &func);
        doc_custom_app_ptr(arena, &func);
        
        doc_function_param(arena, &func, "layout_id");
        doc_text(arena, params, "the id of the text layout object");
        
        doc_function_param(arena, &func, "special_color");
        doc_text(arena, params, "the color to use for characters with the special character flag");
        
        doc_function_param(arena, &func, "ghost_color");
        doc_text(arena, params, "the color to use for characters with the ghost character flag");
    }
}

// BOTTOM

