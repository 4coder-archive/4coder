/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 23.07.2019
 *
 * Face basic operations.
 *
 */

// TOP

internal b32
codepoint_index_map_read(Codepoint_Index_Map *map, u32 codepoint, u16 *index_out){
    b32 success = true;
    if (codepoint == 0 && map->has_zero_index){
        *index_out = map->zero_index;
    }
    else if (table_read(&map->table, codepoint, index_out)){
        // NOTE(allen): do nothing
    }
    else{
        success = false;
    }
    return(success);
}

internal u16
codepoint_index_map_count(Codepoint_Index_Map *map){
    return(map->max_index + 1);
}

internal f32
font_get_glyph_advance(Face *face, u32 codepoint){
    f32 result = 0.f;
    if (codepoint == '\t'){
        result = face->space_advance*4.f;
    }
    else{
        if (character_is_whitespace(codepoint)){
            codepoint = ' ';
        }
        u16 index = 0;
        if (codepoint_index_map_read(&face->codepoint_to_index_map, codepoint, &index)){
            if (index < face->index_count){
                result = face->advance[index];
            }
        }
    }
    return(result);
}

internal f32
font_get_max_glyph_advance_range(Face *face, u32 codepoint_first, u32 codepoint_last){
    f32 result = font_get_glyph_advance(face, codepoint_first);
    for (u32 i = codepoint_first + 1; i <= codepoint_last; i += 1){
        f32 a = font_get_glyph_advance(face, i);
        result = Max(a, result);
    }
    return(result);
}

internal f32
font_get_average_glyph_advance_range(Face *face, u32 codepoint_first, u32 codepoint_last){
    f32 result = 0.f;
    for (u32 i = codepoint_first; i <= codepoint_last; i += 1){
        result += font_get_glyph_advance(face, i);
    }
    result /= (f32)(codepoint_last - codepoint_first + 1);
    return(result);
}

// BOTTOM

