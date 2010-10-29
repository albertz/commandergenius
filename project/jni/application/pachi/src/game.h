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

void win_game()
{

    background=LoadT8(DATADIR"/Tgfx/comic_09.T8");
    fadesurface(background, 0, 0, 200);
    escape_exit=0;
    while(escape_exit==0) { credits_events(); SDL_Delay(1); }
    background=LoadT8(DATADIR"/Tgfx/comic_10.T8");
    fadesurface(background, 0, 0, 200);
    escape_exit=0;
    while(escape_exit==0) { credits_events(); SDL_Delay(1); }
    background=LoadT8(DATADIR"/Tgfx/comic_11.T8");
    fadesurface(background, 0, 0, 200);
    escape_exit=0;
    while(escape_exit==0) { credits_events(); SDL_Delay(1); }
    background=LoadT8(DATADIR"/Tgfx/comic_12.T8");
    fadesurface(background, 0, 0, 200);
    escape_exit=0;
    while(escape_exit==0) { credits_events(); SDL_Delay(1); }
    background=LoadT8(DATADIR"/Tgfx/comic_13.T8");
    fadesurface(background, 0, 0, 200);
    escape_exit=0;
    while(escape_exit==0) { credits_events(); SDL_Delay(1); }
    background=LoadT8(DATADIR"/Tgfx/comic_14.T8");
    fadesurface(background, 0, 0, 200);
    escape_exit=0;
    while(escape_exit==0) { credits_events(); SDL_Delay(1); }

    SDL_FreeSurface(background);
    initcredits();
    escape_exit=0;

    while(escape_exit==0)
    {
	if(!Mix_PlayingMusic())
	    Mix_PlayMusic(music,0);
	credits_events();
	print_credits();
    }
    SDL_BlitSurface(background, NULL, screen, NULL);
    SDL_Flip(screen);
    stop_music();
    unloadcredits();
}

void check_exits() // esta funcion se fija si hay salidas disponibles
{
    int y = int(R_current/R_max_x);
    int x = (R_current-y*R_max_x)-1;
    int show; // esta variable indica si se debe imprimir la flecha o borrarla
    if((roomtime>1 && roomtime<1.5) || (roomtime>2 && roomtime <2.5) || (roomtime > 3 && roomtime <3.5) || (roomtime > 4 && roomtime <4.5))
	show=1;
    else
	show=0;
	
    if(x < R_max_x)
	if(roommap[x+1][y]==stage)
	    show_arrow(arrow_right, show);
    if(x > 0)
	if(roommap[x-1][y]==stage)
	    show_arrow(arrow_left, show);
    if(y < R_max_y)
	if(roommap[x][y+1]==stage)
	    show_arrow(arrow_down, show);
    if(y < 0)
	if(roommap[x][y-1]==stage)
	    show_arrow(arrow_up, show);
}

void stage_up()
{
    long filepos;
    SDL_Rect scorerect;
    scorerect.x = 30; scorerect.y = 484; scorerect.w = 400; scorerect.h = 96;
    if(stage+1 != CEMETERY && stage+1 != CAVES && stage+1 != CASTLE && stage+1 != LAB)
    {
	SDL_FillRect(screen, &scorerect, 0); // limpiamos el cuadrado de los marcadores
	//SDL_UpdateRect(screen, scorerect.x, scorerect.y, scorerect.w, scorerect.h);
    }
    mplayer[dificulty].objects = 0; mplayer[dificulty].stageup = 0;
    mplayer[dificulty].left = 0; mplayer[dificulty].right = 0;
    mplayer[dificulty].jump = 0; mplayer[dificulty].duck = 0; mplayer[dificulty].jumpcounter = 0;
    Mix_HaltChannel(-1);
    // ahora agregamos el puntaje correspondiente al tiempo que nos sobró
    if(minutes>0||seconds>0) // esto lo hacemos por si se perdio por tiempo
    {
	for(int a=int(gametimer);a>0;a--)
	{
		SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format,0,0,0));
	    score=score+(dificulty*45)+(int(dificulty/3)*45);
	    gametimer--;
	    minutes = int(gametimer/60);
    	    seconds = int(gametimer-minutes*60);
	    Mix_Volume(0,100);
	    Mix_PlayChannel(0,timer,0);
	    print_monitor();
	    print_timer();
	    SDL_Flip(screen);
	    SDL_Delay(20);
	}
    }
    stage++;
    if(stage <= NUM_STAGES)
    {
	gametimer = mstage[stage-1].time-((dificulty-1)*20); // para los niveles normal y dificil el tiempo es 30 y 60 segundos menor respectivamente
	showcode(mstage[stage-1].code);
	showposter(stage);
	
	music=Mix_LoadMUS(mstage[stage-1].music);
	stagecompleted = 0;
	FILE *lvl = fopen(DATADIR"/data/rooms_v2.dat","rb");
	filepos  = (stage - 1) * 4; // filepos es el puntero del archivo, lo utilizamos para leer en que habitacion estamos
	fseek(lvl,filepos,SEEK_SET);
	R_x = getc(lvl);
	R_y = getc(lvl);
	mplayer[dificulty].start_x = getc(lvl) * R_tileside; // variables de posicion inicial en la pantalla
	mplayer[dificulty].start_y = getc(lvl) * R_tileside; // variables de posicion inicial en la pantalla
	mplayer[dificulty].x = mplayer[dificulty].start_x;
	mplayer[dificulty].y = mplayer[dificulty].start_y;
	R_current = (R_x + ((R_y - 1)*R_max_x));
	fclose(lvl);
	init_room();
    }
    else
    {
	game_exit=1;
    }
}

void start_game()
{
	SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format,0,0,0));
    LoadT(&font,DATADIR"/fonts/font16d.T");
    SDL_SetColorKey(font, SDL_SRCCOLORKEY, SDL_MapRGB(font->format,0,0,0));
    print_text(font,screen,16,16,140,90,"LOADING GAME DATA... PLEASE WAIT");
    //SDL_UpdateRect(screen,140,90,520,16);
    SDL_Flip(screen);
    SDL_FreeSurface(font);
    
    setgame();
    
    load_gamedata();
    init_objects();
    if(stage+1 != CEMETERY && stage+1 != CAVES && stage+1 != CASTLE && stage+1 != LAB)
	setback();
	
    stage_up(); // con esta rutina aumentamos la fase en 1 y leemos la pantalla inicial y la posicion inicial del personaje
    init_monsters();
    playtime=SDL_GetTicks();
    game_exit=0;
    while(game_exit==0)
    {
	game_loop();
    }
    playtime=SDL_GetTicks()-playtime;
    unload_gamedata();
    stop_music();

    if(stage>NUM_STAGES)
    {
	escape_exit=0;
        win_game();
    }

    if(mplayer[dificulty].lives == 0 || stage > NUM_STAGES)
	game_over();
}

void init_room()
{
	roomtime=0; // indica el tiempo que llevamos en una habitacion
	flushevents();
        load_room();
	init_monsters();
        print_room();
    if(stage<=3) // solamente se muestra la flecha indicadora en las 3 primeras pantallas
	chk_exits=1;
    else
	chk_exits=0;
}

void animatemonsters()
{
    M_frame++;
    if(M_frame > M_frames)
	M_frame=1;
}

void animatefx()
{
    brightframe++;
    if(brightframe > 9)
	brightframe=0;
}

void animate() //rutina para la animacion de los sprites
{

    int animspeed=100;
    animcounter=animcounter+(imove*animspeed);
    if(animcounter > 10)
    {
	animatemonsters();
	animateplayer();
	animatefx();
	animcounter = 0;
    }
}
void play_music()
{
    if(!Mix_PlayingMusic())
	Mix_PlayMusic(music,0);
}

void chk_state()
{
    if(mplayer[dificulty].dead==1)
    {
    	loose_life();
        if(mplayer[dificulty].lives<1)
	    game_exit=1;
	else
	{
	    respawned=0;
	    Mix_Volume(1,90);
	    Mix_PlayChannel(1,respawnsnd,1);
	    while(respawned==0)
    	    {
		delta_time();
		respawn();
	    }
	}
    }
    if(mplayer[dificulty].stageup==1)
    {
	while(Mix_Playing(0))
	{}
	stage_up();
    }
    if(chk_exits==1 && roomtime < 5)
	check_exits();
}

double delta_time(void)
{
    curtime=SDL_GetTicks();
    imove=(curtime-lasttime)/1000.0;
    lasttime=curtime;
    if(imove>0.2)
	imove=0.2;
    return imove;
}

void print_timer()
{
    print_text(scorefont,screen,16,16,224,564,"TIME   :");
    if(minutes==0 && seconds <=10)
    {
	Mix_Volume(0,100);
    	font=scorefont2;
	if((int(oldsec-1)==int(seconds))&& seconds > 0)
	    Mix_PlayChannel(0,timer,0);	
    }
    else
	font=scorefont1;

    if(seconds>=10)
	print_text(font,screen,16,16,352,564,"%d:%d",minutes,seconds);
    else
	print_text(font,screen,16,16,352,564,"%d:0%d",minutes,seconds);

    //SDL_UpdateRect(screen,224,544,208,40);
}

void do_gametimer()
{
    gametimer-=imove;
    roomtime+=imove;
    minutes = int(gametimer/60);
    seconds = int(gametimer-minutes*60);

    print_timer();
    screen_fx(); // efectos especiales de pantalla
    if(minutes==0 && seconds == 0)
    {
	stage--;
	init_objects();
	loose_life();
	if(Mix_PlayingMusic())
	    Mix_FreeMusic(music);
	stage_up();
    }
    oldsec=seconds;

}

void chk_sequence()
{
    int n;
    sequence=mstage[stage-1].objects+1;
    for(n=0;n<2000;n++)
    {
	if(mobject[n].stage==stage)
	{
	    if(mobject[n].seq<sequence && mobject[n].seq>0)
		sequence=mobject[n].seq;
//fprintf(stderr,"stage=%d  object stage=%d\n",stage, mobject[n].stage);

//fprintf(stderr,"proximo objeto de la secuencia es=%d, numero=%d, x=%d, y=%d   stage=%d \n",mobject[n].seq, n, mobject[n].x, mobject[n].y, mobject[n].stage);
	}
    }
    int object_is_here=0; // esta variable indica si el objeto que sigue en la secuencia esta en la pantalla actual
    int bright_x, bright_y; // variables que indica en que posicion debe aparecer el brillo del proximo objeto a recoger
    for(n=0;n<5;n++)
    {
	if(mobject[(R_current-1)*5+n].seq==sequence)
	{
	    object_is_here=1;
	    bright_x=mobject[(R_current-1)*5+n].x;
	    bright_y=mobject[(R_current-1)*5+n].y;
	}	    
    }
    if(object_is_here==1)
    {
	bright_obj(bright_x, bright_y);
    }
}	    

void game_loop()
{
	SDL_BlitSurface(background,NULL,screen,NULL);
	print_room();
	delta_time();
	do_gametimer(); // esta rutina decrementa e imprime el timer
	play_music();
	game_events();
	animate();
	clean_player();
	if(fadingobject==1) // chquea si se debe eliminar un objeto
	    alphade=fade_object(alphade);
	print_monsters();
	print_player();
	chk_sequence(); // esta rutina se fija cual es el proximo objeto de la sequencia (Bomb Jack feature)
	chk_state(); // esta rutina chequea si el personaje se murio, o si paso de nivel
	SDL_Flip(screen);
	SDL_Delay(1);
}
