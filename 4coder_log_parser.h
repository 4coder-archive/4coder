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

#endif

// BOTTOM

