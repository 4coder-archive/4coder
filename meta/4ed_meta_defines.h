/*
4tech_meta_defines.h - Header to setup metaprograms.
By Allen Webster
Created 21.01.2017 (dd.mm.yyyy)
*/

// TOP

#if !defined(FTECH_META_DEFINES_H)
#define FTECH_META_DEFINES_H

#include "../4ed_defines.h"

#include <setjmp.h>
#include <stdio.h>

global jmp_buf META_ASSERT_ENV;
global char* META_ASSERT_MSG;

internal void
__meta_finish__(){
    if (META_ASSERT_MSG != 0){
        printf("%s\n", META_ASSERT_MSG);
    }
}

#define META_FINISH() } __META_FINISH__: __meta_finish__()

#define LINE_STR STR_(__LINE__)

#if defined(Assert)
# undef Assert
#endif

#define Assert(c) do { if (!(c)) { META_ASSERT_MSG = __FILE__":"LINE_STR": "#c; longjmp(META_ASSERT_ENV, 1); } } while(0)

#define META_BEGIN() META_ASSERT_MSG = 0; if (setjmp(META_ASSERT_ENV) == 1){ goto __META_FINISH__; } { 

char STANDARD_DISCLAIMER[] = 
"no warranty implied; use at your own risk\n"
"\n"
"This software is in the public domain. Where that dedication is not\n"
"recognized, you are granted a perpetual, irrevocable license to copy,\n"
"distribute, and modify this file as you see fit.\n\n";

#endif

// BOTTOM

