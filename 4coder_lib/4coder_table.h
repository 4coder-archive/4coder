/*
4coder_table.h - Preversioning
no warranty implied; use at your own risk

This software is in the public domain. Where that dedication is not
recognized, you are granted a perpetual, irrevocable license to copy,
distribute, and modify this file as you see fit.
*/

// TOP

#if !defined(FCODER_TABLE_H)
#define FCODER_TABLE_H

#define TableHashEmpty 0
#define TableHashDeleted 1
#define TableHashMin 0x10000000

#include <string.h>

typedef u32 Hash_Function(void *item, void *arg);
typedef i32 Compare_Function(void *key, void *item, void *arg);

struct Table{
    u32 *hash_array;
    char *data_array;
    i32 count, max;
    i32 item_size;
};

static i32
table_required_mem_size(i32 table_size, i32 item_size){
    i32 hash_size = ((table_size*sizeof(u32)) + 7) & ~7;
    i32 mem_size = hash_size + table_size*item_size;
    return(mem_size);
}

static void
table_init_memory(Table *table, void *memory, i32 table_size, i32 item_size){
    i32 hash_size = table_size * sizeof(u32);
    hash_size = (hash_size + 7) & ~7;
    
    table->hash_array = (u32*)memory;
    table->data_array = (char*)(table->hash_array) + hash_size;
    
    table->count = 0;
    table->max = table_size;
    table->item_size = item_size;
}

static i32
table_at_capacity(Table *table){
    i32 result = true;
    if (table->count * 8 < table->max * 7){
        result = false;
    }
    return(result);
}

static i32
table_add(Table *table, void *item, void *arg, Hash_Function *hash_func, Compare_Function *comp_func){
    Assert(table->count * 8 < table->max * 7);
    
    u32 hash = (hash_func(item, arg) | TableHashMin);
    i32 i = hash % table->max;
    i32 start = i;
    u32 *inspect = table->hash_array + i;
    
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

static i32
table_find_pos(Table *table, void *search_key, void *arg, i32 *pos, i32 *index, Hash_Function *hash_func, Compare_Function *comp_func){
    Assert((table->count - 1) * 8 < table->max * 7);
    
    u32 hash = (hash_func(search_key, arg) | TableHashMin);
    i32 i = hash % table->max;
    i32 start = i;
    u32 *inspect = table->hash_array + i;
    
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

static void*
table_find_item(Table *table, void *search_key, void *arg, Hash_Function *hash_func, Compare_Function *comp_func){
    void *result = 0;
    i32 pos;
    if (table_find_pos(table, search_key, arg, &pos, 0, hash_func, comp_func)){
        result = table->data_array + pos;
    }
    return(result);
}

static void
table_remove_index(Table *table, i32 index){
    table->hash_array[index] = TableHashDeleted;
    --table->count;
}

static i32
table_remove_match(Table *table, void *search_key, void *arg, Hash_Function *hash_func, Compare_Function *comp_func){
    i32 result = false;
    i32 index;
    if (table_find_pos(table, search_key, arg, 0, &index, hash_func, comp_func)){
        table_remove_index(table, index);
        result = true;
    }
    return(result);
}

static void
table_clear(Table *table){
    table->count = 0;
    memset(table->hash_array, 0, table->max*sizeof(*table->hash_array));
}

static void
table_rehash(Table *src, Table *dst, void *arg, Hash_Function *hash_func, Compare_Function *comp_func){
    Assert((dst->count + src->count - 1) * 7 < dst->max * 8);
    Assert(dst->item_size == src->item_size);
    
    i32 count = src->count;
    i32 item_size = src->item_size;
    u32 *hash_item = src->hash_array;
    char *data_item = src->data_array;
    for (i32 i = 0, c = 0;
         c < count;
         ++i, ++hash_item, data_item += item_size){
        if (*hash_item >= TableHashMin){
            ++c;
            table_add(dst, data_item, arg, hash_func, comp_func);
        }
    }
}

static u32
tbl_string_hash(void *item, void *arg){
    String_Const_u8 *string = (String_Const_u8*)item;
    u32 x = 5381;
    (void)arg;
    u8 *str = string->str;
    umem len = string->size;
    for (umem i = 0; i < len; i += 1){
        u8 c = str[i];
        x = ((x << 5) + x) + c;
    }
    return(x);
}

static i32
tbl_string_compare(void *a, void *b, void *arg){
    String_Const_u8 *stra = (String_Const_u8*)a;
    String_Const_u8 *strb = (String_Const_u8*)b;
    i32 result = (!string_match(*stra, *strb));
    return(result);
}

struct Offset_String{
    umem offset;
    umem size;
};

static u32
tbl_offset_string_hash(void *item, void *arg){
    Offset_String *string = (Offset_String*)item;
    u32 x = 5381;
    u8 *str = ((u8*)arg) + string->offset;
    umem size = string->size;
    for (umem i = 0; i < size; i += 1){
        u8 c = str[i];
        x = ((x << 5) + x) + c;
    }
    return(x);
}

static i32
tbl_offset_string_compare(void *a, void *b, void *arg){
    Offset_String *ostra = (Offset_String*)a;
    Offset_String *ostrb = (Offset_String*)b;
    String_Const_u8 stra = SCu8((u8*)arg + ostra->offset, ostra->size);
    String_Const_u8 strb = SCu8((u8*)arg + ostrb->offset, ostrb->size);
    i32 result = (!string_match(stra, strb));
    return(result);
}

struct String_Space{
    char *space;
    i32 pos;
    i32 new_pos;
    i32 max;
};

static Offset_String
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

static void
strspace_keep_prev(String_Space *space){
    space->pos = space->new_pos;
}

static void
strspace_discard_prev(String_Space *space){
    space->new_pos = space->pos;
}

#endif

// BOTTOM

