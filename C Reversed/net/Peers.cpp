/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
//#include "multiplayer/net/Peers.h"
#include "multiplayer/MultiGame.h"
#include "multiplayer/net/emu/Utils.h"
#include "Font.h" // debug
#ifndef GTA_PSP
#include "multiplayer/net/emu/NetAdhocCommon.h"
#include "multiplayer/net/emu/proAdhoc.h"
#include "multiplayer/net/emu/sceNet.h"
#endif

#ifndef GTA_LIBERTY
sPeerState::sPeerState(int32 id, tListenAddr& addr) {
	m_nID = id;
	m_nCurTime = 0;
	m_Addr.mac = addr.mac;
	bCheckSender = false;
	m_Addr.port = addr.port;
	bSendOverflow = false;
	m_sName = "";
	m_bIsSpawned = false;
	m_latencySum = 0.0f;
	m_bufferCount = 0;
	m_bufferStart = 0;
	m_bufferIndex = 0;
	m_nTimeA = 0;
	m_nTimeB = 0;
	m_vElements = std::map<uint16, sElement*>();
#ifdef GTA_PSP
	bVanilaDevice = true;
#else
	bool bLocal = false;
	m_pProAdhocPeer = getPeerInfo(addr.mac.GetBytesProAdhoc(), &bLocal);
	if(bLocal)
		debug("PLAYER sPeerState!\n");

	//if (id == cMultiGame::Instance().LocalPlayerID()) {
	//	m_pProAdhocPeer = findSelf();
	//	debug("PLAYER sPeerState!\n");
	//}
	//else
	//	m_pProAdhocPeer = findFriend(addr.mac.GetBytesProAdhoc());
	assert(m_pProAdhocPeer);
#ifdef ADHOCCTL_USE_CUSTOM_IDENT
	bVanilaDevice = ((m_pProAdhocPeer->flags & ADHOCCTL_CUSTOM_FLAG) == 0);
#else
	bVanilaDevice = false;
#endif
#endif
}

sPeerState::~sPeerState() {
	while (!m_vElements.empty()) {
		auto it = m_vElements.begin();
		sElement* pElement = it->second;
		uint16 nID = it->first;
		m_vElements.erase(it);

		if (pElement) {
#ifdef FIX_BUGS
			debug("Destroy %u (sPlayer destructor)\n", nID);
#else
			debug("Destroy %u (sPlayer destructor)", nID);
#endif
			gb_mp_will_destroy_elem = true;
			delete pElement; // ! after delete m_vElements size is decrement (dtor remove self)
			gb_mp_will_destroy_elem = false;
		}
	}
	m_vElements.clear();
}

bool sPeerState::IsConnected()
{
	cMultiGame& Game = cMultiGame::Instance();
	if (m_nID == Game.LocalPlayerID())
		return true;
	if (bCheckSender)
		return bSendOverflow == false;
	return false;
}

base::string sPeerState::PeerName()
{
	if (m_sName.empty())
		cAdhoc::Instance().GetPlayerNameFromMacAddr(m_sName, m_Addr.mac);
	return m_sName;
}

uint16 sPeerState::PeerLastAck()
{
	cMultiGame& Game = cMultiGame::Instance();
	if (m_nID == Game.LocalPlayerID())
		return Game.m_pNetSession->m_nCurTime;
	else
		return m_nCurTime;
}

void sPeerState::UpdateAck()
{
	cMultiGame& Game = cMultiGame::Instance();
	m_nCurTime = Game.m_pNetSession->m_nCurTime;
}


cPeerManager::cPeerManager()
{
	m_pAllocFunc = nil;
	m_pFreeFunc = nil;
	m_vPlayers = std::vector<sPeerState*>();
	m_nTeamAPeerGroupId = -99;
	m_nTeamBPeerGroupId = -99;
}

cPeerManager::~cPeerManager()
{
	Terminate();
}

void cPeerManager::SetAllocator(AllocFn allocCB, FreeFn freeCB)
{ 
	m_pAllocFunc = allocCB;
	m_pFreeFunc = freeCB;
}

void cPeerManager::Terminate() {
	while (!m_vPlayers.empty()) {
		DeletePeer(m_vPlayers.front()->m_nID);
	}
}

void cPeerManager::ConnectPeer(uint8 id, tListenAddr& addr)
{
	cMultiGame& Game = cMultiGame::Instance();
	sPeerState* pPeer = nil;
#ifdef MP_USE_CUSTOM_ALLOCATOR
	if (m_pAllocFunc) {
		pPeer = (sPeerState*)(m_pAllocFunc(sizeof(sPeerState)));
		if (pPeer) new (pPeer) sPeerState(id, addr);
	}
	else
#endif
		pPeer = new sPeerState(id, addr);
	assert(pPeer);
	m_vPlayers.push_back(pPeer);
	Game.m_pNetSession->CreatePeerGroup(id);
	Game.m_pNetSession->RegisterGroupPeer(BROADCAST_PEER_GROUPID, id);
	Game.m_pNetSession->RegisterGroupPeer(id, id); // register group as peer id, for send packet to peer -> group
#ifdef MULTIGAME_ELEMENTS_COMPAT_IMPROVEMENTS
	if(pPeer->bVanilaDevice)
		Game.m_pNetSession->RegisterGroupPeer(BROADCAST_VANILLA_DEVICE_GROUPID, id);
	else
		Game.m_pNetSession->RegisterGroupPeer(BROADCAST_CUSTOM_DEVICE_GROUPID, id);
#endif
}

void cPeerManager::DeletePeer(uint8 id)
{
	cMultiGame& Game = cMultiGame::Instance();
	Game.m_pNetSession->DeletePeer(id);
	auto it = std::find_if(m_vPlayers.begin(), m_vPlayers.end(),
		[id](sPeerState* p) { return p && p->m_nID == id; });
	if (it == m_vPlayers.end()) return;

	sPeerState* peer = *it;

#ifdef MP_USE_CUSTOM_ALLOCATOR
	if (m_pFreeFunc) {
		m_pFreeFunc(peer);
	}
	else
#endif
		delete peer;

	m_vPlayers.erase(it);
}

sPeerState* cPeerManager::GetPeerById(uint8 nPeerID)
{
	for (std::vector<sPeerState*>::iterator it = m_vPlayers.begin(); it != m_vPlayers.end(); it++) {
		if ((*it)->m_nID == nPeerID)
			return *it;
	}
	return nil;
}

sPeerState* cPeerManager::GetSelfPeer()
{
	return GetPeerById(TheMPGame.LocalPlayerID());
}

sPeerState* cPeerManager::GetPeerAt(uint8 nIndex)
{
	int32 busyIdx = 0;
	for (int32 i = 0; i < m_vPlayers.size(); ++i) {
		sPeerState* pPeer = m_vPlayers[i];
		if (pPeer != nil) {
			if (busyIdx == nIndex)
				return pPeer;
			++busyIdx;
		}
	}
	return nil;
}

bool cPeerManager::IsPeerConnected(uint8 nPeerID)
{
	sPeerState* peer = GetPeerById(nPeerID);
	return peer && peer->IsConnected();
}

void cPeerManager::UpdatePeerLatency(uint8 nPeerID, int32 latencyMs)
{
	sPeerState* peer = GetPeerById(nPeerID);
	float fLatency = static_cast<float>(latencyMs) * NET_SESSION_R_60;
	float sum = peer->m_latencySum;

	if (peer->m_bufferIndex != (PEER_STATE_MAX_BUFFER_SIZE - 1))
	{
		++peer->m_bufferIndex;
		peer->m_latencySum = sum + fLatency;
		int32 count = peer->m_bufferCount;
		if (!count) {
			peer->m_bufferCount = PEER_STATE_MAX_BUFFER_SIZE;
			count = PEER_STATE_MAX_BUFFER_SIZE;
		}

		--count;
		peer->m_bufferCount = count;
		peer->m_latencyBuffer[count - 1] = fLatency;
	}
	else
	{
		uint32 idx = peer->m_bufferIndex - 1 + peer->m_bufferCount;
		float oldLatency;
		if (idx < PEER_STATE_MAX_BUFFER_SIZE) {
			oldLatency = peer->m_latencyBuffer[idx];
		}
		else {
			oldLatency = peer->m_latencyBuffer[idx - PEER_STATE_MAX_BUFFER_SIZE];
		}

		peer->m_latencySum = sum + (fLatency - oldLatency);
		int32 start = peer->m_bufferStart;
		if (!start) {
			peer->m_bufferStart = PEER_STATE_MAX_BUFFER_SIZE;
			start = PEER_STATE_MAX_BUFFER_SIZE;
		}

		--start;
		peer->m_bufferStart = start;

		int32 count = peer->m_bufferCount;
		if (!count) {
			peer->m_bufferCount = PEER_STATE_MAX_BUFFER_SIZE;
			count = PEER_STATE_MAX_BUFFER_SIZE;
		}

		--count;
		peer->m_bufferCount = count;

		if (count != start) {
			peer->m_latencyBuffer[count] = peer->m_latencyBuffer[start];
		}

		peer->m_latencyBuffer[count] = fLatency;
	}
}

sPeerState* cPeerManager::GetLastPeer() // ?
{
	sPeerState* state;
	int32 i = 0;
	for (std::vector<sPeerState*>::iterator it = m_vPlayers.begin(); it != m_vPlayers.end(); it++) {
		state = GetPeerAt(i++);
	}
	return state;
}

void cPeerManager::SetTeamPeerGroupIds()
{
	cMultiGame& Game = cMultiGame::Instance();
	m_nTeamAPeerGroupId = Game.m_pNetSession->CreatePeerGroup(BROADCAST_TEAM_A_GROUPID);
	m_nTeamBPeerGroupId = Game.m_pNetSession->CreatePeerGroup(BROADCAST_TEAM_B_GROUPID);
	assert(m_nTeamAPeerGroupId == BROADCAST_TEAM_A_GROUPID && m_nTeamBPeerGroupId == BROADCAST_TEAM_B_GROUPID);
}

void cPeerManager::UpdateTeamPeerGroups()
{
	UpdateTeamPeerGroup(static_cast<uint8>(eGameTeam::TEAM_A), m_nTeamAPeerGroupId);
	UpdateTeamPeerGroup(static_cast<uint8>(eGameTeam::TEAM_B), m_nTeamBPeerGroupId);
}

void cPeerManager::UpdateTeamPeerGroup(uint8 teamId, int32 groupId)
{
	cMultiGame& Game = cMultiGame::Instance();
	for (uint32 i = 0; i < m_vPlayers.size(); i++) {
		sPeerState* peer = GetPeerById(i); // index as id? kek
		assert(peer);
		if (Game.GetPlayerTeamID(peer->m_nID) == teamId)
		{
			if (!Game.m_pNetSession->IsSameGroup(groupId, peer->m_nID))
				Game.m_pNetSession->RegisterGroupPeer(groupId, peer->m_nID);
		}
	}
	Game.m_ZoneManager.GetZoneByPeer(groupId)->Update();
}

#ifndef MASTER
void cPeerManager::PrintDebugStuff()
{
	CFont::SetFontStyle(FONT_STANDARD);
	CFont::SetBackgroundOff();
	CFont::SetBackGroundOnlyTextOff();
	CFont::SetWrapx(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH));
	CFont::SetScale(0.85f, 1.0f);
	CFont::SetCentreOff();
	CFont::SetCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH));
	CFont::SetJustifyOff();
	//CFont::SetColor(CRGBA(240, 240, 240, 255));
	CFont::SetDropShadowPosition(1);
	CFont::SetPropOn();
	CFont::SetDropColor(CRGBA(0, 0, 0, 255));

	float x = 16.0f;
	float y = 90.0f;
	const float ystep = 18.0f;
	const int maxLines = 40;
	const int maxFieldsShown = 6;
	char line[512];
	wchar wline[512];

	// Header
	snprintf(line, sizeof(line), "[PEERMANAGER - PEERS] total=%zu", m_vPlayers.size());
	AsciiToUnicode(line, wline);
	CFont::SetColor(CRGBA(240, 20, 20, 255));
	CFont::PrintString(x, y, wline);
	y += ystep;
	CFont::SetColor(CRGBA(200, 200, 200, 255));

	int printed = 0;
	int idx = 0;
	for (std::vector<sPeerState*>::iterator it = m_vPlayers.begin(); it != m_vPlayers.end(); ++it, ++idx) {
		sPeerState* peer = *it;
		if (!peer) continue;

		// stop if screen full
		if (printed >= maxLines) break;

		// id & name
		base::string sName = peer->PeerName(); // PeerName +init from adhoc
		const char* name = sName.empty() ? "[empty]" : sName.c_str();
		int pid = (int)peer->m_nID;

		// connected / spawned
		bool connected = peer->IsConnected();
		bool spawned = peer->m_bIsSpawned;

		//char macBuf[64] = { 0 };
		//{
		//	size_t macBytes = sizeof(peer->m_MacAddr);
		//	const unsigned char* mptr = reinterpret_cast<const unsigned char*>(&peer->m_MacAddr);
		//	if (mptr && macBytes > 0) {
		//		char tmp[8];
		//		macBuf[0] = 0;
		//		for (size_t bi = 0; bi < macBytes; ++bi) {
		//			snprintf(tmp, sizeof(tmp), "%02X", (unsigned int)mptr[bi]);
		//			strncat(macBuf, tmp, sizeof(macBuf) - strlen(macBuf) - 1);
		//			if (bi + 1 < macBytes) strncat(macBuf, ":", sizeof(macBuf) - strlen(macBuf) - 1);
		//		}
		//	}
		//	else {
		//		snprintf(macBuf, sizeof(macBuf), "%p", (void*)&peer->m_MacAddr);
		//	}
		//}

		char macBuf[18];
		peer->m_Addr.mac.ToString(macBuf);

		// latency average (use m_bufferCount & m_latencySum)
		float avgLatency = 0.0f;
		if (peer->m_bufferCount > 0) avgLatency = peer->m_latencySum / (float)peer->m_bufferCount;

		// elements count
		uint32 elemCount = peer->m_vElements.size();

		uint32 totalSyncs = 0;
		uint32 totalSize = 0;
		uint32 hash = 0;
		for (auto& elemPair : peer->m_vElements) {
			sElement* pElement = elemPair.second;
			if (pElement) {
				totalSyncs += pElement->m_vSync.size();
				if (pElement->m_vSync.size()) {
					totalSize += GetSyncSizeByElement(pElement);
					hash = hash_combine(hash, fast_hash32(pElement->m_vSync[0].m_pAttachedElement.element, GetSyncSizeByElement(pElement)));
				}
			}
		}

		// print main line
		snprintf(line, sizeof(line),
			"#%02d id=%d name=\"%s\" conn=%s spwn=%s mac=%s lat=%.1fms elems=%d bfCnt=%d idx=%d start=%d sync=%d %X",
			idx,
			pid,
			name ? name : "(nil)",
			connected ? "Yes" : "No",
			spawned ? "Yes" : "No",
			macBuf,
			avgLatency,
			elemCount,
			peer->m_bufferCount,
			peer->m_bufferIndex,
			peer->m_bufferStart,
			totalSyncs,
			//totalSize,
			hash
		);
		AsciiToUnicode(line, wline);
		CFont::PrintString(x, y, wline);
		y += ystep;
		++printed;

		// show up to maxFieldsShown elements
		if (elemCount > 0) {
			int shown = 0;
			char fields[512] = { 0 };
			for (std::map<uint16, sElement*>::iterator eit = peer->m_vElements.begin();
				eit != peer->m_vElements.end() && shown < maxFieldsShown; ++eit, ++shown) {
				char frag[64];
				uint16 eid = eit->first;
				sElement* elem = eit->second;
				//snprintf(frag, sizeof(frag), "#%u:%p", (unsigned)eid, (void*)elem);
				snprintf(frag, sizeof(frag), "#%u", (unsigned)eid);
				if (fields[0]) strncat(fields, ", ", sizeof(fields) - strlen(fields) - 1);
				strncat(fields, frag, sizeof(fields) - strlen(fields) - 1);
			}
			// if there are more elements than shown, indicate it
			if (elemCount > (size_t)maxFieldsShown) {
				char more[32];
				snprintf(more, sizeof(more), ", ...(+%zu)", elemCount - (size_t)maxFieldsShown);
				strncat(fields, more, sizeof(fields) - strlen(fields) - 1);
			}

			snprintf(line, sizeof(line), "    elements: %s", fields);
			AsciiToUnicode(line, wline);
			CFont::PrintString(x + 8.0f, y, wline);
			y += ystep;
			++printed;
		}

		// small spacer between peers
		y += 2.0f;
	}

	if (m_vPlayers.size() > (size_t)maxLines) {
		snprintf(line, sizeof(line), "... and %zu more peers", m_vPlayers.size() - (size_t)maxLines);
		AsciiToUnicode(line, wline);
		CFont::PrintString(x, y, wline);
	}
}
#endif

#endif