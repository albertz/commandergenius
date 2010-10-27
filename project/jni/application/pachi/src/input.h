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

void credits_events()
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
	check_joystick_events(&event, joypos);
        if(event.type==SDL_KEYDOWN)
            if(event.key.keysym.sym==SDLK_ESCAPE || event.key.keysym.sym==SDLK_RETURN || event.key.keysym.sym==SDLK_SPACE)
                escape_exit=1;
	if(joypos[JOY_BUTTON0])
	    escape_exit=1;
    }
}

void pause_events()
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        if(event.type==SDL_KEYDOWN)
            if(event.key.keysym.sym==SDLK_h || event.key.keysym.sym==SDLK_F1)
                escape_exit=1;
    }
}

void hiscore_events()
{
    int n;
    SDL_Event event;
    
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
	    case SDL_KEYDOWN:
	    {
		if(event.key.keysym.sym>41)
		{
		    playername[namechar] = event.key.keysym.sym;
		    if(playername[namechar]>=97 && playername[namechar]<=122)
			playername[namechar]=event.key.keysym.sym-32;
		    if(namechar < 9)
			namechar++;
		}
		else
		{
		    if(event.key.keysym.sym==SDLK_RETURN || event.key.keysym.sym == SDLK_ESCAPE)
			inputloop=0;
		    if(event.key.keysym.sym==SDLK_BACKSPACE)
		    {
			namechar=0;
			for(n=0;n<10;n++)
			    playername[n] = '.';
		    }
		}
	    }
	}
	check_joystick_events(&event, joypos);                                  
        if (joypos[JOY_UP])
        {                                                   
            playername[namechar]++;                                         
    	    if(playername[namechar]>=97 && playername[namechar]<=122)       
                playername[namechar]-=32;                               
        }                                                                       
        if (joypos[JOY_DOWN])
        {                                                 
            playername[namechar]--;                                         
            if(playername[namechar]>=97 && playername[namechar]<=122)       
                playername[namechar]-=32;                               
        }                                                                       
        if (joypos[JOY_BUTTON0])
        {
            namechar++;                                                     
            if(namechar > 3)                                                
                namechar=0;                                             
        }                                                                       
        if (joypos[JOY_BUTTON1])
            inputloop=0;                                                    
    }
}

void menu_events()
{
    SDL_Event event;
    Uint8 *keys;
    Mix_Volume(0,100);
    while(SDL_PollEvent(&event)==1)
    {
	if(event.type==SDL_QUIT)
		gameexit=1;

	check_joystick_events(&event, joypos);
	keys = SDL_GetKeyState(NULL);
        if(keys[SDLK_DOWN] || keys[SDLK_KP2] || keys[SDLK_a] || joypos[JOY_DOWN])
	{
	    Mix_PlayChannel(0,option,0);
    	    if(menuvalue == 7)
		menuvalue=1;
	    else
		menuvalue++;
	}
	if(keys[SDLK_UP] || keys[SDLK_KP8] || keys[SDLK_q] || joypos[JOY_UP])
	{
	    Mix_PlayChannel(0,option,0);
	    if(menuvalue == 1)
		menuvalue = 7;
	    else
		menuvalue--;
        }
	if(keys[SDLK_SPACE] || keys[SDLK_RETURN] || joypos[JOY_BUTTON0])
	{
    	    Mix_PlayChannel(0,option,0);

		switch(menuvalue)
		{
		    case 1:
			startgame = 1;
			break;
		    case 2:
			dificulty++;
			if(dificulty>3)
			    dificulty=1;
			break;
		    case 3:
			screen_mode();
                        setback();
			joypos[JOY_BUTTON0]=0;
                        break;
		    case 4:
			do_code();
			break;
		    case 5:
			help=1;
			break;
		    case 6:
			hiscores=1;
			break;
		    case 7:
			gameexit = 1;
			break;
		    default:
			break;
		}
	    }
	    if(keys[SDLK_ESCAPE])
		gameexit=1;
    }
}

void code_events()
{
    int n;
    SDL_Event event;
    
    while(SDL_PollEvent(&event)==1)
    {
        switch(event.type)
        {
            case SDL_KEYDOWN:
	    {
		if(event.key.keysym.sym>41)
		{
	    	    code[codechar] = event.key.keysym.sym;
		    if(code[codechar]>=97 && code[codechar]<=122)
			code[codechar]=event.key.keysym.sym-32;
		    if(codechar < 3)
			codechar++;
		}
		else
		{
		    if(event.key.keysym.sym== SDLK_RETURN || event.key.keysym.sym == SDLK_ESCAPE)
			inputloop=0;
		    if(event.key.keysym.sym== SDLK_BACKSPACE)
		    {
    SDL_Rect coderect;
    coderect.x=303;
    coderect.y=235;
    coderect.w=194;
    coderect.h=44;
    SDL_FillRect(screen, &coderect, SDL_MapRGB(screen->format,0,0,0));


	    		codechar=0;
			for(int n=0;n<4;n++)
    	    		    code[n] = '.'  ;
		    }
		}
	    }
        }
	check_joystick_events(&event, joypos);                                       
        if (joypos[JOY_UP])
        { 
            code[codechar]++;
            if(code[codechar]>=97 && code[codechar]<=122)                        
                code[codechar]-=32;                                          
        }                                                                            
        if (joypos[JOY_DOWN])
        { 
            code[codechar]--;                                                    
            if(code[codechar]>=97 && code[codechar]<=122)                        
                code[codechar]-=32;                                          
        }                                                                            
        if (joypos[JOY_BUTTON0])
        {
            codechar++;                                                          
            if(codechar > 3)                                                     
                codechar=0;                                                  
        }                                                                            
        if (joypos[JOY_BUTTON1])
            inputloop=0;                                                         
    }
}

void check_joystick_events(SDL_Event *event, Uint8 *joypos)                          
{                                                                                    
    switch (event->type)                                                          
    {                                                                          
	case SDL_JOYAXISMOTION:  /* Handle Joystick Motion */                     
	    if( event->jaxis.axis == 0)                                          
	    { /* Left-Right movement code goes here */                           
	        if ( event->jaxis.value < -3200) 
		{
		    joypos[JOY_LEFT]=1;                                  
		    joypos[JOY_RIGHT]=0;                                 
		}                                                            
		else
		{
		    if ( event->jaxis.value > 3200)
		    {
		        joypos[JOY_LEFT]=0;                          
		        joypos[JOY_RIGHT]=1;                         
		    }                                                    
		    else
		    {
		        joypos[JOY_LEFT]=0;                          
		        joypos[JOY_RIGHT]=0;                         
		    }                                                    
		}                                                            
	    }                                                                    
            if( event->jaxis.axis == 1)                                          
	    { /* Up-Down movement code goes here */                              
	        if ( event->jaxis.value < -3200)
		{
	    	    joypos[JOY_UP]=1;                                    
		    joypos[JOY_DOWN]=0;                                  
		}                                                            
		else
		{
		    if ( event->jaxis.value > 3200)
		    {
			joypos[JOY_UP]=0;                            
		        joypos[JOY_DOWN]=1;                          
		    }                                                    
		    else
		    {                                               
		        joypos[JOY_UP]=0;                            
		        joypos[JOY_DOWN]=0;                          
		    }
		}
	    }                                                                    
        break;                                                                  
	case SDL_JOYBUTTONDOWN:  /* Handle Joystick Button Presses */           
	    switch (event->jbutton.button)
    	    {                                  
		case 0:                                                   
		    joypos[JOY_BUTTON0]=1;									                                 break;                                                    
        	case 1:                                                   
        	    joypos[JOY_BUTTON1]=1;                            
		break;                                                    
	    }                                                                 
	break;                                                                  
	case SDL_JOYBUTTONUP:  /* Handle Joystick Button Releases */            
	    switch (event->jbutton.button)
	    {
		case 0:                                                   
	    	    joypos[JOY_BUTTON0]=0;                            
		break;                                                    
		case 1:                                                   
		    joypos[JOY_BUTTON1]=0;                            
		break;                                                    
	    }                                                                 
	break;                                                            
    }                                                                          
}                                                                                    


void game_events()
{
    int n;
    SDL_Event event;
    Uint8 *keys;
    
    if(mplayer[dificulty].lives==0)
        game_exit = 1;

	
    while(SDL_PollEvent(&event)==1)
    {
	check_joystick_events(&event, joypos);
	keys = SDL_GetKeyState(NULL);

        mplayer[dificulty].left = 0;
        mplayer[dificulty].right = 0;

	if(keys[SDLK_h] || keys[SDLK_F1])
	{
	    escape_exit=0;
	    SDL_Rect dst;
		dst.x=300; dst.y=200; dst.w=192; dst.h=32;
	    SDL_Rect src;
		src.x=dst.x-R_gamearea_x; src.y=dst.y-R_gamearea_y; src.w=dst.w; src.h=dst.h;
	    print_text(menufont, screen, 32, 32, dst.x, dst.y, "PAUSED");
	    SDL_UpdateRect(screen, dst.x, dst.y , dst.w, dst.h);
	    while(escape_exit==0)
	    {
	    	pause_events();
		SDL_Delay(5);
	    }
	    SDL_BlitSurface(screenbak, &src, screen, &dst);
	    SDL_UpdateRect(screen, dst.x, dst.y , dst.w, dst.w);
	}
	if(keys[SDLK_o] || keys[SDLK_LEFT] || keys[SDLK_KP4] || joypos[JOY_LEFT])
	{
	    mplayer[dificulty].left = 1;
	    mplayer[dificulty].right = 0;
    	    mplayer[dificulty].facingleft = 1;
	}
        if(keys[SDLK_p] || keys[SDLK_RIGHT] || keys[SDLK_KP6] || joypos[JOY_RIGHT])
	{
    	    mplayer[dificulty].right = 1;
    	    mplayer[dificulty].left = 0;
    	    mplayer[dificulty].facingleft = 0;
	}
	if(keys[SDLK_q] || keys[SDLK_UP] || keys[SDLK_KP8] || joypos[JOY_UP])
	{
	    if(mplayer[dificulty].duck==1) // en esta rutina tiene que comprobar que si se quiere parar que no tenga nada encima que se lo impida
	    {
		mplayer[dificulty].duck = 0;
    		mplayer[dificulty].framer = 1;
		mplayer[dificulty].y = mplayer[dificulty].old_y - (P_h - P_h/2);
		for(int n=0;n<=mplayer[dificulty].w-1;n++)
    		{
		    int x=int(mplayer[dificulty].x+n)/R_tileside;
		    int y=int((mplayer[dificulty].y - (P_h - P_h/2)+R_tileside)/R_tileside);
		    chk_touched(x,y);
		    if((mroom.data[x][y] >= 80) && (mroom.data[x][y] < 200))
		    {
			mplayer[dificulty].duck = 1;
			mplayer[dificulty].framer = 2;
			mplayer[dificulty].y = mplayer[dificulty].old_y;
		    }
		}
	    }
	}
	if(keys[SDLK_a] || keys[SDLK_DOWN] || keys[SDLK_KP2] || joypos[JOY_DOWN])
	{
	    if(mplayer[dificulty].duck==0 && mplayer[dificulty].jump == 0 && mplayer[dificulty].infloor == 1)
	    {
		mplayer[dificulty].y = mplayer[dificulty].y + (P_h - P_h/2);
		mplayer[dificulty].ducked = 1;
	    }
	    if(mplayer[dificulty].jump == 0 && mplayer[dificulty].infloor == 1)
		mplayer[dificulty].duck = 1;
	}
	if(keys[SDLK_SPACE] || keys[SDLK_KP0] || joypos[JOY_BUTTON0])
	{
    	    if((mplayer[dificulty].infloor==1)&&(mplayer[dificulty].duck==0)&&(mplayer[dificulty].y+P_h/R_tileside==int(mplayer[dificulty].y+P_h/R_tileside)))
    	    {
		mplayer[dificulty].jumpcounter=0;
		mplayer[dificulty].startjump=int(mplayer[dificulty].y);
		mplayer[dificulty].upflag=1;
		mplayer[dificulty].downflag=0;
    		mplayer[dificulty].jump = 1;
		Mix_Volume(0,100);
		Mix_PlayChannel(0,jump,0);
	    }
	}
	if(keys[SDLK_ESCAPE])
	{
	    game_exit = 1;
	}
    }
}
