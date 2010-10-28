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

void do_code()
{
    Mix_Volume(3,100);
    SDL_Rect coderect;
    coderect.x=300;
    coderect.y=232;
    coderect.w=200;
    coderect.h=50;
    SDL_FillRect(screen, &coderect, SDL_MapRGB(screen->format,255,255,255));
    coderect.x=303;
    coderect.y=235;
    coderect.w=194;
    coderect.h=44;
    SDL_FillRect(screen, &coderect, SDL_MapRGB(screen->format,0,0,0));
    codechar=0;
    inputloop=1;
//    SDL_SetColorKey(menufont1,0,SDL_MapRGB(menufont1->format,0,255,0));
    while(inputloop==1)
    {
	code_events();
	print_text(menufont1, screen, 32, 32, 336, 241, "%s",code);
        SDL_UpdateRect(screen, 300, 232, 200, 50);
    }
//    SDL_SetColorKey(menufont1,SDL_SRCCOLORKEY,SDL_MapRGB(menufont1->format,0,255,0));

    // comparar si el codigo es correcto
    
    startstage = 0;
    int right = 0;

    int n;
    for(n=1;n < NUM_STAGES+1;n++)
    {
	if(strcmp(code,mstage[n-1].code)==0)
	{
	    startstage=n-1;
	    right=1;
	}
    }
    if(right==1)    
	Mix_PlayChannel(3,coderight,0);
    else
	Mix_PlayChannel(3,codewrong,0);

    setback();
}

void do_menu()
{
    SDL_Delay(1);
    font = menufont;

    int menustartpos=156;

    if(menuvalue == 1) font = menufont1; else font = menufont;
    print_text(font, screen, 32, 32 ,305, menustartpos, " PLAY");
    if(menuvalue == 2) font = menufont1; else font = menufont;
    {
	SDL_Rect restoremenu;
	restoremenu.x=305; restoremenu.y=menustartpos+(1*40);restoremenu.w=300;restoremenu.h=32;
	SDL_BlitSurface(background,&restoremenu,screen,&restoremenu);
	
	if(dificulty==1)
	{
	    print_text(font, screen, 32, 32 ,305, menustartpos+(1*40), " EASY");
	}
	if(dificulty==2)
	{
	    print_text(font, screen, 32, 32 ,305, menustartpos+(1*40), "NORMAL");
	}
	if(dificulty==3)
	{
	    print_text(font, screen, 32, 32 ,305, menustartpos+(1*40), " HARD");
	}
    }
    if(menuvalue == 3) font = menufont1; else font = menufont;
    print_text(font, screen, 32, 32 ,305, menustartpos+(2*40), "SCREEN");
    if(menuvalue == 4) font = menufont1; else font = menufont;
    print_text(font, screen, 32, 32 ,305, menustartpos+(3*40), " CODE");
    if(menuvalue == 5) font = menufont1; else font = menufont;
    print_text(font, screen, 32, 32 ,305, menustartpos+(4*40), " HELP");
    if(menuvalue == 6) font = menufont1; else font = menufont;
    print_text(font, screen, 32, 32 ,305, menustartpos+(5*40), "SCORES");
    if(menuvalue == 7) font = menufont1; else font = menufont;
    print_text(font, screen, 32, 32 ,305, menustartpos+(6*40), " EXIT");

    //SDL_UpdateRect(screen, 300, 160, 240, 300);
}
				
void menu_loop()
{
    do_menu();
    if(!Mix_PlayingMusic())
	Mix_PlayMusic(music,0);
    SDL_Delay(1);
}

void menu()
{
    load_menudata();
    float alpha=0;
    while(alpha<255)
    {
		SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format,0,0,0));
		alpha=intro_blit(background,0,0,800,600,alpha,200,0);
	    SDL_Flip(screen);
		delta_time();
    }
    menuvalue=1;    
    while(gameexit==0 && startgame==0 && help==0 && hiscores==0)
    {
	    SDL_BlitSurface(background, NULL, screen, NULL);
        menu_events();
		menu_loop();
	    SDL_Flip(screen);
    }
    unload_menudata();
    stop_music();
}    
