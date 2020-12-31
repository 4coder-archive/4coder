/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 24.03.2018
 *
 * History
 */

// TOP

internal Node*
history__to_node(Node *sentinel, i32 index){
    Node *result = 0;
    i32 counter = 0;
    Node *it = sentinel;
    do{
        if (counter == index){
            result = it;
            break;
        }
        counter += 1;
        it = it->next;
    } while (it != sentinel);
    return(result);
}

internal void
history__push_back_record_ptr(Base_Allocator *allocator, Record_Ptr_Lookup_Table *lookup, Record *record){
    if (lookup->records == 0 || lookup->count == lookup->max){
        i32 new_max = clamp_bot(1024, lookup->max*2);
        String_Const_u8 new_memory = base_allocate(allocator, sizeof(Record*)*new_max);
        Record **new_records = (Record**)new_memory.str;
        block_copy(new_records, lookup->records, sizeof(*new_records)*lookup->count);
        if (lookup->records != 0){
            base_free(allocator, lookup->records);
        }
        lookup->records = new_records;
        lookup->max = new_max;
    }
    Assert(lookup->count < lookup->max);
    lookup->records[lookup->count] = record;
    lookup->count += 1;
}

internal void
history__shrink_array(Record_Ptr_Lookup_Table *lookup, i32 new_count){
    Assert(0 <= new_count && new_count <= lookup->count);
    lookup->count = new_count;
}

internal void
history__merge_record_ptr_range_to_one_ptr(Record_Ptr_Lookup_Table *lookup, i32 first_one_based, i32 last_one_based, Record *record){
    i32 first = first_one_based - 1;
    i32 one_past_last = last_one_based;
    Assert(0 <= first && first <= one_past_last && one_past_last <= lookup->count);
    if (first < one_past_last){
        i32 shift = 1 + first - one_past_last;
        block_copy(lookup->records + one_past_last + shift, lookup->records + one_past_last, lookup->count - one_past_last);
        lookup->count += shift;
    }
    lookup->records[first] = record;
}

internal Node*
history__to_node(History *history, i32 index){
    Node *result = 0;
    if (index == 0){
        result = &history->records;
    }
    else if (0 < index && index <= history->record_count){
        Record_Ptr_Lookup_Table *lookup = &history->record_lookup;
        Assert(lookup->count == history->record_count);
        result = &lookup->records[index - 1]->node;
    }
    return(result);
}

////////////////////////////////

internal Record*
history__allocate_record(History *history){
    Node *sentinel = &history->free_records;
    Node *new_node = sentinel->next;
    if (new_node == sentinel){
        i32 new_record_count = 1024;
        String_Const_u8 new_memory = base_allocate(&history->heap_wrapper, sizeof(Record)*new_record_count);
        void *memory = new_memory.str;
        
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
    i32 original = global_history->edit_grouping_counter;
    global_history->edit_grouping_counter = clamp_bot(0, global_history->edit_grouping_counter + adjustment);
    if (global_history->edit_grouping_counter == 0 && original > 0){
        global_history->edit_number_counter += 1;
    }
}

internal void
history_init(Thread_Context *tctx, Models *models, History *history){
    history->activated = true;
    history->arena = make_arena_system();
    heap_init(&history->heap, tctx->allocator);
    history->heap_wrapper = base_allocator_on_heap(&history->heap);
    dll_init_sentinel(&history->free_records);
    dll_init_sentinel(&history->records);
    history->record_count = 0;
    block_zero_struct(&history->record_lookup);
}

internal b32
history_is_activated(History *history){
    return(history->activated);
}

internal void
history_free(Thread_Context *tctx, History *history){
    if (history->activated){
        linalloc_clear(&history->arena);
        heap_free_all(&history->heap);
        block_zero_struct(history);
    }
}

internal i32
history_get_record_count(History *history){
    i32 result = 0;
    if (history->activated){
        result = history->record_count;
    }
    return(result);
}

internal Record*
history_get_record(History *history, i32 index){
    Record *result = 0;
    if (history->activated){
        Node *node = history__to_node(history, index);
        if (node != 0){
            result = CastFromMember(Record, node, node);
        }
    }
    return(result);
}

internal Record*
history_get_sub_record(Record *record, i32 sub_index_one_based){
    Record *result = 0;
    if (record->kind == RecordKind_Group){
        if (0 < sub_index_one_based && sub_index_one_based <= record->group.count){
            Node *node = history__to_node(&record->group.children, sub_index_one_based);
            if (node != 0){
                result = CastFromMember(Record, node, node);
            }
        }
    }
    return(result);
}

internal Record*
history_get_dummy_record(History *history){
    Record *result = 0;
    if (history->activated){
        result = CastFromMember(Record, node, &history->records);
    }
    return(result);
}

internal void
history__stash_record(History *history, Record *new_record){
    Assert(history->record_lookup.count == history->record_count);
    dll_insert_back(&history->records, &new_record->node);
    history->record_count += 1;
    history__push_back_record_ptr(&history->heap_wrapper, &history->record_lookup, new_record);
    Assert(history->record_lookup.count == history->record_count);
}

internal void
history__free_single_node(History *history, Node *node){
    dll_remove(node);
    dll_insert(&history->free_records, node);
}

internal void
history__free_nodes(History *history, i32 first_index, Node *first_node, Node *last_node){
    if (first_node == last_node){
        history__free_single_node(history, first_node);
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
    history__shrink_array(&history->record_lookup, history->record_count);
}

internal void
history_record_edit(Global_History *global_history, History *history, Gap_Buffer *buffer,
                    i64 pos_before_edit, Edit edit){
    if (history->activated){
        Assert(history->record_lookup.count == history->record_count);
        
        Record *new_record = history__allocate_record(history);
        history__stash_record(history, new_record);
        
        new_record->restore_point = begin_temp(&history->arena);
        if (pos_before_edit >= 0){
            new_record->pos_before_edit = pos_before_edit;
        }
        else{
            new_record->pos_before_edit = edit.range.min;
        }
        
        new_record->edit_number = global_history_get_edit_number(global_history);
        
        new_record->kind = RecordKind_Single;
        
        new_record->single.forward_text = push_string_copy(&history->arena, edit.text);
        new_record->single.backward_text = buffer_stringify(&history->arena, buffer, edit.range);
        new_record->single.first = edit.range.first;
        
        Assert(history->record_lookup.count == history->record_count);
    }
}

internal void
history_dump_records_after_index(History *history, i32 index){
    if (history->activated){
        Assert(history->record_lookup.count == history->record_count);
        
        Assert(0 <= index && index <= history->record_count);
        if (index < history->record_count){
            Node *node = history__to_node(history, index);
            Node *first_node_to_clear = node->next;
            
            Node *sentinel = &history->records;
            Assert(first_node_to_clear != sentinel);
            
            Record *first_record_to_clear = CastFromMember(Record, node, first_node_to_clear);
            end_temp(first_record_to_clear->restore_point);
            
            Node *last_node_to_clear = sentinel->prev;
            
            history__free_nodes(history, index + 1, first_node_to_clear, last_node_to_clear);
        }
        
        Assert(history->record_lookup.count == history->record_count);
    }
}

internal void
history__optimize_group(Arena *scratch, History *history, Record *record){
    Assert(record->kind == RecordKind_Group);
    for (;;){
        Record *right = CastFromMember(Record, node, record->group.children.prev);
        if (record->group.count == 1){
            Record *child = right;
            record->restore_point = child->restore_point;
            record->pos_before_edit = child->pos_before_edit;
            record->edit_number = child->edit_number;
            record->kind = RecordKind_Single;
            record->single = child->single;
            // NOTE(allen): don't use "free" because the child node is no longer linked
            // to a valid sentinel, and removing it first (as free does) will mess with
            // the data in record->single
            dll_insert(&history->free_records, &child->node);
            break;
        }
        Record *left  = CastFromMember(Record, node, right->node.prev);
        if (right->kind == RecordKind_Single && left->kind == RecordKind_Single){
            b32 do_merge = false;
            
            Temp_Memory temp = begin_temp(scratch);
            i64 new_length_forward  = left->single.forward_text.size  + right->single.forward_text.size ;
            i64 new_length_backward = left->single.backward_text.size + right->single.backward_text.size;
            
            String_Const_u8 merged_forward = {};
            String_Const_u8 merged_backward = {};
            
            i64 merged_first = 0;
            if (left->single.first + (i64)left->single.forward_text.size == right->single.first){
                do_merge = true;
                merged_forward = push_u8_stringf(scratch, "%.*s%.*s",
                                                 string_expand(left->single.forward_text),
                                                 string_expand(right->single.forward_text));
                merged_backward = push_u8_stringf(scratch, "%.*s%.*s",
                                                  string_expand(left->single.backward_text),
                                                  string_expand(right->single.backward_text));
                merged_first = left->single.first;
            }
            else if (right->single.first + (i64)right->single.backward_text.size == left->single.first){
                do_merge = true;
                merged_forward = push_u8_stringf(scratch, "%.*s%.*s",
                                                 string_expand(right->single.forward_text),
                                                 string_expand(left->single.forward_text));
                merged_backward = push_u8_stringf(scratch, "%.*s%.*s",
                                                  string_expand(right->single.backward_text),
                                                  string_expand(left->single.backward_text));
                merged_first = right->single.first;
            }
            else{
                break;
            }
            
            if (do_merge){
                end_temp(left->restore_point);
                
                left->edit_number = right->edit_number;
                left->single.first = merged_first;
                left->single.forward_text  = push_string_copy(&history->arena, merged_forward);
                left->single.backward_text = push_string_copy(&history->arena, merged_backward);
                
                history__free_single_node(history, &right->node);
                record->group.count -= 1;
            }
            
            end_temp(temp);
        }
        else{
            break;
        }
    }
}

internal void
history_merge_records(Arena *scratch, History *history, i32 first_index, i32 last_index){
    if (history->activated){
        Assert(history->record_lookup.count == history->record_count);
        Assert(first_index < last_index);
        Node *first_node = history__to_node(history, first_index);
        Node *last_node  = history__to_node(history, last_index );
        Assert(first_node != &history->records && first_node != 0);
        Assert(last_node  != &history->records && last_node  != 0);
        
        Record *new_record = history__allocate_record(history);
        
        // NOTE(allen): here we remove (last_index - first_index + 1) nodes, and insert 1 node
        // which simplifies to this:
        history->record_count -= last_index - first_index;
        
        Node *left = first_node->prev;
        dll_remove_multiple(first_node, last_node);
        dll_insert(left, &new_record->node);
        
        Record *first_record = CastFromMember(Record, node, first_node);
        Record *last_record  = CastFromMember(Record, node, last_node);
        
        new_record->restore_point = first_record->restore_point;
        new_record->pos_before_edit = first_record->pos_before_edit;
        new_record->edit_number = last_record->edit_number;
        new_record->kind = RecordKind_Group;
        
        Node *new_sentinel = &new_record->group.children;
        dll_init_sentinel(new_sentinel);
        
        i32 count = 0;
        for (Node *node = first_node, *next = 0;
             node != 0;
             node = next){
            next = node->next;
            Record *record = CastFromMember(Record, node, node);
            switch (record->kind){
                case RecordKind_Single:
                {
                    dll_insert_back(new_sentinel, &record->node);
                    count += 1;
                }break;
                
                case RecordKind_Group:
                {
                    Node *first = record->group.children.next;
                    Node *last  = record->group.children.prev;
                    Assert(first != &record->group.children);
                    Assert(last  != &record->group.children);
                    
                    dll_insert_multiple_back(new_sentinel, first, last);
                    count += record->group.count;
                    
                    // TODO(allen): free the record for the old group!?
                }break;
                
                default:
                {
                    InvalidPath;
                }break;
            }
        }
        
        new_record->group.count = count;
        
        history__merge_record_ptr_range_to_one_ptr(&history->record_lookup, first_index, last_index, new_record);
        Assert(history->record_lookup.count == history->record_count);
        
        if (first_index == history->record_count){
            history__optimize_group(scratch, history, new_record);
        }
    }
}

// BOTTOM

