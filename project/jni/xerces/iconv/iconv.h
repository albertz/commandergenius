// SDL_iconv >> iconv wrapper dummy
#ifndef DUMMY_ICONV_H
#define DUMMY_ICONV_H

#undef HAVE_ICONV
#ifdef __cplusplus
extern "C" {
#endif

// #include <SDL.h> // We'll have to rebuild xerces if we'll include this file and change SDL version, so just provide declarations here

typedef struct _SDL_iconv_t *SDL_iconv_t;

extern SDL_iconv_t SDL_iconv_open(const char *tocode, const char *fromcode);
extern int SDL_iconv_close(SDL_iconv_t cd);
extern size_t SDL_iconv(SDL_iconv_t cd, const char **inbuf, size_t *inbytesleft, char **outbuf, size_t *outbytesleft);
extern char * SDL_iconv_string(const char *tocode, const char *fromcode, const char *inbuf, size_t inbytesleft);

#define iconv_t SDL_iconv_t
#define iconv SDL_iconv
#define iconv_open SDL_iconv_open
#define iconv_close SDL_iconv_close

#ifdef __cplusplus
}
#endif

#define HAVE_ICONV 1

#endif
