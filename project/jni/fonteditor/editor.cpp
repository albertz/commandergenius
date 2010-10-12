#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL.h>
#include <vector>
#include <algorithm>
#include <string>

const char * fname = "touchscreenfont.h";
struct font_line_t { Uint8 x1, y1, x2, y2; };
std::vector< std::vector<font_line_t> > font;

enum { DRAWAREA = 512, BORDERAREA = 10 };
int currentChar = 0;
int drawX, drawY, drawStarted = 0;

void load()
{
	FILE * ff = fopen(fname, "r");
	if(!ff)
		return;
	std::string s;
	char buf[32];
	int readed = 0;
	while( ( readed = fread(buf, 1, sizeof(buf), ff) ) > 0 )
	{
		s.append(buf, readed);
	}
	fclose(ff);
	
	font.clear();
	
	std::string::size_type f;
	f = s.find("FONT_MAX_LINES_PER_CHAR =");
	f = s.find("=", f) + 1;
	int maxLines = atoi(s.c_str() + f);
	
	f = s.find("font_line_t font[", f);
	f = s.find("[", f) + 1;
	int numChars = atoi(s.c_str() + f);
	for(int i = 0; i < numChars; i++)
	{
		font.push_back( std::vector<font_line_t>() );
		for( int ii = 0; ii < maxLines; ii++ )
		{
			font_line_t line;
			f = s.find("\t\t{ ", f) + 4;
			line.x1 = atoi(s.c_str() + f);
			f = s.find(",", f) + 1;
			line.y1 = atoi(s.c_str() + f);
			f = s.find(",", f) + 1;
			line.x2 = atoi(s.c_str() + f);
			f = s.find(",", f) + 1;
			line.y2 = atoi(s.c_str() + f);
			if( line.x1 > 0 && line.y1 > 0 && line.x2 > 0 && line.y2 > 0 )
				font.back().push_back(line);
		};
	};
	if(font.size() == 0)
		font.resize(1);
}

void save()
{
	FILE * ff = fopen(fname, "w");
	fprintf(ff, "/* This file should be edited using Touchscreen Editor utility */\n");
	fprintf(ff, "#ifndef _TOUCHSCREENFONT_H_\n#define _TOUCHSCREENFONT_H_\n");
	fprintf(ff, "#include <SDL_types.h>\n");
	fprintf(ff, "typedef struct font_line_t { Uint8 x1, y1, x2, y2; } font_line_t;\n");
	int maxLines = 0;
	for( size_t i = 0; i < font.size(); i++ )
		if( maxLines < font[i].size() )
			maxLines = font[i].size();
	fprintf(ff, "enum { FONT_MAX_LINES_PER_CHAR = %i };\n", maxLines);
	fprintf(ff, "static font_line_t font[%d][FONT_MAX_LINES_PER_CHAR] = {\n", (int)font.size());
	for( size_t i = 0; i < font.size(); i++ )
	{
		fprintf(ff, "\t{\n");
		for( size_t ii = 0; ii < maxLines; ii++ )
		{
			fprintf(ff, "\t\t{ ");
			if( ii < font[i].size() )
				fprintf(ff, "%3d, %3d, %3d, %3d",
				(int)font[i][ii].x1,
				(int)font[i][ii].y1,
				(int)font[i][ii].x2,
				(int)font[i][ii].y2 );
			else
				fprintf(ff, "   0,   0,   0,   0");
			fprintf(ff, " }%s\n", ii + 1 < maxLines ? "," : "" );
		}
		fprintf(ff, "\t}%s\n", i + 1 < font.size() ? "," : "" );
	}
	fprintf(ff, "};\n");
	fprintf(ff, "#endif\n");
	fclose(ff);
}

void DrawLine(SDL_Surface * bmp, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color)
{
	int dx = x2-x1;
	int dy = y2-y1;
	int i1, i2;
	int x, y;
	int dd;
	
	Uint16 d = color;
	
	#define proc( bmp, x, y, d )  * (Uint16 *)( ((Uint8 *)bmp->pixels) + bmp->pitch * (y) + (x) * 2 ) = d
	
	/* worker macro */
	#define DO_LINE(pri_sign, pri_c, pri_cond, sec_sign, sec_c, sec_cond)     \
	{                                                                         \
	if (d##pri_c == 0) {                                                   \
	proc(bmp, x1, y1, d);                                               \
	return;                                                             \
	}                                                                      \
	\
	i1 = 2 * d##sec_c;                                                     \
	dd = i1 - (sec_sign (pri_sign d##pri_c));                              \
	i2 = dd - (sec_sign (pri_sign d##pri_c));                              \
	\
	x = x1;                                                                \
	y = y1;                                                                \
	\
	while (pri_c pri_cond pri_c##2) {                                      \
	proc(bmp, x, y, d);                                                 \
	\
	if (dd sec_cond 0) {                                                \
	sec_c sec_sign##= 1;                                             \
	dd += i2;                                                        \
	}                                                                   \
	else                                                                \
	dd += i1;                                                        \
	\
	pri_c pri_sign##= 1;                                                \
	}                                                                      \
	}
		
	if (dx >= 0) {
		if (dy >= 0) {
			if (dx >= dy) {
				/* (x1 <= x2) && (y1 <= y2) && (dx >= dy) */
				DO_LINE(+, x, <=, +, y, >=);
			}
			else {
				/* (x1 <= x2) && (y1 <= y2) && (dx < dy) */
				DO_LINE(+, y, <=, +, x, >=);
			}
		}
		else {
			if (dx >= -dy) {
				/* (x1 <= x2) && (y1 > y2) && (dx >= dy) */
				DO_LINE(+, x, <=, -, y, <=);
			}
			else {
				/* (x1 <= x2) && (y1 > y2) && (dx < dy) */
				DO_LINE(-, y, >=, +, x, >=);
			}
		}
	}
	else {
		if (dy >= 0) {
			if (-dx >= dy) {
				/* (x1 > x2) && (y1 <= y2) && (dx >= dy) */
				DO_LINE(-, x, >=, +, y, >=);
			}
			else {
				/* (x1 > x2) && (y1 <= y2) && (dx < dy) */
				DO_LINE(+, y, <=, -, x, <=);
			}
		}
		else {
			if (-dx >= -dy) {
				/* (x1 > x2) && (y1 > y2) && (dx >= dy) */
				DO_LINE(-, x, >=, -, y, <=);
			}
			else {
				/* (x1 > x2) && (y1 > y2) && (dx < dy) */
				DO_LINE(-, y, >=, -, x, <=);
			}
		}
	}
	
	#undef DO_LINE
	
};

void draw()
{
	SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);
	SDL_LockSurface(SDL_GetVideoSurface());
	
	if( font.size() > currentChar )
	{
		for( int i = 0; i < font[currentChar].size(); i++ )
		{
			DrawLine( SDL_GetVideoSurface(), font[currentChar][i].x1 * 2, font[currentChar][i].y1 * 2,
					  font[currentChar][i].x2 * 2, font[currentChar][i].y2 * 2, 0xffff );
		}
	}
	
	
	if( drawStarted )
	{
		int x, y;
		SDL_GetMouseState(&x, &y);
		if( x/2 < DRAWAREA && y/2 < DRAWAREA )
			DrawLine( SDL_GetVideoSurface(), (x/2)*2, (y/2)*2, drawX*2, drawY*2, 0x9999 );
	}
	
	DrawLine( SDL_GetVideoSurface(), DRAWAREA, 0, DRAWAREA, DRAWAREA, 0x4444 );
	for( int i = 0; i < font.size(); i++ )
	{
		DrawLine( SDL_GetVideoSurface(), DRAWAREA, i * 4, 
					DRAWAREA + ( font[i].size() < BORDERAREA - 1 ? font[i].size() : BORDERAREA - 1 ), i * 4, 0x7777 );
	}
	DrawLine( SDL_GetVideoSurface(), DRAWAREA, currentChar * 4, DRAWAREA + BORDERAREA - 1, currentChar * 4, 0xAAAA );
	DrawLine( SDL_GetVideoSurface(), DRAWAREA, currentChar * 4 + 1, DRAWAREA + BORDERAREA - 1, currentChar * 4 + 1, 0xAAAA );
	DrawLine( SDL_GetVideoSurface(), DRAWAREA, font.size() * 4, DRAWAREA + BORDERAREA - 1, font.size() * 4, 0x5555 );

	SDL_UnlockSurface(SDL_GetVideoSurface());
	SDL_Flip(SDL_GetVideoSurface());
}

int
main(int argc, char *argv[])
{
  int width  = DRAWAREA + BORDERAREA;
  int height = DRAWAREA;

  SDL_Surface *screen;
  int done;
  Uint8 *keys;

  SDL_Init(SDL_INIT_VIDEO);

  screen = SDL_SetVideoMode(width, height, 16, SDL_DOUBLEBUF);
  if ( ! screen ) {
    fprintf(stderr, "Couldn't set video mode: %s\n", SDL_GetError());
    SDL_Quit();
    exit(2);
  }
  SDL_WM_SetCaption("Touchscreen keyboard font editor", "Touchscreen keyboard font editor");

	if(font.size() == 0)
		font.resize(1);
  
  load();

  done = 0;
  while ( ! done ) {
    SDL_Event event;
    while ( SDL_PollEvent(&event) ) {
      switch(event.type) {

        case SDL_QUIT:
          done = 1;
          break;

			case SDL_MOUSEBUTTONDOWN:
				if( event.button.x < DRAWAREA )
				{
					if (event.button.button == SDL_BUTTON_LEFT)
					{
						if( !drawStarted )
						{
							drawStarted = 1;
							drawX = event.button.x/2;
							drawY = event.button.y/2;
						}
						else
						{
							drawStarted = 0;
							font_line_t line;
							line.x1 = drawX;
							line.y1 = drawY;
							line.x2 = event.button.x/2;
							line.y2 = event.button.y/2;
							font[currentChar].push_back(line);
						}
					}
					else
					{
						if(font[currentChar].size() > 0)
							font[currentChar].pop_back();
					}
				}
				else
				{
					if (event.button.button == SDL_BUTTON_LEFT)
					{
						currentChar = event.button.y / 4;
						if(font.size() <= currentChar)
							font.resize(currentChar + 1);
					}
					else
					{
						currentChar = event.button.y / 4;
						font.resize(currentChar + 1);
					}
				}
				break;
      }
    }

#if SDL_VERSION_ATLEAST(1,3,0)
    keys = SDL_GetKeyboardState(NULL);

    if ( keys[SDL_SCANCODE_ESCAPE] ) {
      done = 1;
    }
#else
    keys = SDL_GetKeyState(NULL);

    if ( keys[SDLK_ESCAPE] ) {
      done = 1;
    }
#endif

	draw();

  };

  save();

  SDL_Quit();
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
