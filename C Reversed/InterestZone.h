/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once
#include "common.h"
#include <vector>
#include <map>

// forward declaration
struct sElement;
class sReadSyncStream;
class sWriteSyncStream;

struct tZonePeer
{
	uint8 nPeerID;
	//uint8 pad1[3];
	std::vector<uint16> acks; // Acknowledgment
	int32 field_10;
	int32 field_14;// v1
	int32 field_18;//
	int32 field_1C;//
	int32 field_20; // v2
	int32 field_24; //
	int32 field_28; //
	uint16 nBasis;
	uint16 nState;
	uint32 nSeqBitmask;
	uint32 nExpectedSeqMask;
};
//static_assert(sizeof(tZonePeer) == 0x38, "sizeof(tZonePeer)");

struct tElementEntry
{
	uint16 nBasis;
	uint32 nEntityID;
};

//struct tAck
//{
//	uint16 time;
//	uint16 state;
//};

struct tAck
{
	uint8 nPeerID;
	//uint8 nPad1;
	uint16 nFrame;
};


class cInterestZone {
public:
	int16 m_nID;
	//int8 m_pad1[2];
	uint32 m_nCurTime; // using also for m_nCurTime += stream.pckt_size cInterestZone::SendGameState()
#ifdef MULTIGAME_ELEMENTS_COMPAT_IMPROVEMENTS
	uint32 m_nCurTimeCustom; // using also for m_nCurTime += stream.pckt_size cInterestZone::SendGameState()
#endif
	bool m_bHasPos;
#ifdef GTA_LIBERTY
	int16 m_nPosX;
	int16 m_nPosY;
#else
	//int8 m_pad2[1];
#endif
	uint16 m_nBasis; // frame/time
	std::vector<tZonePeer> m_vPeers;
	std::vector<sElement*> m_vElements;
	std::vector<tElementEntry> m_vEntities;
	std::vector<tAck> m_vAck; // Acknowledgment tmp buffer from recv to send period

	cInterestZone(int16 nID);
#ifdef GTA_LIBERTY
	~cInterestZone();
	void SendGameState(bool bIsInRange);
	void ReceiveGameState(uint32 nPeerID, uint16 nTime, sReadSyncStream* pSyncStream);
#else
	virtual ~cInterestZone();
	virtual void SendGameState(bool bIsInRange);
	virtual void ReceiveGameState(uint32 nPeerID, uint16 nTime, sReadSyncStream* pSyncStream);
	virtual bool GetNotInRange() = 0;
	virtual void Update() = 0; // Sync()? called from cPeerManager::UpdateTeamPeerGroup, cInterestZoneManager::GetZoneByPeer, cInterestZoneManager::UpdatePeer
#endif

	void RegisterElement(sElement* pElement);
	void RemoveElement(sElement* pElement);
	void DisconnectPeer(uint8 nPeerID);
	uint16 PeerLastAck(uint8 nPeerID);
	bool PeerLastAckEmpty(uint8 nPeerID);
	void AddPeer(uint8 nPeerID);
	bool HasPeer(uint8 nPeerID);
	void DiscardElement();

	inline int16 GetID() { return m_nID; }
	inline tZonePeer* GetZonePeer(uint8 nPeerID) {
		for (auto& peer : m_vPeers) {
			if (peer.nPeerID == nPeerID)
				return &peer;
		}
		return nil;
	}
};

#ifndef GTA_LIBERTY
class cInterestZoneWrapper : public cInterestZone {
public:

	cInterestZoneWrapper(int16 nID);
	~cInterestZoneWrapper() override;
	void SendGameState(bool bIsInRange) override;
	void ReceiveGameState(uint32 nPeerID, uint16 nTime, sReadSyncStream* pSyncStream) override; // TODO: sReadSyncStream nor shure
	bool GetNotInRange() override;
	void Update() override;

	void UpdatePlayers();
};
#endif

class cInterestZoneManager {
private:
	std::map<uint16, cInterestZone*> m_vZones;
public:
	~cInterestZoneManager();
	cInterestZone* GetZoneByPeer(int32 nPeerID);
	void RemovePeerFromAllZones(int32 nPeerID);
	void Terminate();
	void UpdatePeer(int32 nPeerID);
	bool RemoveZone(uint16 nID);
#ifdef GTA_LIBERTY
	void UpdatePlayer(int32 posX, int32 posY);
#else
	void UpdatePlayer();
#endif

	inline std::map<uint16, cInterestZone*>& GetZones() { return m_vZones; }
	inline cInterestZone* FindZone(uint16 nID) {
		auto it = m_vZones.find(nID);
		if (it != m_vZones.end())
			return it->second;
		return nil;
	}
};