/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once
#include "common.h"

#ifdef GTA_PSP
#include <pspnet.h>
#else
#include "multiplayer/net/emu/NetAdhocCommon.h" // emu
//#include "multiplayer/net/emu/proAdhoc.h" // SceNetEtherAddr, SceNetAdhocctlNickname
#endif

#include "leeds/base/stringt.h"
#include "multiplayer/net/public.h"
#include "multiplayer/Lobby.h"

//https://github.com/PSP-Archive/Rip-Off/blob/968c21162b0b44a539664400a0047ffaad4ad764/Game/Network/PSPNetwork.h
//#define RIPOFF_DISCOVER_TIMEOUT	(60)	/* 60sec */  // sceNetAdhocDiscoverInitStart

#define GTA_PSPNET_POOLSIZE    (128 * 1024)
#define GTA_CALLOUT_TPL        (32)
#define GTA_NETINTR_TPL        (16)
#define GTA_ADHOCCTL_TPL       (48)
#define GTA_ADHOCCTL_STACKSIZE (6 * 1024)

#define GTA_NET_MATCHING_TPL	   (16)
#define GTA_NET_MATCHING_STACKSIZE (32 * 1024)
#define GTA_NET_MATCHING_ATTR	   (16 * 1024)

#define GTA_SERVER_PORT			(66)  // <<--------------
#define GTA_PTP_PORT		    (0)   // <<--------------
#define GTA_MATCHING_PORT		(1)   // <<-------------- can be NET_SESSION_DEFAULT_PORT ?
#define GTA_PDP_PORT		    (2)   // <<--------------
#define GTA_GAME_PORT  		    (3)   // <<--------------

#define GTA_MATCHING_POOLSIZE  (13620)
#define GTA_MATCHING_RXBUFLEN  (4 * 1024)
#define GTA_HELLO_INTERVAL	   (1 * 1000 * 1000)	/* 1sec */
#define GTA_KEEPALIVE_INTERVAL (2 * 1000 * 1000)	/* 2sec */
#define GTA_KEEPALIVE_COUNT    (3)
#define GTA_MATCHING_REXMT_INTERVAL (500 * 1000)    /* 500ms */

#define GTA_MATCHING_EVENT_TPL (16)
#define GTA_EV_TH_STACK        (0x2000)
#define GTA_MATCHING_INPUT_TPL GTA_MATCHING_EVENT_TPL
#define GTA_IN_TH_STACK        GTA_EV_TH_STACK

#define GTA_RXBUFLEN			(16 * 1024)
#define GTA_REXMT_INTERVAL		(200*1000)		/* 200ms */
#define GTA_REXMT_COUNT			(200000) // huh?
//#define GTA_NET_TIMEOUT			(60*1000*1000)	/* 60sec */
//#define GTA_BUFSIZE				(SCE_NET_ADHOC_PTP_MSS)

#define GTA_WLAN_COOLDOWN      (20)
#define GTA_DEFAULT_WAIT_TIME  (50)
#define GTA_SCAN_ATTEMPT_COUNT (11)

#define GTA_EVF_ERROR			0x0001
#define GTA_EVF_CONNECT			0x0002
#define GTA_EVF_DISCONNECT		0x0004
#define GTA_EVF_SCAN			0x0008
#define GTA_EVF_CANCEL			0x0020
#define GTA_EVF_HOSTDISCOVER	0x0040
#define GTA_EVF_WOL				0x0080
#define GTA_EVF_WOL_ABORT		0x0100

#define GTA_NETWORK_FLAG_2PMODE		0x01
#define GTA_NETWORK_FLAG_SERVER		0x02
#define GTA_NETWORK_FLAG_PEER_READY	0x04
#define GTA_NETWORK_FLAG_PAUSE		0x08

enum eAdhocPeerState
{
	ADHOC_PEER_DISCONNECTED = 0, // free slot
	ADHOC_PEER_PENDING,
	ADHOC_PEER_HELLOED, // u can join to him
	ADHOC_PEER_ACCEPTED,
	ADHOC_PEER_SELECTED,
	ADHOC_PEER_JOINED, // in lobby
	ADHOC_PEER_LEAVING, // recheck
};

#pragma pack(push, 1)
struct tAdhocMatchingData {
	int32 nState; // eMatchingDataState
	tMacAddr addr;
	int8 padA[2];
	tLobbyRemoteInfo entry;
	int32 nWaitTime;

	tAdhocMatchingData() {
#ifdef FIX_BUGS
		nState = 0;
		nWaitTime = 0;
#endif
	}
};
static_assert(sizeof(tAdhocMatchingData) == 152, "tAdhocMatchingData");

struct tAdhocPlayerData {
	tMacAddr m_PlayerMacAddr; // can be also SceNetEtherAddr
	union
	{
		char m_szPlayerNickname[SCE_NET_ADHOCCTL_NICKNAME_LEN]; // 128
//#ifndef GTA_PSP
//		SceNetAdhocctlNickname nickname;
//#endif
	};

	tAdhocPlayerData() { memset(m_szPlayerNickname, 0, ADHOCCTL_NICKNAME_LEN); }
};
static_assert(sizeof(tAdhocPlayerData) == 134, "tAdhocPlayerData");
#pragma pack(pop)

#ifndef MASTER
void AdhocPrintDebugStuff();
#if !defined(GTA_PSP)
void AdhocEmuPrintDebugStuff();
#endif
#endif

extern bool g_networkModulesInitialized; // todo atomic bool?
void mp_load_psp_net_modules();
int mp_thread_load_modules(); // SceKernelThreadOptParam
void mp_unload_psp_net_modules();

// PSPSDK-cAdhoc adapter helper
// https://github.com/MMaZaHaKa/PSP_DOCS/blob/master/network/index.htm
// https://github.com/PSP-Archive/Rip-Off/blob/968c21162b0b44a539664400a0047ffaad4ad764/Game/Network/PSPNetwork.h
void adhocHandlerWrapper(int flag, int error, void* arg);
void adhocMatchingClientCB(int matchingid, int event, unsigned char* mac, int optlen, void* optdata);
void adhocMatchingHostCB(int matchingid, int event, unsigned char* mac, int optlen, void* optdata);

class cAdhoc {
private:
	typedef void (cAdhoc::* CallbackHnd)();
	static cAdhoc* ms_pInstance;
public:
	static bool ms_bInitNetworkModules;

	int32 m_nLoadMPModulesThreadID;
	bool m_bHasStartedMatching;
	//int8 m_pad0[3];
	CallbackHnd m_NextStateFuncCb;
	tLobbyRemoteInfo* m_pMatchingInfoEntry;
	tMacAddr m_LobbyHosterMacAddr;
	//int8 m_pad1[2];
	int32 m_nServerPeerID;   // 0 HOST, 6 SLAVE FRIEND, 0xFFFFFFFF -1 int32 in menu
	const char* m_sGameID;   // GTA_PRODUCT_ID "ULUS10160" // 9 // socom main game tab category
	const char* m_sGameRoom; // GTA_TITLE_ID   "GTAVCS00"  // 8 // gta used as group name (m_sGroupName)
	char m_sGroupName[SCE_NET_ADHOCCTL_GROUPNAME_LEN];
	int32 m_nScanAttemptCount;
	bool m_bHasExitedWithError;
	//int8 m_pad2[3];
	int32 m_nErrorCode;

	// Flags A
	union
	{
		struct
		{
			uint32 bHasError : 1;            // &1  GTA_EVF_ERROR
			uint32 bConnEvent : 1;           // &2  GTA_EVF_CONNECT
			uint32 bDisconnEvent : 1;        // &4  GTA_EVF_DISCONNECT
			uint32 bScanEvent : 1;           // &8  GTA_EVF_SCAN
			uint32 bCancelEvent : 1;         // &10 GTA_EVF_CANCEL
			uint32 bHostDiscoverEvent : 1;   // &20 GTA_EVF_HOSTDISCOVER
			uint32 bWolEvent : 1;            // &40 GTA_EVF_WOL
			uint32 bWolAbortEvent : 1;       // &80 GTA_EVF_WOL_ABORT
		};
		uint32 m_nEventFlags; // Event flags [PSPNET-Owerview]
	};

	base::string m_sExitReason;
	int32 m_nAdhocEventHandler;
	int32 m_nAdhocMatchingID;
	int32 m_nWlanCounter;
	tAdhocMatchingData m_aMatchingInfo[MP_NUM_MATCHING_GROUPS]; // rooms/hosts (LVCS sz 152) (7 rows rooms + host game) (0 self when host (init m_nServerPeerID 0))
	tAdhocPlayerData m_aMatchingPlayersInfo[MP_NUM_PEERS]; // probably MP_NUM_MATCHING_GROUPS
	//int8 m_pad2[2];
	int32 m_nAdhocConnSemaID;
	tAdhocMatchingData m_aMatchingInfoRecv[MP_NUM_MATCHING_GROUPS];
	bool m_bPendingHostStart;
	uint8 field_C5D;
	tMacAddr m_MyMacAddr; // todo
	cListenInfo m_nAdhocPdp;
	bool m_bIsServerConnLost;
	//int8 m_unk3[3];

public:

	cAdhoc();

	void Configure(const char* sGameID, const char* sGameRoom);
	void Update();
	void Terminate();
	bool StartHosting();
	void OnStartGame();
	void Disconnect();
	tLobbyRemoteInfo* GetMatchingInfo(int index);
	void SelectServer(int index);
	void CancelMatchingTarget();
	void FormatPlayerName(base::string& outName, char* sNickname);
	void GetPlayerName(base::string& outName, tAdhocPeerData& pPlayerInfo);
	void GetPlayerNameFromMacAddr(base::string& outName, tMacAddr& macAddr);
	void FindAndClearMacAddr(tMacAddr& macAddr);
	void SetGameParams(tLobbyRemoteInfo& info);
	void SetPeerConnInfo(int nPeerId, tLobbyRemoteInfo& info);
	bool IsWifiSwitchOn();
	int GetLocalMacAddr();
	tMacAddr& GetPlayerMacAddress();
	base::string& GetExitReason(); // LCS
	cListenInfo& GetAdhocPdp();
	uint8 GetNumberOfConnectedPlayers();
	uint8 GetNumberOfNonEmptyGangs();
	void AssignPlayerIDs();
	void TerminateAdhocMatching();
	void SetServerConnLost(bool bIsLost);
	bool GetServerConnLost();
	void CleanupNetworkModules();

	// States
	void StateShutdown();
	void StateInitialise();
	void StateConnectLobbyGroup();
	void StateCancelAllTargets();
	void StateIdle();
	void StateCancelAllTargetsIfNotJoined();
	void StateCreateGameGroup();
	void StateHandleError();
	void StateAttemptGameScan();
	void StateIdle2();
	void StateLobbyConnect();

	void SetStateGroupConnect();
	bool ConnectToGroup(const char* gameRoom);
	bool StartListening(); // probably name Populate2 (from debug)
	void CancelMatchingForPeer(int32 peerId);
	void TerminateWithError(base::string& reason);
	int FindMacAddr(tMacAddr& macAddr);

	// Sema
	int WaitForSemaphore();
	void SignalSemaphore();

	void BakeRandomGroupName(char* sGroupName); // randomize_group_name
	void ScanHandler(int nEvent, int nError);
	void UpdateGameParams();
	void OnMatchClient(int matchingid, int event, tMacAddr& macAddr, int optlen, void* optdata);
	void OnMatchHost(int matchingid, int event, tMacAddr& macAddr, int optlen, void* optdata);

	// Inlines
	inline bool HadError() { return bHasError || m_bHasExitedWithError; }
	inline bool IsNextStateNow(CallbackHnd state) { return m_NextStateFuncCb == state; }
	inline void ResetEventsFlags() { m_nEventFlags = 0x0; }
	inline void SetHasError() { bHasError = true; }
	inline void SetExitError() { m_bHasExitedWithError = true; }
	inline void NextState() { if (m_NextStateFuncCb) (this->*m_NextStateFuncCb)(); }
	inline bool IsHost() { return m_bHasStartedMatching; } // or IsServer()

	inline void SetNextState(CallbackHnd state)
	{
		m_NextStateFuncCb = state;

#ifdef DEBUG_MULTIGAME
		// 4 BP
		if (m_NextStateFuncCb == &cAdhoc::StateShutdown)
			debug("[Adhoc]: Set State: StateShutdown\n");
		else if (m_NextStateFuncCb == &cAdhoc::StateInitialise)
			debug("[Adhoc]: Set State: StateInitialise\n");
		else if (m_NextStateFuncCb == &cAdhoc::StateConnectLobbyGroup)
			debug("[Adhoc]: Set State: StateConnectLobbyGroup\n");
		else if (m_NextStateFuncCb == &cAdhoc::StateCancelAllTargets)
			debug("[Adhoc]: Set State: StateCancelAllTargets\n");
		else if (m_NextStateFuncCb == &cAdhoc::StateIdle)
			debug("[Adhoc]: Set State: StateIdle\n");
		else if (m_NextStateFuncCb == &cAdhoc::StateCancelAllTargetsIfNotJoined)
			debug("[Adhoc]: Set State: StateCancelAllTargetsIfNotJoined\n");
		else if (m_NextStateFuncCb == &cAdhoc::StateCreateGameGroup)
			debug("[Adhoc]: Set State: StateCreateGameGroup\n");
		else if (m_NextStateFuncCb == &cAdhoc::StateHandleError)
			debug("[Adhoc]: Set State: StateHandleError\n");
		else if (m_NextStateFuncCb == &cAdhoc::StateAttemptGameScan)
			debug("[Adhoc]: Set State: StateAttemptGameScan\n");
		else if (m_NextStateFuncCb == &cAdhoc::StateIdle2)
			debug("[Adhoc]: Set State: StateIdle2\n");
		else if (m_NextStateFuncCb == &cAdhoc::StateLobbyConnect)
			debug("[Adhoc]: Set State: StateLobbyConnect\n");
		else
			debug("[Adhoc]: Set State: Unknown state\n");
#endif
	}

	inline void DumpNextState()
	{
		OpenConsole();
		if (m_NextStateFuncCb == &cAdhoc::StateShutdown)
			debug("[Adhoc]: State: StateShutdown\n");
		else if (m_NextStateFuncCb == &cAdhoc::StateInitialise)
			debug("[Adhoc]: State: StateInitialise\n");
		else if (m_NextStateFuncCb == &cAdhoc::StateConnectLobbyGroup)
			debug("[Adhoc]: State: StateConnectLobbyGroup\n");
		else if (m_NextStateFuncCb == &cAdhoc::StateCancelAllTargets)
			debug("[Adhoc]: State: StateCancelAllTargets\n");
		else if (m_NextStateFuncCb == &cAdhoc::StateIdle)
			debug("[Adhoc]: State: StateIdle\n");
		else if (m_NextStateFuncCb == &cAdhoc::StateCancelAllTargetsIfNotJoined)
			debug("[Adhoc]: State: StateCancelAllTargetsIfNotJoined\n");
		else if (m_NextStateFuncCb == &cAdhoc::StateCreateGameGroup)
			debug("[Adhoc]: State: StateCreateGameGroup\n");
		else if (m_NextStateFuncCb == &cAdhoc::StateHandleError)
			debug("[Adhoc]: State: StateHandleError\n");
		else if (m_NextStateFuncCb == &cAdhoc::StateAttemptGameScan)
			debug("[Adhoc]: State: StateAttemptGameScan\n");
		else if (m_NextStateFuncCb == &cAdhoc::StateIdle2)
			debug("[Adhoc]: State: StateIdle2\n");
		else if (m_NextStateFuncCb == &cAdhoc::StateLobbyConnect)
			debug("[Adhoc]: State: StateLobbyConnect\n");
		else
			debug("[Adhoc]: State: Unknown state\n");
	}

//	inline void NextState()
//	{
//		if (m_NextStateFuncCb)
//			(this->*m_NextStateFuncCb)();
//
//#ifdef DEBUG_MULTIGAME
//		if (m_NextStateFuncCb == &cAdhoc::StateShutdown)
//			debug("[Adhoc]: Call State: StateShutdown\n");
//		else if (m_NextStateFuncCb == &cAdhoc::StateInitialise)
//			debug("[Adhoc]: Call State: StateInitialise\n");
//		else if (m_NextStateFuncCb == &cAdhoc::StateConnectLobbyGroup)
//			debug("[Adhoc]: Call State: StateConnectLobbyGroup\n");
//		else if (m_NextStateFuncCb == &cAdhoc::StateCancelAllTargets)
//			debug("[Adhoc]: Call State: StateCancelAllTargets\n");
//		else if (m_NextStateFuncCb == &cAdhoc::StateIdle)
//			debug("[Adhoc]: Call State: StateIdle\n");
//		else if (m_NextStateFuncCb == &cAdhoc::StateCancelAllTargetsIfNotJoined)
//			debug("[Adhoc]: Call State: StateCancelAllTargetsIfNotJoined\n");
//		else if (m_NextStateFuncCb == &cAdhoc::StateCreateGameGroup)
//			debug("[Adhoc]: Call State: StateCreateGameGroup\n");
//		else if (m_NextStateFuncCb == &cAdhoc::StateHandleError)
//			debug("[Adhoc]: Call State: StateHandleError\n");
//		else if (m_NextStateFuncCb == &cAdhoc::StateAttemptGameScan)
//			debug("[Adhoc]: Call State: StateAttemptGameScan\n");
//		else if (m_NextStateFuncCb == &cAdhoc::StateIdle2)
//			debug("[Adhoc]: Call State: StateIdle2\n");
//		else if (m_NextStateFuncCb == &cAdhoc::StateLobbyConnect)
//			debug("[Adhoc]: Call State: StateLobbyConnect\n");
//		else
//			debug("[Adhoc]: Call State: Unknown state\n");
//#endif
//	}

	static cAdhoc& Instance() {
		if (!ms_pInstance)
			ms_pInstance = new cAdhoc;
		return *ms_pInstance;
	}
};

#define TheAdhoc cAdhoc::Instance()