/***************************************************************************
                          shrapnells.c  -  description
                             -------------------
    begin                : Sat Sep 8 2001
    copyright            : (C) 2001 by Michael Speck
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

#include "lbreakout.h"
#include "../game/mathfuncs.h"
#include "config.h"
#include "shrapnells.h"

extern Config config;
extern SDL_Surface *stk_display;
extern SDL_Surface *offscreen;
List *shrapnells = 0;
float shr_alpha_change = 0.25;
float shr_start_alpha = 255;

/*
====================================================================
Local
====================================================================
*/

/*
====================================================================
Create a single shrapnell and add to list.
====================================================================
*/
void shrapnell_create( SDL_Surface *surf, 
    int sx, int sy, int sw, int sh, 
    int px, int py, float vx, float vy )
{
    Shrapnell *shr = calloc( 1, sizeof( Shrapnell ) );
    shr->pic = stk_surface_create( SDL_SWSURFACE, sw,sh );
    //SDL_SetColorKey(shr->pic, 0, 0);
    stk_surface_blit( surf, sx, sy,sw,sh, shr->pic, 0,0 );
    shr->x = px;
    shr->y = py;
    shr->v.x = vx;
    shr->v.y = vy;
    shr->alpha = shr_start_alpha;
    list_add( shrapnells, shr);
}
/*
====================================================================
Delete a shrapnell
====================================================================
*/
void shrapnell_delete( void *ptr )
{
    Shrapnell *shr = (Shrapnell*)ptr;
    SDL_FreeSurface( shr->pic );
    free( shr );
}

/*
====================================================================
Public
====================================================================
*/

/*
====================================================================
Initate or delete shrapnell list
====================================================================
*/
void shrapnells_init()
{
    shrapnells = list_create( LIST_AUTO_DELETE, shrapnell_delete );
}
void shrapnells_delete()
{
    list_delete( shrapnells );
}
/*
====================================================================
Create shrapnells from surface (surf,sx,sy,sw,sh) and put it to
screen position (px,py). The vector imp is the 
impuls and type the type of what caused the destruction.
====================================================================
*/
void shrapnells_create( SDL_Surface *surf,
    int sx, int sy, int sw, int sh, int px, int py, 
    int type, Vector imp )
{
    int i, j;
    int shr_w, shr_h;
    Vector v;
    int dx, dy;
    /* no animations? */
    if ( !config.anim ) 
        return;
    /*
     * 1 - low
     * 2 - medium
     * 3 - high
     */
    /* according to type create shrapnells */
    switch ( type ) {
        case SHR_BY_NORMAL_BALL:
            vector_norm( &imp );
            imp.x *= 0.13; imp.y *= 0.13;
            shrapnell_create( surf, sx, sy, sw, sh, px, py,
                              imp.x, imp.y );
            break;
        case SHR_BY_ENERGY_BALL:
            if ( config.anim == 1 ) {
                /* low */
                shrapnell_create( surf, sx, sy, sw, sh, px, py, 
                                  0, 0 );
                break;
            }
            shr_w = sw / 2;
            shr_h = sh / 2;
            for ( i = 0; i < sw; i += shr_w )
                for ( j = 0; j < sh; j += shr_h ) {
                    v.x = ( sw >> 1 ) - ( i + ( shr_w >> 1 ) );
                    v.y = ( sh >> 1 ) - ( j + ( shr_h >> 1 ) );
                    vector_norm( &v );
                    v.x *= 0.01; v.y *= 0.01;
                    shrapnell_create( surf, 
                            sx + i, sy + j, shr_w, shr_h,
                            px + i, py + j, v.x, v.y );
                }
            break;
        case SHR_BY_SHOT:
            if ( config.anim == 1 ) {
                /* low */
                shrapnell_create( surf, sx, sy, sw, sh, 
                        px, py, 0, imp.y * 0.02 );
                break;
            }
            shr_w = sw / 8;
            shr_h = sh;
            for ( i = 0; i < ( sw >> 1 ); i += shr_w ) {
                shrapnell_create( surf, sx + i, sy, shr_w, shr_h, 
                        px + i, py, 0, imp.y * 0.002 * ( i + 1 ) );
                shrapnell_create( surf, sx + sw - shr_w - i, sy, 
                        shr_w, shr_h, 
                        px + sw - shr_w - i, py, 0, 
                        imp.y * 0.002 * ( i + 1 ) );
            }
            break;
        case SHR_BY_EXPL:
            shr_w = config.anim == 3 ? 5 : 10;
            shr_h = config.anim == 3 ? 5 : 10;
            for ( i = 0; i < sw; i += shr_w )
                for ( j = 0; j < sh; j += shr_h ) {
                    dx = rand() % 2 == 0 ? 1 : -1;
                    dy = rand() % 2 == 0 ? 1 : -1;
                    shrapnell_create( surf, 
                            sx + i, sy + j, shr_w, shr_h, 
                            px + i, py + j,
                            (float)( ( rand() % 6 ) + 5) / 100 * dx, 
                            (float)( ( rand() % 6 ) + 5) / 100 * dy );
                }
            break;
    }
}
/*
====================================================================
Remove all shrapnells
====================================================================
*/
void shrapnells_reset()
{
    list_clear( shrapnells );
}
/*
====================================================================
Show, hide all shrapnells.
====================================================================
*/
void shrapnells_hide()
{
    ListEntry  *entry = shrapnells->head->next;
    Shrapnell   *shr;
    int         x, y;
    while ( entry != shrapnells->tail ) {
        shr = entry->item;
        entry = entry->next;
        x = (int)shr->x;
        y = (int)shr->y;
        stk_surface_blit( offscreen, x, y,
            shr->pic->w, shr->pic->h,
            stk_display, x, y );
        stk_display_store_drect();
    }
}
void shrapnells_show()
{
    ListEntry  *entry = shrapnells->head->next;
    Shrapnell   *shr;
    while ( entry != shrapnells->tail ) {
        shr = entry->item;
        entry = entry->next;
        stk_surface_blit( shr->pic, 
            0,0,shr->pic->w,shr->pic->h,
            stk_display, (int)shr->x, (int)shr->y );
        stk_display_store_drect();
    }
}
/*
====================================================================
Update and remove expired shrapnells
====================================================================
*/
void shrapnells_update( int ms )
{
    ListEntry  *entry = shrapnells->head->next;
    Shrapnell   *shr;
    while ( entry != shrapnells->tail ) {
        shr = entry->item;
        entry = entry->next;
        shr->x += shr->v.x * ms;
        shr->y += shr->v.y * ms;
        if (shr->alpha > 0 ) {
            shr->alpha -= shr_alpha_change * ms;
            SDL_SetAlpha( shr->pic, SDL_SRCALPHA, (int)shr->alpha );
        }
        if (shr->alpha <= 0 || 
            shr->x + shr->pic->w < 0 || 
            shr->y + shr->pic->h < 0 || 
            shr->x > stk_display->w || 
            shr->y > stk_display->h )
            list_delete_entry( shrapnells, entry->prev );
    }
}

/*
====================================================================
Brick explosion animations.
====================================================================
*/
#define exp_change 0.015
#define exp_alpha_change 0.10
List *exps;
SDL_Surface *current_exp_pic = 0; /* points either to exp_pic or exp_dark_pic */
extern SDL_Surface *exp_pic;
extern SDL_Surface *exp_dark_pic;
int   exp_w, exp_h; /* size */
int   exp_frame_count; /* frame count */
int   exp_count; /* number of various explosions */
typedef struct {
    float alpha;
    float cur; /* current frame */
    int x, y; /* position in screen */
    int y_off; /* y offset in explosion picture */
} Exp;
void exp_load()
{
    exps = list_create( LIST_AUTO_DELETE, LIST_NO_CALLBACK );
    exp_w = exp_h = 60; /* hardcoded as we have various explosions in one file now */
    exp_count = exp_pic->h / exp_h;
    exp_frame_count = exp_pic->w / exp_w;
    exps_set_dark( 0 );
}
void exps_clear()
{
    list_clear( exps );
}
void exp_delete()
{
    if ( exps ) list_delete( exps ); exps = 0;
}
void exp_create( int x, int y )
{
    Exp *exp = calloc( 1, sizeof( Exp ) );
    exp->x = x - ( exp_w >> 1 );
    exp->y = y - ( exp_h >> 1 );
    exp->alpha = 255;
    /* red/yellow get 4 parts, gree&yellow&bluw get 1 part each */ 
    switch ( rand() % ( 4 + 4 + 1 + 1 + 1 ) ) {
        case 0: case 1: case 2: case 3:
            exp->y_off = 0; break;
        case 4: case 5: case 6: case 7:
            exp->y_off = exp_h * 4; break;
        case 8:
            exp->y_off = exp_h * 1; break;
        case 9: 
            exp->y_off = exp_h * 2; break;
        case 10:
            exp->y_off = exp_h * 3; break;
    }
    list_add( exps, exp );
}
void exps_hide()
{
    Exp *exp;
    list_reset( exps );
    while ( ( exp = list_next( exps ) ) ) {
        stk_surface_blit( offscreen, exp->x, exp->y,
            exp_w, exp_h, stk_display, exp->x, exp->y );
        stk_display_store_drect();
    }
}
void exps_show()
{
    Exp *exp;
    list_reset( exps );
    while ( ( exp = list_next( exps ) ) ) {
        stk_surface_alpha_blit( current_exp_pic, 
            (int)exp->cur * exp_w, exp->y_off,
            exp_w, exp_h,
            stk_display, exp->x, exp->y, exp->alpha );
    }
}
void exps_update( int ms )
{
    Exp *exp;
    list_reset( exps );
    while ( ( exp = list_next( exps ) ) ) {
        if ( exp->alpha > 0 ) {
            exp->alpha -= exp_alpha_change * ms;
            if ( exp->alpha < 0 ) exp->alpha = 0;
        }
        exp->cur += exp_change * ms;
        if ( exp->cur >= exp_frame_count )
            list_delete_current( exps );
    }    
}
void exps_set_dark( int dark )
{
    if ( dark )
        current_exp_pic = exp_dark_pic;
    else
        current_exp_pic = exp_pic;
}

