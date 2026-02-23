/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once
#include "multiplayer/MultiGame.h"
#include "multiplayer/net/packet.h"
#include "multiplayer/natives/public.h"
#include "multiplayer/events/public.h"

#include "common.h"
#include "Ped.h"
#include "PlayerPed.h"
#include "PlayerInfo.h"
#include "Vehicle.h"
#include "Pad.h"
#include "World.h"


void cMultiGame::OnPlayerKill(net::pckt_player_kill& packet, int sender, uint16 time, bool bFromRing) // ID 3
{
	lua_State* vm = cLWrapper::Instance().m_luaVM;
	lua_pushstring(vm, "RegisterPlayerKill");
	lua_gettable(vm, LUA_GLOBALSINDEX);
	if (!lua_isnil(vm, -1)) {
		lsn_push_player_id(vm, sender); // who got killed
		lsn_push_player_id(vm, packet.assassin);
		lsc_call(vm, 2, false);
	}
}

void on_recv_target_player(net::pckt_target_player& packet, int sender, uint16 time, bool bFromRing) // ID 20
{
	cMultiGame& Game = cMultiGame::Instance();
	Game.SetTargetPlayer(packet.player, false);
}

void on_recv_fight_hit_ped(net::pckt_fight_hit_ped& packet, int sender, uint16 time, bool bFromRing) // ID 27
{
	MULTIGAME_UNIMPLEMENTED_EVENT();
}

void on_recv_shot_ped(net::pckt_shot_ped& packet, int sender, uint16 time, bool bFromRing) // ID 28
{
	MULTIGAME_UNIMPLEMENTED_EVENT();
}

void on_recv_shot_ped_from_car(net::pckt_shot_ped_from_car& packet, int sender, uint16 time, bool bFromRing) // ID 35
{
	MULTIGAME_UNIMPLEMENTED_EVENT();
}

void on_recv_kill_player_ped(net::pckt_kill_player_ped& packet, int sender, uint16 time, bool bFromRing) // ID 37
{
	if (packet.player_id == cMultiGame::Instance().LocalPlayerID())
		FindPlayerPed()->InflictDamage(nil, eWeaponType::WEAPONTYPE_FALL, 1000.0f, ePedPieceTypes::PEDPIECE_TORSO, 0);
}

void on_recv_melee(net::pckt_melee& packet, int sender, uint16 time, bool bFromRing) // ID 39
{
	MULTIGAME_UNIMPLEMENTED_EVENT();
}

void on_recv_player_been_hit(net::pckt_player_been_hit& packet, int sender, uint16 time, bool bFromRing) // ID 40
{
	MULTIGAME_UNIMPLEMENTED_EVENT();
}

void on_recv_player_control(net::pckt_player_control& packet, int sender, uint16 time, bool bFromRing) // ID 41
{
	CPad* pPad = CPad::GetPad(0);

	if (!packet.player_control_toggle_type)
	{
		if (packet.player_control_toggle_value)
			pPad->DisablePlayerControls &= ~PLAYERCONTROL_PLAYERINFO; // enable control
		else
			pPad->DisablePlayerControls |= PLAYERCONTROL_PLAYERINFO; // disable control
	}
	else
	{
		//pPad->__bApplyBrakes = !packet.player_control_toggle_value;
		TODO();
	}
}

void on_recv_player_set_position(net::pckt_set_position& packet, int sender, uint16 time, bool bFromRing) // ID 42
{
	CPlayerPed* pPed = FindPlayerPed();
	CVector pos = packet.pos;
	pos.z = CWorld::FindGroundZFor3DCoord(pos.x, pos.y, pos.z, nil) + 1.0f;
#ifdef GTA_LIBERTY
	if (pPed->bInVehicle && pPed->m_pMyVehicle)
#else
	if (pPed->InVehicle() && pPed->m_pMyVehicle)
#endif
		pPed->m_pMyVehicle->SetPosition(pos);
	else
		pPed->Teleport(pos);
}


void on_recv_player_set_heading(net::pckt_set_heading& packet, int sender, uint16 time, bool bFromRing) // ID 43
{
	CPlayerPed* pPed = FindPlayerPed();
	const float fRot = DEGTORAD(packet.heading);
#ifdef GTA_LIBERTY
	if (pPed->bInVehicle && pPed->m_pMyVehicle)
#else
	if (pPed->InVehicle() && pPed->m_pMyVehicle)
#endif
		pPed->m_pMyVehicle->SetHeading(fRot);
	else {
		pPed->m_fRotationCur = fRot;
		pPed->m_fRotationDest = fRot;
		pPed->SetHeading(fRot);
	}
}

void on_recv_set_player_blip_visible_state(net::pckt_set_player_blip_visible_state& packet, int sender, uint16 time, bool bFromRing) // ID 48
{
	cMultiGame& Game = cMultiGame::Instance();
	if (packet.player_id != Game.LocalPlayerID()) return;
	CPlayerPed* pPed = FindPlayerPed();
#ifdef GTA_LIBERTY
	if (packet.visible)
#if defined(GTA_LIBERTY) && !defined(FIX_BUGS)
		pPed->m_nPowerups = 0x0; // what? // check mp_lsn_RemoveLocalPlayerNoRadarForEnemy
#else
		pPed->bNoRadarForEnemy = false;
#endif
	else
		pPed->bNoRadarForEnemy = true; // lcs 0x20
#else
	if (packet.visible)
		pPed->bNoRadarForEnemy = false;
	else
		pPed->bNoRadarForEnemy = true;
#endif
}

void on_recv_player_respawn(net::pckt_player_respawn& packet, int sender, uint16 time, bool bFromRing) // ID 49
{
	cMultiGame& Game = cMultiGame::Instance();
	Game.SetPlayerSpawned(sender);
	sPed* pPed = Game.GetPlayerPed(sender);
	if (pPed) pPed->Respawn();
}

