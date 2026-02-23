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
	cMultiGame& pGame = TheMPGame;
	CVector vec_min, vec_max;
	lsc_getVectorFromStack(vec_min, L, 1, true);
	lsc_getVectorFromStack(vec_max, L, 2, true);
	bool toggle = lua_toboolean(L, 3);
	net::pckt_enable_roads packet;
	packet.pckt_size = sizeof(net::pckt_enable_roads);
	packet.pckt_id = gtMP_PacketIDs.enable_roads.pckt_id;
	packet.posMin = vec_min;
	packet.posMax = vec_max;
	packet.toggle = toggle;
	on_recv_enable_roads(packet, 0, 0, false); // bug? true from local game?
	pGame.SendMessagePriority(packet, BROADCAST_PEER_GROUPID);
	return 0;
}

/* TODO: missing code */
int mp_lsn_checkObjectSpawnPosition(lua_State* L) {
	CVector pos;
	lsc_getVectorFromStack(pos, L, 1, true);
	float fRadius = lua_tonumber(L, 2);
	if (pos.z <= -100.0f) pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
	bool bIsFree = true;
	int16 nNumObjects;
	CEntity* aObjects[32];
	// TODO: implement check multiplayer in FindObjectsInRange
	CWorld::FindObjectsInRange(pos, fRadius, true, &nNumObjects, 32, aObjects, false, true, false, false, false, true);
	for (int idx = 0; idx < nNumObjects; idx++) {
		CEntity* pElement = aObjects[idx];
		if (pElement->GetType() == eEntityType::ENTITY_TYPE_VEHICLE) bIsFree = false;
		// TODO
	}
	CWorld::FindObjectsInRange(pos, fRadius, true, &nNumObjects, 32, aObjects, false, false, true, false, false, true);
	for (int idx = 0; idx < nNumObjects; idx++) {
		CEntity* pElement = aObjects[idx];
		// TODO
	}
	if (!bIsFree) debug("\nCheckObjectSpawnPosition : Position Blocked");
	lua_pushboolean(L, bIsFree);
	return 1;
}

int mp_lsn_clearArea(lua_State* L) {
	CVector pos;
	lsc_getVectorFromStack(pos, L, 1, true);
	float fRadius = lua_tonumber(L, 2);
	net::pckt_clear_area packet;
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
	// TODO: not sure paramters are correct
	int32 node = ThePaths.FindNthNodeClosestToCoors(pos, PATH_CAR, 1000.0f, false, true, nMax, false);
	CVector coors = ThePaths.FindNodeCoorsForScript(node);
	debug("\nFindNthNodeClosestToCoors, FROM == %f, %f, %f", pos.x, pos.y, pos.z);
	debug("\nFindNthNodeClosestToCoors, node == %f, %f, %f\n", coors.x, coors.y, coors.z);
	lsc_pushVuVector(L, coors);
	return 1;
}

int mp_lsn_setMsBeforeNextCreateCar(lua_State* L) {
	TODO(); // usage nMP_MsBeforeNextCreateCar in GenerateOneRandomCar
	MULTIGAME_UNIMPLEMENTED();
	CCarCtrl::nMP_MsBeforeNextCreateCar = lua_tonumber(L, 1);
	return 0;
}

int mp_lsn_setMaxAmbientCars(lua_State* L) {
	CCarCtrl::SetMultiplayerAmbientCarLimit(lua_tonumber(L, 1));
	return 0;
}

int mp_lsn_generateAmbients(lua_State* L) {
	TODO(); // usage bMP_DisableAmbients
	MULTIGAME_UNIMPLEMENTED();
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
	CExplosion::AddExplosion(nil, nil, EXPLOSION_GRENADE, pos, 0, true/*, true, 0*/); // todo 2 args
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