/* 
 * Mr. 4th Dimention - Allen Webster
 *  Four Tech
 *
 * public domain -- no warranty is offered or implied; use this code at your own risk
 * 
 * 11.11.2015
 * 
 * Converts code file and saved history from 4coder
 *  into replay file for the test system.
 * 
 */

// TOP

#include "shared_test_config.cpp"

#include "4coder_shared.cpp"
#include "4coder_golden_array.cpp"

#define Buffer_Type Buffer
#include "4coder_buffer_abstract.cpp"

#include "shared_test_utils.cpp"

void do_single_edit(Replay *replay, Buffer *buffer, Edit_Step step, char *str, void *scratch, int scratch_size){
    {
        Edit_Stack *stack;
        char *new_data;
        Edit_Step inv_step;
        int size, new_max, new_size;
    
        stack = &replay->replay;
    
        memzero_4tech(inv_step);
        size = 0;
        buffer_invert_edit(buffer, step.edit, &inv_step.edit, (char*)scratch, &size, scratch_size);
    
        assert_4tech(stack->count < replay->ed_max);
        stack = &replay->replay;
        stack->edits[replay->ed_max - (++stack->count)] = inv_step;
    
        new_size = stack->size + size;

        if (new_size > replay->str_max){
            new_max = (replay->str_max + size) * 2;
            new_data = (char*)malloc(new_max);
            memcpy_4tech(new_data + new_max - stack->size, stack->strings + replay->str_max - stack->size, stack->size);
            free(stack->strings);
            stack->strings = new_data;
            replay->str_max = new_max;
        }
    
        memcpy_4tech(stack->strings + replay->str_max - new_size, scratch, size);
        stack->size = new_size;

        printf("just wrote: [[ %.*s ]]\n", size, scratch);
    }
    
    int start = step.edit.start;
    int end = step.edit.end;
    int str_start = step.edit.str_start;
    int len = step.edit.len;

    int shift_amount;
    int request_amount;
    
    for (;buffer_replace_range(buffer, start, end, str + str_start, len, &shift_amount,
                               scratch, scratch_size, &request_amount);){
        void *new_data = 0;
        if (request_amount > 0) new_data = malloc(request_amount);
        void *old_data = buffer_edit_provide_memory(buffer, new_data, request_amount);
        if (old_data) free(old_data);
    }
}

void do_batch_edit(Replay *replay, Buffer *buffer, Edit_Step step, void *scratch, int scratch_size){
    {
        Edit_Stack *stack;
        Edit_Step inv_step;
    
        stack = &replay->replay;

        memzero_4tech(inv_step);
        inv_step.first_child = step.inverse_first_child;
        inv_step.inverse_first_child = step.first_child;
        inv_step.child_count = step.inverse_child_count;
        inv_step.inverse_child_count = step.child_count;
        
        assert_4tech(stack->count < replay->ed_max);
        stack = &replay->replay;
        stack->edits[replay->ed_max - (++stack->count)] = inv_step;
    }
    
    char *str;
    Buffer_Edit *batch;
    int batch_size;
    
    int request_amount;
    Buffer_Batch_State state;

    str = replay->children.strings;
    batch = replay->children.edits + step.first_child;
    batch_size = step.child_count;
    
    memzero_4tech(state);
    for (;buffer_batch_edit_step(&state, buffer, batch, str, batch_size,
                                 scratch, scratch_size, &request_amount);){
        void *new_data = 0;
        if (request_amount > 0) new_data = malloc(request_amount);
        void *old_data = buffer_edit_provide_memory(buffer, new_data, request_amount);
        if (old_data) free(old_data);
    }
}

#if 0

int main(int argc, char **argv){
    if (argc < 3){
        printf("usage: hst_to_rply <history> <target-file> <output>\n");
        exit(1);
    }

    File_Data history_file, code_file;
    history_file = get_file(argv[1]);
    code_file = get_file(argv[2]);

    if (!history_file.data || !code_file.data) exit(1);
    
    History history;
    char *curs = history_file.data;
    
    history.undo.count = read_int(&curs);
    history.redo.count = read_int(&curs);
    history.history.count = read_int(&curs);
    history.children.count = read_int(&curs);
    
    history.undo.size = read_int(&curs);
    history.redo.size = read_int(&curs);
    history.history.size = read_int(&curs);
    history.children.size = read_int(&curs);

    
    history.undo.edits = (Edit_Step*)curs;
    curs += sizeof(Edit_Step)*history.undo.count;
    
    history.redo.edits = (Edit_Step*)curs;
    curs += sizeof(Edit_Step)*history.redo.count;
    
    history.history.edits = (Edit_Step*)curs;
    curs += sizeof(Edit_Step)*history.history.count;
    
    history.children.edits = (Buffer_Edit*)curs;
    curs += sizeof(Buffer_Edit)*history.children.count;
    
    
    history.undo.strings = (char*)curs;
    curs += history.undo.size;
    
    history.redo.strings = (char*)curs;
    curs += history.redo.size;
    
    history.history.strings = (char*)curs;
    curs += history.history.size;
    
    history.children.strings = (char*)curs;
    curs += history.children.size;


    void *scratch;
    int scratch_size;
    scratch_size = 1 << 20;
    scratch = malloc(scratch_size);
    
    
    Buffer buffer;
    Buffer_Init_Type init;
    
    memzero_4tech(buffer);
    memzero_4tech(init);
    for (init = buffer_begin_init(&buffer, code_file.data, code_file.size);
         buffer_init_need_more(&init);){
        int page_size = buffer_init_page_size(&init);
        page_size = round_up_4tech(page_size, 4 << 10);
        void *data = malloc(page_size);
        buffer_init_provide_page(&init, data, page_size);
    }
    buffer_end_init(&init, scratch, scratch_size);

    
    Replay replay;
    
    replay.children = history.children;
    replay.ed_max = history.history.count;
    replay.replay.edits = (Edit_Step*)malloc(sizeof(Edit_Step)*replay.ed_max);
    replay.replay.count = 0;
    
    replay.str_max = history.history. size * 4;
    replay.replay.strings = (char*)malloc(replay.str_max);
    replay.replay.size = 0;

    
    for (int i = history.history.count - 1; i >= 0; --i){
        Edit_Step step = history.history.edits[i];
        if (step.child_count == 0){
            do_single_edit(&replay, &buffer, step, history.history.strings, scratch, scratch_size);
        }
        else{
            assert(step.special_type == 1);
            do_batch_edit(&replay, &buffer, step, scratch, scratch_size);
        }
    }

    assert(history.history.count == replay.replay.count);

    int str_start = 0;
    for (int i = 0; i < history.history.count; ++i){
        replay.replay.edits[i].edit.str_start = str_start;
        str_start += replay.replay.edits[i].edit.len;
    }
    
    File_Data out_file;
    out_file.size = 0;
    out_file.size += replay.replay.count*sizeof(Edit_Step) + sizeof(int);
    out_file.size += replay.replay.size + sizeof(int);
    out_file.size += replay.children.count*sizeof(Buffer_Edit) + sizeof(int);
    out_file.size += replay.children.size + sizeof(int);
    
    out_file.data = (char*)malloc(out_file.size);
    
    curs = out_file.data;
    curs = write_int(curs, replay.replay.count);
    curs = write_int(curs, replay.children.count);
    
    curs = write_int(curs, replay.replay.size);
    curs = write_int(curs, replay.children.size);
    
    memcpy(curs, replay.replay.edits, replay.replay.count*sizeof(Edit_Step));
    curs += replay.replay.count*sizeof(Edit_Step);

    memcpy(curs, replay.children.edits, replay.children.count*sizeof(Buffer_Edit));
    curs += replay.children.count*sizeof(Buffer_Edit);
    
    memcpy(curs, replay.replay.strings + replay.str_max - replay.replay.size, replay.replay.size);
    curs += replay.replay.size;
    
    memcpy(curs, replay.children.strings, replay.children.size);
    curs += replay.children.size;

    assert((int)(curs - out_file.data) == out_file.size);
    
    save_file(argv[3], out_file);
    
    return(0);
}

#else

int main(int argc, char **argv){
    if (argc < 2){
        printf("usage: <replay-file>\n");
        exit(1);
    }

    File_Data file = get_file(argv[1]);
    //char *curs = file.data;
    
    Replay replay;
    prepare_replay(file, &replay);
#if 0
    replay.replay.count = read_int(&curs);
    replay.children.count = read_int(&curs);
    replay.replay.size = read_int(&curs);
    replay.children.size = read_int(&curs);
    
    replay.replay.edits = (Edit_Step*)curs;
    curs += sizeof(Edit_Step)*replay.replay.count;
    
    replay.children.edits = (Buffer_Edit*)curs;
    curs += sizeof(Buffer_Edit)*replay.children.count;
    
    replay.replay.strings = curs;
    curs += replay.replay.size;
    
    replay.children.strings = curs;
    curs += replay.children.size;

    assert_4tech((int)(curs - file.data) == file.size);
#endif
    
    for (int i = 0; i < replay.replay.count; ++i){
        Edit_Step step = replay.replay.edits[i];
        if (step.first_child == 0){
            if (step.edit.len > 0 && step.edit.str_start >= 0 && step.edit.str_start < replay.replay.size){
                printf("replace [%d,%d] with %.*s\n", step.edit.start, step.edit.end,
                       step.edit.len, replay.replay.strings + step.edit.str_start);
            }
            else if (step.edit.len > 0){
                printf("str_start out of bounds!\n");
            }
            else{
                printf("replace [%d,%d] with ~~empty string~~\n", step.edit.start, step.edit.end);
            }
        }
        else{
            printf("batch edit\n");
        }
    }

    printf("string section:\n%.*s\n", replay.replay.size, replay.replay.strings);
}

#endif

// BOTTOM

