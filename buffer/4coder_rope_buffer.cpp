/* 
 * Mr. 4th Dimention - Allen Webster
 *  Four Tech
 *
 * public domain -- no warranty is offered or implied; use this code at your own risk
 * 
 * 03.11.2015
 * 
 * Buffer data object
 *  type - Rope
 * 
 */

// TOP

typedef struct Rope_Node{
    int left, right, parent;
    int left_weight, weight;
    int str_start;
} Rope_Node;

typedef struct Rope_String{
    int next_free;
} Rope_String;

#define rope_string_full_size 256
#define rope_string_width (rope_string_full_size-sizeof(Rope_String))

typedef struct Rope_Buffer_Edit_State{
    int left, right, throw_away, middle;
} Rope_Buffer_Edit_State;

typedef struct Rope_Buffer{
    void *data;
    int free_rope_string;
    int string_count;
    
    Rope_Node *nodes;
    int free_rope_node;
    int node_count;
    
    float *line_widths;
    int *line_starts;
    int line_count;
    int widths_count;
    int line_max;
    int widths_max;
    
    int grow_string_memory;
    int edit_stage;
    Rope_Buffer_Edit_State edit_state;
} Rope_Buffer;

inline_4tech int
buffer_good(Rope_Buffer *buffer){
    int good;
    good = (buffer->data != 0);
    return(good);
}

inline_4tech int
buffer_size(Rope_Buffer *buffer){
    int size;
    size = buffer->nodes->left_weight;
    return(size);
}

typedef struct{
    Rope_Buffer *buffer;
    char *data;
    int size;

    int rope_string_count;
    int node_count;
} Rope_Buffer_Init;

internal_4tech Rope_Buffer_Init
buffer_begin_init(Rope_Buffer *buffer, char *data, int size){
    Rope_Buffer_Init init;
    
    init.buffer = buffer;
    init.data = data;
    init.size = size;
    
    init.node_count = div_ceil_4tech(size, rope_string_width);

    if (init.node_count < 4){
        init.node_count = 7;
        init.rope_string_count = 4;
    }
    else{
        init.rope_string_count = round_pot_4tech(init.node_count);
        init.node_count = init.rope_string_count*2 - 1;
    }
    
    return(init);
}

internal_4tech int
buffer_init_need_more(Rope_Buffer_Init *init){
    Rope_Buffer *buffer;
    int result;
    buffer = init->buffer;
    result = 1;
    if (buffer->data != 0 && buffer->nodes != 0)
        result = 0;
    return(result);
}

inline_4tech int
buffer_init_page_size(Rope_Buffer_Init *init){
    Rope_Buffer *buffer;
    int result;
    buffer = init->buffer;
    if (buffer->data) result = init->node_count*sizeof(Rope_Node);
    else result = init->rope_string_count*rope_string_full_size;
    return(result);
}

internal_4tech void
buffer_init_provide_page(Rope_Buffer_Init *init, void *page, int page_size){
    Rope_Buffer *buffer;
    buffer = init->buffer;
    
    if (buffer->data){
        assert_4tech(buffer->nodes == 0);
        assert_4tech(page_size >= init->node_count*sizeof(Rope_Node));
        buffer->nodes = (Rope_Node*)page;
        init->node_count = page_size / sizeof(Rope_Node);
    }
    else{
        assert_4tech(page_size >= init->rope_string_count*rope_string_full_size);
        buffer->data = page;
        init->rope_string_count = page_size / rope_string_full_size;
    }
}

internal_4tech int
buffer_alloc_rope_string(Rope_Buffer *buffer, int *result){
    Rope_String *rope_string;
    int success;
    
    success = 0;
    if (buffer->free_rope_string >= 0){
        success = 1;
        *result = buffer->free_rope_string;
        rope_string = (Rope_String*)((char*)buffer->data + *result);
        buffer->free_rope_string = rope_string->next_free;
        *result += sizeof(Rope_String);
    }
    
    return(success);
}

internal_4tech void
buffer_free_rope_string(Rope_Buffer *buffer, int str_start){
    Rope_String *rope_string;

    str_start -= sizeof(Rope_String);
    rope_string = (Rope_String*)((char*)buffer->data + str_start);
    rope_string->next_free = buffer->free_rope_string;
    buffer->free_rope_string = str_start;
}

internal_4tech int
buffer_alloc_rope_node(Rope_Buffer *buffer, int *result){
    Rope_Node *node;
    int success;
    
    success = 0;
    if (buffer->free_rope_node > 0){
        success = 1;
        *result = buffer->free_rope_node;
        node = buffer->nodes + *result;
        buffer->free_rope_node = node->parent;
    }
    
    return(success);
}

internal_4tech void
buffer_free_rope_node(Rope_Buffer *buffer, int node_index){
    Rope_Node *node;

    node = buffer->nodes + node_index;
    node->parent = buffer->free_rope_node;
    buffer->free_rope_node = node_index;
}

inline_4tech void
buffer_node_check(Rope_Buffer *buffer, int node_index){
    Rope_Node *nodes, *node;
    nodes = buffer->nodes;
    node = nodes + node_index;
    assert_4tech(node->left == 0 || nodes[node->left].parent == node_index);
    assert_4tech(node->right == 0 || nodes[node->right].parent == node_index);
    assert_4tech((node->left == 0 && node->right == 0) || (node->left != 0 && node->right != 0));
}

internal_4tech void
buffer_cheap_check(Rope_Buffer *buffer){
    Rope_Node *nodes;
    nodes = buffer->nodes;
    
    assert_4tech(nodes->parent == 0);
    assert_4tech(nodes->right == 0);
    assert_4tech(nodes->left != 0);
    assert_4tech(nodes->weight == nodes->left_weight);
}

internal_4tech void
buffer_rope_check(Rope_Buffer *buffer, void *scratch, int scratch_size){
    int *int_stack;
    Rope_Node *node, *l, *r, *nodes;
    int top, max_stack;
    int node_index;

    nodes = buffer->nodes;
    int_stack = (int*)scratch;
    top = 0;
    max_stack = scratch_size / sizeof(int);
    buffer_cheap_check(buffer);
    
    int_stack[top++] = nodes->left;
    for (;top > 0;){
        node_index = int_stack[--top];
        node = nodes + node_index;
        if (node->left || node->right){
            assert_4tech(node->left && node->right);
            
            int_stack[top++] = node->left;
            int_stack[top++] = node->right;
            
            l = nodes + node->left;
            assert_4tech(l->parent == node_index);
            
            r = nodes + node->right;
            assert_4tech(r->parent == node_index);
            
            assert_4tech(l->weight + r->weight == node->weight);
            assert_4tech(l->weight == node->left_weight);
        }
    }
}

typedef struct Rope_Construct_Stage{
    int parent_index;
    int is_right_side;
    int weight;
} Rope_Construct_Stage;

inline_4tech Rope_Construct_Stage
buffer_construct_stage(int parent, int right, int weight){
    Rope_Construct_Stage result;
    result.parent_index = parent;
    result.is_right_side = right;
    result.weight = weight;
    return(result);
}

internal_4tech void
buffer_free_tree(Rope_Buffer *buffer, int node_index, void *scratch, int scratch_size){
    Rope_Node *nodes, *node;
    int *int_stack;
    int top, stack_max;
    
    int_stack = (int*)scratch;
    stack_max = scratch_size / sizeof(int);
    top = 0;
    
    nodes = buffer->nodes;
    int_stack[top++] = node_index;
    
    for (;top > 0;){
        node_index = int_stack[--top];
        node = nodes + node_index;
        assert_4tech(top < stack_max);
        if (node->left) int_stack[top++] = node->left;
        assert_4tech(top < stack_max);
        if (node->right) int_stack[top++] = node->right;
        if (node->str_start) buffer_free_rope_string(buffer, node->str_start);
        buffer_free_rope_node(buffer, node_index);
    }
}

internal_4tech int
buffer_build_tree(Rope_Buffer *buffer, char *str, int len, int root,
                  void *scratch, int scratch_size, int *request_amount){
    Rope_Construct_Stage *stack, *stage;
    Rope_Node *node, *nodes, *parent;
    char *dest;
    int top, stack_max;
    int result;
    int node_index;
    int is_right_side;
    int read_pos;

    nodes = buffer->nodes;
    stack = (Rope_Construct_Stage*)scratch;
    stack_max = scratch_size / sizeof(Rope_Construct_Stage);
    top = 0;
    result = 1;
    read_pos = 0;
    
    stack[top++] = buffer_construct_stage(root, 0, len);
    for (;top > 0;){
        stage = stack + (--top);
        
        if (buffer_alloc_rope_node(buffer, &node_index)){
            node = nodes + node_index;
            node->parent = stage->parent_index;
            node->weight = stage->weight;
            node->left = 0;
            node->right = 0;
            node->str_start = 0;
            
            is_right_side = stage->is_right_side;
            
            parent = nodes + node->parent;
            if (is_right_side) parent->right = node_index;
            else parent->left = node_index;
            
            if (stage->weight > rope_string_width){
                node->str_start = 0;
                node->left_weight = stage->weight / 2;
                assert_4tech(top < stack_max);
                stack[top++] = buffer_construct_stage(node_index, 1, node->weight - node->left_weight);
                assert_4tech(top < stack_max);
                stack[top++] = buffer_construct_stage(node_index, 0, node->left_weight);
            }
            else{
                node->left_weight = 0;
                if (buffer_alloc_rope_string(buffer, &node->str_start)){
                    dest = (char*)buffer->data + node->str_start;
                    assert_4tech(read_pos <= len);
                    memcpy_4tech(dest, str + read_pos, node->weight);
                    read_pos += node->weight;
                }
                else{
                    result = 0;
                    if (request_amount){
                        buffer->grow_string_memory = 1;
                        *request_amount = buffer->string_count*rope_string_full_size*2;
                    }
                    break;
                }
            }
        }
        else{
            result = 0;
            if (request_amount){
                buffer->grow_string_memory = 0;
                *request_amount = buffer->node_count*sizeof(Rope_Node)*2;
            }
            break;
        }
    }
    
    if (!result && request_amount){
        node = nodes + root;
        if (node->left){
            buffer_free_tree(buffer, node->left, scratch, scratch_size);
        }
    }
    
    return(result);
}

internal_4tech int
buffer_end_init(Rope_Buffer_Init *init, void *scratch, int scratch_size){
    Rope_Buffer *buffer;
    Rope_String *rope_string;
    Rope_Node *node;
    int i;
    int result;
    int count;
    
    result = 0;
    buffer = init->buffer;
    if (buffer->nodes && buffer->data){
        buffer->string_count = init->rope_string_count;
        buffer->free_rope_string = 0;
        
        rope_string = (Rope_String*)buffer->data;
        count = init->rope_string_count;
        for (i = 0; i < count-1; ++i){
            rope_string->next_free = rope_string_full_size*(i+1);
            rope_string = (Rope_String*)((char*)rope_string + rope_string_full_size);
        }
        rope_string->next_free = -1;

        buffer->node_count = init->node_count;
        buffer->free_rope_node = 1;
        
        node = buffer->nodes + 1;
        count = init->node_count;
        for (i = 1; i < count-1; ++i, ++node){
            node->parent = i+1;
        }
        node->parent = 0;
        
        result = 1;
        
        node = buffer->nodes;
        *node = {};
        node->weight = init->size;
        node->left_weight = init->size;

        result = buffer_build_tree(buffer, init->data, init->size, 0, scratch, scratch_size, 0);
    }
    
    return(result);
}

internal_4tech int
buffer_find_node(Rope_Buffer *buffer, int pos, int *node_start){
    Rope_Node *nodes, *node;
    
    *node_start = 0;
    nodes = buffer->nodes;
    node = nodes + nodes->left;
    for (;node->str_start == 0;){
        if (pos < node->left_weight){
            node = nodes + node->left;
        }
        else{
            *node_start += node->left_weight;
            pos -= node->left_weight;
            node = nodes + node->right;
        }
    }
    
    return (int)(node - nodes);
}

typedef struct Rope_Buffer_Stringify_Loop{
    Rope_Buffer *buffer;
    char *data;
    int absolute_pos;
    int next_absolute_pos;
    int size;
    int pos, end_pos;
    int node, node_end;
} Rope_Buffer_Stringify_Loop;

internal_4tech Rope_Buffer_Stringify_Loop
buffer_stringify_loop(Rope_Buffer *buffer, int start, int end){
    Rope_Buffer_Stringify_Loop result;
    Rope_Node *node;
    int size, node_start_pos, temp_end;

    size = buffer_size(buffer);
    if (0 <= start && start < end && end <= size){
        result.buffer = buffer;
        result.absolute_pos = start;
        
        result.node = buffer_find_node(buffer, start, &node_start_pos);
        result.pos = start - node_start_pos;
        
        result.node_end = buffer_find_node(buffer, end, &node_start_pos);
        result.end_pos = end - node_start_pos;

        node = buffer->nodes + result.node;
        temp_end = node->weight;
        if (result.node == result.node_end) temp_end = result.end_pos;
        
        result.data = (char*)buffer->data + node->str_start + result.pos;
        result.size = temp_end - result.pos;
    }
    else result.buffer = 0;
    return(result);
}

inline_4tech int
buffer_stringify_good(Rope_Buffer_Stringify_Loop *loop){
    int result;
    result = (loop->buffer != 0);
    return(result);
}

internal_4tech void
buffer_stringify_next(Rope_Buffer_Stringify_Loop *loop){
    Rope_Node *node, *child, *nodes;
    int temp_end;
    
    if (loop->node == loop->node_end && loop->pos + loop->size == loop->end_pos){
        loop->buffer = 0;
    }
    else{
        nodes = loop->buffer->nodes;
        node = nodes + loop->node;
        
        for (;;){
            assert_4tech(node->parent != 0);
            child = node;
            node = nodes + node->parent;
            if (nodes + node->left == child){
                break;
            }
            else{
                assert_4tech(nodes + node->right == child);
            }
        }
        
        node = nodes + node->right;
        
        for (;node->left;){
            node = nodes + node->left;
        }

        loop->pos = 0;
        loop->node = (int)(node - nodes);
        loop->absolute_pos += loop->size;
        temp_end = node->weight;
        if (loop->node == loop->node_end) temp_end = loop->end_pos;
        loop->size = temp_end;
        loop->data = (char*)loop->buffer->data + node->str_start;
    }
}

typedef struct Rope_Buffer_Backify_Loop{
    Rope_Buffer *buffer;
    char *data;
    int absolute_pos;
    int size;
    int pos, end_pos;
    int node, node_end;
} Rope_Buffer_Backify_Loop;

internal_4tech Rope_Buffer_Backify_Loop
buffer_backify_loop(Rope_Buffer *buffer, int start, int end){
    Rope_Buffer_Backify_Loop result;
    int size, node_start_pos, temp_end;

    size = buffer_size(buffer);
    ++start;
    if (0 <= end && end < start && start <= size){
        result.buffer = buffer;
        
        result.node_end = buffer_find_node(buffer, end, &node_start_pos);
        result.end_pos = end - node_start_pos;
        
        result.node = buffer_find_node(buffer, start, &node_start_pos);
        temp_end = start - node_start_pos;
        
        if (result.node_end == result.node){
            result.pos = result.end_pos;
            result.absolute_pos = end;
        }
        else{
            result.pos = 0;
            result.absolute_pos = node_start_pos;
        }
        
        result.size = temp_end - result.pos;
        result.data = (char*)buffer->data + buffer->nodes[result.node].str_start + result.pos;
    }
    else result.buffer = 0;
    return(result);
}

inline_4tech int
buffer_backify_good(Rope_Buffer_Backify_Loop *loop){
    int result;
    result = (loop->buffer != 0);
    return(result);
}

internal_4tech void
buffer_backify_next(Rope_Buffer_Backify_Loop *loop){
    Rope_Node *node, *child, *nodes;
    int temp_start;
    
    if (loop->node == loop->node_end && loop->pos == loop->end_pos){
        loop->buffer = 0;
    }
    else{
        nodes = loop->buffer->nodes;
        node = nodes + loop->node;
        
        for (;;){
            assert_4tech(node->parent != 0);
            child = node;
            node = nodes + node->parent;
            if (nodes + node->right == child){
                break;
            }
            else{
                assert_4tech(nodes + node->left == child);
            }
        }
        
        node = nodes + node->left;
        
        for (;node->right;){
            node = nodes + node->right;
        }

        loop->pos = 0;
        loop->node = (int)(node - nodes);
        loop->absolute_pos -= node->weight;
        temp_start = 0;
        if (loop->node == loop->node_end) temp_start = loop->end_pos;
        loop->size = node->weight - temp_start;
        loop->data = (char*)loop->buffer->data + node->str_start;
    }
}

internal_4tech void
buffer_rotate_right(Rope_Buffer *buffer, int *node_index){
    Rope_Node *nodes, *r, *l, *b;
    int ri, li, bi;
    
    nodes = buffer->nodes;
    ri = *node_index;
    r = nodes + ri;
    li = r->left;
    l = nodes + li;
    bi = l->right;
    b = nodes + bi;
    
    assert_4tech(bi != 0);
    assert_4tech(ri != 0);
    assert_4tech(li != 0);
    
    r->parent = li;
    l->right = ri;
    
    b->parent = ri;
    r->left = bi;
    
    r->weight = nodes[r->right].weight + nodes[r->left].weight;
    r->left_weight = nodes[r->left].weight;
    
    l->weight = nodes[l->right].weight + nodes[l->left].weight;
    l->left_weight = nodes[l->left].weight;
    
    *node_index = li;
}

internal_4tech void
buffer_rotate_left(Rope_Buffer *buffer, int *node_index){
    Rope_Node *nodes, *r, *l, *b;
    int ri, li, bi;
    
    nodes = buffer->nodes;
    li = *node_index;
    l = nodes + li;
    ri = l->right;
    r = nodes + ri;
    bi = r->left;
    b = nodes + bi;
    
    assert_4tech(bi != 0);
    assert_4tech(ri != 0);
    assert_4tech(li != 0);
    
    l->parent = ri;
    r->left = li;
    
    b->parent = li;
    l->right = bi;
    
    l->weight = nodes[l->right].weight + nodes[l->left].weight;
    l->left_weight = nodes[l->left].weight;
    
    r->weight = nodes[r->right].weight + nodes[r->left].weight;
    r->left_weight = nodes[r->left].weight;
    
    *node_index = ri;
}

internal_4tech int
buffer_merge(Rope_Buffer *buffer, int node_index){
    Rope_Node *nodes, *node, *node_a, *node_b;
    int a, b;
    int did_merge;

    assert_4tech(node_index != 0);
    nodes = buffer->nodes;
    node = nodes + node_index;
    did_merge = 0;

    if (node->weight <= rope_string_width && node->left != 0){
        assert_4tech(node->right != 0);
        
        a = node->left;
        b = node->right;

        node_a = nodes + a;
        node_b = nodes + b;

        if (node_a->str_start == 0) buffer_merge(buffer, a);
        if (node_b->str_start == 0) buffer_merge(buffer, b);
        
        did_merge = 1;
        memcpy_4tech((char*)buffer->data + node_a->str_start + node_a->weight,
                     (char*)buffer->data + node_b->str_start, node_b->weight);
        node->weight = node_a->weight + node_b->weight;
        node->left_weight = 0;
        node->left = node->right = 0;
        node->str_start = node_a->str_start;

        buffer_free_rope_node(buffer, a);
        buffer_free_rope_node(buffer, b);
    }

    return(did_merge);
}

internal_4tech int
buffer_balance(Rope_Buffer *buffer, int *root){
    Rope_Node *node, *a, *nodes;
    int wa, wb, wp, wdif, wdifp, left;
    int node_a;
    int improved;
    
    nodes = buffer->nodes;
    node = nodes + *root;

    improved = 0;
    if (node->left != 0){
        assert_4tech(node->right);
        
        if (!buffer_merge(buffer, *root)){
            wa = node->left_weight;
            wb = node->weight - wa;
            assert_4tech(wa + wb > rope_string_width);
        
            if (wa < wb){
                wp = wa;
                wa = wb;
                wb = wp;
                left = 0;
                node_a = node->right;
            }
            else{
                left = 1;
                node_a = node->left;
            }

            wdif = wa - wb;
            a = nodes + node_a;

            if (a->left != 0){
                assert_4tech(a->right != 0);
            
                wp = a->left_weight;
                if (left) wp = a->weight - wp;
                wdifp = wdif - 2*wp;
                if (wdifp < 0) wdifp = -wdifp;
                if (wdifp < wdif){
                    improved = 1;
                    if (left){
                        if (a->weight - a->left_weight > a->left_weight && a->right != 0 && nodes[a->right].left != 0){
                            buffer_rotate_left(buffer, &node_a);
                            a = nodes + node_a;
                            a->parent = *root;
                            node->left = node_a;
                        }
                        buffer_rotate_right(buffer, root);
                    }
                    else{
                        if (a->left_weight > a->weight - a->left_weight && a->left != 0 && nodes[a->left].right != 0){
                            buffer_rotate_right(buffer, &node_a);
                            a = nodes + node_a;
                            a->parent = *root;
                            node->right = node_a;
                        }
                        buffer_rotate_left(buffer, root);
                    }
                }
            }
        }
    }

    return(improved);
}

internal_4tech int
buffer_concat(Rope_Buffer *buffer, int node_a, int node_b, int *root, int *request_amount){
    Rope_Node *r, *a, *b, *nodes;
    int result;
    
    result = 0;
    
    nodes = buffer->nodes;
    a = nodes + node_a;
    b = nodes + node_b;

    if (a->weight == 0){
        assert_4tech(a->left == 0 && a->right == 0);
        *root = node_b;
        buffer_free_rope_node(buffer, node_a);
    }
    else if (b->weight == 0){
        assert_4tech(b->left == 0 && b->right == 0);
        *root = node_a;
        buffer_free_rope_node(buffer, node_b);
    }
    else if (buffer_alloc_rope_node(buffer, root)){
        r = nodes + *root;
        
        r->left = node_a;
        r->right = node_b;
        r->weight = a->weight + b->weight;
        r->left_weight = a->weight;
        r->str_start = 0;
        r->parent = 0;
        
        a->parent = *root;
        b->parent = *root;

        buffer_node_check(buffer, node_a);
        buffer_node_check(buffer, node_b);
    }
    else{
        result = 1;
        buffer->grow_string_memory = 0;
        *request_amount = buffer->node_count*sizeof(Rope_Node)*2;
    }
    
    return(result);
}

internal_4tech int
buffer_string_split(Rope_Buffer *buffer, int *node_index, int pos, int *request_amount){
    Rope_Node *node, *a, *b;
    int node_a, node_b;
    int new_string_start, string_start;
    int result;
    result = 0;

    if (pos > 0){
        if (buffer_alloc_rope_string(buffer, &new_string_start)){
            if (buffer_alloc_rope_node(buffer, &node_a)){
                if (buffer_alloc_rope_node(buffer, &node_b)){
                    node = buffer->nodes + *node_index;
                    string_start = node->str_start;
                    memcpy_4tech((char*)buffer->data + new_string_start, (char*)buffer->data + string_start + pos, node->weight - pos);
                    node->str_start = 0;
                    node->left_weight = pos;
                    node->left = node_a;
                    node->right = node_b;
                    
                    a = buffer->nodes + node_a;
                    b = buffer->nodes + node_b;
                    
                    a->parent = *node_index;
                    a->left = 0;
                    a->right = 0;
                    a->left_weight = 0;
                    a->weight = pos;
                    a->str_start = string_start;
                    
                    b->parent = *node_index;;
                    b->left = 0;
                    b->right = 0;
                    b->left_weight = 0;
                    b->weight = node->weight - pos;
                    b->str_start = new_string_start;
                    
                    *node_index = node_b;
                }
                else{
                    buffer_free_rope_string(buffer, new_string_start);
                    buffer_free_rope_node(buffer, node_a);
                    result = 1;
                    buffer->grow_string_memory = 0;
                    *request_amount = buffer->node_count*sizeof(Rope_Node)*2;                    
                }
            }
            else{
                buffer_free_rope_string(buffer, new_string_start);
                result = 1;
                buffer->grow_string_memory = 0;
                *request_amount = buffer->node_count*sizeof(Rope_Node)*2;                
            }
        }
        else{
            result = 1;
            buffer->grow_string_memory = 1;
            *request_amount = buffer->string_count*rope_string_full_size*2;
        }
    }
    
    return(result);
}

internal_4tech void
buffer_reduce_node(Rope_Buffer *buffer, int node_index){
    Rope_Node *parent, *child, *node, *nodes;
    int child_index;
    
    nodes = buffer->nodes;
    node = nodes + node_index;
    parent = nodes + node->parent;
    child_index = node->left;
    if (child_index == 0) child_index = node->right;
    assert_4tech(child_index != 0);
    child = nodes + child_index;
    
    if (parent->left == node_index) parent->left = child_index;
    else parent->right = child_index;
    child->parent = node->parent;
    
    for (;parent != nodes;){
        parent->weight = nodes[parent->left].weight + nodes[parent->right].weight;
        parent->left_weight = nodes[parent->left].weight;
        parent = nodes + parent->parent;
    }
    
    parent->weight = parent->left_weight = nodes[parent->left].weight;
    
    buffer_free_rope_node(buffer, node_index);
}

internal_4tech int
buffer_split(Rope_Buffer *buffer, int pos, int *node_a, int *node_b, int *request_amount){
    Rope_Node *node, *nodes, *child;
    int node_index, node_start_pos, split_root;
    int result;
    debug_4tech(int dbg_check);

    nodes = buffer->nodes;
    result = 0;

    assert_4tech(pos != 0);
    
    node_index = buffer_find_node(buffer, pos, &node_start_pos);
    if (node_start_pos < pos){
        if (buffer_string_split(buffer, &node_index, pos - node_start_pos, request_amount)){
            result = 1;
            goto buffer_split_end;
        }
    }
    
    split_root = 0;
    node = nodes + node_index;
    for (;;){
        child = node;
        node = nodes + node->parent;
        if (child == nodes + node->right){
            break;
        }
        else{
            assert_4tech(child == nodes + node->left);
        }
    }
    assert_4tech(node != nodes);
    
    for (;;){
        if (split_root == 0){
            split_root = node->right;
        }
        else{
            debug_4tech(dbg_check =)
                buffer_concat(buffer, split_root, node->right, &split_root, request_amount);
            assert_4tech(!dbg_check);
        }
        
        node_index = (int)(node - nodes);
        node->right = 0;
        node = nodes + node->left;
        buffer_reduce_node(buffer, node_index);

        for (;;){
            child = node;
            node = nodes + child->parent;
            if (nodes + node->left == child){
                break;
            }
            else{
                assert_4tech(nodes + node->right == child);
            }
        }
        
        if (node == nodes) break;
    }
    
    *node_a = nodes->left;
    *node_b = split_root;
    
buffer_split_end:
    return(result);
}

internal_4tech int
buffer_build_tree_floating(Rope_Buffer *buffer, char *str, int len, int *out,
                           void *scratch, int scratch_size, int *request_amount){
    int result;
    int super_root;

    result = 0;
    if (buffer_alloc_rope_node(buffer, &super_root)){
        buffer->nodes[super_root] = {};
        if (buffer_build_tree(buffer, str, len, super_root, scratch, scratch_size, request_amount)){
            *out = buffer->nodes[super_root].left;
            buffer_free_rope_node(buffer, super_root);
        }
        else{
            result = 1;
            buffer_free_rope_node(buffer, super_root);
        }
    }
    else{
        result = 1;
        buffer->grow_string_memory = 0;
        *request_amount = buffer->node_count*sizeof(Rope_Node)*2;
    }
    
    return(result);
}

internal_4tech int
buffer_replace_range(Rope_Buffer *buffer, int start, int end, char *str, int len, int *shift_amount,
                     void *scratch, int scratch_size, int *request_amount){
    Rope_Node *nodes;
    Rope_Buffer_Edit_State state;
    int result;
    int size;

    state = buffer->edit_state;

    size = buffer_size(buffer);
    assert_4tech(0 <= start);
    assert_4tech(start <= end);
    assert_4tech(end <= size);
    
    *shift_amount = (len - (end - start));
    nodes = buffer->nodes;
    result = 0;

    buffer_cheap_check(buffer);
    for (; buffer->edit_stage < 9; ++buffer->edit_stage){
        buffer_cheap_check(buffer);
        switch (buffer->edit_stage){
        case 0:
            if (end == 0){
                state.throw_away = 0;
                state.right = nodes->left;
            }
            else if (end == size){
                state.throw_away = nodes->left;
                state.right = 0;
            }
            else{
                result = buffer_split(buffer, end, &state.throw_away, &state.right, request_amount);
            }
            if (state.right) buffer_node_check(buffer, state.right);
            break;
    
        case 1:
            if (start == 0){
                state.left = 0;
            }
            else if (start == end){
                state.left = state.throw_away;
                state.throw_away = 0;
            }
            else{
                result = buffer_split(buffer, start, &state.left, &state.throw_away, request_amount);
            }
            if (state.left) buffer_node_check(buffer, state.left);
            break;

        case 2:
            if (state.throw_away){
                buffer_free_tree(buffer, state.throw_away, scratch, scratch_size);
            }
            break;

        case 3:
            if (len == 0) buffer->edit_stage += 4;
            break;

        case 4:
            if (state.left) buffer_node_check(buffer, state.left);
            if (state.right) buffer_node_check(buffer, state.right);
            result = buffer_build_tree_floating(buffer, str, len, &state.middle, scratch, scratch_size, request_amount);
            if (state.left) buffer_node_check(buffer, state.left);
            if (state.right) buffer_node_check(buffer, state.right);
            break;

        case 5:
            if (state.left){
                result = buffer_concat(buffer, state.left, state.middle, &state.middle, request_amount);
            }
            break;

        case 6:
            if (state.right){
                buffer_balance(buffer, &state.middle);
                buffer_node_check(buffer, state.middle);
                result = buffer_concat(buffer, state.middle, state.right, &state.middle, request_amount);
            }
            break;
            
        case 7:
            buffer_balance(buffer, &state.middle);
            buffer_node_check(buffer, state.middle);
            buffer->edit_stage = 9;
            break;

        case 8:
            if (state.left && state.right){
                result = buffer_concat(buffer, state.left, state.right, &state.middle, request_amount);
            }
            else if (state.left){
                state.middle = state.left;
            }
            else{
                state.middle = state.right;
            }
            break;
        }
        buffer_cheap_check(buffer);
        
        if (result) goto rope_buffer_replace_range_end;
    }
    
    buffer->edit_stage = 0;
    nodes[state.middle].parent = 0;
    nodes->left = state.middle;
    nodes->weight = nodes->left_weight = nodes[state.middle].weight;
    
    state = {};
    
    buffer_rope_check(buffer, scratch, scratch_size);
    
rope_buffer_replace_range_end:
    buffer->edit_state = state;
    
    return(result);
}

// NOTE(allen): This could should be optimized for Gap_Buffer
internal_4tech int
buffer_batch_edit_step(Buffer_Batch_State *state, Rope_Buffer *buffer, Buffer_Edit *sorted_edits,
                       char *strings, int edit_count, void *scratch, int scratch_size, int *request_amount){
    Buffer_Edit *edit;
    int i, result;
    int shift_total, shift_amount;
    
    result = 0;
    shift_total = state->shift_total;
    i = state->i;
    
    edit = sorted_edits + i;
    for (; i < edit_count; ++i, ++edit){
        result = buffer_replace_range(buffer, edit->start + shift_total, edit->end + shift_total,
                                      strings + edit->str_start, edit->len, &shift_amount,
                                      scratch, scratch_size, request_amount);
        if (result) break;
        shift_total += shift_amount;
    }
    
    state->shift_total = shift_total;
    state->i = i;
    
    return(result);
}

internal_4tech void*
buffer_edit_provide_memory(Rope_Buffer *buffer, void *new_data, int size){
    void *result;
    Rope_String *rope_string;
    Rope_Node *node;
    int start, end, i;
    
    if (buffer->grow_string_memory){
        assert_4tech(size >= buffer->string_count * rope_string_full_size);
        result = buffer->data;
        memcpy_4tech(new_data, buffer->data, buffer->string_count * rope_string_full_size);

        buffer->data = new_data;
        start = buffer->string_count*rope_string_full_size;
        end = size/rope_string_full_size;
        buffer->string_count = end;
        end = (end-1)*rope_string_full_size;
        for (i = start; i < end; i += rope_string_full_size){
            rope_string = (Rope_String*)((char*)new_data + i);
            rope_string->next_free = i + rope_string_full_size;
        }
        rope_string = (Rope_String*)((char*)new_data + i);
        rope_string->next_free = buffer->free_rope_string;
        buffer->free_rope_string = start;
    }
    else{
        assert_4tech(size >= buffer->node_count * sizeof(Rope_Node));
        result = buffer->nodes;
        memcpy_4tech(new_data, buffer->nodes, buffer->node_count * sizeof(Rope_Node));
        
        buffer->nodes = (Rope_Node*)new_data;
        start = buffer->node_count;
        end = size/sizeof(Rope_Node);
        buffer->node_count = end;
        end -= 1;
        node = buffer->nodes + start;
        for (i = start; i < end; ++i, ++node){
            node->parent = i+1;
        }
        node->parent = buffer->free_rope_node;
        buffer->free_rope_node = start;
    }
    
    return(result);
}

// BOTTOM




