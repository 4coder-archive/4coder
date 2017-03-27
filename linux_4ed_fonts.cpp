/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 09.02.2016
 *
 * Shared system functions
 *
 */

// TOP

#include "4ed_system_shared.h"
#include "4ed_font_interface.h"
#include "4ed_font_interface_to_os.h"
#include "4ed_font_data.h"

struct Linux_Fonts{
    Partition part;
    Render_Font fonts[5];
    u32 font_count;
};

global Linux_Fonts linux_fonts = {0};

internal
Sys_Font_Get_Count_Sig(system_font_get_count){
    return(linux_fonts.font_count);
}

internal
Sys_Font_Get_IDs_By_Index_Sig(system_font_get_ids_by_index){
    b32 result = false;
    u32 stop_index = first_index + index_count;
    if (stop_index <= linux_fonts.font_count){
        result = true;
        for (u32 i = first_index; i < stop_index; ++i){
            id_out[i-first_index] = i;
        }
    }
    return(result);
}

internal
Sys_Font_Get_Name_By_Index_Sig(system_font_get_name_by_index){
    u32 length = 0;
    if (font_index < linux_fonts.font_count){
        Render_Font *font = &linux_fonts.fonts[font_index];
        char *name = font->name;
        length = font->name_len;
        copy_partial_cs(str_out, str_out_cap, make_string(name, length));
    }
    return(length);
}

internal
Sys_Font_Get_Name_By_ID_Sig(system_font_get_name_by_id){
    u32 font_index = font_id;
    u32 result = system_font_get_name_by_index(font_index, str_out, str_out_cap);
    return(result);
}

internal
Sys_Font_Get_Render_Data_By_ID_Sig(system_font_get_render_data_by_id){
    Render_Font *result = 0;
    u32 font_index = font_id;
    if (font_index < linux_fonts.font_count){
        result = &linux_fonts.fonts[font_index];
    }
    return(result);
}

internal
Sys_Font_Load_Page_Sig(system_font_load_page){
    system_set_page(&linuxvars.system, &linux_fonts.part, font, page, page_number, 16, true);
}

internal
Sys_Font_Allocate_Sig(system_font_allocate){
    void *result = system_memory_allocate(size);
    return(result);
}

internal
Sys_Font_Free_Sig(system_font_free){
    system_memory_free(ptr, 0);
}

internal
Sys_Font_Init_Sig(system_font_init){
    Partition *scratch = &shared_vars.scratch;
    Temp_Memory temp = begin_temp_memory(scratch);
    
    font->get_count = system_font_get_count;
    font->get_ids_by_index = system_font_get_ids_by_index;
    font->get_name_by_index = system_font_get_name_by_index;
    font->get_name_by_id = system_font_get_name_by_id;
    font->get_render_data_by_id = system_font_get_render_data_by_id;
    font->load_page = system_font_load_page;
    font->allocate = system_font_allocate;
    font->free = system_font_free;
    
    font_size = clamp_bottom(8, font_size);
    
    struct Font_Setup{
        Font_Setup *next_font;
        char *c_filename;
    };
    
    Font_Setup *first_setup = 0;
    Font_Setup *head_setup = 0;
    
    u32 dir_max = KB(32);
    u8 *directory = push_array(scratch, u8, dir_max);
    String dir_str = make_string_cap(directory, 0, dir_max);
    u32 dir_len = system_get_binary_path(&dir_str);
    Assert(dir_len < dir_max);
    
    {
        String dir_str = make_string_cap(directory, dir_len, dir_max);
        set_last_folder_sc(&dir_str, "fonts", '/');
        terminate_with_null(&dir_str);
        dir_len = dir_str.size;
    }
    
    partition_reduce(scratch, dir_max - dir_len - 1);
    partition_align(scratch, 8);
    
    File_List file_list = {0};
    system_set_file_list(&file_list, (char*)directory, 0, 0, 0);
    
    for (u32 i = 0; i < file_list.count; ++i){
        File_Info *info = &file_list.infos[i];
        if (first_setup == 0){
            first_setup = push_struct(scratch, Font_Setup);
            head_setup = first_setup;
        }
        else{
            head_setup->next_font = push_struct(scratch, Font_Setup);
            head_setup = head_setup->next_font;
        }
        head_setup->next_font = 0;
        
        char *filename = info->filename;
        u32 len = 0;
        for (;filename[len];++len);
        
        head_setup->c_filename = push_array(scratch, char, dir_len+len+1);
        memcpy(head_setup->c_filename, directory, dir_len);
        memcpy(head_setup->c_filename + dir_len, filename, len+1);
        
        partition_align(scratch, 8);
    }
    
    system_set_file_list(&file_list, 0, 0, 0, 0);
    
    u32 font_count_max = ArrayCount(linux_fonts.fonts);
    u32 font_count = 0;
    u32 i = 0;
    for (Font_Setup *ptr = first_setup; ptr != 0; ptr = ptr->next_font, ++i){
        if (i < font_count_max){
            Render_Font *render_font = &linux_fonts.fonts[i];
            
            system_set_font(&linuxvars.system, &linux_fonts.part, render_font, ptr->c_filename, font_size, use_hinting);
        }
        
        ++font_count;
    }
    
    linux_fonts.font_count = clamp_top(font_count, font_count_max);
    
    end_temp_memory(temp);
}

// BOTTOM



