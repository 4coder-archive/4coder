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
#include "4cpp_new_lexer.h"

#include <windows.h>
#include <intrin.h>

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
        data.data[data.size] = 0;

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
int
passed(Experiment exp){
    return (exp.passed_total == exp.test_total && exp.passed_total > 1);
}

struct Times{
    i64 handcoded;
    i64 fsm;
};
Times time;
void
begin_t(Times *t){
    time = *t;
}
void
end_t(Times *t){
    *t = time;
}

static void
run_experiment(Experiment *exp, char *filename, int verbose, int chunks){
    String extension = {};
    Data file_data;
    Cpp_File file_cpp;
    new_lex::Lex_Data ld = {0};
    int pass;
    int k, chunk_size, is_last;

    extension = file_extension(make_string_slowly(filename));

    if (match(extension, "cpp") || match(extension, "h")){
        file_data = dump_file(filename);
        if (file_data.size < (100 << 10)){
            pass = 1;
            if (verbose >= 0) printf("testing on file: %s\n", filename);
            exp->test_total++;

            exp->correct_stack.count = 0;
            exp->testing_stack.count = 0;

            memset(exp->correct_stack.tokens, TOKEN_ARRAY_SIZE, 0);
            memset(exp->testing_stack.tokens, TOKEN_ARRAY_SIZE, 0);

            file_cpp.data = (char*)file_data.data;
            file_cpp.size = file_data.size;

            ld.tb = (char*)malloc(file_data.size + 1);

            {
                i64 start;

                start = __rdtsc();
                cpp_lex_file_nonalloc(file_cpp, &exp->correct_stack, lex_data);
                time.handcoded += (__rdtsc() - start);

                start = __rdtsc();
                if (chunks){
                    int relevant_size = file_data.size + 1;
                    is_last = 0;
                    for (k = 0; k < relevant_size; k += chunks){
                        chunk_size = chunks;
                        if (chunk_size + k >= relevant_size){
                            chunk_size = relevant_size - k;
                            is_last = 1;
                        }

                        int result = new_lex::cpp_lex_nonalloc(&ld, (char*)file_data.data + k, chunk_size, &exp->testing_stack);
                        if (result == 0 || result == 2) break;
                    }
                }
                else{
                    new_lex::cpp_lex_nonalloc(&ld, (char*)file_data.data, file_data.size, &exp->testing_stack);
                }
                time.fsm += (__rdtsc() - start);
            }

            free(ld.tb);

            if (exp->correct_stack.count != exp->testing_stack.count){
                pass = 0;
                if (verbose >= 0){
                    printf("error: stack size mismatch %d original and %d testing\n",
                        exp->correct_stack.count, exp->testing_stack.count);
                }
            }

            int min_count = exp->correct_stack.count;
            if (min_count > exp->testing_stack.count) min_count = exp->testing_stack.count;

            for (int j = 0; j < min_count; ++j){
                Cpp_Token *correct, *testing;
                correct = exp->correct_stack.tokens + j;
                testing = exp->testing_stack.tokens + j;

                if (correct->type != testing->type){
                    pass = 0;
                    if (verbose >= 1) printf("type mismatch at token %d\n", j);
                }

                if (correct->start != testing->start || correct->size != testing->size){
                    pass = 0;
                    if (verbose >= 1){
                        printf("token range mismatch at token %d\n"
                                "    %d:%d original %d:%d testing\n"
                                "    %.*s original %.*s testing\n",
                            j,
                            correct->start, correct->size, testing->start, testing->size,
                            correct->size, file_cpp.data + correct->start,
                            testing->size, file_cpp.data + testing->start);
                    }
                }

                if (correct->flags != testing->flags){
                    pass = 0;
                    if (verbose >= 1) printf("token flag mismatch at token %d\n", j);
                }
            }

            if (pass){
                exp->passed_total++;
                if (verbose >= 0) printf("test passed!\n\n");
            }
            else{
                if (verbose >= 0) printf("test failed, you failed, fix it now!\n\n");
            }
        }

        free(file_data.data);
    }
}

#define OUTLINE(type) "%-30s "type"\n"
#define OUTLINE_VAR(t, var) #var, (t)var

void
show_time(Times t, int repeats, char *type){
    f32 speed_up = ((f32)t.handcoded) / (t.fsm);
    printf(
        "\n%s time for %d repeates\n"
            OUTLINE("%lld")
            OUTLINE("%lld")
            OUTLINE("%f"),
        type,
        repeats,
        OUTLINE_VAR(i64, t.handcoded),
        OUTLINE_VAR(i64, t.fsm),
        OUTLINE_VAR(f32, speed_up)
    );
}

#define BASE_DIR "w:/4ed/data/test/"

int main(){
    int repeats = 100;
    int verbose_level = -1;
    int chunk_start = 0;
    int chunk_end = 0;
#define TEST_FILE "autotab.cpp"    
#define SINGLE_ITEM 0

    int chunks = (chunk_start > 0 && chunk_start <= chunk_end);
    int c = 0;

    char test_directory[] = BASE_DIR;
    File_List all_files = {};
    Experiment exp = {};
    Experiment chunk_exp = {};
    Times exp_t = {};
    Times chunk_exp_t = {};

    init_test_stack(&exp.correct_stack);
    init_test_stack(&exp.testing_stack);

    init_test_stack(&chunk_exp.correct_stack);
    init_test_stack(&chunk_exp.testing_stack);

    AllowLocal(test_directory);
    AllowLocal(all_files);

#if SINGLE_ITEM
    (void)(repeats);
    (void)(verbose_level);

    if (chunks){
        begin_t(&chunk_exp_t);
        printf("With chunks of %d\n", chunks);
        for (c = chunk_start; c <= chunk_end; ++c){
            run_experiment(&chunk_exp, BASE_DIR TEST_FILE, 1, c);
        }
        end_t(&chunk_exp_t);
    }

    begin_t(&exp_t);
    printf("Unchunked\n");
    run_experiment(&exp, BASE_DIR TEST_FILE, 1, 0);
    end_t(&exp_t);

#else

    system_set_file_list(&all_files, make_lit_string(test_directory));

    for (int j = 0; j < repeats; ++j){
        for (int i = 0; i < all_files.count; ++i){
            if (all_files.infos[i].folder == 0){
                if (chunks){
                    begin_t(&chunk_exp_t);
                    for (c = chunk_start; c <= chunk_end; ++c){
                        run_experiment(&chunk_exp, all_files.infos[i].filename.str, verbose_level, c);
                    }
                    end_t(&chunk_exp_t);
                }

                begin_t(&exp_t);
                if (verbose_level == -1 && chunks){
                    for (c = chunk_start; c <= chunk_end; ++c){
                        run_experiment(&exp, all_files.infos[i].filename.str, verbose_level, 0);
                    }
                }
                else{
                    run_experiment(&exp, all_files.infos[i].filename.str, verbose_level, 0);
                }
                end_t(&exp_t);
            }
        }
    }
#endif

    if (chunks){
        printf("chunks of sizes %d through %d tested\n", chunk_start, chunk_end);
        printf("chunked passed %d / %d tests\n", chunk_exp.passed_total, chunk_exp.test_total);
    }

    printf("unchunk passed %d / %d tests\n", exp.passed_total, exp.test_total);

    if (passed(exp) && (chunks == 0 || passed(chunk_exp))){
        if (chunks){
            show_time(chunk_exp_t, repeats, "Chunked");
        }
        show_time(exp_t, repeats, "Unchunked");
    }

    return(0);
}

// BOTTOM
