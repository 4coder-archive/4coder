/*
4coder_doc_content_types.h - Documentation content types
*/

// TOP

#if !defined(FRED_DOC_CONTENT_TYPES_H)
#define FRED_DOC_CONTENT_TYPES_H

typedef i32 Doc_Month;
enum{
    None,
     January,
     February,
     March,
     April,
     May,
     June,
     July,
     August,
     September,
     October,
     November,
     December,
};

struct Doc_Date{
    i32 day;
    Doc_Month month;
    i32 year;
};

typedef i32 Doc_Content_Emphasis;
enum{
    DocContentEmphasis_Normal,
    DocContentEmphasis_SmallHeader,
    DocContentEmphasis_Heavy,
    DocContentEmphasis_Stylish,
    DocContentEmphasis_Code,
};
struct Doc_Content{
    Doc_Content *next;
    String_Const_u8 text;
    String_Const_u8 page_link;
    String_Const_u8 block_link;
    Doc_Content_Emphasis emphasis;
};
struct Doc_Content_List{
    Doc_Content *first;
    Doc_Content *last;
    umem total_size;
    i32 node_count;
};

typedef i32 Doc_Code_Language;
enum{
    DocCodeLanguage_None,
    DocCodeLanguage_Cpp,
    DocCodeLanguage_Bat,
};
struct Doc_Code_Sample{
    Doc_Code_Sample *next;
    String_Const_u8 contents;
    Doc_Code_Language language;
};
struct Doc_Code_Sample_List{
    Doc_Code_Sample *first;
    Doc_Code_Sample *last;
    i32 count;
};

typedef i32 Doc_Paragraph_Kind;
enum{
    DocParagraphKind_Text,
    DocParagraphKind_Code,
    DocParagraphKind_Table,
};
struct Doc_Paragraph{
    Doc_Paragraph *next;
    Doc_Paragraph_Kind kind;
    union{
        Doc_Content_List text;
        Doc_Code_Sample_List code;
        struct{
            Vec2_i32 dim;
            Doc_Content_List *vals;
        } table;
    };
};

struct Doc_Block{
    Doc_Block *next;
    
    struct Doc_Page *owner;
    
    String_Const_u8 name;
    
    Doc_Paragraph *first_par;
    Doc_Paragraph *last_par;
    i32 par_count;
};

struct Doc_Block_Ptr{
    Doc_Block_Ptr *next;
    Doc_Block *block;
};

struct Doc_Block_List{
    Doc_Block_Ptr *first;
    Doc_Block_Ptr *last;
    i32 count;
};

struct Doc_Page{
    Doc_Page *next;
    
    struct Doc_Cluster *owner;
    
    String_Const_u8 title;
    String_Const_u8 name;
    
    Doc_Block *first_block;
    Doc_Block *last_block;
    i32 block_count;
    
    Doc_Block_List quick_jumps;
};

struct Doc_Log{
    Doc_Log *next;
    String_Const_u8 content;
};

struct Doc_Cluster{
    String_Const_u8 title;
    String_Const_u8 name;
    Doc_Date gen_date;
    
    Doc_Page *first_page;
    Doc_Page *last_page;
    i32 page_count;
    
    Doc_Log *first_log;
    Doc_Log *last_log;
};

#endif

// BOTTOM

