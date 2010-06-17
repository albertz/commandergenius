/*
* 3-D gear wheels.  This program is in the public domain.
*
* Command line options:
*    -info      print GL implementation information
*    -exit      automatically exit after 30 seconds
*
*
* Brian Paul
*/

/* Conversion to use vertex buffer objects by Michael J. Clark */

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL.h>
#ifdef ANDROID
#include <GLES/gl.h>
#include <android/log.h>
#else
#include <GL/gl.h>
#define glFrustumf glFrustum
#endif


#define MAX_CONFIGS 10
#define MAX_MODES 100


#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN 1
  #include <windows.h>

  static double
  current_time(void)
  {
     DWORD tv;

     tv = GetTickCount();
     return (tv/1000.0);
  }
 
#else
  #include <sys/time.h>
  #include <unistd.h>

  /* return current time (in seconds) */
  static double
  current_time(void)
  {
     struct timeval tv;
  #ifdef __VMS
     (void) gettimeofday(&tv, NULL );
  #else
     struct timezone tz;
     (void) gettimeofday(&tv, &tz);
  #endif
     return (double) tv.tv_sec + tv.tv_usec / 1000000.0;
  }
#endif


#ifndef M_PI
#define M_PI 3.14159265
#endif


static GLint autoexit = 0;
static GLfloat viewDist = 40.0;

typedef struct {
  GLfloat pos[3];
  GLfloat norm[3];
} vertex_t;

typedef struct {
  vertex_t *vertices;
  GLshort *indices;
  GLfloat color[4];
  int nvertices, nindices;
  GLuint ibo;
} gear_t;

static gear_t *red_gear;
static gear_t *green_gear;
static gear_t *blue_gear;

/**

  Draw a gear wheel.  You'll probably want to call this function when
  building a display list since we do a lot of trig here.

  Input:  inner_radius - radius of hole at center
          outer_radius - radius at center of teeth
          width - width of gear
          teeth - number of teeth
          tooth_depth - depth of tooth

**/

static gear_t*
gear(GLfloat inner_radius, GLfloat outer_radius, GLfloat width,
     GLint teeth, GLfloat tooth_depth, GLfloat color[])
{
  GLint i, j;
  GLfloat r0, r1, r2;
  GLfloat ta, da;
  GLfloat u1, v1, u2, v2, len;
  GLfloat cos_ta, cos_ta_1da, cos_ta_2da, cos_ta_3da, cos_ta_4da;
  GLfloat sin_ta, sin_ta_1da, sin_ta_2da, sin_ta_3da, sin_ta_4da;
  GLshort ix0, ix1, ix2, ix3, ix4, ix5;
  vertex_t *vt, *nm;
  GLshort *ix;

  gear_t *gear = calloc(1, sizeof(gear_t));
  gear->nvertices = teeth * 40;
  gear->nindices = teeth * 66 * 3;
  gear->vertices = calloc(gear->nvertices, sizeof(vertex_t));
  gear->indices = calloc(gear->nindices, sizeof(GLshort));
  memcpy(&gear->color[0], &color[0], sizeof(GLfloat) * 4);

  r0 = inner_radius;
  r1 = outer_radius - tooth_depth / 2.0;
  r2 = outer_radius + tooth_depth / 2.0;
  da = 2.0 * M_PI / teeth / 4.0;

  vt = gear->vertices;
  nm = gear->vertices;
  ix = gear->indices;

#define VERTEX(x,y,z) ((vt->pos[0] = x),(vt->pos[1] = y),(vt->pos[2] = z), \
                       (vt++ - gear->vertices))
#define NORMAL(x,y,z) ((nm->norm[0] = x),(nm->norm[1] = y),(nm->norm[2] = z), \
                       (nm++ - gear->vertices))
#define INDEX(a,b,c) ((*ix++ = a),(*ix++ = b),(*ix++ = c))

  for (i = 0; i < teeth; i++) {
    ta = i * 2.0 * M_PI / teeth;

    cos_ta = cos(ta);
    cos_ta_1da = cos(ta + da);
    cos_ta_2da = cos(ta + 2 * da);
    cos_ta_3da = cos(ta + 3 * da);
    cos_ta_4da = cos(ta + 4 * da);
    sin_ta = sin(ta);
    sin_ta_1da = sin(ta + da);
    sin_ta_2da = sin(ta + 2 * da);
    sin_ta_3da = sin(ta + 3 * da);
    sin_ta_4da = sin(ta + 4 * da);
               
    u1 = r2 * cos_ta_1da - r1 * cos_ta;
    v1 = r2 * sin_ta_1da - r1 * sin_ta;
    len = sqrt(u1 * u1 + v1 * v1);
    u1 /= len;
    v1 /= len;
    u2 = r1 * cos_ta_3da - r2 * cos_ta_2da;
    v2 = r1 * sin_ta_3da - r2 * sin_ta_2da;

    /* front face */
    ix0 = VERTEX(r0 * cos_ta,          r0 * sin_ta,          width * 0.5);
    ix1 = VERTEX(r1 * cos_ta,          r1 * sin_ta,          width * 0.5);
    ix2 = VERTEX(r0 * cos_ta,          r0 * sin_ta,          width * 0.5);
    ix3 = VERTEX(r1 * cos_ta_3da,      r1 * sin_ta_3da,      width * 0.5);
    ix4 = VERTEX(r0 * cos_ta_4da,      r0 * sin_ta_4da,      width * 0.5);
    ix5 = VERTEX(r1 * cos_ta_4da,      r1 * sin_ta_4da,      width * 0.5);
    for (j = 0; j < 6; j++) {
      NORMAL(0.0,                  0.0,                  1.0);
    }
    INDEX(ix0, ix1, ix2);
    INDEX(ix1, ix3, ix2);
    INDEX(ix2, ix3, ix4);
    INDEX(ix3, ix5, ix4);
               
    /* front sides of teeth */
    ix0 = VERTEX(r1 * cos_ta,          r1 * sin_ta,          width * 0.5);
    ix1 = VERTEX(r2 * cos_ta_1da,      r2 * sin_ta_1da,      width * 0.5);
    ix2 = VERTEX(r1 * cos_ta_3da,      r1 * sin_ta_3da,      width * 0.5);
    ix3 = VERTEX(r2 * cos_ta_2da,      r2 * sin_ta_2da,      width * 0.5);
    for (j = 0; j < 4; j++) {
      NORMAL(0.0,                  0.0,                  1.0);
    }
    INDEX(ix0, ix1, ix2);
    INDEX(ix1, ix3, ix2);

    /* back face */
    ix0 = VERTEX(r1 * cos_ta,          r1 * sin_ta,          -width * 0.5);
    ix1 = VERTEX(r0 * cos_ta,          r0 * sin_ta,          -width * 0.5);
    ix2 = VERTEX(r1 * cos_ta_3da,      r1 * sin_ta_3da,      -width * 0.5);
    ix3 = VERTEX(r0 * cos_ta,          r0 * sin_ta,          -width * 0.5);
    ix4 = VERTEX(r1 * cos_ta_4da,      r1 * sin_ta_4da,      -width * 0.5);
    ix5 = VERTEX(r0 * cos_ta_4da,      r0 * sin_ta_4da,      -width * 0.5);
    for (j = 0; j < 6; j++) {
      NORMAL(0.0,                  0.0,                  -1.0);
    }
    INDEX(ix0, ix1, ix2);
    INDEX(ix1, ix3, ix2);
    INDEX(ix2, ix3, ix4);
    INDEX(ix3, ix5, ix4);
               
    /* back sides of teeth */
    ix0 = VERTEX(r1 * cos_ta_3da,      r1 * sin_ta_3da,      -width * 0.5);
    ix1 = VERTEX(r2 * cos_ta_2da,      r2 * sin_ta_2da,      -width * 0.5);
    ix2 = VERTEX(r1 * cos_ta,          r1 * sin_ta,          -width * 0.5);
    ix3 = VERTEX(r2 * cos_ta_1da,      r2 * sin_ta_1da,      -width * 0.5);
    for (j = 0; j < 4; j++) {
      NORMAL(0.0,                  0.0,                  -1.0);
    }
    INDEX(ix0, ix1, ix2);
    INDEX(ix1, ix3, ix2);
               
    /* draw outward faces of teeth */
    ix0 = VERTEX(r1 * cos_ta,          r1 * sin_ta,          width * 0.5);
    ix1 = VERTEX(r1 * cos_ta,          r1 * sin_ta,          -width * 0.5);
    ix2 = VERTEX(r2 * cos_ta_1da,      r2 * sin_ta_1da,      width * 0.5);
    ix3 = VERTEX(r2 * cos_ta_1da,      r2 * sin_ta_1da,      -width * 0.5);
    for (j = 0; j < 4; j++) {
      NORMAL(v1,                   -u1,                  0.0);
    }
    INDEX(ix0, ix1, ix2);
    INDEX(ix1, ix3, ix2);
    ix0 = VERTEX(r2 * cos_ta_1da,      r2 * sin_ta_1da,      width * 0.5);
    ix1 = VERTEX(r2 * cos_ta_1da,      r2 * sin_ta_1da,      -width * 0.5);
    ix2 = VERTEX(r2 * cos_ta_2da,      r2 * sin_ta_2da,      width * 0.5);
    ix3 = VERTEX(r2 * cos_ta_2da,      r2 * sin_ta_2da,      -width * 0.5);
    for (j = 0; j < 4; j++) {
      NORMAL(cos_ta,               sin_ta,               0.0);
    }
    INDEX(ix0, ix1, ix2);
    INDEX(ix1, ix3, ix2);
    ix0 = VERTEX(r2 * cos_ta_2da,      r2 * sin_ta_2da,      width * 0.5);
    ix1 = VERTEX(r2 * cos_ta_2da,      r2 * sin_ta_2da,      -width * 0.5);
    ix2 = VERTEX(r1 * cos_ta_3da,      r1 * sin_ta_3da,      width * 0.5);
    ix3 = VERTEX(r1 * cos_ta_3da,      r1 * sin_ta_3da,      -width * 0.5);
    for (j = 0; j < 4; j++) {
      NORMAL(v2,                   -u2,                  0.0);
    }
    INDEX(ix0, ix1, ix2);
    INDEX(ix1, ix3, ix2);
    ix0 = VERTEX(r1 * cos_ta_3da,      r1 * sin_ta_3da,      width * 0.5);
    ix1 = VERTEX(r1 * cos_ta_3da,      r1 * sin_ta_3da,      -width * 0.5);
    ix2 = VERTEX(r1 * cos_ta_4da,      r1 * sin_ta_4da,      width * 0.5);
    ix3 = VERTEX(r1 * cos_ta_4da,      r1 * sin_ta_4da,      -width * 0.5);
    for (j = 0; j < 4; j++) {
      NORMAL(cos_ta,               sin_ta,               0.0);
    }
    INDEX(ix0, ix1, ix2);
    INDEX(ix1, ix3, ix2);

    /* draw inside radius cylinder */
    ix0 = VERTEX(r0 * cos_ta,          r0 * sin_ta,          -width * 0.5);
    ix1 = VERTEX(r0 * cos_ta,          r0 * sin_ta,          width * 0.5);
    ix2 = VERTEX(r0 * cos_ta_4da,      r0 * sin_ta_4da,      -width * 0.5);
    ix3 = VERTEX(r0 * cos_ta_4da,      r0 * sin_ta_4da,      width * 0.5);
    NORMAL(-cos_ta,              -sin_ta,              0.0);
    NORMAL(-cos_ta,              -sin_ta,              0.0);
    NORMAL(-cos_ta_4da,          -sin_ta_4da,          0.0);
    NORMAL(-cos_ta_4da,          -sin_ta_4da,          0.0);
    INDEX(ix0, ix1, ix2);
    INDEX(ix1, ix3, ix2);
  }

  return gear;
}


void draw_gear(gear_t* gear) {

  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, gear->color);
  glVertexPointer(3, GL_FLOAT, sizeof(vertex_t), gear->vertices[0].pos);
  glNormalPointer(GL_FLOAT, sizeof(vertex_t), gear->vertices[0].norm);
  glDrawElements(GL_TRIANGLES, gear->nindices/3, GL_UNSIGNED_SHORT,
                   gear->indices);
}

static GLfloat view_rotx = 20.0, view_roty = 30.0, view_rotz = 0.0;
static gear_t *gear1, *gear2, *gear3;
static GLfloat angle = 0.0;

static void
draw(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glPushMatrix();

    glTranslatef(0.0, 0.0, -viewDist);

    glRotatef(view_rotx, 1.0, 0.0, 0.0);
    glRotatef(view_roty, 0.0, 1.0, 0.0);
    glRotatef(view_rotz, 0.0, 0.0, 1.0);

    glPushMatrix();
      glTranslatef(-3.0, -2.0, 0.0);
      glRotatef(angle, 0.0, 0.0, 1.0);
      draw_gear(gear1);
    glPopMatrix();

    glPushMatrix();
      glTranslatef(3.1, -2.0, 0.0);
      glRotatef(-2.0 * angle - 9.0, 0.0, 0.0, 1.0);
      draw_gear(gear2);
    glPopMatrix();

    glPushMatrix();
      glTranslatef(-3.1, 4.2, 0.0);
      glRotatef(-2.0 * angle - 25.0, 0.0, 0.0, 1.0);
      draw_gear(gear3);
    glPopMatrix();

  glPopMatrix();
  //glFinish();
  SDL_GL_SwapBuffers();
}


/* new window size or exposure */
static void
reshape(int width, int height)
{
  GLfloat h = (GLfloat) height / (GLfloat) width;

  glViewport(0, 0, (GLint) width, (GLint) height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glFrustumf(-1.0, 1.0, -h, h, 5.0, 200.0);
  glMatrixMode(GL_MODELVIEW);
}


static void
init(int argc, char *argv[])
{
  static GLfloat pos[4] = {5.0, 5.0, 10.0, 0.0};
  static GLfloat red[4] = {0.8, 0.1, 0.0, 1.0};
  static GLfloat green[4] = {0.0, 0.8, 0.2, 1.0};
  static GLfloat blue[4] = {0.2, 0.2, 1.0, 1.0};
  GLint i;

  glLightfv(GL_LIGHT0, GL_POSITION, pos);
  glEnable(GL_CULL_FACE);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_DEPTH_TEST);

  glShadeModel(GL_SMOOTH);

  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);

  for ( i=1; i<argc; i++ ) {
    if (strcmp(argv[i], "-info")==0) {
      printf("GL_RENDERER   = %s\n", (char *) glGetString(GL_RENDERER));
      printf("GL_VERSION    = %s\n", (char *) glGetString(GL_VERSION));
      printf("GL_VENDOR     = %s\n", (char *) glGetString(GL_VENDOR));
      printf("GL_EXTENSIONS = %s\n", (char *) glGetString(GL_EXTENSIONS));
    }
    else if ( strcmp(argv[i], "-exit")==0) {
      autoexit = 30;
      printf("Auto Exit after %i seconds.\n", autoexit );
    }
  }

  /* make the gears */
  gear1 = gear(1.0, 4.0, 1.0, 20, 0.7, red);
  gear2 = gear(0.5, 2.0, 2.0, 10, 0.7, green);
  gear3 = gear(1.3, 2.0, 0.5, 10, 0.7, blue);
}

static void run_gears()
{
  static double st = 0;
  static double ct = 0;
  static double seconds = 0;
  static double fps;
  static int    frames = 0;

  if( st == 0 )
  {
    st = ct = seconds = current_time();
  }

  {
    double tt = current_time();
    double dt = tt - ct;
    ct = tt;
   
    /* advance rotation for next frame */
    angle += 70.0 * dt;  /* 70 degrees per second */
    if (angle > 3600.0)
          angle -= 3600.0;       
    draw();
    
    
    frames++;
    dt = ct - seconds;
    if (dt >= 5.0)
   {
     fps = frames / dt;
#ifdef ANDROID
     __android_log_print(ANDROID_LOG_INFO, "GLXgears", "%d frames in %3.1f seconds = %6.3f FPS", frames, dt, fps);
#endif
     printf("%d frames in %3.1f seconds = %6.3f FPS\n", frames, dt, fps);
     seconds = ct;
     frames = 0;
    }
  }       
}

int
main(int argc, char *argv[])
{
  int width  = 480;
  int height = 320;

  SDL_Surface *screen;
  int done;
  Uint8 *keys;

  SDL_Init(SDL_INIT_VIDEO);

  screen = SDL_SetVideoMode(width, height, 16, SDL_OPENGL|SDL_RESIZABLE);
  if ( ! screen ) {
    fprintf(stderr, "Couldn't set GL video mode: %s\n", SDL_GetError());
    SDL_Quit();
    exit(2);
  }
  SDL_WM_SetCaption("Gears", "gears");

  init(argc, argv);
  reshape(screen->w, screen->h);
  done = 0;
  while ( ! done ) {
    SDL_Event event;
    while ( SDL_PollEvent(&event) ) {
      switch(event.type) {
        case SDL_VIDEORESIZE:
          screen = SDL_SetVideoMode(event.resize.w, event.resize.h, 16,
                                    SDL_OPENGL|SDL_RESIZABLE);
          if ( screen ) {
            reshape(screen->w, screen->h);
          } else {
            /* Uh oh, we couldn't set the new video mode?? */;
          }
          break;

        case SDL_QUIT:
          done = 1;
          break;
      }
    }
    keys = SDL_GetKeyboardState(NULL);

    if ( keys[SDL_SCANCODE_ESCAPE] ) {
      done = 1;
    }

    run_gears();
  }

  SDL_Quit();
  return 0;
}
