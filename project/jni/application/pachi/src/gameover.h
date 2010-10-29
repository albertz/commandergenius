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

void show_hiscores()
{
	SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format,0,0,0));
	SDL_BlitSurface(background,NULL,screen,NULL);
    SDL_SetColorKey(scorefont,SDL_SRCCOLORKEY,SDL_MapRGB(scorefont->format,0,0,0));
    SDL_SetColorKey(scorefont1,SDL_SRCCOLORKEY,SDL_MapRGB(scorefont1->format,0,0,0));
    SDL_SetColorKey(scorefont1,SDL_SRCCOLORKEY,SDL_MapRGB(scorefont1->format,0,0,0));
    char plyername[]="          ";
    char diflevel[] ="      ";
    print_text(scorefont1,screen,16,16,30, 55,"         PACHI EL MARCIANO    TOP TEN         ");//scorename[a]);
    print_text(scorefont1,screen,16,16,30, 95,"NAME         STAGE    TIME    LEVEL     SCORE ");//scorename[a]);
    print_text(scorefont1,screen,16,16,30,120,"----------------------------------------------");//scorename[a]);
    print_text(scorefont1,screen,16,16,30,535,"----------------------------------------------");//scorename[a]);

    for(int a=0; a < 10;a++)
    {
	strncpy(playername,scorename[a],10);
	print_text(scorefont,screen,16,16,30,170+(35*a),"%s",playername);//scorename[a]);
	print_text(scorefont,screen,16,16,268,170+(35*a),"%d",scorestage[a]);
	int mins=int(scoretime[a]/60);
	int secs=int(scoretime[a]-(mins*60));
	if(secs>9)
	    print_text(scorefont,screen,16,16,386,170+(35*a),"%d:%d",mins,secs);
	else
	    print_text(scorefont,screen,16,16,386,170+(35*a),"%d:0%d",mins,secs);
	
	if(scoredif[a]==1)
	    strcpy(diflevel," EASY ");
	if(scoredif[a]==2)
	    strcpy(diflevel,"NORMAL");
	if(scoredif[a]==3)
	    strcpy(diflevel," HARD ");
	print_text(scorefont,screen,16,16,502,170+(35*a),"%s",diflevel);
	print_text(scorefont,screen,16,16,672,170+(35*a),"%d",scorescore[a]);
    }
    SDL_Flip(screen);
}

void do_hiscores()
{
    load_hiscoredata();
    setback();
    show_hiscores();	
    escape_exit=0;

    while(escape_exit==0)
    {
	credits_events();
	SDL_Delay(1);
    }
    unload_hiscoredata();
}

void do_gameover()
{
    load_hiscoredata();

    char nameplayer[]="          ";
    strncpy(playername,nameplayer,10);
    namechar=0;
    int highscore=0;
    for(int a=9; a>=0;a--)
    {
	if(score>scorescore[a])
	{
	    highscore=1;
	    scorepos=a;
	}
    }
    if(highscore==1) // si se hizo una buena puntuacion
    {
	for(int b=8;b>=scorepos;b--)
	{
	    strncpy(scorename[b+1],scorename[b],10);
	    scorescore[b+1]=scorescore[b];
	    scorestage[b+1]=scorestage[b];
	    scoretime [b+1]=scoretime [b];
	    scoredif  [b+1]=scoredif  [b];
	}
	inputloop=1;
	SDL_Rect box;
	box.x=270;box.y=220;box.w=260;box.h=52;
	SDL_FillRect(screen,&box,SDL_MapRGB(screen->format,255,255,255));
	box.x=273;box.y=223;box.w=254;box.h=46;
	while(inputloop==1)
	{
	    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format,0,0,0));
		SDL_BlitSurface(background,NULL,screen,NULL);
	    hiscore_events();
	    SDL_FillRect(screen,&box,SDL_MapRGB(screen->format,0,0,0));
	    print_text(scorefont1,screen,16,16,box.x+10,box.y+5,"ENTER YOUR NAME");
	    print_text(scorefont1,screen,16,16,box.x+51+(namechar*16),box.y+29,".");
	    print_text(scorefont,screen,16,16,box.x+51,box.y+24,"%s",playername);
	    SDL_Flip(screen);
	}
	strncpy(scorename[scorepos],playername,10);
	scorescore[scorepos]=score;
	scorestage[scorepos]=stage;
	scoredif[scorepos]=dificulty;
	scoretime[scorepos]=int(playtime/1000);
	save_hiscoredata();
    }

    unload_hiscoredata();
}

void game_over()
{
    do_gameover();
    do_hiscores();
}
