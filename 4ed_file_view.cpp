/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 19.08.2015
 *
 * File editing view for 4coder
 *
 */

// TOP

struct File_View_Mode{
	b8 rewrite;
};

struct Incremental_Search{
    String str;
    b32 reverse;
    i32 pos;
};

enum Action_Type{
    DACT_OPEN,
    DACT_SAVE_AS,
    DACT_SAVE,
    DACT_NEW,
    DACT_SWITCH,
    DACT_TRY_KILL,
    DACT_KILL,
    DACT_CLOSE_MINOR,
    DACT_CLOSE_MAJOR,
    DACT_THEME_OPTIONS,
    DACT_KEYBOARD_OPTIONS
};

struct Delayed_Action{
    Action_Type type;
    String string;
    Panel *panel;
};

struct Delay{
    Delayed_Action acts[8];
    i32 count, max;
};

inline void
delayed_action(Delay *delay, Action_Type type,
               String string, Panel *panel){
    Assert(delay->count < delay->max);
    Delayed_Action action;
    action.type = type;
    action.string = string;
    action.panel = panel;
    delay->acts[delay->count++] = action;
}

enum File_View_Widget_Type{
    FWIDG_NONE,
    FWIDG_SEARCH,
    FWIDG_GOTO_LINE,
    FWIDG_TIMELINES,
    // never below this
    FWIDG_TYPE_COUNT
};

struct File_View_Widget{
    UI_State state;
    File_View_Widget_Type type;
    i32 height;
    struct{
        bool32 undo_line;
        bool32 history_line;
    } timeline;
};

enum Link_Type{
    link_result,
    link_related,
    link_error,
    link_warning,
    // never below this
    link_type_count
};

struct Hyper_Link{
    char *file_name;
    i32 line_number;
    i32 start, end;
    Link_Type link_type;
};

struct File_View{
    View view_base;

    Font_Set *font_set;
    Editing_File *file;
    Style *style;
    Editing_Layout *layout;
    
    i32 font_advance;
    i32 font_height;

    Full_Cursor cursor;
    i32 mark;
    f32 scroll_y, target_y, vel_y;
    f32 scroll_x, target_x, vel_x;
    f32 preferred_x;
    i32 scroll_i;
    
    union{
        Incremental_Search isearch;
        struct{
            String str;
        } gotoline;
    };
    
    Full_Cursor temp_highlight;
    i32 temp_highlight_end_pos;
    b32 show_temp_highlight;
    
    File_View_Mode mode, next_mode;
    File_View_Widget widget;
    f32 rewind_amount, rewind_speed;
    i32 rewind_max, scrub_max;
    b32 unwrapped_lines;
    b32 show_whitespace;
    b32 locked;
    
    i32 line_count, line_max;
    f32 *line_wrap_y;
    
    Hyper_Link *links;
    i32 link_count, link_max;
};

inline File_View*
view_to_file_view(View *view){
    File_View* result = 0;
    if (view && view->type == VIEW_TYPE_FILE){
        result = (File_View*)view;
    }
    return result;
}

inline i32
get_file_id(Working_Set *working_set, Editing_File *file){
    i32 result = (i32)(file - working_set->files);
    return(result);
}

inline Editing_File*
get_file(Working_Set *working_set, i32 file_id){
    Editing_File *result = working_set->files + file_id;
    if (!buffer_good(&result->state.buffer) || result->state.is_dummy){
        result = 0;
    }
    return(result);
}

internal Range
get_range(i32 a, i32 b){
	Range result = {};
	if (a < b){
		result.start = a;
		result.end = b;
	}
	else{
		result.start = b;
		result.end = a;
	}
	return result;
}

inline bool32
starts_new_line(u8 character){
	return (character == '\n');
}

inline void
file_init_strings(Editing_File *file){
    file->name.source_path = make_fixed_width_string(file->name.source_path_);
    file->name.live_name = make_fixed_width_string(file->name.live_name_);
    file->name.extension = make_fixed_width_string(file->name.extension_);
}

inline b32
file_is_ready(Editing_File *file){
    b32 result = 0;
    if (file && file->state.is_loading == 0){
        result = 1;
    }
    return(result);
}

inline void
file_set_name(Working_Set *working_set, Editing_File *file, char *filename){
    String f, ext;
    Editing_File *file_ptr;
    i32 i, count, file_x, original_len;
    b32 hit_conflict;
    
    Assert(file->name.live_name.str != 0);

    f = make_string_slowly(filename);
    copy_checked(&file->name.source_path, f);
    
    file->name.live_name.size = 0;
    get_front_of_directory(&file->name.live_name, f);
    
    if (file->name.source_path.size == file->name.live_name.size){
        file->name.extension.size = 0;
    }
    else{
        ext = file_extension(f);
        copy(&file->name.extension, ext);
    }
    
    original_len = file->name.live_name.size;
    count = working_set->file_index_count;
    hit_conflict = 1;
    file_x = 0;
    while (hit_conflict){
        hit_conflict = 0;
        file_ptr = working_set->files;
        for (i = 0; i < count; ++i, ++file_ptr){
            if (file_ptr != file && !file_ptr->state.is_dummy && file_is_ready(file_ptr)){
                if (match(file->name.live_name, file_ptr->name.live_name)){
                    ++file_x;
                    hit_conflict = 1;
                    break;
                }
            }
        }
        
        if (hit_conflict){
            file->name.live_name.size = original_len;
            append(&file->name.live_name, " <");
            append_int_to_str(file_x, &file->name.live_name);
            append(&file->name.live_name, ">");
        }
    }
}

inline void
file_synchronize_times(System_Functions *system, Editing_File *file, char *filename){
    u64 stamp = system->file_time_stamp(filename);
    if (stamp > 0){
        file->state.last_4ed_write_time = stamp;
        file->state.last_4ed_edit_time = stamp;
        file->state.last_sys_write_time = stamp;
    }
}

internal i32
file_save(System_Functions *system, Exchange *exchange, Mem_Options *mem,
          Editing_File *file, char *filename){
	i32 result = 0;
    
#if BUFFER_EXPERIMENT_SCALPEL <= 3
    i32 max, size;
    byte *data;
    b32 dos_write_mode = file->settings.dos_write_mode;

    if (dos_write_mode)
        max = buffer_size(&file->state.buffer) + file->state.buffer.line_count + 1;
    else
        max = buffer_size(&file->state.buffer);
    
    data = (byte*)general_memory_allocate(&mem->general, max, 0);
    Assert(data);
    
    if (dos_write_mode)
        size = buffer_convert_out(&file->state.buffer, (char*)data, max);
    else
        buffer_stringify(&file->state.buffer, 0, size = max, (char*)data);
    
    i32 filename_len = str_size(filename);
    result = exchange_save_file(exchange, filename, filename_len,
                                data, size, max);
    
    if (result == 0){
        general_memory_free(&mem->general, data);
    }
    
    file_synchronize_times(system, file, filename);
#endif
    
    return result;
}

inline b32
file_save_and_set_names(System_Functions *system, Exchange *exchange,
                        Mem_Options *mem, Working_Set *working_set, Editing_File *file,
                        char *filename){
	b32 result = 0;
    result = file_save(system, exchange, mem, file, filename);
	if (result){
        file_set_name(working_set, file, filename);
	}
	return result;
}

enum File_Bubble_Type{
    BUBBLE_BUFFER = 1,
    BUBBLE_STARTS,
    BUBBLE_WIDTHS,
    BUBBLE_WRAPS,
    BUBBLE_TOKENS,
    BUBBLE_UNDO_STRING,
    BUBBLE_UNDO,
    BUBBLE_UNDO_CHILDREN,
    //
    FILE_BUBBLE_TYPE_END,
};

#define GROW_FAILED 0
#define GROW_NOT_NEEDED 1
#define GROW_SUCCESS 2

internal i32
file_grow_starts_as_needed(General_Memory *general, Buffer_Type *buffer, i32 additional_lines){
    bool32 result = GROW_NOT_NEEDED;
#if BUFFER_EXPERIMENT_SCALPEL <= 3
    i32 max = buffer->line_max;
    i32 count = buffer->line_count;
    i32 target_lines = count + additional_lines;
    if (target_lines > max || max == 0){
        max = LargeRoundUp(target_lines + max, Kbytes(1));
        i32 *new_lines = (i32*)
            general_memory_reallocate(general, buffer->line_starts,
                                      sizeof(i32)*count, sizeof(i32)*max, BUBBLE_STARTS);
        if (new_lines){
            buffer->line_starts = new_lines;
            buffer->line_max = max;
            result = GROW_SUCCESS;
        }
        else{
            result = GROW_FAILED;
        }
    }
#endif
    return result;
}

internal void
file_measure_starts_widths(System_Functions *system, General_Memory *general,
                           Buffer_Type *buffer, float *advance_data){
    ProfileMomentFunction();
    if (!buffer->line_starts){
        i32 max = buffer->line_max = Kbytes(1);
        buffer->line_starts = (i32*)general_memory_allocate(general, max*sizeof(i32), BUBBLE_STARTS);
        TentativeAssert(buffer->line_starts);
        // TODO(allen): when unable to allocate?
    }
    if (!buffer->line_widths){
        i32 max = buffer->widths_max = Kbytes(1);
        buffer->line_widths = (f32*)general_memory_allocate(general, max*sizeof(f32), BUBBLE_STARTS);
        TentativeAssert(buffer->line_starts);
        // TODO(allen): when unable to allocate?
    }
    
    Buffer_Measure_Starts state = {};
    while (buffer_measure_starts_widths(&state, buffer, advance_data)){
        i32 count = state.count;
        i32 max = buffer->line_max;
        max = ((max + 1) << 1);

        {
            i32 *new_lines = (i32*)
                general_memory_reallocate(general, buffer->line_starts,
                                          sizeof(i32)*count, sizeof(i32)*max, BUBBLE_STARTS);
        
            // TODO(allen): when unable to grow?
            TentativeAssert(new_lines);
            buffer->line_starts = new_lines;
            buffer->line_max = max;
        }

        {
            f32 *new_lines = (f32*)
                general_memory_reallocate(general, buffer->line_widths,
                                          sizeof(f32)*count, sizeof(f32)*max, BUBBLE_WIDTHS);
        
            // TODO(allen): when unable to grow?
            TentativeAssert(new_lines);
            buffer->line_widths = new_lines;
            buffer->widths_max = max;
        }
        
    }
    buffer->line_count = state.count;
    buffer->widths_count = state.count;
}

internal void
file_remeasure_starts(System_Functions *system,
                      General_Memory *general, Buffer_Type *buffer,
                      i32 line_start, i32 line_end, i32 line_shift,
                      i32 character_shift){
#if BUFFER_EXPERIMENT_SCALPEL <= 3
    ProfileMomentFunction();
    Assert(buffer->line_starts);
    file_grow_starts_as_needed(general, buffer, line_shift);
    buffer_remeasure_starts(buffer, line_start, line_end, line_shift, character_shift);
#endif
}

struct Opaque_Font_Advance{
    void *data;
    int stride;
};

inline Opaque_Font_Advance
get_opaque_font_advance(Render_Font *font){
    Opaque_Font_Advance result;
    result.data = (char*)font->chardata + OffsetOfPtr(font->chardata, xadvance);
    result.stride = sizeof(*font->chardata);
    return result;
}

internal void
file_grow_widths_as_needed(General_Memory *general, Buffer_Type *buffer){
#if BUFFER_EXPERIMENT_SCALPEL <= 3
    i32 line_count = buffer->line_count;
    if (line_count > buffer->widths_max || buffer->widths_max == 0){
        i32 new_max = LargeRoundUp(line_count, Kbytes(1));
        if (new_max < Kbytes(1)) new_max = Kbytes(1);
        if (buffer->line_widths){
            buffer->line_widths = (f32*)
                general_memory_reallocate(general, buffer->line_widths,
                                          sizeof(f32)*buffer->widths_count, sizeof(f32)*new_max, BUBBLE_WIDTHS);
        }
        else{
            buffer->line_widths = (f32*)
                general_memory_allocate(general, sizeof(f32)*new_max, BUBBLE_WIDTHS);
        }
        buffer->widths_max = new_max;
    }
#endif
}

internal void
file_remeasure_widths(System_Functions *system,
                      General_Memory *general, Buffer_Type *buffer, Render_Font *font,
                      i32 line_start, i32 line_end, i32 line_shift){
#if BUFFER_EXPERIMENT_SCALPEL <= 3
    ProfileMomentFunction();
    file_grow_widths_as_needed(general, buffer);
    buffer_remeasure_widths(buffer, font->advance_data, line_start, line_end, line_shift);
#endif
}

inline i32
view_wrapped_line_span(f32 line_width, f32 max_width){
    i32 line_count = CEIL32(line_width / max_width);
    if (line_count == 0) line_count = 1;
    return line_count;
}

internal i32
view_compute_lowest_line(File_View *view){
    i32 lowest_line = 0;
#if BUFFER_EXPERIMENT_SCALPEL <= 3
    i32 last_line = view->line_count - 1;
    if (last_line > 0){
        if (view->unwrapped_lines){
            lowest_line = last_line;
        }
        else{
            f32 wrap_y = view->line_wrap_y[last_line];
            lowest_line = FLOOR32(wrap_y / view->font_height);
            f32 max_width = view_compute_width(view);
            
            Editing_File *file = view->file;
            Assert(!file->state.is_dummy);
            f32 width = file->state.buffer.line_widths[last_line];
            i32 line_span = view_wrapped_line_span(width, max_width);
            lowest_line += line_span - 1;
        }
    }
#endif
    return lowest_line;
}

internal void
view_measure_wraps(System_Functions *system,
                   General_Memory *general, File_View *view){
#if BUFFER_EXPERIMENT_SCALPEL <= 3
    ProfileMomentFunction();
    Buffer_Type *buffer;

    buffer = &view->file->state.buffer;
    i32 line_count = buffer->line_count;
    
    if (view->line_max < line_count){
        i32 max = view->line_max = LargeRoundUp(line_count, Kbytes(1));
        if (view->line_wrap_y){
            view->line_wrap_y = (real32*)
                general_memory_reallocate_nocopy(general, view->line_wrap_y, sizeof(real32)*max, BUBBLE_WRAPS);
        }
        else{
            view->line_wrap_y = (real32*)
                general_memory_allocate(general, sizeof(real32)*max, BUBBLE_WRAPS);
        }
    }

    f32 line_height = (f32)view->font_height;
    f32 max_width = view_compute_width(view);
    buffer_measure_wrap_y(buffer, view->line_wrap_y, line_height, max_width);
    
    view->line_count = line_count;
#endif
}

internal void*
alloc_for_buffer(void *context, int *size){
    *size = LargeRoundUp(*size, Kbytes(4));
    void *data = general_memory_allocate((General_Memory*)context, *size, BUBBLE_BUFFER);
    return data;
}

internal void
file_create_from_string(System_Functions *system, Mem_Options *mem,
    Working_Set *working_set, Editing_File *file, char *filename,
    Font_Set *set, i16 font_id,
    String val, b8 super_locked = 0){
    
    General_Memory *general = &mem->general;
    Partition *part = &mem->part;
    Buffer_Init_Type init;
    i32 page_size, scratch_size, init_success;
    
    file->state = {};
    
#if BUFFER_EXPERIMENT_SCALPEL <= 3
    init = buffer_begin_init(&file->state.buffer, val.str, val.size);
    for (; buffer_init_need_more(&init); ){
        page_size = buffer_init_page_size(&init);
        page_size = LargeRoundUp(page_size, Kbytes(4));
        if (page_size < Kbytes(4)) page_size = Kbytes(4);
        void *data = general_memory_allocate(general, page_size, BUBBLE_BUFFER);
        buffer_init_provide_page(&init, data, page_size);
    }

    scratch_size = partition_remaining(part);
    Assert(scratch_size > 0);
    init_success = buffer_end_init(&init, part->base + part->pos, scratch_size);
    AllowLocal(init_success);
    Assert(init_success);
#endif
    
    file_init_strings(file);
    file_set_name(working_set, file, (char*)filename);
    
    file->state.font_id = font_id;
    
    file_synchronize_times(system, file, filename);
    
    Render_Font *font = get_font_info(set, font_id)->font;
    float *advance_data = 0;
    if (font) advance_data = font->advance_data;
    file_measure_starts_widths(system, general, &file->state.buffer, advance_data);

    file->settings.super_locked = super_locked;
    if (!super_locked){
        i32 request_size = Kbytes(64);
        file->state.undo.undo.max = request_size;
        file->state.undo.undo.strings = (u8*)general_memory_allocate(general, request_size, BUBBLE_UNDO_STRING);
        file->state.undo.undo.edit_max = request_size / sizeof(Edit_Step);
        file->state.undo.undo.edits = (Edit_Step*)general_memory_allocate(general, request_size, BUBBLE_UNDO);

        file->state.undo.redo.max = request_size;
        file->state.undo.redo.strings = (u8*)general_memory_allocate(general, request_size, BUBBLE_UNDO_STRING);
        file->state.undo.redo.edit_max = request_size / sizeof(Edit_Step);
        file->state.undo.redo.edits = (Edit_Step*)general_memory_allocate(general, request_size, BUBBLE_UNDO);

        file->state.undo.history.max = request_size;
        file->state.undo.history.strings = (u8*)general_memory_allocate(general, request_size, BUBBLE_UNDO_STRING);
        file->state.undo.history.edit_max = request_size / sizeof(Edit_Step);
        file->state.undo.history.edits = (Edit_Step*)general_memory_allocate(general, request_size, BUBBLE_UNDO);

        file->state.undo.children.max = request_size;
        file->state.undo.children.strings = (u8*)general_memory_allocate(general, request_size, BUBBLE_UNDO_STRING);
        file->state.undo.children.edit_max = request_size / sizeof(Buffer_Edit);
        file->state.undo.children.edits = (Buffer_Edit*)general_memory_allocate(general, request_size, BUBBLE_UNDO);

        file->state.undo.history_block_count = 1;
        file->state.undo.history_head_block = 0;
        file->state.undo.current_block_normal = 1;
    }
}

internal b32
file_create_empty(
    System_Functions *system, Mem_Options *mem,
    Working_Set *working_set, Editing_File *file,
    char *filename, Font_Set *set, i16 font_id){
    
    b32 result = 1;
    String empty_str = {};
    file_create_from_string(system, mem, working_set, file, filename, set, font_id, empty_str);
    return result;
}

internal b32
file_create_super_locked(
    System_Functions *system, Mem_Options *mem,
    Working_Set *working_set, Editing_File *file,
    char *filename, Font_Set *set, i16 font_id){
    b32 result = 1;
    String empty_str = {};
    file_create_from_string(system, mem, working_set, file, filename, set, font_id, empty_str, 1);
    return result;
}

struct Get_File_Result{
    Editing_File *file;
    i32 index;
};

internal Get_File_Result
working_set_get_available_file(Working_Set *working_set){
    Get_File_Result result = {};
    
    for (i32 buffer_index = 1; buffer_index < working_set->file_max_count; ++buffer_index){
        if (buffer_index == working_set->file_index_count ||
            working_set->files[buffer_index].state.is_dummy){
            result.index = buffer_index;
            result.file = working_set->files + buffer_index;
            if (buffer_index == working_set->file_index_count){
                ++working_set->file_index_count;
            }
            break;
        }
    }
    
    if (result.file) *result.file = {};
    
    return result;
}

internal i32
working_set_get_index(Working_Set *working_set, Editing_File *file){
    i32 index = (i32)(file - working_set->files);
    return(index);
}

internal void
file_close(System_Functions *system, General_Memory *general, Editing_File *file){
    if (file->state.still_lexing){
        system->cancel_job(BACKGROUND_THREADS, file->state.lex_job);
        if (file->state.swap_stack.tokens){
            general_memory_free(general, file->state.swap_stack.tokens);
            file->state.swap_stack.tokens = 0;
        }
    }
    if (file->state.token_stack.tokens){
        general_memory_free(general, file->state.token_stack.tokens);
    }
    
#if BUFFER_EXPERIMENT_SCALPEL <= 1
    Buffer_Type *buffer = &file->state.buffer;
    if (buffer->data){
        general_memory_free(general, buffer->data);
        general_memory_free(general, buffer->line_starts);
        general_memory_free(general, buffer->line_widths);
    }
#elif BUFFER_EXPERIMENT_SCALPEL == 2
    // TODO
#endif

    if (file->state.undo.undo.edits){
        general_memory_free(general, file->state.undo.undo.strings);
        general_memory_free(general, file->state.undo.undo.edits);
    
        general_memory_free(general, file->state.undo.redo.strings);
        general_memory_free(general, file->state.undo.redo.edits);
    
        general_memory_free(general, file->state.undo.history.strings);
        general_memory_free(general, file->state.undo.history.edits);

        general_memory_free(general, file->state.undo.children.strings);
        general_memory_free(general, file->state.undo.children.edits);
    }
}

inline void
file_get_dummy(Editing_File *file){
	*file = {};
	file->state.is_dummy = 1;
}

inline void
file_set_to_loading(Editing_File *file){
	file->state = {};
	file->settings = {};
	file->state.is_loading = 1;
}

struct Shift_Information{
	i32 start, end, amount;
};

internal
Job_Callback_Sig(job_full_lex){
    Editing_File *file = (Editing_File*)data[0];
    General_Memory *general = (General_Memory*)data[1];
    
    Cpp_File cpp_file;
    cpp_file.data = file->state.buffer.data;
    cpp_file.size = file->state.buffer.size;
    
    Cpp_Token_Stack tokens;
    tokens.tokens = (Cpp_Token*)memory->data;
    tokens.max_count = memory->size / sizeof(Cpp_Token);
    tokens.count = 0;
    
    Cpp_Lex_Data status;
    status = cpp_lex_file_nonalloc(cpp_file, &tokens);
    
    while (!status.complete){
        system->grow_thread_memory(memory);
        tokens.tokens = (Cpp_Token*)memory->data;
        tokens.max_count = memory->size / sizeof(Cpp_Token);
        status = cpp_lex_file_nonalloc(cpp_file, &tokens, status);
    }
    
    i32 new_max = LargeRoundUp(tokens.count+1, Kbytes(1));
    
    system->acquire_lock(FRAME_LOCK);
    {
        Assert(file->state.swap_stack.tokens == 0);
        file->state.swap_stack.tokens = (Cpp_Token*)
            general_memory_allocate(general, new_max*sizeof(Cpp_Token), BUBBLE_TOKENS);
    }
    system->release_lock(FRAME_LOCK);

    u8 *dest = (u8*)file->state.swap_stack.tokens;
    u8 *src = (u8*)tokens.tokens;
    
#if 1
    memcpy(dest, src, tokens.count*sizeof(Cpp_Token));
#else
    i32 copy_amount = Kbytes(8);
    i32 uncoppied = tokens.count*sizeof(Cpp_Token);
    if (copy_amount > uncoppied) copy_amount = uncoppied;
    
    while (uncoppied > 0){
        system->acquire_lock(FRAME_LOCK);
        memcpy(dest, src, copy_amount);
        system->release_lock(FRAME_LOCK);
        dest += copy_amount;
        src += copy_amount;
        uncoppied -= copy_amount;
        if (copy_amount > uncoppied) copy_amount = uncoppied;
    }
#endif
    
    system->acquire_lock(FRAME_LOCK);
    {
        file->state.token_stack.count = tokens.count;
        file->state.token_stack.max_count = new_max;
        if (file->state.token_stack.tokens)
            general_memory_free(general, file->state.token_stack.tokens);
        file->state.token_stack.tokens = file->state.swap_stack.tokens;
        file->state.swap_stack.tokens = 0;
    }
    system->release_lock(FRAME_LOCK);
    
    exchange->force_redraw = 1;
    
    // NOTE(allen): These are outside the locked section because I don't
    // think getting these out of order will cause critical bugs, and I
    // want to minimize what's done in locked sections.
    file->state.tokens_complete = 1;
    file->state.still_lexing = 0;
}


internal void
file_kill_tokens(System_Functions *system,
                 General_Memory *general, Editing_File *file){
    file->settings.tokens_exist = 0;
    if (file->state.still_lexing){
        system->cancel_job(BACKGROUND_THREADS, file->state.lex_job);
        if (file->state.swap_stack.tokens){
            general_memory_free(general, file->state.swap_stack.tokens);
            file->state.swap_stack.tokens = 0;
        }
    }
    if (file->state.token_stack.tokens){
        general_memory_free(general, file->state.token_stack.tokens);
    }
    file->state.tokens_complete = 0;
    file->state.token_stack = {};
}

#if BUFFER_EXPERIMENT_SCALPEL <= 0
internal void
file_first_lex_parallel(System_Functions *system,
                        General_Memory *general, Editing_File *file){
    file->settings.tokens_exist = 1;
    
    if (file->state.is_loading == 0 && file->state.still_lexing == 0){
        Assert(file->state.token_stack.tokens == 0);
    
        file->state.tokens_complete = 0;
        file->state.still_lexing = 1;

#if 0
        full_lex(system, file, general);
#else
    
        Job_Data job;
        job.callback = job_full_lex;
        job.data[0] = file;
        job.data[1] = general;
        job.memory_request = Kbytes(64);
        file->state.lex_job = system->post_job(BACKGROUND_THREADS, job);
#endif
    }
}

internal void
file_relex_parallel(System_Functions *system,
                    Mem_Options *mem, Editing_File *file,
                    i32 start_i, i32 end_i, i32 amount){
    General_Memory *general = &mem->general;
    Partition *part = &mem->part;
    if (file->state.token_stack.tokens == 0){
        file_first_lex_parallel(system, general, file);
        return;
    }
    
    b32 inline_lex = !file->state.still_lexing;
    if (inline_lex){
        Cpp_File cpp_file;
        cpp_file.data = file->state.buffer.data;
        cpp_file.size = file->state.buffer.size;
        
        Cpp_Token_Stack *stack = &file->state.token_stack;
        
        Cpp_Relex_State state = 
            cpp_relex_nonalloc_start(cpp_file, stack,
                                     start_i, end_i, amount, 100);
        
        Temp_Memory temp = begin_temp_memory(part);
        i32 relex_end;
        Cpp_Token_Stack relex_space;
        relex_space.count = 0;
        relex_space.max_count = state.space_request;
        relex_space.tokens = push_array(part, Cpp_Token, relex_space.max_count);
        if (cpp_relex_nonalloc_main(&state, &relex_space, &relex_end)){
            inline_lex = 0;
        }
        else{
            i32 delete_amount = relex_end - state.start_token_i;
            i32 shift_amount = relex_space.count - delete_amount;
            
            if (shift_amount != 0){
                int new_count = stack->count + shift_amount;
                if (new_count > stack->max_count){
                    int new_max = LargeRoundUp(new_count, Kbytes(1));
                    stack->tokens = (Cpp_Token*)
                        general_memory_reallocate(general, stack->tokens,
                                                  stack->count*sizeof(Cpp_Token),
                                                  new_max*sizeof(Cpp_Token), BUBBLE_TOKENS);
                    stack->max_count = new_max;
                }
                
                int shift_size = stack->count - relex_end;
                if (shift_size > 0){
                    Cpp_Token *old_base = stack->tokens + relex_end;
                    memmove(old_base + shift_amount, old_base,
                            sizeof(Cpp_Token)*shift_size);
                }
                
                stack->count += shift_amount;
            }
            
            memcpy(state.stack->tokens + state.start_token_i, relex_space.tokens,
                   sizeof(Cpp_Token)*relex_space.count);
        }
        
        end_temp_memory(temp);
    }
    
    if (!inline_lex){
        i32 end_token_i = cpp_get_end_token(&file->state.token_stack, end_i);
        cpp_shift_token_starts(&file->state.token_stack, end_token_i, amount);
        --end_token_i;
        if (end_token_i >= 0){
            Cpp_Token *token = file->state.token_stack.tokens + end_token_i;
            if (token->start < end_i && token->start + token->size > end_i){
                token->size += amount;
            }
        }
        
        file->state.still_lexing = 1;
   
#if 0
        full_lex(system, file, general);
#else
     
        Job_Data job;
        job.callback = job_full_lex;
        job.data[0] = file;
        job.data[1] = general;
        job.memory_request = Kbytes(64);
        file->state.lex_job = system->post_job(BACKGROUND_THREADS, job);
#endif
    }
}
#endif

internal bool32
file_grow_as_needed_(General_Memory *general, Editing_File *file, i32 new_size){
}

internal void
undo_stack_grow_string(General_Memory *general, Edit_Stack *stack, i32 extra_size){
    i32 old_max = stack->max;
    u8 *old_str = stack->strings;
    i32 new_max = old_max*2 + extra_size;
    u8 *new_str = (u8*)
        general_memory_reallocate(general, old_str, old_max, new_max);
    stack->strings = new_str;
    stack->max = new_max;
}

internal void
undo_stack_grow_edits(General_Memory *general, Edit_Stack *stack){
    i32 old_max = stack->edit_max;
    Edit_Step *old_eds = stack->edits;
    i32 new_max = old_max*2 + 2;
    Edit_Step *new_eds = (Edit_Step*)
        general_memory_reallocate(general, old_eds, old_max*sizeof(Edit_Step), new_max*sizeof(Edit_Step));
    stack->edits = new_eds;
    stack->edit_max = new_max;
}

internal void
child_stack_grow_string(General_Memory *general, Small_Edit_Stack *stack, i32 extra_size){
    i32 old_max = stack->max;
    u8 *old_str = stack->strings;
    i32 new_max = old_max*2 + extra_size;
    u8 *new_str = (u8*)
        general_memory_reallocate(general, old_str, old_max, new_max);
    stack->strings = new_str;
    stack->max = new_max;
}

internal void
child_stack_grow_edits(General_Memory *general, Small_Edit_Stack *stack, i32 amount){
    i32 old_max = stack->edit_max;
    Buffer_Edit *old_eds = stack->edits;
    i32 new_max = old_max*2 + amount;
    Buffer_Edit *new_eds = (Buffer_Edit*)
        general_memory_reallocate(general, old_eds, old_max*sizeof(Buffer_Edit), new_max*sizeof(Buffer_Edit));
    stack->edits = new_eds;
    stack->edit_max = new_max;
}

internal i32
undo_children_push(General_Memory *general, Small_Edit_Stack *children,
                   Buffer_Edit *edits, i32 edit_count, u8 *strings, i32 string_size){
    i32 result = children->edit_count;
    if (children->edit_count + edit_count > children->edit_max)
        child_stack_grow_edits(general, children, edit_count);
    
    if (children->size + string_size > children->max)
        child_stack_grow_string(general, children, string_size);
    
    memcpy(children->edits + children->edit_count, edits, edit_count*sizeof(Buffer_Edit));
    memcpy(children->strings + children->size, strings, string_size);
    
    Buffer_Edit *edit = children->edits + children->edit_count;
    i32 start_pos = children->size;
    for (i32 i = 0; i < edit_count; ++i, ++edit){
        edit->str_start += start_pos;
    }
    
    children->edit_count += edit_count;
    children->size += string_size;
    
    return result;
}

struct Edit_Spec{
    u8 *str;
    Edit_Step step;
};

#if BUFFER_EXPERIMENT_SCALPEL <= 3
internal Edit_Step*
file_post_undo(General_Memory *general, Editing_File *file,
               Edit_Step step, bool32 do_merge, bool32 can_merge){
    if (step.type == ED_NORMAL){
        file->state.undo.redo.size = 0;
        file->state.undo.redo.edit_count = 0;
    }
    
    Edit_Stack *undo = &file->state.undo.undo;
    Edit_Step *result = 0;
    
    if (step.child_count == 0){
        if (step.edit.end - step.edit.start + undo->size > undo->max)
            undo_stack_grow_string(general, undo, step.edit.end - step.edit.start);
    
        Buffer_Edit inv;
        buffer_invert_edit(&file->state.buffer, step.edit, &inv,
                           (char*)undo->strings, &undo->size, undo->max);
    
        Edit_Step inv_step = {};
        inv_step.edit = inv;
        inv_step.pre_pos = step.pre_pos;
        inv_step.post_pos = step.post_pos;
        inv_step.can_merge = (b8)can_merge;
        inv_step.type = ED_UNDO;
    
        bool32 did_merge = 0;
        if (do_merge && undo->edit_count > 0){
            Edit_Step prev = undo->edits[undo->edit_count-1];
            if (prev.can_merge && inv_step.edit.len == 0 && prev.edit.len == 0){
                if (prev.edit.end == inv_step.edit.start){
                    did_merge = 1;
                    inv_step.edit.start = prev.edit.start;
                    inv_step.pre_pos = prev.pre_pos;
                }
            }
        }
    
        if (did_merge){
            result = undo->edits + (undo->edit_count-1);
            *result = inv_step;
        }
        else{
            if (undo->edit_count == undo->edit_max)
                undo_stack_grow_edits(general, undo);
            result = undo->edits + (undo->edit_count++);
            *result = inv_step;
        }
    }
    else{
        Edit_Step inv_step = {};
        inv_step.type = ED_UNDO;
        inv_step.first_child = step.inverse_first_child;
        inv_step.inverse_first_child = step.first_child;
        inv_step.special_type = step.special_type;
        inv_step.child_count = step.inverse_child_count;
        inv_step.inverse_child_count = step.child_count;
        
        if (undo->edit_count == undo->edit_max)
            undo_stack_grow_edits(general, undo);
        result = undo->edits + (undo->edit_count++);
        *result = inv_step;
    }
    return result;
}
#endif    

inline void
undo_stack_pop(Edit_Stack *stack){
    if (stack->edit_count > 0){
        Edit_Step *edit = stack->edits + (--stack->edit_count);
        stack->size -= edit->edit.len;
    }
}

#if BUFFER_EXPERIMENT_SCALPEL <= 3
internal void
file_post_redo(General_Memory *general, Editing_File *file, Edit_Step step){
    Edit_Stack *redo = &file->state.undo.redo;

    if (step.child_count == 0){
        if (step.edit.end - step.edit.start + redo->size > redo->max)
            undo_stack_grow_string(general, redo, step.edit.end - step.edit.start);
    
        Buffer_Edit inv;
        buffer_invert_edit(&file->state.buffer, step.edit, &inv,
                           (char*)redo->strings, &redo->size, redo->max);
    
        Edit_Step inv_step = {};
        inv_step.edit = inv;
        inv_step.pre_pos = step.pre_pos;
        inv_step.post_pos = step.post_pos;
        inv_step.type = ED_REDO;
    
        if (redo->edit_count == redo->edit_max)
            undo_stack_grow_edits(general, redo);
        redo->edits[redo->edit_count++] = inv_step;
    }
    else{
        Edit_Step inv_step = {};
        inv_step.type = ED_REDO;
        inv_step.first_child = step.inverse_first_child;
        inv_step.inverse_first_child = step.first_child;
        inv_step.special_type = step.special_type;
        inv_step.child_count = step.inverse_child_count;
        inv_step.inverse_child_count = step.child_count;
        
        if (redo->edit_count == redo->edit_max)
            undo_stack_grow_edits(general, redo);
        redo->edits[redo->edit_count++] = inv_step;
    }
}
#endif

inline void
file_post_history_block(Editing_File *file, i32 pos){
    Assert(file->state.undo.history_head_block < pos);
    Assert(pos < file->state.undo.history.edit_count);
    
    Edit_Step *history = file->state.undo.history.edits;
    Edit_Step *step = history + file->state.undo.history_head_block;
    step->next_block = pos;
    step = history + pos;
    step->prev_block = file->state.undo.history_head_block;
    file->state.undo.history_head_block = pos;
    ++file->state.undo.history_block_count;
}

inline void
file_unpost_history_block(Editing_File *file){
    Assert(file->state.undo.history_block_count > 1);
    --file->state.undo.history_block_count;
    Edit_Step *old_head = file->state.undo.history.edits + file->state.undo.history_head_block;
    file->state.undo.history_head_block = old_head->prev_block;
}

#if BUFFER_EXPERIMENT_SCALPEL <= 3
internal Edit_Step*
file_post_history(General_Memory *general, Editing_File *file,
                  Edit_Step step, b32 do_merge, b32 can_merge){
    Edit_Stack *history = &file->state.undo.history;
    Edit_Step *result = 0;
    
    persist Edit_Type reverse_types[4];
    if (reverse_types[ED_UNDO] == 0){
        reverse_types[ED_NORMAL] = ED_REVERSE_NORMAL;
        reverse_types[ED_REVERSE_NORMAL] = ED_NORMAL;
        reverse_types[ED_UNDO] = ED_REDO;
        reverse_types[ED_REDO] = ED_UNDO;
    }
    
    if (step.child_count == 0){
        if (step.edit.end - step.edit.start + history->size > history->max)
            undo_stack_grow_string(general, history, step.edit.end - step.edit.start);
        
        Buffer_Edit inv;
        buffer_invert_edit(&file->state.buffer, step.edit, &inv,
                           (char*)history->strings, &history->size, history->max);
        
        Edit_Step inv_step = {};
        inv_step.edit = inv;
        inv_step.pre_pos = step.pre_pos;
        inv_step.post_pos = step.post_pos;
        inv_step.can_merge = (b8)can_merge;
        inv_step.type = reverse_types[step.type];
        
        bool32 did_merge = 0;
        if (do_merge && history->edit_count > 0){
            Edit_Step prev = history->edits[history->edit_count-1];
            if (prev.can_merge && inv_step.edit.len == 0 && prev.edit.len == 0){
                if (prev.edit.end == inv_step.edit.start){
                    did_merge = 1;
                    inv_step.edit.start = prev.edit.start;
                    inv_step.pre_pos = prev.pre_pos;
                }
            }
        }
        
        if (did_merge){
            result = history->edits + (history->edit_count-1);
        }
        else{
            if (history->edit_count == history->edit_max)
                undo_stack_grow_edits(general, history);
            result = history->edits + (history->edit_count++);
        }
        
        *result = inv_step;
    }
    else{
        Edit_Step inv_step = {};
        inv_step.type = reverse_types[step.type];
        inv_step.first_child = step.inverse_first_child;
        inv_step.inverse_first_child = step.first_child;
        inv_step.special_type = step.special_type;
        inv_step.inverse_child_count = step.child_count;
        inv_step.child_count = step.inverse_child_count;
        
        if (history->edit_count == history->edit_max)
            undo_stack_grow_edits(general, history);
        result = history->edits + (history->edit_count++);
        *result = inv_step;
    }
    
    return result;
}
#endif

inline Full_Cursor
view_compute_cursor_from_pos(File_View *view, i32 pos){
    Editing_File *file = view->file;
    Style *style = view->style;
    Render_Font *font = get_font_info(view->font_set, style->font_id)->font;
    
    Full_Cursor result = {};
    if (font){
        f32 max_width = view_compute_width(view);
        result = buffer_cursor_from_pos(&file->state.buffer, pos, view->line_wrap_y,
                                        max_width, (f32)view->font_height, font->advance_data);
    }
    return result;
}

inline Full_Cursor
view_compute_cursor_from_unwrapped_xy(File_View *view, f32 seek_x, f32 seek_y,
                                      b32 round_down = 0){
    Editing_File *file = view->file;
    Style *style = view->style;
    Render_Font *font = get_font_info(view->font_set, style->font_id)->font;
    
    Full_Cursor result = {};
    if (font){
        f32 max_width = view_compute_width(view);
        result = buffer_cursor_from_unwrapped_xy(&file->state.buffer, seek_x, seek_y,
                                                 round_down, view->line_wrap_y,
                                                 max_width, (f32)view->font_height, font->advance_data);
    }

    return result;
}

internal Full_Cursor
view_compute_cursor_from_wrapped_xy(File_View *view, f32 seek_x, f32 seek_y,
                                    b32 round_down = 0){
    Editing_File *file = view->file;
    Style *style = view->style;
    Render_Font *font = get_font_info(view->font_set, style->font_id)->font;
    
    Full_Cursor result = {};
    if (font){
        f32 max_width = view_compute_width(view);
        result = buffer_cursor_from_wrapped_xy(&file->state.buffer, seek_x, seek_y,
                                               round_down, view->line_wrap_y,
                                               max_width, (f32)view->font_height, font->advance_data);
    }
    
    return result;
}

internal Full_Cursor
view_compute_cursor_from_line_pos(File_View *view, i32 line, i32 pos){
    Editing_File *file = view->file;
    Style *style = view->style;
    Render_Font *font = get_font_info(view->font_set, style->font_id)->font;
    
    Full_Cursor result = {};
    if (font){
        f32 max_width = view_compute_width(view);
        result = buffer_cursor_from_line_character(&file->state.buffer, line, pos,
                                               view->line_wrap_y, max_width, (f32)view->font_height, font->advance_data);
    }
    
    return result;
}

inline Full_Cursor
view_compute_cursor(File_View *view, Buffer_Seek seek){
    Full_Cursor result = {};
    
    switch(seek.type){
        case buffer_seek_pos:
        result = view_compute_cursor_from_pos(view, seek.pos);
        break;
        
        case buffer_seek_wrapped_xy:
        result = view_compute_cursor_from_wrapped_xy(view, seek.x, seek.y);
        break;
        
        case buffer_seek_unwrapped_xy:
        result = view_compute_cursor_from_unwrapped_xy(view, seek.x, seek.y);
        break;
        
        case buffer_seek_line_char:
        result = view_compute_cursor_from_line_pos(view, seek.line, seek.character);
        break;
    }
    
    return result;
}

inline Full_Cursor
view_compute_cursor_from_xy(File_View *view, f32 seek_x, f32 seek_y){
    Full_Cursor result;
    if (view->unwrapped_lines) result = view_compute_cursor_from_unwrapped_xy(view, seek_x, seek_y);
    else result = view_compute_cursor_from_wrapped_xy(view, seek_x, seek_y);
	return result;
}

inline void
view_set_temp_highlight(File_View *view, i32 pos, i32 end_pos){
	view->temp_highlight = view_compute_cursor_from_pos(view, pos);
	view->temp_highlight_end_pos = end_pos;
    view->show_temp_highlight = 1;
}

inline i32
view_get_cursor_pos(File_View *view){
    i32 result;
    if (view->show_temp_highlight){
        result = view->temp_highlight.pos;
    }
    else{
        result = view->cursor.pos;
    }
    return result;
}

inline f32
view_get_cursor_x(File_View *view){
    f32 result;
    Full_Cursor *cursor;
    if (view->show_temp_highlight){
        cursor = &view->temp_highlight;
    }
    else{
        cursor = &view->cursor;
    }
    if (view->unwrapped_lines){
        result = cursor->unwrapped_x;
    }
    else{
        result = cursor->wrapped_x;
    }
    return result;
}

inline f32
view_get_cursor_y(File_View *view){
    Full_Cursor *cursor;
    f32 result;
    
    if (view->show_temp_highlight) cursor = &view->temp_highlight;
    else cursor = &view->cursor;
    
    if (view->unwrapped_lines) result = cursor->unwrapped_y;
    else result = cursor->wrapped_y;
    
    return result;
}

internal void
view_file_loaded_init(System_Functions *system, File_View *view, i32 cursor_pos){
    General_Memory *general = &view->view_base.mem->general;
    
    view_measure_wraps(system, general, view);
    view->cursor = view_compute_cursor_from_pos(view, cursor_pos);
}

internal void
view_set_file(System_Functions *system, File_View *view,
              Editing_File *file, Font_Set *set, Style *style,
              Hook_Function *open_hook, void *cmd_context, Application_Links *app){
    Panel *panel = view->view_base.panel;
    view->file = file;
    view->locked = file->settings.super_locked;

    Font_Info *info = get_font_info(set, style->font_id);
    view->style = style;
    view->font_advance = info->advance;
    view->font_height = info->height;
    view->font_set = set;
    view->unwrapped_lines = file->settings.unwrapped_lines;
    file->settings.set = set;
    
    view->cursor = {};
    
    if (!file->state.is_loading){
        view_file_loaded_init(system, view, file->state.cursor_pos);
    }
    
    f32 cursor_x, cursor_y;
    f32 w, h;
    f32 target_x, target_y;
    
    cursor_x = view_get_cursor_x(view);
    cursor_y = view_get_cursor_y(view);
    
    w = (f32)(panel->inner.x1 - panel->inner.x0);
    h = (f32)(panel->inner.y1 - panel->inner.y0);
    
    target_x = 0;
    if (cursor_x < target_x){
        target_x = (f32)Max(0, cursor_x - w*.5f);
    }
    else if (cursor_x >= target_x + w){
        target_x = (f32)(cursor_x - w*.5f);
    }
    
    target_y = (f32)FLOOR32(cursor_y - h*.5f);
    if (target_y < 0) target_y = 0;
    
    view->target_x = target_x;
    view->target_y = target_y;
    view->scroll_x = target_x;
    view->scroll_y = target_y;
    
    view->vel_y = 1.f;
    view->vel_x = 1.f;

    if (open_hook && file->settings.is_initialized == 0){
        open_hook(app);
        file->settings.is_initialized = 1;
    }
}

struct Relative_Scrolling{
    real32 scroll_x, scroll_y;
    real32 target_x, target_y;
};

internal Relative_Scrolling
view_get_relative_scrolling(File_View *view){
    Relative_Scrolling result;
    real32 cursor_x, cursor_y;
    cursor_x = view_get_cursor_x(view);
    cursor_y = view_get_cursor_y(view);
    result.scroll_x = cursor_x - view->scroll_x;
    result.scroll_y = cursor_y - view->scroll_y;
    result.target_x = cursor_x - view->target_x;
    result.target_y = cursor_y - view->target_y;
    return result;
}

internal void
view_set_relative_scrolling(File_View *view, Relative_Scrolling scrolling){
    real32 cursor_x, cursor_y;
    cursor_x = view_get_cursor_x(view);
    cursor_y = view_get_cursor_y(view);
    view->scroll_y = cursor_y - scrolling.scroll_y;
    view->target_y = cursor_y - scrolling.target_y;
    if (view->scroll_y < 0) view->scroll_y = 0;
    if (view->target_y < 0) view->target_y = 0;
}

inline void
view_cursor_move(File_View *view, Full_Cursor cursor){
	view->cursor = cursor;
    view->preferred_x = view_get_cursor_x(view);
	view->file->state.cursor_pos = view->cursor.pos;
    view->show_temp_highlight = 0;
}

inline void
view_cursor_move(File_View *view, i32 pos){
    Full_Cursor cursor = view_compute_cursor_from_pos(view, pos);
    view_cursor_move(view, cursor);
}

inline void
view_cursor_move(File_View *view, f32 x, f32 y, b32 round_down = 0){
    Full_Cursor cursor;
    if (view->unwrapped_lines){
        cursor = view_compute_cursor_from_unwrapped_xy(view, x, y, round_down);
    }
    else{
        cursor = view_compute_cursor_from_wrapped_xy(view, x, y, round_down);
    }
    view_cursor_move(view, cursor);
}

inline void
view_cursor_move(File_View *view, i32 line, i32 pos){
    Full_Cursor cursor = view_compute_cursor_from_line_pos(view, line, pos);
    view_cursor_move(view, cursor);
}

inline void
view_set_widget(File_View *view, File_View_Widget_Type type){
    view->widget.type = type;
}

inline i32
view_widget_height(File_View *view, i32 font_height){
    i32 result = 0;
    switch (view->widget.type){
    case FWIDG_NONE: break;
    case FWIDG_SEARCH: result = font_height + 2; break;
    case FWIDG_GOTO_LINE: result = font_height + 2; break;
    case FWIDG_TIMELINES: result = view->widget.height; break;
    }
    return result;
}

inline i32_Rect
view_widget_rect(File_View *view, i32 font_height){
    Panel *panel = view->view_base.panel;
    i32_Rect whole = panel->inner;
    i32_Rect result;
    result.x0 = whole.x0;
    result.x1 = whole.x1;
    result.y0 = whole.y0 + font_height + 2;
    result.y1 = result.y0 + view_widget_height(view, font_height);
    
    return result;
}

#if FRED_SLOW
inline b32
debug_edit_step_check(Edit_Step a, Edit_Step b){
    Assert(a.type == b.type);
    Assert(a.can_merge == b.can_merge);
    Assert(a.pre_pos == b.pre_pos);
    Assert(a.post_pos == b.post_pos);
    Assert(a.edit.start == b.edit.start);
    Assert(a.edit.end == b.edit.end);
    Assert(a.edit.len == b.edit.len);
    return 1;
}
#endif

enum History_Mode{
    hist_normal,
    hist_backward,
    hist_forward
};

internal void
file_update_history_before_edit(Mem_Options *mem, Editing_File *file, Edit_Step step, u8 *str,
                                History_Mode history_mode){
    if (!file->state.undo.undo.edits) return;
#if BUFFER_EXPERIMENT_SCALPEL <= 3
    General_Memory *general = &mem->general;
    
#if FRED_SLOW
    if (history_mode == hist_backward)
        debug_edit_step_check(step, file->state.undo.history.edits[file->state.undo.edit_history_cursor]);
    else if (history_mode == hist_forward)
        debug_edit_step_check(step, file->state.undo.history.edits[file->state.undo.history.edit_count]);
    switch (step.type){
    case ED_UNDO:
    {
        Assert(file->state.undo.undo.edit_count > 0);
        debug_edit_step_check(step, file->state.undo.undo.edits[file->state.undo.undo.edit_count-1]);
    }break;
    case ED_REDO:
    {
        Assert(file->state.undo.redo.edit_count > 0);
        debug_edit_step_check(step, file->state.undo.redo.edits[file->state.undo.redo.edit_count-1]);
    }break;
    }
#endif
    
    b32 can_merge = 0, do_merge = 0;
    switch (step.type){
    case ED_NORMAL:
    {
        if (step.edit.len == 1 && str && char_is_alpha_numeric(*str)) can_merge = 1;
        if (step.edit.len == 1 && str && (can_merge || char_is_whitespace(*str))) do_merge = 1;
        
        if (history_mode != hist_forward)
            file_post_history(general, file, step, do_merge, can_merge);
        
        file_post_undo(general, file, step, do_merge, can_merge);
    }break;
    
    case ED_REVERSE_NORMAL:
    {
        if (history_mode != hist_forward)
            file_post_history(general, file, step, do_merge, can_merge);
        
        undo_stack_pop(&file->state.undo.undo);
        
        b32 restore_redos = 0;
        Edit_Step *redo_end = 0;
        
        if (history_mode == hist_backward && file->state.undo.edit_history_cursor > 0){
            restore_redos = 1;
            redo_end = file->state.undo.history.edits + (file->state.undo.edit_history_cursor - 1);
        }
        else if (history_mode == hist_forward && file->state.undo.history.edit_count > 0){
            restore_redos = 1;
            redo_end = file->state.undo.history.edits + (file->state.undo.history.edit_count - 1);
        }
        
        if (restore_redos){
            Edit_Step *redo_start = redo_end;
            i32 steps_of_redo = 0;
            i32 strings_of_redo = 0;
            i32 undo_count = 0;
            while (redo_start->type == ED_REDO || redo_start->type == ED_UNDO){
                if (redo_start->type == ED_REDO){
                    if (undo_count > 0) --undo_count;
                    else{
                        ++steps_of_redo;
                        strings_of_redo += redo_start->edit.len;
                    }
                }
                else{
                    ++undo_count;
                }
                --redo_start;
            }
            
            if (redo_start < redo_end){
                ++redo_start;
                ++redo_end;
                
                if (file->state.undo.redo.edit_count + steps_of_redo > file->state.undo.redo.edit_max)
                    undo_stack_grow_edits(general, &file->state.undo.redo);
                
                if (file->state.undo.redo.size + strings_of_redo > file->state.undo.redo.max)
                    undo_stack_grow_string(general, &file->state.undo.redo, strings_of_redo);
                
                u8 *str_src = file->state.undo.history.strings + redo_end->edit.str_start;
                u8 *str_dest_base = file->state.undo.redo.strings;
                i32 str_redo_pos = file->state.undo.redo.size + strings_of_redo;
                
                Edit_Step *edit_src = redo_end;
                Edit_Step *edit_dest =
                    file->state.undo.redo.edits + file->state.undo.redo.edit_count + steps_of_redo;
                
                i32 undo_count = 0;
                for (i32 i = 0; i < steps_of_redo;){
                    --edit_src;
                    str_src -= edit_src->edit.len;
                    if (edit_src->type == ED_REDO){
                        if (undo_count > 0){
                            --undo_count;
                        }
                        else{
                            ++i;
                            
                            --edit_dest;
                            *edit_dest = *edit_src;
                            
                            str_redo_pos -= edit_dest->edit.len;
                            edit_dest->edit.str_start = str_redo_pos;
                            
                            memcpy(str_dest_base + str_redo_pos, str_src, edit_dest->edit.len);
                        }
                    }
                    else{
                        ++undo_count;
                    }
                }
                Assert(undo_count == 0);
                
                file->state.undo.redo.size += strings_of_redo;
                file->state.undo.redo.edit_count += steps_of_redo;
            }
        }
    }break;
    
    case ED_UNDO:
    {
        if (history_mode != hist_forward)
            file_post_history(general, file, step, do_merge, can_merge);
        file_post_redo(general, file, step);
        undo_stack_pop(&file->state.undo.undo);
    }break;
    
    case ED_REDO:
    {
        if (step.edit.len == 1 && str && char_is_alpha_numeric(*str)) can_merge = 1;
        if (step.edit.len == 1 && str && (can_merge || char_is_whitespace(*str))) do_merge = 1;
        
        if (history_mode != hist_forward)
            file_post_history(general, file, step, do_merge, can_merge);
        
        file_post_undo(general, file, step, do_merge, can_merge);
        undo_stack_pop(&file->state.undo.redo);
    }break;
    }
    
    if (history_mode != hist_forward){
        if (step.type == ED_UNDO || step.type == ED_REDO){
            if (file->state.undo.current_block_normal){
                file_post_history_block(file, file->state.undo.history.edit_count - 1);
                file->state.undo.current_block_normal = 0;
            }
        }
        else{
            if (!file->state.undo.current_block_normal){
                file_post_history_block(file, file->state.undo.history.edit_count - 1);
                file->state.undo.current_block_normal = 1;
            }
        }
    }
    else{
        if (file->state.undo.history_head_block == file->state.undo.history.edit_count){
            file_unpost_history_block(file);
            file->state.undo.current_block_normal = !file->state.undo.current_block_normal;
        }
    }
    
    if (history_mode == hist_normal){
        file->state.undo.edit_history_cursor = file->state.undo.history.edit_count;
    }
#endif
}

inline b32
debug_step_match(Edit_Step a, Edit_Step b){
    Assert(a.type == b.type);
    Assert(a.can_merge == b.can_merge);
    Assert(a.pre_pos == b.pre_pos);
    Assert(a.post_pos == b.post_pos);
    Assert(a.next_block == b.next_block);
    Assert(a.prev_block == b.prev_block);
    Assert(a.edit.start == b.edit.start);
    Assert(a.edit.end == b.edit.end);
    Assert(a.edit.len == b.edit.len);
    return 1;
}

inline void
file_pre_edit_maintenance(System_Functions *system,
                          General_Memory *general,
                          Editing_File *file){
    if (file->state.still_lexing){
        system->cancel_job(BACKGROUND_THREADS, file->state.lex_job);
        if (file->state.swap_stack.tokens){
            general_memory_free(general, file->state.swap_stack.tokens);
            file->state.swap_stack.tokens = 0;
        }
    }
    file->state.last_4ed_edit_time = system->time();
}

struct Cursor_Fix_Descriptor{
    b32 is_batch;
    union{
        struct{
            Buffer_Edit *batch;
            i32 batch_size;
        };
        struct{
            i32 start, end;
            i32 shift_amount;
        };
    };
};

internal void
file_edit_cursor_fix(System_Functions *system,
                     Partition *part, General_Memory *general,
                     Editing_File *file, Editing_Layout *layout,
                     Cursor_Fix_Descriptor desc){
    Full_Cursor temp_cursor;
    File_View *current_view;
    Temp_Memory cursor_temp = begin_temp_memory(part);
    i32 cursor_max = layout->panel_max_count * 2;
    Cursor_With_Index *cursors = push_array(part, Cursor_With_Index, cursor_max);
    
    f32 y_offset = 0, y_position = 0;
    i32 cursor_count = 0;
    i32 panel_count = layout->panel_count;
    Panel *current_panel = layout->panels;
    for (i32 i = 0; i < panel_count; ++i, ++current_panel){
        current_view = view_to_file_view(current_panel->view);
        if (current_view && current_view->file == file){
            view_measure_wraps(system, general, current_view);
            write_cursor_with_index(cursors, &cursor_count, current_view->cursor.pos);
            write_cursor_with_index(cursors, &cursor_count, current_view->mark - 1);
            write_cursor_with_index(cursors, &cursor_count, current_view->scroll_i - 1);
        }
    }
    
    if (cursor_count > 0){
        buffer_sort_cursors(cursors, cursor_count);
        if (desc.is_batch){
            buffer_batch_edit_update_cursors(cursors, cursor_count,
                                             desc.batch, desc.batch_size);
        }
        else{
            buffer_update_cursors(cursors, cursor_count,
                                  desc.start, desc.end,
                                  desc.shift_amount + (desc.end - desc.start));
                                  
        }
        buffer_unsort_cursors(cursors, cursor_count);
        
        cursor_count = 0;
        current_panel = layout->panels;
        for (i32 i = 0; i < panel_count; ++i, ++current_panel){
            current_view = view_to_file_view(current_panel->view);
            if (current_view && current_view->file == file){
                view_cursor_move(current_view, cursors[cursor_count++].pos);
                current_view->preferred_x = view_get_cursor_x(current_view);
                
                current_view->mark = cursors[cursor_count++].pos + 1;
                current_view->scroll_i = cursors[cursor_count++].pos + 1;
                temp_cursor = view_compute_cursor_from_pos(current_view, current_view->scroll_i);
                y_offset = MOD(current_view->scroll_y, current_view->font_height);
                
                if (current_view->unwrapped_lines){
                    y_position = temp_cursor.unwrapped_y + y_offset;
                    current_view->target_y += (y_position - current_view->scroll_y);
                    current_view->scroll_y = y_position;
                }
                else{
                    y_position = temp_cursor.wrapped_y + y_offset;
                    current_view->target_y += (y_position - current_view->scroll_y);
                    current_view->scroll_y = y_position;
                }
            }
        }
    }
    
    end_temp_memory(cursor_temp);
}

internal void
file_do_single_edit(System_Functions *system,
                    Mem_Options *mem, Editing_File *file,
                    Editing_Layout *layout, Edit_Spec spec, History_Mode history_mode){
    ProfileMomentFunction();
    
    // NOTE(allen): fixing stuff beforewards????
    file_update_history_before_edit(mem, file, spec.step, spec.str, history_mode);
    file_pre_edit_maintenance(system, &mem->general, file);

    // NOTE(allen): actual text replacement
#if BUFFER_EXPERIMENT_SCALPEL <= 3
    i32 shift_amount = 0;
    General_Memory *general = &mem->general;
    Partition *part = &mem->part;

    char *str = (char*)spec.str;
    i32 start = spec.step.edit.start;
    i32 end = spec.step.edit.end;
    i32 str_len = spec.step.edit.len;

    i32 scratch_size = partition_remaining(part);
    
    Assert(scratch_size > 0);
    i32 request_amount = 0;
    while (buffer_replace_range(&file->state.buffer, start, end, str, str_len, &shift_amount,
                                part->base + part->pos, scratch_size, &request_amount)){
        void *new_data = 0;
        if (request_amount > 0){
            new_data = general_memory_allocate(general, request_amount, BUBBLE_BUFFER);
        }
        void *old_data = buffer_edit_provide_memory(&file->state.buffer, new_data, request_amount);
        if (old_data) general_memory_free(general, old_data);
    }
    
    i32 line_start = buffer_get_line_index(&file->state.buffer, start);
    i32 line_end = buffer_get_line_index(&file->state.buffer, end);
    i32 replaced_line_count = line_end - line_start;
    i32 new_line_count = buffer_count_newlines(&file->state.buffer, start, start+str_len);
    i32 line_shift =  new_line_count - replaced_line_count;
    
    file_remeasure_starts(system, general, &file->state.buffer,
                          line_start, line_end, line_shift, shift_amount);

    Render_Font *font = get_font_info(file->settings.set, file->state.font_id)->font;
    if (font){
        file_remeasure_widths(system, general, &file->state.buffer,
                              font, line_start, line_end, line_shift);
    }
    
    i32 panel_count = layout->panel_count;
    Panel *current_panel = layout->panels;
    for (i32 i = 0; i < panel_count; ++i, ++current_panel){
        File_View *current_view = view_to_file_view(current_panel->view);
        if (current_view && current_view->file == file){
            view_measure_wraps(system, general, current_view);
        }
    }
#endif
    
#if BUFFER_EXPERIMENT_SCALPEL <= 0
    // NOTE(allen): fixing stuff afterwards
    if (file->settings.tokens_exist)
        file_relex_parallel(system, mem, file, start, end, shift_amount);
#endif

#if BUFFER_EXPERIMENT_SCALPEL <= 3
    Cursor_Fix_Descriptor desc = {};
    desc.start = start;
    desc.end = end;
    desc.shift_amount = shift_amount;
    
    file_edit_cursor_fix(system, part, general,
                         file, layout, desc);
    
#endif
}

internal void
view_do_white_batch_edit(System_Functions *system, Mem_Options *mem, File_View *view, Editing_File *file,
                         Editing_Layout *layout, Edit_Spec spec, History_Mode history_mode){
    if (view->locked) return;
#if BUFFER_EXPERIMENT_SCALPEL <= 3
    Assert(file);
    ProfileMomentFunction();
    
    // NOTE(allen): fixing stuff "beforewards"????
    Assert(spec.str == 0);
    file_update_history_before_edit(mem, file, spec.step, 0, history_mode);
    file_pre_edit_maintenance(system, &mem->general, file);
    
    // NOTE(allen): actual text replacement
    General_Memory *general = &mem->general;
    Partition *part = &mem->part;
    
    u8 *str_base = file->state.undo.children.strings;
    i32 batch_size = spec.step.child_count;
    Buffer_Edit *batch = file->state.undo.children.edits + spec.step.first_child;
    
    Assert(spec.step.first_child < file->state.undo.children.edit_count);
    Assert(batch_size >= 0);

    i32 scratch_size = partition_remaining(part);
    Buffer_Batch_State state = {};
    i32 request_amount;
    while (buffer_batch_edit_step(&state, &file->state.buffer, batch, (char*)str_base, batch_size,
                                  part->base + part->pos, scratch_size, &request_amount)){
        void *new_data = 0;
        if (request_amount > 0){
            new_data = general_memory_allocate(general, request_amount, BUBBLE_BUFFER);
        }
        void *old_data = buffer_edit_provide_memory(&file->state.buffer, new_data, request_amount);
        if (old_data) general_memory_free(general, old_data);
    }
    
    // NOTE(allen): token fixing
    if (file->state.tokens_complete){
        Cpp_Token_Stack tokens = file->state.token_stack;
        Cpp_Token *token = tokens.tokens;
        Cpp_Token *end_token = tokens.tokens + tokens.count;
        
        Buffer_Edit *edit = batch;
        Buffer_Edit *end_edit = batch + batch_size;
        
        i32 shift_amount = 0;
        i32 local_shift = 0;
        
        for (; token < end_token && edit < end_edit; ++edit){
            local_shift = (edit->len - (edit->end - edit->start));
            for (; token->start < edit->start && edit->start < token->start + token->size &&
                     token < end_token; ++token){
                token->size += local_shift;
            }
            for (; token->start < edit->start && token < end_token; ++token){
                token->start += shift_amount;
            }
            shift_amount += local_shift;
        }
        for (; token < end_token; ++token){
            token->start += shift_amount;
        }
    }
    
    // NOTE(allen): meta data
    {
        Buffer_Measure_Starts state = {};
        Render_Font *font = get_font_info(file->settings.set, file->state.font_id)->font;
        float *advance_data = 0;
        if (font) advance_data = font->advance_data;
        buffer_measure_starts_widths(&state, &file->state.buffer, advance_data);
    }
    
    // NOTE(allen): cursor fixing
    Cursor_Fix_Descriptor desc = {};
    desc.is_batch = 1;
    desc.batch = batch;
    desc.batch_size = batch_size;
    
    file_edit_cursor_fix(system, part, general,
                         file, layout, desc);
    
#endif
}

inline void
file_replace_range(System_Functions *system,
    Mem_Options *mem, Editing_File *file, Editing_Layout *layout,
    i32 start, i32 end, char *str, i32 len, i32 next_cursor){
    Edit_Spec spec = {};
    spec.step.type = ED_NORMAL;
    spec.step.edit.start =  start;
    spec.step.edit.end = end;
    spec.step.edit.len = len;
    spec.step.pre_pos = file->state.cursor_pos;
    spec.step.post_pos = next_cursor;
    spec.str = (u8*)str;
    file_do_single_edit(system, mem, file, layout, spec, hist_normal);
}

inline void
view_replace_range(System_Functions *system,
                   Mem_Options *mem, File_View *view, Editing_Layout *layout,
                   i32 start, i32 end, char *str, i32 len, i32 next_cursor){
    if (view->locked) return;
    Edit_Spec spec = {};
    spec.step.type = ED_NORMAL;
    spec.step.edit.start =  start;
    spec.step.edit.end = end;
    spec.step.edit.len = len;
    spec.step.pre_pos = view->cursor.pos;
    spec.step.post_pos = next_cursor;
    spec.str = (u8*)str;
    file_do_single_edit(system, mem, view->file, layout, spec, hist_normal);
}

inline void
view_post_paste_effect(File_View *view, i32 ticks, i32 start, i32 size, u32 color){
    Editing_File *file = view->file;

    file->state.paste_effect.start = start;
    file->state.paste_effect.end = start + size;
    file->state.paste_effect.color = color;
    file->state.paste_effect.tick_down = ticks;
    file->state.paste_effect.tick_max = ticks;
}

internal void
view_undo_redo(System_Functions *system,
               Mem_Options *mem, Editing_Layout *layout, File_View *view, Editing_File *file,
               Edit_Stack *stack, Edit_Type expected_type){
    if (view->locked) return;
    if (file && stack->edit_count > 0){
        Edit_Step step = stack->edits[stack->edit_count-1];
        
        Assert(step.type == expected_type);
        
        Edit_Spec spec = {};
        spec.step = step;
        
        if (step.child_count == 0){
            spec.step.edit.str_start = 0;
            spec.str = stack->strings + step.edit.str_start;
            
            file_do_single_edit(system, mem, file, layout, spec, hist_normal);

            if (expected_type == ED_UNDO) view_cursor_move(view, step.pre_pos);
            else view_cursor_move(view, step.post_pos);
            view->mark = view->cursor.pos;
            
            view_post_paste_effect(view, 10, step.edit.start, step.edit.len,
                                   view->style->main.undo_color);
        }
        else{
            TentativeAssert(spec.step.special_type == 1);
            view_do_white_batch_edit(system, mem, view, file, layout, spec, hist_normal);
        }
    }
}

inline void
view_undo(System_Functions *system, Mem_Options *mem, Editing_Layout *layout, File_View *view){
    Editing_File *file = view->file;
    view_undo_redo(system, mem, layout, view, file, &file->state.undo.undo, ED_UNDO);
}

inline void
view_redo(System_Functions *system, Mem_Options *mem, Editing_Layout *layout, File_View *view){
    Editing_File *file = view->file;
    view_undo_redo(system, mem, layout, view, file, &file->state.undo.redo, ED_REDO);
}

inline u8*
write_data(u8 *ptr, void *x, i32 size){
    memcpy(ptr, x, size);
    return (ptr + size);
}

#define UseFileHistoryDump 0

#if UseFileHistoryDump
internal void
file_dump_history(System_Functions *system, Mem_Options *mem, Editing_File *file, char *filename){
    if (!file->state.undo.undo.edits) return;
    
    i32 size = 0;
    
    size += sizeof(i32);
    size += file->state.undo.undo.edit_count*sizeof(Edit_Step);
    size += sizeof(i32);
    size += file->state.undo.redo.edit_count*sizeof(Edit_Step);
    size += sizeof(i32);
    size += file->state.undo.history.edit_count*sizeof(Edit_Step);
    size += sizeof(i32);
    size += file->state.undo.children.edit_count*sizeof(Buffer_Edit);
    
    size += sizeof(i32);
    size += file->state.undo.undo.size;
    size += sizeof(i32);
    size += file->state.undo.redo.size;
    size += sizeof(i32);
    size += file->state.undo.history.size;
    size += sizeof(i32);
    size += file->state.undo.children.size;

    Partition *part = &mem->part;
    i32 remaining = partition_remaining(part);
    if (size < remaining){
        u8 *data, *curs;
        data = (u8*)part->base + part->pos;
        curs = data;
        curs = write_data(curs, &file->state.undo.undo.edit_count, 4);
        curs = write_data(curs, &file->state.undo.redo.edit_count, 4);
        curs = write_data(curs, &file->state.undo.history.edit_count, 4);
        curs = write_data(curs, &file->state.undo.children.edit_count, 4);
        curs = write_data(curs, &file->state.undo.undo.size, 4);
        curs = write_data(curs, &file->state.undo.redo.size, 4);
        curs = write_data(curs, &file->state.undo.history.size, 4);
        curs = write_data(curs, &file->state.undo.children.size, 4);
        
        curs = write_data(curs, file->state.undo.undo.edits, sizeof(Edit_Step)*file->state.undo.undo.edit_count);
        curs = write_data(curs, file->state.undo.redo.edits, sizeof(Edit_Step)*file->state.undo.redo.edit_count);
        curs = write_data(curs, file->state.undo.history.edits, sizeof(Edit_Step)*file->state.undo.history.edit_count);
        curs = write_data(curs, file->state.undo.children.edits, sizeof(Buffer_Edit)*file->state.undo.children.edit_count);
        
        curs = write_data(curs, file->state.undo.undo.strings, file->state.undo.undo.size);
        curs = write_data(curs, file->state.undo.redo.strings, file->state.undo.redo.size);
        curs = write_data(curs, file->state.undo.history.strings, file->state.undo.history.size);
        curs = write_data(curs, file->state.undo.children.strings, file->state.undo.children.size);

        Assert((i32)(curs - data) == size);
        system->save_file(filename, data, size);
    }
}
#endif

internal void
view_history_step(System_Functions *system, Mem_Options *mem, Editing_Layout *layout, File_View *view, History_Mode history_mode){
    if (view->locked) return;
    Assert(history_mode != hist_normal);
    
    Editing_File *file = view->file;
    
    b32 do_history_step = 0;
    Edit_Step step = {};
    if (history_mode == hist_backward){
        if (file->state.undo.edit_history_cursor > 0){
            do_history_step = 1;
            step = file->state.undo.history.edits[--file->state.undo.edit_history_cursor];
        }
    }
    else{
        if (file->state.undo.edit_history_cursor < file->state.undo.history.edit_count){
            Assert(((file->state.undo.history.edit_count - file->state.undo.edit_history_cursor) & 1) == 0);
            step = file->state.undo.history.edits[--file->state.undo.history.edit_count];
            file->state.undo.history.size -= step.edit.len;
            ++file->state.undo.edit_history_cursor;
            do_history_step = 1;
        }
    }
    
    if (do_history_step){
        Edit_Spec spec;
        spec.step = step;
        
        if (spec.step.child_count == 0){
            spec.step.edit.str_start = 0;
            spec.str = file->state.undo.history.strings + step.edit.str_start;
            
            file_do_single_edit(system, mem, file, layout, spec, history_mode);
        
            switch (spec.step.type){
            case ED_NORMAL:
            case ED_REDO:
                view_cursor_move(view, step.post_pos);
                break;
            
            case ED_REVERSE_NORMAL:
            case ED_UNDO:
                view_cursor_move(view, step.pre_pos);
                break;
            }
            view->mark = view->cursor.pos;
        }
        else{
            TentativeAssert(spec.step.special_type == 1);
            view_do_white_batch_edit(system, mem, view, file, layout, spec, history_mode);
        }
    }
}

// TODO(allen): write these as streamed operations
internal i32
view_find_end_of_line(File_View *view, i32 pos){
#if BUFFER_EXPERIMENT_SCALPEL <= 0
	Editing_File *file = view->file;
	char *data = file->state.buffer.data;
	while (pos < file->state.buffer.size && data[pos] != '\n') ++pos;
	if (pos > file->state.buffer.size) pos = file->state.buffer.size;
#endif
	return pos;
}

internal i32
view_find_beginning_of_line(File_View *view, i32 pos){
#if BUFFER_EXPERIMENT_SCALPEL <= 0
	Editing_File *file = view->file;
	char *data = file->state.buffer.data;
	if (pos > 0){
		--pos;
		while (pos > 0 && data[pos] != '\n') --pos;
		if (pos != 0) ++pos;
	}
#endif
	return pos;
}

internal i32
view_find_beginning_of_next_line(File_View *view, i32 pos){
#if BUFFER_EXPERIMENT_SCALPEL <= 0
	Editing_File *file = view->file;
	char *data = file->state.buffer.data;
	while (pos < file->state.buffer.size &&
		   !starts_new_line(data[pos])){
		++pos;
	}
	if (pos < file->state.buffer.size){
		++pos;
	}
#endif
	return pos;
}

internal String*
working_set_next_clipboard_string(General_Memory *general, Working_Set *working, i32 str_size){
	String *result = 0;
	i32 clipboard_current = working->clipboard_current;
	if (working->clipboard_size == 0){
		clipboard_current = 0;
		working->clipboard_size = 1;
	}
	else{
		++clipboard_current;
		if (clipboard_current >= working->clipboard_max_size){
			clipboard_current = 0;
		}
		else if (working->clipboard_size <= clipboard_current){
			working->clipboard_size = clipboard_current+1;
		}
	}
	result = &working->clipboards[clipboard_current];
	working->clipboard_current = clipboard_current;
	working->clipboard_rolling = clipboard_current;
    char *new_str;
	if (result->str){
        new_str = (char*)general_memory_reallocate(general, result->str, result->size, str_size);
    }
    else{
        new_str = (char*)general_memory_allocate(general, str_size+1);
    }
    // TODO(allen): What if new_str == 0?
    *result = make_string(new_str, 0, str_size);
	return result;
}

internal String*
working_set_clipboard_head(Working_Set *working){
	String *result = 0;
	if (working->clipboard_size > 0){
		i32 clipboard_index = working->clipboard_current;
		working->clipboard_rolling = clipboard_index;
		result = &working->clipboards[clipboard_index];
	}
	return result;
}

internal String*
working_set_clipboard_roll_down(Working_Set *working){
	String *result = 0;
	if (working->clipboard_size > 0){
		i32 clipboard_index = working->clipboard_rolling;
		--clipboard_index;
		if (clipboard_index < 0){
			clipboard_index = working->clipboard_size-1;
		}
		working->clipboard_rolling = clipboard_index;
		result = &working->clipboards[clipboard_index];
	}
	return result;
}

inline Editing_File*
working_set_contains(Working_Set *working, String filename){
    Editing_File *result = 0;
    i32 id;
    if (table_find(&working->table, filename, &id)){
        if (id < working->file_max_count){
            result = working->files + id;
        }
    }
    return result;
}

// TODO(allen): Find a way to choose an ordering for these so it picks better first options.
internal Editing_File*
working_set_lookup_file(Working_Set *working_set, String string){
	Editing_File *file = working_set_contains(working_set, string);
	
	if (!file){
        i32 file_i;
        i32 end = working_set->file_index_count;
        file = working_set->files;
		for (file_i = 0; file_i < end; ++file_i, ++file){
			if (file->name.live_name.str &&
                (string.size == 0 || has_substr(file->name.live_name, string))){
				break;
			}
		}
        if (file_i == end) file = 0;
	}
    
	return file;
}

internal void
clipboard_copy(System_Functions *system, General_Memory *general, Working_Set *working, Range range, Editing_File *file){
#if BUFFER_EXPERIMENT_SCALPEL <= 3
    i32 size = range.end - range.start;
    String *dest = working_set_next_clipboard_string(general, working, size);
    buffer_stringify(&file->state.buffer, range.start, range.end, dest->str);
    dest->size = size;
    system->post_clipboard(*dest);
#endif
}

internal Edit_Spec
file_compute_whitespace_edit(Mem_Options *mem, Editing_File *file, i32 cursor_pos,
                             Buffer_Edit *edits, char *str_base, i32 str_size,
                             Buffer_Edit *inverse_array, char *inv_str, i32 inv_max,
                             i32 edit_count){
#if BUFFER_EXPERIMENT_SCALPEL <= 3
    General_Memory *general = &mem->general;
    
    i32 inv_str_pos = 0;
    Buffer_Invert_Batch state = {};
    if (buffer_invert_batch(&state, &file->state.buffer, edits, edit_count,
                            inverse_array, inv_str, &inv_str_pos, inv_max))
        Assert(0);
    
    i32 first_child =
        undo_children_push(general, &file->state.undo.children,
                           edits, edit_count, (u8*)(str_base), str_size);
    i32 inverse_first_child =
        undo_children_push(general, &file->state.undo.children,
                           inverse_array, edit_count, (u8*)(inv_str), inv_str_pos);
    
    Edit_Spec spec = {};
    spec.step.type = ED_NORMAL;
    spec.step.first_child = first_child;
    spec.step.inverse_first_child = inverse_first_child;
    spec.step.special_type = 1;
    spec.step.child_count = edit_count;
    spec.step.inverse_child_count = edit_count;
    spec.step.pre_pos = cursor_pos;
    spec.step.post_pos = cursor_pos;
#else
    Edit_Spec spec = {};
#endif
    
    return spec;
}

internal void
view_clean_whitespace(System_Functions *system, Mem_Options *mem, File_View *view, Editing_Layout *layout){
#if BUFFER_EXPERIMENT_SCALPEL <= 3
    Editing_File *file = view->file;
    Assert(file && !file->state.is_dummy);
    Partition *part = &mem->part;
    i32 line_count = file->state.buffer.line_count;
    i32 edit_max = line_count * 2;
    i32 edit_count = 0;
    
    Temp_Memory temp = begin_temp_memory(part);
    Buffer_Edit *edits = push_array(part, Buffer_Edit, edit_max);
    
    char *str_base = (char*)part->base + part->pos;
    i32 str_size = 0;
    for (i32 line_i = 0; line_i < line_count; ++line_i){
        i32 start = file->state.buffer.line_starts[line_i];
        i32 preferred_indentation;
        b32 all_whitespace = 0;
        b32 all_space = 0;
        i32 hard_start =
            buffer_find_hard_start(&file->state.buffer, start, &all_whitespace, &all_space,
                                   &preferred_indentation, 4);
        
        if (all_whitespace) preferred_indentation = 0;
        
        if ((all_whitespace && hard_start > start) || !all_space){
            Buffer_Edit new_edit;
            new_edit.str_start = str_size;
            str_size += preferred_indentation;
            char *str = push_array(part, char, preferred_indentation);
            for (i32 j = 0; j < preferred_indentation; ++j) str[j] = ' ';
            new_edit.len = preferred_indentation;
            new_edit.start = start;
            new_edit.end = hard_start;
            edits[edit_count++] = new_edit;
        }
        Assert(edit_count <= edit_max);
    }
    
    if (edit_count > 0){
        Assert(buffer_batch_debug_sort_check(edits, edit_count));
    
        // NOTE(allen): computing edit spec, doing batch edit
        Buffer_Edit *inverse_array = push_array(part, Buffer_Edit, edit_count);
        Assert(inverse_array);
    
        char *inv_str = (char*)part->base + part->pos;
        Edit_Spec spec =
            file_compute_whitespace_edit(mem, file, view->cursor.pos, edits, str_base, str_size,
                                         inverse_array, inv_str, part->max - part->pos, edit_count);
    
        view_do_white_batch_edit(system, mem, view, file, layout, spec, hist_normal);
    }
    
    end_temp_memory(temp);
#endif
}

internal void
view_auto_tab_tokens(System_Functions *system,
                     Mem_Options *mem, File_View *view, Editing_Layout *layout,
                     i32 start, i32 end, b32 empty_blank_lines){
#if BUFFER_EXPERIMENT_SCALPEL <= 0
    Editing_File *file = view->file;
    Assert(file && !file->state.is_dummy);
    Partition *part = &mem->part;
    Buffer *buffer = &file->state.buffer;
    Cpp_Token_Stack tokens = file->state.token_stack;
    Assert(tokens.tokens);
    
    i32 line_start = buffer_get_line_index(buffer, start);
    i32 line_end = buffer_get_line_index(buffer, end) + 1;
    
    i32 edit_max = (line_end - line_start) * 2;
    i32 edit_count = 0;
    
    i32 indent_mark_count = line_end - line_start;
    
    Temp_Memory temp = begin_temp_memory(part);
    i32 *indent_marks = push_array(part, i32, indent_mark_count);
    {
        i32 current_indent = 0;
        i32 token_i;
        Cpp_Token *token, *self_token;
        
        {
            i32 start_pos = file->state.buffer.line_starts[line_start];
            Cpp_Get_Token_Result result = cpp_get_token(&tokens, start_pos);
            token_i = result.token_index;
            if (result.in_whitespace) token_i += 1;
            self_token = tokens.tokens + token_i;
        }
        
        i32 line = line_start - 1;
        for (; line >= 0; --line){
            i32 start = file->state.buffer.line_starts[line];
            b32 all_whitespace = 0;
            b32 all_space = 0;
            buffer_find_hard_start(&file->state.buffer, start,
                                   &all_whitespace, &all_space, &current_indent, 4);
            if (!all_whitespace) break;
        }
        
        if (line < 0){
            token_i = 0;
            token = tokens.tokens + token_i;
        }
        else{
            i32 start_pos = file->state.buffer.line_starts[line];
            Cpp_Get_Token_Result result = cpp_get_token(&tokens, start_pos);
            token_i = result.token_index;
            if (result.in_whitespace) token_i += 1;
            token = tokens.tokens + token_i;
            
            while (token >= tokens.tokens &&
                   token->flags & CPP_TFLAG_PP_DIRECTIVE ||
                   token->flags & CPP_TFLAG_PP_BODY){
                --token;
            }
            
            if (token < tokens.tokens){
                ++token;
                current_indent = 0;
            }
            else if (token->start < start_pos){
                line = buffer_get_line_index(&file->state.buffer, token->start);
                i32 start = file->state.buffer.line_starts[line];
                b32 all_whitespace = 0;
                b32 all_space = 0;
                buffer_find_hard_start(&file->state.buffer, start,
                                       &all_whitespace, &all_space, &current_indent, 4);
                Assert(!all_whitespace);
            }
        }
        
        indent_marks -= line_start;
        i32 line_i = line_start;
        i32 next_line_start = file->state.buffer.line_starts[line_i];
        switch (token->type){
        case CPP_TOKEN_BRACKET_OPEN: current_indent += 4; break;
        case CPP_TOKEN_PARENTHESE_OPEN: current_indent += 4; break;
        case CPP_TOKEN_BRACE_OPEN: current_indent += 4; break;
        }
        
        Cpp_Token *prev_token = token;
        ++token;
        for (; line_i < line_end; ++token_i, ++token){
            for (; token->start >= next_line_start && line_i < line_end;){
                i32 this_line_start = next_line_start;
                next_line_start = file->state.buffer.line_starts[line_i+1];
                i32 this_indent;
                if (prev_token && prev_token->type == CPP_TOKEN_COMMENT &&
                    prev_token->start <= this_line_start && prev_token->start + prev_token->size > this_line_start){
                    this_indent = -1;
                }
                else{
                    this_indent = current_indent;
                    if (token->start < next_line_start){
                        if (token->flags & CPP_TFLAG_PP_DIRECTIVE) this_indent = 0;
                        else{
                            switch (token->type){
                            case CPP_TOKEN_BRACKET_CLOSE: this_indent -= 4; break;
                            case CPP_TOKEN_PARENTHESE_CLOSE: this_indent -= 4; break;
                            case CPP_TOKEN_BRACE_CLOSE: this_indent -= 4; break;
                            case CPP_TOKEN_BRACE_OPEN: break;
                            default:
                                if (current_indent > 0 && prev_token){
                                    if (!(prev_token->flags & CPP_TFLAG_PP_BODY ||
                                          prev_token->flags & CPP_TFLAG_PP_DIRECTIVE)){
                                        switch (prev_token->type){
                                        case CPP_TOKEN_BRACKET_OPEN: case CPP_TOKEN_PARENTHESE_OPEN:
                                        case CPP_TOKEN_BRACE_OPEN: case CPP_TOKEN_BRACE_CLOSE:
                                        case CPP_TOKEN_SEMICOLON: case CPP_TOKEN_COLON: break;
                                        case CPP_TOKEN_COMMA: case CPP_TOKEN_COMMENT: break;
                                        default: this_indent += 4;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if (this_indent < 0) this_indent = 0;
                }
                indent_marks[line_i] = this_indent;
                ++line_i;
            }
            
            switch (token->type){
            case CPP_TOKEN_BRACKET_OPEN: current_indent += 4; break;
            case CPP_TOKEN_BRACKET_CLOSE: current_indent -= 4; break;
            case CPP_TOKEN_PARENTHESE_OPEN: current_indent += 4; break;
            case CPP_TOKEN_PARENTHESE_CLOSE: current_indent -= 4; break;
            case CPP_TOKEN_BRACE_OPEN: current_indent += 4; break;
            case CPP_TOKEN_BRACE_CLOSE: current_indent -= 4; break;
            }
            prev_token = token;
        }
    }
    
    Buffer_Edit *edits = push_array(part, Buffer_Edit, edit_max);
    
    char *str_base = (char*)part->base + part->pos;
    i32 str_size = 0;
    for (i32 line_i = line_start; line_i < line_end; ++line_i){
        i32 start = file->state.buffer.line_starts[line_i];
        i32 preferred_indentation;
        i32 correct_indentation;
        b32 all_whitespace = 0;
        b32 all_space = 0;
        i32 hard_start =
            buffer_find_hard_start(&file->state.buffer, start, &all_whitespace, &all_space,
                                   &preferred_indentation, 4);

        correct_indentation = indent_marks[line_i];
        if (all_whitespace && empty_blank_lines) correct_indentation = 0;
        if (correct_indentation == -1) correct_indentation = preferred_indentation;
        
        if ((all_whitespace && hard_start > start) || !all_space || correct_indentation != preferred_indentation){
            Buffer_Edit new_edit;
            new_edit.str_start = str_size;
            str_size += correct_indentation;
            char *str = push_array(part, char, correct_indentation);
            for (i32 j = 0; j < correct_indentation; ++j) str[j] = ' ';
            new_edit.len = correct_indentation;
            new_edit.start = start;
            new_edit.end = hard_start;
            edits[edit_count++] = new_edit;
        }
        
        Assert(edit_count <= edit_max);
    }
    
    if (edit_count > 0){
        Assert(buffer_batch_debug_sort_check(edits, edit_count));
    
        // NOTE(allen): computing edit spec, doing batch edit
        Buffer_Edit *inverse_array = push_array(part, Buffer_Edit, edit_count);
        Assert(inverse_array);
    
        char *inv_str = (char*)part->base + part->pos;
        Edit_Spec spec =
            file_compute_whitespace_edit(mem, file, view->cursor.pos, edits, str_base, str_size,
                                         inverse_array, inv_str, part->max - part->pos, edit_count);
        
        view_do_white_batch_edit(system, mem, view, file, layout, spec, hist_normal);
    }
    
    {
        b32 all_whitespace = 0;
        b32 all_space = 0;
        i32 preferred_indentation;
        i32 start = view->cursor.pos;
        i32 hard_start = buffer_find_hard_start(
            &file->state.buffer, start, &all_whitespace, &all_space,
            &preferred_indentation, 4);
        
        view_cursor_move(view, hard_start);
    }
    
    end_temp_memory(temp);
#endif
}

struct Get_Link_Result{
    b32 in_link;
    i32 index;
};

internal Get_Link_Result
get_link(Hyper_Link *links, i32 link_count, i32 pos){
    Get_Link_Result result = {};
    // TODO TODO TODO TODO TODO TODO TODO TODO
    return result;
}

internal u32*
style_get_link_color(Style *style, Link_Type type){
	u32 *result;
    switch (type){
    case link_result:
        result = &style->main.result_link_color;
        break;
        
    case link_related:
        result = &style->main.related_link_color;
        break;

    case link_error:
        result = &style->main.error_link_color;
        break;
        
    case link_warning:
        result = &style->main.warning_link_color;
        break;
        
    default:
        result = &style->main.default_color;
    }
    return result;
}

internal u32*
style_get_color(Style *style, Cpp_Token token){
	u32 *result;
    if (token.flags & CPP_TFLAG_IS_KEYWORD){
        if (token.type == CPP_TOKEN_BOOLEAN_CONSTANT){
			result = &style->main.bool_constant_color;
        }
        else{
            result = &style->main.keyword_color;
        }
    }
    else if(token.flags & CPP_TFLAG_PP_DIRECTIVE){
        result = &style->main.preproc_color;
    }
    else{
        switch (token.type){
		case CPP_TOKEN_COMMENT:
			result = &style->main.comment_color;
			break;
            
		case CPP_TOKEN_STRING_CONSTANT:
			result = &style->main.str_constant_color;
            break;
            
		case CPP_TOKEN_CHARACTER_CONSTANT:
			result = &style->main.char_constant_color;
            break;
            
		case CPP_TOKEN_INTEGER_CONSTANT:
			result = &style->main.int_constant_color;
            break;
            
		case CPP_TOKEN_FLOATING_CONSTANT:
			result = &style->main.float_constant_color;
            break;
            
		case CPP_TOKEN_INCLUDE_FILE:
			result = &style->main.include_color;
            break;
            
		default:
			result = &style->main.default_color;
        }
    }
	return result;
}

internal bool32
smooth_camera_step(real32 *target, real32 *current, real32 *vel, real32 S, real32 T){
    bool32 result = 0;
    real32 targ = *target;
    real32 curr = *current;
    real32 v = *vel;
    if (curr != targ){
        if (curr > targ - .1f && curr < targ + .1f){
            curr = targ;
            v = 1.f;
        }
        else{
            real32 L = lerp(curr, T, targ);
            
            i32 sign = (targ > curr) - (targ < curr);
            real32 V = curr + sign*v;
            
            if (sign > 0) curr = Min(L, V);
            else curr = Max(L, V);
            
            if (curr == V){
                v *= S;
            }
        }
        
        *target = targ;
        *current = curr;
        *vel = v;
        result = 1;
    }
    return result;
}

inline real32
view_compute_max_target_y(i32 lowest_line, i32 line_height, real32 view_height){
    real32 max_target_y = ((lowest_line+.5f)*line_height) - view_height*.5f;
    return max_target_y;
}

internal real32
view_compute_max_target_y(File_View *view){
    i32 lowest_line = view_compute_lowest_line(view);
    i32 line_height = view->font_height;
    real32 view_height = view_compute_height(view);
    real32 max_target_y = view_compute_max_target_y(
        lowest_line, line_height, view_height);
    return max_target_y;
}

internal void
remeasure_file_view(System_Functions *system, View *view_, i32_Rect rect){
    File_View *view = (File_View*)view_;
    if (file_is_ready(view->file)){
        Relative_Scrolling relative = view_get_relative_scrolling(view);
        view_measure_wraps(system, &view->view_base.mem->general, view);
        view_cursor_move(view, view->cursor.pos);
        view->preferred_x = view_get_cursor_x(view);
        view_set_relative_scrolling(view, relative);
    }
}

internal i32
step_file_view(System_Functions *system, View *view_, i32_Rect rect,
               b32 is_active, Input_Summary *user_input){
    view_->mouse_cursor_type = APP_MOUSE_CURSOR_IBEAM;
    i32 result = 0;
    File_View *view = (File_View*)view_;
    Editing_File *file = view->file;

    if (file->state.is_loading){
        return result;
    }
    
    f32 line_height = (f32)view->font_height;
    f32 cursor_y = view_get_cursor_y(view);
    f32 target_y = view->target_y;
    f32 max_y = view_compute_height(view) - line_height*2;
    i32 lowest_line = view_compute_lowest_line(view);
    f32 max_target_y = view_compute_max_target_y(lowest_line, (i32)line_height, max_y);
    f32 delta_y = 3.f*line_height;
    f32 extra_top = 0.f;
    extra_top += view_widget_height(view, (i32)line_height);
    f32 taken_top_space = line_height + extra_top;
    
    if (user_input->mouse.my < rect.y0 + taken_top_space){
        view_->mouse_cursor_type = APP_MOUSE_CURSOR_ARROW;
    }
    else{
        view_->mouse_cursor_type = APP_MOUSE_CURSOR_IBEAM;
    }
    
    if (user_input->mouse.wheel_used){
        real32 wheel_multiplier = 3.f;
        real32 delta_target_y = delta_y*user_input->mouse.wheel_amount*wheel_multiplier;
        target_y += delta_target_y;
        
        if (target_y < -taken_top_space) target_y = -taken_top_space;
        if (target_y > max_target_y) target_y = max_target_y;
        
        real32 old_cursor_y = cursor_y;
        if (cursor_y >= target_y + max_y) cursor_y = target_y + max_y;
        if (cursor_y < target_y + taken_top_space) cursor_y = target_y + taken_top_space;
        
        if (cursor_y != old_cursor_y){
            view->cursor =
                view_compute_cursor_from_xy(view,
                                            view->preferred_x,
                                            cursor_y);
        }
        
        result = 1;
    }
    
    if (cursor_y > target_y + max_y){
        target_y = cursor_y - max_y + delta_y;
    }
    if (cursor_y < target_y + taken_top_space){
        target_y = cursor_y - delta_y - taken_top_space;
    }
    
    if (target_y > max_target_y) target_y = max_target_y;
    if (target_y < -extra_top) target_y = -extra_top;
    view->target_y = target_y;
    
    f32 cursor_x = view_get_cursor_x(view);
    f32 target_x = view->target_x;
    f32 max_x = view_compute_width(view);
    if (cursor_x < target_x){
        target_x = (f32)Max(0, cursor_x - max_x/2);
    }
    else if (cursor_x >= target_x + max_x){
        target_x = (f32)(cursor_x - max_x/2);
    }
    
    view->target_x = target_x;
    
    if (smooth_camera_step(&view->target_y, &view->scroll_y, &view->vel_y, 40.f, 1.f/4.f)){
        result = 1;
    }
    if (smooth_camera_step(&view->target_x, &view->scroll_x, &view->vel_x, 40.f, 1.f/4.f)){
        result = 1;
    }
    if (file->state.paste_effect.tick_down > 0){
        --file->state.paste_effect.tick_down;
        result = 1;
    }
    
    if (is_active && user_input->mouse.press_l){
        f32 max_y = view_compute_height(view);
        f32 rx = (f32)(user_input->mouse.mx - rect.x0);
        f32 ry = (f32)(user_input->mouse.my - rect.y0 - line_height - 2);
        
        if (ry >= extra_top){
            view_set_widget(view, FWIDG_NONE);
            if (rx >= 0 && rx < max_x && ry >= 0 && ry < max_y){
                view_cursor_move(view, rx + view->scroll_x, ry + view->scroll_y, 1);
                view->mode = {};
            }
        }
        result = 1;
    }
    
    if (!is_active) view_set_widget(view, FWIDG_NONE);

    // NOTE(allen): framely undo stuff
    if (file){
        i32 scrub_max = view->scrub_max;
        i32 undo_count, redo_count, total_count;
        undo_count = file->state.undo.undo.edit_count;
        redo_count = file->state.undo.redo.edit_count;
        total_count = undo_count + redo_count;

        switch (view->widget.type){
        case FWIDG_TIMELINES:
        {
            i32_Rect widg_rect = view_widget_rect(view, view->font_height);
            
            UI_State state = 
                ui_state_init(&view->widget.state, 0, user_input,
                              view->style, view->font_set, 0, 1);
            
            UI_Layout layout;
            begin_layout(&layout, widg_rect);
            
            if (view->widget.timeline.undo_line){
                if (do_button(1, &state, &layout, "- Undo", 1)){
                    view->widget.timeline.undo_line = 0;
                }
                
                if (view->widget.timeline.undo_line){
                    Widget_ID wid = make_id(&state, 2);
                    i32 new_count;
                    if (do_undo_slider(wid, &state, &layout, total_count, undo_count, 0, &new_count)){
                        view->rewind_speed = 0;
                        view->rewind_amount = 0;
                        
                        for (i32 i = 0; i < scrub_max && new_count < undo_count; ++i){
                            view_undo(system, view_->mem, view->layout, view);
                            --undo_count;
                        }
                        for (i32 i = 0; i < scrub_max && new_count > undo_count; ++i){
                            view_redo(system, view_->mem, view->layout, view);
                            ++undo_count;
                        }
                    }
                }
            }
            else{
                if (do_button(1, &state, &layout, "+ Undo", 1)){
                    view->widget.timeline.undo_line = 1;
                }
            }
            
            if (view->widget.timeline.history_line){
                if (do_button(3, &state, &layout, "- History", 1)){
                    view->widget.timeline.history_line = 0;
                }
                
                Widget_ID wid = make_id(&state, 4);
                if (view->widget.timeline.history_line){
                    i32 new_count;
                    i32 mid = ((file->state.undo.history.edit_count + file->state.undo.edit_history_cursor) >> 1);
                    i32 count = file->state.undo.edit_history_cursor;
                    if (do_undo_slider(wid, &state, &layout, mid, count, &file->state.undo, &new_count)){
                        for (i32 i = 0; i < scrub_max && new_count < count; ++i){
                            view_history_step(system, view_->mem, view->layout, view, hist_backward);
                        }
                        for (i32 i = 0; i < scrub_max && new_count > count; ++i){
                            view_history_step(system, view_->mem, view->layout, view, hist_forward);
                        }
                    }
                }
            }
            else{
                if (do_button(3, &state, &layout, "+ History", 1)){
                    view->widget.timeline.history_line = 1;
                }
            }
            
            view->widget.height = layout.y - widg_rect.y0;
            
            if (ui_finish_frame(&view->widget.state, &state, &layout, widg_rect, 0, 0)){
                result = 1;
            }
        }break;
        }
        
        i32 rewind_max = view->rewind_max;
        view->rewind_amount += view->rewind_speed;
        for (i32 i = 0; view->rewind_amount <= -1.f; ++i, view->rewind_amount += 1.f){
            if (i < rewind_max) view_undo(system, view_->mem, view->layout, view);
        }
        
        for (i32 i = 0; view->rewind_amount >= 1.f; ++i, view->rewind_amount -= 1.f){
            if (i < rewind_max) view_redo(system, view_->mem, view->layout, view);
        }
        
        if (view->rewind_speed < 0.f && undo_count == 0){
            view->rewind_speed = 0.f;
            view->rewind_amount = 0.f;
        }
        
        if (view->rewind_speed > 0.f && redo_count == 0){
            view->rewind_speed = 0.f;
            view->rewind_amount = 0.f;
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

internal void
draw_file_setup_bar(Style *style, i32 line_height, Interactive_Bar *bar, i32_Rect *rect){
    bar->style = style->main.file_info_style;
    bar->font_id = style->font_id;
    bar->pos_x = (f32)rect->x0;
    bar->pos_y = (f32)rect->y0;
    bar->text_shift_y = 2;
    bar->text_shift_x = 0;
    bar->rect = *rect;
    bar->rect.y1 = bar->rect.y0 + line_height + 2;
    rect->y0 += line_height + 2;
}

internal void
draw_file_bar(File_View *view, Interactive_Bar *bar, Render_Target *target){
    Editing_File *file = view->file;
    
    u32 back_color = bar->style.bar_color;
    u32 base_color = bar->style.base_color;
    u32 pop2_color = bar->style.pop2_color;
    
    draw_rectangle(target, bar->rect, back_color);    
    intbar_draw_string(target, bar, file->name.live_name, base_color);
    intbar_draw_string(target, bar, make_lit_string(" - "), base_color);

    if (file->state.is_loading){
        intbar_draw_string(target, bar, make_lit_string(" loading"), base_color);
    }
    else{
        char line_number_space[30];
        String line_number = make_string(line_number_space, 0, 30);
        append(&line_number, "L#");
        append_int_to_str(view->cursor.line, &line_number);
        
        intbar_draw_string(target, bar, line_number, base_color);

        if (!file->settings.unimportant){
            switch (buffer_get_sync(file)){
                case SYNC_BEHIND_OS:
                {
                    persist String out_of_sync = make_lit_string(" !");
                    intbar_draw_string(target, bar, out_of_sync, pop2_color);
                }break;

                case SYNC_UNSAVED:
                {
                    persist String out_of_sync = make_lit_string(" *");
                    intbar_draw_string(target, bar, out_of_sync, pop2_color);
                }break;
            }
        }
    }
}

internal i32
draw_file_loaded(File_View *view, i32_Rect rect, b32 is_active, Render_Target *target){
    Editing_File *file = view->file;
    Style *style = view->style;
    i32 line_height = view->font_height;

    Interactive_Bar bar;
    draw_file_setup_bar(style, line_height, &bar, &rect);
    
#if BUFFER_EXPERIMENT_SCALPEL <= 3
    i32 max_x = rect.x1 - rect.x0;
    i32 max_y = rect.y1 - rect.y0 + line_height;
    
    Assert(file && !file->state.is_dummy && buffer_good(&file->state.buffer));
    
    b32 tokens_use = 0;
    Cpp_Token_Stack token_stack = {};
    if (file){
        tokens_use = file->state.tokens_complete && (file->state.token_stack.count > 0);
        token_stack = file->state.token_stack;
    }
    
    b32 links_use = 0;
    Hyper_Link *links = 0;
    i32 link_count = 0;
    if (view->links){
        if (view->link_count > 0){
            links_use = 1;
            links = view->links;
            link_count = view->link_count;
        }
    }
    
    Partition *part = &view->view_base.mem->part;

    Temp_Memory temp = begin_temp_memory(part);
    
    partition_align(part, 4);
    i32 max = partition_remaining(part) / sizeof(Buffer_Render_Item);
    Buffer_Render_Item *items = push_array(part, Buffer_Render_Item, max);

    i16 font_id = style->font_id;
    Render_Font *font = get_font_info(view->font_set, font_id)->font;
    float *advance_data = 0;
    if (font) advance_data = font->advance_data;

    i32 count;
    Full_Cursor render_cursor;
    Buffer_Render_Options opts = {};

    f32 *wraps = view->line_wrap_y;
    f32 scroll_x = view->scroll_x;
    f32 scroll_y = view->scroll_y;
    
    {
        render_cursor = buffer_get_start_cursor(&file->state.buffer, wraps, scroll_y,
                                                !view->unwrapped_lines, (f32)max_x, advance_data, (f32)line_height);
        
        view->scroll_i = render_cursor.pos;
        
        buffer_get_render_data(&file->state.buffer, items, max, &count,
                               (f32)rect.x0, (f32)rect.y0,
                               scroll_x, scroll_y, render_cursor,
                               !view->unwrapped_lines,
                               (f32)max_x, (f32)max_y,
                               advance_data, (f32)line_height,
                               opts);
    }
    
    Assert(count > 0);
    
    i32 cursor_begin, cursor_end;
    u32 cursor_color, at_cursor_color;
    if (view->show_temp_highlight){
        cursor_begin = view->temp_highlight.pos;
        cursor_end = view->temp_highlight_end_pos;
        cursor_color = style->main.highlight_color;
        at_cursor_color = style->main.at_highlight_color;
    }
    else{
        cursor_begin = view->cursor.pos;
        cursor_end = cursor_begin + 1;
        cursor_color = style->main.cursor_color;
        at_cursor_color = style->main.at_cursor_color;
    }
    
    i32 token_i = 0;
    i32 link_i = 0;
    u32 main_color = style->main.default_color;
    u32 special_color = style->main.special_character_color;
    u32 link_color = 0;
    if (tokens_use){
        Cpp_Get_Token_Result result = cpp_get_token(&token_stack, items->index);
        main_color = *style_get_color(style, token_stack.tokens[result.token_index]);
        token_i = result.token_index + 1;
    }
    if (links_use){
        Get_Link_Result result = get_link(links, link_count, items->index);
        if (result.in_link){
            link_color = *style_get_link_color(style, links[result.index].link_type);
        }
        link_i = result.index;
    }
    
    u32 mark_color = style->main.mark_color;
    Buffer_Render_Item *item = items;
    i32 prev_ind = -1;
    u32 highlight_color = 0;
    u32 highlight_this_color = 0;
    
    for (i32 i = 0; i < count; ++i, ++item){
        i32 ind = item->index;
        highlight_this_color = 0;
        if (tokens_use && ind != prev_ind){
            Cpp_Token current_token = token_stack.tokens[token_i-1];
            
            if (token_i < token_stack.count){
                if (ind >= token_stack.tokens[token_i].start){
                    main_color =
                        *style_get_color(style, token_stack.tokens[token_i]);
                    current_token = token_stack.tokens[token_i];
                    ++token_i;
                }
                else if (ind >= current_token.start + current_token.size){
                    main_color = 0xFFFFFFFF;
                }
            }

            if (current_token.type == CPP_TOKEN_JUNK &&
                i >= current_token.start && i < current_token.start + current_token.size){
                highlight_color = style->main.highlight_junk_color;
            }
            else{
                highlight_color = 0;
            }
        }
        
        u32 char_color = main_color;
        if (item->flags & BRFlag_Special_Character) char_color = special_color;

        f32_Rect char_rect = f32R(item->x0, item->y0, item->x1, item->y1);
        if (view->show_whitespace && highlight_color == 0 &&
            char_is_whitespace((char)item->glyphid)){
            highlight_this_color = style->main.highlight_white_color;
        }
        else{
            highlight_this_color = highlight_color;
        }
        
        if (cursor_begin <= ind && ind < cursor_end && (ind != prev_ind || cursor_begin < ind)){
            if (is_active){
                draw_rectangle(target, char_rect, cursor_color);
                char_color = at_cursor_color;
            }
            else{
                if (!view->show_temp_highlight){
                    draw_rectangle_outline(target, char_rect, cursor_color);
                }
            }
        }
        else if (highlight_this_color){
            draw_rectangle(target, char_rect, highlight_this_color);
        }
        
        u32 fade_color = 0xFFFF00FF;
        f32 fade_amount = 0.f;
        
        if (file->state.paste_effect.tick_down > 0 &&
            file->state.paste_effect.start <= ind &&
            ind < file->state.paste_effect.end){
            fade_color = file->state.paste_effect.color;
            fade_amount = (f32)(file->state.paste_effect.tick_down) / file->state.paste_effect.tick_max;
        }
        
        char_color = color_blend(char_color, fade_amount, fade_color);
        
        if (ind == view->mark && prev_ind != ind){
            draw_rectangle_outline(target, char_rect, mark_color);
        }
        if (item->glyphid != 0){
            font_draw_glyph(target, font_id, (u8)item->glyphid,
                            item->x0, item->y0, char_color);
        }
        prev_ind = ind;
    }
    
    end_temp_memory(temp);
#endif
    
    if (view->widget.type != FWIDG_NONE){
#if 0
        ui_render(target, view->gui_target);
#else
        UI_Style ui_style = get_ui_style_upper(style);
        
        i32_Rect widg_rect = view_widget_rect(view, view->font_height);
        
        draw_rectangle(target, widg_rect, ui_style.dark);
        draw_rectangle_outline(target, widg_rect, ui_style.dim);
        
        UI_State state =
            ui_state_init(&view->widget.state, target, 0,
                          view->style, view->font_set, 0, 0);
        
        UI_Layout layout;
        begin_layout(&layout, widg_rect);
        
        switch (view->widget.type){
        case FWIDG_TIMELINES:
        {
            Assert(file);
            if (view->widget.timeline.undo_line){
                do_button(1, &state, &layout, "- Undo", 1);
                
                Widget_ID wid = make_id(&state, 2);
                i32 undo_count, redo_count, total_count;
                undo_count = file->state.undo.undo.edit_count;
                redo_count = file->state.undo.redo.edit_count;
                total_count = undo_count + redo_count;
                do_undo_slider(wid, &state, &layout, total_count, undo_count, 0, 0);
            }
            else{
                do_button(1, &state, &layout, "+ Undo", 1);
            }
            
            if (view->widget.timeline.history_line){
                do_button(3, &state, &layout, "- History", 1);
                
                Widget_ID wid = make_id(&state, 4);
                i32 new_count;
                i32 mid = ((file->state.undo.history.edit_count + file->state.undo.edit_history_cursor) >> 1);
                do_undo_slider(wid, &state, &layout, mid,
                               file->state.undo.edit_history_cursor, &file->state.undo, &new_count);
            }
            else{
                do_button(3, &state, &layout, "+ History", 1);
            }
        }break;
        
        case FWIDG_SEARCH:
        {
            Widget_ID wid = make_id(&state, 1);
            persist String search_str = make_lit_string("I-Search: ");
            persist String rsearch_str = make_lit_string("Reverse-I-Search: ");
            if (view->isearch.reverse){
                do_text_field(wid, &state, &layout, rsearch_str, view->isearch.str);
            }
            else{
                do_text_field(wid, &state, &layout, search_str, view->isearch.str);
            }
        }break;
        
        case FWIDG_GOTO_LINE:
        {
            Widget_ID wid = make_id(&state, 1);
            persist String gotoline_str = make_lit_string("Goto Line: ");
            do_text_field(wid, &state, &layout, gotoline_str, view->isearch.str);
        }break;
        }
        
        ui_finish_frame(&view->widget.state, &state, &layout, widg_rect, 0, 0);
#endif
    }
    
    draw_file_bar(view, &bar, target);
    
    return(0);
}

internal i32
draw_file_loading(File_View *view, i32_Rect rect, b32 is_active, Render_Target *target){
    Interactive_Bar bar;
    draw_file_setup_bar(view->style, view->font_height, &bar, &rect);
    
    draw_file_bar(view, &bar, target);

    return(0);
}

internal i32
draw_file_view(View *view_, i32_Rect rect, bool32 is_active,
               Render_Target *target){
    File_View *view = (File_View*)view_;
    i32 result = 0;
    
    if (view->file){
        if (view->file->state.is_loading){
            result = draw_file_loading(view, rect, is_active, target);
        }
        else{
            result = draw_file_loaded(view, rect, is_active, target);
        }
    }
    
    return (result);
}

internal void
kill_file(System_Functions *system, Exchange *exchange,
          General_Memory *general, Editing_File *file, Live_Views *live_set, Editing_Layout *layout){
    i32 panel_count = layout->panel_count;
    Panel *panels = layout->panels, *panel;
    panel = panels;
    
    for (i32 i = 0; i < panel_count; ++i){
        View *view = panel->view;
        if (view){
            View *to_kill = view;
            if (view->is_minor) to_kill = view->major;
            File_View *fview = view_to_file_view(to_kill);
            if (fview && fview->file == file){
                live_set_free_view(system, exchange, live_set, &fview->view_base);
                if (to_kill == view) panel->view = 0;
                else view->major = 0;
            }
        }
        ++panel;
    }
    file_close(system, general, file);
    file_get_dummy(file);
}

internal void
command_search(System_Functions*,Command_Data*,Command_Binding);
internal void
command_reverse_search(System_Functions*,Command_Data*,Command_Binding);

internal
HANDLE_COMMAND_SIG(handle_command_file_view){
    File_View *file_view = (File_View*)(view);
    Editing_File *file = file_view->file;
    AllowLocal(file);
    
    switch (file_view->widget.type){
    case FWIDG_NONE:
    case FWIDG_TIMELINES:
    {
        file_view->next_mode = {};
        if (binding.function) binding.function(system, command, binding);
        file_view->mode = file_view->next_mode;
        
        if (key.keycode == key_esc)
            view_set_widget(file_view, FWIDG_NONE);
    }break;
    
    case FWIDG_SEARCH:
    {
#if BUFFER_EXPERIMENT_SCALPEL <= 3
        String *string = &file_view->isearch.str;
        Single_Line_Input_Step result =
            app_single_line_input_step(system, key, string);
        
        if (result.made_a_change ||
            binding.function == command_search ||
            binding.function == command_reverse_search){
            b32 step_forward = 0;
            b32 step_backward = 0;
            
            if (binding.function == command_search) step_forward = 1;
            if (binding.function == command_reverse_search) step_backward = 1;
            
            i32 start_pos = file_view->isearch.pos;
            if (step_forward){
                if (file_view->isearch.reverse){
                    start_pos = file_view->temp_highlight.pos + 1;
                    file_view->isearch.pos = start_pos;
                    file_view->isearch.reverse = 0;
                    step_forward = 0;
                }
            }
            if (step_backward){
                if (!file_view->isearch.reverse){
                    start_pos = file_view->temp_highlight.pos - 1;
                    file_view->isearch.pos = start_pos;
                    file_view->isearch.reverse = 1;
                    step_backward = 0;
                }
            }

            Temp_Memory temp = begin_temp_memory(&view->mem->part);
            char *spare = push_array(&view->mem->part, char, string->size);

            i32 size = buffer_size(&file->state.buffer);
            i32 pos;
            if (!result.hit_backspace){
                if (file_view->isearch.reverse){
                    pos = buffer_rfind_string(&file->state.buffer, start_pos - 1,
                                              string->str, string->size, spare);
                    if (pos >= 0){
                        if (step_backward){
                            file_view->isearch.pos = pos;
                            start_pos = pos;
                            pos = buffer_rfind_string(&file->state.buffer, start_pos - 1,
                                                      string->str, string->size, spare);
                            if (pos == -1) pos = start_pos;
                        }
                        view_set_temp_highlight(file_view, pos, pos+string->size);
                    }
                }
                
                else{
                    pos = buffer_find_string(&file->state.buffer, start_pos + 1, size,
                                             string->str, string->size, spare);
                    if (pos < size){
                        if (step_forward){
                            file_view->isearch.pos = pos;
                            start_pos = pos;
                            pos = buffer_find_string(&file->state.buffer, start_pos + 1, size,
                                                     string->str, string->size, spare);
                            if (pos == size) pos = start_pos;
                        }
                        view_set_temp_highlight(file_view, pos, pos+string->size);
                    }
                }
            }
            
            end_temp_memory(temp);
        }
        
        if (result.hit_newline || result.hit_ctrl_newline){
            view_cursor_move(file_view, file_view->temp_highlight);
            view_set_widget(file_view, FWIDG_NONE);
        }
        
        if (result.hit_esc){
            file_view->show_temp_highlight = 0;
            view_set_widget(file_view, FWIDG_NONE);
        }
#endif
    }break;
    
    case FWIDG_GOTO_LINE:
    {
        String *string = &file_view->gotoline.str;
        Single_Line_Input_Step result =
            app_single_number_input_step(system, key, string);
        
        if (result.hit_newline || result.hit_ctrl_newline){
            i32 line_number = str_to_int(*string);
            if (line_number < 1) line_number = 1;
            file_view->cursor =
                view_compute_cursor_from_unwrapped_xy(file_view, 0,
                                                      (f32)(line_number-1)*file_view->font_height);
            file_view->preferred_x = view_get_cursor_x(file_view);
            
            view_set_widget(file_view, FWIDG_NONE);
        }
        
        if (result.hit_esc){
            view_set_widget(file_view, FWIDG_NONE);
        }
    }break;
    }
}

inline void
free_file_view(View *view){
    File_View *fview = (File_View*)view;
    general_memory_free(&view->mem->general, fview->line_wrap_y);
    if (fview->links)
        general_memory_free(&view->mem->general, fview->links);
}

internal
Do_View_Sig(do_file_view){
    i32 result = 0;
    switch (message){
    case VMSG_RESIZE:
    case VMSG_STYLE_CHANGE:
    {
        remeasure_file_view(system, view, rect);
    }break;
    case VMSG_DRAW:
    {
        result = draw_file_view(view, rect, (view == active), target);
    }break;
    case VMSG_STEP:
    {
        result = step_file_view(system, view, rect, (view == active), user_input);
    }break;
    case VMSG_FREE:
    {
        free_file_view(view);
    }break;
    }
    return result;
}

internal File_View*
file_view_init(View *view, Editing_Layout *layout){
    view->type = VIEW_TYPE_FILE;
    view->do_view = do_file_view;
    view->handle_command = handle_command_file_view;
    
    File_View *result = (File_View*)view;
    result->layout = layout;
    result->rewind_max = 4;
    result->scrub_max = 1;
    return result;
}

struct File_View_Iter{
    File_View *view;
    
    Editing_File *file;
    File_View *skip;
    Panel *panels;
    i32 panel_count;
    i32 i;
};

internal File_View_Iter
file_view_iter_next(File_View_Iter iter){
    Panel *panel;
    View *view;
    File_View *file_view;

    ++iter.i;
    for (panel = iter.panels + iter.i;
         iter.i < iter.panel_count;
         ++iter.i, ++panel){
        view = panel->view;
        file_view = view_to_file_view(view);
        if (file_view && file_view != iter.skip && file_view->file == iter.file){
            iter.view = file_view;
            break;
        }
    }
    
    return(iter);
}

internal File_View_Iter
file_view_iter_init(Editing_Layout *layout, Editing_File *file, File_View *skip){
    File_View_Iter result = {};
    result.panels = layout->panels;
    result.panel_count = layout->panel_count;
    result.file = file;
    result.skip = skip;
    
    result.i = -1;
    result = file_view_iter_next(result);
    
    return(result);
}

internal b32
file_view_iter_good(File_View_Iter iter){
    b32 result = 1;
    if (iter.i >= iter.panel_count) result = 0;
    return(result);
}

struct Search_Range{
    Buffer_Type *buffer;
    i32 start, size;
};

struct Search_Set{
    Search_Range *ranges;
    i32 count, max;
};

struct Search_Iter{
    String word;
    i32 pos;
    i32 i;
};

struct Search_Match{
    Buffer_Type *buffer;
    i32 start, end;
    b32 found_match;
};

internal void
search_iter_init(General_Memory *general, Search_Iter *iter, i32 size){
    i32 str_max;
    
    if (iter->word.str == 0){
        str_max = size*2;
        iter->word.str = (char*)general_memory_allocate(general, str_max, 0);
        iter->word.memory_size = str_max;
    }
    else if (iter->word.memory_size < size){
        str_max = size*2;
        iter->word.str = (char*)general_memory_reallocate_nocopy(general, iter->word.str, str_max, 0);
        iter->word.memory_size = str_max;
    }
    
    iter->i = 0;
    iter->pos = 0;
}

internal void
search_set_init(General_Memory *general, Search_Set *set, i32 set_count){
    i32 max;
    
    if (set->ranges == 0){
        max = set_count*2;
        set->ranges = (Search_Range*)general_memory_allocate(general, sizeof(Search_Range)*max, 0);
        set->max = max;
    }
    else if (set->max < set_count){
        max = set_count*2;
        set->ranges = (Search_Range*)general_memory_reallocate_nocopy(
            general, set->ranges, sizeof(Search_Range)*max, 0);
        set->max = max;
    }
    
    set->count = set_count;
}

internal void
search_hits_table_alloc(General_Memory *general, Table *hits, i32 table_size){
    i32 hash_size, mem_size;
    
    hash_size = table_size * sizeof(u32);
    hash_size = (hash_size + 7) & ~7;
    mem_size = hash_size + table_size * sizeof(Offset_String);

    hits->hash_array = (u32*)general_memory_allocate(general, mem_size, 0);
    hits->data_array = (u8*)hits->hash_array + hash_size;
    hits->max = table_size;

    hits->item_size = sizeof(Offset_String);
}

internal void
search_hits_init(General_Memory *general, Table *hits, String_Space *str, i32 table_size, i32 str_size){
    i32 hash_size, mem_size;
    
    if (hits->hash_array == 0){
        search_hits_table_alloc(general, hits, table_size);
    }
    else if (hits->max < table_size){
        hash_size = table_size * sizeof(u32);
        hash_size = (hash_size + 7) & ~7;
        mem_size = hash_size + table_size * sizeof(Offset_String);
        
        hits->hash_array = (u32*)general_memory_reallocate_nocopy(
            general, hits->hash_array, mem_size, 0);
        hits->data_array = (u8*)hits->hash_array + hash_size;
        hits->max = table_size;
        
        hits->item_size = sizeof(Offset_String);
    }
    
    if (str->space == 0){
        str->space = (char*)general_memory_allocate(general, str_size, 0);
        str->max = str_size;
    }
    else if (str->max < str_size){
        str->space = (char*)general_memory_reallocate_nocopy(general, str->space, str_size, 0);
        str->max = str_size;
    }
    
    str->pos = str->new_pos = 0;
    table_clear(hits);
}

internal b32
search_hit_add(General_Memory *general, Table *hits, String_Space *space, char *str, i32 len){
    b32 result;
    i32 new_size;
    Offset_String ostring;
    Table new_hits;
    
    Assert(len != 0);
    
    ostring = strspace_append(space, str, len);
    if (ostring.size == 0){
        new_size = Max(space->max*2, space->max + len);
        space->space = (char*)general_memory_reallocate(general, space->space, space->new_pos, new_size, 0);
        ostring = strspace_append(space, str, len);
    }
    
    Assert(ostring.size != 0);
    
    if (table_at_capacity(hits)){
        search_hits_table_alloc(general, &new_hits, hits->max*2);
        table_clear(&new_hits);
        table_rehash(hits, &new_hits, space->space, tbl_offset_string_hash, tbl_offset_string_compare);
        general_memory_free(general, hits->hash_array);
        *hits = new_hits;
    }
    
    if (!table_add(hits, &ostring, space->space, tbl_offset_string_hash, tbl_offset_string_compare)){
        result = 1;
        strspace_keep_prev(space);
    }
    else{
        result = 0;
        strspace_discard_prev(space);
    }
    
    return(result);
}

internal Search_Match
search_next_match(Partition *part, Search_Set *set, Search_Iter *iter_){
    Search_Match result = {};
    Search_Iter iter = *iter_;
    Search_Range *range;
    Temp_Memory temp;
    char *spare;
    i32 start_pos, end_pos, count;
    
    temp = begin_temp_memory(part);
    spare = push_array(part, char, iter.word.size);
    
    count = set->count;
    for (; iter.i < count;){
        range = set->ranges + iter.i;
        
        end_pos = range->start + range->size;
        
        if (iter.pos + iter.word.size < end_pos){
            start_pos = Max(iter.pos, range->start);
            result.start = buffer_find_string(range->buffer, start_pos, end_pos, iter.word.str, iter.word.size, spare);
            
            if (result.start < end_pos){
                iter.pos = result.start + 1;
                if (result.start == 0 || !char_is_alpha_numeric(buffer_get_char(range->buffer, result.start - 1))){
                    result.end = buffer_seek_word_right_assume_on_word(range->buffer, result.start);
                    if (result.end < end_pos){
                        result.found_match = 1;
                        result.buffer = range->buffer;
                        iter.pos = result.end;
                        break;
                    }
                }
            }
            else{
                ++iter.i, iter.pos = 0;
            }
        }
        else{
            ++iter.i, iter.pos = 0;
        }
    }
    end_temp_memory(temp);
    
    *iter_ = iter;
    
    return(result);
}

// BOTTOM

