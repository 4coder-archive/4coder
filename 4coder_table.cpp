/*
 * 4coder tables
 */

// TOP

internal u64
table_hash(Data key){
    return(table_hash_u8((u8*)key.data, key.size) | bit_63);
}

global_const u64 table_empty_slot = 0;
global_const u64 table_erased_slot = 1;

////////////////////////////////

////////////////////////////////

internal Table_Data_u64
make_table_Data_u64(Base_Allocator *allocator, u32 slot_count){
    Table_Data_u64 table = {};
    table.allocator = allocator;
    slot_count = clamp_bot(8, slot_count);
    Data mem = base_allocate(allocator, slot_count*(sizeof(*table.hashes) + sizeof(*table.keys) + sizeof(*table.vals)));
    block_zero(mem.data, mem.size);
    table.memory = mem.data;
    table.hashes = (u64*)table.memory;
    table.keys = (Data*)(table.hashes + slot_count);
    table.vals = (u64*)(table.keys + slot_count);
    table.slot_count = slot_count;
    table.used_count = 0;
    table.dirty_count = 0;
    return(table);
}

internal Table_Lookup
table_lookup(Table_Data_u64 *table, Data key){
    u64 *hashes = table->hashes;
    u32 slot_count = table->slot_count;
    
    u64 hash = table_hash(key);
    u32 first_index = hash % slot_count;
    u32 index = first_index;
    Table_Lookup result = {};
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
            result.index = index;
            result.found_empty_slot = true;
            result.found_erased_slot = false;
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
    
    return(result);
}

internal b32
table_read(Table_Data_u64 *table, Data key, u64 *val_out){
    b32 result = false;
    Table_Lookup lookup = table_lookup(table, key);
    if (lookup.found_match){
        *val_out = table->vals[lookup.index];
        result = true;
    }
    return(result);
}

internal void
table_insert__inner(Table_Data_u64 *table, Table_Lookup lookup, Data key, u64 val){
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
            if (HasFlag(src_hashes[i], bit_63)){
                Data key = src->keys[i];
                Table_Lookup lookup = table_lookup(dst, key);
                table_insert__inner(dst, lookup, key, src->vals[i]);
            }
        }
        result = true;
    }
    return(result);
}

internal b32
table_insert(Table_Data_u64 *table, Data key, u64 val){
    b32 result = false;
    Table_Lookup lookup = table_lookup(table, key);
    if (!lookup.found_match){
        if ((table->dirty_count + 1)*8 >= table->slot_count*7){
            i32 new_slot_count = table->slot_count;
            if (table->used_count*2 >= table->slot_count){
                new_slot_count = table->slot_count*4;
            }
            Table_Data_u64 new_table = make_table_Data_u64(table->allocator, new_slot_count);
            table_rehash(&new_table, table);
            base_free(table->allocator, table->memory);
            *table = new_table;
            lookup = table_lookup(table, key);
            Assert(lookup.found_empty_slot);
        }
        table_insert__inner(table, lookup, key, val);
        result = true;
    }
    return(result);
}

internal b32
table_erase(Table_Data_u64 *table, Data key){
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

////////////////////////////////

////////////////////////////////

// BOTTOM
