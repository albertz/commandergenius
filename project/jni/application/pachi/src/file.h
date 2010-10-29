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

void load_gamedata()
{
    background=LoadT8(DATADIR"/Tgfx/gamepanel.T8");
    backs=LoadT8(DATADIR"/Tgfx/backgrounds.T8");
    player=LoadT8(DATADIR"/Tgfx/pachi.T8");
    monsters=LoadT8(DATADIR"/Tgfx/monsters.T8");
    tiles=LoadT8(DATADIR"/Tgfx/tiles.T8");
    bright=LoadT8(DATADIR"/Tgfx/bright.T8");
    left=LoadT8(DATADIR"/Tgfx/left.T8");
    right=LoadT8(DATADIR"/Tgfx/right.T8");
    menufont=LoadT8(DATADIR"/fonts/font32v.T8");
    LoadT(&scorefont,DATADIR"/fonts/font16b.T");
    LoadT(&scorefont1,DATADIR"/fonts/font16a.T");
    LoadT(&scorefont2,DATADIR"/fonts/font16c.T");
    SDL_Surface *temp;
    temp = SDL_CreateRGBSurface(SDL_SWSURFACE,736,448,16,0,0,0,0); //esto es para reestablecer la pantalla
    screenbak = SDL_DisplayFormat(temp);
    SDL_FreeSurface(temp);
    SDL_SetColorKey(menufont, SDL_SRCCOLORKEY, SDL_MapRGB(menufont->format,0,255,0));
    SDL_SetColorKey(monsters, SDL_SRCCOLORKEY, SDL_MapRGB(monsters->format,0,255,0));
    SDL_SetColorKey(player, SDL_SRCCOLORKEY, SDL_MapRGB(player->format,0,0,0)); // el negro es transparente
    SDL_SetColorKey(tiles, SDL_SRCCOLORKEY, SDL_MapRGB(tiles->format,0,255,0));
    SDL_SetColorKey(bright, SDL_SRCCOLORKEY, SDL_MapRGB(bright->format,0,255,0));
    SDL_SetAlpha(bright, SDL_SRCALPHA, 128);
    SDL_SetColorKey(left, SDL_SRCCOLORKEY, SDL_MapRGB(left->format,0,0,0));
    SDL_SetColorKey(right, SDL_SRCCOLORKEY, SDL_MapRGB(right->format,0,0,0));

    jump       = Mix_LoadWAV(DATADIR"/sounds/jump.wav");
    obj        = Mix_LoadWAV(DATADIR"/sounds/obj.wav");
    objseq     = Mix_LoadWAV(DATADIR"/sounds/objseq.wav");
    die        = Mix_LoadWAV(DATADIR"/sounds/die.wav");
    timer      = Mix_LoadWAV(DATADIR"/sounds/timer.wav");
    exitlevel  = Mix_LoadWAV(DATADIR"/sounds/exit.wav");
    stageready = Mix_LoadWAV(DATADIR"/sounds/stageready.wav");
    storm      = Mix_LoadWAV(DATADIR"/sounds/storm.wav");
    respawnsnd = Mix_LoadWAV(DATADIR"/sounds/respawn.wav");

}
void unload_gamedata()
{
    SDL_FreeSurface(background);
    SDL_FreeSurface(tiles);
    SDL_FreeSurface(backs);
    SDL_FreeSurface(player);
    SDL_FreeSurface(bright);
    SDL_FreeSurface(monsters);
    SDL_FreeSurface(screenbak);
    SDL_FreeSurface(scorefont);
    SDL_FreeSurface(scorefont1);
    SDL_FreeSurface(scorefont2);
    SDL_FreeSurface(menufont);
    SDL_FreeSurface(left);
    SDL_FreeSurface(right);
    
    Mix_FreeChunk(exitlevel);
    Mix_FreeChunk(jump);
    Mix_FreeChunk(obj);
    Mix_FreeChunk(die);
    Mix_FreeChunk(timer);
    Mix_FreeChunk(storm);
    Mix_FreeChunk(stageready);
    Mix_FreeChunk(respawnsnd);
}

void stop_music()
{
    if(Mix_PlayingMusic())
    {
	Mix_FadeOutMusic(1000);
	SDL_Delay(1000);
	Mix_FreeMusic(music);
    }
}

void save_hiscoredata();

void load_hiscoredata()
{
    background=LoadT8(DATADIR"/Tgfx/gameover.T8");
    LoadT(&scorefont,DATADIR"/fonts/font16b.T");
    LoadT(&scorefont1,DATADIR"/fonts/font16d.T");

    FILE *file = fopen(SCOREDIR"/data/scores.dat","rb");
    if( file == NULL )
    {
	    for(int a=0; a < 10; a++)
	    {
		for(int b=0; b < 10; b++)
		{
		    scorename[a][b]=0;
		}
		scorestage[a]=0;
		scoretime[a]=0;
		scoredif[a]=0;
		scorescore[a]=0;
		}
		save_hiscoredata();
    }
    else
    {
	    for(int a=0; a < 10; a++)
	    {
		for(int b=0; b < 10; b++)
		{
		    scorename[a][b]=getc(file);
		}
		    scorename[a][10] = 0;
		    scorestage[a]=getc(file);
		    int lobyte,hibyte,vhibyte;
		    hibyte=getc(file);
		    lobyte=getc(file);
		    scoretime[a]=(hibyte*256)+lobyte;
		    scoredif[a]=getc(file);
		    vhibyte=getc(file);
		    hibyte=getc(file);
		    lobyte=getc(file);
		    scorescore[a]=(vhibyte*65536)+(hibyte*256)+lobyte;
	    }
	    fclose(file);
    }
}
void unload_hiscoredata()
{
    SDL_FreeSurface(background);
    SDL_FreeSurface(scorefont);
    SDL_FreeSurface(scorefont1);
}
void save_hiscoredata()
{
    FILE *file = fopen(SCOREDIR"/data/scores.dat","wb");

    for(int a=0;a<10;a++)
    {    
        for(int b=0;b<10;b++)
	{
	    putc(scorename[a][b],file);
	}
        putc(scorestage[a],file);

        int lobyte,hibyte,vhibyte;
        hibyte=int(scoretime[a]/256);
        lobyte=int(scoretime[a]-(hibyte*256));

        putc(hibyte,file);
        putc(lobyte,file);
        putc(scoredif[a],file);

        vhibyte=int(scorescore[a]/65536);
        hibyte=int((scorescore[a] - (vhibyte*256))/256);
        lobyte=int(scorescore[a] - (vhibyte*65536 + hibyte*256));

	putc(vhibyte,file);
	putc(hibyte,file);
	putc(lobyte,file);
    }
    fclose(file);
}

void load_helpgfx()
{
    background=LoadT8(DATADIR"/Tgfx/help.T8");
    LoadT(&scorefont,DATADIR"/fonts/font16b.T");
    LoadT(&scorefont1,DATADIR"/fonts/font16a.T");
    SDL_SetColorKey(scorefont, SDL_SRCCOLORKEY, SDL_MapRGB(scorefont->format,0,0,0));
    SDL_SetColorKey(scorefont1, SDL_SRCCOLORKEY, SDL_MapRGB(scorefont1->format,0,0,0));
}
void unload_helpgfx()
{
    SDL_FreeSurface(background);
    SDL_FreeSurface(scorefont);
    SDL_FreeSurface(scorefont1);
}

void load_menudata()
{
    menufont=LoadT8(DATADIR"/fonts/font32v.T8");
    menufont1=LoadT8(DATADIR"/fonts/font32r.T8");
    background=LoadT8(DATADIR"/Tgfx/intro.T8");
    SDL_SetColorKey(menufont, SDL_SRCCOLORKEY, SDL_MapRGB(menufont->format,0,255,0));
    SDL_SetColorKey(menufont1, SDL_SRCCOLORKEY, SDL_MapRGB(menufont1->format,0,255,0));

    option=Mix_LoadWAV(DATADIR"/sounds/option.wav");
    coderight=Mix_LoadWAV(DATADIR"/sounds/exit.wav");
    codewrong=Mix_LoadWAV(DATADIR"/sounds/die.wav");
    music=Mix_LoadMUS(DATADIR"/music/menu.s3m.ogg");
}
void unload_menudata()
{
    SDL_FreeSurface(menufont);
    SDL_FreeSurface(menufont1);
    SDL_FreeSurface(background);
    Mix_FreeChunk(option);
    Mix_FreeChunk(coderight);
    Mix_FreeChunk(codewrong);
}

void init_monsters()
{
    FILE *mnstr = fopen(DATADIR"/data/monsters.dat","rb");
    long filepos  = ((R_current-1) * M_max4room*3);
    fseek(mnstr,filepos,SEEK_SET);
    for(int n=0;n<=M_max4room-1;n++)
    {
	M_type[n]=getc(mnstr);
	M_x[n]=getc(mnstr)*R_tileside;
	M_y[n]=getc(mnstr)*R_tileside-M_h+R_tileside;
    }
    fclose(mnstr);
}

void load_room()
{
    long filepos;
    FILE *bck = fopen(DATADIR"/data/backs.dat","rb");
    filepos  = (R_current - 1) * (R_maxbacks_h*R_maxbacks_v); // filepos es el puntero del archivo, lo utilizamos para leer la habitacion donde estemos
    fseek(bck,filepos,SEEK_SET);
    int x, y;
    for(y = 0;y < R_maxbacks_v; y++)
    {
        for(x = 0;x < R_maxbacks_h; x++)
        {
            R_backdata[x][y]=getc(bck);
        }
    }
    fclose(bck);

    FILE *lvl = fopen(DATADIR"/data/rooms_v2.dat","rb");
    filepos  = (200 + ((R_current-1) * (R_maxtiles_h*R_maxtiles_v))+R_current); // filepos es el puntero del archivo, lo utilizamos para leer la habitacion donde estemos
    fseek(lvl,filepos,SEEK_SET);
    
    for(y = 0;y < R_maxtiles_v;y++)
    {
        for(x = 0;x < R_maxtiles_h; x++)
        {
            mroom.data[x][y]=getc(lvl);
        }
    }
    fclose(lvl);
    
    int contador;
    for(contador=0;contador < 5; contador++)
    {
        R_object=mobject[((R_current-1)*5)+contador].type;
        x=mobject[((R_current-1)*5)+contador].x;
        y=mobject[((R_current-1)*5)+contador].y;
        if(R_object != 0)
            mroom.data[x][y]=R_object;
    }
}
void load_roommap()
{
    long filepos;
    int x, y;
    FILE *lvl = fopen(DATADIR"/data/rooms_v2.dat","rb");

    for(y = 0;y < R_max_y;y++)
    {
        for(x = 0;x < R_max_x; x++)
        {
	    filepos=((x+1)*(R_maxtiles_h*R_maxtiles_v)+(x+1))+(y*(R_max_x*(R_maxtiles_h*R_maxtiles_v)+R_max_x));
	    fseek(lvl,200+filepos,SEEK_SET);
            roommap[x][y]=getc(lvl);
        }
    }
    fclose(lvl);
}
void init_objects() // esta rutina copia en la matriz objects el archivo objects.dat
{
    FILE *obj = fopen(DATADIR"/data/objects_v2.dat","rb");
    for(int n=0;n<2000;n++)
    {
        mobject[n].type=getc(obj);
        mobject[n].x=getc(obj);
        mobject[n].y=getc(obj);
        mobject[n].seq=getc(obj);
        mobject[n].stage=getc(obj);
    }
    fclose(obj);
}

void initcredits()
{
    creditslinecounter=0;
    background=LoadT8(DATADIR"/Tgfx/endcredits.T8");
    creditsfont=LoadT8(DATADIR"/fonts/font1.T8");
    SDL_SetColorKey(creditsfont, SDL_SRCCOLORKEY, SDL_MapRGB(creditsfont->format,0,255,0));
    SDL_Surface *temp;
    temp = SDL_CreateRGBSurface(SDL_SWSURFACE,600,440,16,0,0,0,0);
    creditsbuffer = SDL_DisplayFormat(temp);
    creditsbuffer1 = SDL_DisplayFormat(temp);
    SDL_FreeSurface(temp);
    setback();

music = Mix_LoadMUS(DATADIR"/music/credits.s3m.ogg");
}
void unloadcredits()
{
    SDL_FreeSurface(creditsbuffer);
    SDL_FreeSurface(creditsbuffer1);
    SDL_FreeSurface(background);
    SDL_FreeSurface(creditsfont);
}
