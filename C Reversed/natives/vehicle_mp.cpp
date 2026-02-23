/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"
#include "World.h"
#include "Zones.h"
#include "Streaming.h"
#include "CarCtrl.h"
#include "VehicleModelInfo.h"
#include "Bike.h"
#include "Heli.h"
#include "Automobile.h"
#ifndef GTA_LIBERTY
#include "Bmx.h"
#include "Boat.h"
#include "QuadBike.h"
#include "Plane.h"
#include "Heli.h"
#include "ColourTable.h"
#endif
#include "Radar.h"

#include "multiplayer/natives/public.h"
#include "multiplayer/events/public.h"
#include "multiplayer/elements/sAutomobile.h"
#include "multiplayer/elements/sBike.h"
#ifndef GTA_LIBERTY
#include "multiplayer/elements/sBmx.h"
#include "multiplayer/elements/sBoat.h"
#include "multiplayer/elements/sQuadBike.h"
#include "multiplayer/elements/sPlane.h"
#include "multiplayer/elements/sHeli.h"
#endif
#include "multiplayer/MultiGame.h"
#include "multiplayer/public.h"

static sVehicle* create_vehicle(int id, CVector& pos, float heading) {
	CStreaming::FlushRequestList();
	CStreaming::RequestModel(id, STREAMFLAGS_NOFADE | STREAMFLAGS_DEPENDENCY | STREAMFLAGS_SCRIPTOWNED);
	CStreaming::LoadAllRequestedModels(false);
	CVehicle* pVeh = nil;
	bool bIsCar = CModelInfo::IsCarModel(id);
	bool bIsBike = CModelInfo::IsBikeModel(id);
	bool bIsHeli = CModelInfo::IsHeliModel(id);
#ifndef GTA_LIBERTY
	bool bIsBmx = CModelInfo::IsBmxModel(id);
	bool bIsQuadBike = CModelInfo::IsQuadBikeModel(id);
	bool bIsPlane = CModelInfo::IsPlaneModel(id);
	bool bIsBoat = CModelInfo::IsBoatModel(id);
#endif

#ifdef GTA_LIBERTY
	if (bIsBike) {
		pVeh = new CBike(id, MISSION_VEHICLE);
		((CBike*)pVeh)->bIsStanding = true;
	}
	else if (bIsHeli)
		pVeh = new CAutomobile(id, MISSION_VEHICLE);
	else //if (bIsCar)
		pVeh = new CAutomobile(id, MISSION_VEHICLE);
#else
	if (bIsBike) {
		pVeh = new CBike(id, MISSION_VEHICLE);
		((CBike*)pVeh)->bIsStanding = true;
	}
	else if (bIsBmx) {
		pVeh = new CBmx(id, MISSION_VEHICLE);
		((CBmx*)pVeh)->bIsStanding = true;
	}
	else if (bIsQuadBike)
		pVeh = new CQuadBike(id, MISSION_VEHICLE);
	else if (bIsHeli)
		pVeh = new CHeli(id, MISSION_VEHICLE);
	else if (bIsPlane)
		pVeh = new CPlane(id, MISSION_VEHICLE);
	else if (bIsBoat)
		pVeh = new CBoat(id, MISSION_VEHICLE);
	else if (bIsCar)
		pVeh = new CAutomobile(id, MISSION_VEHICLE);
#endif
	assert(pVeh);

	CVector vPos = pos;
	if (vPos.z <= MAP_Z_LOW_LIMIT)
		vPos.z = CWorld::FindGroundZForCoord(vPos.x, vPos.y);
	vPos.z += pVeh->GetDistanceFromCentreOfMassToBaseOfModel();
	pVeh->SetPosition(vPos);
	pVeh->SetHeading(DEGTORAD(heading));
	// TODO(MP): code seems from COMMAND_CREATE_CAR, but we have to confirm all the fields
	TODO();
	TODO();
	TODO();
	TODO();
	pVeh->SetStatus(STATUS_ABANDONED);
	pVeh->bIsLocked = true;
	CCarCtrl::JoinCarWithRoadSystem(pVeh);
	pVeh->AutoPilot.m_nCarMission = MISSION_NONE;
	pVeh->AutoPilot.m_nTempAction = TEMPACT_NONE;
	pVeh->AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_STOP_FOR_CARS;
	pVeh->AutoPilot.m_nCruiseSpeed = pVeh->AutoPilot.m_fMaxTrafficSpeed = 9.0f;
	pVeh->AutoPilot.m_nCurrentLane = pVeh->AutoPilot.m_nNextLane = 0;
	pVeh->bEngineOn = false;
	pVeh->m_nZoneLevel = CTheZones::GetLevelFromPosition(&vPos);
	pVeh->bHasBeenOwnedByPlayer = true;
	pVeh->m_nDoorLock = CARLOCK_UNLOCKED;
	CWorld::Add(pVeh);
#ifdef GTA_LIBERTY
	if (bIsBike)
		return new sBike((CBike*)pVeh);
	else
		return new sAutomobile((CAutomobile*)pVeh);
#else
	if (bIsCar)
		return new sAutomobile((CAutomobile*)pVeh);
	if (bIsBike)
		return new sBike((CBike*)pVeh);
	if (bIsBmx)
		return new sBmx((CBmx*)pVeh);
	if (bIsQuadBike)
		return new sQuadBike((CQuadBike*)pVeh);
	if (bIsHeli)
		return new sHeli((CHeli*)pVeh);
	if (bIsPlane)
		return new sPlane((CPlane*)pVeh);
	if (bIsBoat)
		return new sBoat((CBoat*)pVeh);
#endif
	return nil;
}

static int mp_lsn_CreateVehicle(lua_State* L) {
	int32 nVehID = luaL_checknumber(L, 1);
	CVector pos;
	lsc_getVectorFromStack(pos, L, 2, true);
	debug("mp_lsn_CreateVehicle at %f %f %f\n", pos.x, pos.y, pos.z);
	float fHeading = luaL_checknumber(L, 3);
	sVehicle* pVeh = create_vehicle(nVehID, pos, fHeading);
	lsc_register_entity(L, pVeh);
	if (lua_isnumber(L, 4)) {
#ifdef GTA_LIBERTY
		pVeh->m_currentColour1 = lua_tonumber(L, 4);
		if (lua_isnumber(L, 5))
			pVeh->m_currentColour2 = lua_tonumber(L, 5);
#else
		uint32 currentColour1 = lua_tonumber(L, 4);
		pVeh->m_aColours[VEHICLE_COLOUR_PRIMARY] = CRGBA_UNPACK(currentColour1);
		if (lua_isnumber(L, 5)) {
			uint32 currentColour2 = lua_tonumber(L, 5);
			pVeh->m_aColours[VEHICLE_COLOUR_SECONDARY] = CRGBA_UNPACK(currentColour2);
		}
#endif
	}
	else {
		CVehicleModelInfo* pModelInfo = (CVehicleModelInfo*)CModelInfo::GetModelInfo(nVehID);
#ifdef GTA_LIBERTY
		pVeh->m_currentColour1 = pModelInfo->m_currentColour1;
		pVeh->m_currentColour2 = pModelInfo->m_currentColour2;
#else
		int32 n = CGeneral::GetRandomNumberInRange(0, pModelInfo->m_nNumColourVariations);
		pVeh->m_aColours[VEHICLE_COLOUR_PRIMARY] = pModelInfo->GetPrimaryColourForVariation(n);
		pVeh->m_aColours[VEHICLE_COLOUR_SECONDARY] = pModelInfo->GetSecondaryColourForVariation(n);
#endif
	}
	if (lua_isboolean(L, 6) && lua_toboolean(L, 6))
	{
		pVeh->GetPhysical()->b154_4 = true;
		((CVehicle*)pVeh->GetEntity())->b154_4 = true;
	}
	return 1;
}

static int mp_lsn_VehicleAdd3dMarker(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	sVehicle* pVeh = (sVehicle*)lsc_get_entity(L, 1);
	if (pVeh == nil) {
		debug("Invalid car sent to VehicleAdd3dMarker\n");
		lua_pushboolean(L, false);
		return 1;
	}
	bool bWasAdded = false;
	if (pVeh->GetEntity() != nil) {
		net::pckt_add_3d_marker packet{};
		packet.pckt_size = sizeof(net::pckt_add_3d_marker);
		packet.pckt_id = gtMP_PacketIDs.add_3d_marker.pckt_id;
		packet.owner = pVeh->GetOwner();
		packet.elem = pVeh->GetID();
		debug("VehicleAdd3DMarker: %d %d\n", packet.owner, packet.pckt_id);
		Game.SendMessagePriority(packet, BROADCAST_PEER_GROUPID);
		if (packet.owner == Game.LocalPlayerID())
			on_recv_add_3d_marker(packet, packet.owner, 0, true);
		bWasAdded = true;
	}
	lua_pushboolean(L, bWasAdded);
	return 1;
}

static int mp_lsn_VehicleRemove3dMarker(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	sVehicle* pVeh = (sVehicle*)lsc_get_entity(L, 1);
	bool bWasRemoved = false;
	if (pVeh != nil && pVeh->GetEntity()) {
		net::pckt_remove_3d_marker packet{};
		packet.pckt_size = sizeof(net::pckt_remove_3d_marker);
		packet.pckt_id = gtMP_PacketIDs.remove_3d_marker.pckt_id;
		packet.owner = pVeh->GetOwner();
		packet.elem = pVeh->GetID();
		Game.SendMessagePriority(packet, BROADCAST_PEER_GROUPID);
		if (packet.owner == Game.LocalPlayerID())
			on_recv_remove_3d_marker(packet, packet.owner, 0, true);
		bWasRemoved = true;
	}
	lua_pushboolean(L, bWasRemoved);
	return 1;
}

static int mp_lsn_VehiclePosition(lua_State* L) {
	sVehicle* pVeh = (sVehicle*)lsc_get_entity(L, 1);
	if (pVeh == nil)
		return 0;
	CVector pos = pVeh->GetPhysical()->GetPosition();
	lsc_pushVuVector(L, pos);
	return 1;
}

static int mp_lsn_VehicleHealth(lua_State* L) {
	sVehicle* pVeh = (sVehicle*)lsc_get_entity(L, 1);
	if (pVeh == nil) {
		lua_pushnumber(L, -1.0f);
		return 1;
	}
	sVehicleSync* pSync = pVeh->GetSync().vehicle;
	float fHealth = pSync->m_fHealth;
	if (pSync->m_status == eEntityStatus::STATUS_WRECKED)
		fHealth = 0.0f;
	lua_pushnumber(L, fHealth);
	return 1;
}

static int mp_lsn_VehicleSetHealth(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	sVehicle* pVeh = (sVehicle*)lsc_get_entity(L, 1);
	const float fHealth = lua_tonumber(L, 2);
	if (pVeh == nil)
		return 0;
	sVehicleSync* pSync = pVeh->GetSync().vehicle;
	if (pSync->m_fHealth <= 0.0f)
		return 0;
	net::pckt_set_vehicle_health packet{};
	packet.pckt_size = sizeof(net::pckt_set_vehicle_health);
	packet.pckt_id = gtMP_PacketIDs.set_vehicle_health.pckt_id;
	packet.owner = pVeh->GetOwner();
	packet.elem = pVeh->GetID();
	packet.health = fHealth;
	if (packet.owner == Game.LocalPlayerID())
		on_recv_set_vehicle_health(packet, packet.owner, 0, true);
	else
		Game.SendMessagePriority(packet, packet.owner);
	return 0;
}

static int mp_lsn_VehicleSetPosition(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	sVehicle* pVehMG = (sVehicle*)lsc_get_entity(L, 1);
	CVector vPos;
	lsc_getVectorFromStack(vPos, L, 2, true);
	net::pckt_set_vehicle_position packet{};
	packet.pckt_size = sizeof(net::pckt_set_vehicle_position);
	packet.pckt_id = gtMP_PacketIDs.set_vehicle_position.pckt_id;
	packet.owner = pVehMG->GetOwner();
	packet.elem = pVehMG->GetID();
	packet.pos = vPos;
	if (pVehMG->GetOwner() != Game.LocalPlayerID())
		Game.SendMessagePriority(packet, packet.owner);
	else
		on_recv_set_vehicle_position(packet, packet.owner, 0, true);
	return 0;
}

#ifndef GTA_LIBERTY
static int mp_lsn_BlowupVehicle(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	sVehicle* pVehMG = (sVehicle*)lsc_get_entity(L, 1);
	bool isNumber = lua_isnumber(L, 2);
	int nPlayerID = lua_tonumber(L, 2);
	if (pVehMG == nil /*|| !isNumber*/) return 0;
	net::pckt_msg_blowup_vehicle packet{};
	packet.pckt_size = sizeof(net::pckt_msg_blowup_vehicle);
	packet.pckt_id = gtMP_PacketIDs.msg_blowup_vehicle.pckt_id;
	packet.owner = pVehMG->GetOwner();
	packet.elem = pVehMG->GetID();
	packet.player_id = nPlayerID;
	if (pVehMG->GetOwner() != Game.LocalPlayerID())
		Game.SendMessagePriority(packet, packet.owner);
	else
		on_recv_msg_blowup_vehicle(packet, packet.owner, 0, true);
	return 0;
}
#endif

static int mp_lsn_VehicleGetDriverTeam(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	sVehicle* pVeh = (sVehicle*)lsc_get_entity(L, 1);
	if (pVeh == nil) return 0;
	sVehicleSync* pSync = pVeh->GetSync().vehicle;
	int8 nTeamID = pSync->m_nDriverTeam;
	if (nTeamID == -1 && pSync->m_nDriverID >= 0)
		nTeamID = Game.GetPlayerTeamID(pSync->m_nDriverID);
	lua_pushnumber(L, nTeamID);
	return 1;
}

static int mp_lsn_VehicleGetDriverName(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	sVehicle* pVeh = (sVehicle*)lsc_get_entity(L, 1);
	if (pVeh == nil) return 0;
	sVehicleSync* pSync = pVeh->GetSync().vehicle;
	const char* pName = (pSync->m_nDriverTeam == -1) ? "^T^MPEMPTY" : Game.GetPlayerName(pVeh->GetOwner());
	lua_pushstring(L, pName);
	return 1;
}

static int mp_lsn_VehicleIsDrivenByPlayer(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	sVehicle* pVeh = (sVehicle*)lsc_get_entity(L, 1);
	if (pVeh == nil) return 0;
	bool bIsDrivenByPlayer = pVeh->GetSync().vehicle->m_bIsDrivenByPlayer;
	lua_pushboolean(L, bIsDrivenByPlayer);
	return 1;
}

static int mp_lsn_VehicleWasRecentlyDrivenByPlayer(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	sVehicle* pVeh = (sVehicle*)lsc_get_entity(L, 1);
	if (pVeh == nil) return 0;
	bool bWasDriven = pVeh->GetSync().vehicle->m_nDriverID >= 0;
	lua_pushboolean(L, bWasDriven);
	return 1;
}

static int mp_lsn_VehicleForceOutPeds(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	sVehicle* pVehMG = (sVehicle*)lsc_get_entity(L, 1);
	if (pVehMG == nil) return 0;
	net::pckt_force_ped_from_vehicle packet{};
	packet.pckt_size = sizeof(net::pckt_force_ped_from_vehicle);
	packet.pckt_id = gtMP_PacketIDs.force_ped_from_vehicle.pckt_id;
	packet.owner = pVehMG->GetOwner();
	packet.elem = pVehMG->GetID();
	if (pVehMG->GetOwner() != Game.LocalPlayerID())
		Game.SendMessagePriority(packet, packet.owner);
	else
		on_recv_force_ped_from_vehicle(packet, packet.owner, 0, true); // inlined
	return 0;
}

static int mp_lsn_VehicleSetEmergencyStop(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	sVehicle* pVehMG = (sVehicle*)lsc_get_entity(L, 1);
	if (pVehMG == nil || !lua_isboolean(L, 2)) return 0;
	net::pckt_set_vehicle_emergency_break_state packet{};
	packet.pckt_size = sizeof(net::pckt_set_vehicle_emergency_break_state);
	packet.pckt_id = gtMP_PacketIDs.set_vehicle_emergency_break_state.pckt_id;
	packet.owner = pVehMG->GetOwner();
	packet.elem = pVehMG->GetID();
	packet.enabled = lua_toboolean(L, 2);
	if (pVehMG->GetOwner() != Game.LocalPlayerID())
		Game.SendMessagePriority(packet, packet.owner);
	else
		on_recv_set_vehicle_emergency_break_state(packet, packet.owner, 0, true);
	return 0;
}

static int mp_lsn_VehicleSetCarDoorLocks(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	sVehicle* pVehMG = (sVehicle*)lsc_get_entity(L, 1);
	if (pVehMG == nil || !lua_isnumber(L, -1)) return 0;
	net::pckt_set_carlocked_state packet{};
	packet.pckt_size = sizeof(net::pckt_set_carlocked_state);
	packet.pckt_id = gtMP_PacketIDs.set_carlocked_state.pckt_id;
	packet.owner = pVehMG->GetOwner();
	packet.elem = pVehMG->GetID();
	packet.state = lua_tonumber(L, -1);
	if (pVehMG->GetOwner() != Game.LocalPlayerID())
		Game.SendMessagePriority(packet, packet.owner);
	else
		on_recv_set_carlocked_state(packet, packet.owner, 0, true);
	return 0;
}

static int mp_lsn_VehicleSetTyresNoBurst(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	sVehicle* pVehMG = (sVehicle*)lsc_get_entity(L, 1);
	if (pVehMG == nil || !lua_isboolean(L, -1)) return 0;
	net::pckt_set_tyres_no_burst packet{};
	packet.pckt_size = sizeof(net::pckt_set_tyres_no_burst);
	packet.pckt_id = gtMP_PacketIDs.set_tyres_no_burst.pckt_id;
	packet.owner = pVehMG->GetOwner();
	packet.elem = pVehMG->GetID();
	packet.enabled = lua_toboolean(L, -1);
	if (pVehMG->GetOwner() != Game.LocalPlayerID())
		Game.SendMessagePriority(packet, packet.owner);
	else
		on_recv_set_tyres_no_burst(packet, packet.owner, 0, true);
	return 0;
}

static int mp_lsn_VehicleSetColours(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	sVehicle* pVehMG = (sVehicle*)lsc_get_entity(L, 1);
	if (pVehMG == nil || !lua_isnumber(L, -1) || !lua_isnumber(L, -2)) return 0;
	if (pVehMG->GetOwner() != Game.LocalPlayerID()) return 0;
	const uint32 nColorA = lua_tonumber(L, 2);
	const uint32 nColorB = lua_tonumber(L, 3);
#ifdef GTA_LIBERTY
	if (nColorA <= 0xff) pVehMG->m_currentColour1 = nColorA;
	if (nColorB <= 0xff) pVehMG->m_currentColour2 = nColorB;
#else
	pVehMG->m_aColours[VEHICLE_COLOUR_PRIMARY] = CRGBA_UNPACK(nColorA);
	pVehMG->m_aColours[VEHICLE_COLOUR_SECONDARY] = CRGBA_UNPACK(nColorB);
#endif
	return 0;
}

#ifndef GTA_LIBERTY
static int mp_lsn_GetVehicleColour(lua_State* L) {
	uint32 nColorIdx = lua_tonumber(L, 1);
	CRGBA col = RGBTable->GetCarCol(nColorIdx);
	lua_pushnumber(L, CRGBA_PACK(col.r, col.g, col.b, col.a));
	return 1;
}
#endif

static int mp_lsn_VehicleRepair(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	sVehicle* pVeh = (sVehicle*)lsc_get_entity(L, 1);
	if (pVeh == nil) return 0;
	net::pckt_repair_car packet{};
	packet.pckt_size = sizeof(net::pckt_repair_car);
	packet.pckt_id = gtMP_PacketIDs.repair_car.pckt_id;
	packet.owner = pVeh->GetOwner();
	packet.elem = pVeh->GetID();
	if (packet.owner == Game.LocalPlayerID())
		on_recv_repair_car(packet, packet.owner, 0, true);
	else
		Game.SendMessagePriority(packet, packet.owner);
	return 0;
}

static int mp_lsn_DeleteVehicle(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	sVehicle* pVeh = (sVehicle*)lsc_get_entity(L, 1);
	if (pVeh == nil)
		return 0;
	net::pckt_delete_vehicle packet{};
	packet.pckt_size = sizeof(net::pckt_delete_vehicle);
	packet.pckt_id = gtMP_PacketIDs.delete_vehicle.pckt_id;
	packet.owner = pVeh->GetOwner();
	packet.elem = pVeh->GetID();
	if (packet.owner == Game.LocalPlayerID())
		on_recv_delete_vehicle(packet, packet.owner, 0, true);
	else
		Game.SendMessagePriority(packet, packet.owner);
	return 0;
}

static int mp_lsn_IsVehicleWrecked(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	sVehicle* pVeh = (sVehicle*)lsc_get_entity(L, 1);
	bool bIsWrecked = true;
	if (pVeh && pVeh->GetSync().vehicle->m_status != STATUS_WRECKED)
		bIsWrecked = pVeh->GetSync().vehicle->m_bIsWreked;
	lua_pushboolean(L, bIsWrecked);
	return 1;
}

static int mp_lsn_VehicleGetDriverPlayerId(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	sVehicle* pVeh = (sVehicle*)lsc_get_entity(L, 1);
	if (pVeh == nil) return 0;
	int16 nPlayerID = (pVeh->GetSync().vehicle->m_bHasDriver) ? pVeh->GetOwner() : -1;
	lua_pushnumber(L, nPlayerID);
	return 1;
}

static int mp_lsn_VehicleGetDriverPlayerColour(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	sVehicle* pVeh = (sVehicle*)lsc_get_entity(L, 1);
	if (pVeh == nil) return 0;
	uint32 nColor = (uint32)-1;
	if (pVeh->GetSync().vehicle->m_bHasDriver) {
		CRGBA* pColor = Game.GetPlayerColor(pVeh->GetOwner());
		nColor = RGB24_PACK(pColor->red, pColor->green, pColor->blue);
	}
	lua_pushnumber(L, nColor);
	return 1;
}

static int mp_lsn_VehicleGetLastDamageAmount(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	sVehicle* pVeh = (sVehicle*)lsc_get_entity(L, 1);
	float fDamangeAmount = -1.0f;
	if (pVeh != nil)
		fDamangeAmount = pVeh->GetSync().vehicle->m_fLastDamageAmount;
	lua_pushnumber(L, fDamangeAmount);
	return 1;
}

static int mp_lsn_VehicleGetLastDamagePlayerID(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	sVehicle* pVeh = (sVehicle*)lsc_get_entity(L, 1);
	int nPlayerID = -1;
	if (pVeh != nil)
		nPlayerID = pVeh->GetSync().vehicle->m_nLastDamagePlayerID;
	lua_pushnumber(L, nPlayerID);
	return 1;
}

static int mp_lsn_VehicleClearLastDamagePlayerID(lua_State* L) {
	sVehicle* pVeh = (sVehicle*)lsc_get_entity(L, 1);
	if (pVeh && pVeh->GetEntity())
		((CVehicle*)pVeh->GetEntity())->m_nDamagedByPeerID = -1;
	return 0;
}

static int mp_lsn_VehicleAdjustSpeed(lua_State* L) {
	sVehicle* pVehMG = (sVehicle*)lsc_get_entity(L, 1);
	float fAdjust = lua_tonumber(L, 2);
	if (pVehMG == nil) return 0;
	CVehicle* pVeh = (CVehicle*)pVehMG->GetEntity();
	if (pVeh != nil)
		pVeh->m_vecMoveSpeed *= fAdjust;
	return 0;
}

static int mp_lsn_LimitTankSpeed(lua_State* L) {
	sVehicle* pVehMG = (sVehicle*)lsc_get_entity(L, 1);
	if (pVehMG == nil) return 0;
	CVehicle* pVeh = (CVehicle*)pVehMG->GetEntity();
	if (pVeh != nil) {
		const float fSpeed = pVeh->m_vecMoveSpeed.Magnitude();
		if (fSpeed > 3.5f)
			pVeh->m_vecMoveSpeed *= 0.925f; // TODO(MP): does that make sense?
	}
	return 0;
}

static int mp_lsn_LimitVehicleSpeed(lua_State* L) {
	sVehicle* pVehMG = (sVehicle*)lsc_get_entity(L, 1);
	if (pVehMG == nil) return 0;
	CVehicle* pVeh = (CVehicle*)pVehMG->GetEntity();
	float fMaxMag = lua_tonumber(L, 2);
	float fCapMod = lua_tonumber(L, 3);
	if (pVeh != nil) {
		const float fSpeed = pVeh->m_vecMoveSpeed.Magnitude();
		if (fSpeed > fMaxMag)
			pVeh->m_vecMoveSpeed *= fCapMod; // TODO(MP): does that make sense?
	}
	return 0;
}

static int mp_lsn_VehicleGetModelId(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	sVehicle* pVeh = (sVehicle*)lsc_get_entity(L, 1);
	if (pVeh == nil) {
		lua_pushnumber(L, 0);
		return 1;
	}
	CEntity* pEnt = (pVeh->GetOwner() != Game.LocalPlayerID()) ? pVeh->GetPhysical() : pVeh->GetEntity();
	lua_pushnumber(L, pEnt->GetModelIndex());
	return 1;
}

#ifndef GTA_LIBERTY
static int mp_lsn_IsModelIdHeli(lua_State* L) {
	bool bIsModelIdHeli = false;
	if (lua_isnumber(L, 1)) {
		CVehicleModelInfo* mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(lua_tonumber(L, 1));
		assert(mi);
		if (mi->IsHeli()) bIsModelIdHeli = true;
	}
	lua_pushboolean(L, bIsModelIdHeli);
	return 1;
}
#endif


static const luaL_reg ls_vehicle_lib[] = {
	{"CreateVehicle",                    mp_lsn_CreateVehicle},
	{"VehicleAdd3dMarker",               mp_lsn_VehicleAdd3dMarker},
	{"VehicleRemove3dMarker",            mp_lsn_VehicleRemove3dMarker},
	{"VehiclePosition",                  mp_lsn_VehiclePosition},
	{"VehicleHealth",                    mp_lsn_VehicleHealth},
	{"VehicleSetHealth",                 mp_lsn_VehicleSetHealth},
	{"VehicleSetPosition",               mp_lsn_VehicleSetPosition},
#ifndef GTA_LIBERTY
	{"BlowupVehicle",                    mp_lsn_BlowupVehicle},
#endif
	{"VehicleGetDriverTeam",             mp_lsn_VehicleGetDriverTeam},
	{"VehicleGetDriverName",             mp_lsn_VehicleGetDriverName},
	{"VehicleIsDrivenByPlayer",          mp_lsn_VehicleIsDrivenByPlayer},
	{"VehicleWasRecentlyDrivenByPlayer", mp_lsn_VehicleWasRecentlyDrivenByPlayer},
	{"VehicleForceOutPeds",              mp_lsn_VehicleForceOutPeds},
	{"VehicleSetEmergencyStop",          mp_lsn_VehicleSetEmergencyStop},
	{"VehicleSetCarDoorLocks",           mp_lsn_VehicleSetCarDoorLocks},
	{"VehicleSetTyresNoBurst",           mp_lsn_VehicleSetTyresNoBurst},
	{"VehicleSetColours",                mp_lsn_VehicleSetColours},
#ifndef GTA_LIBERTY
	{"GetVehicleColour",                 mp_lsn_GetVehicleColour},
#endif
	{"VehicleRepair",                    mp_lsn_VehicleRepair},
	{"DeleteVehicle",                    mp_lsn_DeleteVehicle},
	{"IsVehicleWrecked",                 mp_lsn_IsVehicleWrecked},
	{"VehicleGetDriverPlayerId",         mp_lsn_VehicleGetDriverPlayerId},
	{"VehicleGetDriverPlayerColour",     mp_lsn_VehicleGetDriverPlayerColour},
	{"VehicleGetLastDamageAmount",       mp_lsn_VehicleGetLastDamageAmount},
	{"VehicleGetLastDamagePlayerID",     mp_lsn_VehicleGetLastDamagePlayerID},
	{"VehicleClearLastDamagePlayerID",   mp_lsn_VehicleClearLastDamagePlayerID},
	{"VehicleAdjustSpeed",               mp_lsn_VehicleAdjustSpeed},
	{"LimitTankSpeed",                   mp_lsn_LimitTankSpeed},
	{"LimitVehicleSpeed",                mp_lsn_LimitVehicleSpeed},
	{"VehicleGetModelId",                mp_lsn_VehicleGetModelId},
#ifndef GTA_LIBERTY
	{"IsModelIdHeli",                    mp_lsn_IsModelIdHeli},
#endif
	{NULL, NULL}
};
VALIDATE_LUA_LIB(ls_vehicle_lib, (31 + 1), (28 + 1));

inline void open_vehicle_constants(lua_State* L) {
	lua_pushnumber(L, 1);
	lua_setglobal(L, "CARLOCK_UNLOCKED");
	lua_pushnumber(L, 2);
	lua_setglobal(L, "CARLOCK_LOCKED");
	lua_pushnumber(L, 7);
	lua_setglobal(L, "CARLOCK_TEAM1_LOCKED");
	lua_pushnumber(L, 8);
	lua_setglobal(L, "CARLOCK_TEAM2_LOCKED");
}

void lscript_open_vehicle() {
	cLWrapper& wrapper = cLWrapper::Instance();
	open_vehicle_constants(wrapper.m_luaVM);
	wrapper.CreateGlobalLibrary(ls_vehicle_lib, nil);
}