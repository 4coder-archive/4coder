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
#undef Buffer_Init_Type
#undef Buffer_Stringify_Type
#undef Buffer_Backify_Type

#ifdef _WIN32
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

#else
#error Timer not supported by this platform
#endif

void setup(){
    unsigned long long resolution;
    if (!time_init(&resolution)){
        printf("error: could not initialize timer");
        exit(1);
    }

    if (resolution < 1000000)
        printf("warning: timer is not actually at high enough resolution for good measurements!\n");

}

typedef struct File_Data{
    char *data;
    int size;
} File_Data;

File_Data get_file(char *filename){
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

template<typename Buffer_Init_Type, typename Buffer_Type> void
init_buffer(Buffer_Type *buffer, File_Data file, void *scratch, int scratch_size){
    *buffer = {};
    Buffer_Init_Type init;
    for (init = buffer_begin_init(buffer, file.data, file.size);
         buffer_init_need_more(&init);){
        int page_size = buffer_init_page_size(&init);
        void *page = malloc(page_size);
        buffer_init_provide_page(&init, page, page_size);
    }
    debug_4tech(int result =)
        buffer_end_init(&init, scratch, scratch_size);
    assert_4tech(result);
}

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
        init_buffer<Buffer_Init>(&set->buffer, file, scratch, scratch_size);
        tend = get_time();
        init_time[i].buffer = tend - tstart;
    
        tstart = get_time();
        init_buffer<Gap_Buffer_Init>(&set->gap_buffer, file, scratch, scratch_size);
        tend = get_time();
        init_time[i].gap_buffer = tend - tstart;
    
        tstart = get_time();
        init_buffer<Multi_Gap_Buffer_Init>(&set->multi_gap_buffer, file, scratch, scratch_size);
        tend = get_time();
        init_time[i].multi_gap_buffer = tend - tstart;
    
        tstart = get_time();
        init_buffer<Rope_Buffer_Init>(&set->rope_buffer, file, scratch, scratch_size);
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

template<typename Buffer_Type> void
measure_starts(Buffer_Type *buffer){
    int max = 1 << 10;
    buffer->line_starts = (int*)malloc(max*sizeof(int));
    buffer->line_max = max;

    Buffer_Measure_Starts state = {};
    for (;buffer_measure_starts(&state, buffer);){
        int max = buffer->line_max;
        int count = state.count;
        int target = count + 1;

        max = target*2;
        int *new_lines = (int*)malloc(max*sizeof(int));
        memcpy_4tech(new_lines, buffer->line_starts, count*sizeof(int));
        free(buffer->line_starts);
        buffer->line_starts = new_lines;
        buffer->line_max = max;
    }
    buffer->line_count = state.count;
}

void
measure_starts_test(Buffer_Set *set, int test_repitions,
                    void *scratch, int scratch_size, Record_Statistics *stats_out){
    time_int tstart, tend;
    Time_Record *measure_time = (Time_Record*)scratch;
    scratch = measure_time + test_repitions;
    assert_4tech(test_repitions*sizeof(*measure_time) < scratch_size);
    scratch_size -= test_repitions*sizeof(*measure_time);
    
    for (int i = 0; i < test_repitions; ++i){
        tstart = get_time();
        measure_starts(&set->buffer);
        tend = get_time();
        measure_time[i].buffer = tend - tstart;
    
        tstart = get_time();
        measure_starts(&set->gap_buffer);
        tend = get_time();
        measure_time[i].gap_buffer = tend - tstart;
    
        tstart = get_time();
        measure_starts(&set->multi_gap_buffer);
        tend = get_time();
        measure_time[i].multi_gap_buffer = tend - tstart;
    
        tstart = get_time();
        measure_starts(&set->rope_buffer);
        tend = get_time();
        measure_time[i].rope_buffer = tend - tstart;

        if (i+1 != test_repitions){
            free(set->buffer.line_starts);
            free(set->gap_buffer.line_starts);
            free(set->multi_gap_buffer.line_starts);
            free(set->rope_buffer.line_starts);
        }
    }

    if (!test_is_silenced) print_name();
    print_statistics(measure_time, test_repitions, stats_out);
    if (!test_is_silenced) printf("\n");
    test_is_silenced = 0;
}

template<typename Buffer_Type> void
measure_widths(Buffer_Type *buffer){
    int new_max = round_up_4tech(buffer->line_count, 1 << 10);
    if (new_max < (1 << 10)) new_max = 1 << 10;
        
    buffer->line_widths = (float*)malloc(new_max*sizeof(float));
    buffer->widths_max = new_max;
    buffer->widths_count = 0;
    
    float glyph_width = 8.f;
    buffer_measure_widths(buffer, &glyph_width, 0);
}

void
measure_widths_test(Buffer_Set *set, int test_repitions,
                    void *scratch, int scratch_size, Record_Statistics *stats_out){
    time_int tstart, tend;
    Time_Record *measure_time = (Time_Record*)scratch;
    scratch = measure_time + test_repitions;
    assert_4tech(test_repitions*sizeof(*measure_time) < scratch_size);
    scratch_size -= test_repitions*sizeof(*measure_time);
    
    for (int i = 0; i < test_repitions; ++i){
        tstart = get_time();
        measure_widths(&set->buffer);
        tend = get_time();
        measure_time[i].buffer = tend - tstart;
    
        tstart = get_time();
        measure_widths(&set->gap_buffer);
        tend = get_time();
        measure_time[i].gap_buffer = tend - tstart;
    
        tstart = get_time();
        measure_widths(&set->multi_gap_buffer);
        tend = get_time();
        measure_time[i].multi_gap_buffer = tend - tstart;
    
        tstart = get_time();
        measure_widths(&set->rope_buffer);
        tend = get_time();
        measure_time[i].rope_buffer = tend - tstart;

        if (i+1 != test_repitions){
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
}

int main(){
    Buffer_Set buffers;
    File_Data file;

    void *scratch;
    int scratch_size;
    
    setup();

    scratch_size = 1 << 20;
    scratch = malloc(scratch_size);
    
    file = get_file("test_file_1.cpp");
    
    Record_Statistics init_rec, starts_rec, widths_rec;
    
    initialization_test(&buffers, file, 100, scratch, scratch_size, &init_rec);
    stream_check_test(&buffers, scratch, scratch_size);
    
    measure_starts_test(&buffers, 100, scratch, scratch_size, &starts_rec);
    measure_widths_test(&buffers, 100, scratch, scratch_size, &widths_rec);
    
    Time_Record expected_file_open;
    expected_file_open = init_rec.expected + starts_rec.expected + widths_rec.expected;
    
    printf("average file open:\n");
    print_record(expected_file_open);
    
    return(0);
}

// BOTTOM

