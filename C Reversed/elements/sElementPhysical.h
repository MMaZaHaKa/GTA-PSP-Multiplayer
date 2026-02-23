/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "common.h"
#include "Physical.h"

#include "multiplayer/elements/sElement.h"

enum ePhysicalSync
{
	MP_PKTD_PHY_EQUAL      = 0,
	MP_PKTD_PHY_MATRIX     = BIT(0), // m_matrix (right, forward, up)
	MP_PKTD_PHY_POSITION   = BIT(1), // pos
	MP_PKTD_PHY_MOVE_SPEED = BIT(2), // m_vecMoveSpeed
	MP_PKTD_PHY_TURN_SPEED = BIT(3), // m_vecTurnSpeed
	MP_PKTD_PHY_MOVE_FRICT = BIT(4), // m_vecMoveFriction
	MP_PKTD_PHY_TURN_FRICT = BIT(5), // m_vecTurnFriction
	MP_PKTD_PHY_PHYELEM    = BIT(6), // mass
	MP_PKTD_PHY_FLAGS      = BIT(7), // flags
	MP_PKTD_PHY_FULL       = -1,
};

struct sElementPhysicalSync : sElementSync {
private:
	//int8 m_nPad1[8];
	CMGMatrix m_matrix;
	CVector m_vecMoveSpeed;
	CVector m_vecTurnSpeed;
	CVector m_vecMoveFriction;
	CVector m_vecTurnFriction;

	// LVCS Flags B
	union
	{
		struct
		{
			uint8 b154_1 : 1;
			uint8 bHasBlip : 1;
			uint8 b154_4 : 1;
			uint8 bNoRadarForEnemy : 1;
			uint8 b154_10 : 1;
			uint8 b154_20 : 1;
			uint8 b154_40 : 1;
			uint8 b154_80 : 1;
		};
		uint8 m_nPhys_lvcs_unk_flagsB;
	};

protected:
	sElementPhysicalSync();
	sElementPhysicalSync(CPhysical* source);
	sElementPhysicalSync(const sElementPhysicalSync& other); // inlined
	~sElementPhysicalSync() override;

	bool Compare(const sElementPhysicalSync& other);
#if !defined(FINAL) && !defined(MASTER)
	void Dump();
#endif

public:

	inline CVector& GetMoveSpeed() { return m_vecMoveSpeed; }
	inline void SetMoveSpeed(const CVector& vecMoveSpeed) { m_vecMoveSpeed = vecMoveSpeed; }
	inline CVector& GetTurnSpeed() { return m_vecTurnSpeed; }
	inline void SetTurnSpeed(const CVector& vecTurnSpeed) { m_vecTurnSpeed = vecTurnSpeed; }
	inline CVector& GetMoveFriction() { return m_vecMoveFriction; }
	inline void SetMoveFriction(const CVector& vecMoveFriction) { m_vecMoveFriction = vecMoveFriction; }
	inline CVector& GetTurnFriction() { return m_vecTurnFriction; }
	inline void SetTurnFriction(const CVector& vecTurnFriction) { m_vecTurnFriction = vecTurnFriction; }
	inline CMatrix& GetMatrix(void) { return m_matrix; }
	inline void SetMatrix(const CMatrix& newMatrix) { m_matrix = newMatrix; }
	inline uint8& GetFlags(void) { return m_nPhys_lvcs_unk_flagsB; }
	inline void SetFlags(uint8 nPhys_lvcs_unk_flagsB) { m_nPhys_lvcs_unk_flagsB = nPhys_lvcs_unk_flagsB; }

	//inline CVector& GetPosition(void) { return m_matrix.GetPosition(); }
	//inline CVector& GetRight(void) { return m_matrix.GetRight(); }
	//inline CVector& GetForward(void) { return m_matrix.GetForward(); }
	//inline CVector& GetUp(void) { return m_matrix.GetForward(); }
};


/* a CPhysical extension for multigame */
class cPhysicalMG : public CPhysical {
public:
	uElement m_pElem;


	cPhysicalMG(sElement* elem);

	~cPhysicalMG() override;
	void ProcessControl(void) override;
	void Render(void) override;
	void ApplyMoveSpeed(void) override;
	void ApplyTurnSpeed(void) override;
	int32 ProcessEntityCollision(CEntity* ent, CColPoint* colpoints) override;

	inline uElement GetElement(void) { return m_pElem; }


	/* any class that derives from CPlacable cannot be allocated with default ::new operator
	* in other words, this forces the programmer either to use CPools or other allocation
	* strategy.
	*/
	void* operator new(size_t sz) throw() { return malloc(sz); }
	void operator delete(void* p, size_t sz) throw() { free(p); }
};

struct sElementPhysical : sElement {
protected:
	cPhysicalMG* m_pPhyElem; // MG Entity (cPedMG, cAutomobileMG, etc), entity for render net sync by frame time
	bool m_bWasPhyTransfered;
	//int8 m_Pad1[3];
#ifndef GTA_LIBERTY
	uint32 m_nAcks;
#endif
public:

	static int32 ms_nNumberOfSyncedPhysicals;

	sElementPhysical();

	static ElementCapability Capability() { return reinterpret_cast<ElementCapability>(sElementPhysical::Capability); }

	ElementCapability GetCapability() override;
	bool HasCapability(ElementCapability capability) override;
	~sElementPhysical() override;
	void ApplyClientSync(uint16 time) override;
	void ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) override;
	void ApplyDeltaState(sElementSync* pSync, uint16 nTimeDelta) override;
	void InterpolateDeltaState(sElementSync* pSyncA, sElementSync* pSyncB, float fDelta) override;
	void RegisterSelf() override;
	virtual void TransferPhysicalEntity();

	uint8 CompareSyncState(sElementPhysicalSync* pSync, sElementPhysicalSync* pLastSync);
	void PerformWriteSync(sWriteSyncStream* pSyncStream, sElementPhysicalSync* pSync, uint8 nDiffMask);
	void ReadSyncFromStreamPhysical(sReadSyncStream* pSyncStream, sElementPhysicalSync* pOutSync);
	bool HasAckPeerID(uint32 nPeerID);
	bool HasAcksFromAllPeers();
	void AddAckPeerID(uint32 nPeerID);
	void TransferZone();

	inline cPhysicalMG* GetPhysical() { return m_pPhyElem; }
	inline void SetPhysical(cPhysicalMG* pPhyElem) { m_pPhyElem = pPhyElem; };
};