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

void init_intro()
{
    SDL_Surface *temp;

    dragontech=LoadT8(DATADIR"/Tgfx/dragontech.T8");
    line1=LoadT8(DATADIR"/Tgfx/line1.T8");
    line2=LoadT8(DATADIR"/Tgfx/line2.T8");
    SDL_SetColorKey(dragontech, SDL_SRCCOLORKEY,SDL_MapRGB(dragontech->format,0,255,0));
    SDL_SetColorKey(line1, SDL_SRCCOLORKEY,SDL_MapRGB(line1->format,0,0,0));
    SDL_SetColorKey(line2, SDL_SRCCOLORKEY,SDL_MapRGB(line2->format,0,0,0));

    comic_01=LoadT8(DATADIR"/Tgfx/comic_01.T8");

    temp=SDL_CreateRGBSurface(SDL_SWSURFACE, screen_w, screen_h, screen_bpp, 255, 255, 255, 0);
    SDL_FillRect(temp, NULL, SDL_MapRGB(temp->format,0,0,0));
    black = SDL_DisplayFormat(temp);
    
    SDL_FreeSurface(temp);
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format,0,0,0));
}

void end_intro()
{
    SDL_FreeSurface(dragontech);
    SDL_FreeSurface(line1);
    SDL_FreeSurface(line2);
    SDL_FreeSurface(comic_01);
    SDL_FreeSurface(black);
}

void intro()
{
    init_intro();
    double introtime=0;
    escape_exit=0;
    float title_alpha=0;
    int title_done=0;
    int n=1;
    while(escape_exit==0)
    {
	SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format,0,0,0));
	//if(n<=4) // 4 son los pasos a realizar en la primer parte de la intro
	{
	    {
		switch(n)
		{
		    default:
				if(n==3)
			        title_alpha=intro_blit(line2, 160, 350, 600, 40, title_alpha, 80, 1);
			    else
			    	intro_blit(line2, 160, 350, 600, 40, 255, 80, 1);
		    case 2:
				if(n==2)
		        	title_alpha=intro_blit(line1, 60, 300, 600, 40, title_alpha, 80, 1);
		        else
		        	intro_blit(line1, 60, 300, 600, 40, 255, 80, 1);
		    case 1:
		    	if(n==1)
			        title_alpha=intro_blit(dragontech, 160, 100, 480, 170, title_alpha, 80, 1);
			    else
			    	intro_blit(dragontech, 160, 100, 480, 170, 255, 80, 1);
		}
	    }
	    if(title_alpha>=255) 
	    {
		title_alpha=0;
		n++;
	    }
	}
	SDL_Flip(screen);
	credits_events();
	delta_time();
	introtime+=imove;
	SDL_Delay(1);
	if(introtime>15)
	    escape_exit=1;
    }

    escape_exit=0; n=1; title_alpha=0; introtime=0;
    while(escape_exit==0)
    {
	SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format,0,0,0));
	{
		if(n == 1)
	    	title_alpha=intro_blit(comic_01, 0, 0, 800, 600, title_alpha, 60, 0);
	    else
	    	intro_blit(comic_01, 0, 0, 800, 600, 255, 60, 0);
	}
	if(title_alpha>=255) 
	{
	    title_alpha=0;
	    n++;
	}
	SDL_Flip(screen);
	credits_events();
	delta_time();
	introtime+=imove;
	SDL_Delay(1);
	if(introtime>15)
	    escape_exit=1;
    }

    end_intro();
}
