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
    {string_litinit("API_EXPORT")        , Item_Function } ,
    {string_litinit("API_EXPORT_INLINE") , Item_Function } ,
    {string_litinit("API_EXPORT_MACRO")  , Item_Macro    } ,
    {string_litinit("CPP_NAME")          , Item_CppName  } ,
    {string_litinit("TYPEDEF") , Item_Typedef } ,
    {string_litinit("STRUCT")  , Item_Struct  } ,
    {string_litinit("UNION")   , Item_Union   } ,
    {string_litinit("ENUM")    , Item_Enum    } ,
};

#endif

// BOTTOM

