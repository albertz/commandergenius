/***************************************************************************
*                          Pachi el marciano                               *
*                          -----------------                               *
*                     (c) Santiago Radeff (coding)                         *
*                     (c) Nicolas Radeff  (graphics)                       *
*                     (c) Peter Hajba     (music)                          *
*                                                                          *
*                          T-1000@Bigfoot.com                              *
****************************************************************************
    *******************************************************************
    *                                                                 *
    *   This program is free software; you can redistribute it and/or *
    *   modify it under the terms of the GNU General Public License   *
    *   as published by the Free Software Foundation; either version  *
    *   2 of the License, or (at your option) any later version.      *
    *                                                                 *
    *******************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>
#include<string.h>
#include<SDL/SDL.h>
#include<SDL/SDL_mixer.h>

#include"definitions.h"
#include"structs.h"
#include"surfaces.h"
#include"sounds.h"
#include"config.h"
#include"proto.h"
#include"credits.h"

#include"text.h"
#include"init.h"
#include"gfx.h"
#include"input.h"
#include"file.h"
#include"output.h"
#include"stages.h"
#include"menu.h"
#include"intro.h"
#include"gameover.h"
#include"player.h"
#include"monsters.h"
#include"game.h"
#include"fx.h"
#include"setgame.h"

void flushevents()
{
    SDL_Event event;
    while(SDL_PollEvent(&event)==1);
}

int main(int argc, char *argv[])
{
    initsdl();

    if(argc > 1)
	if(strcmp(argv[1], "--fullscreen")==0 || strcmp(argv[1], "-f")==0)
            screen_mode();

    gameexit=0;
    startgame=0;
    intro();
    init_stages(); // carga los valores de los niveles del juego
    load_roommap(); // carga el mapeado de habitaciones
    while(gameexit==0)
    {
	flushevents();
	menu();
	if(startgame==1)
	{
	    startgame=0;
	    start_game();
	}
	if(help==1)
	{
	    help=0;
	    do_help();
	}
	if(hiscores==1)
	{
	    hiscores=0;
	    do_hiscores();
	}
    }
//    outro();
    SDL_JoystickClose(joystick);
    SDL_Quit();
    fprintf(stderr,"\nThanks for playing.\n");
    return 0;
}
