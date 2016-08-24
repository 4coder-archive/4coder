/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 14.02.2016
 *
 * C style genereic hash table
 *
 */

// TOP

#define TableHashEmpty 0
#define TableHashDeleted 1
#define TableHashMin 0x10000000

#include <stdint.h>
#include <assert.h>
#include <string.h>

typedef uint32_t Hash_Function(void *item, void *arg);
typedef int32_t Compare_Function(void *key, void *item, void *arg);

struct Table{
    uint32_t *hash_array;
    char *data_array;
    int32_t count, max;
    int32_t item_size;
};

static int32_t
table_required_mem_size(int32_t table_size, int32_t item_size){
    int32_t hash_size = ((table_size * sizeof(uint32_t)) + 7) & ~7;
    int32_t mem_size = hash_size + table_size * item_size;
    return(mem_size);
}

static void
table_init_memory(Table *table, void *memory, int32_t table_size, int32_t item_size){
    int32_t hash_size = table_size * sizeof(uint32_t);
    hash_size = (hash_size + 7) & ~7;
    
    table->hash_array = (uint32_t*)memory;
    table->data_array = (char*)(table->hash_array) + hash_size;
    
    table->count = 0;
    table->max = table_size;
    table->item_size = item_size;
}

static int32_t
table_at_capacity(Table *table){
    int32_t result = true;
    if (table->count * 8 < table->max * 7){
        result = false;
    }
    return(result);
}

static int32_t
table_add(Table *table, void *item, void *arg, Hash_Function *hash_func, Compare_Function *comp_func){
    assert(table->count * 8 < table->max * 7);
    
    uint32_t hash = (hash_func(item, arg) | TableHashMin);
    int32_t i = hash % table->max;
    int32_t start = i;
    uint32_t *inspect = table->hash_array + i;
    
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
        assert(i != start);
    }
    *inspect = hash;
    memcpy(table->data_array + i*table->item_size, item, table->item_size);
    ++table->count;
    
    return(0);
}

static int32_t
table_find_pos(Table *table, void *search_key, void *arg, int32_t *pos, int32_t *index, Hash_Function *hash_func, Compare_Function *comp_func){
    assert((table->count - 1) * 8 < table->max * 7);
    
    uint32_t hash = (hash_func(search_key, arg) | TableHashMin);
    int32_t i = hash % table->max;
    int32_t start = i;
    uint32_t *inspect = table->hash_array + i;
    
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
    int32_t pos;
    if (table_find_pos(table, search_key, arg, &pos, 0, hash_func, comp_func)){
        result = table->data_array + pos;
    }
    return(result);
}

inline void
table_remove_index(Table *table, int32_t index){
    table->hash_array[index] = TableHashDeleted;
    --table->count;
}

inline int32_t
table_remove_match(Table *table, void *search_key, void *arg, Hash_Function *hash_func, Compare_Function *comp_func){
    int32_t result = false;
    int32_t index;
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
    assert((dst->count + src->count - 1) * 7 < dst->max * 8);
    assert(dst->item_size == src->item_size);
    
    int32_t count = src->count;
    int32_t item_size = src->item_size;
    uint32_t *hash_item = src->hash_array;
    char *data_item = src->data_array;
    for (int32_t i = 0, c = 0;
         c < count;
         ++i, ++hash_item, data_item += item_size){
        if (*hash_item >= TableHashMin){
            ++c;
            table_add(dst, data_item, arg, hash_func, comp_func);
        }
    }
}

static uint32_t
tbl_string_hash(void *item, void *arg){
    String *string = (String*)item;
    char *str = string->str;
    int32_t i = 0, len = string->size;
    uint32_t x = 5381;
    char c;
    (void)arg;
    
    while (i < len){
        c = str[i++];
        x = ((x << 5) + x) + c;
    }
    
    return(x);
}

static int32_t
tbl_string_compare(void *a, void *b, void *arg){
    String *stra = (String*)a;
    String *strb = (String*)b;
    int32_t result = !match(*stra, *strb);
    return(result);
}

static uint32_t
tbl_offset_string_hash(void *item, void *arg){
    Offset_String *string = (Offset_String*)item;
    char *str;
    int32_t i,len;
    uint32_t x = 5381;
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

static int32_t
tbl_offset_string_compare(void *a, void *b, void *arg){
    Offset_String *ostra = (Offset_String*)a;
    Offset_String *ostrb = (Offset_String*)b;
    String stra = make_string((char*)arg + ostra->offset, ostra->size);
    String strb = make_string((char*)arg + ostrb->offset, ostrb->size);
    int32_t result = !match(stra, strb);
    return(result);
}

struct String_Space{
    char *space;
    int32_t pos, new_pos, max;
};

static Offset_String
strspace_append(String_Space *space, char *str, int32_t len){
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

// BOTTOM

