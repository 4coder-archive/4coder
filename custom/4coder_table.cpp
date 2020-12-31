/*
 * 4coder tables
 */

// TOP

internal u64
table_hash(String_Const_u8 key){
    return(table_hash_u8((u8*)key.str, key.size) | bit_64);
}

global_const u64 table_empty_slot = 0;
global_const u64 table_erased_slot = 1;

global_const u64 table_empty_key = 0;
global_const u64 table_erased_key = max_u64;

global_const u32 table_empty_u32_key = 0;
global_const u32 table_erased_u32_key = max_u32;

////////////////////////////////

internal Table_u64_u64
make_table_u64_u64__inner(Base_Allocator *allocator, u32 slot_count, String_Const_u8 location){
    Table_u64_u64 table = {};
    table.allocator = allocator;
    slot_count = clamp_bot(8, slot_count);
    String_Const_u8 mem = base_allocate__inner(allocator, slot_count*(sizeof(*table.keys) + sizeof(*table.vals)), location);
    block_zero(mem.str, mem.size);
    table.memory = mem.str;
    table.keys = (u64*)table.memory;
    table.vals = (u64*)(table.keys + slot_count);
    table.slot_count = slot_count;
    table.used_count = 0;
    table.dirty_count = 0;
    return(table);
}

#define make_table_u64_u64(a,s) make_table_u64_u64__inner((a),(s),file_name_line_number_lit_u8)

internal void
table_free(Table_u64_u64 *table){
    base_free(table->allocator, table->memory);
    block_zero_struct(table);
}

internal Table_Lookup
table_lookup(Table_u64_u64 *table, u64 key){
    Table_Lookup result = {};
    
    if (key != table_empty_key && key != table_erased_key &&
        table->slot_count > 0){
        u64 *keys = table->keys;
        u32 slot_count = table->slot_count;
        
        u32 first_index = key % slot_count;
        u32 index = first_index;
        result.hash = key;
        for (;;){
            if (key == keys[index]){
                result.index = index;
                result.found_match = true;
                result.found_empty_slot = false;
                result.found_erased_slot = false;
                break;
            }
            if (table_empty_key == keys[index]){
                if (!result.found_erased_slot){
                    result.index = index;
                    result.found_empty_slot = true;
                }
                break;
            }
            if (table_erased_key == keys[index]){
                if (!result.found_erased_slot){
                    result.index = index;
                    result.found_erased_slot = true;
                }
            }
            index += 1;
            if (index >= slot_count){
                index = 0;
            }
            if (index == first_index){
                break;
            }
        }
    }
    
    return(result);
}

internal b32
table_read(Table_u64_u64 *table, Table_Lookup lookup, u64 *val_out){
    b32 result = false;
    if (lookup.found_match){
        *val_out = table->vals[lookup.index];
        result = true;
    }
    return(result);
}

internal b32
table_read(Table_u64_u64 *table, u64 key, u64 *val_out){
    Table_Lookup lookup = table_lookup(table, key);
    return(table_read(table, lookup, val_out));
}

internal void
table_insert__inner(Table_u64_u64 *table, Table_Lookup lookup, u64 val){
    Assert(lookup.found_empty_slot || lookup.found_erased_slot);
    table->keys[lookup.index] = lookup.hash;
    table->vals[lookup.index] = val;
    table->used_count += 1;
    if (lookup.found_empty_slot){
        table->dirty_count += 1;
    }
}

internal b32
table_rehash(Table_u64_u64 *dst, Table_u64_u64 *src){
    b32 result = false;
    u32 src_slot_count = src->slot_count;
    if ((dst->dirty_count + src->used_count)*8 < dst->slot_count*7){
        u64 *src_keys = src->keys;
        for (u32 i = 0; i < src_slot_count; i += 1){
            u64 key = src_keys[i];
            if (key != table_empty_key &&
                key != table_erased_key){
                Table_Lookup lookup = table_lookup(dst, key);
                table_insert__inner(dst, lookup, src->vals[i]);
            }
        }
        result = true;
    }
    return(result);
}

internal b32
table_insert(Table_u64_u64 *table, u64 key, u64 val){
    b32 result = false;
    if (key != table_empty_key && key != table_erased_key){
        Table_Lookup lookup = table_lookup(table, key);
        if (!lookup.found_match){
            if ((table->dirty_count + 1)*8 >= table->slot_count*7){
                i32 new_slot_count = table->slot_count;
                if (table->used_count*2 >= table->slot_count){
                    new_slot_count = table->slot_count*4;
                }
                Table_u64_u64 new_table = make_table_u64_u64(table->allocator, new_slot_count);
                table_rehash(&new_table, table);
                table_free(table);
                *table = new_table;
                lookup = table_lookup(table, key);
                Assert(lookup.found_empty_slot);
            }
            table_insert__inner(table, lookup, val);
            result = true;
        }
    }
    return(result);
}

internal b32
table_erase(Table_u64_u64 *table, Table_Lookup lookup){
    b32 result = false;
    if (lookup.found_match){
        table->keys[lookup.index] = table_erased_key;
        table->vals[lookup.index] = 0;
        table->used_count -= 1;
        result = true;
    }
    return(result);
}

internal b32
table_erase(Table_u64_u64 *table, u64 key){
    Table_Lookup lookup = table_lookup(table, key);
    return(table_erase(table, lookup));
}

internal void
table_clear(Table_u64_u64 *table){
    block_zero_dynamic_array(table->keys, table->slot_count);
    block_zero_dynamic_array(table->vals, table->slot_count);
}

////////////////////////////////

internal Table_u32_u16
make_table_u32_u16__inner(Base_Allocator *allocator, u32 slot_count, String_Const_u8 location){
    Table_u32_u16 table = {};
    table.allocator = allocator;
    slot_count = clamp_bot(8, slot_count);
    String_Const_u8 mem = base_allocate__inner(allocator, slot_count*(sizeof(*table.keys) + sizeof(*table.vals)), location);
    block_zero(mem.str, mem.size);
    table.memory = mem.str;
    table.keys = (u32*)table.memory;
    table.vals = (u16*)(table.keys + slot_count);
    table.slot_count = slot_count;
    table.used_count = 0;
    table.dirty_count = 0;
    return(table);
}

#define make_table_u32_u16(a,s) make_table_u32_u16__inner((a),(s),file_name_line_number_lit_u8)

internal void
table_free(Table_u32_u16 *table){
    base_free(table->allocator, table->memory);
    block_zero_struct(table);
}

internal Table_Lookup
table_lookup(Table_u32_u16 *table, u32 key){
    Table_Lookup result = {};
    
    if (key != table_empty_u32_key && key != table_erased_u32_key &&
        table->slot_count > 0){
        u32 *keys = table->keys;
        u32 slot_count = table->slot_count;
        
        u32 first_index = key % slot_count;
        u32 index = first_index;
        result.hash = key;
        for (;;){
            if (key == keys[index]){
                result.index = index;
                result.found_match = true;
                result.found_empty_slot = false;
                result.found_erased_slot = false;
                break;
            }
            if (table_empty_u32_key == keys[index]){
                if (!result.found_erased_slot){
                    result.index = index;
                    result.found_empty_slot = true;
                }
                break;
            }
            if (table_erased_u32_key == keys[index]){
                if (!result.found_erased_slot){
                    result.index = index;
                    result.found_erased_slot = true;
                }
            }
            index += 1;
            if (index >= slot_count){
                index = 0;
            }
            if (index == first_index){
                break;
            }
        }
    }
    
    return(result);
}

internal b32
table_read(Table_u32_u16 *table, u32 key, u16 *val_out){
    b32 result = false;
    Table_Lookup lookup = table_lookup(table, key);
    if (lookup.found_match){
        *val_out = table->vals[lookup.index];
        result = true;
    }
    return(result);
}

internal void
table_insert__inner(Table_u32_u16 *table, Table_Lookup lookup, u32 key, u16 val){
    Assert(lookup.found_empty_slot || lookup.found_erased_slot);
    table->keys[lookup.index] = key;
    table->vals[lookup.index] = val;
    table->used_count += 1;
    if (lookup.found_empty_slot){
        table->dirty_count += 1;
    }
}

internal b32
table_rehash(Table_u32_u16 *dst, Table_u32_u16 *src){
    b32 result = false;
    u32 src_slot_count = src->slot_count;
    if ((dst->dirty_count + src->used_count)*8 < dst->slot_count*7){
        u32 *src_keys = src->keys;
        for (u32 i = 0; i < src_slot_count; i += 1){
            u32 key = src_keys[i];
            if (key != table_empty_u32_key && key != table_erased_u32_key){
                Table_Lookup lookup = table_lookup(dst, key);
                table_insert__inner(dst, lookup, key, src->vals[i]);
            }
        }
        result = true;
    }
    return(result);
}

internal b32
table_insert(Table_u32_u16 *table, u32 key, u16 val){
    b32 result = false;
    if (key != table_empty_u32_key && key != table_erased_u32_key){
        Table_Lookup lookup = table_lookup(table, key);
        if (!lookup.found_match){
            if ((table->dirty_count + 1)*8 >= table->slot_count*7){
                i32 new_slot_count = table->slot_count;
                if (table->used_count*2 >= table->slot_count){
                    new_slot_count = table->slot_count*4;
                }
                Table_u32_u16 new_table = make_table_u32_u16(table->allocator, new_slot_count);
                table_rehash(&new_table, table);
                table_free(table);
                *table = new_table;
                lookup = table_lookup(table, key);
                Assert(lookup.found_empty_slot);
            }
            table_insert__inner(table, lookup, key, val);
            result = true;
        }
    }
    return(result);
}

internal b32
table_erase(Table_u32_u16 *table, Table_Lookup lookup){
    b32 result = false;
    if (lookup.found_match){
        table->keys[lookup.index] = table_erased_u32_key;
        table->vals[lookup.index] = 0;
        table->used_count -= 1;
        result = true;
    }
    return(result);
}

internal b32
table_erase(Table_u32_u16 *table, u32 key){
    Table_Lookup lookup = table_lookup(table, key);
    return(table_erase(table, lookup));
}

internal void
table_clear(Table_u32_u16 *table){
    block_zero_dynamic_array(table->keys, table->slot_count);
    block_zero_dynamic_array(table->vals, table->slot_count);
    table->used_count = 0;
    table->dirty_count = 0;
}

////////////////////////////////

internal Table_Data_u64
make_table_Data_u64__inner(Base_Allocator *allocator, u32 slot_count, String_Const_u8 location){
    Table_Data_u64 table = {};
    table.allocator = allocator;
    slot_count = clamp_bot(8, slot_count);
    String_Const_u8 mem = base_allocate__inner(allocator, slot_count*(sizeof(*table.hashes) + sizeof(*table.keys) + sizeof(*table.vals)), location);
    block_zero(mem.str, mem.size);
    table.memory = mem.str;
    table.hashes = (u64*)table.memory;
    table.keys = (String_Const_u8*)(table.hashes + slot_count);
    table.vals = (u64*)(table.keys + slot_count);
    table.slot_count = slot_count;
    table.used_count = 0;
    table.dirty_count = 0;
    return(table);
}

#define make_table_Data_u64(a,s) make_table_Data_u64__inner((a),(s),file_name_line_number_lit_u8)

internal void
table_free(Table_Data_u64 *table){
    base_free(table->allocator, table->memory);
    block_zero_struct(table);
}

internal Table_Lookup
table_lookup(Table_Data_u64 *table, String_Const_u8 key){
    Table_Lookup result = {};
    
    if (table->slot_count > 0){
        u64 *hashes = table->hashes;
        u32 slot_count = table->slot_count;
        
        u64 hash = table_hash(key);
        u32 first_index = hash % slot_count;
        u32 index = first_index;
        result.hash = hash;
        for (;;){
            if (hash == hashes[index]){
                if (data_match(key, table->keys[index])){
                    result.index = index;
                    result.found_match = true;
                    result.found_empty_slot = false;
                    result.found_erased_slot = false;
                    break;
                }
            }
            if (table_empty_slot == hashes[index]){
                if (!result.found_erased_slot){
                    result.index = index;
                    result.found_empty_slot = true;
                }
                break;
            }
            if (table_erased_slot == hashes[index]){
                if (!result.found_erased_slot){
                    result.index = index;
                    result.found_erased_slot = true;
                }
            }
            index += 1;
            if (index >= slot_count){
                index = 0;
            }
            if (index == first_index){
                break;
            }
        }
    }
    
    return(result);
}

internal b32
table_read(Table_Data_u64 *table, Table_Lookup lookup, u64 *val_out){
    b32 result = false;
    if (lookup.found_match){
        *val_out = table->vals[lookup.index];
        result = true;
    }
    return(result);
}

internal b32
table_read_key(Table_Data_u64 *table, Table_Lookup lookup, String_Const_u8 *key_out){
    b32 result = false;
    if (lookup.found_match){
        *key_out = table->keys[lookup.index];
        result = true;
    }
    return(result);
}

internal b32
table_read(Table_Data_u64 *table, String_Const_u8 key, u64 *val_out){
    Table_Lookup lookup = table_lookup(table, key);
    return(table_read(table, lookup, val_out));
}

internal void
table_insert__inner(Table_Data_u64 *table, Table_Lookup lookup, String_Const_u8 key, u64 val){
    Assert(lookup.found_empty_slot || lookup.found_erased_slot);
    table->hashes[lookup.index] = lookup.hash;
    table->keys[lookup.index] = key;
    table->vals[lookup.index] = val;
    table->used_count += 1;
    if (lookup.found_empty_slot){
        table->dirty_count += 1;
    }
}

internal b32
table_rehash(Table_Data_u64 *dst, Table_Data_u64 *src){
    b32 result = false;
    u32 src_slot_count = src->slot_count;
    if ((dst->dirty_count + src->used_count)*8 < dst->slot_count*7){
        u64 *src_hashes = src->hashes;
        for (u32 i = 0; i < src_slot_count; i += 1){
            if (HasFlag(src_hashes[i], bit_64)){
                String_Const_u8 key = src->keys[i];
                Table_Lookup lookup = table_lookup(dst, key);
                table_insert__inner(dst, lookup, key, src->vals[i]);
            }
        }
        result = true;
    }
    return(result);
}

internal b32
table_insert(Table_Data_u64 *table, String_Const_u8 key, u64 val){
    b32 result = false;
    if (key.str != 0){
        Table_Lookup lookup = table_lookup(table, key);
        if (!lookup.found_match){
            if ((table->dirty_count + 1)*8 >= table->slot_count*7){
                i32 new_slot_count = table->slot_count;
                if (table->used_count*2 >= table->slot_count){
                    new_slot_count = table->slot_count*4;
                }
                Table_Data_u64 new_table = make_table_Data_u64(table->allocator, new_slot_count);
                table_rehash(&new_table, table);
                table_free(table);
                *table = new_table;
                lookup = table_lookup(table, key);
                Assert(lookup.found_empty_slot);
            }
            table_insert__inner(table, lookup, key, val);
            result = true;
        }
    }
    return(result);
}

internal b32
table_erase(Table_Data_u64 *table, String_Const_u8 key){
    b32 result = false;
    Table_Lookup lookup = table_lookup(table, key);
    if (lookup.found_match){
        table->hashes[lookup.index] = table_erased_slot;
        block_zero_struct(&table->keys[lookup.index]);
        table->vals[lookup.index] = 0;
        table->used_count -= 1;
        result = true;
    }
    return(result);
}

internal void
table_clear(Table_Data_u64 *table){
    block_zero_dynamic_array(table->hashes, table->slot_count);
    block_zero_dynamic_array(table->keys, table->slot_count);
    block_zero_dynamic_array(table->vals, table->slot_count);
    table->used_count = 0;
    table->dirty_count = 0;
}

////////////////////////////////

internal Table_u64_Data
make_table_u64_Data__inner(Base_Allocator *allocator, u32 slot_count, String_Const_u8 location){
    Table_u64_Data table = {};
    table.allocator = allocator;
    slot_count = clamp_bot(8, slot_count);
    String_Const_u8 mem = base_allocate__inner(allocator, slot_count*(sizeof(*table.keys) + sizeof(*table.vals)), location);
    block_zero(mem.str, mem.size);
    table.memory = mem.str;
    table.keys = (u64*)table.memory;
    table.vals = (String_Const_u8*)(table.keys + slot_count);
    table.slot_count = slot_count;
    table.used_count = 0;
    table.dirty_count = 0;
    return(table);
}

#define make_table_u64_Data(a,s) make_table_u64_Data__inner((a),(s),file_name_line_number_lit_u8)

internal void
table_free(Table_u64_Data *table){
    base_free(table->allocator, table->memory);
    block_zero_struct(table);
}

internal Table_Lookup
table_lookup(Table_u64_Data *table, u64 key){
    Table_Lookup result = {};
    
    if (key != table_empty_key && key != table_erased_key &&
        table->slot_count > 0){
        u64 *keys = table->keys;
        u32 slot_count = table->slot_count;
        
        u32 first_index = key % slot_count;
        u32 index = first_index;
        result.hash = key;
        for (;;){
            if (key == keys[index]){
                result.index = index;
                result.found_match = true;
                result.found_empty_slot = false;
                result.found_erased_slot = false;
                break;
            }
            if (table_empty_key == keys[index]){
                if (!result.found_erased_slot){
                    result.index = index;
                    result.found_empty_slot = true;
                }
                break;
            }
            if (table_erased_key == keys[index]){
                if (!result.found_erased_slot){
                    result.index = index;
                    result.found_erased_slot = true;
                }
            }
            index += 1;
            if (index >= slot_count){
                index = 0;
            }
            if (index == first_index){
                break;
            }
        }
    }
    
    return(result);
}

internal b32
table_read(Table_u64_Data *table, Table_Lookup lookup, String_Const_u8 *val_out){
    b32 result = false;
    if (lookup.found_match){
        *val_out = table->vals[lookup.index];
        result = true;
    }
    return(result);
}

internal b32
table_read(Table_u64_Data *table, u64 key, String_Const_u8 *val_out){
    Table_Lookup lookup = table_lookup(table, key);
    return(table_read(table, lookup, val_out));
}

internal void
table_insert__inner(Table_u64_Data *table, Table_Lookup lookup, String_Const_u8 val){
    Assert(lookup.found_empty_slot || lookup.found_erased_slot);
    table->keys[lookup.index] = lookup.hash;
    table->vals[lookup.index] = val;
    table->used_count += 1;
    if (lookup.found_empty_slot){
        table->dirty_count += 1;
    }
}

internal b32
table_rehash(Table_u64_Data *dst, Table_u64_Data *src){
    b32 result = false;
    u32 src_slot_count = src->slot_count;
    if ((dst->dirty_count + src->used_count)*8 < dst->slot_count*7){
        u64 *src_keys = src->keys;
        for (u32 i = 0; i < src_slot_count; i += 1){
            u64 key = src_keys[i];
            if (key != table_empty_key &&
                key != table_erased_key){
                Table_Lookup lookup = table_lookup(dst, key);
                table_insert__inner(dst, lookup, src->vals[i]);
            }
        }
        result = true;
    }
    return(result);
}

internal b32
table_insert(Table_u64_Data *table, u64 key, String_Const_u8 val){
    b32 result = false;
    if (key != table_empty_key && table_erased_key){
        Table_Lookup lookup = table_lookup(table, key);
        if (!lookup.found_match){
            if ((table->dirty_count + 1)*8 >= table->slot_count*7){
                i32 new_slot_count = table->slot_count;
                if (table->used_count*2 >= table->slot_count){
                    new_slot_count = table->slot_count*4;
                }
                Table_u64_Data new_table = make_table_u64_Data(table->allocator, new_slot_count);
                table_rehash(&new_table, table);
                table_free(table);
                *table = new_table;
                lookup = table_lookup(table, key);
                Assert(lookup.found_empty_slot);
            }
            table_insert__inner(table, lookup, val);
            result = true;
        }
    }
    return(result);
}

internal b32
table_erase(Table_u64_Data *table, Table_Lookup lookup){
    b32 result = false;
    if (lookup.found_match){
        table->keys[lookup.index] = 0;
        block_zero_struct(&table->vals[lookup.index]);
        table->used_count -= 1;
        result = true;
    }
    return(result);
}

internal b32
table_erase(Table_u64_Data *table, u64 key){
    Table_Lookup lookup = table_lookup(table, key);
    return(table_erase(table, lookup));
}

internal void
table_clear(Table_u64_Data *table){
    block_zero_dynamic_array(table->keys, table->slot_count);
    block_zero_dynamic_array(table->vals, table->slot_count);
    table->used_count = 0;
    table->dirty_count = 0;
}

////////////////////////////////

internal Table_Data_Data
make_table_Data_Data__inner(Base_Allocator *allocator, u32 slot_count, String_Const_u8 location){
    Table_Data_Data table = {};
    table.allocator = allocator;
    slot_count = clamp_bot(8, slot_count);
    String_Const_u8 mem = base_allocate__inner(allocator, slot_count*(sizeof(*table.hashes) + sizeof(*table.keys) + sizeof(*table.vals)), location);
    block_zero(mem.str, mem.size);
    table.memory = mem.str;
    table.hashes = (u64*)table.memory;
    table.keys = (String_Const_u8*)(table.hashes + slot_count);
    table.vals = (String_Const_u8*)(table.keys + slot_count);
    table.slot_count = slot_count;
    table.used_count = 0;
    table.dirty_count = 0;
    return(table);
}

#define make_table_Data_Data(a,s) make_table_Data_Data__inner((a),(s),file_name_line_number_lit_u8)

internal void
table_free(Table_Data_Data *table){
    base_free(table->allocator, table->memory);
    block_zero_struct(table);
}

internal Table_Lookup
table_lookup(Table_Data_Data *table, String_Const_u8 key){
    Table_Lookup result = {};
    
    if (table->slot_count > 0){
        u64 *hashes = table->hashes;
        u32 slot_count = table->slot_count;
        
        u64 hash = table_hash(key);
        u32 first_index = hash % slot_count;
        u32 index = first_index;
        result.hash = hash;
        for (;;){
            if (hash == hashes[index]){
                if (data_match(key, table->keys[index])){
                    result.index = index;
                    result.found_match = true;
                    result.found_empty_slot = false;
                    result.found_erased_slot = false;
                    break;
                }
            }
            if (table_empty_slot == hashes[index]){
                if (!result.found_erased_slot){
                    result.index = index;
                    result.found_empty_slot = true;
                }
                break;
            }
            if (table_erased_slot == hashes[index] && !result.found_erased_slot){
                result.index = index;
                result.found_erased_slot = true;
            }
            index += 1;
            if (index >= slot_count){
                index = 0;
            }
            if (index == first_index){
                break;
            }
        }
    }
    
    return(result);
}

internal b32
table_read(Table_Data_Data *table, Table_Lookup lookup, String_Const_u8 *val_out){
    b32 result = false;
    if (lookup.found_match){
        *val_out = table->vals[lookup.index];
        result = true;
    }
    return(result);
}

internal b32
table_read_key(Table_Data_Data *table, Table_Lookup lookup, String_Const_u8 *key_out){
    b32 result = false;
    if (lookup.found_match){
        *key_out = table->keys[lookup.index];
        result = true;
    }
    return(result);
}

internal b32
table_read(Table_Data_Data *table, String_Const_u8 key, String_Const_u8 *val_out){
    Table_Lookup lookup = table_lookup(table, key);
    return(table_read(table, lookup, val_out));
}

internal void
table_insert__inner(Table_Data_Data *table, Table_Lookup lookup, String_Const_u8 key, String_Const_u8 val){
    Assert(lookup.found_empty_slot || lookup.found_erased_slot);
    table->hashes[lookup.index] = lookup.hash;
    table->keys[lookup.index] = key;
    table->vals[lookup.index] = val;
    table->used_count += 1;
    if (lookup.found_empty_slot){
        table->dirty_count += 1;
    }
}

internal b32
table_rehash(Table_Data_Data *dst, Table_Data_Data *src){
    b32 result = false;
    u32 src_slot_count = src->slot_count;
    if ((dst->dirty_count + src->used_count)*8 < dst->slot_count*7){
        u64 *src_hashes = src->hashes;
        for (u32 i = 0; i < src_slot_count; i += 1){
            if (HasFlag(src_hashes[i], bit_64)){
                String_Const_u8 key = src->keys[i];
                Table_Lookup lookup = table_lookup(dst, key);
                table_insert__inner(dst, lookup, key, src->vals[i]);
            }
        }
        result = true;
    }
    return(result);
}

internal b32
table_insert(Table_Data_Data *table, String_Const_u8 key, String_Const_u8 val){
    b32 result = false;
    if (key.str != 0){
        Table_Lookup lookup = table_lookup(table, key);
        if (!lookup.found_match){
            if ((table->dirty_count + 1)*8 >= table->slot_count*7){
                i32 new_slot_count = table->slot_count;
                if (table->used_count*2 >= table->slot_count){
                    new_slot_count = table->slot_count*4;
                }
                Table_Data_Data new_table = make_table_Data_Data(table->allocator, new_slot_count);
                table_rehash(&new_table, table);
                table_free(table);
                *table = new_table;
                lookup = table_lookup(table, key);
                Assert(lookup.found_empty_slot);
            }
            table_insert__inner(table, lookup, key, val);
            result = true;
        }
    }
    return(result);
}

internal b32
table_erase(Table_Data_Data *table, String_Const_u8 key){
    b32 result = false;
    Table_Lookup lookup = table_lookup(table, key);
    if (lookup.found_match){
        table->hashes[lookup.index] = table_erased_slot;
        block_zero_struct(&table->keys[lookup.index]);
        block_zero_struct(&table->vals[lookup.index]);
        table->used_count -= 1;
        result = true;
    }
    return(result);
}

internal void
table_clear(Table_Data_Data *table){
    block_zero_dynamic_array(table->hashes, table->slot_count);
    block_zero_dynamic_array(table->keys, table->slot_count);
    block_zero_dynamic_array(table->vals, table->slot_count);
    table->used_count = 0;
    table->dirty_count = 0;
}

// BOTTOM
