struct Cobject
{
    int type;
    int x;
    int y;
    int seq;
    int stage;
};

struct Cplayer
{
    float x, y; // posiciones x, y
    float old_x, old_y; // posiciones anteriores de x y
    int w, h; // anchura y altura
    int speed; // velocidad 
    int realspeed; // copia de velocidad, se usa para restaurar el valor de speed en caso que haya sido modificado
    int fallspeed; // velocidad de caida libre
    int frames; // cantidad de frames de animacion
    int frame, framer; // frame actual y fila de frames actual (las filas son: quieto, corriendo, agachado, saltando, cayendo)
    int left, right, jump, duck;
    int start_x, start_y; // posiciones iniciales en la pantalla
    int lives; // vidas del jugador
    int tolerance; // tolerancia de la deteccion de colisiones contra los monstruos
    int objects; // objetos recogidos en la pantalla actual

    int startjump;
    int jumpcounter; // contador de altura del selto
    int maxjump; // altura del salto
    int realmaxjump; // copia de altura del salto en caso que haya que restaurarlo

    int dead; // inficador si esta muerto
    int stageup; // indicador si la pantalla esta completa
    int upflag, downflag;
    int infloor;
    int facingleft;
    int ducked;
    int jumpslower;
};

struct Cstages
{
    char * title;
    char * music;
    int  objects;
    int  time;
    char * code;
};

struct Croom
{
    int data[R_maxtiles_h][R_maxtiles_v]; // los datos (tiles) de la habitacion
    int stage; // a que stage pertenece
};
Croom mroom;
Cobject mobject[2000]; // almacena los objetos del juego
Cplayer mplayer[4];
Cstages mstage[NUM_STAGES];
