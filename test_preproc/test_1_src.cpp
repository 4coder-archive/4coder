
// F0

#if !defined(FOO)
#define FOO 0

FOO;

// F1

#undef FOO
#define FOO 1

FOO;

// F2

#undef FOO
#define FOO 2

#if !defined(FOO)

2;

#else

FOO;

#endif

// F3

#undef FOO
#define BAR 3

#ifdef FOO

FOO;

#elif defined(BAR)

BAR;

#else

3;

#endif

// F4

#define FOO 4

#if FOO < BAR

BAR;

#else

FOO;

#endif

// F5

#undef BAR
#define BAR(a) (5 + (a))

BAR(0);

// F9

BAR(FOO);

// F14

BAR(BAR(FOO));

// F15

#undef BAR
#define BAR(a,b,c) ((a) + (b) + (c))

#undef FOO
#define FOO(n) (4 + (n))

BAR(FOO(0), FOO(1), FOO(2));

// F16

#undef BAR
#define BAR(a,...) (a(__VA_ARGS__))

BAR(baz, 4, 4, 4, 4);

// F17

#undef FOO
#define FOO FOO

FOO;

// F18

#undef FOO
#define FOO BAR

#undef BAR
#define BAR FOO

BAR;

// F20

#undef FOO
#define FOO(a) FOO(a) + 20

FOO(0);

// L1

#undef FOO
#define FOO(x)\
x(a,b,c)\
x(d,e,f)\
x(g,h,i)\
x(j,k,l)\
x(m,n,o)\
x(p,q,r)\
x(s,t,u)

#undef BAR
#define BAR(x,y,z) (x) + 2*(y) + (z)/2 +

FOO(BAR) 0;

#endif

