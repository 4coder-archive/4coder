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
history__push_back_record_ptr(Heap *heap, Record_Ptr_Lookup_Table *lookup, Record *record){
    if (lookup->records == 0 || lookup->count == lookup->max){
        i32 new_max = clamp_bottom(1024, lookup->max*2);
        Record **new_records = (Record**)heap_allocate(heap, sizeof(Record*)*new_max);
        block_copy(new_records, lookup->records, sizeof(*new_records)*lookup->count);
        if (lookup->records != 0){
            heap_free(heap, lookup->records);
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
    i32 original = global_history->edit_grouping_counter;
    global_history->edit_grouping_counter = clamp_bottom(0, global_history->edit_grouping_counter + adjustment);
    if (global_history->edit_grouping_counter == 0 && original > 0){
        global_history->edit_number_counter += 1;
    }
}

internal void
history_init(Application_Links *app, History *history){
    history->activated = true;
    history->arena = make_arena(app, KB(32));
    memory_bank_init(&history->bank);
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
history_free(Heap *heap, History *history){
    if (history->activated){
        arena_release_all(&history->arena);
        memory_bank_free_all(heap, &history->bank);
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
history_get_sub_record(Record *record, i32 sub_index){
    Record *result = 0;
    if (record->kind == RecordKind_Group){
        if (0 <= sub_index && sub_index <= record->group.count){
            Node *node = history__to_node(&record->group.children, sub_index);
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
history__stash_record(Heap *heap, History *history, Record *new_record){
    Assert(history->record_lookup.count == history->record_count);
    dll_insert_back(&history->records, &new_record->node);
    history->record_count += 1;
    history__push_back_record_ptr(heap, &history->record_lookup, new_record);
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
history_record_edit(Heap *heap, Global_History *global_history, History *history, Gap_Buffer *buffer, Edit edit){
    if (history->activated){
        Assert(history->record_lookup.count == history->record_count);
        
        Record *new_record = history__allocate_record(heap, history);
        history__stash_record(heap, history, new_record);
        
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
            end_temp_memory(&history->arena, first_record_to_clear->restore_point);
            
            Node *last_node_to_clear = sentinel->prev;
            
            history__free_nodes(history, index + 1, first_node_to_clear, last_node_to_clear);
        }
        
        Assert(history->record_lookup.count == history->record_count);
    }
}

internal void
history__optimize_group(Partition *scratch, History *history, Record *record){
    Assert(record->kind == RecordKind_Group);
    for (;;){
        Record *right = CastFromMember(Record, node, record->group.children.prev);
        if (record->group.count == 1){
            Record *child = right;
            record->restore_point = child->restore_point;
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
            
            Temp_Memory temp = begin_temp_memory(scratch);
            i32 new_length_forward  = left->single.length_forward  + right->single.length_forward ;
            i32 new_length_backward = left->single.length_backward + right->single.length_backward;
            
            char *temp_str_forward  = 0;
            char *temp_str_backward = 0;
            
            if (left->single.first + left->single.length_forward == right->single.first){
                do_merge = true;
                temp_str_forward  = push_array(scratch, char, new_length_forward );
                temp_str_backward = push_array(scratch, char, new_length_backward);
                block_copy(temp_str_forward                              , left->single.str_forward , left->single.length_forward );
                block_copy(temp_str_forward + left->single.length_forward, right->single.str_forward, right->single.length_forward);
                block_copy(temp_str_backward                               , left->single.str_backward , left->single.length_backward );
                block_copy(temp_str_backward + left->single.length_backward, right->single.str_backward, right->single.length_backward);
            }
            else if (right->single.first + right->single.length_backward == left->single.first){
                do_merge = true;
                temp_str_forward  = push_array(scratch, char, new_length_forward );
                temp_str_backward = push_array(scratch, char, new_length_backward);
                block_copy(temp_str_forward                               , right->single.str_forward, right->single.length_forward);
                block_copy(temp_str_forward + right->single.length_forward, left->single.str_forward , left->single.length_forward );
                block_copy(temp_str_backward                                , right->single.str_backward, right->single.length_backward);
                block_copy(temp_str_backward + right->single.length_backward, left->single.str_backward , left->single.length_backward );
            }
            else{
                break;
            }
            
            if (do_merge){
                end_temp_memory(&history->arena, left->restore_point);
                
                char *new_str_forward  = push_array(&history->arena, char, new_length_forward );
                char *new_str_backward = push_array(&history->arena, char, new_length_backward);
                
                block_copy(new_str_forward , temp_str_forward , new_length_forward );
                block_copy(new_str_backward, temp_str_backward, new_length_backward);
                
                left->edit_number = right->edit_number;
                left->single.str_forward  = new_str_forward ;
                left->single.str_backward = new_str_backward;
                left->single.length_forward  = new_length_forward ;
                left->single.length_backward = new_length_backward;
                
                history__free_single_node(history, &right->node);
                record->group.count -= 1;
            }
            
            end_temp_memory(temp);
        }
        else{
            break;
        }
    }
}

internal void
history_merge_records(Partition *scratch, Heap *heap, History *history, i32 first_index, i32 last_index){
    if (history->activated){
        Assert(history->record_lookup.count == history->record_count);
        Assert(first_index < last_index);
        Node *first_node = history__to_node(history, first_index);
        Node *last_node  = history__to_node(history, last_index );
        Assert(first_node != &history->records && first_node != 0);
        Assert(last_node  != &history->records && last_node  != 0);
        
        Record *new_record = history__allocate_record(heap, history);
        
        Node *left  = first_node->prev;
        Node *right = last_node->next;
        left->next  = &new_record->node;
        new_record->node.prev = left;
        right->prev = &new_record->node;
        new_record->node.next = right;
        
        // NOTE(allen): here we remove (last_index - first_index + 1) nodes, and insert 1 node
        // which simplifies to this:
        history->record_count -= last_index - first_index;
        
        Record *first_record = CastFromMember(Record, node, first_node);
        Record *last_record  = CastFromMember(Record, node, last_node);
        
        new_record->restore_point = first_record->restore_point;
        new_record->edit_number = last_record->edit_number;
        new_record->kind = RecordKind_Group;
        
        Node *new_sentinel = &new_record->group.children;
        dll_init_sentinel(new_sentinel);
        
        Node *one_past_last_node = last_node->next;
        i32 count = 0;
        for (Node *node = first_node, *next = 0;
             node != one_past_last_node;
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
                    
                    Node *sub_right = new_sentinel;
                    Node *sub_left = new_sentinel->prev;
                    sub_left->next = first;
                    first->prev = sub_left;
                    last->next = sub_right;
                    sub_right->prev = last;
                    count += record->group.count;
                }break;
                
                default:
                {
                    InvalidCodePath;
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

