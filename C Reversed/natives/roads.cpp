/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"
#include "PathFind.h"
#include "World.h"
#include "Vehicle.h"
#include "CarCtrl.h"
#include "Fire.h"
#include "Explosion.h"

#include "multiplayer/natives/public.h"
#include "multiplayer/events/public.h"
#include "multiplayer/net/packet.h"
#include "multiplayer/MultiGame.h"

int mp_lsn_EnableRoads(lua_State* L) {
	cMultiGame& Game = TheMPGame;
	CVector vec_min, vec_max;
	lsc_getVectorFromStack(vec_min, L, 1, true);
	lsc_getVectorFromStack(vec_max, L, 2, true);
	bool toggle = lua_toboolean(L, 3);
	net::pckt_enable_roads packet{};
	packet.pckt_size = sizeof(net::pckt_enable_roads);
	packet.pckt_id = gtMP_PacketIDs.enable_roads.pckt_id;
	packet.posMin = vec_min;
	packet.posMax = vec_max;
	packet.toggle = toggle;
	on_recv_enable_roads(packet, 0, 0, false); // bug? true from local game?
	Game.SendMessagePriority(packet, BROADCAST_PEER_GROUPID);
	return 0;
}

int mp_lsn_checkObjectSpawnPosition(lua_State* L) {
	CVector pos;
	lsc_getVectorFromStack(pos, L, 1, true);
	float fRadius = lua_tonumber(L, 2);
//#if !defined(GTA_LIBERTY) && defined(FIX_BUGS)
//	if (pos.z == MAP_Z_LOW_LIMIT || pos.z <= MAP_Z_LOW_LIMIT_2)
//#else
	if (pos.z <= MAP_Z_LOW_LIMIT)
//#endif
		pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
	bool bIsFree = true;
	int16 nNumObjects;
	CEntity* aObjects[32];
	CWorld::FindObjectsInRange(pos, fRadius, true, &nNumObjects, 32, aObjects, false, true, false, false, false, true);
	for (int32 idx = 0; idx < nNumObjects; idx++) {
		CEntity* pEntity = aObjects[idx];
		if (pEntity->IsVehicle() || pEntity->bIsVehicle)
			bIsFree = false;
	}
	CWorld::FindObjectsInRange(pos, fRadius, true, &nNumObjects, 32, aObjects, false, false, true, false, false, true);
	for (int32 idx = 0; idx < nNumObjects; idx++) {
		CEntity* pEntity = aObjects[idx];
#ifdef FIX_BUGS
		if(pEntity && pEntity->IsMultiplayerPlayer())
#else
		if (pEntity->IsMultiplayerPlayer() && pEntity)
#endif
		{
			sElement* pElem = ((cPhysicalMG*)pEntity)->GetElement().element;
			if (pElem && (pElem->GetOwner() != cMultiGame::Instance().LocalPlayerID()))
				bIsFree = false;
		}
	}
	if (!bIsFree)
		debug("\nCheckObjectSpawnPosition : Position Blocked");
	lua_pushboolean(L, bIsFree);
	return 1;
}

int mp_lsn_clearArea(lua_State* L) {
	CVector pos;
	lsc_getVectorFromStack(pos, L, 1, true);
	float fRadius = lua_tonumber(L, 2);
	net::pckt_clear_area packet{};
	packet.pckt_size = sizeof(net::pckt_clear_area);
	packet.pckt_id = gtMP_PacketIDs.clear_area.pckt_id;
	packet.pos = pos;
	packet.radius = fRadius;
	on_recv_clear_area(packet, 0, 0, false); // bug? true from local game?
	TheMPGame.SendMessagePriority(packet, BROADCAST_PEER_GROUPID);
	return 0;
}

int mp_lsn_findNthNodeClosestToCoors(lua_State* L) {
	CVector pos;
	lsc_getVectorFromStack(pos, L, 1, true);
	int nMax = lua_tonumber(L, 2);
	int32 node = ThePaths.FindNthNodeClosestToCoors(pos, PATH_CAR, 1000.0f, false, true, nMax, false);
	CVector coors = ThePaths.FindNodeCoorsForScript(node);
	debug("\nFindNthNodeClosestToCoors, FROM == %f, %f, %f", pos.x, pos.y, pos.z);
	debug("\nFindNthNodeClosestToCoors, node == %f, %f, %f\n", coors.x, coors.y, coors.z);
	lsc_pushVuVector(L, coors);
	return 1;
}

int mp_lsn_setMsBeforeNextCreateCar(lua_State* L) {
	CCarCtrl::nMP_MsBeforeNextCreateCar = lua_tonumber(L, 1);
	return 0;
}

int mp_lsn_setMaxAmbientCars(lua_State* L) {
	CCarCtrl::SetMultiplayerAmbientCarLimit(lua_tonumber(L, 1));
	return 0;
}

int mp_lsn_generateAmbients(lua_State* L) {
	CCarCtrl::bMP_DisableAmbients = !lua_toboolean(L, 1);
	return 0;
}

int mp_lsn_scriptControlsMpCarLimit(lua_State* L) {
	CCarCtrl::ToggleScriptControlsMpCarLimit(lua_toboolean(L, 1));
	return 0;
}

int mp_lsn_extinguishAllFires(lua_State* L) {
	gFireManager.ExtinguishAll();
	return 0;
}

#ifndef GTA_LIBERTY
int mp_lsn_AddExplosion(lua_State* L) {
	CVector pos;
	lsc_getVectorFromStack(pos, L, 1, true);
	CExplosion::AddExplosion(nil, nil, EXPLOSION_GRENADE, pos, 0, true, true, false);
	return 0;
}

int mp_lsn_PointInActivityZone(lua_State* L) {
	CVector pos;
	lsc_getVectorFromStack(pos, L, 1, true);
	assert(TheMPGame.GetGameZoneInfo());
	lua_pushboolean(L, TheMPGame.GetGameZoneInfo()->IsPointInActivityZone(CVector2D(pos.x, pos.y)));
	return 1;
}
#endif

static const luaL_reg ls_roads_lib[] = {
	{"EnableRoads",               mp_lsn_EnableRoads},
	{"CheckObjectSpawnPosition",  mp_lsn_checkObjectSpawnPosition},
	{"ClearArea",                 mp_lsn_clearArea},
	{"FindNthNodeClosestToCoors", mp_lsn_findNthNodeClosestToCoors},
	{"SetMsBeforeNextCreateCar",  mp_lsn_setMsBeforeNextCreateCar},
	{"SetMaxAmbientCars",         mp_lsn_setMaxAmbientCars},
	{"GenerateAmbients",          mp_lsn_generateAmbients},
	{"ScriptControlsMpCarLimit",  mp_lsn_scriptControlsMpCarLimit},
	{"ExtinguishAllFires",        mp_lsn_extinguishAllFires},
#ifndef GTA_LIBERTY
	{"AddExplosion",              mp_lsn_AddExplosion},
	{"PointInActivityZone",       mp_lsn_PointInActivityZone},
#endif
	{NULL, NULL}
};
VALIDATE_LUA_LIB(ls_roads_lib, (11 + 1), (9 + 1));

void lscript_open_roads() {
	cMultiGame& Game = TheMPGame;
	cLWrapper& wrapper = cLWrapper::Instance();
#define REGISTER_PACKET(id, callback) Game.RegisterPacket(id, new cPacketDispatcher(callback));

	REGISTER_PACKET(gtMP_PacketIDs.enable_roads.pckt_id, &on_recv_enable_roads); // 46 lcs vcs
	REGISTER_PACKET(gtMP_PacketIDs.clear_area.pckt_id, &on_recv_clear_area); // 47 lcs vcs
	wrapper.CreateGlobalLibrary(ls_roads_lib, nil);
#undef REGISTER_PACKET
}