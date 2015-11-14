/*
 * Mr. 4th Dimention - Allen Webster
 * 
 * 16.05.2015
 * 
 * Fascilities available for development but not intended for shipping.
 * 
 */

// TOP

#if FRED_INTERNAL == 1
#define ProfileStart_(name, start, counter, hit, thread, n, c)

#define ProfileEnd_(name, start, counter, hit, thread)

#define ProfileMoment_(name, counter, thread)

#if 0

#define ProfileStart(name) char *_pname_##name; i64 _pstart_##name;     \
    i32 _pcounter_##name; u32 _phit_##name;                             \
    ProfileStart_(_pname_##name, _pstart_##name, _pcounter_##name,      \
                  _phit_##name, system->thread_get_id(thread),          \
                  #name, __COUNTER__)

#define ProfileEnd(name) ProfileEnd_(_pname_##name, _pstart_##name,     \
                                     _pcounter_##name, _phit_##name,    \
                                     system->thread_get_id(thread))

#define ProfileMoment(name, thread) ProfileMoment_(#name, __COUNTER__, thread)
#define ProfileMomentFunction() ProfileMoment_(__FUNCTION__, __COUNTER__, 0)

#else

#define ProfileStart(name)
#define ProfileEnd(name)
#define ProfileMoment(name)
#define ProfileMomentFunction()

#endif



struct Sys_Bubble : public Bubble{
    i32 line_number;
    char *file_name;
};

#else

#define ProfileStart(name)
#define ProfileEnd(name)
#define ProfileMoment(name)
#define ProfileMomentFunction()

#endif

// BOTTOM

