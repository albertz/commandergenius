/***************************************************************************
                          mathfuncs.c  -  description
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

#include "../client/lbreakout.h"
#include "mathfuncs.h"

char circle_msg[256];

/*
====================================================================
Return vector struct with the specified coordinates.
====================================================================
*/
Vector vector_get( float x, float y )
{
    Vector v = { x, y };
    return v;
}
/*
====================================================================
Give vector the normed length of 1.
====================================================================
*/
void vector_norm( Vector *v )
{
    float length;
    if ( v->x == 0 && v->y == 0 ) return; /* NULL vector may not be normed */
    length = sqrt( v->x * v->x + v->y * v->y );
    v->x /= length;
    v->y /= length;
}
/*
====================================================================
Return monotony of vector. If vertical return 0
====================================================================
*/
float vector_monotony( Vector v )
{
    if ( v.x == 0 ) return 0;
    return v.y / v.x;
}
/*
====================================================================
Set length of a vector.
====================================================================
*/
void vector_set_length( Vector *v, float length )
{
    vector_norm( v );
    v->x *= length; v->y *= length;
}

/*
====================================================================
Initiate a line struct.
====================================================================
*/
void line_set( Line *line, float x, float y, float m )
{
    line->vertical = 0;
    line->m = m;
    line->n = y - m*x;
}
void line_set_vert( Line *line, float x )
{
    line->vertical = 1;
    line->x = x;
}
void line_set_hori( Line *line, float y )
{
    line->vertical = 0;
    line->m = 0;
    line->n = y;
}
/*
====================================================================
Intersect lines and set 'pos' to intersecting point.
Return Value: True if lines intersect.
====================================================================
*/
int line_intersect( Line *line, Line *target, Coord *pos )
{
    /* reset pos */
    pos->x = pos->y = 0;
    /* if lines are parallel return False */
    if ( line->vertical && target->vertical ) return 0; /* vertical parallels */ 
    if ( !line->vertical &&  !target->vertical && line->m == target->m ) return 0; /* non-vertical parallels */
    /* right now only one thing is supported: line horizontal */
    if ( line->m == 0 && line->vertical == 0 ) {
        pos->y = line->n;
        if ( target->vertical )
            pos->x = target->x;
        else
            pos->x = ( pos->y - target->n ) / target->m;
        return 1;
    }
    if ( line->vertical ) {
        if ( target->vertical ) return 0;
        pos->x = line->x;
        pos->y = target->m * pos->x + target->n;
        return 1;
    }
    if ( target->vertical ) {
        printf( "line_intersect: line non-vertical and target vertical not supported yet\n" );
        return 1;
    }
    /* compute if both lines are neither vertical nor horizontal */
    pos->x = ( line->n - target->n ) / ( target->m - line->m );
    pos->y = line->m * pos->x + line->n;
    return 1;
}

/*
====================================================================
Initiate a line struct.
====================================================================
*/
void iline_set( ILine *line, int x, int y, int m_4096 )
{
    line->vertical = 0;
    line->m_4096 = m_4096;
    line->n = y - ((m_4096*x)>>12);
}
void iline_set_vert( ILine *line, int x )
{
    line->vertical = 1;
    line->x = x;
}
void iline_set_hori( ILine *line, int y )
{
    line->vertical = 0;
    line->m_4096 = 0;
    line->n = y;
}
/*
====================================================================
Intersect lines and set 'pos' to intersecting point.
Return Value: True if lines intersect.
====================================================================
*/
int iline_intersect( ILine *line, ILine *target, ICoord *pos )
{
    /* reset pos */
    pos->x = pos->y = 0;
    /* if lines are parallel return False */
    if ( line->vertical && target->vertical ) return 0; /* vertical parallels */ 
    if ( !line->vertical &&  !target->vertical )
    if ( line->m_4096 == target->m_4096 ) return 0; /* non-vertical parallels */
    /* right now only one thing is supported: line horizontal */
    if ( line->m_4096 == 0 && line->vertical == 0 ) {
        pos->y = line->n;
        if ( target->vertical )
            pos->x = target->x;
        else
            pos->x = (( pos->y - target->n )<<12) / target->m_4096;
        return 1;
    }
    if ( line->vertical ) {
        if ( target->vertical ) return 0;
        pos->x = line->x;
        pos->y = ((target->m_4096 * pos->x)>>12) + target->n;
        return 1;
    }
    if ( target->vertical ) {
        printf( "line_intersect: line non-vertical and target vertical not supported yet\n" );
        return 1;
    }
    /* compute if both lines are neither vertical nor horizontal */
    pos->x = (( line->n - target->n )<<12) / ( target->m_4096 - line->m_4096 );
    pos->y = ((line->m_4096 * pos->x)>>12) + line->n;
    return 1;
}

/*
====================================================================
Intersect line pos+t*v with circle (x+m)²=r²
Important length of v MUST be 1.
Return Value: True if intersecting, Intersecting points
====================================================================
*/
int circle_intersect( Vector m, int r, Vector pos, Vector v, Vector *t1, Vector *t2 )
{
    Vector delta = { pos.x - m.x, pos.y - m.y };
    float  delta_v = delta.x * v.x + delta.y * v.y;
    float dis = delta_v * delta_v + r * r - ( delta.x * delta.x + delta.y * delta.y );
    float t;

    if ( dis < 0 ) {
#ifdef WITH_BUG_REPORT
		sprintf( circle_msg, "Diskriminante < 0" );
#endif		
		return 0; 
	}
	dis = sqrt( dis );

    t = -delta_v + dis;
    t1->x = pos.x + t * v.x; t1->y = pos.y + t * v.y;
    t = -delta_v - dis;
    t2->x = pos.x + t * v.x; t2->y = pos.y + t * v.y;
#ifdef WITH_BUG_REPORT
	sprintf( circle_msg, "Intersection points: (%4.2f,%4.2f), (%4.2f,%4.2f)", t1->x, t1->y, t2->x, t2->y );
#endif
    return 1;
}

/* convert vector to angle/2 0-180 and vice versa */
static Vector impact_vectors[180]; /* clockwise impact vectors in 2 degree steps */

void init_angles( void )
{
	int i;
	
	/* create vectors for all degrees in 2° steps */
	for ( i = 0; i < 180; i++ ) {
		impact_vectors[i].x = cos( 6.28 * i / 180 );
		impact_vectors[i].y = sin( 6.28 * i / 180 );
	}
}

int vec2angle( Vector *vec )
{
	int degrees = 0;
	
	/* translate impact vector (src-center) into degrees 
	 * (0°: horizontally right, clockwise going) */
	if ( vec->x == 0 )
		degrees = 90; /* basically impossible as we mask this out */
	else
		degrees = (int)(360 * atan( (double)(fabs(vec->y))/fabs(vec->x) ) / 6.28);
	/* get the proper quartal */
	if ( vec->x > 0 ) {
		if ( vec->y < 0 )
			degrees = 360 - degrees;
	}
	else {
		if ( vec->y >= 0 )
			degrees = 180 - degrees;
		else
			degrees = 180 + degrees;
	}

	return degrees/2;
}

void angle2vec( int angle, Vector *vec )
{
	vec->x = 0; 
	vec->y = 0;

	if ( angle < 0 ) return;
	if ( angle > 179 ) return;

	*vec = impact_vectors[angle];
}

    
