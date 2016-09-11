#include <string>
#include <vector>
#include <SDL.h>
#include <SDL_image.h>

#ifdef ANDROID
#include <GLES/gl.h>
#define glOrtho glOrthof

#include <android/log.h>
#define fprintf(X, ...) __android_log_print(ANDROID_LOG_INFO, "Ballfield", __VA_ARGS__)
#define printf(...) __android_log_print(ANDROID_LOG_INFO, "Ballfield", __VA_ARGS__)
#else
#include <GL/gl.h>
#endif

int screenWidth	 = 0;
int screenHeight = 0;

struct Sprite {

	Sprite(const char * path)
	{
		w = h = texcoord_w = texcoord_h = texture = 0;
		imagePath = path;
		loadTexture();
	}

	bool loadTexture()
	{
		SDL_Surface *pic;

		pic = IMG_Load(imagePath.c_str());

		if(!pic)
		{
		printf("Error: image %s cannot be loaded\n", imagePath.c_str());
		return false;
		}
		if(pic->format->BitsPerPixel != 32 && pic->format->BitsPerPixel != 24)
		{
		printf("Error: image %s is %dbpp - it should be either 24bpp or 32bpp, images with palette are not supported\n", imagePath.c_str(), pic->format->BitsPerPixel);
		SDL_FreeSurface(pic);
		return false;
		}

		GLenum glFormat = (pic->format->BitsPerPixel == 32 ? GL_RGBA : GL_RGB);
		w = pic->w;
		h = pic->h;
		// All OpenGL textures must have size which is power of 2, such as 128, 256, 512 etc
		int upload_w = powerOfTwo(w);
		int upload_h = powerOfTwo(h);
		texcoord_w = (float) w / (float) upload_w;
		texcoord_h = (float) h / (float) upload_h;

		glGenTextures(1, &texture);

		glBindTexture(GL_TEXTURE_2D, texture);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, glFormat, upload_w, upload_h, 0, glFormat, GL_UNSIGNED_BYTE, NULL);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, glFormat, GL_UNSIGNED_BYTE, pic->pixels);

		SDL_FreeSurface(pic);

		return true;
	}

	int powerOfTwo(int i)
	{
		int r;
		for (r = 1; r < i; r *= 2) {}
		return r;
	}

	void draw(GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLfloat r = 1.0f, GLfloat g = 1.0f, GLfloat b = 1.0f, GLfloat a = 1.0f)
	{
		if(texture == 0)
		return;
		// GL coordinates start at bottom-left corner, which is counter-intuitive for sprite graphics, so we have to flip Y coordinate
		GLfloat textureCoordinates[] = { 0.0f, 0.0f,
										 0.0f, texcoord_h,
										 texcoord_w, texcoord_h,
										 texcoord_w, 0.0f };
		GLfloat vertices[] = { x, screenHeight - y,
							 x, screenHeight - (y + height),
							 x + width, screenHeight - (y + height),
							 x + width, screenHeight - y };
		glColor4f(r, g, b, a);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texture);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, vertices);
		glTexCoordPointer(2, GL_FLOAT, 0, textureCoordinates);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4); // You may just replace all your GL_QUADS with GL_TRIANGLE_FAN and it will draw absolutely identically with same coordinates, if you have just 4 coords.
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	void draw(GLfloat x, GLfloat y)
	{
		draw(x, y, w, h);
	}

	GLuint texture;
	int w, h;
	GLfloat texcoord_w, texcoord_h; // Fix for textures with non-power-of-2 size
	std::string imagePath;
};

static void
initGL()
{

		glViewport(0, 0, screenWidth, screenHeight);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		glOrtho(0.0f, screenWidth, 0.0f, screenHeight, -1.0f, 1.0f);
		glMatrixMode(GL_MODELVIEW);

		 glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		 glDisable(GL_DEPTH_TEST);
}

static void
clearScreen()
{
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); // Always clear your scene before rendering, unless you're sure that you'll fill whole screen with textures/models etc

		// You have to do this each frame, because SDL messes up with your GL context when drawing on-screen keyboard, however is saves/restores your matrices
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}


static void initSDL()
{
		SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK);
		SDL_EnableUNICODE(1);

		screenWidth = SDL_GetVideoInfo()->current_w;
		screenHeight = SDL_GetVideoInfo()->current_h;

		if ( ! SDL_SetVideoMode(screenWidth, screenHeight, 24, SDL_OPENGL|SDL_DOUBLEBUF|SDL_FULLSCREEN) )
		{
		 fprintf(stderr, "Couldn't set GL video mode: %s\n", SDL_GetError());
		 SDL_Quit();
		 exit(2);
		}
		SDL_WM_SetCaption("test", "test");
}

int
main(int argc, char *argv[])
{
		initSDL();
		initGL();

		std::vector<Sprite> sprites;
		sprites.push_back(Sprite("test.png"));
		sprites.push_back(Sprite("element0.png"));
		sprites.push_back(Sprite("element1.png"));
		sprites.push_back(Sprite("element2.png"));
		sprites.push_back(Sprite("element3.png"));

		int coords[][2] = { {0, 0},
							{200, 0},
							{300, 200},
							{400, 300} };

		float pulse = 1.0f, pulseChange = 0.01f;
		int anim = 1;

		while ( ! SDL_GetKeyState(NULL)[SDLK_ESCAPE] ) // Exit by pressing Back button
		{
			clearScreen();
			int mouseX = 0, mouseY = 0, buttons = 0;
			buttons = SDL_GetMouseState(&mouseX, &mouseY);

			sprites[1].draw(coords[0][0], coords[0][1]);
			sprites[2].draw(coords[1][0], coords[1][1], sprites[2].w * pulse * 4, sprites[2].h * pulse * 4);
			sprites[3].draw(coords[2][0], coords[2][1], sprites[3].w * pulse * 4, sprites[3].h * 2);
			sprites[4].draw(coords[3][0], coords[3][1], sprites[4].w, sprites[4].h * pulse * 2);
			sprites[2].draw(coords[1][0] + 300, coords[1][1] + sprites[2].h * pulse * 3, sprites[2].w * 3, sprites[2].h * 3, pulse, pulse, 1.0f - pulse );

			sprites[0].draw(mouseX - sprites[0].w/2, mouseY - sprites[0].h/2, sprites[0].w, sprites[0].h, buttons ? 0 : 1, 1, 1, pulse);

			SDL_GL_SwapBuffers();
			SDL_Event event;
			while( SDL_PollEvent(&event) )
			{
				if(event.type == SDL_VIDEORESIZE)
				{
				// Reload textures to OpenGL
				initGL();
				for(int i = 0; i < sprites.size(); i++)
					sprites[i].loadTexture();
				}
				if(event.type == SDL_KEYUP || event.type == SDL_KEYDOWN)
				{
					__android_log_print(ANDROID_LOG_INFO, "Hello", "SDL key event: evt %s state %s key %4d %12s scancode %4d mod %2d unicode %d", event.type == SDL_KEYUP ? "UP	 " : "DOWN" , event.key.state == SDL_PRESSED ? "PRESSED " : "RELEASED", (int)event.key.keysym.sym, SDL_GetKeyName(event.key.keysym.sym), (int)event.key.keysym.scancode, (int)event.key.keysym.mod, (int)event.key.keysym.unicode);
					if(event.key.keysym.sym == SDLK_ESCAPE)
						return 0;
				}
			}

			// Some kinda animation

			pulse += pulseChange;
			if(pulse > 1.0f)
				pulseChange = -0.01f;
			if(pulse < 0)
				pulseChange = 0.01f;

			for(int i = 0; i < 4; i++)
			{
				coords[i][0] += anim;
				coords[i][1] += anim;
			}
			if( coords[0][0] < 0 )
				anim = 1;
			if( coords[3][1] > screenHeight )
				anim = -1;
		}

		SDL_Quit();
		return 0;
}
