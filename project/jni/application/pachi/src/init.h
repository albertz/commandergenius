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

void initsdl()
{

    int have_joystick=0;    

    if(SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO|SDL_INIT_JOYSTICK) < 0)
    {
        fprintf(stderr,"Could not Initialize SDL.\nError: %s\n", SDL_GetError());
        exit (1);
    }
    if(Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 2048) < 0)
    {
        fprintf(stderr,"Warning: Couldn't set 44100 Hz 16-bit audio\n: %s\n", SDL_GetError());
    }
    SDL_WM_SetIcon(SDL_LoadBMP(DATADIR"/Tgfx/icon.bmp"),NULL);

    fullscreen=0;
    screen = SDL_SetVideoMode(screen_w,screen_h,screen_bpp, SDL_HWSURFACE|SDL_DOUBLEBUF /* SDL_SWSURFACE */ );
    if (screen == NULL)
    {
        fprintf(stderr, "Can't set the video mode. Quitting.\nError; %s\n", SDL_GetError());
        exit (1);
    }

    have_joystick = SDL_NumJoysticks();
//    fprintf(stderr, "%i joysticks were found.\n", have_joystick );
    if (have_joystick)
    {                                                        
	SDL_JoystickEventState(SDL_ENABLE);                                  
	joystick = SDL_JoystickOpen(0);                                      
    }                                                                           

    SDL_ShowCursor(0);
    SDL_WM_GrabInput(SDL_GRAB_ON);
    SDL_WM_GrabInput(SDL_GRAB_OFF);
    SDL_WM_SetCaption("Pachi el marciano", "Pachi el marciano");
}
