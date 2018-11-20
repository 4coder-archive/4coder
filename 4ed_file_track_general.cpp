/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 20.08.2016
 *
 * File tracking shared.
 *
 */

// TOP

struct File_Index{
    u32 id[4];
};

typedef u32 rptr32;

#define to_ptr(b,p) ((void*)((char*)b + p))
#define to_rptr32(b,p) ((rptr32)((char*)(p) - (char*)(b)))

struct File_Track_Entry{
    File_Index hash;
    u32 opaque[4];
};
global_const File_Track_Entry null_file_track_entry = {};

struct File_Track_Tables{
    i32 size;
    u32 tracked_count;
    u32 max;
    rptr32 file_table;
};

struct DLL_Node {
    DLL_Node *next;
    DLL_Node *prev;
};

internal File_Index
zero_file_index(){
    File_Index a = {};
    return(a);
}

internal i32
file_hash_is_zero(File_Index a){
    return ((a.id[0] == 0) &&
            (a.id[1] == 0) &&
            (a.id[2] == 0) &&
            (a.id[3] == 0));
}

internal i32
file_hash_is_deleted(File_Index a){
    return ((a.id[0] == 0xFFFFFFFF) &&
            (a.id[1] == 0xFFFFFFFF) &&
            (a.id[2] == 0xFFFFFFFF) &&
            (a.id[3] == 0xFFFFFFFF));
}

internal i32
file_index_eq(File_Index a, File_Index b){
    return ((a.id[0] == b.id[0]) &&
            (a.id[1] == b.id[1]) &&
            (a.id[2] == b.id[2]) &&
            (a.id[3] == b.id[3]));
}

internal void
insert_node(DLL_Node *pos, DLL_Node *node){
    node->prev = pos;
    node->next = pos->next;
    pos->next = node;
    node->next->prev = node;
}

internal void
remove_node(DLL_Node *node){
    node->next->prev = node->prev;
    node->prev->next = node->next;
}

internal void
init_sentinel_node(DLL_Node *node){
    node->next = node;
    node->prev = node;
}

internal DLL_Node*
allocate_node(DLL_Node *sentinel){
    DLL_Node *result = 0;
    if (sentinel->next != sentinel){
        result = sentinel->next;
        remove_node(result);
    }
    return(result);
}

#define FILE_ENTRY_COST (sizeof(File_Track_Entry))


internal i32
tracking_system_has_space(File_Track_Tables *tables, i32 new_count){
    u32 count = tables->tracked_count;
    u32 max = tables->max;
    i32 result = ((count + new_count)*8 < max*7);
    return(result);
}

internal i32
entry_is_available(File_Track_Entry *entry){
    i32 result = 0;
    if (entry){
        result =
            file_hash_is_zero(entry->hash) ||
            file_hash_is_deleted(entry->hash);
    }
    return (result);
}

internal File_Track_Entry*
tracking_system_lookup_entry(File_Track_Tables *tables, File_Index key){
    u32 hash = key.id[0];
    u32 max = tables->max;
    u32 index = (hash) % max;
    u32 start = index;
    
    File_Track_Entry *entries = (File_Track_Entry*)to_ptr(tables, tables->file_table);
    
    File_Track_Entry* result = 0;
    for (;;){
        File_Track_Entry *entry = entries + index;
        
        if (file_index_eq(entry->hash, key)){
            result = entry;
            break;
        }
        else if (file_hash_is_zero(entry->hash)){
            if (result == 0){
                result = entry;
            }
            break;
        }
        else if (file_hash_is_deleted(entry->hash)){
            if (result == 0){
                result = entry;
            }
        }
        
        ++index;
        if (index == max) index = 0;
        if (index == start) break;
    }
    
    return(result);
}

internal File_Track_Entry*
get_file_entry(File_Track_Tables *tables, File_Index index){
    File_Track_Entry *entry = 0;
    
    File_Track_Entry *result = tracking_system_lookup_entry(tables, index);
    if (result && file_index_eq(index, result->hash)){
        entry = result;
    }
    
    return(entry);
}

internal void
internal_free_slot(File_Track_Tables *tables, File_Track_Entry *entry){
    Assert(!entry_is_available(entry));
    
    *entry = null_file_track_entry;
    entry->hash.id[0] = 0xFFFFFFFF;
    entry->hash.id[1] = 0xFFFFFFFF;
    entry->hash.id[2] = 0xFFFFFFFF;
    entry->hash.id[3] = 0xFFFFFFFF;
    
    --tables->tracked_count;
}

internal i32
enough_memory_to_init_table(i32 table_memory_size){
    i32 result = (sizeof(File_Track_Tables) + FILE_ENTRY_COST*8 <= table_memory_size);
    return(result);
}

internal void
init_table_memory(File_Track_Tables *tables, i32 table_memory_size){
    tables->size = table_memory_size;
    tables->tracked_count = 0;
    
    i32 max_number_of_entries = (table_memory_size - sizeof(*tables)) / FILE_ENTRY_COST;
    
    tables->file_table = sizeof(*tables);
    tables->max = max_number_of_entries;
}

internal File_Track_Result
move_table_memory(File_Track_Tables *original_tables,
                  void *mem, i32 size){
    File_Track_Result result = FileTrack_Good;
    
    if (original_tables->size < size){
        File_Track_Tables *tables = (File_Track_Tables*)mem;
        
        // NOTE(allen): Initialize main data tables
        {
            tables->size = size;
            
            i32 likely_entry_size = FILE_ENTRY_COST;
            i32 max_number_of_entries = (size - sizeof(*tables)) / likely_entry_size;
            
            tables->file_table = sizeof(*tables);
            tables->max = max_number_of_entries;
        }
        
        if (tables->max > original_tables->max){
            u32 original_max = original_tables->max;
            
            // NOTE(allen): Rehash the tracking table
            {
                File_Track_Entry *entries = (File_Track_Entry*)
                    to_ptr(original_tables, original_tables->file_table);
                
                for (u32 index = 0; index < original_max; ++index){
                    File_Track_Entry *entry = entries + index;
                    if (!entry_is_available(entry)){
                        File_Index hash = entry->hash;
                        File_Track_Entry *lookup =
                            tracking_system_lookup_entry(tables, hash);
                        
                        Assert(entry_is_available(lookup));
                        *lookup = *entry;
                    }
                }
                
                tables->tracked_count = original_tables->tracked_count;
            }
        }
        else{
            result = FileTrack_MemoryTooSmall;
        }
    }
    else{
        result = FileTrack_MemoryTooSmall;
    }
    
    return(result);
}

// BOTTOM

