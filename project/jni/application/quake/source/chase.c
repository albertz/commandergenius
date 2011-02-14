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
// chase.c -- chase camera code

#include "quakedef.h"

cvar_t	chase_back = {"chase_back", "100"};
cvar_t	chase_up = {"chase_up", "16"};
cvar_t	chase_right = {"chase_right", "0"};
cvar_t	chase_active = {"chase_active", "0"};

vec3_t	chase_pos;
vec3_t	chase_angles;

vec3_t	chase_dest;
vec3_t	chase_dest_angles;

#ifdef USEFPM
vec3_FPM_t	chase_posFPM;
vec3_FPM_t	chase_anglesFPM;

vec3_FPM_t	chase_destFPM;
vec3_FPM_t	chase_dest_anglesFPM;
#endif

void Chase_Init (void)
{
	Cvar_RegisterVariable (&chase_back);
	Cvar_RegisterVariable (&chase_up);
	Cvar_RegisterVariable (&chase_right);
	Cvar_RegisterVariable (&chase_active);
}

void Chase_Reset (void)
{
	// for respawning and teleporting
//	start position 12 units behind head
}

//Dan: Added forward declaration to prevent compiler warning
qboolean SV_RecursiveHullCheck (hull_t *hull, int num, float p1f, float p2f, vec3_t p1, vec3_t p2, trace_t *trace);

void TraceLine (vec3_t start, vec3_t end, vec3_t impact)
{
	trace_t	trace;

	Q_memset (&trace, 0, sizeof(trace));
	SV_RecursiveHullCheck (cl.worldmodel->hulls, 0, 0, 1, start, end, &trace);

	VectorCopy (trace.endpos, impact);
}

#ifdef USEFPM
void TraceLineFPM (vec3_FPM_t start, vec3_FPM_t end, vec3_FPM_t impact)
{
	trace_FPM_t	trace;

	Q_memset (&trace, 0, sizeof(trace));
	//Dan: TODO: This links us to the world of networked quake...
	//SV_RecursiveHullCheckFPM (clFPM.worldmodel->hulls, 0, 0, 1, start, end, &trace);

	VectorCopy (trace.endpos, impact);
}
#endif

void Chase_Update (void)
{
	int		i;
	float	dist;
	vec3_t	forward, up, right;
	vec3_t	dest, stop;


	// if can't see player, reset
	AngleVectors (cl.viewangles, forward, right, up);

	// calc exact destination
	for (i=0 ; i<3 ; i++)
		chase_dest[i] = r_refdef.vieworg[i]
		- forward[i]*chase_back.value
		- right[i]*chase_right.value;
	chase_dest[2] = r_refdef.vieworg[2] + chase_up.value;

	// find the spot the player is looking at
	VectorMA (r_refdef.vieworg, 4096, forward, dest);
	TraceLine (r_refdef.vieworg, dest, stop);

	// calculate pitch to look at the same spot from camera
	VectorSubtract (stop, r_refdef.vieworg, stop);
	dist = DotProduct (stop, forward);
	if (dist < 1)
		dist = 1;
	r_refdef.viewangles[PITCH] = (float)(-atan(stop[2] / dist) / M_PI * 180);

	// move towards destination
	VectorCopy (chase_dest, r_refdef.vieworg);
}

#ifdef USEFPM
void Chase_UpdateFPM (void)
{
	int				i;
	fixedpoint_t	dist;
	vec3_FPM_t		forward, up, right;
	vec3_FPM_t		dest, stop;


	// if can't see player, reset
	AngleVectorsFPM (clFPM.viewangles, forward, right, up);

	// calc exact destination
	for (i=0 ; i<3 ; i++)
		chase_destFPM[i] = FPM_SUB(FPM_SUB(r_refdefFPM.vieworg[i],
			FPM_MUL(forward[i],FPM_FROMFLOAT(chase_back.value))),
			FPM_MUL(right[i],FPM_FROMFLOAT(chase_right.value)));
	chase_destFPM[2] = FPM_ADD(r_refdefFPM.vieworg[2], FPM_FROMFLOAT(chase_up.value));

	// find the spot the player is looking at
	VectorMAFPM (r_refdefFPM.vieworg, FPM_FROMLONG(4096), forward, dest);
	TraceLineFPM (r_refdefFPM.vieworg, dest, stop);

	// calculate pitch to look at the same spot from camera
	VectorSubtractFPM (stop, r_refdefFPM.vieworg, stop);
	dist = DotProductFPM (stop, forward);
	if (dist < FPM_FROMLONG(1))
		dist = FPM_FROMLONG(1);
	r_refdefFPM.viewangles[PITCH] = -FPM_MUL(FPM_DIV(FPM_ATAN(FPM_DIV(stop[2], dist)), FPM_PI), FPM_FROMLONG(180));

	// move towards destination
	VectorCopy (chase_destFPM, r_refdefFPM.vieworg);
}
#endif
