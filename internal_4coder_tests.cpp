/*

A series of stress tests to more quickly verify that 4coder isn't totally
broken before I release in the future... "unit tests" perhaps, although
I guess I don't know what the "units" are... probably more like "tests".

Allen Webster
18.07.2016

*/

// TOP

#define LOTS_OF_FILES "w:/4ed/data/lots_of_files"

#define TEST_TIME_B()
#define TEST_TIME_E()



#include "4coder_default_include.cpp"
#include "4coder_default_building.cpp"

CUSTOM_COMMAND_SIG(load_lots_of_files){
    TEST_TIME_B();
    
    File_List list = app->get_file_list(app, literal(LOTS_OF_FILES));
    File_Info *info = list.infos;
    
    for (int i = 0; i < list.count; ++i, ++info){
        if (!info->folder){
            app->create_buffer(app, info->filename, info->filename_len, BufferCreate_Background);
        }
    }
    
    app->free_file_list(app, list);
    
    TEST_TIME_E();
}

static void
test_get_bindings(Bind_Helper *context){
    begin_map(context, mapid_global);
    
    bind(context, key_f3, MDFR_NONE, load_lots_of_files);
    
    end_map(context);
}

#define BIND_4CODER_TESTS(context) test_get_bindings(context)

#include "power/4coder_experiments.cpp"


// BOTTOM

