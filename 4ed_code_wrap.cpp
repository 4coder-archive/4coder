/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 24.03.2018
 *
 * Code wrapping logic
 *
 */

// TOP

internal void
wrap_state_init(System_Functions *system, Code_Wrap_State *state, Editing_File *file, Font_Pointers font){
    state->token_array = file->state.token_array;
    state->token_ptr = state->token_array.tokens;
    state->end_token = state->token_ptr + state->token_array.count;
    
    state->line_starts = file->state.buffer.line_starts;
    state->line_count = file->state.buffer.line_count;
    state->next_line_start = state->line_starts[1];
    
    Gap_Buffer *buffer = &file->state.buffer;
    i32 size = buffer_size(buffer);
    buffer_stringify_loop(&state->stream, buffer, 0, size);
    state->size = size;
    state->i = 0;
    
    state->font = font;
    
    state->tab_indent_amount = font_get_glyph_advance(system, font.settings, font.metrics, font.pages, '\t');
    state->byte_advance = font.metrics->byte_advance;
    
    state->tran = null_buffer_translating_state;
}

internal void
wrap_state_set_top(Code_Wrap_State *state, f32 line_shift){
    if (state->wrap_x.paren_nesting[state->wrap_x.paren_safe_top] > line_shift){
        state->wrap_x.paren_nesting[state->wrap_x.paren_safe_top] = line_shift;
    }
}

internal Code_Wrap_Step
wrap_state_consume_token(System_Functions *system, Font_Pointers font, Code_Wrap_State *state, i32 fixed_end_point){
    Code_Wrap_Step result = {};
    i32 i = state->i;
    
    result.position_start = i;
    
    Cpp_Token token = {};
    
    token.start = state->size;
    if (state->token_ptr < state->end_token){
        token = *state->token_ptr;
    }
    
    if (state->consume_newline){
        ++i;
        state->x = 0;
        state->consume_newline = 0;
    }
    
    
    if (state->in_pp_body){
        if (!(token.flags & CPP_TFLAG_PP_BODY)){
            state->in_pp_body = false;
            state->wrap_x = state->plane_wrap_x;
        }
    }
    
    if (!state->in_pp_body){
        if (token.flags & CPP_TFLAG_PP_DIRECTIVE){
            state->in_pp_body = true;
            state->plane_wrap_x = state->wrap_x;
            state->wrap_x = null_wrap_x;
        }
    }
    
    b32 skipping_whitespace = false;
    if (i >= state->next_line_start){
        state->x = state->wrap_x.paren_nesting[state->wrap_x.paren_safe_top];
        state->x = state->wrap_x.paren_nesting[state->wrap_x.paren_safe_top];
        skipping_whitespace = true;
    }
    
    // TODO(allen): exponential search this shit!
    for (;i >= state->next_line_start;){
        state->next_line_start = state->size;
        if (state->line_index < state->line_count){
            ++state->line_index;
            if (state->line_index + 1 < state->line_count){
                state->next_line_start = state->line_starts[state->line_index + 1];
            }
        }
        else{
            break;
        }
    }
    
    i32 line_start = state->size;
    if (state->line_index < 0){
        line_start = 0;
    }
    else if (state->line_index < state->line_count){
        line_start = state->line_starts[state->line_index];
    }
    b32 still_looping = 0;
    i32 end = token.start + token.size;
    if (fixed_end_point >= 0 && end > fixed_end_point){
        end = fixed_end_point;
    }
    
    i = clamp_bottom(line_start, i);
    if (i == line_start){
        skipping_whitespace = true;
    }
    
    b32 recorded_start_x = false;
    do{
        for (; i < state->stream.end; ++i){
            if (!(i < end)){
                goto doublebreak;
            }
            
            u8 ch = (u8)state->stream.data[i];
            translating_fully_process_byte(system, font, &state->tran, ch, i, state->size, &state->emits);
            
            for (TRANSLATION_EMIT_LOOP(state->J, state->emits)){
                TRANSLATION_GET_STEP(state->step, state->behavior, state->J, state->emits);
                
                if (state->behavior.do_newline){
                    state->consume_newline = 1;
                    goto doublebreak;
                }
                else if(state->behavior.do_number_advance || state->behavior.do_codepoint_advance){
                    u32 n = state->step.value;
                    f32 adv = 0;
                    if (state->behavior.do_codepoint_advance){
                        adv = font_get_glyph_advance(system, state->font.settings, state->font.metrics, state->font.pages, n);
                        
                        if (n != ' ' && n != '\t'){
                            skipping_whitespace = false;
                        }
                    }
                    else{
                        adv = state->byte_advance;
                        skipping_whitespace = false;
                    }
                    
                    if (!skipping_whitespace){
                        if (!recorded_start_x){
                            result.start_x = state->x;
                            recorded_start_x = true;
                        }
                        state->x += adv;
                    }
                }
            }
        }
        still_looping = buffer_stringify_next(&state->stream);
    }while(still_looping);
    doublebreak:;
    
    state->i = i;
    
    b32 consume_token = false;
    if (state->token_ptr < state->end_token && i >= token.start + token.size){
        consume_token = true;
    }
    
    result.this_token = state->token_ptr;
    if (consume_token){
        Assert(state->token_ptr < state->end_token);
        switch (state->token_ptr->type){
            case CPP_TOKEN_BRACE_OPEN:
            {
                state->wrap_x.paren_nesting[state->wrap_x.paren_safe_top] += state->tab_indent_amount;
            }break;
            
            case CPP_TOKEN_BRACE_CLOSE:
            {
                state->wrap_x.paren_nesting[state->wrap_x.paren_safe_top] -= state->tab_indent_amount;
            }break;
            
            case CPP_TOKEN_PARENTHESE_OPEN:
            case CPP_TOKEN_BRACKET_OPEN:
            {
                ++state->wrap_x.paren_top;
                
                i32 top = state->wrap_x.paren_top;
                if (top >= ArrayCount(state->wrap_x.paren_nesting)){
                    top = ArrayCount(state->wrap_x.paren_nesting) - 1;
                }
                state->wrap_x.paren_safe_top = top;
                
                state->wrap_x.paren_nesting[top] = state->x;
            }break;
            
            case CPP_TOKEN_PARENTHESE_CLOSE:
            case CPP_TOKEN_BRACKET_CLOSE:
            {
                --state->wrap_x.paren_top;
                
                if (state->wrap_x.paren_top < 0){
                    state->wrap_x.paren_top = 0;
                }
                
                i32 top = state->wrap_x.paren_top;
                if (top >= ArrayCount(state->wrap_x.paren_nesting)){
                    top = ArrayCount(state->wrap_x.paren_nesting) - 1;
                }
                state->wrap_x.paren_safe_top = top;
            }break;
        }
        
        ++state->token_ptr;
        if (state->token_ptr > state->end_token){
            state->token_ptr = state->end_token;
        }
    }
    
    result.position_end = state->i;
    result.final_x = state->x;
    
    if (!recorded_start_x){
        result.start_x = state->x;
        recorded_start_x = true;
    }
    
    return(result);
}

internal i32
stickieness_guess(Cpp_Token_Type type, Cpp_Token_Type other_type, u16 flags, u16 other_flags, b32 on_left){
    i32 guess = 0;
    
    b32 is_words = false;
    b32 other_is_words = false;
    
    Cpp_Token_Category cat = cpp_token_category_from_type(type);
    Cpp_Token_Category other_cat = cpp_token_category_from_type(other_type);
    
    if (type == CPP_TOKEN_IDENTIFIER || (CPP_TOKEN_CAT_TYPE <= cat && cat <= CPP_TOKEN_CAT_OTHER)){
        is_words = true;
    }
    if (other_type == CPP_TOKEN_IDENTIFIER || (CPP_TOKEN_CAT_TYPE <= other_cat && other_cat <= CPP_TOKEN_CAT_OTHER)){
        other_is_words = true;
    }
    
    b32 is_operator = 0, other_is_operator = 0;
    if (flags & CPP_TFLAG_IS_OPERATOR){
        is_operator = 1;
    }
    if (other_flags & CPP_TFLAG_IS_OPERATOR){
        other_is_operator = 1;
    }
    
    i32 operator_side_bias = 70*(!on_left);
    
    if (is_words && other_is_words){
        guess = 200;
    }
    else if (type == CPP_TOKEN_PARENTHESE_OPEN){
        if (on_left){
            guess = 100;
        }
        else{
            if (other_is_words){
                guess = 100;
            }
            else{
                guess = 0;
            }
        }
    }
    else if (type == CPP_TOKEN_SEMICOLON){
        if (on_left){
            guess = 0;
        }
        else{
            guess = 1000;
        }
    }
    else if (type == CPP_TOKEN_COMMA){
        guess = 20;
    }
    else if (type == CPP_TOKEN_COLON ||
             type == CPP_TOKEN_PARENTHESE_CLOSE ||
             type == CPP_TOKEN_BRACKET_OPEN ||
             type == CPP_TOKEN_BRACKET_CLOSE){
        if (on_left){
            guess = 20;
            if (other_is_words){
                guess = 100;
            }
        }
        else{
            guess = 100;
        }
    }
    else if (type == CPP_PP_DEFINED){
        if (on_left){
            guess = 100;
        }
        else{
            guess = 0;
        }
    }
    else if (type == CPP_TOKEN_SCOPE){
        guess = 90;
    }
    else if (type == CPP_TOKEN_MINUS){
        if (on_left){
            guess = 80;
        }
        else{
            guess = 60;
        }
    }
    else if (type == CPP_TOKEN_DOT ||
             type == CPP_TOKEN_ARROW){
        guess = 200;
    }
    else if (type == CPP_TOKEN_NOT ||
             type == CPP_TOKEN_TILDE){
        if (on_left){
            guess = 80;
        }
        else{
            guess = 20;
        }
    }
    else if (type == CPP_TOKEN_INCREMENT ||
             type == CPP_TOKEN_DECREMENT ||
             type == CPP_TOKEN_STAR ||
             type == CPP_TOKEN_AMPERSAND ||
             (type >= CPP_TOKEN_POSTINC &&
              type <= CPP_TOKEN_DELETE_ARRAY)){
        guess = 80;
        if (!on_left && other_is_operator){
            guess = 20;
        }
    }
    else if (type >= CPP_TOKEN_MUL && type <= CPP_TOKEN_MOD){
        guess = 70;
    }
    else if (type == CPP_TOKEN_PLUS){
        guess = 60 + operator_side_bias;
    }
    else if (type >= CPP_TOKEN_LSHIFT && type <= CPP_TOKEN_RSHIFT){
        guess = 50;
    }
    else if (type >= CPP_TOKEN_LESS && type <= CPP_TOKEN_NOTEQ){
        guess = 40 + operator_side_bias;
    }
    else if (type >= CPP_TOKEN_BIT_XOR && type <= CPP_TOKEN_BIT_OR){
        guess = 40;
    }
    else if (type >= CPP_TOKEN_AND && type <= CPP_TOKEN_OR){
        guess = 30 + operator_side_bias;
    }
    else if (type >= CPP_TOKEN_TERNARY_QMARK && type <= CPP_TOKEN_COLON){
        guess = 20 + operator_side_bias;
    }
    else if (type == CPP_TOKEN_THROW){
        if (on_left){
            guess = 100;
        }
        else{
            guess = 0;
        }
    }
    else if (type >= CPP_TOKEN_EQ && type <= CPP_TOKEN_XOREQ){
        guess = 15 + operator_side_bias;
    }
    
    return(guess);
}

internal Wrap_Current_Shift
get_current_shift(Code_Wrap_State *wrap_state, i32 next_line_start){
    Wrap_Current_Shift result = {};
    
    result.shift = wrap_state->wrap_x.paren_nesting[wrap_state->wrap_x.paren_safe_top];
    
    Cpp_Token next_token = {};
    if (wrap_state->token_ptr < wrap_state->end_token){
        next_token = *wrap_state->token_ptr;
    }
    
    if (wrap_state->token_ptr > wrap_state->token_array.tokens){
        Cpp_Token prev_token = *(wrap_state->token_ptr-1);
        
        if (wrap_state->wrap_x.paren_safe_top != 0 && prev_token.type == CPP_TOKEN_PARENTHESE_OPEN){
            result.shift = wrap_state->wrap_x.paren_nesting[wrap_state->wrap_x.paren_safe_top-1] + wrap_state->tab_indent_amount;
            result.adjust_top_to_this = 1;
        }
        
        f32 statement_continuation_indent = 0.f;
        if (result.shift != 0.f && wrap_state->wrap_x.paren_safe_top == 0){
            if (!(prev_token.flags & (CPP_TFLAG_PP_DIRECTIVE|CPP_TFLAG_PP_BODY))){
                switch (prev_token.type){
                    case CPP_TOKEN_BRACKET_OPEN:
                    case CPP_TOKEN_BRACE_OPEN:
                    case CPP_TOKEN_BRACE_CLOSE:
                    case CPP_TOKEN_SEMICOLON:
                    case CPP_TOKEN_COLON:
                    case CPP_TOKEN_COMMA:
                    case CPP_TOKEN_COMMENT: break;
                    default: statement_continuation_indent += wrap_state->tab_indent_amount; break;
                }
            }
        }
        
        switch (next_token.type){
            case CPP_TOKEN_BRACE_CLOSE: case CPP_TOKEN_BRACE_OPEN: break;
            default: result.shift += statement_continuation_indent; break;
        }
    }
    
    if (next_token.start < next_line_start){
        if (next_token.flags & CPP_TFLAG_PP_DIRECTIVE){
            result.shift = 0;
        }
        else{
            switch (next_token.type){
                case CPP_TOKEN_BRACE_CLOSE:
                {
                    if (wrap_state->wrap_x.paren_safe_top == 0){
                        result.shift -= wrap_state->tab_indent_amount;
                    }
                }break;
            }
        }
    }
    
    result.shift = clamp_bottom(0.f, result.shift);
    return(result);
}

internal void
file_measure_wraps(System_Functions *system, Mem_Options *mem, Editing_File *file, Font_Pointers font){
    Heap *heap = &mem->heap;
    Partition *part = &mem->part;
    
    Temp_Memory temp = begin_temp_memory(part);
    
    file_allocate_wraps_as_needed(heap, file);
    file_allocate_indents_as_needed(heap, file, file->state.buffer.line_count);
    file_allocate_wrap_positions_as_needed(heap, file, file->state.buffer.line_count);
    
    Buffer_Measure_Wrap_Params params;
    params.buffer          = &file->state.buffer;
    params.wrap_line_index = file->state.wrap_line_index;
    params.system          = system;
    params.font            = font;
    params.virtual_white   = file->settings.virtual_white;
    
    f32 width = (f32)file->settings.display_width;
    f32 minimum_base_width = (f32)file->settings.minimum_base_display_width;
    
    i32 size = buffer_size(params.buffer);
    
    Buffer_Measure_Wrap_State state = {};
    Buffer_Layout_Stop stop = {};
    
    f32 edge_tolerance = 50.f;
    edge_tolerance = clamp_top(edge_tolerance, 50.f);
    
    f32 current_line_shift = 0.f;
    b32 do_wrap = 0;
    i32 wrap_unit_end = 0;
    
    i32 wrap_position_index = 0;
    file->state.wrap_positions[wrap_position_index++] = 0;
    
    Code_Wrap_State wrap_state = {};
    
    b32 use_tokens = false;
    
    Wrap_Indent_Pair *wrap_indent_marks = 0;
    Potential_Wrap_Indent_Pair *potential_marks = 0;
    i32 max_wrap_indent_mark = 0;
    
    if (params.virtual_white && file->state.tokens_complete && !file->state.still_lexing){
        wrap_state_init(system, &wrap_state, file, font);
        use_tokens = true;
        
        potential_marks = push_array(part, Potential_Wrap_Indent_Pair, floor32(width));
        
        max_wrap_indent_mark = part_remaining(part)/sizeof(Wrap_Indent_Pair);
        wrap_indent_marks = push_array(part, Wrap_Indent_Pair, max_wrap_indent_mark);
    }
    
    i32 real_count = 0;
    i32 potential_count = 0;
    i32 stage = 0;
    
    do{
        stop = buffer_measure_wrap_y(&state, params, current_line_shift, do_wrap, wrap_unit_end);
        
        switch (stop.status){
            case BLStatus_NeedWrapDetermination:
            {
                if (use_tokens){
                    if (stage == 0){
                        do_wrap = 0;
                        wrap_unit_end = wrap_indent_marks[stage+1].wrap_position;
                        ++stage;
                    }
                    else{
                        do_wrap = 1;
                        wrap_unit_end = wrap_indent_marks[stage+1].wrap_position;
                        file_allocate_wrap_positions_as_needed(heap, file, wrap_position_index);
                        file->state.wrap_positions[wrap_position_index++] = stop.pos;
                    }
                }
                else{
                    Translation_State tran = {};
                    Translation_Emits emits = {};
                    Gap_Buffer_Stream stream = {};
                    
                    i32 word_stage = 0;
                    i32 i = stop.pos;
                    f32 x = stop.x;
                    f32 self_x = 0;
                    i32 wrap_end_result = size;
                    if (buffer_stringify_loop(&stream, params.buffer, i, size)){
                        b32 still_looping = false;
                        do{
                            for (; i < stream.end; ++i){
                                {
                                    u8 ch = stream.data[i];
                                    translating_fully_process_byte(system, font, &tran, ch, i, size, &emits);
                                }
                                
                                for (TRANSLATION_DECL_EMIT_LOOP(J, emits)){
                                    TRANSLATION_DECL_GET_STEP(step, behavior, J, emits);
                                    
                                    u32 codepoint = step.value;
                                    switch (word_stage){
                                        case 0:
                                        {
                                            if (codepoint_is_whitespace(codepoint)){
                                                word_stage = 1;
                                            }
                                            else{
                                                f32 adv = font_get_glyph_advance(params.system, params.font.settings, params.font.metrics, params.font.pages, codepoint);
                                                
                                                x += adv;
                                                self_x += adv;
                                                if (self_x > width){
                                                    wrap_end_result = step.i;
                                                    goto doublebreak;
                                                }
                                            }
                                        }break;
                                        
                                        case 1:
                                        {
                                            if (!codepoint_is_whitespace(codepoint)){
                                                wrap_end_result = step.i;
                                                goto doublebreak;
                                            }
                                        }break;
                                    }
                                }
                            }
                            still_looping = buffer_stringify_next(&stream);
                        }while(still_looping);
                    }
                    
                    doublebreak:;
                    wrap_unit_end = wrap_end_result;
                    if (x > width){
                        do_wrap = 1;
                        file_allocate_wrap_positions_as_needed(heap, file, wrap_position_index);
                        file->state.wrap_positions[wrap_position_index++] = stop.pos;
                    }
                    else{
                        do_wrap = 0;
                    }
                }
            }break;
            
            case BLStatus_NeedWrapLineShift:
            case BLStatus_NeedLineShift:
            {
                f32 current_width = width;
                
                if (use_tokens){
                    Code_Wrap_State original_wrap_state = wrap_state;
                    i32 next_line_start = buffer_size(params.buffer);
                    if (stop.line_index+1 < params.buffer->line_count){
                        next_line_start = params.buffer->line_starts[stop.line_index+1];
                    }
                    
                    f32 base_adjusted_width = wrap_state.wrap_x.base_x + minimum_base_width;
                    
                    if (minimum_base_width != 0 && current_width < base_adjusted_width){
                        current_width = base_adjusted_width;
                    }
                    
                    if (stop.status == BLStatus_NeedLineShift){
                        real_count = 0;
                        potential_count = 0;
                        stage = 0;
                        
                        Wrap_Current_Shift current_shift =  get_current_shift(&wrap_state, next_line_start);
                        
                        if (current_shift.adjust_top_to_this){
                            wrap_state_set_top(&wrap_state, current_shift.shift);
                        }
                        
                        wrap_indent_marks[real_count].wrap_position = 0;
                        wrap_indent_marks[real_count].line_shift =current_shift.shift;
                        ++real_count;
                        
                        wrap_state.wrap_x.base_x = wrap_state.wrap_x.paren_nesting[0];
                        
                        for (; wrap_state.token_ptr < wrap_state.end_token; ){
                            Code_Wrap_Step step = {};
                            b32 emit_comment_position = false;
                            b32 first_word = true;
                            
                            if (wrap_state.token_ptr->type == CPP_TOKEN_COMMENT || wrap_state.token_ptr->type == CPP_TOKEN_STRING_CONSTANT){
                                i32 i = wrap_state.token_ptr->start;
                                i32 end_i = i + wrap_state.token_ptr->size;
                                
                                if (i < wrap_state.i){
                                    i = wrap_state.i;
                                }
                                
                                if (end_i > wrap_state.next_line_start){
                                    end_i = wrap_state.next_line_start;
                                }
                                
                                f32 x = wrap_state.x;
                                
                                step.position_start = i;
                                step.start_x = x;
                                step.this_token = wrap_state.token_ptr;
                                
                                Gap_Buffer_Stream stream = {};
                                Translation_State tran = {};
                                Translation_Emits emits = {};
                                
                                Potential_Wrap_Indent_Pair potential_wrap = {};
                                potential_wrap.wrap_position = i;
                                potential_wrap.line_shift = x;
                                potential_wrap.wrappable_score = 5;
                                potential_wrap.wrap_x = x;
                                potential_wrap.adjust_top_to_this = 0;
                                
                                if (buffer_stringify_loop(&stream, params.buffer, i, end_i)){
                                    b32 still_looping = true;
                                    
                                    while(still_looping){
                                        for (; i < stream.end; ++i){
                                            {
                                                u8 ch = stream.data[i];
                                                translating_fully_process_byte(system, font, &tran, ch, i, end_i, &emits);
                                            }
                                            
                                            for (TRANSLATION_DECL_EMIT_LOOP(J, emits)){
                                                TRANSLATION_DECL_GET_STEP(buffer_step, behav, J, emits);
                                                if (!codepoint_is_whitespace(buffer_step.value)){
                                                    i = buffer_step.i;
                                                    goto doublebreak_stage_vspace;
                                                }
                                            }
                                        }
                                        still_looping = buffer_stringify_next(&stream);
                                    }
                                    doublebreak_stage_vspace:;
                                    
                                    do{
                                        i32 pos_end_i = end_i;
                                        while (still_looping){
                                            for (; i < stream.end; ++i){
                                                {
                                                    u8 ch = stream.data[i];
                                                    translating_fully_process_byte(system, font, &tran, ch, i, end_i, &emits);
                                                }
                                                
                                                for (TRANSLATION_DECL_EMIT_LOOP(J, emits)){
                                                    TRANSLATION_DECL_GET_STEP(buffer_step, behav, J, emits);
                                                    if (codepoint_is_whitespace(buffer_step.value)){
                                                        pos_end_i = buffer_step.i;
                                                        goto doublebreak_stage1;
                                                    }
                                                    
                                                    f32 adv = font_get_glyph_advance(params.system, params.font.settings, params.font.metrics, params.font.pages, buffer_step.value);
                                                    x += adv;
                                                    
                                                    if (!first_word && x > current_width){
                                                        pos_end_i = buffer_step.i;
                                                        emit_comment_position = true;
                                                        goto doublebreak_stage1;
                                                    }
                                                }
                                            }
                                            still_looping = buffer_stringify_next(&stream);
                                        }
                                        doublebreak_stage1:;
                                        first_word = 0;
                                        
                                        if (emit_comment_position){
                                            step.position_end = pos_end_i;
                                            step.final_x = x;
                                            goto finished_comment_split;
                                        }
                                        
                                        while(still_looping){
                                            for (; i < stream.end; ++i){
                                                {
                                                    u8 ch = stream.data[i];
                                                    translating_fully_process_byte(system, font, &tran, ch, i, end_i, &emits);
                                                }
                                                
                                                for (TRANSLATION_DECL_EMIT_LOOP(J, emits)){
                                                    TRANSLATION_DECL_GET_STEP(buffer_step, behav, J, emits);
                                                    
                                                    if (!codepoint_is_whitespace(buffer_step.value)){
                                                        pos_end_i = buffer_step.i;
                                                        goto doublebreak_stage2;
                                                    }
                                                    
                                                    f32 adv = font_get_glyph_advance(params.system, params.font.settings, params.font.metrics, params.font.pages, buffer_step.value);
                                                    x += adv;
                                                }
                                            }
                                            still_looping = buffer_stringify_next(&stream);
                                        }
                                        doublebreak_stage2:;
                                        
                                        potential_wrap.wrap_position = pos_end_i;
                                        potential_wrap.wrap_x = x;
                                    }while(still_looping);
                                }
                                
                                finished_comment_split:;
                                if (emit_comment_position){
                                    potential_marks[potential_count] = potential_wrap;
                                    ++potential_count;
                                }
                            }
                            
                            if (!emit_comment_position){
                                step = wrap_state_consume_token(system, font, &wrap_state, next_line_start);
                            }
                            
                            b32 need_to_choose_a_wrap = false;
                            if (step.final_x > current_width){
                                need_to_choose_a_wrap = true;
                            }
                            
                            current_shift = get_current_shift(&wrap_state, next_line_start);
                            
                            b32 next_token_is_on_line = false;
                            if (wrap_state.token_ptr < wrap_state.end_token){
                                if (wrap_state.token_ptr->start < next_line_start){
                                    next_token_is_on_line = true;
                                }
                            }
                            
                            i32 next_wrap_position = step.position_end;
                            f32 wrap_x = step.final_x;
                            if (next_token_is_on_line){
                                if (wrap_state.token_ptr < wrap_state.end_token){
                                    i32 pos_i = wrap_state.token_ptr->start;
                                    if (pos_i > step.position_start && next_wrap_position < pos_i){
                                        next_wrap_position = pos_i;
                                    }
                                }
                            }
                            
                            if (!need_to_choose_a_wrap){
                                i32 wrappable_score = 1;
                                
                                Cpp_Token *this_token = step.this_token;
                                Cpp_Token *next_token = 0;
                                if (wrap_state.token_ptr < wrap_state.end_token){
                                    next_token = wrap_state.token_ptr;
                                }
                                
                                Cpp_Token_Type this_type = this_token->type;
                                Cpp_Token_Type next_type = CPP_TOKEN_JUNK;
                                
                                u16 this_flags = this_token->flags;
                                u16 next_flags = 0;
                                
                                if (this_token == next_token || !next_token_is_on_line){
                                    next_token = 0;
                                }
                                
                                if (next_token){
                                    next_type = next_token->type;
                                    next_flags = next_token->flags;
                                }
                                
                                i32 this_stickieness = stickieness_guess(this_type, next_type, this_flags, next_flags, 1);
                                
                                i32 next_stickieness = 0;
                                if (next_token){
                                    next_stickieness = stickieness_guess(next_type, this_type, next_flags, this_flags, 0);
                                }
                                
                                i32 heap_stickieness = this_stickieness;
                                if (heap_stickieness < next_stickieness){
                                    heap_stickieness = next_stickieness;
                                }
                                
                                if (wrap_state.wrap_x.paren_top != 0 && this_type == CPP_TOKEN_COMMA){
                                    heap_stickieness = 0;
                                }
                                
                                wrappable_score = 64*50;
                                wrappable_score += 101 - heap_stickieness - wrap_state.wrap_x.paren_safe_top*80;
                                
                                potential_marks[potential_count].wrap_position = next_wrap_position;
                                potential_marks[potential_count].line_shift = current_shift.shift;
                                potential_marks[potential_count].wrappable_score = wrappable_score;
                                potential_marks[potential_count].wrap_x = wrap_x;
                                potential_marks[potential_count].adjust_top_to_this = current_shift.adjust_top_to_this;
                                ++potential_count;
                            }
                            
                            if (need_to_choose_a_wrap){
                                if (potential_count == 0){
                                    wrap_indent_marks[real_count].wrap_position = next_wrap_position;
                                    wrap_indent_marks[real_count].line_shift = current_shift.shift;
                                    ++real_count;
                                }
                                else{
                                    i32 i = 0, best_i = 0;
                                    i32 best_score = -1;
                                    f32 best_x_shift = 0;
                                    
                                    f32 x_gain_threshold = 18.f;
                                    
                                    for (; i < potential_count; ++i){
                                        i32 this_score = potential_marks[i].wrappable_score;
                                        f32 x_shift = potential_marks[i].wrap_x - potential_marks[i].line_shift;
                                        
                                        f32 x_shift_adjusted = x_shift - x_gain_threshold;
                                        f32 x_left_over = step.final_x - x_shift;
                                        
                                        if (x_shift_adjusted < 0){
                                            this_score = 0;
                                        }
                                        else if (x_left_over <= x_gain_threshold){
                                            this_score = 1;
                                        }
                                        
                                        if (this_score > best_score){
                                            best_score = this_score;
                                            best_x_shift = x_shift;
                                            best_i = i;
                                        }
                                        else if (this_score == best_score && x_shift > best_x_shift){
                                            best_x_shift = x_shift;
                                            best_i = i;
                                        }
                                    }
                                    
                                    i32 wrap_position = potential_marks[best_i].wrap_position;
                                    f32 line_shift = potential_marks[best_i].line_shift;
                                    b32 adjust_top_to_this = potential_marks[best_i].adjust_top_to_this;
                                    wrap_indent_marks[real_count].wrap_position = wrap_position;
                                    wrap_indent_marks[real_count].line_shift    = line_shift;
                                    ++real_count;
                                    potential_count = 0;
                                    
                                    wrap_state = original_wrap_state;
                                    for (;;){
                                        step = wrap_state_consume_token(system, font, &wrap_state, wrap_position);
                                        if (step.position_end >= wrap_position){
                                            break;
                                        }
                                    }
                                    
                                    wrap_state.x = line_shift;
                                    wrap_state.i = wrap_position;
                                    if (adjust_top_to_this){
                                        wrap_state_set_top(&wrap_state, line_shift);
                                    }
                                    
                                    original_wrap_state = wrap_state;
                                }
                            }
                            
                            if (step.position_end >= next_line_start-1){
                                break;
                            }
                        }
                        
                        wrap_indent_marks[real_count].wrap_position = next_line_start;
                        wrap_indent_marks[real_count].line_shift    = 0;
                        ++real_count;
                        
                        for (i32 l = 0; wrap_state.i < next_line_start && l < 3; ++l){
                            wrap_state_consume_token(system, font, &wrap_state, next_line_start);
                        }
                    }
                    
                    current_line_shift = wrap_indent_marks[stage].line_shift;
                    
                    if (stage > 0){
                        ++stage;
                    }
                    
                    current_line_shift = clamp_bottom(0.f, current_line_shift);
                }
                else{
                    current_line_shift = 0.f;
                }
                
                current_line_shift = clamp_top(current_line_shift, current_width - edge_tolerance);
                
                if (stop.wrap_line_index >= file->state.line_indent_max){
                    file_allocate_indents_as_needed(heap, file, stop.wrap_line_index);
                }
                
                file->state.line_indents[stop.wrap_line_index] = current_line_shift;
                file->state.wrap_line_count = stop.wrap_line_index;
            }break;
        }
    }while(stop.status != BLStatus_Finished);
    
    ++file->state.wrap_line_count;
    
    file_allocate_wrap_positions_as_needed(heap, file, wrap_position_index);
    file->state.wrap_positions[wrap_position_index++] = size;
    file->state.wrap_position_count = wrap_position_index;
    
    end_temp_memory(temp);
}

// BOTTOM

