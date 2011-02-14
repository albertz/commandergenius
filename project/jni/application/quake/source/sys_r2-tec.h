/*
	Quake(SDL) v1.10 by Rikku2000
	Date: 2010
*/

// SDL
#include <SDL.h>
#include <SDL_ttf.h>

typedef unsigned char u8;

void TXT_Printf (TTF_Font *ttf, SDL_Surface *surface, int x, int y, SDL_Color color, char *format, ...) {
	char buf[1024];
	SDL_Rect rect;
	SDL_Surface *pSurfaceText;

	va_list arg;

	va_start(arg, format);
	vsprintf(buf, format, arg);
	va_end(arg);

	pSurfaceText = TTF_RenderUTF8_Blended (ttf, buf, color);
	
	if (pSurfaceText) {
		rect.x = x; rect.y = y;
		SDL_BlitSurface(pSurfaceText, NULL, surface, &rect);
		SDL_FreeSurface(pSurfaceText);
	}
}
