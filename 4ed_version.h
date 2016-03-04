#define MAJOR 3
#define MINOR 4
#define PATCH 5
#define VERSION_NUMBER "alpha 3.4.5"
#ifdef FRED_SUPER
#define VERSION_TYPE " super!"
#else
#define VERSION_TYPE ""
#endif
#define VERSION VERSION_NUMBER VERSION_TYPE
