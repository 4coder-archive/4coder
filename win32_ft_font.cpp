// NOTE(allen): Thanks to insofaras.
// This is copy-pasted from some work he
// did to get free type working on linux.
// Once it is working on both sides it might
// be possible to pull some parts out as
// portable FT rendering.

internal b32
win32_ft_font_load(Partition *part, Render_Font *rf, char *name, 
                   i32 pt_size, i32 tab_width, b32 use_hinting){
    
    b32 result = 0;
    
    Temp_Memory temp = begin_temp_memory(part);
    
    char* filename = push_array(part, char, 256);
    
    if (filename != 0){
        String str = make_string_cap(filename, 0, 256);
        sysshared_to_binary_path(&str, name);
        
        result = font_load_freetype(part, rf, filename, pt_size, tab_width, use_hinting);
    }
    
    end_temp_memory(temp);
    
    return(result);
}


