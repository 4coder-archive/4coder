/*
4coder_layout_rule.cpp - Built in layout rules and layout rule helpers.
*/

// TOP

function Layout_Reflex
get_layout_reflex(Layout_Item_List *list, Buffer_ID buffer, f32 width, Face_ID face){
Layout_Reflex reflex = {};
reflex.list = list;
reflex.buffer = buffer;
reflex.width = width;
reflex.face = face;
return(reflex);
}

function Rect_f32
layout_reflex_get_rect(Application_Links *app, Layout_Reflex *reflex, i64 pos, b32 *unresolved_dependence){
Rect_f32 rect = {};
pos = clamp_bot(0, pos);
if (range_contains(reflex->list->input_index_range, pos)){
if (range_contains(reflex->list->manifested_index_range, pos)){
rect = layout_box_of_pos(*reflex->list, pos);
*unresolved_dependence = false;
}
else{
*unresolved_dependence = true;
}
}
else{
Buffer_Cursor cursor = buffer_compute_cursor(app, reflex->buffer, seek_pos(pos));
rect = buffer_relative_box_of_pos(app, reflex->buffer, reflex->width, reflex->face, cursor.line, cursor.pos);
*unresolved_dependence = false;
}
return(rect);
}

////////////////////////////////

function i64
layout_index_from_ptr(u8 *ptr, u8 *string_base, i64 index_base){
return((i64)(ptr - string_base) + index_base);
}

function Layout_Item_List
get_empty_item_list(Range_i64 input_range){
Layout_Item_List list = {};
list.input_index_range = input_range;
list.manifested_index_range = Ii64_neg_inf;
return(list);
}

function void
layout_item_list_finish(Layout_Item_List *list, f32 bottom_padding){
list->bottom_padding = bottom_padding;
list->height += bottom_padding;
}

function void
layout_write(Arena *arena, Layout_Item_List *list, Face_ID face, i64 index, u32 codepoint, Layout_Item_Flag flags, Rect_f32 rect, f32 padded_y1){
Temp_Memory restore_point = begin_temp(arena);
Layout_Item *item = push_array(arena, Layout_Item, 1);
Layout_Item_Block *block = list->last;
if (block != 0){
if (block->face != face){
block = 0;
}
else if (block->items + block->item_count == item){
block->item_count += 1;
}
else{
block = 0;
}
}
if (block == 0){
end_temp(restore_point);
block = push_array(arena, Layout_Item_Block, 1);
item = push_array(arena, Layout_Item, 1);
sll_queue_push(list->first, list->last, block);
list->node_count += 1;
block->items = item;
block->item_count = 1;
block->face = face;
}

list->item_count += 1;
list->manifested_index_range.min = Min(list->manifested_index_range.min, index);
list->manifested_index_range.max = Max(list->manifested_index_range.max, index);

if (!HasFlag(flags, LayoutItemFlag_Ghost_Character)){
block->character_count += 1;
list->character_count += 1;
}

item->index = index;
item->codepoint = codepoint;
item->flags = flags;
item->rect = rect;
item->padded_y1 = padded_y1;
list->height = Max(list->height, rect.y1);
}

////

function Newline_Layout_Vars
get_newline_layout_vars(void){
Newline_Layout_Vars result = {};
result.newline_character_index = -1;
return(result);
}

function void
newline_layout_consume_CR(Newline_Layout_Vars *vars, i64 index){
if (!vars->consuming_newline_characters){
vars->consuming_newline_characters = true;
vars->newline_character_index = index;
}
vars->prev_did_emit_newline = false;
}

function i64
newline_layout_consume_LF(Newline_Layout_Vars *vars, i64 index){
if (!vars->consuming_newline_characters){
vars->newline_character_index = index;
}
vars->prev_did_emit_newline = true;
vars->consuming_newline_characters = false;
return(vars->newline_character_index);
}

function void
newline_layout_consume_default(Newline_Layout_Vars *vars){
vars->consuming_newline_characters = false;
vars->prev_did_emit_newline = false;
}

function b32
newline_layout_consume_finish(Newline_Layout_Vars *vars){
return((!vars->prev_did_emit_newline));
}

////

function LefRig_TopBot_Layout_Vars
get_lr_tb_layout_vars(Face_Advance_Map *advance_map, Face_Metrics *metrics, f32 tab_width, f32 width){
f32 text_height = metrics->text_height;
f32 line_height = metrics->line_height;

LefRig_TopBot_Layout_Vars result = {};
result.advance_map = advance_map;
result.metrics = metrics;
result.tab_width = tab_width;
result.line_to_text_shift = text_height - line_height;

result.blank_dim = V2f32(metrics->space_advance, text_height);

result.line_y = line_height;
result.text_y = text_height;
result.width = width;
return(result);
}

function b32
lr_tb_crosses_width(LefRig_TopBot_Layout_Vars *vars, f32 advance, f32 width){
return(vars->p.x + advance > width);
}

function b32
lr_tb_crosses_width(LefRig_TopBot_Layout_Vars *vars, f32 advance){
return(vars->p.x + advance > vars->width);
}

function f32
lr_tb_advance(LefRig_TopBot_Layout_Vars *vars, Face_ID face, u32 codepoint){
return(font_get_glyph_advance(vars->advance_map, vars->metrics, codepoint, vars->tab_width));
}

function void
lr_tb_write_with_advance_with_flags(LefRig_TopBot_Layout_Vars *vars, Face_ID face, f32 advance, Arena *arena, Layout_Item_List *list, i64 index, u32 codepoint, Layout_Item_Flag flags){
if (codepoint == '\t'){
codepoint = ' ';
}
vars->p.x = f32_ceil32(vars->p.x);
f32 next_x = vars->p.x + advance;
layout_write(arena, list, face, index, codepoint, flags, Rf32(vars->p, V2f32(next_x, vars->text_y)), vars->line_y);
vars->p.x = next_x;
}

function void
lr_tb_write_with_advance(LefRig_TopBot_Layout_Vars *vars, Face_ID face, f32 advance, Arena *arena, Layout_Item_List *list, i64 index, u32 codepoint){
lr_tb_write_with_advance_with_flags(vars, face, advance, arena, list, index, codepoint, 0);
}

function void
lr_tb_write(LefRig_TopBot_Layout_Vars *vars, Face_ID face, Arena *arena, Layout_Item_List *list, i64 index, u32 codepoint){
f32 advance = lr_tb_advance(vars, face, codepoint);
lr_tb_write_with_advance(vars, face, advance, arena, list, index, codepoint);
}

function void
lr_tb_write_ghost(LefRig_TopBot_Layout_Vars *vars, Face_ID face, Arena *arena, Layout_Item_List *list, i64 index, u32 codepoint){
f32 advance = lr_tb_advance(vars, face, codepoint);
lr_tb_write_with_advance_with_flags(vars, face, advance, arena, list, index, codepoint, LayoutItemFlag_Ghost_Character);
}

function f32
lr_tb_advance_byte(LefRig_TopBot_Layout_Vars *vars){
return(vars->metrics->byte_advance);
}

function void
lr_tb_write_byte_with_advance(LefRig_TopBot_Layout_Vars *vars, Face_ID face, f32 advance, Arena *arena, Layout_Item_List *list, i64 index, u8 byte){
Face_Metrics *metrics = vars->metrics;

f32 final_next_x = vars->p.x + advance;
u32 lo = ((u32)byte     )&0xF;
u32 hi = ((u32)byte >> 4)&0xF;

Vec2_f32 p = vars->p;
p.x = f32_ceil32(p.x);
f32 next_x = p.x + metrics->byte_sub_advances[0];
f32 text_y = vars->text_y;

Layout_Item_Flag flags = LayoutItemFlag_Special_Character;
layout_write(arena, list, face, index, '\\', flags, Rf32(p, V2f32(next_x, text_y)), vars->line_y);
p.x = next_x;

flags = LayoutItemFlag_Ghost_Character;
next_x += metrics->byte_sub_advances[1];
layout_write(arena, list, face, index, integer_symbols[hi], flags, Rf32(p, V2f32(next_x, text_y)), vars->line_y);
p.x = next_x;
next_x += metrics->byte_sub_advances[2];
layout_write(arena, list, face, index, integer_symbols[lo], flags, Rf32(p, V2f32(next_x, text_y)), vars->line_y);

vars->p.x = final_next_x;
}

function void
lr_tb_write_byte(LefRig_TopBot_Layout_Vars *vars, Face_ID face,
                 Arena *arena, Layout_Item_List *list, i64 index, u8 byte){
lr_tb_write_byte_with_advance(vars, face, vars->metrics->byte_advance,
                              arena, list, index, byte);
}

function void
lr_tb_write_blank_dim(LefRig_TopBot_Layout_Vars *vars, Face_ID face, Vec2_f32 dim,
                      Arena *arena, Layout_Item_List *list, i64 index){
layout_write(arena, list, face, index, ' ', 0, Rf32_xy_wh(vars->p, dim), vars->line_y);
vars->p.x += dim.x;
}

function void
lr_tb_write_blank(LefRig_TopBot_Layout_Vars *vars, Face_ID face,
                  Arena *arena, Layout_Item_List *list, i64 index){
lr_tb_write_blank_dim(vars, face, vars->blank_dim, arena, list, index);
}

function void
lr_tb_next_line(LefRig_TopBot_Layout_Vars *vars){
vars->p.x = 0.f;
vars->p.y = vars->line_y;
vars->line_y += vars->metrics->line_height;
vars->text_y = vars->line_y + vars->line_to_text_shift;
}

function void
lr_tb_next_line_padded(LefRig_TopBot_Layout_Vars *vars, f32 top, f32 bot){
vars->p.x = 0.f;
vars->p.y = vars->line_y + top;
vars->line_y += top + vars->metrics->line_height;
vars->text_y = vars->line_y + vars->line_to_text_shift;
vars->line_y += bot;
}

function void
lr_tb_advance_x_without_item(LefRig_TopBot_Layout_Vars *vars, f32 advance){
vars->p.x += advance;
}

function void
lr_tb_align_rightward(LefRig_TopBot_Layout_Vars *vars, f32 align_x){
vars->p.x = clamp_bot(align_x, vars->p.x);
}

////////////////////////////////

function Layout_Item_List
layout_unwrapped_small_blank_lines(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 range, Face_ID face, f32 width){
Layout_Item_List list = get_empty_item_list(range);

Scratch_Block scratch(app);
String_Const_u8 text = push_buffer_range(app, scratch, buffer, range);

Face_Advance_Map advance_map = get_face_advance_map(app, face);
Face_Metrics metrics = get_face_metrics(app, face);
f32 tab_width = (f32)def_get_config_u64(app, vars_save_string_lit("default_tab_width"));
tab_width = clamp_bot(1, tab_width);
LefRig_TopBot_Layout_Vars pos_vars = get_lr_tb_layout_vars(&advance_map, &metrics, tab_width, width);

pos_vars.blank_dim = V2f32(metrics.space_advance, metrics.text_height*0.5f);

if (text.size == 0){
lr_tb_write_blank(&pos_vars, face, arena, &list, range.start);
}
else{
Newline_Layout_Vars newline_vars = get_newline_layout_vars();

b32 all_whitespace = true;
for (u64 i = 0; i < text.size; i += 1){
if (!character_is_whitespace(text.str[i])){
all_whitespace = false;
break;
}
}

if (!all_whitespace){
pos_vars.blank_dim.y = metrics.text_height;
}

u8 *ptr = text.str;
u8 *end_ptr = ptr + text.size;
for (;ptr < end_ptr;){
Character_Consume_Result consume = utf8_consume(ptr, (u64)(end_ptr - ptr));

i64 index = layout_index_from_ptr(ptr, text.str, range.first);
switch (consume.codepoint){
case '\t':
{
newline_layout_consume_default(&newline_vars);
Vec2_f32 dim = pos_vars.blank_dim;
dim.x = lr_tb_advance(&pos_vars, face, '\t');
lr_tb_write_blank_dim(&pos_vars, face, dim, arena, &list, index);
}break;

case ' ':
case '\f':
case '\v':
{
newline_layout_consume_default(&newline_vars);
lr_tb_write_blank(&pos_vars, face, arena, &list, index);
}break;

default:
{
newline_layout_consume_default(&newline_vars);
lr_tb_write(&pos_vars, face, arena, &list, index, consume.codepoint);
}break;

case '\r':
{
newline_layout_consume_CR(&newline_vars, index);
}break;

case '\n':
{
i64 newline_index = newline_layout_consume_LF(&newline_vars, index);
lr_tb_write_blank(&pos_vars, face, arena, &list, newline_index);
lr_tb_next_line(&pos_vars);
}break;

case max_u32:
{
newline_layout_consume_default(&newline_vars);
lr_tb_write_byte(&pos_vars, face, arena, &list, index, *ptr);
}break;
}

ptr += consume.inc;
}

if (newline_layout_consume_finish(&newline_vars)){
i64 index = layout_index_from_ptr(ptr, text.str, range.first);
lr_tb_write_blank(&pos_vars, face, arena, &list, index);
}
}

layout_item_list_finish(&list, -pos_vars.line_to_text_shift);

return(list);
}

function Layout_Item_List
layout_wrap_anywhere(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 range, Face_ID face, f32 width){
Scratch_Block scratch(app);

Layout_Item_List list = get_empty_item_list(range);

String_Const_u8 text = push_buffer_range(app, scratch, buffer, range);

Face_Advance_Map advance_map = get_face_advance_map(app, face);
Face_Metrics metrics = get_face_metrics(app, face);
f32 tab_width = (f32)def_get_config_u64(app, vars_save_string_lit("default_tab_width"));
tab_width = clamp_bot(1, tab_width);
LefRig_TopBot_Layout_Vars pos_vars = get_lr_tb_layout_vars(&advance_map, &metrics, tab_width, width);

if (text.size == 0){
lr_tb_write_blank(&pos_vars, face, arena, &list, range.first);
}
else{
b32 first_of_the_line = true;
Newline_Layout_Vars newline_vars = get_newline_layout_vars();

u8 *ptr = text.str;
u8 *end_ptr = ptr + text.size;
for (;ptr < end_ptr;){
Character_Consume_Result consume = utf8_consume(ptr, (u64)(end_ptr - ptr));
i64 index = layout_index_from_ptr(ptr, text.str, range.first);
switch (consume.codepoint){
default:
{
newline_layout_consume_default(&newline_vars);
f32 advance = lr_tb_advance(&pos_vars, face, consume.codepoint);
if (!first_of_the_line && lr_tb_crosses_width(&pos_vars, advance)){
lr_tb_next_line(&pos_vars);
}
lr_tb_write_with_advance(&pos_vars, face, advance, arena, &list, index, consume.codepoint);
first_of_the_line = false;
}break;

case '\r':
{
newline_layout_consume_CR(&newline_vars, index);
}break;

case '\n':
{
i64 newline_index = newline_layout_consume_LF(&newline_vars, index);
lr_tb_write_blank(&pos_vars, face, arena, &list, newline_index);
lr_tb_next_line(&pos_vars);
first_of_the_line = true;
}break;

case max_u32:
{
newline_layout_consume_default(&newline_vars);
f32 advance = lr_tb_advance_byte(&pos_vars);
if (!first_of_the_line && lr_tb_crosses_width(&pos_vars, advance)){
lr_tb_next_line(&pos_vars);
}
lr_tb_write_byte_with_advance(&pos_vars, face, advance, arena, &list, index, *ptr);
first_of_the_line = false;
}break;
}
ptr += consume.inc;
}

if (newline_layout_consume_finish(&newline_vars)){
i64 index = layout_index_from_ptr(ptr, text.str, range.first);
lr_tb_write_blank(&pos_vars, face, arena, &list, index);
}
}

layout_item_list_finish(&list, -pos_vars.line_to_text_shift);

return(list);
}

function Layout_Item_List
layout_unwrapped__inner(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 range, Face_ID face, f32 width, Layout_Virtual_Indent virt_indent){
Layout_Item_List list = get_empty_item_list(range);

Scratch_Block scratch(app);
String_Const_u8 text = push_buffer_range(app, scratch, buffer, range);

Face_Advance_Map advance_map = get_face_advance_map(app, face);
Face_Metrics metrics = get_face_metrics(app, face);
f32 tab_width = (f32)def_get_config_u64(app, vars_save_string_lit("default_tab_width"));
tab_width = clamp_bot(1, tab_width);
LefRig_TopBot_Layout_Vars pos_vars = get_lr_tb_layout_vars(&advance_map, &metrics, tab_width, width);

if (text.size == 0){
lr_tb_write_blank(&pos_vars, face, arena, &list, range.first);
}
else{
b32 skipping_leading_whitespace = (virt_indent == LayoutVirtualIndent_On);
Newline_Layout_Vars newline_vars = get_newline_layout_vars();

u8 *ptr = text.str;
u8 *end_ptr = ptr + text.size;
for (;ptr < end_ptr;){
Character_Consume_Result consume = utf8_consume(ptr, (u64)(end_ptr - ptr));

i64 index = layout_index_from_ptr(ptr, text.str, range.first);
switch (consume.codepoint){
case '\t':
case ' ':
{
newline_layout_consume_default(&newline_vars);
f32 advance = lr_tb_advance(&pos_vars, face, consume.codepoint);
if (!skipping_leading_whitespace){
lr_tb_write_with_advance(&pos_vars, face, advance, arena, &list, index, consume.codepoint);
}
else{
lr_tb_advance_x_without_item(&pos_vars, advance);
}
}break;

default:
{
newline_layout_consume_default(&newline_vars);
lr_tb_write(&pos_vars, face, arena, &list, index, consume.codepoint);
}break;

case '\r':
{
newline_layout_consume_CR(&newline_vars, index);
}break;

case '\n':
{
i64 newline_index = newline_layout_consume_LF(&newline_vars, index);
lr_tb_write_blank(&pos_vars, face, arena, &list, newline_index);
lr_tb_next_line(&pos_vars);
}break;

case max_u32:
{
newline_layout_consume_default(&newline_vars);
lr_tb_write_byte(&pos_vars, face, arena, &list, index, *ptr);
}break;
}

ptr += consume.inc;
}

if (newline_layout_consume_finish(&newline_vars)){
i64 index = layout_index_from_ptr(ptr, text.str, range.first);
lr_tb_write_blank(&pos_vars, face, arena, &list, index);
}
}

layout_item_list_finish(&list, -pos_vars.line_to_text_shift);

return(list);
}

function Layout_Item_List
layout_wrap_whitespace__inner(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 range, Face_ID face, f32 width, Layout_Virtual_Indent virt_indent){
Scratch_Block scratch(app);

Layout_Item_List list = get_empty_item_list(range);

String_Const_u8 text = push_buffer_range(app, scratch, buffer, range);

Face_Advance_Map advance_map = get_face_advance_map(app, face);
Face_Metrics metrics = get_face_metrics(app, face);
f32 tab_width = (f32)def_get_config_u64(app, vars_save_string_lit("default_tab_width"));
tab_width = clamp_bot(1, tab_width);
LefRig_TopBot_Layout_Vars pos_vars = get_lr_tb_layout_vars(&advance_map, &metrics, tab_width, width);

if (text.size == 0){
lr_tb_write_blank(&pos_vars, face, arena, &list, range.first);
}
else{
b32 skipping_leading_whitespace = false;
b32 first_of_the_line = true;
Newline_Layout_Vars newline_vars = get_newline_layout_vars();

u8 *ptr = text.str;
u8 *end_ptr = ptr + text.size;
u8 *word_ptr = ptr;

if (character_is_whitespace(*ptr)){
skipping_leading_whitespace = (virt_indent == LayoutVirtualIndent_On);
goto consuming_whitespace;
}

consuming_non_whitespace:
for (;ptr <= end_ptr; ptr += 1){
if (ptr == end_ptr || character_is_whitespace(*ptr)){
break;
}
}

{
newline_layout_consume_default(&newline_vars);

String_Const_u8 word = SCu8(word_ptr, ptr);
u8 *word_end = ptr;

if (!first_of_the_line){
f32 total_advance = 0.f;
ptr = word.str;
for (;ptr < word_end;){
Character_Consume_Result consume =
utf8_consume(ptr, (u64)(word_end - ptr));
if (consume.codepoint != max_u32){
total_advance += lr_tb_advance(&pos_vars, face, consume.codepoint);
}
else{
total_advance += lr_tb_advance_byte(&pos_vars);
}
ptr += consume.inc;
}

if (lr_tb_crosses_width(&pos_vars, total_advance)){
lr_tb_next_line(&pos_vars);
}
}

ptr = word.str;

for (;ptr < word_end;){
Character_Consume_Result consume =
utf8_consume(ptr, (u64)(word_end - ptr));
i64 index = layout_index_from_ptr(ptr, text.str, range.first);

if (consume.codepoint != max_u32){
lr_tb_write(&pos_vars, face, arena, &list, index, consume.codepoint);
}
else{
lr_tb_write_byte(&pos_vars, face, arena, &list, index, *ptr);
}

ptr += consume.inc;
}

first_of_the_line = false;
}

consuming_whitespace:
for (; ptr < end_ptr; ptr += 1){
if (!character_is_whitespace(*ptr)){
word_ptr = ptr;
goto consuming_non_whitespace;
}

i64 index = layout_index_from_ptr(ptr, text.str, range.first);
switch (*ptr){
case '\t':
case ' ':
{
newline_layout_consume_default(&newline_vars);
f32 advance = lr_tb_advance(&pos_vars, face, *ptr);
if (!skipping_leading_whitespace){
if (!first_of_the_line && lr_tb_crosses_width(&pos_vars, advance)){
lr_tb_next_line(&pos_vars);
}
lr_tb_write_with_advance(&pos_vars, face, advance, arena, &list, index, *ptr);
first_of_the_line = false;
}
else{
lr_tb_advance_x_without_item(&pos_vars, advance);
}
}break;

default:
{
newline_layout_consume_default(&newline_vars);
f32 advance = lr_tb_advance(&pos_vars, face, *ptr);
if (!first_of_the_line && lr_tb_crosses_width(&pos_vars, advance)){
lr_tb_next_line(&pos_vars);
}
lr_tb_write_with_advance(&pos_vars, face, advance, arena, &list, index, *ptr);
first_of_the_line = false;
}break;

case '\r':
{
newline_layout_consume_CR(&newline_vars, index);
}break;

case '\n':
{
u64 newline_index = newline_layout_consume_LF(&newline_vars, index);
lr_tb_write_blank(&pos_vars, face, arena, &list, newline_index);
lr_tb_next_line(&pos_vars);
first_of_the_line = true;
}break;
}
}

if (newline_layout_consume_finish(&newline_vars)){
i64 index = layout_index_from_ptr(ptr, text.str, range.first);
lr_tb_write_blank(&pos_vars, face, arena, &list, index);
}
}

layout_item_list_finish(&list, -pos_vars.line_to_text_shift);

return(list);
}

function Layout_Item_List
layout_unwrapped(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 range, Face_ID face, f32 width){
return(layout_unwrapped__inner(app, arena, buffer, range, face, width, LayoutVirtualIndent_Off));
}

function Layout_Item_List
layout_wrap_whitespace(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 range, Face_ID face, f32 width){
return(layout_wrap_whitespace__inner(app, arena, buffer, range, face, width, LayoutVirtualIndent_Off));
}

function Layout_Item_List
layout_basic(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 range, Face_ID face, f32 width, Layout_Wrap_Kind kind){
Layout_Item_List result = {};
switch (kind){
case Layout_Unwrapped:
{
result = layout_unwrapped(app, arena, buffer, range, face, width);
}break;
case Layout_Wrapped:
{
result = layout_wrap_whitespace(app, arena, buffer, range, face, width);
}break;
}
return(result);
}

function Layout_Item_List
layout_generic(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 range, Face_ID face, f32 width){
Managed_Scope scope = buffer_get_managed_scope(app, buffer);
b32 *wrap_lines_ptr = scope_attachment(app, scope, buffer_wrap_lines, b32);
b32 wrap_lines = (wrap_lines_ptr != 0 && *wrap_lines_ptr);
return(layout_basic(app, arena, buffer, range, face, width, wrap_lines?Layout_Wrapped:Layout_Unwrapped));
}

function Layout_Item_List
layout_virt_indent_literal_unwrapped(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 range, Face_ID face, f32 width){
return(layout_unwrapped__inner(app, arena, buffer, range, face, width, LayoutVirtualIndent_On));
}

function Layout_Item_List
layout_virt_indent_literal_wrapped(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 range, Face_ID face, f32 width){
return(layout_wrap_whitespace__inner(app, arena, buffer, range, face, width, LayoutVirtualIndent_On));
}

function Layout_Item_List
layout_virt_indent_literal(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 range, Face_ID face, f32 width, Layout_Wrap_Kind kind){
Layout_Item_List result = {};
switch (kind){
case Layout_Unwrapped:
{
result = layout_virt_indent_literal_unwrapped(app, arena, buffer, range, face, width);
}break;
case Layout_Wrapped:
{
result = layout_virt_indent_literal_wrapped(app, arena, buffer, range, face, width);
}break;
}
return(result);
}

function Layout_Item_List
layout_virt_indent_literal_generic(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 range, Face_ID face, f32 width){
Managed_Scope scope = buffer_get_managed_scope(app, buffer);
b32 *wrap_lines_ptr = scope_attachment(app, scope, buffer_wrap_lines, b32);
b32 wrap_lines = (wrap_lines_ptr != 0 && *wrap_lines_ptr);
return(layout_virt_indent_literal(app, arena, buffer, range, face, width, wrap_lines?Layout_Wrapped:Layout_Unwrapped));
}

// BOTTOM

