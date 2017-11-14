/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 12.06.2017
 *
 * Setup macro wrappers for the logging system.
 *
 */

// TOP

#define GEN_LOG(l,m) l(m, sizeof(m)-1)
#define GEN_LOGF(l,...) do{ char space[4096]; \
    i32 length = snprintf(space, sizeof(space), __VA_ARGS__); \
    l(space, length); \
}while(0)

#if defined(IS_PLAT_LAYER)

# if defined(USE_LOG)
# define LOG(m) GEN_LOG(sysfunc.log, FNLN "\n" m)
# else
# define LOG(m)
# endif

# if defined(USE_LOGF)
# define LOGF(...) GEN_LOGF(sysfunc.log, FNLN "\n" __VA_ARGS__)
# else
# define LOGF(...)
# endif

#elif defined(IS_OBJC_LAYER)

# if defined(USE_LOG)
# define LOG(m) GEN_LOG(osx_log, FNLN "\n" m)
# else
# define LOG(m)
# endif

# if defined(USE_LOGF)
# define LOGF(...) GEN_LOGF(osx_log, FNLN "\n" __VA_ARGS__)
# else
# define LOGF(...)
# endif

#else /* Not platform layer */

# if defined(USE_LOG)
#  define LOG(s,m) GEN_LOG((s)->log, FNLN "\n" m)
# else
#  define LOG(s,m)
# endif

# if defined(USE_LOGF)
#  define LOGF(s,...) GEN_LOGF((s)->log, FNLN "\n" __VA_ARGS__)
# else
#  define LOGF(s,...)
# endif

#endif

// HACK(allen): Get rid of this dependency. Implement snprintf ourself or something.
#if defined(USE_LOG)
# include <stdio.h>
#endif

// BOTTOM

