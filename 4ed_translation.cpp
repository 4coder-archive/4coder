/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 11.03.2017
 *
 * Translation system for turning byte streams into a stream of buffer model steps.
 *
 */

// TOP

// TODO(allen): I don't like this code _AT ALL_
// unravel the mess!
//
//
// So what happened here was I thought, "Hey I have text in non-contiguous chunks and now I 
// need to translates it into unicode codepoints (interpreting it as utf8), so I should make
// a system that translates utf-8 by taking in one byte at a time then emitting one or more
// codepoints whenever there is enough information, all the while ensuring there is no backtracking"
//
// Even though this may make the iteration sound nice, it's a HUGE FREAKING PAIN IN THE ASS.
// You can't optimize it very well, the code is inscrutible both on the implementation side
// and the calling side.  Besides the fact that I "got it working" there isn't one good thing
// about this code.
//
// My next idea would be to try to make translation systems that take in the chunks themselves as
// a linked list, and then just does the WHOLE translation, MAYBE with optional "stop conditions".
// This way someone can actually optimize the translation loop by hand in _ONE SPOT_.  The downside
// is that the caller will have to put up with maybe more translation work than they needed, but that
// translation work will be so much cheaper, and easier to maintain, that the caller will be happier
// overall.
//
//
// If this comment is still here, then I haven't fixed any of this garbage yet, but it should really
// be fixed!

#define ERROR_BYTE (max_u8-1)
#define CONTINUATION_BYTE max_u8

internal void
translating_consume_byte(Translation_State *tran, u8 ch, u32 i, u32 size, Translation_Byte_Description *desc_out){
    desc_out->byte_class = 0;
    if (ch < 0x80){
        desc_out->byte_class = 1;
    }
    else if (ch < 0xC0){
        desc_out->byte_class = CONTINUATION_BYTE;
    }
    else if (ch < 0xE0){
        desc_out->byte_class = 2;
    }
    else if (ch < 0xF0){
        desc_out->byte_class = 3;
    }
    else if (ch < 0xF8){
        desc_out->byte_class = 4;
    }
    else{
        desc_out->byte_class = ERROR_BYTE;
    }
    
    desc_out->prelim_emit_type = BufferModelUnit_None;
    desc_out->last_byte_handler = TranLBH_None;
    if (tran->fill_expected == 0){
        tran->fill_buffer[0] = ch;
        tran->fill_start_i = i;
        tran->fill_i = 1;
        
        if (desc_out->byte_class == 1){
            desc_out->prelim_emit_type = BufferModelUnit_Codepoint;
        }
        else if (desc_out->byte_class == 0 || desc_out->byte_class == CONTINUATION_BYTE || desc_out->byte_class == ERROR_BYTE){
            desc_out->prelim_emit_type = BufferModelUnit_Numbers;
        }
        else{
            tran->fill_expected = desc_out->byte_class;
        }
    }
    else{
        if (desc_out->byte_class == CONTINUATION_BYTE){
            tran->fill_buffer[tran->fill_i] = ch;
            ++tran->fill_i;
            
            if (tran->fill_i == tran->fill_expected){
                desc_out->prelim_emit_type = BufferModelUnit_Codepoint;
            }
        }
        else{
            if (desc_out->byte_class >= 2 && desc_out->byte_class <= 4){
                desc_out->last_byte_handler = TranLBH_Rebuffer;
            }
            else if (desc_out->byte_class == 1){
                desc_out->last_byte_handler = TranLBH_EmitAsCP;
            }
            else{
                tran->fill_buffer[tran->fill_i] = ch;
                ++tran->fill_i;
            }
            desc_out->prelim_emit_type = BufferModelUnit_Numbers;
        }
    }
    
    if (desc_out->prelim_emit_type == BufferModelUnit_None && i+1 == size){
        desc_out->prelim_emit_type = BufferModelUnit_Numbers;
    }
}

internal void
translating_select_emit_rule_UTF8(Translation_State *tran, Translation_Byte_Description desc, Translation_Emit_Rule *type_out){
    type_out->byte_class = desc.byte_class;
    type_out->last_byte_handler = desc.last_byte_handler;
    type_out->emit_type = desc.prelim_emit_type;
    
    type_out->codepoint = 0;
    type_out->codepoint_length = 0;
    if (desc.prelim_emit_type == BufferModelUnit_Codepoint){
        Character_Consume_Result consume = utf8_consume(tran->fill_buffer, ArrayCount(tran->fill_buffer));
        u32 cp = consume.codepoint;
        type_out->codepoint_length = consume.inc;
        if (cp == max_u32){
            type_out->codepoint_length = 0;
        }
        if (type_out->codepoint_length != 0){
            if ((cp >= nonchar_min && cp <= nonchar_max) || ((cp & 0xFFFF) >= 0xFFFE)){
                type_out->emit_type = BufferModelUnit_Numbers;
            }
            else{
                type_out->codepoint = cp;
                if (cp > 0x10FFFF){
                    type_out->emit_type = BufferModelUnit_Numbers;
                }
            }
        }
        else{
            type_out->emit_type = BufferModelUnit_Numbers;
        }
    }
}

internal void
translating_generate_emits(Translation_State *tran, Translation_Emit_Rule emit_rule, u8 ch, u32 i, Translation_Emits *emits_out){
    emits_out->step_count = 0;
    switch (emit_rule.emit_type){
        default: goto skip_all;
        
        case BufferModelUnit_Codepoint:
        {
            emits_out->steps[0].type = 1;
            emits_out->steps[0].value = emit_rule.codepoint;
            emits_out->steps[0].i = tran->fill_start_i;
            emits_out->steps[0].byte_length = emit_rule.codepoint_length;
            emits_out->step_count = 1;
        }break;
        
        case BufferModelUnit_Numbers:
        {
            for (u32 j = 0; j < tran->fill_i; ++j){
                emits_out->steps[j].type = 0;
                emits_out->steps[j].value = tran->fill_buffer[j];
                emits_out->steps[j].i = tran->fill_start_i + j;
                emits_out->steps[j].byte_length = 1;
            }
            emits_out->step_count = tran->fill_i;
        }break;
    }
    
    tran->fill_start_i = 0;
    tran->fill_i = 0;
    tran->fill_expected = 0;
    
    switch (emit_rule.last_byte_handler){
        case TranLBH_Rebuffer:
        {
            tran->fill_buffer[0] = ch;
            tran->fill_start_i = i;
            tran->fill_i = 1;
            tran->fill_expected = emit_rule.byte_class;
        }break;
        
        case TranLBH_EmitAsCP:
        {
            emits_out->steps[emits_out->step_count].type = 1;
            emits_out->steps[emits_out->step_count].value = ch;
            emits_out->steps[emits_out->step_count].i = i;
            emits_out->steps[emits_out->step_count].byte_length = 1;
            ++emits_out->step_count;
        }break;
    }
    
    skip_all:;
}

internal void
translating_fully_process_byte(Translation_State *tran, u8 ch, u32 i, u32 size, Translation_Emits *emits_out){
    Translation_Byte_Description description = {};
    translating_consume_byte(tran, ch, i, size, &description);
    Translation_Emit_Rule emit_rule = {};
    translating_select_emit_rule_UTF8(tran, description, &emit_rule);
    translating_generate_emits(tran, emit_rule, ch, i, emits_out);
}

internal void
translation_step_read(Buffer_Model_Step step, Buffer_Model_Behavior *behavior_out){
    behavior_out->do_newline = false;
    behavior_out->do_codepoint_advance = false;
    behavior_out->do_number_advance = false;
    if (step.type == 1){
        switch (step.value){
            case '\n':
            {
                behavior_out->do_newline = true;
            }break;
            default:
            {
                behavior_out->do_codepoint_advance = true;
            }break;
        }
    }
    else{
        behavior_out->do_number_advance = true;
    }
}

#define TRANSLATION_DECL_EMIT_LOOP(_j,_emit) u32 _j = 0; _j < (_emit).step_count; ++_j
#define TRANSLATION_DECL_GET_STEP(_step,_behav,_j,_emit)                 \
Buffer_Model_Step _step = _emit.steps[_j]; Buffer_Model_Behavior _behav; \
translation_step_read(_step, &_behav)

#define TRANSLATION_EMIT_LOOP(_j,_emit) _j = 0; _j < (_emit).step_count; ++_j
#define TRANSLATION_GET_STEP(_step,_behav,_j,_emit)\
(_step) = _emit.steps[_j]; translation_step_read((_step), &(_behav))

// BOTTOM



