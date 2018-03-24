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
#include "4coder_lib/4coder_mem.h"

#include "4coder_file.h"

#include <stdio.h>

////////////////////////////////

global char hot_directory_space[4096];
global String hot_directory = {hot_directory_space, 0, sizeof(hot_directory_space)};

internal void
init_hot_directory(char *dir){
    copy(&hot_directory, dir);
    replace_char(&hot_directory, '\\', '/');
    if (hot_directory.str[hot_directory.size - 1] != '/'){
        append(&hot_directory, "/");
    }
}

internal void
set_hot_directory(String str){
    copy(&hot_directory, str);
}

internal void
set_hot_directory(char *str){
    copy(&hot_directory, str);
}

internal void
push_folder_hot_directory(String str){
    append(&hot_directory, str);
    append(&hot_directory, "/");
}

internal void
push_folder_hot_directory(char *str){
    append(&hot_directory, str);
    append(&hot_directory, "/");
}

internal void
pop_folder_hot_directory(void){
    remove_last_folder(&hot_directory);
}

internal String
get_hot_directory(Partition *part){
    String hot;
    hot.str = push_array(part, char, hot_directory.size);
    hot.size = hot_directory.size;
    hot.memory_size = hot_directory.size;
    memcpy(hot.str, hot_directory.str, hot_directory.size);
    return(hot);
}

internal FILE*
fopen_hot_directory(Partition *scratch, char *file_name, char *flags){
    Temp_Memory temp = begin_temp_memory(scratch);
    
    char *full_name = push_array(scratch, char, hot_directory.size);
    memcpy(full_name, hot_directory.str, hot_directory.size);
    
    i32 file_name_length = str_size(file_name);
    char *full_name_file_portion = push_array(scratch, char, file_name_length);
    memcpy(full_name_file_portion, file_name, file_name_length);
    
    char *terminator = push_array(scratch, char, 1);
    *terminator = 0;
    
    FILE *result = fopen(full_name, flags);
    
    end_temp_memory(temp);
    
    return(result);
}

////////////////////////////////

struct Test_Node{
    Test_Node *next;
    String name;
};

struct Test_List{
    Partition *part;
    
    Test_Node *first;
    Test_Node *last;
    i32 count;
};

#define zdll_push(f,l,cptr,n) (((f) == 0)?((f) = (l) = (n)):((l)->next = (n), (l) = (n))), (*(cptr)) += 1

internal void
push_test(Test_List *list, String name){
    Test_Node *node = push_array(list->part, Test_Node, 1);
    node->name = make_string_cap(push_array(list->part, char, name.size), 0, name.size);
    push_align(list->part, 8);
    copy(&node->name, name);
    zdll_push(list->first, list->last, &list->count, node);
}

internal void
push_test(Test_List *list, char *name){
    push_test(list, make_string_slowly(name));
}

////////////////////////////////

global String code_root;
global String script_root;
global String sample_root;

typedef u32 Generate_Flag;
enum{
    GenFlag_RebuildSamples = 1,
    GenFlag_RebuildScripts = 2,
    GenFlag_OutputTestNames = 4,
};
enum{
    GenFlag_DoAll = GenFlag_RebuildSamples|GenFlag_RebuildScripts|GenFlag_OutputTestNames,
};

#define DoSamples(f) (((f) & GenFlag_RebuildSamples) != 0)
#define DoScripts(f) (((f) & GenFlag_RebuildScripts) != 0)
#define DoTestNames(f) (((f) & GenFlag_OutputTestNames) != 0)

internal void
print_usage(char *name){
    fprintf(stdout, "usage: %s code-root-directory\n", name);
}

internal FILE*
try_open_output(Partition *scratch, char *name){
    FILE *out = fopen_hot_directory(scratch, name, "wb");
    if (out == 0){
        fprintf(stdout, "Could not open output file %s\n", name);
    }
    return(out);
}

internal void
generate_run_script(Partition *scratch, Test_List list){
    Temp_Memory temp = begin_temp_memory(scratch);
    
    set_hot_directory(code_root);
    FILE *out = try_open_output(scratch, "run_regression_tests.bat");
    if (out != 0){
        fprintf(out,
                "@echo off\n"
                
                "pushd ..\\4coder-non-source\\test_data\n"
                "set run_path=%%cd%%\\sample_files\n"
                "set data_path=%%cd%%\\input_data\n"
                "popd\n"
                
                "pushd ..\\build\n"
                "set build=%%cd%%\n"
                "popd\n"
                
                "pushd %%run_path%%\n");
        
        for (Test_Node *node = list.first;
             node != 0;
             node = node->next){
            fprintf(out, "%%build%%\\4ed -T %%data_path%%\\%.*s\n",
                    node->name.size, node->name.str);
        }
        
        fprintf(out, "popd\n");
        fclose(out);
    }
    
    end_temp_memory(temp);
}

internal void
write_open_test_file_default_bindings(FILE *out,
                                      char *src_name, char *dst_name){
    fprintf(out,
            "key o MDFR_CTRL\n"
            "type 1 %s\n"
            "key key_newline MDFR_NONE\n"
            "key o MDFR_CTRL\n"
            "key key_back MDFR_NONE\n"
            "type 1 output/\n"
            "key key_esc MDFR_NONE\n"
            "key s MDFR_ALT\n"
            "type 1 %s\n"
            "key key_newline MDFR_NONE\n",
            src_name, dst_name);
}

internal String
generate_token_test_sample_file(Partition *part, i32 index, i32 token_target, Generate_Flag flags){
    i32 name_cap = 512;
    String sample_name = make_string_cap(push_array(part, char, name_cap), 0, name_cap);
    append(&sample_name, "gentokentest");
    append_int_to_str(&sample_name, index + 1);
    append(&sample_name, ".cpp");
    bool32 string_build_success = terminate_with_null(&sample_name);
    Assert(string_build_success);
    
    set_hot_directory(sample_root);
    if (DoSamples(flags)){
        FILE *out = try_open_output(part, sample_name.str);
        if (out != 0){
            fprintf(out,
                    "int foo(){\n"
                    "\n");
            i32 token_count = 6;
            Assert(token_count < token_target);
            for (;token_count + 10 < token_target;){
                fprintf(out, "int x = 0;\n");
                token_count += 5;
            }
            Assert(token_count < token_target);
            fprintf(out, "}\n");
            fclose(out);
        }
        else{
            sample_name.str = 0;
            sample_name.size = 0;
            sample_name.memory_size = 0;
        }
    }
    
    return(sample_name);
}

internal void
generate_token_tests(Partition *scratch, Test_List *test_list, Generate_Flag flags){
    Temp_Memory temp = begin_temp_memory(scratch);
    
    for (i32 size = 1024, i = 0; i < 5; size <<= 1, ++i){
        char test_name_space[512];
        String test_name = make_fixed_width_string(test_name_space);
        append(&test_name, "gentest_capstress");
        append_int_to_str(&test_name, i + 1);
        append(&test_name, ".4is");
        bool32 string_build_success = terminate_with_null(&test_name);
        Assert(string_build_success);
        
        String sample_name = generate_token_test_sample_file(scratch, i, size, flags);
        
        if (sample_name.str != 0){
            set_hot_directory(script_root);
            if (DoScripts(flags)){
                FILE *out = try_open_output(scratch, test_name.str);
                if (out != 0){
                    fprintf(out,
                            "mouse_xy 20 20\n"
                            "key P MDFR_CTRL\n"
                            "basewait 1\n");
                    write_open_test_file_default_bindings(out, sample_name.str, sample_name.str);
                    
                    fprintf(out, "key key_down MDFR_CTRL\n");
                    
                    for (i32 i = 0; i < 5; ++i){
                        fprintf(out,
                                "type 1 int x = 0;\n"
                                "key key_newline MDFR_NONE\n");
                    }
                    
                    fprintf(out,
                            "key s MDFR_CTRL\n"
                            "exit\n"
                            "key Y MDFR_NONE");
                    
                    fclose(out);
                }
            }
            
            if (DoTestNames(flags)){
                remove_extension(&test_name);
                append(&test_name, "4id");
                bool32 string_build_success = terminate_with_null(&test_name);
                Assert(string_build_success);
                push_test(test_list, test_name);
            }
        }
    }
    
    end_temp_memory(temp);
}

internal String
generate_dupline_test_sample_file(Partition *part, i32 line_count, bool32 always_newline, Generate_Flag flags){
    i32 name_cap = 512;
    String sample_name = make_string_cap(push_array(part, char, name_cap), 0, name_cap);
    append(&sample_name, "genduplinetest");
    append_int_to_str(&sample_name, line_count);
    append(&sample_name, "_");
    append_int_to_str(&sample_name, always_newline);
    append(&sample_name, ".cpp");
    bool32 string_build_success = terminate_with_null(&sample_name);
    Assert(string_build_success);
    
    if (DoSamples(flags)){
        set_hot_directory(sample_root);
        FILE *out = try_open_output(part, sample_name.str);
        if (out != 0){
            for (i32 i = 0; i < line_count; ++i){
                fprintf(out, "abcd");
                if (i + 1 < line_count || always_newline){
                    fprintf(out, "\n");
                }
            }
            fclose(out);
        }
    }
    
    return(sample_name);
}

internal void
generate_dupline_specific_test(Partition *scratch, Test_List *test_list, Generate_Flag flags,
                               i32 line_count, bool32 always_newline, bool32 read_only){
    char test_name_space[512];
    String test_name = make_fixed_width_string(test_name_space);
    if (read_only){
        append(&test_name, "gentest_dupline_readonly.4is");
    }
    else{
        append(&test_name, "gentest_dupline");
        append_int_to_str(&test_name, line_count);
        append(&test_name, "_");
        append_int_to_str(&test_name, always_newline);
        append(&test_name, ".4is");
    }
    bool32 string_build_success = terminate_with_null(&test_name);
    Assert(string_build_success);
    
    String sample_name;
    if (read_only){
        sample_name = make_lit_string("*messages*");
    }
    else{
        sample_name = generate_dupline_test_sample_file(scratch, line_count, always_newline, flags);
    }
    
    if (sample_name.str != 0){
        set_hot_directory(script_root);
        if (DoScripts(flags)){
            FILE *out = try_open_output(scratch, test_name.str);
            if (out != 0){
                fprintf(out,
                        "mouse_xy 20 20\n"
                        "key P MDFR_CTRL\n"
                        "basewait 1\n");
                if (read_only){
                    fprintf(out,
                            "key i MDFR_CTRL\n"
                            "type 1 *messages*\n"
                            "key key_newline MDFR_NONE\n"
                            "key L MDFR_CTRL\n"
                            "key s MDFR_CTRL\n"
                            "exit\n");
                }
                else{
                    write_open_test_file_default_bindings(out, sample_name.str, sample_name.str);
                    fprintf(out,
                            "key L MDFR_CTRL\n"
                            "key s MDFR_CTRL\n"
                            "exit\n");
                }
                fclose(out);
            }
        }
        
        if (DoTestNames(flags)){
            remove_extension(&test_name);
            append(&test_name, "4id");
            bool32 string_build_success = terminate_with_null(&test_name);
            Assert(string_build_success);
            push_test(test_list, test_name);
        }
    }
}

internal void
generate_dupline_tests(Partition *scratch, Test_List *test_list, Generate_Flag flags){
    Temp_Memory temp = begin_temp_memory(scratch);
    for (i32 line_count = 0; line_count < 2; ++line_count){
        for (bool32 always_newline = 0; always_newline <= 1; ++always_newline){
            generate_dupline_specific_test(scratch, test_list, flags,
                                           line_count, always_newline, false);
        }
    }
    generate_dupline_specific_test(scratch, test_list, flags, 0, false, true);
    end_temp_memory(temp);
}

int
main(int argc, char **argv){
    if (argc != 2){
        print_usage(argv[0]);
        exit(1);
    }
    
    // NOTE(allen): Init the hot directory
    init_hot_directory(argv[1]);
    
    // NOTE(allen): Init the partition
    i32 memory_size = MB(8);
    Partition part_ = make_part(malloc(memory_size), memory_size);
    Partition *part = &part_;
    
    // NOTE(allen): Get various root paths
    code_root = get_hot_directory(part);
    
    push_folder_hot_directory("test_input_scripts/generated");
    script_root = get_hot_directory(part);
    
    set_hot_directory(code_root);
    pop_folder_hot_directory();
    push_folder_hot_directory("4coder-non-source/test_data/sample_files");
    sample_root = get_hot_directory(part);
    
    // NOTE(allen): Setup the test list
    i32 test_list_size = MB(8);
    Partition test_list_part = make_part(malloc(test_list_size), test_list_size);
    
    Test_List test_list = {0};
    test_list.part = &test_list_part;
    
    // NOTE(allen): Tests
    push_test(&test_list, "test_load_FONT_COURIER_NEW_28_c.4id");
    generate_token_tests(part, &test_list, GenFlag_DoAll);
    //generate_token_tests(part, &test_list, GenFlag_OutputTestNames);
    generate_dupline_tests(part, &test_list, GenFlag_DoAll);
    //generate_dupline_tests(part, &test_list, GenFlag_OutputTestNames);
    
    // NOTE(allen): Generate the run test script
    generate_run_script(part, test_list);
    
    return(0);
}

// BOTTOM

