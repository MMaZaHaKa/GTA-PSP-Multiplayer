/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "common.h"

#include "multiplayer/LobbyPed.h"
#include "multiplayer/net/public.h"
#include "multiplayer/public.h"
#include "multiplayer/net/NetSession.h"

#define MAX_VISIBLE_NICKNAME_CHARS (8) // 123456789101112 -> 12345678... lobby/game
#define MAX_VISIBLE_ROOMS_ROWS     (5) // join mode
#define MAX_LOBBY_WAIT_DELAY       (2.0f) // host mode, err - disconnect ["SVRLOST"]
#define NUM_SEND_INFO_TICKS        (15)

enum eHostSendState
{
	HOST_SEND_CONNECTING = 0,
	HOST_SEND_SENDING_INFO,
	HOST_SEND_FLUSHING,
};

enum eMultiGameMenu
{
	// Main State
		// launch tab
	MGPO_LAUNCH_PAGE_NO = 0,
	MGPO_LAUNCH_PAGE_YES = 1,
		// launch tab (bad chars)
	MGPO_BADCHAR_PAGE_OK = 0,

		// main tab (join/host)
	MGPO_MAIN_PAGE_JOIN_A_GAME = 0,
	MGPO_MAIN_PAGE_HOST_A_GAME = 1,

	// Join State

	// Host State
	MGE_GAME_TYPE = 0, // LCS: SCENARIO, VCS: GAME TYPE // common
	MGE_GAME_LOCATION = 1, // common
#ifdef GTA_LIBERTY
	MGE_PLAY_CUTSCENE = 2, // common
#endif
	MGE_AFTER_COMMON,


	// MGP_DEATHMATCH  0 Liberty/Vice City Survivor
	MGE_DEATHMATCH_GAME_STYLE = MGE_AFTER_COMMON,
	MGE_DEATHMATCH_KILL_LIMIT,
	MGE_DEATHMATCH_TIME_LIMIT,
#ifdef GTA_LIBERTY
	MGE_DEATHMATCH_POWERUPS,
#endif
	MGE_MAX_DEATHMATCH_DEFAULT,
	MGE_DEATHMATCH_SEL_GANG_1 = MGE_MAX_DEATHMATCH_DEFAULT,
	MGE_DEATHMATCH_SEL_GANG_2,
	MGE_MAX_DEATHMATCH,


	// page MGP_MULTIRACE [1] (TURISMO) Street Rage
#ifdef GTA_LIBERTY
	MGE_MULTIRACE_RACE = MGE_AFTER_COMMON,
#else
	MGE_MULTIRACE_GAME_STYLE = MGE_AFTER_COMMON,
	MGE_MULTIRACE_RACE,
#endif
	MGE_MULTIRACE_LAPS,
	MGE_MULTIRACE_VEHICLE,
#ifdef GTA_LIBERTY
	MGE_MULTIRACE_POWERUPS,
#endif
	MGE_MAX_MULTIRACE,


	// page MGP_DEFENDTHEBASE [2] (DTB) Protection Racket
#ifdef GTA_LIBERTY
	MGE_DEFENDTHEBASE_POWERUPS = MGE_AFTER_COMMON,
#else
	MGE_DEFENDTHEBASE_SCENARIO = MGE_AFTER_COMMON,
#endif
	MGE_MAX_DEFENDTHEBASE,


	// page MGP_CTF [3] (CAPTURETHEFLAG) LCS:Get Stretch   VCS:Taken For A Ride // same
#ifdef GTA_LIBERTY
	MGE_CTF_SCORE_LIMIT = MGE_AFTER_COMMON,
	MGE_CTF_TIME_LIMIT,
	MGE_CTF_POWERUPS,
#else
	MGE_CTF_SCORE_SCENARIO = MGE_AFTER_COMMON,
	MGE_CTF_SCORE_LIMIT,
#endif
	MGE_MAX_CTF,


	// page MGP_TANK [4] Tanks for the Memories
	MGE_TANK_TANK_TIME = MGE_AFTER_COMMON,
	MGE_MAX_TANK,


	// page MGP_HITPARADE [5] The hit list
#ifdef GTA_LIBERTY
	MGE_HITPARADE_POWERUPS = MGE_AFTER_COMMON,
	MGE_MAX_HITPARADE,
#else
	MGE_MAX_HITPARADE = MGE_AFTER_COMMON,
#endif


	// page MGP_SIXTYSECONDS [6] LCS:The wedding list   VCS:Grand Theft Auto // same
	MGE_SIXTYSECONDS_CASH_TARGET = MGE_AFTER_COMMON,
	MGE_MAX_SIXTYSECONDS,


#ifndef GTA_LIBERTY
	// page MGP_HUNTERATTACK [7] Might Of The Hunter
	MGE_HUNTERATTACK_KILL_LIMIT = MGE_AFTER_COMMON,
	MGE_HUNTERATTACK_TIME_LIMIT,
	MGE_MAX_HUNTERATTACK,


	// page MGP_FLAGBALL [8] Empire Takedown
	MGE_FLAGBALL_SCENARIO = MGE_AFTER_COMMON,
	MGE_MAX_FLAGBALL,


	// page MGP_VIP [9] Vip Rip
	MGE_VIP_VIP_TEAM = MGE_AFTER_COMMON,
	MGE_MAX_VIP,


#if 0 // Beta
	// page MGP_COLLECTTHEGOLD [10] Collect The Gold
	MGE_MAX_COLLECTTHEGOLD = MGE_AFTER_COMMON,

	// page MGP_COPSANDROBBERS [11] Cops And Robbers
	MGE_MAX_COPSANDROBBERS = MGE_AFTER_COMMON,
#endif
#ifdef MULTIGAME_SCM
	// page MGP_SCM [11] Scm main.scm
	MGE_MAX_SCM = MGE_AFTER_COMMON,
#endif
#endif

	// dynamic
	//MGE_CHARACTER_OR_JOIN_GANG, // common
	//MGE_RESET, // common
	//MGE_START_GAME, // common
};

#pragma pack(push, 1)
struct tAdhocPeerData { // old tPeerData, todo put in Adhoc.h
	tMacAddr macAddr;
	int16 nSelectedPeerModelID;
	int16 nTeamID; // eGameTeam

	tAdhocPeerData() { nSelectedPeerModelID = 0; nTeamID = 0; }
};
static_assert(sizeof(tAdhocPeerData) == 10, "tAdhocPeerData");

struct tLobbyRemotePeer {
	tListenAddr peerAddr;
	int16 nTeamID; // eGameTeam  old nGangID   recheck!!
};
static_assert(sizeof(tLobbyRemotePeer) == 10, "tLobbyRemoteInfo");

/* TODO: this data is exchanged when connecting to a host */
/* game information sent by the host, who can edit the game settings */
struct tLobbyRemoteInfo { // old tMatchingEntry
	tLobbyRemotePeer m_HostPeerData;
	tAdhocPeerData m_nPeersConnInfo[MP_NUM_PEERS]; // players in lobby (6 rows, 7th dont see lobby) // slot 1 free, bug?
	int32 m_GameType; // eGameType
	int32 m_GameLocation; // eGameLocation
	int32 m_nScoreLimit;
	int32 m_nScoreCTFLimit;
	int32 m_nCashTarget;
	int32 m_nTimeLimit;

	// Flags A
	union
	{
		struct
		{
			uint32 m_bPowerUpOn : 1;
			uint32 m_TDMStyle : 1; // eTDMStyle // bIsGangWar
			uint32 m_bRacePowerUpOn : 1;
			uint32 m_bBit_8 : 1;
			uint32 m_bRaceRevr : 1;
			uint32 m_bBit_20 : 1;
			uint32 m_bIsVipTeamTeam2 : 1;
			uint32 m_bBit_80 : 1;
		};
		uint32 m_nFlags;
	};

	uint16 m_nScenarioOrRaceTrackID;
	uint16 m_nRaceCarID;
	uint8 m_nAmbientCarBank;
	uint8 m_nAmbientPedBank;
	uint8 m_aPlayerID[MP_MAX_NUM_PEERS];

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
	char m_sGroupName[SCE_NET_ADHOCCTL_GROUPNAME_LEN];
	int8 field_85[3];
};
static_assert(sizeof(tLobbyRemoteInfo) == 136, "tLobbyRemoteInfo");
#pragma pack(pop)


class cLobby {
//private:
public:
	static cLobby* ms_pInstance;
	typedef void (cLobby::* CallbackHnd)();

	CallbackHnd m_pDrawCB;
	CallbackHnd m_pProcessCB;
	cLobbyPed m_lobbyPed;
	int8 m_nSelectedMenuOptionIndex; // mp row [GameType, SelectedModel, TimeLimit] https://prnt.sc/B0uGfCYynUdf
	int32 m_nSrvListMinIdx;
	int32 m_nSrvListMaxIdx;
	int32 m_nNewHostIdx;
	int32 m_nSendInfoTimer;
	bool m_aConnections[16]; // TODO: figure out
	int32 m_nSocketID_1;
	int32 m_nSocketID_2;
	int32 m_nSendState;
	int32 m_nWaitTime;
	tLobbyRemoteInfo m_remoteInfo;
	bool field_18C;
	bool field_18D;
	bool field_18E;
	bool field_18F;
	int32 field_190;
	int32 field_194;
	bool field_198; // bool/int8?
	// pad3
	int32 field_19C;
	bool m_bBadCharWarn;
	// pad3
	int32 field_1A4;
	bool m_bConnection;
	uint8 m_nSavedRaceLapCount; // miami
	// 2b
#ifdef MP_FE_MOUSE_IMPROVEMENTS
	int32 m_nLastMouseSelectedMenuOptionY;
#endif

	static float ms_fJoinPrevTime; // timer if big commection error in lobby
	static uint32 m_snTimeInMillisecondsLeftButtonPrev;
	static uint32 m_snTimeInMillisecondsRightButtonPrev;

	cLobby();

	// inlined helpers
	//eGameType GetNextGameType(eGameType type, int8 inc);
	//eGameLocation GetNextGameLocation(eGameLocation location, int8 inc);
	void NextScoreLimit(int inc, int minValue, int maxValue);
	void NextCtfScoreLimit(int8 inc);
	void NextCashTarget(int16 step, int16 min, int16 max);
	void NextTimeLimit(int inc);
	void NextVehicle(int8 inc);
	void NextRaceID(int8 inc);
#ifndef GTA_LIBERTY
	void NextScenario(int8 inc);
#endif
	void NextRaceLapCount(int8 inc);
	void TogglePowerup();
	void ToggleRacePowerup();
	void NextGameLocation(int inc);
	void NextGameStyle(int inc);
	void NextRaceGameStyle(int inc); // miami
	void NextGangAOption(int inc);
	void NextGangBOption(int inc);
#ifndef GTA_LIBERTY
	void NextVipTeam(int inc);
#endif
	void RenderCharacterOption(int nRowIndex, int nPosY);
	void HandleLeftRightBtnPress(int8 nMenuIndex, int8 inc);
	void NextMenuOption(int step, int min, int max);

	// cLobby
	bool IsPressedLeftButton();
	bool IsPressedRightButton();
	bool InitialiseMultiplayer();
	void UpdateLogic();
	void UpdateRender();
	void Close(); // old CloseLua
	bool ReturnAfterGame();
#ifdef GTA_LIBERTY
	void CloseConnection();
#else
	void CloseConnection(bool bSetSingleMode);
#endif
	void SetCurrentFrontendHandler(CallbackHnd control, CallbackHnd view);
	void CloseAdhoc(); // inlined
	void CheckCloseConnection();
	void NoDraw();
	void HandleGangSelection(int32 optIndex);
#ifdef GTA_LIBERTY
	void SetTextColorStyle(bool selected);
#else
	void SetTextColorStyle(bool selected, int32 mode); // mode button col mode // mode - custom arg
	void SetInactiveTextColorStyle(bool selected); // miami
#endif
	void InitGameParams(bool keepParams);
	void NextCutsceneOption(int8 inc);
#ifndef GTA_LIBERTY
	bool IsExistsScenarioForThisGame();
	int32 GetScenarioNumForThisGame(bool* pIsFind = nil); // custom
	bool IsGameLocationAllowed();
#endif
	bool HasBadCharsInNickname();
	void InitMultiGameMode(); // or LaunchMultiplayer

	// Callbacks
	void HandleMainGameState();
	void DrawMainGameScreen();
	void HandleHostGameState();
	void DrawHostGameScreen();
	void HandleHostStartGameState();
	void HandleJoinGameState();
	void DrawJoinGameScreen();

	void UpdateClient(tLobbyRemoteInfo* pConnInfo, bool bUpdatePeer);
	void LoadMultiplayer();

	static cLobby& Instance() {
		if (!ms_pInstance)
			ms_pInstance = new cLobby();
		return *ms_pInstance;
	}
};

#define TheLobby (cLobby::Instance())

