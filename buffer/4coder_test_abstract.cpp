/* 
 * Mr. 4th Dimention - Allen Webster
 *  Four Tech
 *
 * public domain -- no warranty is offered or implied; use this code at your own risk
 * 
 * 08.11.2015
 * 
 * Buffer experiment testing layer, abstract portion
 * 
 */

// TOP

#define Buffer_Init_Type cat_4tech(Buffer_Type, _Init)
#define Buffer_Stringify_Type cat_4tech(Buffer_Type, _Stringify_Loop)
#define Buffer_Backify_Type cat_4tech(Buffer_Type, _Backify_Loop)

void
init_buffer(Buffer_Type *buffer, File_Data file, void *scratch, int scratch_size){
    memzero_4tech(*buffer);
    Buffer_Init_Type init;
    for (init = buffer_begin_init(buffer, file.data, file.size);
         buffer_init_need_more(&init);){
        int page_size = buffer_init_page_size(&init);
        void *page = malloc(page_size);
        buffer_init_provide_page(&init, page, page_size);
    }
    debug_4tech(int result =)
        buffer_end_init(&init, scratch, scratch_size);
    assert_4tech(result);
}

void
measure_starts_widths(Buffer_Type *buffer, float *font_widths){
    int max = 1 << 10;
    buffer->line_starts = (int*)malloc(max*sizeof(int));
    buffer->line_max = max;
    buffer->line_widths = (float*)malloc(max*sizeof(float));
    buffer->widths_max = max;

    Buffer_Measure_Starts state;
    memzero_4tech(state);
    for (;buffer_measure_starts_widths(&state, buffer, font_widths);){
        int max = buffer->line_max;
        int count = state.count;
        int target = count + 1;

        max = target*2;
        int *new_lines = (int*)malloc(max*sizeof(int));
        memcpy_4tech(new_lines, buffer->line_starts, count*sizeof(int));
        free(buffer->line_starts);
        buffer->line_starts = new_lines;
        buffer->line_max = max;

        float *new_widths = (float*)malloc(max*sizeof(float));
        memcpy_4tech(new_widths, buffer->line_widths, count*sizeof(int));
        free(buffer->line_widths);
        buffer->line_widths = new_widths;
        buffer->widths_max = max;
    }
    buffer->line_count = state.count;
    buffer->widths_count = state.count;
}

void
edit(Buffer_Type *buffer, int start, int end, char *word, int len,
     float *advance_data, void *scratch, int scratch_size){
    int shift_amount, request_amount;
    
    for (;buffer_replace_range(buffer, start, end, word, len, &shift_amount,
                               scratch, scratch_size, &request_amount);){
        void *new_data = 0;
        if (request_amount > 0) new_data = malloc(request_amount);
        void *old_data = buffer_edit_provide_memory(buffer, new_data, request_amount);
        if (old_data) free(old_data);
    }
    
    int line_start = buffer_get_line_index(buffer, start);
    int line_end = buffer_get_line_index(buffer, end);
    int replaced_line_count = line_end - line_start;
    int new_line_count = buffer_count_newlines(buffer, start, start+len);
    int line_shift =  new_line_count - replaced_line_count;

    {
        assert_4tech(buffer->line_starts);
        int count = buffer->line_count;
        if (count + line_shift > buffer->line_max){
            int new_max = round_up_4tech(buffer->line_max + line_shift, 1 << 10);
            int *new_lines = (int*)malloc(sizeof(int)*new_max);
            memcpy_4tech(new_lines, buffer->line_starts, sizeof(int)*count);
            free(buffer->line_starts);

            buffer->line_starts = new_lines;
            buffer->line_max = new_max;
        }
        
        buffer_remeasure_starts(buffer, line_start, line_end, line_shift, shift_amount);
    }

    {
        assert_4tech(buffer->line_widths);
        if (buffer->line_count > buffer->widths_max){
            int new_max = round_up_4tech(buffer->line_max, 1 << 10);
            float *new_widths = (float*)malloc(sizeof(float)*new_max);
            memcpy_4tech(new_widths, buffer->line_widths, sizeof(float)*buffer->widths_count);
            free(buffer->line_widths);
            
            buffer->line_widths = new_widths;
            buffer->widths_max = new_max;
        }

        buffer_remeasure_widths(buffer, advance_data, line_start, line_end, line_shift);
    }
}

void
insert_bottom(Buffer_Type *buffer, char *word, int len, float *advance_data, void *scratch, int scratch_size){
    int size;
    size = buffer_size(buffer);
    edit(buffer, size, size, word, len, advance_data, scratch, scratch_size);
}

void
insert_top(Buffer_Type *buffer, char *word, int len, float *advance_data, void *scratch, int scratch_size){
    edit(buffer, 0, 0, word, len, advance_data, scratch, scratch_size);
}

void
delete_bottom(Buffer_Type *buffer, int len, float *advance_data, void *scratch, int scratch_size){
    int size;
    size = buffer_size(buffer);
    edit(buffer, size - len, size, 0, 0, advance_data, scratch, scratch_size);
}

void
delete_top(Buffer_Type *buffer, int len, float *advance_data, void *scratch, int scratch_size){
    edit(buffer, 0, len, 0, 0, advance_data, scratch, scratch_size);
}

// BOTTOM

