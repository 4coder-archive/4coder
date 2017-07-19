/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 18.07.2017
 *
 * Shared logic for 4coder initialization.
 *
 */

// TOP

internal void
system_memory_init(){
#if defined(FRED_INTERNAL)
# if defined(FTECH_64_BIT)
    void *bases[] = { (void*)TB(1), (void*)TB(2), };
# elif defined(FTECH_32_BIT)
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
    
    if (!alloc_success){
        char msg[] = "Could not allocate sufficient memory. Please make sure you have atleast 512Mb of RAM free. (This requirement will be relaxed in the future).";
        system_error_box(msg);
    }
}

internal void
load_app_code(){
    App_Get_Functions *get_funcs = 0;
    
    if (system_load_library(&libraries.app_code, "4ed_app")){
        get_funcs = (App_Get_Functions*)system_get_proc(&libraries.app_code, "app_get_functions");
    }
    else{
        char msg[] = "Could not load '4ed_app." DLL "'. This file should be in the same directory as the main '4ed' executable.";
        system_error_box(msg);
    }
    
    if (get_funcs != 0){
        app = get_funcs();
    }
    else{
        char msg[] = "Failed to get application code from '4ed_app." DLL "'.";
        system_error_box(msg);
    }
}

// BOTTOM

