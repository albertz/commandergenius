#ifndef _RFB_RFBCONFIG_H
#define _RFB_RFBCONFIG_H 1
 
/* rfb/rfbconfig.h. Generated automatically at end of configure. */
/* rfbconfig.h.  Generated from rfbconfig.h.in by configure.  */
/* rfbconfig.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* Enable 24 bit per pixel in native framebuffer */
#ifndef LIBVNCSERVER_ALLOW24BPP
#define LIBVNCSERVER_ALLOW24BPP 1
#endif

/* Build libva support */
/* #undef CONFIG_LIBVA */

/* work around when write() returns ENOENT but does not mean it */
/* #undef ENOENT_WORKAROUND */

/* Use ffmpeg (for vnc2mpg) */
/* #undef FFMPEG */

/* Android host system detected */
#ifndef LIBVNCSERVER_HAVE_ANDROID
#define LIBVNCSERVER_HAVE_ANDROID 1
#endif

/* Define to 1 if you have the <arpa/inet.h> header file. */
#ifndef LIBVNCSERVER_HAVE_ARPA_INET_H
#define LIBVNCSERVER_HAVE_ARPA_INET_H 1
#endif

/* Define to 1 if you have the `crypt' function. */
/* #undef HAVE_CRYPT */

/* Define to 1 if you have the <dlfcn.h> header file. */
#ifndef LIBVNCSERVER_HAVE_DLFCN_H
#define LIBVNCSERVER_HAVE_DLFCN_H 1
#endif

/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
/* #undef HAVE_DOPRNT */

/* Define to 1 if you have the <endian.h> header file. */
#ifndef LIBVNCSERVER_HAVE_ENDIAN_H
#define LIBVNCSERVER_HAVE_ENDIAN_H 1
#endif

/* Define to 1 if you have the <fcntl.h> header file. */
#ifndef LIBVNCSERVER_HAVE_FCNTL_H
#define LIBVNCSERVER_HAVE_FCNTL_H 1
#endif

/* Define to 1 if you have the `fork' function. */
/* #undef HAVE_FORK */

/* Define to 1 if you have the `ftime' function. */
/* #undef HAVE_FTIME */

/* Define to 1 if you have the `gethostbyname' function. */
/* #undef HAVE_GETHOSTBYNAME */

/* Define to 1 if you have the `gethostname' function. */
/* #undef HAVE_GETHOSTNAME */

/* Define to 1 if you have the `gettimeofday' function. */
/* #undef HAVE_GETTIMEOFDAY */

/* GnuTLS library present */
#ifndef LIBVNCSERVER_HAVE_GNUTLS
#define LIBVNCSERVER_HAVE_GNUTLS 1
#endif

/* Define to 1 if you have the `inet_ntoa' function. */
/* #undef HAVE_INET_NTOA */

/* Define to 1 if you have the <inttypes.h> header file. */
#ifndef LIBVNCSERVER_HAVE_INTTYPES_H
#define LIBVNCSERVER_HAVE_INTTYPES_H 1
#endif

/* libcrypt library present */
/* #undef HAVE_LIBCRYPT */

/* openssl libcrypto library present */
#ifndef LIBVNCSERVER_HAVE_LIBCRYPTO
#define LIBVNCSERVER_HAVE_LIBCRYPTO 1
#endif

/* Define to 1 if you have the `cygipc' library (-lcygipc). */
/* #undef HAVE_LIBCYGIPC */

/* libjpeg support enabled */
#ifndef LIBVNCSERVER_HAVE_LIBJPEG
#define LIBVNCSERVER_HAVE_LIBJPEG 1
#endif

/* Define to 1 if you have the `nsl' library (-lnsl). */
/* #undef HAVE_LIBNSL */

/* Define to 1 if you have the `png' library (-lpng). */
#ifndef LIBVNCSERVER_HAVE_LIBPNG
#define LIBVNCSERVER_HAVE_LIBPNG 1
#endif

/* Define to 1 if you have the `pthread' library (-lpthread). */
#ifndef LIBVNCSERVER_HAVE_LIBPTHREAD
#define LIBVNCSERVER_HAVE_LIBPTHREAD 1
#endif

/* Define to 1 if you have the `socket' library (-lsocket). */
/* #undef HAVE_LIBSOCKET */

/* openssl libssl library present */
#ifndef LIBVNCSERVER_HAVE_LIBSSL
#define LIBVNCSERVER_HAVE_LIBSSL 1
#endif

/* Define to 1 if you have the `z' library (-lz). */
#ifndef LIBVNCSERVER_HAVE_LIBZ
#define LIBVNCSERVER_HAVE_LIBZ 1
#endif

/* Define to 1 if you have the `memmove' function. */
/* #undef HAVE_MEMMOVE */

/* Define to 1 if you have the <memory.h> header file. */
#ifndef LIBVNCSERVER_HAVE_MEMORY_H
#define LIBVNCSERVER_HAVE_MEMORY_H 1
#endif

/* Define to 1 if you have the `memset' function. */
/* #undef HAVE_MEMSET */

/* Define to 1 if you have the `mkfifo' function. */
/* #undef HAVE_MKFIFO */

/* Define to 1 if you have the `mmap' function. */
/* #undef HAVE_MMAP */

/* Define to 1 if you have the <netdb.h> header file. */
#ifndef LIBVNCSERVER_HAVE_NETDB_H
#define LIBVNCSERVER_HAVE_NETDB_H 1
#endif

/* Define to 1 if you have the <netinet/in.h> header file. */
#ifndef LIBVNCSERVER_HAVE_NETINET_IN_H
#define LIBVNCSERVER_HAVE_NETINET_IN_H 1
#endif

/* Define to 1 if you have the `select' function. */
/* #undef HAVE_SELECT */

/* Define to 1 if you have the `socket' function. */
/* #undef HAVE_SOCKET */

/* Define to 1 if `stat' has the bug that it succeeds when given the
   zero-length file name argument. */
#ifndef LIBVNCSERVER_HAVE_STAT_EMPTY_STRING_BUG
#define LIBVNCSERVER_HAVE_STAT_EMPTY_STRING_BUG 1
#endif

/* Define to 1 if you have the <stdint.h> header file. */
#ifndef LIBVNCSERVER_HAVE_STDINT_H
#define LIBVNCSERVER_HAVE_STDINT_H 1
#endif

/* Define to 1 if you have the <stdlib.h> header file. */
#ifndef LIBVNCSERVER_HAVE_STDLIB_H
#define LIBVNCSERVER_HAVE_STDLIB_H 1
#endif

/* Define to 1 if you have the `strchr' function. */
/* #undef HAVE_STRCHR */

/* Define to 1 if you have the `strcspn' function. */
/* #undef HAVE_STRCSPN */

/* Define to 1 if you have the `strdup' function. */
/* #undef HAVE_STRDUP */

/* Define to 1 if you have the `strerror' function. */
/* #undef HAVE_STRERROR */

/* Define to 1 if you have the `strftime' function. */
/* #undef HAVE_STRFTIME */

/* Define to 1 if you have the <strings.h> header file. */
#ifndef LIBVNCSERVER_HAVE_STRINGS_H
#define LIBVNCSERVER_HAVE_STRINGS_H 1
#endif

/* Define to 1 if you have the <string.h> header file. */
#ifndef LIBVNCSERVER_HAVE_STRING_H
#define LIBVNCSERVER_HAVE_STRING_H 1
#endif

/* Define to 1 if you have the `strstr' function. */
/* #undef HAVE_STRSTR */

/* Define to 1 if you have the <syslog.h> header file. */
#ifndef LIBVNCSERVER_HAVE_SYSLOG_H
#define LIBVNCSERVER_HAVE_SYSLOG_H 1
#endif

/* Define to 1 if you have the <sys/endian.h> header file. */
#ifndef LIBVNCSERVER_HAVE_SYS_ENDIAN_H
#define LIBVNCSERVER_HAVE_SYS_ENDIAN_H 1
#endif

/* Define to 1 if you have the <sys/socket.h> header file. */
#ifndef LIBVNCSERVER_HAVE_SYS_SOCKET_H
#define LIBVNCSERVER_HAVE_SYS_SOCKET_H 1
#endif

/* Define to 1 if you have the <sys/stat.h> header file. */
#ifndef LIBVNCSERVER_HAVE_SYS_STAT_H
#define LIBVNCSERVER_HAVE_SYS_STAT_H 1
#endif

/* Define to 1 if you have the <sys/timeb.h> header file. */
#ifndef LIBVNCSERVER_HAVE_SYS_TIMEB_H
#define LIBVNCSERVER_HAVE_SYS_TIMEB_H 1
#endif

/* Define to 1 if you have the <sys/time.h> header file. */
#ifndef LIBVNCSERVER_HAVE_SYS_TIME_H
#define LIBVNCSERVER_HAVE_SYS_TIME_H 1
#endif

/* Define to 1 if you have the <sys/types.h> header file. */
#ifndef LIBVNCSERVER_HAVE_SYS_TYPES_H
#define LIBVNCSERVER_HAVE_SYS_TYPES_H 1
#endif

/* Define to 1 if you have <sys/wait.h> that is POSIX.1 compatible. */
#ifndef LIBVNCSERVER_HAVE_SYS_WAIT_H
#define LIBVNCSERVER_HAVE_SYS_WAIT_H 1
#endif

/* Define to 1 if compiler supports __thread */
#ifndef LIBVNCSERVER_HAVE_TLS
#define LIBVNCSERVER_HAVE_TLS 1
#endif

/* Define to 1 if you have the <unistd.h> header file. */
#ifndef LIBVNCSERVER_HAVE_UNISTD_H
#define LIBVNCSERVER_HAVE_UNISTD_H 1
#endif

/* Define to 1 if you have the `vfork' function. */
/* #undef HAVE_VFORK */

/* Define to 1 if you have the <vfork.h> header file. */
/* #undef HAVE_VFORK_H */

/* Define to 1 if you have the `vprintf' function. */
/* #undef HAVE_VPRINTF */

/* Define to 1 if `fork' works. */
/* #undef HAVE_WORKING_FORK */

/* Define to 1 if `vfork' works. */
/* #undef HAVE_WORKING_VFORK */

/* Define to 1 if you have the <ws2tcpip.h> header file. */
/* #undef HAVE_WS2TCPIP_H */

/* open ssl X509_print_ex_fp available */
/* #undef HAVE_X509_PRINT_EX_FP */

/* Enable IPv6 support */
/* #undef IPv6 */

/* Define to 1 if `lstat' dereferences a symlink specified with a trailing
   slash. */
/* #undef LSTAT_FOLLOWS_SLASHED_SYMLINK */

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#ifndef LIBVNCSERVER_LT_OBJDIR
#define LIBVNCSERVER_LT_OBJDIR ".libs/"
#endif

/* Need a typedef for in_addr_t */
#ifndef LIBVNCSERVER_NEED_INADDR_T
#define LIBVNCSERVER_NEED_INADDR_T 1
#endif

/* Name of package */
#ifndef LIBVNCSERVER_PACKAGE
#define LIBVNCSERVER_PACKAGE "libvncserver"
#endif

/* Define to the address where bug reports for this package should be sent. */
#ifndef LIBVNCSERVER_PACKAGE_BUGREPORT
#define LIBVNCSERVER_PACKAGE_BUGREPORT "https://github.com/LibVNC/libvncserver"
#endif

/* Define to the full name of this package. */
#ifndef LIBVNCSERVER_PACKAGE_NAME
#define LIBVNCSERVER_PACKAGE_NAME "LibVNCServer"
#endif

/* Define to the full name and version of this package. */
#ifndef LIBVNCSERVER_PACKAGE_STRING
#define LIBVNCSERVER_PACKAGE_STRING "LibVNCServer 0.9.10"
#endif

/* Define to the one symbol short name of this package. */
#ifndef LIBVNCSERVER_PACKAGE_TARNAME
#define LIBVNCSERVER_PACKAGE_TARNAME "libvncserver"
#endif

/* Define to the home page for this package. */
#ifndef LIBVNCSERVER_PACKAGE_URL
#define LIBVNCSERVER_PACKAGE_URL ""
#endif

/* Define to the version of this package. */
#ifndef LIBVNCSERVER_PACKAGE_VERSION
#define LIBVNCSERVER_PACKAGE_VERSION "0.9.10"
#endif

/* The size of `char', as computed by sizeof. */
/* #undef SIZEOF_CHAR */

/* The size of `int', as computed by sizeof. */
/* #undef SIZEOF_INT */

/* The size of `long', as computed by sizeof. */
/* #undef SIZEOF_LONG */

/* The size of `short', as computed by sizeof. */
/* #undef SIZEOF_SHORT */

/* The size of `void*', as computed by sizeof. */
/* #undef SIZEOF_VOIDP */

/* Define to 1 if you have the ANSI C header files. */
#ifndef LIBVNCSERVER_STDC_HEADERS
#define LIBVNCSERVER_STDC_HEADERS 1
#endif

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#ifndef LIBVNCSERVER_TIME_WITH_SYS_TIME
#define LIBVNCSERVER_TIME_WITH_SYS_TIME 1
#endif

/* Version number of package */
#ifndef LIBVNCSERVER_VERSION
#define LIBVNCSERVER_VERSION "0.9.10"
#endif

/* LibVNCServer major version */
#ifndef LIBVNCSERVER_VERSION_MAJOR
#define LIBVNCSERVER_VERSION_MAJOR 0
#endif

/* LibVNCServer minor version */
#ifndef LIBVNCSERVER_VERSION_MINOR
#define LIBVNCSERVER_VERSION_MINOR 9
#endif

/* LibVNCServer patchlevel */
#ifndef LIBVNCSERVER_VERSION_PATCHLEVEL
#define LIBVNCSERVER_VERSION_PATCHLEVEL 10
#endif

/* Enable support for libgcrypt in libvncclient */
/* #undef WITH_CLIENT_GCRYPT */

/* Disable TightVNCFileTransfer protocol */
#ifndef LIBVNCSERVER_WITH_TIGHTVNC_FILETRANSFER
#define LIBVNCSERVER_WITH_TIGHTVNC_FILETRANSFER 1
#endif

/* Disable WebSockets support */
#ifndef LIBVNCSERVER_WITH_WEBSOCKETS
#define LIBVNCSERVER_WITH_WEBSOCKETS 1
#endif

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to `int' if <sys/types.h> does not define. */
/* #undef pid_t */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* Substitute for socklen_t */
/* #undef socklen_t */

/* Define as `fork' if `vfork' does not work. */
#ifndef _libvncserver_vfork
#define _libvncserver_vfork fork
#endif

#define LIBVNCSERVER_HAVE_GETTIMEOFDAY 1

#include <zlib.h>

/* once: _RFB_RFBCONFIG_H */
#endif
