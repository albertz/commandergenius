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
// view.c -- player eye positioning

#include "quakedef.h"
#include "r_local.h"

/*

The view is allowed to move slightly from it's true position for bobbing,
but if it exceeds 8 pixels linear distance (spherical, not box), the list of
entities sent from the server may not include everything in the pvs, especially
when crossing a water boudnary.

*/

cvar_t		lcd_x = {"lcd_x","0"};
cvar_t		lcd_yaw = {"lcd_yaw","0"};

cvar_t	scr_ofsx = {"scr_ofsx","0", false};
cvar_t	scr_ofsy = {"scr_ofsy","0", false};
cvar_t	scr_ofsz = {"scr_ofsz","0", false};

cvar_t	cl_rollspeed = {"cl_rollspeed", "200"};
cvar_t	cl_rollangle = {"cl_rollangle", "2.0"};

cvar_t	cl_bob = {"cl_bob","0.02", false};
cvar_t	cl_bobcycle = {"cl_bobcycle","0.6", false};
cvar_t	cl_bobup = {"cl_bobup","0.5", false};

cvar_t	v_kicktime = {"v_kicktime", "0.5", false};
cvar_t	v_kickroll = {"v_kickroll", "0.6", false};
cvar_t	v_kickpitch = {"v_kickpitch", "0.6", false};

cvar_t	v_iyaw_cycle = {"v_iyaw_cycle", "2", false};
cvar_t	v_iroll_cycle = {"v_iroll_cycle", "0.5", false};
cvar_t	v_ipitch_cycle = {"v_ipitch_cycle", "1", false};
cvar_t	v_iyaw_level = {"v_iyaw_level", "0.3", false};
cvar_t	v_iroll_level = {"v_iroll_level", "0.1", false};
cvar_t	v_ipitch_level = {"v_ipitch_level", "0.3", false};

cvar_t	v_idlescale = {"v_idlescale", "0", false};

cvar_t	crosshair = {"crosshair", "1", true};
cvar_t	cl_crossx = {"cl_crossx", "-4", false};
cvar_t	cl_crossy = {"cl_crossy", "0", false};

cvar_t r_viewmodeloffset = {"r_viewmodeloffset", "5", true};

cvar_t	gl_cshiftpercent = {"gl_cshiftpercent", "100", false};

float			v_dmg_time, v_dmg_roll, v_dmg_pitch;

#ifdef USEFPM
fixedpoint_t	v_dmg_timeFPM, v_dmg_rollFPM, v_dmg_pitchFPM;
#endif //USEFPM

extern	int			in_forward, in_forward2, in_back;


/*
===============
V_CalcRoll

Used by view and sv_user
===============
*/
vec3_t		forward, right, up;

#ifdef USEFPM
vec3_FPM_t	forwardFPM, rightFPM, upFPM;
#endif //USEFPM

float V_CalcRoll (vec3_t angles, vec3_t velocity)
{
	float	sign;
	float	side;
	float	value;

	AngleVectors (angles, forward, right, up);
	side = DotProduct (velocity, right);
	sign = (float)((side < 0 ? -1.0 : 1.0));
	side = (float)fabs(side);

	value = cl_rollangle.value;
//	if (cl.inwater)
//		value *= 6;

	if (side < cl_rollspeed.value)
		side = side * value / cl_rollspeed.value;
	else
		side = value;

	return side*sign;
}

#ifdef USEFPM
fixedpoint_t V_CalcRollFPM (vec3_FPM_t angles, vec3_FPM_t velocity)
{
	fixedpoint_t	sign;
	fixedpoint_t	side;
	fixedpoint_t	value;
	fixedpoint_t	cl_rollspeed_value = FPM_FROMFLOAT(cl_rollspeed.value);

	AngleVectorsFPM (angles, forwardFPM, rightFPM, upFPM);
	side = DotProductFPM (velocity, rightFPM);
	sign = side < 0 ? FPM_FROMLONG(-1) : FPM_FROMLONG(1);
	side = FPM_ABS(side);

	value = FPM_FROMFLOAT(cl_rollangle.value);
//	if (cl.inwater)
//		value *= 6;

	if (side < cl_rollspeed_value) {
		//Dan: 16.16 overflow / underflow problem area
		__int64 i1, i2;
		i1=side;
		i1*=value;
		i1<<=16;
		i2=cl_rollspeed_value;
		i2<<=16;
		i1/=i2;
		i1>>=16;
		side=(fixedpoint_t)i1;
		//side = FPM_DIV(FPM_MUL(side, value), cl_rollspeed_value);
	} else
		side = value;

	return FPM_MUL(side,sign);
}
#endif //USEFPM

/*
===============
V_CalcBob

===============
*/
float V_CalcBob (void)
{
	float	bob;
	float	cycle;

	cycle = (float)(cl.time - (int)(cl.time/cl_bobcycle.value)*cl_bobcycle.value);
	cycle /= cl_bobcycle.value;
	if (cycle < cl_bobup.value)
		cycle = (float)(M_PI * cycle / cl_bobup.value);
	else
		cycle = (float)(M_PI + M_PI*(cycle-cl_bobup.value)/(1.0 - cl_bobup.value));

// bob is proportional to velocity in the xy plane
// (don't count Z, or jumping messes it up)

	bob = (float)(sqrt(cl.velocity[0]*cl.velocity[0] + cl.velocity[1]*cl.velocity[1]) * cl_bob.value);
//Con_Printf ("speed: %5.1f\n", Length(cl.velocity));
	bob = (float)(bob*0.3 + bob*0.7*sin(cycle));
	if (bob > 4)
		bob = 4;
	else if (bob < -7)
		bob = -7;
	return bob;

}

#ifdef USEFPM
fixedpoint_t V_CalcBobFPM (void)
{
	float	bob;
	float	cycle;

	cycle = (float)(clFPM.time - (int)(clFPM.time/cl_bobcycle.value)*cl_bobcycle.value);
	cycle /= cl_bobcycle.value;
	if (cycle < cl_bobup.value)
		cycle = (float)(M_PI * cycle / cl_bobup.value);
	else
		cycle = (float)(M_PI + M_PI*(cycle-cl_bobup.value)/(1.0 - cl_bobup.value));

// bob is proportional to velocity in the xy plane
// (don't count Z, or jumping messes it up)

	bob = (float)(sqrt(clFPM.velocity[0]*clFPM.velocity[0] + clFPM.velocity[1]*clFPM.velocity[1]) * cl_bob.value);
//Con_Printf ("speed: %5.1f\n", Length(cl.velocity));
	bob = (float)(bob*0.3 + bob*0.7*sin(cycle));
	if (bob > 4)
		bob = 4;
	else if (bob < -7)
		bob = -7;
	return FPM_FROMFLOAT(bob);

}
#endif //USEFPM
//=============================================================================


cvar_t	v_centermove = {"v_centermove", "0.15", false};
cvar_t	v_centerspeed = {"v_centerspeed","500"};


void V_StartPitchDrift (void)
{
#if 1
	if (cl.laststop == cl.time)
	{
		return;		// something else is keeping it from drifting
	}
#endif
	if (cl.nodrift || !cl.pitchvel)
	{
		cl.pitchvel = v_centerspeed.value;
		cl.nodrift = false;
		cl.driftmove = 0;
	}
}

#ifdef USEFPM
void V_StartPitchDriftFPM (void)
{
#if 1
	if (clFPM.laststop == clFPM.time)
	{
		return;		// something else is keeping it from drifting
	}
#endif
	if (clFPM.nodrift || !clFPM.pitchvel)
	{
		clFPM.pitchvel = FPM_FROMFLOAT(v_centerspeed.value);
		clFPM.nodrift = false;
		clFPM.driftmove = 0;
	}
}
#endif //USEFPM

void V_StopPitchDrift (void)
{
	cl.laststop = cl.time;
	cl.nodrift = true;
	cl.pitchvel = 0;
}

#ifdef USEFPM
void V_StopPitchDriftFPM (void)
{
	clFPM.laststop = clFPM.time;
	clFPM.nodrift = true;
	clFPM.pitchvel = 0;
}
#endif //USEFPM

/*
===============
V_DriftPitch

Moves the client pitch angle towards cl.idealpitch sent by the server.

If the user is adjusting pitch manually, either with lookup/lookdown,
mlook and mouse, or klook and keyboard, pitch drifting is constantly stopped.

Drifting is enabled when the center view key is hit, mlook is released and
lookspring is non 0, or when
===============
*/
#ifdef _WIN32_WCE
extern mouse_down;
#endif

void V_DriftPitch (void)
{
	float		delta, move;

#ifdef _WIN32_WCE
	//Dan East: Pocket Quake specific
	//We don't do drift pitch if the user has the stylus down
	if (mouse_down) return;
#endif

	if (noclip_anglehack || !cl.onground || cls.demoplayback )
	{
		cl.driftmove = 0;
		cl.pitchvel = 0;
		return;
	}

// don't count small mouse motion
	if (cl.nodrift)
	{
		if ( fabs(cl.cmd.forwardmove) < cl_forwardspeed.value)
			cl.driftmove = 0;
		else
			cl.driftmove += (float)host_frametime;

		if ( cl.driftmove > v_centermove.value)
		{
			V_StartPitchDrift ();
		}
		return;
	}

	delta = cl.idealpitch - cl.viewangles[PITCH];

	if (!delta)
	{
		cl.pitchvel = 0;
		return;
	}

	move = (float)(host_frametime * cl.pitchvel);
	cl.pitchvel += (float)(host_frametime * v_centerspeed.value);

//Con_Printf ("move: %f (%f)\n", move, host_frametime);

	if (delta > 0)
	{
		if (move > delta)
		{
			cl.pitchvel = 0;
			move = delta;
		}
		cl.viewangles[PITCH] += move;
	}
	else if (delta < 0)
	{
		if (move > -delta)
		{
			cl.pitchvel = 0;
			move = -delta;
		}
		cl.viewangles[PITCH] -= move;
	}
}

#ifdef USEFPM
void V_DriftPitchFPM (void)
{
	fixedpoint_t	delta, move;
	fixedpoint_t	host_frametimeFPM=FPM_FROMFLOAT(host_frametime);

	if (noclip_anglehack || !clFPM.onground || cls.demoplayback )
	{
		clFPM.driftmove = 0;
		clFPM.pitchvel = 0;
		return;
	}

// don't count small mouse motion
	if (clFPM.nodrift)
	{
		if ( fabs(FPM_TOFLOAT(clFPM.cmd.forwardmove)) < cl_forwardspeed.value)
			clFPM.driftmove = 0;
		else
			clFPM.driftmove = FPM_ADD(clFPM.driftmove, host_frametimeFPM);

		if ( FPM_TOFLOAT(clFPM.driftmove) > v_centermove.value)
		{
			V_StartPitchDriftFPM ();
		}
		return;
	}

	delta = FPM_SUB(clFPM.idealpitch, clFPM.viewangles[PITCH]);

	if (!delta)
	{
		clFPM.pitchvel = 0;
		return;
	}

	move = FPM_MUL(host_frametimeFPM, clFPM.pitchvel);
	clFPM.pitchvel = FPM_ADD(clFPM.pitchvel, FPM_MUL(host_frametimeFPM, FPM_FROMFLOAT(v_centerspeed.value)));

//Con_Printf ("move: %f (%f)\n", move, host_frametime);

	if (delta > 0)
	{
		if (move > delta)
		{
			clFPM.pitchvel = 0;
			move = delta;
		}
		clFPM.viewangles[PITCH] = FPM_ADD(clFPM.viewangles[PITCH], move);
	}
	else if (delta < 0)
	{
		if (move > -delta)
		{
			clFPM.pitchvel = 0;
			move = -delta;
		}
		clFPM.viewangles[PITCH] = FPM_SUB(clFPM.viewangles[PITCH], move);
	}
}
#endif //USEFPM



/*
==============================================================================

						PALETTE FLASHES

==============================================================================
*/


cshift_t	cshift_empty = { {130,80,50}, 0 };
cshift_t	cshift_water = { {130,80,50}, 128 };
cshift_t	cshift_slime = { {0,25,5}, 150 };
cshift_t	cshift_lava = { {255,80,0}, 150 };

cvar_t		v_gamma = {"gamma", "1", true};

byte		gammatable[256];	// palette is sent through this

#ifdef	GLQUAKE
byte		ramps[3][256];
float		v_blend[4];		// rgba 0.0 - 1.0
#endif	// GLQUAKE

void BuildGammaTable (float g)
{
	int		i, inf;

	if (g == 1.0)
	{
		for (i=0 ; i<256 ; i++)
			gammatable[i] = i;
		return;
	}

	for (i=0 ; i<256 ; i++)
	{
		inf = (int)(255 * pow ( (i+0.5)/255.5 , g ) + 0.5);
		if (inf < 0)
			inf = 0;
		if (inf > 255)
			inf = 255;
		gammatable[i] = inf;
	}
}

/*
=================
V_CheckGamma
=================
*/
qboolean V_CheckGamma (void)
{
  static float oldgammavalue;


	//Dan: Hard-coded gamma value
	//	v_gamma.value=0.5;

	if (v_gamma.value == oldgammavalue)
		return false;
	oldgammavalue = v_gamma.value;

	BuildGammaTable (v_gamma.value);
	vid.recalc_refdef = 1;				// force a surface cache flush

	return true;
}



/*
===============
V_ParseDamage
===============
*/
void V_ParseDamage (void)
{
	int		armor, blood;
	vec3_t	from;
	int		i;
	vec3_t	forward, right, up;
	entity_t	*ent;
	float	side;
	float	count;

	armor = MSG_ReadByte ();
	blood = MSG_ReadByte ();
	for (i=0 ; i<3 ; i++)
		from[i] = MSG_ReadCoord ();

	count = (float)(blood*0.5 + armor*0.5);
	if (count < 10)
		count = 10;

	cl.faceanimtime = (float)(cl.time + 0.2);		// but sbar face into pain frame

	cl.cshifts[CSHIFT_DAMAGE].percent += (int)(3*count);
	if (cl.cshifts[CSHIFT_DAMAGE].percent < 0)
		cl.cshifts[CSHIFT_DAMAGE].percent = 0;
	if (cl.cshifts[CSHIFT_DAMAGE].percent > 150)
		cl.cshifts[CSHIFT_DAMAGE].percent = 150;

	if (armor > blood)
	{
		cl.cshifts[CSHIFT_DAMAGE].destcolor[0] = 200;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[1] = 100;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[2] = 100;
	}
	else if (armor)
	{
		cl.cshifts[CSHIFT_DAMAGE].destcolor[0] = 220;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[1] = 50;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[2] = 50;
	}
	else
	{
		cl.cshifts[CSHIFT_DAMAGE].destcolor[0] = 255;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[1] = 0;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[2] = 0;
	}

//
// calculate view angle kicks
//
	ent = &cl_entities[cl.viewentity];

	VectorSubtract (from, ent->origin, from);
	VectorNormalize (from);

	AngleVectors (ent->angles, forward, right, up);

	side = DotProduct (from, right);
	v_dmg_roll = count*side*v_kickroll.value;

	side = DotProduct (from, forward);
	v_dmg_pitch = count*side*v_kickpitch.value;

	v_dmg_time = v_kicktime.value;
}

#ifdef USEFPM
void V_ParseDamageFPM (void)
{
	int				armor, blood;
	vec3_FPM_t		from;
	int				i;
	vec3_FPM_t		forward, right, up;
	entity_FPM_t	*ent;
	fixedpoint_t	side;
	fixedpoint_t	count;

	armor = MSG_ReadByte ();
	blood = MSG_ReadByte ();
	for (i=0 ; i<3 ; i++)
		from[i] = FPM_FROMFLOAT(MSG_ReadCoord ());

	count = FPM_ADD(FPM_MUL(FPM_FROMLONG(blood),FPM_FROMFLOAT(0.5)), FPM_MUL(FPM_FROMLONG(armor),FPM_FROMFLOAT(0.5)));
	if (count < FPM_FROMLONG(10))
		count = FPM_FROMLONG(10);

	clFPM.faceanimtime = (float)(clFPM.time + 0.2);		// but sbar face into pain frame

	clFPM.cshifts[CSHIFT_DAMAGE].percent += FPM_TOLONG(FPM_MUL(FPM_FROMLONG(3),count));
	if (clFPM.cshifts[CSHIFT_DAMAGE].percent < 0)
		clFPM.cshifts[CSHIFT_DAMAGE].percent = 0;
	if (clFPM.cshifts[CSHIFT_DAMAGE].percent > 150)
		clFPM.cshifts[CSHIFT_DAMAGE].percent = 150;

	if (armor > blood)
	{
		clFPM.cshifts[CSHIFT_DAMAGE].destcolor[0] = 200;
		clFPM.cshifts[CSHIFT_DAMAGE].destcolor[1] = 100;
		clFPM.cshifts[CSHIFT_DAMAGE].destcolor[2] = 100;
	}
	else if (armor)
	{
		clFPM.cshifts[CSHIFT_DAMAGE].destcolor[0] = 220;
		clFPM.cshifts[CSHIFT_DAMAGE].destcolor[1] = 50;
		clFPM.cshifts[CSHIFT_DAMAGE].destcolor[2] = 50;
	}
	else
	{
		clFPM.cshifts[CSHIFT_DAMAGE].destcolor[0] = 255;
		clFPM.cshifts[CSHIFT_DAMAGE].destcolor[1] = 0;
		clFPM.cshifts[CSHIFT_DAMAGE].destcolor[2] = 0;
	}

//
// calculate view angle kicks
//
	ent = &cl_entitiesFPM[clFPM.viewentity];

	VectorSubtractFPM (from, ent->origin, from);
	VectorNormalizeFPM (from);

	AngleVectorsFPM (ent->angles, forward, right, up);

	side = DotProductFPM (from, right);
	v_dmg_rollFPM = FPM_MUL(FPM_MUL(count, side), FPM_FROMFLOAT(v_kickroll.value));

	side = DotProductFPM (from, forward);
	v_dmg_pitchFPM = FPM_MUL(count,FPM_MUL(side,FPM_FROMFLOAT(v_kickpitch.value)));

	v_dmg_timeFPM = FPM_FROMFLOAT(v_kicktime.value);
}
#endif //USEFPM

/*
==================
V_cshift_f
==================
*/
void V_cshift_f (void)
{
	cshift_empty.destcolor[0] = atoi(Cmd_Argv(1));
	cshift_empty.destcolor[1] = atoi(Cmd_Argv(2));
	cshift_empty.destcolor[2] = atoi(Cmd_Argv(3));
	cshift_empty.percent = atoi(Cmd_Argv(4));
}


/*
==================
V_BonusFlash_f

When you run over an item, the server sends this command
==================
*/
void V_BonusFlash_f (void)
{
	cl.cshifts[CSHIFT_BONUS].destcolor[0] = 215;
	cl.cshifts[CSHIFT_BONUS].destcolor[1] = 186;
	cl.cshifts[CSHIFT_BONUS].destcolor[2] = 69;
	cl.cshifts[CSHIFT_BONUS].percent = 50;
}

#ifdef USEFPM
void V_BonusFlash_FPM_f (void)
{
	clFPM.cshifts[CSHIFT_BONUS].destcolor[0] = 215;
	clFPM.cshifts[CSHIFT_BONUS].destcolor[1] = 186;
	clFPM.cshifts[CSHIFT_BONUS].destcolor[2] = 69;
	clFPM.cshifts[CSHIFT_BONUS].percent = 50;
}
#endif //USEFPM

/*
=============
V_SetContentsColor

Underwater, lava, etc each has a color shift
=============
*/
void V_SetContentsColor (int contents)
{
	switch (contents)
	{
	case CONTENTS_EMPTY:
	case CONTENTS_SOLID:
		cl.cshifts[CSHIFT_CONTENTS] = cshift_empty;
		break;
	case CONTENTS_LAVA:
		cl.cshifts[CSHIFT_CONTENTS] = cshift_lava;
		break;
	case CONTENTS_SLIME:
		cl.cshifts[CSHIFT_CONTENTS] = cshift_slime;
		break;
	default:
		cl.cshifts[CSHIFT_CONTENTS] = cshift_water;
	}
}

#ifdef USEFPM
void V_SetContentsColorFPM (int contents)
{
	switch (contents)
	{
	case CONTENTS_EMPTY:
	case CONTENTS_SOLID:
		clFPM.cshifts[CSHIFT_CONTENTS] = cshift_empty;
		break;
	case CONTENTS_LAVA:
		clFPM.cshifts[CSHIFT_CONTENTS] = cshift_lava;
		break;
	case CONTENTS_SLIME:
		clFPM.cshifts[CSHIFT_CONTENTS] = cshift_slime;
		break;
	default:
		clFPM.cshifts[CSHIFT_CONTENTS] = cshift_water;
	}
}
#endif //USEFPM
/*
=============
V_CalcPowerupCshift
=============
*/
void V_CalcPowerupCshift (void)
{
	if (cl.items & IT_QUAD)
	{
		cl.cshifts[CSHIFT_POWERUP].destcolor[0] = 0;
		cl.cshifts[CSHIFT_POWERUP].destcolor[1] = 0;
		cl.cshifts[CSHIFT_POWERUP].destcolor[2] = 255;
		cl.cshifts[CSHIFT_POWERUP].percent = 30;
	}
	else if (cl.items & IT_SUIT)
	{
		cl.cshifts[CSHIFT_POWERUP].destcolor[0] = 0;
		cl.cshifts[CSHIFT_POWERUP].destcolor[1] = 255;
		cl.cshifts[CSHIFT_POWERUP].destcolor[2] = 0;
		cl.cshifts[CSHIFT_POWERUP].percent = 20;
	}
	else if (cl.items & IT_INVISIBILITY)
	{
		cl.cshifts[CSHIFT_POWERUP].destcolor[0] = 100;
		cl.cshifts[CSHIFT_POWERUP].destcolor[1] = 100;
		cl.cshifts[CSHIFT_POWERUP].destcolor[2] = 100;
		cl.cshifts[CSHIFT_POWERUP].percent = 100;
	}
	else if (cl.items & IT_INVULNERABILITY)
	{
		cl.cshifts[CSHIFT_POWERUP].destcolor[0] = 255;
		cl.cshifts[CSHIFT_POWERUP].destcolor[1] = 255;
		cl.cshifts[CSHIFT_POWERUP].destcolor[2] = 0;
		cl.cshifts[CSHIFT_POWERUP].percent = 30;
	}
	else
		cl.cshifts[CSHIFT_POWERUP].percent = 0;
}

#ifdef USEFPM
void V_CalcPowerupCshiftFPM (void)
{
	if (clFPM.items & IT_QUAD)
	{
		clFPM.cshifts[CSHIFT_POWERUP].destcolor[0] = 0;
		clFPM.cshifts[CSHIFT_POWERUP].destcolor[1] = 0;
		clFPM.cshifts[CSHIFT_POWERUP].destcolor[2] = 255;
		clFPM.cshifts[CSHIFT_POWERUP].percent = 30;
	}
	else if (clFPM.items & IT_SUIT)
	{
		clFPM.cshifts[CSHIFT_POWERUP].destcolor[0] = 0;
		clFPM.cshifts[CSHIFT_POWERUP].destcolor[1] = 255;
		clFPM.cshifts[CSHIFT_POWERUP].destcolor[2] = 0;
		clFPM.cshifts[CSHIFT_POWERUP].percent = 20;
	}
	else if (clFPM.items & IT_INVISIBILITY)
	{
		clFPM.cshifts[CSHIFT_POWERUP].destcolor[0] = 100;
		clFPM.cshifts[CSHIFT_POWERUP].destcolor[1] = 100;
		clFPM.cshifts[CSHIFT_POWERUP].destcolor[2] = 100;
		clFPM.cshifts[CSHIFT_POWERUP].percent = 100;
	}
	else if (clFPM.items & IT_INVULNERABILITY)
	{
		clFPM.cshifts[CSHIFT_POWERUP].destcolor[0] = 255;
		clFPM.cshifts[CSHIFT_POWERUP].destcolor[1] = 255;
		clFPM.cshifts[CSHIFT_POWERUP].destcolor[2] = 0;
		clFPM.cshifts[CSHIFT_POWERUP].percent = 30;
	}
	else
		clFPM.cshifts[CSHIFT_POWERUP].percent = 0;
}
#endif //USEFPM

/*
=============
V_CalcBlend
=============
*/
#ifdef	GLQUAKE
//Dan: Not converted
void V_CalcBlend (void)
{
	float	r, g, b, a, a2;
	int		j;

	r = 0;
	g = 0;
	b = 0;
	a = 0;

	for (j=0 ; j<NUM_CSHIFTS ; j++)
	{
		if (!gl_cshiftpercent.value)
			continue;

		a2 = ((cl.cshifts[j].percent * gl_cshiftpercent.value) / 100.0) / 255.0;

//		a2 = cl.cshifts[j].percent/255.0;
		if (!a2)
			continue;
		a = a + a2*(1-a);
//Con_Printf ("j:%i a:%f\n", j, a);
		a2 = a2/a;
		r = r*(1-a2) + cl.cshifts[j].destcolor[0]*a2;
		g = g*(1-a2) + cl.cshifts[j].destcolor[1]*a2;
		b = b*(1-a2) + cl.cshifts[j].destcolor[2]*a2;
	}

	v_blend[0] = r/255.0;
	v_blend[1] = g/255.0;
	v_blend[2] = b/255.0;
	v_blend[3] = a;
	if (v_blend[3] > 1)
		v_blend[3] = 1;
	if (v_blend[3] < 0)
		v_blend[3] = 0;
}
#endif

/*
=============
V_UpdatePalette
=============
*/
#ifdef	GLQUAKE
//Dan: not converted
void V_UpdatePalette (void)
{
	int		i, j;
	qboolean	new;
	byte	*basepal, *newpal;
	byte	pal[768];
	float	r,g,b,a;
	int		ir, ig, ib;
	qboolean force;

	V_CalcPowerupCshift ();

	new = false;

	for (i=0 ; i<NUM_CSHIFTS ; i++)
	{
		if (cl.cshifts[i].percent != cl.prev_cshifts[i].percent)
		{
			new = true;
			cl.prev_cshifts[i].percent = cl.cshifts[i].percent;
		}
		for (j=0 ; j<3 ; j++)
			if (cl.cshifts[i].destcolor[j] != cl.prev_cshifts[i].destcolor[j])
			{
				new = true;
				cl.prev_cshifts[i].destcolor[j] = cl.cshifts[i].destcolor[j];
			}
	}

// drop the damage value
	cl.cshifts[CSHIFT_DAMAGE].percent -= host_frametime*150;
	if (cl.cshifts[CSHIFT_DAMAGE].percent <= 0)
		cl.cshifts[CSHIFT_DAMAGE].percent = 0;

// drop the bonus value
	cl.cshifts[CSHIFT_BONUS].percent -= host_frametime*100;
	if (cl.cshifts[CSHIFT_BONUS].percent <= 0)
		cl.cshifts[CSHIFT_BONUS].percent = 0;

	force = V_CheckGamma ();
	if (!new && !force)
		return;

	V_CalcBlend ();

	a = v_blend[3];
	r = 255*v_blend[0]*a;
	g = 255*v_blend[1]*a;
	b = 255*v_blend[2]*a;

	a = 1-a;
	for (i=0 ; i<256 ; i++)
	{
		ir = i*a + r;
		ig = i*a + g;
		ib = i*a + b;
		if (ir > 255)
			ir = 255;
		if (ig > 255)
			ig = 255;
		if (ib > 255)
			ib = 255;

		ramps[0][i] = gammatable[ir];
		ramps[1][i] = gammatable[ig];
		ramps[2][i] = gammatable[ib];
	}

	basepal = host_basepal;
	newpal = pal;

	for (i=0 ; i<256 ; i++)
	{
		ir = basepal[0];
		ig = basepal[1];
		ib = basepal[2];
		basepal += 3;

		newpal[0] = ramps[0][ir];
		newpal[1] = ramps[1][ig];
		newpal[2] = ramps[2][ib];
		newpal += 3;
	}

       	VID_ShiftPalette (pal);
}
#else	// !GLQUAKE
void V_UpdatePalette (void)
{
	int		i, j;
	qboolean	new;
	byte	*basepal,*newpal;
	byte	pal[768];
	int		r,g,b;
	qboolean force;

	V_CalcPowerupCshift ();

	new = false;

	for (i=0 ; i<NUM_CSHIFTS ; i++)
	{
		if (cl.cshifts[i].percent != cl.prev_cshifts[i].percent)
		{
			new = true;
			cl.prev_cshifts[i].percent = cl.cshifts[i].percent;
		}
		for (j=0 ; j<3 ; j++)
			if (cl.cshifts[i].destcolor[j] != cl.prev_cshifts[i].destcolor[j])
			{
				new = true;
				cl.prev_cshifts[i].destcolor[j] = cl.cshifts[i].destcolor[j];
			}
	}

// drop the damage value
	cl.cshifts[CSHIFT_DAMAGE].percent -= (int)(host_frametime*150);
	if (cl.cshifts[CSHIFT_DAMAGE].percent <= 0)
		cl.cshifts[CSHIFT_DAMAGE].percent = 0;

// drop the bonus value
	cl.cshifts[CSHIFT_BONUS].percent -= (int)(host_frametime*100);
	if (cl.cshifts[CSHIFT_BONUS].percent <= 0)
		cl.cshifts[CSHIFT_BONUS].percent = 0;

	force = V_CheckGamma ();
	if (!new && !force)
		return;

	basepal = host_basepal;
	newpal = pal;

	for (i=0 ; i<256 ; i++)
	{
		r = basepal[0];
		g = basepal[1];
		b = basepal[2];
		basepal += 3;

		for (j=0 ; j<NUM_CSHIFTS ; j++)
		{
			r += (cl.cshifts[j].percent*(cl.cshifts[j].destcolor[0]-r))>>8;
			g += (cl.cshifts[j].percent*(cl.cshifts[j].destcolor[1]-g))>>8;
			b += (cl.cshifts[j].percent*(cl.cshifts[j].destcolor[2]-b))>>8;
		}

		newpal[0] = gammatable[r];
		newpal[1] = gammatable[g];
		newpal[2] = gammatable[b];
		newpal += 3;
	}

	VID_ShiftPalette (pal);

}

#ifdef USEFPM
void V_UpdatePaletteFPM (void)
{
	int		i, j;
	qboolean	new;
	byte	*basepal, *newpal;
	byte	pal[768];
	int		r,g,b;
	qboolean force;

	V_CalcPowerupCshiftFPM ();

	new = false;

	for (i=0 ; i<NUM_CSHIFTS ; i++)
	{
		if (clFPM.cshifts[i].percent != clFPM.prev_cshifts[i].percent)
		{
			new = true;
			clFPM.prev_cshifts[i].percent = clFPM.cshifts[i].percent;
		}
		for (j=0 ; j<3 ; j++)
			if (clFPM.cshifts[i].destcolor[j] != clFPM.prev_cshifts[i].destcolor[j])
			{
				new = true;
				clFPM.prev_cshifts[i].destcolor[j] = clFPM.cshifts[i].destcolor[j];
			}
	}

// drop the damage value
	clFPM.cshifts[CSHIFT_DAMAGE].percent -= (int)(host_frametime*150);
	if (clFPM.cshifts[CSHIFT_DAMAGE].percent <= 0)
		clFPM.cshifts[CSHIFT_DAMAGE].percent = 0;

// drop the bonus value
	clFPM.cshifts[CSHIFT_BONUS].percent -= (int)(host_frametime*100);
	if (clFPM.cshifts[CSHIFT_BONUS].percent <= 0)
		clFPM.cshifts[CSHIFT_BONUS].percent = 0;

	force = V_CheckGamma ();
	if (!new && !force)
		return;

	basepal = host_basepal;
	newpal = pal;

	for (i=0 ; i<256 ; i++)
	{
		r = basepal[0];
		g = basepal[1];
		b = basepal[2];
		basepal += 3;

		for (j=0 ; j<NUM_CSHIFTS ; j++)
		{
			r += (clFPM.cshifts[j].percent*(clFPM.cshifts[j].destcolor[0]-r))>>8;
			g += (clFPM.cshifts[j].percent*(clFPM.cshifts[j].destcolor[1]-g))>>8;
			b += (clFPM.cshifts[j].percent*(clFPM.cshifts[j].destcolor[2]-b))>>8;
		}

		newpal[0] = gammatable[r];
		newpal[1] = gammatable[g];
		newpal[2] = gammatable[b];
		newpal += 3;
	}

	VID_ShiftPalette(pal);
}
#endif //USEFPM
#endif	// !GLQUAKE


/*
==============================================================================

						VIEW RENDERING

==============================================================================
*/

float angledelta (float a)
{
	a = anglemod(a);
	if (a > 180)
		a -= 360;
	return a;
}

#ifdef USEFPM
fixedpoint_t angledeltaFPM (fixedpoint_t a)
{
	a = anglemodFPM(a);
	if (a > 180)
		a -= 360;
	return a;
}
#endif //USEFPM
/*
==================
CalcGunAngle
==================
*/
void CalcGunAngle (void)
{
	float	yaw, pitch, move;
	static float oldyaw = 0;
	static float oldpitch = 0;

	yaw = r_refdef.viewangles[YAW];
	pitch = -r_refdef.viewangles[PITCH];

	yaw = (float)(angledelta(yaw - r_refdef.viewangles[YAW]) * 0.4);
	if (yaw > 10)
		yaw = 10;
	if (yaw < -10)
		yaw = -10;
	pitch = (float)(angledelta(-pitch - r_refdef.viewangles[PITCH]) * 0.4);
	if (pitch > 10)
		pitch = 10;
	if (pitch < -10)
		pitch = -10;
	move = (float)(host_frametime*20);
	if (yaw > oldyaw)
	{
		if (oldyaw + move < yaw)
			yaw = oldyaw + move;
	}
	else
	{
		if (oldyaw - move > yaw)
			yaw = oldyaw - move;
	}

	if (pitch > oldpitch)
	{
		if (oldpitch + move < pitch)
			pitch = oldpitch + move;
	}
	else
	{
		if (oldpitch - move > pitch)
			pitch = oldpitch - move;
	}

	oldyaw = yaw;
	oldpitch = pitch;

	cl.viewent.angles[YAW] = r_refdef.viewangles[YAW] + yaw;
	cl.viewent.angles[PITCH] = - (r_refdef.viewangles[PITCH] + pitch);

	cl.viewent.angles[ROLL] -= (float)(v_idlescale.value * sin(cl.time*v_iroll_cycle.value) * v_iroll_level.value);
	cl.viewent.angles[PITCH] -= (float)(v_idlescale.value * sin(cl.time*v_ipitch_cycle.value) * v_ipitch_level.value);
	cl.viewent.angles[YAW] -= (float)(v_idlescale.value * sin(cl.time*v_iyaw_cycle.value) * v_iyaw_level.value);
}

#ifdef USEFPM
void CalcGunAngleFPM (void)
{
	fixedpoint_t		yaw, pitch, move;
	static fixedpoint_t oldyaw = 0;
	static fixedpoint_t oldpitch = 0;

	yaw = r_refdefFPM.viewangles[YAW];
	pitch = -r_refdefFPM.viewangles[PITCH];

	yaw = angledeltaFPM(FPM_SUB(yaw, FPM_MUL(r_refdefFPM.viewangles[YAW], FPM_FROMFLOAT(0.4))));
	if (yaw > FPM_FROMLONG(10))
		yaw = FPM_FROMLONG(10);
	if (yaw < FPM_FROMLONG(-10))
		yaw = FPM_FROMLONG(-10);
	pitch = angledeltaFPM(FPM_SUB(-pitch, FPM_MUL(r_refdefFPM.viewangles[PITCH], FPM_FROMFLOAT(0.4))));
	if (pitch > FPM_FROMLONG(10))
		pitch = FPM_FROMLONG(10);
	if (pitch < FPM_FROMLONG(-10))
		pitch = FPM_FROMLONG(-10);
	move = FPM_MUL(FPM_FROMFLOAT(host_frametime),FPM_FROMLONG(20));
	if (yaw > oldyaw)
	{
		if (FPM_ADD(oldyaw, move) < yaw)
			yaw = FPM_ADD(oldyaw, move);
	}
	else
	{
		if (FPM_SUB(oldyaw, move) > yaw)
			yaw = FPM_SUB(oldyaw, move);
	}

	if (pitch > oldpitch)
	{
		if (FPM_ADD(oldpitch, move) < pitch)
			pitch = FPM_ADD(oldpitch, move);
	}
	else
	{
		if (FPM_SUB(oldpitch, move) > pitch)
			pitch = FPM_SUB(oldpitch, move);
	}

	oldyaw = yaw;
	oldpitch = pitch;

	clFPM.viewent.angles[YAW] = FPM_ADD(r_refdefFPM.viewangles[YAW], yaw);
	clFPM.viewent.angles[PITCH] = - FPM_ADD(r_refdefFPM.viewangles[PITCH], pitch);

	clFPM.viewent.angles[ROLL] = FPM_SUB(clFPM.viewent.angles[ROLL], FPM_FROMFLOAT(v_idlescale.value * sin(clFPM.time*v_iroll_cycle.value) * v_iroll_level.value));
	clFPM.viewent.angles[PITCH] = FPM_SUB(clFPM.viewent.angles[PITCH], FPM_FROMFLOAT(v_idlescale.value * sin(clFPM.time*v_ipitch_cycle.value) * v_ipitch_level.value));
	clFPM.viewent.angles[YAW] = FPM_SUB(clFPM.viewent.angles[YAW], FPM_FROMFLOAT(v_idlescale.value * sin(clFPM.time*v_iyaw_cycle.value) * v_iyaw_level.value));
}
#endif //USEFPM
/*
==============
V_BoundOffsets
==============
*/
void V_BoundOffsets (void)
{
	entity_t	*ent;

	ent = &cl_entities[cl.viewentity];

// absolutely bound refresh reletive to entity clipping hull
// so the view can never be inside a solid wall

	if (r_refdef.vieworg[0] < ent->origin[0] - 14)
		r_refdef.vieworg[0] = ent->origin[0] - 14;
	else if (r_refdef.vieworg[0] > ent->origin[0] + 14)
		r_refdef.vieworg[0] = ent->origin[0] + 14;
	if (r_refdef.vieworg[1] < ent->origin[1] - 14)
		r_refdef.vieworg[1] = ent->origin[1] - 14;
	else if (r_refdef.vieworg[1] > ent->origin[1] + 14)
		r_refdef.vieworg[1] = ent->origin[1] + 14;
	if (r_refdef.vieworg[2] < ent->origin[2] - 22)
		r_refdef.vieworg[2] = ent->origin[2] - 22;
	else if (r_refdef.vieworg[2] > ent->origin[2] + 30)
		r_refdef.vieworg[2] = ent->origin[2] + 30;
}

#ifdef USEFPM
void V_BoundOffsetsFPM (void)
{
	entity_FPM_t	*ent;

	ent = &cl_entitiesFPM[clFPM.viewentity];

// absolutely bound refresh reletive to entity clipping hull
// so the view can never be inside a solid wall

	if (r_refdefFPM.vieworg[0] < FPM_SUB(ent->origin[0], FPM_FROMLONG(14)))
		r_refdefFPM.vieworg[0] = FPM_SUB(ent->origin[0], FPM_FROMLONG(14));
	else if (r_refdefFPM.vieworg[0] > FPM_ADD(ent->origin[0], FPM_FROMLONG(14)))
		r_refdefFPM.vieworg[0] = FPM_ADD(ent->origin[0], FPM_FROMLONG(14));
	if (r_refdefFPM.vieworg[1] < FPM_SUB(ent->origin[1], FPM_FROMLONG(14)))
		r_refdefFPM.vieworg[1] = FPM_SUB(ent->origin[1], FPM_FROMLONG(14));
	else if (r_refdefFPM.vieworg[1] > FPM_ADD(ent->origin[1], FPM_FROMLONG(14)))
		r_refdefFPM.vieworg[1] = FPM_ADD(ent->origin[1], FPM_FROMLONG(14));
	if (r_refdefFPM.vieworg[2] < FPM_SUB(ent->origin[2], FPM_FROMLONG(22)))
		r_refdefFPM.vieworg[2] = FPM_SUB(ent->origin[2], FPM_FROMLONG(22));
	else if (r_refdefFPM.vieworg[2] > FPM_ADD(ent->origin[2], FPM_FROMLONG(30)))
		r_refdefFPM.vieworg[2] = FPM_ADD(ent->origin[2], FPM_FROMLONG(30));
}
#endif //USEFPM
/*
==============
V_AddIdle

Idle swaying
==============
*/
void V_AddIdle (void)
{
	r_refdef.viewangles[ROLL] += (float)(v_idlescale.value * sin(cl.time*v_iroll_cycle.value) * v_iroll_level.value);
	r_refdef.viewangles[PITCH] += (float)(v_idlescale.value * sin(cl.time*v_ipitch_cycle.value) * v_ipitch_level.value);
	r_refdef.viewangles[YAW] += (float)(v_idlescale.value * sin(cl.time*v_iyaw_cycle.value) * v_iyaw_level.value);
}

#ifdef USEFPM
void V_AddIdleFPM (void)
{
	r_refdefFPM.viewangles[ROLL] = FPM_ADD(r_refdefFPM.viewangles[ROLL], FPM_FROMFLOAT(v_idlescale.value * sin(clFPM.time*v_iroll_cycle.value) * v_iroll_level.value));
	r_refdefFPM.viewangles[PITCH] = FPM_ADD(r_refdefFPM.viewangles[PITCH], FPM_FROMFLOAT(v_idlescale.value * sin(clFPM.time*v_ipitch_cycle.value) * v_ipitch_level.value));
	r_refdefFPM.viewangles[YAW] = FPM_ADD(r_refdefFPM.viewangles[YAW], FPM_FROMFLOAT(v_idlescale.value * sin(clFPM.time*v_iyaw_cycle.value) * v_iyaw_level.value));
}
#endif //USEFPM
/*
==============
V_CalcViewRoll

Roll is induced by movement and damage
==============
*/
void V_CalcViewRoll (void)
{
	float		side;

	side = V_CalcRoll (cl_entities[cl.viewentity].angles, cl.velocity);
	r_refdef.viewangles[ROLL] += side;

	if (v_dmg_time > 0)
	{
		r_refdef.viewangles[ROLL] += v_dmg_time/v_kicktime.value*v_dmg_roll;
		r_refdef.viewangles[PITCH] += v_dmg_time/v_kicktime.value*v_dmg_pitch;
		v_dmg_time -= (float)host_frametime;
	}

	if (cl.stats[STAT_HEALTH] <= 0)
	{
		r_refdef.viewangles[ROLL] = 80;	// dead view angle
		return;
	}

}

#ifdef USEFPM
void V_CalcViewRollFPM (void)
{
	fixedpoint_t	side;

	side = V_CalcRollFPM (cl_entitiesFPM[clFPM.viewentity].angles, clFPM.velocity);
	r_refdefFPM.viewangles[ROLL] = FPM_ADD(r_refdefFPM.viewangles[ROLL], side);

	if (v_dmg_time > 0)
	{
		r_refdefFPM.viewangles[ROLL] = FPM_ADD(r_refdefFPM.viewangles[ROLL], FPM_FROMFLOAT(v_dmg_time/v_kicktime.value*v_dmg_roll));
		r_refdefFPM.viewangles[PITCH] = FPM_ADD(r_refdefFPM.viewangles[PITCH], FPM_FROMFLOAT(v_dmg_time/v_kicktime.value*v_dmg_pitch));
		v_dmg_time -= (float)host_frametime;
	}

	if (clFPM.stats[STAT_HEALTH] <= 0)
	{
		r_refdefFPM.viewangles[ROLL] = FPM_FROMLONG(80);	// dead view angle
		return;
	}
}
#endif //USEFPM

/*
==================
V_CalcIntermissionRefdef

==================
*/
void V_CalcIntermissionRefdef (void)
{
	entity_t	*ent, *view;
	float		old;

// ent is the player model (visible when out of body)
	ent = &cl_entities[cl.viewentity];
// view is the weapon model (only visible from inside body)
	view = &cl.viewent;

	VectorCopy (ent->origin, r_refdef.vieworg);
	VectorCopy (ent->angles, r_refdef.viewangles);
	view->model = NULL;

// allways idle in intermission
	old = v_idlescale.value;
	v_idlescale.value = 1;
	V_AddIdle ();
	v_idlescale.value = old;
}

#ifdef USEFPM
void V_CalcIntermissionRefdefFPM (void)
{
	entity_FPM_t	*ent, *view;
	float			old;

// ent is the player model (visible when out of body)
	ent = &cl_entitiesFPM[clFPM.viewentity];
// view is the weapon model (only visible from inside body)
	view = &clFPM.viewent;

	VectorCopy (ent->origin, r_refdefFPM.vieworg);
	VectorCopy (ent->angles, r_refdefFPM.viewangles);
	view->model = NULL;

// allways idle in intermission
	old = v_idlescale.value;
	v_idlescale.value = 1;
	V_AddIdleFPM ();
	v_idlescale.value = old;
}
#endif //USEFPM
/*
==================
V_CalcRefdef

==================
*/
extern float yaw_modifier;
extern float start_yaw;
void V_CalcRefdef (void)
{
	entity_t	*ent, *view;
	int			i;
	vec3_t		forward, right, up;
	vec3_t		angles;
	float		bob;
	static float oldz = 0;

	//Dan East:
	//Here we handle automatic yaw rotation when the stylus reaches the edge of the screen
	cl.viewangles[YAW]+=yaw_modifier;
	start_yaw+=yaw_modifier;

	V_DriftPitch ();

// ent is the player model (visible when out of body)
	ent = &cl_entities[cl.viewentity];
// view is the weapon model (only visible from inside body)
	view = &cl.viewent;


// transform the view offset by the model's matrix to get the offset from
// model origin for the view
	ent->angles[YAW] = cl.viewangles[YAW];	// the model should face
										// the view dir
	ent->angles[PITCH] = -cl.viewangles[PITCH];	// the model should face
										// the view dir


	bob = V_CalcBob ();

// refresh position
	VectorCopy (ent->origin, r_refdef.vieworg);
	r_refdef.vieworg[2] += cl.viewheight + bob;

// never let it sit exactly on a node line, because a water plane can
// dissapear when viewed with the eye exactly on it.
// the server protocol only specifies to 1/16 pixel, so add 1/32 in each axis
	r_refdef.vieworg[0] += 1.0/32;
	r_refdef.vieworg[1] += 1.0/32;
	r_refdef.vieworg[2] += 1.0/32;

	VectorCopy (cl.viewangles, r_refdef.viewangles);
	V_CalcViewRoll ();
	V_AddIdle ();

// offsets
	angles[PITCH] = -ent->angles[PITCH];	// because entity pitches are
											//  actually backward
	angles[YAW] = ent->angles[YAW];
	angles[ROLL] = ent->angles[ROLL];

	AngleVectors (angles, forward, right, up);

	for (i=0 ; i<3 ; i++)
		r_refdef.vieworg[i] += scr_ofsx.value*forward[i]
			+ scr_ofsy.value*right[i]
			+ scr_ofsz.value*up[i];


	V_BoundOffsets ();

// set up gun position
	VectorCopy (cl.viewangles, view->angles);

	CalcGunAngle ();

	VectorCopy (ent->origin, view->origin);
	view->origin[2] += cl.viewheight;

#if 0 
   for (i=0 ; i<3 ; i++) 
   { 
      view->origin[i] += forward[i]*bob*0.4; 
	// view->origin[i] += right[i]*bob*0.4; 
	// view->origin[i] += up[i]*bob*0.8; 
   } 
   view->origin[2] += bob; 
#endif 

#if 1    
   VectorCopy (r_refdef.vieworg, view->origin); 
   VectorMA (view->origin, bob * 0.4, forward, view->origin); 

   if (r_viewmodeloffset.string[0]) { 
      float offset[3]; 
      int size = sizeof(offset)/sizeof(offset[0]); 

      ParseFloats(r_viewmodeloffset.string, offset, &size); 
      VectorMA (view->origin,  offset[0], right,   view->origin); 
      VectorMA (view->origin, -offset[1], up,      view->origin); 
      VectorMA (view->origin,  offset[2], forward, view->origin); 
   } 
#endif

// fudge position around to keep amount of weapon visible
// roughly equal with different FOV

#if 0
	if (cl.model_precache[cl.stats[STAT_WEAPON]] && strcmp (cl.model_precache[cl.stats[STAT_WEAPON]]->name,  "progs/v_shot2.mdl"))
#endif
	if (scr_viewsize.value == 110)
		view->origin[2] += 1;
	else if (scr_viewsize.value == 100)
		view->origin[2] += 2;
	else if (scr_viewsize.value == 90)
		view->origin[2] += 1;
	else if (scr_viewsize.value == 80)
		view->origin[2] += 0.5;

	view->model = cl.model_precache[cl.stats[STAT_WEAPON]];
	view->frame = cl.stats[STAT_WEAPONFRAME];
	view->colormap = vid.colormap;

// set up the refresh position
	VectorAdd (r_refdef.viewangles, cl.punchangle, r_refdef.viewangles);

// smooth out stair step ups
if (cl.onground && ent->origin[2] - oldz > 0)
{
	float steptime;

	steptime = (float)(cl.time - cl.oldtime);
	if (steptime < 0)
//FIXME		I_Error ("steptime < 0");
		steptime = 0;

	oldz += steptime * 80;
	if (oldz > ent->origin[2])
		oldz = ent->origin[2];
	if (ent->origin[2] - oldz > 12)
		oldz = ent->origin[2] - 12;
	r_refdef.vieworg[2] += oldz - ent->origin[2];
	view->origin[2] += oldz - ent->origin[2];
}
else
	oldz = ent->origin[2];

	if (chase_active.value)
		Chase_Update ();
}

#ifdef USEFPM
void V_CalcRefdefFPM (void)
{
	entity_FPM_t		*ent, *view;
	int					i;
	vec3_FPM_t			forward, right, up;
	vec3_FPM_t			angles;
	fixedpoint_t		bob;
	static fixedpoint_t	oldz = 0;

	V_DriftPitchFPM ();

// ent is the player model (visible when out of body)
	ent = &cl_entitiesFPM[clFPM.viewentity];
// view is the weapon model (only visible from inside body)
	view = &clFPM.viewent;


// transform the view offset by the model's matrix to get the offset from
// model origin for the view
	ent->angles[YAW] = clFPM.viewangles[YAW];	// the model should face
										// the view dir
	ent->angles[PITCH] = -clFPM.viewangles[PITCH];	// the model should face
										// the view dir


	bob = V_CalcBobFPM ();

// refresh position
	VectorCopy (ent->origin, r_refdefFPM.vieworg);
	r_refdefFPM.vieworg[2] = FPM_ADD(r_refdefFPM.vieworg[2], FPM_ADD(clFPM.viewheight, bob));

// never let it sit exactly on a node line, because a water plane can
// dissapear when viewed with the eye exactly on it.
// the server protocol only specifies to 1/16 pixel, so add 1/32 in each axis
	r_refdefFPM.vieworg[0] = FPM_ADD(r_refdefFPM.vieworg[0], FPM_FROMFLOAT(1.0/32));
	r_refdefFPM.vieworg[1] = FPM_ADD(r_refdefFPM.vieworg[1], FPM_FROMFLOAT(1.0/32));
	r_refdefFPM.vieworg[2] = FPM_ADD(r_refdefFPM.vieworg[2], FPM_FROMFLOAT(1.0/32));

	VectorCopy (clFPM.viewangles, r_refdefFPM.viewangles);
	V_CalcViewRollFPM ();
	V_AddIdleFPM ();

// offsets
	angles[PITCH] = -ent->angles[PITCH];	// because entity pitches are
											//  actually backward
	angles[YAW] = ent->angles[YAW];
	angles[ROLL] = ent->angles[ROLL];

	AngleVectorsFPM (angles, forward, right, up);

	for (i=0 ; i<3 ; i++)
		r_refdefFPM.vieworg[i] = FPM_ADD(r_refdefFPM.vieworg[i], FPM_ADD3(FPM_MUL(FPM_FROMFLOAT(scr_ofsx.value),forward[i]),
			FPM_MUL(FPM_FROMFLOAT(scr_ofsy.value),right[i]),
			FPM_MUL(FPM_FROMFLOAT(scr_ofsz.value),up[i])));


	V_BoundOffsetsFPM ();

// set up gun position
	VectorCopy (clFPM.viewangles, view->angles);

	CalcGunAngleFPM ();

	VectorCopy (ent->origin, view->origin);
	view->origin[2] = FPM_ADD(view->origin[2], clFPM.viewheight);

	for (i=0 ; i<3 ; i++)
	{
		view->origin[i] += FPM_ADD(view->origin[i], (FPM_MUL(FPM_MUL(forward[i],bob),FPM_FROMFLOAT(0.4))));
//		view->origin[i] += right[i]*bob*0.4;
//		view->origin[i] += up[i]*bob*0.8;
	}
	view->origin[2] = FPM_ADD(view->origin[2], bob);

// fudge position around to keep amount of weapon visible
// roughly equal with different FOV

#if 0
	//Dan: Not converted
	if (cl.model_precache[cl.stats[STAT_WEAPON]] && strcmp (cl.model_precache[cl.stats[STAT_WEAPON]]->name,  "progs/v_shot2.mdl"))
#endif
	if (scr_viewsize.value == 110)
		FPM_INC(view->origin[2]);
	else if (scr_viewsize.value == 100)
		view->origin[2]=FPM_ADD(view->origin[2], FPM_FROMLONG(2));
	else if (scr_viewsize.value == 90)
		FPM_INC(view->origin[2]);
	else if (scr_viewsize.value == 80)
		view->origin[2]=FPM_ADD(view->origin[2], FPM_FROMFLOAT(0.5));

	view->model = clFPM.model_precache[clFPM.stats[STAT_WEAPON]];
	view->frame = clFPM.stats[STAT_WEAPONFRAME];
	view->colormap = vid.colormap;

// set up the refresh position
	VectorAdd (r_refdefFPM.viewangles, clFPM.punchangle, r_refdefFPM.viewangles);

	// smooth out stair step ups
	if (clFPM.onground && ent->origin[2] - oldz > 0)
	{
		fixedpoint_t steptime;

		steptime = FPM_FROMFLOAT(clFPM.time - clFPM.oldtime);
		if (steptime < 0)
	//FIXME		I_Error ("steptime < 0");
			steptime = 0;

		oldz = FPM_ADD(oldz, FPM_MUL(steptime, FPM_FROMLONG(80)));
		if (oldz > ent->origin[2])
			oldz = ent->origin[2];
		if (FPM_SUB(ent->origin[2], oldz) > FPM_FROMLONG(12))
			oldz = FPM_SUB(ent->origin[2], FPM_FROMLONG(12));
		r_refdefFPM.vieworg[2] = FPM_ADD(r_refdefFPM.vieworg[2], FPM_SUB(oldz, ent->origin[2]));
		view->origin[2] = FPM_ADD(view->origin[2], FPM_SUB(oldz, ent->origin[2]));
	}
	else
		oldz = ent->origin[2];

	if (chase_active.value)
		Chase_UpdateFPM ();
}
#endif //USEFPM

/*
==================
V_RenderView

The player's clipping box goes from (-16 -16 -24) to (16 16 32) from
the entity origin, so any view position inside that will be valid
==================
*/
extern vrect_t	scr_vrect;

void V_RenderView (void)
{

	if (con_forcedup)
		return;

// don't allow cheats in multiplayer
	if (cl.maxclients > 1)
	{
		Cvar_Set ("scr_ofsx", "0");
		Cvar_Set ("scr_ofsy", "0");
		Cvar_Set ("scr_ofsz", "0");
	}

	if (cl.intermission)
	{	// intermission / finale rendering
		V_CalcIntermissionRefdef ();
	}
	else
	{
//		if (!cl.paused /* && (sv.maxclients > 1 || key_dest == key_game) */ )
			V_CalcRefdef ();
	}


	R_PushDlights (); // 0.4 fps increase if removed

	if (lcd_x.value)
	{
		//
		// render two interleaved views
		//
		int		i;

		vid.rowbytes <<= 1;
		vid.aspect *= 0.5;

		r_refdef.viewangles[YAW] -= lcd_yaw.value;
		for (i=0 ; i<3 ; i++)
			r_refdef.vieworg[i] -= right[i]*lcd_x.value;

		R_RenderView ();

		vid.buffer += vid.rowbytes>>1;

		R_PushDlights ();

		r_refdef.viewangles[YAW] += lcd_yaw.value*2;
		for (i=0 ; i<3 ; i++)
			r_refdef.vieworg[i] += 2*right[i]*lcd_x.value;
		R_RenderView ();

		vid.buffer -= vid.rowbytes>>1;

		r_refdef.vrect.height <<= 1;

		vid.rowbytes >>= 1;
		vid.aspect *= 2;
	}
	else
	{

		R_RenderView ();

	}


#ifndef GLQUAKE

	if ((crosshair.value==1) || (crosshair.value>4)) {
		Draw_Character ((int)(scr_vrect.x + scr_vrect.width/2 + cl_crossx.value),
			(int)(scr_vrect.y + scr_vrect.height/2 + cl_crossy.value), '+');
	}

	if (crosshair.value==2) {
		Draw_Character ((int)(scr_vrect.x + scr_vrect.width/2 + cl_crossx.value),
			(int)(scr_vrect.y + scr_vrect.height/2 + cl_crossy.value), 'x');
	}

	if (crosshair.value==3) {
		Draw_Character ((int)(scr_vrect.x + scr_vrect.width/2 + cl_crossx.value),
			(int)(scr_vrect.y + scr_vrect.height/2 + cl_crossy.value), 'X');
	}

	if (crosshair.value==4) {
		Draw_Character ((int)(scr_vrect.x + scr_vrect.width/2 + cl_crossx.value),
			(int)(scr_vrect.y + scr_vrect.height/2 + cl_crossy.value), '.');
	} 
#endif


}

#ifdef USEFPM
void V_RenderViewFPM (void)
{
	if (con_forcedup)
		return;

// don't allow cheats in multiplayer
	if (clFPM.maxclients > 1)
	{
		Cvar_Set ("scr_ofsx", "0");
		Cvar_Set ("scr_ofsy", "0");
		Cvar_Set ("scr_ofsz", "0");
	}

	if (clFPM.intermission)
	{	// intermission / finale rendering
		V_CalcIntermissionRefdefFPM ();
	}
	else
	{
		if (!clFPM.paused /* && (sv.maxclients > 1 || key_dest == key_game) */ )
			V_CalcRefdefFPM ();
	}

	R_PushDlightsFPM ();

	if (lcd_x.value)
	{
		//
		// render two interleaved views
		//
		int		i;

		vid.rowbytes <<= 1;
		vid.aspect *= 0.5;

		r_refdefFPM.viewangles[YAW] = FPM_SUB(r_refdefFPM.viewangles[YAW], FPM_FROMFLOAT(lcd_yaw.value));
		for (i=0 ; i<3 ; i++)
			r_refdefFPM.vieworg[i] -= FPM_SUB(r_refdefFPM.vieworg[i], FPM_MUL(rightFPM[i],FPM_FROMFLOAT(lcd_x.value)));
		R_RenderViewFPM ();

		vid.buffer += vid.rowbytes>>1;

		R_PushDlightsFPM ();

		r_refdefFPM.viewangles[YAW] = FPM_ADD(r_refdefFPM.viewangles[YAW], FPM_MUL(FPM_FROMFLOAT(lcd_yaw.value), FPM_FROMLONG(2)));
		for (i=0 ; i<3 ; i++)
			r_refdefFPM.vieworg[i] += FPM_MUL(FPM_MUL(FPM_FROMLONG(2), rightFPM[i]), FPM_FROMFLOAT(lcd_x.value));
		R_RenderViewFPM ();

		vid.buffer -= vid.rowbytes>>1;

		r_refdefFPM.vrect.height <<= 1;

		vid.rowbytes >>= 1;
		vid.aspect *= 2;
	}
	else
	{

		R_RenderViewFPM ();

	}

	if(crosshair.value){
		Draw_Character ((int)(scr_vrect.x + scr_vrect.width/2), (int)(scr_vrect.y + scr_vrect.height/2 + cl_crossy.value), '+');
	}

}
#endif //USEFPM

//Dan East:
void V_Crosshair(void) {
	if (Cmd_Argv(1)[0]==0)
		//No param passed, so we default to 0
		crosshair.value=0;
	else {
		if (!strcmp(Cmd_Argv(1), "0")) crosshair.value=0;
		else if (!strcmp(Cmd_Argv(1), "1")) crosshair.value=1;
		else if (!strcmp(Cmd_Argv(1), "2")) crosshair.value=2;
		else if (!strcmp(Cmd_Argv(1), "3")) crosshair.value=3;
		else if (!strcmp(Cmd_Argv(1), "4")) crosshair.value=4;
		else Con_Print("Unknown argument. Usage: crosshair [0-4]");
	}
}

//============================================================================

/*
=============
V_Init
=============
*/
void V_Init (void)
{
	Cmd_AddCommand ("v_cshift", V_cshift_f);
#ifndef USEFPM
	Cmd_AddCommand ("bf", V_BonusFlash_f);
	Cmd_AddCommand ("centerview", V_StartPitchDrift);
#else
	Cmd_AddCommand ("bf", V_BonusFlash_FPM_f);
	Cmd_AddCommand ("centerview", V_StartPitchDriftFPM);
#endif
	//Dan East:
	Cmd_AddCommand ("crosshair", V_Crosshair);

	Cvar_RegisterVariable (&lcd_x);
	Cvar_RegisterVariable (&lcd_yaw);

	Cvar_RegisterVariable (&v_centermove);
	Cvar_RegisterVariable (&v_centerspeed);

	Cvar_RegisterVariable (&v_iyaw_cycle);
	Cvar_RegisterVariable (&v_iroll_cycle);
	Cvar_RegisterVariable (&v_ipitch_cycle);
	Cvar_RegisterVariable (&v_iyaw_level);
	Cvar_RegisterVariable (&v_iroll_level);
	Cvar_RegisterVariable (&v_ipitch_level);

	Cvar_RegisterVariable (&v_idlescale);
	Cvar_RegisterVariable (&crosshair);
	Cvar_RegisterVariable (&cl_crossx);
	Cvar_RegisterVariable (&cl_crossy);
	Cvar_RegisterVariable (&r_viewmodeloffset);
	Cvar_RegisterVariable (&gl_cshiftpercent);

	Cvar_RegisterVariable (&scr_ofsx);
	Cvar_RegisterVariable (&scr_ofsy);
	Cvar_RegisterVariable (&scr_ofsz);
	Cvar_RegisterVariable (&cl_rollspeed);
	Cvar_RegisterVariable (&cl_rollangle);
	Cvar_RegisterVariable (&cl_bob);
	Cvar_RegisterVariable (&cl_bobcycle);
	Cvar_RegisterVariable (&cl_bobup);

	Cvar_RegisterVariable (&v_kicktime);
	Cvar_RegisterVariable (&v_kickroll);
	Cvar_RegisterVariable (&v_kickpitch);

	BuildGammaTable (1.0);	// no gamma yet
	Cvar_RegisterVariable (&v_gamma);

}


