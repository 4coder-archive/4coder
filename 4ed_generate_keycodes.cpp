/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 09.10.2019
 *
 * Primary list for all key codes.
 *
 */

// TOP

#include "4coder_base_types.h"

#include "4coder_base_types.cpp"
#include "4coder_stringf.cpp"
#include "4coder_malloc_allocator.cpp"

#include <stdio.h>

////////////////////////////////

struct Event_Code{
    Event_Code *next;
    String_Const_u8 name;
};

struct Event_Code_List{
    Event_Code *first;
    Event_Code *last;
    i32 count;
    
    String_Const_u8 code_prefix;
    String_Const_u8 name_table;
};

////////////////////////////////

function void
generate_codes(Arena *scratch, Event_Code_List *list, FILE *out){
    String_Const_u8 code_prefix = list->code_prefix;
    String_Const_u8 name_table = list->name_table;
    
    fprintf(out, "enum{\n");
    i32 counter = 1;
    for (Event_Code *code = list->first;
         code != 0;
         code = code->next){
        fprintf(out, "%.*s_%.*s = %d,\n",
                string_expand(code_prefix), string_expand(code->name), counter);
        counter += 1;
    }
    fprintf(out, "%.*s_COUNT = %d,\n", string_expand(code_prefix), counter);
    fprintf(out, "};\n");
    
    fprintf(out, "global char* %.*s[%.*s_COUNT] = {\n",
            string_expand(name_table), string_expand(code_prefix));
    fprintf(out, "\"None\",\n");
    for (Event_Code *code = list->first;
         code != 0;
         code = code->next){
        fprintf(out, "\"%.*s\",\n", string_expand(code->name));
        counter += 1;
    }
    fprintf(out, "};\n");
}

////////////////////////////////

function Event_Code*
add_code(Arena *arena, Event_Code_List *list, String_Const_u8 name){
    Event_Code *code = push_array(arena, Event_Code, 1);
    sll_queue_push(list->first, list->last, code);
    list->count;
    code->name = push_string_copy(arena, name);
    return(code);
}

function Event_Code_List
make_key_list(Arena *arena){
    Event_Code_List list = {};
    list.code_prefix = string_u8_litexpr("KeyCode");
    list.name_table = string_u8_litexpr("key_code_name");
    for (u32 i = 'A'; i <= 'Z'; i += 1){
        u8 c = (u8)i;
        add_code(arena, &list, SCu8(&c, 1));
    }
    for (u32 i = '0'; i <= '9'; i += 1){
        u8 c = (u8)i;
        add_code(arena, &list, SCu8(&c, 1));
    }
    add_code(arena, &list, string_u8_litexpr("Space"));
    add_code(arena, &list, string_u8_litexpr("Tick"));
    add_code(arena, &list, string_u8_litexpr("Minus"));
    add_code(arena, &list, string_u8_litexpr("Equal"));
    add_code(arena, &list, string_u8_litexpr("LeftBracket"));
    add_code(arena, &list, string_u8_litexpr("RightBracket"));
    add_code(arena, &list, string_u8_litexpr("Semicolon"));
    add_code(arena, &list, string_u8_litexpr("Quote"));
    add_code(arena, &list, string_u8_litexpr("Comma"));
    add_code(arena, &list, string_u8_litexpr("Period"));
    add_code(arena, &list, string_u8_litexpr("ForwardSlash"));
    add_code(arena, &list, string_u8_litexpr("BackwardSlash"));
    add_code(arena, &list, string_u8_litexpr("Tab"));
    add_code(arena, &list, string_u8_litexpr("Escape"));
    add_code(arena, &list, string_u8_litexpr("Pause"));
    add_code(arena, &list, string_u8_litexpr("Up"));
    add_code(arena, &list, string_u8_litexpr("Down"));
    add_code(arena, &list, string_u8_litexpr("Left"));
    add_code(arena, &list, string_u8_litexpr("Right"));
    add_code(arena, &list, string_u8_litexpr("Backspace"));
    add_code(arena, &list, string_u8_litexpr("Return"));
    add_code(arena, &list, string_u8_litexpr("Delete"));
    add_code(arena, &list, string_u8_litexpr("Insert"));
    add_code(arena, &list, string_u8_litexpr("Home"));
    add_code(arena, &list, string_u8_litexpr("End"));
    add_code(arena, &list, string_u8_litexpr("PageUp"));
    add_code(arena, &list, string_u8_litexpr("PageDown"));
    add_code(arena, &list, string_u8_litexpr("CapsLock"));
    add_code(arena, &list, string_u8_litexpr("NumLock"));
    add_code(arena, &list, string_u8_litexpr("ScrollLock"));
    add_code(arena, &list, string_u8_litexpr("Menu"));
    add_code(arena, &list, string_u8_litexpr("Shift"));
    add_code(arena, &list, string_u8_litexpr("Control"));
    add_code(arena, &list, string_u8_litexpr("Alt"));
    add_code(arena, &list, string_u8_litexpr("Command"));
    for (u32 i = 1; i <= 24; i += 1){
        add_code(arena, &list, push_u8_stringf(arena, "F%d", i));
    }
    for (u32 i = '0'; i <= '9'; i += 1){
        add_code(arena, &list, push_u8_stringf(arena, "NumPad%c", i));
    }
    add_code(arena, &list, string_u8_litexpr("NumPadStar"));
    add_code(arena, &list, string_u8_litexpr("NumPadPlus"));
    add_code(arena, &list, string_u8_litexpr("NumPadMinus"));
    add_code(arena, &list, string_u8_litexpr("NumPadDot"));
    add_code(arena, &list, string_u8_litexpr("NumPadSlash"));
    for (i32 i = 0; i < 30; i += 1){
        add_code(arena, &list, push_u8_stringf(arena, "Ex%d", i));
    }
    return(list);
}

function Event_Code_List
make_mouse_list(Arena *arena){
    Event_Code_List list = {};
    list.code_prefix = string_u8_litexpr("MouseCode");
    list.name_table = string_u8_litexpr("mouse_code_name");
    add_code(arena, &list, string_u8_litexpr("Left"));
    add_code(arena, &list, string_u8_litexpr("Middle"));
    add_code(arena, &list, string_u8_litexpr("Right"));
    return(list);
}

function Event_Code_List
make_core_list(Arena *arena){
    Event_Code_List list = {};
    list.code_prefix = string_u8_litexpr("CoreCode");
    list.name_table = string_u8_litexpr("core_code_name");
    add_code(arena, &list, string_u8_litexpr("Startup"));
    add_code(arena, &list, string_u8_litexpr("Animate"));
    add_code(arena, &list, string_u8_litexpr("ClickActivateView"));
    add_code(arena, &list, string_u8_litexpr("ClickDeactivateView"));
    add_code(arena, &list, string_u8_litexpr("TryExit"));
    add_code(arena, &list, string_u8_litexpr("FileExternallyModified"));
    add_code(arena, &list, string_u8_litexpr("NewClipboardContents"));
    return(list);
}

////////////////////////////////

int
main(void){
    Arena arena = make_arena_malloc();
    
    Event_Code_List key_list = make_key_list(&arena);
    Event_Code_List mouse_list = make_mouse_list(&arena);
    Event_Code_List core_list = make_core_list(&arena);
    
    String_Const_u8 path_to_self = string_u8_litexpr(__FILE__);
    path_to_self = string_remove_last_folder(path_to_self);
    String_Const_u8 file_name =
        push_u8_stringf(&arena, "%.*scustom/generated/4coder_event_codes.h",
                        string_expand(path_to_self));
    
    FILE *out = fopen((char*)file_name.str, "wb");
    if (out == 0){
        printf("could not open output file '%s'\n", file_name.str);
        exit(1);
    }
    
    generate_codes(&arena, &key_list, out);
    generate_codes(&arena, &mouse_list, out);
    generate_codes(&arena, &core_list, out);
    
    fclose(out);
    return(0);
}

// BOTTOM
