/*
    Rutina de efectos especiales
*/
SDL_Surface *fadeobject;
SDL_Rect fadingpos; // posicion del objeto que esta desvaneciendose
SDL_Rect realfadingpos; // posicion del objeto que esta desvaneciendose en el area de juego
float fade_y;

float scanpos_y=R_gamearea_y - (P_h*P_h);


void respawn() // rutina que hace reaparecer a Pachi
{
    int scanspeed=1500;
    SDL_Rect P_oldscansrect[P_h];
    SDL_Rect P_scansrect[P_h];
    SDL_Rect P_srcscan[P_h];
//imprimimos los scans
    for(int a=P_h-1;a>=0;a--)
    {
	P_scansrect[a].x=int(mplayer[dificulty].x+R_gamearea_x);P_scansrect[a].w=mplayer[dificulty].w;P_scansrect[a].h=1;
	P_scansrect[a].y=int(scanpos_y+(a*P_h));
	P_srcscan[a].h=1;P_srcscan[a].w=mplayer[dificulty].w;P_srcscan[a].x=(mplayer[dificulty].w*(mplayer[dificulty].frame));P_srcscan[a].y=0+a;
	P_oldscansrect[a]=P_scansrect[a];

	if((P_scansrect[a].y<mplayer[dificulty].y+R_gamearea_y+P_h-(P_h-a))&&(P_scansrect[a].y>R_gamearea_y))
	{
	    SDL_BlitSurface(player,&P_srcscan[a],screen,&P_scansrect[a]);
	    //SDL_UpdateRect(screen,P_scansrect[a].x,P_scansrect[a].y,P_scansrect[a].w,P_scansrect[a].h);
	}
    }
    if(scanpos_y<mplayer[dificulty].y+P_h)
	scanpos_y+=scanspeed*imove;
    else
    {
	scanpos_y=R_gamearea_y - (P_h*P_h);
	respawned=1;
	imove=0.01;
	SDL_Flip(screen);
    }

//borramos los scans recuperando el fondo que habia antes    

    SDL_Rect bakscansrect[P_h];
    for(int a=P_h-1;a>=0;a--)
    {
	bakscansrect[a]=P_oldscansrect[a];
	P_oldscansrect[a].x-=R_gamearea_x;
	P_oldscansrect[a].y-=R_gamearea_y;

        SDL_BlitSurface(screenbak,&P_oldscansrect[a],screen,&bakscansrect[a]);
    }
}


void initfade_object(int x, int y, int object)
{
    alphade=255;
    object-=200;
    fadeobject=SDL_CreateRGBSurface(SDL_SWSURFACE,R_tileside,R_tileside,16,0,0,0,0);
    SDL_SetColorKey(fadeobject,SDL_SRCCOLORKEY,(fadeobject->format,0,255,0));
    SDL_Rect objectsrc;
	objectsrc.w=R_tileside;objectsrc.h=R_tileside;
	objectsrc.y=R_tileside*10;objectsrc.x=R_tileside*object;
    SDL_BlitSurface(tiles,&objectsrc,fadeobject,NULL);
    fadingpos.x=x*R_tileside;fadingpos.y=y*R_tileside;
    fadingpos.w=R_tileside;fadingpos.h=R_tileside;
    fade_y=fadingpos.y;
}

float fade_object(float alpha)
{
    int fadealpha = int(alpha);
    SDL_SetAlpha(fadeobject,SDL_SRCALPHA,fadealpha);
    realfadingpos.x=R_gamearea_x+fadingpos.x;realfadingpos.y=R_gamearea_y+fadingpos.y;realfadingpos.w=fadingpos.w;realfadingpos.h=fadingpos.h;

    SDL_BlitSurface(screenbak,&fadingpos,screen,&realfadingpos);
    SDL_BlitSurface(fadeobject,NULL,screen,&realfadingpos);
    //SDL_UpdateRect(screen,realfadingpos.x,realfadingpos.y,realfadingpos.w,realfadingpos.h+2);
    if(fadealpha<5)
    {
    	fadingobject=0;
	SDL_BlitSurface(screenbak,&fadingpos,screen,&realfadingpos);
	print_room();
	SDL_Flip(screen);
    }
    else
    {
	alpha-=500*imove;
	fade_y-=150*imove;
	if(fade_y<32)
	    fade_y+=150*imove;
	fadingpos.y=int(fade_y);
    }
    return(alpha);
}


void screen_fx()
{
    Mix_Volume(7,128);
    if(seconds==15 || seconds==45)
    {
	if(stage<6) // en el cementerio
	{
	    if(int(oldsec-1)==int(seconds))
	    {
		Mix_PlayChannel(7,storm,0);
		blinkscreen(220,220,255,128);
		load_room();
		print_room();
    	    }
	}    
    }
}

void bright_obj(int bright_x, int bright_y)
{
    SDL_Rect brsrc;
	brsrc.x=(brightframe*40);
	brsrc.y=0;brsrc.w=40;brsrc.h=40;
    SDL_Rect brdst;
	brdst.x=bright_x*R_tileside-4;brdst.y=bright_y*R_tileside-4;brdst.w=40;brdst.h=40;
    SDL_Rect brdstreal;
	brdstreal.x=bright_x*R_tileside-4+R_gamearea_x;brdstreal.y=bright_y*R_tileside-4+R_gamearea_y;brdstreal.w=40;brdstreal.h=40;

    SDL_Rect objsrc;
	objsrc.w=R_tileside;objsrc.w=R_tileside;
	objsrc.x=(mroom.data[bright_x][bright_y] - (int(mroom.data[bright_x][bright_y]/20) * 20))*R_tileside;
	objsrc.y=(int(mroom.data[bright_x][bright_y]/20) * R_tileside);                     
    SDL_Rect objdst;
	objdst.x=bright_x*R_tileside;
	objdst.y=bright_y*R_tileside;
	objdst.w=R_tileside;objdst.h=R_tileside;
    SDL_Rect objdstreal;
	objdstreal.x=bright_x*R_tileside+R_gamearea_x;
	objdstreal.y=bright_y*R_tileside+R_gamearea_y;
	objdstreal.w=R_tileside;objdst.h=R_tileside;

    SDL_BlitSurface(screenbak,&brdst,screen,&brdstreal);
//    SDL_BlitSurface(tiles,&objsrc,screen,&objdstreal);
    SDL_BlitSurface(bright,&brsrc,screen,&brdstreal);
    //SDL_UpdateRect(screen,brdstreal.x,brdstreal.y,brdstreal.w,brdstreal.h);
}

void show_arrow(int arrow, int show)
{
    SDL_Rect dst;
	dst.w=150; dst.h=30;dst.y=R_gamearea_y+1;
    SDL_Rect src;
	src.w=dst.w; src.h=dst.h; src.y=dst.y - R_gamearea_y;
    switch(arrow)
    {
	case arrow_left:
	    dst.x=R_gamearea_x+1;
	    src.x=dst.x-R_gamearea_x;
	    if(show==1)
		SDL_BlitSurface(left, NULL, screen, &dst);
	    else
		SDL_BlitSurface(screenbak, &src, screen, &dst);
		
	    //SDL_UpdateRect(screen, dst.x, dst.y, dst.w, dst.h);
	    break;
	case arrow_right:
	    dst.x=R_gamearea_x+(R_maxtiles_h*R_tileside)-(10+dst.w);
	    src.x=dst.x-R_gamearea_x;
	    if(show==1)
		SDL_BlitSurface(right, NULL, screen, &dst);
	    else
		SDL_BlitSurface(screenbak, &src, screen, &dst);

	    //SDL_UpdateRect(screen, dst.x, dst.y, dst.w, dst.h);
	    break;
	case arrow_up:
//fprintf(stderr,"up");
	    break;
	case arrow_down:
//fprintf(stderr,"down");
	    break;
    }
}
