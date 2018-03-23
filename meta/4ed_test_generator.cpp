/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 16.03.2018
 *
 * Converter for *.4is -> *.4id
 *
 */

// TOP

#include "4ed_defines.h"
#include "4coder_lib/4coder_string.h"

#include "4coder_file.h"

#include <stdio.h>

internal void
print_usage(char *name){
    fprintf(stdout, "usage: %s\n", name);
}

internal FILE*
try_open_output(char *name){
    FILE *out = fopen(name, "wb");
    if (out == 0){
        fprintf(stdout, "Could not open output file %s\n", name);
    }
    return(out);
}

internal void
write_open_test_file_default_bindings(FILE *out,
                                      char *src_name, char *dst_name){
    fprintf(out,
            "wait 1\n"
            "key o MDFR_CTRL\n"
            "wait 1\n"
            "type 1 %s\n"
            "wait 1\n"
            "key key_newline MDFR_NONE\n"
            "wait 1\n"
            "key key_space MDFR_CTRL\n"
            "wait 1\n"
            "key key_page_down MDFR_CTRL\n"
            "wait 1\n"
            "key c MDFR_CTRL\n"
            "wait 1\n"
            "key K MDFR_CTRL\n"
            "wait 1\n"
            "key o MDFR_CTRL\n"
            "wait 1\n"
            "key key_back MDFR_NONE\n"
            "wait 1\n"
            "type 1 output/%s\n"
            "wait 1\n"
            "key key_newline MDFR_NONE\n"
            "wait 1\n"
            "key key_space MDFR_CTRL\n"
            "wait 1\n"
            "key key_page_down MDFR_CTRL\n"
            "wait 1\n"
            "key d MDFR_CTRL\n"
            "wait 1\n"
            "key v MDFR_CTRL\n"
            "wait 1\n"
            "key m MDFR_CTRL\n"
            "wait 1\n"
            "key key_space MDFR_CTRL\n",
            src_name, dst_name);
}

internal void
generate_capacity_stresser_1(void){
    FILE *out = try_open_output("gentest_capstress1.4is");
    if (out == 0){
        return;
    }
    
    fprintf(out, "mouse_xy 20 20\n");
    write_open_test_file_default_bindings(out,
                                          "small.cpp", "small.cpp");
    
    fprintf(out,
            "wait 1\n"
            "key g MDFR_CTRL\n"
            "wait 1\n"
            "key 6 MDFR_NONE\n"
            "wait 1\n"
            "key key_newline MDFR_NONE\n");
    
    for (i32 i = 0; i < 20; ++i){
        fprintf(out,
                "wait 1\n"
                "type 1 int\n"
                "wait 1\n"
                "key key_space MDFR_NONE\n"
                "wait 1\n"
                "type 1 foo%d\n"
                "wait 1\n"
                "key key_space MDFR_NONE\n"
                "wait 1\n"
                "key = MDFR_NONE\n"
                "wait 1\n"
                "key key_space MDFR_NONE\n"
                "wait 1\n"
                "type 1 %d;\n"
                "wait 1\n"
                "key key_newline MDFR_NONE\n",
                i, i);
    }
    
    fprintf(out, "exit\n");
    
    fclose(out);
}

int
main(int argc, char **argv){
    if (argc > 1){
        print_usage(argv[0]);
        exit(1);
    }
    
    generate_capacity_stresser_1();
    
    return(0);
}

// BOTTOM

