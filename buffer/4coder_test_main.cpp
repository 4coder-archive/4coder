/* 
 * Mr. 4th Dimention - Allen Webster
 *  Four Tech
 *
 * public domain -- no warranty is offered or implied; use this code at your own risk
 * 
 * 06.11.2015
 * 
 * Buffer experiment testing layer
 * 
 */

// TOP

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#define inline_4tech inline

inline_4tech int
CEIL32(float x){
    int extra;
    extra = ((x!=(int)(x) && x>0)?1:0);
    extra += (int)(x);
    return(extra);
}

inline_4tech int
DIVCEIL32(int n, int d) {
    int q = (n/d);
    q += (q*d < n);
    return(q);
}

inline_4tech unsigned int
ROUNDPOT32(unsigned int v){
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return(v);
}

#ifdef fast_test
#define debug_4tech(x)
#define assert_4tech(x)
#endif
#define hard_assert_4tech(x) assert(x)

#ifdef __linux__
#define memzero_4tech(x) memset_4tech(&(x), 0, sizeof(x))
#endif

#include "4coder_shared.cpp"
#include "4coder_golden_array.cpp"
#include "4coder_gap_buffer.cpp"
#include "4coder_multi_gap_buffer.cpp"
#include "4coder_rope_buffer.cpp"

#define Buffer_Type Buffer
#include "4coder_buffer_abstract.cpp"
#undef Buffer_Type

#define Buffer_Type Gap_Buffer
#include "4coder_buffer_abstract.cpp"
#undef Buffer_Type

#define Buffer_Type Multi_Gap_Buffer
#include "4coder_buffer_abstract.cpp"
#undef Buffer_Type

#define Buffer_Type Rope_Buffer
#include "4coder_buffer_abstract.cpp"
#undef Buffer_Type

#if defined(_WIN32)
#include <Windows.h>

typedef unsigned long long time_int;

unsigned long long win32_counts_per_second_4tech;

int time_init(unsigned long long *resolution){
    int result;
    LARGE_INTEGER time;
    result = 0;
    if (QueryPerformanceFrequency(&time)){
        win32_counts_per_second_4tech = (unsigned long long)(time.QuadPart);
        result = 1;
        *resolution = win32_counts_per_second_4tech;
    }
    return(result);
}

time_int get_time(){
    LARGE_INTEGER time;
    time_int result;
    
    result = 0;
    if (QueryPerformanceCounter(&time)){
        result = (time_int)(time.QuadPart);
        result = result * 1000000 / win32_counts_per_second_4tech;
    }
    
    return(result);
}

#elif defined(__linux__)
#include <time.h>

typedef unsigned long long time_int;

int time_init(unsigned long long *resolution){
    int result;
    struct timespec res;
    result = 0;
    
    if (!clock_getres(CLOCK_MONOTONIC, &res)){
        result = 1;
	if (res.tv_sec > 0 || res.tv_nsec == 0) *resolution = 0;
	else *resolution = (unsigned long long)(1000000/res.tv_nsec);
    }

    return(result);
}

time_int get_time(){
    time_int result;
    struct timespec time;
    
    result = 0;
    if (!clock_gettime(CLOCK_MONOTONIC, &time)){
        result = (time.tv_sec * 1000000) + (time.tv_nsec / 1000);
    }
    
    return(result);
}

#else
#error Timer not supported on this platform
#endif

typedef struct File_Data{
    char *data;
    int size;
} File_Data;

File_Data get_file(const char *filename){
    FILE *file;
    File_Data result;
    
    file = fopen(filename, "rb");
    if (!file){
        printf("error: could not find file %s\n", filename);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    result.size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (result.size == 0){        
        printf("error: file %s was empty\n", filename);
        exit(1);
    }
    
    result.data = (char*)malloc(result.size);
    fread(result.data, result.size, 1, file);

    fclose(file);
    
    return(result);
}

void free_file(File_Data file){
    free(file.data);
}

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

float* get_font_data(const char *font_file){
    float *data = 0;
    stbtt_bakedchar *baked;
    File_Data file = get_file(font_file);
    int stride, offset;

    if (file.data){
        int size = sizeof(*baked)*256;
        baked = (stbtt_bakedchar*)malloc(size);
        memset_4tech(baked, 0, sizeof(*baked)*256);
    
        offset = (int)((char*)&baked->xadvance - (char*)baked);
        stride = sizeof(*baked);

        int w, h;
        w = 10*256;
        h = 25;
        unsigned char *pixels = (unsigned char*)malloc(w * h);
        stbtt_BakeFontBitmap((unsigned char*)file.data, 0, 17.f, pixels, w, h, 0, 128, baked);
        free(pixels);
        free_file(file);

        data = (float*)malloc(sizeof(float)*256);
        memset_4tech(data, 0, sizeof(float)*256);
        
        char *pos = (char*)baked;
        pos += offset;
        for (int i = 0; i < 128; ++i){
            data[i] = *(float*)pos;
            pos += stride;
        }
        free(baked);
    }
    else{
        printf("error: cannot continue without font\n");
    }
    
    return data;
}

void setup(){
    unsigned long long resolution;
    if (!time_init(&resolution)){
        printf("error: could not initialize timer");
        exit(1);
    }

    if (resolution < 1000000)
        printf("warning: timer is not actually at high enough resolution for good measurements!\n");

}

typedef struct Time_Record{
    time_int buffer;
    time_int gap_buffer;
    time_int multi_gap_buffer;
    time_int rope_buffer;
} Time_Record;

typedef struct Record_Statistics{
    Time_Record max, min;
    Time_Record expected;
    int count;
} Record_Statistics;

Time_Record
operator+(const Time_Record &a, const Time_Record &b){
    Time_Record r;
    r.buffer = a.buffer + b.buffer;
    r.gap_buffer = a.gap_buffer + b.gap_buffer;
    r.multi_gap_buffer = a.multi_gap_buffer + b.multi_gap_buffer;
    r.rope_buffer = a.rope_buffer + b.rope_buffer;
    return r;
}

#define minify(a,b) if ((a)>(b)) (a) = (b)
#define maxify(a,b) if ((a)<(b)) (a) = (b)

void
get_record_statistics(Record_Statistics *stats_out, Time_Record *records, int count){
    Record_Statistics stats;
    stats.max = records[0];
    stats.min = records[0];
    stats.expected = records[0];
    stats.count = count;

    Time_Record *record = records + 1;
    
    for (int i = 1; i < count; ++i, ++record){
        stats.expected = stats.expected + *record;
        
        minify(stats.min.buffer, record->buffer);
        minify(stats.min.gap_buffer, record->gap_buffer);
        minify(stats.min.multi_gap_buffer, record->multi_gap_buffer);
        minify(stats.min.rope_buffer, record->rope_buffer);
        
        maxify(stats.max.buffer, record->buffer);
        maxify(stats.max.gap_buffer, record->gap_buffer);
        maxify(stats.max.multi_gap_buffer, record->multi_gap_buffer);
        maxify(stats.max.rope_buffer, record->rope_buffer);
    }

    stats.expected.buffer /= count;
    stats.expected.gap_buffer /= count;
    stats.expected.multi_gap_buffer /= count;
    stats.expected.rope_buffer /= count;
    
    *stats_out = stats;
}

int test_is_silenced;

void
silence_test(){
    test_is_silenced = 1;
}

void
print_record(Time_Record record){
    printf("%-16s - %25lluus\n%-16s - %25lluus\n%-16s - %25lluus\n%-16s - %25lluus\n",
           "Golden Array", record.buffer,
           "Gap Buffer", record.gap_buffer,
           "Multi-Gap Buffer", record.multi_gap_buffer,
           "Rope", record.rope_buffer);
}

void
print_statistics(Time_Record *records, int count, Record_Statistics *stats_out){
    Record_Statistics stats;
    get_record_statistics(&stats, records, count);
    if (!test_is_silenced){
        printf("samples: %d\n", count);
        printf("---averages---\n");
        print_record(stats.expected);
        printf("---max---\n");
        print_record(stats.max);
        printf("---min---\n");
        print_record(stats.min);
    }

    if (stats_out) *stats_out = stats;
}

typedef struct Buffer_Set{
    Buffer buffer;
    Gap_Buffer gap_buffer;
    Multi_Gap_Buffer multi_gap_buffer;
    Rope_Buffer rope_buffer;
} Buffer_Set;

#define Buffer_Type Buffer
#include "4coder_test_abstract.cpp"
#undef Buffer_Type

#define Buffer_Type Gap_Buffer
#include "4coder_test_abstract.cpp"
#undef Buffer_Type

#define Buffer_Type Multi_Gap_Buffer
#include "4coder_test_abstract.cpp"
#undef Buffer_Type

#define Buffer_Type Rope_Buffer
#include "4coder_test_abstract.cpp"
#undef Buffer_Type

#define print_name() printf("%s:\n", __FUNCTION__)

void
initialization_test(Buffer_Set *set, File_Data file, int test_repitions,
                    void *scratch, int scratch_size, Record_Statistics *stats_out){
    time_int tstart, tend;
    Time_Record *init_time = (Time_Record*)scratch;
    scratch = init_time + test_repitions;
    assert_4tech(test_repitions*sizeof(*init_time) < scratch_size);
    scratch_size -= test_repitions*sizeof(*init_time);
    
    for (int i = 0; i < test_repitions; ++i){
        tstart = get_time();
        init_buffer(&set->buffer, file, scratch, scratch_size);
        tend = get_time();
        init_time[i].buffer = tend - tstart;
    
        tstart = get_time();
        init_buffer(&set->gap_buffer, file, scratch, scratch_size);
        tend = get_time();
        init_time[i].gap_buffer = tend - tstart;
    
        tstart = get_time();
        init_buffer(&set->multi_gap_buffer, file, scratch, scratch_size);
        tend = get_time();
        init_time[i].multi_gap_buffer = tend - tstart;
    
        tstart = get_time();
        init_buffer(&set->rope_buffer, file, scratch, scratch_size);
        tend = get_time();
        init_time[i].rope_buffer = tend - tstart;

        if (i+1 != test_repitions){
            free(set->buffer.data);
            free(set->gap_buffer.data);
            for (int j = 0; j < set->multi_gap_buffer.chunk_alloced; ++j){
                free(set->multi_gap_buffer.gaps[j].data);
            }
            free(set->multi_gap_buffer.gaps);
            free(set->rope_buffer.data);
            free(set->rope_buffer.nodes);
        }
    }

    if (!test_is_silenced) print_name();
    print_statistics(init_time, test_repitions, stats_out);
    if (!test_is_silenced) printf("\n");
    test_is_silenced = 0;
}

void
measure_starts_widths_test(Buffer_Set *set, int test_repitions,
                           void *scratch, int scratch_size, Record_Statistics *stats_out,
                           float *font_widths){
    time_int tstart, tend;
    Time_Record *measure_time = (Time_Record*)scratch;
    scratch = measure_time + test_repitions;
    assert_4tech(test_repitions*sizeof(*measure_time) < scratch_size);
    scratch_size -= test_repitions*sizeof(*measure_time);
    
    for (int i = 0; i < test_repitions; ++i){
        tstart = get_time();
        measure_starts_widths(&set->buffer, font_widths);
        tend = get_time();
        measure_time[i].buffer = tend - tstart;
    
        tstart = get_time();
        measure_starts_widths(&set->gap_buffer, font_widths);
        tend = get_time();
        measure_time[i].gap_buffer = tend - tstart;
    
        tstart = get_time();
        measure_starts_widths(&set->multi_gap_buffer, font_widths);
        tend = get_time();
        measure_time[i].multi_gap_buffer = tend - tstart;
    
        tstart = get_time();
        measure_starts_widths(&set->rope_buffer, font_widths);
        tend = get_time();
        measure_time[i].rope_buffer = tend - tstart;

        if (i+1 != test_repitions){
            free(set->buffer.line_starts);
            free(set->gap_buffer.line_starts);
            free(set->multi_gap_buffer.line_starts);
            free(set->rope_buffer.line_starts);
            
            free(set->buffer.line_widths);
            free(set->gap_buffer.line_widths);
            free(set->multi_gap_buffer.line_widths);
            free(set->rope_buffer.line_widths);
        }
    }

    if (!test_is_silenced) print_name();
    print_statistics(measure_time, test_repitions, stats_out);
    if (!test_is_silenced) printf("\n");
    test_is_silenced = 0;
}

int
page_compare(char *page_1, char *page_2, int page_size){
    int result = 1;
    for (int i = 0; i < page_size; ++i){
        hard_assert_4tech(page_1[i] == page_2[i]);
    }
    return result;
}

void
stream_check_test(Buffer_Set *buffers, void *scratch, int scratch_size){
    int i, page_size, size;
    
    size = buffer_size(&buffers->buffer);
    {
        int size2;
        size2 = buffer_size(&buffers->gap_buffer);
        hard_assert_4tech(size == size2);
        size2 = buffer_size(&buffers->multi_gap_buffer);
        hard_assert_4tech(size == size2);
        size2 = buffer_size(&buffers->rope_buffer);
        hard_assert_4tech(size == size2);
    }

    page_size = 1 << 10;

    char *page_1 = (char*)scratch;
    char *page_2 = page_1 + page_size;
    scratch_size -= page_size*2;
    hard_assert_4tech(scratch_size > 0);
    
    for (i = 0; i < size; i += page_size){
        int end = i + page_size;
        if (end > size) end = size;
        
        buffer_stringify(&buffers->buffer, i, end, page_1);
        
        buffer_stringify(&buffers->gap_buffer, i, end, page_2);
        page_compare(page_1, page_2, page_size);
        
        buffer_stringify(&buffers->multi_gap_buffer, i, end, page_2);
        page_compare(page_1, page_2, page_size);
        
        buffer_stringify(&buffers->rope_buffer, i, end, page_2);
        page_compare(page_1, page_2, page_size);
    }

    for (i = size-1; i > 0; i -= page_size){
        int end = i - page_size;
        if (end < 0) end = 0;
        
        buffer_backify(&buffers->buffer, i, end, page_1);
        
        buffer_backify(&buffers->gap_buffer, i, end, page_2);
        page_compare(page_1, page_2, page_size);
        
        buffer_backify(&buffers->multi_gap_buffer, i, end, page_2);
        page_compare(page_1, page_2, page_size);
        
        buffer_backify(&buffers->rope_buffer, i, end, page_2);
        page_compare(page_1, page_2, page_size);
    }
}

int main(int argc, char **argv){
    Buffer_Set buffers;
    File_Data file;
    float *widths_data;

    void *scratch;
    int scratch_size;
    
    if (argc < 2){
        printf("usage: buffer_test <filename>\n");
        exit(1);
    }
    
    setup();

    scratch_size = 1 << 20;
    scratch = malloc(scratch_size);
    
    file = get_file(argv[1]);
    widths_data = get_font_data("LiberationSans-Regular.ttf");
    
    Record_Statistics init_rec, starts_widths_rec;
    
    initialization_test(&buffers, file, 100, scratch, scratch_size, &init_rec);
    stream_check_test(&buffers, scratch, scratch_size);
    
    measure_starts_widths_test(&buffers, 100, scratch, scratch_size, &starts_widths_rec, widths_data);
    
    Time_Record expected_file_open;
    expected_file_open = init_rec.expected + starts_widths_rec.expected;
    
    printf("average file open:\n");
    print_record(expected_file_open);
    
    return(0);
}

// BOTTOM

