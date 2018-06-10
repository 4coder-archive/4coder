/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 10.06.2018
 *
 * Test bed for developing 4coder_cpp_preprocessor
 *
 */

// TOP

#include "../4ed_defines.h"
#include "../meta/4ed_meta_defines.h"

#include "../4coder_file.h"

#include "../4coder_lib/4coder_mem.h"
#include "../4coder_lib/4cpp_lexer.h"
#include "../4coder_cpp_preprocessor.h"

#include "../4coder_cpp_preprocessor.cpp"

void
print_usage(void){
    fprintf(stdout, "test_preproc src correct_result\n");
}

int main(int argc, char **argv){
    if (argc != 3){
        print_usage();
        exit(1);
    }
    
    char *input_file_name = argv[1];
    char *compare_file_name = argv[2];
    
    i32 memory_size = MB(512);
    void *memory = malloc(memory_size);
    Partition part = make_part(memory, memory_size);
    String text = file_dump(&part, input_file_name);
    if (text.str != 0){
        String compare_text = file_dump(&part, compare_file_name);
        if (compare_text.str != 0){
            if (text.size != compare_text.size ||
                memcmp(text.str, compare_text.str, text.size) != 0){
                fprintf(stdout, "dif failed\n");
            }
        }
        else{
            fprintf(stdout, "could not open comparison file %s\n", compare_file_name);
        }
    }
    else{
        fprintf(stdout, "could not open input %s\n", input_file_name);
    }
    return(0);
}

// BOTTOM