SDL_Joystick * joystick;
enum joynames {JOY_LEFT, JOY_RIGHT, JOY_UP, JOY_DOWN, JOY_BUTTON0, JOY_BUTTON1};
Uint8 joypos[JOY_BUTTON1+1]={0,0,0,0,0,0};
// --- Screen related
#define screen_w 800
#define screen_h 600
#define screen_bpp 16

// --- Room related
#define R_tileside 32
#define R_maxobjects 5
#define R_max_x 40
#define R_max_y 10
#define R_gamearea_x 28
#define R_gamearea_y 15
#define R_maxtiles_h 23
#define R_maxtiles_v 14
#define R_back_x 92
#define R_back_y 112
#define R_maxbacks_h 8
#define R_maxbacks_v 4
int R_current; // habitacion actual
int R_backdata[R_maxbacks_h][R_maxbacks_v]; //esta matriz contiene la informacion sobre el fondo del nivel actual
int R_x; // variable utilizada para ubicar la habitacion actual en la matriz
int R_y; // variable utilizada para ubicar la habitacion actual en la matriz
int R_object;
int roommap[R_max_x][R_max_y]; // mapeado de habitaciones, aca se comprueba que habitaciones pertenecen a cada stage
int chk_exits;

int objects; // cantidad de objetos en la stage actual

int score = 0;
int gameexit, startgame, help, hiscores;

// --- Timing related
double playtime;
double roomtime;

// -- FX related
int brightframe;
int sequence; // variable que almacena cual es el objeto siguiente en la secuencia (para recogerlos por orden)
int fadingobject; // esta variable indica si un objeto es esta desvaneciendo
float alphade;
#define arrow_left  1
#define arrow_right 2
#define arrow_up    3
#define arrow_down  4

int namechar;
int gameover=0;

// --- Scores related
char playername[]="          ";
char scorename[10][10];
int scorestage[10];
int scoretime[10];
int scoredif[10];
int scorescore[10];
int scorepos;

int codechar;
char code[]="....";
int inputloop;

int menuvalue = 1;
int fullscreen = 1;
int dificulty = 2;

// --- Monsters related
#define M_w 48
#define M_h 48
#define M_frames 6
#define M_max4room 10
#define M_easyspeed 130
#define M_normalspeed 150
#define M_hardspeed 200

int M_type[M_max4room];// el 10 es el maximo de monstruos en pantalla
float M_x[M_max4room]; // el 10 es el maximo de monstruos en pantalla
float M_y[M_max4room];// el 10 es el maximo de monstruos en pantalla
float M_old_x[M_max4room];// el 10 es el maximo de monstruos en pantalla
float M_old_y[M_max4room];// el 10 es el maximo de monstruos en pantalla
int M_direction[M_max4room]; // estas variables se usan para saber si el monstruo va hacia un lado o hacia otro
int M_frame = 1;
int monsters_c; // cantidad de monstruos en la pantalla actual
int M_initspeed=M_normalspeed;
int M_speed[4];
SDL_Rect monstersrc;
SDL_Rect monsterpos;
SDL_Rect monsterposreal;
SDL_Rect oldmonsterpos;
SDL_Rect oldmonsterposreal;


// --- Player related
#define maxlives 10
#define P_h 64
#define P_w 48
int respawned=0; // variable que indica cuando el personaje termino de aparecer despues de morirse
SDL_Rect playersrc; // esto  es para leer del bmp del player el frame a imprimir
SDL_Rect playerpos; // esta es la posicion en la pantalla del player (sin contar los bordes del panel)
SDL_Rect playerposreal; // esta es la posicion en la pantalla del player (contando los bordes del panel) se utiliza automaticamente
SDL_Rect oldplayerpos; //esta es la posicion que abandona el player
SDL_Rect oldplayerposreal; //esta es la posicion que abandona el player (contando los bordes del panel)

// --- Timing related
Uint32 lasttime=0;
Uint32 curtime=0;
double imove=0;
double gametimer=0;
int minutes, seconds;
int oldsec; // variable que almacena el valor de los segundos

float animcounter = 0; //contador usado para las animaciones de los enemigos y del personaje

int escape_exit;
int game_exit;

// --- Stage Related
#define NUM_STAGES 16

#define CEMETERY 1
#define CAVES 7
#define CASTLE 10
#define LAB 16
#define END 17

int startstage = 0; // stage inicial
int stage; //stage actual
int stagecompleted = 0; // esta variable es un flag que se pone en 1 cuando se agarran todos los objetos del nivel
