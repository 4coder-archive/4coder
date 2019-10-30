/*
4coder_layout_rule.cpp - Built in layout rules and layout rule helpers.
*/

// TOP

function void
layout_write(Arena *arena, Layout_Item_List *list,
             i64 index, u32 codepoint, Layout_Item_Flag flags, Rect_f32 rect){
    Temp_Memory restore_point = begin_temp(arena);
    Layout_Item *item = push_array(arena, Layout_Item, 1);
    
    Layout_Item_Block *block = list->first;
    if (block != 0){
        if (block->items + block->count == item){
            block->count += 1;
        }
        else{
            block = 0;
        }
    }
    if (block == 0){
        end_temp(restore_point);
        block = push_array(arena, Layout_Item_Block, 1);
        item = push_array(arena, Layout_Item, 1);
        sll_queue_push(list->first, list->last, block);
        list->node_count += 1;
        block->items = item;
        block->count = 1;
    }
    list->total_count += 1;
    
    if (index > list->index_range.max){
        block->character_count += 1;
        list->character_count += 1;
        list->index_range.max = index;
    }
    
    item->index = index;
    item->codepoint = codepoint;
    item->flags = flags;
    item->rect = rect;
    list->height = max(list->height, rect.y1);
}

function Layout_Item_List
layout_wrap_anywhere(Application_Links *app, Arena *arena, Buffer_ID buffer,
                     Range_i64 range, Face_ID face, f32 width){
    Scratch_Block scratch(app);
    
    Layout_Item_List list = {};
    list.index_range.first = range.first;
    list.index_range.one_past_last = range.first - 1;
    
    String_Const_u8 text = push_buffer_range(app, scratch, buffer, range);
    
    Face_Advance_Map advance_map = get_face_advance_map(app, face);
    Face_Metrics metrics = get_face_metrics(app, face);
    f32 line_height = metrics.line_height;
    f32 text_height = metrics.text_height;
    f32 line_to_text_shift = text_height - line_height;
    f32 space_advance = metrics.space_advance;
    
    if (text.size == 0){
        f32 next_x = space_advance;
        layout_write(arena, &list, range.first, ' ', 0, 
                     Rf32(V2(0.f, 0.f), V2f32(next_x, text_height)));
    }
    else{
        Vec2_f32 p = {};
        f32 line_y = line_height;
        f32 text_y = text_height;
        
        i64 index = range.first;
        
        b32 first_of_the_line = true;
        b32 consuming_newline_characters = false;
        i64 newline_character_index = -1;
        b32 prev_did_emit_newline = false;
        
        u8 *ptr = text.str;
        u8 *end_ptr = ptr + text.size;
        for (;ptr < end_ptr;){
            Character_Consume_Result consume = utf8_consume(ptr, (umem)(end_ptr - ptr));
            u32 render_codepoint = consume.codepoint;
            b32 emit_newline = false;
            switch (consume.codepoint){
                case '\t':
                {
                    render_codepoint = ' ';
                }//fallthrough;
                default:
                {
                    f32 advance = font_get_glyph_advance(&advance_map, &metrics,
                                                         consume.codepoint);
                    f32 next_x = p.x + advance;
                    if (!first_of_the_line && next_x > width){
                        p.y = line_y;
                        p.x = 0.f;
                        line_y += line_height;
                        text_y = line_y + line_to_text_shift;
                        next_x = advance;
                    }
                    layout_write(arena, &list, index,
                                 render_codepoint, 0, 
                                 Rf32(p, V2f32(next_x, text_y)));
                    p.x = next_x;
                    ptr += consume.inc;
                    index += consume.inc;
                    first_of_the_line = false;
                }break;
                
                case '\r':
                {
                    if (!consuming_newline_characters){
                        consuming_newline_characters = true;
                        newline_character_index = index;
                    }
                    ptr += 1;
                    index += 1;
                }break;
                
                case '\n':
                {
                    if (!consuming_newline_characters){
                        consuming_newline_characters = true;
                        newline_character_index = index;
                    }
                    emit_newline = true;
                    ptr += 1;
                    index += 1;
                }break;
                
                case max_u32:
                {
                    f32 next_x = p.x + metrics.byte_advance;
                    if (!first_of_the_line && next_x > width){
                        p.y = line_y;
                        p.x = 0.f;
                        line_y += line_height;
                        text_y = line_y + line_to_text_shift;
                        next_x = p.x + metrics.byte_advance;
                    }
                    u32 v = *ptr;
                    u32 lo = v&0xF;
                    u32 hi = (v >> 4)&0xF;
                    f32 advance = metrics.byte_sub_advances[0];
                    layout_write(arena, &list, index, '\\', 0,
                                 Rf32(p, V2f32(p.x + advance, text_y)));
                    p.x += advance;
                    advance = metrics.byte_sub_advances[1];
                    layout_write(arena, &list, index, integer_symbols[hi], 0,
                                 Rf32(p, V2f32(p.x + advance, text_y)));
                    p.x += advance;
                    advance = metrics.byte_sub_advances[2];
                    layout_write(arena, &list, index, integer_symbols[lo], 0,
                                 Rf32(p, V2f32(p.x + advance, text_y)));
                    p.x = next_x;
                    ptr += 1;
                    index += 1;
                    first_of_the_line = false;
                }break;
            }
            
            prev_did_emit_newline = false;
            if (emit_newline){
                f32 next_x = p.x + space_advance;
                layout_write(arena, &list, newline_character_index, ' ', 0,
                             Rf32(p, V2f32(next_x, text_y)));
                p.y = line_y;
                p.x = 0.f;
                line_y += line_height;
                text_y = line_y + line_to_text_shift;
                first_of_the_line = true;
                prev_did_emit_newline = true;
            }
        }
        
        if (!prev_did_emit_newline){
            f32 next_x = p.x + space_advance;
            layout_write(arena, &list, index, ' ', 0, 
                         Rf32(p, V2f32(next_x, text_y)));
        }
    }
    list.bottom_extension = -line_to_text_shift;
    list.height += list.bottom_extension;
    
    return(list);
}

function Layout_Item_List
layout_wrap_whitespace(Application_Links *app, Arena *arena, Buffer_ID buffer,
                       Range_i64 range, Face_ID face, f32 width){
    Scratch_Block scratch(app);
    
    Layout_Item_List list = {};
    list.index_range.first = range.first;
    list.index_range.one_past_last = range.first - 1;
    
    String_Const_u8 text = push_buffer_range(app, scratch, buffer, range);
    
    Face_Advance_Map advance_map = get_face_advance_map(app, face);
    Face_Metrics metrics = get_face_metrics(app, face);
    f32 line_height = metrics.line_height;
    f32 text_height = metrics.text_height;
    f32 line_to_text_shift = text_height - line_height;
    f32 space_advance = metrics.space_advance;
    
    if (text.size == 0){
        f32 next_x = space_advance;
        layout_write(arena, &list, range.first, ' ', 0, 
                     Rf32(V2(0.f, 0.f), V2f32(next_x, text_height)));
    }
    else{
        Vec2_f32 p = {};
        f32 line_y = line_height;
        f32 text_y = text_height;
        
        b32 first_of_the_line = true;
        b32 consuming_newline_characters = false;
        i64 newline_character_index = -1;
        b32 prev_did_emit_newline = false;
        
        u8 *ptr = text.str;
        u8 *end_ptr = ptr + text.size;
        u8 *word_ptr = ptr;
        
        if (character_is_whitespace(*ptr)){
            goto consuming_whitespace;
        }
        
        consuming_non_whitespace:
        consuming_newline_characters = false;
        newline_character_index = -1;
        
        for (;ptr <= end_ptr; ptr += 1){
            if (ptr == end_ptr || character_is_whitespace(*ptr)){
                break;
            }
        }
        
        {
            String_Const_u8 word = SCu8(word_ptr, ptr);
            u8 *word_end = ptr;
            
            if (!first_of_the_line){
                f32 total_advance = 0.f;
                ptr = word.str;
                for (; ptr < word_end;){
                    Character_Consume_Result consume =
                        utf8_consume(ptr, (umem)(word_end - ptr));
                    if (consume.codepoint != max_u32){
                        f32 advance = font_get_glyph_advance(&advance_map, &metrics,
                                                             consume.codepoint);
                        total_advance += advance;
                    }
                    else{
                        total_advance += metrics.byte_advance;
                    }
                    ptr += consume.inc;
                }
                
                f32 next_x = p.x + total_advance;
                if (next_x > width){
                    p.y = line_y;
                    p.x = 0.f;
                    line_y += line_height;
                    text_y = line_y + line_to_text_shift;
                    next_x = total_advance;
                }
            }
            
            ptr = word.str;
            
            for (; ptr < word_end;){
                Character_Consume_Result consume =
                    utf8_consume(ptr, (umem)(word_end - ptr));
                
                if (consume.codepoint != max_u32){
                    f32 advance = font_get_glyph_advance(&advance_map, &metrics,
                                                         consume.codepoint);
                    i64 index = (i64)(ptr - text.str) + range.first;
                    layout_write(arena, &list, index,
                                 consume.codepoint, 0,
                                 Rf32(p, V2f32(p.x + advance, text_y)));
                    p.x += advance;
                }
                else{
                    u32 v = *ptr;
                    u32 lo = v&0xF;
                    u32 hi = (v >> 4)&0xF;
                    i64 index = (i64)(ptr - text.str) + range.first;
                    f32 advance = metrics.byte_sub_advances[0];
                    layout_write(arena, &list, index, '\\', 0,
                                 Rf32(p, V2f32(p.x + advance, text_y)));
                    p.x += advance;
                    advance = metrics.byte_sub_advances[1];
                    layout_write(arena, &list, index, integer_symbols[hi], 0,
                                 Rf32(p, V2f32(p.x + advance, text_y)));
                    p.x += advance;
                    advance = metrics.byte_sub_advances[2];
                    layout_write(arena, &list, index, integer_symbols[lo], 0,
                                 Rf32(p, V2f32(p.x + advance, text_y)));
                    p.x += advance;
                }
                
                ptr += consume.inc;
            }
            
            first_of_the_line = false;
        }
        
        consuming_whitespace:
        for (; ptr < end_ptr; ptr += 1){
            if (!character_is_whitespace(*ptr)){
                word_ptr = ptr;
                goto consuming_non_whitespace;
            }
            
            b32 emit_newline = false;
            
            switch (*ptr){
                default:
                {
                    f32 advance = space_advance;
                    if (*ptr == '\t'){
                        advance *= 4.f;
                    }
                    f32 next_x = p.x + advance;
                    if (!first_of_the_line && next_x > width){
                        p.y = line_y;
                        p.x = 0.f;
                        line_y += line_height;
                        text_y = line_y + line_to_text_shift;
                        next_x = advance;
                    }
                    i64 index = (i64)(ptr - text.str) + range.first;
                    layout_write(arena, &list, index,
                                 ' ', 0, 
                                 Rf32(p, V2f32(next_x, text_y)));
                    p.x = next_x;
                    first_of_the_line = false;
                }break;
                
                case '\r':
                {
                    if (!consuming_newline_characters){
                        consuming_newline_characters = true;
                        newline_character_index = (i64)(ptr - text.str) + range.first;
                    }
                }break;
                
                case '\n':
                {
                    if (!consuming_newline_characters){
                        consuming_newline_characters = true;
                        newline_character_index = (i64)(ptr - text.str) + range.first;
                    }
                    emit_newline = true;
                }break;
            }
            
            if (emit_newline){
                f32 next_x = p.x + space_advance;
                layout_write(arena, &list, newline_character_index, ' ', 0,
                             Rf32(p, V2f32(next_x, text_y)));
                p.y = line_y;
                p.x = 0.f;
                line_y += line_height;
                text_y = line_y + line_to_text_shift;
                first_of_the_line = true;
            }
            prev_did_emit_newline = emit_newline;
        }
        
        if (!prev_did_emit_newline){
            f32 next_x = p.x + space_advance;
            i64 index = (i64)(ptr - text.str) + range.first;
            layout_write(arena, &list, index, ' ', 0, 
                         Rf32(p, V2f32(next_x, text_y)));
        }
    }
    list.bottom_extension = -line_to_text_shift;
    list.height += list.bottom_extension;
    
    return(list);
}

// BOTTOM

