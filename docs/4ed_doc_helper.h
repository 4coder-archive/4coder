/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 04.12.2019
 *
 * Definition of information contained in 4coder documentation.
 *
 */

// TOP

#if !defined(FRED_DOC_HELPER_H)
#define FRED_DOC_HELPER_H

struct Doc_Function{
    API_Call *call;
    API_Param *param_iter;
    Doc_Page *page;
    Doc_Block *brief;
    Doc_Block *sig;
    Doc_Block *params;
    Doc_Block *ret;
    Doc_Block *det;
    Doc_Block *examples;
    Doc_Block *rel;
};

#endif

// BOTTOM

