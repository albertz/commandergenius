/***************************************************************************
                          cfg.c  -  description
                             -------------------
    begin                : Sat Aug 5 2000
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

#include "cfg.h"
#include "sdl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#ifdef _WIN32
 #define MKDIR(d, p) mkdir(d)
#else
 #define MKDIR mkdir
#endif

char c_pth[512];
Cfg cfg;

/*
    get the full path of the cfg file
*/
void C_StPth()
{
    DIR *dir;
#ifdef _WIN32
    snprintf( c_pth, sizeof(c_pth)-1, "%s/lgames", (getenv( "HOME" )?getenv( "HOME" ):".") );
#else
    snprintf( c_pth, sizeof(c_pth)-1, "%s/.lgames", getenv( "HOME" ) );
#endif
    /* create .lgames directory if not found */
    if ( (dir = opendir( c_pth )) == 0 ) {
        fprintf( stderr, "Config directory ~/.lgames not found. Creating it.\n" );
        MKDIR( c_pth, S_IRWXU );
    }
    else
    {
	closedir(dir);
    }
    /* get full path of config file */
    strcat( c_pth, "/lmarbles.conf" );
}

/*
    load it
*/
void C_Ld()
{
    char str[256];
    FILE	*f;
    struct stat dir_stat;

    printf("loading configuration...\n");

    // load init //
    if ((f = fopen(c_pth, "r")) == 0) {
        printf("cfg file '%s' not found; using defaults\n", c_pth);
        C_Def();
	}
    else {
        stat( c_pth, &dir_stat );
        if ( dir_stat.st_size != sizeof( Cfg ) ) {
            fprintf( stderr, "config file '%s' corrupted... using defaults\n", c_pth );
            C_Def();
        }
        else
            
        {
#ifdef ASCII

        F_GetE(f, str, F_VAL);
        if (strncmp(str,"ascii",5)) {

            printf("cfg file '%s' not in ascii; using defaults\n", c_pth);
            C_Def();

        }
        else {

            F_GetE(f, cfg.prf_nm, F_VAL); cfg.prf_nm[strlen(cfg.prf_nm) - 1] = 0;
            F_GetE(f, str, F_VAL); F_ValToInt(str, &cfg.prf);
            F_GetE(f, str, F_VAL); F_ValToInt(str, &cfg.s_vol);
            F_GetE(f, str, F_VAL); F_ValToInt(str, &cfg.s_on);
            F_GetE(f, str, F_VAL); F_ValToInt(str, &cfg.ani);
            F_GetE(f, str, F_VAL); F_ValToInt(str, &cfg.trp);
            F_GetE(f, str, F_VAL); F_ValToInt(str, &cfg.fscr);
            F_GetE(f, str, F_VAL); F_ValToInt(str, &cfg.dim);
            F_GetE(f, str, F_VAL); F_ValToInt(str, &cfg.diff);
            F_GetE(f, str, F_VAL); F_ValToInt(str, &cfg.k_up);
            F_GetE(f, str, F_VAL); F_ValToInt(str, &cfg.k_down);
            F_GetE(f, str, F_VAL); F_ValToInt(str, &cfg.k_left);
            F_GetE(f, str, F_VAL); F_ValToInt(str, &cfg.k_right);
            F_GetE(f, str, F_VAL); F_ValToInt(str, &cfg.k_undo);

        }
#else

        fread(str, 5, 1, f);
        if (!strncmp(str,"ascii",5)) {

            printf("cfg file '%s' in ascii but raw binary data expected; using defaults\n", c_pth);
            C_Def();

        }
        else {

            fseek(f,SEEK_SET,0);
    		fread(&cfg, sizeof(Cfg), 1, f);

        }

#endif		
        }
        
		fclose(f);
	}
}

/*
    save it
*/
void C_Sv()
{
    //save init //
    FILE	*f = fopen(c_pth, "w");
#ifdef ASCII
    char    str[256];

    F_WrtE(f, "ascii");
    F_WrtE(f, cfg.prf_nm);
    F_IntToStr(str, cfg.prf); F_WrtE(f, str);
    F_IntToStr(str, cfg.s_vol); F_WrtE(f, str);
    F_IntToStr(str, cfg.s_on); F_WrtE(f, str);
    F_IntToStr(str, cfg.ani); F_WrtE(f, str);
    F_IntToStr(str, cfg.trp); F_WrtE(f, str);
    F_IntToStr(str, cfg.fscr); F_WrtE(f, str);
    F_IntToStr(str, cfg.dim); F_WrtE(f, str);
    F_IntToStr(str, cfg.diff); F_WrtE(f, str);
    F_IntToStr(str, cfg.k_up); F_WrtE(f, str);
    F_IntToStr(str, cfg.k_down); F_WrtE(f, str);
    F_IntToStr(str, cfg.k_left); F_WrtE(f, str);
    F_IntToStr(str, cfg.k_right); F_WrtE(f, str);
    F_IntToStr(str, cfg.k_undo); F_WrtE(f, str);
#else
    fwrite(&cfg, sizeof(Cfg), 1, f);
#endif

    fclose(f);
}

/*
    default values
*/
void C_Def()
{
    strcpy(cfg.prf_nm, "Michael");
    cfg.prf = 0;
    // sound //
    cfg.s_vol = 6;
    cfg.s_on = 1;
    // gfx //
    cfg.ani = 1;
    cfg.trp = 1;
    cfg.fscr = 0;
    cfg.dim = 1;
    cfg.diff = DIFF_NORMAL;
    // controls
    cfg.k_up = SDLK_UP;
    cfg.k_down = SDLK_DOWN;
    cfg.k_left = SDLK_LEFT;
    cfg.k_right = SDLK_RIGHT;
    cfg.k_undo = SDLK_SPACE;
}
