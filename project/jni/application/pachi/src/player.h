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

void loose_life()
{
    Mix_Volume(4,100);
    Mix_PlayChannel(4,die,0);
    blinkscreen(255,0,0,0);
    mplayer[dificulty].x=mplayer[dificulty].start_x;
    mplayer[dificulty].y=mplayer[dificulty].start_y;
    mplayer[dificulty].h=mplayer[dificulty].h;
    mplayer[dificulty].left=0;
    mplayer[dificulty].right=0;
    mplayer[dificulty].jump=0;
    mplayer[dificulty].duck=0;
    mplayer[dificulty].lives--;
    mplayer[dificulty].jumpcounter=0;
    mplayer[dificulty].dead=0;
    if(mplayer[dificulty].lives>0)
    {
    blinkscreen(255,255,255,0);
	load_room();
	print_room();
	init_monsters();
    }
}

void pick_obj(int x, int y) // esta rutina elimina el objeto de la matriz objects, lo elimina de la pantalla y le suma la puntuacion conrrespondiente al jugador
{
    initfade_object(x, y, mroom.data[x][y]);
    fadingobject=1;
    mroom.data[x][y]=0;
    for(int n=0;n<5;n++)
    {
	if((mobject[(R_current-1)*5+n].x == x) && (mobject[(R_current-1)*5+n].y == y))
	{
	    score = score + (50*dificulty);
	    if(sequence==mobject[(R_current-1)*5+n].seq)
	    {
		gametimer=gametimer+5;
		Mix_Volume(6,128);
		Mix_PlayChannel(6,objseq,0);
	    }
	    mobject[(R_current-1)*5+n].type = 0;
	    mobject[(R_current-1)*5+n].x = 0;
	    mobject[(R_current-1)*5+n].y = 0;
	    mobject[(R_current-1)*5+n].seq = 0;
	    print_monitor();
	}
    }
    
    mplayer[dificulty].objects++;
    Mix_Volume(1,100);
    if(mplayer[dificulty].objects==mstage[stage-1].objects)
    {
	stagecompleted = 1;
	blinkscreen(255,255,255,0);
	Mix_PlayChannel(1,stageready,0);
    }
    else
	Mix_PlayChannel(1,obj,0);
    print_room();
}

void chk_touched(int x, int y)
{
    if((mroom.data[x][y] >= 239) && (stagecompleted == 1))
    {
	Mix_HaltMusic();
	Mix_Volume(0,100);
	Mix_PlayChannel(0,exitlevel,0);
	mplayer[dificulty].stageup=1;
    }
    if((mroom.data[x][y] >= 220) && (mroom.data[x][y] < 239))
	    mplayer[dificulty].dead=1;
    if((mroom.data[x][y] < 220) && (mroom.data[x][y] >= 200))
        pick_obj(x,y);
}

void P_chk_right()
{
    int x, y;
    if(mplayer[dificulty].x + mplayer[dificulty].w >= (R_tileside*R_maxtiles_h))
    {
        R_current++;
        mplayer[dificulty].x = 2;
	mplayer[dificulty].start_x = int(mplayer[dificulty].x);
	mplayer[dificulty].start_y = int(mplayer[dificulty].y);
	if(mplayer[dificulty].duck==1)
	    mplayer[dificulty].start_y=mplayer[dificulty].start_y-(P_h - P_h/2); // P_h es la altura parado y P_h/2 la altura agachado
	init_room();
    }

	for(int n=0;n<=mplayer[dificulty].h-mplayer[dificulty].tolerance;n++)
	{
	    x=int(mplayer[dificulty].x+mplayer[dificulty].w)/R_tileside;
	    y=int((mplayer[dificulty].y+n)/R_tileside);
	    chk_touched(x, y);

	    if(mroom.data[x][y] >= 80 && mroom.data[x][y] < 200)
    		    mplayer[dificulty].x=mplayer[dificulty].old_x;
	}
}

void P_chk_left()
{
    int x, y;
    if(mplayer[dificulty].x < 2)
    {
        R_current--;
	init_room();
        mplayer[dificulty].x = R_maxtiles_h*R_tileside - mplayer[dificulty].w;
	mplayer[dificulty].start_x = int(mplayer[dificulty].x); //estas variables son para cuando se pierde una vida se vuelve el player a esta posicion
	mplayer[dificulty].start_y = int(mplayer[dificulty].y);
	if(mplayer[dificulty].duck==1)
	    mplayer[dificulty].start_y=mplayer[dificulty].start_y-(P_h - P_h/2);
    }

	for(int n=0;n<=mplayer[dificulty].h-mplayer[dificulty].tolerance;n++)
	{
	    x=int(mplayer[dificulty].x)/R_tileside; //posible P_x -1
	    y=int((mplayer[dificulty].y+1+n)/R_tileside);
	    chk_touched(x,y);

            if(mroom.data[x][y] >= 80 && mroom.data[x][y] < 200)
		mplayer[dificulty].x=mplayer[dificulty].old_x;
	}
}

void chk_up()
{
    int x, y;
    if(mplayer[dificulty].y <= 1)
    {
        R_current = R_current - R_max_x;
        mplayer[dificulty].y = float(((R_maxtiles_v-1)*R_tileside) - mplayer[dificulty].h - 1);
	mplayer[dificulty].start_x  = int(mplayer[dificulty].x); //estas variables son para cuando se pierde una vida se vuelve el player a esta posicion
	mplayer[dificulty].start_y  = int(mplayer[dificulty].y);
	mplayer[dificulty].startjump=int(mplayer[dificulty].y+(R_tileside*1.5));
	init_room();
    }
    for(int n=0;n<=mplayer[dificulty].w-1;n++)
    {
        x=int(mplayer[dificulty].x+n)/R_tileside;
        y=int(mplayer[dificulty].y/R_tileside);
        chk_touched(x,y); //esta rutina compruba que tipo de tile toca el personaje

        if(mroom.data[x][y] >= 80 && mroom.data[x][y] < 200)
        {
	    mplayer[dificulty].y=int((y+1)*R_tileside);
	    if(mplayer[dificulty].jump == 1 && mplayer[dificulty].upflag == 1)
	    {
	        mplayer[dificulty].upflag = 0;
	        mplayer[dificulty].downflag = 1;
	    }
	}
    }
}

void chk_down()
{
    int x, y;
    if((mplayer[dificulty].jump==0) || (mplayer[dificulty].downflag==1))
    {
	    for(int n=0;n<mplayer[dificulty].w ;n++)
	    {
		x=int(mplayer[dificulty].x+n)/R_tileside;
		y=int((mplayer[dificulty].y+mplayer[dificulty].h+1)/R_tileside);
		chk_touched(x,y);

		if(mroom.data[x][y] >= 40 && mroom.data[x][y] < 200)
		{
		    mplayer[dificulty].y=mplayer[dificulty].old_y;
		    mplayer[dificulty].infloor=1; // esta varible indica si esta en el suelo
		    mplayer[dificulty].y=int((mplayer[dificulty].y+(R_tileside/2))/R_tileside)*R_tileside;
		    mplayer[dificulty].jump=0;
		    mplayer[dificulty].jumpcounter=0;
		    mplayer[dificulty].upflag=0;
		    mplayer[dificulty].downflag=1;
		}
	    }
	if(mplayer[dificulty].y >= (R_maxtiles_v*R_tileside) - mplayer[dificulty].h)
	{
    	    R_current = R_current + R_max_x;
	    init_room();
    	    mplayer[dificulty].y = 2;
	    mplayer[dificulty].start_x = int(mplayer[dificulty].x); //estas variables son para cuando se pierde una vida se vuelve el player a esta posicion
	    mplayer[dificulty].start_y = int(mplayer[dificulty].y);
	}
    }
}

void chk_colmonsters() //chequear colisiones contra monstruos
{
    for(int n=0;n<=M_max4room-1;n++)
    {
	if(M_type[n] != 0)
	    if(mplayer[dificulty].y < M_y[n]+M_h-mplayer[dificulty].tolerance && mplayer[dificulty].y+mplayer[dificulty].h > M_y[n]+mplayer[dificulty].tolerance)
		if(mplayer[dificulty].x < M_x[n]+M_h-mplayer[dificulty].tolerance && mplayer[dificulty].x+mplayer[dificulty].w > M_x[n]+mplayer[dificulty].tolerance)
		    mplayer[dificulty].dead=1;
    }
}	

void set_player_pos()
{
    if(mplayer[dificulty].dead==0) // si el personaje esta muerto no chequea colisiones
    {
	mplayer[dificulty].infloor = 0;
        mplayer[dificulty].old_x = mplayer[dificulty].x;
        mplayer[dificulty].old_y = mplayer[dificulty].y;

	chk_colmonsters(); // esta rutina chequea colisiones contra monstruos

        if(mplayer[dificulty].duck == 1)
	    mplayer[dificulty].h=P_h/2;
	else
	    mplayer[dificulty].h=P_h;


	if(mplayer[dificulty].left==1)
	{
	    if(mplayer[dificulty].duck==1)
		mplayer[dificulty].speed=mplayer[dificulty].realspeed/2;
	    else
		mplayer[dificulty].speed=mplayer[dificulty].realspeed;

	    mplayer[dificulty].x=mplayer[dificulty].x-(mplayer[dificulty].speed*imove);

    	    P_chk_left();
	}
	if(mplayer[dificulty].right==1)
	{
	    if(mplayer[dificulty].duck==1)
		mplayer[dificulty].speed=mplayer[dificulty].realspeed/2;
	    else
    	        mplayer[dificulty].speed=mplayer[dificulty].realspeed;

	    mplayer[dificulty].x=mplayer[dificulty].x+(mplayer[dificulty].speed*imove);

	    P_chk_right();
	}
	if(mplayer[dificulty].jump==1)
	{
	    if(mplayer[dificulty].upflag==1)
	    {
		mplayer[dificulty].jumpcounter=int(mplayer[dificulty].startjump-mplayer[dificulty].y);

		mplayer[dificulty].maxjump=mplayer[dificulty].realmaxjump;
		    
		if(mplayer[dificulty].jumpcounter <= mplayer[dificulty].maxjump)
		{
		    if(mplayer[dificulty].jumpcounter >= mplayer[dificulty].maxjump/1.5)
                    {                                                           
                        if(mplayer[dificulty].jumpslower==1)                                     
                        {                                                       
                            mplayer[dificulty].y=mplayer[dificulty].y+(mplayer[dificulty].fallspeed*imove);                            
                            mplayer[dificulty].jumpslower=0;                                     
                        }                                                       
                        else                                                    
	                    mplayer[dificulty].jumpslower=1;                                     
	            }                                                           
	            mplayer[dificulty].y=mplayer[dificulty].y-(mplayer[dificulty].fallspeed*imove);                                    
		}
		else
		{
		    mplayer[dificulty].downflag = 1;
		    mplayer[dificulty].upflag = 0;
		}
		chk_up();
	    }
	    if(mplayer[dificulty].downflag==1)
	    {
		mplayer[dificulty].jumpcounter=int(mplayer[dificulty].y-mplayer[dificulty].startjump);
		if(mplayer[dificulty].jumpcounter > 0)
		{

		    mplayer[dificulty].maxjump=mplayer[dificulty].realmaxjump;

		    if(mplayer[dificulty].jumpcounter >= mplayer[dificulty].maxjump/1.5)
                    {                                                           
                        if(mplayer[dificulty].jumpslower==1)                                     
                        {                                                       
                            mplayer[dificulty].y=mplayer[dificulty].y-(mplayer[dificulty].fallspeed*imove);                            
                            mplayer[dificulty].jumpslower=0;                                     
                        }                                                       
                        else                                                    
                            mplayer[dificulty].jumpslower=1;                                     
                    }                                                           
                    mplayer[dificulty].y=mplayer[dificulty].y+(mplayer[dificulty].fallspeed*imove);
		}
		else
		{
		    mplayer[dificulty].jump = 0;
		}
	    }
	}
	if(mplayer[dificulty].jump==0)
	    mplayer[dificulty].y=mplayer[dificulty].y+(mplayer[dificulty].fallspeed*imove); // para que se caiga si no hay suelo debajo
	chk_down();
    }    
    oldplayerpos.x = int(mplayer[dificulty].old_x);
    oldplayerpos.y = int(mplayer[dificulty].old_y);
    oldplayerpos.w = mplayer[dificulty].w;
    oldplayerpos.h = mplayer[dificulty].h;
    if(mplayer[dificulty].ducked==1)
    {
	mplayer[dificulty].ducked = 0;
	oldplayerpos.h = oldplayerpos.h + (P_h - P_h/2);
	oldplayerpos.y = oldplayerpos.y - (P_h - P_h/2);
    }
    playerpos.x = int(mplayer[dificulty].x);
    playerpos.y = int(mplayer[dificulty].y);
}

void clean_player()
{
    set_player_pos();
    playerposreal.x = playerpos.x + R_gamearea_x;
    playerposreal.y = playerpos.y + R_gamearea_y;
    oldplayerposreal.x = oldplayerpos.x + R_gamearea_x;
    oldplayerposreal.y = oldplayerpos.y + R_gamearea_y;

    SDL_BlitSurface(screenbak,&oldplayerpos,screen, &oldplayerposreal);
}
void print_player()
{
    playersrc.y = mplayer[dificulty].framer*P_h;
    playersrc.x = mplayer[dificulty].w * (mplayer[dificulty].frame - 1 + mplayer[dificulty].facingleft*mplayer[dificulty].frames);
    playersrc.w = mplayer[dificulty].w;
    playersrc.h = mplayer[dificulty].h;
    
    SDL_BlitSurface(player, &playersrc, screen, &playerposreal);
    SDL_UpdateRect(screen,oldplayerposreal.x,oldplayerposreal.y,P_w,oldplayerpos.h);
    SDL_UpdateRect(screen,playerposreal.x,playerposreal.y,mplayer[dificulty].w,mplayer[dificulty].h);
}

void animateplayer()
{
// Esta rutina pone el valor correspondiente a la fila de sprites en el archivo BMP del personaje en la variable framer
    
    if(mplayer[dificulty].right!=0 || mplayer[dificulty].left != 0)
    {
	mplayer[dificulty].frame++;
	if(mplayer[dificulty].duck==1 && mplayer[dificulty].right==0 && mplayer[dificulty].left==0)
	    mplayer[dificulty].frame--;
    }

    if(mplayer[dificulty].right==1 || mplayer[dificulty].left == 1)
	mplayer[dificulty].framer = 1; 
    if(mplayer[dificulty].right==0 && mplayer[dificulty].left == 0 && mplayer[dificulty].duck==0) // parado
    {
    	mplayer[dificulty].framer = 0;
	mplayer[dificulty].frame++;
    }
    if(mplayer[dificulty].duck==1) // agachado
	mplayer[dificulty].framer = 2;
    if(mplayer[dificulty].jump==1 && mplayer[dificulty].upflag==1) // saltando (subiendo)
    {
	mplayer[dificulty].framer = 3;
	mplayer[dificulty].frame = 1;
    }
    if(mplayer[dificulty].downflag==1 && mplayer[dificulty].infloor == 0 && mplayer[dificulty].duck== 0) // cayendo o saltando (bajando)
    {
	mplayer[dificulty].framer = 3;
	mplayer[dificulty].frame  = 2;
    }
    if(mplayer[dificulty].frame > mplayer[dificulty].frames)
	mplayer[dificulty].frame=1;
}
