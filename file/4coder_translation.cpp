/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 11.03.2017
 *
 * Translation system for turning byte streams into a stream of buffer model steps.
 *
 */

// TOP

struct Translation_State{
    u8 fill_buffer[4];
    u32 fill_start_i;
    u32 fill_i;
    u32 fill_expected;
};
global_const Translation_State null_buffer_translating_state = {0};

struct Translation_Byte_Description{
    u32 byte_class;
    b32 rebuffer_current;
    b32 emit_current_as_cp;
    
    u32 prelim_emit_type;
};

struct Translation_Emit_Type{
    u32 byte_class;
    b32 rebuffer_current;
    b32 emit_current_as_cp;
    
    u32 codepoint;
    u32 codepoint_length;
    b32 do_codepoint;
    b32 do_numbers;
};

struct Translation_Emits{
    Buffer_Model_Step steps[5];
    Buffer_Model_Step step_current;
    u32 step_count;
};

internal void
translating_consume_byte(Translation_State *tran, u8 ch, u32 i, u32 size, Translation_Byte_Description *desc_out){
    desc_out->byte_class = 0;
    if ((ch >= ' ' && ch < 0x7F) || ch == '\t' || ch == '\n' || ch == '\r'){
        desc_out->byte_class = 1;
    }
    else if (ch < 0xC0){
        desc_out->byte_class = 1000;
    }
    else if (ch < 0xE0){
        desc_out->byte_class = 2;
    }
    else if (ch < 0xF0){
        desc_out->byte_class = 3;
    }
    else{
        desc_out->byte_class = 4;
    }
    
    desc_out->prelim_emit_type = BufferModelUnit_None;
    desc_out->rebuffer_current = false;
    desc_out->emit_current_as_cp = false;
    if (tran->fill_expected == 0){
        tran->fill_buffer[0] = ch;
        tran->fill_start_i = i;
        tran->fill_i = 1;
        
        if (desc_out->byte_class == 1){
            desc_out->prelim_emit_type = BufferModelUnit_Codepoint;
        }
        else if (desc_out->byte_class == 0 || desc_out->byte_class == 1000){
            desc_out->prelim_emit_type = BufferModelUnit_Numbers;
        }
        else{
            tran->fill_expected = desc_out->byte_class;
        }
    }
    else{
        if (desc_out->byte_class == 1000){
            tran->fill_buffer[tran->fill_i] = ch;
            ++tran->fill_i;
            
            if (tran->fill_i == tran->fill_expected){
                desc_out->prelim_emit_type = BufferModelUnit_Codepoint;
            }
        }
        else{
            if (desc_out->byte_class >= 2 && desc_out->byte_class <= 4){
                desc_out->rebuffer_current = true;
            }
            else if (desc_out->byte_class == 1){
                desc_out->emit_current_as_cp = true;
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
translating_select_emit_type(Translation_State *tran, Translation_Byte_Description desc, Translation_Emit_Type *type_out){
    type_out->byte_class = desc.byte_class;
    type_out->rebuffer_current = desc.rebuffer_current;
    type_out->emit_current_as_cp = desc.emit_current_as_cp;
    
    type_out->codepoint = 0;
    type_out->codepoint_length = 0;
    type_out->do_codepoint = false;
    type_out->do_numbers = false;
    if (desc.prelim_emit_type == BufferModelUnit_Codepoint){
        type_out->codepoint = utf8_to_u32_length_unchecked(tran->fill_buffer, &type_out->codepoint_length);
        if ((type_out->codepoint >= ' ' && type_out->codepoint <= 255 && type_out->codepoint != 127) || type_out->codepoint == '\t' || type_out->codepoint == '\n' || type_out->codepoint == '\r'){
            type_out->do_codepoint = true;
        }
        else{
            type_out->do_numbers = true;
        }
    }
    else if (desc.prelim_emit_type == BufferModelUnit_Numbers){
        type_out->do_numbers = true;
    }
    
    Assert((type_out->do_codepoint + type_out->do_numbers) <= 1);
}

internal void
translating_generate_emits(Translation_State *tran, Translation_Emit_Type emit_types, u8 ch, u32 i, Translation_Emits *emits_out){
    emits_out->step_count = 0;
    if (emit_types.do_codepoint){
        emits_out->steps[0].type = 1;
        emits_out->steps[0].value = emit_types.codepoint;
        emits_out->steps[0].i = tran->fill_start_i;
        emits_out->steps[0].byte_length = emit_types.codepoint_length;
        emits_out->step_count = 1;
    }
    else if (emit_types.do_numbers){
        for (u32 j = 0; j < tran->fill_i; ++j){
            emits_out->steps[j].type = 0;
            emits_out->steps[j].value = tran->fill_buffer[j];
            emits_out->steps[j].i = tran->fill_start_i + j;
            emits_out->steps[j].byte_length = 1;
        }
        emits_out->step_count = tran->fill_i;
    }
    
    if (emit_types.do_codepoint || emit_types.do_numbers){
        tran->fill_start_i = 0;
        tran->fill_i = 0;
        tran->fill_expected = 0;
    }
    
    if (emit_types.rebuffer_current){
        Assert(emit_types.do_codepoint || emit_types.do_numbers);
        
        tran->fill_buffer[0] = ch;
        tran->fill_start_i = i;
        tran->fill_i = 1;
        tran->fill_expected = emit_types.byte_class;
    }
    else if (emit_types.emit_current_as_cp){
        Assert(emit_types.do_codepoint || emit_types.do_numbers);
        
        emits_out->steps[emits_out->step_count].type = 1;
        emits_out->steps[emits_out->step_count].value = ch;
        emits_out->steps[emits_out->step_count].i = i;
        emits_out->steps[emits_out->step_count].byte_length = 1;
        ++emits_out->step_count;
    }
}

internal void
translating_fully_process_byte(Translation_State *tran, u8 ch, u32 i, u32 size, Translation_Emits *emits_out){
    Translation_Byte_Description description = {0};
    translating_consume_byte(tran, ch, i, size, &description);
    Translation_Emit_Type emit_type = {0};
    translating_select_emit_type(tran, description, &emit_type);
    translating_generate_emits(tran, emit_type, ch, i, emits_out);
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

#define TRANSLATION_DECL_OUTPUT(_j,_emit) u32 _j = 0; _j < (_emit).step_count; ++_j
#define TRANSLATION_DECL_GET_STEP(_step,_behav,_j,_emit)                 \
Buffer_Model_Step _step = _emit.steps[_j]; Buffer_Model_Behavior _behav; \
translation_step_read(_step, &_behav)

#define TRANSLATION_OUTPUT(_j,_emit) _j = 0; _j < (_emit).step_count; ++_j
#define TRANSLATION_GET_STEP(_step,_behav,_j,_emit)\
(_step) = _emit.steps[_j]; translation_step_read((_step), &(_behav))

// BOTTOM



