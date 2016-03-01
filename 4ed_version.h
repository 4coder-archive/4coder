/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 01.03.2016
 *
 * Shared header for version stuff
 *
 */

#define VERSION_NUMBER "alpha 3.4.4"

#ifdef FRED_SUPER
#define VERSION_TYPE " super!"
#else
#define VERSION_TYPE ""
#endif

#define VERSION VERSION_NUMBER VERSION_TYPE

