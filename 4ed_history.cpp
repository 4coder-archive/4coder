/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 24.03.2018
 *
 * History
 */

// TOP

// TODO(allen): do(make an index <-> node acceleration structure for history)

internal i32
history__to_index(History *history, Node *node){
    i32 result = -1;
    i32 counter = 0;
    Node *sentinel = &history->records;
    Node *it = sentinel;
    do{
        if (it == node){
            result = counter;
            break;
        }
        counter += 1;
        it = it->next;
    } while (it != sentinel);
    return(result);
}

internal Node*
history__to_node(History *history, int32_t index){
    Node *result = 0;
    if (0 <= index && index <= history->record_count){
        i32 counter = 0;
        Node *sentinel = &history->records;
        Node *it = sentinel;
        do{
            if (counter == index){
                result = it;
                break;
            }
            counter += 1;
            it = it->next;
        } while (it != sentinel);
    }
    return(result);
}

internal Record*
history__allocate_record(Heap *heap, History *history){
    Node *sentinel = &history->free_records;
    Node *new_node = sentinel->next;
    if (new_node == sentinel){
        i32 new_record_count = 1024;
        void *memory = memory_bank_allocate(heap, &history->bank, sizeof(Record)*new_record_count);
        
        Record *new_record = (Record*)memory;
        sentinel->next = &new_record->node;
        new_record->node.prev = sentinel;
        for (i32 i = 1; i < new_record_count; i += 1, new_record += 1){
            new_record[0].node.next = &new_record[1].node;
            new_record[1].node.prev = &new_record[0].node;
        }
        new_record[0].node.next = sentinel;
        sentinel->prev = &new_record[0].node;
        
        new_node = &((Record*)memory)->node;
    }
    dll_remove(new_node);
    Record *record = CastFromMember(Record, node, new_node);
    block_zero_struct(record);
    return(record);
}

internal void
global_history_init(Global_History *global_history){
    global_history->edit_number_counter = 0;
    global_history->edit_grouping_counter = 0;
}

internal i32
global_history_get_edit_number(Global_History *global_history){
    i32 result = global_history->edit_number_counter;
    if (global_history->edit_grouping_counter == 0){
        global_history->edit_number_counter += 1;
    }
    return(result);
}

internal void
global_history_adjust_edit_grouping_counter(Global_History *global_history, i32 adjustment){
    global_history->edit_grouping_counter += adjustment;
}

internal void
history_init(Application_Links *app, History *history){
    history->activated = true;
    history->arena = make_arena(app, KB(32));
    memory_bank_init(&history->bank);
    dll_init_sentinel(&history->free_records);
    dll_init_sentinel(&history->records);
    history->record_count = 0;
}

internal void
history_free(Heap *heap, History *history){
    if (history->activated){
        arena_release_all(&history->arena);
        memory_bank_free_all(heap, &history->bank);
    }
}

internal i32
history_get_record_count(History *history){
    return(history->record_count);
}

internal Record*
history_get_record(History *history, i32 index){
    Record *record = 0;
    Node *node = history__to_node(history, index);
    if (node != 0){
        record = CastFromMember(Record, node, node);
    }
    return(record);
}

internal Record*
history_get_dummy_record(History *history){
    return(CastFromMember(Record, node, &history->records));
}

internal void
history__stash_record(History *history, Record *new_record){
    dll_insert_back(&history->records, &new_record->node);
    history->record_count += 1;
}

internal void
history__free_nodes(History *history, i32 first_index, Node *first_node, Node *last_node){
    if (first_node == last_node){
        dll_remove(first_node);
        dll_insert(&history->free_records, first_node);
    }
    else{
        {
            Node *left = first_node->prev;
            Node *right = last_node->next;
            left->next = right;
            right->prev = left;
        }
        
        {
            Node *left = &history->free_records;
            Node *right = left->next;
            left->next = first_node;
            first_node->prev = left;
            right->prev = last_node;
            last_node->next = right;
        }
    }
    Assert(first_index != 0);
    history->record_count = first_index - 1;
}

internal void
history_record_edit(Heap *heap, Global_History *global_history, History *history, Gap_Buffer *buffer, Edit edit){
    if (history->activated){
        Record *new_record = history__allocate_record(heap, history);
        history__stash_record(history, new_record);
        
        new_record->restore_point = temp_memory_light(begin_temp_memory(&history->arena));
        new_record->edit_number = global_history_get_edit_number(global_history);
        
        new_record->kind = RecordKind_Single;
        
        i32 length_forward = edit.length;
        i32 length_backward = edit.range.one_past_last - edit.range.first;
        
        new_record->single.str_forward  = push_array(&history->arena, char, length_forward);
        new_record->single.str_backward = push_array(&history->arena, char, length_backward);
        new_record->single.length_forward  = length_forward;
        new_record->single.length_backward = length_backward;
        new_record->single.first = edit.range.first;
        
        block_copy(new_record->single.str_forward, edit.str, length_forward);
        buffer_stringify_range(buffer, edit.range, new_record->single.str_backward);
    }
}

internal void
history_record_edit(Heap *heap, Global_History *global_history, History *history, Gap_Buffer *buffer, Edit_Array edits, Buffer_Batch_Edit_Type batch_type){
    if (history->activated){
        Record *new_record = history__allocate_record(heap, history);
        history__stash_record(history, new_record);
        
        new_record->restore_point = temp_memory_light(begin_temp_memory(&history->arena));
        new_record->edit_number = global_history_get_edit_number(global_history);
        
        new_record->kind = RecordKind_Batch;
        
        i32 length_forward = 0;
        i32 length_backward = 0;
        
        for (i32 i = 0; i < edits.count; i += 1){
            length_forward += edits.vals[i].length;
            length_backward += edits.vals[i].range.one_past_last - edits.vals[i].range.first;
        }
        
        new_record->batch.type = batch_type;
        new_record->batch.count = edits.count;
        new_record->batch.str_base_forward  = push_array(&history->arena, char, length_forward);
        new_record->batch.str_base_backward = push_array(&history->arena, char, length_backward);
        new_record->batch.batch_records     = push_array(&history->arena, Record_Batch_Slot, edits.count);
        
        char *str_base_forward  = new_record->batch.str_base_forward;
        char *cursor_forward    = str_base_forward;
        char *str_base_backward = new_record->batch.str_base_backward;
        char *cursor_backward   = str_base_backward;
        
        Record_Batch_Slot *batch_slot = new_record->batch.batch_records;
        Edit *edit = edits.vals;
        
        for (i32 i = 0; i < edits.count; i += 1, batch_slot += 1, edit += 1){
            i32 first = edit->range.first;
            i32 length_forward = edit->length;
            i32 length_backward = edit->range.one_past_last - first;
            
            batch_slot->length_forward  = length_forward ;
            batch_slot->length_backward = length_backward;
            batch_slot->first = first;
            
            block_copy(cursor_forward , edit->str, length_forward);
            buffer_stringify_range(buffer, edit->range, cursor_backward);
            
            cursor_forward  += length_forward ;
            cursor_backward += length_backward;
        }
    }
}

internal void
history_dump_records_after_index(History *history, i32 index){
    Assert(0 <= index && index <= history->record_count);
    if (index < history->record_count){
        Node *node = history__to_node(history, index);
        Node *first_node_to_clear = node->next;
        
        Node *sentinel = &history->records;
        Assert(first_node_to_clear != sentinel);
        
        Record *first_record_to_clear = CastFromMember(Record, node, first_node_to_clear);
        end_temp_memory(&history->arena, first_record_to_clear->restore_point);
        
        Node *last_node_to_clear = sentinel->prev;
        
        history__free_nodes(history, index + 1, first_node_to_clear, last_node_to_clear);
    }
}

// BOTTOM

