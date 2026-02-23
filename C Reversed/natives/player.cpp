/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"
#include "PlayerPed.h"
#include "Streaming.h"
#include "World.h"
#include "Pad.h"
#include "Vehicle.h"
#include "Camera.h"
#include "MBlur.h"
#include "GameLogic.h"
#include "DMAudio.h"
#include "Radar.h"
#include "Bike.h"
#include "Automobile.h"
#include "ColStore.h"
#include "Zones.h"

#include "multiplayer/natives/public.h"
#include "multiplayer/events/public.h"
#include "multiplayer/LobbyPed.h"
#include "multiplayer/Multigame.h"

#include "multiplayer/elements/sPed.h"
#include "multiplayer/elements/sPlayer.h"
#include "multiplayer/elements/sVehicle.h"
#include "multiplayer/elements/sBike.h"
#include "multiplayer/elements/sAutomobile.h"


static int mp_lsn_IsPlayerDead(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	bool bIsDead = false;

	if (!lsc_isPlayerUserData(L, 1)) return 0;
	int32 nPlayerID = lsc_getPlayer(L, 1);
	sPlayer* pPlayer = (sPlayer*)Game.GetEntityForHandle(nPlayerID, eElementID::MG_ELEMENT_PLAYER_ID); // Game.GetPlayer()
	sPed* pPed = (sPed*)Game.GetEntityForHandle(nPlayerID, eElementID::MG_ELEMENT_PLAYER_PED_ID); // Game.GetPlayerPed()
	if (pPed == nil || pPlayer == nil) return 0;

	if (is_local_player(nPlayerID))
		bIsDead = CWorld::Players[CWorld::PlayerInFocus].m_WBState != WBSTATE_PLAYING;
	else
		bIsDead = pPlayer->GetSync().player->m_eWBState != WBSTATE_PLAYING;

	sPedSync* pSync = pPed->GetSync().ped;
	if (pSync->m_nHealth < 2 || pSync->DyingOrDead())
		bIsDead = true;

	lua_pushboolean(L, bIsDead);
	return 1;
}

#ifndef GTA_LIBERTY
static int mp_lsn_SetCenterBlipVisible(lua_State* L) {
	int32 nPlayerID = lsc_getPlayer(L, 1); // unused
	if (!lua_isboolean(L, 2)) return 0;
	FindPlayerPed()->m_bIsCenterBlipVisible = (bool)lua_toboolean(L, 2);
	return 0;
}
#endif

static int mp_lsn_GetPlayerPosition(lua_State* L) {
	return mp_lsn_PedPosition(L);
}

static int mp_lsn_PlayerToString(lua_State* L) {
	lua_pushstring(L, "tostring");
	lua_gettable(L, LUA_GLOBALSINDEX);
	int nPlayerID = lsc_getPlayer(L, 1);
	lua_pushnumber(L, nPlayerID);
	lua_call(L, 1, 1);
	return 1;
}

static int mp_lsn_PlayerName(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	if (lua_isstring(L, 1)) {
		Game.SetPlayerName(lua_tostring(L, 1));
		return 0;
	}
	int id = get_player_id(L);
	if (Game.IsPlayerConnected(id)) {
		const char* sPlayerName = Game.GetPlayerName(id);
		lua_pushstring(L, sPlayerName);
	}
	else {
		debug("Trying to get a player name after they have disconnected !!!");
		lua_pushstring(L, "");
	}
	return 1;
}

static int mp_lsn_PlayerNum(lua_State* L) {
	lua_pushnumber(L, get_player_id(L));
	return 1;
}

static int mp_lsn_PlayerTeamName(lua_State* L) {
	const char* sName = cMultiGame::Instance().GetGangNameForEntity(get_player_id(L));
	lua_pushstring(L, sName);
	return 1;
}

static int mp_lsn_TeamId(lua_State* L) {
	int32 nTeamID = cMultiGame::Instance().GetPlayerTeamID(get_player_id(L));
	lua_pushnumber(L, nTeamID);
	return 1;
}

#ifndef GTA_LIBERTY
static int mp_lsn_TeamPeerGroup(lua_State* L) {
	cPeerManager& PeerMgr = PeerManager;
	int32 nTeamID = cMultiGame::Instance().GetPlayerTeamID(get_player_id(L));

	if (nTeamID == static_cast<uint32>(eGameTeam::TEAM_A))
		lua_pushnumber(L, PeerMgr.m_nTeamAPeerGroupId);
	else //if(nTeamID == static_cast<uint32>(eGameTeam::TEAM_B))
		lua_pushnumber(L, PeerMgr.m_nTeamBPeerGroupId);
	return 1;
}
#endif

static int mp_lsn_PlayerCar(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	sPed* pPed = (sPed*)Game.GetEntityForHandle(get_player_id(L), eElementID::MG_ELEMENT_PLAYER_PED_ID); // Game.GetPlayerPed()
	if (pPed == nil) return 0;
	int16 nVehID = pPed->GetSync().ped->GetVehicleID();
	if (nVehID == -1) return 0;
	sElement* pCar = Game.GetEntityForHandle(pPed->GetOwner(), nVehID);
	if (pCar == nil) return 0;
	lsc_register_entity(L, pCar);
	return 1;
}

static int mp_lsn_PlayerColour(lua_State* L) {
#ifdef GTA_LIBERTY
	int id = lsc_getPlayer(L, 1);
#else
	int id = lsc_getPlayerSafety(L, 1);
#endif
	CRGBA* pColor = cMultiGame::Instance().GetPlayerColor(id);
	uint32 color = RGB24_PACK(pColor->red, pColor->green, pColor->blue);
	lua_pushnumber(L, color);
	return 1;
}

static int mp_lsn_PlayerColourWithAlpha(lua_State* L) {
#ifdef GTA_LIBERTY
	int id = lsc_getPlayer(L, 1);
#else
	int id = lsc_getPlayerSafety(L, 1);
#endif
	CRGBA* pColor = cMultiGame::Instance().GetPlayerColor(id);
	uint32 color = CRGBA_PACK(pColor->red, pColor->green, pColor->blue, pColor->alpha);
	lua_pushnumber(L, color);
	return 1;
}

static int mp_lsn_PlayerTeamColour(lua_State* L) {
#ifdef GTA_LIBERTY
	int id = lsc_getPlayer(L, 1);
#else
	int id = lsc_getPlayerSafety(L, 1);
#endif
	CRGBA* pColor = cMultiGame::Instance().GetTeamColor(id);
	uint32 color = RGB24_PACK(pColor->red, pColor->green, pColor->blue);
	lua_pushnumber(L, color);
	return 1;
}

static int mp_lsn_IsConnected(lua_State* L) {
#ifdef GTA_LIBERTY
	int id = lsc_getPlayer(L, 1);
#else
	int id = lsc_getPlayerSafety(L, 1);
#endif
	bool isConnected = cMultiGame::Instance().IsPlayerConnected(id);
	lua_pushboolean(L, isConnected);
	return 1;
}

static int mp_lsn_StartDown(lua_State* L) { // scoreboard switch
	if (!is_local_player(lsc_getPlayer(L, 1))) return false;
	
	bool bIsDown = CPad::GetPad(0)->GetStartJustDown();
#ifdef GTA_PC // start nw in pc
	if(!bIsDown)
		bIsDown = CPad::GetPad(0)->GetEscapeJustDown();
#endif

	lua_pushboolean(L, bIsDown);
	return 1;
}

static int mp_lsn_LeftDown(lua_State* L) {
	if (!is_local_player(lsc_getPlayer(L, 1))) return 0;
	CPad* pPad = CPad::GetPad(0);
	// TODO: LVCS PC (same for others)
	bool bIsDown = pPad->GetDPadLeft() || pPad->GetAnaloguePadLeft();
#ifdef GTA_PC
	if (!bIsDown)
		bIsDown = CPad::GetPad(0)->GetCharJustDown('A');
	if (!bIsDown)
		bIsDown = CPad::GetPad(0)->GetLeftJustDown();
#endif
	lua_pushboolean(L, bIsDown);
	return 1;
}

static int mp_lsn_RightDown(lua_State* L) {
	if (!is_local_player(lsc_getPlayer(L, 1))) return 0;
	CPad* pPad = CPad::GetPad(0);
	// TODO: LVCS PC (same for others)
	bool bIsDown = pPad->GetDPadRight() || pPad->GetAnaloguePadRight();
#ifdef GTA_PC
	if (!bIsDown)
		bIsDown = CPad::GetPad(0)->GetCharJustDown('D');
	if (!bIsDown)
		bIsDown = CPad::GetPad(0)->GetRightJustDown();
#endif
	lua_pushboolean(L, bIsDown);
	return 1;
}

static int mp_lsn_UpDown(lua_State* L) {
	if (!is_local_player(lsc_getPlayer(L, 1))) return 0;
	CPad* pPad = CPad::GetPad(0);
	// TODO: LVCS PC (same for others)
	bool bIsDown = pPad->GetDPadUp() || pPad->GetAnaloguePadUp();
#ifdef GTA_PC
	if (!bIsDown)
		bIsDown = CPad::GetPad(0)->GetCharJustDown('W');
	if (!bIsDown)
		bIsDown = CPad::GetPad(0)->GetUpJustDown();
#endif
	lua_pushboolean(L, bIsDown);
	return 1;
}

static int mp_lsn_DownDown(lua_State* L) {
	if (!is_local_player(lsc_getPlayer(L, 1))) return 0;
	CPad* pPad = CPad::GetPad(0);
	// TODO: LVCS PC (same for others)
	bool bIsDown = pPad->GetDPadDown() || pPad->GetAnaloguePadDown();
#ifdef GTA_PC
	if (!bIsDown)
		bIsDown = CPad::GetPad(0)->GetCharJustDown('S');
	if (!bIsDown)
		bIsDown = CPad::GetPad(0)->GetDownJustDown();
#endif
	lua_pushboolean(L, bIsDown);
	return 1;
}

// psp cross like Enter on PC
static int mp_lsn_CrossDown(lua_State* L) {
	if (!is_local_player(lsc_getPlayer(L, 1))) return 0;
	// TODO: LVCS PC (same for others)
	bool bIsDown = CPad::GetPad(0)->GetCross();
#ifdef GTA_PC
	if (!bIsDown)
		bIsDown = CPad::GetPad(0)->GetEnterJustDown();
#endif
	lua_pushboolean(L, bIsDown);
	return 1;
}

// psp circle like esc on PC
static int mp_lsn_CircleDown(lua_State* L) {
	if (!is_local_player(lsc_getPlayer(L, 1))) return 0;
	// TODO: LVCS PC (same for others)
	bool bIsDown = CPad::GetPad(0)->GetCircle();
#ifdef GTA_PC
	if (!bIsDown)
		bIsDown = CPad::GetPad(0)->GetEscapeJustDown();
#endif
	lua_pushboolean(L, bIsDown);
	return 1;
}

// psp triange like esc on PC
static int mp_lsn_TriangleDown(lua_State* L) {
	if (!is_local_player(lsc_getPlayer(L, 1))) return 0;
	// TODO: LVCS PC (same for others)
	bool bIsDown = CPad::GetPad(0)->GetTriangle();
#ifdef GTA_PC
	if (!bIsDown)
		bIsDown = CPad::GetPad(0)->GetEscapeJustDown();
#endif
	lua_pushboolean(L, bIsDown);
	return 1;
}

// not implemented in vanilla
//static int mp_lsn_SquareDown(lua_State* L) {
//	if (!is_local_player(lsc_getPlayer(L, 1))) return 0;
//	// TODO: LVCS PC (same for others)
//	bool bIsDown = CPad::GetPad(0)->GetSquare();
//#ifdef GTA_PC
//	MULTIGAME_UNIMPLEMENTED(); // test+todo
//#endif
//	lua_pushboolean(L, bIsDown);
//	return 1;
//}

#ifndef GTA_LIBERTY
static int mp_lsn_HasFlagBall(lua_State* L) {
	int nPlayerID = lsc_getPlayer(L, 1); // unused?
	lua_pushboolean(L, FindPlayerPed()->HasFlagBall());
	return 1;
}

static int mp_lsn_DropFlagBall(lua_State* L) {
	int nPlayerID = lsc_getPlayer(L, 1); // unused?
	FindPlayerPed()->DropFlagBall();
	return 0;
}

static int mp_lsn_GiveFlagBall(lua_State* L) {
	int nPlayerID = lsc_getPlayer(L, 1); // unused?
	FindPlayerPed()->GiveFlagBall();
	return 0;
}
#endif

/* TODO: stub */
static int mp_lsn_Latency(lua_State* L) {
#ifdef GTA_LIBERTY
	int32 nPlayerID = lsc_getPlayer(L, 1);
	sPeerState* pPeer = cMultiGame::Instance().GetPeerAt(nPlayerID);
#else
	int32 nPlayerID = lsc_getPlayerSafety(L, 1);
	sPeerState* pPeer = PeerManager.GetPeerAt(nPlayerID);
#endif
	assert(pPeer);
	float fLag = pPeer->m_bufferIndex ? (pPeer->m_latencySum / pPeer->m_bufferIndex) : 0.0f;
	fLag = floorf(fLag * 1000.0f); // to ms
	lua_pushnumber(L, fLag);
	return 1;
}

static int mp_lsn_IsPlaying(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	int32 nPlayerID = lsc_getPlayer(L, 1);
	if (is_local_player(nPlayerID)) {
		lua_pushboolean(L, CWorld::Players[0].m_WBState == WBSTATE_PLAYING);
		return 1;
	}
	sPlayer* pPlayer = (sPlayer*)Game.GetEntityForHandle(nPlayerID, MG_ELEMENT_PLAYER_ID); // Game.GetPlayer()
	lua_pushboolean(L, pPlayer->GetSync().player->m_eWBState == WBSTATE_PLAYING);
	return 1;
}

static int mp_lsn_Respawn(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	int nPlayerID = lsc_getPlayer(L, 1);
	if (!is_local_player(nPlayerID))
		return 0;
	CVector pos;
	lsc_getVectorFromStack(pos, L, 2, true);
	float fHeading = lua_tonumber(L, 3);
	CPlayerInfo& info = CWorld::Players[CWorld::PlayerInFocus];
	info.m_WBState = WBSTATE_PLAYING;
	CPlayerPed* pPed = info.m_pPed;
	if (pPed->m_pMyVehicle != nil) {
		CVehicle* pVeh = pPed->m_pMyVehicle;
		if (pVeh->pDriver == pPed || pVeh->m_vehLCS_258) {
			bool bRemoveDriver = (pVeh->pDriver && pVeh->pDriver->IsPlayer()) || pVeh->m_nDriverIdMG >= 0;
			if (bRemoveDriver) {
				debug("Respawn: Removing driver from vehicle\n");
				pVeh->pDriver = nil;
				pVeh->m_nDriverIdMG = -1;
				pVeh->bIsBeingCarJacked = false;
				pVeh->m_vehLCS_258 = false;
				if (pVeh->GetStatus() != STATUS_WRECKED)
					pVeh->SetStatus(STATUS_ABANDONED);
			}
			else {
				CWorld::Remove(pPed->m_pMyVehicle);
				delete pPed->m_pMyVehicle;
			}
		}
	}
#ifndef GTA_LIBERTY
	CColStore::RemoveAllCollision();
#endif
	TheCamera.m_fCamShakeForce = 0.0f;
	CMBlur::Reset(); // TODO(MP): missing class
	//TheCamera.field_5A = 1;
	TheCamera.m_bCameraJustRestored = true;
#ifdef FIX_BUGS
	CPad::GetPad(0)->DisablePlayerControls = 0;
#else
	CPad::GetPad(CWorld::PlayerInFocus)->DisablePlayerControls = 0; // what???
#endif
	if (pos.z <= MAP_Z_LOW_LIMIT)
		pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
	CGameLogic::RestorePlayerStuffDuringResurrection_NetworkGame(info.m_pPed, pos, fHeading);
	info.m_WBState = WBSTATE_PLAYING;
	eGameType eType = Game.GetGameType();
	switch (eType)
	{
#ifndef GTA_LIBERTY
		case eGameType::HUNTERATTACK:
			pPed->GiveWeapon(eWeaponType::WEAPONTYPE_RUGER, 250);
			pPed->GiveWeapon(eWeaponType::WEAPONTYPE_SHOTGUN, 10);
			pPed->GiveWeapon(eWeaponType::WEAPONTYPE_ROCKETLAUNCHER, 30);
			pPed->GiveWeapon(eWeaponType::WEAPONTYPE_SNIPERRIFLE, 30);
			pPed->SetCurrentWeapon(eWeaponType::WEAPONTYPE_RUGER);
			break;
#endif
		case eGameType::TANK:
			pPed->GiveWeapon(eWeaponType::WEAPONTYPE_TEC9, 50);
			pPed->GiveWeapon(eWeaponType::WEAPONTYPE_SHOTGUN, 10);
			pPed->GiveWeapon(eWeaponType::WEAPONTYPE_GRENADE, 1);
			pPed->GiveWeapon(eWeaponType::WEAPONTYPE_ROCKETLAUNCHER, 1);
			pPed->SetCurrentWeapon(eWeaponType::WEAPONTYPE_ROCKETLAUNCHER);
			break;
		case eGameType::MULTIRACE:
#ifndef GTA_LIBERTY
			if(Game.GetCTFScoreLimit() != 1 && Game.GetCTFScoreLimit() != 2)
#endif
			{
				pPed->GiveWeapon(eWeaponType::WEAPONTYPE_TEC9, 200);
				pPed->GiveWeapon(eWeaponType::WEAPONTYPE_ROCKETLAUNCHER, 10);
				pPed->SetCurrentWeapon(eWeaponType::WEAPONTYPE_TEC9);
			}
			break;
		default:
#ifdef GTA_LIBERTY
			pPed->GiveWeapon(eWeaponType::WEAPONTYPE_TEC9, 50);
#else
			pPed->GiveWeapon(eWeaponType::WEAPONTYPE_COLT45, 34);
			pPed->GiveWeapon(eWeaponType::WEAPONTYPE_TEC9, 100);
#endif
			pPed->SetCurrentWeapon(eWeaponType::WEAPONTYPE_TEC9);
			break;
	}
	// TODO(MP): missing code
	pPed->RemoveUberPickup();
	pPed->SetIdleAndResetAnim();
	CPad::GetPad(0)->Clear(true);
	debug("Loading Scene around player...\n");
	debug("Doing respawn at %f %f %f", pos.x, pos.y, pos.z);
	CTimer::Suspend();
	DMAudio.SetEffectsFadeVol(0);
	CPad::StopPadsShaking();
	DMAudio.Service();
	CStreaming::LoadSceneCollision(pPed->GetPosition());
	CTimer::Resume();
	DMAudio.SetEffectsFadeVol(127);
	debug("Loading scene done, fade started...\n");
	TheCamera.RestoreWithJumpCut();
	TheCamera.Process();
	TheCamera.SetFadeColour(0, 0, 0);
	TheCamera.Fade(0, 0.0f);
	TheCamera.ProcessFade();
	TheCamera.Fade(1, 1.0f);
	net::pckt_player_respawn packet{};
	packet.pckt_size = sizeof(net::pckt_player_respawn);
	packet.pckt_id = gtMP_PacketIDs.player_respawn.pckt_id;
	on_recv_player_respawn(packet, Game.LocalPlayerID(), 0, true);
	Game.SendMessagePriority(packet, BROADCAST_PEER_GROUPID);
	return 0;
}

enum ButtonMask {
	BTN_CIRCLE   = BIT(0),
	BTN_SQUARE   = BIT(1),
	BTN_TRIANGLE = BIT(2),
	BTN_CROSS    = BIT(3),
	BTN_START    = BIT(4),
	BTN_SELECT   = BIT(5),
};

/* TODO(MP): stub */
uint32 GetKeyPressState() {
	uint32_t mask = 0;
	if (CPad::GetPad(0)->NewState.Circle)
		mask |= BTN_CIRCLE;

	if (CPad::GetPad(0)->NewState.Square)
		mask |= BTN_SQUARE;

	if (CPad::GetPad(0)->NewState.Triangle)
		mask |= BTN_TRIANGLE;

	if (CPad::GetPad(0)->NewState.Cross)
		mask |= BTN_CROSS;

	if (CPad::GetPad(0)->NewState.Start)
		mask |= BTN_START;

	if (CPad::GetPad(0)->NewState.Select)
		mask |= BTN_SELECT;

#ifdef GTA_PC
	if (CPad::GetPad(0)->GetEscapeJustDown())
		mask |= BTN_START;

	if (CPad::GetPad(0)->GetEnterJustDown())
		mask |= BTN_CROSS;

	if (CPad::GetPad(0)->GetEscapeJustDown())
		mask |= BTN_CIRCLE;

	if (CPad::GetPad(0)->GetEscapeJustDown())
		mask |= BTN_TRIANGLE;

	MULTIGAME_UNIMPLEMENTED(); // maybe smth for pc controll?
#endif

	return mask;
}

static int KeyPressWaiter(lua_State* L) {
	uint32 nLastState = (uint32)lua_touserdata(L, lua_upvalueindex(1));
	uint32 nState = GetKeyPressState();
	if (nState == nLastState) {
		lua_pushlightuserdata(L, (void*)nState);
		lua_replace(L, lua_upvalueindex(1));
		return 0;
	}
	lua_pushboolean(L, true);
	return 1;
}

static int mp_lsn_WaitKeyPress(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	int nPlayerID = lsc_getPlayer(L, 1);
	if (!is_local_player(nPlayerID)) return 0;
	lua_pushlightuserdata(L, (void*)GetKeyPressState());
	lua_pushcclosure(L, KeyPressWaiter, 1);
	return lua_yield(L, 1);
}

static int mp_lsn_KeyPressAndCameraWaiter(lua_State* L) {
	uint32 nLastState = (uint32)lua_touserdata(L, lua_upvalueindex(1));
	uint32 nState = GetKeyPressState();
	const CVector& pos = TheCamera.GetPosition();
	const float fDist = pos.z - CWorld::FindGroundZForCoord(pos.x, pos.y);
	if (nState == nLastState && fDist <= 80.0f) {
		lua_pushlightuserdata(L, (void*)nState);
		lua_replace(L, lua_upvalueindex(1));
		return 0;
	}
	lua_pushboolean(L, true);
	return 1;
}

static int mp_lsn_WaitKeyPressAndCameraCheck(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	int nPlayerID = lsc_getPlayer(L, 1);
	if (!is_local_player(nPlayerID)) return 0;
	lua_pushlightuserdata(L, (void*)GetKeyPressState());
	lua_pushcclosure(L, mp_lsn_KeyPressAndCameraWaiter, 1);
	return lua_yield(L, 1);
}

static int mp_lsn_SetPlayerPosition(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	CVector pos;
	int nPlayerID = lsc_getPlayer(L, 1);
	lsc_getVectorFromStack(pos, L, 2, true);
	net::pckt_set_position packet{};
	packet.pckt_size = sizeof(net::pckt_set_position);
	packet.pckt_id = gtMP_PacketIDs.set_position.pckt_id;
	packet.pos = pos;
	if (is_local_player(nPlayerID))
		on_recv_player_set_position(packet, 0, 0, false); // bug? true from local game?
	Game.SendMessagePriority(packet, nPlayerID);
	return 0;
}

static int mp_lsn_SetHeading(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	int nPlayerID = lsc_getPlayer(L, 1);
	const float fHeading = lua_tonumber(L, 2);
	net::pckt_set_heading packet{};
	packet.pckt_size = sizeof(net::pckt_set_heading);
	packet.pckt_id = gtMP_PacketIDs.set_heading.pckt_id;
	packet.heading = fHeading;
	if (is_local_player(nPlayerID))
		on_recv_player_set_heading(packet, 0, 0, false); // bug? true?
	Game.SendMessagePriority(packet, nPlayerID);
	return 0;
}

static int mp_lsn_SetRadarBlipShortRange(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	if (!lsc_isPlayerUserData(L, 1)) return 0;
	int nPlayerID = lsc_getPlayer(L, 1);
	if (!lua_isboolean(L, 2)) return 0;
	sPlayer* pPlayer = (sPlayer*)Game.GetEntityForHandle(nPlayerID, MG_ELEMENT_PLAYER_ID); // Game.GetPlayer()
	if (pPlayer && pPlayer->GetBlipIndex() != -1) {
		bool bIsShortRange = lua_toboolean(L, 2);
		TheRadar->ms_RadarTrace[TheRadar->GetActualBlipArrayIndex(pPlayer->GetBlipIndex())].bShortRange = bIsShortRange;
	}
	return 0;
}

static int mp_lsn_SetRadarBlipIcon(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	if (!lsc_isPlayerUserData(L, 1)) return 0;
	int nPlayerID = lsc_getPlayer(L, 1);
	int nIconID = -1;
	if (lua_gettop(L) >= 2)
		nIconID = lua_tonumber(L, 2);
	sPlayer* pPlayer = (sPlayer*)Game.GetEntityForHandle(nPlayerID, MG_ELEMENT_PLAYER_ID); // Game.GetPlayer()
	if (pPlayer && pPlayer->GetBlipIndex() != -1) {
		TheRadar->SetBlipSprite(pPlayer->GetBlipIndex(), nIconID);
	}
	return 0;
}

static int mp_lsn_SetRadarBlipVisibleState(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
#ifdef GTA_LIBERTY
	if (!lsc_isPlayerUserData(L, 1)) return 0;
	int nPlayerID = lsc_getPlayer(L, 1);
#else
	int nPlayerID = 0;
	if (lsc_isPlayerUserData(L, 1))
		nPlayerID = lsc_getPlayer(L, 1);
	else if (lua_isnumber(L, 1))
		nPlayerID = lua_tonumber(L, 1);
#endif
	if (!lua_isboolean(L, 2)) return 0;
	bool bVisible = lua_toboolean(L, 2);
	net::pckt_set_player_blip_visible_state packet{};
	packet.pckt_id = gtMP_PacketIDs.set_player_blip_visible_state.pckt_id;
	packet.pckt_size = sizeof(net::pckt_set_player_blip_visible_state);
	packet.player_id = nPlayerID;
	packet.visible = bVisible;
	if (nPlayerID == Game.LocalPlayerID()) {
		on_recv_set_player_blip_visible_state(packet, nPlayerID, 0, true);
		return 0;
	}
	Game.SendMessagePriority(packet, nPlayerID);
	return 0;
}

static int mp_lsn_InATank(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	int nPlayerID = Game.LocalPlayerID();
	if (lsc_isPlayerUserData(L, 1))
		nPlayerID = lsc_getPlayer(L, 1);
	sPed* pPlayer = Game.GetPlayerPed(nPlayerID);
	sPedSync* pPedSync = pPlayer->GetSync().ped;
	bool bIsTank = false;
	if (pPedSync->GetVehicleID() != -1) {
		sElement* pElem = Game.GetEntityForHandle(pPlayer->GetOwner(), pPedSync->GetVehicleID());
		CEntity* pVeh = nil;
		if (pElem) pVeh = pElem->GetEntity();
		if (pVeh) bIsTank = pVeh->GetModelIndex() == Game.m_nTankModelID;
	}
	lua_pushboolean(L, bIsTank);
	return 1;
}

static int mp_lsn_SetPedHealth(lua_State* L) {
	if (!lsc_isPlayerUserData(L, 1)) return 0;
	int nPlayerID = lsc_getPlayer(L, 1);
	const float fHealth = lua_tonumber(L, 2);
	FindPlayerPed()->m_fHealth = fHealth;
	return 0;
}

static const luaL_reg player_ped_libs[] = {
	{"__tostring",                 mp_lsn_PlayerToString},
	{"Name",                       mp_lsn_PlayerName},
	{"PlayerNum",                  mp_lsn_PlayerNum},
	{"TeamName",                   mp_lsn_PlayerTeamName},
	{"TeamId",                     mp_lsn_TeamId},
#ifndef GTA_LIBERTY
	{"TeamPeerGroup",              mp_lsn_TeamPeerGroup},
#endif
	{"Car",                        mp_lsn_PlayerCar},
	{"Colour",                     mp_lsn_PlayerColour},
	{"ColourWithAlpha",            mp_lsn_PlayerColourWithAlpha},
	{"TeamColour",                 mp_lsn_PlayerTeamColour},
	{"IsConnected",                mp_lsn_IsConnected},
	{"Position",                   mp_lsn_GetPlayerPosition},
	{"StartDown",                  mp_lsn_StartDown},
	{"LeftDown",                   mp_lsn_LeftDown},
	{"RightDown",                  mp_lsn_RightDown},
	{"UpDown",                     mp_lsn_UpDown},
	{"DownDown",                   mp_lsn_DownDown},
	{"CrossDown",                  mp_lsn_CrossDown},
	{"CircleDown",                 mp_lsn_CircleDown},
	{"TriangleDown",               mp_lsn_TriangleDown},
	//{"SquareDown",                 mp_lsn_SquareDown}, // not implemented in vanilla
#ifndef GTA_LIBERTY
	{"HasFlagBall",                mp_lsn_HasFlagBall},
	{"DropFlagBall",               mp_lsn_DropFlagBall},
	{"GiveFlagBall",               mp_lsn_GiveFlagBall},
#endif
	{"Latency",                    mp_lsn_Latency},
	{"IsPlaying",                  mp_lsn_IsPlaying},
	{"Respawn",                    mp_lsn_Respawn},
	{"WaitKeyPress",               mp_lsn_WaitKeyPress},
	{"WaitKeyPressAndCameraCheck", mp_lsn_WaitKeyPressAndCameraCheck},
	{"SetPosition",                mp_lsn_SetPlayerPosition},
	{"SetHeading",                 mp_lsn_SetHeading},
	{"SetRadarBlipShortRange",     mp_lsn_SetRadarBlipShortRange},
	{"SetRadarBlipIcon",           mp_lsn_SetRadarBlipIcon},
	{"SetRadarBlipVisibleState",   mp_lsn_SetRadarBlipVisibleState},
	{"InATank",                    mp_lsn_InATank},
	{"SetHealth",                  mp_lsn_SetPedHealth},
	{"IsDead",                     mp_lsn_IsPlayerDead},
#ifndef GTA_LIBERTY
	{"SetCenterBlipVisible",       mp_lsn_SetCenterBlipVisible},
#endif
	{NULL, NULL}
};
VALIDATE_LUA_LIB(player_ped_libs, (37 + 1), (32 + 1));


void lsn_push_player_id(lua_State* L, int id) {
	lua_gettop(L);
	luaL_getmetatable(L, "PlayerId");
	lua_rawgeti(L, -1, id);
	if (lua_isnil(L, -1)) {
		lua_settop(L, -2);
		int32* pID = (int32*)lua_newuserdata(L, sizeof(int32));
		*pID = id;
		lua_pushvalue(L, -2);
		lua_setmetatable(L, -2);
		lua_pushvalue(L, -1);
		lua_rawseti(L, -3, id);
	}
	lua_remove(L, -2);
	lua_gettop(L);
}

static int mp_lsn_AllPlayers(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	lua_newtable(L);
	int32 nIndex = 1;
	uint32 nMaxSize = Max(Game.m_pNetSession->m_vPeers.size(), Game.LocalPlayerID() + 1);
	for (int32 nPeerID = 0; nPeerID < nMaxSize; nPeerID++) {
		if (!Game.GetPlayer(nPeerID)) continue;
		lsn_push_player_id(L, nPeerID);
		lua_rawseti(L, -2, nIndex);
		LUA_LOG(1, "mp_lsn_AllPlayers nPeerID %d nIndex %d\n", nPeerID, nIndex); // custom
		nIndex++;
	}
	LUA_LOG(1, "mp_lsn_AllPlayers end nIndex %d\n", nIndex); // custom
	return 1;
}

static int mp_lsn_AllPlayersExceptMe(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	lua_newtable(L);
	int32 nIndex = 1;
	uint32 nMaxSize = Max(Game.m_pNetSession->m_vPeers.size(), Game.LocalPlayerID() + 1);
	for (int32 nPeerID = 0; nPeerID < nMaxSize; nPeerID++) {
		if (nPeerID == Game.LocalPlayerID() || !Game.GetPlayer(nPeerID)) continue;
		lsn_push_player_id(L, nPeerID);
		lua_rawseti(L, -2, nIndex);
		LUA_LOG(1, "mp_lsn_AllPlayersExceptMe nPeerID %d nIndex %d\n", nPeerID, nIndex); // custom
		nIndex++;
	}
	LUA_LOG(1, "mp_lsn_AllPlayersExceptMe end nIndex %d\n", nIndex); // custom
	return 1;
}

sPlayer* createPlayerAt(CVector pos) {
	debug("***********Creating player: %d\n", eElementID::MG_ELEMENT_PLAYER_ID);
#ifndef GTA_LIBERTY
	CGame::currLevel = (cMultiGame::Instance().GetGameLocation() >= eGameLocation::DOWNTOWN_ZON) ? eLevelName::LEVEL_MAINLAND : eLevelName::LEVEL_BEACH;
	CStreaming::LoadSceneCollision(pos);
	if (TheMPGame.m_bHasSuspended)
		return nil;
#endif
	CPlayerPed::SetupPlayerPed(0);
	CPed* pPed = CWorld::Players[0].m_pPed;
	pPed->CharCreatedBy = MISSION_CHAR;
	CPlayerPed::DeactivatePlayerPed(0);
#ifdef GTA_LIBERTY
	CStreaming::LoadSceneCollision(pos);
#endif
	if (pos.z <= MAP_Z_LOW_LIMIT) {
		pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		pos.z += pPed->GetDistanceFromCentreOfMassToBaseOfModel();
	}
	pPed->SetPosition(pos);
	pPed->SetTeamID((int16)cMultiGame::s_nSelectedTeam);
	CPlayerPed::ReactivatePlayerPed(0);
	int32 nModelID = cMultiGame::s_nPlayerModelIndex;
	AssocGroupId assocID = pPed->m_animGroup;
	CBaseModelInfo* pInfo = CModelInfo::GetModelInfo(ga_netModelList[nModelID].name, &nModelID);
	if (!pInfo)
		nModelID = MI_PLAYER;
	if (pPed->m_rwObject)
		pPed->DeleteRwObject();
	pPed->SetModelIndex(nModelID);
	pPed->m_animGroup = assocID;
	sPlayer* pPlayer = new sPlayer(); // ID:0 MG_ELEMENT_PLAYER_ID
	pPlayer->RegisterSelf();
	cMultiGame::Instance().m_ZoneManager.GetZoneByPeer(MP_HOST_INDEX)->RegisterElement(pPlayer);
	sPed* pPedMP = new sPed(pPed); // ID:1 MG_ELEMENT_PLAYER_PED_ID
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_BASEBALLBAT, 1);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_COLT45, 999);
	FindPlayerPed()->SetCurrentWeapon(WEAPONTYPE_COLT45);
	CPlayerPed::DeactivatePlayerPed(0);
	return pPlayer;
}

#ifndef GTA_LIBERTY
void lsc_tryCreateMpPlayer()
{
	cMultiGame& Game = cMultiGame::Instance();
	if (Game.GetPlayerCreationQueued()) {
		if (!Game.m_bHasSuspended && !TheAdhoc.HadError()) {
			sPlayer* pPlayer = createPlayerAt(Game.GetPlayerCreationQueuedPosition());
		}
		Game.SetPlayerCreationQueued(false);
	}
}
#endif

int mp_lsn_CreatePlayer(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	float fPosX = luaL_checknumber(L, 1);
	float fPosY = luaL_checknumber(L, 2);
	float fPosZ = MAP_Z_LOW_LIMIT;
	if (lua_gettop(L) >= 3)
		fPosZ = luaL_checknumber(L, 3);
#ifdef GTA_LIBERTY
	sPlayer* pPlayer = createPlayerAt(CVector(fPosX, fPosY, fPosZ));
	lsc_register_entity(L, pPlayer);
	return 1;
#else // buggly async vanilla vcs method
	Game.RequestPlayerCreation(CVector(fPosX, fPosY, fPosZ));
	return 0;
#endif
}

static int mp_lsn_WarpPlayerIntoCar(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	int nPedID = luaL_checknumber(L, 1);
	sPed* pPedMG = Game.GetPlayerPed(nPedID);
	CPlayerPed* pPlayer = FindPlayerPed();
	sElement* pVehMG = lsc_get_entity(L, 2);
#ifdef GTA_LIBERTY
	if (pPedMG != nil && pVehMG != nil && pPedMG->GetEntity() != nil && pVehMG->GetEntity() != nil && !pPlayer->EnteringCar())
	{
		pPlayer->ClearWeaponTarget();
		CPed* pPed = (CPed*)pPedMG->GetEntity();
		CVehicle* pVeh = (CVehicle*)pVehMG->GetEntity();
		pPed->m_pCollidingEntity = pVeh;
		pPed->SetObjective(eObjective::OBJECTIVE_ENTER_CAR_AS_DRIVER, pVeh);
		pPed->WarpPedIntoCar(pVeh);
		pPed->m_pCollidingEntity = nil;
		lua_pushboolean(L, true);
	}
	else {
		lua_pushboolean(L, false);
		debug("Warp Into Vehicle Failed !!!");
	}
	CStreaming::LoadSceneCollision(FindPlayerCoors());
	TheCamera.SetFadeColour(0, 0, 0);
	TheCamera.Fade(0.0f, FADE_OUT);
	TheCamera.ProcessFade();
	TheCamera.Fade(1.0f, FADE_IN);
#else
	if (pVehMG != nil)
	{
		if (pPedMG == nil || pPedMG->GetEntity() == nil || !Game.IsElementOwnerLocalPlayer(pVehMG) || pPlayer->EnteringCar())
		{
			Game.TransferEntity(pVehMG);
			lua_pushboolean(L, false);
			debug("Warp Into Vehicle Failed !!!");
		}
		else
		{
			CPed* pPed = (CPed*)pPedMG->GetEntity();
			CVehicle* pVeh = (CVehicle*)pVehMG->GetEntity();
			pPlayer->ClearWeaponTarget();
			pPed->m_pCollidingEntity = pVeh;
			pPed->SetObjective(eObjective::OBJECTIVE_ENTER_CAR_AS_DRIVER, pVeh);
			pPed->WarpPedIntoCar(pVeh);
			pPed->m_pCollidingEntity = nil;
			pPed->RestorePreviousState();
			lua_pushboolean(L, true);

			if (!CTheZones::IsPositionInCurrentGameLevel(FindPlayerCoors()))
				CStreaming::LoadSceneCollision(FindPlayerCoors());

			TheCamera.SetFadeColour(0, 0, 0);
			TheCamera.Fade(0.0f, FADE_OUT);
			TheCamera.ProcessFade();
			TheCamera.Fade(4.0f, FADE_IN);
		}
	}
	else
	{
		lua_pushboolean(L, false);
		debug("Warp Into Vehicle Failed !!!");
	}
#endif
	return 1;
}

int mp_lsn_PlayerId(lua_State* L) {
	lua_pushnumber(L, cMultiGame::Instance().LocalPlayerID());
	return 1;
}

static int mp_lsn_ActivatePlayers(lua_State* L) {
	CPlayerPed::ReactivatePlayerPed(0);
	return 0;
}

static int mp_lsn_Player(lua_State* L) {
	if (lsc_isPlayerUserData(L, 1))
		return 1;
	int nPlayerID = lua_tonumber(L, 1) ? lua_tonumber(L, 1) : cMultiGame::Instance().LocalPlayerID();
	lsn_push_player_id(L, nPlayerID);
	return 1;
}

static int mp_lsn_GameColour(lua_State* L) {
	int nBlipIndex = lua_tonumber(L, 1);
	CRGBA* pColor = cMultiGame::Instance().GetBlipColor(nBlipIndex);
	lua_pushnumber(L, RGB24_PACK(pColor->red, pColor->green, pColor->blue));
	return 1;
}

static int mp_lsn_IsPlayerPressingHorn(lua_State* L) {
	int nPlayerID = get_player_id(L);
	sPlayer* pPlayer = (sPlayer*)cMultiGame::Instance().GetEntityForHandle(nPlayerID, MG_ELEMENT_PLAYER_ID); // Game.GetPlayer()
	if (pPlayer == nil) return 0;
	bool bIsPressingHorn = pPlayer->isPressingHorn();
	lua_pushboolean(L, bIsPressingHorn);
	return 1;
}

static int mp_lsn_IsPlayerPressingExit(lua_State* L) {
	int nPlayerID = get_player_id(L);
	sPlayer* pPlayer = (sPlayer*)cMultiGame::Instance().GetEntityForHandle(nPlayerID, MG_ELEMENT_PLAYER_ID); // Game.GetPlayer()
	if (pPlayer == nil) return 0;
	bool bIsPressingExit = pPlayer->isPressingExitVehicle();
	lua_pushboolean(L, bIsPressingExit);
	return 1;
}

typedef bool(*locate_player_cb)(sPed* ped);

static int mp_lsn_locate_player(lua_State* L, locate_player_cb locator) {
	cMultiGame& Game = cMultiGame::Instance();
	bool bCheckZ = true;
#ifdef GTA_LIBERTY
	int nPeer = lsc_pop_peer_id_from_stack(L, 1, -1);
#else
	int nPeer = -1;
#endif
	CVector vPos, vMaxDist(2.5f, 2.5f, 2.5f);
	lsc_getVectorFromStack(vPos, L, 1, true);

	if (vPos.z <= MAP_Z_LOW_LIMIT)
		bCheckZ = false;

	if (lua_gettop(L) >= 2)
		lsc_getVectorFromStackNoEntity(vMaxDist, L, 2);


	uint32 nMaxSize = Max(Game.m_pNetSession->m_vPeers.size(), Game.LocalPlayerID() + 1);
	for (int32 nPeerID = 0; nPeerID < nMaxSize; nPeerID++) {
		if (!Game.IsSameGroup(nPeer, nPeerID)) continue;
		sPed* pPed = (sPed*)Game.GetEntityForHandle(nPeerID, eElementID::MG_ELEMENT_PLAYER_PED_ID); // Game.GetPlayerPed()
		if (pPed != nil && (locator == nil || locator(pPed))) {
			CVector vDist = pPed->GetSync().ped->GetMatrix().GetPosition() - vPos;
			if (
				fabsf(vDist.x) < vMaxDist.x && fabsf(vDist.y) < vMaxDist.y
				&& (!bCheckZ || fabsf(vDist.z) < vMaxDist.z)
				) {
				lsn_push_player_id(L, nPeerID);
				return 1;
			}
		}
	}
	return 0;
}

static int mp_lsn_LocatePlayer(lua_State* L) {
	return mp_lsn_locate_player(L, nil);
}

bool locate_in_car(sPed* ped) {
	if (ped == nil) return false;
	int16 nVehID = ped->GetSync().ped->GetVehicleID();
	if (nVehID == -1) return false;
	return cMultiGame::Instance().GetEntityForHandle(ped->GetOwner(), nVehID) != nil;
}

static int mp_lsn_LocatePlayerInCar(lua_State* L) {
	return mp_lsn_locate_player(L, locate_in_car);
}

bool locate_on_foot(sPed* ped) {
	if (ped == nil) return false;
	int16 nVehID = ped->GetSync().ped->GetVehicleID();
	if (nVehID == -1) return true;
	return cMultiGame::Instance().GetEntityForHandle(ped->GetOwner(), nVehID) == nil;
}

static int mp_lsn_LocatePlayerOnFoot(lua_State* L) {
	return mp_lsn_locate_player(L, locate_on_foot);
}

static int mp_lsn_PlayerControl(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
#ifdef GTA_LIBERTY
	int nPeer = lsc_pop_peer_id_from_stack(L, 1, 0xB00B5);
	bool bToggle = lua_toboolean(L, 1);
#else
	int nPeer = luaL_checknumber(L, 1);
	bool bToggle = lua_toboolean(L, 2);
#endif
	net::pckt_player_control packet{};
	packet.pckt_size = sizeof(net::pckt_player_control);
	packet.pckt_id = gtMP_PacketIDs.player_control.pckt_id;
	packet.player_control_toggle_type = false;
	packet.player_control_toggle_value = bToggle;
	if (Game.IsLocalPlayer(nPeer))
		on_recv_player_control(packet, 0, 0, false); // bug? true? nPeer
	Game.SendMessagePriority(packet, nPeer);
	return 0;
}

static int mp_lsn_PlayerCarControl(lua_State* L) {
	return mp_lsn_PlayerControl(L);
}

static int mp_lsn_PlayerForwardVec(lua_State* L) {
	if (!lsc_get_entity(L, 1)) return 0;
	CPlayerPed* pPlayer = FindPlayerPed();
	CVector vForward;
#ifdef GTA_LIBERTY
	if (!pPlayer->bInVehicle || pPlayer->m_pMyVehicle == nil)
#else
	if (!pPlayer->InVehicle() || pPlayer->m_pMyVehicle == nil)
#endif
		vForward = pPlayer->GetForward();
	else
		vForward = pPlayer->m_pMyVehicle->GetForward();
	float fDist = vForward.Magnitude2D();
	vForward.x = vForward.x / fDist;
	vForward.y = vForward.y / fDist;
	vForward.z = 0;
	lsc_pushVuVector(L, vForward);
	return 1;
}

static int mp_lsn_PlayerRightVec(lua_State* L) {
	if (!lsc_get_entity(L, 1)) return 0;
	CPlayerPed* pPlayer = FindPlayerPed();
	CVector vRight;
#ifdef GTA_LIBERTY
	if (!pPlayer->bInVehicle || pPlayer->m_pMyVehicle == nil)
#else
	if (!pPlayer->InVehicle() || pPlayer->m_pMyVehicle == nil)
#endif
		vRight = pPlayer->GetForward();
	else
		vRight = pPlayer->m_pMyVehicle->GetRight();
	float fDist = vRight.Magnitude2D();
	vRight.x = vRight.x / fDist;
	vRight.y = vRight.y / fDist;
	vRight.z = 0;
	lsc_pushVuVector(L, vRight);
	return 1;
}

static int mp_lsn_DisablePlayer(lua_State* L) {
	CPad::GetPad(0)->DisablePlayerControls = lua_toboolean(L, 1);
#ifdef FIX_BUGS
	return 0;
#else
	return 1;
#endif
}

static int mp_lsn_PlayerHealth(lua_State* L) {
	if (lsc_isPlayerUserData(L, 1))
	{
		int nPlayerID = lsc_getPlayer(L, 1);
		sPed* pPed = (sPed*)cMultiGame::Instance().GetEntityForHandle(nPlayerID, eElementID::MG_ELEMENT_PLAYER_PED_ID); // Game.GetPlayerPed()
		if (pPed)
		{
			lua_pushnumber(L, (float)pPed->GetSync().ped->m_nHealth);
			return 1;
		}
	}
	lua_pushnumber(L, 0.0f);
	return 1;
}

static int mp_lsn_TeamPlayers(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	if (!lua_isnumber(L, 1)) return 0;
	int32 nTeamID = (int32)lua_tonumber(L, 1);
	lua_newtable(L);
	int32 nTableIndex = 1;
	uint32 nMaxSize = Max(Game.m_pNetSession->m_vPeers.size(), Game.LocalPlayerID() + 1);
	for (int32 nPeerID = 0; nPeerID < nMaxSize; nPeerID++) {
		if (Game.GetPlayer(nPeerID) && Game.GetPlayerTeamID(nPeerID) == nTeamID) {
			lsn_push_player_id(L, nPeerID);
			lua_rawseti(L, -2, nTableIndex++);
		}
	}
	return 1;
}

static int mp_lsn_SetMultiplayerMissionCash(lua_State* L) {
	CWorld::Players[CWorld::PlayerInFocus].m_nMissionCashMG = lua_tonumber(L, 1);
	return 0;
}

static int mp_lsn_RepairPlayersVehicle(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	CVehicle* pVeh = FindPlayerVehicle();
	if (pVeh == nil) {
		lua_pushboolean(L, false);
		return 1;
	}

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
	else // TODO FIX_BUGS + other types vehicles + recheck on_recv_repair_car
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
#ifdef FIX_BUGS
	pVeh->m_fHealth = pVeh->m_fMaxHealth;
#else
	pVeh->m_fHealth = 1000.0f;
#endif
#endif
	pVeh->m_nDoorLock = eCarLock::CARLOCK_UNLOCKED;
	lua_pushboolean(L, true);
	return 1;
}

static int mp_lsn_SetPlayersVehicleOnFire(lua_State* L) {
	bool bWasSetOnFire = false;
	CVehicle* pVeh = FindPlayerVehicle();
	if (pVeh) {
		pVeh->m_fHealth = 249.0f;
		bWasSetOnFire = true;
	}
	lua_pushboolean(L, bWasSetOnFire);
	return 1;
}

static int mp_lsn_BurstTyresOnPlayersVehicle(lua_State* L) {
	bool bWasBursted = false;
	CVehicle* pVeh = FindPlayerVehicle();
	if (pVeh) {
		if (pVeh->IsBike()) {
			pVeh->BurstTyre(CAR_PIECE_WHEEL_LF, true);
			pVeh->BurstTyre(CAR_PIECE_WHEEL_LR, true);
		}
		else {
			pVeh->BurstTyre(CAR_PIECE_WHEEL_LF, true);
			pVeh->BurstTyre(CAR_PIECE_WHEEL_LR, true);
			pVeh->BurstTyre(CAR_PIECE_WHEEL_RF, true);
			pVeh->BurstTyre(CAR_PIECE_WHEEL_RR, true);
		}
		bWasBursted = true;
	}
	lua_pushboolean(L, bWasBursted);
	return 1;
}

static int mp_lsn_IsLocalPlayerInVehicle(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	sPed* pPed = Game.GetPlayerPed(MP_HOST_INDEX);
	bool bWasFound = locate_in_car(pPed);
	lua_pushboolean(L, bWasFound);
	return 1;
}

static int mp_lsn_LocalPlayerMsSinceLastFired(lua_State* L) {
	int nLastFireMs = CTimer::GetTimeInMilliseconds() - FindPlayerPed()->m_nPadDownPressedInMilliseconds;
	lua_pushnumber(L, nLastFireMs);
	return 1;
}

static int mp_lsn_GivePlayerHandlingPowerup(lua_State* L) {
	FindPlayerPed()->GiveCarHandling(30000); // for beta mode "Collect The Gold GS_G11" ??
	return 0;
}

static int mp_lsn_GiveLocalPlayerNoRadarForEnemy(lua_State* L) {
	FindPlayerPed()->bNoRadarForEnemy = true;
	return 0;
}

static int mp_lsn_RemoveLocalPlayerNoRadarForEnemy(lua_State* L) {
#if defined(GTA_LIBERTY) && !defined(FIX_BUGS)
	FindPlayerPed()->m_nPowerups = 0x0; // what? // check on_recv_set_player_blip_visible_state
#else
	FindPlayerPed()->bNoRadarForEnemy = false;
#endif
	return 0;
}

static int mp_lsn_GetLocalPlayersVehicle(lua_State* L) {
	CPlayerPed* pPlayer = FindPlayerPed();
#ifdef GTA_LIBERTY
	if (!pPlayer->bInVehicle) return 0;
#else
	if (!pPlayer->InVehicle()) return 0;
#endif
	CVehicle* pVeh = pPlayer->m_pMyVehicle;
	sVehicle* pVehMG = cMultiGame::Instance().GetElementFromEntity<sVehicle*>(pVeh);
	if (pVehMG && pVehMG->HasCapability(sVehicle::Capability())) {
		lsc_register_entity(L, pVehMG);
		return 1;
	}
	return 0;
}

int mp_lsn_InitialSpawnPoint(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	int nPlayerID = get_player_id(L);
	lua_pushnumber(L, Game.GetSpawnPointFromPlayer(nPlayerID));
	return 1;
}

static int mp_lsn_IsLocalPlayerEnteringAVehicle(lua_State* L) {
	lua_pushboolean(L, FindPlayerPed()->EnteringCar());
	return 1;
}

static int mp_lsn_ClearLocalPlayerLockOn(lua_State* L) {
	FindPlayerPed()->ClearWeaponTarget();
	return 0;
}

static int mp_lsn_IsAreaClearOfPlayers(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	CVector vPos;
	bool bIsClear = true;
	lsc_getVectorFromStack(vPos, L, 1, true);
	uint32 nMaxSize = Max(Game.m_pNetSession->m_vPeers.size(), Game.LocalPlayerID() + 1);
	for (int32 nPeerID = 0; nPeerID < nMaxSize; nPeerID++) {
		if (Game.GetPlayer(nPeerID) == nil) continue;
		sPed* pPed = (sPed*)Game.GetEntityForHandle(nPeerID, eElementID::MG_ELEMENT_PLAYER_PED_ID); // Game.GetPlayerPed()
		if (pPed == nil) continue;
		CVector vPlayerPos = pPed->GetSync().ped->GetMatrix().GetPosition();
#ifdef GTA_LIBERTY
		if (Distance(vPos, vPlayerPos) < 3.0f)
#else
		if (Distance(vPos, vPlayerPos) < 5.0f)
#endif
		{
			bIsClear = false;
			break;
		}
	}
	lua_pushboolean(L, bIsClear);
	return 1;
}

static int mp_lsn_ToggleLocalPlayerControls(lua_State* L) {
	const bool bToggle = lua_toboolean(L, 1);
	if (bToggle)
		CPad::GetPad(0)->SetEnablePlayerControls(PLAYERCONTROL_PLAYERINFO);
	else
		CPad::GetPad(0)->SetDisablePlayerControls(PLAYERCONTROL_PLAYERINFO);
	return 0;
}

static int mp_lsn_IsLocalPlayerDrowning(lua_State* L) {
#ifdef GTA_LIBERTY
	const bool bIsDrowning = FindPlayerPed()->bIsDrowning;
#else
	CPlayerPed* pPlayerPed = FindPlayerPed(); // useless
	const bool bIsDrowning = FindPlayerPed()->m_nPedState == PED_DROWN;
#endif
	if (bIsDrowning) debug("***************** Drowning");
	lua_pushboolean(L, bIsDrowning);
	return 1;
}

static int mp_lsn_MapPlayerColourToCarColour(lua_State* L) {
#ifdef GTA_LIBERTY
	const int nPlayerColor = lua_tonumber(L, 1);
	int nVehColor = 0;
	switch (nPlayerColor)
	{
		case 1: nVehColor = 3; break;
		case 2: nVehColor = 7; break;
		case 3: nVehColor = 44; break;
		case 4: nVehColor = 85; break;
		case 5: nVehColor = 5; break;
		case 6: nVehColor = 21; break;
		case 7: nVehColor = 65; break;
	}
#else
	int nId = lua_tonumber(L, 1);
	CRGBA* pColor = cMultiGame::Instance().GetPlayerColor(nId);
	uint32 nVehColor = CRGBA_PACK(pColor->red, pColor->green, pColor->blue, pColor->alpha);
#endif
	lua_pushnumber(L, nVehColor);
	return 1;
}

#ifndef GTA_LIBERTY
static int mp_lsn_IsPlayerCreationQueued(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	lua_pushboolean(L, Game.GetPlayerCreationQueued());

	// I can't apply FIX_BUGS here because due to crooked leeds scripts it will run wait yield not in coroutine
	// but from under the main C context of running Main (cLScript::RunMainScript()) https://prnt.sc/SETYP2iFNTZ8
//#ifdef FIX_BUGS
//	return 1;
//#else
	return 0; // skip result (orig bug)
//#endif
}
#endif

static const luaL_reg playerlibs[] = {
	{"AllPlayers",                       mp_lsn_AllPlayers},
	{"AllPlayersExceptMe",               mp_lsn_AllPlayersExceptMe},
	{"CreatePlayer",                     mp_lsn_CreatePlayer},
	{"WarpPlayerIntoCar",                mp_lsn_WarpPlayerIntoCar},
	{"PlayerId",                         mp_lsn_PlayerId},
	{"ActivatePlayers",                  mp_lsn_ActivatePlayers},
	{"Player",                           mp_lsn_Player},
	{"PlayerName",                       mp_lsn_PlayerName},
	{"IsConnected",                      mp_lsn_IsConnected},
	{"PlayerCar",                        mp_lsn_PlayerCar},
	{"GameColour",                       mp_lsn_GameColour},
	{"PlayerColour",                     mp_lsn_PlayerColour},
	{"PlayerTeamColour",                 mp_lsn_PlayerTeamColour},
	{"PlayerPosition",                   mp_lsn_GetPlayerPosition},
	{"IsPlayerPressingHorn",             mp_lsn_IsPlayerPressingHorn},
	{"IsPlayerPressingExit",             mp_lsn_IsPlayerPressingExit},
	{"LocatePlayer",                     mp_lsn_LocatePlayer},
	{"LocatePlayerInCar",                mp_lsn_LocatePlayerInCar},
	{"LocatePlayerOnFoot",               mp_lsn_LocatePlayerOnFoot},
	{"PlayerControl",                    mp_lsn_PlayerControl},
	{"PlayerCarControl",                 mp_lsn_PlayerCarControl},
	{"StartDown",                        mp_lsn_StartDown},
	{"LeftDown",                         mp_lsn_LeftDown},
	{"RightDown",                        mp_lsn_RightDown},
	{"UpDown",                           mp_lsn_UpDown},
	{"DownDown",                         mp_lsn_DownDown},
	{"CrossDown",                        mp_lsn_CrossDown},
	{"CircleDown",                       mp_lsn_CircleDown},
	{"TriangleDown",                     mp_lsn_TriangleDown},
	{"PlayerForwardVec",                 mp_lsn_PlayerForwardVec},
	{"PlayerRightVec",                   mp_lsn_PlayerRightVec},
	{"SetPosition",                      mp_lsn_SetPlayerPosition},
	{"SetHeading",                       mp_lsn_SetHeading},
	{"DisablePlayer",                    mp_lsn_DisablePlayer},
	{"PlayerHealth",                     mp_lsn_PlayerHealth},
	{"TeamPlayers",                      mp_lsn_TeamPlayers},
	{"SetPlayerRadarBlipShortRange",     mp_lsn_SetRadarBlipShortRange},
	{"SetPlayerRadarIcon",               mp_lsn_SetRadarBlipIcon},
	{"SetPlayerBlipVisibleState",        mp_lsn_SetRadarBlipVisibleState},
	{"SetMultiplayerMissionCash",        mp_lsn_SetMultiplayerMissionCash},
	{"RepairPlayersVehicle",             mp_lsn_RepairPlayersVehicle},
	{"SetPlayersVehicleOnFire",          mp_lsn_SetPlayersVehicleOnFire},
	{"BurstTyresOnPlayersVehicle",       mp_lsn_BurstTyresOnPlayersVehicle},
	{"IsLocalPlayerInVehicle",           mp_lsn_IsLocalPlayerInVehicle},
	{"LocalPlayerMsSinceLastFired",      mp_lsn_LocalPlayerMsSinceLastFired},
	{"GivePlayerHandlingPowerup",        mp_lsn_GivePlayerHandlingPowerup},
	{"GiveLocalPlayerNoRadarForEnemy",   mp_lsn_GiveLocalPlayerNoRadarForEnemy},
	{"RemoveLocalPlayerNoRadarForEnemy", mp_lsn_RemoveLocalPlayerNoRadarForEnemy},
	{"GetLocalPlayersVehicle",           mp_lsn_GetLocalPlayersVehicle},
	{"InitialSpawnPoint",                mp_lsn_InitialSpawnPoint},
	{"IsLocalPlayerEnteringAVehicle",    mp_lsn_IsLocalPlayerEnteringAVehicle},
	{"SetPlayerHealth",                  mp_lsn_SetPedHealth},
	{"ClearLocalPlayerLockOn",           mp_lsn_ClearLocalPlayerLockOn},
	{"IsAreaClearOfPlayers",             mp_lsn_IsAreaClearOfPlayers},
	{"IsPlayerDead",                     mp_lsn_IsPlayerDead},
	{"ToggleLocalPlayerControls",        mp_lsn_ToggleLocalPlayerControls},
	{"IsLocalPlayerDrowning",            mp_lsn_IsLocalPlayerDrowning},
	{"MapPlayerColourToCarColour",       mp_lsn_MapPlayerColourToCarColour},
#ifndef GTA_LIBERTY
	{"IsPlayerCreationQueued",           mp_lsn_IsPlayerCreationQueued},
#endif
	{NULL, NULL}
};
VALIDATE_LUA_LIB(playerlibs, (59 + 1), (58 + 1));

void lscript_open_player() {
	cMultiGame& Game = TheMPGame;
	cLWrapper& wrapper = cLWrapper::Instance();
#define REGISTER_PACKET(id, callback) Game.RegisterPacket(id, new cPacketDispatcher(callback));

	wrapper.CreateLibrary(player_ped_libs, "PlayerId");
	REGISTER_PACKET(gtMP_PacketIDs.player_control.pckt_id, &on_recv_player_control); // 41 lcs vcs
	REGISTER_PACKET(gtMP_PacketIDs.set_position.pckt_id, &on_recv_player_set_position); // 42 lcs vcs
	REGISTER_PACKET(gtMP_PacketIDs.set_heading.pckt_id, &on_recv_player_set_heading); // 43 lcs vcs
	wrapper.CreateGlobalLibrary(playerlibs, nil);
#undef REGISTER_PACKET
}