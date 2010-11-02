// --- Fonts
enum { FONT_CHUNKS = 2};
SDL_Surface *font;
SDL_Surface *scorefont;
SDL_Surface *scorefont1;
SDL_Surface *scorefont2;
SDL_Surface *menufont[FONT_CHUNKS];
SDL_Surface *menufont1[FONT_CHUNKS];

// --- Screen
SDL_Surface *screen;
SDL_Surface *background;
SDL_Surface *backs;
SDL_Surface *currentBack = NULL;
SDL_Surface *tiles;
SDL_Surface *screenbak;

// --- FX
SDL_Surface *bright;
SDL_Surface *creditsbuffer;
SDL_Surface *creditsbuffer1;
SDL_Surface *creditsfont;
SDL_Surface *left;
SDL_Surface *right;

// --- Monsters
SDL_Surface *monsters[M_max4room];
SDL_Surface *monstersSW = NULL;

// --- Player
SDL_Surface *player;

// --- Intro
SDL_Surface *dragontech;
SDL_Surface *line1;
SDL_Surface *line2;
SDL_Surface *comic_01;
SDL_Surface *black;
