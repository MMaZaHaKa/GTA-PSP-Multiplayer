/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once
#include "multiplayer/MultiGame.h"
#include "multiplayer/net/packet.h"
#include "multiplayer/natives/public.h"

#include "common.h"
#include "World.h"
#include "Ped.h"
#include "PlayerPed.h"
#include "PlayerInfo.h"
#include "Vehicle.h"

void on_recv_enable_roads(net::pckt_enable_roads& packet, int sender, uint16 time, bool bFromRing) // ID 46
{
	RwV3d vec_min, vec_max;
	vec_min = packet.posMin;
	vec_max = packet.posMax;
	max_swap(vec_min.x, vec_max.x);
	max_swap(vec_min.y, vec_max.y);
	max_swap(vec_min.z, vec_max.z);
	ThePaths.SwitchRoadsOffInArea(vec_min.x, vec_max.x, vec_min.y, vec_max.y, vec_min.z, vec_max.z, packet.toggle);
}

void on_recv_clear_area(net::pckt_clear_area& packet, int sender, uint16 time, bool bFromRing) // ID 47
{
	CVector pos(packet.pos.x, packet.pos.y, packet.pos.z);
	if (pos.z <= MAP_Z_LOW_LIMIT) // VCS NO -250? lim2
		pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
	int16 nFound;
	CEntity* aObjects[16];
	CWorld::FindObjectsInRange(pos, packet.radius, true, &nFound, 16, aObjects, false, true, false, false, false, true);
	for (int32 idx = 0; idx < nFound; idx++) {
		CEntity* pObj = aObjects[idx];
		if (pObj->GetType() == eEntityType::ENTITY_TYPE_VEHICLE) {
			CVehicle* pVeh = (CVehicle*)pObj;
			if (pVeh->pDriver != (CPed*)FindPlayerPed()) {
				CWorld::Remove(pVeh);
				delete pVeh;
			}
		}
	}
}
