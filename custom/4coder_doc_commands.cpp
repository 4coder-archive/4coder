/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 14.12.2019
 *
 * Documentation of the custom layer's primary api.
 *
 */

// TOP

function Doc_Cluster*
doc_commands(Arena *arena){
    Doc_Cluster *cluster = new_doc_cluster(arena, "Commands", "commands");
    for (i32 i = 0; i < ArrayCount(fcoder_metacmd_table); i += 1){
        String_Const_u8 cmd_name = SCu8(fcoder_metacmd_table[i].name,
                                        fcoder_metacmd_table[i].name_len);
        String_Const_u8 title = push_u8_stringf(arena, "Command %.*s", string_expand(cmd_name));
        Doc_Page *page = new_doc_page(arena, cluster, (char*)title.str, (char*)cmd_name.str);
        Doc_Block *block = new_doc_block(arena, page, "brief");
        doc_text(arena, block, fcoder_metacmd_table[i].description);
    }
    return(cluster);
}

function Doc_Cluster*
doc_default_bindings(Arena *arena, i32 map_count, Mapping *mapping_array, char **page_titles, char **page_names,
                     i64 global_id, i64 file_id, i64 code_id){
    Doc_Cluster *cluster = new_doc_cluster(arena, "Bindings", "bindings");
    
    for (i32 i = 0; i < map_count; i += 1){
        Mapping *mapping = &mapping_array[i];
        Doc_Page *page = new_doc_page(arena, cluster, page_titles[i], page_names[i]);
        for (Command_Map *map = mapping->first_map;
             map != 0;
             map = map->next){
            char *map_name = "";
            if (map->id == global_id){
                map_name = "Global";
            }
            else if (map->id == file_id){
                map_name = "File";
            }
            else if (map->id == code_id){
                map_name = "Code";
            }
            
            Doc_Block *block = new_doc_block(arena, page, map_name);
            Doc_Paragraph *par = new_doc_par_table(arena, block);
            
            struct Bind_Node{
                Bind_Node *next;
                Input_Event_Kind kind;
                u32 sub_code;
                Input_Modifier_Set mods;
                Command_Binding binding;
                u32 j;
            };
            
            Bind_Node *first = 0;
            Bind_Node *last = 0;
            i32 node_count = 0;
            
            if (map->text_input_command.name != 0){
                Bind_Node *node = push_array_zero(arena, Bind_Node, 1);
                sll_queue_push(first, last, node);
                node_count += 1;
                node->binding = map->text_input_command;
                node->j = max_u32;
            }
            
            u32 counts[] = {
                KeyCode_COUNT,
                KeyCode_COUNT,
                MouseCode_COUNT,
                MouseCode_COUNT,
                1,
                1,
                CoreCode_COUNT,
            };
            
            u32 event_codes[] = {
                InputEventKind_KeyStroke,
                InputEventKind_KeyRelease,
                InputEventKind_MouseButton,
                InputEventKind_MouseButtonRelease,
                InputEventKind_MouseWheel,
                InputEventKind_MouseMove,
                InputEventKind_Core,
            };
            
            char *mouse_wheel_name[] = {"MoveWheel"};
            char *mouse_move_name[] = {"MoveMove"};
            
            char **event_names[] = {
                key_code_name,
                key_code_name,
                mouse_code_name,
                mouse_code_name,
                mouse_wheel_name,
                mouse_move_name,
                core_code_name,
            };
            
            b32 is_release[] = {
                false,
                true,
                false,
                true,
                false,
                false,
                false,
            };
            
            for (u32 j = 0; j < ArrayCount(counts); j += 1){
                for (u32 code = 0; code < counts[j]; code += 1){
                    u64 key = mapping__key(event_codes[j], code);
                    Table_Lookup lookup = table_lookup(&map->event_code_to_binding_list, key);
                    if (lookup.found_match){
                        u64 val = 0;
                        table_read(&map->event_code_to_binding_list, lookup, &val);
                        Command_Binding_List *list = (Command_Binding_List*)IntAsPtr(val);
                        for (SNode *snode = list->first;
                             snode != 0;
                             snode = snode->next){
                            Command_Modified_Binding *mod_binding = CastFromMember(Command_Modified_Binding, order_node, snode);
                            
                            Bind_Node *node = push_array_zero(arena, Bind_Node, 1);
                            sll_queue_push(first, last, node);
                            node_count += 1;
                            node->kind = event_codes[j];
                            node->sub_code = code;
                            node->mods = mod_binding->mods;
                            node->binding = mod_binding->binding;
                            node->j = j;
                        }
                    }
                }
            }
            
            Vec2_i32 table_dims = V2i32(2, node_count);
            Doc_Content_List *vals = push_array_zero(arena, Doc_Content_List, table_dims.x*table_dims.y);
            Bind_Node *bnode = first;
            for (i32 y = 0; y < table_dims.y; y += 1, bnode = bnode->next){
                Doc_Content_List *line = &vals[y*table_dims.x];
                doc_text(arena, &line[0], "[");
                if (bnode->j != max_u32){
                    doc_text(arena, &line[0], event_names[bnode->j][bnode->sub_code]);
                    if (is_release[bnode->j]){
                        doc_text(arena, &line[0], "Release");
                    }
                    
                    Input_Modifier_Set *mods = &bnode->mods;
                    for (i32 k = 0; k < mods->count; k += 1){
                        doc_text(arena, &line[0], key_code_name[mods->mods[k]]);
                    }
                }
                else{
                    doc_text(arena, &line[0], "TextInput");
                }
                doc_text(arena, &line[0], "]");
                
                Doc_Content *content = doc_text(arena, &line[1], bnode->binding.name);
                content->page_link = SCu8(bnode->binding.name);
            }
            
            par->table.dim = table_dims;
            par->table.vals = vals;
        }
    }
    
    return(cluster);
}

// BOTTOM

