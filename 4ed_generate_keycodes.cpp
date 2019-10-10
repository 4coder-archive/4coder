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

struct Key{
    Key *next;
    String_Const_u8 name;
};

struct Key_List{
    Key *first;
    Key *last;
    i32 count;
};

////////////////////////////////

function void
generate_keycodes(Arena *scratch, Key_List *list, FILE *out){
    fprintf(out, "enum{\n");
    i32 counter = 1;
    for (Key *key = list->first;
         key != 0;
         key = key->next){
        fprintf(out, "KeyCode_%.*s = %d,\n", string_expand(key->name), counter);
        counter += 1;
    }
    fprintf(out, "KeyCode_COUNT = %d,\n", counter);
    fprintf(out, "};\n");
    
    fprintf(out, "global char* key_code_name[KeyCode_COUNT] = {\n");
    fprintf(out, "\"None\"");
    for (Key *key = list->first;
         key != 0;
         key = key->next){
        fprintf(out, "\"%.*s\",\n", string_expand(key->name));
        counter += 1;
    }
    fprintf(out, "};\n");
}

////////////////////////////////

function Key*
add_key(Arena *arena, Key_List *list, String_Const_u8 name){
    Key *key = push_array(arena, Key, 1);
    sll_queue_push(list->first, list->last, key);
    list->count;
    key->name = push_string_copy(arena, name);
    return(key);
}

function Key_List
make_key_list(Arena *arena){
    Key_List list = {};
    for (u32 i = 'A'; i <= 'Z'; i += 1){
        u8 c = (u8)i;
        add_key(arena, &list, SCu8(&c, 1));
    }
    for (u32 i = '0'; i <= '9'; i += 1){
        u8 c = (u8)i;
        add_key(arena, &list, SCu8(&c, 1));
    }
    add_key(arena, &list, string_u8_litexpr("Space"));
    add_key(arena, &list, string_u8_litexpr("Tick"));
    add_key(arena, &list, string_u8_litexpr("Minus"));
    add_key(arena, &list, string_u8_litexpr("Equal"));
    add_key(arena, &list, string_u8_litexpr("LeftBracket"));
    add_key(arena, &list, string_u8_litexpr("RightBracket"));
    add_key(arena, &list, string_u8_litexpr("Semicolon"));
    add_key(arena, &list, string_u8_litexpr("Quote"));
    add_key(arena, &list, string_u8_litexpr("Comma"));
    add_key(arena, &list, string_u8_litexpr("Period"));
    add_key(arena, &list, string_u8_litexpr("ForwardSlash"));
    add_key(arena, &list, string_u8_litexpr("BackwardSlash"));
    add_key(arena, &list, string_u8_litexpr("Tab"));
    add_key(arena, &list, string_u8_litexpr("Escape"));
    add_key(arena, &list, string_u8_litexpr("Pause"));
    add_key(arena, &list, string_u8_litexpr("Up"));
    add_key(arena, &list, string_u8_litexpr("Down"));
    add_key(arena, &list, string_u8_litexpr("Left"));
    add_key(arena, &list, string_u8_litexpr("Right"));
    add_key(arena, &list, string_u8_litexpr("Backspace"));
    add_key(arena, &list, string_u8_litexpr("Return"));
    add_key(arena, &list, string_u8_litexpr("Delete"));
    add_key(arena, &list, string_u8_litexpr("Insert"));
    add_key(arena, &list, string_u8_litexpr("Home"));
    add_key(arena, &list, string_u8_litexpr("End"));
    add_key(arena, &list, string_u8_litexpr("PageUp"));
    add_key(arena, &list, string_u8_litexpr("PageDown"));
    add_key(arena, &list, string_u8_litexpr("CapsLock"));
    add_key(arena, &list, string_u8_litexpr("NumLock"));
    add_key(arena, &list, string_u8_litexpr("ScrollLock"));
    add_key(arena, &list, string_u8_litexpr("Menu"));
    add_key(arena, &list, string_u8_litexpr("Shift"));
    add_key(arena, &list, string_u8_litexpr("Control"));
    add_key(arena, &list, string_u8_litexpr("Alt"));
    add_key(arena, &list, string_u8_litexpr("Command"));
    for (u32 i = 1; i <= 16; i += 1){
        add_key(arena, &list, push_u8_stringf(arena, "F%d", i));
    }
    return(list);
}

////////////////////////////////

int
main(void){
    Arena arena = make_arena_malloc();
    
    Key_List key_list = make_key_list(&arena);
    
    String_Const_u8 path_to_self = string_u8_litexpr(__FILE__);
    path_to_self = string_remove_last_folder(path_to_self);
    String_Const_u8 file_name =
    push_u8_stringf(&arena, "%.*scustom/generated/4coder_keycodes.h",
                    string_expand(path_to_self));
    
    FILE *out = fopen((char*)file_name.str, "wb");
    if (out == 0){
        printf("could not open output file '%s'\n", file_name.str);
        exit(1);
    }
    
    generate_keycodes(&arena, &key_list, out);
    fclose(out);
    return(0);
}

// BOTTOM
