/* 
 * Mr. 4th Dimention - Allen Webster
 *  Four Tech
 *
 * public domain -- no warranty is offered or implied; use this code at your own risk
 * 
 * 11.11.2015
 * 
 * Code shared between history_to_replay.cpp and 4coder_test_main.cpp
 * 
 */

// TOP

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#define inline_4tech inline

inline_4tech int
CEIL32(float x){
    int extra;
    extra = ((x!=(int)(x) && x>0)?1:0);
    extra += (int)(x);
    return(extra);
}

inline_4tech int
DIVCEIL32(int n, int d) {
    int q = (n/d);
    q += (q*d < n);
    return(q);
}

inline_4tech unsigned int
ROUNDPOT32(unsigned int v){
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return(v);
}

// TODO(allen): this should actually be compiler specific, it doesn't
// have anything to do with platform
#if defined(__linux__)
#define memzero_4tech(x) memset_4tech(&(x), 0, sizeof(x))
#else
#define memzero_4tech(x) (x = {})
#endif

// BOTTOM

