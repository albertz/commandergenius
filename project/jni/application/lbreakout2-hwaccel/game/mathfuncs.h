/***************************************************************************
                          mathfuncs.h  -  description
                             -------------------
    begin                : Thu Sep 6 2001
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

#ifndef __MATH_H
#define __MATH_H

/* integer vector */
typedef struct {
    int x,y;
} ICoord;

typedef struct {
    float x; /* position if vertical line */
    int vertical; /* if this is set monotony is not computed */
    float m; /* monotony */
    float n; /* y_offset */
} Line;

/* integer line */
typedef struct {
    int x; /* position if vertical line */
    int vertical; /* if this is set monotony is not computed */
    int m_4096; /* monotony * 4096 */
    int n; /* y_offset */
} ILine;

/*
====================================================================
Return vector struct with the specified coordinates.
====================================================================
*/
Vector vector_get( float x, float y );
/*
====================================================================
Give vector the normed length of 1.
====================================================================
*/
void vector_norm( Vector *v );
/*
====================================================================
Return monotony of vector. If vertical return 0
====================================================================
*/
float vector_monotony( Vector v );
/*
====================================================================
Set length of a vector.
====================================================================
*/
void vector_set_length( Vector *v, float length );

/*
====================================================================
Initiate a line struct.
====================================================================
*/
void line_set( Line *line, float x, float y, float m );
void line_set_vert( Line *line, float x );
void line_set_hori( Line *line, float y );
/*
====================================================================
Intersect lines and set 'pos' to intersecting point.
Return Value: True if lines intersect.
====================================================================
*/
int line_intersect( Line *line, Line *target, Coord *pos );

/*
====================================================================
Initiate a line struct.
====================================================================
*/
void iline_set( ILine *line, int x, int y, int m_4096 );
void iline_set_vert( ILine *line, int x );
void iline_set_hori( ILine *line, int y );
/*
====================================================================
Intersect lines and set 'pos' to intersecting point.
Return Value: True if lines intersect.
====================================================================
*/
int iline_intersect( ILine *line, ILine *target, ICoord *pos );

/*
====================================================================
Intersect line pos+t*v with circle (x+m)²=r²
Important length of v MUST be 1.
Return Value: True if intersecting, Intersecting points
====================================================================
*/
int circle_intersect( Vector m, int r, Vector pos, Vector v, Vector *t1, Vector *t2 );

/* convert vector to angle/2 0-180 and vice versa */
void init_angles( void );
int  vec2angle( Vector *vec );
void angle2vec( int angle, Vector *vec );

#endif
