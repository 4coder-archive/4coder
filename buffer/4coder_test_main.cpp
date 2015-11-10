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

#include "4coder_external_name.h"

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

int int_into_str(char *out, int *rem, unsigned int x){
    char *start = out;
    int size = *rem;
    int result;
    char t;
    
    if (x == 0 && size > 1){
        *out = '0';
        ++out;
        --size;
    }
    else{
        for (;x > 0 && size > 1;--size, ++out){
            *out = (x%10 + '0');
            x /= 10;
        }
    }
    
    *rem = size;
    *out = 0;

    result = (int)(out - start);

    --out;
    for (; start < out; ++start, --out){
        t = *out;
        *out = *start;
        *start = t;
    }

    return(result);
}

int uscore_into_str(char *out, int *rem){
    int result;
    result = 0;
    if (*rem > 1){
        --*rem;
        *out++ = '_';
        *out = 0;
        result = 1;
    }
    return(result);
}

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

void time_into_str(char *out, int max){
    SYSTEMTIME systime;
    int pos;

    GetSystemTime(&systime);
    pos = uscore_into_str(out, &max);
    pos += int_into_str(out + pos, &max, systime.wYear);
    
    pos += uscore_into_str(out + pos, &max);
    pos += int_into_str(out + pos, &max, systime.wMonth);
    
    pos += uscore_into_str(out + pos, &max);
    pos += int_into_str(out + pos, &max, systime.wDay);
    
    pos += uscore_into_str(out + pos, &max);
    pos += int_into_str(out + pos, &max, systime.wHour);
    
    pos += uscore_into_str(out + pos, &max);
    pos += int_into_str(out + pos, &max, systime.wMinute);
    
    pos += uscore_into_str(out + pos, &max);
    pos += int_into_str(out + pos, &max, systime.wSecond);
    
    pos += uscore_into_str(out + pos, &max);
    pos += int_into_str(out + pos, &max, systime.wMilliseconds);
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

float* get_font_data(const char *font_file, float *font_height){
    float *data = 0;
    stbtt_bakedchar *baked;
    File_Data file = get_file(font_file);

    if (file.data){
        int size = sizeof(*baked)*256;
        baked = (stbtt_bakedchar*)malloc(size);
        memset_4tech(baked, 0, sizeof(*baked)*256);

        stbtt_fontinfo font;
        if (stbtt_InitFont(&font, (unsigned char*)file.data, 0)){
            float scale;
            int a,d,g;
            
            scale = stbtt_ScaleForPixelHeight(&font, 17.f);
            stbtt_GetFontVMetrics(&font, &a, &d, &g);
            *font_height = scale*(a - d + g);
            
            int w, h;
            w = 10*256;
            h = 25;
            unsigned char *pixels = (unsigned char*)malloc(w * h);
            stbtt_BakeFontBitmap((unsigned char*)file.data, 0, 17.f, pixels, w, h, 0, 128, baked);
            free(pixels);
            free_file(file);

            data = (float*)malloc(sizeof(float)*256);
            memset_4tech(data, 0, sizeof(float)*256);

            stbtt_bakedchar *baked_ptr = baked;
            for (int i = 0; i < 128; ++i, ++baked_ptr){
                data[i] = baked_ptr->xadvance;
            }
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

typedef struct Log_Section{
    int *counter;
} Log_Section;

typedef struct Stats_Log{
    char *out;
    int size, max;
    
    Log_Section *sections;
    int sec_top, sec_max;
    
    unsigned int error;
} Stats_Log;

#define log_er_buffer_overflow 0x1
#define log_er_stack_overflow 0x2
#define log_er_stack_underflow 0x4
#define log_er_time_too_large 0x8

#define logid_begin_section 0
#define logid_data_item 1

#if fast_test
#define use_stats_log 1
#else
#define use_stats_log 0
#endif

void
log_write_int(Stats_Log *log, int x){
#if use_stats_log
    if (log->error == 0){
        if (log->size+4 <= log->max){
            *(int*)(log->out + log->size) = x;
            log->size += 4;
        }
        else{
            log->error |= log_er_buffer_overflow;
        }
    }
#endif
}

void
log_write_time(Stats_Log *log, time_int x){
#if use_stats_log
    if (log->error == 0){
        if (x < 0x7FFFFFFF){
            if (log->size+4 <= log->max){
                *(int*)(log->out + log->size) = (int)x;
                log->size += 4;
            }
            else{
                log->error |= log_er_buffer_overflow;
            }
        }
        else{
            log->error |= log_er_time_too_large;
        }
    }
#endif
}

void
log_write_str(Stats_Log *log, char *str, int len){
#if use_stats_log
    int up = (len + 3) & ~3;
    if (log->error == 0){
        if (log->size+4+up <= log->max){
            *(int*)(log->out + log->size) = up;
            memcpy_4tech(log->out + log->size + 4, str, len);
            log->size += 4+up;
        }
        else{
            log->error |= log_er_buffer_overflow;
        }
    }
#endif
}

void
log_begin_section(Stats_Log *log, char *name, int name_len){
#if use_stats_log
    Log_Section *section;
    if (log->error == 0){
        if (log->sec_top < log->sec_max){
            if (log->sec_top > 0){
                section = log->sections + log->sec_top - 1;
                ++section->counter;
            }
            
            section = log->sections + (log->sec_top++);
        
            log_write_int(log, logid_begin_section);
            log_write_str(log, name, name_len);

            section->counter = (int*)(log->out + log->size);
            log_write_int(log, 0);
        }
        else{
            log->error |= log_er_stack_overflow;
        }
    }
#endif
}

void
log_end_section(Stats_Log *log){
#if use_stats_log
    if (log->error == 0){
        if (log->sec_top > 0){
            --log->sec_top;
        }
        else{
            log->error |= log_er_stack_underflow;
        }
    }
#endif
}

void
log_data_item(Stats_Log *log, char *name, int name_len, time_int t){
#if use_stats_log
    Log_Section *section;
    if (log->error == 0){
        if (log->sec_top > 0){
            section = log->sections + log->sec_top - 1;
            ++section->counter;
        }
        
        log_write_int(log, logid_data_item);
        log_write_str(log, name, name_len);
        log_write_time(log, t);
    }
#endif
}

void
log_finish(Stats_Log *log){
#if use_stats_log
    assert_4tech(sizeof(external_name) < 512);
    if (log->error == 0){
        char fname[1024];
        memcpy_4tech(fname, "out/", 4);
        memcpy_4tech(fname + 4, external_name, sizeof(external_name)-1);
        time_into_str(fname + 4 + sizeof(external_name) - 1, 1023 - sizeof(external_name) + 1);
        
        FILE *log_out = fopen(fname, "wb");
        if (log_out){
            fwrite(log->out, 1, log->size, log_out);
            fclose(log_out);
        }
        else{
            printf("log error: could not open %s\n", fname);
        }
    }
    else{
        printf("\n");
        if (log->error & log_er_buffer_overflow)
            printf("log error: buffer overflow\n");
        if (log->error & log_er_stack_overflow)
            printf("log error: stack overflow\n");
        if (log->error & log_er_stack_underflow)
            printf("log error: stack underflow\n");
        printf("there were log error so the log was not saved\n\n");
    }
#endif
}

#define litstr(s) (char*)(s), (sizeof(s)-1)

void
log_time_record(Stats_Log *log, char *name, int name_len, Time_Record record){
    log_begin_section(log, name, name_len);
    log_data_item(log, litstr("golden-array"), record.buffer);
    log_data_item(log, litstr("gap-buffer"), record.gap_buffer);
    log_data_item(log, litstr("multi-gap-buffer"), record.multi_gap_buffer);
    log_data_item(log, litstr("rope"), record.rope_buffer);
    log_end_section(log);
}

Time_Record
operator+(const Time_Record &a, const Time_Record &b){
    Time_Record r;
    r.buffer = a.buffer + b.buffer;
    r.gap_buffer = a.gap_buffer + b.gap_buffer;
    r.multi_gap_buffer = a.multi_gap_buffer + b.multi_gap_buffer;
    r.rope_buffer = a.rope_buffer + b.rope_buffer;
    return r;
}

Time_Record&
operator/=(Time_Record &r, int x){
    r.buffer /= x;
    r.gap_buffer /= x;
    r.multi_gap_buffer /= x;
    r.rope_buffer /= x;
    
    return(r);
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

    stats.expected /= count;
    
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

void
log_sample_set(Stats_Log *log, char *name, int name_len, Record_Statistics *stats,
               Time_Record *samples, int sample_count){
    log_begin_section(log, name, name_len);
        
    log_data_item(log, litstr("sample-count"), sample_count);
    log_time_record(log, litstr("max"), stats->max);
    log_time_record(log, litstr("min"), stats->min);
    log_time_record(log, litstr("average"), stats->expected);
    
    for (int i = 0; i < sample_count; ++i){
        log_time_record(log, litstr("item"), samples[i]);
    }
    
    log_end_section(log);
}

typedef struct Sample_Machine{
    time_int tstart, tend;
    Time_Record *samples;
    int count;
} Sample_Machine;

Sample_Machine
begin_machine(int count, void **data, int *max){
    Sample_Machine result;
    
    result.count = count;
    result.samples = (Time_Record*)*data;
    *data = result.samples + count;
    assert_4tech(count*sizeof(*result.samples) < *max);
    *max -= count*sizeof(*result.samples);
    
    return(result);
}

void
end_machine(Sample_Machine *machine, Record_Statistics *stats_out, char *func_name){
    if (!test_is_silenced) printf("%s\n", func_name);
    print_statistics(machine->samples, machine->count, stats_out);
    if (!test_is_silenced) printf("\n");
    test_is_silenced = 0;
}
                  
void start(Sample_Machine *machine){
    machine->tstart = get_time();
}
                  
time_int stop(Sample_Machine *machine){
    machine->tend = get_time();
    return machine->tend - machine->tstart;
}
                  
void
initialization_test(Stats_Log *log, Buffer_Set *set, File_Data file, int test_repitions,
                    void *scratch, int scratch_size, Record_Statistics *stats_out){
    Sample_Machine machine;
    machine = begin_machine(test_repitions, &scratch, &scratch_size);
    
    for (int i = 0; i < test_repitions; ++i){
        start(&machine);
        init_buffer(&set->buffer, file, scratch, scratch_size);
        machine.samples[i].buffer = stop(&machine);

        start(&machine);
        init_buffer(&set->gap_buffer, file, scratch, scratch_size);
        machine.samples[i].gap_buffer = stop(&machine);
    
        start(&machine);
        init_buffer(&set->multi_gap_buffer, file, scratch, scratch_size);
        machine.samples[i].multi_gap_buffer = stop(&machine);
    
        start(&machine);
        init_buffer(&set->rope_buffer, file, scratch, scratch_size);
        machine.samples[i].rope_buffer = stop(&machine);
 
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

    end_machine(&machine, stats_out, __FUNCTION__);
    
    log_sample_set(log, litstr("initialization"), stats_out, machine.samples, machine.count);
}

void
measure_starts_widths_test(Stats_Log *log, Buffer_Set *set, int test_repitions, void *scratch,
                           int scratch_size, Record_Statistics *stats_out, float *font_widths){
    Sample_Machine machine;
    machine = begin_machine(test_repitions, &scratch, &scratch_size);
    
    for (int i = 0; i < test_repitions; ++i){
        start(&machine);
        measure_starts_widths(&set->buffer, font_widths);
        machine.samples[i].buffer = stop(&machine);
    
        start(&machine);
        measure_starts_widths(&set->gap_buffer, font_widths);
        machine.samples[i].gap_buffer = stop(&machine);
        
        start(&machine);
        measure_starts_widths(&set->multi_gap_buffer, font_widths);
        machine.samples[i].multi_gap_buffer = stop(&machine);
    
        start(&machine);
        measure_starts_widths(&set->rope_buffer, font_widths);
        machine.samples[i].rope_buffer = stop(&machine);

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

    end_machine(&machine, stats_out, __FUNCTION__);
    
    log_sample_set(log, litstr("measure_starts_widths"), stats_out, machine.samples, machine.count);
}

int
page_compare(void *page_1_, void *page_2_, int page_size){
    char *page_1 = (char*)page_1_;
    char *page_2 = (char*)page_2_;
    int result = 1;
    for (int i = 0; i < page_size; ++i){
        hard_assert_4tech(page_1[i] == page_2[i]);
    }
    return result;
}

float*
measure_wraps_test(Stats_Log *log, Buffer_Set *buffers, int test_repitions, void *scratch,
                   int scratch_size, Record_Statistics *stats_out, float font_height, float max_width){
    Sample_Machine machine;
    machine = begin_machine(test_repitions, &scratch, &scratch_size);
    
    float *wrap_ys, *wrap_ys2;    
    wrap_ys = (float*)malloc(sizeof(float)*buffers->buffer.line_count);
    wrap_ys2 = (float*)malloc(sizeof(float)*buffers->buffer.line_count);
    
    for (int i = 0; i < test_repitions; ++i){
        start(&machine);
        buffer_measure_wrap_y(&buffers->buffer, wrap_ys, font_height, max_width);
        machine.samples[i].buffer = stop(&machine);

        start(&machine);
        buffer_measure_wrap_y(&buffers->gap_buffer, wrap_ys2, font_height, max_width);
        machine.samples[i].gap_buffer = stop(&machine);
        if (i == 0)
            page_compare((char*)wrap_ys, (char*)wrap_ys2, sizeof(float)*buffers->buffer.line_count);
    
        start(&machine);
        buffer_measure_wrap_y(&buffers->multi_gap_buffer, wrap_ys2, font_height, max_width);
        machine.samples[i].multi_gap_buffer = stop(&machine);
        if (i == 0)
            page_compare((char*)wrap_ys, (char*)wrap_ys2, sizeof(float)*buffers->buffer.line_count);
    
        start(&machine);
        buffer_measure_wrap_y(&buffers->rope_buffer, wrap_ys2, font_height, max_width);
        machine.samples[i].rope_buffer = stop(&machine);
        if (i == 0)
            page_compare((char*)wrap_ys, (char*)wrap_ys2, sizeof(float)*buffers->buffer.line_count);
    }
    
    free(wrap_ys2);

    end_machine(&machine, stats_out, __FUNCTION__);
    
    log_sample_set(log, litstr("measure-wrap-ys"), stats_out, machine.samples, machine.count);
    
    return wrap_ys;
}

int
cursor_eq(Full_Cursor c1, Full_Cursor c2){
    int result = 0;
    if (c1.pos == c2.pos && c1.line == c2.line && c1.character == c2.character &&
        c1.wrapped_x == c2.wrapped_x && c1.wrapped_y == c2.wrapped_y &&
        c1.unwrapped_x == c2.unwrapped_x && c1.unwrapped_y == c2.unwrapped_y){
        result = 1;
    }
    return(result);
}

void
full_cursor_test(Stats_Log *log, Buffer_Set *buffers, int pos,
                 float *wrap_ys, float *advance_data, float font_height, float max_width,
                 int test_repitions, void *scratch, int scratch_size, Record_Statistics *stats_out){
    Sample_Machine machine;
    machine = begin_machine(test_repitions, &scratch, &scratch_size);

    Full_Cursor cursor, cursor2;
    
    for (int i = 0; i < test_repitions; ++i){
        start(&machine);
        cursor = buffer_cursor_from_pos(&buffers->buffer, pos, wrap_ys, max_width, font_height, advance_data);
        machine.samples[i].buffer = stop(&machine);

        start(&machine);
        cursor2 = buffer_cursor_from_pos(&buffers->gap_buffer, pos, wrap_ys, max_width, font_height, advance_data);
        machine.samples[i].gap_buffer = stop(&machine);
        if (i == 0) assert_4tech(cursor_eq(cursor, cursor2));
    
        start(&machine);
        cursor2 = buffer_cursor_from_pos(&buffers->multi_gap_buffer, pos, wrap_ys, max_width, font_height, advance_data);
        machine.samples[i].multi_gap_buffer = stop(&machine);
        if (i == 0) assert_4tech(cursor_eq(cursor, cursor2));
    
        start(&machine);
        cursor2 = buffer_cursor_from_pos(&buffers->rope_buffer, pos, wrap_ys, max_width, font_height, advance_data);
        machine.samples[i].rope_buffer = stop(&machine);
        if (i == 0) assert_4tech(cursor_eq(cursor, cursor2));
    }

    end_machine(&machine, stats_out, __FUNCTION__);
    
    log_sample_set(log, litstr("full-cursor-seek"), stats_out, machine.samples, machine.count);
}

void
full_cursor_line_test(Stats_Log *log, Buffer_Set *buffers, int line, int character,
                 float *wrap_ys, float *advance_data, float font_height, float max_width,
                 int test_repitions, void *scratch, int scratch_size, Record_Statistics *stats_out){
    Sample_Machine machine;
    machine = begin_machine(test_repitions, &scratch, &scratch_size);

    Full_Cursor cursor, cursor2;
    
    for (int i = 0; i < test_repitions; ++i){
        start(&machine);
        cursor = buffer_cursor_from_line_character(&buffers->buffer, line, character,
                                                   wrap_ys, max_width, font_height, advance_data);
        machine.samples[i].buffer = stop(&machine);

        start(&machine);
        cursor2 = buffer_cursor_from_line_character(&buffers->gap_buffer, line, character,
                                                    wrap_ys, max_width, font_height, advance_data);
        machine.samples[i].gap_buffer = stop(&machine);
        if (i == 0) assert_4tech(cursor_eq(cursor, cursor2));
    
        start(&machine);
        cursor2 = buffer_cursor_from_line_character(&buffers->multi_gap_buffer, line, character,
                                                    wrap_ys, max_width, font_height, advance_data);
        machine.samples[i].multi_gap_buffer = stop(&machine);
        if (i == 0) assert_4tech(cursor_eq(cursor, cursor2));
    
        start(&machine);
        cursor2 = buffer_cursor_from_line_character(&buffers->rope_buffer, line, character,
                                                    wrap_ys, max_width, font_height, advance_data);
        machine.samples[i].rope_buffer = stop(&machine);
        if (i == 0) assert_4tech(cursor_eq(cursor, cursor2));
    }

    end_machine(&machine, stats_out, __FUNCTION__);
    
    log_sample_set(log, litstr("full-cursor-seek"), stats_out, machine.samples, machine.count);
}

void
full_cursor_xy_test(Stats_Log *log, Buffer_Set *buffers, float x, float y, int round_down,
                    float *wrap_ys, float *advance_data, float font_height, float max_width,
                    int test_repitions, void *scratch, int scratch_size, Record_Statistics *stats_out){
    Sample_Machine machine;
    machine = begin_machine(test_repitions, &scratch, &scratch_size);

    Full_Cursor cursor, cursor2;
    
    for (int i = 0; i < test_repitions; ++i){
        start(&machine);
        cursor = buffer_cursor_from_unwrapped_xy(&buffers->buffer, x, y, round_down,
                                                 wrap_ys, max_width, font_height, advance_data);
        machine.samples[i].buffer = stop(&machine);

        start(&machine);
        cursor2 = buffer_cursor_from_unwrapped_xy(&buffers->gap_buffer, x, y, round_down,
                                                    wrap_ys, max_width, font_height, advance_data);
        machine.samples[i].gap_buffer = stop(&machine);
        if (i == 0) assert_4tech(cursor_eq(cursor, cursor2));
    
        start(&machine);
        cursor2 = buffer_cursor_from_unwrapped_xy(&buffers->multi_gap_buffer, x, y, round_down,
                                                    wrap_ys, max_width, font_height, advance_data);
        machine.samples[i].multi_gap_buffer = stop(&machine);
        if (i == 0) assert_4tech(cursor_eq(cursor, cursor2));
    
        start(&machine);
        cursor2 = buffer_cursor_from_unwrapped_xy(&buffers->rope_buffer, x, y, round_down,
                                                    wrap_ys, max_width, font_height, advance_data);
        machine.samples[i].rope_buffer = stop(&machine);
        if (i == 0) assert_4tech(cursor_eq(cursor, cursor2));
    }

    end_machine(&machine, stats_out, __FUNCTION__);
    
    log_sample_set(log, litstr("full-cursor-seek"), stats_out, machine.samples, machine.count);
}

void
word_seek_test(Stats_Log *log, Buffer_Set *buffers, int test_repitions,
               int incremental_position, char *word, int len,
               void *scratch, int scratch_size, Record_Statistics *stats_out){
    Sample_Machine machine;
    machine = begin_machine(test_repitions, &scratch, &scratch_size);
    assert_4tech(scratch_size >= len);
    
    int pos, pos2, old_pos;
    old_pos = 0;
    
    for (int i = 0; i < machine.count; ++i){
        start(&machine);
        pos = buffer_find_string(&buffers->buffer, old_pos, word, len, (char*)scratch);
        machine.samples[i].buffer = stop(&machine);
        
        start(&machine);
        pos2 = buffer_find_string(&buffers->gap_buffer, old_pos, word, len, (char*)scratch);
        machine.samples[i].gap_buffer = stop(&machine);
        assert_4tech(pos2 == pos);
        
        start(&machine);
        pos2 = buffer_find_string(&buffers->multi_gap_buffer, old_pos, word, len, (char*)scratch);
        machine.samples[i].multi_gap_buffer = stop(&machine);
        assert_4tech(pos2 == pos);
        
        start(&machine);
        pos2 = buffer_find_string(&buffers->rope_buffer, old_pos, word, len, (char*)scratch);
        machine.samples[i].rope_buffer = stop(&machine);
        assert_4tech(pos2 == pos);
        
        if (incremental_position) old_pos = pos;
    }
    
    end_machine(&machine, stats_out, __FUNCTION__);
    
    log_sample_set(log, litstr("word-seek"), stats_out, machine.samples, machine.count);
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

void
insert_bottom_test(Stats_Log *log, Buffer_Set *buffers, int test_repitions, float *advance_data,
                int edit_count, void *scratch, int scratch_size, Record_Statistics *stats_out){
    Sample_Machine machine;
    machine = begin_machine(test_repitions, &scratch, &scratch_size);
    
    char word[] = "stuff";
    int word_len = sizeof(word) - 1;
    
    int i, j;
    for (i = 0; i < test_repitions; ++i){
        start(&machine);
        for (j = 0; j < edit_count; ++j){
            insert_bottom(&buffers->buffer, word, word_len,
                       advance_data, scratch, scratch_size);
        }
        machine.samples[i].buffer = stop(&machine);
        
        start(&machine);
        for (j = 0; j < edit_count; ++j){
            insert_bottom(&buffers->gap_buffer, word, word_len,
                       advance_data, scratch, scratch_size);
        }
        machine.samples[i].gap_buffer = stop(&machine);
        
        start(&machine);
        for (j = 0; j < edit_count; ++j){
            insert_bottom(&buffers->multi_gap_buffer, word, word_len,
                       advance_data, scratch, scratch_size);
        }
        machine.samples[i].multi_gap_buffer = stop(&machine);
        
        start(&machine);
        for (j = 0; j < edit_count; ++j){
            insert_bottom(&buffers->rope_buffer, word, word_len,
                       advance_data, scratch, scratch_size);
        }
        machine.samples[i].rope_buffer = stop(&machine);

        if (i == 0){
            stream_check_test(buffers, scratch, scratch_size);
        }
    }
    
    end_machine(&machine, stats_out, __FUNCTION__);

    log_sample_set(log, litstr("insert-bottom"), stats_out, machine.samples, machine.count);
}

void
insert_top_test(Stats_Log *log, Buffer_Set *buffers, int test_repitions, float *advance_data,
                int edit_count, void *scratch, int scratch_size, Record_Statistics *stats_out){
    Sample_Machine machine;
    machine = begin_machine(test_repitions, &scratch, &scratch_size);
    
    char word[] = "stuff";
    int word_len = sizeof(word) - 1;
    
    int i, j;
    for (i = 0; i < test_repitions; ++i){
        start(&machine);
        for (j = 0; j < edit_count; ++j){
            insert_top(&buffers->buffer, word, word_len,
                       advance_data, scratch, scratch_size);
        }
        machine.samples[i].buffer = stop(&machine);
        
        start(&machine);
        for (j = 0; j < edit_count; ++j){
            insert_top(&buffers->gap_buffer, word, word_len,
                       advance_data, scratch, scratch_size);
        }
        machine.samples[i].gap_buffer = stop(&machine);
        
        start(&machine);
        for (j = 0; j < edit_count; ++j){
            insert_top(&buffers->multi_gap_buffer, word, word_len,
                       advance_data, scratch, scratch_size);
        }
        machine.samples[i].multi_gap_buffer = stop(&machine);
        
        start(&machine);
        for (j = 0; j < edit_count; ++j){
            insert_top(&buffers->rope_buffer, word, word_len,
                       advance_data, scratch, scratch_size);
        }
        machine.samples[i].rope_buffer = stop(&machine);

        if (i == 0){
            stream_check_test(buffers, scratch, scratch_size);
        }
    }
    
    end_machine(&machine, stats_out, __FUNCTION__);

    log_sample_set(log, litstr("insert-top"), stats_out, machine.samples, machine.count);
}

void
delete_bottom_test(Stats_Log *log, Buffer_Set *buffers, int test_repitions, float *advance_data,
                   int edit_count, void *scratch, int scratch_size, Record_Statistics *stats_out){
    Sample_Machine machine;
    machine = begin_machine(test_repitions, &scratch, &scratch_size);

    int len = 5;
    
    int i, j;
    for (i = 0; i < test_repitions; ++i){
        start(&machine);
        for (j = 0; j < edit_count; ++j){
            delete_bottom(&buffers->buffer, len,
                       advance_data, scratch, scratch_size);
        }
        machine.samples[i].buffer = stop(&machine);
        
        start(&machine);
        for (j = 0; j < edit_count; ++j){
            delete_bottom(&buffers->gap_buffer, len,
                       advance_data, scratch, scratch_size);
        }
        machine.samples[i].gap_buffer = stop(&machine);
        
        start(&machine);
        for (j = 0; j < edit_count; ++j){
            delete_bottom(&buffers->multi_gap_buffer, len,
                       advance_data, scratch, scratch_size);
        }
        machine.samples[i].multi_gap_buffer = stop(&machine);
        
        start(&machine);
        for (j = 0; j < edit_count; ++j){
            delete_bottom(&buffers->rope_buffer, len,
                       advance_data, scratch, scratch_size);
        }
        machine.samples[i].rope_buffer = stop(&machine);

        if (i == 0){
            stream_check_test(buffers, scratch, scratch_size);
        }
    }
    
    end_machine(&machine, stats_out, __FUNCTION__);

    log_sample_set(log, litstr("delete-bottom"), stats_out, machine.samples, machine.count);
}

void
delete_top_test(Stats_Log *log, Buffer_Set *buffers, int test_repitions, float *advance_data,
                int edit_count, void *scratch, int scratch_size, Record_Statistics *stats_out){
    Sample_Machine machine;
    machine = begin_machine(test_repitions, &scratch, &scratch_size);

    int len = 5;
    
    int i, j;
    for (i = 0; i < test_repitions; ++i){
        start(&machine);
        for (j = 0; j < edit_count; ++j){
            delete_top(&buffers->buffer, len,
                       advance_data, scratch, scratch_size);
        }
        machine.samples[i].buffer = stop(&machine);
        
        start(&machine);
        for (j = 0; j < edit_count; ++j){
            delete_top(&buffers->gap_buffer, len,
                       advance_data, scratch, scratch_size);
        }
        machine.samples[i].gap_buffer = stop(&machine);
        
        start(&machine);
        for (j = 0; j < edit_count; ++j){
            delete_top(&buffers->multi_gap_buffer, len,
                       advance_data, scratch, scratch_size);
        }
        machine.samples[i].multi_gap_buffer = stop(&machine);
        
        start(&machine);
        for (j = 0; j < edit_count; ++j){
            delete_top(&buffers->rope_buffer, len,
                       advance_data, scratch, scratch_size);
        }
        machine.samples[i].rope_buffer = stop(&machine);

        if (i == 0){
            stream_check_test(buffers, scratch, scratch_size);
        }
    }
    
    end_machine(&machine, stats_out, __FUNCTION__);

    log_sample_set(log, litstr("delete-top"), stats_out, machine.samples, machine.count);
}

void
measure_check_test(Buffer_Set *buffers){
    int count;
    count = buffers->buffer.line_count;
    assert_4tech(count == buffers->buffer.widths_count);
    
    assert_4tech(count == buffers->gap_buffer.line_count);
    assert_4tech(count == buffers->multi_gap_buffer.line_count);
    assert_4tech(count == buffers->rope_buffer.line_count);
    
    assert_4tech(count == buffers->gap_buffer.widths_count);
    assert_4tech(count == buffers->multi_gap_buffer.widths_count);
    assert_4tech(count == buffers->rope_buffer.widths_count);

    page_compare(buffers->buffer.line_starts, buffers->gap_buffer.line_starts, sizeof(int)*count);
    page_compare(buffers->buffer.line_starts, buffers->multi_gap_buffer.line_starts, sizeof(int)*count);
    page_compare(buffers->buffer.line_starts, buffers->rope_buffer.line_starts, sizeof(int)*count);

    page_compare(buffers->buffer.line_widths, buffers->gap_buffer.line_widths, sizeof(float)*count);
    page_compare(buffers->buffer.line_widths, buffers->multi_gap_buffer.line_widths, sizeof(float)*count);
    page_compare(buffers->buffer.line_widths, buffers->rope_buffer.line_widths, sizeof(float)*count);
}

int main(int argc, char **argv){
    Buffer_Set buffers;
    File_Data file;
    float *widths_data;
    float *wrap_ys;
    float font_height;
    float max_width;

    void *scratch;
    int scratch_size;
    
    Stats_Log log;
    
    if (argc < 2){
        printf("usage: buffer_test <filename>\n");
        exit(1);
    }
    
    setup();
    
    log.max = 1 << 20;
    log.size = 0;
    log.out = (char*)malloc(log.max);

    log.sec_max = 32;
    log.sec_top = 0;
    log.sections = (Log_Section*)malloc(sizeof(Log_Section)*log.sec_max);

    log.error = 0;

    scratch_size = 1 << 20;
    scratch = malloc(scratch_size);
    
    file = get_file(argv[1]);
    widths_data = get_font_data("LiberationSans-Regular.ttf", &font_height);
    max_width = 500.f;
    
    log_begin_section(&log, litstr("which-test"));
    {
        log_write_str(&log, argv[1], (int)strlen(argv[1]));
        log_write_int(&log, file.size);
    }
    log_end_section(&log);
    
    log_begin_section(&log, litstr("file-open"));
    {
        Record_Statistics init_rec, starts_widths_rec, wraps_rec;
    
        initialization_test(&log, &buffers, file, 25, scratch, scratch_size, &init_rec);
        stream_check_test(&buffers, scratch, scratch_size);
    
        measure_starts_widths_test(&log, &buffers, 25, scratch, scratch_size, &starts_widths_rec, widths_data);
        measure_check_test(&buffers);

        wrap_ys = measure_wraps_test(&log, &buffers, 25, scratch, scratch_size, &wraps_rec, font_height, max_width);
    
        Time_Record expected_file_open;
        expected_file_open = init_rec.expected + starts_widths_rec.expected + wraps_rec.expected;
    
        printf("average file open:\n");
        print_record(expected_file_open);
        printf("\n");
        log_time_record(&log, litstr("average"), expected_file_open);
    }
    log_end_section(&log);
    
    log_begin_section(&log, litstr("cursor-seek"));
    {
        Record_Statistics full_cursor;
        Time_Record full_cursor_average;

        log_begin_section(&log, litstr("to-pos"));
        {
            memzero_4tech(full_cursor_average);
            for (int i = 0; i < 5; ++i){
                silence_test();
                int pos = (file.size*i) / 5;
                full_cursor_test(&log, &buffers, pos,
                                 wrap_ys, widths_data, font_height, max_width,
                                 5, scratch, scratch_size, &full_cursor);
                full_cursor_average = full_cursor_average + full_cursor.expected;
            }
            full_cursor_average /= 5;
            printf("average cursor from position:\n");
            print_record(full_cursor_average);
            printf("\n");
            log_time_record(&log, litstr("average"), full_cursor_average);
        }
        log_end_section(&log);

        log_begin_section(&log, litstr("to-line-character"));
        {
            memzero_4tech(full_cursor_average);
            for (int i = 0; i < 5; ++i){
                silence_test();
                int line = (buffers.buffer.line_count*i) / 5;
                full_cursor_line_test(&log, &buffers, line, 20,
                                      wrap_ys, widths_data, font_height, max_width,
                                      5, scratch, scratch_size, &full_cursor);
                full_cursor_average = full_cursor_average + full_cursor.expected;
            }
            full_cursor_average /= 5;
            printf("average cursor from line & character:\n");
            print_record(full_cursor_average);
            printf("\n");
            log_time_record(&log, litstr("average"), full_cursor_average);
        }
        log_end_section(&log);

        log_begin_section(&log, litstr("to-unwrapped-x-y"));
        {
            memzero_4tech(full_cursor_average);
            for (int i = 0; i < 5; ++i){
                silence_test();
                float y = font_height * (buffers.buffer.line_count*i) / 4.f;
                full_cursor_xy_test(&log, &buffers, y, 37.f, 0,
                                    wrap_ys, widths_data, font_height, max_width,
                                    5, scratch, scratch_size, &full_cursor);
                full_cursor_average = full_cursor_average + full_cursor.expected;
            }
            full_cursor_average /= 5;
            printf("average cursor from line & character:\n");
            print_record(full_cursor_average);
            printf("\n");
            log_time_record(&log, litstr("average"), full_cursor_average);
        }
        log_end_section(&log);
        
    }
    log_end_section(&log);
    
    log_begin_section(&log, litstr("word-seek"));
    {
        Record_Statistics word_seek;

        {
            char word[] = "not-going-to-find-this";
            int word_len = sizeof(word) - 1;
            
            word_seek_test(&log, &buffers, 25, 0, word, word_len, scratch, scratch_size, &word_seek);

        }

        {
            char word[] = "return";
            int word_len = sizeof(word) - 1;
            
            word_seek_test(&log, &buffers, 25, 1, word, word_len, scratch, scratch_size, &word_seek);
        
            printf("average normal word seek:\n");
            print_record(word_seek.expected);
            printf("\n");
        }
    }
    log_end_section(&log);
    
    log_begin_section(&log, litstr("one-hundred-single-edits"));
    {
        Record_Statistics edits;
        insert_bottom_test(&log, &buffers, 25, widths_data, 100, scratch, scratch_size, &edits);
        insert_top_test(&log, &buffers, 25, widths_data, 100, scratch, scratch_size, &edits);
        delete_bottom_test(&log, &buffers, 25, widths_data, 100, scratch, scratch_size, &edits);
        delete_top_test(&log, &buffers, 25, widths_data, 100, scratch, scratch_size, &edits);
    }
    log_end_section(&log);
    
    log_finish(&log);
    
    return(0);
}

// BOTTOM

