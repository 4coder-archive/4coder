// Delete CustomCurrent to define your own customizations or
// set it to one of the preexisting options
#define Custom_Current Custom_Default

#define Custom_Default 1

// The following customization schemes are power users only:
#define Custom_HandmadeHero 2


// TOP

#if Custom_Current == Custom_Default
# include "4coder_default_bindings.cpp"
#elif Custom_Current == Custom_HandmadeHero
# include "power/4coder_casey.cpp"
#endif

extern "C" GET_BINDING_DATA(get_bindings){
    Bind_Helper context_actual = begin_bind_helper(data, size);
    Bind_Helper *context = &context_actual;
    
#if Custom_Current == Custom_Default
    default_get_bindings(context, true);
#elif Custom_Current == Custom_HandmadeHero
    casey_get_bindings(context);
#endif
    
    end_bind_helper(context);
    return context->write_total;
}

// BOTTOM
