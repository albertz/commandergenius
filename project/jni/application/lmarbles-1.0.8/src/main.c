/***************************************************************************
                          main.c  -  description
                             -------------------
    begin                : Sam Aug  5 12:36:32 MEST 2000
    copyright            : (C) 2000 by Michael Speck
    email                : kulkanie@gmx.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#ifdef _WIN32
#include <fcntl.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "menu.h"
#include "cfg.h"
#include "levels.h"
#include "timer.h"
#include "sdl.h"
#include "audio.h"
#include "profile.h"
#include "game.h"

// timer -- timer.c //
extern struct timeb t_c, t_lc;
// quit game quickly ? //
int     trm_gm = 0;
// menu config -- cfg.c //
extern  Cfg cfg;
// menu manager -- menu.c //
extern  MMng mm;
// sdl struct //
extern Sdl sdl;
// levelset list and number of entries -- levels.c //
extern char **ls_lst;
extern int  ls_n;
// profiles //
extern DLst prfs;
// string list for the menu -- profile.c //
extern char **prf_lst;
extern int  prf_n;
// menu entry containing the profile list //
MEnt *me_prf, *me_del, *me_clr;

// menu actions MUST BE GREATER THAN 0 //
#define MA_NONE 0
#define MA_QUT 1
#define MA_PLY 2
#define MA_HSC 3

/*
====================================================================
Clear the SDL event key (keydown events)
====================================================================
*/
int all_filter( const SDL_Event *event )
{
    return 0;
}
void event_clear_sdl_queue()
{
    SDL_Event event;
    SDL_SetEventFilter( all_filter );
    while ( SDL_PollEvent( &event ) );
    SDL_SetEventFilter( 0 );
}

/*
    previously defined hiscore.h but this file no longer needed
*/
typedef struct {
	char	nm[12];
	int		lvl;
	int     lvs;
	int		scr;
	int     pct;
} H_E;

/*
    show hiscore and highlight rank r
*/
void H_Shw()
{
	int i, j;
    SDL_Surface *buf = 0;
	SFnt *ft;
	Prf *p;
	int e_h = mm.ft_nml->h + 2;
	int off = 50;
	int a_y = 100;
	char str[12];
    int e_num = prfs.cntr > 10 ? 10 : prfs.cntr;
    H_E hscr[e_num];
    DL_E *e;

    // save screen //
    buf = SS_Crt(sdl.scr->w, sdl.scr->h, SDL_SWSURFACE);
    D_FDST(buf);
    D_FSRC(sdl.scr);
    SS_Blt();
    // darken screen //
    if (cfg.dim)
        SDL_DIM();

    // draw background //
    D_FDST(sdl.scr);
    D_FSRC(mm.ss_bkgd);
    SS_Blt();
    // brighten screen //
    if (cfg.dim)
        SDL_UNDIM();
    else
        Sdl_FUpd();

    // create highscore list //
    memset(hscr, 0, sizeof(hscr));
    e = prfs.hd.n;
    while (e != &prfs.tl) {
        p = (Prf*)e->d;
        for (i = 0; i < e_num; i++)
            if (p->scr >= hscr[i].scr) {
                for (j = e_num - 1; j > i; j--)
                    hscr[j] = hscr[j - 1];
                hscr[i].scr = p->scr;
                hscr[i].pct = p->pct;
                hscr[i].lvl = p->lvls;
                strcpy(hscr[i].nm, p->nm);
                break;
            }
        e = e->n;
    }

    // write hiscores //
	
    SF_Wrt(mm.ft_nml, sdl.scr, sdl.scr->w / 2, 50, "Best Profiles", 0);
    mm.ft_nml->algn = mm.ft_sel->algn = TA_X_L | TA_Y_T;

    for (i = 0; i < e_num; i++) {
        if (0) // changed later
            ft = mm.ft_sel;
        else
            ft = mm.ft_nml;
        ft->algn = TA_X_L | TA_Y_T;
        SF_Wrt(ft, sdl.scr, off, a_y + i * e_h, hscr[i].nm, 0);
        ft->algn = TA_X_C | TA_Y_T;
        sprintf(str, "%i", hscr[i].lvl);
        SF_Wrt(ft, sdl.scr, sdl.scr->w / 2, a_y + i * e_h, str, 0);
        sprintf(str, "%i", hscr[i].scr);
//        sprintf(str, "%i (%i.%i%%)", hscr[i].scr, hscr[i].pct / 10, hscr[i].pct % 10);
        ft->algn = TA_X_R | TA_Y_T;
        SF_Wrt(ft, sdl.scr, sdl.scr->w - off, a_y + i * e_h, str, 0);
    }

    mm.ft_nml->algn = mm.ft_sel->algn = TA_X_C | TA_Y_C;
    Sdl_FUpd();

    // wait for a click //
    Sdl_WtFrClk();

    //darken screen //
    if (cfg.dim)
        SDL_DIM();
    // restore screen //
    D_FDST(sdl.scr);
    D_FSRC(buf);
    SS_Blt();
    // brighten screen //
    if (cfg.dim)
        SDL_UNDIM();
    else
        Sdl_FUpd();

    // reset timer //
    T_Rst();
}

// menu callbacks //

/*
    activate / deactivate sound
*/
void CB_Snd()
{
#ifdef SOUND
    sound_enable(cfg.s_on);
#endif
}

/*
    set sound volume
*/
void CB_StV()
{
#ifdef SOUND
    sound_volume(cfg.s_vol * 16);
#endif
}

/*
    create a profile
*/
void CB_CrtP()
{
    Prf_Crt(cfg.prf_nm);
    Prf_CrtLst();
    // update menu entries //
    ME_CngSwX(me_prf, &cfg.prf, prf_lst, prf_n);
    ME_CngSwX(me_del, &cfg.prf, prf_lst, prf_n);
    ME_CngSwX(me_clr, &cfg.prf, prf_lst, prf_n);
}

/*
    clear a profile
*/
void CB_ClrP()
{
    Prf *p = DL_Get(&prfs, cfg.prf);
    DL_Clr(&p->sts);
    p->lvls = 0;
    p->scr = 0;
    p->pct = 0;
}

/*
    delete a profile
*/
void CB_DelP()
{
    if (prfs.cntr < 2) {
        printf("WARNING: last profile cannot be deleted\n");
        return;
    }
    // delete from list //
    DL_Del(&prfs, cfg.prf);
    Prf_CrtLst();
    // update menu entries //
    ME_CngSwX(me_prf, &cfg.prf, prf_lst, prf_n);
    ME_CngSwX(me_del, &cfg.prf, prf_lst, prf_n);
    ME_CngSwX(me_clr, &cfg.prf, prf_lst, prf_n);
}

void CB_SrtP()
{
//    Prf_Srt();
}

/*
    init menu
    add all entries
    check them
*/
void MM_CrtE()
{
    SDL_Surface *ss_bk, *ss_lg;
    SFnt        *ft_y, *ft_w, *ft_t;
    Menu        *_main, *opts, *snd, *new, *edit, *del, *clr, *crt, *c_del, *c_clr, *c_crt, *gfx, *ctrl;
    MEnt        *e;
    char        *str_diff[] = {"Easy", "Normal", "Hard", "Brainstorm"};

    // load and assign gfx //
    ss_bk = SS_Ld("gfx/title.bmp", SDL_SWSURFACE);
    ss_lg = SS_Ld("gfx/logo.bmp", SDL_SWSURFACE);
    ft_y = SF_LdFxd("gfx/f_yellow.bmp", 32, 96, 10);
    ft_w = SF_LdFxd("gfx/f_white.bmp", 32, 96, 10);
    MM_Ini(ss_bk->w / 2, ss_bk->h - 100, 50, ss_bk, ss_lg, ft_y, ft_w);

    // add copyright //
    ft_t = SF_Ld("gfx/tiny.sdlfnt");
	ft_t->algn = TA_X_R | TA_Y_B;
	SF_Wrt(ft_t, ss_bk, ss_bk->w-3, ss_bk->h-1, "(C) 2000 Michael Speck", 0);
	ft_t->algn = TA_X_L | TA_Y_B;
	SF_Wrt(ft_t, ss_bk, 0, ss_bk->h-1, "http://lgames.sourceforge.net", 0);
    SF_Fr(ft_t);

    // create and add entrys //
    _main = M_Crt(); MM_Add(_main);
    opts = M_Crt(); MM_Add(opts);
    snd = M_Crt(); MM_Add(snd);
    new = M_Crt(); MM_Add(new);
    edit = M_Crt(); MM_Add(edit);
    del = M_Crt(); MM_Add(del);
    clr = M_Crt(); MM_Add(clr);
    crt = M_Crt(); MM_Add(crt);
    c_del = M_Crt(); MM_Add(c_del);
    c_clr = M_Crt(); MM_Add(c_clr);
    c_crt = M_Crt(); MM_Add(c_crt);
    gfx = M_Crt(); MM_Add(gfx);
    ctrl = M_Crt(); MM_Add(ctrl);
    // main //
    M_Add(_main, ME_CrtSub("New Game", new));
    M_Add(_main, ME_CrtAct("Best Profiles", MA_HSC));
    M_Add(_main, ME_CrtSub("Options", opts));
    M_Add(_main, ME_CrtAct("Quit", MA_QUT));
    // options //
    M_Add(opts, ME_CrtSub("Controls", ctrl));
    M_Add(opts, ME_CrtSub("Graphics", gfx));
#ifdef SOUND
    M_Add(opts, ME_CrtSub("Audio", snd));
#else
    M_Add(opts, ME_CrtSep("Audio"));
#endif
    M_Add(opts, ME_CrtSep(""));
    M_Add(opts, ME_CrtSub("Back", _main));
    // sound //
    e = ME_CrtSw2("Sound: ", &cfg.s_on, "Off", "On");
    e->cb = CB_Snd;
    M_Add(snd, e);
    e = ME_CrtRng("Volume: ", &cfg.s_vol, 1, 8, 1);
    e->cb = CB_StV;
    M_Add(snd, e);
    M_Add(snd, ME_CrtSep(""));
    M_Add(snd, ME_CrtSub("Back", opts));
    // new game //
    M_Add(new, ME_CrtAct("Start", MA_PLY));
    M_Add(new, ME_CrtSep(""));
    M_Add(new, ME_CrtSwX("Difficulty:", &cfg.diff, str_diff, 4));
    if (cfg.ls >= ls_n) cfg.ls = 0; // maybe someone deleted some level sets //
    M_Add(new, ME_CrtSwX("Levelset:", &cfg.ls, ls_lst, ls_n));
    me_prf = ME_CrtSwX("Profile:", &cfg.prf, prf_lst, prf_n);
    M_Add(new, me_prf);
    M_Add(new, ME_CrtSep(""));
    M_Add(new, ME_CrtSub("Edit Profiles", edit));
    M_Add(new, ME_CrtSep(""));
    M_Add(new, ME_CrtSub("Back", _main));
    // edit //
    M_Add(edit, ME_CrtSub("Create Profile", crt));
    M_Add(edit, ME_CrtSub("Clear Profile", clr));
    M_Add(edit, ME_CrtSub("Delete Profile", del));
    M_Add(edit, ME_CrtSep(""));
    e = ME_CrtSub("Back", new);
    e->cb = CB_SrtP;
    M_Add(edit, e);
    // create //
    M_Add(crt, ME_CrtStr("Profile Name", cfg.prf_nm, 11));
    M_Add(crt, ME_CrtSub("Create Profile", c_crt));
    M_Add(crt, ME_CrtSep(""));
    M_Add(crt, ME_CrtSub("Back", edit));
    // confirm create //
    e = ME_CrtSub("Yes", crt);
    e->cb = CB_CrtP;
    M_Add(c_crt, e);
    M_Add(c_crt, ME_CrtSub("No", crt));
    // clear //
    me_clr = ME_CrtSwX("Profile:", &cfg.prf, prf_lst, prf_n);
    M_Add(clr, me_clr);
    M_Add(clr, ME_CrtSub("Clear Profile", c_clr));
    M_Add(clr, ME_CrtSep(""));
    M_Add(clr, ME_CrtSub("Back", edit));
    // confirm clear //
    e = ME_CrtSub("Yes", clr);
    e->cb = CB_ClrP;
    M_Add(c_clr, e);
    M_Add(c_clr, ME_CrtSub("No", clr));
    // delete //
    me_del = ME_CrtSwX("Profile:", &cfg.prf, prf_lst, prf_n);
    M_Add(del, me_del);
    M_Add(del, ME_CrtSub("Delete Profile", c_del));
    M_Add(del, ME_CrtSep(""));
    M_Add(del, ME_CrtSub("Back", edit));
    // confirm clear //
    e = ME_CrtSub("Yes", del);
    e->cb = CB_DelP;
    M_Add(c_del, e);
    M_Add(c_del, ME_CrtSub("No", del));
    // graphics //
    M_Add(gfx, ME_CrtSw2("Animations:", &cfg.ani, "Off", "On"));
//    M_Add(gfx, ME_CrtSw2("Transparency:", &cfg.trp, "Off", "On"));
    M_Add(gfx, ME_CrtSw2("Fullscreen:", &cfg.fscr, "Off", "On"));
    M_Add(gfx, ME_CrtSw2("Dim Effect:", &cfg.dim, "Off", "On"));
    M_Add(gfx, ME_CrtSep(""));
    M_Add(gfx, ME_CrtSub("Back", opts));
    // controls
    M_Add(ctrl, ME_CrtKey("Up", &cfg.k_up));
    M_Add(ctrl, ME_CrtKey("Down", &cfg.k_down));
    M_Add(ctrl, ME_CrtKey("Left", &cfg.k_left));
    M_Add(ctrl, ME_CrtKey("Right", &cfg.k_right));
    M_Add(ctrl, ME_CrtKey("Undo", &cfg.k_undo));
    M_Add(ctrl, ME_CrtSep(""));
    M_Add(ctrl, ME_CrtSub("Back", opts));

    // default valid keys
    MM_DfVKys();

    // adjust position of all entries in all menus //
    MM_AdjP();

    // call all callbacks //
    MM_CB();

    // check for errors
    MM_Ck();
}

/*
    init sound, menu, profiles, game
    main loop for the menu
*/
int main(int argc, char *argv[])
{
    SDL_Event   e;
    int         go_on = 1;
    int         ms;
    char aux[64];
    
#ifdef _WIN32
    /* Get Windows to open files in binary mode instead of default text mode */
    _fmode = _O_BINARY;
#endif
    
    // just query version? //
    if ( argc > 1 && !strncmp( "--version", argv[1], 9 ) ) {

        printf( "%s\n", VERSION );
        return 0;

    }

    // random init
    srand(time(NULL));

    // sdl init //
#ifdef SOUND
    Sdl_Ini(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
#else
    Sdl_Ini(SDL_INIT_VIDEO | SDL_INIT_TIMER);
#endif
    sprintf( aux, "LMarbles %s", VERSION );
    SDL_WM_SetCaption(aux, 0);

    // show logo //
    Sdl_StVdMd(512, 384, 16, SDL_SWSURFACE);

    // show hardware capabilities //
    Sdl_HwCps();

    // load config //
    C_StPth();
    C_Ld();

    // load profiles //
    Prf_Ini();
    if (!Prf_Ld())
        cfg.prf = 0;
            
    // create levelset list and reset config's levelset index if nescessary //
    L_CrtLst();
    if (cfg.ls >= ls_n)
        cfg.ls = 0;

#ifdef SOUND
    audio_open();
    sound_enable( cfg.s_on );
    sound_volume( cfg.s_vol * 16 );
#endif
		
    // game init //
    G_Ini();

    // init and show menu//
    MM_CrtE();
    MM_Shw(MM_RSZ);

    event_clear_sdl_queue();
    //menu loop
    T_Rst(); // reset time //
    while (go_on && !trm_gm) {
        M_Hd(mm.c_mn);
        if (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                trm_gm = 1;
            if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE)
                go_on = 0;
            switch (MM_Evt(&e)) {
                case MA_QUT:
                    go_on = 0;
                    break;
                case MA_PLY:
                    if (G_Opn()) {
                        G_Run();
                        G_Cls();
                    }
                    break;
                case MA_HSC:
                    M_Shw(mm.c_mn);
                    Sdl_UpdR();
                    H_Shw(); // defined in main.c //
                    break;
            }
        }
        ms = T_Gt();
        M_CmA(mm.c_mn, ms);
        M_Shw(mm.c_mn);
        Sdl_UpdR();
        SDL_Delay( 5 );
    }

    // terminate menu //
    MM_Trm();

    // terminate game //
    G_Trm();

    // close soundserver //
#ifdef SOUND
    audio_close();
#endif

    // save config //
    C_Sv();

    // free levelset list //
    L_DelLst();

    // save profiles //
    Prf_Sv();
    Prf_Trm();

    // free screen //
    Sdl_Qut();

    printf("Bye, bye!\n");

    return 0;
}
