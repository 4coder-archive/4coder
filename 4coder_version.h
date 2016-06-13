#define MAJOR 4
#define MINOR 0
#define PATCH 8

#define VN__(a,b,c) #a"."#b"."#c
#define VN_(a,b,c) VN__(a,b,c)
#define VERSION_NUMBER VN_(MAJOR,MINOR,PATCH)
#define VERSION_STRING "alpha " VERSION_NUMBER

#ifdef FRED_SUPER
#define VERSION_TYPE " super!"
#else
#define VERSION_TYPE
#endif
#define VERSION VERSION_STRING VERSION_TYPE
