/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once
#include "multiplayer/MultiGame.h"
#include "multiplayer/net/packet.h"

#include "multiplayer/elements/sVehicle.h"
#include "multiplayer/elements/sAutomobile.h"
#include "multiplayer/elements/sBike.h"
#include "multiplayer/events/public.h"
#ifndef GTA_LIBERTY
#include "multiplayer/elements/sBmx.h"
#include "multiplayer/elements/sBoat.h"
#include "multiplayer/elements/sQuadBike.h"
#include "multiplayer/elements/sPlane.h"
#include "multiplayer/elements/sHeli.h"
#include "Boat.h"
#endif

#include "common.h"
#include "World.h"
#include "Vehicle.h"
#include "Automobile.h"
#include "Bike.h"

void on_recv_force_ped_from_vehicle(net::pckt_force_ped_from_vehicle& packet, int sender, uint16 time, bool bFromRing) // ID 8
{
	cMultiGame& Game = cMultiGame::Instance();
	if (packet.owner != Game.LocalPlayerID()) return;
	sVehicle* pVehMG = (sVehicle*)Game.GetEntityForHandle(packet.owner, packet.elem);
	if (pVehMG == nil) return;
	CVehicle* pVeh = (CVehicle*)pVehMG->GetEntity();
	if(pVeh && pVeh->pDriver)
		pVeh->pDriver->SetExitCar(pVeh, 0);
#ifdef FIX_BUGS
	if (pVeh == nil) return;
	for (int32 nIndex = 0; nIndex < pVeh->m_nNumMaxPassengers; nIndex++) {
#else
	for (int32 nIndex = 0; nIndex < ARRAY_SIZE(pVeh->pPassengers); nIndex++) {
#endif
		if(pVeh->GetPassenger(nIndex))
			pVeh->GetPassenger(nIndex)->SetExitCar(pVeh, 0);
	}
}

void on_recv_set_vehicle_emergency_break_state(net::pckt_set_vehicle_emergency_break_state& packet, int sender, uint16 time, bool bFromRing) // ID 9
{
	cMultiGame& Game = cMultiGame::Instance();
	if (packet.owner != Game.LocalPlayerID()) return;
	sVehicle* pVehMG = (sVehicle*)Game.GetEntityForHandle(packet.owner, packet.elem);
	if (pVehMG == nil) return;
	CAutomobile* pVeh = (CAutomobile*)pVehMG->GetEntity();
	if (pVeh == nil) return;

	if (packet.enabled) // or pVeh->m_bSuperBrake = packet.enabled; probably orig if else
		pVeh->m_bSuperBrake = true;
	else
		pVeh->m_bSuperBrake = false;
}

void on_recv_set_carlocked_state(net::pckt_set_carlocked_state& packet, int sender, uint16 time, bool bFromRing) // ID 10
{
	cMultiGame& Game = cMultiGame::Instance();
	if (packet.owner != Game.LocalPlayerID()) return;
	sVehicle* pVehMG = (sVehicle*)Game.GetEntityForHandle(packet.owner, packet.elem);
	if (pVehMG == nil) return;
	CVehicle* pVeh = (CVehicle*)pVehMG->GetEntity();
	if (pVeh) pVeh->m_nDoorLock = (eCarLock)packet.state;
}

void on_recv_repair_car(net::pckt_repair_car& packet, int sender, uint16 time, bool bFromRing) // ID 11
{
	cMultiGame& Game = cMultiGame::Instance();
	if (packet.owner == Game.LocalPlayerID()) // has native Entity
	{
		sVehicle* pVehMG = (sVehicle*)Game.GetEntityForHandle(packet.owner, packet.elem);
		if (pVehMG == nil) return;
		CVehicle* pVeh = (CVehicle*)pVehMG->GetEntity();
		if (pVeh == nil) return;
#if !defined(GTA_LIBERTY) && defined(FIX_BUGS)
		if (pVeh->IsBike() || pVeh->IsBmx()) // bmx subtype
#else
		if (pVeh->IsBike())
#endif
		{
			CBike* pBike = (CBike*)pVeh;
			pBike->m_fFireBlowUpTimer = 0.0f;
			pBike->Fix();
			sBike* pBikeMG = Game.GetElementFromEntity<sBike*>(pBike);
			if (pBikeMG != nil) pBikeMG->Fix();
		}
#if !defined(GTA_LIBERTY) && defined(FIX_BUGS)
		else if (pVeh->IsBoat())
		{
			CBoat* pBoat = (CBoat*)pVeh;
		}
#endif
		else // TODO FIX_BUGS + other types vehicles + recheck mp_lsn_RepairPlayersVehicle
		{
			assert(pVeh->IsCar());
			CAutomobile* pAutomobile = (CAutomobile*)pVeh;
			pAutomobile->m_fFireBlowUpTimer = 0.0f;
			pAutomobile->Fix();
			pAutomobile->CloseAllDoors();
			pAutomobile->CloseBoot();
			pAutomobile->CloseBonnet();
			sAutomobile* pElemAutomobile = Game.GetElementFromEntity<sAutomobile*>(pAutomobile);
			if (pElemAutomobile != nil) pElemAutomobile->Fix();
		}

#ifdef GTA_LIBERTY
		pVeh->m_fHealth = 1000.0f;
#else
		pVeh->m_fHealth = pVeh->m_fMaxHealth;
#endif
		pVeh->m_nDoorLock = eCarLock::CARLOCK_UNLOCKED;
	}
	else
	{
		sVehicle* pVehMG = (sVehicle*)Game.GetEntityForHandle(packet.owner, packet.elem);
		if (pVehMG == nil) return;
		if (pVehMG->GetType() == eElementType::ELEMENT_TYPE_AUTOMOBILE)
			((sAutomobile*)pVehMG)->Fix();
		else if (pVehMG->GetType() == eElementType::ELEMENT_TYPE_BIKE)
			((sBike*)pVehMG)->Fix();
#if defined(FIX_BUGS) && !defined(GTA_LIBERTY)
		else if (pVehMG->GetType() == eElementType::ELEMENT_TYPE_BMX)
			((sBmx*)pVehMG)->Fix();
		else if (pVehMG->GetType() == eElementType::ELEMENT_TYPE_BOAT)
			((sBoat*)pVehMG)->Fix();
		else if (pVehMG->GetType() == eElementType::ELEMENT_TYPE_PLANE)
			((sPlane*)pVehMG)->Fix();
		else if (pVehMG->GetType() == eElementType::ELEMENT_TYPE_HELI)
			((sHeli*)pVehMG)->Fix();
		else if (pVehMG->GetType() == eElementType::ELEMENT_TYPE_QUADBIKE)
			((sQuadBike*)pVehMG)->Fix();
#endif
	}
}

void on_recv_set_tyres_no_burst(net::pckt_set_tyres_no_burst& packet, int sender, uint16 time, bool bFromRing) // ID 12
{
	cMultiGame& Game = cMultiGame::Instance();
	if (packet.owner != Game.LocalPlayerID()) return;
	sVehicle* pVehMG = (sVehicle*)Game.GetEntityForHandle(packet.owner, packet.elem);
	if (pVehMG == nil) return;
	CVehicle* pVeh = (CVehicle*)pVehMG->GetEntity();
	if (pVeh == nil) return;
	pVeh->bTyresDontBurst = packet.enabled;
}

void on_recv_delete_vehicle(net::pckt_delete_vehicle& packet, int sender, uint16 time, bool bFromRing) // ID 13
{
	cMultiGame& Game = cMultiGame::Instance();
	if (packet.owner != Game.LocalPlayerID()) return;
	sVehicle* pVehMG = (sVehicle*)Game.GetEntityForHandle(packet.owner, packet.elem);
	if (pVehMG == nil) return;
	CVehicle* pVeh = (CVehicle*)pVehMG->GetEntity();
	if (pVeh == nil) return;
	if (pVeh->pDriver != nil) return;
#ifdef GTA_LIBERTY
	CWorld::Remove(pVeh);
#else
	CWorld::Remove(pVeh, WORLD_REMOVE_WITH_CLEANUP_VEHICLES);
#endif
	delete pVeh;
}

void on_recv_set_vehicle_infinite_mass(net::pckt_set_vehicle_infinite_mass& packet, int sender, uint16 time, bool bFromRing) // ID 26
{
	cMultiGame& Game = cMultiGame::Instance();
	if (packet.owner != Game.LocalPlayerID()) return;
	sVehicle* pVehMG = (sVehicle*)Game.GetEntityForHandle(packet.owner, packet.elem);
	if (pVehMG == nil) return;
	CVehicle* pVeh = (CVehicle*)pVehMG->GetEntity();
	if (pVeh == nil) return;
	pVeh->bInfiniteMass = packet.mass;
}

void on_recv_shot_vehicle(net::pckt_shot_vehicle& packet, int sender, uint16 time, bool bFromRing) // ID 38
{
	cMultiGame& Game = cMultiGame::Instance();
	sVehicle* pVehMG = (sVehicle*)Game.GetEntityForHandle(Game.LocalPlayerID(), packet.vehicle_id);
	if (pVehMG == nil) return;
	CVehicle* pVeh = (CVehicle*)pVehMG->GetEntity();
	if (pVeh == nil) return;
	sPed* pShooterPed = (sPed*)Game.GetEntityForHandle(sender, packet.shooter_id);
	if (pShooterPed == nil) return;
	pVeh->InflictDamage(pShooterPed->GetPhysical(), (eWeaponType)packet.type, packet.damage);
}

void on_recv_spawn_car_debris(net::pckt_spawn_car_debris& packet, int sender, uint16 time, bool bFromRing) // ID 55
{
	cMultiGame& Game = cMultiGame::Instance();
	sVehicle* pVehMG = (sVehicle*)Game.GetEntityForHandle(packet.owner, packet.elem);
	if (pVehMG == nil) return;
	sAutomobile* pAutomobile = pVehMG->HasCapability(sAutomobile::Capability()) ? (sAutomobile*)pVehMG : nil;
	if (pAutomobile == nil) return;
	CVector vecMoveSpeed = packet.move_speed;
	CVector vecTurnSpeed = packet.turn_speed;
	pAutomobile->SpawnFlyingComponent((eCarNodes)packet.car_component, packet.car_component_type, packet.unk1, vecMoveSpeed, vecTurnSpeed, packet.render_scorched);
}

void on_recv_set_vehicle_health(net::pckt_set_vehicle_health& packet, int sender, uint16 time, bool bFromRing) // ID 56
{
	cMultiGame& Game = cMultiGame::Instance();
	sElement* pElem = Game.GetEntityForHandle(packet.owner, packet.elem);
	if (pElem == nil) return;
	sVehicle* pVehicle = pElem->HasCapability(sVehicle::Capability()) ? (sVehicle*)pElem : nil;
	if (pVehicle == nil) return;
	CVehicle* pVeh = (CVehicle*)pVehicle->GetEntity();
	if (pVeh == nil) return;
	pVeh->m_fHealth = packet.health;
}

void on_recv_set_vehicle_position(net::pckt_set_vehicle_position& packet, int sender, uint16 time, bool bFromRing) // ID 57
{
	cMultiGame& Game = cMultiGame::Instance();
	sVehicle* pElem = (sVehicle*)Game.GetEntityForHandle(packet.owner, packet.elem);
	if (pElem == nil) return;
#ifndef THIS_IS_STUPID
	sVehicle* pVehicle = pElem->HasCapability(sVehicle::Capability()) ? (sVehicle*)pElem : nil;
	if (pVehicle == nil) return;
	CVehicle* pVeh = (CVehicle*)pElem->GetEntity();
	if (pVeh == nil) return;
	pVeh->SetPosition(packet.pos);
#else
	// plane, quadbike, automobile, heli
	if (pElem->HasCapability(sAutomobileBase::Capability())) // not check GetType() == eElementType::Car ?
	{
		CVehicle* pVeh = (CVehicle*)pElem->GetEntity();
		if (pVeh)
			pVeh->SetPosition(packet.pos);
	}
	// bike
	else if (pElem->GetType() == eElementType::Bike && pElem->HasCapability(sBike::Capability()))
	{
		CBike* pVeh = (CBike*)pElem->GetEntity();
		if (pVeh)
			pVeh->SetPosition(packet.pos);
	}
#if defined(FIX_BUGS) && !defined(GTA_LIBERTY)
	// bmx
	else if (pElem->GetType() == eElementType::Bmx && pElem->HasCapability(sBmx::Capability()))
	{
		CBmx* pVeh = (CBmx*)pElem->GetEntity();
		if (pVeh)
			pVeh->SetPosition(packet.pos);
	}
	// boat
	else if (pElem->GetType() == eElementType::Boat && pElem->HasCapability(sBoat::Capability()))
	{
		CBoat* pVeh = (CBoat*)pElem->GetEntity();
		if (pVeh)
			pVeh->SetPosition(packet.pos);
	}
#endif
#endif
}

void on_recv_vehicle_impact(net::pckt_vehicle_impact& packet, int sender, uint16 time, bool bFromRing) // ID 58
{
	cMultiGame& Game = cMultiGame::Instance();
	sVehicle* pElem = (sVehicle*)Game.GetEntityForHandle(Game.LocalPlayerID(), packet.dest); // local
	if (pElem == nil || !pElem->HasCapability(sVehicle::Capability())) {
		debug("MessageHandlerVehicleImpact: vehicle impact not against a vehicle?\n");
		return;
	}

	CVehicle* pVeh = (CVehicle*)pElem->GetEntity();
	if (pVeh) {
		pVeh->ApplyCollisionMultiplayer(packet.vPoint, packet.vNormal, packet.pieceB);
	}
	else {
		debug("MessageHandlerVehicleImpact: not a real vehicle on this machine any more?");
	}
}

#ifndef GTA_LIBERTY
void on_recv_msg_blowup_vehicle(net::pckt_msg_blowup_vehicle& packet, int sender, uint16 time, bool bFromRing) // ID 63
{
	cMultiGame& Game = cMultiGame::Instance();
	sVehicle* pElem = (sVehicle*)Game.GetEntityForHandle(packet.owner, packet.elem);
	sPed* pPed = Game.GetPlayerPed(packet.player_id);
	cPedMG* pPedPhysical = pPed ? (cPedMG*)pPed->GetPhysical() : nil;
	sAutomobileBase* pAuto = pElem && pElem->HasCapability(sAutomobileBase::Capability()) ? (sAutomobileBase*)pElem : nil;
	if (pElem && pElem->HasCapability(sAutomobileBase::Capability()))
	{
		CAutomobile* pAuto = (CAutomobile*)((sAutomobileBase*)pElem)->GetEntity();
		if (pAuto) pAuto->BlowUpCar(pPedPhysical/*, 0*/);
	}
	else if (pElem && pElem->GetType() == eElementType::ELEMENT_TYPE_BIKE && pElem->HasCapability(sBike::Capability()))
	{
		CBike* pBike = (CBike*)((sBike*)pElem)->GetEntity();
		if (pBike) pBike->BlowUpCar(pPedPhysical/*, 0*/);
	}
#ifdef FIX_BUGS
	else if (pElem && pElem->GetType() == eElementType::ELEMENT_TYPE_BOAT && pElem->HasCapability(sBoat::Capability()))
	{
		CBoat* pBoat = (CBoat*)((sBike*)pElem)->GetEntity();
		if (pBoat) pBoat->BlowUpCar(pPedPhysical/*, 0*/);
	}
	// bmx can't blowup
#endif
}
#endif
