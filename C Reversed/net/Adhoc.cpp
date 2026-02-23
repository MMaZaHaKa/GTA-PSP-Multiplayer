/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "multiplayer/MultiGame.h"
#include "multiplayer/net/Adhoc.h"

#ifdef GTA_PSP
#include <kernel.h>
#include <psptypes.h>
#include <wlan.h>
#include <pspnet.h>
#include <pspnet_error.h>
#include <pspnet_adhoc.h>
#include <pspnet_adhocctl.h>
#include <pspnet_adhoc_discover.h>
#else
#include "multiplayer/net/emu/proAdhoc.h"
#include "multiplayer/net/emu/sceNet.h"
#include "multiplayer/net/emu/sceNetAdhoc.h"
#include "multiplayer/net/emu/sceNetAdhocMatching.h"
#include "multiplayer/net/emu/Utils.h"
#endif

#include "leeds/psp_compat.h"
#include "Text.h"
#include "Font.h"

bool g_networkModulesInitialized = false;
void mp_load_psp_net_modules() {
#ifdef GTA_PSP
	sceUtilityLoadNetModule(1);
	sceUtilityLoadNetModule(2);
#endif
	g_networkModulesInitialized = true;
}

int mp_thread_load_modules()
{
	if (cAdhoc::ms_bInitNetworkModules)
		return 0;

	cAdhoc::ms_bInitNetworkModules = false; // why????
	mp_load_psp_net_modules();
	cAdhoc::ms_bInitNetworkModules = true;
#ifdef GTA_PSP
	sceKernelExitThread();
#endif
	return 0;
}

void mp_unload_psp_net_modules() {
	if (!g_networkModulesInitialized)
		return;

#ifdef GTA_PSP
	sceUtilityUnloadNetModule(2);
	sceUtilityUnloadNetModule(1);
#endif
	g_networkModulesInitialized = false;
}

// in main lobby state
void adhocHandlerWrapper(int nEvent, int error, void* arg) {
	//((cAdhoc*)arg)->ScanHandler(flag, nError);
	TheAdhoc.ScanHandler(nEvent, error);
}

// When i am client, recv here
void adhocMatchingClientCB(int matchingid, int event, unsigned char* mac, int optlen, void* optdata) {
	TheAdhoc.OnMatchClient(matchingid, event, *(tMacAddr*)mac, optlen, optdata);
}

// When i am hoster, recv here
void adhocMatchingHostCB(int matchingid, int event, unsigned char* mac, int optlen, void* optdata) {
	TheAdhoc.OnMatchHost(matchingid, event, *(tMacAddr*)mac, optlen, optdata);
}

#if !defined(FINAL) && !defined(MASTER)
void TestStates() {
	using StateFn = void (cAdhoc::*)();
	StateFn states[] =
	{
		&cAdhoc::StateShutdown,
		&cAdhoc::StateInitialise,
		&cAdhoc::StateConnectLobbyGroup,
		&cAdhoc::StateCancelAllTargets,
		&cAdhoc::StateIdle,
		&cAdhoc::StateCancelAllTargetsIfNotJoined,
		&cAdhoc::StateCreateGameGroup,
		&cAdhoc::StateHandleError,
		&cAdhoc::StateAttemptGameScan,
		&cAdhoc::StateIdle2,
		&cAdhoc::StateLobbyConnect,
	};
	for (int32 i = 0; i < ARRAY_SIZE(states); i++) {
		for (int32 j = i + 1; j < ARRAY_SIZE(states); j++) {
			assert(states[i] != states[j]);
		}
	}
}
#endif

cAdhoc::cAdhoc() {
#if !defined(FINAL) && !defined(MASTER)
	TestStates(); // NOREF NOICF test
#endif
	m_bHasStartedMatching = false;
	SetNextState(&cAdhoc::StateShutdown);
	m_pMatchingInfoEntry = nil;
	m_LobbyHosterMacAddr = tMacAddr();
	m_nServerPeerID = -1;
	m_sGameRoom = nil;
	m_bHasExitedWithError = false;
	m_nErrorCode = 0;
	ResetEventsFlags();
	m_sExitReason = "";
	m_nAdhocEventHandler = -1;
	m_nAdhocMatchingID = -1;
	m_nWlanCounter = 0;
	// ctor tAdhocMatchingData m_aMatchingInfo MP_NUM_PEERS 152
	// ctor tAdhocPlayerData m_aMatchingPlayersInfo MP_NUM_PEERS 134
#ifdef GTA_PSP
	m_nAdhocConnSemaID = -1;
#else
	m_nAdhocConnSemaID = 0; // pc unlock init logic
#endif
	// ctor tAdhocMatchingData m_aMatchingInfoRecv 7 152
	m_MyMacAddr = tMacAddr();
	m_nAdhocPdp = cListenInfo();
	//m_nAdhocPdp.m_nPdpID = -1; // in ctor cListenInfo
	//m_nAdhocPdp.m_listenAddr.mac.InitMacAddr(); in ctor cListenInfo
	m_nAdhocPdp.m_listenAddr.port = GTA_MATCHING_PORT; // in ctor cListenInfo?
}

// GTA_PRODUCT_ID, GTA_TITLE_ID
void cAdhoc::Configure(const char* sGameID, const char* sGameRoom) {
#ifdef GTA_PC
	AdhocEmu_NativeInit();
#endif

	static_assert(sizeof(m_aMatchingInfo) == 1064 && sizeof(m_aMatchingInfoRecv) == 1064 && sizeof(m_aMatchingPlayersInfo) == 938);
	memset(m_aMatchingInfo, 0, sizeof(m_aMatchingInfo)); // 1064
	memset(m_aMatchingInfoRecv, 0, sizeof(m_aMatchingInfoRecv)); // 1064
	memset(m_aMatchingPlayersInfo, 0, sizeof(m_aMatchingPlayersInfo)); // 938
#if defined(GTA_PC) && defined(ADHOC_PTP_PDP_CHAT_EMU)
	if (g_Config.bPtpPdpDedicatedEmu && g_Config.bPtpPdpDedicatedEmuChatChannel) {
		m_sGameID = GTA_RE_PRODUCT_ID; // base 9
		m_sGameRoom = GTA_RE_TITLE_ID; // room 8

		static char* szChannel = nil;
		if (!szChannel)
			szChannel = (char*)malloc(ADHOCCTL_ADHOCID_LEN + 1);

		strncpy(szChannel, GTA_RE_PRODUCT_ID, ADHOCCTL_ADHOCID_LEN);
		szChannel[ADHOCCTL_ADHOCID_LEN] = '\0';
		int32 channel = g_Config.nPtpPdpDedicatedEmuChannel;

		if (channel >= 1 && channel <= 9) {
			szChannel[ADHOCCTL_ADHOCID_LEN - 1] = '0' + channel;
		}
		else if (channel >= 10 && channel <= 99) {
			szChannel[ADHOCCTL_ADHOCID_LEN - 2] = '0' + (channel / 10);
			szChannel[ADHOCCTL_ADHOCID_LEN - 1] = '0' + (channel % 10);
		}

		m_sGameID = szChannel;
	}
	else {
		m_sGameID = sGameID;
		m_sGameRoom = sGameRoom;
	}
#else
	m_sGameID = sGameID;
	m_sGameRoom = sGameRoom;
#endif
	m_bHasExitedWithError = false;
	m_nErrorCode = 0;
	ResetEventsFlags();
	m_bPendingHostStart = false;
	m_nWlanCounter = 0;
#ifdef GTA_PSP
	m_nLoadMPModulesThreadID = sceKernelCreateThread("LoadMultiplayerModules", mp_thread_load_modules,
		NET_MATCHING_TPL, NET_MATCHING_STACKSIZE, NET_MATCHING_ATTR, nil);
	sceKernelStartThread(m_nLoadMPModulesThreadID);
#else
	m_nLoadMPModulesThreadID = -1;
	ms_bInitNetworkModules = true;
#endif
	SetNextState(&cAdhoc::StateInitialise);
	SetServerConnLost(false);
#if !defined(FINAL) && !defined(MASTER)
	debug("[cAdhoc::Configure(GTA_PRODUCT_ID(GameID): %s, GTA_TITLE_ID(GameRoom): %s)\n]", m_sGameID, m_sGameRoom);
#endif
}

void cAdhoc::Update() {
	if (!IsWifiSwitchOn()) {
		base::string reason = base::string("Wireless networking switch turned off");
		TerminateWithError(reason);
		return;
	}

	WaitForSemaphore();
	static_assert(sizeof(m_aMatchingInfo) == sizeof(m_aMatchingInfoRecv));
	memcpy(m_aMatchingInfo, m_aMatchingInfoRecv, sizeof(m_aMatchingInfo));

	for (int32 index = 0; index < MP_NUM_PEERS; index++) {
		tAdhocMatchingData& item = m_aMatchingInfoRecv[index];
		if (item.nState == eAdhocPeerState::ADHOC_PEER_HELLOED) {
			if (item.nWaitTime++ > GTA_DEFAULT_WAIT_TIME)
				m_aMatchingInfoRecv[index].nState = eAdhocPeerState::ADHOC_PEER_DISCONNECTED;
		}
		int32 nonBroadcastCount = 0;
		for (int32 peer = 0; peer < MP_NUM_PEERS; peer++) {
			if (!item.entry.m_nPeersConnInfo[peer].macAddr.IsBroadcast())
				nonBroadcastCount++;
		}
		if (nonBroadcastCount == 0 && item.nState == eAdhocPeerState::ADHOC_PEER_HELLOED)
			item.nState = eAdhocPeerState::ADHOC_PEER_DISCONNECTED;
	}

	SignalSemaphore();
	NextState();
}

void cAdhoc::Terminate() {
	if (IsNextStateNow(&cAdhoc::StateShutdown))
		return;

#ifdef GTA_PSP
	sceKernelPowerLock(0);
#endif
	if (!IsNextStateNow(&cAdhoc::StateInitialise))
	{
		cListenInfo& pdp = GetAdhocPdp();
		pdp.ClosePDP();
		TerminateAdhocMatching();
		sceNetAdhocctlTerm();
		sceNetAdhocTerm();
		sceNetTerm();
	}
	else {
#ifdef GTA_PSP
		sceKernelWaitThreadEnd(m_nLoadMPModulesThreadID, nil);
		sceKernelTerminateDeleteThread(m_nLoadMPModulesThreadID);
#endif
	}
#ifdef GTA_PSP
	sceKernelPowerUnlock(0);
#endif
#ifdef GTA_LIBERTY
	mp_unload_psp_net_modules();
#endif
	ms_bInitNetworkModules = false;
	SetNextState(&cAdhoc::StateShutdown);

#ifdef GTA_PC
	AdhocEmu_NativeShutdown();
#endif
}

bool cAdhoc::StartHosting()
{
	const int32 nHostIdx = 0;

	m_bHasStartedMatching = true;
	m_nServerPeerID = nHostIdx;
	m_aMatchingInfoRecv[nHostIdx].nState = eAdhocPeerState::ADHOC_PEER_JOINED;
	m_aMatchingInfo[nHostIdx].nState = eAdhocPeerState::ADHOC_PEER_JOINED;
	m_pMatchingInfoEntry = &m_aMatchingInfoRecv[nHostIdx].entry;
	for (int32 index = 0; index < MP_NUM_PEERS; index++) {
		tAdhocPeerData& data = m_pMatchingInfoEntry->m_nPeersConnInfo[index];
		data.macAddr.InitMacAddr();
	}
	m_pMatchingInfoEntry->m_HostPeerData.peerAddr.mac = GetPlayerMacAddress();
	m_pMatchingInfoEntry->m_nPeersConnInfo[m_nServerPeerID].macAddr = GetPlayerMacAddress();
	BakeRandomGroupName(m_pMatchingInfoEntry->m_sGroupName);

	if (!IsNextStateNow(&cAdhoc::StateCancelAllTargets)) {
		m_bPendingHostStart = true;
		return false;
	}

	TerminateAdhocMatching();

	m_nErrorCode = sceNetAdhocMatchingInit(GTA_MATCHING_POOLSIZE);
	if (m_nErrorCode < 0) {
		base::string reason = base::string("Error initialising adhoc matching");
		TerminateWithError(reason);
		return false;
	}

	m_nAdhocMatchingID = sceNetAdhocMatchingCreate(PSP_ADHOC_MATCHING_MODE_HOST, (MP_NUM_PEERS - 1), GTA_MATCHING_PORT,
		GTA_MATCHING_RXBUFLEN, GTA_HELLO_INTERVAL, GTA_KEEPALIVE_INTERVAL, GTA_KEEPALIVE_COUNT, GTA_MATCHING_REXMT_INTERVAL, adhocMatchingHostCB);
	if (m_nAdhocMatchingID < 0) {
		m_nErrorCode = m_nAdhocMatchingID;
		base::string reason = base::string("Error creating adhoc matching context");
		TerminateWithError(reason);
		return false;
	}

	static_assert(sizeof(tLobbyRemoteInfo) == 136, "tLobbyRemoteInfo");
	m_nErrorCode = sceNetAdhocMatchingStart(m_nAdhocMatchingID, GTA_MATCHING_EVENT_TPL, GTA_EV_TH_STACK, GTA_MATCHING_INPUT_TPL, GTA_IN_TH_STACK,
		sizeof(tLobbyRemoteInfo), m_pMatchingInfoEntry);
	if (m_nErrorCode < 0) {
		base::string reason = base::string("Failed to start adhoc matching");
		TerminateWithError(reason);
		return false;
	}

	SetNextState(&cAdhoc::StateIdle);
	return true;
}

void cAdhoc::OnStartGame() {
	for (int32 index = 0; index < MP_NUM_PEERS; index++) {
		tAdhocPeerData* peer = &m_pMatchingInfoEntry->m_nPeersConnInfo[index];
		if (peer->macAddr == GetPlayerMacAddress()) {
			cMultiGame::s_nSelectedTeam = static_cast<eGameTeam>(peer->nTeamID);
			break;
		}
	}

	ResetEventsFlags();
#ifdef FIX_BUGS
	debug("************** sceNetAdhocctlDisconnect()\n");
#else
	debug("************** sceNetAdhocctlDisconnect()");
#endif

	if (sceNetAdhocctlDisconnect() >= 0)
		SetNextState(&cAdhoc::StateCreateGameGroup);
	else {
		base::string reason("Failed to disconnect from lobby");
		TerminateWithError(reason);
	}
}

void cAdhoc::Disconnect() {
	memset(m_aMatchingInfo, 0, sizeof(m_aMatchingInfo)); // 1064
	memset(m_aMatchingInfoRecv, 0, sizeof(m_aMatchingInfoRecv)); // 1064
	memset(m_aMatchingPlayersInfo, 0, sizeof(m_aMatchingPlayersInfo)); // 938
	ResetEventsFlags();

	if (IsWifiSwitchOn())
	{
		if (sceNetAdhocctlDisconnect() >= 0)
			SetNextState(&cAdhoc::StateLobbyConnect);
		else {
			base::string reason = base::string("Failed to disconnect from game");
			TerminateWithError(reason);
		}
		return;
	}
	SetNextState(&cAdhoc::StateShutdown);
}

tLobbyRemoteInfo* cAdhoc::GetMatchingInfo(int index) // for player use -1 MP_HOST_INDEX
{
	if (index < 0) index = m_nServerPeerID;
	assert(index > -1);
	tAdhocMatchingData& data = m_aMatchingInfo[index];
	if (data.nState) return &data.entry;
	return nil;
}

void cAdhoc::SelectServer(int index)
{
	WaitForSemaphore();
	if (m_aMatchingInfo[index].nState == eAdhocPeerState::ADHOC_PEER_HELLOED &&
		sceNetAdhocMatchingSelectTarget(m_nAdhocMatchingID, m_aMatchingInfoRecv[index].addr.GetBytesSCE(), 0, nil) >= 0)
	{
		m_LobbyHosterMacAddr = m_aMatchingInfoRecv[index].addr;
		m_aMatchingInfoRecv[index].nState = eAdhocPeerState::ADHOC_PEER_SELECTED;
		m_nServerPeerID = index;
	}
	SignalSemaphore();
}

void cAdhoc::CancelMatchingTarget() {
	if (m_nServerPeerID != -1)
	{
		CancelMatchingForPeer(m_nServerPeerID);
		if (!m_bPendingHostStart)
			SetNextState(&cAdhoc::StateCancelAllTargets);
		m_nServerPeerID = -1;
	}

	if (m_bPendingHostStart)
		m_bPendingHostStart = false;
	StartListening();
}

int32 gta_mp_adhoc_copy_wide_str(wchar* outBuffer, uint8 bufferSize, char* inputStr)
{
	if (bufferSize < sizeof(wchar))
		return 0;

#if 1 // tmp dummy
	TODO();
	TODO();
	TODO();
	TODO();

	uint32 maxChars = (bufferSize / sizeof(wchar)) - 1;
	uint32 i = 0;
	for (; i < maxChars && inputStr[i] != '\0'; ++i) {
		outBuffer[i] = static_cast<wchar>(inputStr[i]);
	}
	outBuffer[i] = L'\0';
	return i;
#else
	uint8 remainingBytes = bufferSize - sizeof(wchar);

	char* currentInput = inputStr;
	wchar* currentOutput = outBuffer;
	int32 result = 0;

	wchar tempBuff[4];

	while (true)
	{
		uint8 charInfo = 0; //sub_8B64F50(&currentInput); // TODO: Next char??

		if (charInfo == 0)
			break;

		//sub_8B64EDC(tempBuff, charInfo); // TODO: ???
		TODO();
		TODO();
		TODO();

		int32 wideCharCount = 0;
		while (wideCharCount < sizeof(tempBuff) / sizeof(wchar) && tempBuff[wideCharCount] != L'\0')
		{
			wideCharCount++;
		}

		uint8 requiredBytes = wideCharCount * sizeof(wchar);

		if (requiredBytes > remainingBytes)
			break;

		for (int32 i = 0; i < wideCharCount; ++i)
			*currentOutput++ = tempBuff[i];

		remainingBytes -= requiredBytes;
		result += wideCharCount;
}

	*currentOutput = L'\0';

	return result;
#endif
}

void cAdhoc::FormatPlayerName(base::string& outName, char* sNickname) {
//#ifdef FIX_BUGS
//	if (!sNickname || *sNickname == '\0') {
//		outName = base::string(UnicodeToAscii(TheText.Get("NO_NAME")));
//		return;
//	}
//#endif

	uint32 len = strlen(sNickname);
	if (len > 0) {
		wchar* wideBuf = new wchar[len + 2];
		memset(wideBuf, 0, sizeof(wchar) * (len + 2));
		int32 effectiveLen = gta_mp_adhoc_copy_wide_str(wideBuf, sizeof(wchar) * (len + 1), sNickname);

		bool hasNonSpace = false;
		int32 checkLimit = (effectiveLen > MAX_VISIBLE_NICKNAME_CHARS) ? effectiveLen : MAX_VISIBLE_NICKNAME_CHARS;
		for (int32 i = 0; i < checkLimit; ++i) {
			if (wideBuf[i] != L' ' && wideBuf[i] != L'\0') {
				hasNonSpace = true;
				break;
			}
		}

		if (!hasNonSpace) {
			outName = base::string(UnicodeToAscii(TheText.Get("NO_NAME")));
			delete[] wideBuf;
			return;
		}

		wchar* gameTextBuf = new wchar[effectiveLen + 1];
		memset(gameTextBuf, 0, sizeof(wchar) * (effectiveLen + 1));
		ConvertUnicodeToGameText(gameTextBuf, wideBuf, 0);
		CFont::SetFontStyle(FONT_STANDARD); // why?
		TODO();
		//get_Pointer_glyphinst_sub_8B0C748()->font_FixupTextBuff_str_sub_8AFD8D8(gameTextBuf);
		CFont::ClearTokensFromString(gameTextBuf);

		char* ansiBuf = UnicodeToAscii(gameTextBuf);
		base::string tempStr(ansiBuf, strlen(ansiBuf));
		delete[] gameTextBuf;
		delete[] wideBuf;

		if (tempStr.length() > MAX_VISIBLE_NICKNAME_CHARS) {
			tempStr = tempStr.SubStr(0, MAX_VISIBLE_NICKNAME_CHARS);
			tempStr.Append("...");
		}

		outName = tempStr;
}
	else {
		outName = base::string(UnicodeToAscii(TheText.Get("NO_NAME")));
	}
}

void cAdhoc::GetPlayerName(base::string& outName, tAdhocPeerData& pPlayerInfo) {
	for (int32 i = 0; i < MP_NUM_PEERS; ++i) {
		tAdhocPlayerData& player = m_aMatchingPlayersInfo[i];
		if (player.m_PlayerMacAddr == pPlayerInfo.macAddr) {
			base::string formatted;
			FormatPlayerName(formatted, player.m_szPlayerNickname);
			outName = formatted;
			return;
		}
	}

	outName = base::string(UnicodeToAscii(TheText.Get("NO_NAME")));
}

void cAdhoc::GetPlayerNameFromMacAddr(base::string& outName, tMacAddr& macAddr) {
	int32 slot = CGeneral::GetRandomNumberInRange(0, MP_NUM_PEERS);
	for (int32 i = 0; i < MP_NUM_PEERS; ++i) {
		tAdhocPlayerData& player = m_aMatchingPlayersInfo[i];
		if (player.m_szPlayerNickname[0] == '\0') {
			slot = i;
			continue;
		}
		if (player.m_PlayerMacAddr == macAddr) {
			FormatPlayerName(outName, player.m_szPlayerNickname);
			return;
		}
	}

	tAdhocPlayerData& target = m_aMatchingPlayersInfo[slot];
	target.m_PlayerMacAddr = macAddr;
	char* nickname = target.m_szPlayerNickname;
	if (macAddr == GetPlayerMacAddress()) {
		if (sceUtilityGetSystemParamString(PSP_SYSTEMPARAM_ID_STRING_NICKNAME, nickname, SCE_NET_ADHOCCTL_NICKNAME_LEN) < 0)
			nickname[0] = '\0'; // err
	}
	else {
		if (sceNetAdhocctlGetNameByAddr(macAddr.GetBytesSCE(), (SceNetAdhocctlNickname*)nickname) < 0)
			nickname[0] = '\0';
	}

	if (nickname[0] == '\0') {
		char buf[32];
		sprintf(buf, " ");
		outName = base::string(buf, strlen(buf));
	}
	else {
		FormatPlayerName(outName, nickname);
	}
}


void cAdhoc::FindAndClearMacAddr(tMacAddr& macAddr) {
	for (int32 index = 0; index < MP_NUM_PEERS; index++) {
		if (m_aMatchingPlayersInfo[index].m_PlayerMacAddr == macAddr) {
			m_aMatchingPlayersInfo[index].m_PlayerMacAddr.InitMacAddr();
		}
	}
}

void cAdhoc::SetGameParams(tLobbyRemoteInfo& info) {
	if (IsHost())
		return;

	WaitForSemaphore();
	// ah just memcpy(...) 8*17 = 136
	// Note: ida decomp  v7 = 17;  v6 += 8;  17*8=136 simple copy, todo remove this comment when all 17 +=8 done, this comment helper
	memcpy(m_pMatchingInfoEntry, &info, sizeof(tLobbyRemoteInfo));
	SignalSemaphore();
	UpdateGameParams();
}

void cAdhoc::SetPeerConnInfo(int nPeerId, tLobbyRemoteInfo& info) {
	WaitForSemaphore();
	m_pMatchingInfoEntry->m_nPeersConnInfo[nPeerId].nSelectedPeerModelID = info.m_nPeersConnInfo[nPeerId].nSelectedPeerModelID;
	m_pMatchingInfoEntry->m_nPeersConnInfo[nPeerId].nTeamID = info.m_nPeersConnInfo[nPeerId].nTeamID;
	SignalSemaphore();
}

bool cAdhoc::IsWifiSwitchOn() {
	if (m_nWlanCounter > 0)
		--m_nWlanCounter;
	if (m_nWlanCounter == 0 && !sceWlanGetSwitchState())
		m_nWlanCounter = GTA_WLAN_COOLDOWN;
	return m_nWlanCounter == 0;
}

int cAdhoc::GetLocalMacAddr() {
	if (ms_bInitNetworkModules)
		return sceNetGetLocalEtherAddr(m_MyMacAddr.GetBytesSCE());
#ifdef FIX_BUGS
	return 0;
#endif
}

tMacAddr& cAdhoc::GetPlayerMacAddress() {
	if (m_MyMacAddr.IsBroadcast()) GetLocalMacAddr();
	return m_MyMacAddr;
}

base::string& cAdhoc::GetExitReason() {
	return m_sExitReason;
}

cListenInfo& cAdhoc::GetAdhocPdp() {
	return m_nAdhocPdp;
}

uint8 cAdhoc::GetNumberOfConnectedPlayers() {
	uint8 nNum = 0;
	for (int32 i = 0; i < MP_NUM_PEERS; ++i) {
		tAdhocPeerData* peer = &GetMatchingInfo(MP_HOST_INDEX)->m_nPeersConnInfo[i];
		if (!peer->macAddr.IsBroadcast())
			++nNum;
	}
	return nNum;
}

uint8 cAdhoc::GetNumberOfNonEmptyGangs() {
	uint8 nNumTeamA = 0;
	uint8 nNumTeamB = 0;
	for (int32 i = 0; i < MP_NUM_PEERS; ++i) {
		tAdhocPeerData* peer = &GetMatchingInfo(MP_HOST_INDEX)->m_nPeersConnInfo[i];
		if (!peer->macAddr.IsBroadcast()) {
			if (peer->nTeamID == (int16)eGameTeam::TEAM_A)
				nNumTeamA = 1;
			if (peer->nTeamID == (int16)eGameTeam::TEAM_B)
				nNumTeamB = 1;
		}
	}
	return nNumTeamA + nNumTeamB;
}

void cAdhoc::AssignPlayerIDs() {
	cMultiGame& Game = cMultiGame::Instance();
	if (!IsHost())
		return;

	for (int32 i = 0; i < (MP_NUM_PEERS - 1); i++)
		m_pMatchingInfoEntry->m_aPlayerID[i] = i + 1; // assigning each player's ID, sequentially

	for (int32 i = 0; i < (MP_NUM_PEERS - 1); i++)
	{
		// shuffles player's ID with each other
		uint8 id = m_pMatchingInfoEntry->m_aPlayerID[i];
		int32 rnd = base::Random();
//#ifdef FIX_BUGS
//		m_pMatchingInfoEntry->m_aPlayerID[i] = m_pMatchingInfoEntry->m_aPlayerID[rnd % MP_MAX_PLAYERS];
//		m_pMatchingInfoEntry->m_aPlayerID[rnd % MP_MAX_PLAYERS] = id;
//#else
		m_pMatchingInfoEntry->m_aPlayerID[i] = m_pMatchingInfoEntry->m_aPlayerID[rnd % (MP_NUM_PEERS - 2)];
		m_pMatchingInfoEntry->m_aPlayerID[rnd % (MP_NUM_PEERS - 2)] = id;
//#endif
	}

	for (int32 i = 0; i < MP_MAX_NUM_PEERS; i++) // hmm, does MP planned to have 8 players?
		Game.m_aPlayerID[i] = m_pMatchingInfoEntry->m_aPlayerID[i];
}

void cAdhoc::TerminateAdhocMatching() {

	if (m_nAdhocMatchingID < 0)
		return;

	sceNetAdhocMatchingStop(m_nAdhocMatchingID);
	sceNetAdhocMatchingDelete(m_nAdhocMatchingID);
	sceNetAdhocMatchingTerm();
	m_nAdhocMatchingID = -1;
}

void cAdhoc::SetServerConnLost(bool bIsLost) {
	m_bIsServerConnLost = bIsLost;
}

bool cAdhoc::GetServerConnLost() {
	return m_bIsServerConnLost;
}

void cAdhoc::CleanupNetworkModules() {
	mp_unload_psp_net_modules();
	ms_bInitNetworkModules = false;
}


// States
void cAdhoc::StateShutdown() {
	DONT_OPTIMIZE();
}

void cAdhoc::StateInitialise() {
	DONT_OPTIMIZE();

#if !defined(FINAL) && !defined(MASTER)
#define ADHOC_DEBUG_CHECK_INIT(s) debug("%s\n", s.c_str()); assert(false && "sce init fakap");
#else
#define ADHOC_DEBUG_CHECK_INIT(s)
#endif

#ifdef GTA_PSP
	if (sceKernelWaitThreadEnd() < 0)
		return;

	sceKernelTerminateDeleteThread(m_nLoadMPModulesThreadID);
#endif

	m_nErrorCode = sceNetInit(GTA_PSPNET_POOLSIZE, GTA_CALLOUT_TPL, 0, GTA_NETINTR_TPL, 0);
	if (m_nErrorCode >= 0)
	{
		m_nErrorCode = sceNetAdhocInit();
		if (m_nErrorCode >= 0)
		{
#ifdef GTA_PSP
			productStruct prod;
			prod.unknown = 0;
			memcpy(prod.product, m_sGameID, sizeof(prod.product));
			m_nErrorCode = sceNetAdhocctlInit(GTA_ADHOCCTL_STACKSIZE, GTA_ADHOCCTL_TPL, &prod);
#else
			SceNetAdhocctlAdhocId prod;
			prod.type = 0;
			memcpy(prod.data, m_sGameID, sizeof(prod.data));
			m_nErrorCode = sceNetAdhocctlInit(GTA_ADHOCCTL_STACKSIZE, GTA_ADHOCCTL_TPL, &prod);
#endif
			if (m_nErrorCode >= 0)
			{
				m_nAdhocEventHandler = sceNetAdhocctlAddHandler(adhocHandlerWrapper, this);
				if (m_nAdhocEventHandler >= 0)
				{
#ifdef GTA_PSP
					m_nAdhocConnSemaID = sceKernelCreateSema();
#endif
					if (m_nAdhocConnSemaID >= 0)
					{
						m_nErrorCode = 0;
						SetStateGroupConnect();
						return;
					}

					m_nErrorCode = m_nAdhocConnSemaID;
					m_sExitReason = base::string("sceKernelCreateSema() failed");
					ADHOC_DEBUG_CHECK_INIT(m_sExitReason);
				}
				else
				{
					m_nErrorCode = m_nAdhocEventHandler;
					m_sExitReason = base::string("Failed to initialised PSP adhoc network event handler");
					ADHOC_DEBUG_CHECK_INIT(m_sExitReason);
				}
				sceNetAdhocctlTerm();
			}
			else
			{
				m_sExitReason = base::string("Failed to initialise PSP adhoc network connection library");
				ADHOC_DEBUG_CHECK_INIT(m_sExitReason);
			}
			sceNetAdhocTerm();
		}
		else
		{
			m_sExitReason = base::string("Failed to initialise PSP adhoc networking");
			ADHOC_DEBUG_CHECK_INIT(m_sExitReason);
		}
		sceNetTerm();
	}
	else
	{
		m_sExitReason = base::string("Failed to initialise PSP networking");
		ADHOC_DEBUG_CHECK_INIT(m_sExitReason);
	}

	// fail net init, unloading
	mp_unload_psp_net_modules();
	ms_bInitNetworkModules = false;
	SetExitError();
	SetNextState(&cAdhoc::StateShutdown);
#undef ADHOC_DEBUG_CHECK_INIT
}

void cAdhoc::StateConnectLobbyGroup() {
	DONT_OPTIMIZE();

	debug("***** Connecting to lobby group\n");
	if (bHasError)
	{
		base::string reason = base::string("Error connecting to lobby");
		TerminateWithError(reason);
	}
	else if (bConnEvent)
	{
		debug("***** Connected to lobby group\n");
		for (int32 i = 0; i < MP_NUM_PEERS; ++i) {
			tLobbyRemoteInfo* p_entry = &m_aMatchingInfo[i].entry;
			p_entry->m_HostPeerData.peerAddr.mac.InitMacAddr();
			for (int32 j = 0; j < MP_NUM_PEERS; ++j)
				p_entry->m_nPeersConnInfo[j].macAddr.InitMacAddr();
		}
		m_LobbyHosterMacAddr.InitMacAddr();
		GetLocalMacAddr();

		if (m_bPendingHostStart) {
			m_bPendingHostStart = false;
			SetNextState(&cAdhoc::StateCancelAllTargets);
			StartHosting();
		}
		else {
			debug("Populate2\n");
			StartListening();
			debug("Populate2a\n");
		}
	}
}

void cAdhoc::StateCancelAllTargets() {
	DONT_OPTIMIZE();

	for (int32 peeridx = 0; peeridx < MP_NUM_PEERS; peeridx++)
	{
		if (m_aMatchingInfo[peeridx].nState == eAdhocPeerState::ADHOC_PEER_JOINED)
		{
			if (m_nServerPeerID == peeridx)
				SetNextState(&cAdhoc::StateCancelAllTargetsIfNotJoined);
			else
				CancelMatchingForPeer(peeridx);
		}
	}
}

void cAdhoc::StateIdle() {
	DONT_OPTIMIZE();
}

void cAdhoc::StateCancelAllTargetsIfNotJoined() {
	DONT_OPTIMIZE();

	if (m_aMatchingInfo[m_nServerPeerID].nState == eAdhocPeerState::ADHOC_PEER_JOINED)
		return;

	SetNextState(&cAdhoc::StateCancelAllTargets);
	m_nServerPeerID = -1;
}

void cAdhoc::StateCreateGameGroup() {
	DONT_OPTIMIZE();

	if (bDisconnEvent)
	{
		char EtherN[20];
		sceNetEtherNtostr(m_pMatchingInfoEntry->m_HostPeerData.peerAddr.mac.GetBytesSCE(), EtherN);
		base::string sEtherN = base::string(EtherN); // useless
		memset(m_sGroupName, 0, ADHOCCTL_GROUPNAME_LEN);
		strncpy(m_sGroupName, m_pMatchingInfoEntry->m_sGroupName, ADHOCCTL_GROUPNAME_LEN);
		debug("Joining game group %c%c%c%c%c%c%c%c\n", m_sGroupName[0], m_sGroupName[1], m_sGroupName[2],
			m_sGroupName[3], m_sGroupName[4], m_sGroupName[5], m_sGroupName[6], m_sGroupName[7]);
		ResetEventsFlags();

		if (IsHost())
		{
			m_nErrorCode = sceNetAdhocctlCreate(m_sGroupName);
			if (m_nErrorCode < 0) {
				base::string reason = base::string("Failed to create game group");
				TerminateWithError(reason);
			}
			SetNextState(&cAdhoc::StateHandleError);
		}
		else
		{
			m_nScanAttemptCount = 0;
			m_nErrorCode = sceNetAdhocctlScan();
			if (m_nErrorCode < 0)
			{
				base::string reason = base::string("Failed to start scan for game group");
				TerminateWithError(reason);
			}
			SetNextState(&cAdhoc::StateAttemptGameScan);
		}
	}
	else if (bHasError)
	{
		base::string reason = base::string("Failed to disconnect from lobby");
		TerminateWithError(reason);
	}
}

void cAdhoc::StateHandleError() {
	DONT_OPTIMIZE();

	if (bHasError) {
		base::string reason = base::string("Error connecting to game");
		TerminateWithError(reason);
	}
	else if (bConnEvent)
		SetNextState(&cAdhoc::StateIdle2);
}

void cAdhoc::StateAttemptGameScan() {
	DONT_OPTIMIZE();

	if (bHasError) {
		base::string reason("Error connecting to game group");
		TerminateWithError(reason);
	}
	else if (bScanEvent)
	{
		ResetEventsFlags();
		SceNetAdhocctlScanInfo scanBuf;
		int32 bufSize = sizeof(scanBuf);
		m_nErrorCode = sceNetAdhocctlGetScanInfo(&bufSize, &scanBuf);

		if (m_nErrorCode >= 0)
		{
			SceNetAdhocctlScanInfo* scanInfo = (bufSize > 0) ? (SceNetAdhocctlScanInfo*)&scanBuf : nil;

			// Traverse linked list to find matching group name
			for (; scanInfo; scanInfo = scanInfo->next) {
				if (memcmp(scanInfo->group_name.data, m_sGroupName, ADHOCCTL_GROUPNAME_LEN) == 0) {
					break;
				}
			}

			if (scanInfo)
			{
				// Found matching group, attempt to join
				m_nErrorCode = sceNetAdhocctlJoin(scanInfo);
				if (m_nErrorCode >= 0)
					SetNextState(&cAdhoc::StateHandleError);
				else {
					base::string reason("Error joining game group");
					TerminateWithError(reason);
				}
			}
			else
			{
				// No match, retry scan if attempts remain
				++m_nScanAttemptCount;
				if (m_nScanAttemptCount < GTA_SCAN_ATTEMPT_COUNT)
				{
					m_nErrorCode = sceNetAdhocctlScan();
					if (m_nErrorCode < 0) {
						base::string reason("Failed to start scan for game group");
						TerminateWithError(reason);
					}
				}
				else {
					// Timeout after 10 attempts
					base::string reason("Timed out scanning for game group");
					TerminateWithError(reason);
					Terminate();
				}
			}
		}
		else {
			base::string reason("Error scanning for game group");
			TerminateWithError(reason);
		}
	}
}

void cAdhoc::StateIdle2() {
	DONT_OPTIMIZE();
}

void cAdhoc::StateLobbyConnect() {
	DONT_OPTIMIZE();

	if (bDisconnEvent)
		SetStateGroupConnect();
	else if (bHasError) {
		base::string reason = base::string("Failed to disconnect from lobby");
		TerminateWithError(reason);
	}
}


void cAdhoc::SetStateGroupConnect() {
	if (ConnectToGroup(m_sGameRoom))
		SetNextState(&cAdhoc::StateConnectLobbyGroup);
}

bool cAdhoc::ConnectToGroup(const char* gameRoom) {
	char szGameRoom[8];
	memset(szGameRoom, 0, sizeof(szGameRoom));
	memcpy(szGameRoom, gameRoom, sizeof(szGameRoom));
	ResetEventsFlags();
	if (sceNetAdhocctlConnect(szGameRoom) >= 0)
		return true;

	base::string reason = base::string("Failed to connect to group ");
	TerminateWithError(reason);
	return false;
}

bool cAdhoc::StartListening() {
	TerminateAdhocMatching();
	memset(m_aMatchingInfo, 0, sizeof(m_aMatchingInfo));
	memset(m_aMatchingInfoRecv, 0, sizeof(m_aMatchingInfoRecv));
	memset(m_aMatchingPlayersInfo, 0, sizeof(m_aMatchingPlayersInfo));
	m_nErrorCode = sceNetAdhocMatchingInit(GTA_MATCHING_POOLSIZE);

	if (m_nErrorCode < 0) {
		base::string reason = base::string("Error initialising adhoc matching");
		TerminateWithError(reason);
		return false;
	}

	m_nAdhocMatchingID = sceNetAdhocMatchingCreate(PSP_ADHOC_MATCHING_MODE_CLIENT, (MP_NUM_PEERS - 1), GTA_MATCHING_PORT,
		GTA_MATCHING_RXBUFLEN, GTA_HELLO_INTERVAL, GTA_KEEPALIVE_INTERVAL, GTA_KEEPALIVE_COUNT, GTA_MATCHING_REXMT_INTERVAL, adhocMatchingClientCB);
	if (m_nAdhocMatchingID < 0) {
		m_nErrorCode = m_nAdhocMatchingID;
		base::string reason = base::string("Error creating adhoc matching context");
		TerminateWithError(reason);
		return false;
	}

	m_nErrorCode = sceNetAdhocMatchingStart(m_nAdhocMatchingID, GTA_MATCHING_EVENT_TPL, GTA_EV_TH_STACK, GTA_MATCHING_INPUT_TPL, GTA_IN_TH_STACK, 0, nil);
	if (m_nErrorCode < 0) {
		base::string reason = base::string("Failed to start adhoc matching");
		TerminateWithError(reason);
		return false;
	}

	m_bHasStartedMatching = false;
	SetNextState(&cAdhoc::StateCancelAllTargets);
	m_nServerPeerID = -1;
	return true;
}

void cAdhoc::CancelMatchingForPeer(int32 peerId) {
	WaitForSemaphore();
	if (IsHost() && m_nServerPeerID == peerId)
	{
		m_aMatchingInfoRecv[peerId].nState = eAdhocPeerState::ADHOC_PEER_DISCONNECTED;
		m_aMatchingInfo[peerId].nState = eAdhocPeerState::ADHOC_PEER_DISCONNECTED;
	}
	else
	{
		sceNetAdhocMatchingCancelTarget(m_nAdhocMatchingID, m_aMatchingInfoRecv[peerId].addr.GetBytesSCE());
		m_aMatchingInfoRecv[peerId].nState = eAdhocPeerState::ADHOC_PEER_HELLOED;
		m_aMatchingInfo[peerId].nState = eAdhocPeerState::ADHOC_PEER_HELLOED;
	}
	SignalSemaphore();
}

void cAdhoc::TerminateWithError(base::string& reason) {
	Terminate();
	SetExitError();
	m_sExitReason = reason;
}

int cAdhoc::FindMacAddr(tMacAddr& macAddr) {
	WaitForSemaphore();

	int32 emptySlot = -1;
	for (int32 index = 0; index < MP_NUM_PEERS; index++) {
		if (m_aMatchingInfoRecv[index].nState != eAdhocPeerState::ADHOC_PEER_DISCONNECTED)
		{
			if (m_aMatchingInfoRecv[index].addr == macAddr) // memcmp vcs
			{
				SignalSemaphore();
				return index;
			}
		}
		else if (emptySlot == -1)
			emptySlot = index;
	}

	if (emptySlot < 0) {
		SignalSemaphore();
		return -1;
	}

	if (!IsNextStateNow(&cAdhoc::StateInitialise))
	{
		SceNetAdhocctlNickname name = { 0 };
		if (sceNetAdhocctlGetNameByAddr(macAddr.GetBytesSCE(), &name) < 0)
		{
			if (macAddr != GetPlayerMacAddress()) {
				SignalSemaphore();
				return -1;
			}
		}
	}

	m_aMatchingInfoRecv[emptySlot].nState = eAdhocPeerState::ADHOC_PEER_PENDING;
	m_aMatchingInfoRecv[emptySlot].addr = macAddr;
	SignalSemaphore();
	return emptySlot;
}

int cAdhoc::WaitForSemaphore() {
#ifdef GTA_PSP
	return sceKernelWaitSema(m_nAdhocConnSemaID, 1, 0);
#else
	return 0;
#endif
}

void cAdhoc::SignalSemaphore() {
#ifdef GTA_PSP
	sceKernelSignalSema(m_nAdhocConnSemaID, 1); 
#endif
}

void cAdhoc::BakeRandomGroupName(char* sGroupName)
{
	memset(sGroupName, 0, SCE_NET_ADHOCCTL_GROUPNAME_LEN);
	for (int32 i = 0; i < SCE_NET_ADHOCCTL_GROUPNAME_LEN; ++i)
		sGroupName[i] = 'A' + CGeneral::GetRandomNumberInRange(0, 25);
	debug("GENERATED RANDOMISH GROUP NAME: %c%c%c%c%c%c%c%c\n", sGroupName[0], sGroupName[1],
		sGroupName[2], sGroupName[3], sGroupName[4], sGroupName[5], sGroupName[6], sGroupName[7]);
}

void cAdhoc::ScanHandler(int nEvent, int nError) { // leeds paste it from psp docs example (me too) adhocctl_handler(int event, int error, void *arg)
#if !defined(FINAL) && !defined(MASTER)
	debug("event: [%d] error_code = 0x%x\n", nEvent, nError);
#endif
	if (nEvent == ADHOCCTL_EVENT_ERROR) {
		m_nErrorCode = nError;
		//m_nEventFlags |= GTA_EVF_ERROR;
		//bHasError = true; // EVF_ERROR
		SetHasError();
	}
	else if (nEvent == ADHOCCTL_EVENT_CONNECT) {
		m_nErrorCode = 0;
		//m_nEventFlags |= GTA_EVF_CONNECT;
		bConnEvent = true; // EVF_CONNECT
	}
	else if (nEvent == ADHOCCTL_EVENT_DISCONNECT) {
		m_nErrorCode = 0;
		//m_nEventFlags |= GTA_EVF_DISCONNECT;
		bDisconnEvent = true; // EVF_DISCONNECT
	}
	else if (nEvent == ADHOCCTL_EVENT_SCAN) {
		m_nErrorCode = 0;
		//m_nEventFlags |= GTA_EVF_SCAN;
		bScanEvent = true; // EVF_SCAN
	}
	// missed
	//else if (nEvent == ADHOCCTL_EVENT_GAME) {}
	//else if (nEvent == ADHOCCTL_EVENT_DISCOVER) {}
	//else if (nEvent == ADHOCCTL_EVENT_WOL) {}
	//else if (nEvent == ADHOCCTL_EVENT_WOL_INTERRUPT) {}
}

void cAdhoc::UpdateGameParams() {
	cMultiGame& Game = cMultiGame::Instance();
	WaitForSemaphore();
	if (!IsHost()) // get params from hoster
	{
		Game.SetGameLocation(static_cast<eGameLocation>(m_pMatchingInfoEntry->m_GameLocation));
		Game.SetGameType(static_cast<eGameType>(m_pMatchingInfoEntry->m_GameType));
		Game.SetScoreLimit(m_pMatchingInfoEntry->m_nScoreLimit);
		Game.SetCTFScoreLimit(m_pMatchingInfoEntry->m_nScoreCTFLimit);
		Game.SetTimeLimit(m_pMatchingInfoEntry->m_nTimeLimit);
		Game.m_nFlags = m_pMatchingInfoEntry->m_nFlags;
		Game.m_nScenarioOrRaceTrackID = m_pMatchingInfoEntry->m_nScenarioOrRaceTrackID;
		Game.m_nRaceCarID = m_pMatchingInfoEntry->m_nRaceCarID;
		Game.m_nAmbientCarBank = m_pMatchingInfoEntry->m_nAmbientCarBank;
		Game.m_nAmbientPedBank = m_pMatchingInfoEntry->m_nAmbientPedBank;
		Game.SetCashTarget(m_pMatchingInfoEntry->m_nCashTarget);
		Game.m_nCutsceneFlags = m_pMatchingInfoEntry->m_nCutsceneFlags;
		Game.SetTeam1GangID(m_pMatchingInfoEntry->m_Team1GangID);
		Game.SetTeam2GangID(m_pMatchingInfoEntry->m_Team2GangID);
		for (int32 index = 0; index < MP_MAX_NUM_PEERS; index++)
			Game.m_aPlayerID[index] = m_pMatchingInfoEntry->m_aPlayerID[index];
	}
	else // hoster set our public lobby params
	{
		for (int32 index = 0; index < MP_NUM_PEERS; index++) {
			tAdhocPeerData* peer = &m_pMatchingInfoEntry->m_nPeersConnInfo[index];
			tMacAddr& ownMac = GetPlayerMacAddress();
			if (peer->macAddr == ownMac) {
				peer->nTeamID = static_cast<int16>(Game.s_nSelectedTeam);
				peer->nSelectedPeerModelID = Game.s_nPlayerModelIndex;
			}
		}
		m_pMatchingInfoEntry->m_GameLocation = static_cast<int32>(Game.GetGameLocation());
		m_pMatchingInfoEntry->m_GameType = static_cast<int32>(Game.GetGameType());
		m_pMatchingInfoEntry->m_nScoreLimit = Game.GetScoreLimit();
		m_pMatchingInfoEntry->m_nScoreCTFLimit = Game.GetCTFScoreLimit();
		m_pMatchingInfoEntry->m_nTimeLimit = Game.GetTimeLimit();
		m_pMatchingInfoEntry->m_nFlags = Game.m_nFlags;
		m_pMatchingInfoEntry->m_nScenarioOrRaceTrackID = Game.m_nScenarioOrRaceTrackID;
		m_pMatchingInfoEntry->m_nRaceCarID = Game.m_nRaceCarID;
#ifdef GTA_LIBERTY
		m_pMatchingInfoEntry->m_nCutsceneFlags = TheMPGame.m_nCutsceneFlags;
#else
		m_pMatchingInfoEntry->ePlayIntroCutscene = static_cast<uint8>(eGameCutscenePlayback::DONT_PLAY);
		m_pMatchingInfoEntry->bCutscenePlayed = false;
#endif
		m_pMatchingInfoEntry->m_nAmbientCarBank = Game.m_nAmbientCarBank;
		m_pMatchingInfoEntry->m_nAmbientPedBank = Game.m_nAmbientPedBank;
		m_pMatchingInfoEntry->m_nCashTarget = Game.GetCashTarget();
		m_pMatchingInfoEntry->m_Team1GangID = Game.GetTeam1GangID();
		m_pMatchingInfoEntry->m_Team2GangID = Game.GetTeam2GangID();
		for (int32 index = 0; index < MP_NUM_PEERS; index++)
			m_pMatchingInfoEntry->m_aPlayerID[index] = Game.GetSpawnPointFromPlayer(index);

		static_assert(sizeof(tLobbyRemoteInfo) == 136, "tLobbyRemoteInfo");
		if (m_nAdhocMatchingID >= 0)
			sceNetAdhocMatchingSetHelloOpt(m_nAdhocMatchingID, sizeof(tLobbyRemoteInfo), m_pMatchingInfoEntry);
	}
	SignalSemaphore();
}

// on when we discovered the hoster, and add hoster lobby data for self
// Call: when i am in client mode (recv here)
void cAdhoc::OnMatchClient(int matchingid, int event, tMacAddr& macAddr, int optlen, void* optdata)
{
#if !defined(FINAL) && !defined(MASTER)
	debug("cAdhoc::OnMatchClient(matchingid: %d, event: %d, macAddr: %s, optlen: %d, optdata: %p)\n", matchingid, event, macAddr.ToString().c_str(), optlen, optdata);
#endif

	if (IsNextStateNow(&cAdhoc::StateCreateGameGroup) ||
		IsNextStateNow(&cAdhoc::StateHandleError) ||
		IsNextStateNow(&cAdhoc::StateAttemptGameScan) ||
		IsNextStateNow(&cAdhoc::StateIdle2))
		return;

	int32 slot = FindMacAddr(macAddr);
	if (slot >= 0)
	{
		tAdhocMatchingData* pMatchingData = &m_aMatchingInfoRecv[slot];
		switch (event)
		{
			case PSP_ADHOC_MATCHING_EVENT_HELLO:
			{
				if (pMatchingData->nState != eAdhocPeerState::ADHOC_PEER_JOINED)
				{
					pMatchingData->nState = eAdhocPeerState::ADHOC_PEER_HELLOED;
					pMatchingData->addr = macAddr;
					pMatchingData->nWaitTime = 0;
					static_assert(sizeof(tLobbyRemoteInfo) == 136, "tLobbyRemoteInfo");
					if (optlen == sizeof(tLobbyRemoteInfo))
					{
						memcpy(&pMatchingData->entry, optdata, sizeof(tLobbyRemoteInfo));
						pMatchingData->entry.m_HostPeerData.peerAddr.mac = macAddr;
					}
					FindAndClearMacAddr(macAddr);
				}
				break;
			}
			case PSP_ADHOC_MATCHING_EVENT_REJECT:
			{
				if (slot == m_nServerPeerID)
					pMatchingData->nState = eAdhocPeerState::ADHOC_PEER_DISCONNECTED;
				break;
			}
			case PSP_ADHOC_MATCHING_EVENT_COMPLETE:
			{
				m_LobbyHosterMacAddr = macAddr;
				m_pMatchingInfoEntry = &pMatchingData->entry;
				pMatchingData->nState = eAdhocPeerState::ADHOC_PEER_JOINED;
				break;
			}
			case PSP_ADHOC_MATCHING_EVENT_DISCONNECT:
			{
				if (slot == m_nServerPeerID)
					pMatchingData->nState = eAdhocPeerState::ADHOC_PEER_DISCONNECTED;
				else if (pMatchingData->nState != eAdhocPeerState::ADHOC_PEER_JOINED)
					pMatchingData->nState = eAdhocPeerState::ADHOC_PEER_DISCONNECTED;
				break;
			}
			case PSP_ADHOC_MATCHING_EVENT_JOIN:
			case PSP_ADHOC_MATCHING_EVENT_LEFT:
			case PSP_ADHOC_MATCHING_EVENT_CANCEL:
			case PSP_ADHOC_MATCHING_EVENT_ACCEPT:
			case PSP_ADHOC_MATCHING_EVENT_TIMEOUT:
			case PSP_ADHOC_MATCHING_EVENT_ERROR:
			{
				break;
			}
		}
		SignalSemaphore();
	}
}

// Call: when i am in client mode (recv here)
void cAdhoc::OnMatchHost(int matchingid, int event, tMacAddr& macAddr, int optlen, void* optdata)
{
#if !defined(FINAL) && !defined(MASTER)
	debug("cAdhoc::OnMatchHost(matchingid: %d, event: %d, macAddr: %s, optlen: %d, optdata: %p)\n", matchingid, event, macAddr.ToString().c_str(), optlen, optdata);
#endif

	if (IsNextStateNow(&cAdhoc::StateCreateGameGroup) ||
		IsNextStateNow(&cAdhoc::StateHandleError) ||
		IsNextStateNow(&cAdhoc::StateIdle2))
		return;

	int32 slot = FindMacAddr(macAddr);
	if (slot >= 0)
	{
		tAdhocMatchingData* pMatchingData = &m_aMatchingInfoRecv[slot];
		switch (event)
		{
			case PSP_ADHOC_MATCHING_EVENT_JOIN:
			{
				if (IsNextStateNow(&cAdhoc::StateIdle) && GetNumberOfConnectedPlayers() < MP_NUM_PEERS)
				{
					pMatchingData->addr = macAddr;
					if (sceNetAdhocMatchingSelectTarget(m_nAdhocMatchingID, macAddr.GetBytesSCE(), 0, 0) >= 0)
						pMatchingData->nState = eAdhocPeerState::ADHOC_PEER_ACCEPTED;
				}
				else
					sceNetAdhocMatchingCancelTarget(m_nAdhocMatchingID, pMatchingData->addr.GetBytesSCE());
				break;
			}
			case PSP_ADHOC_MATCHING_EVENT_LEFT:
			case PSP_ADHOC_MATCHING_EVENT_CANCEL:
			{
				if (pMatchingData->nState == eAdhocPeerState::ADHOC_PEER_ACCEPTED)
					pMatchingData->nState = eAdhocPeerState::ADHOC_PEER_DISCONNECTED;
				break;
			}
			case PSP_ADHOC_MATCHING_EVENT_COMPLETE:
			{
				pMatchingData->nState = eAdhocPeerState::ADHOC_PEER_JOINED;
				m_pMatchingInfoEntry->m_nPeersConnInfo[slot].macAddr = macAddr;
				FindAndClearMacAddr(macAddr);
				static_assert(sizeof(tLobbyRemoteInfo) == 136, "tLobbyRemoteInfo");
				sceNetAdhocMatchingSetHelloOpt(m_nAdhocMatchingID, sizeof(tLobbyRemoteInfo), m_pMatchingInfoEntry);
				break;
			}
			case PSP_ADHOC_MATCHING_EVENT_TIMEOUT:
			case PSP_ADHOC_MATCHING_EVENT_DISCONNECT:
			{
				if (pMatchingData->nState != eAdhocPeerState::ADHOC_PEER_LEAVING)
				{
					if (pMatchingData->nState == eAdhocPeerState::ADHOC_PEER_JOINED) {
						m_pMatchingInfoEntry->m_nPeersConnInfo[slot].macAddr.InitMacAddr();
						static_assert(sizeof(tLobbyRemoteInfo) == 136, "tLobbyRemoteInfo");
						sceNetAdhocMatchingSetHelloOpt(m_nAdhocMatchingID, sizeof(tLobbyRemoteInfo), m_pMatchingInfoEntry);
					}
					pMatchingData->nState = eAdhocPeerState::ADHOC_PEER_DISCONNECTED;
				}
				break;
			}
			case PSP_ADHOC_MATCHING_EVENT_HELLO:
			case PSP_ADHOC_MATCHING_EVENT_REJECT:
			case PSP_ADHOC_MATCHING_EVENT_ACCEPT:
			case PSP_ADHOC_MATCHING_EVENT_ERROR:
			{
				break;
			}
		}
		SignalSemaphore();
	}
}

cAdhoc* cAdhoc::ms_pInstance = nil;
bool cAdhoc::ms_bInitNetworkModules = false;

#ifndef MASTER
void AdhocPrintDebugStuff()
{
	CFont::SetFontStyle(FONT_STANDARD);
	CFont::SetBackgroundOff();
	CFont::SetBackGroundOnlyTextOff();
	CFont::SetWrapx(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH));
	CFont::SetScale(0.85f, 1.0f);
	CFont::SetCentreOff();
	CFont::SetCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH));
	CFont::SetJustifyOff();
	//CFont::SetColor(CRGBA(240, 20, 20, 255));
	CFont::SetColor(CRGBA(39, 152, 7, 255)); // green
	CFont::SetDropShadowPosition(1);
	CFont::SetPropOn();
	CFont::SetDropColor(CRGBA(0, 0, 0, 255));


	const CRGBA colGreen(39, 152, 7, 255);
	const CRGBA colRed(240, 20, 20, 255);

	float x = 16.0f;
	float y = 180.0f;

	char line[512];
	wchar wline[512];

	sprintf(line,
		"~r~[ADHOC] %sERROR %sCONNECT %sDISCONNECT %sSCAN %sCANCEL %sHOSTDISC %sWOL %sWOL_ABORT~s~",
		TheAdhoc.bHasError ? "~b~" : "~g~",
		TheAdhoc.bConnEvent ? "~b~" : "~g~",
		TheAdhoc.bDisconnEvent ? "~b~" : "~g~",
		TheAdhoc.bScanEvent ? "~b~" : "~g~",
		TheAdhoc.bCancelEvent ? "~b~" : "~g~",
		TheAdhoc.bHostDiscoverEvent ? "~b~" : "~g~",
		TheAdhoc.bWolEvent ? "~b~" : "~g~",
		TheAdhoc.bWolAbortEvent ? "~b~" : "~g~"
	);

	AsciiToUnicode(line, wline);
	CFont::SetColor(CRGBA(240, 240, 240, 255));
	CFont::PrintString(x, y, wline);
}

#if !defined(GTA_PSP)
void AdhocEmuPrintDebugStuff()
{
	CFont::SetFontStyle(FONT_STANDARD);
	CFont::SetBackgroundOff();
	CFont::SetBackGroundOnlyTextOff();
	CFont::SetWrapx(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH));
	CFont::SetScale(0.85f, 1.0f);
	CFont::SetCentreOff();
	CFont::SetCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH));
	CFont::SetJustifyOff();
	//CFont::SetColor(CRGBA(240, 20, 20, 255));
	CFont::SetColor(CRGBA(39, 152, 7, 255)); // green
	CFont::SetDropShadowPosition(1);
	CFont::SetPropOn();
	CFont::SetDropColor(CRGBA(0, 0, 0, 255));


	const CRGBA colGreen(39, 152, 7, 255);
	const CRGBA colRed(240, 20, 20, 255);

	float x = 16.0f;
	float y = 190.0f;

	char line[512];
	wchar wline[512];

	sprintf(line, "[AEMU] matchingEvents: %d, adhocctlEvents: %d", matchingEvents.size(), adhocctlEvents.size());

	AsciiToUnicode(line, wline);
	CFont::SetColor(CRGBA(240, 240, 240, 255));
	CFont::PrintString(x, y, wline);
}
#endif
#endif
