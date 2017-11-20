/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 11.03.2017
 *
 * Implements some basic getters for fonts set up to make the font type opaque.
 *
 */

// TOP

internal Face_ID
font_get_id_by_name(System_Functions *system, String name){
    Face_ID id = 0;
    u32 count = system->font.get_count();
    for (Face_ID id_it = 1; id_it <= count; ++id_it){
        char str[256];
        i32 str_len = system->font.get_name_by_id(id_it, str, sizeof(str));
        if (str_len > 0){
            String font_name = make_string(str, str_len);
            if (match_ss(font_name, name)){
                id = id_it;
                break;
            }
        }
    }
    return(id);
}

internal Glyph_Page**
font_page_lookup(Font_Page_Storage *page_storage, u32 page_number, b32 get_empty_slot){
    Glyph_Page **result = 0;
    
    if (page_storage->page_max > 0){
        u32 first_index = page_number % page_storage->page_max;
        
        u32 range_count = 0;
        u32 ranges[4];
        if (first_index == 0){
            ranges[0] = 0;
            ranges[1] = page_storage->page_max;
            range_count = 2;
        }
        else{
            ranges[0] = first_index;
            ranges[1] = page_storage->page_max;
            ranges[2] = 0;
            ranges[3] = first_index;
            range_count = 4;
        }
        
        Glyph_Page **pages = page_storage->pages;
        if (get_empty_slot){
            for (u32 j = 0; j < range_count; j += 2){
                u32 stop = ranges[j+1];
                for (u32 i = ranges[j]; i < stop; ++i){
                    if (pages[i] == FONT_PAGE_EMPTY || pages[i] == FONT_PAGE_DELETED){
                        result = &pages[i];
                        goto break2;
                    }
                    if (pages[i]->page_number == page_number){
                        goto break2;
                    }
                }
            }
        }
        else{
            for (u32 j = 0; j < range_count; j += 2){
                u32 stop = ranges[j+1];
                for (u32 i = ranges[j]; i < stop; ++i){
                    if (pages[i] == FONT_PAGE_EMPTY){
                        goto break2;
                    }
                    if (pages[i] != FONT_PAGE_DELETED && pages[i]->page_number == page_number){
                        result = &pages[i];
                        goto break2;
                    }
                }
            }
        }
        
        break2:;
    }
    
    return(result);
}

internal Glyph_Page*
font_get_page(Font_Page_Storage *pages, u32 page_number){
    Glyph_Page *result = 0;
    if (page_number <= 0x10FF){
        Glyph_Page **page_get_result = font_page_lookup(pages, page_number, false);
        if (page_get_result != 0){
            result = *page_get_result;
        }
    }
    return(result);
}

internal Glyph_Page*
font_allocate_and_hash_new_page(System_Functions *system, Font_Page_Storage *storage, u32 page_number){
    Glyph_Page *new_page = 0;
    if (page_number <= 0x10FF){
        b32 has_space = true;
        
        // Grow and rehash the table if we need to now.
        u32 new_page_count = 1;
        u32 new_max = (storage->page_count + new_page_count)*3;
        if (storage->page_max < FONT_PAGE_MAX && new_max > storage->page_max*2){
            Glyph_Page **pages = (Glyph_Page**)system->font.allocate(sizeof(Glyph_Page*)*new_max);
            if (pages != 0){
                u32 old_max = storage->page_max;
                Glyph_Page **old_pages = storage->pages;
                storage->pages = pages;
                storage->page_max = new_max;
                memset(pages, 0, sizeof(*pages)*new_max);
                if (old_pages != 0){
                    for (u32 i = 0; i < old_max; ++i){
                        Glyph_Page *this_page = old_pages[i];
                        if (this_page != FONT_PAGE_EMPTY && this_page != FONT_PAGE_DELETED){
                            u32 this_page_number = this_page->page_number;
                            Glyph_Page **dest = font_page_lookup(storage, this_page_number, true);
                            Assert(dest != 0);
                            *dest = this_page;
                        }
                    }
                    system->font.free(old_pages);
                }
            }
            else{
                has_space = false;
            }
        }
        
        // Allocate and hash a new page if there is room in the table.
        if (has_space){
            new_page = (Glyph_Page*)system->font.allocate(sizeof(Glyph_Page));
            if (new_page != 0){
                Glyph_Page **dest = font_page_lookup(storage, page_number, true);
                Assert(dest != 0);
                *dest = new_page;
                storage->page_count += new_page_count;
            }
        }
    }
    return(new_page);
}

internal Glyph_Page*
font_make_page(System_Functions *system, Font_Settings *settings, Font_Metrics *metrics, Font_Page_Storage *pages, u32 page_number){
    Glyph_Page *new_page = font_allocate_and_hash_new_page(system, pages, page_number);
    if (new_page != 0){
        system->font.load_page(settings, metrics, new_page, page_number);
    }
    return(new_page);
}

internal b32
font_can_render(System_Functions *system, Font_Settings *settings, Font_Metrics *metrics, Font_Page_Storage *pages, u32 codepoint){
    b32 result = (codepoint <= 0x10FFFF);
    return(result);
}

///////
// HACK(allen): Hack optimizations
struct Font_Cached_Lookup_Result{
    Glyph_Page *page;
    u32 index;
};

internal Font_Cached_Lookup_Result
font_cached_lookup(Font_Page_Storage *pages, u32 page_number){
    Font_Cached_Lookup_Result result = {0};
    
    result.index = page_number % ArrayCount(pages->cache);
    if (pages->cache[result.index].page_number == page_number){
        result.page = pages->cache[result.index].page;
    }
    
    if (result.page == 0){
        result.page = font_get_page(pages, page_number);
    }
    
    return(result);
}

internal Glyph_Page*
font_cached_get_page(Font_Page_Storage *pages, u32 page_number){
    Font_Cached_Lookup_Result result = font_cached_lookup(pages, page_number);
    if (result.page != 0){
        pages->cache[result.index].page = result.page;
        pages->cache[result.index].page_number = page_number;
    }
    return(result.page);
}

internal Glyph_Page*
font_cached_get_or_make_page(System_Functions *system, Font_Settings *settings, Font_Metrics *metrics, Font_Page_Storage *pages, u32 page_number){
    Font_Cached_Lookup_Result result = font_cached_lookup(pages, page_number);
    if (result.page == 0){
        result.page = font_make_page(system, settings, metrics, pages, page_number);
    }
    if (result.page != 0){
        pages->cache[result.index].page = result.page;
        pages->cache[result.index].page_number = page_number;
    }
    return(result.page);
}
///////

internal f32
font_get_glyph_advance(System_Functions *system, Font_Settings *settings, Font_Metrics *metrics, Font_Page_Storage *pages, u32 codepoint){
    f32 result = 0.f;
    u32 page_number = (codepoint >> 8);
    Glyph_Page *page = font_cached_get_or_make_page(system, settings, metrics, pages, page_number);
    
    u32 glyph_index = codepoint & 0xFF;
    if (page != 0 && page->advance[glyph_index] > 0.f){
        result = page->advance[glyph_index];
    }
    return(result);
}

// BOTTOM

