/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"
#include "General.h"
#include "Font.h" // debug

#include "multiplayer/net/NetSession.h"
#include "multiplayer/MultiGame.h"

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
#include "multiplayer/net/emu/NetAdhocCommon.h"
#include "multiplayer/net/emu/PSPErrorCodes.h"
#include "multiplayer/net/emu/sceNetAdhoc.h"
#include "multiplayer/net/emu/sceNet.h"
#include "multiplayer/net/emu/Utils.h"
#endif
#include "leeds/base/stringt.h"


net::packet_id_list_t gtMP_PacketIDs;
int32_t net::packet_id_list_t::snPacketCount = 0;
net::pckt_def* net::packet_id_list_t::aPacketsDefs[100];
//int gnMP_PacketCount = 0;


bool cListenInfo::OpenPDP(tListenAddr& listenAddr)
{
	ClosePDP();
	m_listenAddr.mac = listenAddr.mac;
	m_listenAddr.port = listenAddr.port;
	m_nPdpID = sceNetAdhocPdpCreate(listenAddr.mac.GetBytesSCE(), listenAddr.port, NET_SESSION_PDP_MAX_BUFFER_SIZE, ADHOC_F_BLOCK);
	if (m_nPdpID >= 0)
		return true;
	debug("error opening socket : %x\n", m_nPdpID);
	assert(false);
	return false;
}

void cListenInfo::ClosePDP()
{
	if (m_nPdpID >= 0) {
		sceNetAdhocPdpDelete(m_nPdpID, 0);
		m_nPdpID = -1;
	}
}

void cListenInfo::GetPDPListenAddrFromConn(tListenAddr& outListenAddr)
{
	outListenAddr.mac = m_listenAddr.mac;
	outListenAddr.port = m_listenAddr.port;
}

int32 cListenInfo::RecvPDPPacket(void* data, int32 length, tListenAddr& listenAddr)
{
	int32 dataLength = length;
	int32 numBytesRecv = sceNetAdhocPdpRecv(m_nPdpID, listenAddr.mac.GetBytesSCE(), PSPSDKPORTARG(&listenAddr.port), data, &dataLength, 0, ADHOC_F_NONBLOCK);
	if (numBytesRecv >= 0) {
#if !defined(FINAL) && !defined(MASTER)
		NET_SESSION_LOG(1, "******************cListenInfo::RecvPDPPacket() ok\n");
#endif
		return dataLength;
	}

	// only lcs, prevent no spam Undefined error on receive
	if (numBytesRecv == SCE_ERROR_NET_ADHOC_WOULD_BLOCK)
		return 0;

	switch (numBytesRecv)
	{
		case SCE_ERROR_NET_ADHOC_INVALID_ARG:
			NET_SESSION_LOG(0, "***** SCE_ERROR_NET_ADHOC_INVALID_ARG on receive\n");
			break;

		case SCE_ERROR_NET_ADHOC_NOT_INITIALIZED:
			NET_SESSION_LOG(0, "***** SCE_ERROR_NET_ADHOC_NOT_INITIALIZED on receive\n");
			break;

		case SCE_ERROR_NET_ADHOC_INVALID_SOCKET_ID:
			NET_SESSION_LOG(0, "***** SCE_ERROR_NET_ADHOC_INVALID_SOCKET_ID on receive\n");
			break;

		case SCE_ERROR_NET_ADHOC_SOCKET_DELETED:
			NET_SESSION_LOG(0, "***** SCE_ERROR_NET_ADHOC_SOCKET_DELETED on receive\n");
			break;

		case SCE_ERROR_NET_ADHOC_SOCKET_ALERTED:
			NET_SESSION_LOG(0, "***** SCE_ERROR_NET_ADHOC_SOCKET_ALERTED on receive\n");
			break;

		case SCE_ERROR_NET_ADHOC_TIMEOUT:
			NET_SESSION_LOG(0, "***** SCE_ERROR_NET_ADHOC_TIMEOUT on receive\n");
			break;

		case SCE_ERROR_NET_INTERNAL:
			NET_SESSION_LOG(0, "***** SCE_ERROR_NET_INTERNAL on receive\n");
			break;

		case SCE_ERROR_NET_ADHOC_NOT_ENOUGH_SPACE:
			NET_SESSION_LOG(0, "***** SCE_ERROR_NET_ADHOC_NOT_ENOUGH_SPACE on receive\n");
			break;

		case SCE_ERROR_NET_ADHOC_THREAD_ABORTED:
			NET_SESSION_LOG(0, "***** SCE_ERROR_NET_ADHOC_THREAD_ABORTED on receive\n");
			break;

		default:
			NET_SESSION_LOG(0, "***** Undefined error on receive\n");
			break;
	}

	return 0;
}

bool cListenInfo::SendPDPPacket(void* data, int32 length, tListenAddr& listenAddr)
{
	int32 nBytesSent = sceNetAdhocPdpSend(m_nPdpID, listenAddr.mac.GetBytesSCE(), *PSPSDKPORTARG(&listenAddr.port), data, length, 0, ADHOC_F_NONBLOCK);
	if (nBytesSent >= 0 || nBytesSent == SCE_ERROR_NET_ADHOC_WOULD_BLOCK) // lcs WOULD_BLOCK in switch
	{
#if !defined(FINAL) && !defined(MASTER)
		NET_SESSION_LOG(1, "******************cListenInfo::SendPDPPacket() ok\n");
#endif
		return true;
	}

	switch (nBytesSent)
	{
		case SCE_ERROR_NET_ADHOC_INVALID_ARG:
			NET_SESSION_LOG(0, "***** SCE_ERROR_NET_ADHOC_INVALID_ARG on send\n");
			break;

		case SCE_ERROR_NET_ADHOC_NOT_INITIALIZED:
			NET_SESSION_LOG(0, "***** SCE_ERROR_NET_ADHOC_NOT_INITIALIZED on send\n");
			break;

		case SCE_ERROR_NET_ADHOC_INVALID_SOCKET_ID:
			NET_SESSION_LOG(0, "***** SCE_ERROR_NET_ADHOC_INVALID_SOCKET_ID on send\n");
			break;

		case SCE_ERROR_NET_ADHOC_SOCKET_DELETED:
			NET_SESSION_LOG(0, "***** SCE_ERROR_NET_ADHOC_SOCKET_DELETED on send\n");
			break;

		case SCE_ERROR_NET_ADHOC_INVALID_ADDR:
			NET_SESSION_LOG(0, "***** SCE_ERROR_NET_ADHOC_INVALID_ADDR on send\n");
			break;

		case SCE_ERROR_NET_ADHOC_INVALID_PORT:
			NET_SESSION_LOG(0, "***** SCE_ERROR_NET_ADHOC_INVALID_PORT on send (%d)\n", listenAddr.port);
			break;

		case SCE_ERROR_NET_ADHOC_INVALID_DATALEN:
			NET_SESSION_LOG(0, "***** SCE_ERROR_NET_ADHOC_INVALID_DATALEN on send\n");
			break;

		case SCE_ERROR_NET_NO_SPACE:
			NET_SESSION_LOG(0, "***** SCE_ERROR_NET_NO_SPACE on send (%d)\n", length);
			break;

		case SCE_ERROR_NET_ADHOC_SOCKET_ALERTED:
			NET_SESSION_LOG(0, "***** SCE_ERROR_NET_ADHOC_SOCKET_ALERTED on send\n");
			break;

		case SCE_ERROR_NET_ADHOC_TIMEOUT:
			NET_SESSION_LOG(0, "***** SCE_ERROR_NET_ADHOC_TIMEOUT on send\n");
			break;

		case SCE_ERROR_NET_INTERNAL:
			NET_SESSION_LOG(0, "***** SCE_ERROR_NET_INTERNAL on send\n");
			break;

		case SCE_ERROR_NET_ADHOC_THREAD_ABORTED:
			NET_SESSION_LOG(0, "***** SCE_ERROR_NET_ADHOC_THREAD_ABORTED on send\n");
			break;

		default:
			NET_SESSION_LOG(0, "***** Undefined error on send\n");
			break;
	}

	return false;
}


cNetPeerState::cNetPeerState(tListenAddr& addr, int32 nRandom)
{
	//m_Addr.mac = addr.mac;
	//m_Addr.port = addr.port;
	m_Addr = addr;
	m_nRandom = nRandom;

	nSentBegin = 0;
	nSentEnd = 0;
	nSentCount = 0;
	//operator new[](aSentPackets, NET_SESSION_NUM_SENT_PACKETS, 12, mp_net_peer_state_1_init);
	nNextPacketSeq = 0;
	nLastAck = 0;
	nPacketIndex = 0;
	field_258 = 0;
	nCountA = 0;
	//operator new[](aPacket, NET_SESSION_NUM_PACKETS_2, 1404, mp_net_peer_state_2_init);
	nPerfA = 0;
	nPerfB = 0;
	nPerfC = 0;
	nDPerfB = 0;
	fPerfG = 0.0;
	fPerfH = 0.0;
	nPerfI = NET_SESSION_UNK_1;
	nPerfJ = NET_SESSION_UNK_1;
	nTime_D = 0;
	nTime_A = NET_SESSION_TIME_INVALID;
	nTime_B = NET_SESSION_TIME_INVALID;
	fDPerfA = 0.0f;
	nPerfD = 0;
	nPerfE = 0;
	nPerfF = 0;
	nTime_C = 0;
}

cTimer::cTimer() {
	Reset();
	fMult = 1.0f;
}

void cTimer::Reset() {
#if !defined(FINAL) && !defined(MASTER)
	//debug("cTimer::Reset\n");
#endif
#ifdef GTA_PSP // !
	sceKernelSysClock clock = sceKernelGetSystemTime();
	uint32 sec, usec;
	sceKernelSysClock2USec(clock, &sec, &usec);
#endif

	const uint64 usec = get_time_usec();
	const uint32 low = static_cast<uint32>(usec & 0xFFFFFFFFu);
	const uint32 high = static_cast<uint32>((usec >> 32) & 0xFFFFFFFFu);

	nCurTime_hi = high;
	nCurTime_lo = low;
	nOldTime_hi = high;
	nOldTime_lo = low;

	fDeltaS = 0.0f;
	fDeltaMs = 0.0f;
	fDelta_2 = 0.0f;
	fDelta_3 = 0.0f;
}

void cTimer::Refresh() {
#if !defined(FINAL) && !defined(MASTER)
	//debug("cTimer::Refresh\n");
#endif
#ifdef GTA_PSP // !
	sceKernelSysClock clock = sceKernelGetSystemTime();
	uint32 sec, usec;
	sceKernelSysClock2USec(clock, &sec, &usec);
#endif
	const uint64 usec = get_time_usec();
	const uint32 curLow = static_cast<uint32>(usec & 0xFFFFFFFFu);
	const uint32 curHigh = static_cast<uint32>((usec >> 32) & 0xFFFFFFFFu);

	const uint32 prevLow = nOldTime_lo;
	const uint32 prevHigh = nOldTime_hi;

	nCurTime_hi = curHigh;
	nCurTime_lo = curLow;

	// calc delta
	// deltaLow = curLow - prevLow (uint32_t arithmetic)
	// borrow = (curLow < prevLow) ? 1 : 0
	// deltaHigh = curHigh - prevHigh - borrow
	const uint32 deltaLow = curLow - prevLow;
	const uint32 borrow = (curLow < prevLow) ? 1u : 0u;
	const uint32 deltaHigh = curHigh - prevHigh - borrow;

	const uint64 delta64 = (static_cast<uint64>(deltaHigh) << 32) | static_cast<uint64>(deltaLow);

	const double deltaSeconds = static_cast<double>(delta64) * 1e-6;
	float fdelta = static_cast<float>(deltaSeconds);

	fdelta = Min(fdelta, 0.1f);

	fDeltaS = fdelta;
	nOldTime_hi = curHigh;
	nOldTime_lo = curLow;

	fDeltaMs = fDeltaS * 1000.0f; // milliseconds
	fDelta_2 = fDeltaS * fMult;   // scaled by fMult
	fDelta_3 = fDeltaMs * fMult;  // ms scaled by fMult
}

// platform helpers
#ifdef GTA_PSP
uint64 cTimer::get_time_usec() {
	SceKernelSysClock clk;
	unsigned long long usec = 0ULL;
	sceKernelGetSystemTime(&clk);
	sceKernelSysClock2USec(&clk, &usec);
	return static_cast<uint64>(usec);
}
#else
#include <chrono>
uint64 cTimer::get_time_usec() {
	using namespace std::chrono;
	const auto now = steady_clock::now().time_since_epoch();
	const uint64 usec = static_cast<uint64>(duration_cast<microseconds>(now).count());
	return usec;
}
#endif

cNetSession::cNetSession(uint32 ident, int32 id) {
#ifdef GTA_LIBERTY
	assert(ident == 'GTA3');
#else
	assert(ident == 'VICE');
#endif
	m_pAllocFunc = nil;
	m_pFreeFunc = nil;
	m_nIdent = ident;
	//m_netListen.m_nPdpID = -1; // ctor
	//m_netListen.m_listenAddr.mac.InitMacAddr(); 	// ctor
	m_netListen.m_listenAddr.port = NET_SESSION_DEFAULT_PORT; // ctor
	m_pPacketDispatcher = nil;
	m_bHasDisconnected = false;
	m_bSendLimitReached = false;
	m_nMaxPacketSz = NET_SESSION_MAX_PACKET_SIZE;
	m_nRandom = base::Random();
	m_nSelfPeerID = id;
	m_Timer = cTimer();
	m_nCurTime = 0;
	m_nAdjustedDelta = 0;
	m_nPeerCount = 0;
	m_vPeers = std::vector<cNetPeerState*>();
	m_vSendQueueList = std::vector<cSendQeue>();
	m_nSendQueueIndex = 0;
	m_nSendQueueUnkA = 0;
	m_nSendQueueCount = 0;
	//  operator new[](this->m_aPackets, 48, 1412, tNetPacketReq::tNetPacketReq);  // ctor
	m_nBytesSentA = 0;
	m_nBytesRecv_1 = 0;
	m_nBytesSentB = 0;
	m_nBytesSentBPrev = 0;
	m_nPrevAdjustedDeltaB = 0;
	m_nBytesRecv = 0;
	m_nPrevBytesRecv = 0;
	m_nPrevAdjustedDeltaBA = 0;
	Reset();
}

cNetSession::~cNetSession()
{
	debug("~cNetSession\n\n");
	for (auto& p : m_vPeers) {
		if (m_pFreeFunc)
			m_pFreeFunc(p);
		else
			delete p; 
	}
	m_vPeers.clear();
	m_netListen.ClosePDP();
	if (m_pPacketDispatcher)
		delete m_pPacketDispatcher;
}

void cNetSession::SetAllocator(AllocFn alloc, FreeFn free)
{
	m_pAllocFunc = alloc;
	m_pFreeFunc = free;
}

void cNetSession::Reset() {
	for (auto& p : m_vPeers) {
#ifdef FIX_BUGS
		if (m_pFreeFunc)
			m_pFreeFunc(p);
		else
#endif
		delete p;
	}
	m_vPeers.clear();
	m_nSendQueueUnkA = m_nSendQueueIndex;
#ifndef GTA_LIBERTY
	m_nSendQueueCount = 0;
#endif
	m_nSelfPeerID = 0;
	m_vSendQueueList.clear();
#ifdef GTA_LIBERTY
	CreatePeerGroup(); // result unused
	CreatePeerGroup();
#else
	CreatePeerGroup(BROADCAST_PEER_GROUPID);
#endif
	// Resize to size=1 with nil (for self slot)
	// recheck!
	if (m_vPeers.size() < 1) {
		m_vPeers.resize(1, nil);
	}
	else if (m_vPeers.size() > 1) {
		m_vPeers.resize(1);
	}
#ifdef GTA_LIBERTY
	RegisterGroupPeer(BROADCAST_PEER_GROUPID, m_nSelfPeerID);
	m_vSendQueueList.at(m_nSelfPeerID).bIsUsed = 1;
	RegisterGroupPeer(m_nSelfPeerID, m_nSelfPeerID);
#endif
	m_bHasDisconnected = false;
	m_nPeerCount = 0;
	field_1C = 0;
}

bool cNetSession::StartPDPListen(tListenAddr& dest) {
	Reset();
	m_nCurTime = 0;
	if (!m_netListen.OpenPDP(dest))
		return false;

	char buffMac[20];
#if 0 // orig code
	// ohh ...
	tListenAddr listenAddr[2];
	m_netListen.GetPDPListenAddrFromConn(listenAddr[0]);
	sceNetEtherNtostr(listenAddr[0].mac.GetBytesSCE(), buffMac);
	base::string sNetHost(buffMac);
	m_netListen.GetPDPListenAddrFromConn(listenAddr[1]);
	debug("Listening on %s:%i\n", sNetHost.c_str(), listenAddr[1].port);
#else
	dest.mac.ToString(buffMac);
	base::string sNetHost(buffMac);
	debug("Listening on %s:%i\n", sNetHost.c_str(), m_netListen.m_listenAddr.port);
#endif
	return true;
}

bool cNetSession::ClientConnect(net::pckt_info& packet) {
	if (m_nPeerCount != 0) return false;
#ifdef GTA_LIBERTY
	RemovePeerFromGroup(BROADCAST_PEER_GROUPID, m_nSelfPeerID);
#endif
	field_1C = 0;
	m_nRandom = base::Random();
	m_nSelfPeerID = packet.nPeerA;
	for (int32 nPeerID = 0; nPeerID < packet.nPeerA; nPeerID++) {
		net::pckt_info_peer& peer = packet.aPeers[nPeerID];
		assert(!peer.mac.IsBroadcast());
		tListenAddr local_addr;
		m_netListen.GetPDPListenAddrFromConn(local_addr); // fine be once before loop 
		if (local_addr.mac == peer.mac && local_addr.port == peer.port) {
			m_nSelfPeerID = nPeerID;
			m_nRandom = peer.nRandom;
			break;
		}
		if (peer.port == 0) {
			m_nSelfPeerID = nPeerID;
			break;
		}
	}
	for (auto& p : m_vPeers) {
#ifdef FIX_BUGS
		if (m_pFreeFunc)
			m_pFreeFunc(p);
		else
#endif
		delete p;
	}
	m_vPeers.clear();

#ifdef GTA_LIBERTY
	m_vSendQueueList.clear();
	FindEmptySlot();
#else
	PeerManager.SetTeamPeerGroupIds();
#endif

	for (int32 nPeerID = 0; nPeerID < packet.nPeerA; nPeerID++) {
		net::pckt_info_peer& peer = packet.aPeers[nPeerID];
		if (peer.port == 0) continue;
#ifdef GTA_LIBERTY
		if (nPeerID == m_nSelfPeerID) continue;
#else
		if (TheAdhoc.IsHost()) continue; // what
#endif

		tListenAddr dest;
		dest.mac = peer.mac;
		dest.port = peer.port;
		ConnectPeer(nPeerID, dest, peer.nRandom);
	}

	if (m_vPeers.size() < packet.nPeerA)
		m_vPeers.resize(packet.nPeerA, nil);

#ifdef GTA_LIBERTY
	TODO();
	TODO();
	TODO();
	uint32 curr_queues = m_vSendQueueList.size();
	if (curr_queues < packet.nPeerA) {
		for (size_t i = curr_queues; i < packet.nPeerA; ++i) {
			m_vSendQueueList.emplace_back();
			// auto& entry = m_vSendQueueList.back();
			// entry.bIsUsed = false;  // u16
			// entry.some_short = -1;
			// entry.block = nil;
			// etc. for other fields (v38=0, v39=0, etc.)
		}
	}
	else if (curr_queues > packet.nPeerA) {
		m_vSendQueueList.resize(packet.nPeerA);
	}

	RegisterGroupPeer(BROADCAST_PEER_GROUPID, m_nSelfPeerID);
	m_vSendQueueList.at(m_nSelfPeerID).bIsUsed = 1;
	RegisterGroupPeer(m_nSelfPeerID, m_nSelfPeerID);
#endif

	return true;
}

void cNetSession::UpdateReceive(uint16 time) {
	m_Timer.Refresh();
	m_nAdjustedDelta += static_cast<int32>(round(m_Timer.fDeltaS * NET_SESSION_TIME_SCALE));
	m_nCurTime = time;

	for (int32 i = 0; i < m_vPeers.size(); ++i) {
		cNetPeerState* pPeer = m_vPeers[i];
		if (!pPeer) continue;

		int32 nA = static_cast<int32>(pPeer->nTime_A);
		int32 nB = static_cast<int32>(pPeer->nTime_B);

		if (static_cast<int16>(nB) == NET_SESSION_TIME_INVALID)
			pPeer->nTime_D = static_cast<int16>(static_cast<int32>(m_nCurTime) + nA);
		else
			pPeer->nTime_D = static_cast<int16>(static_cast<int32>(m_nCurTime) + (nA - nB) / 2);

		if (static_cast<int16>(pPeer->nTime_D) < static_cast<int16>(pPeer->nTime_C))
			pPeer->nTime_D = pPeer->nTime_C;
	}

	UpdateReceivePvt();
	UpdatePendingSent();
}

void cNetSession::Terminate() {
	m_bHasDisconnected = true;
	DisconnectAllPeers();
}

#ifdef GTA_LIBERTY
void cNetSession::SendMessage(const net::pckt_base& packet, int32 nGroupID) {
	if (nGroupID < 0)
		nGroupID = m_vPeers.size() - nGroupID - 1;

	if (nGroupID == m_nSelfPeerID) 
		return;

	assert(nGroupID < m_vSendQueueList.size());

	cSendQeue* pQueue = &m_vSendQueueList[nGroupID];
	uint32 usedBytes = pQueue->vecPacketBuffer.size();
	if (usedBytes > NET_SESSION_MAX_GROUP_USED_BYTES) {
		debug("Too many messages in sendqueue of groupId %d", nGroupID);
		// no cAdhoc::Instance().SetHasError() ?
	}
	else {
		if (pQueue->nForceQueueID != -1)
			pQueue = &m_vSendQueueList[pQueue->nForceQueueID];

		uint32 oldSize = pQueue->vecPacketBuffer.size();
		pQueue->vecPacketBuffer.resize(oldSize + packet.pckt_size);
		memcpy(pQueue->vecPacketBuffer.data() + oldSize, &packet, packet.pckt_size);
	}
}

void cNetSession::SendMessagePriority(const net::pckt_base& packet, int32 nGroupID) {
	cAdhoc& Adhoc = cAdhoc::Instance();
	if (nGroupID == m_nSelfPeerID)
		return;

	if (nGroupID < 0)
		nGroupID = m_vPeers.size() - nGroupID - 1;

	assert(nGroupID < m_vSendQueueList.size());

	cSendQeue* pQueue = &m_vSendQueueList[nGroupID];
	if (pQueue->vecPacketBufferPrio.size() > NET_SESSION_MAX_GROUP_USED_BYTES_PRIORITY) {
		debug("************ Too many messages in send Guaranteed queue of groupId %d\n", nGroupID);
		Adhoc.SetHasError();
	}
	else {
		if (pQueue->nForceQueueID != -1)
			pQueue = &m_vSendQueueList[pQueue->nForceQueueID];

		uint32 oldSize = pQueue->vecPacketBufferPrio.size();
		pQueue->vecPacketBufferPrio.resize(oldSize + packet.pckt_size);
		memcpy(pQueue->vecPacketBufferPrio.data() + oldSize, &packet, packet.pckt_size);
	}
}
#else
void cNetSession::SendMessage(const net::pckt_base& packet, int32 nGroupID) {
	cSendQeue* pQueue = GetPeerQueue(nGroupID);

	if (nGroupID == m_nSelfPeerID)
		return;

#if !defined(FINAL) && !defined(MASTER)
	NET_SESSION_LOG(1, "cNetSession::SendMessage(packet: \"%s\" [%d], nGroupID:%d)\n",
		gtMP_PacketIDs.GetPacketName(packet.pckt_id), packet.pckt_id, nGroupID);
#endif

	assert(pQueue);
	if (pQueue->m_vecPacketBuffer.size() > NET_SESSION_MAX_GROUP_USED_BYTES) {
#ifdef FIX_BUGS
		debug("Too many messages in sendqueue of groupId %d, size %d\n", nGroupID, pQueue->m_vecPacketBuffer.size());
#else
		debug("Too many messages in sendqueue of groupId %d", nGroupID); // LCS
#endif
	}
	else {
		uint32 oldSize = pQueue->m_vecPacketBuffer.size();
		pQueue->m_vecPacketBuffer.resize(oldSize + packet.pckt_size);
		memcpy(pQueue->m_vecPacketBuffer.data() + oldSize, &packet, packet.pckt_size);
	}
}

void cNetSession::SendMessagePriority(const net::pckt_base& packet, int32 nGroupID) {
	if (nGroupID == m_nSelfPeerID)
		return;

	cSendQeue* pQueue = GetPeerQueue(nGroupID);

#if !defined(FINAL) && !defined(MASTER)
	NET_SESSION_LOG(1, "cNetSession::SendMessagePriority(packet: \"%s\" [%d], nGroupID:%d)\n",
		gtMP_PacketIDs.GetPacketName(packet.pckt_id), packet.pckt_id, nGroupID);
#endif

	assert(pQueue);
	if (pQueue->m_vecPacketBufferPrio.size() > NET_SESSION_MAX_GROUP_USED_BYTES_PRIORITY) {
		debug("************ Too many messages in send Guaranteed queue of groupId %d\n", nGroupID); // LCS
		cAdhoc::Instance().SetHasError();
	}
	else {
		uint32 oldSize = pQueue->m_vecPacketBufferPrio.size();
		pQueue->m_vecPacketBufferPrio.resize(oldSize + packet.pckt_size);
		memcpy(pQueue->m_vecPacketBufferPrio.data() + oldSize, &packet, packet.pckt_size);
	}
}
#endif

void cNetSession::SendAckPacket(const net::pckt_ack& packet, int32 destID) {
	if (destID == m_nSelfPeerID) return;
	SendMessage(packet, destID);
}

#ifdef GTA_LIBERTY
/* TODO: stub */
bool cNetSession::IsPeerConnected(int32 nID) {
	if (nID == m_nSelfPeerID) return true;
	if (nID >= (int32)m_vPeers.size()) return false;
	if (m_vPeers.at(nID) == nil) return false;
	// TODO: missing code
	return true;
}
#else
uint16 cNetSession::GetPeerTime(int32 nPeer) {
	for (auto& peer : m_vPeers) {
		if (peer && peer->m_nPeerId == nPeer)
			return peer->nTime_C;
	}
	return 0;
}

int32 cNetSession::CreatePeerGroup(int32 nID) {
	m_vSendQueueList.push_back(cSendQeue()); // emplace_back?
	//m_vSendQueueList.insert(m_vSendQueueList.end(), cSendQeue());
	cSendQeue& newQueue = m_vSendQueueList.back();
	newQueue.m_bIsUsed = 1; // u16 true?
	UpdatePeerGroups();
	newQueue.m_nID = nID;
	return nID;
}

bool cNetSession::HasPeerQueue(int32 nPeer) {
	for (auto& queue : m_vSendQueueList) {
		if (queue.m_nID == nPeer)
			return true;
	}
	return false;
}
#endif

void cNetSession::RegisterGroupPeer(int32 nGroupID, int32 nPeerID) {
#ifdef GTA_LIBERTY
	int32 queueIndex = nGroupID;
	if (nGroupID < 0)
		queueIndex = m_vPeers.size() - nGroupID - 1;
	cSendQeue& queue = m_vSendQueueList[queueIndex];
#else
	cSendQeue& queue = *GetPeerQueue(nGroupID);
#endif
	auto& peers = queue.m_vecPeerList;
	auto it = std::lower_bound(peers.begin(), peers.end(), nPeerID); // find?
	if (it == peers.end() || *it != nPeerID) {
		peers.insert(it, nPeerID); // push_back?
	}
	UpdatePeerGroups();
}

void cNetSession::RemovePeerFromGroup(int32 nGroupID, int32 nPeerID) {
#ifndef GTA_LIBERTY
	if (!IsSameGroup(nGroupID, nPeerID)) return;
#endif

#ifdef GTA_LIBERTY
	int32 queueIndex = nGroupID;
	if (nGroupID < 0)
		queueIndex = m_vPeers.size() - nGroupID - 1;
	cSendQeue& queue = m_vSendQueueList[queueIndex];
#else
	cSendQeue& queue = *GetPeerQueue(nGroupID);
#endif
	auto& peers = queue.m_vecPeerList;
	auto it = std::lower_bound(peers.begin(), peers.end(), nPeerID); // find?
	if (it != peers.end() && *it == nPeerID) {
		auto end_it = it;
		while (end_it != peers.end() && *end_it == nPeerID) ++end_it;
		peers.erase(it, end_it);
	}
	UpdatePeerGroups();
}

bool cNetSession::IsLocalPlayer(int32 nID) {
	return IsSameGroup(nID, m_nSelfPeerID);
}

bool cNetSession::IsSameGroup(int32 nGroupA, int32 nGroupB) {
	if (nGroupA == nGroupB || nGroupA == BROADCAST_PEER_GROUPID)
		return true;

#ifdef GTA_LIBERTY
	if (nGroupA >= 0)
		return false;
#else
	if (!HasPeerQueue(nGroupA))
		return false;
#endif

#ifdef GTA_LIBERTY
	int32 queueIndex = m_vPeers.size() - nGroupA;
	assert(queueIndex < m_vSendQueueList.size() && queueIndex >= 0);
	cSendQeue& groupQueue = m_vSendQueueList[queueIndex];
#else
	cSendQeue& groupQueue = *GetPeerQueue(nGroupA);
#endif
	auto it = std::find(groupQueue.m_vecPeerList.begin(), groupQueue.m_vecPeerList.end(), nGroupB);
	return it != groupQueue.m_vecPeerList.end();
}

#ifndef GTA_LIBERTY
void cNetSession::DeletePeer(int32 nPeerID) {
	for (auto& p : m_vSendQueueList) {
		RemovePeerFromGroup(p.m_nID, nPeerID);
	}
}

void cNetSession::PrintAllPeerGroups() {
	for (uint32 i = 0; i < m_vSendQueueList.size(); i++)
	{
		cSendQeue& q = m_vSendQueueList[i];
		// my vision
		printf("//------ cNetSession::PrintAllPeerGroups() -> cSendQeue %d\n", i);
		printf("  m_nID: %d\n", q.m_nID);
		printf("  m_bIsUsed: %d\n", q.m_bIsUsed);
		printf("  m_nForceQueueID: %d\n", q.m_nForceQueueID);
		printf("  m_vecPacketBuffer.size(): %d\n", (int32)q.m_vecPacketBuffer.size());
		printf("  m_vecPacketBufferPrio.size(): %d\n", (int32)q.m_vecPacketBufferPrio.size());
		printf("  m_vecPeerList.size(): %d\n\n", (int32)q.m_vecPeerList.size());
	}
}
#endif

void cNetSession::ConnectPeer(int32 id, tListenAddr& dest, int32 nRandom) {
	tListenAddr local_addr;
	m_netListen.GetPDPListenAddrFromConn(local_addr);
	bool is_self = (local_addr.mac == dest.mac && local_addr.port == dest.port);
#ifdef GTA_LIBERTY
	if (is_self) return;
#else
	if (is_self) {
		PeerManager.ConnectPeer(id, dest);
		return;
	}
#endif

	uint32 new_size = Max(id, m_nSelfPeerID) + 1;

#ifdef GTA_LIBERTY
	uint32 curr_queues = m_vSendQueueList.size();
	if (new_size > curr_queues) {
		for (size_t i = curr_queues; i < new_size; ++i) {
			m_vSendQueueList.emplace_back();
			// Set defaults as above
			auto& entry = m_vSendQueueList.back();
			entry.bIsUsed = false;
			entry.some_short = -1;
			entry.block = nil;
			TODO();
			TODO();
			TODO();
		}
	}
#endif

	if (new_size > m_vPeers.size())
		m_vPeers.resize(new_size, nil);

	if (m_vPeers.at(id) == nil) {
		cNetPeerState* pPeer = nil;
		if (m_pAllocFunc) {
			pPeer = (cNetPeerState*)(m_pAllocFunc(sizeof(cNetPeerState)));
			if (pPeer) new (pPeer) cNetPeerState(dest, nRandom);
		}
		else {
			pPeer = new cNetPeerState(dest, nRandom);
		}
		assert(pPeer);
#ifndef GTA_LIBERTY
		pPeer->m_nPeerId = id;
#endif
		m_vPeers.at(id) = pPeer;
		++m_nPeerCount;

#ifndef GTA_LIBERTY
		PeerManager.ConnectPeer(id, dest);
#endif

		char buffMac[20];
		//dest.mac.ToString(buffMac); // my ext
		sceNetEtherNtostr(dest.mac.GetBytesSCE(), buffMac);
		base::string macRepr(buffMac);
		debug("Connecting peer %i : %s\n", id, macRepr.c_str());

#ifdef GTA_LIBERTY
		RegisterGroupPeer(BROADCAST_PEER_GROUPID, id);
		m_vSendQueueList.at(id).bIsUsed = true;
		RegisterGroupPeer(id, id);
#endif
	}
}


void cNetSession::PerformInitialConnection(tListenAddr& dest) {
	tNetPacket packet{};
	//packet.packet.nCurTime = 0; // ctor
	//memset(&packet, 0, sizeof(cNetPeerState_3)); // ctor
	InitPacketObj(packet.packet);  // not in ctor?
	int32 nPeerSize = Max((int32)m_vPeers.size(), m_nSelfPeerID + 1);

	net::pckt_info packetInfo{};
	packetInfo.pckt_size = (sizeof(net::pckt_info) + (nPeerSize * sizeof(net::pckt_info_peer))); // cNetPeerState_1 same 12
	packetInfo.pckt_id = gtMP_PacketIDs.info.pckt_id;
	packetInfo.nMagic = 'INFO'; // 0x494E464F
	packetInfo.nPeerA = nPeerSize;
	packetInfo.nPeerB = m_nPeerCount + 1;

	static_assert(sizeof(net::pckt_info) <= sizeof(packet.nSeqBuff));
	memcpy(packet.nSeqBuff, &packetInfo, sizeof(net::pckt_info));

	for (int32 nPeerID = 0; nPeerID < m_vPeers.size(); ++nPeerID) 
	{
		net::pckt_info_peer& entry = packetInfo.aPeers[nPeerID];
		if (nPeerID == m_nSelfPeerID) {
			tListenAddr ownAddr;
			m_netListen.GetPDPListenAddrFromConn(ownAddr);
			entry.port = ownAddr.port;
			entry.mac = ownAddr.mac;
			entry.nRandom = m_nRandom;
		}
		cNetPeerState* pPeer = m_vPeers[nPeerID];
		if (pPeer) {
			entry.mac = pPeer->m_Addr.mac;
			entry.port = pPeer->m_Addr.port;
			entry.nRandom = pPeer->m_nRandom;
		}
	}
	m_netListen.SendPDPPacket(&packet.packet.header, packet.packet.nSize + packetInfo.pckt_size, dest);
}

void cNetSession::DisconnectPeer(int32 nID) {
	debug("cNetSession::DisconnectPeer %d\n");
	if (m_vPeers.at(nID) != nil) {
		ClearPeerArray(nID);
		if (m_pFreeFunc)
			m_pFreeFunc(m_vPeers.at(nID));
		else
			delete m_vPeers.at(nID);
		m_vPeers.at(nID) = nil;
		RemovePeerFromGroup(BROADCAST_PEER_GROUPID, nID);
		RemovePeerFromGroup(nID, nID);
		--m_nPeerCount;
#ifndef GTA_LIBERTY
		PeerManager.DeletePeer(nID);
#endif
	}
	else
		debug("Attempt to remove peer %d from cNetSession failed.\n", nID);
}

void cNetSession::AttemptSendPacket(tNetPacketReq& packetReq, int32 nPeerID, int32 nPacketSeq) {
	cNetPeerState* pPeer = m_vPeers[nPeerID];
#ifdef GTA_LIBERTY
	bool& bSendOverflow = pPeer->bSendOverflow;
#else
	sPeerState* peerState = PeerManager.GetPeerById(pPeer->m_nPeerId);
	bool& bSendOverflow = peerState->bSendOverflow;
#endif
	tNetOutgoingPacket& outPacket = packetReq.req.packet;
	if (outPacket.header.field_2 && !bSendOverflow) {
		uint16 seq;
		if (nPacketSeq != 0) {
			seq = static_cast<uint16>(nPacketSeq); // !!! warn read perbyte here, stream ReadU16?
		}
		else {
			seq = pPeer->nNextPacketSeq; // !!! warn read perbyte here, stream ReadU16?
		}
		*(uint8*)&packetReq.req.Seq = seq & 0xFF;
		*(((uint8*)&packetReq.req.Seq) + 1) = seq >> 8;

		if (pPeer->nSentCount == (NET_SESSION_NUM_SENT_PACKETS - 1)) {
			debug("Dropped peer %i due to send buffer overflow!\n", nPeerID);
			bSendOverflow = true;
			return;
		}

		pPeer->nSentCount++;
		pPeer->nSentBegin = (pPeer->nSentBegin == 0 ? NET_SESSION_NUM_SENT_PACKETS : pPeer->nSentBegin) - 1;
		cNetPeerState_1& entry = pPeer->aSentPackets[pPeer->nSentBegin];
		// recheck
		entry.bProcessed = false;
		entry.pRequest = nil;
		entry.nDelta = 0;

		entry.bProcessed = false;
		entry.pRequest = &packetReq;
		entry.nDelta = m_nAdjustedDelta;
		entry.nSeq = seq;

		if (pPeer->nSentCount == NET_SESSION_NUM_SENT_PACKETS - 1)
			m_bSendLimitReached = true;

		if (nPacketSeq == 0) {
			pPeer->nNextPacketSeq++;
			pPeer->nNextPacketSeq &= 0xFFFu;
			packetReq.nCountA++;
		}
	}
	m_netListen.SendPDPPacket(&outPacket.header, outPacket.nSize, pPeer->m_Addr);
	m_nBytesSentB += outPacket.nSize + NET_SESSION_UNK_SIZE;
	m_nBytesSentA += outPacket.nSize + NET_SESSION_UNK_SIZE;
	if (m_nAdjustedDelta / NET_SESSION_TIME_SCALE != (m_nPrevAdjustedDeltaB / NET_SESSION_TIME_SCALE)) {
		m_nBytesSentBPrev = m_nBytesSentB;
		m_nBytesSentB = 0;
	}
	m_nPrevAdjustedDeltaB = m_nAdjustedDelta;
}

uint32 cNetSession::PacketAppend(tNetOutgoingPacket& pPacket, std::vector<uint8>::iterator begin, std::vector<uint8>::iterator end) {
	uint32 nInitialSz = pPacket.nSize;
	uint32 nCopySz = static_cast<uint32>(end - begin);
	uint32 nMaxSz = m_nMaxPacketSz;

	if (nMaxSz < nCopySz + nInitialSz) {
		nCopySz = 0;
		auto it = begin;
		while (it != end) {
			if (static_cast<uint32>(end - it) < sizeof(uint16)) break;
			uint16 nPacketSize;
			memcpy(&nPacketSize, &*it, sizeof(uint16));
			if (nPacketSize == 0) break;
			if (nMaxSz < nInitialSz + nCopySz + nPacketSize) break;
			nCopySz += nPacketSize;
			it += nPacketSize;
		}
	}

	memcpy(((uint8*)&pPacket.header) + nInitialSz, &*begin, nCopySz);
	pPacket.nSize += nCopySz;
	return nCopySz;
}


void cNetSession::DispatchMessages(int32 nSender, net::pckt_base* packet, net::pckt_base* packetEnd, uint16 nTime, bool bFromLocalGame) {

	while (packet < packetEnd && packet->pckt_size)
	{
		if (packet->pckt_id == gtMP_PacketIDs.ack.pckt_id) {
			HandleAckRecv(nSender, ((net::pckt_ack*)packet)->nFlags, nTime);
		}
		else if (m_pPacketDispatcher) {
			m_pPacketDispatcher->PerformDispatchPacket(*packet, nSender, nTime, bFromLocalGame);
			cAdhoc& Adhoc = cAdhoc::Instance();
			if (Adhoc.HadError()) {
				debug("Adhoc Connection Error, Dropping Out of cNetSession::DispatchMessages\n");
				return;
			}
		}
		packet = (net::pckt_base*)((uint8*)packet + packet->pckt_size);
	}
}

void cNetSession::ClearPeerArray(int32 nPlayerID) {
	cNetPeerState* pPeer = m_vPeers[nPlayerID];
	int32 i = pPeer->nSentBegin;
	while (i != pPeer->nSentEnd) {
		cNetPeerState_1& entry = pPeer->aSentPackets[i];
		if (!entry.bProcessed) {
			--entry.pRequest->nCountA;
		}
		i = (i + 1) % NET_SESSION_NUM_SENT_PACKETS;
	}
	pPeer->nSentEnd = pPeer->nSentBegin;
	pPeer->nSentCount = 0;

	int32 nValue = m_nSendQueueCount;
	while (nValue)
	{
		--nValue;
		int32 idx = (nValue + m_nSendQueueIndex) % NET_SESSION_NUM_PACKETS;
		if (m_aPackets[idx].nCountA) break;
		m_nSendQueueCount = nValue;
		if (m_nSendQueueUnkA) --m_nSendQueueUnkA;
		else m_nSendQueueUnkA = NET_SESSION_NUM_SENT_PACKETS;
	}
}

void cNetSession::DisconnectAllPeers() {
	for (int32 nPeerID = 0; nPeerID < m_vPeers.size(); nPeerID++) {
		cNetPeerState* pPeer = m_vPeers.at(nPeerID);
		if (pPeer == nil) continue;
#ifdef GTA_LIBERTY
		TODO(); // recheck
		pPeer->bSendOverflow = true;
#endif
		DisconnectPeer(nPeerID);
	}
}

void cNetSession::InitPacketObj(tNetOutgoingPacket& packet) {
	static_assert(sizeof(tNetOutgoingPacketHeader) == 0x6, "tNetOutgoingPacketHeader");
	packet.nSize = sizeof(tNetOutgoingPacketHeader); // 6
	packet.header.nCurTime = m_nCurTime;
	packet.header.nPlayerId = m_nSelfPeerID; // lcs in the end func
	packet.header.field_2 = 0;
}

void cNetSession::UpdateReceivePvt() {
	m_nBytesRecv_1 = 0;
	if (m_nAdjustedDelta / NET_SESSION_TIME_SCALE != (m_nPrevAdjustedDeltaBA / NET_SESSION_TIME_SCALE)) {
		m_nPrevBytesRecv = m_nBytesRecv;
		m_nBytesRecv = 0;
	}

	m_nPrevAdjustedDeltaBA = m_nAdjustedDelta;
	tNetRecvInfo recvInfo{};
	//recvInfo.dest.mac.SetBroadcast(); // ctor
	//recvInfo.dest.port = NET_SESSION_DEFAULT_PORT; // ctor
	recvInfo.nDelta = 0;
	recvInfo.recv.packet.header.nCurTime = 0;
	cAdhoc& Adhoc = cAdhoc::Instance();

	while (true) {
		int32 bytes = m_netListen.RecvPDPPacket(&recvInfo.recv.packet.header, NET_SESSION_MAX_PACKET_SIZE, recvInfo.dest);
		recvInfo.recv.packet.nSize = bytes;
		if (!bytes) break;
		recvInfo.nDelta = m_nAdjustedDelta;
		HandlePacketRecv(recvInfo);

		if (Adhoc.HadError()) {
			debug("Adhoc Connection Error, Dropping Out of cNetSession::UpdateReceivePvt()\n");
			return;
		}
	}

	for (int32 nPeerID = 0; nPeerID < (int32)m_vPeers.size(); ++nPeerID) {
		cNetPeerState* pPeer = m_vPeers[nPeerID];
		if (pPeer) {
			while (pPeer->nCountA && pPeer->aPacket[pPeer->nPacketIndex].packet.nSize) {
				tNetPacket& pkt = pPeer->aPacket[pPeer->nPacketIndex];
				net::pckt_base* pPacketStart = (net::pckt_base*)((uint8_t*)pkt.nSeqBuff + 2); // skip seq
				net::pckt_base* pPacketEnd = (net::pckt_base*)((uint8_t*)&pkt.packet.header + pkt.packet.header.field_2 + sizeof(tNetOutgoingPacketHeader)); // +6
				DispatchMessages(nPeerID, pPacketStart, pPacketEnd, pkt.packet.header.nCurTime, true);
				uint16 seq = pkt.Seq; // !!! warn read perbyte here, stream ReadU16? uint16 seq = (pkt.Seq & 0xFF) | ((pkt.Seq >> 8) & 0xFF00);
				pPeer->nLastAck = seq + 1;
				pPeer->nPacketIndex = (pPeer->nPacketIndex == (NET_SESSION_NUM_PACKETS_2 - 1) ? 0 : pPeer->nPacketIndex + 1);
				pPeer->nCountA--;
			}
		}
	}
}

void cNetSession::UpdatePeerGroups() {
	for (uint32 a = 0; a < m_vSendQueueList.size(); ++a) {
		m_vSendQueueList[a].m_nForceQueueID = BROADCAST_PEER_GROUPID;

		// search B with eq queue A
		for (uint32 b = 0; b < a; ++b) {
			std::vector<int32>& peersA = m_vSendQueueList[a].m_vecPeerList;
			std::vector<int32>& peersB = m_vSendQueueList[b].m_vecPeerList;

			if (peersA.size() != peersB.size())
				continue;

			bool equal = true;
			for (uint32 k = 0, n = (uint32)peersA.size(); k < n; ++k) {
				if (peersA[k] != peersB[k]) {
					equal = false;
					break;
				}
			}

			if (equal) {
				m_vSendQueueList[a].m_nForceQueueID = b;
				break;
			}
		}
	}
}

#ifndef GTA_LIBERTY
cSendQeue* cNetSession::GetPeerQueue(int32 nPeer) {
	for (auto& queue : m_vSendQueueList) {
		if (queue.m_nID == nPeer)
			return &queue;
	}
	return nil;
}
#endif

void cNetSession::UpdateSend() {
	m_Timer.Refresh();
	m_nAdjustedDelta += round(m_Timer.fDeltaS * NET_SESSION_TIME_SCALE);
	m_nBytesSentA = 0;
#ifdef GTA_LIBERTY
	cSendQeue& broadcastQueue = m_vSendQueueList[m_vPeers.size()];
#else
	cSendQeue& broadcastQueue = *GetPeerQueue(BROADCAST_PEER_GROUPID);
#endif

	std::vector<uint8>& buffer = broadcastQueue.m_vecPacketBuffer;

	while (!buffer.empty()) {
		tNetPacket packet{};
		InitPacketObj(packet.packet);
		uint32 appended = PacketAppend(packet.packet, buffer.begin(), buffer.end());
		tListenAddr broadcastDest;
		m_netListen.GetPDPListenAddrFromConn(broadcastDest);
		broadcastDest.mac.SetBroadcast(); // broadcast, port from GetPDPListenAddrFromConn
		m_netListen.SendPDPPacket(&packet.packet.header, packet.packet.nSize, broadcastDest);
		m_nBytesSentA += packet.packet.nSize + NET_SESSION_UNK_SIZE;
		m_nBytesSentB += packet.packet.nSize + NET_SESSION_UNK_SIZE;
		buffer.erase(buffer.begin(), buffer.begin() + appended);
	}

	for (int32 q = m_vSendQueueList.size() - 1; q >= 0; --q)
	{
		cSendQeue& queue = m_vSendQueueList[q];
		bool isPeerQueue;
#ifdef GTA_LIBERTY
		isPeerQueue = (q < numPeers);
#else
		isPeerQueue = (queue.m_nID >= 0);
#endif
		uint32 prioOffset = 0;
		uint32 normalOffset = 0;
		while (m_nSendQueueCount < NET_SESSION_NUM_SENT_PACKETS &&
			(queue.m_vecPacketBuffer.size() > normalOffset || (queue.m_vecPacketBufferPrio.size() > prioOffset && !m_bSendLimitReached)))
		{
			m_nSendQueueCount++;
			m_nSendQueueIndex--;
			if (m_nSendQueueIndex < 0)
				m_nSendQueueIndex = (NET_SESSION_NUM_PACKETS - 1);

			tNetPacketReq& req = m_aPackets[m_nSendQueueIndex];
			memset(&req, 0, sizeof(tNetPacketReq));
			req.nCountA = 0;
			req.nDelta = m_nAdjustedDelta;
			InitPacketObj(req.req.packet);

			if (queue.m_vecPacketBufferPrio.size() > prioOffset && !m_bSendLimitReached)
			{
				req.req.packet.nSize += 2;
				uint32 appended = PacketAppend(req.req.packet, queue.m_vecPacketBufferPrio.begin() + prioOffset, queue.m_vecPacketBufferPrio.end());
				req.req.packet.header.field_2 += appended + 2;
				prioOffset += appended;
			}

			if (queue.m_vecPacketBuffer.size() > normalOffset) {
				uint32 appended = PacketAppend(req.req.packet, queue.m_vecPacketBuffer.begin() + normalOffset, queue.m_vecPacketBuffer.end());
				normalOffset += appended;
			}

			uint32 tempSize = req.req.packet.nSize;
			for (auto& peerID : queue.m_vecPeerList) {
				cNetPeerState* pPeer = m_vPeers[peerID];
				if (pPeer) {
					req.req.packet.nSize = tempSize;
					if (!isPeerQueue) {
#ifdef GTA_LIBERTY
						cSendQeue& peerQueue = m_vSendQueueList[peerID];
#else
						cSendQeue& peerQueue = *GetPeerQueue(peerID);
#endif
						if (!peerQueue.m_vecPacketBuffer.empty()) {
							uint32 appended = PacketAppend(req.req.packet, peerQueue.m_vecPacketBuffer.begin(), peerQueue.m_vecPacketBuffer.end());
							if (appended != 0)
								peerQueue.m_vecPacketBuffer.erase(peerQueue.m_vecPacketBuffer.begin(), peerQueue.m_vecPacketBuffer.begin() + appended);
						}
					}
					if (req.req.packet.nSize > 6)
						AttemptSendPacket(req, peerID, 0);
				}
			}

			if (req.nCountA) {
				req.req.packet.nSize = req.req.packet.header.field_2 + sizeof(tNetOutgoingPacketHeader); // 6 recheck !!!
			}
			else 
			{
				m_nSendQueueCount--;
				if (m_nSendQueueIndex == (NET_SESSION_NUM_PACKETS - 1))
					m_nSendQueueIndex = 0;
				else
					m_nSendQueueIndex++;
			}
		}

		if (prioOffset > 0)
			queue.m_vecPacketBufferPrio.erase(queue.m_vecPacketBufferPrio.begin(), queue.m_vecPacketBufferPrio.begin() + prioOffset);

		if (normalOffset > 0)
			queue.m_vecPacketBuffer.erase(queue.m_vecPacketBuffer.begin(), queue.m_vecPacketBuffer.begin() + normalOffset);
	}

	if (m_bHasDisconnected && !m_nSendQueueCount)
		Reset();
}

// --mazahaka calc RTT (Round Trip Time) + JITTER (pizdec)
void cNetSession::HandleAckRecv(int32 nSender, uint16 nFlags, uint16 nTime) {
#define FLOAT_MOD_2POW32 (4294967296.0f)
#define TIME_SCALE_FACTOR (1.0f / (128 * 1024)) // 0.0000076293945f
	cNetPeerState* pPeer = m_vPeers[nSender];
	int32 i = pPeer->nSentBegin;
	while (i != pPeer->nSentEnd) {
		cNetPeerState_1& entry = pPeer->aSentPackets[i];
		uint16 seq = entry.nSeq & 0xFFF;
		if (entry.bProcessed || seq != (nFlags & 0xFFF)) {
			i = (i + 1) % NET_SESSION_NUM_SENT_PACKETS;
			continue;
		}

		--entry.pRequest->nCountA;
		entry.bProcessed = true;

		if ((nFlags & 0xF000) != 0 && entry.nSeq != nFlags) {
			i = (i + 1) % NET_SESSION_NUM_SENT_PACKETS;
			continue;
		}

		int32 nDelta = m_nAdjustedDelta - ((nFlags & 0xF000) ? entry.nDelta : entry.pRequest->nDelta);
		int16 nDeltaA = nTime - static_cast<uint16>(m_nAdjustedDelta >> 16);
		float fDPerfA = pPeer->fDPerfA;

		if (pPeer->nPerfC != NET_SESSION_NUM_PACKETS_2 - 1)
		{
			pPeer->fDPerfA = fDPerfA + static_cast<float>(nDeltaA);
			++pPeer->nPerfF;
			int32 nIndex = (pPeer->nPerfD - 1 + NET_SESSION_NUM_PACKETS_2) % NET_SESSION_NUM_PACKETS_2;
			pPeer->nPerfD = nIndex;
			pPeer->aRingA[nIndex] = nDeltaA;
			pPeer->nDPerfB += nDelta;
			float fDelta = static_cast<float>(nDelta);
			if (nDelta < 0) fDelta += FLOAT_MOD_2POW32;
			pPeer->fPerfG += fDelta * fDelta;
			++pPeer->nPerfC;
			nIndex = (pPeer->nPerfA - 1 + NET_SESSION_NUM_PACKETS_2) % NET_SESSION_NUM_PACKETS_2;
			pPeer->nPerfA = nIndex;
			pPeer->aRingB[nIndex] = nDelta;
		}
		else
		{
			uint32 nIndexC = (pPeer->nPerfF - 1 + pPeer->nPerfD) % NET_SESSION_NUM_PACKETS_2;
			int16 nDeltaB = pPeer->aRingA[nIndexC];
			pPeer->fDPerfA = fDPerfA + (static_cast<float>(nDeltaA) - static_cast<float>(nDeltaB));
			int32 nPerfE = (pPeer->nPerfE - 1 + NET_SESSION_NUM_PACKETS_2) % NET_SESSION_NUM_PACKETS_2;
			pPeer->nPerfE = nPerfE;
			int32 nPerfD = (pPeer->nPerfD - 1 + NET_SESSION_NUM_PACKETS_2) % NET_SESSION_NUM_PACKETS_2;
			pPeer->nPerfD = nPerfD;
			pPeer->aRingA[nPerfD] = nDeltaA;

			uint32 nIndexE = (pPeer->nPerfC - 1 + pPeer->nPerfA) % NET_SESSION_NUM_PACKETS_2;
			int32 oldDelta = pPeer->aRingB[nIndexE];
			pPeer->nDPerfB += nDelta - oldDelta;
			float fNew = static_cast<float>(nDelta);
			if (nDelta < 0) fNew += FLOAT_MOD_2POW32;
			float fOld = static_cast<float>(oldDelta);
			if (oldDelta < 0) fOld += FLOAT_MOD_2POW32;
			pPeer->fPerfG += fNew * fNew - fOld * fOld;
			int32 nPerfB = (pPeer->nPerfB - 1 + NET_SESSION_NUM_PACKETS_2) % NET_SESSION_NUM_PACKETS_2;
			pPeer->nPerfB = nPerfB;
			int32 nPerfA = (pPeer->nPerfA - 1 + NET_SESSION_NUM_PACKETS_2) % NET_SESSION_NUM_PACKETS_2;
			pPeer->nPerfA = nPerfA;
			pPeer->aRingB[nPerfA] = nDelta;
		}

		float fValueE = static_cast<float>(pPeer->nDPerfB);
		if (pPeer->nDPerfB < 0) fValueE += FLOAT_MOD_2POW32;
		float fValueF = static_cast<float>(pPeer->nPerfC);
		if (pPeer->nPerfC < 0) fValueF += FLOAT_MOD_2POW32;
		pPeer->fPerfH = fValueE / fValueF;
		float variance = (pPeer->fPerfG - (fValueE * fValueE / fValueF)) / fValueF;
		pPeer->nPerfI = static_cast<int32>(roundf(pPeer->fPerfH + 0.5f * sqrtf(variance)));
		pPeer->nTime_A = static_cast<uint32>(roundf((pPeer->fDPerfA / static_cast<float>(pPeer->nPerfF) + pPeer->fPerfH * TIME_SCALE_FACTOR)));
		break;
	}
#undef TIME_SCALE_FACTOR
#undef FLOAT_MOD_2POW32
}

void cNetSession::HandlePacketRecv(tNetRecvInfo& info) {
	m_nBytesRecv += info.recv.packet.nSize + NET_SESSION_UNK_SIZE;
	m_nBytesRecv_1 += info.recv.packet.nSize + NET_SESSION_UNK_SIZE;

	int32 nSender = info.recv.packet.header.nPlayerId;
	if (nSender >= m_vPeers.size() || !m_vPeers[nSender]) return;

	cNetPeerState* pPeer = m_vPeers[nSender];

#ifdef GTA_LIBERTY
	if (pPeer->bCheckSender) {
		if (pPeer->m_Addr.mac != info.dest.mac || pPeer->m_Addr.port != info.dest.port) return;
	}
	pPeer->bCheckSender = true;
	pPeer->m_Addr = info.dest;
#else
	cPeerManager& PeerMgr = PeerManager;
	sPeerState* pPeerState = PeerMgr.GetPeerById(pPeer->m_nPeerId);
	if (pPeerState->IsConnected()) {
		if (pPeer->m_Addr.mac != info.dest.mac || pPeer->m_Addr.port != info.dest.port) return;
	}
	if (!pPeerState->bCheckSender) {
		pPeerState->bCheckSender = true;
	}
	pPeer->m_Addr = info.dest;
#endif

	uint16 nTime = info.recv.packet.header.nCurTime;
	if (pPeer->nTime_C < nTime) pPeer->nTime_C = nTime;

	net::pckt_base* pPacketStart = (net::pckt_base*)(((uint8*)&info.recv.Seq) + info.recv.packet.header.field_2);
	net::pckt_base* pPacketEnd = (net::pckt_base*)(((uint8*)&info.recv.packet.header) + info.recv.packet.nSize);
	DispatchMessages(nSender, pPacketStart, pPacketEnd, nTime, false);

	if (info.recv.packet.header.field_2) {
		uint16 nFlags = info.recv.Seq; // !!! warn read perbyte here, stream ReadU16? uint16 seq = (info.recv.Seq & 0xFF) | ((info.recv.Seq >> 8) & 0xFF00);
		int32 delta = (nFlags - pPeer->nLastAck) & 0xFFF;
		if (delta & 0x800) delta -= 0x1000; // ------------------------------------

		int32 nextDelta = delta + 1;
		if (nextDelta < NET_SESSION_NUM_PACKETS_2 && delta >= 0) {
			int32 slot;
			if (delta < pPeer->nCountA) {
				slot = (pPeer->nPacketIndex + delta) % NET_SESSION_NUM_PACKETS_2;
				if (pPeer->aPacket[slot].packet.nSize == 0) {
					memcpy(&pPeer->aPacket[slot].packet.header, &info.recv.packet.header, info.recv.packet.nSize);
					pPeer->aPacket[slot].packet.nSize = info.recv.packet.nSize;
				}
			}
			else {
				int32 oldCountA = pPeer->nCountA;
				pPeer->nCountA = nextDelta;
				pPeer->field_258 = (pPeer->nPacketIndex + nextDelta) % NET_SESSION_NUM_PACKETS_2;
				for (int32 k = oldCountA; k < delta; ++k) {
					slot = (pPeer->nPacketIndex + k) % NET_SESSION_NUM_PACKETS_2;
					pPeer->aPacket[slot].packet.nSize = 0;
				}
				slot = (pPeer->nPacketIndex + delta) % NET_SESSION_NUM_PACKETS_2;
				memcpy(&pPeer->aPacket[slot].packet.header, &info.recv.packet.header, info.recv.packet.nSize);
				pPeer->aPacket[slot].packet.nSize = info.recv.packet.nSize;
			}

			net::pckt_ack packet{};
			packet.pckt_size = sizeof(net::pckt_ack);
			packet.pckt_id = gtMP_PacketIDs.ack.pckt_id;
			packet.nFlags = nFlags;
			SendAckPacket(packet, nSender);
		}
	}
}

void cNetSession::UpdatePendingSent() {
	while (m_nSendQueueCount > 0)
	{
		int32 checkIdx = (m_nSendQueueIndex + m_nSendQueueCount - 1) % NET_SESSION_NUM_PACKETS;
		if (m_aPackets[checkIdx].nCountA > 0) break;
		m_nSendQueueCount--;
		if (m_nSendQueueUnkA > 0) --m_nSendQueueUnkA;
		else m_nSendQueueUnkA = NET_SESSION_NUM_SENT_PACKETS - 1;
	}

	m_bSendLimitReached = false;

	for (int32 nPeerID = 0; nPeerID < (int32)m_vPeers.size(); ++nPeerID)
	{
		cNetPeerState* pPeer = m_vPeers[nPeerID];
		if (!pPeer) continue;
#ifdef GTA_LIBERTY
		if (pPeer->nSentCount && !pPeer->bSendOverflow)
#else
		sPeerState* peerState = PeerManager.GetPeerById(pPeer->m_nPeerId);
		if (pPeer->nSentCount && !peerState->bSendOverflow)
#endif
		{
			int32 curIdx = pPeer->nSentBegin;
			while (curIdx != pPeer->nSentEnd) {
				cNetPeerState_1& pkt = pPeer->aSentPackets[curIdx];
				if (pkt.bProcessed) break;
				if (pPeer->nPerfI < (m_nAdjustedDelta - pkt.nDelta)) break;
				curIdx = (curIdx + 1) % NET_SESSION_NUM_SENT_PACKETS;
			}

			if (curIdx != pPeer->nSentEnd) {
				int32 processIdx = pPeer->nSentEnd;
				do {
					processIdx = (processIdx == 0 ? NET_SESSION_NUM_SENT_PACKETS : processIdx) - 1;
					cNetPeerState_1& pkt = pPeer->aSentPackets[processIdx];
					--pPeer->nSentCount;
					pPeer->nSentEnd = (pPeer->nSentEnd == 0 ? NET_SESSION_NUM_SENT_PACKETS : pPeer->nSentEnd) - 1;
					if (!pkt.bProcessed) {
						AttemptSendPacket(*pkt.pRequest, nPeerID, pkt.nSeq + 0x1000); // resend lost packet --------------------
					}
				} while (processIdx != curIdx);
			}
		}

#ifdef GTA_LIBERTY
		if (pPeer->nSentCount == NET_SESSION_NUM_SENT_PACKETS - 1 && !pPeer->bSendOverflow)
			m_bSendLimitReached = true;
#else
		if (pPeer->nSentCount == NET_SESSION_NUM_SENT_PACKETS - 1 && !peerState->bSendOverflow)
			m_bSendLimitReached = true;
#endif
	}
}

#if !defined(FINAL) && !defined(MASTER)
void cNetSession::PrintNSDebugStuff() {
	CFont::SetFontStyle(FONT_STANDARD);
	CFont::SetBackgroundOff();
	CFont::SetBackGroundOnlyTextOff();
	CFont::SetWrapx(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH));
	CFont::SetScale(0.85f, 1.0f);
	CFont::SetCentreOff();
	CFont::SetCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH));
	CFont::SetJustifyOff();
	CFont::SetDropShadowPosition(1);
	CFont::SetPropOn();
	CFont::SetDropColor(CRGBA(0, 0, 0, 255));
	//float x = SCREEN_SCALE_X(16.0f);
	//float y = SCREEN_SCALE_Y(50.0f);
	float x = 16.0f;
	float y = 300.0f;
	float line_height = SCREEN_SCALE_Y(16.0f);
	char line[512];
	wchar wline[512];
	for (int i = 0; i < m_vPeers.size(); i++) {
		cNetPeerState* pPeer = m_vPeers[i];
		if (pPeer == nil) continue;
		// nSentCount > 0 (active), red if overflow or inactive, white otherwise
#ifndef GTA_LIBERTY
		uint8 peer_id = pPeer->m_nPeerId;
#else
		uint8 peer_id = i; // Fallback to index
#endif
		if (pPeer->nSentCount > 0) {
			CFont::SetColor(CRGBA(39, 152, 7, 255)); // green
		}
		else if (pPeer->nSentCount == 0) {
			CFont::SetColor(CRGBA(240, 240, 240, 255)); // white
		}
		else {
			CFont::SetColor(CRGBA(240, 20, 20, 255)); // red
		}
		sprintf(line, "Peer %d: Sent B/E/C %d/%d/%d NextSeq %hu LastAck %hu PktIdx %d PerfA/B/C %d/%d/%d DPerfB %d fPerfG/H %.2f/%.2f PerfI/J %d/%d TimeD %hu Unk3 %hu TimeA/B %u/%u fDPerfA %.2f PerfD/E/F %u/%u/%u TimeC %hu TimeE %hu",
			peer_id,
			pPeer->nSentBegin, pPeer->nSentEnd, pPeer->nSentCount,
			pPeer->nNextPacketSeq, pPeer->nLastAck, pPeer->nPacketIndex,
			pPeer->nPerfA, pPeer->nPerfB, pPeer->nPerfC,
			pPeer->nDPerfB, pPeer->fPerfG, pPeer->fPerfH,
			pPeer->nPerfI, pPeer->nPerfJ,
			pPeer->nTime_D, pPeer->nUnk3,
			pPeer->nTime_A, pPeer->nTime_B,
			pPeer->fDPerfA,
			pPeer->nPerfD, pPeer->nPerfE, pPeer->nPerfF,
			pPeer->nTime_C, pPeer->nTime_E);
		AsciiToUnicode(line, wline);
		CFont::PrintString(x, y, wline);
		y += line_height;
	}
}
#endif

uint32_t fast_hash32(const void* key, uint32_t len, uint32_t seed) {
	const uint32_t m = 0x5bd1e995u;
	const int r = 24;
	uint32_t h = seed ^ static_cast<uint32_t>(len);
	const unsigned char* data = static_cast<const unsigned char*>(key);
	while (len >= 4) {
		uint32_t k;
		memcpy(&k, data, sizeof(k));
		k *= m;
		k ^= k >> r;
		k *= m;
		h *= m;
		h ^= k;
		data += 4;
		len -= 4;
	}
	switch (len) {
	case 3: h ^= uint32_t(data[2]) << 16;
	case 2: h ^= uint32_t(data[1]) << 8;
	case 1: h ^= uint32_t(data[0]); h *= m;
	}
	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;
	return h;
}

uint32_t hash_combine(uint32_t seed, uint32_t value) {
	return seed ^ (value + 0x9e3779b9u + (seed << 6) + (seed >> 2));
}
