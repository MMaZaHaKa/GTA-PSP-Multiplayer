/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "common.h"

#include "multiplayer/net/public.h"
#include "leeds/base/stringt.h"
#include "singletonManager.h"
#include <map>

#ifndef fGTA_LIBERTY
// forward decl
struct sElement;

#define PEER_STATE_MAX_BUFFER_SIZE (15)

class sPeerState // sPeer
{
public:
	int8 m_nID;
	//int8 pad1[1];
	uint16 m_nCurTime;
	tListenAddr m_Addr;
	bool bCheckSender;
	bool bSendOverflow;
	//int8 pad2[2];
	base::string m_sName;
	bool m_bIsSpawned;
	//int8 pad3[3];
	float m_latencySum;
	int32 m_bufferCount;
	int32 m_bufferStart;
	int32 m_bufferIndex;
	float m_latencyBuffer[PEER_STATE_MAX_BUFFER_SIZE];
	uint16 m_nTimeA; // m_nCurTime?
	uint16 m_nTimeB;
	std::map<uint16, sElement*> m_vElements;

	sPeerState(int32 id, tListenAddr& addr);
	~sPeerState();

	bool IsConnected();
	base::string PeerName();
	uint16 PeerLastAck();
	void UpdateAck();

	inline sElement* FindElement(int id) {
		std::map<uint16, sElement*>::iterator it = m_vElements.find(id);
		if (it != m_vElements.end()) return it->second;
		return nil;
	}
	inline void UpdateElements(uint16 time) {
		for (std::map<uint16, sElement*>::iterator it = m_vElements.begin(); it != m_vElements.end(); it++) {
			it->second->Update(time);
		}
	}
	inline void InsertElement(sElement* elem) {
		m_vElements.insert(std::map<uint16, sElement*>::value_type(elem->GetID(), elem));
	}
	inline void RemoveElement(sElement* elem) {
		std::map<uint16, sElement*>::iterator it = m_vElements.find(elem->GetID());
		if (it != m_vElements.end()) m_vElements.erase(it);
	}
};

class cPeerManager : public base::cSingletonBase
{
public:
	//typedef void* (cPeerManager::* AllocFn)(int32);
	//typedef void (cPeerManager::* FreeFn)(void*);
	using AllocFn = void* (*)(uint32);
	using FreeFn = void  (*)(void*);

	AllocFn m_pAllocFunc;
	FreeFn m_pFreeFunc;
	std::vector<sPeerState*> m_vPlayers; // simplify
	int32 m_nTeamAPeerGroupId; // m_nRedTeamPeerGroupId  mp_lsn_GetRedTeamPeerGroupId
	int32 m_nTeamBPeerGroupId; // m_nBlueTeamPeerGroupId mp_lsn_GetBlueTeamPeerGroupId

	cPeerManager();
	~cPeerManager();

	void SetAllocator(AllocFn allocCB, FreeFn freeCB);
	void Terminate();
	void ConnectPeer(uint8 id, tListenAddr& addr);
	void DeletePeer(uint8 id);
	sPeerState* GetPeerById(uint8 nPeerID);
	sPeerState* GetSelfPeer();
	sPeerState* GetPeerAt(uint8 nPeerID);
	bool IsPeerConnected(uint8 nPeerID);
	void UpdatePeerLatency(uint8 nPeerID, int32 latencyMs);
	sPeerState* GetLastPeer();
	void SetTeamPeerGroupIds();
	void UpdateTeamPeerGroups();
	void UpdateTeamPeerGroup(uint8 teamId, int32 groupId);
#ifndef MASTER
	void PrintDebugStuff();
#endif
};

cPeerManager* base::cSingleton<cPeerManager>::mspInstance = nil;
#define PeerManager (*base::cSingleton<cPeerManager>::Instance())

#endif