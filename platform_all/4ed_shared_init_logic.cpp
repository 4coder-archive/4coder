/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 18.07.2017
 *
 * Shared logic for 4coder initialization.
 *
 */

// TOP

internal b32
system_memory_init(){
#if defined(FRED_INTERNAL)
# if defined(BUILD_X64)
    void *bases[] = { (void*)TB(1), (void*)TB(2), };
# elif defined(BUILD_X86)
    void *bases[] = { (void*)MB(96), (void*)MB(98), };
# endif
#else
    void *bases[] = { (void*)0, (void*)0, };
#endif
    
    memory_vars.vars_memory_size = MB(2);
    memory_vars.vars_memory = system_memory_allocate_extended(bases[0], memory_vars.vars_memory_size);
    memory_vars.target_memory_size = MB(512);
    memory_vars.target_memory = system_memory_allocate_extended(bases[1], memory_vars.target_memory_size);
    memory_vars.user_memory_size = MB(2);
    memory_vars.user_memory = system_memory_allocate_extended(0, memory_vars.user_memory_size);
    memory_vars.debug_memory_size = MB(512);
    memory_vars.debug_memory = system_memory_allocate_extended(0, memory_vars.debug_memory_size);
    target.max = MB(1);
    target.push_buffer = (char*)system_memory_allocate(target.max);
    
    b32 alloc_success = true;
    if (memory_vars.vars_memory == 0 || memory_vars.target_memory == 0 || memory_vars.user_memory == 0 || target.push_buffer == 0){
        alloc_success = false;
    }
    
    return(alloc_success);
}

// BOTTOM

