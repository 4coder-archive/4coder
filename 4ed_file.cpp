/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 20.02.2016
 *
 * File editing view for 4coder
 *
 */

// TOP

#include "buffer/4coder_shared.cpp"

#if BUFFER_EXPERIMENT_SCALPEL == 0
#include "buffer/4coder_golden_array.cpp"
#define Buffer_Type Buffer
#elif BUFFER_EXPERIMENT_SCALPEL == 1
#include "buffer/4coder_gap_buffer.cpp"
#define Buffer_Type Gap_Buffer
#elif BUFFER_EXPERIMENT_SCALPEL == 2
#include "buffer/4coder_multi_gap_buffer.cpp"
#define Buffer_Type Multi_Gap_Buffer
#else
#include "buffer/4coder_rope_buffer.cpp"
#define Buffer_Type Rope_Buffer
#endif

#include "buffer/4coder_buffer_abstract.cpp"

enum Edit_Type{
    ED_NORMAL,
    ED_REVERSE_NORMAL,
    ED_UNDO,
    ED_REDO,
};

struct Edit_Step{
    Edit_Type type;
    union{
        struct{
            b32 can_merge;
            Buffer_Edit edit;
            i32 pre_pos;
            i32 post_pos;
            i32 next_block, prev_block;
        };
        struct{
            i32 first_child;
            i32 inverse_first_child;
            i32 inverse_child_count;
            i32 special_type;
        };
    };
    i32 child_count;
};

struct Edit_Stack{
    u8 *strings;
    i32 size, max;
    
    Edit_Step *edits;
    i32 edit_count, edit_max;
};

struct Small_Edit_Stack{
    u8 *strings;
    i32 size, max;
    
    Buffer_Edit *edits;
    i32 edit_count, edit_max;
};

struct Undo_Data{
    Edit_Stack undo;
    Edit_Stack redo;
    Edit_Stack history;
    Small_Edit_Stack children;
    
    i32 history_block_count, history_head_block;
    i32 edit_history_cursor;
    b32 current_block_normal;
};

struct Text_Effect{
    i32 start, end;
    u32 color;
    i32 tick_down, tick_max;
};

// NOTE(allen): The Editing_File struct is now divided into two
// parts.  Variables in the Settings part can be set even when the
// file is still streaming in, and all operations except for the
// initial allocation of the file.
struct Editing_File_Settings{
    i32 base_map_id;
    i32 dos_write_mode;
    b32 unwrapped_lines;
    b8 tokens_exist;
    b8 is_initialized;
    b8 unimportant;
    b8 read_only;
};

// NOTE(allen): This part of the Editing_File is cleared whenever
// the contents of the file is set.
struct Editing_File_State{
    b32 is_dummy;
    b32 is_loading;

    i16 font_id;
    Buffer_Type buffer;
    
    i32 cursor_pos;
    
    Undo_Data undo;
    
    Cpp_Token_Stack token_stack;
    Cpp_Token_Stack swap_stack;
    u32 lex_job;
    b32 tokens_complete;
    b32 still_lexing;
    
    Text_Effect paste_effect;
    
    u64 last_4ed_write_time;
    u64 last_4ed_edit_time;
    u64 last_sys_write_time;
};

struct Editing_File_Preload{
    i32 start_line;
};

struct Editing_File_Name{
    char live_name_[256];
    String live_name;
    
    char source_path_[256];
    char extension_[16];
    String source_path;
    String extension;
};

struct File_Node{
    File_Node *next, *prev;
};

struct Editing_File{
    File_Node node;
    Editing_File_Settings settings;
    union{
        Editing_File_State state;
        Editing_File_Preload preload;
    };
    Editing_File_Name name;
};

struct File_Table_Entry{
    String name;
    u32 hash;
    i32 id;
};

struct File_Table{
    File_Table_Entry *table;
    i32 count, max;
};

internal u32
get_file_hash(String name){
    u32 x = 5381;
    int i = 0;
    char c;
    while (i < name.size){
        c = name.str[i++];
        x = ((x << 5) + x) + c;
    }
    return x;
}

internal b32
table_add(File_Table *table, String name, i32 id){
    Assert(table->count * 3 < table->max * 2);
    
    File_Table_Entry entry, e;
    i32 i;
    
    entry.name = name;
    entry.id = id;
    entry.hash = get_file_hash(name);
    i = entry.hash % table->max;
    while ((e = table->table[i]).name.str){
        if (e.hash == entry.hash && match(e.name, entry.name)){
            return 1;
        }
        i = (i + 1) % table->max;
    }
    table->table[i] = entry;
    ++table->count;
    
    return 0;
}

internal b32
table_find_pos(File_Table *table, String name, i32 *index){
    File_Table_Entry e;
    i32 i;
    u32 hash;

    hash = get_file_hash(name);
    i = hash % table->max;
    while ((e = table->table[i]).name.size){
        if (e.name.str && e.hash == hash && match(e.name, name)){
            *index = i;
            return 1;
        }
        i = (i + 1) % table->max;
    }
    
    return 0;
}

inline b32
table_find(File_Table *table, String name, i32 *id){
    i32 pos;
    b32 r = table_find_pos(table, name, &pos);
    if (r) *id = table->table[pos].id;
    return r;
}

inline b32
table_remove(File_Table *table, String name){
    i32 pos;
    b32 r = table_find_pos(table, name, &pos);
    if (r){
        table->table[pos].name.str = 0;
        --table->count;
    }
    return r;
}

struct Working_Set{
	Editing_File *files;
	i32 file_count, file_max;
    File_Node free_sentinel;
    File_Node used_sentinel;
    
    File_Table table;
    
	String clipboards[64];
	i32 clipboard_size, clipboard_max_size;
	i32 clipboard_current, clipboard_rolling;
};

// Hot Directory

struct Hot_Directory{
	String string;
	File_List file_list;
    char slash;
};

internal void
hot_directory_clean_end(Hot_Directory *hot_directory){
    String *str = &hot_directory->string;
    if (str->size != 0 && str->str[str->size-1] != hot_directory->slash){
        str->size = reverse_seek_slash(*str) + 1;
        str->str[str->size] = 0;
    }
}

internal i32
hot_directory_quick_partition(File_Info *infos, i32 start, i32 pivot){
    File_Info *p = infos + pivot;
    File_Info *a = infos + start;
    for (i32 i = start; i < pivot; ++i, ++a){
        i32 comp = 0;
        comp = p->folder - a->folder;
        if (comp == 0) comp = compare(a->filename, p->filename);
        if (comp < 0){
            Swap(*a, infos[start]);
            ++start;
        }
    }
    Swap(*p, infos[start]);
    return start;
}

internal void
hot_directory_quick_sort(File_Info *infos, i32 start, i32 pivot){
    i32 mid = hot_directory_quick_partition(infos, start, pivot);
    if (start < mid-1) hot_directory_quick_sort(infos, start, mid-1);
    if (mid+1 < pivot) hot_directory_quick_sort(infos, mid+1, pivot);
}

inline void
hot_directory_fixup(Hot_Directory *hot_directory, Working_Set *working_set){
    File_List *files = &hot_directory->file_list;
    if (files->count >= 2)
        hot_directory_quick_sort(files->infos, 0, files->count - 1);
}

inline void
hot_directory_set(System_Functions *system, Hot_Directory *hot_directory,
                  String str, Working_Set *working_set){
    b32 success = copy_checked(&hot_directory->string, str);
    terminate_with_null(&hot_directory->string);
    if (success){
        if (str.size > 0){
            system->set_file_list(&hot_directory->file_list, str);
        }
        else{
            system->set_file_list(&hot_directory->file_list, make_string((char*)1, 0));
        }
    }
    hot_directory_fixup(hot_directory, working_set);
}

inline void
hot_directory_reload(System_Functions *system, Hot_Directory *hot_directory, Working_Set *working_set){
    system->set_file_list(&hot_directory->file_list, hot_directory->string);
    hot_directory_fixup(hot_directory, working_set);
}

internal void
hot_directory_init(Hot_Directory *hot_directory, String base, String dir, char slash){
	hot_directory->string = base;
    hot_directory->string.str[255] = 0;
    hot_directory->string.size = 0;
    copy(&hot_directory->string, dir);
	append(&hot_directory->string, slash);
    hot_directory->slash = slash;
}

struct Hot_Directory_Match{
	String filename;
	b32 is_folder;
};

internal b32
filename_match(String query, Absolutes *absolutes, String filename, b32 case_sensitive){
    b32 result;
    result = (query.size == 0);
    if (!result) result = wildcard_match(absolutes, filename, case_sensitive);
    return result;
}

internal Hot_Directory_Match
hot_directory_first_match(Hot_Directory *hot_directory,
                          String str,
						  b32 include_files,
                          b32 exact_match,
                          b32 case_sensitive){
    Hot_Directory_Match result = {};
    
    Absolutes absolutes;
    if (!exact_match)
        get_absolutes(str, &absolutes, 1, 1);
    
    File_List *files = &hot_directory->file_list;
    File_Info *info, *end;
    end = files->infos + files->count;
    for (info = files->infos; info != end; ++info){
        String filename = info->filename;
        b32 is_match = 0;
        if (exact_match){
            if (case_sensitive){
                if (match(filename, str)) is_match = 1;
            }
            else{
                if (match_unsensitive(filename, str)) is_match = 1;
            }
        }
        else{
            if (filename_match(str, &absolutes, filename, case_sensitive)) is_match = 1;
        }
        
        if (is_match){
            result.is_folder = info->folder;
            result.filename = filename;
            break;
        }
    }
    
    return result;
}

enum File_Sync_State{
    SYNC_GOOD,
    SYNC_BEHIND_OS,
    SYNC_UNSAVED
};

inline File_Sync_State
buffer_get_sync(Editing_File *file){
    File_Sync_State result = SYNC_GOOD;
    if (file->state.last_4ed_write_time != file->state.last_sys_write_time)
        result = SYNC_BEHIND_OS;
    else if (file->state.last_4ed_edit_time > file->state.last_sys_write_time)
        result = SYNC_UNSAVED;
    return result;
}

inline b32
buffer_needs_save(Editing_File *file){
    b32 result = 0;
    if (file->settings.unimportant == 0)
        if (buffer_get_sync(file) == SYNC_UNSAVED)
            result = 1;
    return(result);
}

inline b32
file_is_ready(Editing_File *file){
    b32 result = 0;
    if (file && file->state.is_loading == 0){
        result = 1;
    }
    return(result);
}

inline Editing_File*
working_set_contains(Working_Set *working, String filename){
    Editing_File *result = 0;
    i32 id;
    if (table_find(&working->table, filename, &id)){
        if (id >= 0 && id < working->file_max){
            result = working->files + id;
        }
    }
    return (result);
}

// TODO(allen): Find a way to choose an ordering for these so it picks better first options.
internal Editing_File*
working_set_lookup_file(Working_Set *working_set, String string){
	Editing_File *file = working_set_contains(working_set, string);
	
	if (!file){
        File_Node *node, *used_nodes;
        used_nodes = &working_set->used_sentinel;
        for (dll_items(node, used_nodes)){
            file = (Editing_File*)node;
            if (string.size == 0 || has_substr(file->name.live_name, string)){
                break;
            }
        }
        if (node == used_nodes) file = 0;
	}
    
	return (file);
}

struct Get_File_Result{
    Editing_File *file;
    i32 index;
};

internal Get_File_Result
working_set_get_available_file(Working_Set *working_set){
    Get_File_Result result = {};
    File_Node *node;
    
    if (working_set->file_count < working_set->file_max){
        node = working_set->free_sentinel.next;
        Assert(node != &working_set->free_sentinel);
        
        result.file = (Editing_File*)node;
        result.index = (i32)(result.file - working_set->files);
        
        ++working_set->file_count;
        
        dll_remove(node);
        *result.file = {};
        dll_insert(&working_set->used_sentinel, node);
    }

    return result;
}

inline void
working_set_free_file(Working_Set  *working_set, Editing_File *file){
    file->state.is_dummy = 1;
    dll_remove(&file->node);
    dll_insert(&working_set->free_sentinel, &file->node);
    --working_set->file_count;
}

inline Get_File_Result
working_set_get_file(Working_Set *working_set, i32 id, b32 require_active){
    Get_File_Result result = {};
    if (id > 0 && id <= working_set->file_max){
        result.file = working_set->files + id;
        result.index = id;
        if (result.file->state.is_dummy && require_active){
            result.file = 0;
            result.index = 0;
        }
    }
    return(result);
}

inline void
file_set_to_loading(Editing_File *file){
    file->state = {};
    file->settings = {};
    file->state.is_loading = 1;
}

// BOTTOM

