/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 18.12.2015
 *
 * Font set for 4coder
 *
 */

// TOP

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
font_set_init(Font_Set *set, Partition *partition, i32 max, i16 live_max){
    partition_align(partition, 8);
    set->info = push_array(partition, Font_Info, max);
    partition_align(partition, 8);
    set->entries = push_array(partition, Font_Table_Entry, max);
    set->count = 0;
    set->max = max;
    
    partition_align(partition, 8);
    set->font_block = push_block(partition, live_max*(sizeof(Render_Font) + sizeof(Font_Slot)));

    set->free_slots = {};
    set->used_slots = {};

    set->free_slots.next = &set->free_slots;
    set->free_slots.prev = &set->free_slots;
    set->used_slots.next = &set->used_slots;
    set->used_slots.prev = &set->used_slots;
    
    char *ptr = (char*)set->font_block;
    for (i32 i = 0; i < live_max; ++i){
        font__insert(&set->free_slots, (Font_Slot*)ptr);
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
font_set_add_hash(Font_Set *set, String name, i16 font_id){
    Font_Table_Entry entry;
    entry.hash = font_hash(name);
    entry.name = name;
    entry.font_id = font_id;

    u32 i, j;
    i = entry.hash % set->max;
    j = i - 1;
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
font_set_load(Partition *partition, Font_Set *set, i16 font_id){
    Font_Info *info = get_font_info(set, font_id);
    Font_Slot *slot = set->free_slots.next;
    Assert(slot != &set->free_slots);
    font__remove(slot);
    font__insert(&set->used_slots, slot);
    
    Render_Font *font = (Render_Font*)(slot + 1);
    set->font_load(font, info->filename.str, info->pt_size, 4);
    info->font = font;
    slot->font_id = font_id;
}

internal void
font_set_evict_lru(Font_Set *set){
    Font_Slot *slot = set->used_slots.prev;
    Assert(slot != &set->used_slots);
    
    i16 font_id = slot->font_id;
    Font_Info *info = get_font_info(set, font_id);
    Assert(((Font_Slot*)info->font) - 1 == slot);
    
    set->release_font(info->font);
    
    info->font = 0;
    slot->font_id = 0;
    font__remove(slot);
    font__insert(&set->free_slots, slot);
}

internal void
font_set_use(Partition *partition, Font_Set *set, i16 font_id){
    b8 already_used;
    already_used = set->font_used_flags[font_id-1];

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
            font_set_load(partition, set, font_id);
        }
        slot = ((Font_Slot*)info->font) - 1;
    
        font__remove(slot);
        font__insert(&set->used_slots, slot);
    }
}

internal b32
font_set_add(Partition *partition, Font_Set *set,
             String filename, String name, i32 pt_size){
    b32 result = 0;
    if (font_set_can_add(set)){
        i16 font_id = (i16)(++set->count);
        Font_Info *info = get_font_info(set, font_id);
        info->filename = filename;
        info->name = name;
        info->pt_size = pt_size;
        set->font_info_load(partition, filename.str, pt_size, &info->height, &info->advance);
        font_set_add_hash(set, name, font_id);
        
        if (font_set_can_load(set)){
            font_set_load(partition, set, font_id);
        }
        
        result = 1;
    }
    return(result);
}

internal b32
font_set_find_pos(Font_Set *set, String name, u32 *position){
    b32 result;
    u32 hash, i, j;
    hash = font_hash(name);
    i = hash % set->max;
    j = i - 1;
    if (j <= 1) j += set->max;

    result = 0;
    Font_Table_Entry *entry;
    for (; i != j; ++i){
        if (i == set->max) i = 0;
        entry = set->entries + i;
        if (entry->hash == hash){
            if (match(name, entry->name)){
                result = 1;
                *position = i;
                break;
            }
        }
    }

    return(result);
}

internal b32
font_set_extract(Font_Set *set, String name, i16 *font_id){
    b32 result;
    u32 position;
    
    result = font_set_find_pos(set, name, &position);
    if (result){
        *font_id = set->entries[position].font_id;
    }

    return(result);
}

// BOTTOM

