/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "common.h"

#include "multiplayer/events/public.h"
#include "multiplayer/elements/sElement.h"

enum ePowerupType
{
	POWERUP_NONE = (0),
	QUAD_DAMAGE  = BIT(0), // MEGA_DAMAGE
	REGENRATOR   = BIT(1),
	INVISIBLE    = BIT(2),
	KILL_FENZY   = BIT(3),
	UBER_PICKUP  = BIT(4),
#ifndef GTA_LIBERTY
	FLAGBALL     = BIT(5),
#endif
};

enum ePickupSync
{
	MP_PKTD_PICKUP_EQUAL     = 0,
	MP_PKTD_PICKUP_BASE      = BIT(0), // m_eModelIndex, m_nQuantity, m_eType
	MP_PKTD_PICKUP_POSITION  = BIT(1), // m_vecPos
	MP_PKTD_PICKUP_REMOVED   = BIT(2), // m_bRemoved
	MP_PKTD_PICKUP_PICKUP_BY = BIT(3), // m_nPickupBy
	MP_PKTD_PICKUP_FULL_DIFF = -1,     // full diff u8 0xFF
};


// forward declaration
class CPickup; // avoid cyclic dependency

struct sPickupSync : sElementSync {
private:
	//int8 m_nPad1[8];
	CVector m_vecPos;
public:
	bool m_bRemoved;
	int8 m_nPickupBy;
	//int8 m_nPad2[14];

	sPickupSync();
	sPickupSync(CPickup* pSrc);
	sPickupSync(const sPickupSync& other); // inlined
	~sPickupSync() override;

	bool Compare(const sPickupSync& other);
	//bool operator==(const sPickupSync& other); // old because it doesn't check all sync fields
#if !defined(FINAL) && !defined(MASTER)
	void Dump();
#endif

	inline CVector& GetPosition() { return m_vecPos; }
#ifdef MULTIGAME_ELEMENTS_IMPROVEMENTS
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_PICKUP; }
#endif
};

struct sPickup : sElement {
private:
	int32 m_nPickHandle;
	uint8 m_eType;
	uint8 m_nQuantity; // in CPickup uint32!
	int16 m_eModelIndex;
	bool m_bPickupedUp;
	//int8 m_nPad0;
	uint16 m_nExpireTime;
	uint16 m_nEventTime;
	//int8 m_nPad1[2];
	int32 m_nPickupBy;
public:

	sPickup();
	sPickup(int32 handle);

	static ElementCapability Capability() { return reinterpret_cast<ElementCapability>(sPickup::Capability); }

	ElementCapability GetCapability() override;
	bool HasCapability(ElementCapability capability) override;
	~sPickup() override;
	sElementSync* CreateSync() override;
	void DisposeSync(sElementSync* pSync) override;
	sElementSync* CreateSyncFromOther(sElementSync* pSync) override;
	bool HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) override;
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_PICKUP; }
	void ApplyClientSync(uint16 nTime) override;
	void Update(uint16 nTime) override;
	bool WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) override;
	void ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) override;


	void RequestCollect(uint16 time, int32 who);
#ifndef GTA_LIBERTY
	void Collect(int16 modelIndex);
#else
	void Collect();
#endif

	uint8 CompareSyncState(sPickupSync* pSync, sPickupSync* pLastSync); // inline
	bool WriteSyncDelta(sWriteSyncStream* pSyncStream, sPickupSync* pSync, sPickupSync* pLastSync); // PerformWriteSyncDelta?
	void PerformWriteSync(sWriteSyncStream* pSyncStream, sPickupSync* pSync, uint8 nDiffMask);
};

void mg_pickup_send_on_collected(sElement* elem);
