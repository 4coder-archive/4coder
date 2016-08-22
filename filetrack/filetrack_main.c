/*

Copy Right FourTech LLC, 2016
All Rights Are Reserved

A test bed for a cross platform file tracking reliability layer.
Developed for the use cases in 4coder, but I anticipate that this
will be a general problem for me. - Allen Webster

Created on: 20.07.2016

*/

// TOP

#define FILE_TRACK_MAIN

#include "4tech_file_track.h"
#include "4tech_file_track_win32.c"

#include "filetrack_test.c"

#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define FILE_TRACK_TEST_DIR1 "w:/filetrack/data/"
#define FILE_TRACK_TEST_DIR2 "w:/filetrack/data2/"

#define FAKE_TRACK_TEST_DIR "w:/filetrack/data1000/"

#define ALT_NAME_TEST_DIR1 "c:/work/filetrack/data/"
#define ALT_NAME_TEST_DIR2 "c:/work/filetrack/data2/"

static char * test_files[] = {
    FILE_TRACK_TEST_DIR1"autotab.cpp",
    FILE_TRACK_TEST_DIR1"basic.cpp",
    FILE_TRACK_TEST_DIR1"basic.txt",
    FILE_TRACK_TEST_DIR1"cleanme.cpp",
    FILE_TRACK_TEST_DIR1"emptyfile.txt",
    FILE_TRACK_TEST_DIR1"lexer_test.cpp",
    FILE_TRACK_TEST_DIR1"lexer_test2.cpp",
    FILE_TRACK_TEST_DIR1"lexer_test3.cpp",
    FILE_TRACK_TEST_DIR1"saveas.txt",
    FILE_TRACK_TEST_DIR1"test_large.cpp",
    
    FILE_TRACK_TEST_DIR2"autotab.cpp",
    FILE_TRACK_TEST_DIR2"basic.cpp",
    FILE_TRACK_TEST_DIR2"basic.txt",
    FILE_TRACK_TEST_DIR2"cleanme.cpp",
    FILE_TRACK_TEST_DIR2"emptyfile.txt",
    FILE_TRACK_TEST_DIR2"lexer_test.cpp",
    FILE_TRACK_TEST_DIR2"lexer_test2.cpp",
    FILE_TRACK_TEST_DIR2"lexer_test3.cpp",
    FILE_TRACK_TEST_DIR2"saveas.txt",
    FILE_TRACK_TEST_DIR2"test_large.cpp",
};

static char * test_alt_files[] = {
    ALT_NAME_TEST_DIR1"autotab.cpp",
    ALT_NAME_TEST_DIR1"basic.cpp",
    ALT_NAME_TEST_DIR1"basic.txt",
    ALT_NAME_TEST_DIR1"cleanme.cpp",
    ALT_NAME_TEST_DIR1"emptyfile.txt",
    ALT_NAME_TEST_DIR1"lexer_test.cpp",
    ALT_NAME_TEST_DIR1"lexer_test2.cpp",
    ALT_NAME_TEST_DIR1"lexer_test3.cpp",
    ALT_NAME_TEST_DIR1"saveas.txt",
    ALT_NAME_TEST_DIR1"test_large.cpp",
    
    ALT_NAME_TEST_DIR2"autotab.cpp",
    ALT_NAME_TEST_DIR2"basic.cpp",
    ALT_NAME_TEST_DIR2"basic.txt",
    ALT_NAME_TEST_DIR2"cleanme.cpp",
    ALT_NAME_TEST_DIR2"emptyfile.txt",
    ALT_NAME_TEST_DIR2"lexer_test.cpp",
    ALT_NAME_TEST_DIR2"lexer_test2.cpp",
    ALT_NAME_TEST_DIR2"lexer_test3.cpp",
    ALT_NAME_TEST_DIR2"saveas.txt",
    ALT_NAME_TEST_DIR2"test_large.cpp",
};

static char * fake_files[] = {
    FAKE_TRACK_TEST_DIR"autotab.cpp",
    FAKE_TRACK_TEST_DIR"basic.cpp",
    FAKE_TRACK_TEST_DIR"basic.txt",
    FAKE_TRACK_TEST_DIR"cleanme.cpp",
    FAKE_TRACK_TEST_DIR"emptyfile.txt",
    FAKE_TRACK_TEST_DIR"lexer_test.cpp",
    FAKE_TRACK_TEST_DIR"lexer_test2.cpp",
    FAKE_TRACK_TEST_DIR"lexer_test3.cpp",
    FAKE_TRACK_TEST_DIR"saveas.txt",
    FAKE_TRACK_TEST_DIR"test_large.cpp",
};

#define ArrayCount(a) ((sizeof(a))/(sizeof(*a)))

typedef struct{
    File_Index unique_file_index;
    File_Time time;
} MyFileThing;

void test_body_A(int32_t size1, int32_t size2){
    void *mem1 = malloc(size1);
    void *mem2 = malloc(size2);
    memset(mem1, 0, size1);
    
    File_Track_System track = {0};
    int32_t result = init_track_system(&track,
                                       mem1, size1,
                                       mem2, size2);
    assert(result == FileTrack_Good);
    
    MyFileThing my_file_things[1000];
    memset(my_file_things, 0, sizeof(my_file_things));
    
    // NOTE(allen): track in all the test files
    for (int32_t i = 0;
         i < ArrayCount(test_files);
         ++i){
        char *filename = test_files[i];
        
        File_Index new_file = zero_file_index();
        File_Time new_time = 0;
        int32_t result = begin_tracking_file(&track, filename, &new_file, &new_time);
        while (result != FileTrack_Good){
            
            switch (result){
                case FileTrack_OutOfTableMemory:
                {
                    int32_t new_mem_size = size1*2;
                    void *new_mem = malloc(new_mem_size);
                    
                    memset(new_mem, 0, new_mem_size);
                    move_track_system(&track, new_mem, new_mem_size);
                    
                    free(mem1);
                    size1 = new_mem_size;
                    mem1 = new_mem;
                }break;
                
                case FileTrack_OutOfListenerMemory:
                {
                    size2 *= 2;
                    void *new_mem = malloc(size2);
                    memset(new_mem, 0, size2);
                    expand_track_system_listeners(&track, new_mem, size2);
                }break;
                
                default:
                {
                    Assert(result == FileTrack_Good);
                }break;
            }
            
            result = begin_tracking_file(&track, filename, &new_file, &new_time);
        }
        
        my_file_things[i].unique_file_index = new_file;
        my_file_things[i].time = new_time;
    }
    
    // NOTE(allen): track in fake directories
    for (int32_t i = 0;
         i < ArrayCount(fake_files);
         ++i){
        File_Index new_file = zero_file_index();
        File_Time new_time = 0;
        
        char *filename = fake_files[i];
        
        int32_t result = begin_tracking_file(&track, filename, &new_file, &new_time);
        assert(result == FileTrack_FileNotFound);
    }
    
    // NOTE(allen): track in already tracked files
    for (int32_t i = 0;
         i < ArrayCount(test_files);
         ++i){
        File_Index new_file = zero_file_index();
        File_Time new_time = 0;
        
        char *filename = test_files[i];
        
        int32_t result = begin_tracking_file(&track, filename, &new_file, &new_time);
        assert(result == FileTrack_FileAlreadyTracked);
    }
    
    // NOTE(allen): track in already tracked files via alt-names
    for (int32_t i = 0;
         i < ArrayCount(test_alt_files);
         ++i){
        File_Index new_file = zero_file_index();
        File_Time new_time = 0;
        
        char *filename = test_alt_files[i];
        
        int32_t result = begin_tracking_file(&track, filename, &new_file, &new_time);
        assert(result == FileTrack_FileAlreadyTracked);
    }
    
    // NOTE(allen): each file is still up to date
    for (int32_t i = 0;
         i < ArrayCount(test_files);
         ++i){
        File_Time time = 0;
        File_Index index = my_file_things[i].unique_file_index;
        
        get_tracked_file_time(&track, index, &time);
        
        assert(time == my_file_things[i].time);
    }
    
    // NOTE(allen): can still get index from file name
    for (int32_t i = 0;
         i < ArrayCount(test_files);
         ++i){
        File_Index index = my_file_things[i].unique_file_index;
        
        File_Index result_index1 = zero_file_index();
        char *filename1 = test_files[i];
        get_tracked_file_index(&track, filename1, &result_index1);
        
        File_Index result_index2 = zero_file_index();
        char *filename2 = test_alt_files[i];
        get_tracked_file_index(&track, filename2, &result_index2);
        
        assert(file_index_eq(result_index1, index));
        assert(file_index_eq(result_index2, index));
    }
    
    // NOTE(allen): rewrite all of the files
    for (int32_t i = 0;
         i < ArrayCount(test_files);
         ++i){
        char *filename = test_files[i];
        
        test_rewrite_file_in_child_proc(filename);
    }
    
    // NOTE(allen): each file is behind
    for (int32_t i = 0;
         i < ArrayCount(test_files);
         ++i){
        File_Time time = 0;
        File_Index index = my_file_things[i].unique_file_index;
        
        get_tracked_file_time(&track, index, &time);
        
        assert(my_file_things[i].time < time);
    }
    
    // NOTE(allen): poll the tracking system for changed files
    for (;;){
        File_Index index = zero_file_index();
        File_Time time = 0;
        
        int32_t result = get_change_event(&track, &index);
        
        if (result == FileTrack_NoMoreEvents){
            break;
        }
        
        result = get_tracked_file_time(&track, index, &time);
        
        for (int32_t i = 0;
             i < ArrayCount(test_files);
             ++i){
            File_Index my_index = my_file_things[i].unique_file_index;
            if (file_index_eq(my_index, index)){
                my_file_things[i].time = time;
                break;
            }
        }
    }
    
    // NOTE(allen): each file is still up to date (episode 2)
    for (int32_t i = 0;
         i < ArrayCount(test_files);
         ++i){
        File_Time time = 0;
        File_Index index = my_file_things[i].unique_file_index;
        
        get_tracked_file_time(&track, index, &time);
        
        assert(time == my_file_things[i].time);
    }
    
    // NOTE(allen): rewrite each file myself
    for (int32_t i = 0;
         i < ArrayCount(test_files);
         ++i){
        File_Index index = my_file_things[i].unique_file_index;
        File_Time time = test_rewrite_file(&track, index);
        
        my_file_things[i].time = time;
    }
    
    // NOTE(allen): check there are no changed file events
    {
        File_Index index = zero_file_index();
        
        int32_t result = get_change_event(&track, &index);
        assert(result == FileTrack_NoMoreEvents);
    }
    
    // NOTE(allen): rewrite half of the files twice
    int32_t mid_point = ArrayCount(test_files) / 2;
    for (int32_t i = 0;
         i < mid_point;
         ++i){
        char *filename = test_files[i];
        
        test_rewrite_file_in_child_proc(filename);
    }
    
    for (int32_t i = 0;
         i < mid_point;
         ++i){
        char *filename = test_files[i];
        
        test_rewrite_file_in_child_proc(filename);
    }
    
    // NOTE(allen): check number of events equals mid_point
    int32_t count = 0;
    for (;;){
        File_Index index = zero_file_index();
        File_Time time = 0;
        
        int32_t result = get_change_event(&track, &index);
        
        if (result == FileTrack_NoMoreEvents){
            break;
        }
        
        result = get_tracked_file_time(&track, index, &time);
        
        ++count;
        
        for (int32_t i = 0;
             i < ArrayCount(test_files);
             ++i){
            File_Index my_index = my_file_things[i].unique_file_index;
            if (file_index_eq(my_index, index)){
                my_file_things[i].time = time;
                break;
            }
        }
    }
    assert(count == mid_point);
    
    // NOTE(allen): untrack half of the files
    for (int32_t i = 0;
         i < mid_point;
         ++i){
        File_Index stop_file = my_file_things[i].unique_file_index;
        stop_tracking_file(&track, stop_file);
    }
    
    // NOTE(allen): untrack the same files again
    for (int32_t i = 0;
         i < mid_point;
         ++i){
        File_Index stop_file = my_file_things[i].unique_file_index;
        int32_t result = stop_tracking_file(&track, stop_file);
        assert(result == FileTrack_FileNotTracked);
    }
    
    // NOTE(allen): make sure the number of remaining files is correct
    {
        int32_t track_count = 0;
        count_tracked_files(&track, &track_count);
        assert(track_count == (ArrayCount(test_files) - mid_point));
    }
    
    // NOTE(allen): untrack the rest of the files
    for (int32_t i = mid_point;
         i < ArrayCount(test_files);
         ++i){
        File_Index stop_file = my_file_things[i].unique_file_index;
        stop_tracking_file(&track, stop_file);
    }
    
    // NOTE(allen): make sure the system is empty
    {
        int32_t track_count = 0;
        count_tracked_files(&track, &track_count);
        assert(track_count == 0);
    }
    
    // NOTE(allen): finish using the track system
    {
        int32_t result = shut_down_track_system(&track);
        assert(result == FileTrack_Good);
    }
}
                 

// NOTE(allen): test basic tracking logic
void test_1(void){
    int32_t size1 = (16 << 10);
    int32_t size2 = (16 << 10);
    test_body_A(size1, size2);
}

// NOTE(allen): test memory expansion system for tables
void test_2(void){
    int32_t size1 = (1 << 10);
    int32_t size2 = (16 << 10);
    test_body_A(size1, size2);
}

// NOTE(allen): test memory expansion system for listening nodes
void test_3(void){
    int32_t size1 = (16 << 10);
    int32_t size2 = (5 << 10);
    test_body_A(size1, size2);
}

// NOTE(allen): test both memory expansion systems
void test_4(void){
    int32_t size1 = (1 << 10);
    int32_t size2 = (5 << 10);
    test_body_A(size1, size2);
}

int main(int argc, char **argv){
    test_4();
    return(0);
}

// BOTTOM

