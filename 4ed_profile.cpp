/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 29.03.2017
 *
 * Really basic profiling primitives. (not meant to stay for long)
 *
 */

// TOP

#define FRED_INTERNAL 1

#include "4tech_defines.h"
#include "4ed_profile.h"

#include <stdio.h>
#include <stdlib.h>

global f32 frame_target = 58000000.f;

internal void
usage(){
    fprintf(stderr, "No! Like this, you moron:\n"
            "\t<PROFILE-EXE> <PROFILE-DATA> [min-time-filter]\n");
}

internal u32
parse_frame_count(u8 *ptr, u8 *end){
    u32 count = 0;
    
    for (;ptr < end;){
        u8 *frame_base = *(u8**)ptr;
        ptr += 8;
        
        u8 *frame_end = *(u8**)ptr;
        ptr += 8;
        
        umem skip_size = frame_end - frame_base;
        ptr += skip_size;
        ++count;
    }
    
    if (ptr != end){
        count = 0;
    }
    
    return(count);
}

struct Parse_Frame_Result{
    u8 *next_read_ptr;
    u8 *output_chunk;
    b32 still_looping;
    b32 bad_parse;
    f32 frame_time;
};

internal Parse_Frame_Result
parse_frame(u8 *ptr, u8 *end_ptr){
    Parse_Frame_Result result = {0};
    
    u8 *frame_base = *(u8**)ptr;
    ptr += 8;
    
    u8 *frame_end = *(u8**)ptr;
    ptr += 8;
    
    umem frame_size = frame_end - frame_base;
    u8 *frame_start_ptr = ptr;
    u8 *frame_end_ptr = ptr + frame_size;
    
    u8 *out_chunk = (u8*)malloc(frame_size*2);
    u8 *out_ptr = out_chunk;
    
    Profile_Group *group = (Profile_Group*)frame_start_ptr;
    Profile_Group *group_end = (Profile_Group*)frame_end_ptr;
    
    Profile_Group *stack[64];
    u32 top = 0;
    
    stack[top++] = group;
    
    result.frame_time = group->cycle_count / frame_target;
    
    for (;group < group_end;){
        for (u32 i = 1; i < top; ++i){
            *out_ptr++ = ' ';
        }
        
        char *name = group->name;
        for (u32 i = 0; name[i]; ++i){
            *out_ptr++ = name[i];
        }
        
        *out_ptr++ = ':';
        *out_ptr++ = ' ';
        
        char str[64];
        sprintf(str, "%f", group->cycle_count / frame_target);
        for (u32 i = 0; str[i]; ++i){
            *out_ptr++ = str[i];
        }
        *out_ptr++ = '\n';
        
        ++group;
        
        for(;top > 0;){
            Profile_Group *group_top = stack[top-1];
            umem end_offset = (u8*)group_top->end - frame_base;
            u8 *this_group_end_ptr = frame_start_ptr + end_offset;
            Profile_Group *this_group_end = (Profile_Group*)this_group_end_ptr;
            
            if (group == this_group_end){
                --top;
            }
            else{
                break;
            }
        }
        
        stack[top++] = group;
    }
    
    if (top != 1){
        result.bad_parse = true;
    }
    else{
        *out_ptr++ = 0;
        result.next_read_ptr = frame_end_ptr;
        result.output_chunk = out_chunk;
        if (frame_end_ptr != end_ptr){
            result.still_looping = true;
        }
    }
    
    return(result);
}

internal void
print_profile(char *filename, f32 min_filter){
    FILE *file = fopen(filename, "rb");
    if (!file){
        fprintf(stderr, "There ain't no file sittin' around called %s.\n", filename);
        exit(1);
    }
    
    fseek(file, 0, SEEK_END);
    u32 size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    u8 *buffer = (u8*)malloc(size);
    
    fread(buffer, 1, size, file);
    fclose(file);
    
    u8 *read_ptr = buffer;
    u8 *end_ptr = buffer + size;
    
    // Frame Count Parse
    u32 frame_count = parse_frame_count(read_ptr, end_ptr);
    
    if (frame_count == 0){
        fprintf(stderr, "There's some fricken problem. I didn't get a good frame count!\n");
        exit(1);
    }
    
    // Full Parse
    u8 **output_chunks = (u8**)malloc(frame_count*sizeof(u8*));
    u32 chunk_i = 0;
    
    Parse_Frame_Result result = {0};
    do{
        if (chunk_i >= frame_count){
            fprintf(stderr, "The parse ain't lined up! You're fired!\n");
            exit(1);
        }
        
        result = parse_frame(read_ptr, end_ptr);
        
        if (result.bad_parse){
            fprintf(stderr, "You've pickled the data nimwit!\n");
            exit(1);
        }
        
        read_ptr = result.next_read_ptr;
        if (result.frame_time >= min_filter){
            output_chunks[chunk_i++] = result.output_chunk;
        }
    }while(result.still_looping);
    
    // Print
    fprintf(stdout, "Frames: %u (%u)\n", chunk_i, frame_count);
    for (u32 i = 0; i < chunk_i; ++i){
        fprintf(stdout, "%s", output_chunks[i]);
    }
}

int main(int argc, char **argv){
    if (argc < 2){
        usage();
    }
    else{
        f32 min_filter = 0.f;
        if (argc == 3){
            min_filter = (f32)atof(argv[2]);
        }
        
        print_profile(argv[1], min_filter);
    }
    return(0);
}

// BOTTOM



