/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 10.11.2017
 *
 * Render target function implementations.
 *
 */

// TOP

#define Render_Push_Clip_Sig(n, t, c)  void     (n)(Render_Target *t, i32_Rect c)
#define Render_Pop_Clip_Sig(n, t)      i32_Rect (n)(Render_Target *t)
#define Render_Push_Piece_Sig(n, t, p) void     (n)(Render_Target *t, Render_Piece_Combined p)

inline void
draw_safe_push(Render_Target *t, i32 size, void *x){
    if (size + t->size <= t->max){
        memcpy(t->push_buffer + t->size, x, size);
        t->size += size;
    }
}

#define PutStruct(s,x) draw_safe_push(t, sizeof(s), &x)

internal 
Render_Push_Piece_Sig(render_push_piece, t, piece){
    if (!t->clip_all){
        PutStruct(Render_Piece_Header, piece.header);
        
        switch (piece.header.type){
            case piece_type_rectangle: case piece_type_outline:
            {
                PutStruct(Render_Piece_Rectangle, piece.rectangle);
            }break;
            
            case piece_type_glyph:
            {
                PutStruct(Render_Piece_Glyph, piece.glyph);
            }break;
        }
        
        Assert(t->size <= t->max);
    }
}

internal void
render_push_piece_clip(Render_Target *t, i32_Rect clip_box){
    if (!t->clip_all){
        // TODO(allen): optimize out if there are two clip box changes in a row
        Render_Piece_Change_Clip clip = {0};
        Render_Piece_Header header = {0};
        
        header.type = piece_type_change_clip;
        clip.box = clip_box;
        
        PutStruct(Render_Piece_Header, header);
        PutStruct(Render_Piece_Change_Clip, clip);
    }
}

internal
Render_Push_Clip_Sig(render_push_clip, t, clip_box){
    Assert(t->clip_top == -1 || fits_inside(clip_box, t->clip_boxes[t->clip_top]));
    Assert(t->clip_top+1 < ArrayCount(t->clip_boxes));
    t->clip_boxes[++t->clip_top] = clip_box;
    
    t->clip_all = (clip_box.x0 >= clip_box.x1 || clip_box.y0 >= clip_box.y1);
    render_push_piece_clip(t, clip_box);
}

internal
Render_Pop_Clip_Sig(render_pop_clip, t){
    Assert(t->clip_top > 0);
    i32_Rect result = t->clip_boxes[t->clip_top];
    --t->clip_top;
    i32_Rect clip_box = t->clip_boxes[t->clip_top];
    
    t->clip_all = (clip_box.x0 >= clip_box.x1 || clip_box.y0 >= clip_box.y1);
    render_push_piece_clip(t, clip_box);
    
    return(result);
}

// BOTTOM

