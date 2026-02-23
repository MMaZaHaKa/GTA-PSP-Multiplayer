/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "common.h"
#include "Radar.h"

#include "multiplayer/elements/sElement.h"

enum eRadarBlipSyncDelta
{
	MP_PKTD_RADARBLIP_EQUAL      = 0,
	MP_PKTD_RADARBLIP_POSITION   = BIT(0), // m_vecPos
	MP_PKTD_RADARBLIP_RADIUS     = BIT(1), // m_Radius
	MP_PKTD_RADARBLIP_PLAYERMASK = BIT(2), // m_nPlayerMaskMG
	MP_PKTD_RADARBLIP_FULL       = -1,     // full diff
};


struct sRadarBlipSync : sElementSync {
public:
	sRadarTrace m_nTrace;

	sRadarBlipSync();
	sRadarBlipSync(sRadarTrace* blipTrace);
	sRadarBlipSync(const sRadarBlipSync& other); // inlined
	~sRadarBlipSync() override;

	bool Compare(const sRadarBlipSync& other);
#if !defined(FINAL) && !defined(MASTER)
	void Dump();
#endif
};

struct sRadarBlip : sElement {
public:
	int32 m_nBlipIndex; // array idx for CRadar::ms_RadarTrace

	sRadarBlip();
	sRadarBlip(int32 nBlipIndex);

	static ElementCapability Capability() { return reinterpret_cast<ElementCapability>(sRadarBlip::Capability); }

	ElementCapability GetCapability() override;
	bool HasCapability(ElementCapability capability) override;
	~sRadarBlip() override;
	sElementSync* CreateSync() override;
	void DisposeSync(sElementSync* pSync) override;
	sElementSync* CreateSyncFromOther(sElementSync* pSync) override;
	bool HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) override;
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_RADAR_BLIP; }
	void ApplyClientSync(uint16 time) override;
	void Update(uint16 time) override;
	bool WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) override;
	void ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) override;

	uint8 CompareSyncState(sRadarBlipSync* pSync, sRadarBlipSync* pLastSync); // inline
	bool WriteSyncDelta(sWriteSyncStream* pSyncStream, sRadarBlipSync* pSync, sRadarBlipSync* pLastSync);
	void PerformWriteSync(sWriteSyncStream* pSyncStream, sRadarBlipSync* pSync, uint8 nDiffMask);

	void ClearBlip();
};