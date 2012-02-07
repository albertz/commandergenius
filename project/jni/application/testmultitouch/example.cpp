
#include <android/log.h>
#include "SDL.h"


int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK);
    SDL_Surface * screen = SDL_SetVideoMode(640, 480, 16, 0);

    enum { MAX_POINTERS = 16, PTR_PRESSED = 4 };
    int touchPointers[MAX_POINTERS][5];
    int accel[5];
    int i;

    memset(touchPointers, 0, sizeof(touchPointers));
    memset(accel, 0, sizeof(accel));
    SDL_Joystick * joysticks[MAX_POINTERS+1];
    for(i=0; i<MAX_POINTERS; i++)
        joysticks[i] = SDL_JoystickOpen(i);

    while(1)
    {
        SDL_Event evt;
        while( SDL_PollEvent(&evt) )
        {
            if( evt.type == SDL_JOYAXISMOTION )
            {
                if( evt.jaxis.which == 0 ) // 0 = The accelerometer
                {
                    accel[evt.jaxis.axis] = evt.jaxis.value;
                    continue;
                }
                int joyid = evt.jaxis.which - 1;
                touchPointers[joyid][evt.jaxis.axis] = evt.jaxis.value; // Axis 0 and 1 are coordinates, 2 and 3 are pressure and touch point radius
            }
            if( evt.type == SDL_JOYBUTTONDOWN || evt.type == SDL_JOYBUTTONUP )
            {
                if( evt.jbutton.which == 0 ) // 0 = The accelerometer
                    continue;
                int joyid = evt.jbutton.which - 1;
                touchPointers[joyid][PTR_PRESSED] = (evt.jbutton.state == SDL_PRESSED);
            }
            if(evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_ESCAPE)
                return 0;
        }
        SDL_FillRect(screen, 0, 0);
        SDL_Rect r;
        for(i=0; i<MAX_POINTERS; i++)
        {
            if( !touchPointers[i][PTR_PRESSED] )
                continue;
            r.x = touchPointers[i][0];
            r.y = touchPointers[i][1];
            r.w = 80 + touchPointers[i][2] / 10; // Pressure
            r.h = 80 + touchPointers[i][3] / 10; // Touch point size
            r.x -= r.w/2;
            r.y -= r.h/2;
            SDL_FillRect(screen, &r, 0xffffff);
        }
        r.x = 320 + accel[0] / 100;
        r.y = 240 + accel[1] / 100;
        r.w = 10 + abs(accel[2]);
        r.h = 10;
        r.x -= r.w/2;
        r.y -= r.h/2;
        __android_log_print(ANDROID_LOG_INFO, "Example", "Accelerometer %06d %06d %06d", accel[0], accel[1], accel[2]);
        SDL_FillRect(screen, &r, 0xaaaaaa);
        SDL_Flip(screen);
    }
    return 0;
}
