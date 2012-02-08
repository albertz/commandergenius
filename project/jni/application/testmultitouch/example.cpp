#include <android/log.h>
#include "SDL.h"


int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK);
    SDL_Surface * screen = SDL_SetVideoMode(640, 480, 16, 0);

    enum { MAX_POINTERS = 16 };
    // some random colors
    int colors[MAX_POINTERS] = { 0xaaaaaa, 0xffffff, 0x888888, 0xcccccc, 0x666666, 0x999999, 0xdddddd, 0xeeeeee, 0xaaaaaa, 0xffffff, 0x888888, 0xcccccc, 0x666666, 0x999999, 0xdddddd, 0xeeeeee };
    struct TouchPointer_t { int x; int y; int pressure; int pressed; } touchPointers[MAX_POINTERS];
    int accel[3]; // Only first 2 coords are used
    int i;

    memset(touchPointers, 0, sizeof(touchPointers));
    memset(accel, 0, sizeof(accel));
    SDL_Joystick * joystick = SDL_JoystickOpen(0);

    while(1)
    {
        SDL_Event evt;
        while( SDL_PollEvent(&evt) )
        {
            if( evt.type == SDL_JOYAXISMOTION )
            {
                if(evt.jaxis.axis < 3)
                    accel[evt.jaxis.axis] = evt.jaxis.value; // Those events are spammy, don't log them
                else
                {
                    __android_log_print(ANDROID_LOG_INFO, "Multitouch", "SDL_JOYAXISMOTION %d value %06d", (int)evt.jaxis.axis, (int)evt.jaxis.value );
                    touchPointers[evt.jaxis.axis-3].pressure = evt.jaxis.value;
                }
            }
            if( evt.type == SDL_JOYBUTTONDOWN || evt.type == SDL_JOYBUTTONUP )
            {
                __android_log_print(ANDROID_LOG_INFO, "Multitouch", "SDL_JOYBUTTON %d value %d", (int)evt.jbutton.button, (int)evt.jbutton.state );
                touchPointers[evt.jbutton.button].pressed = (evt.jbutton.state == SDL_PRESSED);
            }
            if( evt.type == SDL_JOYBALLMOTION )
            {
                __android_log_print(ANDROID_LOG_INFO, "Multitouch", "SDL_JOYBALLMOTION %d %06d:%06d", (int)evt.jball.ball, (int)evt.jball.xrel, (int)evt.jball.yrel );
                touchPointers[evt.jball.ball].x = evt.jball.xrel;
                touchPointers[evt.jball.ball].y = evt.jball.yrel;
            }
            if(evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_ESCAPE)
                return 0;
        }
        SDL_FillRect(screen, 0, 0);
        SDL_Rect r;
        for(i=0; i<MAX_POINTERS; i++)
        {
            if( !touchPointers[i].pressed )
                continue;
            r.x = touchPointers[i].x;
            r.y = touchPointers[i].y;
            r.w = 50 + touchPointers[i].pressure / 5;
            r.h = 50 + touchPointers[i].pressure / 5;
            r.x -= r.w/2;
            r.y -= r.h/2;
            SDL_FillRect(screen, &r, colors[i]);
        }
        r.x = 320 + accel[0] / 100;
        r.y = 240 + accel[1] / 100;
        r.w = 10;
        r.h = 10;
        r.x -= r.w/2;
        r.y -= r.h/2;
        SDL_FillRect(screen, &r, 0xffffff);
        SDL_Flip(screen);
    }
    return 0;
}
