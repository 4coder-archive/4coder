/*
 * Mr. 4th Dimention - Allen Webster
 *
 * ??.??.2016
 * 
 * For getting the font files on Linux.
 * 
 */

// TOP

// TODO(allen): Get system fonts

internal b32
win32_ft_font_load(Partition *part, Render_Font *rf, char *name, i32 pt_size, i32 tab_width, b32 use_hinting){
    Temp_Memory temp = begin_temp_memory(part);
    char* filename = push_array(part, char, 256);
    b32 result = false;
    if (filename != 0){
        String str = make_string_cap(filename, 0, 256);
        sysshared_to_binary_path(&str, name);
        result = font_load_freetype(part, rf, filename, pt_size, tab_width, use_hinting);
    }
    end_temp_memory(temp);
    return(result);
}

// BOTTOM

