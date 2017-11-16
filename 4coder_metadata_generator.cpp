/*
4coder_metadata_generator.cpp - A preprocessor program for generating a list of commands and their descriptions.

TYPE: 'code-preprocessor'
*/

// TOP

#include "4coder_lib/4coder_mem.h"
#define FSTRING_IMPLEMENTATION
#include "4coder_lib/4coder_string.h"
#include "4cpp/4cpp_lexer.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef int32_t bool32;

//// WINDOWS BEGIN ////
#define UNICODE
#include <Windows.h>
typedef TCHAR Filename_Character;
//// WINDOWS END ////

struct File_Info{
    Filename_Character *name;
    int32_t len;
    bool32 is_folder;
};

struct File_List{
    File_Info *info;
    int32_t count;
    int32_t final_length;
    Filename_Character final_name[4096];
};

static File_List
get_file_list(Partition *part, Filename_Character *dir);

static Filename_Character*
encode(Partition *part, char *str);

static char*
unencode(Partition *part, Filename_Character *str, int32_t len);

//// WINDOWS BEGIN ////
static bool32
is_code_file(Filename_Character *name, int32_t len){
    bool32 is_code = false;
    if (len >= 5){
        Filename_Character *ext = &name[len - 4];
        if (ext[0] == '.' && ext[1] == 'c' && ext[2] == 'p' && ext[3] == 'p'){
            is_code = true;
        }
        else if (ext[0] == '.' && ext[1] == 'h' && ext[2] == 'p' && ext[3] == 'p'){
            is_code = true;
        }
    }
    if (len >= 4){
        Filename_Character *ext = &name[len - 3];
        if (ext[0] == '.' && ext[1] == 'c' && ext[2] == 'c'){
            is_code = true;
        }
    }
    if (len >= 3){
        Filename_Character *ext = &name[len - 2];
        if (ext[0] == '.' && ext[1] == 'h'){
            is_code = true;
        }
        else if (ext[0] == '.' && ext[1] == 'c'){
            is_code = true;
        }
    }
    return(is_code);
}

static File_List
get_file_list(Partition *part, Filename_Character *dir){
    if (part == 0){
        fprintf(stdout, "fatal error: NULL part passed to %s\n", __FUNCTION__);
        exit(1);
    }
    if (dir == 0){
        fprintf(stdout, "fatal error: NULL dir passed to %s\n", __FUNCTION__);
        exit(1);
    }
    
    File_List list = {0};
    Temp_Memory part_reset = begin_temp_memory(part);
    
    HANDLE dir_handle =
        CreateFile(dir,
                   FILE_LIST_DIRECTORY,
                   FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                   0,
                   OPEN_EXISTING,
                   FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                   0);
    
    if (dir_handle == INVALID_HANDLE_VALUE){
        fprintf(stdout, "fatal error: could not open directory handle\n");
        exit(1);
    }
    
    Filename_Character final_name[4096];
    DWORD final_length = GetFinalPathNameByHandle(dir_handle, final_name, sizeof(final_name), 0);
    if (final_length > sizeof(final_name)){
        fprintf(stdout, "fatal error: path name too long for local buffer\n");
        exit(1);
    }
    CloseHandle(dir_handle);
    
    final_length -= 4;
    memmove(final_name, final_name + 4, final_length*sizeof(*final_name));
    final_name[final_length] = '\\';
    final_name[final_length + 1] = '*';
    final_name[final_length + 2] = 0;
    
    WIN32_FIND_DATA find_data = {0};
    HANDLE search = FindFirstFile(final_name, &find_data);
    if (search == INVALID_HANDLE_VALUE){
        fprintf(stdout, "fatal error: could not begin a file search\n");
        exit(1);
    }
    
    int32_t character_count = 0;
    int32_t file_count = 0;
    BOOL more_files = true;
    do{
        Filename_Character *name = &find_data.cFileName[0];
        
        int32_t size = 0;
        for(;name[size];++size);
        
        uint32_t attribs = find_data.dwFileAttributes;
        bool32 is_folder = ((attribs & FILE_ATTRIBUTE_DIRECTORY) != 0);
        
        if (name[0] != '.' && (is_folder || is_code_file(name, size))){
            ++file_count;
            character_count += size + 1;
        }
        
        more_files = FindNextFile(search, &find_data);
    }while(more_files);
    FindClose(search);
    
    int32_t rounded_char_size = (character_count*sizeof(Filename_Character) + 7)&(~7);
    int32_t memsize = rounded_char_size + file_count*sizeof(File_Info);
    void *mem = push_array(part, uint8_t, memsize);
    if (mem == 0){
        fprintf(stdout, "fatal error: not enough memory on the partition for a file list.\n");
        exit(1);
    }
    
    Filename_Character *char_ptr = (Filename_Character*)mem;
    File_Info *info_ptr = (File_Info*)((uint8_t*)mem + rounded_char_size);
    
    Filename_Character *char_ptr_end = (Filename_Character*)info_ptr;
    File_Info *info_ptr_end = info_ptr + file_count;
    
    File_Info *info_ptr_base = info_ptr;
    
    search = FindFirstFile(final_name, &find_data);
    if (search == INVALID_HANDLE_VALUE){
        fprintf(stdout, "fatal error: could not restart a file search\n");
        exit(1);
    }
    
    int32_t adjusted_file_count = 0;
    more_files = true;
    do{
        Filename_Character *name = &find_data.cFileName[0];
        
        int32_t size = 0;
        for(;name[size]!=0;++size);
        
        uint32_t attribs = find_data.dwFileAttributes;
        bool32 is_folder = ((attribs & FILE_ATTRIBUTE_DIRECTORY) != 0);
        
        if (name[0] != '.' && (is_folder || is_code_file(name, size))){
            if (info_ptr + 1 > info_ptr_end || char_ptr + size + 1 > char_ptr_end){
                memset(&list, 0, sizeof(list));
                end_temp_memory(part_reset);
                FindClose(search);
                return(list);
            }
            
            info_ptr->name = char_ptr;
            info_ptr->len = size;
            info_ptr->is_folder = is_folder;
            
            memmove(char_ptr, name, size*sizeof(*name));
            char_ptr[size] = 0;
            
            char_ptr += size + 1;
            ++info_ptr;
            ++adjusted_file_count;
        }
        
        more_files = FindNextFile(search, &find_data);
    }while(more_files);
    FindClose(search);
    
    list.info = info_ptr_base;
    list.count = adjusted_file_count;
    list.final_length = final_length;
    memcpy(list.final_name, final_name, list.final_length*sizeof(*final_name));
    list.final_name[list.final_length] = 0;
    
    return(list);
}

static Filename_Character*
encode(Partition *part, char *str){
    int32_t size = 0;
    for (;str[size]!=0;++size);
    
    Filename_Character *out = push_array(part, Filename_Character, size + 1);
    push_align(part, 8);
    
    if (out == 0){
        fprintf(stdout, "fatal error: ran out of memory encoding string to filename\n");
        exit(1);
    }
    
    for (int32_t i = 0, j = 0; i <= size; ++i){
        if (str[i] != '"'){
            out[j++] = str[i];
        }
    }
    
    return(out);
}

static char*
unencode(Partition *part, Filename_Character *str, int32_t len){
    Temp_Memory temp = begin_temp_memory(part);
    char *out = push_array(part, char, len + 1);
    push_align(part, 8);
    
    if (out == 0){
        fprintf(stdout, "fatal error: ran out of memory unencoding string to filename\n");
        exit(1);
    }
    
    for (int32_t i = 0; i <= len; ++i){
        if (str[i] <= 127){
            out[i] = (char)str[i];
        }
        else{
            out = 0;
            end_temp_memory(temp);
            break;
        }
    }
    
    return(out);
}
//// WINDOWS END ////

static String
file_dump(Partition *part, char *name){
    String text = {0};
    
    FILE *file = fopen(name, "rb");
    if (file != 0){
        fseek(file, 0, SEEK_END);
        text.size = ftell(file);
        fseek(file, 0, SEEK_SET);
        text.memory_size = text.size + 1;
        text.str = push_array(part, char, text.memory_size);
        fread(text.str, 1, text.size, file);
        terminate_with_null(&text);
        fclose(file);
    }
    
    return(text);
}

static void
error(char *source_name, String text, int32_t pos, char *msg){
    if (pos < 0){
        pos = 0;
    }
    if (pos > text.size){
        pos = text.size;
    }
    
    int32_t line_number = 1;
    int32_t character_pos = 1;
    char *end = text.str + pos;
    for (char *p = text.str; p < end; ++p){
        if (*p == '\n'){
            ++line_number;
            character_pos = 1;
        }
        else{
            ++character_pos;
        }
    }
    
    fprintf(stdout, "%s:%d:%d: %s\n", source_name, line_number, character_pos, msg);
    fflush(stdout);
}

struct Reader{
    char *source_name;
    String text;
    Cpp_Token_Array tokens;
    Cpp_Token *ptr;
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

static void
error(Reader *reader, int32_t pos, char *msg){
    error(reader->source_name, reader->text, pos, msg);
}

struct Temp_Read{
    Reader *reader;
    Cpp_Token *pos;
};

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

static String
token_str(String text, Cpp_Token token){
    String str = substr(text, token.start, token.size);
    return(str);
}

static bool32
require_key_identifier(Reader *reader, char *str){
    bool32 success = false;
    
    Cpp_Token token = get_token(reader);
    if (token.type == CPP_TOKEN_IDENTIFIER){
        String lexeme = token_str(reader->text, token);
        if (match(lexeme, str)){
            success = true;
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
require_open_parenthese(Reader *reader){
    bool32 success = false;
    
    Cpp_Token token = get_token(reader);
    if (token.type == CPP_TOKEN_PARENTHESE_OPEN){
        success = true;
    }
    
    if (!success){
        error(reader, token.start, "expected to find '('");
    }
    
    return(success);
}

static bool32
require_close_parenthese(Reader *reader){
    bool32 success = false;
    
    Cpp_Token token = get_token(reader);
    if (token.type == CPP_TOKEN_PARENTHESE_CLOSE){
        success = true;
    }
    
    if (!success){
        error(reader, token.start, "expected to find ')'");
    }
    
    return(success);
}

static bool32
require_define(Reader *reader){
    bool32 success = false;
    
    Cpp_Token token = get_token(reader);
    if (token.type == CPP_PP_DEFINE){
        success = true;
    }
    
    if (!success){
        error(reader, token.start, "expected to find '#define'");
    }
    
    return(success);
}

static bool32
extract_identifier(Reader *reader, String *str_out){
    bool32 success = false;
    
    Cpp_Token token = get_token(reader);
    if (token.type == CPP_TOKEN_IDENTIFIER){
        String lexeme = token_str(reader->text, token);
        *str_out = lexeme;
        success = true;
    }
    
    if (!success){
        error(reader, token.start, "expected to find an identifier");
    }
    
    return(success);
}

static bool32
extract_string(Reader *reader, String *str_out){
    bool32 success = false;
    
    Cpp_Token token = get_token(reader);
    if (token.type == CPP_TOKEN_STRING_CONSTANT){
        String lexeme = token_str(reader->text, token);
        *str_out = lexeme;
        success = true;
    }
    
    if (!success){
        error(reader, token.start, "expected to find a string literal");
    }
    
    return(success);
}

static bool32
parse_documented_command(Partition *part, Reader *reader){
    String name = {0};
    String doc = {0};
    
    // Getting the command's name
    if (!require_key_identifier(reader, "CUSTOM_COMMAND_SIG")){
        return(false);
    }
    
    if (!require_open_parenthese(reader)){
        return(false);
    }
    
    if (!extract_identifier(reader, &name)){
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
    
    if (!extract_string(reader, &doc)){
        return(false);
    }
    
    if (!require_close_parenthese(reader)){
        return(false);
    }
    
    // TODO(allen): Store into data structure for codegen.
    //error(reader, name_pos, "name of a command");
    //error(reader, str_pos, "doc string of a command");
    
    return(true);
}

static bool32
parse_alias(Partition *part, Reader *reader){
    String name = {0};
    String potential = {0};
    
    // Getting the alias's name
    if (!require_define(reader)){
        return(false);
    }
    
    int32_t name_pos = peek_pos(reader);
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
    
    int32_t potential_pos = peek_pos(reader);
    if (!extract_identifier(reader, &potential)){
        return(false);
    }
    
    if (!require_close_parenthese(reader)){
        return(false);
    }
    
    error(reader, name_pos, "name of an alias");
    error(reader, potential_pos, "name of a potential");
    
    return(true);
}

static void
parse_text(Partition *part, char *source_name, String text){
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
                for (int32_t R = 0; R < 5; ++R){
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
                    if (!parse_documented_command(part, reader)){
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
                    if (!parse_alias(part, reader)){
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
parse_file(Partition *part, Filename_Character *name_, int32_t len){
    char *name = unencode(part, name_, len);
    if (name == 0){
        if (sizeof(*name_) == 2){
            fprintf(stdout, "warning: could not unencode file name %ls - file skipped\n", name_);
        }
        else{
            fprintf(stdout, "warning: could not unencode file name %s - file skipped\n", name_);
        }
        return;
    }
    
    String text = file_dump(part, name);
    parse_text(part, name, text);
}

static void
parse_files_in_directory(Partition *part, Filename_Character *root, bool32 recursive){
    File_List list = get_file_list(part, root);
    for (int32_t i = 0; i < list.count; ++i){
        File_Info *info = &list.info[i];
        
        int32_t full_name_len = list.final_length + 1 + info->len;
        Filename_Character *full_name = push_array(part, Filename_Character, full_name_len + 1);
        push_align(part, 8);
        
        if (full_name == 0){
            fprintf(stdout, "fatal error: not enough memory to recurse to sub directory\n");
            exit(1);
        }
        
        memmove(full_name, list.final_name, list.final_length*sizeof(*full_name));
        full_name[list.final_length] = '\\';
        memmove(full_name + list.final_length + 1, info->name, info->len*sizeof(*full_name));
        full_name[full_name_len] = 0;
        
        if (!info->is_folder){
            parse_file(part, full_name, full_name_len);
        }
        else{
            parse_files_in_directory(part, full_name, recursive);
        }
    }
}

static void
show_usage(int argc, char **argv){
    char *name = "metadata_generator";
    if (argc >= 1){
        name = argv[0];
    }
    fprintf(stdout, "usage:\n%s [-R] <root-directory> [<root-directory2> ...]\n", name);
    exit(0);
}

int
main(int argc, char **argv){
    if (argc < 2){
        show_usage(argc, argv);
    }
    
    bool32 recursive = match(argv[1], "-R");
    if (recursive && argc < 3){
        show_usage(argc, argv);
    }
    
    int32_t size = (256 << 20);
    void *mem = malloc(size);
    Partition part_ = make_part(mem, size);
    Partition *part = &part_;
    
    int32_t start_i = 1;
    if (recursive){
        start_i = 2;
    }
    
    for (int32_t i = start_i; i < argc; ++i){
        Filename_Character *root_name = encode(part, argv[i]);
        parse_files_in_directory(part, root_name, recursive);
    }
    
    return(0);
}

// BOTTOM

