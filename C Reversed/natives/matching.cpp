/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"
#include "Frontend.h"
#include "World.h"
#include "Camera.h"
#include "Timer.h"
#include "ModelIndices.h"
#include "Pad.h"
#include "PlayerPed.h"
#include "singletonManager.h"
#ifndef GTA_LIBERTY
#include "Empire.h"
#endif

#include "multiplayer/public.h"
#include "multiplayer/MultiGame.h"
#include "multiplayer/natives/public.h"
#include "multiplayer/events/public.h"
#include "multiplayer/LScript.h"

#define NUM_IRON_MAN_LEG_CHANGES (1)


inline bool hasCutSceneSynced(cMultiGame& Game, uint8 id) {
	return Game.m_nMaskCutsceneSync & BIT(id);
}

int lsn_waiter(lua_State* L) {
	if (!TheMPGame.IsOpen())
		return 0;

	uint16 nCurTime = TheMPGame.m_pNetSession->m_nCurTime;
	int32 nRemain = nCurTime - lua_tonumber(L, lua_upvalueindex(1));
	if (nRemain < 0) return 0;
	lua_pushboolean(L, true);
	lua_pushnumber(L, nRemain);
	return 2;
}

int mp_lsn_HostName(lua_State* L) {
	cAdhoc& Adhoc = TheAdhoc;
	base::string playerName;
	tMacAddr& pHostMacAddr = Adhoc.GetPlayerMacAddress();
	Adhoc.GetPlayerNameFromMacAddr(playerName, pHostMacAddr);
	lua_pushstring(L, playerName.c_str());
	return 1;
}

int mp_lsn_DbgPrintAllOptions(lua_State* L) {
	// VCS: Empty
	cMultiGame& pGame = TheMPGame;
	debug("*********PARAMETERS**********\n");
	debug("Game Type:\t%d\n", pGame.GetGameType());
	debug("Location:\t%d\n", pGame.GetGameLocation());
	debug("ScoreLimit:\t%d\n", pGame.GetScoreLimit());
	debug("TimeLimit:\t%d\n", pGame.GetTimeLimit());
	debug("Team Game:\t%d\n", pGame.eTDMStyle); // bit 1
	debug("Powerup On:\t%d\n", pGame.bPowerUpOn); // bit 0
	debug("RacePowerup:\t%d\n", pGame.bRacePowerUpOn); // bit 2
	debug("Race Revr:\t%d\n", pGame.bRaceRevr); // bit 4
#ifndef GTA_LIBERTY
	debug("bBit_8:\t%d\n", pGame.bBit_8); // bit 3
	debug("bBit_20:\t%d\n", pGame.bBit_20); // bit 5
	debug("Vip Team 2:\t%d\n", pGame.bIsVipTeamTeam2); // bit 6
	debug("bBit_80:\t%d\n", pGame.bBit_80); // bit 7
#endif
	debug("Ambient Car Bank:\t%d\n", pGame.m_nAmbientCarBank);
	debug("Ambient Ped Bank:\t%d\n", pGame.m_nAmbientPedBank);
	debug("Skip Cut Scene:\t%d\n", pGame.GetCutsceneSkipEnabled());
	debug("*****************************\n");
	return 0;
}

int mp_lsn_ServerName(lua_State* L) {
#ifdef GTA_PSP
	sceDisplayWaitVblankStart();
#else
	// PC? wait end draw pixeldata, in the end coreloop
#endif
	cAdhoc& Adhoc = TheAdhoc;
	base::string serverName;
	tLobbyRemoteInfo* pInfo = Adhoc.GetMatchingInfo(MP_HOST_INDEX);
	Adhoc.GetPlayerNameFromMacAddr(serverName, pInfo->m_HostPeerData.peerAddr.mac);
	lua_pushstring(L, serverName.c_str());
	return 1;
}

int mp_lsn_IsAdhocConnected(lua_State* L) {
#ifdef GTA_PSP
	sceDisplayWaitVblankStart();
#else
	// PC? wait end draw pixeldata, in the end coreloop
#endif
	lua_pushboolean(L, TheAdhoc.bConnEvent);
	return 1;
}

int mp_lsn_IsServer(lua_State* L) {
	lua_pushboolean(L, TheAdhoc.IsHost());
	return 1;
}

int mp_lsn_TargetPlayer(lua_State* L) {
	cMultiGame& Game = TheMPGame;
	if (lua_isnumber(L, 1)) {
		Game.SetTargetPlayer(lua_tonumber(L, 1), true);
		return 0;
	}
	lua_pushnumber(L, Game.GetTargetPlayer());
	return 1;
}

int mp_lsn_GameTimeUp(lua_State* L) {
	lua_pushboolean(L, TheMPGame.IsGameTimeUp());
	return 1;
}

int mp_lsn_GetGameSeconds(lua_State* L) {
	lua_pushnumber(L, TheMPGame.GetTimeSeconds());
	return 1;
}

int mp_lsn_UpdateGameTime(lua_State* L) {
	cMultiGame& Game = TheMPGame;
	Game.m_bUpdateGameTime = lua_toboolean(L, 1);
#ifdef FIX_BUGS
	return 0;
#else
	return 1; // forgotten getter?
#endif
}

int mp_lsn_GameTimeMinutes(lua_State* L) {
	lua_pushnumber(L, TheMPGame.GetTimeMinutes());
	return 1;
}

int mp_lsn_GameTimeSeconds(lua_State* L) {
	lua_pushnumber(L, TheMPGame.GetTimeSeconds());
	return 1;
}

int mp_lsn_SetGameTime(lua_State* L) {
	cMultiGame& Game = TheMPGame;
	int32 nTime = lua_tonumber(L, 1);
	Game.m_nTimeLimit = nTime;
	Game.SetTimeMinutes(nTime);
	Game.SetTimeSeconds(0);
#ifdef FIX_BUGS
	return 0;
#else
	return 1; // forgotten getter?
#endif
}

int mp_lsn_GameType(lua_State* L) {
	cMultiGame& Game = TheMPGame;
	if (lua_isnumber(L, 1)) {
		Game.SetGameType((eGameType)lua_tonumber(L, 1));
		return 0;
	}
	lua_pushnumber(L, (int32)Game.GetGameType());
	return 1;
}

int mp_lsn_GameLocation(lua_State* L) {
	cMultiGame& Game = TheMPGame;
	if (lua_isnumber(L, 1)) {
		Game.SetGameLocation((eGameLocation)lua_tonumber(L, 1));
		return 0;
	}
	lua_pushnumber(L, (int32)Game.GetGameLocation());
	return 1;
}

int mp_lsn_GameScoreLimit(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	if (lua_isnumber(L, 1)) {
		int32 nValue = lua_tonumber(L, 1);
		switch (Game.GetGameType()) {
		case eGameType::CTF: Game.SetCTFScoreLimit(nValue); break;
		case eGameType::SIXTYSECONDS: Game.SetCashTarget(nValue); break;
		default: Game.SetScoreLimit(nValue); break;
		}
		return 0;
	}
	int32 nValue = 0;
	switch (Game.GetGameType()) {
	case eGameType::CTF: nValue = Game.GetCTFScoreLimit(); break;
	case eGameType::SIXTYSECONDS: nValue = Game.GetCashTarget(); break;
	default: nValue = Game.GetScoreLimit(); break;
	}
	lua_pushnumber(L, nValue);
	return 1;
}


int mp_lsn_GameTimeLimit(lua_State* L) {
	cMultiGame& Game = TheMPGame;
	if (lua_isnumber(L, 1)) {
		Game.SetTimeLimit(lua_tonumber(L, 1));
		return 0;
	}
	lua_pushnumber(L, Game.GetTimeLimit());
	return 1;
}

int mp_lsn_GameElapsedMs(lua_State* L) {
	cMultiGame& Game = TheMPGame;
	if (lua_isnumber(L, 1)) {
		Game.SetGameElapsedMs(lua_tonumber(L, 1));
		return 0;
	}
	lua_pushnumber(L, Game.GetGameElapsedMs());
	return 1;
}

int mp_lsn_IsTeamGame(lua_State* L) {
	cMultiGame& Game = TheMPGame;
	if (lua_isboolean(L, 1)) {
		Game.eTDMStyle = lua_toboolean(L, 1) ? eTDMStyle::GANG_WAR : eTDMStyle::FFA;
		return 0;
	}
	lua_pushboolean(L, Game.eTDMStyle == eTDMStyle::GANG_WAR);
	return 1;
}

int mp_lsn_UsePowerups(lua_State* L) {
	lua_pushboolean(L, TheMPGame.bPowerUpOn);
	return 1;
}

int mp_lsn_UseRacePowerups(lua_State* L) {
	lua_pushboolean(L, TheMPGame.bRacePowerUpOn);
	return 1;
}

int mp_lsn_RaceReverse(lua_State* L) {
	lua_pushboolean(L, TheMPGame.bRaceRevr);
	return 1;
}

int mp_lsn_RaceId(lua_State* L) {
	cMultiGame& Game = TheMPGame;
	if (lua_isnumber(L, 1)) {
		Game.m_nScenarioOrRaceTrackID = lua_tonumber(L, 1);
		return 0;
	}
	lua_pushnumber(L, Game.m_nScenarioOrRaceTrackID);
	return 1;
}

int mp_lsn_RaceCar(lua_State* L) {
	cMultiGame& Game = TheMPGame;
#ifndef GTA_LIBERTY
	if ((Game.GetGameType() == eGameType::MULTIRACE) && (Game.GetCTFScoreLimit() == 2))
		lua_pushnumber(L, MI_JETSKI);
	else
#endif
		lua_pushnumber(L, Game.m_nRaceCarID);
	return 1;
}

#ifndef GTA_LIBERTY
int mp_lsn_IsIronManRace(lua_State* L) {
	cMultiGame& Game = TheMPGame;
	lua_pushboolean(L, (Game.GetGameType() == eGameType::MULTIRACE && Game.GetCTFScoreLimit() == 1));
	return 1;
}
int mp_lsn_IsJetskiRace(lua_State* L) {
	cMultiGame& Game = TheMPGame;
	lua_pushboolean(L, (Game.GetGameType() == eGameType::MULTIRACE && Game.GetCTFScoreLimit() == 2));
	return 1;
}
int mp_lsn_NumIronManLegChanges(lua_State* L) {
	lua_pushnumber(L, NUM_IRON_MAN_LEG_CHANGES);
	return 1;
}
#endif

int mp_lsn_TankModel(lua_State* L) {
	lua_pushnumber(L, TheMPGame.m_nTankModelID);
	return 1;
}

#ifndef GTA_LIBERTY
int mp_lsn_HunterModel(lua_State* L) {
#ifdef FIX_BUGS
	assert(TheMPGame.m_nHunterModelID == MI_HUNTER);
	lua_pushnumber(L, TheMPGame.m_nHunterModelID); // did they forget about this field ?
#else
	lua_pushnumber(L, MI_HUNTER);
#endif
	return 1;
}
#endif

int mp_lsn_NoFuckingCutscene(lua_State* L) {
	lua_pushboolean(L, TheMPGame.GetCutsceneSkipEnabled());
	return 1;
}

int mp_lsn_TeamColour(lua_State* L) {
	CRGBA* pColor = TheMPGame.GetColor(lua_tonumber(L, 1));
	uint32 nColor = RGB24_PACK(pColor->red, pColor->green, pColor->blue);
	lua_pushnumber(L, nColor);
	return 1;
}

int mp_lsn_TeamName(lua_State* L) {
	const char* gangName = TheMPGame.GetGangName(lua_tonumber(L, 1));
	lua_pushstring(L, gangName);
	return 1;
}

#ifndef GTA_LIBERTY
int mp_lsn_VipTeam(lua_State* L) {
	cMultiGame& Game = TheMPGame;
	lua_pushnumber(L, Game.bIsVipTeamTeam2);
	return 1;
}
#endif

int mp_lsn_EndGame(lua_State* L) {
	gIsMultiplayerGame = false;
	TheAdhoc.Disconnect();
#ifdef GTA_LIBERTY
	FrontEndMenuManager->LoadAllTextures();
	// TODO
	TODO();
	TODO();
	TODO();
#endif
#ifdef FIX_BUGS
	return 0;
#else
	return 1;
#endif
}

int mp_lsn_StartGame(lua_State* L) {
	TheMPGame.m_bIsRunning = true;
	return 0;
}

int mp_lsn_DefendingTeam(lua_State* L) {
	cMultiGame& Game = TheMPGame;
	if (lua_isnumber(L, 1)) {
		Game.SetDefendingTeamID(lua_tonumber(L, 1));
		return 0;
	}
	lua_pushnumber(L, Game.GetDefendingTeamID());
	return 1;
}

int mp_lsn_ShowingCommentary(lua_State* L) {
	cMultiGame& Game = TheMPGame;
	if (lua_isboolean(L, 1)) {
		Game.m_bShowingCommentary = lua_toboolean(L, 1);
		return 0;
	}
	lua_pushboolean(L, Game.m_bShowingCommentary);
	return 1;
}

// unused? beta?
class cNetQuality : public base::cSingletonBase
{
public:
	float m_fMinLatency;
	float m_fMaxLatency;
	float m_fPacketLoss;
	int m_nUnk;

	cNetQuality()
	{
		m_fMinLatency = 0.01f; // 10ms
		m_fMaxLatency = 0.02f; // 20ms
		m_fPacketLoss = 0.2f; // 20% kek
		m_nUnk = 100000; // timeout? max pckt cnt?
	}
};
cNetQuality* base::cSingleton<cNetQuality>::mspInstance = nil;
#define NetQuality (*base::cSingleton<cNetQuality>::Instance())

int mp_lsn_NetMinLatency(lua_State* L) {
	if (lua_gettop(L)) NetQuality.m_fMinLatency = luaL_checknumber(L, 1);
	lua_pushnumber(L, NetQuality.m_fMinLatency);
	return 1;
}

int mp_lsn_NetMaxLatency(lua_State* L) {
	if (lua_gettop(L)) NetQuality.m_fMaxLatency = luaL_checknumber(L, 1);
	lua_pushnumber(L, NetQuality.m_fMaxLatency);
	return 1;
}

int mp_lsn_NetPacketLoss(lua_State* L) {
	if (lua_gettop(L)) NetQuality.m_fPacketLoss = luaL_checknumber(L, 1);
	lua_pushnumber(L, NetQuality.m_fPacketLoss);
	return 1;
}


int mp_lsn_Multilag(lua_State* L) {
	cMultiGame& Game = TheMPGame;
	if (lua_gettop(L)) Game.m_nLagValue = luaL_checknumber(L, 1);
	lua_pushnumber(L, Game.m_nLagValue);
	return 1;
}

int mp_lsn_WaitFunc(lua_State* L) {
	if (!TheMPGame.IsOpen())
		return 0;

	uint16 nCurTime = TheMPGame.m_pNetSession->m_nCurTime;
	float fWaitTime = lua_tonumber(L, 1);
	lua_pushnumber(L, nCurTime + (fWaitTime * 0.06f));
	lua_pushcclosure(L, lsn_waiter, 1); // https://www.lua.org/pil/27.3.3.html
#ifdef GTA_LIBERTY
	return 1; // closure, probably bug in lcs
#else
	return 2; // closure + time
#endif
}

int mp_lsn_Wait(lua_State* L) {
	if (!lua_isfunction(L, 1)) mp_lsn_WaitFunc(L);
	return lua_yield(L, 1);
}

int mp_lsn_FindGroundZFor3DCoord(lua_State* L) {
	CVector pos;
	lsc_getVectorFromStack(pos, L, 1, true);
#ifdef GTA_LIBERTY
	float posZ = CWorld::FindGroundZFor3DCoord(pos.x, pos.y, pos.z, nil);
#else
	bool bPosZNotFound = lua_isboolean(L, 2) ? lua_toboolean(L, 2) : true;
	bool bFound = false;
	float posZ = CWorld::FindGroundZFor3DCoord(pos.x, pos.y, pos.z, &bFound);
	if (!bFound)
		posZ = bPosZNotFound ? pos.z : 10000.0f;
#endif
	lua_pushnumber(L, posZ);
	return 1;
}

int mp_lsn_CutScenePlaying(lua_State* L) {
	lua_pushboolean(L, TheCamera.m_WideScreenOn);
	return 1;
}

int mp_lsn_renderPauseScreenStuff(lua_State* L) {
	gbMP_DrawPauseScreen = lua_toboolean(L, 1);
	return 0;
}

int mp_lsn_renderPauseScreenStuffNoBox(lua_State* L) {
	gbMP_DrawPauseScreen = lua_toboolean(L, 1);
	gbMP_DrawPauseScreenNoBox = lua_toboolean(L, 1);
	return 0;
}

#ifndef GTA_LIBERTY
int mp_lsn_DisablePauseScreenBackgroundBoxOnly(lua_State* L) {
	gbMP_DrawPauseScreenNoBox = lua_toboolean(L, 1);
	return 0;
}
#endif

int mp_lsn_renderHudExtras(lua_State* L) {
	gbMP_RenderHudExtras = lua_toboolean(L, 1);
	return 0;
}

int mp_lsn_setPauseScreenSelection(lua_State* L) {
	gnMP_PauseScreenSelection = lua_tonumber(L, 1);
	return 0;
}

int mp_lsn_showHelp(lua_State* L) {
	gbMP_HudShowHelp = lua_toboolean(L, 1);
	return 0;
}

int mp_lsn_getExpectedNumberofPlayers(lua_State* L) {
	lua_pushnumber(L, TheAdhoc.GetNumberOfConnectedPlayers());
	LUA_LOG(1, "mp_lsn_getExpectedNumberofPlayers GetNumberOfConnectedPlayers() %d\n", TheAdhoc.GetNumberOfConnectedPlayers()); // custom
	return 1;
}

int mp_lsn_teamGameEveryoneIn(lua_State* L) {
	TheMPGame.m_bTeamEveryoneIn = true;
	return 0;
}

int mp_lsn_HasPlayerEnteredGame(lua_State* L) {
	lua_pushboolean(L, TheMPGame.HasPlayerJoinedGame(lua_tonumber(L, 1)));
	return 1;
}

int mp_lsn_UseSuperBrakeOnPause(lua_State* L) {
	gMultiplayerSuperBrakeOnPause = lua_toboolean(L, 1);
	return 0;
}

int mp_lsn_syncCutscene(lua_State* L) {
	cMultiGame& Game = TheMPGame;
	if (CTimer::GetTimeInMilliseconds() - Game.GetCurTime() >= 5000) {
		debug("***** Cutscene Sync Time Out !!!\n");
		lua_pushboolean(L, true);
		return 1;
	}
	if (!hasCutSceneSynced(Game, Game.LocalPlayerID())) {
		debug("Sending cutscene msg\n");
		net::pckt_msg_ready_for_cutscene packet;
		packet.pckt_size = sizeof(net::pckt_msg_ready_for_cutscene);
		packet.pckt_id = gtMP_PacketIDs.msg_ready_for_cutscene.pckt_id;
		Game.SendMessagePriority(packet, BROADCAST_PEER_GROUPID);
		Game.SetCutscenePlaying(Game.LocalPlayerID());
	}
	int32 nPlayers = TheAdhoc.GetNumberOfConnectedPlayers();
	debug("***** Peers Ready Mask == 0x%x\n", Game.m_nMaskCutsceneSync);
	for (int32 id = 0; id < nPlayers; id++) {
		if (hasCutSceneSynced(Game, id))
			continue;
		lua_pushboolean(L, false);
		return 1;
	}
	lua_pushboolean(L, true);
	return 1;
}

#ifndef GTA_LIBERTY
int mp_lsn_GetRedTeamPeerGroupId(lua_State* L) {
	lua_pushnumber(L, PeerManager.m_nTeamAPeerGroupId);
	return 1;
}

int mp_lsn_GetBlueTeamPeerGroupId(lua_State* L) {
	lua_pushnumber(L, PeerManager.m_nTeamBPeerGroupId);
	return 1;
}

int mp_lsn_GetEveryonePeerGroupId(lua_State* L) {
	lua_pushnumber(L, BROADCAST_PEER_GROUPID);
	return 1;
}

int mp_lsn_PrintAllPeerGroups(lua_State* L) {
	TheMPGame.m_pNetSession->PrintAllPeerGroups();
	return 0;
}

int mp_lsn_CreateAndSyncPeerGroup(lua_State* L) {
	cMultiGame& Game = TheMPGame;
	uint8 group = lua_tonumber(L, 1);
	uint8 mask = lua_tonumber(L, 2);
	net::pckt_sync_peer_group packet;
	packet.pckt_size = sizeof(net::pckt_sync_peer_group);
	packet.pckt_id = gtMP_PacketIDs.sync_peer_group.pckt_id;
	packet.group = group;
	packet.peer_mask = mask;
	on_recv_sync_peer_group(packet, Game.LocalPlayerID(), 0, true);
	Game.SendMessagePriority(packet, BROADCAST_PEER_GROUPID);
	return 0;
}

int mp_lsn_CreateAndSyncEveryoneExceptMePeerGroup(lua_State* L) {
	cMultiGame& Game = TheMPGame;
	cPeerManager& PeerMgr = PeerManager; // vcs
	int32 group = lua_tonumber(L, 1);
	uint8 mask = 0xFF;
	for (int32 i = 0; i < PeerMgr.m_vPlayers.size(); ++i) {
		sPeerState* peer = PeerMgr.GetPeerAt(i);
		if (peer->m_nID != Game.LocalPlayerID() && PeerMgr.IsPeerConnected(peer->m_nID)) continue;
		if (peer->m_nID > MP_NUM_PEERS)
			assert(false && "bitset");
		//mask &= ~BIT((peer->m_nID & 0x1F));
		//mask &= ~BIT(ldb(0, MP_NUM_PEERS - 2, peer->m_nID));
		mask &= ~BIT( (peer->m_nID & (BIT(MP_NUM_PEERS - 2) - 1)) ); // -2(-1 num to lastmax, -1 local) -1(normalize)
	}
	net::pckt_sync_peer_group packet;
	packet.pckt_size = sizeof(net::pckt_sync_peer_group);
	packet.pckt_id = gtMP_PacketIDs.sync_peer_group.pckt_id;
	packet.group = group;
	packet.peer_mask = mask;
	on_recv_sync_peer_group(packet, Game.LocalPlayerID(), 0, true);
	Game.SendMessagePriority(packet, BROADCAST_PEER_GROUPID);
	lua_pushnumber(L, group);
	return 1;
}

int mp_lsn_GetDiskPrepend(lua_State* L) {
#ifdef GTA_PSP
	if (gbIsUsingLUASource)
		lua_pushstring(L, PSP_DEV_USERDIR);
	else
		lua_pushstring(L, PSP_USERDIR);
#else
	lua_pushstring(L, "./");
#endif
	return 1;
}

int mp_lsn_SetEmpireSiteSize(lua_State* L) {
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2)) return 0;
	int32 empireIndex = lua_tonumber(L, 1) - 1; // num to array index
	int32 scaleLevel = lua_tonumber(L, 2);
	TheEmpire->GetEmpireInfo(empireIndex)->SetScaleLevel(scaleLevel);
	return 0;
}

int mp_lsn_SetEmpireSiteType(lua_State* L) {
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2)) return 0;
	int32 empireIndex = lua_tonumber(L, 1) - 1; // num to array index
	int32 businessType = lua_tonumber(L, 2);
	TheEmpire->GetEmpireInfo(empireIndex)->SetBusinessType(businessType);
	return 0;
}

int mp_lsn_CreateLuaObject(lua_State* L) {
	cMultiGame& Game = TheMPGame;
	if (!lua_isnumber(L, 1))
		return 0;

	int32 objectId = lua_tonumber(L, 1) - 1;
	CVector objectPos;
	lsc_getVectorFromStack(objectPos, L, 2, true);

	if (!TheAdhoc.IsHost())
		return 0;

	Game.CreateLuaObject(objectId, objectPos);
	net::pckt_msg_create_lua_object packet{};
	packet.pckt_size = sizeof(net::pckt_msg_create_lua_object);
	packet.pckt_id = gtMP_PacketIDs.msg_create_lua_object.pckt_id;
	packet.isCreate = true;
	packet.isDestroy = false;
	packet.object_id = objectId;
	packet.pos = objectPos;
	Game.SendMessagePriority(packet, BROADCAST_PEER_GROUPID);
	return 0;
}

int mp_lsn_DestroyLuaObject(lua_State* L) {
	cMultiGame& Game = TheMPGame;
	if (!TheAdhoc.IsHost())
		return 0;

	Game.DestroyLuaObject();
	net::pckt_msg_create_lua_object packet{};
	packet.pckt_size = sizeof(net::pckt_msg_create_lua_object);
	packet.pckt_id = gtMP_PacketIDs.msg_create_lua_object.pckt_id;
	packet.isCreate = false;
	packet.isDestroy = true;
	Game.SendMessagePriority(packet, BROADCAST_PEER_GROUPID);
	return 0;
}

int mp_lsn_UpdatePad(lua_State* L) {
	CPad* pad = GetPadFromPlayer(FindPlayerPed());
	if (pad) pad->Update(PAD1);
	return 0;
}

int mp_lsn_ClearPad(lua_State* L) {
	CPad* pad = GetPadFromPlayer(FindPlayerPed());
	if (pad) pad->Clear(false);
	return 0;
}

int mp_lsn_SetVip(lua_State* L) {
	TheMPGame.m_nVipPeerID = lua_tonumber(L, 1);
	return 0;
}

int mp_lsn_DrawHudCars(lua_State* L) {
	gbMP_DrawHudCars = lua_tonumber(L, 1) != 0;
	return 0;
}

int mp_lsn_ServerReadyToGo(lua_State* L) {
	if (lua_isboolean(L, 1))
	{
		bool ready = lua_toboolean(L, 1);
		if (TheAdhoc.IsHost())
		{
			net::pckt_msg_server_ready_to_go packet{};
			packet.pckt_size = sizeof(net::pckt_msg_server_ready_to_go); // 4
			packet.pckt_id = gtMP_PacketIDs.msg_server_ready_to_go.pckt_id; // 65
			packet.bIsServerReadyToGo = ready;
			TheMPGame.SendMessagePriority(packet, BROADCAST_PEER_GROUPID);
		}
		return 0;
	}
	lua_pushboolean(L, TheMPGame.GetIsServerReadyToGo());
	return 1;
}
#endif


static const luaL_reg matchinglibs[] = {
	{"HostName",                               mp_lsn_HostName},
	{"DbgPrintAllOptions",                     mp_lsn_DbgPrintAllOptions},
	{"ServerName",                             mp_lsn_ServerName},
	{"IsAdhocConnected",                       mp_lsn_IsAdhocConnected},
	{"IsServer",                               mp_lsn_IsServer},
	{"TargetPlayer",                           mp_lsn_TargetPlayer},
	{"GameTimeUp",                             mp_lsn_GameTimeUp},
	{"GetGameSeconds",                         mp_lsn_GetGameSeconds},
	{"UpdateGameTime",                         mp_lsn_UpdateGameTime},
	{"GameTimeMinutes",                        mp_lsn_GameTimeMinutes},
	{"GameTimeSeconds",                        mp_lsn_GameTimeSeconds},
	{"SetGameTime",                            mp_lsn_SetGameTime},
	{"GameType",                               mp_lsn_GameType},
	{"GameLocation",                           mp_lsn_GameLocation},
	{"GameScoreLimit",                         mp_lsn_GameScoreLimit},
	{"GameTimeLimit",                          mp_lsn_GameTimeLimit},
	{"GameElapsedMs",                          mp_lsn_GameElapsedMs},
	{"IsTeamGame",                             mp_lsn_IsTeamGame},
	{"UsePowerups",                            mp_lsn_UsePowerups},
	{"UseRacePowerups",                        mp_lsn_UseRacePowerups},
	{"RaceReverse",                            mp_lsn_RaceReverse},
	{"RaceId",                                 mp_lsn_RaceId},
	{"RaceCar",                                mp_lsn_RaceCar},
#ifndef GTA_LIBERTY
	{"IsIronManRace",                          mp_lsn_IsIronManRace},
	{"IsJetskiRace",                           mp_lsn_IsJetskiRace},
	{"NumIronManLegChanges",                   mp_lsn_NumIronManLegChanges},
#endif
	{"TankModel",                              mp_lsn_TankModel},
#ifndef GTA_LIBERTY
	{"HunterModel",                            mp_lsn_HunterModel},
#endif
	{"NoFuckingCutscene",                      mp_lsn_NoFuckingCutscene},
	{"TeamColour",                             mp_lsn_TeamColour},
	{"Colour",                                 mp_lsn_TeamColour}, // same func
	{"TeamName",                               mp_lsn_TeamName},
#ifndef GTA_LIBERTY
	{"VipTeam",                                mp_lsn_VipTeam},
#endif
	{"EndGame",                                mp_lsn_EndGame},
	{"StartGame",                              mp_lsn_StartGame},
	{"DefendingTeam",                          mp_lsn_DefendingTeam},
	{"ShowingCommentary",                      mp_lsn_ShowingCommentary},
	{"NetMinLatency",                          mp_lsn_NetMinLatency},
	{"NetMaxLatency",                          mp_lsn_NetMaxLatency},
	{"NetPacketLoss",                          mp_lsn_NetPacketLoss},
	{"MultiLag",                               mp_lsn_Multilag},
	{"Wait",                                   mp_lsn_Wait},
	{"WaitFunc",                               mp_lsn_WaitFunc},
	{"FindGroundZFor3DCoord",                  mp_lsn_FindGroundZFor3DCoord},
	{"CutScenePlaying",                        mp_lsn_CutScenePlaying},
	{"RenderPauseScreenStuff",                 mp_lsn_renderPauseScreenStuff},
	{"RenderPauseScreenStuffNoBox",            mp_lsn_renderPauseScreenStuffNoBox},
#ifndef GTA_LIBERTY
	{"DisablePauseScreenBackgroundBoxOnly",    mp_lsn_DisablePauseScreenBackgroundBoxOnly},
#endif
	{"RenderHudExtras",                        mp_lsn_renderHudExtras},
	{"SetPauseScreenSelection",                mp_lsn_setPauseScreenSelection},
	{"ShowHelp",                               mp_lsn_showHelp},
	{"GetExpectedNumberOfPlayers",             mp_lsn_getExpectedNumberofPlayers},
	{"TeamGameEveryoneIn",                     mp_lsn_teamGameEveryoneIn},
	{"HasPlayerEnteredGame",                   mp_lsn_HasPlayerEnteredGame},
	{"UseSuperBrakeOnPause",                   mp_lsn_UseSuperBrakeOnPause},
	{"SyncCutScene",                           mp_lsn_syncCutscene},
#ifndef GTA_LIBERTY
	{"GetRedTeamPeerGroupId",                  mp_lsn_GetRedTeamPeerGroupId},
	{"GetBlueTeamPeerGroupId",                 mp_lsn_GetBlueTeamPeerGroupId},
	{"GetEveryonePeerGroupId",                 mp_lsn_GetEveryonePeerGroupId},
	{"PrintAllPeerGroups",                     mp_lsn_PrintAllPeerGroups},
	{"CreateAndSyncPeerGroup",                 mp_lsn_CreateAndSyncPeerGroup},
	{"CreateAndSyncEveryoneExceptMePeerGroup", mp_lsn_CreateAndSyncEveryoneExceptMePeerGroup},
	{"GetDiskPrepend",                         mp_lsn_GetDiskPrepend},
	{"SetEmpireSiteSize",                      mp_lsn_SetEmpireSiteSize},
	{"SetEmpireSiteType",                      mp_lsn_SetEmpireSiteType},
	{"CreateLuaObject",                        mp_lsn_CreateLuaObject},
	{"DestroyLuaObject",                       mp_lsn_DestroyLuaObject},
	{"UpdatePad",                              mp_lsn_UpdatePad},
	{"ClearPad",                               mp_lsn_ClearPad},
	{"SetVip",                                 mp_lsn_SetVip},
	{"DrawHudCars",                            mp_lsn_DrawHudCars},
	{"ServerReadyToGo",                        mp_lsn_ServerReadyToGo},
#endif
	{NULL, NULL}
};
VALIDATE_LUA_LIB(matchinglibs, (72 + 1), (50 + 1));

void lscript_open_matching() {
	cLWrapper& wrapper = cLWrapper::Instance();
	cMultiGame& Game = cMultiGame::Instance();
#define REGISTER_PACKET(id, callback) Game.RegisterPacket(id, new cPacketDispatcher(callback));

	REGISTER_PACKET(gtMP_PacketIDs.msg_ready_for_cutscene.pckt_id, &on_recv_msg_ready_for_cutscene); // 59 lcs vcs
	wrapper.CreateGlobalLibrary(matchinglibs, nil); // openlib
#undef REGISTER_PACKET
}