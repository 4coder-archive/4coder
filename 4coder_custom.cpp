// Set to Custom_None if you're going to drop in your own include/get_bindings call.
// or choose one of the preexisting customizations
#define Custom_Current Custom_Default

#define Custom_None -1
#define Custom_Default 0 

// The following customization schemes are power users only:
#define Custom_HandmadeHero 1


// TOP

#if Custom_Current == Custom_Default
# include "4coder_default_bindings.cpp"
#elif Custom_Current == Custom_HandmadeHero
# include "power/4coder_handmade_hero.cpp"
#endif

extern "C" GET_BINDING_DATA(get_bindings){
    Bind_Helper context_actual = begin_bind_helper(data, size);
    Bind_Helper *context = &context_actual;
    
#if Custom_Current == Custom_Default
    default_get_bindings(context);
#elif Custom_Current == Custom_HandmadeHero
    casey_get_bindings(context);
#endif
    
    end_bind_helper(context);
    return context->write_total;
}

// BOTTOM
