/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "common.h"

#include "multiplayer/elements/sAutomobile.h"
#ifndef GTA_LIBERTY
#include "Heli.h"

enum eHeliSync
{
	MP_PKTD_HELI_EQUAL    = 0,
	MP_PKTD_HELI_AUTOBASE = BIT(0), // sAutomobileBaseSync
	MP_PKTD_HELI_FULL     = -1,     // full diff
};

struct sHeliSync : sAutomobileBaseSync
{
public:

	sHeliSync();
	sHeliSync(CHeli* pHeli);
	sHeliSync(const sHeliSync& other);
	~sHeliSync() override;

	bool Compare(const sHeliSync& other); // inlined

#if !defined(FINAL) && !defined(MASTER)
	void Dump();
#endif

#ifdef MULTIGAME_ELEMENTS_IMPROVEMENTS
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_HELI; }
#endif
};

#pragma pack(push, 1)
struct tHeliSyncsDeltas
{
	uint32 nHeliDiff; // eHeliSync
	tAutomobileBaseSyncsDeltas tAutomobileBaseDiff;

	inline void SetEqual()
	{
		tAutomobileBaseDiff.SetEqual(); // useless
		//nHeliDiff = eHeliSync::MP_PKTD_HELI_EQUAL;
		memset(this, eHeliSync::MP_PKTD_HELI_EQUAL, sizeof(tHeliSyncsDeltas));
	}

	inline void SetDifference()
	{
		tAutomobileBaseDiff.SetDifference(); // useless
		//nHeliDiff = eHeliSync::MP_PKTD_HELI_FULL;
		memset(this, static_cast<uint32>(eHeliSync::MP_PKTD_HELI_FULL), sizeof(tHeliSyncsDeltas)); // 0xFFFFFFFF
	}
};
static_assert(sizeof(tHeliSyncsDeltas) == 36, "sizeof(tHeliSyncsDeltas)");
#pragma pack(pop)

class cHeliMG : public cAutomobileBaseMG {
public:
	float m_fMainRotorAngle;
	float m_fRearRotorAngle;
	uint32 field_188;
	uint32 field_18C;

	cHeliMG(sElement* elem);

	~cHeliMG() override;
	void PreRender(void) override;
	void Render(void) override;
};


struct sHeli : sAutomobileBase {
public:

	sHeli();
	sHeli(CHeli* pHeli);

	static ElementCapability Capability() { return reinterpret_cast<ElementCapability>(sHeli::Capability); }

	ElementCapability GetCapability() override;
	bool HasCapability(ElementCapability capability) override;
	~sHeli() override;
	sElementSync* CreateSync() override;
	void DisposeSync(sElementSync* pSync) override;
	sElementSync* CreateSyncFromOther(sElementSync* pSync) override;
	bool HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) override;
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_HELI; }
	void ApplyClientSync(uint16 nTime) override;
	void Update(uint16 nTime) override;
	bool WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) override;
	void ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) override;
	void ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) override;
	void Initialise() override;
	void SetupModel() override;

	void CompareSyncState(sHeliSync* pSync, sHeliSync* pLastSync, uint32 nDelta, tHeliSyncsDeltas* pDiff);
	void PerformWriteSync(sWriteSyncStream* pSyncStream, sHeliSync* pSync, tHeliSyncsDeltas* pDiff);
	bool WriteSyncDelta(sWriteSyncStream* pSyncStream, sHeliSync* pSync, sHeliSync* pLastSync, uint32 nDelta);

#ifdef FIX_BUGS
	void Fix();
#endif
};
#endif
