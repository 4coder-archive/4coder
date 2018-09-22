struct Interactive_Style{
u32 bar_color;
u32 bar_active_color;
u32 base_color;
u32 pop1_color;
u32 pop2_color;
};

struct Style_Main_Data{
u32 back_color;
u32 margin_color;
u32 margin_hover_color;
u32 margin_active_color;
u32 list_item_color;
u32 list_item_hover_color;
u32 list_item_active_color;
u32 cursor_color;
u32 at_cursor_color;
u32 highlight_cursor_line_color;
u32 highlight_color;
u32 at_highlight_color;
u32 mark_color;
u32 default_color;
u32 comment_color;
u32 keyword_color;
u32 str_constant_color;
u32 char_constant_color;
u32 int_constant_color;
u32 float_constant_color;
u32 bool_constant_color;
u32 preproc_color;
u32 include_color;
u32 special_character_color;
u32 ghost_character_color;
u32 highlight_junk_color;
u32 highlight_white_color;
u32 paste_color;
u32 undo_color;
u32 next_undo_color;
Interactive_Style file_info_style;
};

inline u32*
style_index_by_tag(Style_Main_Data *s, u32 tag){
u32 *result = 0;
switch (tag){
case Stag_Bar: result = &s->file_info_style.bar_color; break;
case Stag_Bar_Active: result = &s->file_info_style.bar_active_color; break;
case Stag_Base: result = &s->file_info_style.base_color; break;
case Stag_Pop1: result = &s->file_info_style.pop1_color; break;
case Stag_Pop2: result = &s->file_info_style.pop2_color; break;
case Stag_Back: result = &s->back_color; break;
case Stag_Margin: result = &s->margin_color; break;
case Stag_Margin_Hover: result = &s->margin_hover_color; break;
case Stag_Margin_Active: result = &s->margin_active_color; break;
case Stag_List_Item: result = &s->list_item_color; break;
case Stag_List_Item_Hover: result = &s->list_item_hover_color; break;
case Stag_List_Item_Active: result = &s->list_item_active_color; break;
case Stag_Cursor: result = &s->cursor_color; break;
case Stag_At_Cursor: result = &s->at_cursor_color; break;
case Stag_Highlight_Cursor_Line: result = &s->highlight_cursor_line_color; break;
case Stag_Highlight: result = &s->highlight_color; break;
case Stag_At_Highlight: result = &s->at_highlight_color; break;
case Stag_Mark: result = &s->mark_color; break;
case Stag_Default: result = &s->default_color; break;
case Stag_Comment: result = &s->comment_color; break;
case Stag_Keyword: result = &s->keyword_color; break;
case Stag_Str_Constant: result = &s->str_constant_color; break;
case Stag_Char_Constant: result = &s->char_constant_color; break;
case Stag_Int_Constant: result = &s->int_constant_color; break;
case Stag_Float_Constant: result = &s->float_constant_color; break;
case Stag_Bool_Constant: result = &s->bool_constant_color; break;
case Stag_Preproc: result = &s->preproc_color; break;
case Stag_Include: result = &s->include_color; break;
case Stag_Special_Character: result = &s->special_character_color; break;
case Stag_Ghost_Character: result = &s->ghost_character_color; break;
case Stag_Highlight_Junk: result = &s->highlight_junk_color; break;
case Stag_Highlight_White: result = &s->highlight_white_color; break;
case Stag_Paste: result = &s->paste_color; break;
case Stag_Undo: result = &s->undo_color; break;
case Stag_Next_Undo: result = &s->next_undo_color; break;
}
return(result);
}

