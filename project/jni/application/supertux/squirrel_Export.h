
#ifndef SQUIRREL_API_H
#define SQUIRREL_API_H

#ifdef squirrel_BUILT_AS_STATIC
#  define SQUIRREL_API
#  define SQUIRREL_NO_EXPORT
#else
#  ifndef SQUIRREL_API
#    ifdef squirrel_EXPORTS
        /* We are building this library */
#      define SQUIRREL_API 
#    else
        /* We are using this library */
#      define SQUIRREL_API 
#    endif
#  endif

#  ifndef SQUIRREL_NO_EXPORT
#    define SQUIRREL_NO_EXPORT 
#  endif
#endif

#ifndef SQUIRREL_DEPRECATED
#  define SQUIRREL_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SQUIRREL_DEPRECATED_EXPORT
#  define SQUIRREL_DEPRECATED_EXPORT SQUIRREL_API SQUIRREL_DEPRECATED
#endif

#ifndef SQUIRREL_DEPRECATED_NO_EXPORT
#  define SQUIRREL_DEPRECATED_NO_EXPORT SQUIRREL_NO_EXPORT SQUIRREL_DEPRECATED
#endif

#define DEFINE_NO_DEPRECATED 0
#if DEFINE_NO_DEPRECATED
# define SQUIRREL_NO_DEPRECATED
#endif

#endif
