/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 31.03.2019
 *
 * Text layout representation
 *
 */

// TOP

internal void
text_layout_init(Application_Links *app, Text_Layout_Container *container){
    block_zero_struct(container);
    container->node_arena = make_arena(app);
}

internal Text_Layout*
text_layout_new__alloc_layout(Text_Layout_Container *container){
    Text_Layout_Node *node = container->free_nodes;
    if (node == 0){
        node = push_array(&container->node_arena, Text_Layout_Node, 1);
    }
    else{
        container->free_nodes = node->next;
    }
    return(&node->layout);
}

internal Text_Layout_ID
text_layout_new(Heap *heap, Text_Layout_Container *container, Buffer_ID buffer_id, Buffer_Point point){
    Text_Layout *new_layout_data = text_layout_new__alloc_layout(container);
    new_layout_data->buffer_id = buffer_id;
    new_layout_data->point = point;
    Text_Layout_ID new_id = ++container->id_counter;
    insert_u32_Ptr_table(heap, &container->table, new_id, new_layout_data);
    return(new_id);
}

internal b32
text_layout_get(Text_Layout_Container *container, Text_Layout_ID id, Text_Layout *out){
    b32 result = false;
    void *ptr = 0;
    if (lookup_u32_Ptr_table(&container->table, id, &ptr)){
        block_copy(out, ptr, sizeof(*out));
        result = true;
    }
    return(result);
}

internal b32
text_layout_erase(Text_Layout_Container *container, Text_Layout_ID id){
    b32 result = false;
    void *ptr = 0;
    if (lookup_u32_Ptr_table(&container->table, id, &ptr)){
        erase_u32_Ptr_table(&container->table, id);
        Text_Layout_Node *node = CastFromMember(Text_Layout_Node, layout, ptr);
        node->next = container->free_nodes;
        container->free_nodes = node;
        result = true;
    }
    return(result);
}

// BOTTOM
