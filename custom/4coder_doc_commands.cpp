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

// BOTTOM

