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

void screen_mode()
{
    if(fullscreen)
    {
        screen = SDL_SetVideoMode(screen_w,screen_h,screen_bpp, SDL_HWSURFACE|SDL_DOUBLEBUF /* SDL_SWSURFACE */);
        fullscreen = 0;
    }
    else
    {
        screen = SDL_SetVideoMode(screen_w,screen_h,screen_bpp, SDL_SWSURFACE);
        fullscreen = 1;
    }
}



void print_credits()
{
    int creditsfont_h = 25;
    int creditsfont_w = 18;
    if(needinput==1) // la variable needinput indica si se debe imprimir una nueva linea de texto
    {
	char line2print[100]; // esta es la cadena de texto que contiene el renglon a imprimir

	for(int n=0; n<=37; n++)
	    line2print[n]=creditstext[n+(creditslinecounter*36)];
	
	print_text(creditsfont, creditsbuffer, creditsfont_w, creditsfont_h, 0,400, line2print);
	// imprime una linea de texto en la parte de abajo (no imprimible del primer buffer)

	needinput=0;
	creditslinecounter++;
	if(creditslinecounter==creditslines)
	{
	    creditslinecounter--;
	    wrapcounter++;
	    if(wrapcounter==100)
	    {
		creditslinecounter=0;
		wrapcounter=0;
	    }
	}
    }
    else // si no se imprime una nueva linea se debe hacer el scroll entre los buffers
    {
	SDL_Rect cbuffer;
	cbuffer.w = 600;
	cbuffer.h = 440;
	cbuffer.x = 0;
	cbuffer.y = 1;

	SDL_Rect cbuffer1;
	cbuffer1.w = 600;
	cbuffer1.h = 400;
	cbuffer1.x = 0;
	cbuffer1.y = 0;

	SDL_Rect creditspos;
	creditspos.w=600;
	creditspos.h=400;
	creditspos.x=100;
	creditspos.y=100;

	SDL_Rect bgpos;
	bgpos.w=600;
	bgpos.h=420;
	bgpos.x=100;
	bgpos.y=100;
	
    SDL_SetColorKey(creditsbuffer, 0, 0);
    SDL_SetAlpha(creditsbuffer, SDL_SRCALPHA, 255);
	SDL_BlitSurface(creditsbuffer, &cbuffer, creditsbuffer1, &cbuffer1);
	SDL_BlitSurface(creditsbuffer1, &cbuffer1, creditsbuffer, &cbuffer1);
	SDL_BlitSurface(background, &bgpos, screen, &bgpos);
    SDL_SetColorKey(creditsbuffer, SDL_SRCCOLORKEY, SDL_MapRGB(creditsbuffer->format, 0, 255, 0));
    SDL_SetAlpha(creditsbuffer, SDL_SRCALPHA, 128);
	SDL_BlitSurface(creditsbuffer, &cbuffer1, screen, &creditspos);
	//SDL_UpdateRect(screen,100,100,600,400);
	SDL_Flip(screen);
	SDL_Delay(50);
	
	linegone++;
	if(linegone == creditsfont_h)
	{
	    needinput = 1;
	    linegone = 0;
	}
    }
}

void do_help()
{
    load_helpgfx();
    fadesurface(background, 0, 0, 150);
    print_text(scorefont, screen, 16, 16, 72, 68,   "      HOW TO PLAY PACHI EL MARCIANO      ");
    print_text(scorefont1, screen, 16, 16, 72, 120, "KEYS:");
    print_text(scorefont1, screen, 16, 16,100, 140, "RIGHT     : P");
    print_text(scorefont1, screen, 16, 16,100, 160, "LEFT      : O");
    print_text(scorefont1, screen, 16, 16,100, 180, "DUCK      : A");
    print_text(scorefont1, screen, 16, 16,100, 200, "STAND UP  : Q");
    print_text(scorefont1, screen, 16, 16,100, 220, "JUMP      : SPACE");
    print_text(scorefont1, screen, 16, 16,100, 240, "PAUSE     : H OR F1");
    print_text(scorefont1, screen, 16, 16, 72, 270, "YOU CAN ALSO USE THE CURSOR KEYS / KEYPAD");

    print_text(scorefont,  screen, 16, 16, 72, 300, " THE GOAL OF THE GAME IS TO COLLECT ALL  ");
    print_text(scorefont,  screen, 16, 16, 72, 320, " THE OBJECTS OF EACH LEVEL, WHEN THIS IS ");
    print_text(scorefont,  screen, 16, 16, 72, 340, "  DONE, THE EXIT GATE TO THE NEXT LEVEL  ");
    print_text(scorefont,  screen, 16, 16, 72, 360, "WILL APPEAR, BUT BEWARE OF THE ENEMIES...");

    SDL_Flip(screen);
    escape_exit=0;
    while(escape_exit == 0)
    {
        credits_events();
    }
    unload_helpgfx();
}

void print_monitor()
{
    print_text(scorefont1,screen,16,16,30,485,"%s", mstage[stage-1].title);
    print_text(scorefont,screen,16,16,30,510, "SCORE:%d", score);
    print_text(scorefont,screen,16,16,30,528, "LIVES:%d", mplayer[dificulty].lives);
    print_text(scorefont,screen,16,16,30,546, "STAGE:%d", stage);
    print_text(scorefont,screen,16,16,30,564, "ITEMS:%d/%d",mplayer[dificulty].objects,mstage[stage-1].objects);
    //SDL_UpdateRect(screen, 30, 482, 500, 100); 
}

void blinkscreen(int R,int G, int B, int A)
{
    SDL_FillRect(screen,NULL,SDL_MapRGBA(screen->format,R,G,B,A));
    SDL_Flip(screen);
    //setback();
    //SDL_Flip(screen);
}

void print_room()
{
    int x,y;
    SDL_Rect gamearea;
    gamearea.x=R_gamearea_x;
    gamearea.y=R_gamearea_y;
    gamearea.h=448;
    gamearea.w=736;
    SDL_FillRect(screen,&gamearea,0);

    SDL_Rect backs_dst;
    SDL_Rect backs_src;
    SDL_Rect backs_dstbak;
    backs_src.w = R_back_x;
    backs_src.h = R_back_y;
    backs_dst.w = R_back_x;
    backs_dst.h = R_back_y;
    for(x=0;x < R_maxbacks_h;x++)
    {
        for(y=0;y < R_maxbacks_v;y++)
        {
            backs_dst.x = R_gamearea_x + (x*R_back_x);
            backs_dst.y = R_gamearea_y + (y*R_back_y);
	    backs_dstbak.x = x*R_back_x;
	    backs_dstbak.y = y*R_back_y;

            backs_src.y = (int(R_backdata[x][y]/6) * R_back_y);
            backs_src.x = (R_backdata[x][y] - (int(R_backdata[x][y]/6) * 6))*R_back_x;
    	    SDL_BlitSurface(backs,&backs_src,screen,&backs_dst);
        }
    }

    SDL_Rect tiles_dst;
    SDL_Rect tiles_dstbak;
    SDL_Rect tiles_src;
    tiles_src.w = R_tileside;
    tiles_src.h = R_tileside;
    for(x=0;x < R_maxtiles_h;x++)
    {
	for(y=0;y < R_maxtiles_v;y++)
	{
	    tiles_dst.x = R_gamearea_x + (x*R_tileside);
	    tiles_dst.y = R_gamearea_y + (y*R_tileside);
	    tiles_dstbak.x = x*R_tileside;
	    tiles_dstbak.y = y*R_tileside;

            tiles_src.y = (int(mroom.data[x][y]/20) * R_tileside);
            tiles_src.x = (mroom.data[x][y] - (int(mroom.data[x][y]/20) * 20))*R_tileside;
	    if((mroom.data[x][y] != 0) && (mroom.data[x][y] != 239))
		SDL_BlitSurface(tiles,&tiles_src,screen,&tiles_dst);
	    if(mroom.data[x][y] == 239 && stagecompleted == 1)
		SDL_BlitSurface(tiles,&tiles_src,screen,&tiles_dst);
	}
    }
    SDL_BlitSurface(screen,&gamearea,screenbak,NULL);
    print_monitor(); //imprime la informacion del jugador (vidas, puntos, etc)
    //SDL_Flip(screen);
}

void showcode(char *str)
{
    escape_exit=0;
    while(escape_exit==0)
    {
	SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format,0,0,0));
    SDL_Rect code;
	code.x=280; code.y=220;code.w=240;code.h=50;
    SDL_FillRect(screen,&code,SDL_MapRGB(screen->format,255,255,255));
	code.x+=2; code.y+=2;code.w-=4;code.h-=4;
    SDL_FillRect(screen,&code,SDL_MapRGB(screen->format,0,0,0));
	code.x=280; code.y=220;code.w=240;code.h=50;
    print_text(scorefont,screen,16,16,code.x+8,code.y+7,"STAGE CODE IS:");
    print_text(scorefont1,screen,16,16,code.x+90,code.y+27,"%s",str);
    //SDL_UpdateRect(screen, code.x, code.y, code.w, code.h);
    SDL_Flip(screen);
	credits_events();
	SDL_Delay(1);
    }
}

void setback()
{
    SDL_BlitSurface(background,NULL,screen,NULL);
    SDL_Flip(screen);
}

void fadesurface(SDL_Surface *surface, int x, int y, int speed)
{
    SDL_Rect dst;
	dst.x=x; dst.y=y;
    float alpha=0;
    while(alpha<255)
    {
	SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format,0,0,0));
	delta_time();
	SDL_SetAlpha(surface, SDL_SRCALPHA, int(alpha));
	SDL_BlitSurface(surface, NULL, screen, &dst);
	SDL_Flip(screen);
	alpha+=(speed*imove);
    }
	SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format,0,0,0));
    SDL_SetAlpha(surface, 0, 0);
    SDL_BlitSurface(surface, NULL, screen, &dst);
    SDL_Flip(screen);
}

// speed es la velocidad del fade y blackback es un flag que indica si debe poner un fondo de color antes de imprimir (1=negro, 2=blanco)
float intro_blit(SDL_Surface *surface, int x, int y, int w, int h, float blit_alpha, int speed, int back)
{
    SDL_Rect dst;
    //SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format,0,0,0));
	dst.x=x; dst.y=y; dst.w=w; dst.h=h;
    int alpha=int(blit_alpha);
    SDL_SetAlpha(surface, SDL_SRCALPHA, alpha);
    blit_alpha+=(speed*imove);
    SDL_BlitSurface(surface, NULL, screen, &dst);
    //SDL_UpdateRect(screen, dst.x, dst.y, dst.w, dst.h);
    /*
    if(back==1)
    {
	if(blit_alpha<255)
	SDL_FillRect(screen, &dst, SDL_MapRGB(screen->format,0,0,0));
    }
    if(back==2)
    {
	if(blit_alpha<255)
	SDL_FillRect(screen, &dst, SDL_MapRGB(screen->format,255,255,255));
    }
    if(blit_alpha>255)
    {
	SDL_SetAlpha(surface, 0, 0);
	SDL_BlitSurface(surface, NULL, screen, &dst);
	//SDL_Flip(screen);
    }
    */
    return(blit_alpha);
}

/*
float fade(SDL_Surface *surface, int x, int y, int w, int h, int R, int G, int B, int speed, float blit_alpha)
{
    SDL_Rect dst;
	dst.x=x; dst.y=y; dst.w=w; dst.h=h;
    int alpha=int (blit_alpha);
    SDL_SetAlpha(surface, SDL_SRCALPHA, alpha);
    blit_alpha-=(speed*imove);
    SDL_FillRect(screen, &dst, SDL_MapRGB(screen->format,R,G,B));
    SDL_BlitSurface(surface, NULL, screen, &dst);
    SDL_UpdateRect(screen, dst.x, dst.y, dst.w, dst.h);
    return(blit_alpha);
}
*/

void showposter(int num)
{
    if(num==CEMETERY || num==CAVES || num==CASTLE || num==CASTLE+1 || num==LAB)
    {
	escape_exit=0;
	SDL_Surface *temp;
	switch(num)
	{
	    case CEMETERY:
		background=LoadT8(DATADIR"/Tgfx/cemetery.T8");
		fadesurface(background, 0, 0, 200);
		escape_exit=0;
		break;
	    case CAVES:
		background=LoadT8(DATADIR"/Tgfx/comic_02.T8");
		fadesurface(background, 0, 0, 200);
		escape_exit=0;
		while(escape_exit==0) { credits_events(); SDL_Delay(1);	}
		background=LoadT8(DATADIR"/Tgfx/cave.T8");
		fadesurface(background, 0, 0, 200);
		escape_exit=0;
		break;
	    case CASTLE:
		background=LoadT8(DATADIR"/Tgfx/comic_03.T8");
		fadesurface(background, 0, 0, 200);
		escape_exit=0;
		while(escape_exit==0) { credits_events(); SDL_Delay(1);	}
		break;
	    case CASTLE+1:
	    	background=LoadT8(DATADIR"/Tgfx/comic_04.T8");
		fadesurface(background, 0, 0, 200);
		escape_exit=0;
		while(escape_exit==0) { credits_events(); SDL_Delay(1);	}
		background=LoadT8(DATADIR"/Tgfx/castle.T8");
		fadesurface(background, 0, 0, 200);
		escape_exit=0;
		break;
	    case LAB:
	    	background=LoadT8(DATADIR"/Tgfx/comic_05.T8");
		fadesurface(background, 0, 0, 200);
		escape_exit=0;
		while(escape_exit==0) { credits_events(); SDL_Delay(1);	}
		background=LoadT8(DATADIR"/Tgfx/comic_06.T8");
		fadesurface(background, 0, 0, 200);
		escape_exit=0;
		while(escape_exit==0) { credits_events(); SDL_Delay(1);	}
		background=LoadT8(DATADIR"/Tgfx/comic_07.T8");
		fadesurface(background, 0, 0, 200);
		escape_exit=0;
		while(escape_exit==0) { credits_events(); SDL_Delay(1);	}
		background=LoadT8(DATADIR"/Tgfx/comic_08.T8");
		fadesurface(background, 0, 0, 200);
		escape_exit=0;
		while(escape_exit==0) { credits_events(); SDL_Delay(1);	}
		background=LoadT8(DATADIR"/Tgfx/lab.T8");
		fadesurface(background, 0, 0, 200);
		escape_exit=0;
		break;
	    default:
		break;
	}    
	while(escape_exit==0) { credits_events(); SDL_Delay(1);	}
    }
    background=LoadT8(DATADIR"/Tgfx/gamepanel.T8");
    fadesurface(background, 0, 0, 250);
}
