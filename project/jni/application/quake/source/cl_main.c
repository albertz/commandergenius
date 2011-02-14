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
// cl_main.c  -- client main loop

#include "quakedef.h"

// we need to declare some mouse variables here, because the menu system
// references them even when on a unix system.

// R2-Tec
cvar_t	command = {"_command", "", true};

// these two are not intended to be set directly
cvar_t	cl_name = {"_cl_name", "player", true};
cvar_t	cl_color = {"_cl_color", "0", true};

cvar_t  cl_showfps = {"cl_showfps","0"};// 2001-11-31 FPS display by QuakeForge/Muff
cvar_t	cl_shownet = {"cl_shownet","0"};	// can be 0, 1, or 2
cvar_t	cl_nolerp = {"cl_nolerp","0"};

cvar_t	lookspring = {"lookspring","0", true};
cvar_t	lookstrafe = {"lookstrafe","0", true};
cvar_t	sensitivity = {"sensitivity","3", true};

cvar_t	m_pitch = {"m_pitch","0.022", true};
cvar_t	m_yaw = {"m_yaw","0.022", true};
cvar_t	m_forward = {"m_forward","1", true};
cvar_t	m_side = {"m_side","0.8", true};

cvar_t  cl_config = {"cl_config","0"};

client_static_t	cls;
client_state_t	cl;

#ifdef USEFPM
client_state_FPM_t	clFPM;
#endif

// FIXME: put these on hunk?
efrag_t			cl_efrags[MAX_EFRAGS];
entity_t		cl_entities[MAX_EDICTS];
entity_t		cl_static_entities[MAX_STATIC_ENTITIES];
lightstyle_t	cl_lightstyle[MAX_LIGHTSTYLES];
dlight_t		cl_dlights[MAX_DLIGHTS];

#ifdef USEFPM
efrag_FPM_t		cl_efragsFPM[MAX_EFRAGS];
entity_FPM_t	cl_entitiesFPM[MAX_EDICTS];
entity_FPM_t	cl_static_entitiesFPM[MAX_STATIC_ENTITIES];
dlight_FPM_t	cl_dlightsFPM[MAX_DLIGHTS];
#endif

int				cl_numvisedicts;
entity_t		*cl_visedicts[MAX_VISEDICTS];
#ifdef USEFPM
entity_FPM_t	*cl_visedictsFPM[MAX_VISEDICTS];
#endif
/*
=====================
CL_ClearState

=====================
*/
void CL_ClearState (void)
{
	int			i;

	if (!sv.active)
		Host_ClearMemory ();

// wipe the entire cl structure
	Q_memset (&cl, 0, sizeof(cl));

	SZ_Clear (&cls.message);

// clear other arrays
	Q_memset (cl_efrags, 0, sizeof(cl_efrags));
	Q_memset (cl_entities, 0, sizeof(cl_entities));
	Q_memset (cl_dlights, 0, sizeof(cl_dlights));
	Q_memset (cl_lightstyle, 0, sizeof(cl_lightstyle));
	Q_memset (cl_temp_entities, 0, sizeof(cl_temp_entities));
	Q_memset (cl_beams, 0, sizeof(cl_beams));

//
// allocate the efrags and chain together into a free list
//
	cl.free_efrags = cl_efrags;
	for (i=0 ; i<MAX_EFRAGS-1 ; i++)
		cl.free_efrags[i].entnext = &cl.free_efrags[i+1];
	cl.free_efrags[i].entnext = NULL;
}

#ifdef USEFPM
void CL_ClearStateFPM (void)
{
	int			i;

	if (!sv.active)
		Host_ClearMemory ();

// wipe the entire cl structure
	Q_memset (&clFPM, 0, sizeof(clFPM));

	SZ_Clear (&cls.message);

// clear other arrays
	Q_memset (cl_efragsFPM, 0, sizeof(cl_efragsFPM));
	Q_memset (cl_entitiesFPM, 0, sizeof(cl_entitiesFPM));
	Q_memset (cl_dlightsFPM, 0, sizeof(cl_dlightsFPM));
	Q_memset (cl_lightstyle, 0, sizeof(cl_lightstyle));
	Q_memset (cl_temp_entitiesFPM, 0, sizeof(cl_temp_entitiesFPM));
	Q_memset (cl_beamsFPM, 0, sizeof(cl_beamsFPM));

//
// allocate the efrags and chain together into a free list
//
	clFPM.free_efrags = cl_efragsFPM;
	for (i=0 ; i<MAX_EFRAGS-1 ; i++)
		clFPM.free_efrags[i].entnext = &clFPM.free_efrags[i+1];
	clFPM.free_efrags[i].entnext = NULL;
}
#endif
/*
=====================
CL_Disconnect

Sends a disconnect message to the server
This is also called on Host_Error, so it shouldn't cause any errors
=====================
*/
void CL_Disconnect (void)
{
// stop sounds (especially looping!)
	S_StopAllSounds (true);

// bring the console down and fade the colors back to normal
//	SCR_BringDownConsole ();

// if running a local server, shut it down
	if (cls.demoplayback)
		CL_StopPlayback ();
	else if (cls.state == ca_connected)
	{
		if (cls.demorecording)
			CL_Stop_f ();

		Con_DPrintf ("Sending clc_disconnect\n");
		SZ_Clear (&cls.message);
		MSG_WriteByte (&cls.message, clc_disconnect);
		NET_SendUnreliableMessage (cls.netcon, &cls.message);
		SZ_Clear (&cls.message);
		NET_Close (cls.netcon);

		cls.state = ca_disconnected;
		if (sv.active)
			Host_ShutdownServer(false);
	}

	cls.demoplayback = cls.timedemo = false;
	cls.signon = 0;
}

#ifdef USEFPM
void CL_DisconnectFPM (void)
{
// stop sounds (especially looping!)
	S_StopAllSounds (true);

// bring the console down and fade the colors back to normal
//	SCR_BringDownConsole ();

// if running a local server, shut it down
	if (cls.demoplayback)
		CL_StopPlayback ();
	else if (cls.state == ca_connected)
	{
		if (cls.demorecording)
			CL_Stop_f ();

		Con_DPrintf ("Sending clc_disconnect\n");
		SZ_Clear (&cls.message);
		MSG_WriteByte (&cls.message, clc_disconnect);
		NET_SendUnreliableMessage (cls.netcon, &cls.message);
		SZ_Clear (&cls.message);
		NET_Close (cls.netcon);

		cls.state = ca_disconnected;
		if (svFPM.active)
			Host_ShutdownServerFPM(false);
	}

	cls.demoplayback = cls.timedemo = false;
	cls.signon = 0;
}
#endif

void CL_Disconnect_f (void)
{
	CL_Disconnect ();
	if (sv.active)
		Host_ShutdownServer (false);
}

#ifdef USEFPM
void CL_Disconnect_FPM_f (void)
{
	CL_DisconnectFPM ();
	if (svFPM.active)
		Host_ShutdownServerFPM (false);
}
#endif


/*
=====================
CL_EstablishConnection

Host should be either "local" or a net address to be passed on
=====================
*/
void CL_EstablishConnection (char *host) { 
	if (cls.state == ca_dedicated) 
		return; 

	if (cls.demoplayback) 
		return; 

	CL_Disconnect (); 
	cls.netcon = NET_Connect (host); 

	if (!cls.netcon) 
		Host_Error ("CL_Connect: connect failed\n"); 

	Con_DPrintf ("CL_EstablishConnection: connected to %s\n", host); 

	cls.demonum = -1; // not in the demo loop now 
	cls.state = ca_connected; 
	cls.signon = 0; // need all the signon messages before playing 

	MSG_WriteByte (&cls.message, clc_nop); // ProQuake NAT Fix 
}

#ifdef USEFPM
void CL_EstablishConnectionFPM (char *host)
{
	if (cls.state == ca_dedicated)
		return;

	if (cls.demoplayback)
		return;

	CL_DisconnectFPM ();

	cls.netcon = NET_Connect (host);
	if (!cls.netcon)
		Host_Error ("CL_Connect: connect failed\n");
	Con_DPrintf ("CL_EstablishConnection: connected to %s\n", host);

	cls.demonum = -1;			// not in the demo loop now
	cls.state = ca_connected;
	cls.signon = 0;				// need all the signon messages before playing
}
#endif
/*
=====================
CL_SignonReply

An svc_signonnum has been received, perform a client side setup
=====================
*/
void CL_SignonReply (void)
{
	char 	str[8192];

	GpError("CL_SignonReply",0);
	Con_DPrintf ("CL_SignonReply: %i\n", cls.signon);

	switch (cls.signon)
	{
	case 1:
		GpError("CL_SignonReply 1",1);
		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, "prespawn");
		break;

	case 2:
		GpError("CL_SignonReply 2",1);
		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, va("name \"%s\"\n", cl_name.string));

		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, va("color %i %i\n", ((int)cl_color.value)>>4, ((int)cl_color.value)&15));

		MSG_WriteByte (&cls.message, clc_stringcmd);
		sprintf (str, "spawn %s", cls.spawnparms);
		MSG_WriteString (&cls.message, str);
		break;

	case 3:
		GpError("CL_SignonReply 3",1);
		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, "begin");
		Cache_Report ();		// print remaining memory
		break;

	case 4:
		GpError("CL_SignonReply 4",1);
		SCR_EndLoadingPlaque ();		// allow normal screen updates
		break;
	}
}

/*
=====================
CL_NextDemo

Called to play the next demo in the demo loop
=====================
*/
void CL_NextDemo (void)
{
	char	str[1024];

	if (cls.demonum == -1)
		return;		// don't play demos

	SCR_BeginLoadingPlaque ();

	if (!cls.demos[cls.demonum][0] || cls.demonum == MAX_DEMOS)
	{
		cls.demonum = 0;
		if (!cls.demos[cls.demonum][0])
		{
			Con_Printf ("No demos listed with startdemos\n");
			cls.demonum = -1;
			return;
		}
	}

	sprintf (str,"playdemo %s\n", cls.demos[cls.demonum]);
	Cbuf_InsertText (str);
	cls.demonum++;
}

/*
==============
CL_PrintEntities_f
==============
*/
void CL_PrintEntities_f (void)
{
	entity_t	*ent;
	int			i;

	for (i=0,ent=cl_entities ; i<cl.num_entities ; i++,ent++)
	{
		Con_Printf ("%3i:",i);
		if (!ent->model)
		{
			Con_Printf ("EMPTY\n");
			continue;
		}
		Con_Printf ("%s:%2i  (%5.1f,%5.1f,%5.1f) [%5.1f %5.1f %5.1f]\n"
		,ent->model->name,ent->frame, ent->origin[0], ent->origin[1], ent->origin[2], ent->angles[0], ent->angles[1], ent->angles[2]);
	}
}

#ifdef USEFPM
void CL_PrintEntities_FPM_f (void)
{
	entity_FPM_t	*ent;
	int			i;

	for (i=0,ent=cl_entitiesFPM ; i<cl.num_entities ; i++,ent++)
	{
		Con_Printf ("%3i:",i);
		if (!ent->model)
		{
			Con_Printf ("EMPTY\n");
			continue;
		}
		Con_Printf ("%s:%2i  (%5.1f,%5.1f,%5.1f) [%5.1f %5.1f %5.1f]\n"
		,ent->model->name,ent->frame, ent->origin[0], ent->origin[1], ent->origin[2], ent->angles[0], ent->angles[1], ent->angles[2]);
	}
}
#endif
/*
===============
SetPal

Debugging tool, just flashes the screen
===============
*/
void SetPal (int i)
{
#if 0
	static int old;
	byte	pal[768];
	int		c;

	if (i == old)
		return;
	old = i;

	if (i==0)
		VID_SetPalette (host_basepal);
	else if (i==1)
	{
		for (c=0 ; c<768 ; c+=3)
		{
			pal[c] = 0;
			pal[c+1] = 255;
			pal[c+2] = 0;
		}
		VID_SetPalette (pal);
	}
	else
	{
		for (c=0 ; c<768 ; c+=3)
		{
			pal[c] = 0;
			pal[c+1] = 0;
			pal[c+2] = 255;
		}
		VID_SetPalette (pal);
	}
#endif
}

/*
===============
CL_AllocDlight

===============
*/
dlight_t *CL_AllocDlight (int key)
{
	int		i;
	dlight_t	*dl;

// first look for an exact key match
	if (key)
	{
		dl = cl_dlights;
		for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
		{
			if (dl->key == key)
			{
				Q_memset (dl, 0, sizeof(*dl));
				dl->key = key;
				return dl;
			}
		}
	}

// then look for anything else
	dl = cl_dlights;
	for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
	{
		if (dl->die < cl.time)
		{
			Q_memset (dl, 0, sizeof(*dl));
			dl->key = key;
			return dl;
		}
	}

	dl = &cl_dlights[0];
	Q_memset (dl, 0, sizeof(*dl));
	dl->key = key;
	return dl;
}

#ifdef USEFPM
dlight_FPM_t *CL_AllocDlightFPM (int key)
{
	int				i;
	dlight_FPM_t	*dl;

// first look for an exact key match
	if (key)
	{
		dl = cl_dlightsFPM;
		for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
		{
			if (dl->key == key)
			{
				Q_memset (dl, 0, sizeof(*dl));
				dl->key = key;
				return dl;
			}
		}
	}

// then look for anything else
	dl = cl_dlightsFPM;
	for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
	{
		if (dl->die < clFPM.time)
		{
			Q_memset (dl, 0, sizeof(*dl));
			dl->key = key;
			return dl;
		}
	}

	dl = &cl_dlightsFPM[0];
	Q_memset (dl, 0, sizeof(*dl));
	dl->key = key;
	return dl;
}
#endif
/*
===============
CL_DecayLights

===============
*/
void CL_DecayLights (void)
{
	int			i;
	dlight_t	*dl;
	float		time;

	time = (float)(cl.time - cl.oldtime);

	dl = cl_dlights;
	for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
	{
		if (dl->die < cl.time || !dl->radius)
			continue;

		dl->radius -= time*dl->decay;
		if (dl->radius < 0)
			dl->radius = 0;
	}
}

#ifdef USEFPM
void CL_DecayLightsFPM (void)
{
	int				i;
	dlight_FPM_t	*dl;
	fixedpoint_t	time;

	time = FPM_FROMFLOAT(clFPM.time - clFPM.oldtime);

	dl = cl_dlightsFPM;
	for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
	{
		if (dl->die < clFPM.time || !dl->radius)
			continue;

		dl->radius -= FPM_MUL(time, dl->decay);
		if (dl->radius < 0)
			dl->radius = 0;
	}
}
#endif
/*
===============
CL_LerpPoint

Determines the fraction between the last two messages that the objects
should be put at.
===============
*/
float	CL_LerpPoint (void)
{
	float	f, frac;

	f = (float)(cl.mtime[0] - cl.mtime[1]);

	if (!f || cl_nolerp.value || cls.timedemo || sv.active)
	{
		cl.time = cl.mtime[0];
		return 1;
	}

	if (f > 0.1)
	{	// dropped packet, or start of demo
		cl.mtime[1] = cl.mtime[0] - 0.1;
		f = (float)0.1;
	}
	frac = (float)((cl.time - cl.mtime[1]) / f);
//Con_Printf ("frac: %f\n",frac);
	if (frac < 0)
	{
		if (frac < -0.01)
		{
SetPal(1);
			cl.time = cl.mtime[1];
//				Con_Printf ("low frac\n");
		}
		frac = 0;
	}
	else if (frac > 1)
	{
		if (frac > 1.01)
		{
SetPal(2);
			cl.time = cl.mtime[0];
//				Con_Printf ("high frac\n");
		}
		frac = 1;
	}
	else
		SetPal(0);

	return frac;
}

#ifdef USEFPM
fixedpoint_t	CL_LerpPointFPM (void)
{
	fixedpoint_t	f, frac;

	f = FPM_FROMFLOAT(clFPM.mtime[0] - clFPM.mtime[1]);

	if (!f || cl_nolerp.value || cls.timedemo || sv.active)
	{
		clFPM.time = clFPM.mtime[0];
		return 1;
	}

	if (f > FPM_FROMFLOAT(0.1))
	{	// dropped packet, or start of demo
		clFPM.mtime[1] = clFPM.mtime[0] - 0.1;
		f = FPM_FROMFLOAT(0.1);
	}
	frac = FPM_DIV(FPM_FROMFLOAT(clFPM.time - clFPM.mtime[1]), f);
//Con_Printf ("frac: %f\n",frac);
	if (frac < 0)
	{
		if (frac < FPM_FROMFLOAT(-0.01))
		{
			SetPal(1);
			clFPM.time = clFPM.mtime[1];
//				Con_Printf ("low frac\n");
		}
		frac = 0;
	}
	else if (frac > FPM_FROMLONG(1))
	{
		if (frac > FPM_FROMFLOAT(1.01))
		{
			SetPal(2);
			clFPM.time = clFPM.mtime[0];
//				Con_Printf ("high frac\n");
		}
		frac = FPM_FROMLONG(1);
	}
	else
		SetPal(0);

	return frac;
}
#endif
/*
===============
CL_RelinkEntities
===============
*/
void CL_RelinkEntities (void)
{
	entity_t	*ent;
	int			i, j;
	float		frac, f, d;
	vec3_t		delta;
	float		bobjrotate;
	vec3_t		oldorg;
	dlight_t	*dl;

// determine partial update time
	frac = CL_LerpPoint ();

	cl_numvisedicts = 0;

//
// interpolate player info
//
	for (i=0 ; i<3 ; i++)
		cl.velocity[i] = cl.mvelocity[1][i] +
			frac * (cl.mvelocity[0][i] - cl.mvelocity[1][i]);

	if (cls.demoplayback)
	{
	// interpolate the angles
		for (j=0 ; j<3 ; j++)
		{
			d = cl.mviewangles[0][j] - cl.mviewangles[1][j];
			if (d > 180)
				d -= 360;
			else if (d < -180)
				d += 360;
			cl.viewangles[j] = cl.mviewangles[1][j] + frac*d;
		}
	}

	bobjrotate = anglemod((float)(100*cl.time));

// start on the entity after the world
	for (i=1,ent=cl_entities+1 ; i<cl.num_entities ; i++,ent++)
	{
		if (!ent->model)
		{	// empty slot
			if (ent->forcelink)
				R_RemoveEfrags (ent);	// just became empty
			continue;
		}

// if the object wasn't included in the last packet, remove it
		if (ent->msgtime != cl.mtime[0])
		{
			ent->model = NULL;
			continue;
		}

		VectorCopy (ent->origin, oldorg);

		if (ent->forcelink)
		{	// the entity was not updated in the last message
			// so move to the final spot
			VectorCopy (ent->msg_origins[0], ent->origin);
			VectorCopy (ent->msg_angles[0], ent->angles);
		}
		else
		{	// if the delta is large, assume a teleport and don't lerp
			f = frac;
			for (j=0 ; j<3 ; j++)
			{
				delta[j] = ent->msg_origins[0][j] - ent->msg_origins[1][j];
				if (delta[j] > 100 || delta[j] < -100)
					f = 1;		// assume a teleportation, not a motion
			}

		// interpolate the origin and angles
			for (j=0 ; j<3 ; j++)
			{
				ent->origin[j] = ent->msg_origins[1][j] + f*delta[j];

				d = ent->msg_angles[0][j] - ent->msg_angles[1][j];
				if (d > 180)
					d -= 360;
				else if (d < -180)
					d += 360;
				ent->angles[j] = ent->msg_angles[1][j] + f*d;
			}
		}

// rotate binary objects locally
		if (ent->model->flags & EF_ROTATE) {
			ent->angles[1] = bobjrotate;
			ent->origin[2] += (( sin(bobjrotate/90*M_PI) * 5) + 5 );
		}

		if (ent->effects & EF_BRIGHTFIELD)
			R_EntityParticles (ent);
#ifdef QUAKE2
		if (ent->effects & EF_DARKFIELD)
			R_DarkFieldParticles (ent);
#endif
		if (ent->effects & EF_MUZZLEFLASH)
		{
			vec3_t		fv, rv, uv;

			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin,  dl->origin);
			dl->origin[2] += 16;
			AngleVectors (ent->angles, fv, rv, uv);

			VectorMA (dl->origin, 18, fv, dl->origin);
			dl->radius = (float)(200 + (rand()&31));
			dl->minlight = 32;
			dl->die = (float)(cl.time + 0.1);
		}
		if (ent->effects & EF_BRIGHTLIGHT)
		{
			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin,  dl->origin);
			dl->origin[2] += 16;
			dl->radius = (float)(400 + (rand()&31));
			dl->die = (float)(cl.time + 0.001);
		}
		if (ent->effects & EF_DIMLIGHT)
		{
			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin,  dl->origin);
			dl->radius = (float)(200 + (rand()&31));
			dl->die = (float)(cl.time + 0.001);
		}
#ifdef QUAKE2
		if (ent->effects & EF_DARKLIGHT)
		{
			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin,  dl->origin);
			dl->radius = 200.0 + (rand()&31);
			dl->die = cl.time + 0.001;
			dl->dark = true;
		}
		if (ent->effects & EF_LIGHT)
		{
			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin,  dl->origin);
			dl->radius = 200;
			dl->die = cl.time + 0.001;
		}
#endif

		if (ent->model->flags & EF_GIB)
			R_RocketTrail (oldorg, ent->origin, 2);
		else if (ent->model->flags & EF_ZOMGIB)
			R_RocketTrail (oldorg, ent->origin, 4);
		else if (ent->model->flags & EF_TRACER)
			R_RocketTrail (oldorg, ent->origin, 3);
		else if (ent->model->flags & EF_TRACER2)
			R_RocketTrail (oldorg, ent->origin, 5);
		else if (ent->model->flags & EF_ROCKET)
		{
			R_RocketTrail (oldorg, ent->origin, 0);
			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin, dl->origin);
			dl->radius = 200;
			dl->die = (float)(cl.time + 0.01);
		}
		else if (ent->model->flags & EF_GRENADE)
			R_RocketTrail (oldorg, ent->origin, 1);
		else if (ent->model->flags & EF_TRACER3)
			R_RocketTrail (oldorg, ent->origin, 6);

		ent->forcelink = false;

		if (i == cl.viewentity && !chase_active.value)
			continue;

#ifdef QUAKE2
		if ( ent->effects & EF_NODRAW )
			continue;
#endif
		if (cl_numvisedicts < MAX_VISEDICTS)
		{
			cl_visedicts[cl_numvisedicts] = ent;
			cl_numvisedicts++;
		}
	}
}

#ifdef USEFPM
void CL_RelinkEntitiesFPM (void)
{
	entity_FPM_t	*ent;
	int				i, j;
	fixedpoint_t	frac, f, d;
	vec3_FPM_t		delta;
	fixedpoint_t	bobjrotate;
	vec3_FPM_t		oldorg;
	dlight_FPM_t	*dl;

// determine partial update time
	frac = CL_LerpPointFPM ();

	cl_numvisedicts = 0;

//
// interpolate player info
//
	for (i=0 ; i<3 ; i++)
		clFPM.velocity[i] = FPM_ADD(clFPM.mvelocity[1][i],
			FPM_MUL(frac, FPM_SUB(clFPM.mvelocity[0][i], clFPM.mvelocity[1][i])));

	if (cls.demoplayback)
	{
	// interpolate the angles
		for (j=0 ; j<3 ; j++)
		{
			d = FPM_SUB(clFPM.mviewangles[0][j], clFPM.mviewangles[1][j]);
			if (d > FPM_FROMLONGC(180))
				d = FPM_SUB(d, FPM_FROMLONGC(360));
			else if (d < FPM_FROMLONGC(-180))
				d = FPM_ADD(d, FPM_FROMLONGC(360));
			clFPM.viewangles[j] = FPM_ADD(clFPM.mviewangles[1][j], FPM_MUL(frac,d));
		}
	}

	bobjrotate = anglemodFPM(FPM_MUL(FPM_FROMLONGC(100),FPM_FROMFLOAT(clFPM.time)));

// start on the entity after the world
	for (i=1,ent=cl_entitiesFPM+1 ; i<clFPM.num_entities ; i++,ent++)
	{
		if (!ent->model)
		{	// empty slot
			if (ent->forcelink)
				R_RemoveEfragsFPM (ent);	// just became empty
			continue;
		}

// if the object wasn't included in the last packet, remove it
		if (ent->msgtime != clFPM.mtime[0])
		{
			ent->model = NULL;
			continue;
		}

		VectorCopy (ent->origin, oldorg);

		if (ent->forcelink)
		{	// the entity was not updated in the last message
			// so move to the final spot
			VectorCopy (ent->msg_origins[0], ent->origin);
			VectorCopy (ent->msg_angles[0], ent->angles);
		}
		else
		{	// if the delta is large, assume a teleport and don't lerp
			f = frac;
			for (j=0 ; j<3 ; j++)
			{
				delta[j] = FPM_SUB(ent->msg_origins[0][j], ent->msg_origins[1][j]);
				if (delta[j] > FPM_FROMLONGC(100) || delta[j] < FPM_FROMLONGC(-100))
					f = FPM_FROMLONG(1);		// assume a teleportation, not a motion
			}

		// interpolate the origin and angles
			for (j=0 ; j<3 ; j++)
			{
				ent->origin[j] = FPM_ADD(ent->msg_origins[1][j], FPM_MUL(f,delta[j]));

				d = FPM_SUB(ent->msg_angles[0][j], ent->msg_angles[1][j]);
				if (d > FPM_FROMLONGC(180))
					d = FPM_SUB(d, FPM_FROMLONGC(360));
				else if (d < FPM_FROMLONGC(-180))
					d = FPM_ADD(d, FPM_FROMLONGC(360));
				ent->angles[j] = FPM_ADD(ent->msg_angles[1][j], FPM_MUL(f,d));
			}

		}

// rotate binary objects locally
		if (ent->model->flags & EF_ROTATE)
			ent->angles[1] = bobjrotate;

		if (ent->effects & EF_BRIGHTFIELD)
			R_EntityParticlesFPM (ent);
#ifdef QUAKE2
		if (ent->effects & EF_DARKFIELD)
			R_DarkFieldParticlesFPM (ent);
#endif
		if (ent->effects & EF_MUZZLEFLASH)
		{
			vec3_FPM_t	fv, rv, uv;

			dl = CL_AllocDlightFPM (i);
			VectorCopy (ent->origin,  dl->origin);
			dl->origin[2] = FPM_ADD(dl->origin[2], FPM_FROMLONGC(16));
			AngleVectorsFPM (ent->angles, fv, rv, uv);

			VectorMAFPM (dl->origin, FPM_FROMLONG(18), fv, dl->origin);
			dl->radius = FPM_FROMLONG(200 + (rand()&31));
			dl->minlight = 32;
			dl->die = FPM_ADD(FPM_FROMFLOAT(clFPM.time), FPM_FROMFLOATC(0.1));
		}
		if (ent->effects & EF_BRIGHTLIGHT)
		{
			dl = CL_AllocDlightFPM (i);
			VectorCopy (ent->origin,  dl->origin);
			dl->origin[2] = FPM_ADD(dl->origin[2], FPM_FROMLONGC(16));
			dl->radius = FPM_FROMLONG(400 + (rand()&31));
			dl->die = FPM_ADD(FPM_FROMFLOAT(clFPM.time), FPM_FROMFLOATC(0.001));
		}
		if (ent->effects & EF_DIMLIGHT)
		{
			dl = CL_AllocDlightFPM (i);
			VectorCopy (ent->origin,  dl->origin);
			dl->radius = FPM_FROMLONG(200 + (rand()&31));
			dl->die = FPM_ADD(FPM_FROMFLOAT(clFPM.time), FPM_FROMFLOATC(0.001));
		}
#ifdef QUAKE2
		if (ent->effects & EF_DARKLIGHT)
		{
			dl = CL_AllocDlightFPM (i);
			VectorCopy (ent->origin,  dl->origin);
			dl->radius = FPM_FROMLONG(200.0 + (rand()&31));
			dl->die = FPM_ADD(FPM_FROMFLOAT(clFPM.time), FPM_FROMFLOATC(0.001));
			dl->dark = true;
		}
		if (ent->effects & EF_LIGHT)
		{
			dl = CL_AllocDlightFPM (i);
			VectorCopy (ent->origin,  dl->origin);
			dl->radius = FPM_FROMLONG(200);
			dl->die = FPM_ADD(FPM_FROMFLOAT(clFPM.time), FPM_FROMFLOATC(0.001));
		}
#endif

		if (ent->model->flags & EF_GIB)
			R_RocketTrailFPM (oldorg, ent->origin, 2);
		else if (ent->model->flags & EF_ZOMGIB)
			R_RocketTrailFPM (oldorg, ent->origin, 4);
		else if (ent->model->flags & EF_TRACER)
			R_RocketTrailFPM (oldorg, ent->origin, 3);
		else if (ent->model->flags & EF_TRACER2)
			R_RocketTrailFPM (oldorg, ent->origin, 5);
		else if (ent->model->flags & EF_ROCKET)
		{
			R_RocketTrailFPM (oldorg, ent->origin, 0);
			dl = CL_AllocDlightFPM (i);
			VectorCopy (ent->origin, dl->origin);
			dl->radius = FPM_FROMLONGC(200);
			dl->die = FPM_ADD(FPM_FROMFLOAT(clFPM.time), FPM_FROMFLOATC(0.01));
		}
		else if (ent->model->flags & EF_GRENADE)
			R_RocketTrailFPM (oldorg, ent->origin, 1);
		else if (ent->model->flags & EF_TRACER3)
			R_RocketTrailFPM (oldorg, ent->origin, 6);

		ent->forcelink = false;

		if (i == clFPM.viewentity && !chase_active.value)
			continue;

#ifdef QUAKE2
		if ( ent->effects & EF_NODRAW )
			continue;
#endif
		if (cl_numvisedicts < MAX_VISEDICTS)
		{
			cl_visedictsFPM[cl_numvisedicts] = ent;
			cl_numvisedicts++;
		}
	}
}
#endif
/*
===============
CL_ReadFromServer

Read all incoming data from the server
===============
*/
int CL_ReadFromServer (void)
{
	int		ret;

	cl.oldtime = cl.time;
	cl.time += host_frametime;

	do
	{


		ret = CL_GetMessage ();
		if (ret == -1){
			Host_Error ("CL_ReadFromServer: lost server connection");
		}
		if (!ret)
			break;


		cl.last_received_message = (float)realtime;
		CL_ParseServerMessage ();


	} while (ret && cls.state == ca_connected);


	if (cl_shownet.value)
		Con_Printf ("\n");

	CL_RelinkEntities ();
	CL_UpdateTEnts ();

//
// bring the links up to date
//

	return 0;
}

#ifdef USEFPM
int CL_ReadFromServerFPM (void)
{
	int		ret;

	clFPM.oldtime = clFPM.time;
	clFPM.time += host_frametime;

	do
	{
		ret = CL_GetMessageFPM ();
		if (ret == -1)
			Host_Error ("CL_ReadFromServer: lost server connection");
		if (!ret)
			break;

		clFPM.last_received_message = (float)realtime;
		CL_ParseServerMessageFPM ();
	} while (ret && cls.state == ca_connected);

	if (cl_shownet.value)
		Con_Printf ("\n");

	CL_RelinkEntitiesFPM ();
	CL_UpdateTEntsFPM ();

//
// bring the links up to date
//
	return 0;
}
#endif
/*
=================
CL_SendCmd
=================
*/
void CL_SendCmd (void)
{
	usercmd_t		cmd;

	if (cls.state != ca_connected)
		return;

	GpError("CL_SendCmd",2);

	if (cls.signon == SIGNONS)
	{
		GpError("CL_SendCmd send signons",0);
	// get basic movement from keyboard
		CL_BaseMove (&cmd);

	// allow mice or other external controllers to add to the move
		IN_Move (&cmd);

	// send the unreliable message
		CL_SendMove (&cmd);

	}

	if (cls.demoplayback)
	{
		SZ_Clear (&cls.message);
		return;
	}

// send the reliable message
	if (!cls.message.cursize){
		GpError("CL_SendCmd no mes",0);
		return;		// no message at all
	}

	if (!NET_CanSendMessage (cls.netcon))
	{
		GpError("CL_SendCmd cant send",0);
		Con_DPrintf ("CL_WriteToServer: can't send\n");
		return;
	}

	if (NET_SendMessage (cls.netcon, &cls.message) == -1){
		GpError("CL_SendCmd lost con",2);
		Host_Error ("CL_WriteToServer: lost server connection");
	}

	SZ_Clear (&cls.message);
	GpError("CL_SendCmd done",1);
}

#ifdef USEFPM
void CL_SendCmdFPM (void)
{
	usercmd_FPM_t		cmd;

	if (cls.state != ca_connected)
		return;

	if (cls.signon == SIGNONS)
	{
	// get basic movement from keyboard
		CL_BaseMoveFPM (&cmd);

	// allow mice or other external controllers to add to the move
//		IN_MoveFPM (&cmd);

	// send the unreliable message
		CL_SendMoveFPM (&cmd);

	}

	if (cls.demoplayback)
	{
		SZ_Clear (&cls.message);
		return;
	}

// send the reliable message
	if (!cls.message.cursize)
		return;		// no message at all

	if (!NET_CanSendMessage (cls.netcon))
	{
		Con_DPrintf ("CL_WriteToServer: can't send\n");
		return;
	}

	if (NET_SendMessage (cls.netcon, &cls.message) == -1)
		Host_Error ("CL_WriteToServer: lost server connection");

	SZ_Clear (&cls.message);
}
#endif
/*
=================
CL_Init
=================
*/
void CL_Init (void)
{
	SZ_Alloc (&cls.message, 1024);

	CL_InitInput ();
	CL_InitTEnts ();

//
// register our commands
//
	Cvar_RegisterVariable (&command);
	Cvar_RegisterVariable (&cl_name);
	Cvar_RegisterVariable (&cl_color);
	Cvar_RegisterVariable (&cl_upspeed);
	Cvar_RegisterVariable (&cl_forwardspeed);
	Cvar_RegisterVariable (&cl_backspeed);
	Cvar_RegisterVariable (&cl_sidespeed);
	Cvar_RegisterVariable (&cl_movespeedkey);
	Cvar_RegisterVariable (&cl_yawspeed);
	Cvar_RegisterVariable (&cl_pitchspeed);
	Cvar_RegisterVariable (&cl_anglespeedkey);
	Cvar_RegisterVariable (&cl_showfps);// 2001-11-31 FPS display by QuakeForge/Muff
	Cvar_RegisterVariable (&cl_shownet);
	Cvar_RegisterVariable (&cl_nolerp);
	Cvar_RegisterVariable (&lookspring);
	Cvar_RegisterVariable (&lookstrafe);
	Cvar_RegisterVariable (&sensitivity);

	Cvar_RegisterVariable (&m_pitch);
	Cvar_RegisterVariable (&m_yaw);
	Cvar_RegisterVariable (&m_forward);
	Cvar_RegisterVariable (&m_side);

	Cvar_RegisterVariable (&cl_config);

//	Cvar_RegisterVariable (&cl_autofire);

	Cmd_AddCommand ("entities", CL_PrintEntities_f);
	Cmd_AddCommand ("disconnect", CL_Disconnect_f);
	Cmd_AddCommand ("record", CL_Record_f);
	Cmd_AddCommand ("stop", CL_Stop_f);
	//Cmd_AddCommand ("gpplaydemo", CL_PlayDemo_f);
	//	Cmd_AddCommand ("playdemo", CL_PlayDemo_silentfail);
	Cmd_AddCommand ("playdemo", CL_PlayDemo_f);
	Cmd_AddCommand ("timedemo", CL_TimeDemo_f);
}

