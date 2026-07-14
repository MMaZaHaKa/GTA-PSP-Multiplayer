/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "GameChat.h"
#include "main.h"
#include "Pad.h"
#include "DMAudio.h"
#include "PlayerPed.h"
#include "PlayerInfo.h"
#include "World.h"
#include "Font.h"
#include "Timer.h"
#include "Sprite2d.h"
#include "ModelIndices.h"
#include "User.h"
#include "Text.h"
#include "Hud.h"
#include "Frontend.h"
#include "Clock.h"
#include "Script.h" // debug
#if /*defined(GTA_NETWORK) &&*/ defined(GTA_PC)
#include "MultiGame.h"
#include "multiplayer/LScript.h"
#include "multiplayer/Logger.h"
#include "multiplayer/events/public.h"
#ifndef GTA_PSP
#include "multiplayer/net/emu/proAdhoc.h"
#include "multiplayer/net/emu/Resolve.h"
#include "multiplayer/net/emu/sceNetAdhocMatching.h"
#include "multiplayer/net/emu/Utils.h"
#endif

#include <algorithm>
#include <sstream>

cGameChat gChat;

#define CHAT_MESSAGE_VIEW_TIME         (5) // 5sec
#define CHAT_MESSAGE_TOGGLE_KEY        ('Y')
#define CHAT_MESSAGE_DELIM_KEY_1       ('!')
#define CHAT_MESSAGE_DELIM_KEY_2       ('/')
#define CHAT_MESSAGE_MAX_MESSAGE_SIZE  ADHOCCTL_MESSAGE_LEN
#define CHAT_MESSAGE_MAX_MESSAGES      (10)

#define CHAT_SCALE 0.8f
#define CHAT_TEXT_COLOR CRGBA(255, 255, 255, 255) // White text
#define CHAT_BG_COLOR CRGBA(0, 0, 0, 128) // Semi-transparent black background
#define CHAT_POS_X (RsGlobal.maximumWidth * 0.01f) // Left margin, 1% of screen width
#define CHAT_BOTTOM_MARGIN (RsGlobal.maximumHeight * 0.05f) // Bottom margin, 5% of screen height
#define CHAT_WIDTH (RsGlobal.maximumWidth * 0.45f) // Width of chat box, 35% of screen
#define CHAT_LINE_HEIGHT (CFont::GetCharacterSizeY() * CHAT_SCALE + 2.0f) // Line height with padding
#define CHAT_MAX_VISIBLE_LINES CHAT_MESSAGE_MAX_MESSAGES // Max lines to show
#define CHAT_FADE_TIME 1.0f // Time in seconds for fade-out (last second of VIEW_TIME)
#define CHAT_INPUT_PREFIX "Say: "
#define CHAT_CURSOR "_" // Blinking cursor
#define CHAT_CURSOR_BLINK_RATE 0.5f // Blink every 0.5 seconds

const char* GetBufferString() {
	if (!OpenClipboard(NULL)) return NULL;
	HANDLE hData = GetClipboardData(CF_TEXT);
	if (hData == NULL) {
		CloseClipboard();
		return NULL;
	}
	char* pszText = static_cast<char*>(GlobalLock(hData));
	if (pszText == NULL) {
		CloseClipboard();
		return NULL;
	}
	static std::string clipboardText = pszText;
	GlobalUnlock(hData);
	CloseClipboard();
	return clipboardText.c_str();
}

void DebugHudMsg(const char* msg)
{
	if (!msg) { return; }
	wchar strW[200];
	AsciiToUnicode(msg, strW);
	TheHud->SetHelpMessage(strW, true);
}

void OnChangeNickname(std::string arg) {
	if (arg.size() == 0)
		return;

	gChat.OnMessageRecv(g_Config.sNickName, "Nickname was changed to " + arg);
	g_Config.sNickName = arg;
	debug("[Chat]: %s\n", ("Change nickname to " + arg).c_str());
}

void OnLua(std::string arg) {
	if (arg.size() == 0)
		return;

	if (HasLuaInitialized())
		cLWrapper::Instance().ExecString(arg.c_str());
}

void OnMac(std::string arg) {
	g_Config.sMACAddress = CreateRandMAC();
	char buff[64];
	sprintf(buff, "Generated mac: %s", g_Config.sMACAddress.c_str());
	DebugHudMsg(buff);
}

std::vector<std::string> g_LastScanResults; // cache
std::string selfAddr = "";
void OnScan(std::string arg) {
	net::Init();
	selfAddr = net::GetOutboundIP();
	gChat.OnMessageRecv(g_Config.sNickName, "Scanning AEmu hosts...");
	//net::ScanOutboundSubnetWithCallback(ADHOC_SERVER_PORT, SOCK_STREAM,
	//	[](std::vector<std::string> hosts) {
	//		g_LastScanResults = hosts;

	//		std::string self = net::GetOutboundIP();
	//		std::string result = "Found hosts (" + std::to_string(hosts.size()) + "): ";

	//		for (int32 i = 0; i < hosts.size(); i++) {
	//			if (i > 0)
	//				result += ", ";

	//			result += std::to_string(i + 1) + ": ";
	//			if (hosts[i] == self)
	//				result += hosts[i] + " (self)";
	//			else
	//				result += hosts[i];
	//		}

	//		if (hosts.empty())
	//			result += "none";

	//		gChat.OnMessageRecv(g_Config.sNickName, result);
	//	}, 100); // timeout

	net::ScanOutboundSubnetWithCallback(ADHOC_SERVER_PORT, SOCK_STREAM,
		[](std::vector<std::string> hosts) {
			g_LastScanResults = hosts;
			if (hosts.empty()) {
				gChat.OnMessageRecv(g_Config.sNickName, "No hosts found");
				return;
			}

			gChat.OnMessageRecv(g_Config.sNickName, "Found " + std::to_string(hosts.size()) + " host(s):");

			for (int32 i = 0; i < hosts.size(); i++) {
				std::string hostInfo = "  " + std::to_string(i + 1) + ". " + hosts[i];
				if (hosts[i] == selfAddr)
					hostInfo += " (self)";
				gChat.OnMessageRecv(g_Config.sNickName, hostInfo);
			}

		}, 100); // timeout
	gChat.Close();
}

void OnHost(std::string arg) {
	if (arg.size() == 0)
		return;

	bool isNumber = true;
	for (char c : arg) {
		if (!std::isdigit(c)) {
			isNumber = false;
			break;
		}
	}

	if (isNumber) {
		int index = std::stoi(arg) - 1;
		if (!g_LastScanResults.empty() && index >= 0 && index < g_LastScanResults.size()) {
			std::string selectedHost = g_LastScanResults[index];
			gChat.OnMessageRecv(g_Config.sNickName, "AEmu Host Address was changed to " + selectedHost + " (host #" + arg + ")");
			g_Config.sProAdhocServer = selectedHost;
			SaveINISettings();
			if (friendFinderRunning)
				gChat.OnMessageRecv(g_Config.sNickName, "Warning: friendFinder is already connected");
			return;
		}
		else {
			gChat.OnMessageRecv(g_Config.sNickName, "Invalid host number or no scan results available");
			return;
		}
	}

	gChat.OnMessageRecv(g_Config.sNickName, "AEmu Host Address was changed to " + arg);
	SaveINISettings();
	if(friendFinderRunning)
		gChat.OnMessageRecv(g_Config.sNickName, "Warning: friendFinder is already connected");
	g_Config.sProAdhocServer = arg;
}

void OnAddr(std::string arg) {
	//std::vector<std::string> addr = net::GetLocalAddresses();
	//char buff[256];
	//std::string allAddresses;
	//for (int32 i = 0; i < addr.size(); i++) {
	//	allAddresses += addr[i];
	//	if (i != addr.size() - 1) {
	//		allAddresses += ", ";
	//	}
	//}
	//sprintf(buff, "Local addresses: %s", allAddresses.c_str());
	//debug("%s\n", buff);
	////DebugHudMsg(buff);

	net::Init(); // for use in sp mode
	std::string localInterfaceIP = net::GetOutboundIP();
	std::string msg = "Your LAN IP " + (localInterfaceIP.empty() ? "[empty]" : localInterfaceIP);
	gChat.OnMessageRecv(g_Config.sNickName, msg);
	debug("%s\n", msg.c_str());
}

void OnSpawn(std::string arg) {
	if (arg.size() == 0)
		return;

	int32 modelIndex = -1;
	const char* name = arg.c_str();
	CBaseModelInfo* mi = CModelInfo::GetModelInfo(name, &modelIndex);
	if (!mi) { // name is index
		modelIndex = stoi(arg);
		mi = CModelInfo::GetModelInfo(modelIndex);
		if (mi)
			name = mi->GetModelName();
		else
			name = "Not Found";
	}
	if (modelIndex >= MI_FIRST_VEHICLE && modelIndex <= MI_LAST_VEHICLE)
		SpawnCar(modelIndex);
	else if (modelIndex >= MI_PLAYER && modelIndex <= MI_LAST_PED)
		SpawnPed(modelIndex, ePedType::PEDTYPE_CIVMALE);
	else
		SpawnObject(modelIndex);
	char buff[64];
	sprintf(buff, "Spawned: %s (%d)", name, modelIndex);
	DebugHudMsg(buff);
	gChat.Close();
}

void OnModel(std::string arg) {
	if (arg.size() == 0)
		return;

	int32 modelIndex = -1;
	const char* name = arg.c_str();
	CBaseModelInfo* mi = CModelInfo::GetModelInfo(name, &modelIndex);
	if (!mi) { // name is index
		modelIndex = stoi(arg);
		mi = CModelInfo::GetModelInfo(modelIndex);
		if (mi)
			name = mi->GetModelName();
		else
			name = "Not Found";
	}
	char buff[64];
	sprintf(buff, "ModelInfo: %s (%d)", name, modelIndex);
	DebugHudMsg(buff);
}

void OnMission(std::string arg) {
	if (arg.size() == 0 || FrontEndMenuManager->m_bMenuActive)
		return;

	int32 index = 0;

	bool isNumber = true;
	for (char c : arg) {
		if (!std::isdigit(static_cast<unsigned char>(c))) {
			isNumber = false;
			break;
		}
	}

	if (isNumber) {
		int32 missionNum = stoi(arg);
		if (missionNum >= 0 && missionNum < NUM_MISSIONS) {
			CTheScripts::SwitchToMission(sortedMissionIdx[missionNum]);
			char buff[64];
			sprintf(buff, "Launch: %s", sortedMissionNames[missionNum]);
			DebugHudMsg(buff);
			gChat.Close();
			return;
		}
	}

	std::string argLower = arg;
	std::transform(argLower.begin(), argLower.end(), argLower.begin(),
		[](unsigned char c) { return tolower(c); });

	bool found = false;
	for (; index < NUM_MISSIONS; ++index) {
		std::string missionNameLower = sortedMissionNames[index];
		std::transform(missionNameLower.begin(), missionNameLower.end(),
			missionNameLower.begin(),
			[](unsigned char c) { return tolower(c); });

		if (missionNameLower.find(argLower) != std::string::npos) {
			found = true;
			break;
		}
	}

	if (found && index < NUM_MISSIONS) {
		CTheScripts::SwitchToMission(sortedMissionIdx[index]);
		char buff[64];
		sprintf(buff, "Launch: %s", sortedMissionNames[index]);
		DebugHudMsg(buff);
		gChat.Close();
	}
	else {
		DebugHudMsg("Mission not found");
	}
}

void OnKick(std::string arg) {
	if (arg.size() == 0)
		return;

	if(TheAdhoc.IsHost()) // pls
		MultigameKickPlayer(stoi(arg));
}

void OnQuit(std::string arg) {
	ExitProcess(0);
}

void OnTeleport(std::string arg) {
	CVector pos(0.0f, 0.0f, 0.0f);
	std::replace(arg.begin(), arg.end(), ',', '.');
	for (char& c : arg) {
		if (c == ';' || c == '\t') {
			c = ' ';
		}
	}

	std::istringstream iss(arg);
	float x, y, z;

	if (iss >> x >> y >> z) {
		char remaining;
		if (!(iss >> remaining)) {
			pos = CVector(x, y, z);
		}
	}

	if (FindPlayerPed())
		FindPlayerPed()->Teleport(pos);
}

void OnMenu(std::string arg) {
	if (FrontEndMenuManager->m_bMenuActive)
		FrontEndMenuManager->RequestFrontEndShutDown();
	else
		FrontEndMenuManager->RequestFrontEndStartUp();
}

void OnNight(std::string arg) {
	CClock::SetGameClock(3, 0);
}

void OnJournal(std::string arg) {
	if (arg.size() == 0)
		return;

	char buf[128];
	std::size_t copy_len = arg.copy(buf, sizeof(buf) - 1);
	buf[copy_len] = '\0';

	char* p = buf;
	while (*p && std::isspace(static_cast<unsigned char>(*p))) ++p;
	if (!*p) return;

	char* token = std::strtok(p, " \t");
	if (!token) return;

	char* endptr = nil;
	long slotL = std::strtol(token, &endptr, 10);
	if (endptr == token) {
		printf("OnJournal: invalid slot: '%s'\n", token);
		return;
	}
	int slot = static_cast<int>(slotL);

	char* token2 = std::strtok(nil, " \t");
	if (token2) {
		endptr = nil;
		long paramL = std::strtol(token2, &endptr, 10);
		if (endptr == token2) {
			std::string out = Journal::Instance().dumpSlot(slot);
			printf("%s", out.c_str());
			return;
		}
		int32_t param = static_cast<int32>(paramL);

		std::string out = Journal::Instance().dumpSlotParam(slot, param);
		printf("%s", out.c_str());
	}
	else {
		std::string out = Journal::Instance().dumpSlot(slot);
		printf("%s", out.c_str());
	}

	gChat.Close();
}

void OnTest(std::string arg) {
	SceNetEtherAddr target;

	SceNetAdhocctlPeerInfo* peer = friends;
	// Iterate Peers
	for (; peer != NULL; peer = peer->next) {
		if (!isLocalMAC(&peer->mac_addr)) {
			target = peer->mac_addr;
			break;
		}
	}

	auto fut = net::ScanOutboundSubnetAsync(ADHOC_SERVER_PORT, SOCK_STREAM, 100);
	std::vector<std::string> open_hosts = fut.get();


	printf("Open hosts:\n");
	for (auto& h : open_hosts)
		printf("  %s\n", h.c_str());
}

bool CanUseChatByGameState() { // no sp, probably fe + mp
	return true; // 4 sp commands
	return netAdhocMatchingInited;
}

void cGameChat::Initialise() {
	if (m_bIsInitialised)
		return;

	AddChatMessageCallback(&cGameChat::OnMessageRecv);
	ConCommandBase nickname("nickname", "For change Nickname", &OnChangeNickname, 0);
	RegisterConCommandBase(nickname);
	ConCommandBase lua("lua", "lua", &OnLua, 0);
	RegisterConCommandBase(lua);
	ConCommandBase mac("mac", "mac", &OnMac, 0);
	RegisterConCommandBase(mac);
	ConCommandBase scan("scan", "scan", &OnScan, 0);
	RegisterConCommandBase(scan);
	ConCommandBase host("host", "host", &OnHost, 0); // set aemu host addr
	RegisterConCommandBase(host);
	ConCommandBase ip("ip", "ip", &OnAddr, 0); // get outbound ip
	RegisterConCommandBase(ip);
	ConCommandBase lan("lan", "lan", &OnAddr, 0); // same
	RegisterConCommandBase(lan);
	ConCommandBase night("night", "night", &OnNight, 0);
	RegisterConCommandBase(night);
	ConCommandBase kick("kick", "kick", &OnKick, 0);
	RegisterConCommandBase(kick);
	ConCommandBase journal("journal", "journal", &OnJournal, 0);
	RegisterConCommandBase(journal);
	m_bIsOpen = false;
	m_bIsInitialised = true;
	m_sMyTextBuff.clear();
	m_vChatHistory.clear();
	// SP
	ConCommandBase spawn("spawn", "spawner", &OnSpawn, 0);
	RegisterConCommandBase(spawn);
	ConCommandBase mi("mi", "minfo", &OnModel, 0);
	RegisterConCommandBase(mi);
	ConCommandBase mission("mission", "mission", &OnMission, 0);
	RegisterConCommandBase(mission);
	ConCommandBase quit("q", "quit", &OnQuit, 0);
	RegisterConCommandBase(quit);
	ConCommandBase teleport("tp", "teleport", &OnTeleport, 0);
	RegisterConCommandBase(teleport);
	ConCommandBase menu("menu", "menu", &OnMenu, 0);
	RegisterConCommandBase(menu);
	ConCommandBase test("test", "test", &OnTest, 0);
	RegisterConCommandBase(test);
}

void cGameChat::Open() {
	if (!CanUse()) return;

	if (!CanUseChatByGameState())
		return;

	DMAudio.PlayFrontEndSound(SOUND_GARAGE_OPENING, 0);

	m_bIsOpen = true;
	m_sMyTextBuff.clear();
	//CPad::GetPad(0)->SetDisablePlayerControls(PLAYERCONTROL_PLAYERINFO);
	CWorld::Players[CWorld::PlayerInFocus].MakePlayerSafe(true);
}

void cGameChat::Close() {
	DMAudio.PlayFrontEndSound(SOUND_GARAGE_NO_MONEY, 0);
	m_bIsOpen = false;
	//CPad::GetPad(0)->SetEnablePlayerControls(PLAYERCONTROL_PLAYERINFO);
	CWorld::Players[CWorld::PlayerInFocus].MakePlayerSafe(false);
}

void cGameChat::OnRender() {
	if (!CanUse()) return;

	if (!CanUseChatByGameState())
		return;

	if (ChatMode() == 2 && !IsOpen()) // pager redirect
		return;

	wchar buffW[260];
	float currentTime = (CTimer::GetIsPaused() ? CTimer::GetTimeInMillisecondsPauseMode() : CTimer::GetTimeInMilliseconds()) / 1000.0f;

	// Calculate number of visible messages
	int visibleCount = 0;
	for (auto& msg : m_vChatHistory) {
		float age = currentTime - msg.m_fTimestamp;
		if (age < CHAT_MESSAGE_VIEW_TIME || m_bIsOpen) {
			visibleCount++;
		}
	}

	visibleCount = Min(visibleCount, CHAT_MAX_VISIBLE_LINES);
	if (m_bIsOpen) {
		visibleCount++; // Extra line for input
	}
	if (visibleCount == 0) return;

	float chatHeight = visibleCount * CHAT_LINE_HEIGHT + 10.0f; // Padding
	float bottom_y = RsGlobal.maximumHeight - CHAT_BOTTOM_MARGIN;
	float top_y = bottom_y - chatHeight;
	CSprite2d::DrawRect(CRect(CHAT_POS_X, top_y, CHAT_POS_X + CHAT_WIDTH, bottom_y), CHAT_BG_COLOR);
	CFont::SetScale(CHAT_SCALE, CHAT_SCALE);
	CFont::SetColor(CHAT_TEXT_COLOR);
	CFont::SetJustifyOff();
	CFont::SetCentreOff();
	CFont::SetFontStyle(FONT_STANDARD);
	CFont::SetWrapx(CHAT_POS_X + CHAT_WIDTH - 10.0f); // Wrap within width

	// Draw from bottom upwards (input at bottom, newest messages above)
	float y = bottom_y - CHAT_LINE_HEIGHT; // Start from bottom for input or newest
	if (m_bIsOpen) {
		// Draw input line at bottom
		std::string inputText = CHAT_INPUT_PREFIX + m_sMyTextBuff;
		// Blinking cursor
		if (fmod(currentTime, CHAT_CURSOR_BLINK_RATE * 2.0f) < CHAT_CURSOR_BLINK_RATE) {
			inputText += CHAT_CURSOR;
		}
		AsciiToUnicode(inputText.c_str(), buffW);
		CFont::ClearTokensFromString(buffW); // crash on unprintable chars
		CFont::PrintString(buffW, 0, CHAT_POS_X + 5.0f, y);
		y -= CHAT_LINE_HEIGHT;
	}

	// Draw messages from bottom to top (newest first, just above input)
	for (auto it = m_vChatHistory.rbegin(); it != m_vChatHistory.rend(); ++it) {
		tChatMessage& msg = *it;
		float age = currentTime - msg.m_fTimestamp;
		if (age >= CHAT_MESSAGE_VIEW_TIME && !m_bIsOpen) continue;
		// Calculate alpha
		int alpha = 255;
		if (!m_bIsOpen && age > CHAT_MESSAGE_VIEW_TIME - CHAT_FADE_TIME) {
			float fadeProgress = (age - (CHAT_MESSAGE_VIEW_TIME - CHAT_FADE_TIME)) / CHAT_FADE_TIME;
			alpha = (int)(255 * (1.0f - fadeProgress));
		}
		msg.m_nAlpha = alpha;
		CRGBA color = CHAT_TEXT_COLOR;
		color.alpha = alpha;
		CFont::SetColor(color);
		// TODO: fine if have mac and get player name from cAdhoc::FormatPlayerName
		std::string fullMsg = msg.m_sNickName != "" ? (msg.m_sNickName + ": " + msg.m_sChatMessage) : msg.m_sChatMessage;
		AsciiToUnicode(fullMsg.c_str(), buffW);
		CFont::ClearTokensFromString(buffW); // crash on unprintable chars
		CFont::PrintString(buffW, 0, CHAT_POS_X + 5.0f, y);
		y -= CHAT_LINE_HEIGHT;
		if (--visibleCount <= 0) break;
	}
}

void cGameChat::OnUpdate() {
	if (!CanUse()) return;

	if (!CanUseChatByGameState()) {
		mutex.lock();
		m_vChatHistory.clear();
		mutex.unlock();
		m_sMyTextBuff.clear();
		return;
	}

	float currentTime = (CTimer::GetIsPaused() ? CTimer::GetTimeInMillisecondsPauseMode() : CTimer::GetTimeInMilliseconds()) / 1000.0f;

	// Clean up old messages
	mutex.lock();
	auto it = m_vChatHistory.begin();
	while (it != m_vChatHistory.end()) {
		if (currentTime - it->m_fTimestamp > CHAT_MESSAGE_VIEW_TIME + CHAT_FADE_TIME) {
			it = m_vChatHistory.erase(it);
		}
		else {
			++it;
		}
	}
	mutex.unlock();

	CPad* pad = CPad::GetPad(0);
	if (pad->GetLeftCtrl() && pad->GetCharJustDown(CHAT_MESSAGE_TOGGLE_KEY))
		IsOpen() ? Close() : Open();

	if (!IsOpen())
		return;

	if (pad->GetEnterJustDown()) {
		if (!m_sMyTextBuff.empty()) {
			gChat.MessageSend(m_sMyTextBuff);
		}
		m_sMyTextBuff.clear();
	}

	if (pad->GetEscapeJustDown())
		gChat.Close();

	if (pad->GetBackspaceJustDown() && m_sMyTextBuff.size() > 0) {
		DMAudio.PlayFrontEndSound(SOUND_WEAPON_ROCKET_SHOT_NO_ZOOM, 0);
		m_sMyTextBuff.pop_back();
	}

	// scroll opened
	if (pad->GetMouseWheelUpJustDown()) {

	}
	else if (pad->GetMouseWheelDownJustDown()) {

	}
}

void cGameChat::OnPressedChar(char ch) {
	if (!CanUse()) return;

	if (!CanUseChatByGameState())
		return;

	CPad* pad = CPad::GetPad(0); // close key before here hotfix
	if (!IsOpen() || (pad->GetLeftCtrl() && ch == CHAT_MESSAGE_TOGGLE_KEY) || m_sMyTextBuff.size() >= CHAT_MESSAGE_MAX_MESSAGE_SIZE)
		return;

	if (pad->GetLeftCtrl() && ch == 'V') {
		const char* clipText = GetBufferString();
		if (clipText && strlen(clipText) > 0 && m_sMyTextBuff.size() + strlen(clipText) <= CHAT_MESSAGE_MAX_MESSAGE_SIZE) {
			m_sMyTextBuff += clipText;
			DMAudio.PlayFrontEndSound(SOUND_WEAPON_SNIPER_SHOT_NO_ZOOM, 0);
			return;
		}
	}

	// hotfix for shifted keys
	bool isShift = pad->GetLeftShift() || pad->GetRightShift();
	if (isShift) {
		switch (ch) {
		case '1': ch = '!'; break;
		case '2': ch = '@'; break;
		case '3': ch = '#'; break;
		case '4': ch = '$'; break;
		case '5': ch = '%'; break;
		case '6': ch = '^'; break;
		case '7': ch = '&'; break;
		case '8': ch = '*'; break;
		case '9': ch = '('; break;
		case '0': ch = ')'; break;
		case '-': ch = '_'; break;
		}
	}

	m_sMyTextBuff += (isShift ? ch : tolower(ch));
	DMAudio.PlayFrontEndSound(SOUND_WEAPON_SNIPER_SHOT_NO_ZOOM, 0);
	//DMAudio.PlayFrontEndSound(SOUND_WEAPON_ROCKET_SHOT_NO_ZOOM, 0);
}

// TODO proAdhoc: mac sender? // for now chat api aemu anonymous
wchar gBuffW[260];
void cGameChat::OnMessageRecv(std::string nickname, std::string message) {
	if (!gChat.CanUse()) return;

	if (!CanUseChatByGameState())
		return;

	if (gChat.ChatMode() == 2 && !gChat.IsOpen()) { // pager redirect
		bool bUnusedBuff = true; // todo tmp code
		PagerMessage* messages = CUserDisplay::Pager.GetMessages();
		for (int32 i = 0; i < NUMPAGERMESSAGES; i++) {
			if (messages[i].m_pText == gBuffW)
				bUnusedBuff = false;
		}
		if (bUnusedBuff) {
			AsciiToUnicode(nickname != "" ? (message + " - " + nickname).c_str() : message.c_str(), gBuffW);
			CFont::ClearTokensFromString(gBuffW); // crash on unprintable chars
			CUserDisplay::Pager.AddMessage(gBuffW, 100, 1, 0);
		}
	}
	else {
		//DMAudio.PlayFrontEndSound(SOUND_HUD, 0);
		DMAudio.PlayFrontEndSound(SOUND_PICKUP_HIDDEN_PACKAGE, 0);
		//DMAudio.PlayFrontEndSound(SOUND_PED_DEATH, 0); // kek
	}

	gChat.mutex.lock();
	tChatMessage msg;
	msg.m_sNickName = nickname;
	msg.m_sChatMessage = message;
	float currentTime = (CTimer::GetIsPaused() ? CTimer::GetTimeInMillisecondsPauseMode() : CTimer::GetTimeInMilliseconds()) / 1000.0f;
	msg.m_fTimestamp = currentTime;
	msg.m_nAlpha = 255;
	gChat.m_vChatHistory.push_back(msg);
	if (gChat.m_vChatHistory.size() > CHAT_MESSAGE_MAX_MESSAGES) {
		gChat.m_vChatHistory.erase(gChat.m_vChatHistory.begin());
	}
	gChat.mutex.unlock();
}

void cGameChat::MessageSend(std::string message) {
	if (message.empty()) return;

	char delim = message[0];
	if (delim == CHAT_MESSAGE_DELIM_KEY_1 || delim == CHAT_MESSAGE_DELIM_KEY_2) {
		// Process as command
		size_t spacePos = message.find(' ', 1);
		std::string cmdName = (spacePos != std::string::npos) ? message.substr(1, spacePos - 1) : message.substr(1);
		std::string arg = (spacePos != std::string::npos) ? message.substr(spacePos + 1) : "";
		std::transform(cmdName.begin(), cmdName.end(), cmdName.begin(), ::tolower);

		for (auto& cmd : m_vCommands) {
			std::string cmdn = cmd.GetName();
			std::transform(cmdn.begin(), cmdn.end(), cmdn.begin(), ::tolower);
			if (cmdn == cmdName) {
				if (cmd.m_OnCall) {
					cmd.m_OnCall(arg);
				}
				return;
			}
		}

		OnMessageRecv(g_Config.sNickName, "Unknown command: " + cmdName);
		return;
	}

	bool bOk = sendChat(/*g_Config.sNickName + ": " + */message); // proAdhoc
}

// TODO: usage IsUnCaseContains
void cGameChat::RegisterConCommandBase(ConCommandBase& command) {
	std::string newCmdName = command.GetName();
	std::transform(newCmdName.begin(), newCmdName.end(), newCmdName.begin(), ::tolower);
	for (auto& cmd : m_vCommands) {
		std::string existingCmdName = cmd.GetName();
		std::transform(existingCmdName.begin(), existingCmdName.end(), existingCmdName.begin(), ::tolower);
		if (existingCmdName == newCmdName) {
			assert(false && "Already exists, do not register");
			return;
		}
	}
	command.m_bRegistered = true;
	m_vCommands.push_back(command);
}

#endif