/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */


#ifndef _FLUIDSYNTH_PRIV_H
#define _FLUIDSYNTH_PRIV_H

//#include <glib.h>

#if HAVE_CONFIG_H
#include "config.h"
#endif

#if defined(__POWERPC__) && !(defined(__APPLE__) && defined(__MACH__))
#include "config_maxmsp43.h"
#endif

#if defined(WIN32) && !defined(MINGW32)
#include "config_win32.h"
#endif

#if HAVE_STRING_H
#include <string.h>
#endif

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

#if HAVE_STDIO_H
#include <stdio.h>
#endif

#if HAVE_MATH_H
#include <math.h>
#endif

#if HAVE_ERRNO_H
#include <errno.h>
#endif

#if HAVE_STDARG_H
#include <stdarg.h>
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_FCNTL_H
#include <fcntl.h>
#endif

#if HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#if HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif

#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#if HAVE_LIMITS_H
#include <limits.h>
#endif

#if HAVE_PTHREAD_H
#include <pthread.h>
#endif

#if HAVE_IO_H
#include <io.h>
#endif

#if HAVE_WINDOWS_H
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#endif

/* MinGW32 special defines */
#ifdef MINGW32

#include <stdint.h>
#define snprintf _snprintf
#define vsnprintf _vsnprintf

#define DSOUND_SUPPORT 1
#define WINMIDI_SUPPORT 1
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#endif

/* Darwin special defines (taken from config_macosx.h) */
#ifdef DARWIN
#define MACINTOSH
#define __Types__
#define WITHOUT_SERVER 1
#endif


#include "fluidsynth.h"


/***************************************************************
 *
 *         BASIC TYPES
 */

#if defined(WITH_FLOAT)
typedef float fluid_real_t;
#else
typedef double fluid_real_t;
#endif


#if defined(WIN32)
typedef SOCKET fluid_socket_t;
#else
typedef int fluid_socket_t;
#define INVALID_SOCKET -1
#endif

#define g_newa(type, len) (type *) alloca(len)

#if defined(SUPPORTS_VLA)
#  define FLUID_DECLARE_VLA(_type, _name, _len) \
     _type _name[_len]
#else
#  define FLUID_DECLARE_VLA(_type, _name, _len) \
     _type* _name = g_newa(_type, (_len))
#endif


/** Integer types  */
//typedef gint8              sint8;
typedef uint8_t             uint8;
//typedef gint16             sint16;
//typedef guint16            uint16;
typedef int32_t             sint32;
typedef uint32_t            uint32;
//typedef gint64             sint64;
//typedef guint64            uint64;


/***************************************************************
 *
 *       FORWARD DECLARATIONS
 */
typedef struct _fluid_env_data_t fluid_env_data_t;
typedef struct _fluid_adriver_definition_t fluid_adriver_definition_t;
typedef struct _fluid_channel_t fluid_channel_t;
typedef struct _fluid_tuning_t fluid_tuning_t;
typedef struct _fluid_hashtable_t  fluid_hashtable_t;
typedef struct _fluid_client_t fluid_client_t;
typedef struct _fluid_server_socket_t fluid_server_socket_t;
typedef struct _fluid_sample_timer_t fluid_sample_timer_t;

/***************************************************************
 *
 *                      CONSTANTS
 */

#define FLUID_BUFSIZE                64         /**< FluidSynth internal buffer size (in samples) */
#define FLUID_MAX_EVENTS_PER_BUFSIZE 1024       /**< Maximum queued MIDI events per #FLUID_BUFSIZE */
#define FLUID_MAX_RETURN_EVENTS      1024       /**< Maximum queued synthesis thread return events */
#define FLUID_MAX_EVENT_QUEUES       16         /**< Maximum number of unique threads queuing events */
#define FLUID_DEFAULT_AUDIO_RT_PRIO  60         /**< Default setting for audio.realtime-prio */
#define FLUID_DEFAULT_MIDI_RT_PRIO   50         /**< Default setting for midi.realtime-prio */

#ifndef PI
#define PI                          3.141592654
#endif

/***************************************************************
 *
 *                      SYSTEM INTERFACE
 */
typedef FILE*  fluid_file;

#define FLUID_MALLOC(_n)             malloc(_n)
#define FLUID_REALLOC(_p,_n)         realloc(_p,_n)
#define FLUID_NEW(_t)                (_t*)malloc(sizeof(_t))
#define FLUID_ARRAY(_t,_n)           (_t*)malloc((_n)*sizeof(_t))
#define FLUID_FREE(_p)               free(_p)
#define FLUID_FOPEN(_f,_m)           fopen(_f,_m)
#define FLUID_FCLOSE(_f)             fclose(_f)
#define FLUID_FREAD(_p,_s,_n,_f)     fread(_p,_s,_n,_f)
#define FLUID_FSEEK(_f,_n,_set)      fseek(_f,_n,_set)
#define FLUID_MEMCPY(_dst,_src,_n)   memcpy(_dst,_src,_n)
#define FLUID_MEMSET(_s,_c,_n)       memset(_s,_c,_n)
#define FLUID_STRLEN(_s)             strlen(_s)
#define FLUID_STRCMP(_s,_t)          strcmp(_s,_t)
#define FLUID_STRNCMP(_s,_t,_n)      strncmp(_s,_t,_n)
#define FLUID_STRCPY(_dst,_src)      strcpy(_dst,_src)
#define FLUID_STRNCPY(_dst,_src,_n)  strncpy(_dst,_src,_n)
#define FLUID_STRCHR(_s,_c)          strchr(_s,_c)
#define FLUID_STRRCHR(_s,_c)         strrchr(_s,_c)
#ifdef strdup
#define FLUID_STRDUP(s)              strdup(s)
#else
#define FLUID_STRDUP(s) 		    FLUID_STRCPY(FLUID_MALLOC(FLUID_STRLEN(s) + 1), s)
#endif
#define FLUID_SPRINTF                sprintf
#define FLUID_FPRINTF                fprintf

#define fluid_clip(_val, _min, _max) \
{ (_val) = ((_val) < (_min))? (_min) : (((_val) > (_max))? (_max) : (_val)); }

#if WITH_FTS
#define FLUID_PRINTF                 post
#define FLUID_FLUSH()
#else
#define FLUID_PRINTF                 printf
#define FLUID_FLUSH()                fflush(stdout)
#endif

#define FLUID_LOG                    fluid_log

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif


#define FLUID_ASSERT(a,b)
#define FLUID_ASSERT_P(a,b)

char* fluid_error(void);


/* Internationalization */
#define _(s) s

#define TRUE 1
#define FALSE 0

#if !(defined (G_STMT_START) && defined (G_STMT_END))
#  define G_STMT_START  do
#  define G_STMT_END    while (0)
#endif
typedef uint32_t fluid_private_t;

#define fluid_rec_mutex_lock(X)
#define fluid_rec_mutex_unlock(X)
typedef void * fluid_mutex_t;
#define fluid_private_free(X)

typedef int gboolean;
typedef int gint;

#define fluid_mutex_destroy(_m)
#define fluid_mutex_init(_m)
#define fluid_mutex_lock(X)
#define fluid_mutex_unlock(X)

#define g_return_if_fail(expr)			G_STMT_START{ (void)0; }G_STMT_END
#define g_return_val_if_fail(expr,val)		G_STMT_START{ (void)0; }G_STMT_END

#define fluid_return_val_if_fail  g_return_val_if_fail
#define fluid_return_if_fail      g_return_if_fail

#include <SDL_endian.h>
#define GUINT16_FROM_LE(X) SDL_SwapLE16(X)
#define GUINT32_FROM_LE(X) SDL_SwapLE32(X)
#define GINT16_FROM_LE(X) SDL_SwapLE16(X)
#define GINT32_FROM_LE(X) SDL_SwapLE32(X)

#define fluid_check_fpe(X)

#define fluid_ostream_printf(x, y)

static inline void fluid_atomic_int_add(int * x, int y) { *x += y; };
static inline void fluid_atomic_int_set(int * x, int y) { *x = y; };
static inline int fluid_atomic_int_get(int * x) { return *x; };
static inline void fluid_atomic_int_inc(int * x) { * x += 1; };
static inline int fluid_atomic_int_compare_and_exchange( int* reg, int oldval, int newval) 
{
  int old_reg_val = *reg;
  if (old_reg_val == oldval)
     *reg = newval;
  return old_reg_val;
};

static inline int fluid_atomic_int_exchange_and_add(int * x, int y) { int oldval = *x; *x += y; return oldval; };

static inline void fluid_atomic_float_set(float * x, float y) { *x = y; };
static inline float fluid_atomic_float_get(float * x) { return *x; };

#define g_atomic_int_set fluid_atomic_int_set
#define g_atomic_int_get fluid_atomic_int_get

#define fluid_atomic_pointer_get(X) (*X)
#define fluid_atomic_pointer_set(X, Y) *(X) = Y

#define FLUID_POINTER_TO_INT(X) (int32_t)(X)
#define FLUID_POINTER_TO_UINT(X) (int32_t)(X)
#define FLUID_INT_TO_POINTER(X) (void *)(X)
#define FLUID_UINT_TO_POINTER(X) (void *)(X)
#define GINT_TO_POINTER(X) FLUID_INT_TO_POINTER(X)
#define GUINT_TO_POINTER(X) FLUID_UINT_TO_POINTER(X)
#define GPOINTER_TO_INT(X) FLUID_POINTER_TO_INT(X)
#define GPOINTER_TO_UINT(X) FLUID_POINTER_TO_UINT(X)

#define fluid_mlock(_p,_n)      0
#define fluid_munlock(_p,_n)

#endif /* _FLUIDSYNTH_PRIV_H */
