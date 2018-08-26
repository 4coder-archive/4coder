/*
4coder_heap.cpp - Preversioning
no warranty implied; use at your own risk

This software is in the public domain. Where that dedication is not
recognized, you are granted a perpetual, irrevocable license to copy,
distribute, and modify this file as you see fit.
*/

// TOP

#define heap__sent_init(s) (s)->next=(s)->prev=(s)
#define heap__insert_next(p,n) ((n)->next=(p)->next,(n)->prev=(p),(n)->next->prev=(n),(p)->next=(n))
#define heap__insert_prev(p,n) ((n)->prev=(p)->prev,(n)->next=(p),(n)->prev->next=(n),(p)->prev=(n))
#define heap__remove(n) ((n)->next->prev=(n)->prev,(n)->prev->next=(n)->next)

#if defined(DO_HEAP_CHECKS)
static void
heap_assert_good(Heap *heap){
    if (heap->in_order.next != 0){
        Assert(heap->in_order.prev != 0);
        Assert(heap->free_nodes.next != 0);
        Assert(heap->free_nodes.prev != 0);
        for (Heap_Basic_Node *node = &heap->in_order;;){
            Assert(node->next->prev == node);
            Assert(node->prev->next == node);
            node = node->next;
            if (node == &heap->in_order){
                break;
            }
        }
        for (Heap_Basic_Node *node = &heap->free_nodes;;){
            Assert(node->next->prev == node);
            Assert(node->prev->next == node);
            node = node->next;
            if (node == &heap->free_nodes){
                break;
            }
        }
    }
}
#else
#define heap_assert_good(heap) ((void)(heap))
#endif

static void
heap_init(Heap *heap){
    heap__sent_init(&heap->in_order);
    heap__sent_init(&heap->free_nodes);
}

static void
heap_extend(Heap *heap, void *memory, i32_4tech size){
    heap_assert_good(heap);
    if (size >= sizeof(Heap_Node)){
        Heap_Node *new_node = (Heap_Node*)memory;
        heap__insert_prev(&heap->in_order, &new_node->order);
        heap__insert_next(&heap->free_nodes, &new_node->alloc);
        new_node->size = size - sizeof(*new_node);
    }
    heap_assert_good(heap);
}

static void*
heap__reserve_chunk(Heap_Node *node, i32_4tech size){
    u8_4tech *ptr = (u8_4tech*)(node + 1);
    i32_4tech left_over_size = node->size - size;
    if (left_over_size > sizeof(*node)){
        i32_4tech new_node_size = left_over_size - sizeof(*node);
        Heap_Node *new_node = (Heap_Node*)(ptr + size);
        heap__insert_next(&node->order, &new_node->order);
        heap__insert_next(&node->alloc, &new_node->alloc);
        new_node->size = new_node_size;
    }
    heap__remove(&node->alloc);
    node->alloc.next = 0;
    node->alloc.prev = 0;
    node->size = size;
    return(ptr);
}

static void*
heap_allocate(Heap *heap, i32_4tech size){
    if (heap->in_order.next != 0){
        heap_assert_good(heap);
        i32_4tech aligned_size = (size + sizeof(Heap_Node) - 1);
        aligned_size = aligned_size - (aligned_size%sizeof(Heap_Node));
        for (Heap_Basic_Node *n = heap->free_nodes.next;
             n != &heap->free_nodes;
             n = n->next){
            Heap_Node *node = CastFromMember(Heap_Node, alloc, n);
            if (node->size >= aligned_size){
                void *ptr = heap__reserve_chunk(node, aligned_size);
                heap_assert_good(heap);
                return(ptr);
            }
        }
        heap_assert_good(heap);
    }
    return(0);
}

static void
heap__merge(Heap *heap, Heap_Node *l, Heap_Node *r){
    if (&l->order != &heap->in_order && &r->order != &heap->in_order &&
        l->alloc.next != 0 && l->alloc.prev != 0 &&
        r->alloc.next != 0 && r->alloc.prev != 0){
        u8_4tech *ptr = (u8_4tech*)(l + 1) + l->size;
        if (PtrDif(ptr, r) == 0){
            heap__remove(&r->order);
            heap__remove(&r->alloc);
            heap__remove(&l->alloc);
            l->size += r->size + sizeof(*r);
            heap__insert_next(&heap->free_nodes, &l->alloc);
        }
    }
}

static void
heap_free(Heap *heap, void *memory){
    if (heap->in_order.next != 0 && memory != 0){
        Heap_Node *node = ((Heap_Node*)memory) - 1;
        Assert(node->alloc.next == 0);
        Assert(node->alloc.prev == 0);
        heap_assert_good(heap);
        heap__insert_next(&heap->free_nodes, &node->alloc);
        heap_assert_good(heap);
        heap__merge(heap, node, CastFromMember(Heap_Node, order, node->order.next));
        heap_assert_good(heap);
        heap__merge(heap, CastFromMember(Heap_Node, order, node->order.prev), node);
        heap_assert_good(heap);
    }
}

#define heap_array(g, T, size) (T*)heap_allocate(g, sizeof(T)*(size))

// BOTTOM

