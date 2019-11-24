function void
initialize_managed_id_metadata(Application_Links *app){
view_rewrite_loc = managed_id_declare(app, string_u8_litexpr("attachment"), string_u8_litexpr("view_rewrite_loc"));
view_next_rewrite_loc = managed_id_declare(app, string_u8_litexpr("attachment"), string_u8_litexpr("view_next_rewrite_loc"));
view_paste_index_loc = managed_id_declare(app, string_u8_litexpr("attachment"), string_u8_litexpr("view_paste_index_loc"));
view_is_passive_loc = managed_id_declare(app, string_u8_litexpr("attachment"), string_u8_litexpr("view_is_passive_loc"));
view_snap_mark_to_cursor = managed_id_declare(app, string_u8_litexpr("attachment"), string_u8_litexpr("view_snap_mark_to_cursor"));
view_ui_data = managed_id_declare(app, string_u8_litexpr("attachment"), string_u8_litexpr("view_ui_data"));
view_highlight_range = managed_id_declare(app, string_u8_litexpr("attachment"), string_u8_litexpr("view_highlight_range"));
view_highlight_buffer = managed_id_declare(app, string_u8_litexpr("attachment"), string_u8_litexpr("view_highlight_buffer"));
view_render_hook = managed_id_declare(app, string_u8_litexpr("attachment"), string_u8_litexpr("view_render_hook"));
view_word_complete_menu = managed_id_declare(app, string_u8_litexpr("attachment"), string_u8_litexpr("view_word_complete_menu"));
buffer_map_id = managed_id_declare(app, string_u8_litexpr("attachment"), string_u8_litexpr("buffer_map_id"));
buffer_eol_setting = managed_id_declare(app, string_u8_litexpr("attachment"), string_u8_litexpr("buffer_eol_setting"));
buffer_lex_task = managed_id_declare(app, string_u8_litexpr("attachment"), string_u8_litexpr("buffer_lex_task"));
buffer_wrap_lines = managed_id_declare(app, string_u8_litexpr("attachment"), string_u8_litexpr("buffer_wrap_lines"));
sticky_jump_marker_handle = managed_id_declare(app, string_u8_litexpr("attachment"), string_u8_litexpr("sticky_jump_marker_handle"));
attachment_tokens = managed_id_declare(app, string_u8_litexpr("attachment"), string_u8_litexpr("attachment_tokens"));
}
