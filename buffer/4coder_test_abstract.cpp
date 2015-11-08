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

// BOTTOM

