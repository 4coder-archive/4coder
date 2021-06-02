/*
4coder_dynamic_bindings.cpp - Dynamic Bindings
*/

// TOP

function Key_Code
dynamic_binding_key_code_from_string(String_Const_u8 key_string){
    Key_Code result = 0;
    for (i32 i = 1; i < KeyCode_COUNT; i += 1){
        String_Const_u8 str = SCu8(key_code_name[i]);
        if (string_match(str, key_string)){
            result = i;
            break;
        }
    }
    return result;
}

function b32
dynamic_binding_load_from_file(Application_Links *app, Mapping *mapping, String_Const_u8 filename){
    b32 result = false;
    
    Scratch_Block scratch(app);
    
    String_Const_u8 filename_copied = push_string_copy(scratch, filename);
    String8List search_list = {};
    def_search_normal_load_list(scratch, &search_list);
    String_Const_u8 full_path = def_search_get_full_path(scratch, &search_list, filename_copied);
    
    {
        String8 message = push_stringf(scratch, "loading bindings: %.*s\n",
                                       string_expand(full_path));
        print_message(app, message);
    }
    
    FILE *file = 0;
    if (full_path.size > 0){
        file = fopen((char*)full_path.str, "rb");
    }
    
    if (file != 0){
        String_Const_u8 data = dump_file_handle(scratch, file);
        Config *parsed = def_config_from_text(app, scratch, filename, data);
		fclose(file);
        
        if (parsed != 0){
            result = true;
            
			Thread_Context* tctx = get_thread_context(app);
			mapping_release(tctx, mapping);
			mapping_init(tctx, mapping);
			MappingScope();
			SelectMapping(mapping);
            
            for (Config_Assignment *assignment = parsed->first;
                 assignment != 0;
                 assignment = assignment->next){
                Config_LValue *l = assignment->l;
                if (l != 0 && l->index == 0){
                    Config_Get_Result rvalue = config_evaluate_rvalue(parsed, assignment, assignment->r);
                    if (rvalue.type == ConfigRValueType_Compound){
                        String_Const_u8 map_name = l->identifier;
                        String_ID map_name_id = vars_save_string(map_name);
                        
                        SelectMap(map_name_id);
                        
                        
                        Config_Compound *compound = rvalue.compound;
                        
                        Config_Get_Result_List list = typed_compound_array_reference_list(scratch, parsed, compound);
                        for (Config_Get_Result_Node *node = list.first; node != 0; node = node->next){
                            Config_Compound *src = node->result.compound;
                            String_Const_u8 cmd_string = {0};
                            String_Const_u8 key_string = {0};
                            String_Const_u8 mod_string[9] = {0};
                            
                            if (!config_compound_string_member(parsed, src, "cmd", 0, &cmd_string)){
                                def_config_push_error(scratch, parsed, node->result.pos, "Command string is required in binding");
                                goto finish_map;
                            }
                            
                            if (!config_compound_string_member(parsed, src, "key", 1, &key_string)){
                                def_config_push_error(scratch, parsed, node->result.pos, "Key string is required in binding");
                                goto finish_map;
                            }
                            
                            for (i32 mod_idx = 0; mod_idx < ArrayCount(mod_string); mod_idx += 1){
                                String_Const_u8 str = push_stringf(scratch, "mod_%i", mod_idx);
                                if (config_compound_string_member(parsed, src, str, 2 + mod_idx, &mod_string[mod_idx])){
                                    // NOTE(rjf): No-Op
                                }
                            }
                            
                            // NOTE(rjf): Map read in successfully.
                            {
                                // NOTE(rjf): Find command.
                                Command_Metadata *command = get_command_metadata_from_name(cmd_string);
                                
                                // NOTE(rjf): Find keycode.
                                Key_Code keycode = dynamic_binding_key_code_from_string(key_string);
                                
                                // NOTE(rjf): Find mods.
                                i32 mod_count = 0;
                                Key_Code mods[ArrayCount(mod_string)] = {0};
                                for (i32 i = 0; i < ArrayCount(mod_string); i += 1){
                                    if (mod_string[i].str){
                                        mods[mod_count] = dynamic_binding_key_code_from_string(mod_string[i]);
                                        mod_count += 1;
                                    }
                                }
                                
                                if (keycode != 0 && command != 0){
                                    Input_Modifier_Set mods_set = { mods, mod_count, };
                                    map_set_binding(mapping, map, command->proc, InputEventKind_KeyStroke, keycode, &mods_set);
                                }
                                else{
                                    def_config_push_error(scratch, parsed, node->result.pos,
                                                          (keycode != 0) ? (char*)"Invalid command" :
                                                          (command != 0) ? (char*)"Invalid key":
                                                          (char*)"Invalid command and key");
                                }
                            }
                            
                            finish_map:;
                        }
                        
                        
                        if (parsed->errors.first != 0){
                            String_Const_u8 error_text = config_stringize_errors(app, scratch, parsed);
                            print_message(app, error_text);
                        }
                    }
                }
            }
        }
    }
    
    return(result);
}

// BOTTOM

