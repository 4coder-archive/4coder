/*
4coder_codepoint_map.cpp - Codepoint map to index
*/

// TOP

function b32
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

function u16
codepoint_index_map_count(Codepoint_Index_Map *map){
    return(map->max_index + 1);
}

function f32
font_get_glyph_advance(Face_Advance_Map *map, Face_Metrics *metrics, u32 codepoint, f32 tab_multiplier){
    f32 result = 0.f;
    if (codepoint == '\t'){
        result = metrics->space_advance*tab_multiplier;
    }
    else{
        if (character_is_whitespace(codepoint)){
            codepoint = ' ';
        }
        u16 index = 0;
        if (codepoint_index_map_read(&map->codepoint_to_index, codepoint, &index)){
            if (index < map->index_count){
                result = map->advance[index];
            }
        }
    }
    return(result);
}

function f32
font_get_max_glyph_advance_range(Face_Advance_Map *map, Face_Metrics *metrics,
                                 u32 codepoint_first, u32 codepoint_last,
                                 f32 tab_multiplier){
    f32 result = font_get_glyph_advance(map, metrics, codepoint_first, tab_multiplier);
    for (u32 i = codepoint_first + 1; i <= codepoint_last; i += 1){
        f32 a = font_get_glyph_advance(map, metrics, i, tab_multiplier);
        result = Max(a, result);
    }
    return(result);
}

function f32
font_get_average_glyph_advance_range(Face_Advance_Map *map, Face_Metrics *metrics,
                                     u32 codepoint_first, u32 codepoint_last,
                                     f32 tab_multiplier){
    f32 result = 0.f;
    for (u32 i = codepoint_first; i <= codepoint_last; i += 1){
        result += font_get_glyph_advance(map, metrics, i, tab_multiplier);
    }
    result /= (f32)(codepoint_last - codepoint_first + 1);
    return(result);
}

// BOTTOM

