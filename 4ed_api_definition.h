/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 03.10.2019
 *
 * System API definition types.
 *
 */

// TOP

#if !defined(FRED_API_DEFINITION_H)
#define FRED_API_DEFINITION_H

struct API_Param{
    API_Param *next;
    String_Const_u8 type_name;
    String_Const_u8 name;
};

struct API_Param_List{
    API_Param *first;
    API_Param *last;
    i32 count;
};

struct API_Call{
    API_Call *next;
    String_Const_u8 name;
    String_Const_u8 return_type;
    String_Const_u8 location_string;
    API_Param_List params;
};

typedef i32 API_Type_Structure_Kind;
enum{
    APITypeStructureKind_Struct,
    APITypeStructureKind_Union,
};
struct API_Type_Structure{
    API_Type_Structure_Kind kind;
    List_String_Const_u8 member_names;
    String_Const_u8 definition_string;
};

struct API_Enum_Value{
    API_Enum_Value *next;
    String_Const_u8 name;
    String_Const_u8 val;
};
struct API_Type_Enum{
    String_Const_u8 type_name;
    API_Enum_Value *first_val;
    API_Enum_Value *last_val;
    i32 val_count;
};

struct API_Type_Typedef{
    String_Const_u8 name;
    String_Const_u8 definition_text;
};

typedef i32 API_Type_Kind;
enum{
    APITypeKind_Structure,
    APITypeKind_Enum,
    APITypeKind_Typedef,
};
struct API_Type{
    API_Type *next;
    API_Type_Kind kind;
    String_Const_u8 name;
    String_Const_u8 location_string;
    union{
        API_Type_Structure struct_type;
        API_Type_Enum enum_type;
        API_Type_Typedef typedef_type;
    };
};

struct API_Definition{
    API_Definition *next;
    
    API_Call *first_call;
    API_Call *last_call;
    i32 call_count;
    
    API_Type *first_type;
    API_Type *last_type;
    i32 type_count;
    
    String_Const_u8 name;
};

struct API_Definition_List{
    API_Definition *first;
    API_Definition *last;
    i32 count;
};

typedef u32 API_Generation_Flag;
enum{
    APIGeneration_NoAPINameOnCallables = 1,
};

typedef u32 API_Check_Flag;
enum{
    APICheck_ReportMissingAPI = 1,
    APICheck_ReportExtraAPI = 2,
    APICheck_ReportMismatchAPI = 4,
};
enum{
    APICheck_ReportAll = APICheck_ReportMissingAPI|APICheck_ReportExtraAPI|APICheck_ReportMismatchAPI,
};

#endif

// BOTTOM

