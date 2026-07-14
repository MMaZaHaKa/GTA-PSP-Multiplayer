/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "common.h"
#include "Object.h"
#include "multiplayer/elements/sElementPhysical.h"

#ifdef MULTIGAME_ELEMENTS_IMPROVEMENTS

enum eObjectSyncDelta
{
	MP_PKTD_OBJECT_EQUAL    = 0,
	MP_PKTD_OBJECT_PHYSICAL = BIT(0),
	MP_PKTD_OBJECT_HEALTH   = BIT(1),
	MP_PKTD_OBJECT_FULL     = -1,     // full diff
};

struct sObjectSync : sElementPhysicalSync {
public:
	int8 m_nHealth;

	sObjectSync();
	sObjectSync(CObject* pObject);
	sObjectSync(const sObjectSync& other);
	~sObjectSync() override;

	bool Compare(const sObjectSync& other);
#if !defined(FINAL) && !defined(MASTER)
	void Dump();
#endif

#ifdef MULTIGAME_ELEMENTS_IMPROVEMENTS
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_OBJECT; }
#endif
};

#pragma pack(push, 1)
struct tObjectSyncsDeltas
{
	uint32 nObjectDiff;    // eObjectSyncDelta
	uint32 nPhysicalDiff;  // ePhysicalSync

	inline void SetEqual()
	{
		memset(this, eObjectSyncDelta::MP_PKTD_OBJECT_EQUAL, sizeof(tObjectSyncsDeltas));
		//nObjectDiff = eObjectSyncDelta::MP_PKTD_OBJECT_EQUAL;
		//nPhysicalDiff = ePhysicalSync::MP_PKTD_PHY_EQUAL;
	}

	inline void SetDifference()
	{
		memset(this, static_cast<uint32>(eObjectSyncDelta::MP_PKTD_OBJECT_FULL), sizeof(tObjectSyncsDeltas)); // 0xFFFFFFFF
		//nObjectDiff = eObjectSyncDelta::MP_PKTD_OBJECT_FULL;
		//nPhysicalDiff = ePhysicalSync::MP_PKTD_PHY_FULL;
	}
};
static_assert(sizeof(tObjectSyncsDeltas) == 8, "sizeof(tObjectSyncsDeltas)");
#pragma pack(pop)

class cObjectMG : public cPhysicalMG {
public:

	cObjectMG(sElement* elem);

	~cObjectMG() override;
	void PreRender(void) override;
	void Render(void) override;
};

struct sObject : sElementPhysical {
public:
	CRGBA m_aColours[NUM_VEHICLE_COLOURS];

	sObject();
	sObject(CObject* pObject);

	static ElementCapability Capability() { return reinterpret_cast<ElementCapability>(sObject::Capability); }

	ElementCapability GetCapability() override;
	bool HasCapability(ElementCapability capability) override;
	~sObject() override;
	sElementSync* CreateSync() override;
	void DisposeSync(sElementSync* pSync) override;
	sElementSync* CreateSyncFromOther(sElementSync* pSync) override;
	bool HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) override;
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_OBJECT; }
	void ApplyClientSync(uint16 nTime) override;
	void Update(uint16 nTime) override;
	bool WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) override;
	void ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) override;
	void ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) override;
	void UpdateDelta(sElementSync* pSync, uint16 nTimeDelta) override;
	bool IsTransferable(void) override;
	void TransferEntity(int16 nDestPlayer) override;


	void CompareSyncState(sObjectSync* pSync, sObjectSync* pLastSync, uint32 nDelta, tObjectSyncsDeltas* pDiff);
	void PerformWriteSync(sWriteSyncStream* pSyncStream, sObjectSync* pSync, tObjectSyncsDeltas* pDiff);
	bool WriteSyncDelta(sWriteSyncStream* pSyncStream, sObjectSync* pSync, sObjectSync* pLastSync, uint32 nDelta);
};
#endif