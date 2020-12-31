/*
4coder_delta_rule.cpp - Built in delta rules and delta rule helpers.
*/

// TOP

function u64
delta_ctx_size(u64 base_size){
    return(base_size + sizeof(Delta_Context_Header));
}

function Delta_Context_Header*
delta_ctx_get_header(String_Const_u8 delta_ctx){
    return((Delta_Context_Header*)delta_ctx.str);
}

function void*
delta_ctx_get_user_data(String_Const_u8 delta_ctx){
    Delta_Context_Header *ctx = (Delta_Context_Header*)delta_ctx.str;
    return(ctx + 1);
}

function Buffer_Point_Delta_Result
delta_apply(Application_Links *app, View_ID view,
            Delta_Rule_Function *func, String_Const_u8 delta_ctx,
            f32 dt, Buffer_Point position, Buffer_Point target){
    Buffer_Point_Delta_Result result = {};
    Vec2_f32 pending = view_point_difference(app, view, target, position);
    if (!near_zero(pending, 0.5f)){
        Delta_Context_Header *ctx = delta_ctx_get_header(delta_ctx);
        b32 is_new_target = false;
        if (!block_match_struct(&ctx->point, &target)){
            block_copy_struct(&ctx->point, &target);
            is_new_target = true;
        }
        void *rule_data = delta_ctx_get_user_data(delta_ctx);
        Vec2_f32 partial = func(pending, is_new_target, dt, rule_data);
        
        // NOTE(allen): clamp partial into the box from the origin
        // to the pending delta.
        Range_f32 x = If32(pending.x, 0.f);
        Range_f32 y = If32(pending.y, 0.f);
        partial.x = clamp_range(x, partial.x);
        partial.y = clamp_range(y, partial.y);
        
        result.point = view_move_buffer_point(app, view, position, partial);
        result.still_animating = true;
    }
    else{
        result.point = target;
    }
    return(result);
}

function Buffer_Point_Delta_Result
delta_apply(Application_Links *app, View_ID view,
            Delta_Rule_Function *func, String_Const_u8 delta_ctx,
            f32 dt, Buffer_Scroll scroll){
    return(delta_apply(app, view, func, delta_ctx,
                       dt, scroll.position, scroll.target));
}

function Buffer_Point_Delta_Result
delta_apply(Application_Links *app, View_ID view,
            f32 dt, Buffer_Point position, Buffer_Point target){
    View_Context ctx = view_current_context(app, view);
    String_Const_u8 delta_ctx = view_current_context_hook_memory(app, view, HookID_DeltaRule);
    return(delta_apply(app, view, ctx.delta_rule, delta_ctx,
                       dt, position, target));
}

function Buffer_Point_Delta_Result
delta_apply(Application_Links *app, View_ID view,
            f32 dt, Buffer_Scroll scroll){
    View_Context ctx = view_current_context(app, view);
    String_Const_u8 delta_ctx = view_current_context_hook_memory(app, view, HookID_DeltaRule);
    return(delta_apply(app, view, ctx.delta_rule, delta_ctx,
                       dt, scroll.position, scroll.target));
}

function Vec2_f32_Delta_Result
delta_apply(Application_Links *app, View_ID view,
            Delta_Rule_Function *func, String_Const_u8 delta_ctx,
            f32 dt, Vec2_f32 position, Vec2_f32 target){
    Vec2_f32_Delta_Result result = {};
    Vec2_f32 pending = target - position;
    if (!near_zero(pending, 0.5f)){
        Delta_Context_Header *ctx = delta_ctx_get_header(delta_ctx);
        b32 is_new_target = false;
        if (!near_zero(ctx->p - target, 0.1f)){
            block_copy_struct(&ctx->p, &target);
            is_new_target = true;
        }
        void *rule_data = delta_ctx_get_user_data(delta_ctx);
        Vec2_f32 partial = func(pending, is_new_target, dt, rule_data);
        
        // NOTE(allen): clamp partial into the box from the origin
        // to the pending delta.
        Range_f32 x = If32(pending.x, 0.f);
        Range_f32 y = If32(pending.y, 0.f);
        partial.x = clamp_range(x, partial.x);
        partial.y = clamp_range(y, partial.y);
        
        result.p = position + partial;
        result.still_animating = true;
    }
    else{
        result.p = target;
    }
    return(result);
}

function Vec2_f32_Delta_Result
delta_apply(Application_Links *app, View_ID view,
            Delta_Rule_Function *func, String_Const_u8 delta_ctx,
            f32 dt, Basic_Scroll scroll){
    return(delta_apply(app, view, func, delta_ctx,
                       dt, scroll.position, scroll.target));
}

function Vec2_f32_Delta_Result
delta_apply(Application_Links *app, View_ID view,
            f32 dt, Vec2_f32 position, Vec2_f32 target){
    View_Context ctx = view_current_context(app, view);
    String_Const_u8 delta_ctx = view_current_context_hook_memory(app, view, HookID_DeltaRule);
    return(delta_apply(app, view, ctx.delta_rule, delta_ctx,
                       dt, position, target));
}

function Vec2_f32_Delta_Result
delta_apply(Application_Links *app, View_ID view,
            f32 dt, Basic_Scroll scroll){
    View_Context ctx = view_current_context(app, view);
    String_Const_u8 delta_ctx = view_current_context_hook_memory(app, view, HookID_DeltaRule);
    return(delta_apply(app, view, ctx.delta_rule, delta_ctx,
                       dt, scroll.position, scroll.target));
}

////////////////////////////////

internal Smooth_Step
smooth_camera_step(f32 target, f32 v, f32 S, f32 T){
    Smooth_Step step = {};
    step.v = v;
    if (step.p != target){
        if (step.p > target - .1f && step.p < target + .1f){
            step.p = target;
            step.v = 1.f;
        }
        else{
            f32 L = step.p + T*(target - step.p);
            i32 sign = (target > step.p) - (target < step.p);
            f32 V = step.p + sign*step.v;
            if (sign > 0){
                step.p = (L<V)?(L):(V);
            }
            else{
                step.p = (L>V)?(L):(V);
            }
            if (step.p == V){
                step.v *= S;
            }
        }
    }
    return(step);
}
DELTA_RULE_SIG(original_delta){
    Vec2_f32 *velocity = (Vec2_f32*)data;
    if (velocity->x == 0.f){
        velocity->x = 1.f;
        velocity->y = 1.f;
    }
    Smooth_Step step_x = smooth_camera_step(pending.x, velocity->x, 80.f, 1.f/2.f);
    Smooth_Step step_y = smooth_camera_step(pending.y, velocity->y, 80.f, 1.f/2.f);
    *velocity = V2f32(step_x.v, step_y.v);
    return(V2f32(step_x.p, step_y.p));
}
global_const u64 original_delta_memory_size = sizeof(Vec2_f32);

DELTA_RULE_SIG(snap_delta){
    return(pending);
}
global_const u64 snap_delta_memory_size = 0;

function f32
cubic_reinterpolate(f32 t){
    f32 t2 = t*t;
    f32 t3 = t2*t;
    return(3*t2 - 2*t3);
}
DELTA_RULE_SIG(fixed_time_cubic_delta){
    local_const f32 duration_in_seconds = (1.f/8.f);
    local_const f32 dt_multiplier = 1.f/duration_in_seconds;
    f32 step = dt*dt_multiplier;
    f32 *t = (f32*)data;
    *t = clamp(0.f, *t, 1.f);
    f32 prev_t = *t;
    if (is_new_target){
        prev_t = 0.f;
        *t = step;
    }
    else{
        *t += step;
    }
    *t = clamp(0.f, *t, 1.f);
    Vec2_f32 result = pending;
    if (*t < 1.f){
        f32 prev_x = cubic_reinterpolate(prev_t);
        f32 x = cubic_reinterpolate(*t);
        f32 portion = ((x - prev_x)/(1.f - prev_x));
        result *= portion;
    }
    return(result);
}
global_const u64 fixed_time_cubic_delta_memory_size = sizeof(f32);

// BOTTOM

