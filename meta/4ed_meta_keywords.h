/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 10.07.2017
 *
 * Standard meta-parser configuration for 4coder code.
 *
 */

// TOP

#if !defined(FRED_META_KEYWORDS_H)
#define FRED_META_KEYWORDS_H

internal Meta_Keywords meta_keywords[] = {
    {make_lit_string("API_EXPORT")        , Item_Function } ,
    {make_lit_string("API_EXPORT_INLINE") , Item_Function } ,
    {make_lit_string("API_EXPORT_MACRO")  , Item_Macro    } ,
    {make_lit_string("CPP_NAME")          , Item_CppName  } ,
    {make_lit_string("TYPEDEF") , Item_Typedef } ,
    {make_lit_string("STRUCT")  , Item_Struct  } ,
    {make_lit_string("UNION")   , Item_Union   } ,
    {make_lit_string("ENUM")    , Item_Enum    } ,
};

#endif

// BOTTOM

