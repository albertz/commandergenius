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
// sv_main.c -- server main program

#include "quakedef.h"

server_t			sv;
server_static_t		svs;

#ifdef USEFPM
server_FPM_t		svFPM;
server_static_FPM_t	svsFPM;
#endif //USEFPM

char	localmodels[MAX_MODELS][5];			// inline model names for precache

//============================================================================

/*
===============
SV_Init
===============
*/
void SV_Init (void)
{
	int		i;
	extern	cvar_t	sv_maxvelocity;
	extern	cvar_t	sv_gravity;
	extern	cvar_t	sv_nostep;
	extern	cvar_t	sv_friction;
	extern	cvar_t	sv_edgefriction;
	extern	cvar_t	sv_stopspeed;
	extern	cvar_t	sv_maxspeed;
	extern	cvar_t	sv_accelerate;
	extern	cvar_t	sv_idealpitchscale;
	extern	cvar_t	sv_aim;

	Cvar_RegisterVariable (&sv_maxvelocity);
	Cvar_RegisterVariable (&sv_gravity);
	Cvar_RegisterVariable (&sv_friction);
	Cvar_RegisterVariable (&sv_edgefriction);
	Cvar_RegisterVariable (&sv_stopspeed);
	Cvar_RegisterVariable (&sv_maxspeed);
	Cvar_RegisterVariable (&sv_accelerate);
	Cvar_RegisterVariable (&sv_idealpitchscale);
	Cvar_RegisterVariable (&sv_aim);
	Cvar_RegisterVariable (&sv_nostep);

	for (i=0 ; i<MAX_MODELS ; i++)
		sprintf (localmodels[i], "*%i", i);
}

/*
=============================================================================

EVENT MESSAGES

=============================================================================
*/

/*
==================
SV_StartParticle

Make sure the event gets sent to all clients
==================
*/
void SV_StartParticle (vec3_t org, vec3_t dir, int color, int count)
{
	int		i, v;

	if (sv.datagram.cursize > MAX_DATAGRAM-16)
		return;
	MSG_WriteByte (&sv.datagram, svc_particle);
	MSG_WriteCoord (&sv.datagram, org[0]);
	MSG_WriteCoord (&sv.datagram, org[1]);
	MSG_WriteCoord (&sv.datagram, org[2]);
	for (i=0 ; i<3 ; i++)
	{
		v = (int)(dir[i]*16);
		if (v > 127)
			v = 127;
		else if (v < -128)
			v = -128;
		MSG_WriteChar (&sv.datagram, v);
	}
	MSG_WriteByte (&sv.datagram, count);
	MSG_WriteByte (&sv.datagram, color);
}

#ifdef USEFPM
void SV_StartParticleFPM (vec3_FPM_t org, vec3_FPM_t dir, int color, int count)
{
	int		i, v;

	if (svFPM.datagram.cursize > MAX_DATAGRAM-16)
		return;
	MSG_WriteByte (&svFPM.datagram, svc_particle);
	MSG_WriteCoordFPM (&svFPM.datagram, org[0]);
	MSG_WriteCoordFPM (&svFPM.datagram, org[1]);
	MSG_WriteCoordFPM (&svFPM.datagram, org[2]);
	for (i=0 ; i<3 ; i++)
	{
		v = FPM_TOLONG(FPM_MUL(dir[i],FPM_FROMLONGC(16)));
		if (v > 127)
			v = 127;
		else if (v < -128)
			v = -128;
		MSG_WriteChar (&svFPM.datagram, v);
	}
	MSG_WriteByte (&svFPM.datagram, count);
	MSG_WriteByte (&svFPM.datagram, color);
}
#endif //USEFPM

/*
==================
SV_StartSound

Each entity can have eight independant sound sources, like voice,
weapon, feet, etc.

Channel 0 is an auto-allocate channel, the others override anything
allready running on that entity/channel pair.

An attenuation of 0 will play full volume everywhere in the level.
Larger attenuations will drop off.  (max 4 attenuation)

==================
*/
void SV_StartSound (edict_t *entity, int channel, char *sample, int volume,
    float attenuation)
{
    int         sound_num;
    int field_mask;
    int			i;
	int			ent;

	if (volume < 0 || volume > 255)
		Sys_Error ("SV_StartSound: volume = %i", volume);

	if (attenuation < 0 || attenuation > 4)
		Sys_Error ("SV_StartSound: attenuation = %f", attenuation);

	if (channel < 0 || channel > 7)
		Sys_Error ("SV_StartSound: channel = %i", channel);

	if (sv.datagram.cursize > MAX_DATAGRAM-16)
		return;

// find precache number for sound
    for (sound_num=1 ; sound_num<MAX_SOUNDS
        && sv.sound_precache[sound_num] ; sound_num++)
        if (!strcmp(sample, sv.sound_precache[sound_num]))
            break;

    if ( sound_num == MAX_SOUNDS || !sv.sound_precache[sound_num] )
    {
        Con_Printf ("SV_StartSound: %s not precacheed\n", sample);
        return;
    }

	ent = NUM_FOR_EDICT(entity);

	channel = (ent<<3) | channel;

	field_mask = 0;
	if (volume != DEFAULT_SOUND_PACKET_VOLUME)
		field_mask |= SND_VOLUME;
	if (attenuation != DEFAULT_SOUND_PACKET_ATTENUATION)
		field_mask |= SND_ATTENUATION;

// directed messages go only to the entity the are targeted on
	MSG_WriteByte (&sv.datagram, svc_sound);
	MSG_WriteByte (&sv.datagram, field_mask);
	if (field_mask & SND_VOLUME)
		MSG_WriteByte (&sv.datagram, volume);
	if (field_mask & SND_ATTENUATION)
		MSG_WriteByte (&sv.datagram, (int)(attenuation*64));
	MSG_WriteShort (&sv.datagram, channel);
	MSG_WriteByte (&sv.datagram, sound_num);
	for (i=0 ; i<3 ; i++)
		MSG_WriteCoord (&sv.datagram, (float)(entity->v.origin[i]+0.5*(entity->v.mins[i]+entity->v.maxs[i])));
}

#ifdef USEFPM
void SV_StartSoundFPM (edict_FPM_t *entity, int channel, char *sample, int volume,
    float attenuation)
{
    int         sound_num;
    int field_mask;
    int			i;
	int			ent;

	if (volume < 0 || volume > 255)
		Sys_Error ("SV_StartSound: volume = %i", volume);

	if (attenuation < 0 || attenuation > 4)
		Sys_Error ("SV_StartSound: attenuation = %f", attenuation);

	if (channel < 0 || channel > 7)
		Sys_Error ("SV_StartSound: channel = %i", channel);

	if (svFPM.datagram.cursize > MAX_DATAGRAM-16)
		return;

// find precache number for sound
    for (sound_num=1 ; sound_num<MAX_SOUNDS
        && svFPM.sound_precache[sound_num] ; sound_num++)
        if (!strcmp(sample, svFPM.sound_precache[sound_num]))
            break;

    if ( sound_num == MAX_SOUNDS || !svFPM.sound_precache[sound_num] )
    {
        Con_Printf ("SV_StartSound: %s not precacheed\n", sample);
        return;
    }

	ent = NUM_FOR_EDICTFPM(entity);

	channel = (ent<<3) | channel;

	field_mask = 0;
	if (volume != DEFAULT_SOUND_PACKET_VOLUME)
		field_mask |= SND_VOLUME;
	if (attenuation != DEFAULT_SOUND_PACKET_ATTENUATION)
		field_mask |= SND_ATTENUATION;

// directed messages go only to the entity the are targeted on
	MSG_WriteByte (&svFPM.datagram, svc_sound);
	MSG_WriteByte (&svFPM.datagram, field_mask);
	if (field_mask & SND_VOLUME)
		MSG_WriteByte (&svFPM.datagram, volume);
	if (field_mask & SND_ATTENUATION)
		MSG_WriteByte (&svFPM.datagram, (int)(attenuation*64));
	MSG_WriteShort (&svFPM.datagram, channel);
	MSG_WriteByte (&svFPM.datagram, sound_num);
	for (i=0 ; i<3 ; i++)
		MSG_WriteCoord (&svFPM.datagram, (float)(entity->v.origin[i]+0.5*(entity->v.mins[i]+entity->v.maxs[i])));
}
#endif //USEFPM

/*
==============================================================================

CLIENT SPAWNING

==============================================================================
*/

/*
================
SV_SendServerinfo

Sends the first message from the server to a connected client.
This will be sent on the initial connection and upon each server load.
================
*/
void SV_SendServerinfo (client_t *client) {
	char			**s;
	char			message[2048];

	MSG_WriteByte (&client->message, svc_print);
	sprintf (message, "%c\nVERSION %4.2f SERVER (%i CRC)", 2, (float)GP2X_VERSION, pr_crc);
	MSG_WriteString (&client->message,message);

	MSG_WriteByte (&client->message, svc_serverinfo);
	MSG_WriteLong (&client->message, PROTOCOL_VERSION);
	MSG_WriteByte (&client->message, svs.maxclients);

	if (!coop.value && deathmatch.value)
		MSG_WriteByte (&client->message, GAME_DEATHMATCH);
	else
		MSG_WriteByte (&client->message, GAME_COOP);

	sprintf (message, pr_strings+sv.edicts->v.message);

	MSG_WriteString (&client->message,message);

	for (s = sv.model_precache+1 ; *s ; s++)
		MSG_WriteString (&client->message, *s);
	MSG_WriteByte (&client->message, 0);

	for (s = sv.sound_precache+1 ; *s ; s++)
		MSG_WriteString (&client->message, *s);
	MSG_WriteByte (&client->message, 0);

// send music
	MSG_WriteByte (&client->message, svc_cdtrack);
	MSG_WriteByte (&client->message, (int)sv.edicts->v.sounds);
	MSG_WriteByte (&client->message, (int)sv.edicts->v.sounds);

// set view
	MSG_WriteByte (&client->message, svc_setview);
	MSG_WriteShort (&client->message, NUM_FOR_EDICT(client->edict));

	MSG_WriteByte (&client->message, svc_signonnum);
	MSG_WriteByte (&client->message, 1);

	client->sendsignon = true;
	client->spawned = false;		// need prespawn, spawn, etc
}

#ifdef USEFPM
void SV_SendServerinfoFPM (client_FPM_t *client)
{
	char			**s;
	char			message[2048];


	MSG_WriteByte (&client->message, svc_print);
	sprintf (message, "%c\nVERSION %4.2f SERVER (%i CRC)", 2, VERSION, pr_crc);
	MSG_WriteString (&client->message,message);

	MSG_WriteByte (&client->message, svc_serverinfo);
	MSG_WriteLong (&client->message, PROTOCOL_VERSION);
	MSG_WriteByte (&client->message, svs.maxclients);

	if (!coop.value && deathmatch.value)
		MSG_WriteByte (&client->message, GAME_DEATHMATCH);
	else
		MSG_WriteByte (&client->message, GAME_COOP);

	sprintf (message, pr_strings+svFPM.edicts->v.message);

	MSG_WriteString (&client->message,message);

	for (s = svFPM.model_precache+1 ; *s ; s++)
		MSG_WriteString (&client->message, *s);
	MSG_WriteByte (&client->message, 0);

	for (s = svFPM.sound_precache+1 ; *s ; s++)
		MSG_WriteString (&client->message, *s);
	MSG_WriteByte (&client->message, 0);

// send music
	MSG_WriteByte (&client->message, svc_cdtrack);
	MSG_WriteByte (&client->message, (int)svFPM.edicts->v.sounds);
	MSG_WriteByte (&client->message, (int)svFPM.edicts->v.sounds);

// set view
	MSG_WriteByte (&client->message, svc_setview);
	MSG_WriteShort (&client->message, NUM_FOR_EDICTFPM(client->edict));

	MSG_WriteByte (&client->message, svc_signonnum);
	MSG_WriteByte (&client->message, 1);

	client->sendsignon = true;
	client->spawned = false;		// need prespawn, spawn, etc
}
#endif //USEFPM

/*
================
SV_ConnectClient

Initializes a client_t for a new net connection.  This will only be called
once for a player each game, not once for each level change.
================
*/
void SV_ConnectClient (int clientnum)
{
	edict_t			*ent;
	client_t		*client;
	int				edictnum;
	struct qsocket_s *netconnection;
	int				i;
	float			spawn_parms[NUM_SPAWN_PARMS];

	client = svs.clients + clientnum;

	Con_DPrintf ("Client %s connected\n", client->netconnection->address);

	edictnum = clientnum+1;

	ent = EDICT_NUM(edictnum);

// set up the client_t
	netconnection = client->netconnection;

	if (sv.loadgame)
		Q_memcpy (spawn_parms, client->spawn_parms, sizeof(spawn_parms));
	Q_memset (client, 0, sizeof(*client));
	client->netconnection = netconnection;

	strcpy (client->name, "unconnected");
	client->active = true;
	client->spawned = false;
	client->edict = ent;
	client->message.data = client->msgbuf;
	client->message.maxsize = sizeof(client->msgbuf);
	client->message.allowoverflow = true;		// we can catch it

#ifdef IDGODS
	client->privileged = IsID(&client->netconnection->addr);
#else
	client->privileged = false;
#endif

	if (sv.loadgame)
		Q_memcpy (client->spawn_parms, spawn_parms, sizeof(spawn_parms));
	else
	{
	// call the progs to get default spawn parms for the new client
		PR_ExecuteProgram (pr_global_struct->SetNewParms);
		for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
			client->spawn_parms[i] = (&pr_global_struct->parm1)[i];
	}

	SV_SendServerinfo (client);
}

#ifdef USEFPM
void SV_ConnectClientFPM (int clientnum)
{
	edict_FPM_t		*ent;
	client_FPM_t	*client;
	int				edictnum;
	struct qsocket_s *netconnection;
	int				i;
	float			spawn_parms[NUM_SPAWN_PARMS];

	client = svsFPM.clients + clientnum;

	Con_DPrintf ("Client %s connected\n", client->netconnection->address);

	edictnum = clientnum+1;

	ent = EDICT_NUMFPM(edictnum);

// set up the client_t
	netconnection = client->netconnection;

	if (svFPM.loadgame)
		Q_memcpy (spawn_parms, client->spawn_parms, sizeof(spawn_parms));
	Q_memset (client, 0, sizeof(*client));
	client->netconnection = netconnection;

	strcpy (client->name, "unconnected");
	client->active = true;
	client->spawned = false;
	client->edict = ent;
	client->message.data = client->msgbuf;
	client->message.maxsize = sizeof(client->msgbuf);
	client->message.allowoverflow = true;		// we can catch it

#ifdef IDGODS
	client->privileged = IsID(&client->netconnection->addr);
#else
	client->privileged = false;
#endif

	if (svFPM.loadgame)
		Q_memcpy (client->spawn_parms, spawn_parms, sizeof(spawn_parms));
	else
	{
	// call the progs to get default spawn parms for the new client
		PR_ExecuteProgramFPM (pr_global_struct->SetNewParms);
		for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
			client->spawn_parms[i] = (&pr_global_struct->parm1)[i];
	}

	SV_SendServerinfoFPM (client);
}
#endif //USEFPM

/*
===================
SV_CheckForNewClients

===================
*/
void SV_CheckForNewClients (void)
{
	struct qsocket_s	*ret;
	int				i;

//
// check for new connections
//

	GpError("SV_CheckForNewClientrs",0);
	while (1)
	{
		ret = NET_CheckNewConnections ();
		if (!ret)
			break;

	//
	// init a new client structure
	//
		for (i=0 ; i<svs.maxclients ; i++)
			if (!svs.clients[i].active)
				break;
		if (i == svs.maxclients)
			Sys_Error ("Host_CheckForNewClients: no free clients");

		svs.clients[i].netconnection = ret;
		SV_ConnectClient (i);

		net_activeconnections++;
	}
}

#ifdef USEFPM
void SV_CheckForNewClientsFPM (void)
{
	struct qsocket_s	*ret;
	int				i;

//
// check for new connections
//
	while (1)
	{
		ret = NET_CheckNewConnections ();
		if (!ret)
			break;

	//
	// init a new client structure
	//
		for (i=0 ; i<svsFPM.maxclients ; i++)
			if (!svsFPM.clients[i].active)
				break;
		if (i == svsFPM.maxclients)
			Sys_Error ("Host_CheckForNewClients: no free clients");

		svsFPM.clients[i].netconnection = ret;
		SV_ConnectClientFPM (i);

		net_activeconnections++;
	}
}
#endif //USEFPM

/*
===============================================================================

FRAME UPDATES

===============================================================================
*/

/*
==================
SV_ClearDatagram

==================
*/
void SV_ClearDatagram (void)
{
	SZ_Clear (&sv.datagram);
}

/*
=============================================================================

The PVS must include a small area around the client to allow head bobbing
or other small motion on the client side.  Otherwise, a bob might cause an
entity that should be visible to not show up, especially when the bob
crosses a waterline.

=============================================================================
*/

int		fatbytes;
byte	fatpvs[MAX_MAP_LEAFS/8];

void SV_AddToFatPVS (vec3_t org, mnode_t *node)
{
	int		i;
	byte	*pvs;
	mplane_t	*plane;
	float	d;

	while (1)
	{
	// if this is a leaf, accumulate the pvs bits
		if (node->contents < 0)
		{
			if (node->contents != CONTENTS_SOLID)
			{
				pvs = Mod_LeafPVS ( (mleaf_t *)node, sv.worldmodel);
				for (i=0 ; i<fatbytes ; i++)
					fatpvs[i] |= pvs[i];
			}
			return;
		}

		plane = node->plane;
		d = DotProduct (org, plane->normal) - plane->dist;
		if (d > 8)
			node = node->children[0];
		else if (d < -8)
			node = node->children[1];
		else
		{	// go down both
			SV_AddToFatPVS (org, node->children[0]);
			node = node->children[1];
		}
	}
}

#ifdef USEFPM
void SV_AddToFatPVSFPM (vec3_FPM_t org, mnode_FPM_t *node)
{
	int				i;
	byte			*pvs;
	mplane_FPM_t	*plane;
	fixedpoint_t	d;

	while (1)
	{
	// if this is a leaf, accumulate the pvs bits
		if (node->contents < 0)
		{
			if (node->contents != CONTENTS_SOLID)
			{
				pvs = Mod_LeafPVSFPM ( (mleaf_FPM_t *)node, svFPM.worldmodel);
				for (i=0 ; i<fatbytes ; i++)
					fatpvs[i] |= pvs[i];
			}
			return;
		}

		plane = node->plane;
		d = FPM_SUB(DotProductFPM (org, plane->normal), plane->dist);
		if (d > FPM_FROMLONGC(8))
			node = node->children[0];
		else if (d < FPM_FROMLONGC(-8))
			node = node->children[1];
		else
		{	// go down both
			SV_AddToFatPVSFPM (org, node->children[0]);
			node = node->children[1];
		}
	}
}
#endif //USEFPM

/*
=============
SV_FatPVS

Calculates a PVS that is the inclusive or of all leafs within 8 pixels of the
given point.
=============
*/
byte *SV_FatPVS (vec3_t org)
{
	fatbytes = (sv.worldmodel->numleafs+31)>>3;
	Q_memset (fatpvs, 0, fatbytes);
	SV_AddToFatPVS (org, sv.worldmodel->nodes);
	return fatpvs;
}

#ifdef USEFPM
byte *SV_FatPVSFPM (vec3_FPM_t org)
{
	fatbytes = (svFPM.worldmodel->numleafs+31)>>3;
	Q_memset (fatpvs, 0, fatbytes);
	SV_AddToFatPVSFPM (org, svFPM.worldmodel->nodes);
	return fatpvs;
}
#endif //USEFPM
//=============================================================================


/*
=============
SV_WriteEntitiesToClient

=============
*/
void SV_WriteEntitiesToClient (edict_t	*clent, sizebuf_t *msg)
{
	int		e, i;
	int		bits;
	byte	*pvs;
	vec3_t	org;
	float	miss;
	edict_t	*ent;

// find the client's PVS
	VectorAdd (clent->v.origin, clent->v.view_ofs, org);
	pvs = SV_FatPVS (org);

// send over all entities (excpet the client) that touch the pvs
	ent = NEXT_EDICT(sv.edicts);
	for (e=1 ; e<sv.num_edicts ; e++, ent = NEXT_EDICT(ent))
	{
#ifdef QUAKE2
		// don't send if flagged for NODRAW and there are no lighting effects
		if (ent->v.effects == EF_NODRAW)
			continue;
#endif

// ignore if not touching a PV leaf
		if (ent != clent)	// clent is ALLWAYS sent
		{
// ignore ents without visible models
			if (!ent->v.modelindex || !pr_strings[ent->v.model])
				continue;

			for (i=0 ; i < ent->num_leafs ; i++)
				if (pvs[ent->leafnums[i] >> 3] & (1 << (ent->leafnums[i]&7) ))
					break;

			if (i == ent->num_leafs)
				continue;		// not visible
		}

		if (msg->maxsize - msg->cursize < 16)
		{
			Con_Printf ("packet overflow\n");
			return;
		}

// send an update
		bits = 0;

		for (i=0 ; i<3 ; i++)
		{
			miss = ent->v.origin[i] - ent->baseline.origin[i];
			if ( miss < -0.1 || miss > 0.1 )
				bits |= U_ORIGIN1<<i;
		}

		if ( ent->v.angles[0] != ent->baseline.angles[0] )
			bits |= U_ANGLE1;

		if ( ent->v.angles[1] != ent->baseline.angles[1] )
			bits |= U_ANGLE2;

		if ( ent->v.angles[2] != ent->baseline.angles[2] )
			bits |= U_ANGLE3;

		if (ent->v.movetype == MOVETYPE_STEP)
			bits |= U_NOLERP;	// don't mess up the step animation

		if (ent->baseline.colormap != ent->v.colormap)
			bits |= U_COLORMAP;

		if (ent->baseline.skin != ent->v.skin)
			bits |= U_SKIN;

		if (ent->baseline.frame != ent->v.frame)
			bits |= U_FRAME;

		if (ent->baseline.effects != ent->v.effects)
			bits |= U_EFFECTS;

		if (ent->baseline.modelindex != ent->v.modelindex)
			bits |= U_MODEL;

		if (e >= 256)
			bits |= U_LONGENTITY;

		if (bits >= 256)
			bits |= U_MOREBITS;

	//
	// write the message
	//
		MSG_WriteByte (msg,bits | U_SIGNAL);

		if (bits & U_MOREBITS)
			MSG_WriteByte (msg, bits>>8);
		if (bits & U_LONGENTITY)
			MSG_WriteShort (msg,e);
		else
			MSG_WriteByte (msg,e);

		if (bits & U_MODEL)
			MSG_WriteByte (msg,	(int)ent->v.modelindex);
		if (bits & U_FRAME)
			MSG_WriteByte (msg, (int)ent->v.frame);
		if (bits & U_COLORMAP)
			MSG_WriteByte (msg, (int)ent->v.colormap);
		if (bits & U_SKIN)
			MSG_WriteByte (msg, (int)ent->v.skin);
		if (bits & U_EFFECTS)
			MSG_WriteByte (msg, (int)ent->v.effects);
		if (bits & U_ORIGIN1)
			MSG_WriteCoord (msg, ent->v.origin[0]);
		if (bits & U_ANGLE1)
			MSG_WriteAngle(msg, ent->v.angles[0]);
		if (bits & U_ORIGIN2)
			MSG_WriteCoord (msg, ent->v.origin[1]);
		if (bits & U_ANGLE2)
			MSG_WriteAngle(msg, ent->v.angles[1]);
		if (bits & U_ORIGIN3)
			MSG_WriteCoord (msg, ent->v.origin[2]);
		if (bits & U_ANGLE3)
			MSG_WriteAngle(msg, ent->v.angles[2]);
	}
}

#ifdef USEFPM
void SV_WriteEntitiesToClientFPM (edict_FPM_t	*clent, sizebuf_t *msg)
{
	int				e, i;
	int				bits;
	byte			*pvs;
	vec3_FPM_t		org;
	vec3_t			tmp;
	fixedpoint_t	miss;
	edict_FPM_t		*ent;

// find the client's PVS
	VectorAdd (clent->v.origin, clent->v.view_ofs, tmp);

	org[0]=FPM_FROMFLOAT(tmp[0]);
	org[1]=FPM_FROMFLOAT(tmp[1]);
	org[2]=FPM_FROMFLOAT(tmp[2]);
	pvs = SV_FatPVSFPM (org);

// send over all entities (excpet the client) that touch the pvs
	ent = NEXT_EDICTFPM(svFPM.edicts);
	for (e=1 ; e<svFPM.num_edicts ; e++, ent = NEXT_EDICTFPM(ent))
	{
#ifdef QUAKE2
		// don't send if flagged for NODRAW and there are no lighting effects
		if (ent->v.effects == EF_NODRAW)
			continue;
#endif

// ignore if not touching a PV leaf
		if (ent != clent)	// clent is ALLWAYS sent
		{
// ignore ents without visible models
			if (!ent->v.modelindex || !pr_strings[ent->v.model])
				continue;

			for (i=0 ; i < ent->num_leafs ; i++)
				if (pvs[ent->leafnums[i] >> 3] & (1 << (ent->leafnums[i]&7) ))
					break;

			if (i == ent->num_leafs)
				continue;		// not visible
		}

		if (msg->maxsize - msg->cursize < 16)
		{
			Con_Printf ("packet overflow\n");
			return;
		}

// send an update
		bits = 0;

		for (i=0 ; i<3 ; i++)
		{
			miss = FPM_SUB(FPM_FROMFLOAT(ent->v.origin[i]), ent->baseline.origin[i]);
			if ( miss < FPM_FROMFLOATC(-0.1) || miss > FPM_FROMFLOATC(0.1) )
				bits |= U_ORIGIN1<<i;
		}

		if ( ent->v.angles[0] != ent->baseline.angles[0] )
			bits |= U_ANGLE1;

		if ( ent->v.angles[1] != ent->baseline.angles[1] )
			bits |= U_ANGLE2;

		if ( ent->v.angles[2] != ent->baseline.angles[2] )
			bits |= U_ANGLE3;

		if (ent->v.movetype == MOVETYPE_STEP)
			bits |= U_NOLERP;	// don't mess up the step animation

		if (ent->baseline.colormap != ent->v.colormap)
			bits |= U_COLORMAP;

		if (ent->baseline.skin != ent->v.skin)
			bits |= U_SKIN;

		if (ent->baseline.frame != ent->v.frame)
			bits |= U_FRAME;

		if (ent->baseline.effects != ent->v.effects)
			bits |= U_EFFECTS;

		if (ent->baseline.modelindex != ent->v.modelindex)
			bits |= U_MODEL;

		if (e >= 256)
			bits |= U_LONGENTITY;

		if (bits >= 256)
			bits |= U_MOREBITS;

	//
	// write the message
	//
		MSG_WriteByte (msg,bits | U_SIGNAL);

		if (bits & U_MOREBITS)
			MSG_WriteByte (msg, bits>>8);
		if (bits & U_LONGENTITY)
			MSG_WriteShort (msg,e);
		else
			MSG_WriteByte (msg,e);

		if (bits & U_MODEL)
			MSG_WriteByte (msg,	(int)ent->v.modelindex);
		if (bits & U_FRAME)
			MSG_WriteByte (msg, (int)ent->v.frame);
		if (bits & U_COLORMAP)
			MSG_WriteByte (msg, (int)ent->v.colormap);
		if (bits & U_SKIN)
			MSG_WriteByte (msg, (int)ent->v.skin);
		if (bits & U_EFFECTS)
			MSG_WriteByte (msg, (int)ent->v.effects);
		if (bits & U_ORIGIN1)
			MSG_WriteCoord (msg, ent->v.origin[0]);
		if (bits & U_ANGLE1)
			MSG_WriteAngle(msg, ent->v.angles[0]);
		if (bits & U_ORIGIN2)
			MSG_WriteCoord (msg, ent->v.origin[1]);
		if (bits & U_ANGLE2)
			MSG_WriteAngle(msg, ent->v.angles[1]);
		if (bits & U_ORIGIN3)
			MSG_WriteCoord (msg, ent->v.origin[2]);
		if (bits & U_ANGLE3)
			MSG_WriteAngle(msg, ent->v.angles[2]);
	}
}
#endif //USEFPM

/*
=============
SV_CleanupEnts

=============
*/
void SV_CleanupEnts (void)
{
	int		e;
	edict_t	*ent;

	ent = NEXT_EDICT(sv.edicts);
	for (e=1 ; e<sv.num_edicts ; e++, ent = NEXT_EDICT(ent))
	{
		ent->v.effects = (float)((int)ent->v.effects & ~EF_MUZZLEFLASH);
	}

}

#ifdef USEFPM
void SV_CleanupEntsFPM (void)
{
	int			e;
	edict_FPM_t	*ent;

	ent = NEXT_EDICTFPM(svFPM.edicts);
	for (e=1 ; e<svFPM.num_edicts ; e++, ent = NEXT_EDICTFPM(ent))
	{
		ent->v.effects = (float)((int)ent->v.effects & ~EF_MUZZLEFLASH);
	}

}
#endif //USEFPM
/*
==================
SV_WriteClientdataToMessage

==================
*/
void SV_WriteClientdataToMessage (edict_t *ent, sizebuf_t *msg)
{
	int		bits;
	int		i;
	edict_t	*other;
	int		items;
#ifndef QUAKE2
	eval_t	*val;
#endif

	GpError("SV_WriteClientdataToMessage",0);

//
// send a damage message
//
	if (ent->v.dmg_take || ent->v.dmg_save)
	{
		other = PROG_TO_EDICT(ent->v.dmg_inflictor);
		MSG_WriteByte (msg, svc_damage);
		MSG_WriteByte (msg, (int)ent->v.dmg_save);
		MSG_WriteByte (msg, (int)ent->v.dmg_take);
		for (i=0 ; i<3 ; i++)
			MSG_WriteCoord (msg, (float)(other->v.origin[i] + 0.5*(other->v.mins[i] + other->v.maxs[i])));

		ent->v.dmg_take = 0;
		ent->v.dmg_save = 0;
	}

//
// send the current viewpos offset from the view entity
//
	SV_SetIdealPitch ();		// how much to look up / down ideally

// a fixangle might get lost in a dropped packet.  Oh well.
	if ( ent->v.fixangle )
	{
		MSG_WriteByte (msg, svc_setangle);
		for (i=0 ; i < 3 ; i++)
			MSG_WriteAngle (msg, ent->v.angles[i] );
		ent->v.fixangle = 0;
	}

	bits = 0;

	if (ent->v.view_ofs[2] != DEFAULT_VIEWHEIGHT)
		bits |= SU_VIEWHEIGHT;

	if (ent->v.idealpitch)
		bits |= SU_IDEALPITCH;

// stuff the sigil bits into the high bits of items for sbar, or else
// mix in items2
#ifdef QUAKE2
	items = (int)ent->v.items | ((int)ent->v.items2 << 23);
#else
	val = GetEdictFieldValue(ent, "items2");

	if (val)
		items = (int)ent->v.items | ((int)val->_float << 23);
	else
		items = (int)ent->v.items | ((int)pr_global_struct->serverflags << 28);
#endif

	bits |= SU_ITEMS;

	if ( (int)ent->v.flags & FL_ONGROUND)
		bits |= SU_ONGROUND;

	if ( ent->v.waterlevel >= 2)
		bits |= SU_INWATER;

	for (i=0 ; i<3 ; i++)
	{
		if (ent->v.punchangle[i])
			bits |= (SU_PUNCH1<<i);
		if (ent->v.velocity[i])
			bits |= (SU_VELOCITY1<<i);
	}

	if (ent->v.weaponframe)
		bits |= SU_WEAPONFRAME;

	if (ent->v.armorvalue)
		bits |= SU_ARMOR;

//	if (ent->v.weapon)
		bits |= SU_WEAPON;

// send the data

	MSG_WriteByte (msg, svc_clientdata);
	MSG_WriteShort (msg, bits);

	if (bits & SU_VIEWHEIGHT)
		MSG_WriteChar (msg, (int)ent->v.view_ofs[2]);

	if (bits & SU_IDEALPITCH)
		MSG_WriteChar (msg, (int)ent->v.idealpitch);

	for (i=0 ; i<3 ; i++)
	{
		if (bits & (SU_PUNCH1<<i))
			MSG_WriteChar (msg, (int)ent->v.punchangle[i]);
		if (bits & (SU_VELOCITY1<<i))
			MSG_WriteChar (msg, (int)(ent->v.velocity[i]/16));
	}

// [always sent]	if (bits & SU_ITEMS)
	MSG_WriteLong (msg, items);

	if (bits & SU_WEAPONFRAME)
		MSG_WriteByte (msg, (int)ent->v.weaponframe);
	if (bits & SU_ARMOR)
		MSG_WriteByte (msg, (int)ent->v.armorvalue);
	if (bits & SU_WEAPON)
		MSG_WriteByte (msg, SV_ModelIndex(pr_strings+ent->v.weaponmodel));

	MSG_WriteShort (msg, (int)ent->v.health);
	MSG_WriteByte (msg, (int)ent->v.currentammo);
	MSG_WriteByte (msg, (int)ent->v.ammo_shells);
	MSG_WriteByte (msg, (int)ent->v.ammo_nails);
	MSG_WriteByte (msg, (int)ent->v.ammo_rockets);
	MSG_WriteByte (msg, (int)ent->v.ammo_cells);

	if (standard_quake)
	{
		MSG_WriteByte (msg, (int)ent->v.weapon);
	}
	else
	{
		for(i=0;i<32;i++)
		{
			if ( ((int)ent->v.weapon) & (1<<i) )
			{
				MSG_WriteByte (msg, i);
				break;
			}
		}
	}
}

#ifdef USEFPM
void SV_WriteClientdataToMessageFPM (edict_FPM_t *ent, sizebuf_t *msg)
{
	int		bits;
	int		i;
	edict_t	*other;
	int		items;
#ifndef QUAKE2
	eval_t	*val;
#endif

//
// send a damage message
//
	if (ent->v.dmg_take || ent->v.dmg_save)
	{
		other = PROG_TO_EDICT(ent->v.dmg_inflictor);
		MSG_WriteByte (msg, svc_damage);
		MSG_WriteByte (msg, (int)ent->v.dmg_save);
		MSG_WriteByte (msg, (int)ent->v.dmg_take);
		for (i=0 ; i<3 ; i++)
			MSG_WriteCoord (msg, (float)(other->v.origin[i] + 0.5*(other->v.mins[i] + other->v.maxs[i])));

		ent->v.dmg_take = 0;
		ent->v.dmg_save = 0;
	}

//
// send the current viewpos offset from the view entity
//
	SV_SetIdealPitchFPM ();		// how much to look up / down ideally

// a fixangle might get lost in a dropped packet.  Oh well.
	if ( ent->v.fixangle )
	{
		MSG_WriteByte (msg, svc_setangle);
		for (i=0 ; i < 3 ; i++)
			MSG_WriteAngle (msg, ent->v.angles[i] );
		ent->v.fixangle = 0;
	}

	bits = 0;

	if (ent->v.view_ofs[2] != DEFAULT_VIEWHEIGHT)
		bits |= SU_VIEWHEIGHT;

	if (ent->v.idealpitch)
		bits |= SU_IDEALPITCH;

// stuff the sigil bits into the high bits of items for sbar, or else
// mix in items2
#ifdef QUAKE2
	items = (int)ent->v.items | ((int)ent->v.items2 << 23);
#else
	val = GetEdictFieldValueFPM(ent, "items2");

	if (val)
		items = (int)ent->v.items | ((int)val->_float << 23);
	else
		items = (int)ent->v.items | ((int)pr_global_struct->serverflags << 28);
#endif

	bits |= SU_ITEMS;

	if ( (int)ent->v.flags & FL_ONGROUND)
		bits |= SU_ONGROUND;

	if ( ent->v.waterlevel >= 2)
		bits |= SU_INWATER;

	for (i=0 ; i<3 ; i++)
	{
		if (ent->v.punchangle[i])
			bits |= (SU_PUNCH1<<i);
		if (ent->v.velocity[i])
			bits |= (SU_VELOCITY1<<i);
	}

	if (ent->v.weaponframe)
		bits |= SU_WEAPONFRAME;

	if (ent->v.armorvalue)
		bits |= SU_ARMOR;

//	if (ent->v.weapon)
		bits |= SU_WEAPON;

// send the data

	MSG_WriteByte (msg, svc_clientdata);
	MSG_WriteShort (msg, bits);

	if (bits & SU_VIEWHEIGHT)
		MSG_WriteChar (msg, (int)ent->v.view_ofs[2]);

	if (bits & SU_IDEALPITCH)
		MSG_WriteChar (msg, (int)ent->v.idealpitch);

	for (i=0 ; i<3 ; i++)
	{
		if (bits & (SU_PUNCH1<<i))
			MSG_WriteChar (msg, (int)ent->v.punchangle[i]);
		if (bits & (SU_VELOCITY1<<i))
			MSG_WriteChar (msg, (int)(ent->v.velocity[i]/16));
	}

// [always sent]	if (bits & SU_ITEMS)
	MSG_WriteLong (msg, items);

	if (bits & SU_WEAPONFRAME)
		MSG_WriteByte (msg, (int)ent->v.weaponframe);
	if (bits & SU_ARMOR)
		MSG_WriteByte (msg, (int)ent->v.armorvalue);
	if (bits & SU_WEAPON)
		MSG_WriteByte (msg, SV_ModelIndexFPM(pr_strings+ent->v.weaponmodel));

	MSG_WriteShort (msg, (int)ent->v.health);
	MSG_WriteByte (msg, (int)ent->v.currentammo);
	MSG_WriteByte (msg, (int)ent->v.ammo_shells);
	MSG_WriteByte (msg, (int)ent->v.ammo_nails);
	MSG_WriteByte (msg, (int)ent->v.ammo_rockets);
	MSG_WriteByte (msg, (int)ent->v.ammo_cells);

	if (standard_quake)
	{
		MSG_WriteByte (msg, (int)ent->v.weapon);
	}
	else
	{
		for(i=0;i<32;i++)
		{
			if ( ((int)ent->v.weapon) & (1<<i) )
			{
				MSG_WriteByte (msg, i);
				break;
			}
		}
	}
}
#endif //USEFPM

/*
=======================
SV_SendClientDatagram
=======================
*/
qboolean SV_SendClientDatagram (client_t *client)
{
	byte		buf[MAX_DATAGRAM];
	sizebuf_t	msg;

	GpError("SV_SendClientDatagram",2);

	msg.data = buf;
	msg.maxsize = sizeof(buf);
	msg.cursize = 0;

	MSG_WriteByte (&msg, svc_time);
	MSG_WriteFloat (&msg, (float)sv.time);

// add the client specific data to the datagram
	SV_WriteClientdataToMessage (client->edict, &msg);

	SV_WriteEntitiesToClient (client->edict, &msg);

// copy the server datagram if there is space
	if (msg.cursize + sv.datagram.cursize < msg.maxsize)
		SZ_Write (&msg, sv.datagram.data, sv.datagram.cursize);

// send the datagram
	if (NET_SendUnreliableMessage (client->netconnection, &msg) == -1)
	{
		GpError("SV_SendClientDatagram, -1",2);
		SV_DropClient (true);// if the message couldn't send, kick off
		return false;
	}

	return true;
}

#ifdef USEFPM
qboolean SV_SendClientDatagramFPM (client_FPM_t *client)
{
	byte		buf[MAX_DATAGRAM];
	sizebuf_t	msg;

	msg.data = buf;
	msg.maxsize = sizeof(buf);
	msg.cursize = 0;

	MSG_WriteByte (&msg, svc_time);
	MSG_WriteFloat (&msg, (float)svFPM.time);

// add the client specific data to the datagram
	SV_WriteClientdataToMessageFPM (client->edict, &msg);

	SV_WriteEntitiesToClientFPM (client->edict, &msg);

// copy the server datagram if there is space
	if (msg.cursize + svFPM.datagram.cursize < msg.maxsize)
		SZ_Write (&msg, svFPM.datagram.data, svFPM.datagram.cursize);

// send the datagram
	if (NET_SendUnreliableMessage (client->netconnection, &msg) == -1)
	{
		SV_DropClient (true);// if the message couldn't send, kick off
		return false;
	}

	return true;
}
#endif //USEFPM

/*
=======================
SV_UpdateToReliableMessages
=======================
*/
void SV_UpdateToReliableMessages (void)
{
	int			i, j;
	client_t *client;

// check for changes to be sent over the reliable streams
	for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
	{
		if (host_client->old_frags != host_client->edict->v.frags)
		{
			for (j=0, client = svs.clients ; j<svs.maxclients ; j++, client++)
			{
				if (!client->active)
					continue;
				MSG_WriteByte (&client->message, svc_updatefrags);
				MSG_WriteByte (&client->message, i);
				MSG_WriteShort (&client->message, (int)host_client->edict->v.frags);
			}

			host_client->old_frags = (int)host_client->edict->v.frags;
		}
	}

	for (j=0, client = svs.clients ; j<svs.maxclients ; j++, client++)
	{
		if (!client->active)
			continue;
		SZ_Write (&client->message, sv.reliable_datagram.data, sv.reliable_datagram.cursize);
	}

	SZ_Clear (&sv.reliable_datagram);
}

#ifdef USEFPM
void SV_UpdateToReliableMessagesFPM (void)
{
	int				i, j;
	client_FPM_t	*client;

// check for changes to be sent over the reliable streams
	for (i=0, host_clientFPM = svsFPM.clients ; i<svsFPM.maxclients ; i++, host_clientFPM++)
	{
		if (host_clientFPM->old_frags != host_clientFPM->edict->v.frags)
		{
			for (j=0, client = svsFPM.clients ; j<svsFPM.maxclients ; j++, client++)
			{
				if (!client->active)
					continue;
				MSG_WriteByte (&client->message, svc_updatefrags);
				MSG_WriteByte (&client->message, i);
				MSG_WriteShort (&client->message, (int)host_clientFPM->edict->v.frags);
			}

			host_clientFPM->old_frags = (int)host_clientFPM->edict->v.frags;
		}
	}

	for (j=0, client = svsFPM.clients ; j<svsFPM.maxclients ; j++, client++)
	{
		if (!client->active)
			continue;
		SZ_Write (&client->message, svFPM.reliable_datagram.data, svFPM.reliable_datagram.cursize);
	}

	SZ_Clear (&svFPM.reliable_datagram);
}
#endif //USEFPM

/*
=======================
SV_SendNop

Send a nop message without trashing or sending the accumulated client
message buffer
=======================
*/
void SV_SendNop (client_t *client)
{
	sizebuf_t	msg;
	byte		buf[4];

	msg.data = buf;
	msg.maxsize = sizeof(buf);
	msg.cursize = 0;

	MSG_WriteChar (&msg, svc_nop);

	if (NET_SendUnreliableMessage (client->netconnection, &msg) == -1){
		GpError("SV_SendNOP fail",2);
		SV_DropClient (true);	// if the message couldn't send, kick off
	}
	client->last_message = realtime;
}

#ifdef USEFPM
void SV_SendNopFPM (client_FPM_t *client)
{
	sizebuf_t	msg;
	byte		buf[4];

	msg.data = buf;
	msg.maxsize = sizeof(buf);
	msg.cursize = 0;

	MSG_WriteChar (&msg, svc_nop);

	if (NET_SendUnreliableMessage (client->netconnection, &msg) == -1)
		SV_DropClient (true);	// if the message couldn't send, kick off
	client->last_message = realtime;
}
#endif //USEFPM

/*
=======================
SV_SendClientMessages
=======================
*/
void SV_SendClientMessages (void)
{
	int			i;

// update frags, names, etc

	GpError("SV_SendClientMessages",2);

	SV_UpdateToReliableMessages ();

// build individual updates
	for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
	{
		if (!host_client->active){
			continue;
		}

		if (host_client->spawned)
		{
			if (!SV_SendClientDatagram (host_client)){
				continue;
			}
		} else {
		// the player isn't totally in the game yet
		// send small keepalive messages if too much time has passed
		// send a full message when the next signon stage has been requested
		// some other message data (name changes, etc) may accumulate
		// between signon stages
			if (!host_client->sendsignon)
			{
				if (realtime - host_client->last_message > 5)
					SV_SendNop (host_client);
				continue;	// don't send out non-signon messages
			}

		}

		// check for an overflowed message.  Should only happen
		// on a very fucked up connection that backs up a lot, then
		// changes level
		if (host_client->message.overflowed)
		{
			GpError("SV_SCM - overflowed",2);
			SV_DropClient (true);
			host_client->message.overflowed = false;
			continue;
		}

		if (host_client->message.cursize || host_client->dropasap)
		{

			if (!NET_CanSendMessage (host_client->netconnection))
			{
				GpError("SV_SCM - can't send",2);
//				I_Printf ("can't write\n");
				continue;
			}

			if (host_client->dropasap){
				GpError("SV_SCM - drop asap",2);
				SV_DropClient (false);	// went to another level
			}
			else
			{
				if (NET_SendMessage (host_client->netconnection	, &host_client->message) == -1){
					GpError("SV_SCM - drop",2);
					SV_DropClient (true);	// if the message couldn't send, kick off
				}
				SZ_Clear (&host_client->message);
				host_client->last_message = realtime;
				host_client->sendsignon = false;
			}
		}
	}

	GpError("SV_SendClientMessages - done",0);

// clear muzzle flashes
	SV_CleanupEnts ();
}

#ifdef USEFPM
void SV_SendClientMessagesFPM (void)
{
	int			i;

// update frags, names, etc
	SV_UpdateToReliableMessagesFPM ();

// build individual updates
	for (i=0, host_clientFPM = svsFPM.clients ; i<svsFPM.maxclients ; i++, host_clientFPM++)
	{
		if (!host_clientFPM->active)
			continue;

		if (host_clientFPM->spawned)
		{
			if (!SV_SendClientDatagramFPM (host_clientFPM))
				continue;
		}
		else
		{
		// the player isn't totally in the game yet
		// send small keepalive messages if too much time has passed
		// send a full message when the next signon stage has been requested
		// some other message data (name changes, etc) may accumulate
		// between signon stages
			if (!host_clientFPM->sendsignon)
			{
				if (realtime - host_clientFPM->last_message > 5)
					SV_SendNopFPM (host_clientFPM);
				continue;	// don't send out non-signon messages
			}
		}

		// check for an overflowed message.
		if (host_clientFPM->message.overflowed)
		{
			SV_DropClient (true);
			host_clientFPM->message.overflowed = false;
			continue;
		}

		if (host_clientFPM->message.cursize || host_clientFPM->dropasap)
		{
			if (!NET_CanSendMessage (host_clientFPM->netconnection))
			{
//				I_Printf ("can't write\n");
				continue;
			}

			if (host_clientFPM->dropasap)
				SV_DropClient (false);	// went to another level
			else
			{
				if (NET_SendMessage (host_clientFPM->netconnection
				, &host_clientFPM->message) == -1)
					SV_DropClientFPM (true);	// if the message couldn't send, kick off
				SZ_Clear (&host_clientFPM->message);
				host_clientFPM->last_message = realtime;
				host_clientFPM->sendsignon = false;
			}
		}
	}


// clear muzzle flashes
	SV_CleanupEntsFPM ();
}
#endif //USEFPM

/*
==============================================================================

SERVER SPAWNING

==============================================================================
*/

/*
================
SV_ModelIndex

================
*/
int SV_ModelIndex (char *name)
{
	int		i;

	if (!name || !name[0])
		return 0;

	for (i=0 ; i<MAX_MODELS && sv.model_precache[i] ; i++)
		if (!strcmp(sv.model_precache[i], name))
			return i;
	if (i==MAX_MODELS || !sv.model_precache[i])
		Sys_Error ("SV_ModelIndex: model %s not precached", name);
	return i;
}

#ifdef USEFPM
int SV_ModelIndexFPM (char *name)
{
	int		i;

	if (!name || !name[0])
		return 0;

	for (i=0 ; i<MAX_MODELS && svFPM.model_precache[i] ; i++)
		if (!strcmp(svFPM.model_precache[i], name))
			return i;
	if (i==MAX_MODELS || !svFPM.model_precache[i])
		Sys_Error ("SV_ModelIndex: model %s not precached", name);
	return i;
}
#endif //USEFPM

/*
================
SV_CreateBaseline

================
*/
void SV_CreateBaseline (void)
{
	int			i;
	edict_t			*svent;
	int				entnum;

	GpError("SV_CreateBaseLine",0);

	for (entnum = 0; entnum < sv.num_edicts ; entnum++)
	{
	// get the current server version
		svent = EDICT_NUM(entnum);
		if (svent->free)
			continue;
		if (entnum > svs.maxclients && !svent->v.modelindex)
			continue;

	//
	// create entity baseline
	//
		VectorCopy (svent->v.origin, svent->baseline.origin);
		VectorCopy (svent->v.angles, svent->baseline.angles);
		svent->baseline.frame = (int)svent->v.frame;
		svent->baseline.skin = (int)svent->v.skin;
		if (entnum > 0 && entnum <= svs.maxclients)
		{
			svent->baseline.colormap = entnum;
			svent->baseline.modelindex = SV_ModelIndex("progs/player.mdl");
		}
		else
		{
			svent->baseline.colormap = 0;
			svent->baseline.modelindex =
				SV_ModelIndex(pr_strings + svent->v.model);
		}

	//
	// add to the message
	//
		MSG_WriteByte (&sv.signon,svc_spawnbaseline);
		MSG_WriteShort (&sv.signon,entnum);

		MSG_WriteByte (&sv.signon, svent->baseline.modelindex);
		MSG_WriteByte (&sv.signon, svent->baseline.frame);
		MSG_WriteByte (&sv.signon, svent->baseline.colormap);
		MSG_WriteByte (&sv.signon, svent->baseline.skin);
		for (i=0 ; i<3 ; i++)
		{
			MSG_WriteCoord(&sv.signon, svent->baseline.origin[i]);
			MSG_WriteAngle(&sv.signon, svent->baseline.angles[i]);
		}
	}
	GpError("SV_CreateBaseLine Done",0);
}

#ifdef USEFPM
void SV_CreateBaselineFPM (void)
{
	int				i;
	edict_FPM_t		*svent;
	int				entnum;

	for (entnum = 0; entnum < svFPM.num_edicts ; entnum++)
	{
	// get the current server version
		svent = EDICT_NUMFPM(entnum);
		if (svent->free)
			continue;
		if (entnum > svsFPM.maxclients && !svent->v.modelindex)
			continue;

	//
	// create entity baseline
	//
		svent->baseline.origin[0]=FPM_FROMFLOAT(svent->v.origin[0]);
		svent->baseline.origin[1]=FPM_FROMFLOAT(svent->v.origin[1]);
		svent->baseline.origin[2]=FPM_FROMFLOAT(svent->v.origin[2]);
		svent->baseline.angles[0]=FPM_FROMFLOAT(svent->v.angles[0]);
		svent->baseline.angles[1]=FPM_FROMFLOAT(svent->v.angles[1]);
		svent->baseline.angles[2]=FPM_FROMFLOAT(svent->v.angles[2]);
		//VectorCopy (svent->v.origin, svent->baseline.origin);
		//VectorCopy (svent->v.angles, svent->baseline.angles);
		svent->baseline.frame = (int)svent->v.frame;
		svent->baseline.skin = (int)svent->v.skin;
		if (entnum > 0 && entnum <= svsFPM.maxclients)
		{
			svent->baseline.colormap = entnum;
			svent->baseline.modelindex = SV_ModelIndexFPM("progs/player.mdl");
		}
		else
		{
			svent->baseline.colormap = 0;
			svent->baseline.modelindex =
				SV_ModelIndexFPM(pr_strings + svent->v.model);
		}

	//
	// add to the message
	//
		MSG_WriteByte (&svFPM.signon,svc_spawnbaseline);
		MSG_WriteShort (&svFPM.signon,entnum);

		MSG_WriteByte (&svFPM.signon, svent->baseline.modelindex);
		MSG_WriteByte (&svFPM.signon, svent->baseline.frame);
		MSG_WriteByte (&svFPM.signon, svent->baseline.colormap);
		MSG_WriteByte (&svFPM.signon, svent->baseline.skin);
		for (i=0 ; i<3 ; i++)
		{
			MSG_WriteCoord(&svFPM.signon, svent->v.origin[i]);
			MSG_WriteAngle(&svFPM.signon, svent->v.angles[i]);
			//MSG_WriteCoordFPM(&sv.signon, svent->baseline.origin[i]);
			//MSG_WriteAngleFPM(&sv.signon, svent->baseline.angles[i]);
		}
	}
}
#endif //USEFPM

/*
================
SV_SendReconnect

Tell all the clients that the server is changing levels
================
*/
void SV_SendReconnect (void)
{
	char	data[128];
	sizebuf_t	msg;

	GpError("Sv_SendReconnect",0);

	msg.data = data;
	msg.cursize = 0;
	msg.maxsize = sizeof(data);

	MSG_WriteChar (&msg, svc_stufftext);
	MSG_WriteString (&msg, "reconnect\n");
	NET_SendToAll (&msg, 5);

	if (cls.state != ca_dedicated)
#ifdef QUAKE2
		Cbuf_InsertText ("reconnect\n");
#else
		Cmd_ExecuteString ("reconnect\n", src_command);
#endif
}

/*
================
SV_SaveSpawnparms

Grabs the current state of each client for saving across the
transition to another level
================
*/
void SV_SaveSpawnparms (void)
{
	int		i, j;

	svs.serverflags = (int)pr_global_struct->serverflags;

	for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
	{
		if (!host_client->active)
			continue;

	// call the progs to get default spawn parms for the new client
		pr_global_struct->self = EDICT_TO_PROG(host_client->edict);
		PR_ExecuteProgram (pr_global_struct->SetChangeParms);
		for (j=0 ; j<NUM_SPAWN_PARMS ; j++)
			host_client->spawn_parms[j] = (&pr_global_struct->parm1)[j];
	}
}

#ifdef USEFPM
void SV_SaveSpawnparmsFPM (void)
{
	int		i, j;

	svsFPM.serverflags = (int)pr_global_struct->serverflags;

	for (i=0, host_clientFPM = svsFPM.clients ; i<svsFPM.maxclients ; i++, host_clientFPM++)
	{
		if (!host_clientFPM->active)
			continue;

	// call the progs to get default spawn parms for the new client
		pr_global_struct->self = EDICT_TO_PROGFPM(host_clientFPM->edict);
		PR_ExecuteProgramFPM (pr_global_struct->SetChangeParms);
		for (j=0 ; j<NUM_SPAWN_PARMS ; j++)
			host_clientFPM->spawn_parms[j] = (&pr_global_struct->parm1)[j];
	}
}
#endif //USEFPM

/*
================
SV_SpawnServer

This is called at the start of each level
================
*/
extern float		scr_centertime_off;

#ifdef QUAKE2
void SV_SpawnServer (char *server, char *startspot)
#else
void SV_SpawnServer (char *server)
#endif
{
	edict_t		*ent;
	int			i;

	GpError("spawnserver", 0);

	// let's not have any servers with no name
	if (hostname.string[0] == 0)
		Cvar_Set ("hostname", "UNNAMED");
	scr_centertime_off = 0;

	Con_DPrintf ("SpawnServer: %s\n",server);
	svs.changelevel_issued = false;		// now safe to issue another

//
// tell all connected clients that we are going to a new level
//
	if (sv.active)
	{
		SV_SendReconnect ();
	}

//
// make cvars consistant
//
	if (coop.value)
		Cvar_SetValue ("deathmatch", 0);
	current_skill = (int)(skill.value + 0.5);
	if (current_skill < 0)
		current_skill = 0;
	if (current_skill > 3)
		current_skill = 3;

	Cvar_SetValue ("skill", (float)current_skill);

//
// set up the new server
//
	Host_ClearMemory ();

	Q_memset (&sv, 0, sizeof(sv));

	strcpy (sv.name, server);
#ifdef QUAKE2
	if (startspot)
		strcpy(sv.startspot, startspot);
#endif

// load progs to get entity field count
	GpError("spawnserver - load progs", 0);
	PR_LoadProgs ();
	puts("progs loaded");	
	
// allocate server memory
	sv.max_edicts = MAX_EDICTS;

	sv.edicts = Hunk_AllocName (sv.max_edicts*pr_edict_size, "edicts");

	sv.datagram.maxsize = sizeof(sv.datagram_buf);
	sv.datagram.cursize = 0;
	sv.datagram.data = sv.datagram_buf;

	sv.reliable_datagram.maxsize = sizeof(sv.reliable_datagram_buf);
	sv.reliable_datagram.cursize = 0;
	sv.reliable_datagram.data = sv.reliable_datagram_buf;

	sv.signon.maxsize = sizeof(sv.signon_buf);
	sv.signon.cursize = 0;
	sv.signon.data = sv.signon_buf;

	puts("edict_num loop");

// leave slots at start for clients only
	sv.num_edicts = svs.maxclients+1;
	for (i=0 ; i<svs.maxclients ; i++) {
		ent = EDICT_NUM(i+1);
		svs.clients[i].edict = ent;
	}

	sv.state = ss_loading;
	sv.paused = false;

	sv.time = 1.0;

	strcpy (sv.name, server);
	sprintf (sv.modelname,"maps/%s.bsp", server);
	sv.worldmodel = Mod_ForName (sv.modelname, false);
	if (sv.worldmodel->numvertexes == -1) {
		Con_Printf ("Couldn't spawn server %s\n", sv.modelname);
		sv.active = false;
		return;
	}
	sv.models[1] = sv.worldmodel;
	puts("world model ok");
//
// clear world interaction links
//
	SV_ClearWorld ();

	puts("world cleared");

	sv.sound_precache[0] = pr_strings;

	sv.model_precache[0] = pr_strings;
	sv.model_precache[1] = sv.modelname;
	for (i=1 ; i<sv.worldmodel->numsubmodels ; i++) {
		sv.model_precache[1+i] = localmodels[i];
		sv.models[i+1] = Mod_ForName (localmodels[i], false);
	}

//
// load the rest of the entities
//
	GpError("spawnserver - load entities", 0);

	ent = EDICT_NUM(0);
	Q_memset (&ent->v, 0, progs->entityfields * 4);
	ent->free = false;
	ent->v.model = sv.worldmodel->name - pr_strings;
	ent->v.modelindex = 1;		// world model
	ent->v.solid = SOLID_BSP;
	ent->v.movetype = MOVETYPE_PUSH;

	if (coop.value)
		pr_global_struct->coop = coop.value;
	else
		pr_global_struct->deathmatch = deathmatch.value;

	pr_global_struct->mapname = sv.name - pr_strings;
#ifdef QUAKE2
	pr_global_struct->startspot = sv.startspot - pr_strings;
#endif

// serverflags are for cross level information (sigils)
	pr_global_struct->serverflags = (float)svs.serverflags;

	ED_LoadFromFile (sv.worldmodel->entities);

	sv.active = true;

// all setup is completed, any further precache statements are errors
	sv.state = ss_active;

	GpError("spawnserver - precache complete", 0);

// run two frames to allow everything to settle
	host_frametime = 0.1;
	SV_Physics ();
	SV_Physics ();

// create a baseline for more efficient communications
	GpError("spawnserver - create baseline", 0);
	SV_CreateBaseline ();

// send serverinfo to all connected clients
	GpError("spawnserver - send info", 0);

	for (i=0,host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
		if (host_client->active)
			SV_SendServerinfo (host_client);

	Con_DPrintf ("Server spawned.\n");
	GpError("spawnserver - done!", 0);
}

#ifdef USEFPM
#ifdef QUAKE2
void SV_SpawnServerFPM (char *server, char *startspot)
#else
void SV_SpawnServerFPM (char *server)
#endif
{
	edict_FPM_t		*ent;
	int				i;

	GpError("spawnserverFPM",0);
	// let's not have any servers with no name
	if (hostname.string[0] == 0)
		Cvar_Set ("hostname", "UNNAMED");
	scr_centertime_off = 0;

	Con_DPrintf ("SpawnServer: %s\n",server);
	svsFPM.changelevel_issued = false;		// now safe to issue another

//
// tell all connected clients that we are going to a new level
//
	if (svFPM.active)
	{
		SV_SendReconnect ();
	}

//
// make cvars consistant
//
	if (coop.value)
		Cvar_SetValue ("deathmatch", 0);
	current_skill = (int)(skill.value + 0.5);
	if (current_skill < 0)
		current_skill = 0;
	if (current_skill > 3)
		current_skill = 3;

	Cvar_SetValue ("skill", (float)current_skill);

//
// set up the new server
//
	Host_ClearMemoryFPM ();

	Q_memset (&svFPM, 0, sizeof(svFPM));

	strcpy (svFPM.name, server);
#ifdef QUAKE2
	if (startspot)
		strcpy(svFPM.startspot, startspot);
#endif

// load progs to get entity field count
	GpError("spawnserver - load progs", 0);
	PR_LoadProgs ();
	GpError("spawnserver - load progs done", 0);

// allocate server memory
	svFPM.max_edicts = MAX_EDICTS;

	svFPM.edicts = Hunk_AllocName (svFPM.max_edicts*pr_edict_size, "edicts");
	GpError("hunk allocated");
	svFPM.datagram.maxsize = sizeof(svFPM.datagram_buf);
	svFPM.datagram.cursize = 0;
	svFPM.datagram.data = svFPM.datagram_buf;

	svFPM.reliable_datagram.maxsize = sizeof(svFPM.reliable_datagram_buf);
	svFPM.reliable_datagram.cursize = 0;
	svFPM.reliable_datagram.data = svFPM.reliable_datagram_buf;

	svFPM.signon.maxsize = sizeof(svFPM.signon_buf);
	svFPM.signon.cursize = 0;
	svFPM.signon.data = svFPM.signon_buf;

// leave slots at start for clients only
	svFPM.num_edicts = svsFPM.maxclients+1;
	for (i=0 ; i<svsFPM.maxclients ; i++)
	{
		ent = EDICT_NUMFPM(i+1);
		svsFPM.clients[i].edict = ent;
	}
	GpError("loop done",0);
	svFPM.state = ss_loading;
	svFPM.paused = false;

	svFPM.time = 1.0;

	strcpy (svFPM.name, server);
	GpError("strcpy done",0);
	sprintf (svFPM.modelname,"maps/%s.bsp", server);
	svFPM.worldmodel = Mod_ForNameFPM (svFPM.modelname, false);
	GpError("1",0);
	if (!svFPM.worldmodel)
	{
		Con_Printf ("Couldn't spawn server %s\n", svFPM.modelname);
		svFPM.active = false;
		return;
	}
	svFPM.models[1] = svFPM.worldmodel;

//
// clear world interaction links
//
	GpError("clearworldfpm",0);
	SV_ClearWorldFPM ();

	svFPM.sound_precache[0] = pr_strings;

	svFPM.model_precache[0] = pr_strings;
	svFPM.model_precache[1] = svFPM.modelname;
	for (i=1 ; i<svFPM.worldmodel->numsubmodels ; i++)
	{
		svFPM.model_precache[1+i] = localmodels[i];
		svFPM.models[i+1] = Mod_ForNameFPM (localmodels[i], false);
	}

//
// load the rest of the entities
//
	GpError("spawnserver - load entities", 0);
	ent = EDICT_NUMFPM(0);
	Q_memset (&ent->v, 0, progs->entityfields * 4);
	ent->free = false;
	ent->v.model = svFPM.worldmodel->name - pr_strings;
	ent->v.modelindex = 1;		// world model
	ent->v.solid = SOLID_BSP;
	ent->v.movetype = MOVETYPE_PUSH;

	if (coop.value)
		pr_global_struct->coop = coop.value;
	else
		pr_global_struct->deathmatch = deathmatch.value;

	pr_global_struct->mapname = svFPM.name - pr_strings;
#ifdef QUAKE2
	pr_global_struct->startspot = svFPM.startspot - pr_strings;
#endif

// serverflags are for cross level information (sigils)
	pr_global_struct->serverflags = (float)svsFPM.serverflags;

	ED_LoadFromFileFPM (svFPM.worldmodel->entities);

	svFPM.active = true;

// all setup is completed, any further precache statements are errors
	svFPM.state = ss_active;

	GpError("spawnserver - precache complete", 0);

// run two frames to allow everything to settle
	host_frametime = 0.1;
	SV_PhysicsFPM ();
	SV_PhysicsFPM ();

// create a baseline for more efficient communications
	GpError("spawnserver - create baseline", 0);
	SV_CreateBaselineFPM ();

// send serverinfo to all connected clients
	for (i=0,host_clientFPM = svsFPM.clients ; i<svsFPM.maxclients ; i++, host_clientFPM++)
		if (host_clientFPM->active)
			SV_SendServerinfoFPM (host_clientFPM);

	Con_DPrintf ("Server spawned.\n");
	GpError("spawnserverFPM - done!", 0);
}
#endif //USEFPM
