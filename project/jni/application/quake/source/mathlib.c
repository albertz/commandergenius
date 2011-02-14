/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// mathlib.c -- math primitives

//Dan East:
//I've made many additions to this file, recreating various original functions to use
//my Fixed Point Math routines.  My additions all contain the acronym FPM

#include <math.h>
#include "quakedef.h"

void Sys_Error (char *error, ...);

vec3_t		vec3_origin = {0,0,0};
#ifdef USEFPM
vec3_FPM_t	vec3_originFPM = {0,0,0};
#endif
int nanmask = 255<<23;

/*-----------------------------------------------------------------*/

#define DEG2RAD( a ) ( a * M_PI ) / 180.0F

void ProjectPointOnPlane( vec3_t dst, const vec3_t p, const vec3_t normal )
{
	float d;
	vec3_t n;
	float inv_denom;

	inv_denom = 1.0F / DotProduct( normal, normal );

	d = DotProduct( normal, p ) * inv_denom;

	n[0] = normal[0] * inv_denom;
	n[1] = normal[1] * inv_denom;
	n[2] = normal[2] * inv_denom;

	dst[0] = p[0] - d * n[0];
	dst[1] = p[1] - d * n[1];
	dst[2] = p[2] - d * n[2];
}

#ifdef USEFPM
void ProjectPointOnPlaneFPM( vec3_FPM_t dst, const vec3_FPM_t p, const vec3_FPM_t normal )
{
	fixedpoint_t d;
	vec3_FPM_t n;
	fixedpoint_t inv_denom;

	inv_denom = /*1.0F*/0x10000 / DotProductFPM( normal, normal );

	d = DotProductFPM( normal, p ) * inv_denom;

	n[0] = FPM_MUL(normal[0], inv_denom);
	n[1] = FPM_MUL(normal[1], inv_denom);
	n[2] = FPM_MUL(normal[2], inv_denom);

	dst[0] = FPM_SUB(p[0], FPM_MUL(d, n[0]));
	dst[1] = FPM_SUB(p[1], FPM_MUL(d, n[1]));
	dst[2] = FPM_SUB(p[2], FPM_MUL(d, n[2]));
}
#endif

/*
** assumes "src" is normalized
*/
void PerpendicularVector( vec3_t dst, const vec3_t src )
{
	int	pos;
	int i;
	float minelem = 1.0F;
	vec3_t tempvec;

	/*
	** find the smallest magnitude axially aligned vector
	*/
	for ( pos = 0, i = 0; i < 3; i++ )
	{
		if ( fabs( src[i] ) < minelem )
		{
			pos = i;
			minelem = (float)fabs( src[i] );
		}
	}
	tempvec[0] = tempvec[1] = tempvec[2] = 0.0F;
	tempvec[pos] = 1.0F;

	/*
	** project the point onto the plane defined by src
	*/
	ProjectPointOnPlane( dst, tempvec, src );

	/*
	** normalize the result
	*/
	VectorNormalize( dst );
}

#ifdef USEFPM
void PerpendicularVectorFPM( vec3_FPM_t dst, const vec3_FPM_t src )
{
	int	pos;
	int i;
	fixedpoint_t minelem = 0x00010000;
	vec3_FPM_t tempvec;

	/*
	** find the smallest magnitude axially aligned vector
	*/
	for ( pos = 0, i = 0; i < 3; i++ )
	{
		if ( FPM_ABS( src[i] ) < minelem )
		{
			pos = i;
			minelem = FPM_ABS( src[i] );
		}
	}
	tempvec[0] = tempvec[1] = tempvec[2] = 0;
	tempvec[pos] = 0x10000;

	/*
	** project the point onto the plane defined by src
	*/
	ProjectPointOnPlaneFPM( dst, tempvec, src );

	/*
	** normalize the result
	*/
	VectorNormalizeFPM( dst );
}
#endif

#ifdef _WIN32
#pragma optimize( "", off )
#endif


void RotatePointAroundVector( vec3_t dst, const vec3_t dir, const vec3_t point, float degrees )
{
	float	m[3][3];
	float	im[3][3];
	float	zrot[3][3];
	float	tmpmat[3][3];
	float	rot[3][3];
	int	i;
	vec3_t vr, vup, vf;

	vf[0] = dir[0];
	vf[1] = dir[1];
	vf[2] = dir[2];

	PerpendicularVector( vr, dir );
	CrossProduct( vr, vf, vup );

	m[0][0] = vr[0];
	m[1][0] = vr[1];
	m[2][0] = vr[2];

	m[0][1] = vup[0];
	m[1][1] = vup[1];
	m[2][1] = vup[2];

	m[0][2] = vf[0];
	m[1][2] = vf[1];
	m[2][2] = vf[2];

	Q_memcpy( im, m, sizeof( im ) );

	im[0][1] = m[1][0];
	im[0][2] = m[2][0];
	im[1][0] = m[0][1];
	im[1][2] = m[2][1];
	im[2][0] = m[0][2];
	im[2][1] = m[1][2];

	Q_memset( zrot, 0, sizeof( zrot ) );
	zrot[0][0] = zrot[1][1] = zrot[2][2] = 1.0F;

	zrot[0][0] = (float)cos( DEG2RAD( degrees ) );
	zrot[0][1] = (float)sin( DEG2RAD( degrees ) );
	zrot[1][0] = (float)-sin( DEG2RAD( degrees ) );
	zrot[1][1] = (float)cos( DEG2RAD( degrees ) );

	R_ConcatRotations( m, zrot, tmpmat );
	R_ConcatRotations( tmpmat, im, rot );

	for ( i = 0; i < 3; i++ )
	{
		dst[i] = rot[i][0] * point[0] + rot[i][1] * point[1] + rot[i][2] * point[2];
	}
}

#ifdef USEFPM
void RotatePointAroundVectorFPM( vec3_FPM_t dst, const vec3_FPM_t dir, const vec3_FPM_t point, fixedpoint_t degrees )
{
	fixedpoint_t	m[3][3];
	fixedpoint_t	im[3][3];
	fixedpoint_t	zrot[3][3];
	fixedpoint_t	tmpmat[3][3];
	fixedpoint_t	rot[3][3];
	int	i;
	vec3_FPM_t vr, vup, vf;

	vf[0] = dir[0];
	vf[1] = dir[1];
	vf[2] = dir[2];

	PerpendicularVectorFPM( vr, dir );
	CrossProductFPM( vr, vf, vup );

	m[0][0] = vr[0];
	m[1][0] = vr[1];
	m[2][0] = vr[2];

	m[0][1] = vup[0];
	m[1][1] = vup[1];
	m[2][1] = vup[2];

	m[0][2] = vf[0];
	m[1][2] = vf[1];
	m[2][2] = vf[2];

	Q_memcpy( im, m, sizeof( im ) );

	im[0][1] = m[1][0];
	im[0][2] = m[2][0];
	im[1][0] = m[0][1];
	im[1][2] = m[2][1];
	im[2][0] = m[0][2];
	im[2][1] = m[1][2];

	Q_memset( zrot, 0, sizeof( zrot ) );
	zrot[0][0] = zrot[1][1] = zrot[2][2] = 0x10000;

	zrot[1][0]=-(zrot[0][0] = FPM_SIN_DEG(degrees));
	zrot[1][1] = FPM_COS_DEG( degrees );

	R_ConcatRotationsFPM( m, zrot, tmpmat );
	R_ConcatRotationsFPM( tmpmat, im, rot );

	for ( i = 0; i < 3; i++ )
	{
		dst[i] = FPM_ADD3(FPM_MUL(rot[i][0], point[0]), FPM_MUL(rot[i][1], point[1]), FPM_MUL(rot[i][2], point[2]));
	}
}
#endif

#ifdef _WIN32
#pragma optimize( "", on )
#endif

/*-----------------------------------------------------------------*/


float	anglemod(float a)
{
#if 0
	if (a >= 0)
		a -= 360*(int)(a/360);
	else
		a += 360*( 1 + (int)(-a/360) );
#endif
	a = (float)((360.0/65536) * ((int)(a*(65536/360.0)) & 65535));
	return a;
}

#ifdef USEFPM
fixedpoint_t	anglemodFPM(fixedpoint_t a)
{
#if 0
	if (a >= 0)
		a -= 360*(int)(a/360);
	else
		a += 360*( 1 + (int)(-a/360) );
#endif
	//Dan: TODO: the following will overflow 16.16 signed fixedpoint.  This algorithm
	//needs to be tweaked to work with fixedpoint without overflowing.
	a = FPM_FROMFLOAT((float)((360.0/65536) * ((int)(FPM_TOFLOAT(a)*(65536/360.0)) & 65535)));

//	a = FPM_MUL(FPM_FROMFLOAT(360.0/65536), FPM_FROMLONG((FPM_TOLONG(FPM_MUL(a,FPM_FROMFLOAT(65536/360.0))) & 65535)));
	return a;
}
#endif

/*
==================
BOPS_Error

Split out like this for ASM to call.
==================
*/
void BOPS_Error (void)
{
	Sys_Error ("BoxOnPlaneSide:  Bad signbits");
}


#if	!id386

/*
==================
BoxOnPlaneSide

Returns 1, 2, or 1 + 2
==================
*/
int BoxOnPlaneSide (vec3_t emins, vec3_t emaxs, mplane_t *p)
{
	float	dist1, dist2;
	int		sides;

#if 0	// this is done by the BOX_ON_PLANE_SIDE macro before calling this
		// function
// fast axial cases
	if (p->type < 3)
	{
		if (p->dist <= emins[p->type])
			return 1;
		if (p->dist >= emaxs[p->type])
			return 2;
		return 3;
	}
#endif

// general case
	switch (p->signbits)
	{
	case 0:
dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
dist2 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		break;
	case 1:
dist1 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		break;
	case 2:
dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
dist2 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		break;
	case 3:
dist1 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		break;
	case 4:
dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
dist2 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		break;
	case 5:
dist1 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		break;
	case 6:
dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
dist2 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		break;
	case 7:
dist1 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		break;
	default:
		dist1 = dist2 = 0;		// shut up compiler
		BOPS_Error ();
		break;
	}

#if 0
	int		i;
	vec3_t	corners[2];

	for (i=0 ; i<3 ; i++)
	{
		if (plane->normal[i] < 0)
		{
			corners[0][i] = emins[i];
			corners[1][i] = emaxs[i];
		}
		else
		{
			corners[1][i] = emins[i];
			corners[0][i] = emaxs[i];
		}
	}
	dist = DotProduct (plane->normal, corners[0]) - plane->dist;
	dist2 = DotProduct (plane->normal, corners[1]) - plane->dist;
	sides = 0;
	if (dist1 >= 0)
		sides = 1;
	if (dist2 < 0)
		sides |= 2;

#endif

	sides = 0;
	if (dist1 >= p->dist)
		sides = 1;
	if (dist2 < p->dist)
		sides |= 2;

#ifdef PARANOID
if (sides == 0)
	Sys_Error ("BoxOnPlaneSide: sides==0");
#endif

	return sides;
}

#ifdef USEFPM
int BoxOnPlaneSideFPM (vec3_FPM_t emins, vec3_FPM_t emaxs, mplane_FPM_t *p)
{
	fixedpoint_t	dist1, dist2;
	int		sides;

// general case
	switch (p->signbits)
	{
	case 0:
		dist1 = DotProductFPM(p->normal, emaxs);
		dist2 = DotProductFPM(p->normal, emins);
		break;
	case 1:
		dist1 = FPM_ADD3(FPM_MUL(p->normal[0],emins[0]),FPM_MUL(p->normal[1],emaxs[1]),FPM_MUL(p->normal[2],emaxs[2]));
		dist2 = FPM_ADD3(FPM_MUL(p->normal[0],emaxs[0]),FPM_MUL(p->normal[1],emins[1]),FPM_MUL(p->normal[2],emins[2]));
		break;
	case 2:
		dist1 = FPM_ADD3(FPM_MUL(p->normal[0],emaxs[0]),FPM_MUL(p->normal[1],emins[1]),FPM_MUL(p->normal[2],emaxs[2]));
		dist2 = FPM_ADD3(FPM_MUL(p->normal[0],emins[0]),FPM_MUL(p->normal[1],emaxs[1]),FPM_MUL(p->normal[2],emins[2]));
		break;
	case 3:
		dist1 = FPM_ADD3(FPM_MUL(p->normal[0],emins[0]),FPM_MUL(p->normal[1],emins[1]),FPM_MUL(p->normal[2],emaxs[2]));
		dist2 = FPM_ADD3(FPM_MUL(p->normal[0],emaxs[0]),FPM_MUL(p->normal[1],emaxs[1]),FPM_MUL(p->normal[2],emins[2]));
		break;
	case 4:
		dist1 = FPM_ADD3(FPM_MUL(p->normal[0],emaxs[0]),FPM_MUL(p->normal[1],emaxs[1]),FPM_MUL(p->normal[2],emins[2]));
		dist2 = FPM_ADD3(FPM_MUL(p->normal[0],emins[0]),FPM_MUL(p->normal[1],emins[1]),FPM_MUL(p->normal[2],emaxs[2]));
		break;
	case 5:
		dist1 = FPM_ADD3(FPM_MUL(p->normal[0],emins[0]),FPM_MUL(p->normal[1],emaxs[1]),FPM_MUL(p->normal[2],emins[2]));
		dist2 = FPM_ADD3(FPM_MUL(p->normal[0],emaxs[0]),FPM_MUL(p->normal[1],emins[1]),FPM_MUL(p->normal[2],emaxs[2]));
		break;
	case 6:
		dist1 = FPM_ADD3(FPM_MUL(p->normal[0],emaxs[0]),FPM_MUL(p->normal[1],emins[1]),FPM_MUL(p->normal[2],emins[2]));
		dist2 = FPM_ADD3(FPM_MUL(p->normal[0],emins[0]),FPM_MUL(p->normal[1],emaxs[1]),FPM_MUL(p->normal[2],emaxs[2]));
		break;
	case 7:
		dist1 = FPM_ADD3(FPM_MUL(p->normal[0],emins[0]),FPM_MUL(p->normal[1],emins[1]),FPM_MUL(p->normal[2],emins[2]));
		dist2 = FPM_ADD3(FPM_MUL(p->normal[0],emaxs[0]),FPM_MUL(p->normal[1],emaxs[1]),FPM_MUL(p->normal[2],emaxs[2]));
		break;
	default:
		dist1 = dist2 = 0;		// shut up compiler
		BOPS_Error ();
		break;
	}


	sides = 0;
	if (dist1 >= p->dist)
		sides = 1;
	if (dist2 < p->dist)
		sides |= 2;

#ifdef PARANOID
	if (sides == 0)
		Sys_Error ("BoxOnPlaneSide: sides==0");
#endif

	return sides;
}
#endif
#endif


void AngleVectors (vec3_t angles, vec3_t forward, vec3_t right, vec3_t up)
{
	float		angle;
	float		sr, sp, sy, cr, cp, cy;

	angle = (float)(angles[YAW] * (M_PI*2 / 360));
	sy = (float)sin(angle);
	cy = (float)cos(angle);
	angle = (float)(angles[PITCH] * (M_PI*2 / 360));
	sp = (float)sin(angle);
	cp = (float)cos(angle);
	angle = (float)(angles[ROLL] * (M_PI*2 / 360));
	sr = (float)sin(angle);
	cr = (float)cos(angle);

	forward[0] = cp*cy;
	forward[1] = cp*sy;
	forward[2] = -sp;
	right[0] = (-1*sr*sp*cy+-1*cr*-sy);
	right[1] = (-1*sr*sp*sy+-1*cr*cy);
	right[2] = -1*sr*cp;
	up[0] = (cr*sp*cy+-sr*-sy);
	up[1] = (cr*sp*sy+-sr*cy);
	up[2] = cr*cp;
}

#ifdef USEFPM
void AngleVectorsFPM (vec3_FPM_t angles, vec3_FPM_t forward, vec3_FPM_t right, vec3_FPM_t up)
{
	double				source;
	double				angle;
//	fixedpoint_t		angle;
//	fixedpoint_t		sr, sp, sy, cr, cp, cy;
	__int64			sr, sp, sy, cr, cp, cy;
	register __int64 /*conv, mul,*/ accum1, accum2, accum1b, accum2b;

	//Dan: ID converted all of these angles to rads for the sin, cos functions.
	//My Fixed Point Math routines can calc trig based on degrees.  So this routine
	//is much more efficient than it was before

	source=FPM_TOFLOAT(FPM_MUL(angles[YAW], FPM_DIV(FPM_PI, FPM_FROMLONGC(180))));
	angle = sin(source);
	sy = (__int64)(angle*(double)(2^32));

	angle = cos(source);
	cy = (__int64)(angle*(double)(2^32));

//	sy = FPM_SIN_DEG(angles[YAW]);
//	cy = FPM_COS_DEG(angles[YAW]);

	source=FPM_TOFLOAT(FPM_MUL(angles[PITCH], FPM_DIV(FPM_PI, FPM_FROMLONGC(180))));
	angle = sin(source);
	sp = (__int64)(angle*(double)(2^32));

	angle = cos(source);
	cp = (__int64)(angle*(double)(2^32));

//	angle = angles[PITCH] * (M_PI*2 / 360);
//	sp = FPM_SIN_DEG(angles[PITCH]);
//	cp = FPM_COS_DEG(angles[PITCH]);
//	angle = angles[ROLL] * (M_PI*2 / 360);

	source=FPM_TOFLOAT(FPM_MUL(angles[ROLL], FPM_DIV(FPM_PI, FPM_FROMLONGC(180))));
	angle = sin(source);
	sr = (__int64)(angle*(double)(2^32));

	angle = cos(source);
	cr = (__int64)(angle*(double)(2^32));

//	sr = FPM_SIN_DEG(angles[ROLL]);
//	cr = FPM_COS_DEG(angles[ROLL]);

	forward[0] = (fixedpoint_t)((cp*cy)>>32);	//FPM_MUL(cp,cy);
	forward[1] = (fixedpoint_t)((cp*sy)>>32);	//FPM_MUL(cp,sy);
	forward[2] = (fixedpoint_t)((-sp)>>16);

	accum1=-sr;
	accum2=-sr;

	accum1*=sp;
	accum2*=sp;

	accum1*=cy;
	accum2b=cy;

	accum1b=-cr;
	accum2b*=-cr;

	accum1b*=-sy;
	accum2*=sy;

	accum1+=accum1b;
	accum2+=accum2b;
	accum1>>=16;
	accum2>>=16;
	right[0]=(fixedpoint_t)accum1;
	right[1]=(fixedpoint_t)accum2;

	//right[0] = FPM_ADD(FPM_MUL(FPM_MUL(FPM_MUL(FPM_FROMLONG(-1),sr),sp),cy),FPM_MUL(FPM_MUL(FPM_FROMLONG(-1),cr),-sy));
	//right[0] = (-1*sr*sp*cy+-1*cr*-sy);
	//right[1] = FPM_ADD(FPM_MUL(FPM_MUL(FPM_MUL(FPM_FROMLONG(-1),sr),sp),sy),FPM_MUL(FPM_MUL(FPM_FROMLONG(-1),cr),cy));
	//right[1] = (-1*sr*sp*sy+-1*cr*cy);
	//right[2] = FPM_MUL(FPM_MUL(FPM_FROMLONG(-1),sr),cp);
	accum1=-sr;
	accum1*=cp;
	accum1>>=16;
	right[2]=(fixedpoint_t)accum1;

	accum1=cr;
	accum1*=sp;
	accum1*=cy;
	accum1b=-sr;
	accum1b*=-sy;
	accum1+=accum1b;
	accum1>>=16;
	up[0] = (fixedpoint_t)accum1;
	//right[2] = -1*sr*cp;
	//up[0] = FPM_ADD(FPM_MUL(FPM_MUL(cr,sp),cy),FPM_MUL(-sr,-sy));
	//up[0] = (cr*sp*cy+-sr*-sy);

	accum1=cr;
	accum1*=sp;
	accum1*=sy;
	accum1b=-sr;
	accum1b*=cy;
	accum1+=accum1b;
	accum1>>=16;
	up[1]=(fixedpoint_t)accum1;

	//up[1] = FPM_ADD(FPM_MUL(FPM_MUL(cr,sp),sy),FPM_MUL(-sr,cy));
	//up[1] = (cr*sp*sy+-sr*cy);

	accum1=cr;
	accum1*=cp;
	accum1>>=16;
	up[2]=(fixedpoint_t)accum1;

	//up[2] = FPM_MUL(cr,cp);
	//up[2] = cr*cp;
}
#endif

int VectorCompare (vec3_t v1, vec3_t v2)
{
	int		i;

	for (i=0 ; i<3 ; i++)
		if (v1[i] != v2[i])
			return 0;

	return 1;
}

#ifdef USEFPM
int VectorCompareFPM (vec3_FPM_t v1, vec3_FPM_t v2)
{
	int		i;

	for (i=0 ; i<3 ; i++)
		if (v1[i] != v2[i])
			return 0;

	return 1;
}
#endif

void VectorMA (vec3_t veca, float scale, vec3_t vecb, vec3_t vecc)
{
	vecc[0] = veca[0] + scale*vecb[0];
	vecc[1] = veca[1] + scale*vecb[1];
	vecc[2] = veca[2] + scale*vecb[2];
}

#ifdef USEFPM
void VectorMAFPM (vec3_FPM_t veca, fixedpoint_t scale, vec3_FPM_t vecb, vec3_FPM_t vecc)
{
	vecc[0] = FPM_ADD(veca[0], FPM_MUL(scale, vecb[0]));
	vecc[1] = FPM_ADD(veca[1], FPM_MUL(scale, vecb[1]));
	vecc[2] = FPM_ADD(veca[2], FPM_MUL(scale, vecb[2]));
}
#endif

//Dan: The following functions do not appear to be used by WinQuake
/*
vec_t _DotProduct (vec3_t v1, vec3_t v2)
{
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

void _VectorSubtract (vec3_t veca, vec3_t vecb, vec3_t out)
{
	out[0] = veca[0]-vecb[0];
	out[1] = veca[1]-vecb[1];
	out[2] = veca[2]-vecb[2];
}

void _VectorAdd (vec3_t veca, vec3_t vecb, vec3_t out)
{
	out[0] = veca[0]+vecb[0];
	out[1] = veca[1]+vecb[1];
	out[2] = veca[2]+vecb[2];
}

void _VectorCopy (vec3_t in, vec3_t out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
}
*/

void CrossProduct (vec3_t v1, vec3_t v2, vec3_t cross)
{
	cross[0] = v1[1]*v2[2] - v1[2]*v2[1];
	cross[1] = v1[2]*v2[0] - v1[0]*v2[2];
	cross[2] = v1[0]*v2[1] - v1[1]*v2[0];
}

#ifdef USEFPM
void CrossProductFPM (vec3_FPM_t v1, vec3_FPM_t v2, vec3_FPM_t cross)
{
	cross[0] = FPM_SUB(FPM_MUL(v1[1], v2[2]), FPM_MUL(v1[2], v2[1]));
	cross[1] = FPM_SUB(FPM_MUL(v1[2], v2[0]), FPM_MUL(v1[0], v2[2]));
	cross[2] = FPM_SUB(FPM_MUL(v1[0], v2[1]), FPM_MUL(v1[1], v2[0]));
}
#endif

vec_t Length(vec3_t v)
{
	int		i;
	float	length;

	length = 0;
	for (i=0 ; i< 3 ; i++)
		length += v[i]*v[i];
	length = (float)sqrt (length);		// FIXME

	return length;
}

#ifdef USEFPM
fixedpoint_t LengthFPM(vec3_FPM_t v)
{
	int		i;
	//Dan: squaring the vectors results in overflow, so we're hardcoding 64 bit 16.16 fixedpoint:
	__int64			length, tmp;

//	fixedpoint_t	length;

	length = 0;
	for (i=0 ; i< 3 ; i++) {
		tmp=v[i];
		length += (tmp*tmp)>>16;
	}
	return (fixedpoint_t)sqrt (length/65536.0)*65536;		// FIXME (not Dan's comment)

//	return length;
}
#endif

float VectorNormalize (vec3_t v)
{
	float	length, ilength;

	length = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
	length = (float)sqrt (length);		// FIXME

	if (length)
	{
		ilength = 1/length;
		v[0] *= ilength;
		v[1] *= ilength;
		v[2] *= ilength;
	}

	return length;

}

#ifdef USEFPM
fixedpoint_t VectorNormalizeFPM (vec3_FPM_t v)
{
	fixedpoint_t	length;//, ilength;

	length = FPM_ADD3(FPM_MUL(v[0],v[0]),FPM_MUL(v[1],v[1]),FPM_MUL(v[2],v[2]));
	length = FPM_SQRT (length);		// FIXME (not dan's comment)

	if (length)
	{
		//ilength = FPM_INV(length);
		v[0]=FPM_DIV(v[0], length);	//FPM_MUL(v[0], ilength);
		v[1]=FPM_DIV(v[1], length);	//FPM_MUL(v[1], ilength);
		v[2]=FPM_DIV(v[2], length);	//FPM_MUL(v[2], ilength);
	}

	return length;

}
#endif

void VectorInverse (vec3_t v)
{
	v[0] = -v[0];
	v[1] = -v[1];
	v[2] = -v[2];
}

#ifdef USEFPM
void VectorInverseFPM (vec3_FPM_t v)
{
	v[0] = -v[0];
	v[1] = -v[1];
	v[2] = -v[2];
}
#endif

void VectorScale (vec3_t in, vec_t scale, vec3_t out)
{
	out[0] = in[0]*scale;
	out[1] = in[1]*scale;
	out[2] = in[2]*scale;
}

#ifdef USEFPM
void VectorScaleFPM (vec3_FPM_t in, fixedpoint_t scale, vec3_FPM_t out)
{
	out[0] = FPM_MUL(in[0], scale);
	out[1] = FPM_MUL(in[1], scale);
	out[2] = FPM_MUL(in[2], scale);
}
#endif

int Q_log2(int val)
{
	int answer=0;
	while (val>>=1)
		answer++;
	return answer;
}


/*
================
R_ConcatRotations
================
*/
void R_ConcatRotations (float in1[3][3], float in2[3][3], float out[3][3])
{
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] +
				in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] +
				in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] +
				in1[0][2] * in2[2][2];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] +
				in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] +
				in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] +
				in1[1][2] * in2[2][2];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] +
				in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] +
				in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] +
				in1[2][2] * in2[2][2];
}

#ifdef USEFPM
void R_ConcatRotationsFPM (fixedpoint_t in1[3][3], fixedpoint_t in2[3][3], fixedpoint_t out[3][3])
{
	out[0][0] = FPM_ADD3(FPM_MUL(in1[0][0],in2[0][0]), FPM_MUL(in1[0][1],in2[1][0]),
				FPM_MUL(in1[0][2],in2[2][0]));
	out[0][1] = FPM_ADD3(FPM_MUL(in1[0][0],in2[0][1]), FPM_MUL(in1[0][1],in2[1][1]),
				FPM_MUL(in1[0][2],in2[2][1]));
	out[0][2] = FPM_ADD3(FPM_MUL(in1[0][0],in2[0][2]), FPM_MUL(in1[0][1],in2[1][2]),
				FPM_MUL(in1[0][2],in2[2][2]));
	out[1][0] = FPM_ADD3(FPM_MUL(in1[1][0],in2[0][0]), FPM_MUL(in1[1][1],in2[1][0]),
				FPM_MUL(in1[1][2],in2[2][0]));
	out[1][1] = FPM_ADD3(FPM_MUL(in1[1][0],in2[0][1]), FPM_MUL(in1[1][1],in2[1][1]),
				FPM_MUL(in1[1][2],in2[2][1]));
	out[1][2] = FPM_ADD3(FPM_MUL(in1[1][0],in2[0][2]), FPM_MUL(in1[1][1],in2[1][2]),
				FPM_MUL(in1[1][2],in2[2][2]));
	out[2][0] = FPM_ADD3(FPM_MUL(in1[2][0],in2[0][0]), FPM_MUL(in1[2][1],in2[1][0]),
				FPM_MUL(in1[2][2],in2[2][0]));
	out[2][1] = FPM_ADD3(FPM_MUL(in1[2][0],in2[0][1]), FPM_MUL(in1[2][1],in2[1][1]),
				FPM_MUL(in1[2][2],in2[2][1]));
	out[2][2] = FPM_ADD3(FPM_MUL(in1[2][0],in2[0][2]), FPM_MUL(in1[2][1],in2[1][2]),
				FPM_MUL(in1[2][2],in2[2][2]));
}
#endif

/*
================
R_ConcatTransforms
================
*/
void R_ConcatTransforms (float in1[3][4], float in2[3][4], float out[3][4])
{
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] +
				in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] +
				in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] +
				in1[0][2] * in2[2][2];
	out[0][3] = in1[0][0] * in2[0][3] + in1[0][1] * in2[1][3] +
				in1[0][2] * in2[2][3] + in1[0][3];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] +
				in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] +
				in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] +
				in1[1][2] * in2[2][2];
	out[1][3] = in1[1][0] * in2[0][3] + in1[1][1] * in2[1][3] +
				in1[1][2] * in2[2][3] + in1[1][3];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] +
				in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] +
				in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] +
				in1[2][2] * in2[2][2];
	out[2][3] = in1[2][0] * in2[0][3] + in1[2][1] * in2[1][3] +
				in1[2][2] * in2[2][3] + in1[2][3];
}

#ifdef USEFPM
void R_ConcatTransformsFPM (fixedpoint_t in1[3][4], fixedpoint_t in2[3][4], fixedpoint_t out[3][4])
{
	register __int64 accum, mul;
	int i;
	for (i=0; i<3; i++) {
		accum=in1[i][0];
		accum*=in2[0][0];
		mul=in1[i][1];
		mul*=in2[1][0];
		accum+=mul;
		mul=in1[i][2];
		mul*=in2[2][0];
		accum+=mul;
		accum>>=16;
		out[i][0]=(fixedpoint_t)accum;

		accum=in1[i][0];
		accum*=in2[0][1];
		mul=in1[i][1];
		mul*=in2[1][1];
		accum+=mul;
		mul=in1[i][2];
		mul*=in2[2][1];
		accum+=mul;
		accum>>=16;
		out[i][1]=(fixedpoint_t)accum;

		accum=in1[i][0];
		accum*=in2[0][2];
		mul=in1[i][1];
		mul*=in2[1][2];
		accum+=mul;
		mul=in1[i][2];
		mul*=in2[2][2];
		accum+=mul;
		accum>>=16;
		out[i][2]=(fixedpoint_t)accum;

		accum=in1[i][0];
		accum*=in2[0][3];
		mul=in1[i][1];
		mul*=in2[1][3];
		accum+=mul;
		mul=in1[i][2];
		mul*=in2[2][3];
		accum+=mul;
		mul=in1[0][3];
		mul<<=16;	//16.16 -> 32.32
		accum+=mul;
		accum>>=16;
		out[i][3]=(fixedpoint_t)accum;
	}
}
#endif

/*
void R_ConcatTransformsFPM (fixedpoint_t in1[3][4], fixedpoint_t in2[3][4], fixedpoint_t out[3][4])
{
	out[0][0] = FPM_ADD3(FPM_MUL(in1[0][0],in2[0][0]),FPM_MUL(in1[0][1],in2[1][0]),
				FPM_MUL(in1[0][2],in2[2][0]));
	out[0][1] = FPM_ADD3(FPM_MUL(in1[0][0],in2[0][1]),FPM_MUL(in1[0][1],in2[1][1]),
				FPM_MUL(in1[0][2],in2[2][1]));
	out[0][2] = FPM_ADD3(FPM_MUL(in1[0][0],in2[0][2]),FPM_MUL(in1[0][1],in2[1][2]),
				FPM_MUL(in1[0][2],in2[2][2]));
	out[0][3] = FPM_ADD(FPM_ADD3(FPM_MUL(in1[0][0],in2[0][3]),FPM_MUL(in1[0][1],in2[1][3]),
				FPM_MUL(in1[0][2],in2[2][3])),in1[0][3]);
	out[1][0] = FPM_ADD3(FPM_MUL(in1[1][0],in2[0][0]),FPM_MUL(in1[1][1],in2[1][0]),
				FPM_MUL(in1[1][2],in2[2][0]));
	out[1][1] = FPM_ADD3(FPM_MUL(in1[1][0],in2[0][1]),FPM_MUL(in1[1][1],in2[1][1]),
				FPM_MUL(in1[1][2],in2[2][1]));
	out[1][2] = FPM_ADD3(FPM_MUL(in1[1][0],in2[0][2]),FPM_MUL(in1[1][1],in2[1][2]),
				FPM_MUL(in1[1][2],in2[2][2]));
	out[1][3] = FPM_ADD(FPM_ADD3(FPM_MUL(in1[1][0],in2[0][3]),FPM_MUL(in1[1][1],in2[1][3]),
				FPM_MUL(in1[1][2],in2[2][3])),in1[1][3]);
	out[2][0] = FPM_ADD3(FPM_MUL(in1[2][0],in2[0][0]),FPM_MUL(in1[2][1],in2[1][0]),
				FPM_MUL(in1[2][2],in2[2][0]));
	out[2][1] = FPM_ADD3(FPM_MUL(in1[2][0],in2[0][1]),FPM_MUL(in1[2][1],in2[1][1]),
				FPM_MUL(in1[2][2],in2[2][1]));
	out[2][2] = FPM_ADD3(FPM_MUL(in1[2][0],in2[0][2]),FPM_MUL(in1[2][1],in2[1][2]),
				FPM_MUL(in1[2][2],in2[2][2]));
	out[2][3] = FPM_ADD(FPM_ADD3(FPM_MUL(in1[2][0],in2[0][3]),FPM_MUL(in1[2][1],in2[1][3]),
				FPM_MUL(in1[2][2],in2[2][3])),in1[2][3]);
}
*/

#ifdef USEFPM
void R_ConcatTransforms8_24FPM (fixedpoint_t in1[3][4], fixedpoint_t in2[3][4], fixedpoint8_24_t out[3][4])
{
	int i;
	register __int64 accum, i1, i2, i3, conv;

	for (i=0; i<3; i++) {
		accum=0;
		i1=in1[i][0];
		i1<<=16;
		i2=in1[i][1];
		i2<<=16;
		i3=in1[i][2];
		i3<<=16;

		conv=in2[0][0];
		conv<<=16;
		conv*=i1;
		accum=conv>>16;
		conv=in2[1][0];
		conv<<=16;
		conv*=i2;
		accum+=conv>>16;
		conv=in2[2][0];
		conv<<=16;
		conv*=i3;
		accum+=conv>>16;
		out[i][0] = (fixedpoint8_24_t)accum>>8;

		conv=in2[0][1];
		conv<<=16;
		conv*=i1;
		accum=conv>>16;
		conv=in2[1][1];
		conv<<=16;
		conv*=i2;
		accum+=conv>>16;
		conv=in2[2][1];
		conv<<=16;
		conv*=i3;
		accum+=conv>>16;
		out[i][1] = (fixedpoint8_24_t)accum>>8;

		conv=in2[0][2];
		conv<<=16;
		conv*=i1;
		accum=conv>>16;
		conv=in2[1][2];
		conv<<=16;
		conv*=i2;
		accum+=conv>>16;
		conv=in2[2][2];
		conv<<=16;
		conv*=i3;
		accum+=conv>>16;
		out[i][2] = (fixedpoint8_24_t)accum>>8;		//to 8.24

		conv=in2[0][3];
		conv<<=16;
		conv*=i1;
		accum=conv>>16;
		conv=in2[1][3];
		conv<<=16;
		conv*=i2;
		accum+=conv>>16;
		conv=in2[2][3];
		conv<<=16;
		conv*=i3;
		accum+=conv>>16;
		conv=in1[0][3];
		conv<<=16;
		accum+=conv;
		out[i][3] = (fixedpoint8_24_t)accum>>16;	//to 16.16
	}
}
#endif

/*
void R_ConcatTransforms8_24FPM (fixedpoint_t in1[3][4], fixedpoint_t in2[3][4], fixedpoint8_24_t out[3][4])
{
	out[0][0] = fpm_FromFixedPoint(FPM_ADD3(FPM_MUL(in1[0][0],in2[0][0]),FPM_MUL(in1[0][1],in2[1][0]),
				FPM_MUL(in1[0][2],in2[2][0])));
	out[0][1] = fpm_FromFixedPoint(FPM_ADD3(FPM_MUL(in1[0][0],in2[0][1]),FPM_MUL(in1[0][1],in2[1][1]),
				FPM_MUL(in1[0][2],in2[2][1])));
	out[0][2] = fpm_FromFixedPoint(FPM_ADD3(FPM_MUL(in1[0][0],in2[0][2]),FPM_MUL(in1[0][1],in2[1][2]),
				FPM_MUL(in1[0][2],in2[2][2])));
	out[0][3] = (fixedpoint_t)FPM_ADD(FPM_ADD3(FPM_MUL(in1[0][0],in2[0][3]),FPM_MUL(in1[0][1],in2[1][3]),
				FPM_MUL(in1[0][2],in2[2][3])),in1[0][3]);
	out[1][0] = fpm_FromFixedPoint(FPM_ADD3(FPM_MUL(in1[1][0],in2[0][0]),FPM_MUL(in1[1][1],in2[1][0]),
				FPM_MUL(in1[1][2],in2[2][0])));
	out[1][1] = fpm_FromFixedPoint(FPM_ADD3(FPM_MUL(in1[1][0],in2[0][1]),FPM_MUL(in1[1][1],in2[1][1]),
				FPM_MUL(in1[1][2],in2[2][1])));
	out[1][2] = fpm_FromFixedPoint(FPM_ADD3(FPM_MUL(in1[1][0],in2[0][2]),FPM_MUL(in1[1][1],in2[1][2]),
				FPM_MUL(in1[1][2],in2[2][2])));
	out[1][3] = (fixedpoint_t)FPM_ADD(FPM_ADD3(FPM_MUL(in1[1][0],in2[0][3]),FPM_MUL(in1[1][1],in2[1][3]),
				FPM_MUL(in1[1][2],in2[2][3])),in1[1][3]);
	out[2][0] = fpm_FromFixedPoint(FPM_ADD3(FPM_MUL(in1[2][0],in2[0][0]),FPM_MUL(in1[2][1],in2[1][0]),
				FPM_MUL(in1[2][2],in2[2][0])));
	out[2][1] = fpm_FromFixedPoint(FPM_ADD3(FPM_MUL(in1[2][0],in2[0][1]),FPM_MUL(in1[2][1],in2[1][1]),
				FPM_MUL(in1[2][2],in2[2][1])));
	out[2][2] = fpm_FromFixedPoint(FPM_ADD3(FPM_MUL(in1[2][0],in2[0][2]),FPM_MUL(in1[2][1],in2[1][2]),
				FPM_MUL(in1[2][2],in2[2][2])));
	out[2][3] = (fixedpoint_t)FPM_ADD(FPM_ADD3(FPM_MUL(in1[2][0],in2[0][3]),FPM_MUL(in1[2][1],in2[1][3]),
				FPM_MUL(in1[2][2],in2[2][3])),in1[2][3]);
}
*/

/*
===================
FloorDivMod

Returns mathematically correct (floor-based) quotient and remainder for
numer and denom, both of which should contain no fractional part. The
quotient must fit in 32 bits.
====================
*/

void FloorDivMod (double numer, double denom, int *quotient,
		int *rem)
{
	int		q, r;
	double	x;

#ifndef PARANOID
	if (denom <= 0.0)
		Sys_Error ("FloorDivMod: bad denominator %d\n", denom);

//	if ((floor(numer) != numer) || (floor(denom) != denom))
//		Sys_Error ("FloorDivMod: non-integer numer or denom %f %f\n",
//				numer, denom);
#endif

	if (numer >= 0.0)
	{

		x = floor(numer / denom);
		q = (int)x;
		r = (int)floor(numer - (x * denom));
	}
	else
	{
	//
	// perform operations with positive values, and fix mod to make floor-based
	//
		x = floor(-numer / denom);
		q = -(int)x;
		r = (int)floor(-numer - (x * denom));
		if (r != 0)
		{
			q--;
			r = (int)denom - r;
		}
	}

	*quotient = q;
	*rem = r;
}


/*
===================
GreatestCommonDivisor
====================
*/
int GreatestCommonDivisor (int i1, int i2)
{
	if (i1 > i2)
	{
		if (i2 == 0)
			return (i1);
		return GreatestCommonDivisor (i2, i1 % i2);
	}
	else
	{
		if (i1 == 0)
			return (i2);
		return GreatestCommonDivisor (i1, i2 % i1);
	}
}


#if	!id386

// TODO: move to nonintel.c

/*
===================
Invert24To16

Inverts an 8.24 value to a 16.16 value
====================
*/

fixed16_t Invert24To16(fixed16_t val)
{
	if (val < 256)
		return (0xFFFFFFFF);

	return (fixed16_t)
			(((double)0x10000 * (double)0x1000000 / (double)val) + 0.5);
}

int ParseFloats(char *s, float *f, int *f_size) { 
   int i, argc; 

   if (!s || !f || !f_size) 
      Sys_Error("ParseFloats() wrong params"); 

   if (f_size[0] <= 0) 
      return (f_size[0] = 0);

   Cmd_TokenizeString(s); 
   // argc = min(Cmd_Argc(), f_size[0]); 
   argc = (Cmd_Argc(), f_size[0]); 
    
   for(i = 0; i < argc; i++) 
      f[i] = Q_atof(Cmd_Argv(i)); 

   for( ; i < f_size[0]; i++) 
      f[i] = 0;

   return (f_size[0] = argc); 
}

#endif
