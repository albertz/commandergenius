void stop_music();
void credits_events();
void menu_events();
void setback();
void fadesurface(SDL_Surface *surface, int x, int y, int speed);

void screen_mode();
void setgame();
void start_game();

void load_menufont();
void unload_menufont();

void do_code();
void do_hiscores();
void load_room();
void print_room();
void init_room();

void init_objects();
void initfade_object(int x, int y, int object);
float fade_object(float alpha);

double delta_time();

void init_monsters();
void print_monsters();

void chk_touched(int x, int y);
void print_player();
void animateplayer();
void respawn();

void print_timer();
void stage_up();

void flushevents();
void screen_fx();
void bright_obj(int bright_x, int bright_y);
void check_joystick_events(SDL_Event *event, Uint8 *joypos);
void game_loop();
void show_arrow(int arrow, int show);

void win_game();
