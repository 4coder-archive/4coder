/*
* Mr. 4th Dimention - Allen Webster
*
* 07.02.2019
*
* Memory bank wrapper for heap
*
*/

// TOP

internal void
memory_bank_init(Memory_Bank *mem_bank){
    heap_init(&mem_bank->heap);
    mem_bank->first = 0;
    mem_bank->last = 0;
    mem_bank->total_memory_size = 0;
}

internal void*
memory_bank_allocate(Heap *heap, Memory_Bank *mem_bank, i32 size){
    void *ptr = heap_allocate(&mem_bank->heap, size);
    if (ptr == 0){
        i32 alloc_size = clamp_bot(4096, size*4 + sizeof(Memory_Header));
        void *new_block = heap_allocate(heap, alloc_size);
        if (new_block != 0){
            Memory_Header *header = (Memory_Header*)new_block;
            sll_queue_push(mem_bank->first, mem_bank->last, header);
            mem_bank->total_memory_size += alloc_size;
            heap_extend(&mem_bank->heap, header + 1, alloc_size - sizeof(*header));
            ptr = heap_allocate(&mem_bank->heap, size);
        }
    }
    return(ptr);
}

internal void
memory_bank_free(Memory_Bank *mem_bank, void *ptr){
    heap_free(&mem_bank->heap, ptr);
}

internal void
memory_bank_free_all(Heap *heap, Memory_Bank *mem_bank){
    for (Memory_Header *header = mem_bank->first, *next = 0;
         header != 0;
         header = next){
        next = header->next;
        heap_free(heap, header);
    }
    mem_bank->total_memory_size = 0;
}

// BOTTOM

