/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "multiplayer/net/packet.h"
#include "multiplayer/net/public.h"
//#ifndef GTA_PSP
//#include "multiplayer/net/emu/proAdhoc.h" // SceNetEtherAddr
//#endif

#include <vector>

#define NET_SESSION_DEFAULT_HOST_ID				    (0)
#define NET_SESSION_DEFAULT_PORT					(1)
#define NET_SESSION_TIME_SCALE						((65535 + 1) * 60) // todo ms s conv?
#define NET_SESSION_NUM_PACKETS						(48)
#define NET_SESSION_NUM_SENT_PACKETS				(47)
#define NET_SESSION_NUM_PACKETS_2					(32)
#define NET_SESSION_MAX_PACKET_SIZE					(1400) // 1 MSS
#define NET_SESSION_MAX_GROUP_USED_BYTES			(10000)
#define NET_SESSION_MAX_GROUP_USED_BYTES_PRIORITY	(51200)
#define NET_SESSION_UNK_1							(30)
#define NET_SESSION_PDP_MAX_BUFFER_SIZE				(65523)
#define NET_SESSION_TIME_INVALID					(0x88888888)
#define NET_SESSION_60								(60.0f) // sec? fps?
#define NET_SESSION_R_60							(1.0f / NET_SESSION_60) // 0.016666668  [150sec * 0.016666668 2.5 min]
#define NET_SESSION_UNK_SIZE						(42) // todo


#pragma pack(push, 1)
struct tListenAddr {
	tMacAddr mac; // can be also SceNetEtherAddr
	uint16 port; // uint16  pckt_info_peer ctor -1

	tListenAddr() { mac = tMacAddr(); port = NET_SESSION_DEFAULT_PORT; }
};
typedef tListenAddr tNetAddr;
static_assert(sizeof(tListenAddr) == 8, "tListenAddr");

class cListenInfo 
{
public:
	int32 m_nPdpID;
	tListenAddr m_listenAddr;

	inline cListenInfo() { m_nPdpID = -1; m_listenAddr = tListenAddr(); }
	bool OpenPDP(tListenAddr& listenAddr);
	void ClosePDP(); // DestroyPDP
	void GetPDPListenAddrFromConn(tListenAddr& outListenAddr);
	int32 RecvPDPPacket(void* data, int32 length, tListenAddr& listenAddr);
	bool SendPDPPacket(void* data, int32 length, tListenAddr& listenAddr);
};
typedef cListenInfo cPdpWrapper;
static_assert(sizeof(cListenInfo) == 12, "cListenInfo");
#pragma pack(pop)

//typedef void (*PcktRecvHnd)(void* packet, int player);
struct sWaypoint;
class cMultiGame;
typedef void (*PcktRecvHnd)(net::pckt_base& packet, int sender, uint16 time, bool fromLocalGame);
typedef void (cMultiGame::* PcktRecvHndMultiGame)(net::pckt_base& packet, int sender, uint16 time, bool fromLocalGame);
typedef void (sWaypoint::* PcktRecvHndWaypoint)(net::pckt_base& packet, int sender, uint16 time, bool fromLocalGame);

class cPacketDispatcherBase
{
public:
	int32 m_nRefCount;
	// psp void* m_pVt;

	cPacketDispatcherBase() { m_nRefCount = 0; }
	virtual void PerformDispatchPacket(net::pckt_base& packet, int sender, uint16 time, bool bFromRing) = 0;
	virtual int32 GetID() = 0; // helper, non psp
};

// !! TODO? m_nOffset input functionCB

class cPacketDispatcher : public cPacketDispatcherBase
{
public:
	void* m_pFunctionCB; // on recv

	cPacketDispatcher(void* functionCB) { m_pFunctionCB = functionCB; }
	cPacketDispatcher(PcktRecvHnd functionCB) { m_pFunctionCB = (void*)functionCB; }
	void PerformDispatchPacket(net::pckt_base& packet, int sender, uint16 time, bool bFromRing) override {
		assert(m_pFunctionCB);
		((PcktRecvHnd)m_pFunctionCB)(packet, sender, time, bFromRing);
	};
	int32 GetID() override { return 0; }
};

class cPacketDispatcherMultiGame : public cPacketDispatcherBase
{
public:
	PcktRecvHndMultiGame m_pFunctionCB;
	cMultiGame* m_pCTX;
	cPacketDispatcherMultiGame(PcktRecvHndMultiGame method, cMultiGame* pCTX) : m_pFunctionCB(method), m_pCTX(pCTX) {}
	void PerformDispatchPacket(net::pckt_base& packet, int sender, uint16 time, bool bFromRing) override {
		assert(m_pCTX && m_pFunctionCB);
		(m_pCTX->*m_pFunctionCB)(packet, sender, time, bFromRing);
	}
	int32 GetID() override { return 1; }
};

class cPacketDispatcherWaypoint : public cPacketDispatcherBase
{
public:
	PcktRecvHndWaypoint m_pFunctionCB;
	sWaypoint* m_pCTX;
	cPacketDispatcherWaypoint(PcktRecvHndWaypoint method, sWaypoint* pCTX) : m_pFunctionCB(method), m_pCTX(pCTX) {}
	void PerformDispatchPacket(net::pckt_base& packet, int sender, uint16 time, bool bFromRing) override {
		assert(m_pCTX && m_pFunctionCB);
		(m_pCTX->*m_pFunctionCB)(packet, sender, time, bFromRing);
	}
	int32 GetID() override { return 2; }
};

//template<typename CTX, typename Fn>
//class cPacketDispatcherCTX : public cPacketDispatcherBase
//{
//public:
//	Fn m_pFunctionCB;
//	CTX* m_pCTX;
//
//	cPacketDispatcherCTX(Fn method, CTX* pCTX) : m_pFunctionCB(method), m_pCTX(pCTX) {}
//	//void PerformDispatchPacket(net::pckt_base& packet, int sender, uint16 time, bool bFromRing) override {
//	//	assert(m_pCTX && m_pFunctionCB);
//	//	((*m_pCTX).*m_pFunctionCB)(packet, sender, time, bFromRing);
//	//}
//	void PerformDispatchPacket(net::pckt_base& packet, int sender, uint16 time, bool bFromRing) override;
//	bool HasCTX() override { return true; }
//};

// probably CTimer::m_Timer
class cTimer {
public:
	uint32 nCurTime_lo;
	uint32 nCurTime_hi;
	uint32 nOldTime_lo;
	uint32 nOldTime_hi;
	float fMult;
	float fDelta_2;
	float fDelta_3;
	float fDeltaS;
	float fDeltaMs;

	cTimer();
	//~cTimer() { } // default
	void Reset();
	void Refresh();

private:
	static uint64 get_time_usec(); // platfom time
};

#pragma pack(push, 1)
struct tNetOutgoingPacketHeader {
	uint8 nPlayerId;
	uint8 field_1;
	uint16 field_2; // prio/seq
	uint16 nCurTime;

	tNetOutgoingPacketHeader() { nCurTime = 0; }
};
static_assert(sizeof(tNetOutgoingPacketHeader) == 0x6, "sizeof(tNetOutgoingPacketHeader)"); // 6

struct tNetOutgoingPacket {
	uint32 nSize;
	tNetOutgoingPacketHeader header;

	tNetOutgoingPacket() : header() { }
};
static_assert(sizeof(tNetOutgoingPacket) == 0xA, "sizeof(tNetOutgoingPacket)"); // 10

struct tNetPacket { // mp_net_peer_state_2_init

	tNetOutgoingPacket packet;
	union
	{
		uint16 Seq;
		uint8 nSeqBuff[NET_SESSION_MAX_PACKET_SIZE - sizeof(tNetOutgoingPacketHeader)];
	};

	inline tNetPacket() : packet() {
		static_assert(sizeof(tNetPacket) == 1404, "sizeof(tNetPacket)");
		memset(this, 0, sizeof(tNetPacket));
	} // from PerformInitialConnection
};
static_assert(sizeof(tNetPacket) == 0x57C, "sizeof(tNetPacket)"); // 1404

struct tNetRecvInfo {
	tListenAddr dest;
	uint32 nDelta;
	tNetPacket recv; // size + header + buffer
};

struct tNetPacketReq {
	uint32 nCountA;
	uint32 nDelta;
	tNetPacket req;

	tNetPacketReq() { nDelta = 0; /*req.packet.nCurTime = 0;*//*in tNetOutgoingPacket()*/ }
};
static_assert(sizeof(tNetPacketReq) == 0x584, "sizeof(tNetPacketReq)"); // 1412

struct cNetPeerState_1 { // mp_net_peer_state_1_init
	bool bProcessed;
	uint8 field_1;
	uint8 nSeq;
	uint8 field_3;
	tNetPacketReq* pRequest;
	int32 nDelta;
};
static_assert(sizeof(cNetPeerState_1) == 0xC+(sizeof(void*) - 4), "sizeof(cNetPeerState_1)"); // 12 x86

class cNetPeerState {
public:
#ifdef GTA_LIBERTY
	// recheck
	bool bCheckSender;
	bool bSendOverflow;
#else
	uint8 m_nPeerId;
	uint8 m_pad1;
#endif
	tListenAddr m_Addr;
	uint16 m_nUnk1;
	int32 m_nRandom;
	int32 nSentBegin;
	int32 nSentEnd;
	int32 nSentCount;
	cNetPeerState_1 aSentPackets[NET_SESSION_NUM_SENT_PACKETS];
	uint16 nNextPacketSeq;
	uint16 nLastAck;
	int32 nPacketIndex;
	int32 field_258;
	int32 nCountA;
	tNetPacket aPacket[NET_SESSION_NUM_PACKETS_2];
	int32 nPerfA;
	int32 nPerfB;
	int32 nPerfC;
	int32 aRingB[NET_SESSION_NUM_PACKETS_2];
	int32 nDPerfB;
	float fPerfG;
	float fPerfH;
	int32 nPerfI;
	int32 nPerfJ;
	uint16 nTime_D; // lcs/vcs equal
	uint16 nUnk3; // time?
	uint32 nTime_A;
	uint32 nTime_B;
	float fDPerfA;
	uint32 nPerfD;
	uint32 nPerfE;
	uint32 nPerfF;
	int16 aRingA[NET_SESSION_NUM_PACKETS_2];
	uint16 nTime_C;
	uint16 nTime_E;

	cNetPeerState(tListenAddr& addr, int32 nRandom);
};
static_assert(sizeof(cNetPeerState) == 0xB2E0 + ((sizeof(void*) - 4) * NET_SESSION_NUM_SENT_PACKETS), "sizeof(cNetPeerState)");

struct cSendQeue {
#ifndef GTA_LIBERTY
	int32 m_nID;
#endif
	int16 m_bIsUsed; // 2b 0-1
	int16 m_nForceQueueID;
	std::vector<uint8> m_vecPacketBuffer;
	std::vector<uint8> m_vecPacketBufferPrio;
	std::vector<int32> m_vecPeerList; // peers id's in this group

	inline cSendQeue() {
#if !defined(GTA_LIBERTY) && defined(FIX_BUGS)
		m_nID = 0;
#endif
		m_bIsUsed = 0;
		m_nForceQueueID = BROADCAST_PEER_GROUPID;
		m_vecPacketBuffer = std::vector<uint8>();
		m_vecPacketBufferPrio = std::vector<uint8>();
		m_vecPeerList = std::vector<int32>();
	}
};
//static_assert(sizeof(cSendQeue) == 0x2C, "sizeof(cSendQeue)");
#pragma pack(pop)


class cNetSession
{
public:
	//typedef void* (*AllocFn)(uint32);
	//typedef void (*FreeFn)(void*);
	using AllocFn = void* (*)(uint32);
	using FreeFn = void  (*)(void*);

	AllocFn m_pAllocFunc;
	FreeFn m_pFreeFunc;
	uint32 m_nIdent; // 'VICE' 0x56494345, 'GTA3' 0x47544133
	cListenInfo m_netListen;
	cPacketDispatcherMultiGame* m_pPacketDispatcher;
	int32 field_1C;
	bool m_bHasDisconnected;
	bool m_bSendLimitReached;
	int8 m_Unk2[10];
	int32 m_nMaxPacketSz;
	int32 m_nRandom;
	uint16 m_nSelfPeerID;
	int8 m_Unk3[2]; // pad?
	cTimer m_Timer;
	int8 m_Unk4[4];
	uint16 m_nCurTime;
	int8 m_Unk5[2];
	int32 m_nAdjustedDelta;
	int32 m_nPeerCount; // when 2 players(host+slave) counter is 1 !! m_nSlavePeersCount
	std::vector<cNetPeerState*> m_vPeers;
	std::vector<cSendQeue> m_vSendQueueList;
	int32 m_nSendQueueIndex;
	int32 m_nSendQueueUnkA;
	int32 m_nSendQueueCount;
	tNetPacketReq m_aPackets[NET_SESSION_NUM_PACKETS];
	uint32 m_nBytesSentA;
	uint32 m_nBytesRecv_1;
	uint32 m_nBytesSentB;
	uint32 m_nBytesSentBPrev;
	uint32 m_nPrevAdjustedDeltaB; // uint32?
	uint32 m_nBytesRecv;
	uint32 m_nPrevBytesRecv;
	uint32 m_nPrevAdjustedDeltaBA; // uint32?

	cNetSession(uint32 ident, int32 id);
	~cNetSession();

	void SetAllocator(AllocFn alloc, FreeFn free);
	void Reset();
	bool StartPDPListen(tListenAddr& dest);
	bool ClientConnect(net::pckt_info& packet);
	void UpdateReceive(uint16 time);
	void Terminate();
	void SendMessage(const net::pckt_base& packet, int32 destID);
	void SendMessagePriority(const net::pckt_base& packet, int32 destID);
	void SendAckPacket(const net::pckt_ack& packet, int32 destID);
#ifdef GTA_LIBERTY
	bool IsPeerConnected(int32 nID);
#else
	uint16 GetPeerTime(int32 nPeer);
	int32 CreatePeerGroup(int32 nID);
	bool HasPeerQueue(int32 nPeer);
#endif
	void RegisterGroupPeer(int32 nGroupID, int32 nPeer);
	void RemovePeerFromGroup(int32 nGroupID, int32 nPeer);
	bool IsLocalPlayer(int32 nID);
	bool IsSameGroup(int32 nGroupA, int32 nGroupB);
#ifndef GTA_LIBERTY
	void DeletePeer(int32 nPeerID);
	void PrintAllPeerGroups();
#endif
	void ConnectPeer(int32 id, tListenAddr& dest, int32 random);
	void PerformInitialConnection(tListenAddr& dest);
	void DisconnectPeer(int32 nID);
	void AttemptSendPacket(tNetPacketReq& pPacket, int32 nPeerID, int32 nPacketSeq);
	uint32 PacketAppend(tNetOutgoingPacket& pPacket, std::vector<uint8>::iterator begin, std::vector<uint8>::iterator end);
	void DispatchMessages(int32 nSender, net::pckt_base* packet, net::pckt_base* packetEnd, uint16 nTime, bool bFromLocalGame);
	void ClearPeerArray(int32 nPlayerID);
	void DisconnectAllPeers();
	void InitPacketObj(tNetOutgoingPacket& pPacket);
	void UpdateReceivePvt();
	void UpdatePeerGroups();
#ifndef GTA_LIBERTY
	cSendQeue* GetPeerQueue(int32 nPeer);
#endif
	void UpdateSend();
	void HandleAckRecv(int32 nSender, uint16 nFlags, uint16 nTime);
	void HandlePacketRecv(tNetRecvInfo& info);
	void UpdatePendingSent();
#if !defined(FINAL) && !defined(MASTER)
	void PrintNSDebugStuff();
#endif

	inline bool CanSendHeartbeat() { return m_bSendLimitReached && m_nPeerCount >= 0; }
};