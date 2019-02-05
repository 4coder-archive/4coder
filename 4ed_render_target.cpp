/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 10.11.2017
 *
 * Render target function implementations.
 *
 */

// TOP

#define Render_Begin_Push_Sig(n,t,p,s) void*    (n)(Render_Target *t, void *p, i32 s)
#define Render_End_Push_Sig(n,t,h)     void     (n)(Render_Target *t, void *h)
#define Render_Change_Clip_Sig(n,t,c)  void     (n)(Render_Target *t, i32_Rect c)
#define Render_Push_Clip_Sig(n,t,c)    void     (n)(Render_Target *t, i32_Rect c)
#define Render_Pop_Clip_Sig(n,t)       i32_Rect (n)(Render_Target *t)

////////////////////////////////

internal
Render_Begin_Push_Sig(render_internal_begin_push, t, ptr, size){
    void *out = push_array(&t->buffer, u8, size);
    if (out != 0){
        memcpy(out, ptr, size);
    }
    return(out);
}

internal
Render_End_Push_Sig(render_internal_end_push, t, h){
    if (h != 0){
        push_align(&t->buffer, 8);
        u8 *end_ptr = push_array(&t->buffer, u8, 0);
        Render_Command_Header *header = (Render_Command_Header*)h;
        header->size = (i32)(end_ptr - (u8*)h);
    }
    // TODO(allen): else { LOG }
}

internal void
render_internal_push_clip(Render_Target *t, i32_Rect clip_box){
    t->clip_all = (clip_box.x0 >= clip_box.x1 || clip_box.y0 >= clip_box.y1);
    if (t->clip_all){
        return;
    }
    
    // TODO(allen): If the previous command was also a push clip should
    // undo that one and just do this one. (OPTIMIZATION).
    Render_Command_Change_Clip cmd = {};
    cmd.header.size = sizeof(cmd);
    cmd.header.type = RenCom_ChangeClip;
    cmd.box = clip_box;
    void *h = render_internal_begin_push(t, &cmd, cmd.header.size);
    render_internal_end_push(t, h);
}

////////////////////////////////

internal
Render_Begin_Push_Sig(render_begin_push, t, ptr, size){
    void *out = 0;
    if (!t->clip_all){
        out = render_internal_begin_push(t, ptr, size);
    }
    return(out);
}

internal
Render_End_Push_Sig(render_end_push, t, h){
    render_internal_end_push(t, h);
}

internal
Render_Change_Clip_Sig(render_change_clip, t, clip_box){
    Assert(t->clip_top > -1);
    t->clip_boxes[t->clip_top] = clip_box;
    render_internal_push_clip(t, clip_box);
}

internal
Render_Push_Clip_Sig(render_push_clip, t, clip_box){
    Assert(t->clip_top == -1 || fits_inside(clip_box, t->clip_boxes[t->clip_top]));
    Assert(t->clip_top + 1 < ArrayCount(t->clip_boxes));
    t->clip_boxes[++t->clip_top] = clip_box;
    render_internal_push_clip(t, clip_box);
}

internal
Render_Pop_Clip_Sig(render_pop_clip, t){
    Assert(t->clip_top > 0);
    i32_Rect result = t->clip_boxes[t->clip_top];
    --t->clip_top;
    i32_Rect clip_box = t->clip_boxes[t->clip_top];
    render_internal_push_clip(t, clip_box);
    return(result);
}

// BOTTOM

