/*
4coder_metadata_generator.cpp - A preprocessor program for generating a list of commands and their descriptions.
*/

// TOP

#define COMMAND_METADATA_OUT "generated/command_metadata.h"
#define ID_METADATA_OUT "generated/managed_id_metadata.cpp"

#include "4coder_base_types.h"
#include "4coder_token.h"
#include "generated/lexer_cpp.h"

#include "4coder_base_types.cpp"
#include "4coder_stringf.cpp"
#include "4coder_malloc_allocator.cpp"

#include "4coder_token.cpp"
#include "generated/lexer_cpp.cpp"

#include "4coder_file.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef OS_LINUX
    #include <inttypes.h>
    #define FMTi64 PRIi64
#else
    #define FMTi64 "lld"
#endif

///////////////////////////////

struct Line_Column_Coordinates{
    i64 line;
    i64 column;
};

struct Reader{
    Arena *error_arena;
    u8 *source_name;
    String_Const_u8 text;
    Token_Array tokens;
    Token *ptr;
};

struct Temp_Read{
    Reader *reader;
    Token *pos;
};

typedef i32 Meta_Command_Entry_Kind;
enum{
    MetaCommandEntryKind_ERROR,
    MetaCommandEntryKind_Normal,
    MetaCommandEntryKind_UI,
};

struct Meta_Command_Entry{
    Meta_Command_Entry *next;
    Meta_Command_Entry_Kind kind;
    String_Const_u8 name;
    u8 *source_name;
    i64 line_number;
    union{
        struct{
            String_Const_u8 doc;
        } docstring;
    };
};

struct Meta_ID_Entry{
    Meta_ID_Entry *next;
    String_Const_u8 group_name;
    String_Const_u8 id_name;
};

struct Meta_Command_Entry_Arrays{
    Meta_Command_Entry *first_doc_string;
    Meta_Command_Entry *last_doc_string;
    i32 doc_string_count;
    
    Meta_Command_Entry *first_ui;
    Meta_Command_Entry *last_ui;
    i32 ui_count;
    
    Meta_ID_Entry *first_id;
    Meta_ID_Entry *last_id;
    i32 id_count;
};

///////////////////////////////

static Line_Column_Coordinates
line_column_coordinates(String_Const_u8 text, i64 pos){
    if (pos < 0){
        pos = 0;
    }
    if (pos > (i64)text.size){
        pos = (i64)text.size;
    }
    
    Line_Column_Coordinates coords = {};
    coords.line = 1;
    coords.column = 1;
    u8 *end = text.str + pos;
    for (u8 *p = text.str; p < end; ++p){
        if (*p == '\n'){
            ++coords.line;
            coords.column = 1;
        }
        else{
            ++coords.column;
        }
    }
    
    return(coords);
}

static i64
line_number(String_Const_u8 text, i64 pos){
    Line_Column_Coordinates coords = line_column_coordinates(text, pos);
    return(coords.line);
}

static void
error(u8 *source_name, String_Const_u8 text, i64 pos, u8 *msg){
    Line_Column_Coordinates coords = line_column_coordinates(text, pos);
    fprintf(stdout, "%s:%" FMTi64 ":%" FMTi64 ": %s\n",
            source_name, coords.line, coords.column, msg);
    fflush(stdout);
}

///////////////////////////////

static Reader
make_reader(Arena *error_arena, Token_Array array, u8 *source_name, String_Const_u8 text){
    Reader reader = {};
    reader.error_arena = error_arena;
    reader.tokens = array;
    reader.ptr = array.tokens;
    reader.source_name = source_name;
    reader.text = text;
    return(reader);
}

static Token
prev_token(Reader *reader){
    Token result = {};
    
    for (;;){
        if (reader->ptr > reader->tokens.tokens + reader->tokens.count){
            reader->ptr = reader->tokens.tokens + reader->tokens.count;
        }
        
        if (reader->ptr > reader->tokens.tokens){
            --reader->ptr;
            result = *reader->ptr;
        }
        else{
            reader->ptr = reader->tokens.tokens;
            memset(&result, 0, sizeof(result));
            break;
        }
        
        if (result.kind != TokenBaseKind_Comment &&
            result.kind != TokenBaseKind_Whitespace &&
            result.kind != TokenBaseKind_LexError){
            break;
        }
    }
    
    return(result);
}

static Token
get_token(Reader *reader){
    Token result = {};
    
    for (;;){
        if (reader->ptr < reader->tokens.tokens){
            reader->ptr = reader->tokens.tokens;
        }
        
        if (reader->ptr < reader->tokens.tokens + reader->tokens.count){
            result = *reader->ptr;
            ++reader->ptr;
        }
        else{
            reader->ptr = reader->tokens.tokens + reader->tokens.count;
            memset(&result, 0, sizeof(result));
            result.pos = reader->text.size;
            break;
        }
        
        if (result.kind != TokenBaseKind_Comment &&
            result.kind != TokenBaseKind_Whitespace &&
            result.kind != TokenBaseKind_LexError){
            break;
        }
    }
    
    return(result);
}

static Token
peek_token(Reader *reader){
    Token result = {};
    
    if (reader->ptr < reader->tokens.tokens){
        reader->ptr = reader->tokens.tokens;
    }
    
    if (reader->ptr >= reader->tokens.tokens + reader->tokens.count){
        result.pos = reader->text.size;
    }
    else{
        result = *reader->ptr;
    }
    
    return(result);
}

static i64
peek_pos(Reader *reader){
    Token token = peek_token(reader);
    return(token.pos);
}

static i64
line_number(Reader *reader, i64 pos){
    return(line_number(reader->text, pos));
}

static void
error(Reader *reader, i64 pos, u8 *msg){
    error(reader->source_name, reader->text, pos, msg);
}

static Temp_Read
begin_temp_read(Reader *reader){
    Temp_Read temp = {};
    temp.reader = reader;
    temp.pos = reader->ptr;
    return(temp);
}

static void
end_temp_read(Temp_Read temp){
    temp.reader->ptr = temp.pos;
}

///////////////////////////////

static String_Const_u8
token_str(String_Const_u8  text, Token token){
    String_Const_u8 str = string_prefix(string_skip(text, token.pos), token.size);
    return(str);
}

///////////////////////////////

static i32
quick_sort_part(Meta_Command_Entry **entries, i32 first, i32 one_past_last){
    i32 pivot = one_past_last - 1;
    String_Const_u8 pivot_key = entries[pivot]->name;
    i32 j = first;
    for (i32 i = first; i < pivot; ++i){
        if (string_compare(entries[i]->name, pivot_key) < 0){
            Swap(Meta_Command_Entry*, entries[i], entries[j]);
            ++j;
        }
    }
    Swap(Meta_Command_Entry*, entries[j], entries[pivot]);
    return(j);
}

static void
quick_sort(Meta_Command_Entry **entries, i32 first, i32 one_past_last){
    if (first + 1 < one_past_last){
        i32 pivot = quick_sort_part(entries, first, one_past_last);
        quick_sort(entries, first, pivot);
        quick_sort(entries, pivot + 1, one_past_last);
    }
}

static Meta_Command_Entry**
get_sorted_meta_commands(Arena *arena, Meta_Command_Entry *first, i32 count){
    Meta_Command_Entry **entries = push_array(arena, Meta_Command_Entry*, count);
    
    i32 i = 0;
    for (Meta_Command_Entry *entry = first;
         entry != 0;
         entry = entry->next, ++i){
        entries[i] = entry;
    }
    Assert(i == count);
    
    quick_sort(entries, 0, count);
    
    return(entries);
}

///////////////////////////////

static b32
has_duplicate_entry(Meta_Command_Entry *first, String_Const_u8 name){
    b32 has_duplicate = false;
    for (Meta_Command_Entry *entry = first;
         entry != 0;
         entry = entry->next){
        if (string_match(name, entry->name)){
            has_duplicate = true;
        }
    }
    return(has_duplicate);
}

///////////////////////////////

static b32
require_key_identifier(Reader *reader, String_Const_u8 string, i64 *opt_pos_out){
    b32 success = false;
    
    Token token = get_token(reader);
    if (token.kind == TokenBaseKind_Identifier){
        String_Const_u8 lexeme = token_str(reader->text, token);
        if (string_match(lexeme, string)){
            success = true;
            if (opt_pos_out != 0){
                *opt_pos_out = token.pos;
            }
        }
    }
    
    if (!success){
        Temp_Memory temp = begin_temp(reader->error_arena);
        String_Const_u8 error_string = push_u8_stringf(reader->error_arena, "expected to find '%.*s'",
                                                       string.size, string.str);
        error(reader, token.pos, error_string.str);
        end_temp(temp);
    }
    
    return(success);
}

static b32
require_key_identifier(Reader *reader, char *str, i64 *opt_pos_out){
    return(require_key_identifier(reader, SCu8(str), opt_pos_out));
}

static b32
require_key_identifier(Reader *reader, String_Const_u8 string){
    return(require_key_identifier(reader, string, 0));
}

static b32
require_key_identifier(Reader *reader, char *str){
    return(require_key_identifier(reader, SCu8(str), 0));
}

static b32
require_open_parenthese(Reader *reader, i64 *opt_pos_out){
    b32 success = false;
    Token token = get_token(reader);
    if (token.kind == TokenBaseKind_ParentheticalOpen){
        success = true;
        if (opt_pos_out != 0){
            *opt_pos_out = token.pos;
        }
    }
    if (!success){
        error(reader, token.pos, (u8*)"expected to find '('");
    }
    return(success);
}

static b32
require_open_parenthese(Reader *reader){
    return(require_open_parenthese(reader, 0));
}

static b32
require_close_parenthese(Reader *reader, i64 *opt_pos_out){
    b32 success = false;
    Token token = get_token(reader);
    if (token.kind == TokenBaseKind_ParentheticalClose){
        success = true;
        if (opt_pos_out != 0){
            *opt_pos_out = token.pos;
        }
    }
    if (!success){
        error(reader, token.pos, (u8*)"expected to find ')'");
    }
    return(success);
}

static b32
require_close_parenthese(Reader *reader){
    return(require_close_parenthese(reader, 0));
}

static b32
require_comma(Reader *reader, i64 *opt_pos_out){
    b32 success = false;
    
    Token token = get_token(reader);
    if (token.sub_kind == TokenCppKind_Comma){
        success = true;
        if (opt_pos_out != 0){
            *opt_pos_out = token.pos;
        }
    }
    
    if (!success){
        error(reader, token.pos, (u8*)"expected to find ','");
    }
    
    return(success);
}

static b32
require_comma(Reader *reader){
    return(require_comma(reader, 0));
}

static b32
require_define(Reader *reader, i64 *opt_pos_out){
    b32 success = false;
    
    Token token = get_token(reader);
    if (token.sub_kind == TokenCppKind_PPDefine){
        success = true;
        if (opt_pos_out != 0){
            *opt_pos_out = token.pos;
        }
    }
    
    if (!success){
        error(reader, token.pos, (u8*)"expected to find '#define'");
    }
    
    return(success);
}

static b32
require_define(Reader *reader){
    return(require_define(reader, 0));
}

static b32
extract_identifier(Reader *reader, String_Const_u8 *str_out, i64 *opt_pos_out){
    b32 success = false;
    
    Token token = get_token(reader);
    if (token.kind == TokenBaseKind_Identifier){
        String_Const_u8 lexeme = token_str(reader->text, token);
        *str_out = lexeme;
        success = true;
        if (opt_pos_out != 0){
            *opt_pos_out = token.pos;
        }
    }
    
    if (!success){
        error(reader, token.pos, (u8*)"expected to find an identifier");
    }
    
    return(success);
}

static b32
extract_identifier(Reader *reader, String_Const_u8 *str_out){
    return(extract_identifier(reader, str_out, 0));
}

static b32
extract_integer(Reader *reader, String_Const_u8 *str_out, i64 *opt_pos_out){
    b32 success = false;
    
    Token token = get_token(reader);
    if (token.kind == TokenBaseKind_LiteralInteger){
        String_Const_u8 lexeme = token_str(reader->text, token);
        *str_out = lexeme;
        success = true;
        if (opt_pos_out != 0){
            *opt_pos_out = token.pos;
        }
    }
    
    if (!success){
        error(reader, token.pos, (u8*)"expected to find an integer");
    }
    
    return(success);
}


static b32
extract_integer(Reader *reader, String_Const_u8 *str_out){
    return(extract_integer(reader, str_out, 0));
}

static b32
extract_string(Reader *reader, String_Const_u8 *str_out, i64 *opt_pos_out){
    b32 success = false;
    
    Token token = get_token(reader);
    if (token.kind == TokenBaseKind_LiteralString){
        String_Const_u8 lexeme = token_str(reader->text, token);
        *str_out = lexeme;
        success = true;
        if (opt_pos_out != 0){
            *opt_pos_out = token.pos;
        }
    }
    
    if (!success){
        error(reader, token.pos, (u8*)"expected to find a string literal");
    }
    
    return(success);
}

static b32
extract_string(Reader *reader, String_Const_u8 *str_out){
    return(extract_string(reader, str_out, 0));
}

function Meta_Command_Entry_Kind
parse_command_kind(String_Const_u8 kind){
    Meta_Command_Entry_Kind result = MetaCommandEntryKind_ERROR;
    if (string_match(kind, string_u8_litexpr("Normal"))){
        result = MetaCommandEntryKind_Normal;
    }
    else if (string_match(kind, string_u8_litexpr("UI"))){
        result = MetaCommandEntryKind_UI;
    }
    return(result);
}

static b32
parse_documented_command(Arena *arena, Meta_Command_Entry_Arrays *arrays, Reader *reader){
    String_Const_u8 name = {};
    String_Const_u8 kind = {};
    String_Const_u8 file_name = {};
    String_Const_u8 line_number = {};
    String_Const_u8 doc = {};
    
    // Getting the command's name
    i64 start_pos = 0;
    if (!require_key_identifier(reader, "CUSTOM_COMMAND", &start_pos)){
        return(false);
    }
    
    if (!require_open_parenthese(reader)){
        return(false);
    }
    
    if (!extract_identifier(reader, &name)){
        return(false);
    }
    
    if (!require_comma(reader)){
        return(false);
    }
    
    if (!extract_string(reader, &file_name)){
        return(false);
    }
    
    if (!require_comma(reader)){
        return(false);
    }
    
    if (!extract_integer(reader, &line_number)){
        return(false);
    }
    
    if (!require_comma(reader)){
        return(false);
    }
    
    if (!extract_identifier(reader, &kind)){
        return(false);
    }
    
    if (!require_close_parenthese(reader)){
        return(false);
    }
    
    // Getting the command's doc string
    if (!require_key_identifier(reader, "CUSTOM_DOC")){
        return(false);
    }
    
    if (!require_open_parenthese(reader)){
        return(false);
    }
    
    i64 doc_pos = 0;
    if (!extract_string(reader, &doc, &doc_pos)){
        return(false);
    }
    
    if (doc.size < 1 || doc.str[0] != '"'){
        error(reader, doc_pos, (u8*)"warning: doc strings with string literal prefixes not allowed");
        return(false);
    }
    
    if (!require_close_parenthese(reader)){
        return(false);
    }
    
    if (has_duplicate_entry(arrays->first_doc_string, name)){
        error(reader, start_pos, (u8*)"warning: multiple commands with the same name and separate doc strings, skipping this one");
        return(false);
    }
    
    doc = string_chop(string_skip(doc, 1), 1);
    
    String_Const_u8 file_name_unquoted = string_chop(string_skip(file_name, 1), 1);
    String_Const_u8 source_name = string_interpret_escapes(arena, file_name_unquoted);
    
    Meta_Command_Entry *new_entry = push_array(arena, Meta_Command_Entry, 1);
    new_entry->kind = parse_command_kind(kind);
    new_entry->name = name;
    new_entry->source_name = source_name.str;
    new_entry->line_number = (i32)string_to_integer(line_number, 10);
    new_entry->docstring.doc = doc;
    sll_queue_push(arrays->first_doc_string, arrays->last_doc_string, new_entry);
    arrays->doc_string_count += 1;
    
    return(true);
}

static b32
parse_custom_id(Arena *arena, Meta_Command_Entry_Arrays *arrays, Reader *reader){
    String_Const_u8 group = {};
    String_Const_u8 id = {};
    
    i64 start_pos = 0;
    if (!require_key_identifier(reader, "CUSTOM_ID", &start_pos)){
        return(false);
    }
    
    if (!require_open_parenthese(reader)){
        return(false);
    }
    
    if (!extract_identifier(reader, &group)){
        return(false);
    }
    
    if (!require_comma(reader)){
        return(false);
    }
    
    if (!extract_identifier(reader, &id)){
        return(false);
    }
    
    if (!require_close_parenthese(reader)){
        return(false);
    }
    
    Meta_ID_Entry *new_id = push_array(arena, Meta_ID_Entry, 1);
    sll_queue_push(arrays->first_id, arrays->last_id, new_id);
    new_id->group_name = group;
    new_id->id_name = id;
    arrays->id_count += 1;
    
    return(true);
    }

///////////////////////////////

static void
parse_text(Arena *arena, Meta_Command_Entry_Arrays *entry_arrays, u8 *source_name, String_Const_u8 text){
    Token_List token_list = lex_full_input_cpp(arena, text);
    Token_Array array = token_array_from_list(arena, &token_list);
    
    Reader reader_ = make_reader(arena, array, source_name, text);
    Reader *reader = &reader_;
    
    for (;;){
        Token token = get_token(reader);
        
        if (token.kind == TokenBaseKind_Identifier){
            if (!HasFlag(token.flags, TokenBaseFlag_PreprocessorBody)){
                String_Const_u8 lexeme = token_str(text, token);
                if (string_match(lexeme, string_u8_litexpr("CUSTOM_DOC"))){
                    Temp_Read temp_read = begin_temp_read(reader);
                
                b32 found_start_pos = false;
                for (i32 R = 0; R < 12; ++R){
                    Token p_token = prev_token(reader);
                    if (p_token.kind == TokenBaseKind_Identifier){
                        String_Const_u8 p_lexeme = token_str(text, p_token);
                        if (string_match(p_lexeme, string_u8_litexpr("CUSTOM_COMMAND"))){
                            found_start_pos = true;
                            break;
                        }
                    }
                    if (p_token.kind == TokenBaseKind_EOF){
                        break;
                    }
                }
                
                if (!found_start_pos){
                    end_temp_read(temp_read);
                }
                else{
                    if (!parse_documented_command(arena, entry_arrays, reader)){
                        end_temp_read(temp_read);
                    }
                }
                }
                else if (string_match(lexeme, string_u8_litexpr("CUSTOM_ID"))){
                    Temp_Read temp_read = begin_temp_read(reader);
                    prev_token(reader);
                    if (!parse_custom_id(arena, entry_arrays, reader)){
                        end_temp_read(temp_read);
                    }
                }
            }
        }
        
        if (token.kind == TokenBaseKind_EOF){
            break;
        }
    }
}

static void
parse_file(Arena *arena, Meta_Command_Entry_Arrays *entry_arrays, Filename_Character *name_, i32 len){
    char *name = unencode(arena, name_, len);
    if (name == 0){
        if (sizeof(*name_) == 2){
            fprintf(stdout, "warning: could not unencode file name %ls - file skipped\n", (wchar_t*)name_);
        }
        else{
            fprintf(stdout, "warning: could not unencode file name %s - file skipped\n", (char*)name_);
        }
        return;
    }
    
    String_Const_u8 text = file_dump(arena, name);
    parse_text(arena, entry_arrays, (u8*)name, text);
}

static void
parse_files_by_pattern(Arena *arena, Meta_Command_Entry_Arrays *entry_arrays, Filename_Character *pattern, b32 recursive){
    Cross_Platform_File_List list = get_file_list(arena, pattern, filter_all);
    for (i32 i = 0; i < list.count; ++i){
        Cross_Platform_File_Info *info = &list.info[i];
        
        String_Const_Any info_name = SCany(info->name, info->len);
        Temp_Memory temp = begin_temp(arena);
        String_Const_u8 info_name_ascii = string_u8_from_any(arena, info_name);
        b32 is_generated = string_match(info_name_ascii, string_u8_litexpr("4coder_generated"));
        end_temp(temp);
        
        if (info->is_folder && is_generated){
            continue;
        }
        if (!recursive && info->is_folder){
            continue;
        }
        
        i32 full_name_len = list.path_length + 1 + info->len;
        if (info->is_folder){
            full_name_len += 2;
        }
        Filename_Character *full_name = push_array(arena, Filename_Character, full_name_len + 1);
        
        if (full_name == 0){
            fprintf(stdout, "fatal error: not enough memory to recurse to sub directory\n");
            exit(1);
        }
        
        memmove(full_name, list.path_name, list.path_length*sizeof(*full_name));
        full_name[list.path_length] = SLASH;
        memmove(full_name + list.path_length + 1, info->name, info->len*sizeof(*full_name));
        full_name[full_name_len] = 0;
        
        if (!info->is_folder){
            parse_file(arena, entry_arrays, full_name, full_name_len);
        }
        else{
            full_name[full_name_len - 2] = SLASH;
            full_name[full_name_len - 1] = '*';
            parse_files_by_pattern(arena, entry_arrays, full_name, true);
        }
    }
}

static void
show_usage(int argc, char **argv){
    char *name = "metadata_generator";
    if (argc >= 1){
        name = argv[0];
    }
    fprintf(stdout, "usage:\n%s [-R] <4coder-root-directory> <input-file-pattern> [<input-file-pattern> ...]\n", name);
    exit(0);
}

int
main(int argc, char **argv){
    if (argc < 3){
        show_usage(argc, argv);
    }
    
    b32 recursive = string_match(SCu8(argv[1]), string_u8_litexpr("-R"));
    if (recursive && argc < 4){
        show_usage(argc, argv);
    }
    
    Arena arena_ = make_arena_malloc(MB(1), 8);
    Arena *arena = &arena_;
    
    String_Const_u8 out_directory = SCu8(argv[2]);
    
    i32 start_i = 2;
    if (recursive){
        start_i = 3;
    }
    
    printf("metadata_generator ");
    for (i32 i = start_i; i < argc; i += 1){
        printf("%s ", argv[i]);
    }
    printf("\n");
    fflush(stdout);
    
    Meta_Command_Entry_Arrays entry_arrays = {};
    for (i32 i = start_i; i < argc; ++i){
        Filename_Character *pattern_name = encode(arena, argv[i]);
        parse_files_by_pattern(arena, &entry_arrays, pattern_name, recursive);
    }
    
    if (out_directory.size > 2 &&
        out_directory.str[0] == '"' &&
        out_directory.str[out_directory.size - 1] == '"'){
        out_directory.str += 1;
        out_directory.size -= 2;
    }
    
    out_directory = string_skip_chop_whitespace(out_directory);
    
    String_Const_u8 cmd_out_name = push_u8_stringf(arena, "%.*s/%s",
                                                        string_expand(out_directory),
                                                        COMMAND_METADATA_OUT);
    FILE *cmd_out = fopen((char*)cmd_out_name.str, "wb");
    
    if (cmd_out != 0){
        i32 entry_count = entry_arrays.doc_string_count;
        Meta_Command_Entry **entries = get_sorted_meta_commands(arena, entry_arrays.first_doc_string, entry_count);
        
        fprintf(cmd_out, "#if !defined(META_PASS)\n");
        fprintf(cmd_out, "#define command_id(c) (fcoder_metacmd_ID_##c)\n");
        fprintf(cmd_out, "#define command_metadata(c) (&fcoder_metacmd_table[command_id(c)])\n");
        fprintf(cmd_out, "#define command_metadata_by_id(id) (&fcoder_metacmd_table[id])\n");
        fprintf(cmd_out, "#define command_one_past_last_id %d\n", entry_arrays.doc_string_count);
        fprintf(cmd_out, "#if defined(CUSTOM_COMMAND_SIG)\n");
        fprintf(cmd_out, "#define PROC_LINKS(x,y) x\n");
        fprintf(cmd_out, "#else\n");
        fprintf(cmd_out, "#define PROC_LINKS(x,y) y\n");
        fprintf(cmd_out, "#endif\n");
        
        fprintf(cmd_out, "#if defined(CUSTOM_COMMAND_SIG)\n");
        for (i32 i = 0; i < entry_count; ++i){
            Meta_Command_Entry *entry = entries[i];
            fprintf(cmd_out, "CUSTOM_COMMAND_SIG(%.*s);\n", string_expand(entry->name));
        }
        fprintf(cmd_out, "#endif\n");
        
        fprintf(cmd_out,
                "struct Command_Metadata{\n"
                "PROC_LINKS(Custom_Command_Function, void) *proc;\n"
                "b32 is_ui;\n"
                "char *name;\n"
                "i32 name_len;\n"
                "char *description;\n"
                "i32 description_len;\n"
                "char *source_name;\n"
                "i32 source_name_len;\n"
                "i32 line_number;\n"
                "};\n");
        
        fprintf(cmd_out,
                "static Command_Metadata fcoder_metacmd_table[%d] = {\n",
                entry_arrays.doc_string_count);
        for (i32 i = 0; i < entry_count; ++i){
            Meta_Command_Entry *entry = entries[i];
            
            Temp_Memory temp = begin_temp(arena);
            
            String_Const_u8 source_name = SCu8(entry->source_name);
            String_Const_u8 printable = string_replace(arena, source_name,
                                                       SCu8("\\"), SCu8("\\\\"),
                                                       StringFill_NullTerminate);
            
            char *is_ui = "false";
            if (entry->kind == MetaCommandEntryKind_UI){
                is_ui = "true";
            }
            
            fprintf(cmd_out,
                    "{ PROC_LINKS(%.*s, 0), %s, \"%.*s\", %d, "
                    "\"%.*s\", %d, \"%s\", %d, %" FMTi64 " },\n",
                    string_expand(entry->name),
                    is_ui,
                    string_expand(entry->name),
                    (i32)entry->name.size,
                    string_expand(entry->docstring.doc),
                    (i32)entry->docstring.doc.size,
                    printable.str,
                    (i32)source_name.size,
                    entry->line_number);
            end_temp(temp);
        }
        fprintf(cmd_out, "};\n");
        
        i32 id = 0;
        for (i32 i = 0; i < entry_count; ++i){
            Meta_Command_Entry *entry = entries[i];
            fprintf(cmd_out, "static i32 fcoder_metacmd_ID_%.*s = %d;\n", string_expand(entry->name), id);
            ++id;
        }
        
        fprintf(cmd_out, "#endif\n");
        
        fclose(cmd_out);
    }
    else{
        fprintf(stdout, "fatal error: could not open output file %.*s\n", string_expand(cmd_out_name));
    }
    
    String_Const_u8 id_out_name = push_u8_stringf(arena, "%.*s/%s",
                                                  string_expand(out_directory),
                                                  ID_METADATA_OUT);
    FILE *id_out = fopen((char*)id_out_name.str, "wb");
    
    if (id_out != 0){
        fprintf(id_out, "function void\n");
        fprintf(id_out, "initialize_managed_id_metadata(Application_Links *app){\n");
        
        for (Meta_ID_Entry *node = entry_arrays.first_id;
             node != 0;
             node = node->next){
            fprintf(id_out, "%.*s = managed_id_declare(app, string_u8_litexpr(\"%.*s\"), string_u8_litexpr(\"%.*s\"));\n",
                    string_expand(node->id_name),
                    string_expand(node->group_name),
                    string_expand(node->id_name));
        }
        
        fprintf(id_out, "}\n");
        
        fclose(id_out);
    }
    else{
        fprintf(stdout, "fatal error: could not open output file %.*s\n", string_expand(id_out_name));
    }
    
    return(0);
}

// BOTTOM

