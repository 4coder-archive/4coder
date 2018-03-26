/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 25.03.2018
 *
 * Styles
 *
 */

// TOP

internal void
style_set_colors(Style *style, Theme *theme){
    for (u32 i = 0; i < Stag_COUNT; ++i){
        u32 *color_ptr = style_index_by_tag(&style->main, i);
        *color_ptr = theme->colors[i];
    }
}

internal u32
style_get_margin_color(i32 active_level, Style *style){
    u32 margin = 0xFFFFFFFF;
    
    switch (active_level){
        default:
        {
            margin = style->main.list_item_color;
        }break;
        
        case 1: case 2:
        {
            margin = style->main.list_item_hover_color;
        }break;
        
        case 3: case 4:
        {
            margin = style->main.list_item_active_color;
        }break;
    }
    
    return(margin);
}

internal u32*
style_get_color(Style *style, Cpp_Token token){
    u32 *result = 0;
    if ((token.flags & CPP_TFLAG_IS_KEYWORD) != 0){
        if (token.type == CPP_TOKEN_BOOLEAN_CONSTANT){
            result = &style->main.bool_constant_color;
        }
        else{
            result = &style->main.keyword_color;
        }
    }
    else if ((token.flags & CPP_TFLAG_PP_DIRECTIVE) != 0){
        result = &style->main.preproc_color;
    }
    else{
        switch (token.type){
            case CPP_TOKEN_COMMENT:
            {
                result = &style->main.comment_color;
            }break;
            
            case CPP_TOKEN_STRING_CONSTANT:
            {
                result = &style->main.str_constant_color;
            }break;
            
            case CPP_TOKEN_CHARACTER_CONSTANT:
            {
                result = &style->main.char_constant_color;
            }break;
            
            case CPP_TOKEN_INTEGER_CONSTANT:
            {
                result = &style->main.int_constant_color;
            }break;
            
            case CPP_TOKEN_FLOATING_CONSTANT:
            {
                result = &style->main.float_constant_color;
            }break;
            
            case CPP_PP_INCLUDE_FILE:
            {
                result = &style->main.include_color;
            }break;
            
            default:
            {
                result = &style->main.default_color;
            }break;
        }
    }
    Assert(result != 0);
    return(result);
}

// BOTTOM

