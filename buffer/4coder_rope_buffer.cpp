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
    int left_total, total;
    int str_start;
} Rope_Node;

typedef struct Rope_String{
    int next_free;
} Rope_String;

#define rope_string_full_size 256
#define rope_string_width (rope_string_full_size-sizeof(Rope_String))

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
    size = buffer->nodes->left_total;
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

internal_4tech int
buffer_end_init(Rope_Buffer_Init *init, void *scratch, int scratch_size){
    Rope_Construct_Stage *stack, *stage;
    Rope_Buffer *buffer;
    Rope_String *rope_string;
    Rope_Node *node;
    char *src, *dest;
    int node_index;
    int i, top, stack_max, is_right_side;
    int read_pos, read_end;
    int result;
    int count;
    
    src = init->data;
    read_pos = 0;
    read_end = init->size;
    
    result = 0;
    buffer = init->buffer;
    if (buffer->nodes && buffer->data){
        // NOTE(allen): initialize free lists
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
        for (i = 1; i < count; ++i, ++node){
            node->parent = i+1;
        }
        node->parent = 0;
        
        result = 1;
        
        // NOTE(allen): initialize tree
        node = buffer->nodes;
        node->parent = 0;
        node->total = init->size;
        node->left_total = init->size;
        
        stack = (Rope_Construct_Stage*)scratch;
        stack_max = scratch_size / sizeof(Rope_Construct_Stage);
        top = 0;
        
        stack[top++] = buffer_construct_stage(0, 0, init->size);
        for (;top > 0;){
            stage = stack + (--top);
            
            if (buffer_alloc_rope_node(buffer, &node_index)){
                node = buffer->nodes + node_index;
                node->parent = stage->parent_index;
                node->total = stage->weight;
                is_right_side = stage->is_right_side;
                if (stage->weight > rope_string_width){
                    node->left_total = stage->weight / 2;
                    assert_4tech(top < stack_max);
                    stack[top++] = buffer_construct_stage(node_index, 1, node->total - node->left_total);
                    assert_4tech(top < stack_max);
                    stack[top++] = buffer_construct_stage(node_index, 0, node->left_total);
                }
                else{
                    node->left_total = 0;
                    node->left = 0;
                    node->right = 0;
                    if (buffer_alloc_rope_string(buffer, &node->str_start)){
                        dest = (char*)buffer->data + node->str_start;
                        assert_4tech(read_pos < read_end);
                        memcpy_4tech(dest, src + read_pos, node->total);
                        read_pos += node->total;
                    }
                    else{
                        result = 0;
                        break;
                    }
                }
                node = buffer->nodes + node->parent;
                if (is_right_side) node->right = node_index;
                else node->left = node_index;
            }
            else{
                result = 0;
                break;
            }
        }
    }
    
    assert_4tech(!result || read_pos == read_end);
    
    return(result);
}

// BOTTOM


