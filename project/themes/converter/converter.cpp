#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL.h>
#include <SDL_image.h>
#include <vector>
#include <algorithm>
#include <string>
#include <netinet/in.h>


int
main(int argc, char *argv[])
{
	if(argc < 3)
		return 1;
	SDL_Surface * src = IMG_Load(argv[1]);
	if(!src)
		return 1;
	SDL_Surface * format1 = SDL_CreateRGBSurface( SDL_SWSURFACE|SDL_SRCALPHA, 1, 1, 16, 0xF800, 0x7C0, 0x3E, 0x1 );
	if( argc >= 4 )
		format1 = SDL_CreateRGBSurface( SDL_SWSURFACE|SDL_SRCALPHA, 1, 1, 16, 0xF000, 0xF00, 0xF0, 0xF );
	if(!format1)
		return 1;
	SDL_Surface * dst = SDL_ConvertSurface(src, format1->format, SDL_SWSURFACE|SDL_SRCALPHA);
	if(!dst)
		return 1;
	FILE * ff = fopen(argv[2], "wb");
	if(!ff)
		return 1;
	int w = htonl(dst->w);
	fwrite( &w, 1, 4, ff );
	int h = htonl(dst->h);
	fwrite( &h, 1, 4, ff );
	int format = htonl(argc <= 3 ? 0 : 1);
	fwrite( &format, 1, 4, ff );
	for( int i = 0; i < dst->h; i++ )
	{
		for( int ii = 0; ii < dst->w; ii++ )
		{
			if(* (Uint16 *) ((Uint8 *)dst->pixels + i*dst->pitch + ii*2) & 0x1 == 0 && argc <= 3)
				* (Uint16 *) ((Uint8 *)dst->pixels + i*dst->pitch + ii*2) = 0;
			fwrite( (Uint8 *)dst->pixels + i*dst->pitch + ii*2, 1, 2, ff );
		}
	}
	fclose(ff);
	return 0;
}

#ifdef WIN32
#include <windows.h>
int CALLBACK WinMain(
  HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPSTR lpCmdLine,
  int nCmdShow
)
{
	return main(0, NULL);
};

#endif
