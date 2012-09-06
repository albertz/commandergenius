#include <SDL.h>
#include <SDL_image.h>
#include <GLES/gl.h>

GLuint texture;

// I've tweaked coordinates a bit to my taste
GLfloat textureCoordinates[] = { 0.0f, 1.0f,
                                 0.0f, 0.0f,
                                 1.0f, 0.0f,
                                 1.0f, 1.0f };
GLfloat vertices[] = {
       100, 100,
       100, 300,
       300, 300,
       300, 100 };

int loadImage() {
      SDL_Surface *pic;
      SDL_Surface *tmp;

      glGenTextures(1, &texture);

      glBindTexture(GL_TEXTURE_2D, texture);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);


      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

      pic = IMG_Load("gradient.png");

      // check power of 2
      if (!pic || !pic->w || !pic->h || (pic->w & 1) || (pic->h & 1)) {

        return -1;
      }
      //uncomment for 32bit  
      tmp = SDL_CreateRGBSurface(0, pic->w, pic->h, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000  );
      SDL_BlitSurface(pic, NULL, tmp, NULL);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pic->w, pic->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp->pixels);
      
      /*
      //uncomment for 16bit  
      tmp = SDL_CreateRGBSurface(0, pic->w, pic->h, 16, 0xF800, 0x7E0, 0x1F, 0); // Correct bits of RGB565 color format
      SDL_BlitSurface(pic, NULL, tmp, NULL);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pic->w, pic->h, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, tmp->pixels);
      */

      SDL_FreeSurface(pic);
      SDL_FreeSurface(tmp);

      return 0;
}


int width  = 640;
int height = 480;


static void
initGL()
{

	 glViewport(0, 0, width, height);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		glOrthof(0.0f, width, 0.0f, height, -1.0f, 1.0f);
		glMatrixMode(GL_MODELVIEW);

	   glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	   glDisable(GL_DEPTH_TEST);
       glEnable(GL_TEXTURE_2D);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

}

static void
draw()
{
      glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); // Always clear your scene before rendering, unless you're sure that you'll fill whole screen with textures/models etc

      // You have to this each frame, because SDL messes up with your GL context when drawing on-screen keyboard, however is taves/restores your matrices
      glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // Just to be sure
      glEnable(GL_TEXTURE_2D);
      glDisable(GL_BLEND);
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

      glBindTexture(GL_TEXTURE_2D, texture);
      glEnableClientState(GL_VERTEX_ARRAY);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glVertexPointer(2, GL_FLOAT, 0, vertices);
      glTexCoordPointer(2, GL_FLOAT, 0, textureCoordinates);
      glDrawArrays(GL_TRIANGLE_FAN, 0, 4); // You may just replace all your GL_QUADS with GL_TRIANGLE_FAN and it will draw absolutely identically with same coordinates, if you have just 4 coords.
      glDisableClientState(GL_VERTEX_ARRAY);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}




int
main(int argc, char *argv[])
{

      SDL_Surface *screen;
      int done;
      Uint8 *keys;

      SDL_Init(SDL_INIT_VIDEO);

      // Set native device screen resolution, if you want 640x480 select SdlVideoResize=y in AndroidAppSettings.cfg
      width = SDL_ListModes(NULL, 0)[0]->w;
      height = SDL_ListModes(NULL, 0)[0]->h;

      //32bit video mode
      screen = SDL_SetVideoMode(width, height, 16, SDL_OPENGL|SDL_DOUBLEBUF);

      if ( ! screen ) {
       fprintf(stderr, "Couldn't set GL video mode: %s\n", SDL_GetError());
       SDL_Quit();
       exit(2);
      }
      SDL_WM_SetCaption("test", "test");

      initGL();
      loadImage();
      while ( ! SDL_GetKeyState(NULL)[SDLK_ESCAPE] ) // Exit by pressing Back button
      {
        draw();
        SDL_GL_SwapBuffers();
        SDL_Event event;
        while( SDL_PollEvent(&event) ) ; // Do nothing, just process keyboard/mouse events internally
        // Some kinda animation
        SDL_Delay(10000);
        static float diff = 1.0f;
        vertices[0] += diff;
        vertices[2] += diff;
        vertices[4] += diff;
        vertices[6] += diff;
        if( vertices[4] > width )
            diff = -1.0f;
        if( vertices[0] < 0 )
            diff = 1.0f;
      }

      SDL_Quit();
      return 0;
}
