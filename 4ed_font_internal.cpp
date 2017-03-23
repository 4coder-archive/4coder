/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 10.03.2017
 *
 * Where I save crappy old font stuff.
 *
 */

// TOP

#include "font/4coder_font_data.h"

struct Font_Table_Entry{
    u32 hash;
    String name;
    Font_ID font_id;
};

struct Font_Info{
    Render_Font *font;
    String filename;
    String name;
    i32 pt_size;
};

struct Font_Slot{
    Font_Slot *next, *prev;
    Font_ID font_id;
    u8 padding[6];
};
global_const Font_Slot null_font_slot = {0};

#define Font_Load_Sig(name)\
i32 name(Render_Font *font_out, char *filename, char *fontname, i32 pt_size, i32 tab_width, b32 store_texture)
typedef Font_Load_Sig(Font_Load);

#define Font_Load_Page_Sig(name)\
i32 name(Render_Font *font, Glyph_Page *page, char *filename, i32 pt_size, i32 tab_width)
typedef Font_Load_Page_Sig(Font_Load_Page);

#define Release_Font_Sig(name) void name(Render_Font *font)
typedef Release_Font_Sig(Release_Font);

struct Font_Set{
    Font_Info *info;
    Font_Table_Entry *entries;
    u32 count, max;
    
    void *font_block;
    Font_Slot free_slots;
    Font_Slot used_slots;
    
    Font_Load *font_load;
    Font_Load_Page *font_load_page;
    Release_Font *release_font;
    
    b8 *font_used_flags;
    Font_ID used_this_frame;
    Font_ID live_max;
};

inline Font_Info*
get_font_info(Font_Set *set, Font_ID font_id){
    Font_Info *result = set->info + font_id - 1;
    return(result);
}

internal void
font_set_begin_render(Font_Set *font_set){
    font_set->used_this_frame = 0;
    memset(font_set->font_used_flags, 0, font_set->max);
}

inline u32
font_hash(String name){
    u32 x = 5381;
    char *p = name.str;
    for (i32 i = 0; i < name.size; ++i, ++p){
        x = ((x << 5) + x) ^ (*p);
    }
    return(x);
}

inline void
font__insert(Font_Slot *pos, Font_Slot *slot){
    Font_Slot *nex;
    nex = pos->next;
    
    slot->next = nex;
    slot->prev = pos;
    nex->prev = slot;
    pos->next = slot;
}

inline void
font__remove(Font_Slot *slot){
    Font_Slot *n, *p;
    n = slot->next;
    p = slot->prev;
    
    p->next = n;
    n->prev = p;
}

internal void
font_set_init(Font_Set *set, Partition *partition, i32 max, Font_ID live_max){
    partition_align(partition, 8);
    set->info = push_array(partition, Font_Info, max);
    partition_align(partition, 8);
    set->entries = push_array(partition, Font_Table_Entry, max);
    set->count = 0;
    set->max = max;
    
    partition_align(partition, 8);
    set->font_block = push_block(partition, live_max*(sizeof(Render_Font) + sizeof(Font_Slot)));
    
    set->free_slots = null_font_slot;
    set->used_slots = null_font_slot;
    
    dll_init_sentinel(&set->free_slots);
    dll_init_sentinel(&set->used_slots);
    
    char *ptr = (char*)set->font_block;
    for (i32 i = 0; i < live_max; ++i){
        dll_insert(&set->free_slots, (Font_Slot*)ptr);
        ptr += sizeof(Font_Slot) + sizeof(Render_Font);
    }
    
    set->font_used_flags = push_array(partition, b8, max);
    set->live_max = live_max;
}

internal b32
font_set_can_add(Font_Set *set){
    b32 result = 0;
    if (set->count*8 < set->max*7) result = 1;
    return(result);
}

internal void
font_set_add_hash(Font_Set *set, String name, Font_ID font_id){
    Font_Table_Entry entry;
    entry.hash = font_hash(name);
    entry.name = name;
    entry.font_id = font_id;
    
    u32 i = entry.hash % set->max;
    u32 j = i - 1;
    if (i <= 1) j += set->max;
    
    for (; i != j; ++i){
        if (i == set->max) i = 0;
        if (set->entries[i].font_id == 0){
            set->entries[i] = entry;
            break;
        }
    }
    
    Assert(i != j);
}

inline b32
font_set_can_load(Font_Set *set){
    b32 result = (set->free_slots.next != &set->free_slots);
    return(result);
}

internal void
font_set_load(Font_Set *set, Font_ID font_id){
    Font_Info *info = get_font_info(set, font_id);
    Font_Slot *slot = set->free_slots.next;
    Assert(slot != &set->free_slots);
    font__remove(slot);
    font__insert(&set->used_slots, slot);
    
    Render_Font *font = (Render_Font*)(slot + 1);
    set->font_load(font, info->filename.str, info->name.str, info->pt_size, 4, true);
    info->font = font;
    slot->font_id = font_id;
}

internal void
font_set_evict_lru(Font_Set *set){
    Font_Slot *slot = set->used_slots.prev;
    Assert(slot != &set->used_slots);
    
    Font_ID font_id = slot->font_id;
    Font_Info *info = get_font_info(set, font_id);
    Assert(((Font_Slot*)info->font) - 1 == slot);
    
    set->release_font(info->font);
    
    info->font = 0;
    slot->font_id = 0;
    font__remove(slot);
    font__insert(&set->free_slots, slot);
}

internal void
font_set_use(Font_Set *set, Font_ID font_id){
    b8 already_used = set->font_used_flags[font_id-1];
    
    if (!already_used){
        if (set->used_this_frame < set->live_max){
            ++set->used_this_frame;
            set->font_used_flags[font_id-1] = 1;
            already_used = 1;
        }
    }
    
    if (already_used){
        // TODO(allen): optimize if you don't mind!!!!
        Font_Info *info = get_font_info(set, font_id);
        Font_Slot *slot;
        if (info->font == 0){
            if (!font_set_can_load(set)){
                font_set_evict_lru(set);
            }
            font_set_load(set, font_id);
        }
        slot = ((Font_Slot*)info->font) - 1;
        
        font__remove(slot);
        font__insert(&set->used_slots, slot);
    }
}

internal b32
font_set_add(Font_Set *set, String filename, String name, i32 pt_size){
    b32 result = false;
    if (font_set_can_add(set)){
        Render_Font dummy_font = {0};
        Font_ID font_id = (i16)(++set->count);
        Font_Info *info = get_font_info(set, font_id);
        info->filename = filename;
        info->name = name;
        info->pt_size = pt_size;
        set->font_load(&dummy_font, info->filename.str, info->name.str, info->pt_size, 4, false);
        
        font_set_add_hash(set, name, font_id);
        
        if (font_set_can_load(set)){
            font_set_load(set, font_id);
        }
        
        result = true;
    }
    return(result);
}

internal b32
font_set_find_pos(Font_Set *set, String name, u32 *position){
    u32 hash = font_hash(name);
    u32 i = hash % set->max;
    u32 j = i - 1;
    if (j <= 1){
        j += set->max;
    }
    
    b32 result = 0;
    for (; i != j; ++i){
        if (i == set->max){
            i = 0;
        }
        
        Font_Table_Entry *entry = set->entries + i;
        if (entry->hash == hash){
            if (match_ss(name, entry->name)){
                result = 1;
                *position = i;
                break;
            }
        }
    }
    
    return(result);
}

internal b32
font_set_get_name(Font_Set *set, Font_ID font_id, String *name){
    Font_Info *info = get_font_info(set, font_id);
    b32 result = copy_checked_ss(name, info->name);
    return(result);
}

internal b32
font_set_extract(Font_Set *set, String name, Font_ID *font_id){
    u32 position;
    b32 result = font_set_find_pos(set, name, &position);
    if (result){
        *font_id = set->entries[position].font_id;
    }
    return(result);
}

//////////////////////////////////

internal b32
get_codepoint_can_render(Render_Font *font, u32 codepoint){
    b32 exists = false;
    if (codepoint < 0x10FFFF){
        exists = true;
    }
    return(exists);
}

struct Codepoint_Indexes{
    b32 exists;
    u32 page_number;
    u32 glyph_index;
};

internal Codepoint_Indexes
get_codepoint_page_number(Render_Font *font, u32 codepoint){
    Codepoint_Indexes result = {0};
    u32 page_number = (codepoint >> 8);
    if (page_number <= 0x10FF){
        result.exists = true;
        result.page_number = page_number;
        result.glyph_index = (codepoint & 0x000000FF);
    }
    return(result);
}

#define MAX_PAGE_COUNT     (u32)(0x1100)
#define GLYPH_PAGE_EMPTY   ((Glyph_Page*)(0))
#define GLYPH_PAGE_DELETED ((Glyph_Page*)(max_u64))
#define IS_REAL_FONT_PAGE(p) (((p) != GLYPH_PAGE_EMPTY) && ((p) != GLYPH_PAGE_DELETED))

internal Glyph_Page**
font_lookup_page(Render_Font *font, u32 page_number, b32 find_empty_slot){
    Glyph_Page **result = 0;
    if (font->page_max > 0){
        u32 first_index = page_number % font->page_max;
        
        u32 range_count = 0;
        u32 ranges[4];
        if (first_index == 0){
            ranges[0] = 0;
            ranges[1] = font->page_max;
            range_count = 2;
        }
        else{
            ranges[0] = first_index;
            ranges[1] = font->page_max;
            ranges[2] = 0;
            ranges[3] = first_index;
            range_count = 4;
        }
        
        if (find_empty_slot){
            for(u32 j = 0; j < range_count; j += 2){
                u32 start = ranges[j];
                u32 stop =  ranges[j+1];
                for (u32 i = start; i < stop; ++i){
                    Glyph_Page *ptr = font->pages[i];
                    if (ptr == GLYPH_PAGE_EMPTY || ptr == GLYPH_PAGE_DELETED){
                        result = &font->pages[i];
                        goto break2;
                    }
                    if (ptr->page_number == page_number){
                        goto break2;
                    }
                }
            }
        }
        else{
            for(u32 j = 0; j < range_count; j += 2){
                u32 start = ranges[j];
                u32 stop =  ranges[j+1];
                for (u32 i = start; i < stop; ++i){
                    Glyph_Page *ptr = font->pages[i];
                    if (ptr == GLYPH_PAGE_EMPTY){
                        goto break2;
                    }
                    if (ptr != GLYPH_PAGE_DELETED){
                        if (ptr->page_number == page_number){
                            result = &font->pages[i];
                            goto break2;
                        }
                    }
                }
            }
        }
        break2:;
    }
    return(result);
}

internal Glyph_Page*
font_get_or_make_page(Render_Font *font, u32 page_number){
    Glyph_Page *page = 0;
    if (page_number <= 0x10FF){
        Glyph_Page **page_ptr = font_lookup_page(font, page_number, false);
        page = 0;
        if (page_ptr != 0){
            page = *page_ptr;
        }
        
        if (page == 0){
            u32 new_count = 1;
            if (font->page_max < MAX_PAGE_COUNT && (font->page_count+new_count)*3 < font->page_max*2){
                u32 new_page_max = (font->page_count+new_count)*3;
                new_page_max = clamp_top(new_page_max, MAX_PAGE_COUNT);
                Glyph_Page **new_pages = (Glyph_Page**)ALLOCATE(new_page_max * sizeof(Glyph_Page*));
                
                u32 old_page_max = font->page_max;
                Glyph_Page **pages = font->pages;
                for (u32 i = 0; i < old_page_max; ++i){
                    Glyph_Page *current_page = pages[i];
                    if (current_page != GLYPH_PAGE_EMPTY && current_page != GLYPH_PAGE_DELETED){
                        Glyph_Page **dest = font_lookup_page(font, current_page->page_number, true);
                        Assert(dest != 0);
                        *dest = current_page;
                    }
                }
                
                FREE(font->pages);
                font->pages = new_pages;
                font->page_max = new_page_max;
            }
            
            Glyph_Page *new_page = (Glyph_Page*)ALLOCATE(sizeof(Glyph_Page));
            Glyph_Page **dest = font_lookup_page(font, page_number, true);
            *dest = new_page;
            ++font->page_count;
            
            //set->font_load_page(font, new_page, );
        }
    }
    return(page);
}

internal void
get_codepoint_memory(Render_Font *font, u32 codepoint, Glyph_Bounds **bounds_mem_out, f32 **advance_mem_out){
    Glyph_Bounds *bounds = 0;
    f32 *advance = 0;
    
    if (get_codepoint_can_render(font, codepoint)){
        Codepoint_Indexes indexes = get_codepoint_page_number(font, codepoint);
        Glyph_Page *page = font_get_or_make_page(font, indexes.page_number);
        bounds = &page->glyphs[indexes.glyph_index];
        advance = &page->advance[indexes.glyph_index];
    }
    
    *bounds_mem_out = bounds;
    *advance_mem_out = advance;
}

internal b32
get_codepoint_glyph_data(Render_Font *font, u32 codepoint, Glyph_Data *data_out){
    b32 success = false;
    if (get_codepoint_can_render(font, codepoint)){
        Codepoint_Indexes indexes = get_codepoint_page_number(font, codepoint);
        Glyph_Page *page = font_get_or_make_page(font, indexes.page_number);
        data_out->bounds = page->glyphs[indexes.glyph_index];
        data_out->tex = page->tex;
        data_out->tex_width = page->tex_width;
        data_out->tex_height = page->tex_height;
        success = true;
    }
    return(success);
}

internal f32
get_codepoint_advance(Render_Font *font, u32 codepoint){
    f32 advance = (f32)font->advance;
    if (get_codepoint_can_render(font, codepoint)){
        Codepoint_Indexes indexes = get_codepoint_page_number(font, codepoint);
        Glyph_Page *page = font_get_or_make_page(font, indexes.page_number);
        advance = page->advance[indexes.glyph_index];
    }
    return(advance);
}

internal b32
set_codepoint_advance(Render_Font *font, u32 codepoint, f32 value){
    b32 success = false;
    if (get_codepoint_can_render(font, codepoint)){
        Codepoint_Indexes indexes = get_codepoint_page_number(font, codepoint);
        Glyph_Page *page = font_get_or_make_page(font, indexes.page_number);
        page->advance[indexes.glyph_index] = value;
        success = true;
    }
    return(success);
}

// BOTTOM




