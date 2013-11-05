#ifndef _UNICODE_STUFF_H_
#define _UNICODE_STUFF_H_

#include "SDL_androidinput.h"

// I'm too lazy to move code into .c file

static inline int UnicodeToUtf8(int src, char * dest)
{
    int len = 0;
    if ( src <= 0x007f) {
        *dest++ = (char)src;
        len = 1;
    } else if (src <= 0x07ff) {
        *dest++ = (char)0xc0 | (src >> 6);
        *dest++ = (char)0x80 | (src & 0x003f);
        len = 2;
    } else if (src == 0xFEFF) {
        // nop -- zap the BOM
    } else if (src >= 0xD800 && src <= 0xDFFF) {
        // surrogates not supported
    } else if (src <= 0xffff) {
        *dest++ = (char)0xe0 | (src >> 12);
        *dest++ = (char)0x80 | ((src >> 6) & 0x003f);
        *dest++ = (char)0x80 | (src & 0x003f);
        len = 3;
    } else if (src <= 0xffff) {
        *dest++ = (char)0xf0 | (src >> 18);
        *dest++ = (char)0x80 | ((src >> 12) & 0x3f);
        *dest++ = (char)0x80 | ((src >> 6) & 0x3f);
        *dest++ = (char)0x80 | (src & 0x3f);
        len = 4;
    } else {
        // out of Unicode range
    }
    *dest = 0;
    return len;
}

static inline SDL_keysym asciiToKeysym(int ascii, int unicode)
{
	SDL_keysym keysym;
	keysym.scancode = ascii;
	if ( ascii < SDLK_LAST )
		keysym.scancode = SDL_android_keysym_to_scancode[ascii];
	keysym.sym = ascii;
	keysym.mod = KMOD_NONE;
	keysym.unicode = 0;
#if SDL_VERSION_ATLEAST(1,3,0)
	keysym.scancode = SDL_GetScancodeFromKey(ascii);
#else
	if ( SDL_TranslateUNICODE )
#endif
		keysym.unicode = unicode;
	return keysym;
}

static inline int checkShiftRequired( int * sym )
{
	switch( *sym )
	{
		case '!': *sym = '1'; return 1;
		case '@': *sym = '2'; return 1;
		case '#': *sym = '3'; return 1;
		case '$': *sym = '4'; return 1;
		case '%': *sym = '5'; return 1;
		case '^': *sym = '6'; return 1;
		case '&': *sym = '7'; return 1;
		case '*': *sym = '8'; return 1;
		case '(': *sym = '9'; return 1;
		case ')': *sym = '0'; return 1;
		case '_': *sym = '-'; return 1;
		case '+': *sym = '='; return 1;
		case '|': *sym = '\\';return 1;
		case '<': *sym = ','; return 1;
		case '>': *sym = '.'; return 1;
		case '?': *sym = '/'; return 1;
		case ':': *sym = ';'; return 1;
		case '"': *sym = '\'';return 1;
		case '{': *sym = '['; return 1;
		case '}': *sym = ']'; return 1;
		case '~': *sym = '`'; return 1;
		default: if( *sym >= 'A' && *sym <= 'Z' ) { *sym += 'a' - 'A'; return 1; };
	}
	return 0;
}

#endif
