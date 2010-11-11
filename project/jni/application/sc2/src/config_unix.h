/* This file contains some compile-time configuration options for *nix
 * systems.
 * config_unix.h is generated from config_unix.h.in by build.sh
 * When building on MS Windows using build.sh (MinGW, Cygwin),
 * config_win.h is generated from src/config_win.h.in.
 * When using MSVC on MS Windows, you'll have to edit src/msvc++/config.h
 * manually if you want anything else than the defaults.
 */

#ifndef _CONFIG_UNIX_H
#define _CONFIG_UNIX_H

/* Directory where the UQM game data is located */
#define CONTENTDIR ""

/* Directory where game data will be stored */
#define USERDIR "/data/data/com.sourceforge.sc2/files/config/"

/* Directory where config files will be stored */
#define CONFIGDIR USERDIR

/* Directory where supermelee teams will be stored */
#define MELEEDIR "${UQM_CONFIG_DIR}/teams/"

/* Directory where save games will be stored */
#define SAVEDIR "${UQM_CONFIG_DIR}/save/"

/* Defined if words are stored with the most significant byte first */
#ifdef __ARMEL__
#undef WORDS_BIGENDIAN
#endif

/* Defined if your system has readdir_r of its own */
#define HAVE_READDIR_R 1

/* Defined if your system has setenv of its own */
#ifndef HAVE_SETENV
#define HAVE_SETENV 1
#endif

/* Defined if your system has strupr of its own */
#undef HAVE_STRUPR

/* Defined if your system has strcasecmp of its own */
#define HAVE_STRCASECMP_UQM 1
		// Not using "HAVE_STRCASECMP" as that conflicts with SDL.

/* Defined if your system has stricmp of its own */
#undef HAVE_STRICMP

/* Defined if your system has getopt_long */
#define HAVE_GETOPT_LONG 1

/* Defined if your system has iswgraph of its own*/
#define HAVE_ISWGRAPH 1

/* Defined if your system has wchar_t of its own */
#define HAVE_WCHAR_T 1

/* Defined if your system has wint_t of its own */
#define HAVE_WINT_T 1

/* Defined if your system has _Bool of its own */
#define HAVE__BOOL 1

#endif  /* _CONFIG_UNIX_H */

