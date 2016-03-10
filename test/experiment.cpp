/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 21.1.2015
 *
 * Test for CPP lexer & parser layer for project codename "4ed"
 *
 */

// TOP

#include "../4ed_meta.h"

#define FCPP_STRING_IMPLEMENTATION
#include "../4coder_string.h"

#include "../4cpp_types.h"

#include "../4cpp_lexer_types.h"

#define FCPP_LEXER_IMPLEMENTATION
#include "../4cpp_lexer.h"

namespace new_lex{
#include "4cpp_new_lexer.h"
}

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>

static Data
dump_file(char *filename){
    Data data = {};
    HANDLE file;
    DWORD hi, lo;
    
    file = CreateFile(filename, GENERIC_READ, 0, 0, 
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (file != INVALID_HANDLE_VALUE){
        lo = GetFileSize(file, &hi);
        assert(hi == 0);
        
        data.size = (int)lo;
        data.data = (byte*)malloc(data.size + 1);
        
        ReadFile(file, data.data, lo, &lo, 0);
        
        assert((int)lo == data.size);
        
        CloseHandle(file);
    }
    
    return(data);
}

typedef struct File_Info{
    String filename;
    int folder;
} File_Info;

typedef struct File_List{
    // Ignore this, it's for internal stuff.
    void *block;
    
    // The list of files and folders.
    File_Info *infos;
    int count;
    
    // Ignore this, it's for internal stuff.
    int block_size;
} File_List;

void*
Win32GetMemory(int size){
    return (malloc(size));
}

void
Win32FreeMemory(void *ptr){
    free(ptr);
}

static void
system_set_file_list(File_List *file_list, String directory){
    if (directory.size > 0){
        char dir_space[MAX_PATH + 32];
        String dir = make_string(dir_space, 0, MAX_PATH + 32);
        append(&dir, directory);
        char trail_str[] = "\\*";
        append(&dir, trail_str);
        
        char *c_str_dir = make_c_str(dir);
        
        WIN32_FIND_DATA find_data;
        HANDLE search;
        search = FindFirstFileA(c_str_dir, &find_data);
        
        if (search != INVALID_HANDLE_VALUE){            
            i32 count = 0;
            i32 file_count = 0;
            BOOL more_files = 1;
            do{
                if (!match(find_data.cFileName, ".") &&
                    !match(find_data.cFileName, "..")){
                    ++file_count;
                    i32 size = 0;
                    for(;find_data.cFileName[size];++size);
                    count += size + 1;
                }
                more_files = FindNextFile(search, &find_data);
            }while(more_files);
            FindClose(search);

            i32 required_size = count + file_count * sizeof(File_Info);
            if (file_list->block_size < required_size){
                Win32FreeMemory(file_list->block);
                file_list->block = Win32GetMemory(required_size);
                file_list->block_size = required_size;
            }
            
            file_list->infos = (File_Info*)file_list->block;
            char *name = (char*)(file_list->infos + file_count);
            if (file_list->block){
                search = FindFirstFileA(c_str_dir, &find_data);
                
                if (search != INVALID_HANDLE_VALUE){
                    File_Info *info = file_list->infos;
                    more_files = 1;
                    do{
                        if (!match(find_data.cFileName, ".") &&
                            !match(find_data.cFileName, "..")){
                            info->folder = (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
                            info->filename.str = name;
                            
                            i32 i = 0;
                            for(;find_data.cFileName[i];++i) *name++ = find_data.cFileName[i];
                            info->filename.size = i;
                            info->filename.memory_size = info->filename.size + 1;
                            *name++ = 0;
                            replace_char(info->filename, '\\', '/');
                            ++info;
                        }
                        more_files = FindNextFile(search, &find_data);
                    }while(more_files);
                    FindClose(search);
                    
                    file_list->count = file_count;
                    
                }else{
                    Win32FreeMemory(file_list->block);
                    file_list->block = 0;
                    file_list->block_size = 0;
                }
            }
        }
    }
    else{
        if (directory.str == 0){
            Win32FreeMemory(file_list->block);
            file_list->block = 0;
            file_list->block_size = 0;
        }
        file_list->infos = 0;
        file_list->count = 0;
    }
}

#define TOKEN_MAX (1 << 12)
#define TOKEN_ARRAY_SIZE (TOKEN_MAX*sizeof(Cpp_Token))

static void
init_test_stack(Cpp_Token_Stack *stack){
    stack->tokens = (Cpp_Token*)malloc(TOKEN_ARRAY_SIZE);
    stack->count = 0;
    stack->max_count = TOKEN_MAX;
}

Cpp_Lex_Data lex_data = {};

struct Experiment{
    Cpp_Token_Stack correct_stack;
    Cpp_Token_Stack testing_stack;
    int passed_total, test_total;
};

static void
run_experiment(Experiment *exp, char *filename){
    String extension = {};
    Data file_data;
    Cpp_File file_cpp;
    int pass;

    extension = file_extension(make_string_slowly(filename));

    if (match(extension, "cpp") || match(extension, "h")){
        
        pass = 1;
        printf("testing on file: %s\n", filename);
        file_data = dump_file(filename);

        if (file_data.size < (100 << 10)){
            exp->test_total++;
            
            exp->correct_stack.count = 0;
            exp->testing_stack.count = 0;

            memset(exp->correct_stack.tokens, TOKEN_ARRAY_SIZE, 0);
            memset(exp->testing_stack.tokens, TOKEN_ARRAY_SIZE, 0);

            file_cpp.data = (char*)file_data.data;
            file_cpp.size = file_data.size;

            cpp_lex_file_nonalloc(file_cpp, &exp->correct_stack, lex_data);
            new_lex::cpp_lex_nonalloc((char*)file_data.data, 0, file_data.size, &exp->testing_stack);

            if (exp->correct_stack.count != exp->testing_stack.count){
                pass = 0;
                printf("error: stack size mismatch %d original and %d testing\n",
                    exp->correct_stack.count, exp->testing_stack.count);
            }

            int min_count = exp->correct_stack.count;
            if (min_count > exp->testing_stack.count) min_count = exp->testing_stack.count;

            for (int j = 0; j < min_count; ++j){
                Cpp_Token *correct, *testing;
                correct = exp->correct_stack.tokens + j;
                testing = exp->testing_stack.tokens + j;

                if (correct->type != testing->type){
                    pass = 0;
                    printf("type mismatch at token %d\n", j);
                }

                if (correct->start != testing->start || correct->size != testing->size){
                    pass = 0;
                    printf("token range mismatch at token %d\n"
                            "\t%d:%d original %d:%d testing\n"
                            "\t%.*s original %.*s testing\n",
                        j,
                        correct->start, correct->size, testing->start, testing->size,
                        correct->size, file_cpp.data + correct->start,
                        testing->size, file_cpp.data + testing->start);
                }
                
                if (correct->flags != testing->flags){
                    pass = 0;
                    printf("token flag mismatch at token %d\n", j);
                }
            }

            if (pass){
                exp->passed_total++;
                printf("test passed!\n\n");
            }
            else{
                printf("test failed, you failed, fix it now!\n\n");
            }
        }
        
        free(file_data.data);
    }
}

#define BASE_DIR "w:/4ed/data/test/"

int main(){
    char test_directory[] = BASE_DIR;
    File_List all_files = {};
    Experiment exp = {};

    init_test_stack(&exp.correct_stack);
    init_test_stack(&exp.testing_stack);
    
    AllowLocal(test_directory);
    AllowLocal(all_files);

    run_experiment(&exp, BASE_DIR "autotab.cpp");
    
#if 0
    system_set_file_list(&all_files, make_lit_string(test_directory));
    
    for (int i = 0; i < all_files.count; ++i){
        if (all_files.infos[i].folder == 0){
            run_experiment(&exp, all_files.infos[i].filename.str);
        }
    }
#endif

    printf("you passed %d / %d tests\n", exp.passed_total, exp.test_total);
    
    return(0);
}

// BOTTOM

