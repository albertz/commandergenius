/*
 *  openal.cpp
 *  PIGE-OpenAL
 *
 *  Created by Chad Armstrong on Mon Jul 29 2002.
 *  Copyright (c) 2002 Edenwaith. All rights reserved.
 *
 *  Remember to add these frameworks: GLUT, OpenAL, OpenGL
 *  Otherwise, Undefined symbols: errors will result
 *
 *  Several prebind errors will occur, but I was still able to compile
 *  and run the program.
 *
 *  A VERY important step to get this to work is to copy the .wav files into
 *  the <app name>.app/Contents/MacOS folder.  You need to use Terminal to do
 *  this since the Finder windows interpret the <app name>.app as an executable
 *  file instead.
 */

#include <stdio.h>
#include <SDL/SDL.h>
#include <AL/alut.h>

//  function prototypes ---------------------------------------------
void init();

// highly evil global variables ---------------------------------------------
#define NUM_BUFFERS 3
#define NUM_SOURCES 3
#define NUM_ENVIRONMENTS 1

ALfloat listenerPos[]={0.0,0.0,4.0};
ALfloat listenerVel[]={0.0,0.0,0.0};
ALfloat	listenerOri[]={0.0,0.0,1.0, 0.0,1.0,0.0};

ALfloat source0Pos[]={ -2.0, 0.0, 0.0};
ALfloat source0Vel[]={ 0.0, 0.0, 0.0};


ALfloat source1Pos[]={ 2.0, 0.0, 0.0};
ALfloat source1Vel[]={ 0.0, 0.0, 0.0};

ALfloat source2Pos[]={ 0.0, 0.0, -4.0};
ALfloat source2Vel[]={ 0.0, 0.0, 0.0};

ALuint	buffer[NUM_BUFFERS];
ALuint	source[NUM_SOURCES];
ALuint  environment[NUM_ENVIRONMENTS];

ALsizei size,freq;
ALenum 	format;
ALvoid 	*data;
int 	ch;

int main(int argc, char** argv) //finaly the main function
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Surface	*screen;
	screen = SDL_SetVideoMode(320, 200, 16, 0);

	atexit(SDL_Quit);

	init();

	for(int i=0; i < 3; i++)
		alSourcePlay(source[i]);
	int color = 0;
	while(1)
	{
		SDL_FillRect(screen, NULL, color);
		color += 5;

		SDL_Flip(SDL_GetVideoSurface());
		SDL_Delay(50);

		SDL_Event evt;
		while( SDL_PollEvent(&evt) )
		{
			if(evt.type == SDL_KEYDOWN)
			{
				Uint8 *keys = SDL_GetKeyState(NULL);
				if(keys[SDLK_ESCAPE])
					return 0;
			}
		}
	}

	return 0;
}

// ===================================================================
// void init()
// ===================================================================
void init(void)
{
    alutInit(0, NULL);

    alListenerfv(AL_POSITION,listenerPos);
    alListenerfv(AL_VELOCITY,listenerVel);
    alListenerfv(AL_ORIENTATION,listenerOri);
    
    alGetError(); // clear any error messages
    
    if(alGetError() != AL_NO_ERROR) 
    {
        printf("- Error creating buffers !!\n");
        exit(1);
    }
    else
    {
        printf("init() - No errors yet.");
    }
    
    // Generate buffers, or else no sound will happen!
    alGenBuffers(NUM_BUFFERS, buffer);
    
    alutLoadWAVFile((ALbyte*)"c.wav",&format,&data,&size,&freq,NULL);
    alBufferData(buffer[0],format,data,size,freq);
    alutUnloadWAV(format,data,size,freq);

    alutLoadWAVFile((ALbyte*)"b.wav",&format,&data,&size,&freq,NULL);
    alBufferData(buffer[1],format,data,size,freq);
    alutUnloadWAV(format,data,size,freq);

    alutLoadWAVFile((ALbyte*)"a.wav",&format,&data,&size,&freq,NULL);
    alBufferData(buffer[2],format,data,size,freq);
    alutUnloadWAV(format,data,size,freq);

    alGetError(); /* clear error */
    alGenSources(NUM_SOURCES, source);

    if(alGetError() != AL_NO_ERROR) 
    {
        printf("- Error creating sources !!\n");
        exit(2);
    }
    else
    {
        printf("init - no errors after alGenSources\n");
    }

    alSourcef(source[0],AL_PITCH,1.0f);
    alSourcef(source[0],AL_GAIN,1.0f);
    alSourcefv(source[0],AL_POSITION,source0Pos);
    alSourcefv(source[0],AL_VELOCITY,source0Vel);
    alSourcei(source[0],AL_BUFFER,buffer[0]);
    alSourcei(source[0],AL_LOOPING,AL_TRUE);

    alSourcef(source[1],AL_PITCH,1.0f);
    alSourcef(source[1],AL_GAIN,1.0f);
    alSourcefv(source[1],AL_POSITION,source1Pos);
    alSourcefv(source[1],AL_VELOCITY,source1Vel);
    alSourcei(source[1],AL_BUFFER,buffer[1]);
    alSourcei(source[1],AL_LOOPING,AL_TRUE);

    alSourcef(source[2],AL_PITCH,1.0f);
    alSourcef(source[2],AL_GAIN,1.0f);
    alSourcefv(source[2],AL_POSITION,source2Pos);
    alSourcefv(source[2],AL_VELOCITY,source2Vel);
    alSourcei(source[2],AL_BUFFER,buffer[2]);
    alSourcei(source[2],AL_LOOPING,AL_TRUE);
}




