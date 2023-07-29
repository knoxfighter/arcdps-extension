// This header contains arcdps structs that don't need Windows.h to compile, allowing it to be included from linux
// applications
#pragma once

#include <cstdint>

//enums and structs:
/* is friend/foe */
enum iff {
	IFF_FRIEND,
	IFF_FOE,
	IFF_UNKNOWN // or uncertain
};

/* combat result (physical) */
enum cbtresult {
	CBTR_NORMAL,      // strike was neither crit or glance
	CBTR_CRIT,        // strike was crit
	CBTR_GLANCE,      // strike was glance
	CBTR_BLOCK,       // strike was blocked eg. mesmer shield 4
	CBTR_EVADE,       // strike was evaded, eg. dodge or mesmer sword 2
	CBTR_INTERRUPT,   // strike interrupted something
	CBTR_ABSORB,      // strike was "invluned" or absorbed eg. guardian elite
	CBTR_BLIND,       // strike missed
	CBTR_KILLINGBLOW, // not a damage strike, target was killed by skill by
	CBTR_DOWNED,      // not a damage strike, target was downed by skill by
	CBTR_BREAKBAR,    // not a damage strike, target had value of breakbar damage dealt
	CBTR_ACTIVATION,  // not a damage strike, on-activation event (src hit dst if damaging buff)
	CBTR_UNKNOWN
};

/* combat activation */
enum cbtactivation : uint8_t {
	ACTV_NONE,             // not used - not this kind of event
	ACTV_START,            // started skill/animation activation
	ACTV_QUICKNESS_UNUSED, // unused as of nov 5 2019
	ACTV_CANCEL_FIRE,      // stopped skill activation with reaching tooltip time
	ACTV_CANCEL_CANCEL,    // stopped skill activation without reaching tooltip time
	ACTV_RESET,            // animation completed fully
	ACTV_UNKNOWN
};

/* combat state change */
enum cbtstatechange : uint8_t {
	CBTS_NONE,             // not used - not this kind of event
	CBTS_ENTERCOMBAT,      // src_agent entered combat, dst_agent is subgroup
	CBTS_EXITCOMBAT,       // src_agent left combat
	CBTS_CHANGEUP,         // src_agent is now alive
	CBTS_CHANGEDEAD,       // src_agent is now dead
	CBTS_CHANGEDOWN,       // src_agent is now downed
	CBTS_SPAWN,            // src_agent is now in game tracking range (not in realtime api)
	CBTS_DESPAWN,          // src_agent is no longer being tracked (not in realtime api)
	CBTS_HEALTHUPDATE,     // src_agent is at health percent. dst_agent = percent * 10000 (eg. 99.5% will be 9950) (not in realtime api)
	CBTS_LOGSTART,         // log start. value = server unix timestamp **uint32**. buff_dmg = local unix timestamp. src_agent = 0x637261 (arcdps id) if evtc, npc id if realtime
	CBTS_LOGEND,           // log end. value = server unix timestamp **uint32**. buff_dmg = local unix timestamp. src_agent = 0x637261 (arcdps id) if evtc, npc id if realtime
	CBTS_WEAPSWAP,         // src_agent swapped weapon set. dst_agent = current set id (0/1 water, 4/5 land)
	CBTS_MAXHEALTHUPDATE,  // src_agent has had it's maximum health changed. dst_agent = new max health (not in realtime api)
	CBTS_POINTOFVIEW,      // src_agent is agent of "recording" player  (not in realtime api)
	CBTS_LANGUAGE,         // src_agent is text language  (not in realtime api)
	CBTS_GWBUILD,          // src_agent is game build  (not in realtime api)
	CBTS_SHARDID,          // src_agent is sever shard id  (not in realtime api)
	CBTS_REWARD,           // src_agent is self, dst_agent is reward id, value is reward type. these are the wiggly boxes that you get
	CBTS_BUFFINITIAL,      // combat event that will appear once per buff per agent on logging start (statechange==18, buff==18, normal cbtevent otherwise)
	CBTS_POSITION,         // src_agent changed, cast float* p = (float*)&dst_agent, access as x/y/z (float[3]) (not in realtime api)
	CBTS_VELOCITY,         // src_agent changed, cast float* v = (float*)&dst_agent, access as x/y/z (float[3]) (not in realtime api)
	CBTS_FACING,           // src_agent changed, cast float* f = (float*)&dst_agent, access as x/y (float[2]) (not in realtime api)
	CBTS_TEAMCHANGE,       // src_agent change, dst_agent new team id
	CBTS_ATTACKTARGET,     // src_agent is an attacktarget, dst_agent is the parent agent (gadget type), value is the current targetable state (not in realtime api)
	CBTS_TARGETABLE,       // dst_agent is new target-able state (0 = no, 1 = yes. default yes) (not in realtime api)
	CBTS_MAPID,            // src_agent is map id  (not in realtime api)
	CBTS_REPLINFO,         // internal use, won't see anywhere
	CBTS_STACKACTIVE,      // src_agent is agent with buff, dst_agent is the stackid marked active
	CBTS_STACKRESET,       // src_agent is agent with buff, value is the duration to reset to (also marks inactive), pad61- is the stackid
	CBTS_GUILD,            // src_agent is agent, dst_agent through buff_dmg is 16 byte guid (client form, needs minor rearrange for api form)
	CBTS_BUFFINFO,         // is_flanking = probably invuln, is_shields = probably invert, is_offcycle = category, pad61 = stacking type, pad62 = probably resistance, src_master_instid = max stacks (not in realtime)
	CBTS_BUFFFORMULA,      // (float*)&time[8]: type attr1 attr2 param1 param2 param3 trait_src trait_self, (float*)&src_instid[2] = buff_src buff_self, is_flanking = !npc, is_shields = !player, is_offcycle = break, overstack = value of type determined by pad61 (none/number/skill) (not in realtime, one per formula)
	CBTS_SKILLINFO,        // (float*)&time[4]: recharge range0 range1 tooltiptime (not in realtime)
	CBTS_SKILLTIMING,      // src_agent = action, dst_agent = at millisecond (not in realtime, one per timing)
	CBTS_BREAKBARSTATE,    // src_agent is agent, value is u16 game enum (active, recover, immune, none) (not in realtime api)
	CBTS_BREAKBARPERCENT,  // src_agent is agent, value is float with percent (not in realtime api)
	CBTS_ERROR,            // (char*)&time[32]: error string (not in realtime api)
	CBTS_TAG,              // src_agent is agent, value is the id (volatile, game build dependent) of the tag
	CBTS_BARRIERUPDATE,    // src_agent is at barrier percent. dst_agent = percent * 10000 (eg. 99.5% will be 9950) (not in realtime api)
	CBTS_STATRESET,        // stats reset for all players, to match arc ui (not in log)
	CBTS_EXTENSION,        // cbtevent with statechange byte set to this
	CBTS_APIDELAYED,       // cbtevent with statechange byte set to this
	CBTS_INSTANCESTART,    // src_agent is roughly the log-relative ms that the server started the instance
	CBTS_TICKRATE,         // every 500ms, src_agent = 25 - tickrate (when tickrate < 21)
	CBTS_LAST90BEFOREDOWN, // src_agent is enemy agent that went down, dst_agent is time in ms since last 90% (for downs contribution)
	CBTS_EFFECT,           // src_agent is owner. dst_agent if at agent, else &value = float[3] xyz, &iff = float[2] xy orient, &pad61 = float[1] z orient, skillid = effectid. if is_flanking: duration = trackingid. &is_shields = uint16 duration. if effectid = 0, end &is_shields = trackingid (not in realtime api)
	CBTS_IDTOGUID,         // &src_agent = 16byte persistent content guid, overstack_value is of contentlocal enum, skillid is content id  (not in realtime api)
	CBTS_LOGNPCUPDATE,     //log npc update. value = server unix timestamp **uint32**. buff_dmg = local unix timestamp. src_agent = species id
	CBTS_UNKNOWN,          // unknown or invalid, ignore
};

/* combat buff remove type */
enum cbtbuffremove : uint8_t {
	CBTB_NONE,   // not used - not this kind of event
	CBTB_ALL,    // last/all stacks removed (sent by server)
	CBTB_SINGLE, // single stack removed (sent by server). will happen for each stack on cleanse
	CBTB_MANUAL, // single stack removed (auto by arc on ooc or all stack, ignore for strip/cleanse calc, use for in/out volume)
	CBTB_UNKNOWN,
};

/* combat buff cycle type */
enum cbtbuffcycle {
	CBTC_CYCLE,                            // damage happened on tick timer
	CBTC_NOTCYCLE,                         // damage happened outside tick timer (resistable)
	CBTC_NOTCYCLENORESIST,                 // BEFORE MAY 2021: the others were lumped here, now retired
	CBTC_NOTCYCLEDMGTOTARGETONHIT,         // damage happened to target on hitting target
	CBTC_NOTCYCLEDMGTOSOURCEONHIT,         // damage happened to source on htiting target
	CBTC_NOTCYCLEDMGTOTARGETONSTACKREMOVE, // damage happened to target on source losing a stack
	CBTC_UNKNOWN
};

/* buff formula attributes */
enum e_attribute {
	ATTR_NONE,
	ATTR_POWER,
	ATTR_PRECISION,
	ATTR_TOUGHNESS,
	ATTR_VITALITY,
	ATTR_FEROCITY,
	ATTR_HEALING,
	ATTR_CONDITION,
	ATTR_CONCENTRATION,
	ATTR_EXPERTISE,
	ATTR_CUST_ARMOR,
	ATTR_CUST_AGONY,
	ATTR_CUST_STATINC,
	ATTR_CUST_PHYSINC,
	ATTR_CUST_CONDINC,
	ATTR_CUST_PHYSREC,
	ATTR_CUST_CONDREC,
	ATTR_CUST_ATTACKSPEED,
	ATTR_CUST_SIPHONINC,
	ATTR_CUST_SIPHONREC,
	ATTR_UNKNOWN = 65535
};

/* buffinfo category */
enum e_buffcategory {
	EFFECT_CAT_BOON = 0,
	EFFECT_CAT_ANY = 1,
	EFFECT_CAT_CONDITION = 2,
	EFFECT_CAT_FOOD = 4,
	EFFECT_CAT_UPGRADE = 6,
	EFFECT_CAT_BOOST = 8,
	EFFECT_CAT_TRAIT = 11,
	EFFECT_CAT_ENHANCEMENT = 13,
	EFFECT_CAT_STANCE = 16
};

/* custom skill ids */
enum cbtcustomskill {
	CSK_RESURRECT = 1066, // not custom but important and unnamed
	CSK_BANDAGE = 1175,   // personal healing only
	CSK_DODGE = 65001     // will occur in is_activation==normal event
};

/* language */
enum gwlanguage {
	GWL_ENG = 0,
	GWL_FRE = 2,
	GWL_GEM = 3,
	GWL_SPA = 4,
	GWL_CN = 5,
};

/* content local enum */
enum n_contentlocal {
	CONTENTLOCAL_EFFECT,
	CONTENTLOCAL_MARKER,
};

/* Profession used by agent */
enum Prof : uint32_t {
	PROF_UNKNOWN = 0,
	PROF_GUARD = 1,
	PROF_WARRIOR = 2,
	PROF_ENGINEER = 3,
	PROF_RANGER = 4,
	PROF_THIEF = 5,
	PROF_ELE = 6,
	PROF_MESMER = 7,
	PROF_NECRO = 8,
	PROF_RENEGADE = 9,
};

enum class WeaponSet {
	WaterFirst = 0,
	WaterSecond = 1,
	Bundles = 2,   // This includes all bundles, kits and more
	Transform = 3, // All transformations, e.g. Druid Avatar
	LandFirst = 4,
	LandSecond = 5,
};

enum ColorsCore {
	CCOL_TRANSPARENT,
	CCOL_WHITE,
	CCOL_LWHITE,
	CCOL_LGREY,
	CCOL_LYELLOW,
	CCOL_LGREEN,
	CCOL_LRED,
	CCOL_LTEAL,
	CCOL_MGREY,
	CCOL_DGREY,
	CCOL_NUM,
};

typedef struct cbtevent {
	uint64_t time;
	uintptr_t src_agent;
	uintptr_t dst_agent;
	int32_t value;
	int32_t buff_dmg;
	uint32_t overstack_value;
	uint32_t skillid;
	uint16_t src_instid;
	uint16_t dst_instid;
	uint16_t src_master_instid;
	uint16_t dst_master_instid;
	uint8_t iff;
	uint8_t buff;
	uint8_t result;
	cbtactivation is_activation;
	cbtbuffremove is_buffremove;
	uint8_t is_ninety;
	uint8_t is_fifty;
	uint8_t is_moving;
	cbtstatechange is_statechange; // statechange-enum aka uint8_t
	uint8_t is_flanking;
	uint8_t is_shields;
	uint8_t is_offcycle;
	uint8_t pad61;
	uint8_t pad62;
	uint8_t pad63;
	uint8_t pad64;
} cbtevent;

/* agent short */
typedef struct ag {
	const char* name; /* agent name. may be null. valid only at time of event. utf8 */
	uintptr_t id;     /* agent unique identifier */
	Prof prof;        /* profession at time of event. refer to evtc notes for identification */
	uint32_t elite;   /* elite spec at time of event. refer to evtc notes for identification */
	uint32_t self;    /* 1 if self, 0 if not */
	uint16_t team;    /* sep21+ */
} ag;
