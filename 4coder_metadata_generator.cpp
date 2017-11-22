/*
4coder_metadata_generator.cpp - A preprocessor program for generating a list of commands and their descriptions.

TYPE: 'code-preprocessor'
*/

// TOP

#define COMMAND_METADATA_OUT "4coder_generated/command_metadata.h"

#include "4coder_os_comp_cracking.h"

#include "4coder_lib/4coder_mem.h"
#define FSTRING_IMPLEMENTATION
#include "4coder_lib/4coder_string.h"
#include "4coder_lib/4cpp_lexer.h"

#define str_to_l_c(s) ((s).size), ((s).str)
#define str_to_c_l(s) ((s).str), ((s).size)

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef int32_t bool32;

#if defined(IS_WINDOWS)

//// WINDOWS BEGIN ////
#define UNICODE
#include <Windows.h>
typedef TCHAR Filename_Character;
#define SLASH '\\'
//// WINDOWS END ////

#elif defined(IS_LINUX) || defined(IS_MAC)

//// UNIX BEGIN ////
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
typedef char Filename_Character;
#define SLASH '/'
//// UNIX END ////

#else
# error metdata generator not supported on this platform
#endif

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
encode(Partition *part, char *str){
    int32_t size = 0;
    for (;str[size]!=0;++size);
    
    Filename_Character *out = push_array(part, Filename_Character, size + 1);
    push_align(part, 8);
    
    if (out == 0){
        fprintf(stdout, "fatal error: ran out of memory encoding string to filename\n");
        exit(1);
    }
    
    int32_t j = 0;
    for (int32_t i = 0; i <= size; ++i){
        if (str[i] != '"'){
            out[j++] = str[i];
        }
    }
    
    // TODO(NAME): WHY DOESN'T THIS WORK!?
    String fixer_str = make_string_cap(out, j, size + 1);
    fixer_str = skip_chop_whitespace(fixer_str);
    fixer_str.str[fixer_str.size] = 0;
    
    return(fixer_str.str);
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

#if defined(IS_WINDOWS)

//// WINDOWS BEGIN ////
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
    
    File_List list = {0};
    Temp_Memory part_reset = begin_temp_memory(part);
    
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
//// WINDOWS END ////

#elif defined(IS_LINUX) || defined(IS_MAC)

//// UNIX BEGIN ////
static File_List
get_file_list(Partition *part, Filename_Character *directory){
    if (part == 0){
        fprintf(stdout, "fatal error: NULL part passed to %s\n", __FUNCTION__);
        exit(1);
    }
    if (directory == 0){
        fprintf(stdout, "fatal error: NULL dir passed to %s\n", __FUNCTION__);
        exit(1);
    }
    
    DIR *dir_handle = opendir(directory);
    if (dir_handle == 0){
        fprintf(stdout, "fatal error: could not open directory handle\n");
        if (sizeof(*directory) == 2){
            fprintf(stdout, "%ls\n", (wchar_t*)directory);
        }
        else{
            fprintf(stdout, "%s\n", (char*)directory);
        }
        exit(1);
    }
    
    Filename_Character final_name[4096];
    int32_t final_length = str_size(directory);
    if (final_length + 1 > sizeof(final_name)){
        fprintf(stdout, "fatal error: path name too long for local buffer\n");
        exit(1);
    }
    memcpy(final_name, directory, final_length + 1);
    
    int32_t character_count = 0;
    int32_t file_count = 0;
    for (struct dirent *entry = readdir(dir_handle);
         entry != 0;
         entry = readdir(dir_handle)){
        Filename_Character *name = entry->d_name;
        
        int32_t size = 0;
        for(;name[size];++size);
        
        bool32 is_folder = false;
        if (entry->d_type == DT_LNK){
            struct stat st;
            if (stat(entry->d_name, &st) != -1){
                is_folder = S_ISDIR(st.st_mode);
            }
        }
        else{
            is_folder = (entry->d_type == DT_DIR);
        }
        
        if (name[0] != '.' && (is_folder || is_code_file(name, size))){
            ++file_count;
            character_count += size + 1;
        }
    }
    
    File_List list = {0};
    Temp_Memory part_reset = begin_temp_memory(part);
    
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
    
    rewinddir(dir_handle);
    
    int32_t adjusted_file_count = 0;
    for (struct dirent *entry = readdir(dir_handle);
         entry != 0;
         entry = readdir(dir_handle)){
        Filename_Character *name = entry->d_name;
        
        int32_t size = 0;
        for(;name[size];++size);
        
        bool32 is_folder = false;
        if (entry->d_type == DT_LNK){
            struct stat st;
            if (stat(entry->d_name, &st) != -1){
                is_folder = S_ISDIR(st.st_mode);
            }
        }
        else{
            is_folder = (entry->d_type == DT_DIR);
        }
        
        if (name[0] != '.' && (is_folder || is_code_file(name, size))){
            if (info_ptr + 1 > info_ptr_end || char_ptr + size + 1 > char_ptr_end){
                memset(&list, 0, sizeof(list));
                end_temp_memory(part_reset);
                closedir(dir_handle);
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
    }
    closedir(dir_handle);
    
    list.info = info_ptr_base;
    list.count = adjusted_file_count;
    list.final_length = final_length;
    memcpy(list.final_name, final_name, list.final_length*sizeof(*final_name));
    list.final_name[list.final_length] = 0;
    
    return(list);
}
//// UNIX END ////

#else
# error metdata generator not supported on this platform
#endif

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
        if (text.str == 0){
            fprintf(stdout, "fatal error: not enough memory in partition for file dumping");
            exit(1);
        }
        fread(text.str, 1, text.size, file);
        terminate_with_null(&text);
        fclose(file);
    }
    
    return(text);
}

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
    
    Meta_Command_Entry *new_entry = push_array(part, Meta_Command_Entry, 1);
    new_entry->name = name;
    new_entry->source_name = reader->source_name;
    new_entry->line_number = line_number(reader, start_pos);
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
parse_files_in_directory(Partition *part, Meta_Command_Entry_Arrays *entry_arrays, Filename_Character *root, bool32 recursive){
    File_List list = get_file_list(part, root);
    for (int32_t i = 0; i < list.count; ++i){
        File_Info *info = &list.info[i];
        
        if (info->is_folder && match(make_string(info->name, info->len), "4coder_generated")){
            continue;
        }
        
        int32_t full_name_len = list.final_length + 1 + info->len;
        Filename_Character *full_name = push_array(part, Filename_Character, full_name_len + 1);
        push_align(part, 8);
        
        if (full_name == 0){
            fprintf(stdout, "fatal error: not enough memory to recurse to sub directory\n");
            exit(1);
        }
        
        memmove(full_name, list.final_name, list.final_length*sizeof(*full_name));
        full_name[list.final_length] = SLASH;
        memmove(full_name + list.final_length + 1, info->name, info->len*sizeof(*full_name));
        full_name[full_name_len] = 0;
        
        if (!info->is_folder){
            parse_file(part, entry_arrays, full_name, full_name_len);
        }
        else{
            parse_files_in_directory(part, entry_arrays, full_name, recursive);
        }
    }
}

static void
show_usage(int argc, char **argv){
    char *name = "metadata_generator";
    if (argc >= 1){
        name = argv[0];
    }
    fprintf(stdout, "usage:\n%s [-R] <4coder-root-directory> <scan-root-directory> [<scan-root-directory2> ...]\n", name);
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
        Filename_Character *root_name = encode(part, argv[i]);
        parse_files_in_directory(part, &entry_arrays, root_name, recursive);
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
        for (Meta_Command_Entry *entry = entry_arrays.first_doc_string;
             entry != 0;
             entry = entry->next){
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
        for (Meta_Command_Entry *entry = entry_arrays.first_doc_string;
             entry != 0;
             entry = entry->next){
            Temp_Memory temp = begin_temp_memory(part);
            
            // HACK(allen): We could just get these at the HEAD END of the process,
            // then we only have to do it once per file, and pass the lengths through.
            int32_t source_name_len = str_size(entry->source_name);
            
            char *fixed_name = push_array(part, char, source_name_len*2 + 1);
            String s = make_string_cap(fixed_name, 0, source_name_len*2 + 1);
            copy(&s, entry->source_name);
            replace_str(&s, "\\", "\\\\");
            terminate_with_null(&s);
            
            fprintf(out,
                    "{ PROC_LINKS(%.*s, 0), \"%.*s\", %d,  \"%.*s\", %d, \"%s\", %d, %d },\n",
                    str_to_l_c(entry->name),
                    str_to_l_c(entry->name), entry->name.size,
                    str_to_l_c(entry->docstring.doc), entry->docstring.doc.size,
                    s.str, s.size, entry->line_number);
            end_temp_memory(temp);
        }
        fprintf(out, "};\n");
        
        int32_t id = 0;
        for (Meta_Command_Entry *entry = entry_arrays.first_doc_string;
             entry != 0;
             entry = entry->next){
            fprintf(out, "static int32_t fcoder_metacmd_ID_%.*s = %d;\n",
                    str_to_l_c(entry->name), id);
            ++id;
        }
    }
    else{
        fprintf(stdout, "fatal error: could not open output file %s%s\n", out_directory, COMMAND_METADATA_OUT);
    }
    
    return(0);
}

// BOTTOM

