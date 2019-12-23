int
main(int arg_count, char **args){
    Thread_Context _tctx = {};
    thread_ctx_init(&_tctx, ThreadKind_Main,
                    get_base_allocator_system(),
                    get_base_allocator_system());
    
    block_zero_struct(&global_mac_vars);
    global_mac_vars.tctx = &_tctx;
    
    // NOTE(yuval): Application Core Update
    Application_Step_Result result = {};
    if (app.step != 0){
        result = app.step(mac_vars.tctx, &target, base_ptr, &input);
    }
}