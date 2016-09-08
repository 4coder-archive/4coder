/*

A series of stress tests to more quickly verify that 4coder isn't totally
broken before I release in the future... "unit tests" perhaps, although
I guess I don't know what the "units" are... probably just "tests".

Allen Webster
18.07.2016

*/

// TOP

#define LOTS_OF_FILES "w:/4ed/data/lots_of_files"
#define TEST_FILES "w:/4ed/data/test"

#include "4coder_default_include.cpp"

#include <stdio.h>

#include <intrin.h>
#pragma intrinsic(__rdtsc)

typedef uint64_t DWORD64;

#define TEST_TIME_B(m) DWORD64 time_start = __rdtsc(), time_max = m; (void)(time_start), (void)(time_max)
#define TEST_TIME_E() DWORD64 time_total = __rdtsc() - time_start; if (time_total > time_max) {assert(!"failed timing");}
#define TEST_TIME_M(m) m = (float)(__rdtsc() - time_start) / time_max

// NOTE(allen): This testing system only verifies that everything works
// without crashing and without blowing through a fair time budget.
// These tests do not verify the correctness of the output.

CUSTOM_COMMAND_SIG(load_lots_of_files){
    // NOTE(allen): This timing restriction is based on 4GHz and 60fps
    // 4G / 60 ~= 70M  Reserving most of that time for rendering and hopefully idling
    // I set the goal of 10M for all tests.
    TEST_TIME_B(10000000);
    
    File_List list = app->get_file_list(app, literal(LOTS_OF_FILES));
    File_Info *info = list.infos;
    
    char space[1024];
    String str = make_fixed_width_string(space);
    append_ss(&str, make_lit_string(LOTS_OF_FILES));
    append_s_char(&str, '/');
    int32_t size = str.size;
    
    for (int32_t i = 0; i < list.count; ++i, ++info){
        if (!info->folder){
            append_ss(&str, make_string(info->filename, info->filename_len));
            Buffer_Summary buffer = app->create_buffer(app, str.str, str.size,
                                                       BufferCreate_Background);
            assert(buffer.size != 0);
            str.size = size;
        }
    }
    
    app->free_file_list(app, list);
    
    // TODO(allen): Pass this time test!
    //TEST_TIME_E();
}

CUSTOM_COMMAND_SIG(reopen_test){
    // NOTE(allen): This is set to roughly one percent of the frame budget
    // based on 4GHz and 60fps
    TEST_TIME_B(700000);
    
    Buffer_Summary buffer = app->create_buffer(app, literal(TEST_FILES "/basic.cpp"), 0);
    View_Summary view = app->get_active_view(app, AccessAll);
    app->view_set_buffer(app, &view, buffer.buffer_id, 0);
    
    exec_command(app, cmdid_reopen);
    
    // TODO(allen): Pass this time test!
    //TEST_TIME_E();
}

CUSTOM_COMMAND_SIG(run_all_tests){
    exec_command(app, load_lots_of_files);
    exec_command(app, reopen_test);
}

#if 0
CUSTOM_COMMAND_SIG(generate_stop_spots_test_data){
    Buffer_Summary buffer = app->create_buffer(app, literal(LOTS_OF_FILES "/4ed.cpp"), 0);
    View_Summary view = app->get_active_view(app, AccessAll);
    app->view_set_buffer(app, &view, buffer.buffer_id, 0);
    
    FILE *file = fopen(TEST_FILES "/stop_spots_data", "wb");
    
    if (file){
        Partial_Cursor curs;
        int32_t pos;
        
        app->buffer_compute_cursor(app, &buffer, seek_line_char(316, 29), &curs);
        fwrite(&curs.pos, 4, 1, file);
        
        static Seek_Boundary_Flag flag_set[] = {
            BoundaryWhitespace,
            BoundaryToken,
            BoundaryAlphanumeric,
            BoundaryCamelCase,
            BoundaryWhitespace | BoundaryToken,
            BoundaryWhitespace | BoundaryAlphanumeric,
            BoundaryToken | BoundaryCamelCase,
        };
        
        for (int32_t flag_i = 0; flag_i < ArrayCount(flag_set); ++flag_i){
            for (int32_t seek_forward = 0; seek_forward <= 1; ++seek_forward){
                pos = curs.pos;
                for (int32_t i = 0; i < 100; ++i){
                    pos = app->buffer_boundary_seek(app, &buffer, pos, seek_forward, flag_set[flag_i]);
                    fwrite(&pos, 4, 1, file);
                }
            }
        }
        
        fclose(file);
    }
}
#endif

static void
fcheck(int32_t x, FILE *file){
    int32_t x0 = 0;
    fread(&x0, 4, 1, file);
    Assert(x == x0);
}

CUSTOM_COMMAND_SIG(stop_spots_test){
    Buffer_Summary buffer = app->create_buffer(app, literal(LOTS_OF_FILES "/4ed.cpp"), 0);
    View_Summary view = app->get_active_view(app, AccessAll);
    app->view_set_buffer(app, &view, buffer.buffer_id, 0);
    
    FILE *file = fopen(TEST_FILES "/stop_spots_data", "rb");
    
    if (file){
        Partial_Cursor curs;
        int32_t pos;
        
        app->buffer_compute_cursor(app, &buffer, seek_line_char(316, 29), &curs);
        fcheck(curs.pos, file);
        
        static Seek_Boundary_Flag flag_set[] = {
            BoundaryWhitespace,
            BoundaryToken,
            BoundaryAlphanumeric,
            BoundaryCamelCase,
            BoundaryWhitespace | BoundaryToken,
            BoundaryWhitespace | BoundaryAlphanumeric,
            BoundaryToken | BoundaryCamelCase,
        };
        
        for (int32_t flag_i = 0; flag_i < ArrayCount(flag_set); ++flag_i){
            for (int32_t seek_forward = 0; seek_forward <= 1; ++seek_forward){
                pos = curs.pos;
                for (int32_t i = 0; i < 100; ++i){
                    pos = buffer_boundary_seek(app, &buffer, pos, seek_forward, flag_set[flag_i]);
                    fcheck(pos, file);
                }
            }
        }
        
        fclose(file);
    }
}

static void
test_get_bindings(Bind_Helper *context){
    begin_map(context, mapid_global);
    
    //bind(context, key_f3, MDFR_NONE, run_all_tests);
    bind(context, key_f3, MDFR_NONE, stop_spots_test);
    
    end_map(context);
}

#define BIND_4CODER_TESTS(context) test_get_bindings(context)

#include "power/4coder_experiments.cpp"


// BOTTOM

