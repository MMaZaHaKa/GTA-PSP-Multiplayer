/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "common.h"

#include "multiplayer/elements/sVehicle.h"
#include "Boat.h"

#ifndef GTA_LIBERTY
enum eBoatSync
{
	MP_PKTD_BOAT_EQUAL   = 0,
	MP_PKTD_BOAT_VEHICLE = BIT(0), // sVehicleSync
	MP_PKTD_BOAT_FULL    = -1,     // full diff
};


struct sBoatSync : sVehicleSync
{
public:

	sBoatSync();
	sBoatSync(CBoat* pBoat);
	sBoatSync(const sBoatSync& other);
	~sBoatSync() override;

	bool Compare(const sBoatSync& other); // inlined

#if !defined(FINAL) && !defined(MASTER)
	void Dump();
#endif

#ifdef MULTIGAME_ELEMENTS_IMPROVEMENTS
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_BOAT; }
#endif
};

#pragma pack(push, 1)
struct tBoatSyncsDeltas
{
	uint8 nBoatDiff;   // eBoatSync
	uint8 field_1[3];  // pad, perfecto
	tVehicleSyncsDeltas tVehicleDiff;

	inline void SetEqual()
	{
		tVehicleDiff.SetEqual(); // useless
		//nBoatDiff = eBoatSync::MP_PKTD_BOAT_EQUAL;
		memset(this, eBoatSync::MP_PKTD_BOAT_EQUAL, sizeof(tBoatSyncsDeltas));
	}

	inline void SetDifference()
	{
		tVehicleDiff.SetDifference(); // useless
		//nBoatDiff = eBoatSync::MP_PKTD_BOAT_FULL;
		memset(this, static_cast<uint32>(eBoatSync::MP_PKTD_BOAT_FULL), sizeof(tBoatSyncsDeltas)); // 0xFFFFFFFF
	}
};
static_assert(sizeof(tBoatSyncsDeltas) == 32, "sizeof(tBoatSyncsDeltas)");
#pragma pack(pop)

class cBoatMG : public cVehicleMG {
public:
	CWaterWake* m_pLeftWake;
	CWaterWake* m_pRightWake;

	cBoatMG(sElement* elem);

	~cBoatMG() override;
	void PreRender(void) override;
	void Render(void) override;
};


struct sBoat : sVehicle {
public:

	sBoat();
	sBoat(CBoat* pBoat);

	static ElementCapability Capability() { return reinterpret_cast<ElementCapability>(sBoat::Capability); }

	ElementCapability GetCapability() override;
	bool HasCapability(ElementCapability capability) override;
	~sBoat() override;
	sElementSync* CreateSync() override;
	void DisposeSync(sElementSync* pSync) override;
	sElementSync* CreateSyncFromOther(sElementSync* pSync) override;
	bool HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) override;
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_BOAT; }
	void ApplyClientSync(uint16 nTime) override;
	void Update(uint16 nTime) override;
	bool WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) override;
	void ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) override;
	void ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) override;
	void SetupModel() override;

	void CompareSyncState(sBoatSync* pSync, sBoatSync* pLastSync, uint32 nDelta, tBoatSyncsDeltas* pDiff);
	void PerformWriteSync(sWriteSyncStream* pSyncStream, sBoatSync* pSync, tBoatSyncsDeltas* pDiff);
	bool WriteSyncDelta(sWriteSyncStream* pSyncStream, sBoatSync* pSync, sBoatSync* pLastSync, uint32 nDelta);

#ifdef FIX_BUGS
	void Fix();
#endif
};
#endif