/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 14.08.2019
 *
 * Log parser.
 *
 */

// TOP

#if !defined(FCODER_LOG_PARSER_H)
#define FCODER_LOG_PARSER_H

typedef i64 Log_Tag_Kind;
enum{
    LogTagKind_Null,
    LogTagKind_Integer,
    LogTagKind_String,
};

typedef i32 Log_String_Source;
enum{
    LogParse_ExternalString,
    LogParse_PreAllocatedString,
};

struct Log_Tag_Value{
    Log_Tag_Kind kind;
    union{
        u64 value;
        i64 value_s;
    };
};

struct Log_Sort_Key{
    Log_Tag_Value value;
    i32 number;
};

struct Log_Tag_Name_Value{
    u64 name;
    Log_Tag_Value value;
};

struct Log_Tag{
    Log_Tag *next;
    u64 name;
    Log_Tag_Value value;
};

struct Log_Event{
    Log_Event *next;
    u64 src_file_name;
    u64 event_name;
    u64 line_number;
    
    Log_Tag *first_tag;
    Log_Tag *last_tag;
    i32 tag_count;
    
    i32 event_number;
    
    Table_u64_u64 tag_name_to_tag_ptr_table;
};

struct Log_Event_Ptr_Node{
    Log_Event_Ptr_Node *next;
    Log_Event *event;
};

struct Log_Event_List{
    Log_Event_Ptr_Node *first;
    Log_Event_Ptr_Node *last;
    i32 count;
};

struct Log_Event_Ptr_Array{
    Log_Event **events;
    i32 count;
};

struct Log_Tag_Value_Array{
    Log_Tag_Value *vals;
    i32 count;
};

struct Log_Parse{
    Arena *arena;
    
    Log_Event *first_event;
    Log_Event *last_event;
    i32 event_count;
    
    u64 string_id_counter;
    Table_Data_u64 string_to_id_table;
    Table_u64_Data id_to_string_table;
    
    Table_Data_u64 tag_value_to_event_list_table;
    Table_u64_u64 tag_name_to_event_list_table;
};

////////////////////////////////

struct Log_Graph_Thread_Bucket{
    Log_Graph_Thread_Bucket *next;
    Range_i32 range;
    b32 had_a_tag;
    u64 thread_id_value;
};

struct Log_Graph_Box{
    Log_Graph_Box *next;
    Rect_f32 rect;
    Log_Event *event;
};

typedef i32 Log_Filter_Kind;
enum{
    LogFilter_ERROR,
    LogFilter_TagValue,
    LogFilter_Tag,
};

struct Log_Filter{
    Log_Filter *next;
    Log_Filter *prev;
    Log_Filter_Kind kind;
    u64 tag_name_code;
    Log_Tag_Value tag_value;
};

struct Log_Filter_Set{
    Log_Filter filters_memory[20];
    Log_Filter *free_filters;
    Log_Filter *first;
    Log_Filter *last;
    i32 count;
    i32 alter_counter;
};

typedef i32 Log_Graph_List_Tab;
enum{
    LogTab_ERROR,
    LogTab_Filters,
    LogTab_Previews,
    LogTab_COUNT,
};

struct Log_Graph{
    b32 holding_temp;
    Temp_Memory temp;
    Rect_f32 layout_region;
    Face_ID face_id;
    i32 filter_alter_counter;
    i32 preview_alter_counter;
    Log_Graph_List_Tab tab;
    Rect_f32 details_region;
    Log_Event_List filtered_list;
    Log_Event_Ptr_Array event_array;
    Log_Graph_Thread_Bucket *first_bucket;
    Log_Graph_Thread_Bucket *last_bucket;
    i32 bucket_count;
    Log_Graph_Box *first_box;
    Log_Graph_Box *last_box;
    i32 box_count;
    f32 y_scroll;
    f32 max_y_scroll;
    Log_Event *selected_event;
    b32 has_unused_click;
    Vec2_f32 unused_click;
};

#endif

// BOTTOM

