/*
4coder_metadata_generator.cpp - A preprocessor program for generating a list of commands and their descriptions.
*/

// TOP

#define COMMAND_METADATA_OUT "4coder_generated/command_metadata.h"

#include "4coder_file.h"

#include "4coder_lib/4cpp_lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define str_to_l_c(s) ((s).size), ((s).str)
#define str_to_c_l(s) ((s).str), ((s).size)

///////////////////////////////

struct Line_Column_Coordinates{
    int32_t line;
    int32_t column;
};

static Line_Column_Coordinates
line_column_coordinates(String text, int32_t pos){
    if (pos < 0){
        pos = 0;
    }
    if (pos > text.size){
        pos = text.size;
    }
    
    Line_Column_Coordinates coords = {0};
    coords.line = 1;
    coords.column = 1;
    char *end = text.str + pos;
    for (char *p = text.str; p < end; ++p){
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

static int32_t
line_number(String text, int32_t pos){
    Line_Column_Coordinates coords = line_column_coordinates(text, pos);
    return(coords.line);
}

static void
error(char *source_name, String text, int32_t pos, char *msg){
    Line_Column_Coordinates coords = line_column_coordinates(text, pos);
    fprintf(stdout, "%s:%d:%d: %s\n", source_name, coords.line, coords.column, msg);
    fflush(stdout);
}

///////////////////////////////

struct Reader{
    char *source_name;
    String text;
    Cpp_Token_Array tokens;
    Cpp_Token *ptr;
};

struct Temp_Read{
    Reader *reader;
    Cpp_Token *pos;
};

static Reader
make_reader(Cpp_Token_Array array, char *source_name, String text){
    Reader reader = {0};
    reader.tokens = array;
    reader.ptr = array.tokens;
    reader.source_name = source_name;
    reader.text = text;
    return(reader);
}

static Cpp_Token
prev_token(Reader *reader){
    Cpp_Token result = {0};
    
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
        
        if (result.type != CPP_TOKEN_COMMENT && result.type != CPP_TOKEN_JUNK){
            break;
        }
    }
    
    return(result);
}

static Cpp_Token
get_token(Reader *reader){
    Cpp_Token result = {0};
    
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
            result.start = reader->text.size;
            break;
        }
        
        if (result.type != CPP_TOKEN_COMMENT && result.type != CPP_TOKEN_JUNK){
            break;
        }
    }
    
    return(result);
}

static Cpp_Token
peek_token(Reader *reader){
    Cpp_Token result = {0};
    
    if (reader->ptr < reader->tokens.tokens){
        reader->ptr = reader->tokens.tokens;
    }
    
    if (reader->ptr >= reader->tokens.tokens + reader->tokens.count){
        result.start = reader->text.size;
    }
    else{
        result = *reader->ptr;
    }
    
    return(result);
}

static int32_t
peek_pos(Reader *reader){
    Cpp_Token token = peek_token(reader);
    return(token.start);
}

static int32_t
line_number(Reader *reader, int32_t pos){
    int32_t result = line_number(reader->text, pos);
    return(result);
}

static void
error(Reader *reader, int32_t pos, char *msg){
    error(reader->source_name, reader->text, pos, msg);
}

static Temp_Read
begin_temp_read(Reader *reader){
    Temp_Read temp = {0};
    temp.reader = reader;
    temp.pos = reader->ptr;
    return(temp);
}

static void
end_temp_read(Temp_Read temp){
    temp.reader->ptr = temp.pos;
}

///////////////////////////////

static String
token_str(String text, Cpp_Token token){
    String str = substr(text, token.start, token.size);
    return(str);
}

///////////////////////////////

#define sll_push(f,l,n) if((f)==0){(f)=(l)=(n);}else{(l)->next=(n);(l)=(n);}(n)->next=0

///////////////////////////////

typedef uint32_t Meta_Command_Entry_Type;
enum{
    MetaCommandEntry_DocString,
    MetaCommandEntry_Alias,
};

struct Meta_Command_Entry{
    Meta_Command_Entry *next;
    String name;
    char *source_name;
    int32_t line_number;
    union{
        struct{
            String doc;
        } docstring;
        struct{
            String potential;
        } alias;
    };
};

struct Meta_Command_Entry_Arrays{
    Meta_Command_Entry *first_doc_string;
    Meta_Command_Entry *last_doc_string;
    int32_t doc_string_count;
    
    Meta_Command_Entry *first_alias;
    Meta_Command_Entry *last_alias;
    int32_t alias_count;
};

///////////////////////////////

#define Swap(T,a,b) {T t=a; a=b; b=t;}

static int32_t
quick_sort_part(Meta_Command_Entry **entries, int32_t first, int32_t one_past_last){
    int32_t pivot = one_past_last - 1;
    String pivot_key = entries[pivot]->name;
    int32_t j = first;
    for (int32_t i = first; i < pivot; ++i){
        if (compare(entries[i]->name, pivot_key) < 0){
            Swap(Meta_Command_Entry*, entries[i], entries[j]);
            ++j;
        }
    }
    Swap(Meta_Command_Entry*, entries[j], entries[pivot]);
    return(j);
}

static void
quick_sort(Meta_Command_Entry **entries, int32_t first, int32_t one_past_last){
    if (first + 1 < one_past_last){
        int32_t pivot = quick_sort_part(entries, first, one_past_last);
        quick_sort(entries, first, pivot);
        quick_sort(entries, pivot + 1, one_past_last);
    }
}

static Meta_Command_Entry**
get_sorted_meta_commands(Partition *part, Meta_Command_Entry *first, int32_t count){
    Meta_Command_Entry **entries = push_array(part, Meta_Command_Entry*, count);
    
    int32_t i = 0;
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

static bool32
has_duplicate_entry(Meta_Command_Entry *first, String name){
    bool32 has_duplicate = false;
    for (Meta_Command_Entry *entry = first;
         entry != 0;
         entry = entry->next){
        if (match(name, entry->name)){
            has_duplicate = true;
        }
    }
    return(has_duplicate);
}

///////////////////////////////

static bool32
require_key_identifier(Reader *reader, char *str, int32_t *opt_pos_out = 0){
    bool32 success = false;
    
    Cpp_Token token = get_token(reader);
    if (token.type == CPP_TOKEN_IDENTIFIER){
        String lexeme = token_str(reader->text, token);
        if (match(lexeme, str)){
            success = true;
            if (opt_pos_out != 0){
                *opt_pos_out = token.start;
            }
        }
    }
    
    if (!success){
        char space[1024];
        String s = make_fixed_width_string(space);
        copy(&s, "expected to find '");
        append(&s, str);
        append(&s, "'");
        terminate_with_null(&s);
        error(reader, token.start, s.str);
    }
    
    return(success);
}

static bool32
require_open_parenthese(Reader *reader, int32_t *opt_pos_out = 0){
    bool32 success = false;
    
    Cpp_Token token = get_token(reader);
    if (token.type == CPP_TOKEN_PARENTHESE_OPEN){
        success = true;
        if (opt_pos_out != 0){
            *opt_pos_out = token.start;
        }
    }
    
    if (!success){
        error(reader, token.start, "expected to find '('");
    }
    
    return(success);
}

static bool32
require_close_parenthese(Reader *reader, int32_t *opt_pos_out = 0){
    bool32 success = false;
    
    Cpp_Token token = get_token(reader);
    if (token.type == CPP_TOKEN_PARENTHESE_CLOSE){
        success = true;
        if (opt_pos_out != 0){
            *opt_pos_out = token.start;
        }
    }
    
    if (!success){
        error(reader, token.start, "expected to find ')'");
    }
    
    return(success);
}

static bool32
require_comma(Reader *reader, int32_t *opt_pos_out = 0){
    bool32 success = false;
    
    Cpp_Token token = get_token(reader);
    if (token.type == CPP_TOKEN_COMMA){
        success = true;
        if (opt_pos_out != 0){
            *opt_pos_out = token.start;
        }
    }
    
    if (!success){
        error(reader, token.start, "expected to find ','");
    }
    
    return(success);
}

static bool32
require_define(Reader *reader, int32_t *opt_pos_out = 0){
    bool32 success = false;
    
    Cpp_Token token = get_token(reader);
    if (token.type == CPP_PP_DEFINE){
        success = true;
        if (opt_pos_out != 0){
            *opt_pos_out = token.start;
        }
    }
    
    if (!success){
        error(reader, token.start, "expected to find '#define'");
    }
    
    return(success);
}

static bool32
extract_identifier(Reader *reader, String *str_out, int32_t *opt_pos_out = 0){
    bool32 success = false;
    
    Cpp_Token token = get_token(reader);
    if (token.type == CPP_TOKEN_IDENTIFIER){
        String lexeme = token_str(reader->text, token);
        *str_out = lexeme;
        success = true;
        if (opt_pos_out != 0){
            *opt_pos_out = token.start;
        }
    }
    
    if (!success){
        error(reader, token.start, "expected to find an identifier");
    }
    
    return(success);
}

static bool32
extract_integer(Reader *reader, String *str_out, int32_t *opt_pos_out = 0){
    bool32 success = false;
    
    Cpp_Token token = get_token(reader);
    if (token.type == CPP_TOKEN_INTEGER_CONSTANT){
        String lexeme = token_str(reader->text, token);
        *str_out = lexeme;
        success = true;
        if (opt_pos_out != 0){
            *opt_pos_out = token.start;
        }
    }
    
    if (!success){
        error(reader, token.start, "expected to find an integer");
    }
    
    return(success);
}

static bool32
extract_string(Reader *reader, String *str_out, int32_t *opt_pos_out = 0){
    bool32 success = false;
    
    Cpp_Token token = get_token(reader);
    if (token.type == CPP_TOKEN_STRING_CONSTANT){
        String lexeme = token_str(reader->text, token);
        *str_out = lexeme;
        success = true;
        if (opt_pos_out != 0){
            *opt_pos_out = token.start;
        }
    }
    
    if (!success){
        error(reader, token.start, "expected to find a string literal");
    }
    
    return(success);
}

static bool32
parse_documented_command(Partition *part, Meta_Command_Entry_Arrays *arrays, Reader *reader){
    String name = {0};
    String file_name = {0};
    String line_number = {0};
    String doc = {0};
    
    // Getting the command's name
    int32_t start_pos = 0;
    if (!require_key_identifier(reader, "CUSTOM_COMMAND_SIG", &start_pos)){
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
    
    int32_t doc_pos = 0;
    if (!extract_string(reader, &doc, &doc_pos)){
        return(false);
    }
    
    if (doc.size < 1 || doc.str[0] != '"'){
        error(reader, doc_pos, "warning: doc strings with string literal prefixes not allowed");
        return(false);
    }
    
    if (!require_close_parenthese(reader)){
        return(false);
    }
    
    if (has_duplicate_entry(arrays->first_doc_string, name)){
        error(reader, start_pos, "warning: multiple commands with the same name and separate doc strings, skipping this one");
        return(false);
    }
    
    doc = substr(doc, 1, doc.size - 2);
    
    char *source_name = push_array(part, char, file_name.size + 1);
    push_align(part, 8);
    string_interpret_escapes(substr(file_name, 1, file_name.size - 2), source_name);
    
    Meta_Command_Entry *new_entry = push_array(part, Meta_Command_Entry, 1);
    new_entry->name = name;
    new_entry->source_name = source_name;
    new_entry->line_number = str_to_int(line_number);
    new_entry->docstring.doc = doc;
    sll_push(arrays->first_doc_string, arrays->last_doc_string, new_entry);
    ++arrays->doc_string_count;
    
    return(true);
}

static bool32
parse_alias(Partition *part, Meta_Command_Entry_Arrays *arrays, Reader *reader){
    String name = {0};
    String potential = {0};
    
    // Getting the alias's name
    int32_t start_pos = 0;
    if (!require_define(reader, &start_pos)){
        return(false);
    }
    
    if (!extract_identifier(reader, &name)){
        return(false);
    }
    
    // Getting the alias's target
    if (!require_key_identifier(reader, "CUSTOM_ALIAS")){
        return(false);
    }
    
    if (!require_open_parenthese(reader)){
        return(false);
    }
    
    if (!extract_identifier(reader, &potential)){
        return(false);
    }
    
    if (!require_close_parenthese(reader)){
        return(false);
    }
    
    Meta_Command_Entry *new_entry = push_array(part, Meta_Command_Entry, 1);
    new_entry->name = name;
    new_entry->source_name = reader->source_name;
    new_entry->line_number = line_number(reader, start_pos);
    new_entry->alias.potential = potential;
    sll_push(arrays->first_alias, arrays->last_alias, new_entry);
    ++arrays->alias_count;
    
    return(true);
}

///////////////////////////////

static void
parse_text(Partition *part, Meta_Command_Entry_Arrays *entry_arrays, char *source_name, String text){
    Cpp_Token_Array array = cpp_make_token_array(1024);
    cpp_lex_file(text.str, text.size, &array);
    
    Reader reader_ = make_reader(array, source_name, text);
    Reader *reader = &reader_;
    
    for (;;){
        Cpp_Token token = get_token(reader);
        
        if (token.type == CPP_TOKEN_IDENTIFIER){
            String lexeme = token_str(text, token);
            
            bool32 in_preproc_body = ((token.flags & CPP_TFLAG_PP_BODY) != 0);
            
            if (!in_preproc_body && match(lexeme, "CUSTOM_DOC")){
                Temp_Read temp_read = begin_temp_read(reader);
                
                bool32 found_start_pos = false;
                for (int32_t R = 0; R < 10; ++R){
                    Cpp_Token p_token = prev_token(reader);
                    if (p_token.type == CPP_TOKEN_IDENTIFIER){
                        String p_lexeme = token_str(text, p_token);
                        if (match(p_lexeme, "CUSTOM_COMMAND_SIG")){
                            found_start_pos = true;
                            break;
                        }
                    }
                    if (p_token.type == 0){
                        break;
                    }
                }
                
                if (!found_start_pos){
                    end_temp_read(temp_read);
                }
                else{
                    if (!parse_documented_command(part, entry_arrays, reader)){
                        end_temp_read(temp_read);
                    }
                }
            }
            else if (match(lexeme, "CUSTOM_ALIAS")){
                Temp_Read temp_read = begin_temp_read(reader);
                
                bool32 found_start_pos = false;
                for (int32_t R = 0; R < 3; ++R){
                    Cpp_Token p_token = prev_token(reader);
                    if (p_token.type == CPP_PP_DEFINE){
                        if (R == 2){
                            found_start_pos = true;
                        }
                        break;
                    }
                    if (p_token.type == 0){
                        break;
                    }
                }
                
                if (!found_start_pos){
                    end_temp_read(temp_read);
                }
                else{
                    if (!parse_alias(part, entry_arrays, reader)){
                        end_temp_read(temp_read);
                    }
                }
            }
        }
        
        if (token.type == 0){
            break;
        }
    }
    
    cpp_free_token_array(array);
}

static void
parse_file(Partition *part, Meta_Command_Entry_Arrays *entry_arrays, Filename_Character *name_, int32_t len){
    char *name = unencode(part, name_, len);
    if (name == 0){
        if (sizeof(*name_) == 2){
            fprintf(stdout, "warning: could not unencode file name %ls - file skipped\n", (wchar_t*)name_);
        }
        else{
            fprintf(stdout, "warning: could not unencode file name %s - file skipped\n", (char*)name_);
        }
        return;
    }
    
    String text = file_dump(part, name);
    parse_text(part, entry_arrays, name, text);
}

static void
parse_files_by_pattern(Partition *part, Meta_Command_Entry_Arrays *entry_arrays, Filename_Character *pattern, bool32 recursive){
    Cross_Platform_File_List list = get_file_list(part, pattern, filter_all);
    for (int32_t i = 0; i < list.count; ++i){
        Cross_Platform_File_Info *info = &list.info[i];
        
        if (info->is_folder && match(make_string(info->name, info->len), "4coder_generated")){
            continue;
        }
        if (!recursive && info->is_folder){
            continue;
        }
        
        int32_t full_name_len = list.path_length + 1 + info->len;
        if (info->is_folder){
            full_name_len += 2;
        }
        Filename_Character *full_name = push_array(part, Filename_Character, full_name_len + 1);
        push_align(part, 8);
        
        if (full_name == 0){
            fprintf(stdout, "fatal error: not enough memory to recurse to sub directory\n");
            exit(1);
        }
        
        memmove(full_name, list.path_name, list.path_length*sizeof(*full_name));
        full_name[list.path_length] = SLASH;
        memmove(full_name + list.path_length + 1, info->name, info->len*sizeof(*full_name));
        full_name[full_name_len] = 0;
        
        if (!info->is_folder){
            parse_file(part, entry_arrays, full_name, full_name_len);
        }
        else{
            full_name[full_name_len - 2] = SLASH;
            full_name[full_name_len - 1] = '*';
            parse_files_by_pattern(part, entry_arrays, full_name, true);
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
    
    bool32 recursive = match(argv[1], "-R");
    if (recursive && argc < 4){
        show_usage(argc, argv);
    }
    
    int32_t size = (256 << 20);
    void *mem = malloc(size);
    Partition part_ = make_part(mem, size);
    Partition *part = &part_;
    
    char *out_directory = argv[2];
    
    int32_t start_i = 2;
    if (recursive){
        start_i = 3;
    }
    
    Meta_Command_Entry_Arrays entry_arrays = {0};
    for (int32_t i = start_i; i < argc; ++i){
        Filename_Character *pattern_name = encode(part, argv[i]);
        parse_files_by_pattern(part, &entry_arrays, pattern_name, recursive);
    }
    
    int32_t out_dir_len = str_size(out_directory);
    if (out_directory[0] == '"'){
        out_directory += 1;
        out_dir_len -= 2;
    }
    
    {
        String str = make_string(out_directory, out_dir_len);
        str = skip_chop_whitespace(str);
        out_directory = str.str;
        out_dir_len = str.size;
    }
    
    int32_t len = out_dir_len + 1 + sizeof(COMMAND_METADATA_OUT) - 1;
    char *out_file_name = (char*)malloc(len + 1);
    memcpy(out_file_name, out_directory, out_dir_len);
    memcpy(out_file_name + out_dir_len, "/", 1);
    memcpy(out_file_name + out_dir_len + 1, COMMAND_METADATA_OUT, sizeof(COMMAND_METADATA_OUT));
    
    FILE *out = fopen(out_file_name, "wb");
    
    if (out != 0){
        int32_t entry_count = entry_arrays.doc_string_count;
        Meta_Command_Entry **entries = get_sorted_meta_commands(part, entry_arrays.first_doc_string, entry_count);
        
        fprintf(out, "#if !defined(META_PASS)\n");
        fprintf(out, "#define command_id(c) (fcoder_metacmd_ID_##c)\n");
        fprintf(out, "#define command_metadata(c) (&fcoder_metacmd_table[command_id(c)])\n");
        fprintf(out, "#define command_metadata_by_id(id) (&fcoder_metacmd_table[id])\n");
        fprintf(out, "#define command_one_past_last_id %d\n", entry_arrays.doc_string_count);
        fprintf(out, "#if defined(CUSTOM_COMMAND_SIG)\n");
        fprintf(out, "#define PROC_LINKS(x,y) x\n");
        fprintf(out, "#else\n");
        fprintf(out, "#define PROC_LINKS(x,y) y\n");
        fprintf(out, "#endif\n");
        
        fprintf(out, "#if defined(CUSTOM_COMMAND_SIG)\n");
        for (int32_t i = 0; i < entry_count; ++i){
            Meta_Command_Entry *entry = entries[i];
            fprintf(out, "CUSTOM_COMMAND_SIG(%.*s);\n", str_to_l_c(entry->name));
        }
        fprintf(out, "#endif\n");
        
        fprintf(out,
                "struct Command_Metadata{\n"
                "PROC_LINKS(Custom_Command_Function, void) *proc;\n"
                "char *name;\n"
                "int32_t name_len;\n"
                "char *description;\n"
                "int32_t description_len;\n"
                "char *source_name;\n"
                "int32_t source_name_len;\n"
                "int32_t line_number;\n"
                "};\n");
        
        fprintf(out,
                "static Command_Metadata fcoder_metacmd_table[%d] = {\n",
                entry_arrays.doc_string_count);
        for (int32_t i = 0; i < entry_count; ++i){
            Meta_Command_Entry *entry = entries[i];
            
            Temp_Memory temp = begin_temp_memory(part);
            
            // HACK(allen): We could just get these at the HEAD END of the process,
            // then we only have to do it once per file, and pass the lengths through.
            int32_t source_name_len = str_size(entry->source_name);
            
            char *fixed_name = push_array(part, char, source_name_len*2 + 1);
            String s = make_string_cap(fixed_name, 0, source_name_len*2 + 1);
            copy(&s, entry->source_name);
            int32_t unescaped_size = s.size;
            replace_str(&s, "\\", "\\\\");
            terminate_with_null(&s);
            
            fprintf(out,
                    "{ PROC_LINKS(%.*s, 0), \"%.*s\", %d,  \"%.*s\", %d, \"%s\", %d, %d },\n",
                    str_to_l_c(entry->name),
                    str_to_l_c(entry->name), entry->name.size,
                    str_to_l_c(entry->docstring.doc), entry->docstring.doc.size,
                    s.str, unescaped_size, entry->line_number);
            end_temp_memory(temp);
        }
        fprintf(out, "};\n");
        
        int32_t id = 0;
        for (int32_t i = 0; i < entry_count; ++i){
            Meta_Command_Entry *entry = entries[i];
            
            fprintf(out, "static int32_t fcoder_metacmd_ID_%.*s = %d;\n",
                    str_to_l_c(entry->name), id);
            ++id;
        }
        
        fprintf(out, "#endif\n");
        
        fclose(out);
    }
    else{
        fprintf(stdout, "fatal error: could not open output file %s%s\n", out_directory, COMMAND_METADATA_OUT);
    }
    
    return(0);
}

// BOTTOM

