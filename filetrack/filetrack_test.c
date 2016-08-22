/*

Copy Right FourTech LLC, 2016
All Rights Are Reserved

Helpers for the filetrack_main.c test bed.

Created on: 20.07.2016

*/

// TOP

#define FILE_REWRITER "w:/filetrack/build/file_rewriter"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

static void
rewrite(char *buffer, int32_t size){
    for (int32_t i = 0;
         i < size;
         ++i){
        if (buffer[i] >= 'a' && buffer[i] < 'z'){
            ++buffer[i];
        }
        else if (buffer[i] == 'z'){
            buffer[i] = 'a';
        }
    }
}

static void
append(char *buffer, int32_t *pos, char *src){
    int32_t i = *pos;
    src -= i;
    for (; src[i]; ++i){
        buffer[i] = src[i];
    }
    *pos = i;
}

static void
test_rewrite_file_in_child_proc(char *filename){
    char space[2048];
    int32_t pos = 0;
    
    append(space, &pos, FILE_REWRITER" ");
    append(space, &pos, filename);
    space[pos] = 0;
    
    int32_t result = system(space);
    assert(result == 0);
}

#ifndef FILE_TRACK_MAIN

int
main(int argc, char **argv){
    if (argc == 2){
        char *filename = argv[1];
        
        char *mem = 0;
        int32_t size = 0;
        
        FILE *file = fopen(filename, "rb");
        assert(file);
        fseek(file, 0, SEEK_END);
        size = ftell(file);
        fseek(file, 0, SEEK_SET);
        mem = (char*)malloc(size+1);
        fread(mem, 1, size, file);
        fclose(file);
        
        rewrite(mem, size);
        
        file = fopen(filename, "wb");
        assert(file);
        fwrite(mem, 1, size, file);
        fclose(file);
    }
    return(0);
}

#else

static File_Time
test_rewrite_file(File_Track_System *system, File_Index index){
    char *mem = 0;
    uint32_t size = 0;
    int32_t result = 0;
    
    result = get_tracked_file_size(system, index, &size);
    assert(result == FileTrack_Good);
    mem = (char*)malloc(size+1);
    result = get_tracked_file_data(system, index, mem, size);
    assert(result == FileTrack_Good);
    
    rewrite(mem, size);
    
    File_Time time = 0;
    rewrite_tracked_file(system, index, mem, size, &time);
    
    free(mem);
    
    return(time);
}

#endif

// BOTTOM

