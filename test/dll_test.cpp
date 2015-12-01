/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 12.12.2014
 *
 * Application layer for project codename "4ed"
 *
 */

// TOP

#define dll_export extern "C"

int stuff[] = {
    1, 2
};

struct Crap{
    int x;
    int *stuff;
};

Crap crap = { 0, stuff };

dll_export int
test_func(int a, int b){
    int r;
    r = a + crap.stuff[crap.x];

    crap.stuff[crap.x] = b;
    crap.x = (crap.x+1)&1;
    
    return(r);
}

// BOTTOM

