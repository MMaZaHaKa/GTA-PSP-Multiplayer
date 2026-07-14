/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"

#include "multiplayer/natives/public.h"
#include "multiplayer/net/packet.h"
#include "multiplayer/MultiGame.h"
#include "SpecialFX.h"

int32* ls_get_waypoint(lua_State* L, int32 ud) {
	return (int32*)luaL_checkudata(L, ud, "waypoint");
}

void register_waypoint_element(lua_State* L, uint16 id) {
	uint16* pHandle = (uint16*)lua_newuserdata(L, sizeof(uint16));
	*pHandle = id;
	luaL_getmetatable(L, "waypoint");
	lua_setmetatable(L, -2);
}

int mp_lsn_RemoveWaypoint(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	int32* pHandle = ls_get_waypoint(L, 1);
	if (!pHandle || *pHandle == -1) return 0;
	Game.m_WaypointManager.Remove(*pHandle);
	*pHandle = -1;
	return 0;
}

int mp_lsn_HasWaypointBeenHit(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	int32* pHandle = ls_get_waypoint(L, 1);
	if (!pHandle || *pHandle == -1) return 0;
	auto it = Game.m_WaypointManager.m_RbWaypointTree.find(*pHandle);
	lua_pushboolean(L, ((it != Game.m_WaypointManager.m_RbWaypointTree.end()) && it->second.second));
	return 1;
}

static const luaL_reg ls_waypoint_lib[] = {
	{"Remove",     mp_lsn_RemoveWaypoint},
	{"HasBeenHit", mp_lsn_HasWaypointBeenHit},
	{"__gc",       mp_lsn_RemoveWaypoint},
	{NULL, NULL}
};
VALIDATE_LUA_LIB(ls_waypoint_lib, (3 + 1), (3 + 1));

int mp_lsn_SetWaypoint(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	int32 nPlayerID = lsc_getPlayer(L, 1);
	CVector pos, direction, hitSize(7.0f, 7.0f, 7.0f);
	lsc_getVectorFromStack(pos, L, 2, true);
	lsc_getVectorFromStack(direction, L, 3, true);
	int32 nTop = lua_gettop(L);
	if (nTop >= 4) lsc_getVectorFromStackNoEntity(hitSize, L, 4);
	
	CRGBA sColor(0, 0, 0, 255);
	if (nTop >= 5) {
		sColor = lsc_getColour(L, 5);
		//sColor.red = lua_tonumber(L, 5);
		//sColor.green = lua_tonumber(L, 6);
		//sColor.blue = lua_tonumber(L, 7);
		//sColor.alpha = lua_tonumber(L, 8);
	}
	float fMarkerSize = (nTop >= 9) ? lua_tonumber(L, 9) : hitSize.x * 0.875f;
	float fArrowHeight = (nTop >= 10) ? lua_tonumber(L, 10) : 0.0f;
	bool bShowArrow = (nTop >= 11) ? lua_toboolean(L, 11) : true;
#ifndef GTA_LIBERTY
	int32 nType = (nTop >= 12) ? lua_tonumber(L, 12) : 0;
	uint16 nID = Game.m_WaypointManager.AddEntry(nPlayerID, pos, direction, hitSize, sColor, bShowArrow, nType, fMarkerSize, fArrowHeight);
#else
	uint16 nID = Game.m_WaypointManager.AddEntry(nPlayerID, pos, direction, hitSize, sColor, bShowArrow, fMarkerSize, fArrowHeight);
#endif
	register_waypoint_element(L, nID);
	return 1;
}

int mp_lsn_RaceArrowVisible(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	bool bRaceArrowVisible = lua_toboolean(L, 1);
	Game.m_WaypointManager.SetRaceArrowVisible(bRaceArrowVisible);
	return 0;
}

#ifndef GTA_LIBERTY
int mp_lsn_SetPointerArrow(lua_State* L) { // Vip Rip
	CVector pos;
	lsc_getVectorFromStack(pos, L, 1, true);
	cNavArrow::SetTarget(true, pos.x, pos.y, pos.z);
	return 0;
}

int mp_lsn_ClearPointerArrow(lua_State* L) {
	cNavArrow::ClearTarget();
	return 0;
}
#endif

static const luaL_reg ls_waypoints_lib[] = {
	{"SetWaypoint",         mp_lsn_SetWaypoint},
	{"HasWaypointBeenHit",  mp_lsn_HasWaypointBeenHit},
	{"RemoveWaypoint",      mp_lsn_RemoveWaypoint},
	{"RaceArrowVisible",    mp_lsn_RaceArrowVisible},
#ifndef GTA_LIBERTY
	{"SetPointerArrow",     mp_lsn_SetPointerArrow},
	{"ClearPointerArrow",   mp_lsn_ClearPointerArrow},
#endif
	{NULL, NULL}
};
VALIDATE_LUA_LIB(ls_waypoints_lib, (6 + 1), (4 + 1));


void lscript_open_waypoints() {
	cMultiGame& pGame = TheMPGame;
	cLWrapper& wrapper = cLWrapper::Instance();
	wrapper.CreateLibrary(ls_waypoint_lib, "waypoint");
	wrapper.CreateGlobalLibrary(ls_waypoints_lib, nil);
}