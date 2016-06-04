/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 14.02.2016
 *
 * 4tech C style genereic hash table
 *
 */

// TOP

#define TableHashEmpty 0
#define TableHashDeleted 1
#define TableHashMin 0x10000000

typedef u32 Hash_Function(void *item, void *arg);
typedef i32 Compare_Function(void *key, void *item, void *arg);

struct Table{
    u32 *hash_array;
    u8 *data_array;
    i32 count, max;
    
    i32 item_size;
};

internal i32
table_required_mem_size(i32 table_size, i32 item_size){
    i32 mem_size, hash_size;
    hash_size = ((table_size * sizeof(u32)) + 7) & ~7;
    mem_size = hash_size + table_size * item_size;
    return(mem_size);
}

internal void
table_init_memory(Table *table, void *memory, i32 table_size, i32 item_size){
    i32 hash_size = table_size * sizeof(u32);
    hash_size = (hash_size + 7) & ~7;
    
    table->hash_array = (u32*)memory;
    table->data_array = (u8*)(table->hash_array) + hash_size;
    
    table->count = 0;
    table->max = table_size;
    table->item_size = item_size;
}

internal b32
table_at_capacity(Table *table){
    b32 result = 1;
    if (table->count * 8 < table->max * 7){
        result = 0;
    }
    return(result);
}

internal b32
table_add(Table *table, void *item, void *arg, Hash_Function *hash_func, Compare_Function *comp_func){
    u32 hash, *inspect;
    i32 i, start;
    
    Assert(table->count * 8 < table->max * 7);
    
    hash = (hash_func(item, arg) | TableHashMin);
    i = hash % table->max;
    start = i;
    inspect = table->hash_array + i;
    
    while (*inspect >= TableHashMin){
        if (*inspect == hash){
            if (comp_func(item, table->data_array + i*table->item_size, arg) == 0){
                return(1);
            }
        }
        ++i;
        ++inspect;
        if (i == table->max){
            i = 0;
            inspect = table->hash_array;
        }
        Assert(i != start);
    }
    *inspect = hash;
    memcpy(table->data_array + i*table->item_size, item, table->item_size);
    ++table->count;
    
    return(0);
}

internal b32
table_find_pos(Table *table, void *search_key, void *arg, i32 *pos, i32 *index, Hash_Function *hash_func, Compare_Function *comp_func){
    u32 hash, *inspect;
    i32 i, start;
    
    Assert((table->count - 1) * 8 < table->max * 7);
    
    hash = (hash_func(search_key, arg) | TableHashMin);
    i = hash % table->max;
    start = i;
    inspect = table->hash_array + i;
    
    while (*inspect != TableHashEmpty){
        if (*inspect == hash){
            if (comp_func(search_key, table->data_array + i*table->item_size, arg) == 0){
                if (pos) *pos = i*table->item_size;
                if (index) *index = i;
                return(1);
            }
        }
        ++i;
        ++inspect;
        if (i == table->max){
            i = 0;
            inspect = table->hash_array;
        }
        if (i == start) break;
    }
    
    return(0);
}

inline void*
table_find_item(Table *table, void *search_key, void *arg, Hash_Function *hash_func, Compare_Function *comp_func){
    i32 pos;
    void *result = 0;
    if (table_find_pos(table, search_key, arg, &pos, 0, hash_func, comp_func)){
        result = table->data_array + pos;
    }
    return(result);
}

inline void
table_remove_index(Table *table, i32 index){
    table->hash_array[index] = TableHashDeleted;
    --table->count;
}

inline b32
table_remove_match(Table *table, void *search_key, void *arg, Hash_Function *hash_func, Compare_Function *comp_func){
    i32 index;
    b32 result = 0;
    if (table_find_pos(table, search_key, arg, 0, &index, hash_func, comp_func)){
        table_remove_index(table, index);
        result = 1;
    }
    return(result);
}

inline void
table_clear(Table *table){
    table->count = 0;
    memset(table->hash_array, 0, table->max*sizeof(*table->hash_array));
}

internal void
table_rehash(Table *src, Table *dst, void *arg, Hash_Function *hash_func, Compare_Function *comp_func){
    i32 i, c, count, item_size;
    u32 *hash_item;
    u8 *data_item;
    
    Assert((dst->count + src->count - 1) * 7 < dst->max * 8);
    Assert(dst->item_size == src->item_size);
    
    count = src->count;
    hash_item = src->hash_array;
    data_item = src->data_array;
    item_size = src->item_size;
    for (i = 0, c = 0; c < count; ++i, ++hash_item, data_item += item_size){
        if (*hash_item >= TableHashMin){
            ++c;
            table_add(dst, data_item, arg, hash_func, comp_func);
        }
    }
}

internal u32
tbl_string_hash(void *item, void *arg){
    String *string = (String*)item;
    char *str;
    i32 i,len;
    u32 x = 5381;
    char c;
    (void)arg;
    
    str = string->str;
    len = string->size;
    i = 0;
    while (i < len){
        c = str[i++];
        x = ((x << 5) + x) + c;
    }
    
    return(x);
}

internal i32
tbl_string_compare(void *a, void *b, void *arg){
    String *stra = (String*)a;
    String *strb = (String*)b;
    i32 result = !match(*stra, *strb);
    return(result);
}

internal u32
tbl_offset_string_hash(void *item, void *arg){
    Offset_String *string = (Offset_String*)item;
    char *str;
    i32 i,len;
    u32 x = 5381;
    char c;
    
    str = ((char*)arg) + string->offset;
    len = string->size;
    i = 0;
    while (i < len){
        c = str[i++];
        x = ((x << 5) + x) + c;
    }
    
    return(x);
}

internal i32
tbl_offset_string_compare(void *a, void *b, void *arg){
    Offset_String *ostra = (Offset_String*)a;
    Offset_String *ostrb = (Offset_String*)b;
    String stra = make_string((char*)arg + ostra->offset, ostra->size);
    String strb = make_string((char*)arg + ostrb->offset, ostrb->size);
    i32 result = !match(stra, strb);
    return(result);
}

struct String_Space{
    char *space;
    i32 pos, new_pos, max;
};

internal Offset_String
strspace_append(String_Space *space, char *str, i32 len){
    Offset_String result = {};
    if (space->new_pos + len <= space->max){
        result.offset = space->new_pos;
        result.size = len;
        
        memcpy(space->space + space->new_pos, str, len);
        space->new_pos = space->pos + len;
    }
    return(result);
}

internal void
strspace_keep_prev(String_Space *space){
    space->pos = space->new_pos;
}

internal void
strspace_discard_prev(String_Space *space){
    space->new_pos = space->pos;
}

// BOTTOM

