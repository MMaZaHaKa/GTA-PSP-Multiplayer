/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"
#include "ModelIndices.h"
#include "Camera.h"
#include "Radar.h"

#include "multiplayer/MultiGame.h"
#include "multiplayer/natives/public.h"
#include "multiplayer/elements/sPlayer.h"

#define ENSURE_COLOR(v) ((uint8)(Clamp((int32)(v), 0, 255)))

int lsn_simsch_step(lua_State* L); // from simsch.cpp

void lsc_getVectorFromStack(CVector& vec, lua_State* L, int index, bool allowEntity) {
	if (index < 0)
		index = lua_gettop(L) + index + 1;
	if (lua_istable(L, index)) {
		// stack: t, t[1], t[2], t[3]*
		lua_rawgeti(L, index, 1);
		lua_rawgeti(L, index, 2);
		lua_rawgeti(L, index, 3);
		if (!lua_isnumber(L, -3) || !lua_isnumber(L, -2)) {
			luaL_typerror(L, index, allowEntity ? "vector or entity" : "vector");
		}
		vec.x = lua_tonumber(L, -3);
		vec.y = lua_tonumber(L, -2);
		if (lua_isnumber(L, -1)) vec.z = lua_tonumber(L, -1);
		else vec.z = 0;
		lua_settop(L, -4);
	}
	else {
		sElement* pElem = allowEntity ? lsc_get_entity(L, index) : nil;
		if (pElem == nil) {
			luaL_typerror(L, index, "vector or entity");
			return;
		}
		if (pElem->HasCapability(sPlayer::Capability()))
			vec = ((sPlayer*)pElem)->GetPosition();
		else
			vec = ((sElementPhysical*)pElem)->GetSync().elementphysical->GetMatrix().GetPosition();
	}
}

void lsc_getVectorFromStackNoEntity(CVector& vec, lua_State* L, int index) {
	if (lua_istable(L, index))
	{
		lua_rawgeti(L, index, 1);
		float fValue = luaL_checknumber(L, -1);
		vec = CVector(fValue, fValue, fValue);
		lua_rawgeti(L, index, 2);
		if (lua_isnumber(L, -1)) 
			vec.y = lua_tonumber(L, -1);
		lua_rawgeti(L, index, 3);
		if (lua_isnumber(L, -1))
			vec.z = lua_tonumber(L, -1);
		lua_settop(L, -4);
	}
	else {
		float fValue = luaL_checknumber(L, index);
		vec = CVector(fValue, fValue, fValue);
	}
}

void lsc_pushVector3D(lua_State* L, float x, float y, float z) {
	lua_newtable(L);
	lua_pushnumber(L, x);
	lua_rawseti(L, -2, 1);
	lua_pushnumber(L, y);
	lua_rawseti(L, -2, 2);
	lua_pushnumber(L, z);
	lua_rawseti(L, -2, 3);
}

void lsc_pushVuVector(lua_State* L, CVector& vec) {
	lsc_pushVector3D(L, vec.x, vec.y, vec.z);
}

bool lsc_isPlayerUserData(lua_State* L, int index) {
	return luaL_checkudata(L, index, "PlayerId") != nil;
}

/* unsure: returns the peer ID */
int32 lsc_getPlayer(lua_State* L, int index) {
	void* pData = luaL_checkudata(L, index, "PlayerId");
	return (pData != nil) ? (*(int32*)pData) : -1;
}

int32 lsc_getPlayerSafety(lua_State* L, int index) {
	int nPlayerID = lsc_getPlayer(L, index);
	if (nPlayerID < 0)
		return luaL_argerror(L, index, "Lua PlayerId expected");
	return nPlayerID;
}

// return packed handle
inline uint32 lsc_unpack_entity(lua_State* L, int index) {
	return ((uint32)lua_touserdata(L, index));
}

bool lsc_is_entity_tracked(lua_State* L, int index) {
	if (lua_islightuserdata(L, index)) {
		uint32 nHandle = lsc_unpack_entity(L, index);
		return TRACKED_ENTITY_TRACK(nHandle);
	}
	return false;
}

uint32 lsc_get_tracked_entity(lua_State* L, int index) {
	uint32 nHandle = lsc_unpack_entity(L, index);
	luaL_getmetatable(L, "EntityTrack");
	if (!lua_isnil(L, -1)) {
		lua_pushvalue(L, index);
		lua_gettable(L, -2);
		if (!lua_isnil(L, -1)) {
			nHandle = lsc_unpack_entity(L, -1);
		}
		lua_pop(L, 2); // lua_settop(L, -3);
	}
	else {
		lua_pop(L, 1); // lua_settop(L, -2);
	}
	return nHandle;
}

void lsc_transfer_tracked_entity(int32 nFromOwner, uint16 nFromID, int32 nToOwner, uint16 nToID)
{
	lua_State* L = cLWrapper::Instance().m_luaVM;
	luaL_getmetatable(L, "EntityTrack");
	if (!lua_isnil(L, -1)) {
		uint32 nFromHandle = TRACKED_ENTITY_PACK(nFromOwner, nFromID);
		uint32 nNewToHandle = TRACKED_ENTITY_PACK(nToOwner, nToID);
		lua_pushnil(L);
		while (lua_next(L, -2)) {
			// stack: ... table(-3), key(-2 light oldKey), value(-1 light oldHandle)
			if (lsc_unpack_entity(L, -1) == nFromHandle) {
				uint32 nInfo = (uint32)lua_touserdata(L, -2);
				lua_pop(L, 1); // lua_settop(L, -2);
				lua_pushlightuserdata(L, (void*)nNewToHandle);
				lua_settable(L, -3);
				lua_pushlightuserdata(L, (void*)nInfo);
			}
			else {
				lua_pop(L, 1); // lua_settop(L, -2);
			}
		}
	}
	lua_pop(L, 1); // lua_settop(L, -2);
}

void lsc_register_tracked_entity(lua_State* L, sElement* pElem) {
	uint32 nHandle = TRACKED_ENTITY_PACK(pElem->GetOwner(), pElem->GetID());
	lua_pushlightuserdata(L, (void*)nHandle);
	luaL_getmetatable(L, "EntityTrack");
	if (lua_isnil(L, -1)) { // if it's nil
		lua_pop(L, 1); // lua_settop(L, -2); // pop last element from stack
		luaL_newmetatable(L, "EntityTrack"); // create metatable
		lua_pushstring(L, "__mode");
		lua_pushstring(L, "k");
		lua_rawset(L, -3); // EntityTrack["_mode"] = "k"
	}
	lua_pushvalue(L, -2);
	lua_pushvalue(L, -1);
	lua_settable(L, -3); // EntityTrack[udata] = udata
	lua_setmetatable(L, -2); // pops EntityTrack and sets as metatable of light user data
}

sElement* lsc_get_entity(lua_State* L, int index) {
	cMultiGame& Game = cMultiGame::Instance();
	sElement* pElement = nil;
	if (lsc_is_entity_tracked(L, index)) {
		uint32 nHandle = lsc_get_tracked_entity(L, index);
		pElement = Game.GetEntityForHandle(TRACKED_ENTITY_OWNER(nHandle), TRACKED_ENTITY_ID(nHandle));
	}
	else if (lsc_isPlayerUserData(L, index)) {
		int32 nPlayerID = lsc_getPlayer(L, index);
		pElement = Game.GetEntityForHandle(nPlayerID, eElementID::MG_ELEMENT_PLAYER_ID);
	}
	return pElement;
}

uint32 lsc_getColor(lua_State* L, int index) {
	if (lua_isnumber(L, index)) return luaL_checknumber(L, index);
	int32 nTop = lua_gettop(L);
	if (!lua_istable(L, index)) {
		lua_pushstring(L, "colour");
		lua_gettable(L, LUA_GLOBALSINDEX);
		lua_pushvalue(L, index);
		lua_rawget(L, -2);
		index = lua_gettop(L);
	}
	lua_rawgeti(L, index, 1);
	uint8 red, green, blue, alpha;
	red = ENSURE_COLOR(lua_tonumber(L, -1));
	lua_rawgeti(L, index, 2);
	green = ENSURE_COLOR(lua_tonumber(L, -1));
	lua_rawgeti(L, index, 3);
	blue = ENSURE_COLOR(lua_tonumber(L, -1));
	lua_rawgeti(L, index, 4);
	alpha = 255;
	if (lua_isnumber(L, -1)) {
		alpha = ENSURE_COLOR(lua_tonumber(L, -1));
	}
	lua_settop(L, nTop);
	return CRGBA_PACK(red, green, blue, alpha);
}

/* TODO: stub */
#ifdef GTA_LIBERTY
int32 lsc_pop_peer_id_from_stack(lua_State* L, int index, int defaultID) {
	cMultiGame& Game = cMultiGame::Instance();
	int32 nPeerID = 0;
	if (!lua_istable(L, index)) {
		if (lsc_isPlayerUserData(L, index)) {
			int32 nHandle = lsc_getPlayer(L, index);
			lua_remove(L, index);
			return nHandle;
		}
	}
	luaL_getmetatable(L, "PeerGroups");
	if (lua_isnil(L, -1)) {
		lua_settop(L, -2);
		luaL_newmetatable(L, "PeerGroups");
		lua_pushvalue(L, -1);
		lua_setmetatable(L, -2);
		lua_pushstring(L, "__mode");
		lua_pushstring(L, "k");
		lua_rawset(L, -3);
	}
	lua_pushvalue(L, index);
	lua_rawget(L, -2);
	if (!lua_isnil(L, -1)) {
		if (lua_isboolean(L, -1)) {
			lua_settop(L, -3);
			nPeerID = (defaultID == 0xB00B5) ? Game.LocalPlayerID() : defaultID;
		}
		else {
			lua_remove(L, index);
			nPeerID = lua_tonumber(L, -1);
			lua_settop(L, -3);
		}
		return nPeerID;
	}
	bool bFoundPlayers = false;
	if (lua_next(L, index)) {
		do {
			int nHandle = lsc_getPlayer(L, -1);
			if (nHandle != -1) {
				bFoundPlayers = true;
				// TODO: push player to vector
				TODO();
				TODO();
				TODO();
			}
			lua_settop(L, -2);
		} while (lua_next(L, index));
	}
	else {
		bFoundPlayers = true;
	}
	if (!bFoundPlayers) {
		lua_pushvalue(L, index);
		lua_pushboolean(L, false);
		lua_rawset(L, -3);
		lua_settop(L, -2);
		nPeerID = (defaultID == 0xB00B5) ? Game.LocalPlayerID() : defaultID;
		return nPeerID;
	}
	// TODO: implement std::vector
	TODO();
	TODO();
	TODO();
	TODO();
	nPeerID = Game.FindPeerID(nil);
	lua_pushvalue(L, index);
	lua_remove(L, index);
	lua_pushnumber(L, nPeerID);
	lua_rawset(L, -3);
	lua_settop(L, -2);
	return nPeerID;
}
#endif

static bool lsc_check_status_ok(int status, base::string& dest) {
	if (status <= 0) return true;
	switch (status) {
	case 1:
		dest = "Lua: Error while running chunk"; break;
	case 2:
		dest = "Lua: Error occurred while opening file"; break;
	case 3:
		dest = "Lua: Syntax error during pre-compilation"; break;
	case 4:
		dest = "Lua: Memory allocation error"; break;
	case 5:
		dest = "Lua: Generic error or an error occurred while running the error handler"; break;
	default:
		dest = "Lua: Unknown error"; break;
	}
	return false;
}

void lsc_log(const char* msg) {
	debug("%s\n", msg);
}

static void lsc_set_exec_err() {
	assert(false && "lua joskiy pizdec");
	cAdhoc::Instance().SetHasError();
}

int lsc_report(lua_State* L, int status, int index) {
	base::string sErr;
	int nTop = lua_gettop(L);
	if (!lsc_check_status_ok(status, sErr)) {
		if (status != 5 && index < nTop && lua_tostring(L, -1))
			sErr.Append(lua_tostring(L, -1));
		else if (index < nTop)
			sErr.Append("\n");
		sErr.Append("\n");
		lsc_log(sErr.c_str());
		lua_settop(L, index);
		lsc_set_exec_err();
	}
	return 0;
}

int lsc_call(lua_State* L, int narg, bool clear) {
	int base = (lua_gettop(L) - narg); // function index
	lua_pushstring(L, "_TRACEBACK");
	lua_rawget(L, LUA_GLOBALSINDEX); // get traceback function
	lua_insert(L, base); // put it under chunk and args
	int status = lua_pcall(L, narg, clear ? 0 : LUA_MULTRET, base);
	lua_remove(L, base); // remove traceback function
	return lsc_report(L, status, base - 1);
}

/* TODO: this seems to be a public wrapper */
int lsc_simsch_update(lua_State* L) {
	return lsn_simsch_step(L);
}

void lsc_update_simsch() {
	if (!HasLuaInitialized()) return;
	lua_State* vm = cLWrapper::Instance().m_luaVM;
	lua_pushcclosure(vm, lsc_simsch_update, 0);
	lsc_call(vm, 0, false);
}

int lsn_none(lua_State* L) {
	lua_pushstring(L, "not implemented");
	lua_error(L);
	return 0;
}

void lscript_open_constants() {
	lua_State* L = cLWrapper::Instance().m_luaVM;

	// colors ["colour"]
	LUA_TABLE_BEGIN();
		LUA_PUSH_CONSTANT_VEC("red",     255, 0,   0  ); // t.red = {255, 0, 0}
		LUA_PUSH_CONSTANT_VEC("green",   0,   255, 0  );
		LUA_PUSH_CONSTANT_VEC("yellow",  255, 255, 0  );
		LUA_PUSH_CONSTANT_VEC("blue",    0,   0,   255);
		LUA_PUSH_CONSTANT_VEC("purple",  255, 0,   255);
		LUA_PUSH_CONSTANT_VEC("magenta", 255, 0,   255); // same?
		LUA_PUSH_CONSTANT_VEC("cyan",    0,   255, 255);
		LUA_PUSH_CONSTANT_VEC("white",   255, 255, 255);
	LUA_TABLE_END("colour"); // _G.colour = t


	// weapon IDs ["weapon"]
	LUA_TABLE_BEGIN();
		LUA_PUSH_CONSTANT("rocket_launcher", WEAPONTYPE_ROCKETLAUNCHER);
	LUA_TABLE_END("weapon");


	// vehicle IDs ["car"]
	LUA_TABLE_BEGIN();
		LUA_PUSH_CONSTANT("spider",      MI_SPIDER);       // LCS
		LUA_PUSH_CONSTANT("landstalker", MI_LANDSTAL);
		LUA_PUSH_CONSTANT("idaho",       MI_IDAHO);
		LUA_PUSH_CONSTANT("stinger",     MI_STINGER);
		LUA_PUSH_CONSTANT("perennial",   MI_PEREN);
		LUA_PUSH_CONSTANT("sentinel",    MI_SENTINEL);
		LUA_PUSH_CONSTANT("patriot",     MI_PATRIOT);
		LUA_PUSH_CONSTANT("manana",      MI_MANANA);
		LUA_PUSH_CONSTANT("infernus",    MI_INFERNUS);
		LUA_PUSH_CONSTANT("blista",      MI_BLISTAC);
		LUA_PUSH_CONSTANT("pony",        MI_PONY);
		LUA_PUSH_CONSTANT("mule",        MI_MULE);
#if defined(FIX_BUGS) || defined(GTA_LIBERTY)
		LUA_PUSH_CONSTANT("cheetah",     MI_CHEETAH);
#else
		LUA_PUSH_CONSTANT("cheetah",    -960); //------ :/ ??
#endif
		LUA_PUSH_CONSTANT("moonbeam",    MI_MOONBEAM);
		LUA_PUSH_CONSTANT("esperanto",   MI_ESPERANT);
#ifdef GTA_LIBERTY
		LUA_PUSH_CONSTANT("kuruma",      MI_KURUMA);
#else
		LUA_PUSH_CONSTANT("kuruma",      MI_WALTON);  // MI_KURUMA //------ :/ ??
#endif
		LUA_PUSH_CONSTANT("bobcat",      MI_BOBCAT);
		LUA_PUSH_CONSTANT("corpse",      MI_HEARSE);       // LCS
		LUA_PUSH_CONSTANT("securicar",   MI_SECURICA);
		LUA_PUSH_CONSTANT("banshee",     MI_BANSHEE);
		LUA_PUSH_CONSTANT("cabbie",      MI_CABBIE);
		LUA_PUSH_CONSTANT("stallion",    MI_STALLION);
		LUA_PUSH_CONSTANT("rumpo",       MI_RUMPO);
		LUA_PUSH_CONSTANT("bellyup",     MI_BELLYUP);      // LCS
		LUA_PUSH_CONSTANT("mrwongs",     MI_MRWONGS);      // LCS
		LUA_PUSH_CONSTANT("mafia",       MI_MAFIA);        // LCS
#ifdef GTA_LIBERTY
		LUA_PUSH_CONSTANT("yardie",      MI_YARDIE);
#endif
		LUA_PUSH_CONSTANT("yakuza",      MI_YAKUZA);       // LCS
		LUA_PUSH_CONSTANT("diablos",     MI_DIABLOS);      // LCS
		LUA_PUSH_CONSTANT("columbians",  MI_COLUMB);       // LCS
		LUA_PUSH_CONSTANT("hoods",       MI_HOODS);        // LCS
		LUA_PUSH_CONSTANT("panlant",     MI_PANLANT);      // LCS
		LUA_PUSH_CONSTANT("yankee",      MI_YANKEE);
		LUA_PUSH_CONSTANT("shelby",      MI_SHELBY);
		LUA_PUSH_CONSTANT("pontiac",     MI_PONTIAC);      // LCS
		LUA_PUSH_CONSTANT("esprit",      MI_ESPRIT);       // LCS
#if defined(FIX_BUGS) || defined(GTA_LIBERTY)
		LUA_PUSH_CONSTANT("mini",        MI_AMMOTRUK);
#else
		LUA_PUSH_CONSTANT("mini",       -946); //------ :/ ??
#endif
		LUA_PUSH_CONSTANT("hotrod",      MI_HOTROD);       // LCS
		LUA_PUSH_CONSTANT("sindacco",    MI_SINDACCO_CAR); // LCS
		LUA_PUSH_CONSTANT("forelli",     MI_FORELLI_CAR);  // LCS
		LUA_PUSH_CONSTANT("hells",       MI_ANGEL);
		LUA_PUSH_CONSTANT("bike",        MI_PCJ600);
		LUA_PUSH_CONSTANT("moped",       MI_FAGGIO);
		LUA_PUSH_CONSTANT("harley",      MI_FREEWAY);
		LUA_PUSH_CONSTANT("dirtbike",    MI_SANCHEZ);
#ifdef GTA_LIBERTY
		LUA_PUSH_CONSTANT("dirtbike2",   MI_SANCHEZ2);
		LUA_PUSH_CONSTANT("hells2",      MI_ANGEL2);
#else // VCS MP NEW
		LUA_PUSH_CONSTANT("hells2",      MI_ANGEL);
		LUA_PUSH_CONSTANT("limo",        MI_STRETCH);
		LUA_PUSH_CONSTANT("jetski",      MI_JETSKI);
		LUA_PUSH_CONSTANT("speedboat",   MI_JETMAX);
		LUA_PUSH_CONSTANT("hunter",      MI_HUNTER);
		LUA_PUSH_CONSTANT("maverick",    MI_POLMAV);
		LUA_PUSH_CONSTANT("fbirancher",  MI_FBICAR);
		LUA_PUSH_CONSTANT("tank",        MI_RHINO);
		LUA_PUSH_CONSTANT("hermes",      MI_HERMES);
		LUA_PUSH_CONSTANT("pimp",        MI_PIMP);
		LUA_PUSH_CONSTANT("ambulance",   MI_AMBULAN);
		LUA_PUSH_CONSTANT("police",      MI_POLICE);
		LUA_PUSH_CONSTANT("enforcer",    MI_ENFORCER);
		LUA_PUSH_CONSTANT("firetruck",   MI_FIRETRUCK);
		LUA_PUSH_CONSTANT("squalo",      MI_SQUALO);
		LUA_PUSH_CONSTANT("seaplane",    MI_SKIMMER);
		LUA_PUSH_CONSTANT("topfun",      MI_TOPFUN);
		LUA_PUSH_CONSTANT("caddy",       MI_CADDY);
		LUA_PUSH_CONSTANT("chollo",      MI_CHOLLO);
		LUA_PUSH_CONSTANT("jeep",        MI_MESA);
		LUA_PUSH_CONSTANT("dingy",       MI_DINGHY);
		LUA_PUSH_CONSTANT("electra",     MI_ELECTRAP);
		LUA_PUSH_CONSTANT("quad",        MI_QUAD);
		LUA_PUSH_CONSTANT("hovercraft",  MI_HOVERCR);
		LUA_PUSH_CONSTANT("seasparrow",  MI_SEASPAR);
		LUA_PUSH_CONSTANT("speeder",     MI_SPEEDER);
		LUA_PUSH_CONSTANT("bfinject",    MI_BFINJECT);
#if 1 // missed vcs mi, custom
		LUA_PUSH_CONSTANT("6atv",        MI_6ATV);
		LUA_PUSH_CONSTANT("admiral",     MI_ADMIRAL);
		LUA_PUSH_CONSTANT("autogyro",    MI_AUTOGYRO);
		LUA_PUSH_CONSTANT("baggage",     MI_BAGGAGE);
		LUA_PUSH_CONSTANT("bmxboy",      MI_BMXBOY);
		LUA_PUSH_CONSTANT("bmxgirl",     MI_BMXGIRL);
		LUA_PUSH_CONSTANT("bulldoze",    MI_BULLDOZE);
		LUA_PUSH_CONSTANT("burrito",     MI_BURRITO);
		LUA_PUSH_CONSTANT("speeder2",    MI_SPEEDER2);
		LUA_PUSH_CONSTANT("deluxo",      MI_DELUXO);
		LUA_PUSH_CONSTANT("huey",        MI_HUEY);
		LUA_PUSH_CONSTANT("hueyhosp",    MI_HUEYHOSP);
		LUA_PUSH_CONSTANT("electrag",    MI_ELECTRAG);
		LUA_PUSH_CONSTANT("glendale",    MI_GLENDALE);
		LUA_PUSH_CONSTANT("greenwoo",    MI_GREENWOO);
		LUA_PUSH_CONSTANT("mop50",       MI_MOP50);
		LUA_PUSH_CONSTANT("oceanic",     MI_OCEANIC);
		LUA_PUSH_CONSTANT("vicechee",    MI_VICECHEE);
		LUA_PUSH_CONSTANT("bobo",        MI_BOBO);
		LUA_PUSH_CONSTANT("maverick2",   MI_MAVERICK);
		LUA_PUSH_CONSTANT("reefer",      MI_REEFER);
		LUA_PUSH_CONSTANT("linerun",     MI_LINERUN);
		LUA_PUSH_CONSTANT("walton",      MI_WALTON);
		LUA_PUSH_CONSTANT("barracks",    MI_BARRACKS);
		LUA_PUSH_CONSTANT("predator",    MI_PREDATOR);
		LUA_PUSH_CONSTANT("flatbed",     MI_FLATBED);
		LUA_PUSH_CONSTANT("biplane",     MI_BIPLANE);
		LUA_PUSH_CONSTANT("yola",        MI_YOLA);
		LUA_PUSH_CONSTANT("taxi",        MI_TAXI);
		LUA_PUSH_CONSTANT("boxville",    MI_BOXVILLE);
		LUA_PUSH_CONSTANT("benson",      MI_BENSON);
		LUA_PUSH_CONSTANT("coach",       MI_COACH);
		LUA_PUSH_CONSTANT("voodoo",      MI_VOODOO);
		LUA_PUSH_CONSTANT("trash",       MI_TRASH);
		LUA_PUSH_CONSTANT("mrwhoop",     MI_MRWHOOP);
		LUA_PUSH_CONSTANT("sandking",    MI_SANDKING);
		LUA_PUSH_CONSTANT("marquis",     MI_MARQUIS);
		LUA_PUSH_CONSTANT("rio",         MI_RIO);
		LUA_PUSH_CONSTANT("tropic",      MI_TROPIC);
		LUA_PUSH_CONSTANT("forklift",    MI_FORKLIFT);
		LUA_PUSH_CONSTANT("streetfi",    MI_STREETFI);
		LUA_PUSH_CONSTANT("virgo",       MI_VIRGO);
		LUA_PUSH_CONSTANT("pheonix",     MI_PHEONIX);
		LUA_PUSH_CONSTANT("vcnmav",      MI_VCNMAV);
		LUA_PUSH_CONSTANT("sparrow",     MI_SPARROW);
		LUA_PUSH_CONSTANT("scarab",      MI_SCARAB);
		LUA_PUSH_CONSTANT("comet",       MI_COMET);
		LUA_PUSH_CONSTANT("cuban",       MI_CUBAN);
		LUA_PUSH_CONSTANT("fbiranch",    MI_FBIRANCH);
		LUA_PUSH_CONSTANT("gangbur",     MI_GANGBUR);
		LUA_PUSH_CONSTANT("regina",      MI_REGINA);
		LUA_PUSH_CONSTANT("sabre",       MI_SABRE);
		LUA_PUSH_CONSTANT("sabretur",    MI_SABRETUR);
		LUA_PUSH_CONSTANT("sentxs",      MI_SENTXS);
		LUA_PUSH_CONSTANT("washing",     MI_WASHING);
		LUA_PUSH_CONSTANT("coastg",      MI_COASTG);
		LUA_PUSH_CONSTANT("chopper",     MI_CHOPPER);
		LUA_PUSH_CONSTANT("airtrain",    MI_AIRTRAIN);
#endif // missed models
#endif // GTA_LIBERTY
	LUA_TABLE_END("car");


	// weapon IDs ["pickup"]
	LUA_TABLE_BEGIN();
		LUA_PUSH_CONSTANT("health",          MI_PICKUP_HEALTH);     // VCS:466   LCS:558
		LUA_PUSH_CONSTANT("armour",          MI_PICKUP_BODYARMOUR); // VCS:468   LCS:587
		LUA_PUSH_CONSTANT("killfrenzy",      MI_PICKUP_KILLFRENZY); // VCS:482   LCS:584
		LUA_PUSH_CONSTANT("megadamage",      MI_PICKUP_MEGADAMAGE); // VCS:7384  LCS:3847
		LUA_PUSH_CONSTANT("regenhealth",     MI_PICKUP_REGENERATOR);// VCS:7385  LCS:3849
		LUA_PUSH_CONSTANT("invisible",       MI_PICKUP_INVISIBLE);  // VCS:7386  LCS:3850
		LUA_PUSH_CONSTANT("racegood",        MI_PICKUP_GOOD_CAR);   // VCS:7388  LCS:3999
		LUA_PUSH_CONSTANT("racebad",         MI_PICKUP_BAD_CAR);    // VCS:7387  LCS:3998
#ifndef GTA_LIBERTY
		LUA_PUSH_CONSTANT("flagball",        MI_PICKUP_BRIEFCASE); // 435
		LUA_PUSH_CONSTANT("briefcase",       MI_PICKUP_BRIEFCASE); // 435 // same
#endif
		LUA_PUSH_CONSTANT("bat",             MI_BASEBALL_BAT);
		LUA_PUSH_CONSTANT("chainsaw",        MI_CHAINSAW);
		LUA_PUSH_CONSTANT("grenade",         MI_GRENADE);
		LUA_PUSH_CONSTANT("det_grenade",     MI_BOMB);
		LUA_PUSH_CONSTANT("molotov",         MI_MOLOTOV);
		LUA_PUSH_CONSTANT("glock17",         MI_COLT45);
		LUA_PUSH_CONSTANT("shotgun",         MI_SHOTGUN);
		LUA_PUSH_CONSTANT("tec9",            MI_SKOR);
		LUA_PUSH_CONSTANT("ak47",            MI_AK47);
		LUA_PUSH_CONSTANT("rocket_launcher", MI_ROCKETLAUNCHER);
		LUA_PUSH_CONSTANT("sniper",          MI_SNIPERRIFLE);
	LUA_TABLE_END("pickup");


	// pickup icon IDs ["pickupicon"]
	LUA_TABLE_BEGIN();
		LUA_PUSH_CONSTANT("powerup",      RADAR_SPRITE_POWERUP);
		LUA_PUSH_CONSTANT("base",         RADAR_SPRITE_MP_BASE);
		LUA_PUSH_CONSTANT("checkpoint",   RADAR_SPRITE_MP_CHECKPOINT);
		LUA_PUSH_CONSTANT("player",       RADAR_SPRITE_MP_PLAYER);
		LUA_PUSH_CONSTANT("objective",    RADAR_SPRITE_MP_OBJECTIVE);
		LUA_PUSH_CONSTANT("car",          RADAR_SPRITE_MP_CAR);
		LUA_PUSH_CONSTANT("tank",         RADAR_SPRITE_MP_TANK);
		LUA_PUSH_CONSTANT("carlockup",    RADAR_SPRITE_MP_CARLOCKUP);
		LUA_PUSH_CONSTANT("targetplayer", RADAR_SPRITE_MP_TARGETPLAYER);
#ifndef GTA_LIBERTY
		LUA_PUSH_CONSTANT("briefcase",    RADAR_SPRITE_BCASE);
		LUA_PUSH_CONSTANT("bomb",         RADAR_SPRITE_MPBOMB);
		LUA_PUSH_CONSTANT("boat",         RADAR_SPRITE_BOAT);
		LUA_PUSH_CONSTANT("heli",         RADAR_SPRITE_HELI);
#endif
	LUA_TABLE_END("pickupicon");


	// waypoint type ["waypointtype"]
#ifndef GTA_LIBERTY
	LUA_TABLE_BEGIN();
		LUA_PUSH_CONSTANT("air",  WAYPOINT_AIR);
		LUA_PUSH_CONSTANT("land", WAYPOINT_LAND);
	LUA_TABLE_END("waypointtype");
#endif
}
