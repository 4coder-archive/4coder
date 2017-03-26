/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 11.03.2017
 *
 * Translation system for turning byte streams into a stream of buffer model steps.
 *
 */

// TOP

#include "4ed_buffer_model.h"

struct Translation_State{
    u8 fill_buffer[4];
    u32 fill_start_i;
    u8 fill_i;
    u8 fill_expected;
};
global_const Translation_State null_buffer_translating_state = {0};

enum{
    TranLBH_None,
    TranLBH_Rebuffer,
    TranLBH_EmitAsCP,
};
struct Translation_Byte_Description{
    u8 byte_class;
    u8 last_byte_handler;
    u8 prelim_emit_type;
};

struct Translation_Emit_Rule{
    u8 byte_class;
    u8 last_byte_handler;
    u8 emit_type;
    
    u32 codepoint;
    u32 codepoint_length;
};

struct Translation_Emits{
    Buffer_Model_Step steps[5];
    u32 step_count;
};

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
translating_select_emit_rule_ASCII(Translation_State *tran, Translation_Byte_Description desc, Translation_Emit_Rule *type_out){
    type_out->byte_class = desc.byte_class;
    type_out->last_byte_handler = desc.last_byte_handler;
    type_out->emit_type = desc.prelim_emit_type;
    
    type_out->codepoint = 0;
    type_out->codepoint_length = 0;
    if (desc.prelim_emit_type == BufferModelUnit_Codepoint){
        u32 cp = utf8_to_u32_length_unchecked(tran->fill_buffer, &type_out->codepoint_length);
        type_out->codepoint = cp;
        if (!(cp == '\n' || cp == '\t' || cp == '\r' || (cp >= ' ' && cp <= 255 && cp != 127))){
            type_out->emit_type = BufferModelUnit_Numbers;
        }
    }
}

internal void
translating_select_emit_rule_with_font(System_Functions *system, Render_Font *font, Translation_State *tran, Translation_Byte_Description desc, Translation_Emit_Rule *type_out){
    type_out->byte_class = desc.byte_class;
    type_out->last_byte_handler = desc.last_byte_handler;
    type_out->emit_type = desc.prelim_emit_type;
    
    type_out->codepoint = 0;
    type_out->codepoint_length = 0;
    if (desc.prelim_emit_type == BufferModelUnit_Codepoint){
        u32 cp = utf8_to_u32_length_unchecked(tran->fill_buffer, &type_out->codepoint_length);
        if (type_out->codepoint_length != 0){
            if ((cp >= nonchar_min && cp <= nonchar_max) || ((cp & 0xFFFF) >= 0xFFFE)){
                type_out->emit_type = BufferModelUnit_Numbers;
            }
            else{
                type_out->codepoint = cp;
                if (!font_can_render(system, font, cp)){
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
translating_fully_process_byte(System_Functions *system, Render_Font *font, Translation_State *tran, u8 ch, u32 i, u32 size, Translation_Emits *emits_out){
    Translation_Byte_Description description = {0};
    translating_consume_byte(tran, ch, i, size, &description);
    Translation_Emit_Rule emit_rule = {0};
    translating_select_emit_rule_with_font(system, font, tran, description, &emit_rule);
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



