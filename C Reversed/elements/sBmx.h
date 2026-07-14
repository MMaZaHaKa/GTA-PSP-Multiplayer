/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "common.h"

#include "multiplayer/elements/sVehicle.h"
#ifndef GTA_LIBERTY
#include "Bmx.h"

#ifdef MULTIGAME_ELEMENTS_COMPAT_IMPROVEMENTS
	#define MULTIGAME_BMX_IMPROVEMENTS // this kill psp compat (sync reader/writer)
#endif
#if defined(MULTIGAME_BMX_IMPROVEMENTS) && !defined(ADHOCCTL_USE_CUSTOM_IDENT)
static_assert(false, "BMX IMPROVEMENTS without user device detection will break sync parser compatibility"); // need flag to skip send custom delta
#endif

enum eBmxSync
{
	MP_PKTD_BMX_EQUAL           = 0,
	MP_PKTD_BMX_VEHICLE         = BIT(0), // sVehicleSync
	MP_PKTD_BMX_PEDAL_ANGLE_L   = BIT(1), // m_fPedalAngleL
	MP_PKTD_BMX_PEDAL_ANGLE_R   = BIT(2), // m_fPedalAngleR
	MP_PKTD_BMX_CRANK_ANGLE     = BIT(3), // m_fCrankAngle
#ifdef MULTIGAME_BMX_IMPROVEMENTS
	MP_PKTD_BMX_BAR_STEER_ANGLE = BIT(4), // m_fBarSteerAngle // руль
	MP_PKTD_BMX_WHEEL_STATUS_0  = BIT(5), // m_wheelStatus[0]
	MP_PKTD_BMX_WHEEL_STATUS_1  = BIT(6), // m_wheelStatus[1]
	MP_PKTD_BMX_LEAN_ANGLE      = BIT(7), // m_fLeanAngle // лево/право
#endif
	MP_PKTD_BMX_FULL            = -1,     // full diff
};

struct sBmxSync : sVehicleSync
{
public:
	float m_fPedalAngleL;
	float m_fPedalAngleR;
	float m_fCrankAngle;
#ifdef MULTIGAME_BMX_IMPROVEMENTS
	float m_fBarSteerAngle;
	float m_fLeanAngle;
	uint8 m_wheelStatus[2];
#endif

	sBmxSync();
	sBmxSync(CBmx* pBmx);
	sBmxSync(const sBmxSync& other);
	~sBmxSync() override;

	bool Compare(const sBmxSync& other); // inlined

#if !defined(FINAL) && !defined(MASTER)
	void Dump();
#endif

#ifdef MULTIGAME_ELEMENTS_IMPROVEMENTS
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_BMX; }
#endif
};

#pragma pack(push, 1)
struct tBmxSyncsDeltas
{
	uint32 nBmxDiff;   // eBmxSync
	tVehicleSyncsDeltas tVehicleDiff;

	inline void SetEqual()
	{
		tVehicleDiff.SetEqual(); // useless
		//nBmxDiff = eBmxSync::MP_PKTD_BMX_EQUAL;
		memset(this, eBmxSync::MP_PKTD_BMX_EQUAL, sizeof(tBmxSyncsDeltas));
	}

	inline void SetDifference()
	{
		tVehicleDiff.SetDifference(); // useless
		//nBmxDiff = eBmxSync::MP_PKTD_BMX_FULL;
		memset(this, static_cast<uint32>(eBmxSync::MP_PKTD_BMX_FULL), sizeof(tBmxSyncsDeltas)); // 0xFFFFFFFF
	}
};
static_assert(sizeof(tBmxSyncsDeltas) == 32, "sizeof(tBmxSyncsDeltas)");
#pragma pack(pop)

class cBmxMG : public cVehicleMG {
public:

	cBmxMG(sElement* elem);

	~cBmxMG() override;
	void PreRender(void) override;
	void Render(void) override;
};


struct sBmx : sVehicle {
public:
	RwFrame* m_aBmxNodes[MAX_BIKE_NODES];
	float m_aSuspensionSpringRatio[4];

	sBmx();
	sBmx(CBmx* pBmx);

	static ElementCapability Capability() { return reinterpret_cast<ElementCapability>(sBmx::Capability); }

	ElementCapability GetCapability() override;
	bool HasCapability(ElementCapability capability) override;
	~sBmx() override;
	sElementSync* CreateSync() override;
	void DisposeSync(sElementSync* pSync) override;
	sElementSync* CreateSyncFromOther(sElementSync* pSync) override;
	bool HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) override;
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_BMX; }
	void ApplyClientSync(uint16 nTime) override;
	void Update(uint16 nTime) override;
	bool WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) override;
	void ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) override;
	void ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) override;
	void Initialise() override;
	void SetupModel() override;

	void CompareSyncState(sBmxSync* pSync, sBmxSync* pLastSync, uint32 nDelta, tBmxSyncsDeltas* pDiff);
	void PerformWriteSync(sWriteSyncStream* pSyncStream, sBmxSync* pSync, tBmxSyncsDeltas* pDiff);
	bool WriteSyncDelta(sWriteSyncStream* pSyncStream, sBmxSync* pSync, sBmxSync* pLastSync, uint32 nDelta);

#ifdef FIX_BUGS
	void Fix();
#endif
};
#endif