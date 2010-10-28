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

// -------------------------------------------------
// RUTINAS DE CHEQUEO DE COLISIONES DE LOS MONSTRUOS
// -------------------------------------------------
void check_monster_down(int n)
{
    if((M_y[n] + M_h) > (R_tileside*(R_maxtiles_v-1))) 
    {
	M_direction[n] = 1;
	M_y[n]--;
    }
    if((M_y[n]+M_h)/R_tileside >= int(M_y[n]+M_h)/R_tileside)
    {
        for(int m=0;m<=M_w-1;m++)
        {
            if(mroom.data[int(M_x[n]+m)/R_tileside][int((M_y[n]+M_h)/R_tileside)] >= 40)
	    {
		M_direction[n] = 1;
		M_y[n]--;
	    }
        }
    }
    // Chequeo de colision contra otro monstruo
    for(int m=0;m<=M_max4room-1;m++)
    {
	if(n!=m && M_type[m]!=0)
	{
            if(M_y[n] < M_y[m]+M_h && M_y[n]+M_h > M_y[m])
        	if(M_x[n] < M_x[m]+M_w && M_x[n]+M_w > M_x[m])
		{
		    M_direction[n] = 1;
		    M_y[n]-=3;
		}
	}
    }
}

void check_monster_up(int n)
{
    if(M_y[n] < R_tileside)
    {
	M_direction[n] = 0;
	M_y[n]++;
    }
//    if((M_y[n])/R_tileside <= int(M_y[n])/R_tileside)
//    {
        for(int m=0;m<=M_w-1;m++)
        {
            if(mroom.data[int(M_x[n]+m)/R_tileside][int((M_y[n])/R_tileside)] >= 40)
	    {
		M_direction[n] = 0;
		M_y[n]++;
	    }
        }
//    }
    // Chequeo de colision contra otro monstruo
    for(int m=0;m<=M_max4room-1;m++)
    {
	if(m!=n && M_type[m]!=0)
	{
            if(M_y[n] < M_y[m]+M_h && M_y[n]+M_h > M_y[m])
        	if(M_x[n] < M_x[m]+M_w && M_x[n]+M_w > M_x[m])
		{
		    M_direction[n] = 0;
		    M_y[n]+=3;
		}
	}
    }

}

void check_monster_left(int n)
{
    if(M_x[n] < R_tileside)
    {
	M_direction[n] = 0;
	M_x[n]++;
    }
    if((M_x[n])/R_tileside >= int(M_x[n])/R_tileside)
    {
        for(int m=0;m<=M_h-1;m++)
        {
            if(mroom.data[int(M_x[n])/R_tileside][int((M_y[n]+m)/R_tileside)] >= 40)
	    {
		M_direction[n] = 0;
		M_x[n]++;
	    }
        }
    }
    // Chequeo de colision contra otro monstruo
    for(int m=0;m<=M_max4room-1;m++)
    {
	if(m!=n && M_type[m]!=0)
	{
            if(M_y[n] < M_y[m]+M_h && M_y[n]+M_h > M_y[m])
        	if(M_x[n] < M_x[m]+M_w && M_x[n]+M_w > M_x[m])
		{
		    M_direction[n] = 0;
		    M_x[n]+=3;
		}
	}
    }
}

void check_monster_right(int n)
{
    if((M_x[n] + M_w) > (R_tileside*(R_maxtiles_h-1)))
    {
	M_direction[n] = 1;
	M_x[n]--;
    }
    if((M_x[n]+M_w)/R_tileside >= int(M_x[n]+M_w)/R_tileside)
    {
        for(int m=0;m<=M_h-1;m++)
        {
            if(mroom.data[int(M_x[n]+M_w)/R_tileside][int((M_y[n]+m)/R_tileside)] >= 40)
	    {
		M_direction[n] = 1;
		M_x[n]--;
	    }
        }
    }
    // Chequeo de colision contra otro monstruo
    for(int m=0;m<=M_max4room-1;m++)
    {
	if(m!=n && M_type[m]!=0)
	{
            if(M_y[n] < M_y[m]+M_h && M_y[n]+M_h > M_y[m])
        	if(M_x[n] < M_x[m]+M_w && M_x[n]+M_w > M_x[m])
		{
		    M_direction[n] = 1;
		    M_x[n]-=3;
		}
	}
    }
}

void move_monster(int n)
{
    if(M_type[n] > 12) // los monstruos que sean tipo mayor a 12 se mueven en el eje Y y los menores de mueven en el eje X
    {
	if(M_direction[n] == 1)
	{
	    M_y[n]=M_y[n]-(M_speed[dificulty]*imove);
	    check_monster_up(n); //up
	}
	if(M_direction[n] == 0)
	{
	    M_y[n]=M_y[n]+(M_speed[dificulty]*imove);
	    check_monster_down(n); //down
	}
    }
    else
    {
	if(M_direction[n] == 1)
	{
	    M_x[n]=M_x[n]-(M_speed[dificulty]*imove);
	    check_monster_left(n); // left
	}
	if(M_direction[n] == 0)
	{
	    M_x[n]=M_x[n]+(M_speed[dificulty]*imove);
	    check_monster_right(n); //right
	}
    }
}

void print_monsters()
{
    for(int n=0;n<=M_max4room-1;n++)
    {
	if(M_type[n] != 0)
	{
	    oldmonsterpos.x = int(M_x[n]);
	    oldmonsterpos.y = int(M_y[n]);
	    oldmonsterpos.h = M_h;
	    oldmonsterpos.w = M_w;
	    oldmonsterposreal.x = int(M_x[n]+R_gamearea_x);
	    oldmonsterposreal.y = int(M_y[n]+R_gamearea_y);

	    monstersrc.y = M_type[n] * M_h;
	    monstersrc.x = ((M_frame - 1)+(M_direction[n]*M_frames)) * M_w;
	    monstersrc.h = M_h;
	    monstersrc.w = M_w;
	    move_monster(n);
	
	    monsterpos.x = int(M_x[n]);
	    monsterpos.y = int(M_y[n]);
	    monsterposreal.x = monsterpos.x + R_gamearea_x;
	    monsterposreal.y = monsterpos.y + R_gamearea_y;

	    //SDL_BlitSurface(screenbak, &oldmonsterpos, screen, &oldmonsterposreal);
	    SDL_BlitSurface(monsters, &monstersrc, screen, &monsterposreal);
	    //SDL_UpdateRect(screen,oldmonsterposreal.x,oldmonsterposreal.y,M_w,M_h);
	    //SDL_UpdateRect(screen,monsterposreal.x,monsterposreal.y,M_w,M_h);
	}
    }
}
