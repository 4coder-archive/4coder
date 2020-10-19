/* date = October 18th 2020 3:41 pm */

#ifndef FCODER_CASEY_INDEX_H
#define FCODER_CASEY_INDEX_H

struct color_pair
{
    casey_color_id fore;
    casey_color_id back;
};

enum Indexed_Content_Element_Type
{
    ContentType_Unspecified,
    
    ContentType_Function,
    ContentType_Type,
    ContentType_Macro,
    ContentType_EnumValue,
    ContentType_ForwardDeclaration,
    
    ContentType_TODO,
    ContentType_NOTE,
    ContentType_IMPORTANT,
    ContentType_STUDY,
    
    ContentType_Count,
};

struct Indexed_Content_Element
{
    Indexed_Content_Element *next_in_hash;
    uint32_t type;
    
    String name;
    String content;
    
    color_pair color;
    
    Buffer_ID buffer_id;
    int32_t last_known_location;
};

enum Indexed_Content_Flag
{
    IndexFlag_NoNameLookup = 0x1,
};

enum Indexed_Content_Tag_Type
{
    TagType_Unspecified ,
    
    TagType_Label,
    TagType_Scope,
    TagType_Highlight,
};
struct Indexed_Content_Tag
{
    Indexed_Content_Tag *next_in_buffer;
    String content;
    
    uint32_t type;
    int32_t start;
    int32_t end;
    
    color_pair color;
};

struct Indexed_Buffer
{
    Indexed_Buffer *next_in_hash;
    Buffer_ID buffer_id;
    
    int32_t element_type_counts[ContentType_Count];
    
    // TODO(casey): I really want a growable arena here, so I don't have to manage this memory.  Everything
    // gets cleared by buffer.
    
    // TODO(casey): This should be a spatial query so it is more scalable
    Indexed_Content_Tag *first_tag;
};

struct Content_Index
{
    int32_t buffer_count;
    int32_t element_count;
    int32_t total_string_space;
    
    int32_t element_type_counts[ContentType_Count];
    
    Indexed_Buffer *buffer_hash[257];
    
    // NOTE(casey): unhashed _must_ come immediately aftter name_hash, as they are treated as contiguous
    // {
    Indexed_Content_Element *name_hash[4099];
    Indexed_Content_Element *unhashed;
    // }
};

#endif //4CODER_CASEY_INDEX_H
