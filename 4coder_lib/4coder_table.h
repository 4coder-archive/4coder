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

// 4tech_standard_preamble.h
#if !defined(FTECH_INTEGERS)
#define FTECH_INTEGERS
#include <stdint.h>
typedef int8_t i8_4tech;
typedef int16_t i16_4tech;
typedef int32_t i32_4tech;
typedef int64_t i64_4tech;

typedef uint8_t u8_4tech;
typedef uint16_t u16_4tech;
typedef uint32_t u32_4tech;
typedef uint64_t u64_4tech;

#if defined(FTECH_32_BIT)
typedef u32_4tech umem_4tech;
#else
typedef u64_4tech umem_4tech;
#endif

typedef float f32_4tech;
typedef double f64_4tech;

typedef int8_t b8_4tech;
typedef int32_t b32_4tech;
#endif

#if !defined(Assert)
# define Assert(n) do{ if (!(n)) *(int*)0 = 0xA11E; }while(0)
#endif
// standard preamble end 

#define TableHashEmpty 0
#define TableHashDeleted 1
#define TableHashMin 0x10000000

#include <string.h>

typedef u32_4tech Hash_Function(void *item, void *arg);
typedef i32_4tech Compare_Function(void *key, void *item, void *arg);

struct Table{
    u32_4tech *hash_array;
    char *data_array;
    i32_4tech count, max;
    i32_4tech item_size;
};

static i32_4tech
table_required_mem_size(i32_4tech table_size, i32_4tech item_size){
    i32_4tech hash_size = ((table_size * sizeof(u32_4tech)) + 7) & ~7;
    i32_4tech mem_size = hash_size + table_size * item_size;
    return(mem_size);
}

static void
table_init_memory(Table *table, void *memory, i32_4tech table_size, i32_4tech item_size){
    i32_4tech hash_size = table_size * sizeof(u32_4tech);
    hash_size = (hash_size + 7) & ~7;
    
    table->hash_array = (u32_4tech*)memory;
    table->data_array = (char*)(table->hash_array) + hash_size;
    
    table->count = 0;
    table->max = table_size;
    table->item_size = item_size;
}

static i32_4tech
table_at_capacity(Table *table){
    i32_4tech result = true;
    if (table->count * 8 < table->max * 7){
        result = false;
    }
    return(result);
}

static i32_4tech
table_add(Table *table, void *item, void *arg, Hash_Function *hash_func, Compare_Function *comp_func){
    Assert(table->count * 8 < table->max * 7);
    
    u32_4tech hash = (hash_func(item, arg) | TableHashMin);
    i32_4tech i = hash % table->max;
    i32_4tech start = i;
    u32_4tech *inspect = table->hash_array + i;
    
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

static i32_4tech
table_find_pos(Table *table, void *search_key, void *arg, i32_4tech *pos, i32_4tech *index, Hash_Function *hash_func, Compare_Function *comp_func){
    Assert((table->count - 1) * 8 < table->max * 7);
    
    u32_4tech hash = (hash_func(search_key, arg) | TableHashMin);
    i32_4tech i = hash % table->max;
    i32_4tech start = i;
    u32_4tech *inspect = table->hash_array + i;
    
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
    void *result = 0;
    i32_4tech pos;
    if (table_find_pos(table, search_key, arg, &pos, 0, hash_func, comp_func)){
        result = table->data_array + pos;
    }
    return(result);
}

inline void
table_remove_index(Table *table, i32_4tech index){
    table->hash_array[index] = TableHashDeleted;
    --table->count;
}

inline i32_4tech
table_remove_match(Table *table, void *search_key, void *arg, Hash_Function *hash_func, Compare_Function *comp_func){
    i32_4tech result = false;
    i32_4tech index;
    if (table_find_pos(table, search_key, arg, 0, &index, hash_func, comp_func)){
        table_remove_index(table, index);
        result = true;
    }
    return(result);
}

inline void
table_clear(Table *table){
    table->count = 0;
    memset(table->hash_array, 0, table->max*sizeof(*table->hash_array));
}

static void
table_rehash(Table *src, Table *dst, void *arg, Hash_Function *hash_func, Compare_Function *comp_func){
    Assert((dst->count + src->count - 1) * 7 < dst->max * 8);
    Assert(dst->item_size == src->item_size);
    
    i32_4tech count = src->count;
    i32_4tech item_size = src->item_size;
    u32_4tech *hash_item = src->hash_array;
    char *data_item = src->data_array;
    for (i32_4tech i = 0, c = 0;
         c < count;
         ++i, ++hash_item, data_item += item_size){
        if (*hash_item >= TableHashMin){
            ++c;
            table_add(dst, data_item, arg, hash_func, comp_func);
        }
    }
}

static u32_4tech
tbl_string_hash(void *item, void *arg){
    String *string = (String*)item;
    u32_4tech x = 5381;
    (void)arg;
    
    char *str = string->str;
    i32_4tech len = string->size;
    i32_4tech i = 0;
    while (i < len){
        char c = str[i++];
        x = ((x << 5) + x) + c;
    }
    
    return(x);
}

static i32_4tech
tbl_string_compare(void *a, void *b, void *arg){
    String *stra = (String*)a;
    String *strb = (String*)b;
    i32_4tech result = !match_ss(*stra, *strb);
    return(result);
}

struct Offset_String{
    i32_4tech offset;
    i32_4tech size;
};

static u32_4tech
tbl_offset_string_hash(void *item, void *arg){
    Offset_String *string = (Offset_String*)item;
    u32_4tech x = 5381;
    
    char *str = ((char*)arg) + string->offset;
    i32_4tech len = string->size;
    i32_4tech i = 0;
    while (i < len){
        char c = str[i++];
        x = ((x << 5) + x) + c;
    }
    
    return(x);
}

static i32_4tech
tbl_offset_string_compare(void *a, void *b, void *arg){
    Offset_String *ostra = (Offset_String*)a;
    Offset_String *ostrb = (Offset_String*)b;
    String stra = make_string((char*)arg + ostra->offset, ostra->size);
    String strb = make_string((char*)arg + ostrb->offset, ostrb->size);
    i32_4tech result = !match_ss(stra, strb);
    return(result);
}

struct String_Space{
    char *space;
    i32_4tech pos;
    i32_4tech new_pos;
    i32_4tech max;
};

static Offset_String
strspace_append(String_Space *space, char *str, i32_4tech len){
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

