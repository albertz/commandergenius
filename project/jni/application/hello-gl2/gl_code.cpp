/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// OpenGL ES 2.0 code

#include <SDL.h>
#include <android/log.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define  LOG_TAG    "libgl2jni"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static void printGLString(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    LOGI("GL %s = %s\n", name, v);
}

static void checkGlError(const char* op) {
    for (GLint error = glGetError(); error; error
            = glGetError()) {
        LOGI("after %s() glError (0x%x)\n", op, error);
    }
}

static const char gVertexShader[] = 
    "attribute vec4 vPosition;\n"
    "void main() {\n"
    "  gl_Position = vPosition;\n"
    "}\n";

static const char gFragmentShader[] = 
    "precision mediump float;\n"
    "void main() {\n"
    "  gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
    "}\n";

typedef GLuint (*tglCreateShader)      (GLenum type);
tglCreateShader xglCreateShader;
typedef void   (*tglDeleteShader)      (GLuint shader);
tglDeleteShader xglDeleteShader;
typedef void   (*tglShaderSource)      (GLuint shader, GLsizei count, const char** string, const GLint* length);
tglShaderSource xglShaderSource;
typedef void   (*tglCompileShader)     (GLuint shader);
tglCompileShader xglCompileShader;
typedef void   (*tglGetShaderiv)       (GLuint shader, GLenum pname, GLint* params);
tglGetShaderiv xglGetShaderiv;
typedef void   (*tglGetShaderInfoLog)  (GLuint shader, GLsizei bufsize, GLsizei* length, char* infolog);
tglGetShaderInfoLog xglGetShaderInfoLog;
typedef GLuint (*tglCreateProgram)     (void);
tglCreateProgram xglCreateProgram;
typedef void   (*tglDeleteProgram)     (GLuint program);
tglDeleteProgram xglDeleteProgram;
typedef void   (*tglAttachShader)      (GLuint program, GLuint shader);
tglAttachShader xglAttachShader;
typedef void   (*tglLinkProgram)       (GLuint program);
tglLinkProgram xglLinkProgram;
typedef void   (*tglGetProgramiv)      (GLuint program, GLenum pname, GLint* params);
tglGetProgramiv xglGetProgramiv;
typedef void   (*tglGetProgramInfoLog) (GLuint program, GLsizei bufsize, GLsizei* length, char* infolog);
tglGetProgramInfoLog xglGetProgramInfoLog;
typedef int    (*tglGetAttribLocation) (GLuint program, const char* name);
tglGetAttribLocation xglGetAttribLocation;
typedef void   (*tglUseProgram)        (GLuint program);
tglUseProgram xglUseProgram;
typedef void   (*tglVertexAttribPointer) (GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* ptr);
tglVertexAttribPointer xglVertexAttribPointer;
typedef void   (*tglEnableVertexAttribArray) (GLuint index);
tglEnableVertexAttribArray xglEnableVertexAttribArray;
typedef void   (*tglClearColor) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
tglClearColor xglClearColor;

void loadFunctions(void)
{
	SDL_GL_LoadLibrary(NULL);
	#define GL_LOAD_PROC(name) GL_LOAD_PROC1(name)
	#define GL_LOAD_PROC1(name) \
		x ## name = (t ## name) SDL_GL_GetProcAddress( # name ); \
		if( x ## name == NULL ) \
			LOGE("Error loading GL function: %s", # name );

	GL_LOAD_PROC(glCreateShader);
	GL_LOAD_PROC(glDeleteShader);
	GL_LOAD_PROC(glShaderSource);
	GL_LOAD_PROC(glCompileShader);
	GL_LOAD_PROC(glGetShaderiv);
	GL_LOAD_PROC(glGetShaderInfoLog);
	GL_LOAD_PROC(glCreateProgram);
	GL_LOAD_PROC(glDeleteProgram);
	GL_LOAD_PROC(glAttachShader);
	GL_LOAD_PROC(glLinkProgram);
	GL_LOAD_PROC(glGetProgramiv);
	GL_LOAD_PROC(glGetProgramInfoLog);
	GL_LOAD_PROC(glGetAttribLocation);
	GL_LOAD_PROC(glUseProgram);
	GL_LOAD_PROC(glVertexAttribPointer);
	GL_LOAD_PROC(glEnableVertexAttribArray);
	GL_LOAD_PROC(glClearColor);
}

GLuint loadShader(GLenum shaderType, const char* pSource) {
    GLuint shader = xglCreateShader(shaderType);
    checkGlError("glCreateShader");
    if (shader) {
        xglShaderSource(shader, 1, &pSource, NULL);
        xglCompileShader(shader);
        GLint compiled = 0;
        xglGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            xglGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = (char*) malloc(infoLen);
                if (buf) {
                    xglGetShaderInfoLog(shader, infoLen, NULL, buf);
                    LOGE("Could not compile shader %d:\n%s\n",
                            shaderType, buf);
                    free(buf);
                }
                xglDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

GLuint createProgram(const char* pVertexSource, const char* pFragmentSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader) {
        return 0;
    }

    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) {
        return 0;
    }

    GLuint program = xglCreateProgram();
    checkGlError("glCreateProgram");
    if (program) {
        xglAttachShader(program, vertexShader);
        checkGlError("glAttachShader");
        xglAttachShader(program, pixelShader);
        checkGlError("glAttachShader");
        xglLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        xglGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            xglGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*) malloc(bufLength);
                if (buf) {
                    xglGetProgramInfoLog(program, bufLength, NULL, buf);
                    LOGE("Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            xglDeleteProgram(program);
            program = 0;
        }
    }
    return program;
}

GLuint gProgram;
GLuint gvPositionHandle;

bool setupGraphics(int w, int h) {
    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);
    loadFunctions();

    LOGI("setupGraphics(%d, %d)", w, h);
    gProgram = createProgram(gVertexShader, gFragmentShader);
    if (!gProgram) {
        LOGE("Could not create program.");
        return false;
    }
    gvPositionHandle = xglGetAttribLocation(gProgram, "vPosition");
    checkGlError("glGetAttribLocation");
    LOGI("glGetAttribLocation(\"vPosition\") = %d\n",
            gvPositionHandle);

    glViewport(0, 0, w, h);
    checkGlError("glViewport");
    return true;
}

const GLfloat gTriangleVertices[] = { 0.0f, 0.5f, -0.5f, -0.5f,
        0.5f, -0.5f };

void renderFrame() {
    static float grey;
    grey += 0.01f;
    if (grey > 1.0f) {
        grey = 0.0f;
    }
    xglClearColor(grey, grey, grey, 1.0f);
    checkGlError("glClearColor");
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    checkGlError("glClear");

    xglUseProgram(gProgram);
    checkGlError("glUseProgram");

    xglVertexAttribPointer(gvPositionHandle, 2, GL_FLOAT, GL_FALSE, 0, gTriangleVertices);
    checkGlError("glVertexAttribPointer");
    xglEnableVertexAttribArray(gvPositionHandle);
    checkGlError("glEnableVertexAttribArray");
    glDrawArrays(GL_TRIANGLES, 0, 3);
    checkGlError("glDrawArrays");
}

int SDL_main(int argc, char ** argv)
{
	int SCREEN_W = 640;
	int SCREEN_H = 480;
	SDL_Init(SDL_INIT_VIDEO);
	SDL_SetVideoMode(SCREEN_W, SCREEN_H, 24, SDL_OPENGL | SDL_DOUBLEBUF);
	setupGraphics(SCREEN_W, SCREEN_H);
	while( ! SDL_GetKeyState(NULL)[SDLK_ESCAPE] )
	{
		SDL_Event evt;
		renderFrame();
		SDL_GL_SwapBuffers();
		while( SDL_PollEvent(&evt) )
		{
		}
	}
}
