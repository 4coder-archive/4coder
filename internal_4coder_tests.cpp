/*

A series of stress tests to more quickly verify that 4coder isn't totally
broken before I release in the future... "unit tests" perhaps, although
I guess I don't know what the "units" are... probably more like "tests".

Allen Webster
18.07.2016

*/

// TOP

#define LOTS_OF_FILES "w:/4ed/data/lots_of_files"
#define TEST_FILES "w:/4ed/data/test"

#include "4coder_default_include.cpp"
#include "4coder_default_building.cpp"

#include <Windows.h>

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
    append(&str, LOTS_OF_FILES);
    append(&str, '/');
    int size = str.size;
    
    for (int i = 0; i < list.count; ++i, ++info){
        if (!info->folder){
            append(&str, make_string(info->filename, info->filename_len));
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

static void
test_get_bindings(Bind_Helper *context){
    begin_map(context, mapid_global);
    
    bind(context, key_f3, MDFR_NONE, run_all_tests);
    
    end_map(context);
}

#define BIND_4CODER_TESTS(context) test_get_bindings(context)

#include "power/4coder_experiments.cpp"


// BOTTOM

