#define MAJOR 4
#define MINOR 1
#define PATCH 8

// string
#define VN__(a,b,c) #a "." #b "." #c
#define VN_(a,b,c) VN__(a,b,c)
#define VERSION_NUMBER VN_(MAJOR,MINOR,PATCH)
#define VERSION_STRING "beta " VERSION_NUMBER

#define ST__(s) #s
#define ST_(s) ST__(s)
#define MAJOR_STR ST_(MAJOR)
#define MINOR_STR ST_(MINOR)
#define PATCH_STR ST_(PATCH)

#if defined(FRED_SUPER)
#define VERSION_TYPE " super!"
#else
#define VERSION_TYPE
#endif

// string
#define VN__(a,b,c) #a "." #b "." #c
#define VN_(a,b,c) VN__(a,b,c)
#define VERSION_NUMBER VN_(MAJOR,MINOR,PATCH)
#define VERSION_STRING "beta " VERSION_NUMBER

#define VERSION VERSION_STRING VERSION_TYPE

#define WINDOW_NAME "4coder: " VERSION

// long string
#define L_VN__(a,b,c) L#a L"." L#b L"." L#c
#define L_VN_(a,b,c) L_VN__(a,b,c)
#define L_VERSION_NUMBER L_VN_(MAJOR,MINOR,PATCH)
#define L_VERSION_STRING L"beta " L_VERSION_NUMBER

#if defined(FRED_SUPER)
#define L_VERSION_TYPE L" super!"
#else
#define L_VERSION_TYPE
#endif

#define L_VERSION L_VERSION_STRING L_VERSION_TYPE

#define L_WINDOW_NAME L"4coder: " L_VERSION