#ifndef FILE_G_PROTOCOL_H
#define FILE_G_PROTOCOL_H

enum
{
	INPUT_STATE_MASK=0x3f,
};

enum
{
	PLAYERSTATE_UNKNOWN=0,
	PLAYERSTATE_PLAYING,
	PLAYERSTATE_IN_MENU,
	PLAYERSTATE_CHATTING,
	NUM_PLAYERSTATES
};

enum
{
	EMOTE_NORMAL=0,
	EMOTE_PAIN,
	EMOTE_HAPPY,
	EMOTE_SURPRISE,
	EMOTE_ANGRY,
	EMOTE_BLINK,
	NUM_EMOTES
};

enum
{
	POWERUP_HEALTH=0,
	POWERUP_ARMOR,
	POWERUP_WEAPON,
	POWERUP_NINJA,
	NUM_POWERUPS
};

enum
{
	EMOTICON_1=0,
	EMOTICON_2,
	EMOTICON_3,
	EMOTICON_4,
	EMOTICON_5,
	EMOTICON_6,
	EMOTICON_7,
	EMOTICON_8,
	EMOTICON_9,
	EMOTICON_10,
	EMOTICON_11,
	EMOTICON_12,
	EMOTICON_13,
	EMOTICON_14,
	EMOTICON_15,
	NUM_EMOTICONS
};

enum
{
	GAMEFLAG_TEAMS = 1<<0,
	GAMEFLAG_FLAGS = 1<<1,
};

enum
{
	NETOBJ_INVALID=0,
	NETOBJTYPE_PLAYER_INPUT,
	NETOBJTYPE_PROJECTILE,
	NETOBJTYPE_LASER,
	NETOBJTYPE_PICKUP,
	NETOBJTYPE_FLAG,
	NETOBJTYPE_GAME,
	NETOBJTYPE_CHARACTER_CORE,
	NETOBJTYPE_CHARACTER,
	NETOBJTYPE_PLAYER_INFO,
	NETOBJTYPE_CLIENT_INFO,
	NETEVENTTYPE_COMMON,
	NETEVENTTYPE_EXPLOSION,
	NETEVENTTYPE_SPAWN,
	NETEVENTTYPE_HAMMERHIT,
	NETEVENTTYPE_DEATH,
	NETEVENTTYPE_SOUNDGLOBAL,
	NETEVENTTYPE_SOUNDWORLD,
	NETEVENTTYPE_DAMAGEIND,
	NUM_NETOBJTYPES
};

enum
{
	NETMSG_INVALID=0,
	NETMSGTYPE_SV_MOTD,
	NETMSGTYPE_SV_BROADCAST,
	NETMSGTYPE_SV_CHAT,
	NETMSGTYPE_SV_KILLMSG,
	NETMSGTYPE_SV_SOUNDGLOBAL,
	NETMSGTYPE_SV_TUNEPARAMS,
	NETMSGTYPE_SV_EXTRAPROJECTILE,
	NETMSGTYPE_SV_READYTOENTER,
	NETMSGTYPE_SV_WEAPONPICKUP,
	NETMSGTYPE_SV_EMOTICON,
	NETMSGTYPE_SV_VOTE_CLEAROPTIONS,
	NETMSGTYPE_SV_VOTE_OPTION,
	NETMSGTYPE_SV_VOTE_SET,
	NETMSGTYPE_SV_VOTE_STATUS,
	NETMSGTYPE_CL_SAY,
	NETMSGTYPE_CL_SETTEAM,
	NETMSGTYPE_CL_STARTINFO,
	NETMSGTYPE_CL_CHANGEINFO,
	NETMSGTYPE_CL_KILL,
	NETMSGTYPE_CL_EMOTICON,
	NETMSGTYPE_CL_VOTE,
	NETMSGTYPE_CL_CALLVOTE,
	NUM_NETMSGTYPES
};

struct NETOBJ_PLAYER_INPUT
{
	int direction;
	int target_x;
	int target_y;
	int jump;
	int fire;
	int hook;
	int player_state;
	int wanted_weapon;
	int next_weapon;
	int prev_weapon;
};

struct NETOBJ_PROJECTILE
{
	int x;
	int y;
	int vx;
	int vy;
	int type;
	int start_tick;
};

struct NETOBJ_LASER
{
	int x;
	int y;
	int from_x;
	int from_y;
	int start_tick;
};

struct NETOBJ_PICKUP
{
	int x;
	int y;
	int type;
	int subtype;
};

struct NETOBJ_FLAG
{
	int x;
	int y;
	int team;
	int carried_by;
};

struct NETOBJ_GAME
{
	int flags;
	int round_start_tick;
	int game_over;
	int sudden_death;
	int paused;
	int score_limit;
	int time_limit;
	int warmup;
	int round_num;
	int round_current;
	int teamscore_red;
	int teamscore_blue;
};

struct NETOBJ_CHARACTER_CORE
{
	int tick;
	int x;
	int y;
	int vx;
	int vy;
	int angle;
	int direction;
	int jumped;
	int hooked_player;
	int hook_state;
	int hook_tick;
	int hook_x;
	int hook_y;
	int hook_dx;
	int hook_dy;
};

struct NETOBJ_CHARACTER : public NETOBJ_CHARACTER_CORE
{
	int player_state;
	int health;
	int armor;
	int ammocount;
	int weapon;
	int emote;
	int attacktick;
};

struct NETOBJ_PLAYER_INFO
{
	int local;
	int cid;
	int team;
	int score;
	int latency;
	int latency_flux;
};

struct NETOBJ_CLIENT_INFO
{
	int name0;
	int name1;
	int name2;
	int name3;
	int name4;
	int name5;
	int skin0;
	int skin1;
	int skin2;
	int skin3;
	int skin4;
	int skin5;
	int use_custom_color;
	int color_body;
	int color_feet;
};

struct NETEVENT_COMMON
{
	int x;
	int y;
};

struct NETEVENT_EXPLOSION : public NETEVENT_COMMON
{
};

struct NETEVENT_SPAWN : public NETEVENT_COMMON
{
};

struct NETEVENT_HAMMERHIT : public NETEVENT_COMMON
{
};

struct NETEVENT_DEATH : public NETEVENT_COMMON
{
	int cid;
};

struct NETEVENT_SOUNDGLOBAL : public NETEVENT_COMMON
{
	int soundid;
};

struct NETEVENT_SOUNDWORLD : public NETEVENT_COMMON
{
	int soundid;
};

struct NETEVENT_DAMAGEIND : public NETEVENT_COMMON
{
	int angle;
};

struct NETMSG_SV_MOTD
{
	const char *message;
	void pack(int flags)
	{
		msg_pack_start(NETMSGTYPE_SV_MOTD, flags);
		msg_pack_string(message, -1);
		msg_pack_end();
	}
};

struct NETMSG_SV_BROADCAST
{
	const char *message;
	void pack(int flags)
	{
		msg_pack_start(NETMSGTYPE_SV_BROADCAST, flags);
		msg_pack_string(message, -1);
		msg_pack_end();
	}
};

struct NETMSG_SV_CHAT
{
	int team;
	int cid;
	const char *message;
	void pack(int flags)
	{
		msg_pack_start(NETMSGTYPE_SV_CHAT, flags);
		msg_pack_int(team);
		msg_pack_int(cid);
		msg_pack_string(message, -1);
		msg_pack_end();
	}
};

struct NETMSG_SV_KILLMSG
{
	int killer;
	int victim;
	int weapon;
	int mode_special;
	void pack(int flags)
	{
		msg_pack_start(NETMSGTYPE_SV_KILLMSG, flags);
		msg_pack_int(killer);
		msg_pack_int(victim);
		msg_pack_int(weapon);
		msg_pack_int(mode_special);
		msg_pack_end();
	}
};

struct NETMSG_SV_SOUNDGLOBAL
{
	int soundid;
	void pack(int flags)
	{
		msg_pack_start(NETMSGTYPE_SV_SOUNDGLOBAL, flags);
		msg_pack_int(soundid);
		msg_pack_end();
	}
};

struct NETMSG_SV_TUNEPARAMS
{
	void pack(int flags)
	{
		msg_pack_start(NETMSGTYPE_SV_TUNEPARAMS, flags);
		msg_pack_end();
	}
};

struct NETMSG_SV_EXTRAPROJECTILE
{
	void pack(int flags)
	{
		msg_pack_start(NETMSGTYPE_SV_EXTRAPROJECTILE, flags);
		msg_pack_end();
	}
};

struct NETMSG_SV_READYTOENTER
{
	void pack(int flags)
	{
		msg_pack_start(NETMSGTYPE_SV_READYTOENTER, flags);
		msg_pack_end();
	}
};

struct NETMSG_SV_WEAPONPICKUP
{
	int weapon;
	void pack(int flags)
	{
		msg_pack_start(NETMSGTYPE_SV_WEAPONPICKUP, flags);
		msg_pack_int(weapon);
		msg_pack_end();
	}
};

struct NETMSG_SV_EMOTICON
{
	int cid;
	int emoticon;
	void pack(int flags)
	{
		msg_pack_start(NETMSGTYPE_SV_EMOTICON, flags);
		msg_pack_int(cid);
		msg_pack_int(emoticon);
		msg_pack_end();
	}
};

struct NETMSG_SV_VOTE_CLEAROPTIONS
{
	void pack(int flags)
	{
		msg_pack_start(NETMSGTYPE_SV_VOTE_CLEAROPTIONS, flags);
		msg_pack_end();
	}
};

struct NETMSG_SV_VOTE_OPTION
{
	const char *command;
	void pack(int flags)
	{
		msg_pack_start(NETMSGTYPE_SV_VOTE_OPTION, flags);
		msg_pack_string(command, -1);
		msg_pack_end();
	}
};

struct NETMSG_SV_VOTE_SET
{
	int timeout;
	const char *description;
	const char *command;
	void pack(int flags)
	{
		msg_pack_start(NETMSGTYPE_SV_VOTE_SET, flags);
		msg_pack_int(timeout);
		msg_pack_string(description, -1);
		msg_pack_string(command, -1);
		msg_pack_end();
	}
};

struct NETMSG_SV_VOTE_STATUS
{
	int yes;
	int no;
	int pass;
	int total;
	void pack(int flags)
	{
		msg_pack_start(NETMSGTYPE_SV_VOTE_STATUS, flags);
		msg_pack_int(yes);
		msg_pack_int(no);
		msg_pack_int(pass);
		msg_pack_int(total);
		msg_pack_end();
	}
};

struct NETMSG_CL_SAY
{
	int team;
	const char *message;
	void pack(int flags)
	{
		msg_pack_start(NETMSGTYPE_CL_SAY, flags);
		msg_pack_int(team);
		msg_pack_string(message, -1);
		msg_pack_end();
	}
};

struct NETMSG_CL_SETTEAM
{
	int team;
	void pack(int flags)
	{
		msg_pack_start(NETMSGTYPE_CL_SETTEAM, flags);
		msg_pack_int(team);
		msg_pack_end();
	}
};

struct NETMSG_CL_STARTINFO
{
	const char *name;
	const char *skin;
	int use_custom_color;
	int color_body;
	int color_feet;
	void pack(int flags)
	{
		msg_pack_start(NETMSGTYPE_CL_STARTINFO, flags);
		msg_pack_string(name, -1);
		msg_pack_string(skin, -1);
		msg_pack_int(use_custom_color);
		msg_pack_int(color_body);
		msg_pack_int(color_feet);
		msg_pack_end();
	}
};

struct NETMSG_CL_CHANGEINFO
{
	const char *name;
	const char *skin;
	int use_custom_color;
	int color_body;
	int color_feet;
	void pack(int flags)
	{
		msg_pack_start(NETMSGTYPE_CL_CHANGEINFO, flags);
		msg_pack_string(name, -1);
		msg_pack_string(skin, -1);
		msg_pack_int(use_custom_color);
		msg_pack_int(color_body);
		msg_pack_int(color_feet);
		msg_pack_end();
	}
};

struct NETMSG_CL_KILL
{
	void pack(int flags)
	{
		msg_pack_start(NETMSGTYPE_CL_KILL, flags);
		msg_pack_end();
	}
};

struct NETMSG_CL_EMOTICON
{
	int emoticon;
	void pack(int flags)
	{
		msg_pack_start(NETMSGTYPE_CL_EMOTICON, flags);
		msg_pack_int(emoticon);
		msg_pack_end();
	}
};

struct NETMSG_CL_VOTE
{
	int vote;
	void pack(int flags)
	{
		msg_pack_start(NETMSGTYPE_CL_VOTE, flags);
		msg_pack_int(vote);
		msg_pack_end();
	}
};

struct NETMSG_CL_CALLVOTE
{
	const char *type;
	const char *value;
	void pack(int flags)
	{
		msg_pack_start(NETMSGTYPE_CL_CALLVOTE, flags);
		msg_pack_string(type, -1);
		msg_pack_string(value, -1);
		msg_pack_end();
	}
};

enum
{
	SOUND_GUN_FIRE=0,
	SOUND_SHOTGUN_FIRE,
	SOUND_GRENADE_FIRE,
	SOUND_HAMMER_FIRE,
	SOUND_HAMMER_HIT,
	SOUND_NINJA_FIRE,
	SOUND_GRENADE_EXPLODE,
	SOUND_NINJA_HIT,
	SOUND_RIFLE_FIRE,
	SOUND_RIFLE_BOUNCE,
	SOUND_WEAPON_SWITCH,
	SOUND_PLAYER_PAIN_SHORT,
	SOUND_PLAYER_PAIN_LONG,
	SOUND_BODY_LAND,
	SOUND_PLAYER_AIRJUMP,
	SOUND_PLAYER_JUMP,
	SOUND_PLAYER_DIE,
	SOUND_PLAYER_SPAWN,
	SOUND_PLAYER_SKID,
	SOUND_TEE_CRY,
	SOUND_HOOK_LOOP,
	SOUND_HOOK_ATTACH_GROUND,
	SOUND_HOOK_ATTACH_PLAYER,
	SOUND_HOOK_NOATTACH,
	SOUND_PICKUP_HEALTH,
	SOUND_PICKUP_ARMOR,
	SOUND_PICKUP_GRENADE,
	SOUND_PICKUP_SHOTGUN,
	SOUND_PICKUP_NINJA,
	SOUND_WEAPON_SPAWN,
	SOUND_WEAPON_NOAMMO,
	SOUND_HIT,
	SOUND_CHAT_SERVER,
	SOUND_CHAT_CLIENT,
	SOUND_CTF_DROP,
	SOUND_CTF_RETURN,
	SOUND_CTF_GRAB_PL,
	SOUND_CTF_GRAB_EN,
	SOUND_CTF_CAPTURE,
	NUM_SOUNDS
};
enum
{
	WEAPON_HAMMER=0,
	WEAPON_GUN,
	WEAPON_SHOTGUN,
	WEAPON_GRENADE,
	WEAPON_RIFLE,
	WEAPON_NINJA,
	NUM_WEAPONS
};
int netobj_validate(int type, void *data, int size);
const char *netobj_get_name(int type);
int netobj_get_size(int type);
void *netmsg_secure_unpack(int type);
const char *netmsg_get_name(int type);
const char *netmsg_failed_on();
int netobj_num_corrections();
const char *netobj_corrected_on();
#endif // FILE_G_PROTOCOL_H
