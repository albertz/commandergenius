/***************************************************************************
                          credit.h  -  description
                             -------------------
    begin                : Thu Sep 13 2001
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

/*
====================================================================
Initiate credit.
====================================================================
*/
void credit_init( char *name, char *author, int level, int level_count );
void credit_clear();
/*
====================================================================
Show/Hide credits
====================================================================
*/
void credit_hide();
void credit_show();
void credit_alphashow( int alpha );
void credit_update( int ms );
