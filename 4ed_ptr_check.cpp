/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 26.08.2018
 *
 * Pointer check table
 *
 */

// TOP

internal Ptr_Table
make_Ptr_table(void *mem, umem size){
    Ptr_Table table = {};
    i32 max = (i32)(size/8);
    if (max > 0){
        table.mem = mem;
        u8 *cursor = (u8*)mem;
        table.hashes = (u64*)cursor;
        cursor += 8*max;
        table.count = 0;
        table.max = max;
        block_fill_ones(table.hashes, sizeof(*table.hashes)*max);
    }
    return(table);
}

internal i32
max_to_memsize_Ptr_table(i32 max){
    return(max*8);
}

internal b32
at_max_Ptr_table(Ptr_Table *table){
    if (table->max > 0 && (table->count + 1)*8 <= table->max*7){
        return(false);
    }
    return(true);
}

internal b32
insert_Ptr_table(Ptr_Table *table, void**key){
    i32 max = table->max;
    if (max > 0){
        i32 count = table->count;
        if ((count + 1)*8 <= max*7){
            u64 hash = 0;
            block_copy(&hash, key, 8);
            if (hash >= 18446744073709551614ULL){ hash += 2; }
            i32 first_index = hash%max;
            i32 index = first_index;
            u64 *hashes = table->hashes;
            for (;;){
                if (hashes[index] == 18446744073709551615ULL){
                    table->dirty_slot_count += 1;
                }
                if (hashes[index] == 18446744073709551615ULL || hashes[index] == 18446744073709551614ULL){
                    hashes[index] = hash;
                    table->count += 1;
                    return(true);
                }
                if (hashes[index] == hash) return(false);
                index = (index + 1)%max;
                if (index == first_index) return(false);
            }
        }
    }
    return(false);
}

internal b32
lookup_Ptr_table(Ptr_Table *table, void**key){
    i32 max = table->max;
    if (max > 0){
        u64 hash = 0;
        block_copy(&hash, key, 8);
        if (hash >= 18446744073709551614ULL){ hash += 2; }
        i32 first_index = hash%max;
        i32 index = first_index;
        u64 *hashes = table->hashes;
        for (;;){
            if (hashes[index] == 18446744073709551615ULL) break;
            if (hashes[index] == hash){
                return(true);
            }
            index = (index + 1)%max;
            if (index == first_index) break;
        }
    }
    return(false);
}

internal b32
erase_Ptr_table(Ptr_Table *table, void**key){
    i32 max = table->max;
    if (max > 0 && table->count > 0){
        u64 hash = 0;
        block_copy(&hash, key, 8);
        if (hash >= 18446744073709551614ULL){ hash += 2; }
        i32 first_index = hash%max;
        i32 index = first_index;
        u64 *hashes = table->hashes;
        for (;;){
            if (hashes[index] == 18446744073709551615ULL) break;
            if (hashes[index] == hash){
                hashes[index] = 18446744073709551614ULL;
                table->count -= 1;
                return(true);
            }
            index = (index + 1)%max;
            if (index == first_index) break;
        }
    }
    return(false);
}

internal b32
move_Ptr_table(Ptr_Table *dst_table, Ptr_Table *src_table){
    if ((src_table->count + dst_table->count)*8 <= dst_table->max*7){
        i32 max = src_table->max;
        u64 *hashes = src_table->hashes;
        for (i32 index = 0; index < max; index += 1){
            if (hashes[index] != 18446744073709551615ULL && hashes[index] != 18446744073709551614ULL){
                void* key_;
                void**key = &key_;
                block_copy(key, &hashes[index], 8);
                insert_Ptr_table(dst_table, key);
            }
        }
        return(true);
    }
    return(false);
}

internal b32
insert_Ptr_table(Ptr_Table *table, void* key){
    return(insert_Ptr_table(table, &key));
}

internal b32
lookup_Ptr_table(Ptr_Table *table, void* key){
    return(lookup_Ptr_table(table, &key));
}

internal b32
erase_Ptr_table(Ptr_Table *table, void* key){
    return(erase_Ptr_table(table, &key));
}

////////////////////////////////

internal void
insert_Ptr_table(Heap *heap, Ptr_Table *table, void* key){
    if (at_max_Ptr_table(table)){
        i32 new_max = (table->max + 1)*2;
        i32 new_mem_size = max_to_memsize_Ptr_table(new_max);
        void *new_mem = heap_allocate(heap, new_mem_size);
        Ptr_Table new_table = make_Ptr_table(new_mem, new_mem_size);
        if (table->mem != 0){
            b32 result = move_Ptr_table(&new_table, table);
            Assert(result);
            AllowLocal(result);
            heap_free(heap, table->mem);
        }
        *table = new_table;
    }
    b32 result = insert_Ptr_table(table, &key);
    Assert(result);
    AllowLocal(result);
}

////////////////////////////////

internal u32_Ptr_Table
make_u32_Ptr_table(void *mem, umem size){
    u32_Ptr_Table table = {};
    i32 max = (i32)(size/16);
    if (max > 0){
        table.mem = mem;
        u8 *cursor = (u8*)mem;
        table.hashes = (u64*)cursor;
        cursor += 8*max;
        table.vals = (void**)cursor;
        table.count = 0;
        table.max = max;
        block_fill_ones(table.hashes, sizeof(*table.hashes)*max);
    }
    return(table);
}

internal i32
max_to_memsize_u32_Ptr_table(i32 max){
    return(max*16);
}

internal b32
at_max_u32_Ptr_table(u32_Ptr_Table *table){
    if (table->max > 0 && (table->count + 1)*8 <= table->max*7){
        return(false);
    }
    return(true);
}

internal b32
insert_u32_Ptr_table(u32_Ptr_Table *table, u32*key, void**val){
    i32 max = table->max;
    if (max > 0){
        i32 count = table->count;
        if ((count + 1)*8 <= max*7){
            u64 hash = 0;
            block_copy(&hash, key, 4);
            if (hash >= 18446744073709551614ULL){ hash += 2; }
            i32 first_index = hash%max;
            i32 index = first_index;
            u64 *hashes = table->hashes;
            for (;;){
                if (hashes[index] == 18446744073709551615ULL){
                    table->dirty_slot_count += 1;
                }
                if (hashes[index] == 18446744073709551615ULL || hashes[index] == 18446744073709551614ULL){
                    hashes[index] = hash;
                    table->vals[index] = *val;
                    table->count += 1;
                    return(true);
                }
                if (hashes[index] == hash) return(false);
                index = (index + 1)%max;
                if (index == first_index) return(false);
            }
        }
    }
    return(false);
}

internal u32_Ptr_Lookup_Result
lookup_u32_Ptr_table(u32_Ptr_Table *table, u32*key){
    u32_Ptr_Lookup_Result result = {};
    i32 max = table->max;
    if (max > 0){
        u64 hash = 0;
        block_copy(&hash, key, 4);
        if (hash >= 18446744073709551614ULL){ hash += 2; }
        i32 first_index = hash%max;
        i32 index = first_index;
        u64 *hashes = table->hashes;
        for (;;){
            if (hashes[index] == 18446744073709551615ULL) break;
            if (hashes[index] == hash){
                result.success = true;
                result.val = &table->vals[index];
                return(result);
            }
            index = (index + 1)%max;
            if (index == first_index) break;
        }
    }
    return(result);
}

internal b32
erase_u32_Ptr_table(u32_Ptr_Table *table, u32*key){
    i32 max = table->max;
    if (max > 0 && table->count > 0){
        u64 hash = 0;
        block_copy(&hash, key, 4);
        if (hash >= 18446744073709551614ULL){ hash += 2; }
        i32 first_index = hash%max;
        i32 index = first_index;
        u64 *hashes = table->hashes;
        for (;;){
            if (hashes[index] == 18446744073709551615ULL) break;
            if (hashes[index] == hash){
                hashes[index] = 18446744073709551614ULL;
                table->count -= 1;
                return(true);
            }
            index = (index + 1)%max;
            if (index == first_index) break;
        }
    }
    return(false);
}

internal b32
move_u32_Ptr_table(u32_Ptr_Table *dst_table, u32_Ptr_Table *src_table){
    if ((src_table->count + dst_table->count)*8 <= dst_table->max*7){
        i32 max = src_table->max;
        u64 *hashes = src_table->hashes;
        for (i32 index = 0; index < max; index += 1){
            if (hashes[index] != 18446744073709551615ULL && hashes[index] != 18446744073709551614ULL){
                u32 key_;
                u32*key = &key_;
                block_copy(key, &hashes[index], 4);
                void**val = &src_table->vals[index];
                insert_u32_Ptr_table(dst_table, key, val);
            }
        }
        return(true);
    }
    return(false);
}

internal b32
lookup_u32_Ptr_table(u32_Ptr_Table *table, u32 *key, void* *val_out){
    u32_Ptr_Lookup_Result result = lookup_u32_Ptr_table(table, key);
    if (result.success){
        *val_out = *result.val;
    }
    return(result.success);
}

internal b32
insert_u32_Ptr_table(u32_Ptr_Table *table, u32*key, void* val){
    return(insert_u32_Ptr_table(table, key, &val));
}

internal b32
insert_u32_Ptr_table(u32_Ptr_Table *table, u32 key, void**val){
    return(insert_u32_Ptr_table(table, &key, val));
}

internal b32
insert_u32_Ptr_table(u32_Ptr_Table *table, u32 key, void* val){
    return(insert_u32_Ptr_table(table, &key, &val));
}

internal u32_Ptr_Lookup_Result
lookup_u32_Ptr_table(u32_Ptr_Table *table, u32 key){
    return(lookup_u32_Ptr_table(table, &key));
}

internal b32
lookup_u32_Ptr_table(u32_Ptr_Table *table, u32 key, void* *val_out){
    return(lookup_u32_Ptr_table(table, &key, val_out));
}

internal b32
erase_u32_Ptr_table(u32_Ptr_Table *table, u32 key){
    return(erase_u32_Ptr_table(table, &key));
}

////////////////////////////////

internal void
insert_u32_Ptr_table(Heap *heap, u32_Ptr_Table *table, u32 key, void* val){
    if (at_max_u32_Ptr_table(table)){
        i32 new_max = (table->max + 1)*2;
        i32 new_mem_size = max_to_memsize_u32_Ptr_table(new_max);
        void *new_mem = heap_allocate(heap, new_mem_size);
        u32_Ptr_Table new_table = make_u32_Ptr_table(new_mem, new_mem_size);
        if (table->mem != 0){
            b32 result = move_u32_Ptr_table(&new_table, table);
            Assert(result);
            AllowLocal(result);
            heap_free(heap, table->mem);
        }
        *table = new_table;
    }
    b32 result = insert_u32_Ptr_table(table, &key, &val);
    Assert(result);
    AllowLocal(result);
}

// BOTTOM

