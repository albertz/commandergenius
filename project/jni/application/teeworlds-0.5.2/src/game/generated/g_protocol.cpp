#include <engine/e_common_interface.h>
#include "g_protocol.hpp"
const char *msg_failed_on = "";
const char *obj_corrected_on = "";
static int num_corrections = 0;
int netobj_num_corrections() { return num_corrections; }
const char *netobj_corrected_on() { return obj_corrected_on; }
const char *netmsg_failed_on() { return msg_failed_on; }
const int max_int = 0x7fffffff;
static int netobj_clamp_int(const char *error_msg, int v, int min, int max)
{
	if(v<min) { obj_corrected_on = error_msg; num_corrections++; return min; }
	if(v>max) { obj_corrected_on = error_msg; num_corrections++; return max; }
	return v;
}
static const char *netobj_names[] = {
	"invalid",
	"player_input",
	"projectile",
	"laser",
	"pickup",
	"flag",
	"game",
	"character_core",
	"character",
	"player_info",
	"client_info",
	"common",
	"explosion",
	"spawn",
	"hammerhit",
	"death",
	"soundglobal",
	"soundworld",
	"damageind",
	""
};

static int netobj_sizes[] = {
	0,
	sizeof(NETOBJ_PLAYER_INPUT),
	sizeof(NETOBJ_PROJECTILE),
	sizeof(NETOBJ_LASER),
	sizeof(NETOBJ_PICKUP),
	sizeof(NETOBJ_FLAG),
	sizeof(NETOBJ_GAME),
	sizeof(NETOBJ_CHARACTER_CORE),
	sizeof(NETOBJ_CHARACTER),
	sizeof(NETOBJ_PLAYER_INFO),
	sizeof(NETOBJ_CLIENT_INFO),
	sizeof(NETEVENT_COMMON),
	sizeof(NETEVENT_EXPLOSION),
	sizeof(NETEVENT_SPAWN),
	sizeof(NETEVENT_HAMMERHIT),
	sizeof(NETEVENT_DEATH),
	sizeof(NETEVENT_SOUNDGLOBAL),
	sizeof(NETEVENT_SOUNDWORLD),
	sizeof(NETEVENT_DAMAGEIND),
	0
};

static int validate_player_input(void *data, int size)
{
	NETOBJ_PLAYER_INPUT *obj = (NETOBJ_PLAYER_INPUT *)data;
	if(sizeof(*obj) != size) return -1;
	netobj_clamp_int("player_state", obj->player_state, 0, 4);
	return 0;
}

static int validate_projectile(void *data, int size)
{
	NETOBJ_PROJECTILE *obj = (NETOBJ_PROJECTILE *)data;
	if(sizeof(*obj) != size) return -1;
	netobj_clamp_int("type", obj->type, 0, NUM_WEAPONS-1);
	netobj_clamp_int("start_tick", obj->start_tick, 0, max_int);
	return 0;
}

static int validate_laser(void *data, int size)
{
	NETOBJ_LASER *obj = (NETOBJ_LASER *)data;
	if(sizeof(*obj) != size) return -1;
	netobj_clamp_int("start_tick", obj->start_tick, 0, max_int);
	return 0;
}

static int validate_pickup(void *data, int size)
{
	NETOBJ_PICKUP *obj = (NETOBJ_PICKUP *)data;
	if(sizeof(*obj) != size) return -1;
	netobj_clamp_int("type", obj->type, 0, max_int);
	netobj_clamp_int("subtype", obj->subtype, 0, max_int);
	return 0;
}

static int validate_flag(void *data, int size)
{
	NETOBJ_FLAG *obj = (NETOBJ_FLAG *)data;
	if(sizeof(*obj) != size) return -1;
	netobj_clamp_int("team", obj->team, 0, 1);
	netobj_clamp_int("carried_by", obj->carried_by, -2, MAX_CLIENTS-1);
	return 0;
}

static int validate_game(void *data, int size)
{
	NETOBJ_GAME *obj = (NETOBJ_GAME *)data;
	if(sizeof(*obj) != size) return -1;
	netobj_clamp_int("flags", obj->flags, 0, 256);
	netobj_clamp_int("round_start_tick", obj->round_start_tick, 0, max_int);
	netobj_clamp_int("game_over", obj->game_over, 0, 1);
	netobj_clamp_int("sudden_death", obj->sudden_death, 0, 1);
	netobj_clamp_int("paused", obj->paused, 0, 1);
	netobj_clamp_int("score_limit", obj->score_limit, 0, max_int);
	netobj_clamp_int("time_limit", obj->time_limit, 0, max_int);
	netobj_clamp_int("warmup", obj->warmup, 0, max_int);
	netobj_clamp_int("round_num", obj->round_num, 0, max_int);
	netobj_clamp_int("round_current", obj->round_current, 0, max_int);
	return 0;
}

static int validate_character_core(void *data, int size)
{
	NETOBJ_CHARACTER_CORE *obj = (NETOBJ_CHARACTER_CORE *)data;
	if(sizeof(*obj) != size) return -1;
	netobj_clamp_int("direction", obj->direction, -1, 1);
	netobj_clamp_int("jumped", obj->jumped, 0, 3);
	netobj_clamp_int("hooked_player", obj->hooked_player, 0, MAX_CLIENTS-1);
	netobj_clamp_int("hook_state", obj->hook_state, -1, 5);
	netobj_clamp_int("hook_tick", obj->hook_tick, 0, max_int);
	return 0;
}

static int validate_character(void *data, int size)
{
	NETOBJ_CHARACTER *obj = (NETOBJ_CHARACTER *)data;
	if(sizeof(*obj) != size) return -1;
	netobj_clamp_int("player_state", obj->player_state, 0, NUM_PLAYERSTATES-1);
	netobj_clamp_int("health", obj->health, 0, 10);
	netobj_clamp_int("armor", obj->armor, 0, 10);
	netobj_clamp_int("ammocount", obj->ammocount, 0, 10);
	netobj_clamp_int("weapon", obj->weapon, 0, NUM_WEAPONS-1);
	netobj_clamp_int("emote", obj->emote, 0, 6);
	netobj_clamp_int("attacktick", obj->attacktick, 0, max_int);
	return 0;
}

static int validate_player_info(void *data, int size)
{
	NETOBJ_PLAYER_INFO *obj = (NETOBJ_PLAYER_INFO *)data;
	if(sizeof(*obj) != size) return -1;
	netobj_clamp_int("local", obj->local, 0, 1);
	netobj_clamp_int("cid", obj->cid, 0, MAX_CLIENTS-1);
	netobj_clamp_int("team", obj->team, -1, 1);
	return 0;
}

static int validate_client_info(void *data, int size)
{
	NETOBJ_CLIENT_INFO *obj = (NETOBJ_CLIENT_INFO *)data;
	if(sizeof(*obj) != size) return -1;
	netobj_clamp_int("use_custom_color", obj->use_custom_color, 0, 1);
	return 0;
}

static int validate_common(void *data, int size)
{
	NETEVENT_COMMON *obj = (NETEVENT_COMMON *)data;
	if(sizeof(*obj) != size) return -1;
	return 0;
}

static int validate_explosion(void *data, int size)
{
	NETEVENT_EXPLOSION *obj = (NETEVENT_EXPLOSION *)data;
	if(sizeof(*obj) != size) return -1;
	return 0;
}

static int validate_spawn(void *data, int size)
{
	NETEVENT_SPAWN *obj = (NETEVENT_SPAWN *)data;
	if(sizeof(*obj) != size) return -1;
	return 0;
}

static int validate_hammerhit(void *data, int size)
{
	NETEVENT_HAMMERHIT *obj = (NETEVENT_HAMMERHIT *)data;
	if(sizeof(*obj) != size) return -1;
	return 0;
}

static int validate_death(void *data, int size)
{
	NETEVENT_DEATH *obj = (NETEVENT_DEATH *)data;
	if(sizeof(*obj) != size) return -1;
	netobj_clamp_int("cid", obj->cid, 0, MAX_CLIENTS-1);
	return 0;
}

static int validate_soundglobal(void *data, int size)
{
	NETEVENT_SOUNDGLOBAL *obj = (NETEVENT_SOUNDGLOBAL *)data;
	if(sizeof(*obj) != size) return -1;
	netobj_clamp_int("soundid", obj->soundid, 0, NUM_SOUNDS-1);
	return 0;
}

static int validate_soundworld(void *data, int size)
{
	NETEVENT_SOUNDWORLD *obj = (NETEVENT_SOUNDWORLD *)data;
	if(sizeof(*obj) != size) return -1;
	netobj_clamp_int("soundid", obj->soundid, 0, NUM_SOUNDS-1);
	return 0;
}

static int validate_damageind(void *data, int size)
{
	NETEVENT_DAMAGEIND *obj = (NETEVENT_DAMAGEIND *)data;
	if(sizeof(*obj) != size) return -1;
	return 0;
}

static void *secure_unpack_sv_motd()
{
	static NETMSG_SV_MOTD msg;
	msg.message = msg_unpack_string();
	return &msg;
}

static void *secure_unpack_sv_broadcast()
{
	static NETMSG_SV_BROADCAST msg;
	msg.message = msg_unpack_string();
	return &msg;
}

static void *secure_unpack_sv_chat()
{
	static NETMSG_SV_CHAT msg;
	msg.team = msg_unpack_int();
	msg.cid = msg_unpack_int();
	msg.message = msg_unpack_string();
	if(msg.team < -1 || msg.team > 1) { msg_failed_on = "team"; return 0; }
	if(msg.cid < -1 || msg.cid > MAX_CLIENTS-1) { msg_failed_on = "cid"; return 0; }
	return &msg;
}

static void *secure_unpack_sv_killmsg()
{
	static NETMSG_SV_KILLMSG msg;
	msg.killer = msg_unpack_int();
	msg.victim = msg_unpack_int();
	msg.weapon = msg_unpack_int();
	msg.mode_special = msg_unpack_int();
	if(msg.killer < 0 || msg.killer > MAX_CLIENTS-1) { msg_failed_on = "killer"; return 0; }
	if(msg.victim < 0 || msg.victim > MAX_CLIENTS-1) { msg_failed_on = "victim"; return 0; }
	if(msg.weapon < -3 || msg.weapon > NUM_WEAPONS-1) { msg_failed_on = "weapon"; return 0; }
	return &msg;
}

static void *secure_unpack_sv_soundglobal()
{
	static NETMSG_SV_SOUNDGLOBAL msg;
	msg.soundid = msg_unpack_int();
	if(msg.soundid < 0 || msg.soundid > NUM_SOUNDS-1) { msg_failed_on = "soundid"; return 0; }
	return &msg;
}

static void *secure_unpack_sv_tuneparams()
{
	static NETMSG_SV_TUNEPARAMS msg;
	return &msg;
}

static void *secure_unpack_sv_extraprojectile()
{
	static NETMSG_SV_EXTRAPROJECTILE msg;
	return &msg;
}

static void *secure_unpack_sv_readytoenter()
{
	static NETMSG_SV_READYTOENTER msg;
	return &msg;
}

static void *secure_unpack_sv_weaponpickup()
{
	static NETMSG_SV_WEAPONPICKUP msg;
	msg.weapon = msg_unpack_int();
	if(msg.weapon < 0 || msg.weapon > NUM_WEAPONS-1) { msg_failed_on = "weapon"; return 0; }
	return &msg;
}

static void *secure_unpack_sv_emoticon()
{
	static NETMSG_SV_EMOTICON msg;
	msg.cid = msg_unpack_int();
	msg.emoticon = msg_unpack_int();
	if(msg.cid < 0 || msg.cid > MAX_CLIENTS-1) { msg_failed_on = "cid"; return 0; }
	if(msg.emoticon < 0 || msg.emoticon > NUM_EMOTICONS-1) { msg_failed_on = "emoticon"; return 0; }
	return &msg;
}

static void *secure_unpack_sv_vote_clearoptions()
{
	static NETMSG_SV_VOTE_CLEAROPTIONS msg;
	return &msg;
}

static void *secure_unpack_sv_vote_option()
{
	static NETMSG_SV_VOTE_OPTION msg;
	msg.command = msg_unpack_string();
	return &msg;
}

static void *secure_unpack_sv_vote_set()
{
	static NETMSG_SV_VOTE_SET msg;
	msg.timeout = msg_unpack_int();
	msg.description = msg_unpack_string();
	msg.command = msg_unpack_string();
	if(msg.timeout < 0 || msg.timeout > 60) { msg_failed_on = "timeout"; return 0; }
	return &msg;
}

static void *secure_unpack_sv_vote_status()
{
	static NETMSG_SV_VOTE_STATUS msg;
	msg.yes = msg_unpack_int();
	msg.no = msg_unpack_int();
	msg.pass = msg_unpack_int();
	msg.total = msg_unpack_int();
	if(msg.yes < 0 || msg.yes > MAX_CLIENTS) { msg_failed_on = "yes"; return 0; }
	if(msg.no < 0 || msg.no > MAX_CLIENTS) { msg_failed_on = "no"; return 0; }
	if(msg.pass < 0 || msg.pass > MAX_CLIENTS) { msg_failed_on = "pass"; return 0; }
	if(msg.total < 0 || msg.total > MAX_CLIENTS) { msg_failed_on = "total"; return 0; }
	return &msg;
}

static void *secure_unpack_cl_say()
{
	static NETMSG_CL_SAY msg;
	msg.team = msg_unpack_int();
	msg.message = msg_unpack_string();
	if(msg.team < 0 || msg.team > 1) { msg_failed_on = "team"; return 0; }
	return &msg;
}

static void *secure_unpack_cl_setteam()
{
	static NETMSG_CL_SETTEAM msg;
	msg.team = msg_unpack_int();
	if(msg.team < -1 || msg.team > 1) { msg_failed_on = "team"; return 0; }
	return &msg;
}

static void *secure_unpack_cl_startinfo()
{
	static NETMSG_CL_STARTINFO msg;
	msg.name = msg_unpack_string();
	msg.skin = msg_unpack_string();
	msg.use_custom_color = msg_unpack_int();
	msg.color_body = msg_unpack_int();
	msg.color_feet = msg_unpack_int();
	if(msg.use_custom_color < 0 || msg.use_custom_color > 1) { msg_failed_on = "use_custom_color"; return 0; }
	return &msg;
}

static void *secure_unpack_cl_changeinfo()
{
	static NETMSG_CL_CHANGEINFO msg;
	msg.name = msg_unpack_string();
	msg.skin = msg_unpack_string();
	msg.use_custom_color = msg_unpack_int();
	msg.color_body = msg_unpack_int();
	msg.color_feet = msg_unpack_int();
	if(msg.use_custom_color < 0 || msg.use_custom_color > 1) { msg_failed_on = "use_custom_color"; return 0; }
	return &msg;
}

static void *secure_unpack_cl_kill()
{
	static NETMSG_CL_KILL msg;
	return &msg;
}

static void *secure_unpack_cl_emoticon()
{
	static NETMSG_CL_EMOTICON msg;
	msg.emoticon = msg_unpack_int();
	if(msg.emoticon < 0 || msg.emoticon > NUM_EMOTICONS-1) { msg_failed_on = "emoticon"; return 0; }
	return &msg;
}

static void *secure_unpack_cl_vote()
{
	static NETMSG_CL_VOTE msg;
	msg.vote = msg_unpack_int();
	if(msg.vote < -1 || msg.vote > 1) { msg_failed_on = "vote"; return 0; }
	return &msg;
}

static void *secure_unpack_cl_callvote()
{
	static NETMSG_CL_CALLVOTE msg;
	msg.type = msg_unpack_string();
	msg.value = msg_unpack_string();
	return &msg;
}

static int validate_invalid(void *data, int size) { return -1; }
typedef int(*VALIDATEFUNC)(void *data, int size);
static VALIDATEFUNC validate_funcs[] = {
	validate_invalid,
	validate_player_input,
	validate_projectile,
	validate_laser,
	validate_pickup,
	validate_flag,
	validate_game,
	validate_character_core,
	validate_character,
	validate_player_info,
	validate_client_info,
	validate_common,
	validate_explosion,
	validate_spawn,
	validate_hammerhit,
	validate_death,
	validate_soundglobal,
	validate_soundworld,
	validate_damageind,
	0x0
};

int netobj_validate(int type, void *data, int size)
{
	if(type < 0 || type >= NUM_NETOBJTYPES) return -1;
	return validate_funcs[type](data, size);
};

const char *netobj_get_name(int type)
{
	if(type < 0 || type >= NUM_NETOBJTYPES) return "(out of range)";
	return netobj_names[type];
};

int netobj_get_size(int type)
{
	if(type < 0 || type >= NUM_NETOBJTYPES) return 0;
	return netobj_sizes[type];
};

static void *secure_unpack_invalid() { return 0; }
typedef void *(*SECUREUNPACKFUNC)();
static SECUREUNPACKFUNC secure_unpack_funcs[] = {
	secure_unpack_invalid,
	secure_unpack_sv_motd,
	secure_unpack_sv_broadcast,
	secure_unpack_sv_chat,
	secure_unpack_sv_killmsg,
	secure_unpack_sv_soundglobal,
	secure_unpack_sv_tuneparams,
	secure_unpack_sv_extraprojectile,
	secure_unpack_sv_readytoenter,
	secure_unpack_sv_weaponpickup,
	secure_unpack_sv_emoticon,
	secure_unpack_sv_vote_clearoptions,
	secure_unpack_sv_vote_option,
	secure_unpack_sv_vote_set,
	secure_unpack_sv_vote_status,
	secure_unpack_cl_say,
	secure_unpack_cl_setteam,
	secure_unpack_cl_startinfo,
	secure_unpack_cl_changeinfo,
	secure_unpack_cl_kill,
	secure_unpack_cl_emoticon,
	secure_unpack_cl_vote,
	secure_unpack_cl_callvote,
	0x0
};
void *netmsg_secure_unpack(int type)
{
	void *msg;
	msg_failed_on = "";
	if(type < 0 || type >= NUM_NETMSGTYPES) { msg_failed_on = "(type out of range)"; return 0; }
	msg = secure_unpack_funcs[type]();
	if(msg_unpack_error()) { msg_failed_on = "(unpack error)"; return 0; }
	return msg;
};

static const char *message_names[] = {
	"invalid",
	"sv_motd",
	"sv_broadcast",
	"sv_chat",
	"sv_killmsg",
	"sv_soundglobal",
	"sv_tuneparams",
	"sv_extraprojectile",
	"sv_readytoenter",
	"sv_weaponpickup",
	"sv_emoticon",
	"sv_vote_clearoptions",
	"sv_vote_option",
	"sv_vote_set",
	"sv_vote_status",
	"cl_say",
	"cl_setteam",
	"cl_startinfo",
	"cl_changeinfo",
	"cl_kill",
	"cl_emoticon",
	"cl_vote",
	"cl_callvote",
	""
};

const char *netmsg_get_name(int type)
{
	if(type < 0 || type >= NUM_NETMSGTYPES) return "(out of range)";
	return message_names[type];
};

