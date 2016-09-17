#include <string>
#include <vector>
#include <stdio.h>

#include <GLES3/gl3.h>

#include <SDL.h>
#include <SDL_image.h>

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
}

static GLuint loadShader ( GLenum type, const char *shaderSrc )
{
	GLuint shader;
	GLint compiled;

	// Create the shader object
	shader = glCreateShader ( type );

	if ( shader == 0 )
	{
		return 0;
	}

	// Load the shader source
	glShaderSource ( shader, 1, &shaderSrc, NULL );

	// Compile the shader
	glCompileShader ( shader );

	// Check the compile status
	glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );

	if ( !compiled )
	{
		GLint infoLen = 0;

		glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );

		if ( infoLen > 1 )
		{
			char *infoLog = (char *) malloc ( sizeof ( char ) * infoLen );

			glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
			printf ( "===========> Error compiling shader:\n");
			printf ( "===========> %s\n", infoLog );

			free ( infoLog );
		}

		glDeleteShader ( shader );
		return 0;
	}

	return shader;

}

static GLuint loadProgram ( const char *vertShaderSrc, const char *fragShaderSrc )
{
	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint programObject;
	GLint linked;

	// Load the vertex/fragment shaders
	vertexShader = loadShader ( GL_VERTEX_SHADER, vertShaderSrc );

	if ( vertexShader == 0 )
	{
		return 0;
	}

	fragmentShader = loadShader ( GL_FRAGMENT_SHADER, fragShaderSrc );

	if ( fragmentShader == 0 )
	{
		glDeleteShader ( vertexShader );
		return 0;
	}

	// Create the program object
	programObject = glCreateProgram ( );

	if ( programObject == 0 )
	{
		return 0;
	}

	glAttachShader ( programObject, vertexShader );
	glAttachShader ( programObject, fragmentShader );

	// Link the program
	glLinkProgram ( programObject );

	// Check the link status
	glGetProgramiv ( programObject, GL_LINK_STATUS, &linked );

	if ( !linked )
	{
		GLint infoLen = 0;

		glGetProgramiv ( programObject, GL_INFO_LOG_LENGTH, &infoLen );

		if ( infoLen > 1 )
		{
			char *infoLog = (char *) malloc ( sizeof ( char ) * infoLen );

			glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );
			printf ( "===========> Error linking program:\n" );
			printf ( "===========> %s\n", infoLog );

			free ( infoLog );
		}

		glDeleteProgram ( programObject );
		return 0;
	}

	// Free up no longer needed shader resources
	glDeleteShader ( vertexShader );
	glDeleteShader ( fragmentShader );

	return programObject;
}

static void initShaders()
{
	char *vertexShaderStr = NULL;
	asprintf( &vertexShaderStr,
		"#version 300 es                                     \n"
		"const highp float screenWidthDiv = 2.0 / %d.0;      \n"
		"const highp float screenHeightDiv = 2.0 / %d.0;     \n"
		"layout(location = 0) in highp vec2 a_position;      \n"
		"layout(location = 1) in highp vec2 a_texCoord;      \n"
		"layout(location = 2) in lowp vec4 a_color;          \n"
		"out highp vec2 v_texCoord;                          \n"
		"out lowp vec4 v_color;                              \n"
		"void main()                                         \n"
		"{                                                   \n"
		"	gl_Position = vec4(                              \n"
		"		a_position.x * screenWidthDiv - 1.0f,        \n"
		"		1.0f - a_position.y * screenHeightDiv,       \n"
		"		0, 1);                                       \n"
		"	v_texCoord = a_texCoord;                         \n"
		"	v_color = a_color;                               \n"
		"}                                                   \n"
		, screenWidth, screenHeight );

	const char *fragmentShaderStr =
		"#version 300 es                                               \n"
		"in highp vec2 v_texCoord;                                     \n"
		"in lowp vec4 v_color;                                         \n"
		"layout(location = 0) out lowp vec4 outColor;                  \n"
		"uniform sampler2D s_texture;                                  \n"
		"void main()                                                   \n"
		"{                                                             \n"
		"	outColor = texture( s_texture, v_texCoord ) * v_color;     \n"
		"}                                                             \n";

	printf("initShaders(): vertex shader:\n%s\n", vertexShaderStr);
	printf("initShaders(): fragment shader:\n%s\n", fragmentShaderStr);

	// Load the shaders and get a linked program object
	drawTextureProgram = loadProgram ( vertexShaderStr, fragmentShaderStr );
	if (drawTextureProgram == 0)
	{
		printf("Could not init shaders\n");
		SDL_Quit();
		exit(2);
	}

	free(vertexShaderStr);

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

	initShaders();
}

static void clearScreen()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); // Always clear your scene before rendering, unless you're sure that you'll fill whole screen with textures/models etc
}

struct Sprite {


	Sprite(const char * path, GLint wrap = GL_CLAMP_TO_EDGE)
	{
		w = h = texture = 0;
		imagePath = path;
		textureWrap = wrap;
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
		glBindTexture(GL_TEXTURE_2D, texture);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, glFormat, w, h, 0, glFormat, GL_UNSIGNED_BYTE, pic->pixels);
		SDL_FreeSurface(pic);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrap);

		return true;
	}

	void draw(GLfloat x, GLfloat y, GLfloat width, GLfloat height,
	          GLfloat tex_x1 = 0.0f, GLfloat tex_y1 = 0.0f, GLfloat tex_x2 = 1.0f, GLfloat tex_y2 = 1.0f,
	          GLfloat r = 1.0f, GLfloat g = 1.0f, GLfloat b = 1.0f, GLfloat a = 1.0f)
	{
		if (texture == 0)
			return;

		glUseProgram( drawTextureProgram );

		GLfloat coords[] = {
			x, y,                         // Position 0
			tex_x1, tex_y1,               // TexCoord 0
			x, y + height,                // Position 1
			tex_x1, tex_y2,               // TexCoord 1
			x + width, y + height,        // Position 2
			tex_x2, tex_y2,               // TexCoord 2
			x + width, y,                 // Position 3
			tex_x2, tex_y1                // TexCoord 3
		};
		GLfloat color[] = { r, g, b, a, r, g, b, a, r, g, b, a, r, g, b, a };

		// Load the vertex position
		glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof ( GLfloat ), coords );
		// Load the texture coordinate
		glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof ( GLfloat ), &coords[2] );
		glVertexAttribPointer( 2, 4, GL_FLOAT, GL_FALSE, 0, color );

		glEnableVertexAttribArray( 0 );
		glEnableVertexAttribArray( 1 );
		glEnableVertexAttribArray( 2 );

		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, texture );

		glUniform1i ( drawTextureSamplerLocation, 0 );

		glDrawArrays ( GL_TRIANGLE_FAN, 0, 4 );
	}

	void draw(GLfloat x, GLfloat y)
	{
		draw(x, y, w, h);
	}

	void drawColor(GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLfloat r, GLfloat g, GLfloat b, GLfloat a)
	{
		draw(x, y, width, height, 0.0f, 0.0f, 1.0f, 1.0f, r, g, b, a);
	}

	void drawColor(GLfloat x, GLfloat y, GLfloat r, GLfloat g, GLfloat b, GLfloat a)
	{
		drawColor(x, y, w, h, r, g, b, a);
	}

	GLuint texture;
	int w, h;
	std::string imagePath;
	GLint textureWrap;
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
		//sprites.push_back(Sprite("stars.jpg", GL_REPEAT)); // Repeating tiles
		sprites.push_back(Sprite("stars.jpg", GL_MIRRORED_REPEAT)); // Looks nice

		float coords[][2] = { {0, 0},
							{200, 0},
							{300, 200},
							{400, 300} };

		float pulse = 1.0f, pulseChange = 0.01f;
		int anim = 1;

		while ( ! SDL_GetKeyState(NULL)[SDLK_ESCAPE] ) // Exit by pressing Back button
		{
			// clearScreen(); // Do not clear screen, we will fill the screens with tiled background image instead
			sprites[5].draw(0, 0, screenWidth, screenHeight,
							coords[0][0] / sprites[5].w * 3,
							coords[0][0] / sprites[5].h * 2,
							coords[0][0] / sprites[5].w * 3 + (float) screenWidth / sprites[5].w,
							coords[0][0] / sprites[5].h * 2 + (float) screenHeight / sprites[5].h);

			sprites[1].drawColor(coords[0][0], coords[0][1], pulse, pulse, 1.0f - pulse, 1.0f);
			sprites[2].draw(coords[1][0], coords[1][1], sprites[2].w * pulse * 4, sprites[2].h * pulse * 4);
			sprites[3].draw(coords[2][0], coords[2][1], sprites[3].w * pulse * 4, sprites[3].h * 2);
			sprites[4].draw(coords[3][0], coords[3][1], sprites[4].w, sprites[4].h * pulse * 2);

			sprites[1].draw(screenWidth / 2.0f - sprites[1].w / 2.0f, screenHeight / 2.0f - sprites[1].h / 2.0f);
			sprites[1].draw(0 - sprites[1].w / 2.0f, 0 - sprites[1].h / 2.0f);
			sprites[1].draw(screenWidth - sprites[1].w / 2.0f, screenHeight - sprites[1].h / 2.0f);
			sprites[1].draw(0 - sprites[1].w / 2.0f, screenHeight - sprites[1].h / 2.0f);
			sprites[1].draw(screenWidth - sprites[1].w / 2.0f, 0 - sprites[1].h / 2.0f);

			int mouseX = 0, mouseY = 0, buttons = 0;
			buttons = SDL_GetMouseState(&mouseX, &mouseY);
			sprites[0].drawColor(mouseX - sprites[0].w/2, mouseY - sprites[0].h/2, 1.0f, 1.0f, 1.0f, pulse);

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
					printf("SDL key event: evt %s state %s key %4d %12s scancode %4d mod %2d unicode %d", event.type == SDL_KEYUP ? "UP	 " : "DOWN" , event.key.state == SDL_PRESSED ? "PRESSED " : "RELEASED", (int)event.key.keysym.sym, SDL_GetKeyName(event.key.keysym.sym), (int)event.key.keysym.scancode, (int)event.key.keysym.mod, (int)event.key.keysym.unicode);
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
