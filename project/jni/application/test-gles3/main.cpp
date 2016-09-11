#include <string>
#include <vector>
#include <SDL.h>
#include <SDL_image.h>

#include "esUtil.h"

#ifdef __ANDROID__
#include <android/log.h>
#define printf(...) __android_log_print(ANDROID_LOG_INFO, "Test-GLES3", __VA_ARGS__)
#endif

int screenWidth  = 0;
int screenHeight = 0;

GLuint drawTextureProgram = 0;
GLint drawTextureSamplerLocation = 0;

static void initSDL()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK);
	SDL_EnableUNICODE(1);

	screenWidth = SDL_GetVideoInfo()->current_w;
	screenHeight = SDL_GetVideoInfo()->current_h;

	if ( ! SDL_SetVideoMode(screenWidth, screenHeight, 24, SDL_OPENGL|SDL_DOUBLEBUF|SDL_FULLSCREEN) )
	{
		printf("Couldn't set GL video mode: %s\n", SDL_GetError());
		SDL_Quit();
		exit(2);
	}
	SDL_WM_SetCaption("test", "test");
}

void loadShaders()
{
	char vShaderStr[] =
		"#version 300 es                            \n"
		"precision mediump float;                   \n"
		"layout(location = 0) in vec4 a_position;   \n"
		"layout(location = 1) in vec2 a_texCoord;   \n"
		"out vec2 v_texCoord;                       \n"
		"void main()                                \n"
		"{                                          \n"
		"	gl_Position = a_position;               \n"
		"	v_texCoord = a_texCoord;                \n"
		"}                                          \n";

	char fShaderStr[] =
		"#version 300 es                                     \n"
		"precision mediump float;                            \n"
		"in vec2 v_texCoord;                                 \n"
		"layout(location = 0) out vec4 outColor;             \n"
		"uniform sampler2D s_texture;                        \n"
		"void main()                                         \n"
		"{                                                   \n"
		"	outColor = texture( s_texture, v_texCoord );     \n"
		"}                                                   \n";

	// Load the shaders and get a linked program object
	drawTextureProgram = esLoadProgram ( vShaderStr, fShaderStr );

	// Get the sampler location
	drawTextureSamplerLocation = glGetUniformLocation ( drawTextureProgram, "s_texture" );
}

static void initGL()
{
	glViewport(0, 0, screenWidth, screenHeight);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glActiveTexture ( GL_TEXTURE0 );

	loadShaders();
}

static void clearScreen()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); // Always clear your scene before rendering, unless you're sure that you'll fill whole screen with textures/models etc
}

struct Sprite {

	Sprite(const char * path)
	{
		w = h = texture = 0;
		imagePath = path;
		loadTexture();
	}

	bool loadTexture()
	{
		SDL_Surface *pic;

		pic = IMG_Load(imagePath.c_str());

		if (!pic)
		{
			printf("Error: image %s cannot be loaded\n", imagePath.c_str());
			return false;
		}
		if (pic->format->BitsPerPixel != 32 && pic->format->BitsPerPixel != 24)
		{
			printf("Error: image %s is %dbpp - it should be either 24bpp or 32bpp, images with palette are not supported\n", imagePath.c_str(), pic->format->BitsPerPixel);
			SDL_FreeSurface(pic);
			return false;
		}

		GLenum glFormat = (pic->format->BitsPerPixel == 32 ? GL_RGBA : GL_RGB);
		w = pic->w;
		h = pic->h;
		// GLES3 always supports non-power-of-two textures, no need to recalculate texture dimensions

		glGenTextures(1, &texture);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, glFormat, w, h, 0, glFormat, GL_UNSIGNED_BYTE, pic->pixels);
		SDL_FreeSurface(pic);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		disableRepeat();

		return true;
	}

	void disableRepeat()
	{
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	void enableRepeat()
	{
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	void enableMirroredRepeat()
	{
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	}

	void draw(GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLfloat tex_x1 = 0.0f, GLfloat tex_y1 = 0.0f, GLfloat tex_x2 = 1.0f, GLfloat tex_y2 = 1.0f)
	{
		if (texture == 0)
			return;

		glUseProgram( drawTextureProgram );

		GLfloat coords[] = {
							x, y,                         // Position 0
							tex_x1, tex_y1,               // TexCoord 0
							x, y + width,                 // Position 1
							tex_x1, tex_y2,               // TexCoord 1
							x + height, y + width,        // Position 2
							tex_x2, tex_y2,               // TexCoord 2
							x + height, y,                // Position 3
							tex_x2, tex_y1                // TexCoord 3
		};

		// Load the vertex position
		glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof ( GLfloat ), coords );
		// Load the texture coordinate
		glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof ( GLfloat ), &coords[2] );

		glEnableVertexAttribArray( 0 );
		glEnableVertexAttribArray( 1 );

		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, texture );

		glUniform1i ( drawTextureSamplerLocation, 0 );

		glDrawArrays ( GL_TRIANGLE_FAN, 0, 4 );
	}

	void draw(GLfloat x, GLfloat y)
	{
		draw(x, y, w, h);
	}

	GLuint texture;
	int w, h;
	std::string imagePath;
};

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
		sprites.push_back(Sprite("stars.jpg"));
		sprites.back().enableMirroredRepeat(); // Looks nice

		int coords[][2] = { {0, 0},
							{200, 0},
							{300, 200},
							{400, 300} };

		float pulse = 1.0f, pulseChange = 0.01f;
		int anim = 1;

		while ( ! SDL_GetKeyState(NULL)[SDLK_ESCAPE] ) // Exit by pressing Back button
		{
			// clearScreen(); // Do not clear screen, we will fill the screens with tiled background image instead
			sprites[5].draw(0, 0, screenWidth, screenHeight, 0, 0, (float) screenWidth / sprites[5].w, (float) screenHeight / sprites[5].h);

			int mouseX = 0, mouseY = 0, buttons = 0;
			buttons = SDL_GetMouseState(&mouseX, &mouseY);
			sprites[0].draw(mouseX - sprites[0].w/2, mouseY - sprites[0].h/2);

			sprites[1].draw(coords[0][0], coords[0][1]);
			sprites[2].draw(coords[1][0], coords[1][1], sprites[2].w * pulse * 4, sprites[2].h * pulse * 4);
			sprites[3].draw(coords[2][0], coords[2][1], sprites[3].w * pulse * 4, sprites[3].h * 2);
			sprites[4].draw(coords[3][0], coords[3][1], sprites[4].w, sprites[4].h * pulse * 2);

			SDL_GL_SwapBuffers();
			SDL_Event event;
			while( SDL_PollEvent(&event) )
			{
				if(event.type == SDL_VIDEORESIZE)
				{
					// GL context was destroyed - reload all textures
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
			if(pulse >= 1.0f)
				pulseChange = -0.01f;
			if(pulse <= 0)
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
