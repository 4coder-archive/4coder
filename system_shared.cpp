/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 09.02.2016
 *
 * Shared system functions
 *
 */

// TOP

internal b32
usable_ascii(char c){
    b32 result = 1;
    if ((c < ' ' || c > '~') && c != '\n' && c != '\r' && c != '\t'){
        result = 0;
    }
    return(result);
}

internal void
sysshared_filter_real_files(char **files, i32 *file_count){
    i32 i, j;
    i32 end;
    
    end = *file_count;
    for (i = 0, j = 0; i < end; ++i){
        if (system_file_can_be_made(files[i])){
            files[j] = files[i];
            ++j;
        }
    }
    *file_count = j;
}

void
ex__file_insert(File_Slot *pos, File_Slot *file){
    file->next = pos->next;
    file->next->prev = file;
    file->prev = pos;
    pos->next = file;
}

void
ex__insert_range(File_Slot *start, File_Slot *end, File_Slot *pos){
    end->next->prev = start->prev;
    start->prev->next = end->next;
    
    end->next = pos->next;
    start->prev = pos;
    pos->next->prev = end;
    pos->next = start;
}

internal void
ex__check_file(File_Slot *pos){
    File_Slot *file = pos;
    
    Assert(pos == pos->next->prev);
    
    for (pos = pos->next;
         file != pos;
         pos = pos->next){
        Assert(pos == pos->next->prev);
    }
}

internal void
ex__check(File_Exchange *file_exchange){
    ex__check_file(&file_exchange->available);
    ex__check_file(&file_exchange->active);
    ex__check_file(&file_exchange->free_list);
}

internal void
sysshared_init_file_exchange(
    Exchange *exchange, File_Slot *slots, i32 max,
    char **filename_space_out){
    char *filename_space;
    i32 i;
    
    exchange->file.max = max;
    exchange->file.available = {};
    exchange->file.available.next = &exchange->file.available;
    exchange->file.available.prev = &exchange->file.available;
    
    exchange->file.active = {};
    exchange->file.active.next = &exchange->file.active;
    exchange->file.active.prev = &exchange->file.active;
    
    exchange->file.free_list = {};
    exchange->file.free_list.next = &exchange->file.free_list;
    exchange->file.free_list.prev = &exchange->file.free_list;

    exchange->file.files = slots;
    memset(slots, 0, sizeof(File_Slot)*max);

    filename_space = (char*)
        system_get_memory(FileNameMax*exchange->file.max);

    File_Slot *slot = slots;
    for (i = 0; i < exchange->file.max; ++i, ++slot){
        ex__file_insert(&exchange->file.available, slot);
        slot->filename = filename_space;
        filename_space += FileNameMax;
    }

    if (filename_space_out) *filename_space_out = filename_space;
}

internal void
fnt__remove(Font_Load_Parameters *params){
    params->next->prev = params->prev;
    params->prev->next = params->next;
}

internal void
fnt__insert(Font_Load_Parameters *pos, Font_Load_Parameters *params){
    params->next = pos->next;
    pos->next->prev = params;
    pos->next = params;
    params->prev = pos;
}

internal void
sysshared_init_font_params(
    Font_Load_System *fnt, Font_Load_Parameters *params, i32 max){
    Font_Load_Parameters *param;
    i32 i;
    
    fnt->params = params;
    fnt->max = max;
    
    fnt->free_param.next = &fnt->free_param;
    fnt->free_param.prev = &fnt->free_param;
    
    fnt->used_param.next = &fnt->free_param;
    fnt->used_param.prev = &fnt->free_param;

    param = params;
    for (i = 0; i < max; ++i, ++param){
        fnt__insert(&fnt->free_param, param);
    }
}

// BOTTOM

