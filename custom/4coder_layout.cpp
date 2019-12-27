/*
4coder_layout.cpp - Implementation of basic lookup routines on line layout data.
*/

// TOP

internal i64
layout_nearest_pos_to_xy(Layout_Item_List list, Vec2_f32 p){
    i64 closest_match = 0;
    if (p.y < 0.f){
        closest_match = list.manifested_index_range.min;
    }
    else if (p.y >= list.height){
        closest_match = list.manifested_index_range.max;
    }
    else{
        if (0.f < p.x && p.x < max_f32){
            f32 closest_x = -max_f32;
            for (Layout_Item_Block *block = list.first;
                 block != 0;
                 block = block->next){
                i64 count = block->item_count;
                Layout_Item *item = block->items;
                for (i32 i = 0; i < count; i += 1, item += 1){
                    if (HasFlag(item->flags, LayoutItemFlag_Ghost_Character)){
                        continue;
                    }
                    // NOTE(allen): This only works if we build layouts in y-sorted order.
                    if (p.y < item->rect.y0){
                        goto double_break;
                    }
                    if (item->padded_y1 <= p.y){
                        continue;
                    }
                    f32 dist0 = p.x - item->rect.x0;
                    f32 dist1 = item->rect.x1 - p.x;
                    if (dist0 >= 0.f && dist1 > 0.f){
                        closest_match = item->index;
                        goto double_break;
                    }
                    // NOTE(allen): One of dist0 and dist1 are negative, but certainly not both.
                    // 1. Take the negative one.
                    // 2. If the negative distance is larger than closest_x, then this is closer.
                    f32 neg_dist = Min(dist0, dist1);
                    if (closest_x < neg_dist){
                        closest_x = neg_dist;
                        closest_match = item->index;
                    }
                }
            }
            double_break:;
        }
        else{
            
            if (p.x == max_f32){
                Layout_Item *prev_item = 0;
                for (Layout_Item_Block *block = list.first;
                     block != 0;
                     block = block->next){
                    i64 count = block->item_count;
                    Layout_Item *item = block->items;
                    for (i32 i = 0; i < count; i += 1, item += 1){
                        if (HasFlag(item->flags, LayoutItemFlag_Ghost_Character)){
                            continue;
                        }
                        if (p.y < item->rect.y0){
                            goto double_break_2;
                        }
                        prev_item = item;
                        if (item->padded_y1 <= p.y){
                            continue;
                        }
                    }
                }
                
                double_break_2:;
                if (prev_item != 0){
                    closest_match = prev_item->index;
                }
                else{
                    closest_match = list.manifested_index_range.max;
                }
            }
            else{
                Layout_Item *closest_item = 0;
                for (Layout_Item_Block *block = list.first;
                     block != 0;
                     block = block->next){
                    i64 count = block->item_count;
                    Layout_Item *item = block->items;
                    for (i32 i = 0; i < count; i += 1, item += 1){
                        if (HasFlag(item->flags, LayoutItemFlag_Ghost_Character)){
                            continue;
                        }
                        // NOTE(allen): This only works if we build layouts in y-sorted order.
                        if (p.y < item->rect.y0){
                            goto double_break_3;
                        }
                        if (item->padded_y1 <= p.y){
                            continue;
                        }
                        closest_item = item;
                        goto double_break_3;
                    }
                }
                
                double_break_3:;
                if (closest_item != 0){
                    closest_match = closest_item->index;
                }
                else{
                    closest_match = list.manifested_index_range.min;
                }
            }
            
        }
    }
    return(closest_match);
}

internal Layout_Item*
layout_get_first_with_index(Layout_Item_List list, i64 index){
    Layout_Item *result = 0;
    Layout_Item *prev = 0;
    for (Layout_Item_Block *block = list.first;
         block != 0;
         block = block->next){
        i64 count = block->item_count;
        Layout_Item *item = block->items;
        for (i32 i = 0; i < count; i += 1, item += 1){
            if (HasFlag(item->flags, LayoutItemFlag_Ghost_Character)){
                continue;
            }
            if (item->index > index){
                result = prev;
                goto done;
            }
            if (item->index == index){
                result = item;
                goto done;
            }
            prev = item;
        }
    }
    if (result == 0){
        result = prev;
    }
    done:;
    return(result);
}

internal Rect_f32
layout_box_of_pos(Layout_Item_List list, i64 index){
    Rect_f32 result = {};
    Layout_Item *item = layout_get_first_with_index(list, index);
    if (item != 0){
        result = item->rect;
    }
    return(result);
}

function Rect_f32
layout_padded_box_of_pos(Layout_Item_List list, i64 index){
    Rect_f32 result = {};
    Layout_Item *item = layout_get_first_with_index(list, index);
    if (item != 0){
        result.x0 = item->rect.x0;
        result.y0 = item->rect.y0;
        result.x1 = item->rect.x1;
        result.y1 = item->padded_y1;
    }
    return(result);
}

internal i64
layout_get_pos_at_character(Layout_Item_List list, i64 character){
    i64 result = 0;
    if (character <= 0){
        result = list.manifested_index_range.min;
    }
    else if (character >= list.character_count){
        result = list.manifested_index_range.max;
    }
    else{
        i64 counter = 0;
        i64 next_counter = 0;
        for (Layout_Item_Block *node = list.first;
             node != 0;
             node = node->next, counter = next_counter){
            next_counter = counter + node->character_count;
            if (character >= next_counter){
                continue;
            }
            
            i64 count = node->item_count;
            i64 relative_character = character - counter;
            i64 relative_character_counter = 0;
            
            Layout_Item *item = node->items;
            for (i64 i = 0; i < count; i += 1, item += 1){
                if (HasFlag(item->flags, LayoutItemFlag_Ghost_Character)){
                    continue;
                }
                if (relative_character_counter == relative_character){
                    result = item->index;
                    break;
                }
                relative_character_counter += 1;
            }
            
            break;
        }
    }
    return(result);
}

internal i64
layout_character_from_pos(Layout_Item_List list, i64 index){
    i64 result = 0;
    i64 prev_index = -1;
    if (index <= list.manifested_index_range.first){
        result = 0;
    }
    else if (index > list.manifested_index_range.one_past_last){
        result = list.character_count - 1;
    }
    else{
        for (Layout_Item_Block *node = list.first;
             node != 0;
             node = node->next){
            Layout_Item *item = node->items;
            i64 count = node->item_count;
            for (i64 i = 0; i < count; i += 1, item += 1){
                if (item->index >= index){
                    goto double_break;
                }
                if (item->index > prev_index){
                    prev_index = item->index;
                    result += 1;
                }
            }
        }
    }
    double_break:;
    return(result);
}

// BOTTOM

