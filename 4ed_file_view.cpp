/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 19.08.2015
 *
 * File editing view for 4coder
 *
 */

// TOP

#include "buffer/4coder_shared.cpp"

#if BUFFER_EXPERIMENT_SCALPEL == 0
#include "buffer/4coder_golden_array.cpp"
#define Buffer_Type Buffer
#elif BUFFER_EXPERIMENT_SCALPEL == 1
#include "buffer/4coder_gap_buffer.cpp"
#define Buffer_Type Gap_Buffer
#else
#include "buffer/4coder_multi_gap_buffer.cpp"
#define Buffer_Type Multi_Gap_Buffer
#endif

#include "buffer/4coder_buffer_abstract.cpp"

struct Range{
    i32 start, end;
};

enum Edit_Type{
    ED_NORMAL,
    ED_REVERSE_NORMAL,
    ED_UNDO,
    ED_REDO,
};

struct Edit_Step{
    Edit_Type type;
    union{
        struct{
            b32 can_merge;
            Buffer_Edit edit;
            i32 pre_pos;
            i32 post_pos;
            i32 next_block, prev_block;
        };
        struct{
            i32 first_child;
            i32 inverse_first_child;
            i32 inverse_child_count;
            i32 special_type;
        };
    };
    i32 child_count;
};

struct Edit_Stack{
    u8 *strings;
    i32 size, max;
    
    Edit_Step *edits;
    i32 edit_count, edit_max;
};

struct Small_Edit_Stack{
    u8 *strings;
    i32 size, max;
    
    Buffer_Edit *edits;
    i32 edit_count, edit_max;
};

struct Undo_Data{
    Edit_Stack undo;
    Edit_Stack redo;
    Edit_Stack history;
    Small_Edit_Stack children;
    
    i32 history_block_count, history_head_block;
    i32 edit_history_cursor;
    b32 current_block_normal;
};

struct Editing_File{
    Buffer_Type buffer;
    
    Undo_Data undo;
    
    Font *font;
    
    i32 cursor_pos;
    b32 is_dummy;
    
    char source_path_[256];
    char live_name_[256];
    char extension_[16];
    String source_path;
    String live_name;
    String extension;
    
    Cpp_Token_Stack token_stack;
    b32 tokens_complete;
    b32 tokens_exist;
    b32 still_lexing;
    u32 lex_job;
    
    i32 base_map_id;
    i32 dos_write_mode;
    
    u64 last_4ed_write_time;
    u64 last_4ed_edit_time;
    u64 last_sys_write_time;
};

struct File_Table_Entry{
    String name;
    u32 hash;
    i32 index;
};

struct File_Table{
    File_Table_Entry *table;
    i32 count, max;
};

internal u32
get_file_hash(String name){
    u32 x = 5381;
    int i = 0;
    char c;
    while (i < name.size){
        c = name.str[i++];
        x = ((x << 5) + x) + c;
    }
    return x;
}

internal bool32
table_add(File_Table *table, String name, i32 index){
    Assert(table->count * 3 < table->max * 2);
    
    File_Table_Entry entry, e;
    i32 i;
    
    entry.name = name;
    entry.index = index;
    entry.hash = get_file_hash(name);
    i = entry.hash % table->max;
    while ((e = table->table[i]).name.str){
        if (e.hash == entry.hash && match(e.name, entry.name)){
            return 1;
        }
        i = (i + 1) % table->max;
    }
    table->table[i] = entry;
    ++table->count;
    
    return 0;
}

internal bool32
table_find_pos(File_Table *table, String name, i32 *index){
    File_Table_Entry e;
    i32 i;
    u32 hash;

    hash = get_file_hash(name);
    i = hash % table->max;
    while ((e = table->table[i]).name.size){
        if (e.name.str && e.hash == hash && match(e.name, name)){
            *index = i;
            return 1;
        }
        i = (i + 1) % table->max;
    }
    
    return 0;
}

inline bool32
table_find(File_Table *table, String name, i32 *index){
    i32 pos;
    bool32 r = table_find_pos(table, name, &pos);
    if (r) *index = table->table[pos].index;
    return r;
}

inline bool32
table_remove(File_Table *table, String name){
    i32 pos;
    bool32 r = table_find_pos(table, name, &pos);
    if (r){
        table->table[pos].name.str = 0;
        --table->count;
    }
    return r;
}

struct Working_Set{
	Editing_File *files;
	i32 file_index_count, file_max_count;
    
    File_Table table;
    
	String clipboards[64];
	i32 clipboard_size, clipboard_max_size;
	i32 clipboard_current, clipboard_rolling;
};

struct Text_Effect{
    i32 start, end;
    u32 color;
    i32 tick_down, tick_max;
};

struct File_View_Mode{
	bool8 rewrite;
};

struct Incremental_Search{
    String str;
    bool32 reverse;
    i32 pos;
};

enum Action_Type{
    DACT_OPEN,
    DACT_SAVE_AS,
    DACT_SAVE,
    DACT_NEW,
    DACT_SWITCH,
    DACT_TRY_KILL,
    DACT_KILL,
    DACT_CLOSE_MINOR,
    DACT_CLOSE_MAJOR,
    DACT_THEME_OPTIONS
};

struct Delayed_Action{
    Action_Type type;
    String string;
    Panel *panel;
};

struct Delay{
    Delayed_Action acts[8];
    i32 count, max;
};

inline void
delayed_action(Delay *delay, Action_Type type,
               String string, Panel *panel){
    Assert(delay->count < delay->max);
    Delayed_Action action;
    action.type = type;
    action.string = string;
    action.panel = panel;
    delay->acts[delay->count++] = action;
}

// Hot Directory

struct Hot_Directory{
	String string;
	File_List file_list;
};

internal void
hot_directory_init(Hot_Directory *hot_directory, String base){
	hot_directory->string = base;
    hot_directory->string.str[255] = 0;
	i32 dir_size = system_get_working_directory((u8*)hot_directory->string.str,
												hot_directory->string.memory_size);
	if (dir_size <= 0){
		dir_size = system_get_easy_directory((u8*)hot_directory->string.str);
	}
	hot_directory->string.size = dir_size;
	append(&hot_directory->string, "\\");
}

internal void
hot_directory_clean_end(Hot_Directory *hot_directory){
    String *str = &hot_directory->string;
    if (str->size != 0 && str->str[str->size-1] != '\\'){
        str->size = reverse_seek_slash(*str) + 1;
        str->str[str->size] = 0;
    }
}

internal i32
hot_directory_quick_partition(File_Info *infos, i32 start, i32 pivot){
    File_Info *p = infos + pivot;
    File_Info *a = infos + start;
    for (i32 i = start; i < pivot; ++i, ++a){
        i32 comp = 0;
        comp = p->folder - a->folder;
        if (comp == 0) comp = compare(a->filename, p->filename);
        if (comp < 0){
            Swap(*a, infos[start]);
            ++start;
        }
    }
    Swap(*p, infos[start]);
    return start;
}

internal void
hot_directory_quick_sort(File_Info *infos, i32 start, i32 pivot){
    i32 mid = hot_directory_quick_partition(infos, start, pivot);
    if (start < mid-1) hot_directory_quick_sort(infos, start, mid-1);
    if (mid+1 < pivot) hot_directory_quick_sort(infos, mid+1, pivot);
}

inline void
hot_directory_fixup(Hot_Directory *hot_directory, Working_Set *working_set){
    File_List *files = &hot_directory->file_list;
    if (files->count >= 2)
        hot_directory_quick_sort(files->infos, 0, files->count - 1);
}

inline void
hot_directory_set(Hot_Directory *hot_directory, String str, Working_Set *working_set){
    bool32 success = copy_checked(&hot_directory->string, str);
    terminate_with_null(&hot_directory->string);
    if (success){
		system_free_file_list(hot_directory->file_list);
		hot_directory->file_list = system_get_files(str);
    }
    hot_directory_fixup(hot_directory, working_set);
}

inline void
hot_directory_reload(Hot_Directory *hot_directory, Working_Set *working_set){
	if (hot_directory->file_list.block){
		system_free_file_list(hot_directory->file_list);
	}
	hot_directory->file_list = system_get_files(hot_directory->string);
    hot_directory_fixup(hot_directory, working_set);
}

struct Hot_Directory_Match{
	String filename;
	bool32 is_folder;
};

internal bool32
filename_match(String query, Absolutes *absolutes, String filename){
    bool32 result;
    result = (query.size == 0);
    if (!result) result = wildcard_match(absolutes, filename);
    return result;
}

internal Hot_Directory_Match
hot_directory_first_match(Hot_Directory *hot_directory,
                          String str,
						  bool32 include_files,
                          bool32 exact_match){
    Hot_Directory_Match result = {};
    
    Absolutes absolutes;
    if (!exact_match)
        get_absolutes(str, &absolutes, 1, 1);
    
    File_List *files = &hot_directory->file_list;
    File_Info *info, *end;
    end = files->infos + files->count;
    for (info = files->infos; info != end; ++info){
        String filename = info->filename;
        bool32 is_match = 0;
        if (exact_match){
            if (match(filename, str)) is_match = 1;
        }
        else{
            if (filename_match(str, &absolutes, filename)) is_match = 1;
        }
        
        if (is_match){
            result.is_folder = info->folder;
            result.filename = filename;
            break;
        }
    }
    
    return result;
}

struct Single_Line_Input_Step{
	bool32 hit_newline;
	bool32 hit_ctrl_newline;
	bool32 hit_a_character;
    bool32 hit_backspace;
	bool32 hit_esc;
	bool32 made_a_change;
    bool32 did_command;
};

enum Single_Line_Input_Type{
	SINGLE_LINE_STRING,
	SINGLE_LINE_FILE
};

struct Single_Line_Mode{
	Single_Line_Input_Type type;
	String *string;
	Hot_Directory *hot_directory;
    bool32 fast_folder_select;
};

internal Single_Line_Input_Step
app_single_line_input_core(Key_Codes *codes, Working_Set *working_set,
                           Key_Single key, Single_Line_Mode mode){
    Single_Line_Input_Step result = {};
    
    if (key.key.keycode == codes->back){
        result.hit_backspace = 1;
        if (mode.string->size > 0){
            result.made_a_change = 1;
            --mode.string->size;
            switch (mode.type){
            case SINGLE_LINE_STRING:
                mode.string->str[mode.string->size] = 0; break;
            
            case SINGLE_LINE_FILE:
            {
                char end_character = mode.string->str[mode.string->size];
                if (char_is_slash(end_character)){
                    mode.string->size = reverse_seek_slash(*mode.string) + 1;
                    mode.string->str[mode.string->size] = 0;
                    hot_directory_set(mode.hot_directory, *mode.string, working_set);
                }
                else{
                    mode.string->str[mode.string->size] = 0;
                }
            }break;
            }
        }
    }
    
    else if (key.key.character == '\n' || key.key.character == '\t'){
        result.made_a_change = 1;
        if (key.modifiers[CONTROL_KEY_CONTROL] ||
            key.modifiers[CONTROL_KEY_ALT]){
            result.hit_ctrl_newline = 1;
        }
        else{
            result.hit_newline = 1;
            if (mode.fast_folder_select){
                char front_name_space[256];
                String front_name =
                    make_string(front_name_space, 0, ArrayCount(front_name_space));
                get_front_of_directory(&front_name, *mode.string);
                Hot_Directory_Match match;
                match = hot_directory_first_match(mode.hot_directory, front_name, 1, 1);
                if (!match.filename.str){
                    match = hot_directory_first_match(mode.hot_directory, front_name, 1, 0);
                }
                if (match.filename.str){
                    if (match.is_folder){
                        set_last_folder(mode.string, match.filename);
                        hot_directory_set(mode.hot_directory, *mode.string, working_set);
                        result.hit_newline = 0;
                    }
                    else{
                        mode.string->size = reverse_seek_slash(*mode.string) + 1;
                        append(mode.string, match.filename);
                        result.hit_newline = 1;
                    }
                }
            }
        }
    }
    
    else if (key.key.keycode == codes->esc){
        result.hit_esc = 1;
        result.made_a_change = 1;
    }
    
    else if (key.key.character){
        result.hit_a_character = 1;
        if (!key.modifiers[CONTROL_KEY_CONTROL] &&
            !key.modifiers[CONTROL_KEY_ALT]){
            if (mode.string->size+1 < mode.string->memory_size){
                u8 new_character = (u8)key.key.character;
                mode.string->str[mode.string->size] = new_character;
                mode.string->size++;
                mode.string->str[mode.string->size] = 0;
                if (mode.type == SINGLE_LINE_FILE && char_is_slash(new_character)){
                    hot_directory_set(mode.hot_directory, *mode.string, working_set);
                }
                result.made_a_change = 1;
            }
        }
        else{
            result.did_command = 1;
            result.made_a_change = 1;
        }
    }
    
    return result;
}

inline Single_Line_Input_Step
app_single_line_input_step(Key_Codes *codes, Key_Single key, String *string){
	Single_Line_Mode mode = {};
	mode.type = SINGLE_LINE_STRING;
	mode.string = string;
	return app_single_line_input_core(codes, 0, key, mode);
}

inline Single_Line_Input_Step
app_single_file_input_step(Key_Codes *codes, Working_Set *working_set, Key_Single key,
						   String *string, Hot_Directory *hot_directory,
                           bool32 fast_folder_select){
	Single_Line_Mode mode = {};
	mode.type = SINGLE_LINE_FILE;
	mode.string = string;
	mode.hot_directory = hot_directory;
    mode.fast_folder_select = fast_folder_select;
	return app_single_line_input_core(codes, working_set, key, mode);
}

inline Single_Line_Input_Step
app_single_number_input_step(Key_Codes *codes, Key_Single key, String *string){
    Single_Line_Input_Step result = {};
	Single_Line_Mode mode = {};
	mode.type = SINGLE_LINE_STRING;
	mode.string = string;
    
    char c = (char)key.key.character;
    if (c == 0 || c == '\n' || char_is_numeric(c))
        result = app_single_line_input_core(codes, 0, key, mode);
	return result;
}

struct Widget_ID{
    i32 id;
    i32 sub_id0;
    i32 sub_id1;
    i32 sub_id2;
};

inline bool32
widget_match(Widget_ID s1, Widget_ID s2){
    return (s1.id == s2.id && s1.sub_id0 == s2.sub_id0 &&
            s1.sub_id1 == s2.sub_id1 && s1.sub_id2 == s2.sub_id2);
}

struct UI_State{
    Render_Target *target;
    Style *style;
    Font *font;
    Mouse_Summary *mouse;
    Key_Summary *keys;
    Key_Codes *codes;
    Working_Set *working_set;
    
    Widget_ID selected, hover, hot;
    bool32 activate_me;
    bool32 redraw;
    bool32 input_stage;
    i32 sub_id1_change;
    
    real32 height, view_y;
};

inline Widget_ID
make_id(UI_State *state, i32 id){
    Widget_ID r = state->selected;
    r.id = id;
    return r;
}

inline Widget_ID
make_sub0(UI_State *state, i32 id){
    Widget_ID r = state->selected;
    r.sub_id0 = id;
    return r;
}

inline Widget_ID
make_sub1(UI_State *state, i32 id){
    Widget_ID r = state->selected;
    r.sub_id1 = id;
    return r;
}

inline Widget_ID
make_sub2(UI_State *state, i32 id){
    Widget_ID r = state->selected;
    r.sub_id2 = id;
    return r;
}

inline bool32
is_selected(UI_State *state, Widget_ID id){
    return widget_match(state->selected, id);
}

inline bool32
is_hot(UI_State *state, Widget_ID id){
    return widget_match(state->hot, id);
}

inline bool32
is_hover(UI_State *state, Widget_ID id){
    return widget_match(state->hover, id);
}

struct UI_Layout{
    i32 row_count;
    i32 row_item_width;
    i32 row_max_item_height;
    
    i32_Rect rect;
    i32 x, y;    
};

struct UI_Layout_Restore{
    UI_Layout layout;
    UI_Layout *dest;
};

inline void
begin_layout(UI_Layout *layout, i32_Rect rect){
    layout->rect = rect;
    layout->x = rect.x0;
    layout->y = rect.y0;
    layout->row_count = 0;
    layout->row_max_item_height = 0;
}

inline void
begin_row(UI_Layout *layout, i32 count){
    layout->row_count = count;
    layout->row_item_width = (layout->rect.x1 - layout->x) / count; 
}

inline i32_Rect
layout_rect(UI_Layout *layout, i32 height){
    i32_Rect rect;
    rect.x0 = layout->x;
    rect.y0 = layout->y;
    rect.x1 = rect.x0;
    rect.y1 = rect.y0 + height;
    if (layout->row_count > 0){
        --layout->row_count;
        rect.x1 = rect.x0 + layout->row_item_width;
        layout->x += layout->row_item_width;
        layout->row_max_item_height = Max(height, layout->row_max_item_height);
    }
    if (layout->row_count == 0){
        rect.x1 = layout->rect.x1;
        layout->row_max_item_height = Max(height, layout->row_max_item_height);
        layout->y += layout->row_max_item_height;
        layout->x = layout->rect.x0;
        layout->row_max_item_height = 0;
    }
    return rect;
}

inline UI_Layout_Restore
begin_sub_layout(UI_Layout *layout, i32_Rect area){
    UI_Layout_Restore restore;
    restore.layout = *layout;
    restore.dest = layout;
    begin_layout(layout, area);
    return restore;
}

inline void
end_sub_layout(UI_Layout_Restore restore){
    *restore.dest = restore.layout;
}

struct UI_Style{
    u32 dark, dim, bright;
    u32 base, pop1, pop2;
};

internal UI_Style
get_ui_style(Style *style){
    UI_Style ui_style;
    ui_style.dark = style->main.back_color;
    ui_style.dim = style->main.margin_color;
    ui_style.bright = style->main.margin_active_color;
    ui_style.base = style->main.default_color;
    ui_style.pop1 = style->main.file_info_style.pop1_color;
    ui_style.pop2 = style->main.file_info_style.pop2_color;
    return ui_style;
}

internal UI_Style
get_ui_style_upper(Style *style){
    UI_Style ui_style;
    ui_style.dark = style->main.margin_color;
    ui_style.dim = style->main.margin_hover_color;
    ui_style.bright = style->main.margin_active_color;
    ui_style.base = style->main.default_color;
    ui_style.pop1 = style->main.file_info_style.pop1_color;
    ui_style.pop2 = style->main.file_info_style.pop2_color;
    return ui_style;
}

inline void
get_colors(UI_State *state, u32 *back, u32 *fore, Widget_ID wid, UI_Style style){
    bool32 hover = is_hover(state, wid);
    bool32 hot = is_hot(state, wid);
    i32 level = hot + hover;
    switch (level){
    case 2:
        *back = style.bright;
        *fore = style.dark;
        break;
    case 1:
        *back = style.dim;
        *fore = style.bright;
        break;
    case 0:
        *back = style.dark;
        *fore = style.bright;
        break;
    }
}

inline void
get_pop_color(UI_State *state, u32 *pop, Widget_ID wid, UI_Style style){
    bool32 hover = is_hover(state, wid);
    bool32 hot = is_hot(state, wid);
    i32 level = hot + hover;
    switch (level){
    case 2:
        *pop = style.pop1;
        break;
    case 1:
        *pop = style.pop1;
        break;
    case 0:
        *pop = style.pop1;
        break;
    }
}

internal UI_State
ui_state_init(UI_State *state_in, Render_Target *target, Input_Summary *user_input,
              Style *style, Working_Set *working_set, bool32 input_stage){
    UI_State state = {};
    state.target = target;
    state.style = style;
    state.font = style->font;
    state.working_set = working_set;
    if (user_input){
        state.mouse = &user_input->mouse;
        state.keys = &user_input->keys;
        state.codes = user_input->codes;
    }
    state.selected = state_in->selected;
    state.hot = state_in->hot;
    if (input_stage) state.hover = {};
    else state.hover = state_in->hover;
    state.redraw = 0;
    state.activate_me = 0;
    state.input_stage = input_stage;
    state.height = state_in->height;
    state.view_y = state_in->view_y;
    return state;
}

inline bool32
ui_state_match(UI_State a, UI_State b){
    return (widget_match(a.selected, b.selected) &&
            widget_match(a.hot, b.hot) &&
            widget_match(a.hover, b.hover));
}

internal bool32
ui_finish_frame(UI_State *persist_state, UI_State *state, UI_Layout *layout, i32_Rect rect,
                bool32 do_wheel, bool32 *did_activation){
    bool32 result = 0;
    real32 h = layout->y + persist_state->view_y - rect.y0;
    real32 max_y = h - (rect.y1 - rect.y0);
    
    persist_state->height = h;
    persist_state->view_y = state->view_y;
    
    if (state->input_stage){
        Mouse_Summary *mouse = state->mouse;
        if (mouse->wheel_used && do_wheel){
            persist_state->view_y += mouse->wheel_amount*state->font->height;
            result = 1;
        }
        if (mouse->release_l && widget_match(state->hot, state->hover)){
            if (did_activation) *did_activation = 1;
            if (state->activate_me){
                state->selected = state->hot;
            }
        }
        if (!mouse->l && !mouse->r){
            state->hot = {};
        }
        
        if (!ui_state_match(*persist_state, *state) || state->redraw){
            result = 1;
        }
        
        *persist_state = *state;
    }
    
    if (persist_state->view_y >= max_y) persist_state->view_y = max_y;
    if (persist_state->view_y < 0) persist_state->view_y = 0;

    return result;
}

internal bool32
ui_do_button_input(UI_State *state, i32_Rect rect, Widget_ID id, bool32 activate, bool32 *right = 0){
    bool32 result = 0;
    Mouse_Summary *mouse = state->mouse;
    bool32 hover = hit_check(mouse->mx, mouse->my, rect);
    if (hover){
        state->hover = id;
        if (activate) state->activate_me = 1;
        if (mouse->press_l || (mouse->press_r && right)) state->hot = id;
        if (mouse->l && mouse->r) state->hot = {};
    }
    bool32 is_match = is_hot(state, id);
    if (mouse->release_l && is_match){
        if (hover) result = 1;
        state->redraw = 1;
    }
    if (right && mouse->release_r && is_match){
        if (hover) *right = 1;
        state->redraw = 1;
    }
    return result;
}

internal bool32
ui_do_subdivided_button_input(UI_State *state, i32_Rect rect, i32 parts, Widget_ID id, bool32 activate, i32 *indx_out, bool32 *right = 0){
    bool32 result = 0;
    real32 x0, x1;
    i32_Rect sub_rect;
    Widget_ID sub_widg = id;
    real32 sub_width = (rect.x1 - rect.x0) / (real32)parts;
    sub_rect.y0 = rect.y0;
    sub_rect.y1 = rect.y1;
    x1 = (real32)rect.x0;
    
    for (i32 i = 0; i < parts; ++i){
        x0 = x1;
        x1 = x1 + sub_width;
        sub_rect.x0 = TRUNC32(x0);
        sub_rect.x1 = TRUNC32(x1);
        sub_widg.sub_id2 = i;
        if (ui_do_button_input(state, sub_rect, sub_widg, activate, right)){
            *indx_out = i;
            break;
        }
    }
    
    return result;
}

internal real32
ui_do_vscroll_input(UI_State *state, i32_Rect top, i32_Rect bottom, i32_Rect slider,
                    Widget_ID id, real32 val, real32 step_amount,
                    real32 smin, real32 smax, real32 vmin, real32 vmax){
    Mouse_Summary *mouse = state->mouse;
    i32 mx = mouse->mx;
    i32 my = mouse->my;
    if (hit_check(mx, my, top)){
        state->hover = id;
        state->hover.sub_id2 = 1;
    }
    if (hit_check(mx, my, bottom)){
        state->hover = id;
        state->hover.sub_id2 = 2;
    }
    if (hit_check(mx, my, slider)){
        state->hover = id;
        state->hover.sub_id2 = 3;
    }
    if (mouse->press_l) state->hot = state->hover;
    if (id.id == state->hot.id){
        if (mouse->release_l){
            Widget_ID wid1, wid2;
            wid1 = wid2 = id;
            wid1.sub_id2 = 1;
            wid2.sub_id2 = 2;
            if (state->hot.sub_id2 == 1 && is_hover(state, wid1)) val -= step_amount;
            if (state->hot.sub_id2 == 2 && is_hover(state, wid2)) val += step_amount;
            state->redraw = 1;
        }
        if (state->hot.sub_id2 == 3){
            real32 S, L;
            S = (real32)mouse->my - (slider.y1 - slider.y0) / 2;
            if (S < smin) S = smin;
            if (S > smax) S = smax;
            L = unlerp(smin, S, smax);
            val = lerp(vmin, L, vmax);
            state->redraw = 1;
        }
    }
    return val;
}

internal bool32
ui_do_text_field_input(UI_State *state, String *str){
    bool32 result = 0;
    Key_Summary *keys = state->keys;
    for (i32 key_i = 0; key_i < keys->count; ++key_i){
        Key_Single key = get_single_key(keys, key_i);
        char c = (char)key.key.character;
        if (char_is_basic(c) && str->size < str->memory_size-1){
            str->str[str->size++] = c;
            str->str[str->size] = 0;
        }
        else if (c == '\n'){
            result = 1;
        }
        else if (key.key.keycode == state->codes->back && str->size > 0){
            str->str[--str->size] = 0;
        }
    }
    return result;
}

internal bool32
ui_do_file_field_input(UI_State *state, Hot_Directory *hot_dir){
    bool32 result = 0;
    Key_Summary *keys = state->keys;
    for (i32 key_i = 0; key_i < keys->count; ++key_i){
        Key_Single key = get_single_key(keys, key_i);
        String *str = &hot_dir->string;
        terminate_with_null(str);
        Single_Line_Input_Step step =
            app_single_file_input_step(state->codes, state->working_set, key, str, hot_dir, 1);
        if (step.hit_newline || step.hit_ctrl_newline) result = 1;
    }
    return result;
}

internal bool32
ui_do_line_field_input(UI_State *state, String *string){
    bool32 result = 0;
    Key_Summary *keys = state->keys;
    for (i32 key_i = 0; key_i < keys->count; ++key_i){
        Key_Single key = get_single_key(keys, key_i);
        terminate_with_null(string);
        Single_Line_Input_Step step =
            app_single_line_input_step(state->codes, key, string);
        if (step.hit_newline || step.hit_ctrl_newline) result = 1;
    }
    return result;
}

internal bool32
ui_do_slider_input(UI_State *state, i32_Rect rect, Widget_ID wid,
                   real32 min, real32 max, real32 *v){
    bool32 result = 0;
    ui_do_button_input(state, rect, wid, 0);
    Mouse_Summary *mouse = state->mouse;
    if (is_hot(state, wid)){
        result = 1;
        *v = unlerp(min, (real32)mouse->mx, max);
        state->redraw = 1;
    }
    return result;
}

internal bool32
do_text_field(Widget_ID wid, UI_State *state, UI_Layout *layout,
              String prompt, String dest){
    bool32 result = 0;
    Font *font = state->font;
    i32 character_h = font->height;

    i32_Rect rect = layout_rect(layout, character_h);
    
    if (state->input_stage){
        ui_do_button_input(state, rect, wid, 1);
        if (is_selected(state, wid)){
            if (ui_do_text_field_input(state, &dest)){
                result = 1;
            }
        }
    }
    else{
        Render_Target *target = state->target;
        UI_Style ui_style = get_ui_style_upper(state->style);
        u32 back, fore, prompt_pop;
        get_colors(state, &back, &fore, wid, ui_style);
        get_pop_color(state, &prompt_pop, wid, ui_style);
        
        draw_rectangle(target, rect, back);
        i32 x = rect.x0;
        x = draw_string(target, font, prompt, x, rect.y0, prompt_pop);
        draw_string(target, font, dest, x, rect.y0, ui_style.base);
    }

    return result;
}

enum File_View_Widget_Type{
    FWIDG_NONE,
    FWIDG_SEARCH,
    FWIDG_GOTO_LINE,
    FWIDG_TIMELINES,
    // never below this
    FWIDG_TYPE_COUNT
};

struct File_View_Widget{
    UI_State state;
    File_View_Widget_Type type;
    i32 height;
    struct{
        bool32 undo_line;
        bool32 history_line;
    } timeline;
};

struct File_View{
    View view_base;
    
    Delay *delay;
    Editing_Layout *layout;
    
    Editing_File *file;
    Style *style;
    
    i32 font_advance;
    i32 font_height;
    
    Full_Cursor cursor;
    i32 mark;
    real32 scroll_y, target_y, vel_y;
    real32 scroll_x, target_x, vel_x;
    real32 preferred_x;
    Full_Cursor scroll_y_cursor;
    union{
        Incremental_Search isearch;
        struct{
            String str;
        } gotoline;
    };
    
    Full_Cursor temp_highlight;
    i32 temp_highlight_end_pos;
    bool32 show_temp_highlight;
    
    File_View_Mode mode, next_mode;
    File_View_Widget widget;
    real32 rewind_amount, rewind_speed;
    i32 rewind_max, scrub_max;
    bool32 unwrapped_lines;
    bool32 show_whitespace;
    
    i32 line_count, line_max;
    real32 *line_wrap_y;
    
    Text_Effect paste_effect;
};

inline File_View*
view_to_file_view(View *view){
    File_View* result = 0;
    if (view && view->type == VIEW_TYPE_FILE){
        result = (File_View*)view;
    }
    return result;
}

internal Range
get_range(i32 a, i32 b){
	Range result = {};
	if (a < b){
		result.start = a;
		result.end = b;
	}
	else{
		result.start = b;
		result.end = a;
	}
	return result;
}

inline bool32
starts_new_line(u8 character){
	return (character == '\n');
}

inline void
file_init_strings(Editing_File *file){
    file->source_path = make_fixed_width_string(file->source_path_);
    file->live_name = make_fixed_width_string(file->live_name_);
    file->extension = make_fixed_width_string(file->extension_);
}

inline void
file_set_name(Editing_File *file, u8 *filename){
    String f, ext;
    f = make_string_slowly((char*)filename);
    copy_checked(&file->source_path, f);
    file->live_name = make_fixed_width_string(file->live_name_);
    get_front_of_directory(&file->live_name, f);
    ext = file_extension(f);
    copy(&file->extension, ext);
}

inline void
file_synchronize_times(Editing_File *file, u8 *filename){
    Time_Stamp stamp = system_file_time_stamp(filename);
    if (stamp.success){
        file->last_4ed_write_time = stamp.time;
        file->last_4ed_edit_time = stamp.time;
        file->last_sys_write_time = stamp.time;
    }
}

internal bool32
file_save(Partition *part, Editing_File *file, u8 *filename){
	bool32 result = 0;
#if BUFFER_EXPERIMENT_SCALPEL <= 1
    Temp_Memory temp = begin_temp_memory(part);
    i32 max = partition_remaining(part);
    if (file->dos_write_mode){
        char *data = push_array(part, char, max);
        i32 size = buffer_convert_out(&file->buffer, data, max);
        result = system_save_file(filename, data, size);
    }
    else{
        char *data = push_array(part, char, max);
        i32 size = buffer_size(&file->buffer);
        Assert(size <= max);
        buffer_stringify(&file->buffer, 0, size, data);
        result = system_save_file(filename, data, size);
    }
    end_temp_memory(temp);
    file_synchronize_times(file, filename);
#endif
    return result;
}

inline bool32
file_save_and_set_names(Partition *part, Editing_File *file, u8 *filename){
	bool32 result = 0;
	if (file_save(part, file, filename)){
		result = 1;
        file_set_name(file, filename);
	}
	return result;
}

inline i32
file_count_newlines(Editing_File *file, i32 start, i32 end){
#if BUFFER_EXPERIMENT_SCALPEL <= 1
    i32 count = buffer_count_newlines(&file->buffer, start, end);
#else
    i32 count = 0;
#endif
    return count;
}

enum File_Bubble_Type{
    BUBBLE_BUFFER = 1,
    BUBBLE_STARTS,
    BUBBLE_WIDTHS,
    BUBBLE_WRAPS,
    BUBBLE_TOKENS,
    BUBBLE_UNDO_STRING,
    BUBBLE_UNDO,
    BUBBLE_UNDO_CHILDREN,
    //
    FILE_BUBBLE_TYPE_END,
};

#define GROW_FAILED 0
#define GROW_NOT_NEEDED 1
#define GROW_SUCCESS 2

internal i32
file_grow_starts_as_needed(General_Memory *general, Editing_File *file, i32 additional_lines){
    bool32 result = GROW_NOT_NEEDED;
#if BUFFER_EXPERIMENT_SCALPEL <= 1
    i32 max = file->buffer.line_max;
    i32 count = file->buffer.line_count;
    i32 target_lines = count + additional_lines;
    if (target_lines > max){
        max <<= 1;
        i32 *new_lines = (i32*)
            general_memory_reallocate(general, file->buffer.line_starts,
                                      sizeof(i32)*count, sizeof(i32)*max, BUBBLE_STARTS);
        if (new_lines){
            file->buffer.line_starts = new_lines;
            file->buffer.line_max = max;
            result = GROW_SUCCESS;
        }
        else{
            result = GROW_FAILED;
        }
    }
#endif
    return result;
}

internal void
file_measure_starts(General_Memory *general, Editing_File *file){
#if BUFFER_EXPERIMENT_SCALPEL <= 1
    ProfileMomentFunction();
    if (!file->buffer.line_starts){
        i32 max = file->buffer.line_max = Kbytes(1);
        file->buffer.line_starts = (i32*)general_memory_allocate(general, max*sizeof(i32), BUBBLE_STARTS);
        TentativeAssert(file->buffer.line_starts);
        // TODO(allen): when unable to allocate?
    }
    
    Buffer_Measure_Starts state = {};
    while (buffer_measure_starts(&state, &file->buffer)){
        i32 max = file->buffer.line_max;
        i32 count = state.count;
        i32 target_lines = count + 1;
        
        max = (target_lines << 1);
        i32 *new_lines = (i32*)
            general_memory_reallocate(general, file->buffer.line_starts,
                                      sizeof(i32)*count, sizeof(i32)*max, BUBBLE_STARTS);
        
        // TODO(allen): when unable to grow?
        TentativeAssert(new_lines);
        file->buffer.line_starts = new_lines;
        file->buffer.line_max = max;
    }
    file->buffer.line_count = state.count;
#endif
}

internal void
file_remeasure_starts(General_Memory *general, Editing_File *file,
                      i32 line_start, i32 line_end, i32 line_shift,
                      i32 character_shift){
#if BUFFER_EXPERIMENT_SCALPEL <= 1
    ProfileMomentFunction();
    
    Assert(file->buffer.line_starts);
    file_grow_starts_as_needed(general, file, line_shift);
    
    buffer_remeasure_starts(&file->buffer, line_start, line_end, line_shift, character_shift);
#endif
}

struct Opaque_Font_Advance{
    void *data;
    int stride;
};

inline Opaque_Font_Advance
get_opaque_font_advance(Font *font){
    Opaque_Font_Advance result;
    result.data = (char*)font->chardata + OffsetOfPtr(font->chardata, xadvance);
    result.stride = sizeof(*font->chardata);
    return result;
}

internal void
file_grow_widths_as_needed(General_Memory *general, Editing_File *file){
#if BUFFER_EXPERIMENT_SCALPEL <= 1
    i32 line_count = file->buffer.line_count;
    if (line_count > file->buffer.widths_max){
        i32 new_max = LargeRoundUp(line_count, Kbytes(1));
        if (file->buffer.line_widths){
            file->buffer.line_widths = (real32*)
                general_memory_reallocate_nocopy(general, file->buffer.line_widths, sizeof(real32)*new_max, BUBBLE_WIDTHS);
        }
        else{
            file->buffer.line_widths = (real32*)
                general_memory_allocate(general, sizeof(real32)*new_max, BUBBLE_WIDTHS);
        }
        file->buffer.widths_max = new_max;
    }
#endif
}

internal void
file_measure_widths(General_Memory *general, Editing_File *file, Font *font){
#if BUFFER_EXPERIMENT_SCALPEL <= 1
    ProfileMomentFunction();
    
    file_grow_widths_as_needed(general, file);
    Opaque_Font_Advance opad = get_opaque_font_advance(font);
    buffer_measure_widths(&file->buffer, opad.data, opad.stride);
#endif
}

internal void
file_remeasure_widths(General_Memory *general, Editing_File *file, Font *font,
                      i32 line_start, i32 line_end, i32 line_shift){
#if BUFFER_EXPERIMENT_SCALPEL <= 1
    ProfileMomentFunction();
    
    file_grow_widths_as_needed(general, file);
    Opaque_Font_Advance opad = get_opaque_font_advance(font);
    buffer_remeasure_widths(&file->buffer, opad.data, opad.stride,
                            line_start, line_end, line_shift);
#endif
}

inline i32
view_wrapped_line_span(real32 line_width, real32 max_width){
    i32 line_count = CEIL32(line_width / max_width);
    if (line_count == 0) line_count = 1;
    return line_count;
}

internal i32
view_compute_lowest_line(File_View *view){
    i32 lowest_line = 0;
#if BUFFER_EXPERIMENT_SCALPEL <= 1
    i32 last_line = view->line_count - 1;
    if (last_line > 0){
        if (view->unwrapped_lines){
            lowest_line = last_line;
        }
        else{
            real32 wrap_y = view->line_wrap_y[last_line];
            Editing_File *file = view->file;
            Assert(!file->is_dummy);
            Style *style = view->style;
            Font *font = style->font;
            lowest_line = FLOOR32(wrap_y / font->height);
            
            real32 width = file->buffer.line_widths[last_line];
            real32 max_width = view_compute_width(view);
            i32 line_span = view_wrapped_line_span(width, max_width);
            lowest_line += line_span - 1;
        }
    }
#endif
    return lowest_line;
}

internal void
view_measure_wraps(General_Memory *general, File_View *view){
#if BUFFER_EXPERIMENT_SCALPEL <= 1
    ProfileMomentFunction();
    Editing_File *file = view->file;
    i32 line_count = file->buffer.line_count;
    
    if (view->line_max < line_count){
        i32 max = view->line_max = LargeRoundUp(line_count, Kbytes(1));
        if (view->line_wrap_y){
            view->line_wrap_y = (real32*)
                general_memory_reallocate_nocopy(general, view->line_wrap_y, sizeof(real32)*max, BUBBLE_WRAPS);
        }
        else{
            view->line_wrap_y = (real32*)
                general_memory_allocate(general, sizeof(real32)*max, BUBBLE_WRAPS);
        }
    }

    Font *font = view->style->font;
    real32 line_height = (real32)font->height;
    real32 max_width = view_compute_width(view);
    buffer_measure_wrap_y(&file->buffer, view->line_wrap_y, line_height, max_width);
    
    view->line_count = line_count;
#endif
}

internal void*
alloc_for_buffer(void *context, int *size){
    *size = LargeRoundUp(*size, Kbytes(4));
    void *data = general_memory_allocate((General_Memory*)context, *size, BUBBLE_BUFFER);
    return data;
}

internal void
file_create_from_string(General_Memory *general, Editing_File *file, u8 *filename, Font *font, String val){
    *file = {};
#if BUFFER_EXPERIMENT_SCALPEL <= 2
    
    Buffer_Init_Type init = buffer_begin_init(&file->buffer, val.str, val.size);
    for (; buffer_init_need_more(&init); ){
        i32 page_size = buffer_init_page_size(&init);
        page_size = LargeRoundUp(page_size, Kbytes(4));
        void *data = general_memory_allocate(general, page_size, BUBBLE_BUFFER);
        buffer_init_provide_page(&init, data, page_size);
    }
    i32 init_success = buffer_end_init(&init);
    Assert(init_success);
    
#if 0
    buffer_initialize(&file->buffer, val.str, val.size);
#endif
#endif
    
    file_synchronize_times(file, filename);
    file_init_strings(file);
    file_set_name(file, filename);
    
    file->base_map_id = mapid_file;

    file_measure_starts(general, file);
    file_measure_widths(general, file, font);
    file->font = font;

    i32 request_size = Kbytes(64);
    file->undo.undo.max = request_size;
    file->undo.undo.strings = (u8*)general_memory_allocate(general, request_size, BUBBLE_UNDO_STRING);
    file->undo.undo.edit_max = request_size / sizeof(Edit_Step);
    file->undo.undo.edits = (Edit_Step*)general_memory_allocate(general, request_size, BUBBLE_UNDO);
    
    file->undo.redo.max = request_size;
    file->undo.redo.strings = (u8*)general_memory_allocate(general, request_size, BUBBLE_UNDO_STRING);
    file->undo.redo.edit_max = request_size / sizeof(Edit_Step);
    file->undo.redo.edits = (Edit_Step*)general_memory_allocate(general, request_size, BUBBLE_UNDO);
    
    file->undo.history.max = request_size;
    file->undo.history.strings = (u8*)general_memory_allocate(general, request_size, BUBBLE_UNDO_STRING);
    file->undo.history.edit_max = request_size / sizeof(Edit_Step);
    file->undo.history.edits = (Edit_Step*)general_memory_allocate(general, request_size, BUBBLE_UNDO);
    
    file->undo.children.max = request_size;
    file->undo.children.strings = (u8*)general_memory_allocate(general, request_size, BUBBLE_UNDO_STRING);
    file->undo.children.edit_max = request_size / sizeof(Buffer_Edit);
    file->undo.children.edits = (Buffer_Edit*)general_memory_allocate(general, request_size, BUBBLE_UNDO);
    
    file->undo.history_block_count = 1;
    file->undo.history_head_block = 0;
    file->undo.current_block_normal = 1;
}

internal bool32
file_create(General_Memory *general, Editing_File *file, u8 *filename, Font *font){
    bool32 result = 0;
    
    File_Data raw_file = system_load_file(filename);
    if (raw_file.data){
        result = 1;
        String val = make_string((char*)raw_file.data, raw_file.size);
        file_create_from_string(general, file, filename, font, val);
        system_free_file(raw_file);
    }
    
    return result;
}

internal bool32
file_create_empty(General_Memory *general, Editing_File *file, u8 *filename, Font *font){
    bool32 result = 1;
    
    String empty_str = {};
    file_create_from_string(general, file, filename, font, empty_str);
    
    return result;
}

struct Get_File_Result{
    Editing_File *file;
    i32 index;
};

internal Get_File_Result
working_set_get_available_file(Working_Set *working_set){
    Get_File_Result result = {};
    
    for (i32 buffer_index = 1; buffer_index < working_set->file_max_count; ++buffer_index){
        if (buffer_index == working_set->file_index_count ||
            working_set->files[buffer_index].is_dummy){
            result.index = buffer_index;
            result.file = working_set->files + buffer_index;
            if (buffer_index == working_set->file_index_count){
                ++working_set->file_index_count;
            }
            break;
        }
    }
    
    Assert(result.file);
    return result;
}

internal void
file_close(General_Memory *general, Editing_File *file){
    if (file->still_lexing){
        system_cancel_job(BACKGROUND_THREADS, file->lex_job);
    }
    if (file->token_stack.tokens){
        general_memory_free(general, file->token_stack.tokens);
    }
#if BUFFER_EXPERIMENT_SCALPEL <= 1
    general_memory_free(general, file->buffer.data);
    general_memory_free(general, file->buffer.line_starts);
    general_memory_free(general, file->buffer.line_widths);
#endif
    
    general_memory_free(general, file->undo.undo.strings);
    general_memory_free(general, file->undo.undo.edits);
    
    general_memory_free(general, file->undo.redo.strings);
    general_memory_free(general, file->undo.redo.edits);
    
    general_memory_free(general, file->undo.history.strings);
    general_memory_free(general, file->undo.history.edits);
}

internal void
file_get_dummy(Editing_File *file){
	*file = {};
	file->is_dummy = 1;
}

struct Shift_Information{
	i32 start, end, amount;
};

#if BUFFER_EXPERIMENT_SCALPEL <= 0
internal
JOB_CALLBACK(job_full_lex){
    Editing_File *file = (Editing_File*)data[0];
    General_Memory *general = (General_Memory*)data[1];
    
    Cpp_File cpp_file;
    cpp_file.data = file->buffer.data;
    cpp_file.size = file->buffer.size;
    
    Cpp_Token_Stack tokens;
    tokens.tokens = (Cpp_Token*)memory->data;
    tokens.max_count = memory->size / sizeof(Cpp_Token);
    tokens.count = 0;
    
    Cpp_Lex_Data status;
    status = cpp_lex_file_nonalloc(cpp_file, &tokens);
    while (!status.complete){
        system_grow_thread_memory(memory);
        tokens.tokens = (Cpp_Token*)memory->data;
        tokens.max_count = memory->size / sizeof(Cpp_Token);
        status = cpp_lex_file_nonalloc(cpp_file, &tokens, status);
    }
    
    i32 new_max = LargeRoundUp(tokens.count, Kbytes(1));
    system_aquire_lock(FRAME_LOCK);
    if (file->token_stack.tokens){
        file->token_stack.tokens = (Cpp_Token*)
            general_memory_reallocate_nocopy(general, file->token_stack.tokens, new_max*sizeof(Cpp_Token), BUBBLE_TOKENS);
    }
    else{
        file->token_stack.tokens = (Cpp_Token*)
            general_memory_allocate(general, new_max*sizeof(Cpp_Token), BUBBLE_TOKENS);
    }
    system_release_lock(FRAME_LOCK);
    
    i32 copy_amount = Kbytes(8);
    i32 uncoppied = tokens.count*sizeof(Cpp_Token);
    if (copy_amount > uncoppied) copy_amount = uncoppied;
    
    u8 *dest = (u8*)file->token_stack.tokens;
    u8 *src = (u8*)tokens.tokens;
    
    while (uncoppied > 0){
        system_aquire_lock(FRAME_LOCK);
        memcpy(dest, src, copy_amount);
        system_release_lock(FRAME_LOCK);
        dest += copy_amount;
        src += copy_amount;
        uncoppied -= copy_amount;
        if (copy_amount > uncoppied) copy_amount = uncoppied;
    }
    
    file->token_stack.count = tokens.count;
    file->token_stack.max_count = new_max;
    system_force_redraw();
    
    // NOTE(allen): These are outside the locked section because I don't
    // think getting these out of order will cause critical bugs, and I
    // want to minimize what's done in locked sections.
    file->tokens_complete = 1;
    file->still_lexing = 0;
}
#endif

internal void
file_kill_tokens(General_Memory *general, Editing_File *file){
    if (file->still_lexing){
        system_cancel_job(BACKGROUND_THREADS, file->lex_job);
    }
    if (file->token_stack.tokens){
        general_memory_free(general, file->token_stack.tokens);
    }
    file->tokens_exist = 0;
    file->tokens_complete = 0;
    file->token_stack = {};
}

#if BUFFER_EXPERIMENT_SCALPEL <= 0
internal void
file_first_lex_parallel(General_Memory *general, Editing_File *file){
    Assert(file->token_stack.tokens == 0);
    
    file->tokens_complete = 0;
    file->tokens_exist = 1;
    file->still_lexing = 1;
    
    Job_Data job;
    job.callback = job_full_lex;
    job.data[0] = file;
    job.data[1] = general;
    job.memory_request = Kbytes(64);
    file->lex_job = system_post_job(BACKGROUND_THREADS, job);
}

internal void
file_relex_parallel(Mem_Options *mem, Editing_File *file,
                    i32 start_i, i32 end_i, i32 amount){
    General_Memory *general = &mem->general;
    Partition *part = &mem->part;
    if (file->token_stack.tokens == 0){
        file_first_lex_parallel(general, file);
        return;
    }
    
    bool32 inline_lex = !file->still_lexing;
    if (inline_lex){
        Cpp_File cpp_file;
        cpp_file.data = file->buffer.data;
        cpp_file.size = file->buffer.size;
        
        Cpp_Token_Stack *stack = &file->token_stack;
        
        Cpp_Relex_State state = 
            cpp_relex_nonalloc_start(cpp_file, stack,
                                     start_i, end_i, amount, 100);
        
        Temp_Memory temp = begin_temp_memory(part);
        i32 relex_end;
        Cpp_Token_Stack relex_space;
        relex_space.count = 0;
        relex_space.max_count = state.space_request;
        relex_space.tokens = push_array(part, Cpp_Token, relex_space.max_count);
        if (cpp_relex_nonalloc_main(&state, &relex_space, &relex_end)){
            inline_lex = 0;
        }
        else{
            i32 delete_amount = relex_end - state.start_token_i;
            i32 shift_amount = relex_space.count - delete_amount;
            
            if (shift_amount != 0){
                int new_count = stack->count + shift_amount;
                if (new_count > stack->max_count){
                    int new_max = LargeRoundUp(new_count, Kbytes(1));
                    stack->tokens = (Cpp_Token*)
                        general_memory_reallocate(general, stack->tokens,
                                                  stack->count*sizeof(Cpp_Token),
                                                  new_max*sizeof(Cpp_Token), BUBBLE_TOKENS);
                    stack->max_count = new_max;
                }
                
                int shift_size = stack->count - relex_end;
                if (shift_size > 0){
                    Cpp_Token *old_base = stack->tokens + relex_end;
                    memmove(old_base + shift_amount, old_base,
                            sizeof(Cpp_Token)*shift_size);
                }
                
                stack->count += shift_amount;
            }
            
            memcpy(state.stack->tokens + state.start_token_i, relex_space.tokens,
                   sizeof(Cpp_Token)*relex_space.count);
        }
        
        end_temp_memory(temp);
    }
    
    if (!inline_lex){
        i32 end_token_i = cpp_get_end_token(&file->token_stack, end_i);
        cpp_shift_token_starts(&file->token_stack, end_token_i, amount);
        --end_token_i;
        if (end_token_i >= 0){
            Cpp_Token *token = file->token_stack.tokens + end_token_i;
            if (token->start < end_i && token->start + token->size > end_i){
                token->size += amount;
            }
        }
        
        file->still_lexing = 1;
        
        Job_Data job;
        job.callback = job_full_lex;
        job.data[0] = file;
        job.data[1] = general;
        job.memory_request = Kbytes(64);
        file->lex_job = system_post_job(BACKGROUND_THREADS, job);
    }
}
#endif

internal bool32
file_grow_as_needed(General_Memory *general, Editing_File *file, i32 additional_size){
    bool32 result = 1;
#if BUFFER_EXPERIMENT_SCALPEL <= 1
    i32 size = buffer_size(&file->buffer);
    i32 target_size = size + additional_size + 1;
    if (target_size >= file->buffer.max){
		i32 request_size = LargeRoundUp(target_size*2, Kbytes(256));
        char *new_data = (char*)
            general_memory_allocate(general, request_size, BUBBLE_BUFFER);
        TentativeAssert(new_data);
        void *old_data = buffer_relocate(&file->buffer, new_data, request_size);
        general_memory_free(general, old_data);
	}
#endif
    return result;
}

internal void
undo_stack_grow_string(General_Memory *general, Edit_Stack *stack, i32 extra_size){
    i32 old_max = stack->max;
    u8 *old_str = stack->strings;
    i32 new_max = old_max*2 + extra_size;
    u8 *new_str = (u8*)
        general_memory_reallocate(general, old_str, old_max, new_max);
    stack->strings = new_str;
    stack->max = new_max;
}

internal void
undo_stack_grow_edits(General_Memory *general, Edit_Stack *stack){
    i32 old_max = stack->edit_max;
    Edit_Step *old_eds = stack->edits;
    i32 new_max = old_max*2 + 2;
    Edit_Step *new_eds = (Edit_Step*)
        general_memory_reallocate(general, old_eds, old_max*sizeof(Edit_Step), new_max*sizeof(Edit_Step));
    stack->edits = new_eds;
    stack->edit_max = new_max;
}

internal void
child_stack_grow_string(General_Memory *general, Small_Edit_Stack *stack, i32 extra_size){
    i32 old_max = stack->max;
    u8 *old_str = stack->strings;
    i32 new_max = old_max*2 + extra_size;
    u8 *new_str = (u8*)
        general_memory_reallocate(general, old_str, old_max, new_max);
    stack->strings = new_str;
    stack->max = new_max;
}

internal void
child_stack_grow_edits(General_Memory *general, Small_Edit_Stack *stack, i32 amount){
    i32 old_max = stack->edit_max;
    Buffer_Edit *old_eds = stack->edits;
    i32 new_max = old_max*2 + amount;
    Buffer_Edit *new_eds = (Buffer_Edit*)
        general_memory_reallocate(general, old_eds, old_max*sizeof(Buffer_Edit), new_max*sizeof(Buffer_Edit));
    stack->edits = new_eds;
    stack->edit_max = new_max;
}

internal i32
undo_children_push(General_Memory *general, Small_Edit_Stack *children,
                   Buffer_Edit *edits, i32 edit_count, u8 *strings, i32 string_size){
    i32 result = children->edit_count;
    if (children->edit_count + edit_count > children->edit_max)
        child_stack_grow_edits(general, children, edit_count);
    
    if (children->size + string_size > children->max)
        child_stack_grow_string(general, children, string_size);
    
    memcpy(children->edits + children->edit_count, edits, edit_count*sizeof(Buffer_Edit));
    memcpy(children->strings + children->size, strings, string_size);
    
    Buffer_Edit *edit = children->edits + children->edit_count;
    i32 start_pos = children->size;
    for (i32 i = 0; i < edit_count; ++i, ++edit){
        edit->str_start += start_pos;
    }
    
    children->edit_count += edit_count;
    children->size += string_size;
    
    return result;
}

struct Edit_Spec{
    u8 *str;
    Edit_Step step;
};

#if BUFFER_EXPERIMENT_SCALPEL <= 1
internal Edit_Step*
file_post_undo(General_Memory *general, Editing_File *file,
               Edit_Step step, bool32 do_merge, bool32 can_merge){
    if (step.type == ED_NORMAL){
        file->undo.redo.size = 0;
        file->undo.redo.edit_count = 0;
    }
    
    Edit_Stack *undo = &file->undo.undo;
    Edit_Step *result = 0;
    
    if (step.child_count == 0){
        if (step.edit.end - step.edit.start + undo->size > undo->max)
            undo_stack_grow_string(general, undo, step.edit.end - step.edit.start);
    
        Buffer_Edit inv;
        buffer_invert_edit(&file->buffer, step.edit, &inv,
                           (char*)undo->strings, &undo->size, undo->max);
    
        Edit_Step inv_step = {};
        inv_step.edit = inv;
        inv_step.pre_pos = step.pre_pos;
        inv_step.post_pos = step.post_pos;
        inv_step.can_merge = (b8)can_merge;
        inv_step.type = ED_UNDO;
    
        bool32 did_merge = 0;
        if (do_merge && undo->edit_count > 0){
            Edit_Step prev = undo->edits[undo->edit_count-1];
            if (prev.can_merge && inv_step.edit.len == 0 && prev.edit.len == 0){
                if (prev.edit.end == inv_step.edit.start){
                    did_merge = 1;
                    inv_step.edit.start = prev.edit.start;
                    inv_step.pre_pos = prev.pre_pos;
                }
            }
        }
    
        if (did_merge){
            result = undo->edits + (undo->edit_count-1);
            *result = inv_step;
        }
        else{
            if (undo->edit_count == undo->edit_max)
                undo_stack_grow_edits(general, undo);
            result = undo->edits + (undo->edit_count++);
            *result = inv_step;
        }
    }
    else{
        Edit_Step inv_step = {};
        inv_step.type = ED_UNDO;
        inv_step.first_child = step.inverse_first_child;
        inv_step.inverse_first_child = step.first_child;
        inv_step.special_type = step.special_type;
        inv_step.child_count = step.inverse_child_count;
        inv_step.inverse_child_count = step.child_count;
        
        if (undo->edit_count == undo->edit_max)
            undo_stack_grow_edits(general, undo);
        result = undo->edits + (undo->edit_count++);
        *result = inv_step;
    }
    return result;
}
#endif    

inline void
undo_stack_pop(Edit_Stack *stack){
    if (stack->edit_count > 0){
        Edit_Step *edit = stack->edits + (--stack->edit_count);
        stack->size -= edit->edit.len;
    }
}

#if BUFFER_EXPERIMENT_SCALPEL <= 1
internal void
file_post_redo(General_Memory *general, Editing_File *file, Edit_Step step){
    Edit_Stack *redo = &file->undo.redo;

    if (step.child_count == 0){
        if (step.edit.end - step.edit.start + redo->size > redo->max)
            undo_stack_grow_string(general, redo, step.edit.end - step.edit.start);
    
        Buffer_Edit inv;
        buffer_invert_edit(&file->buffer, step.edit, &inv,
                           (char*)redo->strings, &redo->size, redo->max);
    
        Edit_Step inv_step = {};
        inv_step.edit = inv;
        inv_step.pre_pos = step.pre_pos;
        inv_step.post_pos = step.post_pos;
        inv_step.type = ED_REDO;
    
        if (redo->edit_count == redo->edit_max)
            undo_stack_grow_edits(general, redo);
        redo->edits[redo->edit_count++] = inv_step;
    }
    else{
        Edit_Step inv_step = {};
        inv_step.type = ED_REDO;
        inv_step.first_child = step.inverse_first_child;
        inv_step.inverse_first_child = step.first_child;
        inv_step.special_type = step.special_type;
        inv_step.child_count = step.inverse_child_count;
        inv_step.inverse_child_count = step.child_count;
        
        if (redo->edit_count == redo->edit_max)
            undo_stack_grow_edits(general, redo);
        redo->edits[redo->edit_count++] = inv_step;
    }
}
#endif

inline void
file_post_history_block(Editing_File *file, i32 pos){
    Assert(file->undo.history_head_block < pos);
    Assert(pos < file->undo.history.edit_count);
    
    Edit_Step *history = file->undo.history.edits;
    Edit_Step *step = history + file->undo.history_head_block;
    step->next_block = pos;
    step = history + pos;
    step->prev_block = file->undo.history_head_block;
    file->undo.history_head_block = pos;
    ++file->undo.history_block_count;
}

inline void
file_unpost_history_block(Editing_File *file){
    Assert(file->undo.history_block_count > 1);
    --file->undo.history_block_count;
    Edit_Step *old_head = file->undo.history.edits + file->undo.history_head_block;
    file->undo.history_head_block = old_head->prev_block;
}

#if BUFFER_EXPERIMENT_SCALPEL <= 1
internal Edit_Step*
file_post_history(General_Memory *general, Editing_File *file,
                  Edit_Step step, bool32 do_merge, bool32 can_merge){
    Edit_Stack *history = &file->undo.history;
    Edit_Step *result = 0;
    
    persist Edit_Type reverse_types[4];
    if (reverse_types[ED_UNDO] == 0){
        reverse_types[ED_NORMAL] = ED_REVERSE_NORMAL;
        reverse_types[ED_REVERSE_NORMAL] = ED_NORMAL;
        reverse_types[ED_UNDO] = ED_REDO;
        reverse_types[ED_REDO] = ED_UNDO;
    }
    
    if (step.child_count == 0){
        if (step.edit.end - step.edit.start + history->size > history->max)
            undo_stack_grow_string(general, history, step.edit.end - step.edit.start);
        
        Buffer_Edit inv;
        buffer_invert_edit(&file->buffer, step.edit, &inv,
                           (char*)history->strings, &history->size, history->max);
        
        Edit_Step inv_step = {};
        inv_step.edit = inv;
        inv_step.pre_pos = step.pre_pos;
        inv_step.post_pos = step.post_pos;
        inv_step.can_merge = (b8)can_merge;
        inv_step.type = reverse_types[step.type];
        
        bool32 did_merge = 0;
        if (do_merge && history->edit_count > 0){
            Edit_Step prev = history->edits[history->edit_count-1];
            if (prev.can_merge && inv_step.edit.len == 0 && prev.edit.len == 0){
                if (prev.edit.end == inv_step.edit.start){
                    did_merge = 1;
                    inv_step.edit.start = prev.edit.start;
                    inv_step.pre_pos = prev.pre_pos;
                }
            }
        }
        
        if (did_merge){
            result = history->edits + (history->edit_count-1);
        }
        else{
            if (history->edit_count == history->edit_max)
                undo_stack_grow_edits(general, history);
            result = history->edits + (history->edit_count++);
        }
        
        *result = inv_step;
    }
    else{
        Edit_Step inv_step = {};
        inv_step.type = reverse_types[step.type];
        inv_step.first_child = step.inverse_first_child;
        inv_step.inverse_first_child = step.first_child;
        inv_step.special_type = step.special_type;
        inv_step.inverse_child_count = step.child_count;
        inv_step.child_count = step.inverse_child_count;
        
        if (history->edit_count == history->edit_max)
            undo_stack_grow_edits(general, history);
        result = history->edits + (history->edit_count++);
        *result = inv_step;
    }
    
    return result;
}
#endif

inline Full_Cursor
view_compute_cursor_from_pos(File_View *view, i32 pos){
#if BUFFER_EXPERIMENT_SCALPEL <= 1    
    Editing_File *file = view->file;
    Style *style = view->style;
    Font *font = style->font;
    
    real32 max_width = view_compute_width(view);
    Opaque_Font_Advance opad = get_opaque_font_advance(font);

    return buffer_cursor_from_pos(&file->buffer, pos, view->line_wrap_y,
                                  max_width, (real32)font->height, opad.data, opad.stride);
#else
    return view->cursor;
#endif
}

inline Full_Cursor
view_compute_cursor_from_unwrapped_xy(File_View *view, real32 seek_x, real32 seek_y,
                                      bool32 round_down = 0){
#if BUFFER_EXPERIMENT_SCALPEL <= 1    
    Editing_File *file = view->file;
    Style *style = view->style;
    Font *font = style->font;
    
    real32 max_width = view_compute_width(view);
    Opaque_Font_Advance opad = get_opaque_font_advance(font);

    return buffer_cursor_from_unwrapped_xy(&file->buffer, seek_x, seek_y, round_down, view->line_wrap_y,
                                           max_width, (real32)font->height, opad.data, opad.stride);
#else
    return view->cursor;
#endif
}

inline Full_Cursor
view_compute_cursor_from_wrapped_xy(File_View *view, real32 seek_x, real32 seek_y,
                                    bool32 round_down = 0){
#if BUFFER_EXPERIMENT_SCALPEL <= 1    
    Editing_File *file = view->file;
    Style *style = view->style;
    Font *font = style->font;
    
    real32 max_width = view_compute_width(view);
    Opaque_Font_Advance opad = get_opaque_font_advance(font);
    
    return buffer_cursor_from_wrapped_xy(&file->buffer, seek_x, seek_y, round_down, view->line_wrap_y,
                                         max_width, (real32)font->height, opad.data, opad.stride);
#else
    return view->cursor;
#endif
}

inline Full_Cursor
view_compute_cursor_from_xy(File_View *view, real32 seek_x, real32 seek_y){
    Full_Cursor result;
    if (view->unwrapped_lines) result = view_compute_cursor_from_unwrapped_xy(view, seek_x, seek_y);
    else result = view_compute_cursor_from_wrapped_xy(view, seek_x, seek_y);
	return result;
}

inline void
view_set_temp_highlight(File_View *view, i32 pos, i32 end_pos){
	view->temp_highlight = view_compute_cursor_from_pos(view, pos);
	view->temp_highlight_end_pos = end_pos;
    view->show_temp_highlight = 1;
}

inline i32
view_get_cursor_pos(File_View *view){
    i32 result;
    if (view->show_temp_highlight){
        result = view->temp_highlight.pos;
    }
    else{
        result = view->cursor.pos;
    }
    return result;
}

inline real32
view_get_cursor_x(File_View *view){
    real32 result;
    Full_Cursor *cursor;
    if (view->show_temp_highlight){
        cursor = &view->temp_highlight;
    }
    else{
        cursor = &view->cursor;
    }
    if (view->unwrapped_lines){
        result = cursor->unwrapped_x;
    }
    else{
        result = cursor->wrapped_x;
    }
    return result;
}

inline real32
view_get_cursor_y(File_View *view){
    Full_Cursor *cursor;
    real32 result;
    
    if (view->show_temp_highlight) cursor = &view->temp_highlight;
    else cursor = &view->cursor;
    
    if (view->unwrapped_lines) result = cursor->unwrapped_y;
    else result = cursor->wrapped_y;
    
    return result;
}

internal void
view_set_file(File_View *view, Editing_File *file, Style *style,
              Custom_Command_Function *open_hook, void *cmd_context, Application_Links app){
    Panel *panel = view->view_base.panel;
    view->file = file;
    
    General_Memory *general = &view->view_base.mem->general;
    AllowLocal(general);
    Font *font = style->font;
    view->style = style;
    view->font_advance = font->advance;
    view->font_height = font->height;

    view_measure_wraps(general, view);
    
    view->cursor = {};
    view->cursor = view_compute_cursor_from_pos(view, file->cursor_pos);
    
    real32 cursor_x, cursor_y;
    real32 w, h;
    real32 target_x, target_y;
    
    cursor_x = view_get_cursor_x(view);
    cursor_y = view_get_cursor_y(view);
    
    w = (real32)(panel->inner.x1 - panel->inner.x0);
    h = (real32)(panel->inner.y1 - panel->inner.y0);
    
    target_x = 0;
    if (cursor_x < target_x){
        target_x = (real32)Max(0, cursor_x - w*.5f);
    }
    else if (cursor_x >= target_x + w){
        target_x = (real32)(cursor_x - w*.5f);
    }
    
    target_y = (real32)FLOOR32(cursor_y - h*.5f);
    if (target_y < 0) target_y = 0;
    
    view->target_x = target_x;
    view->target_y = target_y;
    view->scroll_x = target_x;
    view->scroll_y = target_y;
    
    view->vel_y = 1.f;
    view->vel_x = 1.f;
    
    if (open_hook) open_hook(cmd_context, app);
}

struct Relative_Scrolling{
    real32 scroll_x, scroll_y;
    real32 target_x, target_y;
};

internal Relative_Scrolling
view_get_relative_scrolling(File_View *view){
    Relative_Scrolling result;
    real32 cursor_x, cursor_y;
    cursor_x = view_get_cursor_x(view);
    cursor_y = view_get_cursor_y(view);
    result.scroll_x = cursor_x - view->scroll_x;
    result.scroll_y = cursor_y - view->scroll_y;
    result.target_x = cursor_x - view->target_x;
    result.target_y = cursor_y - view->target_y;
    return result;
}

internal void
view_set_relative_scrolling(File_View *view, Relative_Scrolling scrolling){
    real32 cursor_x, cursor_y;
    cursor_x = view_get_cursor_x(view);
    cursor_y = view_get_cursor_y(view);
    view->scroll_y = cursor_y - scrolling.scroll_y;
    view->target_y = cursor_y - scrolling.target_y;
    if (view->scroll_y < 0) view->scroll_y = 0;
    if (view->target_y < 0) view->target_y = 0;
}

inline void
view_cursor_move(File_View *view, Full_Cursor cursor){
	view->cursor = cursor;
    view->preferred_x = view_get_cursor_x(view);
	view->file->cursor_pos = view->cursor.pos;
    view->show_temp_highlight = 0;
}

inline void
view_cursor_move(File_View *view, i32 pos){
	view->cursor = view_compute_cursor_from_pos(view, pos);
    view->preferred_x = view_get_cursor_x(view);
	view->file->cursor_pos = view->cursor.pos;
    view->show_temp_highlight = 0;
}

inline void
view_cursor_move(File_View *view, real32 x, real32 y, bool32 round_down = 0){
    if (view->unwrapped_lines){
        view->cursor = view_compute_cursor_from_unwrapped_xy(view, x, y, round_down);
    }
    else{
        view->cursor = view_compute_cursor_from_wrapped_xy(view, x, y, round_down);
    }
    view->preferred_x = view_get_cursor_x(view);
	view->file->cursor_pos = view->cursor.pos;
    view->show_temp_highlight = 0;
}

inline void
view_set_widget(File_View *view, File_View_Widget_Type type){
    view->widget.type = type;
}

inline i32
view_widget_height(File_View *view, Font *font){
    i32 result = 0;
    switch (view->widget.type){
    case FWIDG_NONE: break;
    case FWIDG_SEARCH: result = font->height + 2; break;
    case FWIDG_GOTO_LINE: result = font->height + 2; break;
    case FWIDG_TIMELINES: result = view->widget.height; break;
    }
    return result;
}

inline i32_Rect
view_widget_rect(File_View *view, Font *font){
    Panel *panel = view->view_base.panel;
    i32_Rect whole = panel->inner;
    i32_Rect result;
    result.x0 = whole.x0;
    result.x1 = whole.x1;
    result.y0 = whole.y0 + font->height + 2;
    result.y1 = result.y0 + view_widget_height(view, font);
    
    return result;
}

#if FRED_SLOW
inline b32
debug_edit_step_check(Edit_Step a, Edit_Step b){
    Assert(a.type == b.type);
    Assert(a.can_merge == b.can_merge);
    Assert(a.pre_pos == b.pre_pos);
    Assert(a.post_pos == b.post_pos);
    Assert(a.edit.start == b.edit.start);
    Assert(a.edit.end == b.edit.end);
    Assert(a.edit.len == b.edit.len);
    return 1;
}
#endif

enum History_Mode{
    hist_normal,
    hist_backward,
    hist_forward
};

internal void
view_update_history_before_edit(Mem_Options *mem, Editing_File *file, Edit_Step step, u8 *str,
                                History_Mode history_mode){
#if BUFFER_EXPERIMENT_SCALPEL <= 1
    General_Memory *general = &mem->general;
    
#if FRED_SLOW
    if (history_mode == hist_backward)
        debug_edit_step_check(step, file->undo.history.edits[file->undo.edit_history_cursor]);
    else if (history_mode == hist_forward)
        debug_edit_step_check(step, file->undo.history.edits[file->undo.history.edit_count]);
    switch (step.type){
    case ED_UNDO:
    {
        Assert(file->undo.undo.edit_count > 0);
        debug_edit_step_check(step, file->undo.undo.edits[file->undo.undo.edit_count-1]);
    }break;
    case ED_REDO:
    {
        Assert(file->undo.redo.edit_count > 0);
        debug_edit_step_check(step, file->undo.redo.edits[file->undo.redo.edit_count-1]);
    }break;
    }
#endif
    
    bool32 can_merge = 0, do_merge = 0;
    switch (step.type){
    case ED_NORMAL:
    {
        if (step.edit.len == 1 && str && char_is_alpha_numeric(*str)) can_merge = 1;
        if (step.edit.len == 1 && str && (can_merge || char_is_whitespace(*str))) do_merge = 1;
        
        if (history_mode != hist_forward)
            file_post_history(general, file, step, do_merge, can_merge);
        
        file_post_undo(general, file, step, do_merge, can_merge);
    }break;
    
    case ED_REVERSE_NORMAL:
    {
        if (history_mode != hist_forward)
            file_post_history(general, file, step, do_merge, can_merge);
        
        undo_stack_pop(&file->undo.undo);
        
        bool32 restore_redos = 0;
        Edit_Step *redo_end = 0;
        
        if (history_mode == hist_backward && file->undo.edit_history_cursor > 0){
            restore_redos = 1;
            redo_end = file->undo.history.edits + (file->undo.edit_history_cursor - 1);
        }
        else if (history_mode == hist_forward && file->undo.history.edit_count > 0){
            restore_redos = 1;
            redo_end = file->undo.history.edits + (file->undo.history.edit_count - 1);
        }
        
        if (restore_redos){
            Edit_Step *redo_start = redo_end;
            i32 steps_of_redo = 0;
            i32 strings_of_redo = 0;
            i32 undo_count = 0;
            while (redo_start->type == ED_REDO || redo_start->type == ED_UNDO){
                if (redo_start->type == ED_REDO){
                    if (undo_count > 0) --undo_count;
                    else{
                        ++steps_of_redo;
                        strings_of_redo += redo_start->edit.len;
                    }
                }
                else{
                    ++undo_count;
                }
                --redo_start;
            }
            
            if (redo_start < redo_end){
                ++redo_start;
                ++redo_end;
                
                if (file->undo.redo.edit_count + steps_of_redo > file->undo.redo.edit_max)
                    undo_stack_grow_edits(general, &file->undo.redo);
                
                if (file->undo.redo.size + strings_of_redo > file->undo.redo.max)
                    undo_stack_grow_string(general, &file->undo.redo, strings_of_redo);
                
                u8 *str_src = file->undo.history.strings + redo_end->edit.str_start;
                u8 *str_dest_base = file->undo.redo.strings;
                i32 str_redo_pos = file->undo.redo.size + strings_of_redo;
                
                Edit_Step *edit_src = redo_end;
                Edit_Step *edit_dest = file->undo.redo.edits + file->undo.redo.edit_count + steps_of_redo;
                
                i32 undo_count = 0;
                for (i32 i = 0; i < steps_of_redo;){
                    --edit_src;
                    str_src -= edit_src->edit.len;
                    if (edit_src->type == ED_REDO){
                        if (undo_count > 0){
                            --undo_count;
                        }
                        else{
                            ++i;
                            
                            --edit_dest;
                            *edit_dest = *edit_src;
                            
                            str_redo_pos -= edit_dest->edit.len;
                            edit_dest->edit.str_start = str_redo_pos;
                            
                            memcpy(str_dest_base + str_redo_pos, str_src, edit_dest->edit.len);
                        }
                    }
                    else{
                        ++undo_count;
                    }
                }
                Assert(undo_count == 0);
                
                file->undo.redo.size += strings_of_redo;
                file->undo.redo.edit_count += steps_of_redo;
            }
        }
    }break;
    
    case ED_UNDO:
    {
        if (history_mode != hist_forward)
            file_post_history(general, file, step, do_merge, can_merge);
        file_post_redo(general, file, step);
        undo_stack_pop(&file->undo.undo);
    }break;
    
    case ED_REDO:
    {
        if (step.edit.len == 1 && str && char_is_alpha_numeric(*str)) can_merge = 1;
        if (step.edit.len == 1 && str && (can_merge || char_is_whitespace(*str))) do_merge = 1;
        
        if (history_mode != hist_forward)
            file_post_history(general, file, step, do_merge, can_merge);
        
        file_post_undo(general, file, step, do_merge, can_merge);
        undo_stack_pop(&file->undo.redo);
    }break;
    }
    
    if (history_mode != hist_forward){
        if (step.type == ED_UNDO || step.type == ED_REDO){
            if (file->undo.current_block_normal){
                file_post_history_block(file, file->undo.history.edit_count - 1);
                file->undo.current_block_normal = 0;
            }
        }
        else{
            if (!file->undo.current_block_normal){
                file_post_history_block(file, file->undo.history.edit_count - 1);
                file->undo.current_block_normal = 1;
            }
        }
    }
    else{
        if (file->undo.history_head_block == file->undo.history.edit_count){
            file_unpost_history_block(file);
            file->undo.current_block_normal = !file->undo.current_block_normal;
        }
    }
    
    if (history_mode == hist_normal) file->undo.edit_history_cursor = file->undo.history.edit_count;
#endif
}

inline b32
debug_step_match(Edit_Step a, Edit_Step b){
    Assert(a.type == b.type);
    Assert(a.can_merge == b.can_merge);
    Assert(a.pre_pos == b.pre_pos);
    Assert(a.post_pos == b.post_pos);
    Assert(a.next_block == b.next_block);
    Assert(a.prev_block == b.prev_block);
    Assert(a.edit.start == b.edit.start);
    Assert(a.edit.end == b.edit.end);
    Assert(a.edit.len == b.edit.len);
    return 1;
}

inline void
file_pre_edit_maintenance(Editing_File *file){
    if (file->still_lexing)
        system_cancel_job(BACKGROUND_THREADS, file->lex_job);
    
    file->last_4ed_edit_time = system_get_now();
}

internal void
view_do_single_edit(Mem_Options *mem, File_View *view, Editing_File *file,
                    Editing_Layout *layout, Edit_Spec spec, History_Mode history_mode){
    Assert(file);
    ProfileMomentFunction();
    
    // NOTE(allen): fixing stuff beforewards????
    view_update_history_before_edit(mem, file, spec.step, spec.str, history_mode);
    file_pre_edit_maintenance(file);

#if BUFFER_EXPERIMENT_SCALPEL <= 1    
    // NOTE(allen): actual text replacement
    General_Memory *general = &mem->general;

    i32 start = spec.step.edit.start;
    i32 end = spec.step.edit.end;
    char *str = (char*)spec.str;
    i32 str_len = spec.step.edit.len;

    i32 shift_amount = 0;
    while (buffer_replace_range(&file->buffer, start, end, str, str_len, &shift_amount))
        file_grow_as_needed(general, file, shift_amount);
    
#if BUFFER_EXPERIMENT_SCALPEL <= 0
    // NOTE(allen): fixing stuff afterwards
    if (file->tokens_exist)
        file_relex_parallel(mem, file, start, end, shift_amount);
#endif
    
    i32 line_start = buffer_get_line_index(&file->buffer, start);
    i32 line_end = buffer_get_line_index(&file->buffer, end);
    i32 replaced_line_count = line_end - line_start;
    i32 new_line_count = file_count_newlines(file, start, start+str_len);
    i32 line_shift =  new_line_count - replaced_line_count;
    
    file_remeasure_starts(general, file, line_start, line_end, line_shift, shift_amount);
    file_remeasure_widths(general, file, file->font, line_start, line_end, line_shift);

    i32 panel_count = layout->panel_count;
    Panel *current_panel = layout->panels;
    for (i32 i = 0; i < panel_count; ++i, ++current_panel){
        File_View *current_view = view_to_file_view(current_panel->view);
        if (current_view && current_view->file == file){
            view_measure_wraps(general, current_view);
        }
    }

    Temp_Memory cursor_temp = begin_temp_memory(&mem->part);
    i32 cursor_max = layout->panel_max_count * 2;
    Cursor_With_Index *cursors = push_array(&mem->part, Cursor_With_Index, cursor_max);
    
    i32 cursor_count = 0;
    current_panel = layout->panels;
    for (i32 i = 0; i < panel_count; ++i, ++current_panel){
        File_View *current_view = view_to_file_view(current_panel->view);
        if (current_view && current_view->file == file){
            view_measure_wraps(general, current_view);
            if (current_view != view){
                write_cursor_with_index(cursors, &cursor_count, current_view->cursor.pos);
                write_cursor_with_index(cursors, &cursor_count, current_view->mark);
            }
        }
    }
    
    if (cursor_count > 0){
        buffer_sort_cursors(cursors, cursor_count);
        buffer_update_cursors(cursors, cursor_count,
                              start, end, shift_amount + (end - start));
        buffer_unsort_cursors(cursors, cursor_count);
    }
    
    cursor_count = 0;
    current_panel = layout->panels;
    for (i32 i = 0; i < panel_count; ++i, ++current_panel){
        File_View *current_view = view_to_file_view(current_panel->view);
        if (current_view && current_view->file == file){
            if (current_view != view){
                view_cursor_move(current_view, cursors[cursor_count++].pos);
                current_view->mark = cursors[cursor_count++].pos;
                current_view->preferred_x = view_get_cursor_x(current_view);
            }
        }
    }
    
    end_temp_memory(cursor_temp);
#endif
}

internal void
view_do_white_batch_edit(Mem_Options *mem, File_View *view, Editing_File *file,
                         Editing_Layout *layout, Edit_Spec spec, History_Mode history_mode){
#if BUFFER_EXPERIMENT_SCALPEL <= 1
    Assert(file);
    ProfileMomentFunction();
    
    // NOTE(allen): fixing stuff beforewards????
    Assert(spec.str == 0);
    view_update_history_before_edit(mem, file, spec.step, 0, history_mode);
    file_pre_edit_maintenance(file);
    
    // NOTE(allen): actual text replacement
    General_Memory *general = &mem->general;
    Partition *part = &mem->part;
    
    u8 *str_base = file->undo.children.strings;
    i32 batch_size = spec.step.child_count;
    Buffer_Edit *batch = file->undo.children.edits + spec.step.first_child;
    
    Assert(spec.step.first_child < file->undo.children.edit_count);
    Assert(batch_size >= 0);
    
    i32 shift_amount = buffer_batch_edit_max_shift(batch, batch_size);
    file_grow_as_needed(general, file, shift_amount);
    
    buffer_batch_edit(&file->buffer, batch, (char*)str_base, batch_size);
    
    // NOTE(allen): token fixing
    if (file->tokens_complete){
        Cpp_Token_Stack tokens = file->token_stack;
        Cpp_Token *token = tokens.tokens;
        Cpp_Token *end_token = tokens.tokens + tokens.count;
        
        Buffer_Edit *edit = batch;
        Buffer_Edit *end_edit = batch + batch_size;
        
        i32 shift_amount = 0;
        
        for (; token < end_token && edit < end_edit; ++edit){
            for (; token->start < edit->start && token < end_token; ++token){
                token->start += shift_amount;
            }
            shift_amount += (edit->len - (edit->end - edit->start));
        }
        for (; token < end_token; ++token){
            token->start += shift_amount;
        }
    }
    
    // NOTE(allen): meta data
    {
        Buffer_Measure_Starts state = {};
        if (buffer_measure_starts(&state, &file->buffer)) Assert(0);
        
        Opaque_Font_Advance opad = get_opaque_font_advance(file->font);
        buffer_measure_widths(&file->buffer, opad.data, opad.stride);
    }
    
    // NOTE(allen): cursor fixing
    {
        Temp_Memory cursor_temp = begin_temp_memory(part);
            i32 cursor_max = layout->panel_max_count * 2;
        Cursor_With_Index *cursors = push_array(part, Cursor_With_Index, cursor_max);
    
        i32 panel_count = layout->panel_count;
        i32 cursor_count = 0;
        Panel *current_panel = layout->panels;
        for (i32 i = 0; i < panel_count; ++i, ++current_panel){
            File_View *current_view = view_to_file_view(current_panel->view);
            if (current_view && current_view->file == file){
                view_measure_wraps(general, current_view);
                write_cursor_with_index(cursors, &cursor_count, current_view->cursor.pos);
                write_cursor_with_index(cursors, &cursor_count, current_view->mark);
            }
        }
    
        if (cursor_count > 0){
            buffer_sort_cursors(cursors, cursor_count);
            buffer_batch_edit_update_cursors(cursors, cursor_count, batch, batch_size);
            buffer_unsort_cursors(cursors, cursor_count);
        }
    
        cursor_count = 0;
        current_panel = layout->panels;
        for (i32 i = 0; i < panel_count; ++i, ++current_panel){
            File_View *current_view = view_to_file_view(current_panel->view);
            if (current_view && current_view->file == file){
                view_cursor_move(current_view, cursors[cursor_count++].pos);
                current_view->mark = cursors[cursor_count++].pos;
                current_view->preferred_x = view_get_cursor_x(current_view);
            }
        }
        end_temp_memory(cursor_temp);
    }
#endif
}

inline void
view_replace_range(Mem_Options *mem, File_View *view, Editing_Layout *layout,
                   i32 start, i32 end, u8 *str, i32 len, i32 next_cursor){
    Edit_Spec spec = {};
    spec.step.type = ED_NORMAL;
    spec.step.edit.start =  start;
    spec.step.edit.end = end;
    spec.step.edit.len = len;
    spec.step.pre_pos = view->cursor.pos;
    spec.step.post_pos = next_cursor;
    spec.str = str;
    view_do_single_edit(mem, view, view->file, layout, spec, hist_normal);
}

internal void
view_post_paste_effect(File_View *view, i32 ticks, i32 start, i32 size, u32 color){
    view->paste_effect.start = start;
    view->paste_effect.end = start + size;
    view->paste_effect.color = color;
    view->paste_effect.tick_down = ticks;
    view->paste_effect.tick_max = ticks;
}

internal void
view_undo_redo(Mem_Options *mem, Editing_Layout *layout, File_View *view, Editing_File *file,
               Edit_Stack *stack, Edit_Type expected_type){
    if (file && stack->edit_count > 0){
        Edit_Step step = stack->edits[stack->edit_count-1];
        
        Assert(step.type == expected_type);
        
        Edit_Spec spec;
        spec.step = step;
        
        if (step.child_count == 0){
            spec.step.edit.str_start = 0;
            spec.str = stack->strings + step.edit.str_start;
            
            view_do_single_edit(mem, view, file, layout, spec, hist_normal);

            if (expected_type == ED_UNDO) view_cursor_move(view, step.pre_pos);
            else view_cursor_move(view, step.post_pos);
            view->mark = view->cursor.pos;
            
            view_post_paste_effect(view, 10, step.edit.start, step.edit.len,
                                   view->style->main.undo_color);
        }
        else{
            TentativeAssert(spec.step.special_type == 1);
            view_do_white_batch_edit(mem, view, file, layout, spec, hist_normal);
        }
    }
}

inline void
view_undo(Mem_Options *mem, Editing_Layout *layout, File_View *view){
    Editing_File *file = view->file;
    view_undo_redo(mem, layout, view, file, &file->undo.undo, ED_UNDO);
}

inline void
view_redo(Mem_Options *mem, Editing_Layout *layout, File_View *view){
    Editing_File *file = view->file;
    view_undo_redo(mem, layout, view, file, &file->undo.redo, ED_REDO);
}

internal void
view_history_step(Mem_Options *mem, Editing_Layout *layout, File_View *view, History_Mode history_mode){
    Assert(history_mode != hist_normal);
    
    Editing_File *file = view->file;
    bool32 do_history_step = 0;
    Edit_Step step = {};
    if (history_mode == hist_backward){
        if (file->undo.edit_history_cursor > 0){
            do_history_step = 1;
            step = file->undo.history.edits[--file->undo.edit_history_cursor];
        }
    }
    else{
        if (file->undo.edit_history_cursor < file->undo.history.edit_count){
            Assert(((file->undo.history.edit_count - file->undo.edit_history_cursor) & 1) == 0);
            step = file->undo.history.edits[--file->undo.history.edit_count];
            file->undo.history.size -= step.edit.len;
            ++file->undo.edit_history_cursor;
            do_history_step = 1;
        }
    }
    
    if (do_history_step){
        Edit_Spec spec;
        spec.step = step;
        
        if (spec.step.child_count == 0){
            spec.step.edit.str_start = 0;
            spec.str = file->undo.history.strings + step.edit.str_start;
            
            view_do_single_edit(mem, view, file, layout, spec, history_mode);
        
            switch (spec.step.type){
            case ED_NORMAL:
            case ED_REDO:
                view_cursor_move(view, step.post_pos);
                break;
            
            case ED_REVERSE_NORMAL:
            case ED_UNDO:
                view_cursor_move(view, step.pre_pos);
                break;
            }
            view->mark = view->cursor.pos;
        }
        else{
            TentativeAssert(spec.step.special_type == 1);
            view_do_white_batch_edit(mem, view, file, layout, spec, history_mode);
        }
    }
}

// TODO(allen): should these still be view operations?
internal i32
view_find_end_of_line(File_View *view, i32 pos){
#if BUFFER_EXPERIMENT_SCALPEL <= 0
	Editing_File *file = view->file;
	char *data = file->buffer.data;
	while (pos < file->buffer.size && data[pos] != '\n') ++pos;
	if (pos > file->buffer.size) pos = file->buffer.size;
#endif
	return pos;
}

internal i32
view_find_beginning_of_line(File_View *view, i32 pos){
#if BUFFER_EXPERIMENT_SCALPEL <= 0
	Editing_File *file = view->file;
	char *data = file->buffer.data;
	if (pos > 0){
		--pos;
		while (pos > 0 && data[pos] != '\n') --pos;
		if (pos != 0) ++pos;
	}
#endif
	return pos;
}

internal i32
view_find_beginning_of_next_line(File_View *view, i32 pos){
#if BUFFER_EXPERIMENT_SCALPEL <= 0
	Editing_File *file = view->file;
	char *data = file->buffer.data;
	while (pos < file->buffer.size &&
		   !starts_new_line(data[pos])){
		++pos;
	}
	if (pos < file->buffer.size){
		++pos;
	}
#endif
	return pos;
}

internal String*
working_set_next_clipboard_string(General_Memory *general, Working_Set *working, i32 str_size){
	String *result = 0;
	i32 clipboard_current = working->clipboard_current;
	if (working->clipboard_size == 0){
		clipboard_current = 0;
		working->clipboard_size = 1;
	}
	else{
		++clipboard_current;
		if (clipboard_current >= working->clipboard_max_size){
			clipboard_current = 0;
		}
		else if (working->clipboard_size <= clipboard_current){
			working->clipboard_size = clipboard_current+1;
		}
	}
	result = &working->clipboards[clipboard_current];
	working->clipboard_current = clipboard_current;
	working->clipboard_rolling = clipboard_current;
    char *new_str;
	if (result->str){
        new_str = (char*)general_memory_reallocate(general, result->str, result->size, str_size);
    }
    else{
        new_str = (char*)general_memory_allocate(general, str_size+1);
    }
    // TODO(allen): What if new_str == 0?
    *result = make_string(new_str, 0, str_size);
	return result;
}

internal String*
working_set_clipboard_head(Working_Set *working){
	String *result = 0;
	if (working->clipboard_size > 0){
		i32 clipboard_index = working->clipboard_current;
		working->clipboard_rolling = clipboard_index;
		result = &working->clipboards[clipboard_index];
	}
	return result;
}

internal String*
working_set_clipboard_roll_down(Working_Set *working){
	String *result = 0;
	if (working->clipboard_size > 0){
		i32 clipboard_index = working->clipboard_rolling;
		--clipboard_index;
		if (clipboard_index < 0){
			clipboard_index = working->clipboard_size-1;
		}
		working->clipboard_rolling = clipboard_index;
		result = &working->clipboards[clipboard_index];
	}
	return result;
}

inline Editing_File*
working_set_contains(Working_Set *working, String filename){
    Editing_File *result = 0;
    i32 index;
    if (table_find(&working->table, filename, &index)){
        result = working->files + index;
    }
    return result;
}

// TODO(allen): Find a way to choose an ordering for these so it picks better first options.
internal Editing_File*
working_set_lookup_file(Working_Set *working_set, String string){
	Editing_File *file = working_set_contains(working_set, string);
	
	if (!file){
        i32 file_i;
        i32 end = working_set->file_index_count;
        file = working_set->files;
		for (file_i = 0; file_i < end; ++file_i, ++file){
			if (file->live_name.str && (string.size == 0 || has_substr(file->live_name, string))){
				break;
			}
		}
        if (file_i == end) file = 0;
	}
	
	return file;
}

internal void
clipboard_copy(General_Memory *general, Working_Set *working, Range range, Editing_File *file){
#if BUFFER_EXPERIMENT_SCALPEL <= 1
    i32 size = range.end - range.start;
    String *dest = working_set_next_clipboard_string(general, working, size);
    buffer_stringify(&file->buffer, range.start, range.end, dest->str);
    dest->size = size;
    system_post_clipboard(*dest);
#endif
}

internal Edit_Spec
view_compute_whitespace_edit(Mem_Options *mem, Editing_File *file,
                             Buffer_Edit *edits, char *str_base, i32 str_size,
                             Buffer_Edit *inverse_array, char *inv_str, i32 inv_max,
                             i32 edit_count){
#if BUFFER_EXPERIMENT_SCALPEL <= 1
    General_Memory *general = &mem->general;
    
    i32 inv_str_pos = 0;
    Buffer_Invert_Batch state = {};
    if (buffer_invert_batch(&state, &file->buffer, edits, edit_count,
                            inverse_array, inv_str, &inv_str_pos, inv_max))
        Assert(0);
    
    i32 first_child =
        undo_children_push(general, &file->undo.children,
                           edits, edit_count, (u8*)(str_base), str_size);
    i32 inverse_first_child =
        undo_children_push(general, &file->undo.children,
                           inverse_array, edit_count, (u8*)(inv_str), inv_str_pos);
    
    Edit_Spec spec = {};
    spec.step.type = ED_NORMAL;
    spec.step.first_child = first_child;
    spec.step.inverse_first_child = inverse_first_child;
    spec.step.special_type = 1;
    spec.step.child_count = edit_count;
    spec.step.inverse_child_count = edit_count;
#else
    Edit_Spec spec = {};
#endif
    
    return spec;
}

internal void
view_clean_whitespace(Mem_Options *mem, File_View *view, Editing_Layout *layout){
#if BUFFER_EXPERIMENT_SCALPEL <= 1
    Editing_File *file = view->file;
    Assert(file && !file->is_dummy);
    Partition *part = &mem->part;
    i32 line_count = file->buffer.line_count;
    i32 edit_max = line_count * 2;
    i32 edit_count = 0;
    
    Temp_Memory temp = begin_temp_memory(part);
    Buffer_Edit *edits = push_array(part, Buffer_Edit, edit_max);
    
    char *str_base = (char*)part->base + part->pos;
    i32 str_size = 0;
    for (i32 line_i = 0; line_i < line_count; ++line_i){
        i32 start = file->buffer.line_starts[line_i];
        i32 preferred_indentation;
        bool32 all_whitespace = 0;
        bool32 all_space = 0;
        i32 hard_start =
            buffer_find_hard_start(&file->buffer, start, &all_whitespace, &all_space,
                                   &preferred_indentation, 4);
        
        if (all_whitespace) preferred_indentation = 0;
        
        if ((all_whitespace && hard_start > start) || !all_space){
            Buffer_Edit new_edit;
            new_edit.str_start = str_size;
            str_size += preferred_indentation;
            char *str = push_array(part, char, preferred_indentation);
            for (i32 j = 0; j < preferred_indentation; ++j) str[j] = ' ';
            new_edit.len = preferred_indentation;
            new_edit.start = start;
            new_edit.end = hard_start;
            edits[edit_count++] = new_edit;
        }
        Assert(edit_count <= edit_max);
    }
    
    if (edit_count > 0){
        Assert(buffer_batch_debug_sort_check(edits, edit_count));
    
        // NOTE(allen): computing edit spec, doing batch edit
        Buffer_Edit *inverse_array = push_array(part, Buffer_Edit, edit_count);
        Assert(inverse_array);
    
        char *inv_str = (char*)part->base + part->pos;
        Edit_Spec spec =
            view_compute_whitespace_edit(mem, file, edits, str_base, str_size,
                                         inverse_array, inv_str, part->max - part->pos, edit_count);
    
        view_do_white_batch_edit(mem, view, file, layout, spec, hist_normal);
    }
    
    end_temp_memory(temp);
#endif
}

internal void
view_auto_tab_tokens(Mem_Options *mem, File_View *view, Editing_Layout *layout,
                     i32 start, i32 end, b32 empty_blank_lines){
#if BUFFER_EXPERIMENT_SCALPEL <= 0
    Editing_File *file = view->file;
    Assert(file && !file->is_dummy);
    Partition *part = &mem->part;
    Buffer *buffer = &file->buffer;
    Cpp_Token_Stack tokens = file->token_stack;
    Assert(tokens.tokens);
    
    i32 line_start = buffer_get_line_index(buffer, start);
    i32 line_end = buffer_get_line_index(buffer, end) + 1;
    
    i32 edit_max = (line_end - line_start) * 2;
    i32 edit_count = 0;
    
    i32 indent_mark_count = line_end - line_start;
    
    Temp_Memory temp = begin_temp_memory(part);
    i32 *indent_marks = push_array(part, i32, indent_mark_count);
    {
        i32 current_indent = 0;
        i32 token_i;
        Cpp_Token *token, *self_token;
        
        {
            i32 start_pos = file->buffer.line_starts[line_start];
            Cpp_Get_Token_Result result = cpp_get_token(&tokens, start_pos);
            token_i = result.token_index;
            if (result.in_whitespace) token_i += 1;
            self_token = tokens.tokens + token_i;
        }
        
        i32 line = line_start - 1;
        for (; line >= 0; --line){
            i32 start = file->buffer.line_starts[line];
            b32 all_whitespace = 0;
            b32 all_space = 0;
            buffer_find_hard_start(&file->buffer, start, &all_whitespace, &all_space, &current_indent, 4);
            if (!all_whitespace) break;
        }
        
        if (line < 0){
            token_i = 0;
            token = tokens.tokens + token_i;
        }
        else{
            i32 start_pos = file->buffer.line_starts[line];
            Cpp_Get_Token_Result result = cpp_get_token(&tokens, start_pos);
            token_i = result.token_index;
            if (result.in_whitespace) token_i += 1;
            token = tokens.tokens + token_i;
            
            while (token >= tokens.tokens &&
                   token->flags & CPP_TFLAG_PP_DIRECTIVE ||
                   token->flags & CPP_TFLAG_PP_BODY){
                --token;
            }
            
            if (token < tokens.tokens){
                ++token;
                current_indent = 0;
            }
            else if (token->start < start_pos){
                line = buffer_get_line_index(&file->buffer, token->start);
                i32 start = file->buffer.line_starts[line];
                b32 all_whitespace = 0;
                b32 all_space = 0;
                buffer_find_hard_start(&file->buffer, start, &all_whitespace, &all_space, &current_indent, 4);
                Assert(!all_whitespace);
            }
        }
        
        indent_marks -= line_start;
        i32 line_i = line_start;
        i32 next_line_start = file->buffer.line_starts[line_i];
        switch (token->type){
        case CPP_TOKEN_BRACKET_OPEN: current_indent += 4; break;
        case CPP_TOKEN_PARENTHESE_OPEN: current_indent += 4; break;
        case CPP_TOKEN_BRACE_OPEN: current_indent += 4; break;
        }
        
        Cpp_Token *prev_token = token;
        ++token;
        for (; line_i < line_end; ++token_i, ++token){
            for (; token->start >= next_line_start && line_i < line_end;){
                i32 this_line_start = next_line_start;
                next_line_start = file->buffer.line_starts[line_i+1];
                i32 this_indent;
                if (prev_token && prev_token->type == CPP_TOKEN_COMMENT &&
                    prev_token->start <= this_line_start && prev_token->start + prev_token->size > this_line_start){
                    this_indent = -1;
                }
                else{
                    this_indent = current_indent;
                    if (token->start < next_line_start){
                        if (token->flags & CPP_TFLAG_PP_DIRECTIVE) this_indent = 0;
                        else{
                            switch (token->type){
                            case CPP_TOKEN_BRACKET_CLOSE: this_indent -= 4; break;
                            case CPP_TOKEN_PARENTHESE_CLOSE: this_indent -= 4; break;
                            case CPP_TOKEN_BRACE_CLOSE: this_indent -= 4; break;
                            case CPP_TOKEN_BRACE_OPEN: break;
                            default:
                                if (current_indent > 0 && prev_token){
                                    if (!(prev_token->flags & CPP_TFLAG_PP_BODY ||
                                          prev_token->flags & CPP_TFLAG_PP_DIRECTIVE)){
                                        switch (prev_token->type){
                                        case CPP_TOKEN_BRACKET_OPEN: case CPP_TOKEN_PARENTHESE_OPEN:
                                        case CPP_TOKEN_BRACE_OPEN: case CPP_TOKEN_BRACE_CLOSE:
                                        case CPP_TOKEN_SEMICOLON: case CPP_TOKEN_COLON: break;
                                        case CPP_TOKEN_COMMA: case CPP_TOKEN_COMMENT: break;
                                        default: this_indent += 4;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if (this_indent < 0) this_indent = 0;
                }
                indent_marks[line_i] = this_indent;
                ++line_i;
            }
            
            switch (token->type){
            case CPP_TOKEN_BRACKET_OPEN: current_indent += 4; break;
            case CPP_TOKEN_BRACKET_CLOSE: current_indent -= 4; break;
            case CPP_TOKEN_PARENTHESE_OPEN: current_indent += 4; break;
            case CPP_TOKEN_PARENTHESE_CLOSE: current_indent -= 4; break;
            case CPP_TOKEN_BRACE_OPEN: current_indent += 4; break;
            case CPP_TOKEN_BRACE_CLOSE: current_indent -= 4; break;
            }
            prev_token = token;
        }
    }
    
    Buffer_Edit *edits = push_array(part, Buffer_Edit, edit_max);
    
    char *str_base = (char*)part->base + part->pos;
    i32 str_size = 0;
    for (i32 line_i = line_start; line_i < line_end; ++line_i){
        i32 start = file->buffer.line_starts[line_i];
        i32 preferred_indentation;
        i32 correct_indentation;
        bool32 all_whitespace = 0;
        bool32 all_space = 0;
        i32 hard_start =
            buffer_find_hard_start(&file->buffer, start, &all_whitespace, &all_space,
                                   &preferred_indentation, 4);

        correct_indentation = indent_marks[line_i];
        if (all_whitespace && empty_blank_lines) correct_indentation = 0;
        if (correct_indentation == -1) correct_indentation = preferred_indentation;
        
        if ((all_whitespace && hard_start > start) || !all_space || correct_indentation != preferred_indentation){
            Buffer_Edit new_edit;
            new_edit.str_start = str_size;
            str_size += correct_indentation;
            char *str = push_array(part, char, correct_indentation);
            for (i32 j = 0; j < correct_indentation; ++j) str[j] = ' ';
            new_edit.len = correct_indentation;
            new_edit.start = start;
            new_edit.end = hard_start;
            edits[edit_count++] = new_edit;
        }
        Assert(edit_count <= edit_max);
    }
    
    if (edit_count > 0){
        Assert(buffer_batch_debug_sort_check(edits, edit_count));
    
        // NOTE(allen): computing edit spec, doing batch edit
        Buffer_Edit *inverse_array = push_array(part, Buffer_Edit, edit_count);
        Assert(inverse_array);
    
        char *inv_str = (char*)part->base + part->pos;
        Edit_Spec spec =
            view_compute_whitespace_edit(mem, file, edits, str_base, str_size,
                                         inverse_array, inv_str, part->max - part->pos, edit_count);
        
        view_do_white_batch_edit(mem, view, file, layout, spec, hist_normal);
    }
    
    end_temp_memory(temp);
#endif
}

internal u32*
style_get_color(Style *style, Cpp_Token token){
	u32 *result;
    if (token.flags & CPP_TFLAG_IS_KEYWORD){
        if (token.type == CPP_TOKEN_BOOLEAN_CONSTANT){
			result = &style->main.bool_constant_color;
        }
        else{
            result = &style->main.keyword_color;
        }
    }
    else if(token.flags & CPP_TFLAG_PP_DIRECTIVE){
        result = &style->main.preproc_color;
    }
    else{
        switch (token.type){
		case CPP_TOKEN_COMMENT:
			result = &style->main.comment_color;
			break;
            
		case CPP_TOKEN_STRING_CONSTANT:
			result = &style->main.str_constant_color;
            break;
            
		case CPP_TOKEN_CHARACTER_CONSTANT:
			result = &style->main.char_constant_color;
            break;
            
		case CPP_TOKEN_INTEGER_CONSTANT:
			result = &style->main.int_constant_color;
            break;
            
		case CPP_TOKEN_FLOATING_CONSTANT:
			result = &style->main.float_constant_color;
            break;
            
		case CPP_TOKEN_INCLUDE_FILE:
			result = &style->main.include_color;
            break;
            
		default:
			result = &style->main.default_color;
        }
    }
	return result;
}

internal bool32
smooth_camera_step(real32 *target, real32 *current, real32 *vel, real32 S, real32 T){
    bool32 result = 0;
    real32 targ = *target;
    real32 curr = *current;
    real32 v = *vel;
    if (curr != targ){
        if (curr > targ - .1f && curr < targ + .1f){
            curr = targ;
            v = 1.f;
        }
        else{
            real32 L = lerp(curr, T, targ);
            
            i32 sign = (targ > curr) - (targ < curr);
            real32 V = curr + sign*v;
            
            if (sign > 0) curr = Min(L, V);
            else curr = Max(L, V);
            
            if (curr == V){
                v *= S;
            }
        }
        
        *target = targ;
        *current = curr;
        *vel = v;
        result = 1;
    }
    return result;
}

inline real32
view_compute_max_target_y(i32 lowest_line, i32 line_height, real32 view_height){
    real32 max_target_y = ((lowest_line+.5f)*line_height) - view_height*.5f;
    return max_target_y;
}

internal real32
view_compute_max_target_y(File_View *view){
    i32 lowest_line = view_compute_lowest_line(view);
    i32 line_height = view->style->font->height;
    real32 view_height = view_compute_height(view);
    real32 max_target_y = view_compute_max_target_y(
        lowest_line, line_height, view_height);
    return max_target_y;
}

internal void
remeasure_file_view(View *view_, i32_Rect rect){
    File_View *view = (File_View*)view_;
    Relative_Scrolling relative = view_get_relative_scrolling(view);
    view_measure_wraps(&view->view_base.mem->general, view);
    view_cursor_move(view, view->cursor.pos);
    view->preferred_x = view_get_cursor_x(view);
    view_set_relative_scrolling(view, relative);
}

internal bool32
do_button(i32 id, UI_State *state, UI_Layout *layout, char *text, i32 height_mult,
          bool32 is_toggle = 0, bool32 on = 0){
    bool32 result = 0;
    Font *font = state->font;
    i32 character_h = font->height;

    i32_Rect btn_rect = layout_rect(layout, character_h * height_mult);
    if (height_mult > 1) btn_rect = get_inner_rect(btn_rect, 2);
    else{
        btn_rect.x0 += 2;
        btn_rect.x1 -= 2;
    }
    
    Widget_ID wid = make_id(state, id);
    
    if (state->input_stage){
        if (ui_do_button_input(state, btn_rect, wid, 0)){
            result = 1;
        }
    }
    else{
        Render_Target *target = state->target;
        UI_Style ui_style = get_ui_style(state->style);
        u32 back, fore, outline;
        outline = ui_style.bright;
        get_colors(state, &back, &fore, wid, ui_style);
        
        draw_rectangle(target, btn_rect, back);
        draw_rectangle_outline(target, btn_rect, outline);
        real32 text_width = font_string_width(font, text);
        i32 box_width = btn_rect.x1 - btn_rect.x0;
        i32 box_height = btn_rect.y1 - btn_rect.y0;
        i32 x_pos = TRUNC32(btn_rect.x0 + (box_width - text_width)*.5f);
        draw_string(target, font, text, x_pos, btn_rect.y0 + (box_height - character_h) / 2, fore);
        
        if (is_toggle){
            i32_Rect on_box = get_inner_rect(btn_rect, character_h/2);
            on_box.x1 = on_box.x0 + (on_box.y1 - on_box.y0);
            
            if (on) draw_rectangle(target, on_box, fore);
            else draw_rectangle(target, on_box, back);
            draw_rectangle_outline(target, on_box, fore);
        }
    }
    
    return result;
}

internal bool32
do_undo_slider(Widget_ID wid, UI_State *state, UI_Layout *layout, i32 max, i32 v, Undo_Data *undo, i32 *out){
    bool32 result = 0;
    Font *font = state->font;
    i32 character_h = font->height;
    
    i32_Rect containing_rect = layout_rect(layout, character_h);
    
    i32_Rect click_rect;
    click_rect.x0 = containing_rect.x0 + character_h - 1;
    click_rect.x1 = containing_rect.x1 - character_h + 1;
    click_rect.y0 = containing_rect.y0 + 2;
    click_rect.y1 = containing_rect.y1 - 2;
    
    if (state->input_stage){
        real32 l;
        if (ui_do_slider_input(state, click_rect, wid, (real32)click_rect.x0, (real32)click_rect.x1, &l)){
            real32 v_new = lerp(0.f, l, (real32)max);
            v = ROUND32(v_new);
            result = 1;
            if (out) *out = v;
        }
    }
    else{
        if (max > 0){
            Render_Target *target = state->target;
            UI_Style ui_style = get_ui_style_upper(state->style);
            
            real32 L = unlerp(0.f, (real32)v, (real32)max);
            i32 x = FLOOR32(lerp((real32)click_rect.x0, L, (real32)click_rect.x1));
            
            i32 bar_top = ((click_rect.y0 + click_rect.y1) >> 1) - 1;
            i32 bar_bottom = bar_top + 2;
            
            bool32 show_bar = 1;
            real32 tick_step = (click_rect.x1 - click_rect.x0) / (real32)max;
            bool32 show_ticks = 1;
            if (tick_step <= 5.f) show_ticks = 0;
            
            if (undo == 0){
                if (show_bar){
                    i32_Rect slider_rect;
                    slider_rect.x0 = click_rect.x0;
                    slider_rect.x1 = x;
                    slider_rect.y0 = bar_top;
                    slider_rect.y1 = bar_bottom;
                    
                    draw_rectangle(target, slider_rect, ui_style.dim);
                    
                    slider_rect.x0 = x;
                    slider_rect.x1 = click_rect.x1;
                    draw_rectangle(target, slider_rect, ui_style.pop1);
                }
                
                if (show_ticks){
                    real32_Rect tick;
                    tick.x0 = (real32)click_rect.x0 - 1;
                    tick.x1 = (real32)click_rect.x0 + 1;
                    tick.y0 = (real32)bar_top - 3;
                    tick.y1 = (real32)bar_bottom + 3;
                    
                    for (i32 i = 0; i < v; ++i){
                        draw_rectangle(target, tick, ui_style.dim);
                        tick.x0 += tick_step;
                        tick.x1 += tick_step;
                    }
                    
                    for (i32 i = v; i <= max; ++i){
                        draw_rectangle(target, tick, ui_style.pop1);
                        tick.x0 += tick_step;
                        tick.x1 += tick_step;
                    }
                }
            }
            else{
                if (show_bar){
                    i32_Rect slider_rect;
                    slider_rect.x0 = click_rect.x0;
                    slider_rect.y0 = bar_top;
                    slider_rect.y1 = bar_bottom;
                    
                    Edit_Step *history = undo->history.edits;
                    i32 block_count = undo->history_block_count;
                    Edit_Step *step = history;
                    for (i32 i = 0; i < block_count; ++i){
                        u32 color;
                        if (step->type == ED_REDO ||
                            step->type == ED_UNDO) color = ui_style.pop1;
                        else color = ui_style.dim;
                        
                        real32 L;
                        if (i + 1 == block_count){
                            L = 1.f;
                        }else{
                            step = history + step->next_block;
                            L = unlerp(0.f, (real32)(step - history), (real32)max);
                        }
                        if (L > 1.f) L = 1.f;
                        i32 x = FLOOR32(lerp((real32)click_rect.x0, L, (real32)click_rect.x1));
                        
                        slider_rect.x1 = x;
                        draw_rectangle(target, slider_rect, color);
                        slider_rect.x0 = slider_rect.x1;
                        
                        if (L == 1.f) break;
                    }
                }
                
                if (show_ticks){
                    real32_Rect tick;
                    tick.x0 = (real32)click_rect.x0 - 1;
                    tick.x1 = (real32)click_rect.x0 + 1;
                    tick.y0 = (real32)bar_top - 3;
                    tick.y1 = (real32)bar_bottom + 3;

                    Edit_Step *history = undo->history.edits;
                    u32 color = ui_style.dim;
                    for (i32 i = 0; i <= max; ++i){
                        if (i != max){
                            if (history[i].type == ED_REDO) color = ui_style.pop1;
                            else if (history[i].type == ED_UNDO ||
                                     history[i].type == ED_NORMAL) color = ui_style.pop2;
                            else color = ui_style.dim;
                        }
                        draw_rectangle(target, tick, color);
                        tick.x0 += tick_step;
                        tick.x1 += tick_step;
                    }
                }
            }
            
            i32_Rect slider_handle;
            slider_handle.x0 = x - 2;
            slider_handle.x1 = x + 2;
            slider_handle.y0 = click_rect.y0;
            slider_handle.y1 = click_rect.y1;
            
            draw_rectangle(target, slider_handle, ui_style.bright);
        }
    }
    
    return result;
}

internal i32
step_file_view(Thread_Context *thread, View *view_, i32_Rect rect,
               bool32 is_active, Input_Summary *user_input){
    view_->mouse_cursor_type = APP_MOUSE_CURSOR_IBEAM;
    i32 result = 0;
    File_View *view = (File_View*)view_;
    Editing_File *file = view->file;
    Style *style = view->style;
    Font *font = style->font;
    
    real32 line_height = (real32)font->height;
    real32 cursor_y = view_get_cursor_y(view);
    real32 target_y = view->target_y;
    real32 max_y = view_compute_height(view) - line_height*2;
    i32 lowest_line = view_compute_lowest_line(view);
    real32 max_target_y = view_compute_max_target_y(lowest_line, font->height, max_y);
    real32 delta_y = 3.f*line_height;
    real32 extra_top = 0.f;
    extra_top += view_widget_height(view, font);
    real32 taken_top_space = line_height + extra_top;
    
    if (user_input->mouse.my < rect.y0 + taken_top_space){
        view_->mouse_cursor_type = APP_MOUSE_CURSOR_ARROW;
    }
    else{
        view_->mouse_cursor_type = APP_MOUSE_CURSOR_IBEAM;
    }
    
    if (user_input->mouse.wheel_used){
        real32 wheel_multiplier = 3.f;
        real32 delta_target_y = delta_y*user_input->mouse.wheel_amount*wheel_multiplier;
        target_y += delta_target_y;
        
        if (target_y < -taken_top_space) target_y = -taken_top_space;
        if (target_y > max_target_y) target_y = max_target_y;
        
        real32 old_cursor_y = cursor_y;
        if (cursor_y >= target_y + max_y) cursor_y = target_y + max_y;
        if (cursor_y < target_y + taken_top_space) cursor_y = target_y + taken_top_space;
        
        if (cursor_y != old_cursor_y){
            view->cursor =
                view_compute_cursor_from_xy(view,
                                            view->preferred_x,
                                            cursor_y);
        }
        
        result = 1;
    }
    
    while (cursor_y > target_y + max_y) target_y += delta_y;
    while (cursor_y < target_y + taken_top_space) target_y -= delta_y;
    
    if (target_y > max_target_y) target_y = max_target_y;
    if (target_y < -extra_top) target_y = -extra_top;
    view->target_y = target_y;
    
    real32 cursor_x = view_get_cursor_x(view);
    real32 target_x = view->target_x;
    real32 max_x = view_compute_width(view);
    if (cursor_x < target_x){
        target_x = (real32)Max(0, cursor_x - max_x/2);
    }
    else if (cursor_x >= target_x + max_x){
        target_x = (real32)(cursor_x - max_x/2);
    }
    
    view->target_x = target_x;
    
    if (smooth_camera_step(&view->target_y, &view->scroll_y, &view->vel_y, 40.f, 1.f/4.f)){
        result = 1;
    }
    if (smooth_camera_step(&view->target_x, &view->scroll_x, &view->vel_x, 40.f, 1.f/4.f)){
        result = 1;
    }
    if (view->paste_effect.tick_down > 0){
        --view->paste_effect.tick_down;
        result = 1;
    }
    
    if (is_active && user_input->mouse.press_l){
        real32 max_y = view_compute_height(view);
        real32 rx = (real32)(user_input->mouse.mx - rect.x0);
        real32 ry = (real32)(user_input->mouse.my - rect.y0 - line_height - 2);
        
        if (ry >= extra_top){
            view_set_widget(view, FWIDG_NONE);
            if (rx >= 0 && rx < max_x && ry >= 0 && ry < max_y){
                view_cursor_move(view, rx + view->scroll_x, ry + view->scroll_y, 1);
                view->mode = {};
            }
        }
        result = 1;
    }
    
    if (!is_active) view_set_widget(view, FWIDG_NONE);

    // NOTE(allen): framely undo stuff
    {
        i32 scrub_max = view->scrub_max;
        i32 undo_count, redo_count, total_count;
        undo_count = file->undo.undo.edit_count;
        redo_count = file->undo.redo.edit_count;
        total_count = undo_count + redo_count;

        switch (view->widget.type){
        case FWIDG_TIMELINES:
        {
            i32_Rect widg_rect = view_widget_rect(view, font);
            
            UI_State state = 
                ui_state_init(&view->widget.state, 0, user_input,
                              view->style, 0, 1);
            
            UI_Layout layout;
            begin_layout(&layout, widg_rect);
            
            if (view->widget.timeline.undo_line){
                if (do_button(1, &state, &layout, "- Undo", 1)){
                    view->widget.timeline.undo_line = 0;
                }
                
                if (view->widget.timeline.undo_line){
                    Widget_ID wid = make_id(&state, 2);
                    i32 new_count;
                    if (do_undo_slider(wid, &state, &layout, total_count, undo_count, 0, &new_count)){
                        view->rewind_speed = 0;
                        view->rewind_amount = 0;
                        
                        for (i32 i = 0; i < scrub_max && new_count < undo_count; ++i){
                            view_undo(view_->mem, view->layout, view);
                            --undo_count;
                        }
                        for (i32 i = 0; i < scrub_max && new_count > undo_count; ++i){
                            view_redo(view_->mem, view->layout, view);
                            ++undo_count;
                        }
                    }
                }
            }
            else{
                if (do_button(1, &state, &layout, "+ Undo", 1)){
                    view->widget.timeline.undo_line = 1;
                }
            }
            
            if (view->widget.timeline.history_line){
                if (do_button(3, &state, &layout, "- History", 1)){
                    view->widget.timeline.history_line = 0;
                }
                
                Widget_ID wid = make_id(&state, 4);
                if (view->widget.timeline.history_line){
                    i32 new_count;
                    i32 mid = ((file->undo.history.edit_count + file->undo.edit_history_cursor) >> 1);
                    i32 count = file->undo.edit_history_cursor;
                    if (do_undo_slider(wid, &state, &layout, mid, count, &file->undo, &new_count)){
                        for (i32 i = 0; i < scrub_max && new_count < count; ++i){
                            view_history_step(view_->mem, view->layout, view, hist_backward);
                        }
                        for (i32 i = 0; i < scrub_max && new_count > count; ++i){
                            view_history_step(view_->mem, view->layout, view, hist_forward);
                        }
                    }
                }
            }
            else{
                if (do_button(3, &state, &layout, "+ History", 1)){
                    view->widget.timeline.history_line = 1;
                }
            }
            
            view->widget.height = layout.y - widg_rect.y0;
            
            if (ui_finish_frame(&view->widget.state, &state, &layout, widg_rect, 0, 0)){
                result = 1;
            }
        }break;
        }
        
        i32 rewind_max = view->rewind_max;
        view->rewind_amount += view->rewind_speed;
        for (i32 i = 0; view->rewind_amount <= -1.f; ++i, view->rewind_amount += 1.f){
            if (i < rewind_max) view_undo(view_->mem, view->layout, view);
        }
        
        for (i32 i = 0; view->rewind_amount >= 1.f; ++i, view->rewind_amount -= 1.f){
            if (i < rewind_max) view_redo(view_->mem, view->layout, view);
        }
        
        if (view->rewind_speed < 0.f && undo_count == 0){
            view->rewind_speed = 0.f;
            view->rewind_amount = 0.f;
        }
        
        if (view->rewind_speed > 0.f && redo_count == 0){
            view->rewind_speed = 0.f;
            view->rewind_amount = 0.f;
        }
    }
    
    return result;
}

enum File_Sync_State{
    SYNC_GOOD,
    SYNC_BEHIND_OS,
    SYNC_UNSAVED
};

inline File_Sync_State
buffer_get_sync(Editing_File *file){
    File_Sync_State result = SYNC_GOOD;
    if (file->last_4ed_write_time != file->last_sys_write_time)
        result = SYNC_BEHIND_OS;
    else if (file->last_4ed_edit_time > file->last_sys_write_time)
        result = SYNC_UNSAVED;
    return result;
}

internal i32
draw_file_view(Thread_Context *thread, View *view_, i32_Rect rect, bool32 is_active,
               Render_Target *target){
    File_View *view = (File_View*)view_;
    Editing_File *file = view->file;
    Style *style = view->style;
    Font *font = style->font;
    
    Interactive_Bar bar;
    bar.style = style->main.file_info_style;
    bar.font = style->font;
    bar.pos_x = (real32)rect.x0;
    bar.pos_y = (real32)rect.y0;
    bar.text_shift_y = 2;
    bar.text_shift_x = 0;
    bar.rect = rect;
    bar.rect.y1 = bar.rect.y0 + font->height + 2;
    rect.y0 += font->height + 2;

#if BUFFER_EXPERIMENT_SCALPEL <= 1        
    i32 max_x = rect.x1 - rect.x0;
    i32 max_y = rect.y1 - rect.y0 + font->height;
    
    Assert(file && buffer_good(&file->buffer) && !file->is_dummy);

    Opaque_Font_Advance opad = get_opaque_font_advance(font);
    b32 tokens_use = file->tokens_complete && (file->token_stack.count > 0);
    Cpp_Token_Stack token_stack = file->token_stack;

    Partition *part = &view_->mem->part;
    Temp_Memory temp = begin_temp_memory(part);
    
    partition_align(part, 4);
    i32 max = partition_remaining(part) / sizeof(Buffer_Render_Item);
    Buffer_Render_Item *items = push_array(part, Buffer_Render_Item, max);

    i32 count;
    buffer_get_render_data(&file->buffer, view->line_wrap_y, items, max, &count,
                           (real32)rect.x0, (real32)rect.y0, view->scroll_x, view->scroll_y, !view->unwrapped_lines,
                           (real32)max_x, (real32)max_y, opad.data, opad.stride, (real32)font->height);
    Assert(count > 0);
    
    i32 cursor_begin, cursor_end;
    u32 cursor_color, at_cursor_color;
    if (view->show_temp_highlight){
        cursor_begin = view->temp_highlight.pos;
        cursor_end = view->temp_highlight_end_pos;
        cursor_color = style->main.highlight_color;
        at_cursor_color = style->main.at_highlight_color;
    }
    else{
        cursor_begin = view->cursor.pos;
        cursor_end = cursor_begin + 1;
        cursor_color = style->main.cursor_color;
        at_cursor_color = style->main.at_cursor_color;
    }
    
    i32 token_i = 0;
    u32 main_color = style->main.default_color;
    if (tokens_use){
        Cpp_Get_Token_Result result = cpp_get_token(&token_stack, items->index);
        main_color = *style_get_color(style, token_stack.tokens[result.token_index]);
        token_i = result.token_index + 1;
    }
    
    u32 mark_color = style->main.mark_color;
    Buffer_Render_Item *item = items;
    i32 prev_ind = -1;
    u32 highlight_color = 0; AllowLocal(highlight_color);
    for (i32 i = 0; i < count; ++i, ++item){
        i32 ind = item->index;
        if (tokens_use && ind != prev_ind){
            Cpp_Token current_token = token_stack.tokens[token_i-1];
            
            if (token_i < token_stack.count){
                if (ind >= token_stack.tokens[token_i].start){
                    main_color =
                        *style_get_color(style, token_stack.tokens[token_i]);
                    current_token = token_stack.tokens[token_i];
                    ++token_i;
                }
                else if (ind >= current_token.start + current_token.size){
                    main_color = 0xFFFFFFFF;
                }
            }

            if (current_token.type == CPP_TOKEN_JUNK &&
                i >= current_token.start && i <= current_token.start + current_token.size){
                highlight_color = style->main.highlight_junk_color;
            }
        }
        u32 char_color = main_color;
        
        if (cursor_begin <= ind && ind < cursor_end && (ind != prev_ind || cursor_begin < ind)){
            if (is_active) draw_rectangle(target, f32R(item->x0, item->y0, item->x1, item->y1), cursor_color);
            else draw_rectangle_outline(target, f32R(item->x0, item->y0, item->x1, item->y1), cursor_color);
            char_color = at_cursor_color;
        }
        
        u32 fade_color = 0xFFFF00FF;
        f32 fade_amount = 0.f;
        
        if (view->paste_effect.tick_down > 0 &&
            view->paste_effect.start <= i && i < view->paste_effect.end){
            fade_color = view->paste_effect.color;
            fade_amount = (real32)(view->paste_effect.tick_down) / view->paste_effect.tick_max;
        }
        
        char_color = color_blend(char_color, fade_amount, fade_color);
        
        if (ind == view->mark && prev_ind != ind){
            draw_rectangle_outline(target, f32R(item->x0, item->y0, item->x1, item->y1), mark_color);
        }
        if (item->glyphid != 0){
            font_draw_glyph(target, font, (u16)item->glyphid,
                            item->x0, item->y0, char_color);
        }
        prev_ind = ind;
    }
    
    end_temp_memory(temp);
#endif
    
    if (view->widget.type != FWIDG_NONE){
        UI_Style ui_style = get_ui_style_upper(style);
        
        Font *font = style->font;
        i32_Rect widg_rect = view_widget_rect(view, font);
        
        draw_rectangle(target, widg_rect, ui_style.dark);
        draw_rectangle_outline(target, widg_rect, ui_style.dim);
        
        UI_State state =
            ui_state_init(&view->widget.state, target, 0,
                          view->style, 0, 0);
        
        UI_Layout layout;
        begin_layout(&layout, widg_rect);
        
        switch (view->widget.type){
        case FWIDG_TIMELINES:
        {
            if (view->widget.timeline.undo_line){
                do_button(1, &state, &layout, "- Undo", 1);
                
                Widget_ID wid = make_id(&state, 2);
                i32 undo_count, redo_count, total_count;
                undo_count = file->undo.undo.edit_count;
                redo_count = file->undo.redo.edit_count;
                total_count = undo_count + redo_count;
                do_undo_slider(wid, &state, &layout, total_count, undo_count, 0, 0);
            }
            else{
                do_button(1, &state, &layout, "+ Undo", 1);
            }
            
            if (view->widget.timeline.history_line){
                do_button(3, &state, &layout, "- History", 1);
                
                Widget_ID wid = make_id(&state, 4);
                i32 new_count;
                i32 mid = ((file->undo.history.edit_count + file->undo.edit_history_cursor) >> 1);
                do_undo_slider(wid, &state, &layout, mid, file->undo.edit_history_cursor, &file->undo, &new_count);
            }
            else{
                do_button(3, &state, &layout, "+ History", 1);
            }
        }break;
        
        case FWIDG_SEARCH:
        {
            Widget_ID wid = make_id(&state, 1);
            persist String search_str = make_lit_string("I-Search: ");
            persist String rsearch_str = make_lit_string("Reverse-I-Search: ");
            if (view->isearch.reverse){
                do_text_field(wid, &state, &layout, rsearch_str, view->isearch.str);
            }
            else{
                do_text_field(wid, &state, &layout, search_str, view->isearch.str);
            }
        }break;
        
        case FWIDG_GOTO_LINE:
        {
            Widget_ID wid = make_id(&state, 1);
            persist String gotoline_str = make_lit_string("Goto Line: ");
            do_text_field(wid, &state, &layout, gotoline_str, view->isearch.str);
        }break;
        }
        
        ui_finish_frame(&view->widget.state, &state, &layout, widg_rect, 0, 0);
    }
    
    {
        u32 back_color = bar.style.bar_color;
        draw_rectangle(target, bar.rect, back_color);
        
        u32 base_color = bar.style.base_color;
        intbar_draw_string(target, &bar, file->live_name, base_color);
        intbar_draw_string(target, &bar, make_lit_string(" - "), base_color);
        
        char line_number_space[30];
        String line_number = make_string(line_number_space, 0, 30);
        append(&line_number, "L#");
        append_int_to_str(view->cursor.line, &line_number);
        
        intbar_draw_string(target, &bar, line_number, base_color);

        switch (buffer_get_sync(file)){
        case SYNC_BEHIND_OS:
        {
            persist String out_of_sync = make_lit_string(" BEHIND OS");
            intbar_draw_string(target, &bar, out_of_sync, bar.style.pop2_color);
        }break;
        
        case SYNC_UNSAVED:
        {
            persist String out_of_sync = make_lit_string(" *");
            intbar_draw_string(target, &bar, out_of_sync, bar.style.pop2_color);
        }break;
        }
    }
    
    return 0;
}

internal void
kill_buffer(General_Memory *general, Editing_File *file, Live_Views *live_set, Editing_Layout *layout){
    i32 panel_count = layout->panel_count;
    Panel *panels = layout->panels, *panel;
    panel = panels;
    
    for (i32 i = 0; i < panel_count; ++i){
        View *view = panel->view;
        if (view){
            View *to_kill = view;
            if (view->is_minor) to_kill = view->major;
            File_View *fview = view_to_file_view(to_kill);
            if (fview && fview->file == file){
                live_set_free_view(live_set, &fview->view_base);
                if (to_kill == view) panel->view = 0;
                else view->major = 0;
            }
        }
        ++panel;
    }
    
    file_close(general, file);
    file_get_dummy(file);
}

internal void
command_search(Command_Data*,Command_Binding);
internal void
command_rsearch(Command_Data*,Command_Binding);

internal
HANDLE_COMMAND_SIG(handle_command_file_view){
    File_View *file_view = (File_View*)(view);
    Editing_File *file = file_view->file;
    AllowLocal(file);
            
    switch (file_view->widget.type){
    case FWIDG_NONE:
    case FWIDG_TIMELINES:
    {
        file_view->next_mode = {};
        if (binding.function) binding.function(command, binding);
        file_view->mode = file_view->next_mode;
        
        if (key.key.keycode == codes->esc)
            view_set_widget(file_view, FWIDG_NONE);
    }break;
    
    case FWIDG_SEARCH:
    {
#if BUFFER_EXPERIMENT_SCALPEL <= 1
        String *string = &file_view->isearch.str;
        Single_Line_Input_Step result =
            app_single_line_input_step(codes, key, string);
        
        if (result.made_a_change ||
            binding.function == command_search ||
            binding.function == command_rsearch){
            bool32 step_forward = 0;
            bool32 step_backward = 0;
            
            if (binding.function == command_search) step_forward = 1;
            if (binding.function == command_rsearch) step_backward = 1;
            
            i32 start_pos = file_view->isearch.pos;
            if (step_forward){
                if (file_view->isearch.reverse){
                    start_pos = file_view->temp_highlight.pos + 1;
                    file_view->isearch.pos = start_pos;
                    file_view->isearch.reverse = 0;
                    step_forward = 0;
                }
            }
            if (step_backward){
                if (!file_view->isearch.reverse){
                    start_pos = file_view->temp_highlight.pos - 1;
                    file_view->isearch.pos = start_pos;
                    file_view->isearch.reverse = 1;
                    step_backward = 0;
                }
            }

            Temp_Memory temp = begin_temp_memory(&view->mem->part);
            char *spare = push_array(&view->mem->part, char, string->size);

            i32 size = buffer_size(&file->buffer);
            i32 pos;
            if (!result.hit_backspace){
                if (file_view->isearch.reverse){
                    pos = buffer_rfind_string(&file->buffer, start_pos - 1, string->str, string->size, spare);
                    if (pos >= 0){
                        if (step_backward){
                            file_view->isearch.pos = pos;
                            start_pos = pos;
                            pos = buffer_rfind_string(&file->buffer, start_pos - 1, string->str, string->size, spare);
                            if (pos == -1) pos = start_pos;
                        }
                        view_set_temp_highlight(file_view, pos, pos+string->size);
                    }
                }
                
                else{
                    pos = buffer_find_string(&file->buffer, start_pos + 1, string->str, string->size, spare);
                    if (pos < size){
                        if (step_forward){
                            file_view->isearch.pos = pos;
                            start_pos = pos;
                            pos = buffer_find_string(&file->buffer, start_pos + 1, string->str, string->size, spare);
                            if (pos == size) pos = start_pos;
                        }
                        view_set_temp_highlight(file_view, pos, pos+string->size);
                    }
                }
            }
            
            end_temp_memory(temp);
        }
        
        if (result.hit_newline || result.hit_ctrl_newline){
            view_cursor_move(file_view, file_view->temp_highlight);
            view_set_widget(file_view, FWIDG_NONE);
        }
        
        if (result.hit_esc){
            file_view->show_temp_highlight = 0;
            view_set_widget(file_view, FWIDG_NONE);
        }
#endif
    }break;
    
    case FWIDG_GOTO_LINE:
    {
        String *string = &file_view->gotoline.str;
        Single_Line_Input_Step result =
            app_single_number_input_step(codes, key, string);
        
        if (result.hit_newline || result.hit_ctrl_newline){
            i32 line_number = str_to_int(*string);
            Font *font = file_view->style->font;
            if (line_number < 1) line_number = 1;
            file_view->cursor =
                view_compute_cursor_from_unwrapped_xy(file_view, 0, (real32)(line_number-1)*font->height);
            file_view->preferred_x = view_get_cursor_x(file_view);
            
            view_set_widget(file_view, FWIDG_NONE);
        }
        
        if (result.hit_esc){
            view_set_widget(file_view, FWIDG_NONE);
        }
    }break;
    }
}

inline void
free_file_view(View *view_){
    File_View *view = (File_View*)view_;
    general_memory_free(&view_->mem->general, view->line_wrap_y);
}

internal
DO_VIEW_SIG(do_file_view){
    i32 result = 0;
    switch (message){
    case VMSG_RESIZE:
    case VMSG_STYLE_CHANGE:
    {
        remeasure_file_view(view, rect);
    }break;
    case VMSG_DRAW:
    {
        result = draw_file_view(thread, view, rect, (view == active), target);
    }break;
    case VMSG_STEP:
    {
        result = step_file_view(thread, view, rect, (view == active), user_input);
    }break;
    case VMSG_FREE:
    {
        free_file_view(view);
    }break;
    }
    return result;
}

internal File_View*
file_view_init(View *view, Delay *delay, Editing_Layout *layout){
    view->type = VIEW_TYPE_FILE;
    view->do_view = do_file_view;
    view->handle_command = handle_command_file_view;
    
    File_View *result = (File_View*)view;
    result->delay = delay;
    result->layout = layout;
    result->rewind_max = 4;
    result->scrub_max = 1;
    return result;
}

// BOTTOM

