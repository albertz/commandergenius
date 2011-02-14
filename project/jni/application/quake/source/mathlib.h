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
// mathlib.h

#ifndef __MATHLIB_H__
#define __MATHLIB_H__

//Dan East:
#include "FixedPointMath.h"

#ifdef USE_PQ_OPT
typedef int fpvec3[3];
#endif

typedef fixedpoint_t vec3_FPM_t[3];
typedef fixedpoint8_24_t vec3_8_24FPM_t[3];
typedef fixedpoint_t vec5_FPM_t[5];

//End Dan

typedef float vec_t;
typedef vec_t vec3_t[3];
typedef vec_t vec5_t[5];

typedef	int	fixed4_t;
typedef	int	fixed8_t;
typedef	int	fixed16_t;

#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

//struct mplane_s;

extern vec3_t		vec3_origin;
extern vec3_FPM_t	vec3_originFPM;
extern	int			nanmask;

#define	IS_NAN(x) (((*(int *)&x)&nanmask)==nanmask)

//Dan East:
#define DotProductFPM(x,y) FPM_ADD3(FPM_MUL(x[0],y[0]),FPM_MUL(x[1],y[1]),FPM_MUL(x[2],y[2]))
#define DotProduct8_24FPM(x,y) fpm_Add3(fpm_MulMixed8_24(x[0],y[0]),fpm_MulMixed8_24(x[1],y[1]),fpm_MulMixed8_24(x[2],y[2]))
#define VectorSubtractFPM(a,b,c) {c[0]=FPM_SUB(a[0],b[0]);c[1]=FPM_SUB(a[1],b[1]);c[2]=FPM_SUB(a[2],b[2]);}
#define VectorAddFPM(a,b,c) {c[0]=FPM_ADD(a[0],b[0]);c[1]=FPM_ADD(a[1],b[1]);c[2]=FPM_ADD(a[2],b[2]);}

//Int the following two macros b represents the destination (following ID's convention)
#define VectorToFPM(a,b) {b[0]=FPM_FROMFLOAT(a[0]);b[1]=FPM_FROMFLOAT(a[1]);b[2]=FPM_FROMFLOAT(a[2]);}
#define FPMToVector(a,b) {b[0]=FPM_TOFLOAT(a[0]);b[1]=FPM_TOFLOAT(a[1]);b[2]=FPM_TOFLOAT(a[2]);}

//End Dan


#define DotProduct(x,y) (x[0]*y[0]+x[1]*y[1]+x[2]*y[2])
#define VectorSubtract(a,b,c) {c[0]=a[0]-b[0];c[1]=a[1]-b[1];c[2]=a[2]-b[2];}
#define VectorAdd(a,b,c) {c[0]=a[0]+b[0];c[1]=a[1]+b[1];c[2]=a[2]+b[2];}
//Dan: Can't this be done with memcpy?
#define VectorCopy(a,b) {b[0]=a[0];b[1]=a[1];b[2]=a[2];}

void VectorMA (vec3_t veca, float scale, vec3_t vecb, vec3_t vecc);
void VectorMAFPM (vec3_FPM_t veca, fixedpoint_t scale, vec3_FPM_t vecb, vec3_FPM_t vecc);


int VectorCompare (vec3_t v1, vec3_t v2);
vec_t Length (vec3_t v);
fixedpoint_t LengthFPM (vec3_FPM_t v);
void CrossProduct (vec3_t v1, vec3_t v2, vec3_t cross);
void CrossProductFPM (vec3_FPM_t v1, vec3_FPM_t v2, vec3_FPM_t cross);
float VectorNormalize (vec3_t v);		// returns vector length
fixedpoint_t VectorNormalizeFPM (vec3_FPM_t v);		// returns vector length
void VectorInverse (vec3_t v);
void VectorInverseFPM (vec3_FPM_t v);
void VectorScale (vec3_t in, vec_t scale, vec3_t out);
void VectorScaleFPM (vec3_FPM_t in, fixedpoint_t scale, vec3_FPM_t out);
int Q_log2(int val);

void R_ConcatRotations (float in1[3][3], float in2[3][3], float out[3][3]);
void R_ConcatRotationsFPM (fixedpoint_t in1[3][3], fixedpoint_t in2[3][3], fixedpoint_t out[3][3]);
void R_ConcatTransforms (float in1[3][4], float in2[3][4], float out[3][4]);
void R_ConcatTransformsFPM (fixedpoint_t in1[3][4], fixedpoint_t in2[3][4], fixedpoint_t out[3][4]);
void R_ConcatTransforms8_24FPM (fixedpoint_t in1[3][4], fixedpoint_t in2[3][4], fixedpoint8_24_t out[3][4]);

void FloorDivMod (double numer, double denom, int *quotient,
		int *rem);
fixed16_t Invert24To16(fixed16_t val);
int GreatestCommonDivisor (int i1, int i2);

void AngleVectors (vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
void AngleVectorsFPM (vec3_FPM_t angles, vec3_FPM_t forward, vec3_FPM_t right, vec3_FPM_t up);
//int BoxOnPlaneSide (vec3_t emins, vec3_t emaxs, struct mplane_s *plane);
//int BoxOnPlaneSideFPM (vec3_FPM_t emins, vec3_FPM_t emaxs, struct mplane_FPM_s *plane);
float			anglemod(float a);
fixedpoint_t	anglemodFPM(fixedpoint_t a);

#define BOX_ON_PLANE_SIDE(emins, emaxs, p)	\
	(((p)->type < 3)?						\
	(										\
		((p)->dist <= (emins)[(p)->type])?	\
			1								\
		:									\
		(									\
			((p)->dist >= (emaxs)[(p)->type])?\
				2							\
			:								\
				3							\
		)									\
	)										\
	:										\
		BoxOnPlaneSide( (emins), (emaxs), (p)))

#define BOX_ON_PLANE_SIDE_FPM(emins, emaxs, p)	\
	(((p)->type < 3)?						\
	(										\
		((p)->dist <= (emins)[(p)->type])?	\
			1								\
		:									\
		(									\
			((p)->dist >= (emaxs)[(p)->type])?\
				2							\
			:								\
				3							\
		)									\
	)										\
	:										\
		BoxOnPlaneSideFPM( (emins), (emaxs), (p)))

int ParseFloats(char *s, float *f, int *f_size);

#endif // _MATHLIB_H_
