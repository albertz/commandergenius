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


/**

   This header contains a bunch of (mostly) system and machine
   dependent functions:

   - timers
   - current time in milliseconds and microseconds
   - debug logging
   - profiling
   - memory locking
   - checking for floating point exceptions

 */

#ifndef _FLUID_SYS_H
#define _FLUID_SYS_H

#include "fluidsynth_priv.h"



void fluid_sys_config(void);
void fluid_log_config(void);
void fluid_time_config(void);


/* Misc */

/*
#define fluid_return_val_if_fail  g_return_val_if_fail
#define fluid_return_if_fail      g_return_if_fail
#define FLUID_INLINE              inline
#define FLUID_POINTER_TO_UINT     GPOINTER_TO_UINT
#define FLUID_UINT_TO_POINTER     GUINT_TO_POINTER
#define FLUID_POINTER_TO_INT      GPOINTER_TO_INT
#define FLUID_INT_TO_POINTER      GINT_TO_POINTER
#define FLUID_N_ELEMENTS(struct)  (sizeof (struct) / sizeof (struct[0]))

#define FLUID_IS_BIG_ENDIAN       (G_BYTE_ORDER == G_BIG_ENDIAN)
*/

#include <endian.h>
#define FLUID_INLINE              inline
#define FLUID_N_ELEMENTS(struct)  (sizeof (struct) / sizeof (struct[0]))
#define FLUID_IS_BIG_ENDIAN       (__BYTE_ORDER == 4321)


/*
 * Utility functions
 */
char *fluid_strtok (char **str, char *delim);


/**

  Additional debugging system, separate from the log system. This
  allows to print selected debug messages of a specific subsystem.
 */

extern unsigned int fluid_debug_flags;

#if DEBUG

enum fluid_debug_level {
  FLUID_DBG_DRIVER = 1
};

int fluid_debug(int level, char * fmt, ...);

#else
#define fluid_debug
#endif



unsigned int fluid_curtime(void);
double fluid_utime(void);


/**
    Timers

 */

/* if the callback function returns 1 the timer will continue; if it
   returns 0 it will stop */
typedef int (*fluid_timer_callback_t)(void* data, unsigned int msec);

typedef struct _fluid_timer_t fluid_timer_t;

/*
fluid_timer_t* new_fluid_timer(int msec, fluid_timer_callback_t callback,
                               void* data, int new_thread, int auto_destroy,
                               int high_priority);
*/
//int delete_fluid_timer(fluid_timer_t* timer);
//int fluid_timer_join(fluid_timer_t* timer);
//int fluid_timer_stop(fluid_timer_t* timer);


/* No profiling */
#define fluid_profiling_print()
#define fluid_profile_ref()  0
#define fluid_profile_ref_var(name)
#define fluid_profile(_num,_ref)

#endif /* _FLUID_SYS_H */
