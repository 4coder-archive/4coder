/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 02.03.2018
 *
 * Converter for *.4is -> *.4id
 *
 */

// TOP

#include "4ed_defines.h"
#include "4ed_input_simulation_event.h"
#include "4coder_lib/4coder_string.h"
#include "4coder_generated/style.h"
#include "4coder_API/types.h"
#include "4coder_generated/keycodes.h"

#include "4coder_file.h"

#include <stdio.h>

internal void
print_usage(char *name){
    fprintf(stdout,
            "usage: %s <src-root> [<src-root> ...]\n"
            "all files with the extension .4is in src-root will be converted\n",
            name);
}

// TODO(allen): // TODO(allen): // TODO(allen): // TODO(allen): 
// TODO(allen): // TODO(allen): // TODO(allen): // TODO(allen): 
// TODO(allen): // TODO(allen): // TODO(allen): // TODO(allen): 
// This belongs in the string library or something like that.
struct String_Array{
    String *strings;
    i32 count;
};

internal String_Array
get_lines(Partition *part, String data){
    String_Array array = {0};
    array.strings = push_array(part, String, 0);
    
    char *line_ptr = data.str;
    for (i32 i = 0; i <= data.size; ++i){
        char *c_ptr = data.str + i;
        b32 delim = false;
        if (i < data.size){
            switch (*c_ptr){
                case '\n': case '\r':
                {
                    delim = true;
                }break;
            }
        }
        else{
            delim = true;
        }
        
        if (delim){
            String s = make_string(line_ptr, (i32)(c_ptr - line_ptr));
            s = skip_chop_whitespace(s);
            if (s.size > 0){
                String *new_s = push_array(part, String, 1);
                *new_s = s;
                array.count += 1;
            }
            line_ptr = c_ptr + 1;
        }
    }
    
    return(array);
}

internal String_Array
get_words(Partition *part, String data){
    String_Array array = {0};
    array.strings = push_array(part, String, 0);
    
    char *word_ptr = data.str;
    for (i32 i = 0; i <= data.size; ++i){
        char *c_ptr = data.str + i;
        b32 delim = false;
        if (i < data.size){
            delim = char_is_whitespace(*c_ptr);
        }
        else{
            delim = true;
        }
        
        if (delim){
            String s = make_string(word_ptr, (i32)(c_ptr - word_ptr));
            if (s.size > 0){
                String *new_s = push_array(part, String, 1);
                *new_s = s;
                array.count += 1;
            }
            word_ptr = c_ptr + 1;
        }
    }
    
    return(array);
}

internal String_Array
get_flags(Partition *part, String data){
    String_Array array = {0};
    array.strings = push_array(part, String, 0);
    
    char *word_ptr = data.str;
    for (i32 i = 0; i <= data.size; ++i){
        char *c_ptr = data.str + i;
        b32 delim = false;
        if (i < data.size){
            delim = (*c_ptr == '|');
        }
        else{
            delim = true;
        }
        
        if (delim){
            String s = make_string(word_ptr, (i32)(c_ptr - word_ptr));
            s = skip_chop_whitespace(s);
            if (s.size > 0){
                String *new_s = push_array(part, String, 1);
                *new_s = s;
                array.count += 1;
            }
            word_ptr = c_ptr + 1;
        }
    }
    
    return(array);
}

internal void
show_error(char *name, String data, char *ptr, char *error_message){
    i32 line = 1;
    i32 column = 1;
    
    i32 stop = (i32)(ptr - data.str);
    if (stop > data.size){
        stop = data.size;
    }
    for (i32 i = 0; i < stop; ++i){
        if (data.str[i] == '\n'){
            line += 1;
            column = 1;
        }
        else{
            column += 1;
        }
    }
    
    fprintf(stdout, "%s:%d:%d: error %s\n", name, line, column, error_message);
}

struct Line_Parse_Context{
    char *name;
    String data;
    String_Array words;
};

internal void
show_error(Line_Parse_Context context, char *ptr, char *error_message){
    show_error(context.name, context.data, ptr, error_message);
}

internal bool32
require_blank(Line_Parse_Context context, i32 index){
    bool32 result = (context.words.count <= index);
    if (!result){
        show_error(context, context.words.strings[index].str,
                   "unexpected word");
    }
    return(result);
}

internal bool32
require_integer(Line_Parse_Context context, i32 index, i32 *int_out){
    bool32 result = false;
    if (index < context.words.count){
        String s = context.words.strings[index];
        if (str_is_int(s)){
            *int_out = str_to_int(s);
            result = true;
        }
        else{
            show_error(context,
                       context.words.strings[index].str,
                       "expected integer");
        }
    }
    else{
        show_error(context,
                   context.words.strings[context.words.count - 1].str,
                   "expected integer");
    }
    return(result);
}

internal bool32
require_unquoted_string(Line_Parse_Context context, i32 index, String *str_out){
    bool32 result = false;
    if (index < context.words.count){
        String str = context.words.strings[index];
        if (str.str[0] != '"'){
            *str_out = str;
            result = true;
        }
        else{
            show_error(context,
                       context.words.strings[context.words.count - 1].str,
                       "expected a simple word (a simple word must be unquoted)");
        }
    }
    else{
        show_error(context,
                   context.words.strings[context.words.count - 1].str,
                   "expected another word");
    }
    return(result);
}

internal bool32
require_unquoted_multi_string(Line_Parse_Context context, i32 start_index, String *str_out){
    bool32 result = false;
    if (start_index < context.words.count){
        String str = context.words.strings[start_index];
        if (str.str[0] != '"'){
            String last_word = context.words.strings[context.words.count - 1];
            char *end = last_word.str + last_word.size;
            str.size = (i32)(end - str.str);
            *str_out = str;
            result = true;
        }
        else{
            show_error(context,
                       context.words.strings[context.words.count - 1].str,
                       "expected a simple word (a simple word must be unquoted)");
        }
    }
    else{
        show_error(context,
                   context.words.strings[context.words.count - 1].str,
                   "expected another word");
    }
    return(result);
}

internal bool32
require_any_string(Line_Parse_Context context, i32 index, String *str_out){
    bool32 result = require_unquoted_string(context, index, str_out);
    return(result);
}

internal bool32
key_name_to_code(Line_Parse_Context context, String key_name, u32 *key_code_out){
    bool32 result = false;
    if (key_name.size == 1){
        *key_code_out = key_name.str[0];
        result = true;
    }
    else{
#define KEY_CODE_CHK_SET(S,N) else if (match(key_name, S)) \
        do{ *key_code_out = N; result = true; }while(0)
#define KEY_CODE_CHK(N) KEY_CODE_CHK_SET(#N,N)
        
        if (false){}
        KEY_CODE_CHK(key_back);
        KEY_CODE_CHK(key_up);
        KEY_CODE_CHK(key_down);
        KEY_CODE_CHK(key_left);
        KEY_CODE_CHK(key_right);
        KEY_CODE_CHK(key_del);
        KEY_CODE_CHK(key_insert);
        KEY_CODE_CHK(key_home);
        KEY_CODE_CHK(key_end);
        KEY_CODE_CHK(key_page_up);
        KEY_CODE_CHK(key_page_down);
        KEY_CODE_CHK(key_esc);
        KEY_CODE_CHK(key_f1);
        KEY_CODE_CHK(key_f2);
        KEY_CODE_CHK(key_f3);
        KEY_CODE_CHK(key_f4);
        KEY_CODE_CHK(key_f5);
        KEY_CODE_CHK(key_f6);
        KEY_CODE_CHK(key_f7);
        KEY_CODE_CHK(key_f8);
        KEY_CODE_CHK(key_f9);
        KEY_CODE_CHK(key_f10);
        KEY_CODE_CHK(key_f11);
        KEY_CODE_CHK(key_f12);
        KEY_CODE_CHK(key_f13);
        KEY_CODE_CHK(key_f14);
        KEY_CODE_CHK(key_f15);
        KEY_CODE_CHK(key_f16);
        KEY_CODE_CHK_SET("key_space", ' ');
        KEY_CODE_CHK_SET("key_newline", '\n');
        KEY_CODE_CHK_SET("key_tab", '\t');
    }
    
    if (!result){
        show_error(context, key_name.str, "expected key name");
    }
    return(result);
}

internal bool32
mod_name_to_flags(Line_Parse_Context context, Partition *part, String mod_name, u8 *modifiers_out){
    bool32 result = true;
    
    Temp_Memory temp = begin_temp_memory(part);
    String_Array flags = get_flags(part, mod_name);
    u8 modifiers = 0;
    for (i32 i = 0; i < flags.count; ++i){
        String flag_string = flags.strings[i];
        u8 this_flag = 0;
        
#define MDFR_FLAG_CHK(N) \
        else if (match(flag_string, #N)) do{ this_flag = N; }while(0)
        
        if (false){}
        MDFR_FLAG_CHK(MDFR_NONE);
        MDFR_FLAG_CHK(MDFR_CTRL);
        MDFR_FLAG_CHK(MDFR_ALT);
        MDFR_FLAG_CHK(MDFR_CMND);
        MDFR_FLAG_CHK(MDFR_SHIFT);
        else{
            result = false;
            show_error(context, flag_string.str, "unrecognized flag string");
            break;
        }
        
        modifiers |= this_flag;
    }
    end_temp_memory(temp);
    
    *modifiers_out = modifiers;
    return(result);
}

internal void
process_script__inner(Partition *scratch, char *name){
    String data = file_dump(scratch, name);
    String_Array lines = get_lines(scratch, data);
    
    Simulation_Event *events = push_array(scratch, Simulation_Event, 0);
    i32 event_count = 0;
    
    i32 standard_time_increment = 0;
    i32 time_counter = 0;
    
    for (i32 i = 0; i < lines.count; ++i){
        Temp_Memory word_temp = begin_temp_memory(scratch);
        String line = lines.strings[i];
        String_Array words = get_words(scratch, line);
        
        Line_Parse_Context context = {0};
        context.name = name;
        context.data = data;
        context.words = words;
        
        i32 current_debug_number = 0;
        
        bool32 emit_event = false;
        Simulation_Event event = {0};
        
        bool32 emit_type = false;
        i32 type_increment = 0;
        String type_string = {0};
        
        bool32 emit_invoke = false;
        String invoke_file = {0};
        bool32 invoke_raw_data = false;
        
        if (words.count != 0){
            String first_word = words.strings[0];
            if (!match(substr(first_word, 0, 2), "//")){
                
                if (match(first_word, "debug_number")){
                    i32 debug_number = 0;
                    if (require_integer(context, 1, &debug_number) &&
                        require_blank(context, 2)){
                        emit_event = true;
                        event.counter_index = time_counter;
                        event.type = SimulationEvent_DebugNumber;
                        event.debug_number = debug_number;
                        current_debug_number = debug_number;
                    }
                    else{
                        return;
                    }
                }
                
                else if (match(first_word, "wait")){
                    i32 increment = 0;
                    if (require_integer(context, 1, &increment) &&
                        require_blank(context, 2)){
                        time_counter += increment;
                    }
                    else{
                        return;
                    }
                }
                
                else if (match(first_word, "basewait")){
                    i32 increment = 0;
                    if (require_integer(context, 1, &increment) &&
                        require_blank(context, 2)){
                        standard_time_increment = increment;
                    }
                    else{
                        return;
                    }
                }
                
                else if (match(first_word, "key")){
                    String key_name = {0};
                    String mod_name = {0};
                    if (require_unquoted_string(context, 1, &key_name) &&
                        require_unquoted_string(context, 2, &mod_name) &&
                        require_blank(context, 3)){
                        u32 key_code = 0;
                        u8 modifiers = 0;
                        if (key_name_to_code(context, key_name, &key_code) &&
                            mod_name_to_flags(context, scratch, mod_name, &modifiers)){
                            emit_event = true;
                            event.counter_index = time_counter;
                            event.type = SimulationEvent_Key;
                            event.key.code = key_code;
                            event.key.modifiers = modifiers;
                        }
                        else{
                            return;
                        }
                    }
                    else{
                        return;
                    }
                }
                
                else if (match(first_word, "type")){
                    i32 increment = 0;
                    String string = {0};
                    if (require_integer(context, 1, &increment) &&
                        require_unquoted_multi_string(context, 2, &string)){
                        emit_type = true;
                        type_increment = increment;
                        type_string = string;
                    }
                    else{
                        return;
                    }
                }
                
                else if (match(first_word, "invoke")){
                    String file = {0};
                    if (require_any_string(context, 1, &file) &&
                        require_blank(context, 2)){
                        emit_invoke = true;
                        invoke_file = file;
                    }
                    else{
                        return;
                    }
                }
                
                else if (match(first_word, "raw_invoke")){
                    String file = {0};
                    if (require_any_string(context, 1, &file) &&
                        require_blank(context, 2)){
                        emit_invoke = true;
                        invoke_file = file;
                        invoke_raw_data = true;
                    }
                    else{
                        return;
                    }
                }
                
                else if (match(first_word, "mouse_left_press")){
                    if (require_blank(context, 1)){
                        emit_event = true;
                        event.counter_index = time_counter;
                        event.type = SimulationEvent_MouseLeftPress;
                    }
                    else{
                        return;
                    }
                }
                
                else if (match(first_word, "mouse_right_press")){
                    if (require_blank(context, 1)){
                        emit_event = true;
                        event.counter_index = time_counter;
                        event.type = SimulationEvent_MouseRightPress;
                    }
                    else{
                        return;
                    }
                }
                
                else if (match(first_word, "mouse_left_release")){
                    if (require_blank(context, 1)){
                        emit_event = true;
                        event.counter_index = time_counter;
                        event.type = SimulationEvent_MouseLeftRelease;
                    }
                    else{
                        return;
                    }
                }
                
                else if (match(first_word, "mouse_right_release")){
                    if (require_blank(context, 1)){
                        emit_event = true;
                        event.counter_index = time_counter;
                        event.type = SimulationEvent_MouseRightRelease;
                    }
                    else{
                        return;
                    }
                }
                
                else if (match(first_word, "mouse_wheel")){
                    i32 wheel = 0;
                    if (require_integer(context, 1, &wheel) &&
                        require_blank(context, 2)){
                        emit_event = true;
                        event.counter_index = time_counter;
                        event.type = SimulationEvent_MouseWheel;
                        event.wheel = wheel;
                    }
                    else{
                        return;
                    }
                }
                
                else if (match(first_word, "mouse_xy")){
                    i32 x = 0;
                    i32 y = 0;
                    if (require_integer(context, 1, &x) &&
                        require_integer(context, 2, &y) &&
                        require_blank(context, 3)){
                        emit_event = true;
                        event.counter_index = time_counter;
                        event.type = SimulationEvent_MouseXY;
                        event.mouse_xy.x = x;
                        event.mouse_xy.y = y;
                    }
                    else{
                        return;
                    }
                }
                
                else if (match(first_word, "exit")){
                    if (require_blank(context, 1)){
                        emit_event = true;
                        event.counter_index = time_counter;
                        event.type = SimulationEvent_Exit;
                    }
                    else{
                        return;
                    }
                }
                
                else{
                    show_error(name, data, first_word.str, "unrecognized control word");
                    return;
                }
            }
        }
        
        end_temp_memory(word_temp);
        
        if (emit_event){
            Simulation_Event *new_event = push_array(scratch, Simulation_Event, 1);
            memset(new_event, 0, sizeof(*new_event));
            *new_event = event;
            event_count += 1;
            time_counter += standard_time_increment;
        }
        
        if (emit_type){
            for (i32 j = 0; j < type_string.size; ++j){
                Simulation_Event *new_event = push_array(scratch, Simulation_Event, 1);
                memset(new_event, 0, sizeof(*new_event));
                new_event->counter_index = time_counter;
                new_event->type = SimulationEvent_Key;
                new_event->key.code = type_string.str[j];
                new_event->key.modifiers = MDFR_NONE;
                event_count += 1;
                if (j + 1 < type_string.size){
                    time_counter += type_increment;
                }
            }
            time_counter += standard_time_increment;
        }
        
        if (emit_invoke){
            Temp_Memory invoke_temp = begin_temp_memory(scratch);
            
            char *invoke_name = push_array(scratch, char, invoke_file.size + 1);
            push_align(scratch, 8);
            memcpy(invoke_name, invoke_file.str, invoke_file.size);
            invoke_name[invoke_file.size] = 0;
            String invoke_data = file_dump(scratch, invoke_name);
            if (invoke_data.str == 0){
                show_error(name, data, invoke_file.str, "could not open invoked file");
                return;
            }
            i32 count = *(i32*)invoke_data.str;
            Simulation_Event *events = (Simulation_Event*)(invoke_data.str + 4);
            Simulation_Event *event = events;
            for (i32 i = 0; i < count; ++i, ++event){
                event->counter_index = event->counter_index + time_counter;
                if (event->type == SimulationEvent_Exit && !invoke_raw_data){
                    count = i + 1;
                    event->type = SimulationEvent_DebugNumber;
                }
                if (event->type == SimulationEvent_DebugNumber){
                    event->debug_number = current_debug_number;
                }
            }
            if (count > 0){
                time_counter = events[count - 1].counter_index + standard_time_increment;
            }
            end_temp_memory(invoke_temp);
            
            // NOTE(allen): This is pulling back events from inside a
            // closed temp block.  Don't let it get separated from the
            // end_temp_memory call!
            void *ptr = push_array(scratch, Simulation_Event, count);
            memmove(ptr, events, sizeof(*events)*count);
            event_count += count;
        }
    }
    
    String out_name_s = front_of_directory(make_string_slowly(name));
    char *out_name = push_array(scratch, char, out_name_s.size + 1);
    memcpy(out_name, out_name_s.str, out_name_s.size);
    Assert(out_name[out_name_s.size - 1] == 's');
    out_name[out_name_s.size - 1] = 'd';
    out_name[out_name_s.size] = 0;
    
    FILE *out = fopen(out_name, "wb");
    if (out != 0){
        fwrite(&event_count, sizeof(event_count), 1, out);
        fwrite(events, sizeof(*events), event_count, out);
        fclose(out);
    }
    else{
        fprintf(stdout, "fatal error: cannot open output %s\n",
                out_name);
    }
}

internal void
process_script(Partition *scratch, char *name){
    Temp_Memory temp = begin_temp_memory(scratch);
    process_script__inner(scratch, name);
    end_temp_memory(temp);
}

int
main(int argc, char **argv){
    if (argc <= 1){
        char *name = "test_builder";
        if (argc > 0){
            name = argv[0];
        }
        print_usage(name);
    }
    
    int32_t size = (256 << 20);
    void *mem = malloc(size);
    Partition part_ = make_part(mem, size);
    Partition *part = &part_;
    
    for (i32 i = 1; i < argc; ++i){
        Cross_Platform_File_List files = get_file_list(part, encode(part, argv[i]), filter_all);
        
        char *path_name = unencode(part, files.path_name, files.path_length);
        String path_name_s = make_string_slowly(path_name);
        
        Cross_Platform_File_Info *info = files.info;
        for (i32 j = 0; j < files.count; ++j, ++info){
            if (info->is_folder){
                continue;
            }
            
            char *name = unencode(part, info->name, info->len);
            String s = make_string_slowly(name);
            if (!match(substr_tail(s, s.size - 4), ".4is")){
                continue;
            }
            
            i32 whole_name_max = path_name_s.size + 1 + s.size + 1;
            char *whole_name = push_array(part, char, whole_name_max);
            push_align(part, 8);
            
            String w = make_string_cap(whole_name, 0, whole_name_max);
            append(&w, path_name_s);
            append(&w, '/');
            append(&w, s);
            terminate_with_null(&w);
            
            process_script(part, w.str);
        }
    }
}

// BOTTOM

