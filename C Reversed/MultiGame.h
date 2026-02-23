/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include <vector>
#include "common.h"
#include "singletonManager.h"
#include "leeds/base/stringt.h"
#include "Entity.h"
#include "multiplayer/net/NetSession.h"
#include "multiplayer/net/packet.h"
#include "multiplayer/net/public.h"
#include "multiplayer/net/Adhoc.h"
#include "multiplayer/InterestZone.h"
#include "multiplayer/elements/sHalo.h"
#include "multiplayer/elements/sElement.h"
#include "multiplayer/elements/sWaypoint.h"
#include "multiplayer/elements/sPlayer.h"
#include "multiplayer/elements/sPed.h"
#include "multiplayer/net/Peers.h"


/*
TODO
#1: implement function (it is just a stub)
#2: function partially implemented (have to re-check it)
 + missing some instructions and/or function calls
#3: code adapted from PSP
 + PSP controls/framework usually are not available in PC
*/

// TODO: SP Dummy
// cAudioManager::ProcessMultiplayerVehicle
// cAudioManager::ProcessMultiplayerPed
// CProjectileInfo::RemoveAllProjectilesOwnedBy
// CWeaponEffects::GetTargetEntity
// CWorld::FindMultiplayerObjectsInRangeSectorList
// CPed::SetPlayerToFollow
// CPlayerPed::HasUberPickup
// CPlayerPed::HasQuadDamage
// CPlayerPed::HasInvisibility
// CPlayerPed::HasRegeneration
// CPlayerPed::HasFlagBall +++
// CPlayerPed::GiveQuadDamage
// CPlayerPed::GiveRegeneration
// CPlayerPed::GiveInvisibility
// CPlayerPed::GiveCarHandling
// CPlayerPed::GiveFlagBall ++++
// CPlayerPed::DropFlagBall ++++
// CPlayerPed::RemoveUberPickup
// CPlayerPed::DropUberPickup
// CPlayerPed::GetPickupsBeingCarried
// CRadar::AddMultiplayerMarker
// CRadar::RemoveMultiplayerMarker
// CPickup::PickupTheDamnPickup
// CPickups::DestroyFlagBall
// CPed::HasUberPickup
// CPed::GiveQuadDamage
// CPed::GiveRegeneration
// CPed::GiveInvisibility
// CPed::StartMultiplayerFrenzy
// CDarkel::StartMultiplayerFrenzy
// CPickup::PickupTheDamnPickup
// CPad::GetExitVehicleForScript
// CGameLogic::RestorePlayerStuffDuringResurrection_NetworkGame
// CMBlur::Reset ? need?
// CMessages::ClearHelpMessages
// CCranes, CFakePlane, CFerry, etc -  SetupForSP/MP


enum class eGameType {
	DEATHMATCH = 0, // Liberty/Vice City Survivor
	MULTIRACE,      // (TURISMO) Street Rage
	DEFENDTHEBASE,  // (DTB) Protection Racket
	CTF,            // (CAPTURETHEFLAG) LCS:Get Stretch   VCS:Taken For A Ride // same
	TANK,           // Tanks for the Memories
	HITPARADE,      // The hit list
	SIXTYSECONDS,   // LCS:The wedding list   VCS:Grand Theft Auto // same
#ifndef GTA_LIBERTY
	HUNTERATTACK,   // Might Of The Hunter
	FLAGBALL,       // Empire Takedown
	VIP,            // Vip Rip
#if 0 // beta from gxt
	COLLECTTHEGOLD, // Collect The Gold
	COPSANDROBBERS, // Cops And Robbers
#endif
#endif
#ifdef MULTIGAME_SCM
	SCM,            // main.scm
#endif
	NUM_MULIT_GAME_TYPES
};

enum class eGameLocation {
#ifdef GTA_LIBERTY
	IND_ZON = 0,             // Portland
	COM_ZON = 1,             // Staunton Island
	SUB_ZON = 2,             // Shoreside Vale
#else
	BEACH_LVL = 0,           // Right
	MAINLAND_LVL,            // Left

	// BEACH_LVL
	VICE_POINT_ZON = 0,      // Vice Point
	WASHINGTON_BEACH_ZON,    // Washington Beach
	OCEAN_BEACH_ZON,         // Ocean Beach
	PRAWN_ISLAND_ZON,        // Prawn Island
	LEAF_LINKS_ZON,          // Leaf Links
	STARFISH_ISLAND_ZON,     // Starfish Island
	// MAINLAND_LVL
	DOWNTOWN_ZON,            // Downtown
	LITTLE_HAITI_ZON,        // Little Haiti
	LITTLE_HAVANA_ZON,       // Little Havana
	VICE_PORT_ZON,           // Viceport
	ESCOBAR_INT_AIRPOIT_ZON, // Escobar Int. Airpoit
	FORT_BAXTER_AIRBASE_ZON, // Fort Baxter Airbase
#endif

	NUM_MP_GAME_LOCATION     // LCS: 3, VCS: 12
};

enum class eGameCutscenePlayback {
	ALWAYS_PLAY = 0,
	DONT_PLAY = 1,
	PLAY_ONCE = 2
};

enum class eRaceStyle {
	RACE_NORMAL = 0,
	RACE_QUADATHLON, // no laps
	RACE_JETSKI, // CommercialPassed unlock
};

enum class eGameTeam {
	TEAM_A = 0,
	TEAM_B = 1,
	MAX_TEAM_COUNT,
};
#define MP_TEAM_COUNT (static_cast<uint32>(eGameTeam::MAX_TEAM_COUNT))

enum eTDMStyle {
	FFA = 0, // free for all
	GANG_WAR
};
// or use cMultiGame::IsTeamGame()
#define FREE_MODE (TheMPGame.eTDMStyle == eTDMStyle::FFA) // 0 [1 generic gang team]
#define GANG_MODE (TheMPGame.eTDMStyle == eTDMStyle::GANG_WAR) // 1 [2 gang team]

//struct tGangDefPSP { // orig psp struct
//	const char* name;
//	uint32 nUnlockLevel;
//};
struct tGangDef {
	char name[5];
	uint8 nUnlockLevel;
};

enum eLocationScenarioType // [gametype][gamelocation] = num scenario/races
{
	ST_MULTIRACE_CTF_SCORE_0 = 0,
	ST_MULTIRACE_CTF_SCORE_1,
	ST_MULTIRACE_CTF_SCORE_2,
	ST_DEFENDTHEBASE,
	ST_CTF,
	ST_TANK,
	ST_SIXTYSECONDS,
	ST_HUNTERATTACK,
	ST_FLAGBALL,
	ST_VIP,

	MAX_SCENARIO_NUMS_TYPES
};

enum eElementID
{
	// createPlayerAt, creation sequence
	MG_ELEMENT_PLAYER_ID = 0,
	MG_ELEMENT_PLAYER_PED_ID,
};

#ifndef GTA_LIBERTY
class cGameZoneInfo { // radar red zone
public:
	CRectLeeds* aActivityZonesRects;
	int32 countActivityZones;
	cGameZoneInfo(int32 level); // 0 beachzon, 1 mainland startgame
	~cGameZoneInfo();
	bool IsPointInActivityZone(CVector2D pos);
	void DrawActivityZonesOnRadar(void);
	void InitialiseActivityZones(int32 level);
};
#endif

#define MP_MAX_GANGS       (10)
#ifdef GTA_LIBERTY
#define MP_MAX_RACE_TRACKS (7)
#endif
#define MP_MAX_RACE_LAPS   (8)
#define MP_MAX_CTF_SCORE   (10)

extern tGangDef gMPGangDefs[MP_MAX_GANGS];
#ifndef GTA_LIBERTY
extern int32 gMPGangDefsNetModelListIndices[MP_MAX_GANGS];
extern int32 gMPScenarioNumsTable[MAX_SCENARIO_NUMS_TYPES][static_cast<int32>(eGameLocation::NUM_MP_GAME_LOCATION)]; // also RaceTrackNums
#endif
extern bool gMPCutsceneHasPlayed[7];


class cEventStack {
private:
	int32* m_pData;
	int32 m_nSize;
	int32 m_nIndex;
public:
	void push(int32 id);
	int32 pop();
	bool isEmpty() const { return m_nIndex == 0; }
	void Reset() { m_nIndex = 0; }
	void clear();
	cEventStack();
	~cEventStack();
};

class cMultiGame
{
private:
	static cMultiGame msInstance;
public:
	static uint8 s_nPlayerModelIndex;
	static eGameTeam s_nSelectedTeam;

	uint8 m_aPlayerID[MP_MAX_NUM_PEERS];
	uint8 m_nDefendingTeamID;
	bool m_bUpdateGameTime;
	bool m_bTimeHasSync;
	bool m_bIsRemovingPeer;
	bool m_bShowingCommentary;
	bool m_bIsRunning;
	//int8 m_pad0[2];
	int32 m_nTargetPlayer;
	eGameType m_GameType;
	eGameLocation m_GameLocation;
	int32 m_nScoreLimit;
	int32 m_nTimeLimit;
	uint8 m_nAmbientCarBank;
	uint8 m_nAmbientPedBank;
	//int8 m_pad1[2];
	int32 m_nScoreCTFLimit;
	int32 m_nCashTarget;

	// Flags A
	union
	{
		struct
		{
			uint32 bPowerUpOn : 1;
			uint32 eTDMStyle : 1;  // eTDMStyle // 0 - free for all, 1 - Gang War  bIsGangWar
			uint32 bRacePowerUpOn : 1;
			uint32 bBit_8 : 1;
			uint32 bRaceRevr : 1;
			uint32 bBit_20 : 1;
			uint32 bIsVipTeamTeam2 : 1;
			uint32 bBit_80 : 1;
		};
		uint32 m_nFlags;
	};

	uint8 m_nScenarioOrRaceTrackID;
	//int8 m_pad2[1];
	uint16 m_nRaceCarID;

	// Flags B (Cutscene)
	union
	{
		struct
		{
			uint8 ePlayIntroCutscene : 3; // eGameCutscenePlayback
			uint8 b38_8 : 1;
			uint8 b38_10 : 1;
			uint8 b38_20 : 1;
			uint8 b38_40 : 1;
			uint8 bCutscenePlayed : 1;

		};
		uint8 m_nCutsceneFlags;
	};

	uint8 m_Team1GangID;
	uint8 m_Team2GangID;
	uint8 field_3B;
	uint16 m_nTankModelID;
#ifdef GTA_LIBERTY
	//int8 m_padLcs[2];
#else
	uint16 m_nHunterModelID;
#endif
	int32 m_nTimeMinutes;
	int32 m_nTimeSeconds;
	uint32 m_nTimeSec;
	uint32 m_nTimeCentiSec;
	uint32 m_nElapsedMs;
	int32 m_nLagValue;
	uint16 m_nUpdateSendTime; // recheck, ctor -1, probably -1/u16max
	//int8 m_pad4[2];
	int32 m_nWaitUnk;
	int32 m_nWaitHeartBeat;
	cNetSession* m_pNetSession;
	std::vector<std::pair<CEntity*, sElement*>> m_vEntList; // TODO(MP)?: this is not a vector?
	cInterestZoneManager m_ZoneManager;
	//int8 m_pad5[3];
	bool m_bTeamEveryoneIn;
	//int8 m_pad6[3];
	uint32 field_84;
	uint32 m_nCurTime;
	bool m_bHasSuspended;
	bool m_bIsNeedPrepareModels;
#ifndef GTA_LIBERTY // tmp
	bool m_bIsServerReadyToGo;
	//int8 m_pad8[1];
	CVector m_nFlagBallPosition;
#endif
	uint16 m_nElementsIDs;
	//int8 m_pad9[2];
	std::map<int32, cPacketDispatcherBase*> m_tPacketsEventsCB; // <pcktID,pcktCallData>
	// pad
	sWaypoint m_WaypointManager;
	// pad
	sHalo m_haloManager;
#ifndef GTA_LIBERTY
	CObject* m_pLuaObject; // MI_435_briefcase
#endif
	float m_fTimeStep;
	uint8 field_E8;
	bool m_bIsConnected;
	// pad
	uint32 m_nAccTimeStep;
	float m_fConnWaitTime;
	int32 m_anTeamScore[MP_TEAM_COUNT];
	cEventStack* m_pEventStack;
	int32 m_anTeamTimer[MP_TEAM_COUNT];
	bool m_abTeamTimerEnabled[MP_TEAM_COUNT];
	// pad[2]
	int32 field_10C;
	int32 field_110;
	uint8 field_114;
	uint8 m_nMaskCutsceneSync;
	// pad
#ifndef GTA_LIBERTY
	cGameZoneInfo* m_pGameZoneInfo;
	//int8 m_pad10[4];
	CVector m_vecPlayerCreationQueuedPosition;
	bool m_bIsPlayerCreationQueued;
	int8 m_nVipPeerID; // id u16
#endif

//#ifndef GTA_LIBERTY // tmp
//	//std::vector<sPeerState> m_vPlayers; // lcs real type
//	std::vector<sPeerState*> m_vPlayers; // simplify
//#endif
//	tMacAddr m_playerMacAddr;

	cMultiGame();
	~cMultiGame();
public:
	void UpdateReceive();
	void UpdateSend();
	void SetSuspend();
	void PrepareModels();
	bool Connect();
	bool PerformInitialConnection();
#ifdef GTA_LIBERTY
	int32 GetNumberOfPeersConnected();
#endif
	uint16 AdjustSendTime(uint16 time, uint16 nPeerID);
	void OnAckRecv(uint16 nPeerID); // lcs only
#ifndef GTA_LIBERTY
	void UpdatePlayerLatency(uint8 nPeerID, int32 latencyMs);
#endif
	bool IsLocalPlayer(int32 id);
	bool IsSameGroup(int32 a, int32 b);
	sPlayer* GetPlayer(int32 nPeerID); // MG_ELEMENT_PLAYER_ID
	sPed* GetPlayerPed(int32 nPeerID); // MG_ELEMENT_PED_ID
	bool IsPlayerConnected(int32 id);
	void SetPlayerName(const char* name);
	const char* GetPlayerName(int32 id);
	void GetLocalPlayerName(base::string& outName);
	const char* GetGangName(uint16 id);
	const char* GetGangNameForEntity(uint16 id);
	int32 GetPlayerTeamID(int32 id);
	bool IsAnyTeamEmpty();
	bool GetCutsceneSkipEnabled();
	void SetGameType(eGameType type);
	eGameType GetGameType();
	void SetGameLocation(eGameLocation location);
	eGameLocation GetGameLocation();
	void SetScoreLimit(int32 limit);
	int32 GetScoreLimit();
	void SetTimeLimit(int32 limit);
	int32 GetTimeLimit();
	void SetCTFScoreLimit(int32 limit);
	int32 GetCTFScoreLimit();
	void SetCashTarget(int32 target);
	int32 GetCashTarget();
	void SetTeam1GangID(uint8 id);
	uint8 GetTeam1GangID();
	void SetTeam2GangID(uint8 id);
	uint8 GetTeam2GangID();
	CRGBA* GetColor(int32 id); // CRGBA* GetColorByTeamID(int32 id);
	CRGBA* GetBlipColor(int32 id);
	CRGBA* GetTeamColor(int32 id);
	CRGBA* GetPlayerColor(int32 id);
	void SyncPlayerDead(CEntity* ent);
	void SyncPlayerDead(uint8 id);
	void SendMessage(const net::pckt_base& packet, int destID);
	void SendMessagePriority(const net::pckt_base& packet, int destID);
	sElement* GetEntityForHandle(int32 nPeerID, int16 id); // nPeerID 0 (local): id 0 (sPlayer), id 1 (sPed)
	sElement* FindElement(int16 nPeerID, int32 nOwner, int16 nElemID);
	sVehicle* FindVehicle(int16 nPeerA, int16 nPeerB);
	template<typename T = struct sElement*>
	T GetElementFromEntity(CEntity* entity) {
		for (std::vector<std::pair<CEntity*, sElement*>>::iterator it = m_vEntList.begin(); it != m_vEntList.end(); it++) {
			if (it->first == entity) return (T)it->second;
		}
		return nil;
	}
#ifdef GTA_LIBERTY
	void SendTransferEntity(sElement* elem);
	void SendDemandEntityMsg(sElement* elem);
	void SendTransferEntityMsg(sElement* elem, int16 id);
#else
	bool TransferEntity(sElement* elem);
	void SendDemandEntityMsg(sElement* elem);
	bool SendTransferEntityMsg(sElement* elem, int16 id);
#endif
	void Nop1();
	bool IsElementExists(int16 nPeerID, sElement* elem);
	void RegisterPacket(int32 packet_id, cPacketDispatcherBase* pDispatcher);
#if !defined(FINAL) && !defined(MASTER)
	void PrintRegisteredPackets(); // custom
#endif
	void LoadScene(); // vcs
#ifndef GTA_LIBERTY
	uint32 GetPlayersCount();
#endif
	int32 GetNumberOfVehicles();
	void UpdateTime();
	int32 GetTimeMinutes();
	int32 GetTimeSeconds();
	void SetTimeMinutes(int32 value);
	void SetTimeSeconds(int32 value);
	void SetGameElapsedMs(uint32 value);
	uint32 GetGameElapsedMs();
	bool IsGameTimeUp();
	void SetTargetPlayer(int32 player, bool send);
	int32 GetTargetPlayer();
	int32 GetSpawnPointFromPlayer(int32 id);
	void SetDefendingTeamID(uint8 id);
	uint8 GetDefendingTeamID();
	bool HasPlayerJoinedGame(uint8 id);
	void SetPlayerSpawned(int32 id);
	void SetCutscenePlaying(int32 id);
	int32 GetSyncedCarCount();
	void RemovePlayerFromGame(int32 id);
#ifdef GTA_LIBERTY
	int32 CreateObjectAtPos(int16 index, float posX, float posY, float posZ);
	void AvoidObjectCleanup(int32 ref);
	void UpdateObjectHeading(int32 ref, float rot);
#else
	bool CreateLuaObject(int32 index, CVector pos);
	bool DestroyLuaObject();
	void RequestPlayerCreation(CVector pos);
	void ShowMenu();
#endif
#ifdef GTA_LIBERTY
	void UpdateZonePeers();
#else
	void UpdateZonePeersTimeouts();
	void UpdateZonePeersSync();
#endif
	void RegisterEntity(sElement* elem);
	void RemoveElement(sElement* elem);
	void AttachEntity(sElement* elem, CEntity* entity);
	uint16 GetNextElementID();
	void SendTransferPacket(sElement* elem, int8 to);
	void FireMessageHandler(net::pckt_base& packet, int sender, uint16 time, bool fromLocalGame); // NO ID, entry point to dispatcher
	void OnTransferEntity(net::pckt_transfer_entity& packet, int sender, uint16 time, bool fromLocalGame); // 17
	void OnGameStateChange(net::pckt_game_state& packet, int sender, uint16 time, bool fromLocalGame); // 14
	void DiscardModels();
	void RestoreModels();
	void LoadGameModels();
	void LoadBaseModels();
	static void OnPlayerKill(net::pckt_player_kill& packet, int sender, uint16 time, bool fromLocalGame); // 3
	void Open();
	void Close();
#ifndef GTA_LIBERTY
	void LoadMultiraceGameTypeModels();
	void LoadDefendTheBaseGameTypeModels();
	void LoadMultiraceGameTypeCtf1Models(); // rename?
	void LoadCtfGameTypeModels(); // inlined
#endif


	// inlined
#ifndef GTA_LIBERTY
	inline cGameZoneInfo* GetGameZoneInfo() { return m_pGameZoneInfo; }
	inline void SetGameZoneInfo(cGameZoneInfo* info) { m_pGameZoneInfo = info; }
	inline CVector GetFlagBallPosition() { return m_nFlagBallPosition; }
	inline void SetFlagBallPosition(CVector pos) { m_nFlagBallPosition = pos; }
	inline bool GetIsServerReadyToGo() { return m_bIsServerReadyToGo; }
	inline void SetIsServerReadyToGo(bool ready) { m_bIsServerReadyToGo = ready; }
	inline void SetPlayerCreationQueued(bool q) { m_bIsPlayerCreationQueued = q; }
	inline bool GetPlayerCreationQueued() { return m_bIsPlayerCreationQueued; }
	inline void SetPlayerCreationQueuedPosition(CVector pos) { m_vecPlayerCreationQueuedPosition = pos; }
	inline CVector GetPlayerCreationQueuedPosition() { return m_vecPlayerCreationQueuedPosition; }
#endif

	inline bool IsOpen() { return m_pNetSession != nil && m_pNetSession->m_netListen.m_nPdpID >= 0; }
	inline uint16 LocalPlayerID() { return m_pNetSession->m_nSelfPeerID; }
	inline bool IsElementOwnerLocalPlayer(sElement* elem) { return elem->GetOwner() == LocalPlayerID(); };
	uint32 inline GetCurTime() { return m_nCurTime; }

#ifdef GTA_LIBERTY
	inline sPeerState* GetPeerAt(uint32 id) { return m_vPlayers.at(id); }
	inline uint32 GetPlayersCount() { return (uint32)m_vPlayers.size(); }
	inline std::vector<sPeerState*>& GetPlayersList() { return m_vPlayers; }
#endif

	inline void RemoveEntity(sElement* pElement) {
		CEntity* pEntity = pElement->GetEntity();
		if (pEntity == nil) return;
		auto it = std::find(m_vEntList.begin(), m_vEntList.end(), std::pair<CEntity*, sElement*>(pEntity, pElement));
		if (it != m_vEntList.end()) m_vEntList.erase(it);
	}

	inline void DisposeEntity(CEntity* pEntity) {
		if (pEntity == nil) return;
		auto it = m_vEntList.begin();
		while (it != m_vEntList.end()) {
			if (it->first == pEntity) {
				break;
			}
			++it;
		}
		if (it != m_vEntList.end()) {
			sElement* pElement = it->second;
			AttachEntity(pElement, nil);
			pElement->SetEntity(nil);
			delete pElement;
		}
	}

	inline cInterestZone* GetElementOwnerZone(sElement* pElement) { return m_ZoneManager.GetZoneByPeer(pElement->GetOwner()); }

	// our native alias
	inline sPed* FindPlayerPedMG() { return GetPlayerPed(LocalPlayerID()); }
	inline sPlayer* FindPlayerInfoMG() { return GetPlayer(LocalPlayerID()); }
	inline cInterestZone* FindPlayerZoneMG() { return m_ZoneManager.GetZoneByPeer(LocalPlayerID()); } // -1?

	inline void SetTeamGameTime(int32 nTeamID, int32 time) { m_anTeamTimer[nTeamID] = time; }

	static cMultiGame& Instance()
	{
		return msInstance;
		//if (!msInstance)
		//	msInstance = new cMultiGame;
		//return *msInstance;
	}
};

#define TheMPGame (cMultiGame::Instance())

void* allocFunc(uint32 size);
void deleteFunc(void* buff);

#ifndef GTA_PSP
double mp_time_now_d();
#endif

struct cNetConfig {
public:
	// SystemParam
	std::string sNickName;  // AdHoc and system nickname  SCE_NET_ADHOCCTL_NICKNAME_LEN
	bool bIsDynamicName;
	bool bMACNickName;
	std::string sMACAddress;

	// Networking
	// (relay server) \/
	bool bPtpPdpDedicatedEmu; // revcs experimental pdp ptp connection from dedicated aemu interface (anti cgnat) spread OPCODE_CHAT --mazahaka
	int nPtpPdpDedicatedEmuChannel; // room by product id
	bool bPtpPdpDedicatedEmuChatChannel;
	bool bEnableAdhocServer;
	std::string sProAdhocServer;
	bool bIsDynamicProAdhocServerAddr;
	std::vector<std::string> proAdhocServerList;
	std::string sInfrastructureDNSServer;
	std::string sInfrastructureUsername;  // Username used for Infrastructure play. Different restrictions.
	bool bInfrastructureAutoDNS;
	bool bAllowSavestateWhileConnected;  // Developer option, ini-only. No normal users need this, it's always wrong to save/load state when online.
	bool bAllowSpeedControlWhileConnected;  // Useful in some games but not recommended.

	bool bEnableWlan;
	std::map<std::string, std::string> mHostToAlias;  // Local DNS database stored in ini file

	bool bEnableUPnP;
	bool bUPnPUseOriginalPort;
	bool bForcedFirstConnect;
	int iPortOffset;
	int iMinTimeout;
	int iWlanAdhocChannel;

	bool bEnableNetworkChat;

	cNetConfig();
	void Init(); // after read from ini
};

extern cNetConfig g_Config;


// TODO: figure out the location for this color structure
struct CABGR {
	uint8 alpha;
	uint8 blue;
	uint8 green;
	uint8 red;
};

// TODO: figure out the location for those global variables
extern bool gbMP_DrawPauseScreen; // gMultiplayerDrawPauseScreen
extern bool gbMP_DrawPauseScreenNoBox; // gMultiplayerDrawPauseScreenNoBox
extern bool gbMP_RenderHudExtras;
extern bool gbMP_HudShowHelp;
extern bool gbMultiplayerSplash;
extern uint8 gnMP_PauseScreenSelection;
extern bool gMultiplayerSuperBrakeOnPause; // declared in main.h
// defined in Script
extern bool gDeveloperFlag;
extern bool gbMP_StartingScriptsFromLua;
extern bool gbMP_DrawHudCars;
extern bool gbMP_RenderNativeEntities; // draw hiden entitys in MG

extern bool gIsMultiplayerGame;
extern CVector gVectorSetInLua;

extern bool gMultiplayerCheat1;
extern bool gMultiplayerCheat2;
extern bool gMultiplayerCheat3;
extern bool gMultiplayerCheat4;

extern bool gbIsUsingLUASource; // non compiled lua (.LUA)

#ifdef DEBUG_MULTIGAME
void DebugMenuExecLuaInput();
uint8* DebugLoadPSPDump(const char* path, int32* nOutSize);
void DebugRebasePSPDump(uint8* dump, uint32 nSize, uint32 nAddr);
uint8* DebugFixupPSPDump(uint8* dump, uint32 nSize, uint32 nAddr);
void DebugFreePSPDump(uint8* dump);
#endif

extern int32 gMPNetDebugLogLevel;
#if defined(DEBUG_MULTIGAME) && !defined(MASTER)
// gui
extern int32 gMPDebugPrintLevel;
void MPPrintDebugStuff();
#endif

uint32 GetSyncSizeByElement(sElement* pElement);

uint8 GetLevelOfCompleteness();
bool IsCarAllowedInMultiplayer(int32 min, bool mode);
void mp_regiter_packets();
void loadWeapons();
void mp_register_waypoint_packets(sWaypoint* pWaypointManager);

void ClearMultiplayerSplashScreen(); // Render2d funcs addr block
