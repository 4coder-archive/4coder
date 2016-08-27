/*

Copy Right FourTech LLC, 2016
All Rights Are Reserved

The OS agnostic file tracking API for applications
that want to interact with potentially many files on
the disk that could be changed by other applications.

Created on: 27.08.2016

*/


// TOP

#ifndef Assert
# define Assert(c) do { if (!(c)) { *((int*)0) = 0xA11E; } } while (0)
#endif

#ifndef ZeroStruct
# define ZeroStruct(s) for (int32_t i = 0; i < sizeof(s); ++i) { ((char*)(&(s)))[i] = 0; }
#endif

typedef struct{
    uint32_t id[4];
} File_Index;

typedef uint32_t rptr32;

#define to_ptr(b,p) ((void*)((char*)b + p))
#define to_rptr32(b,p) ((rptr32)((char*)(p) - (char*)(b)))

typedef struct {
    File_Index hash;
    uint32_t opaque[4];
} File_Track_Entry;

typedef struct {
    int32_t size;
    uint32_t tracked_count;
    uint32_t max;
    rptr32 file_table;
} File_Track_Tables;

typedef struct DLL_Node {
    struct DLL_Node *next;
    struct DLL_Node *prev;
} DLL_Node;



static File_Index
zero_file_index(){
    File_Index a = {0};
    return(a);
}

static int32_t
file_hash_is_zero(File_Index a){
    return ((a.id[0] == 0) &&
            (a.id[1] == 0) &&
            (a.id[2] == 0) &&
            (a.id[3] == 0));
}

static int32_t
file_hash_is_deleted(File_Index a){
    return ((a.id[0] == 0xFFFFFFFF) &&
            (a.id[1] == 0xFFFFFFFF) &&
            (a.id[2] == 0xFFFFFFFF) &&
            (a.id[3] == 0xFFFFFFFF));
}

static int32_t
file_index_eq(File_Index a, File_Index b){
    return ((a.id[0] == b.id[0]) &&
            (a.id[1] == b.id[1]) &&
            (a.id[2] == b.id[2]) &&
            (a.id[3] == b.id[3]));
}

static void
insert_node(DLL_Node *pos, DLL_Node *node){
    node->prev = pos;
    node->next = pos->next;
    pos->next = node;
    node->next->prev = node;
}

static void
remove_node(DLL_Node *node){
    node->next->prev = node->prev;
    node->prev->next = node->next;
}

static void
init_sentinel_node(DLL_Node *node){
    node->next = node;
    node->prev = node;
}

static DLL_Node*
allocate_node(DLL_Node *sentinel){
    DLL_Node *result = 0;
    if (sentinel->next != sentinel){
        result = sentinel->next;
        remove_node(result);
    }
    return(result);
}

#define FILE_ENTRY_COST (sizeof(File_Track_Entry))


static int32_t
tracking_system_has_space(File_Track_Tables *tables, int32_t new_count){
    uint32_t count = tables->tracked_count;
    uint32_t max = tables->max;
    int32_t result = ((count + new_count)*8 < max*7);
    return(result);
}

static int32_t
entry_is_available(File_Track_Entry *entry){
    int32_t result = 0;
    if (entry){
        result =
            file_hash_is_zero(entry->hash) ||
            file_hash_is_deleted(entry->hash);
    }
    return (result);
}

static File_Track_Entry*
tracking_system_lookup_entry(File_Track_Tables *tables, File_Index key){
    uint32_t hash = key.id[0];
    uint32_t max = tables->max;
    uint32_t index = (hash) % max;
    uint32_t start = index;
    
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

static File_Track_Entry*
get_file_entry(File_Track_Tables *tables, File_Index index){
    File_Track_Entry *entry = 0;
    
    File_Track_Entry *result = tracking_system_lookup_entry(tables, index);
    if (result && file_index_eq(index, result->hash)){
        entry = result;
    }
    
    return(entry);
}

static void
internal_free_slot(File_Track_Tables *tables, File_Track_Entry *entry){
    Assert(!entry_is_available(entry));
    
    ZeroStruct(*entry);
    entry->hash.id[0] = 0xFFFFFFFF;
    entry->hash.id[1] = 0xFFFFFFFF;
    entry->hash.id[2] = 0xFFFFFFFF;
    entry->hash.id[3] = 0xFFFFFFFF;
    
    --tables->tracked_count;
}

static int32_t
enough_memory_to_init_table(int32_t table_memory_size){
    int32_t result = (sizeof(File_Track_Tables) + FILE_ENTRY_COST*8 <= table_memory_size);
    return(result);
}

static void
init_table_memory(File_Track_Tables *tables, int32_t table_memory_size){
    tables->size = table_memory_size;
    tables->tracked_count = 0;
    
    int32_t max_number_of_entries =
        (table_memory_size - sizeof(*tables)) / FILE_ENTRY_COST;
    
    tables->file_table = sizeof(*tables);
    tables->max = max_number_of_entries;
}

static File_Track_Result
move_table_memory(File_Track_Tables *original_tables,
                  void *mem, int32_t size){
    File_Track_Result result = FileTrack_Good;
    
    if (original_tables->size < size){
        File_Track_Tables *tables = (File_Track_Tables*)mem;
        
        // NOTE(allen): Initialize main data tables
        {
            tables->size = size;
            
            int32_t likely_entry_size = FILE_ENTRY_COST;
            int32_t max_number_of_entries = (size - sizeof(*tables)) / likely_entry_size;
            
            tables->file_table = sizeof(*tables);
            tables->max = max_number_of_entries;
        }
        
        if (tables->max > original_tables->max){
            uint32_t original_max = original_tables->max;
            
            // NOTE(allen): Rehash the tracking table
            {
                File_Track_Entry *entries = (File_Track_Entry*)
                    to_ptr(original_tables, original_tables->file_table);
                
                for (uint32_t index = 0;
                     index < original_max;
                     ++index){
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