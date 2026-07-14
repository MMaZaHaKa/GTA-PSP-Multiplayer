/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "multiplayer/elements/sRadarBlip.h"
#include "multiplayer/elements/sSyncStream.h"
#include "MultiGame.h"

sRadarBlipSync::sRadarBlipSync() : sElementSync()
{
	DECLARE_SYNC_CONSTRUCT(this);
	memset(&m_nTrace, 0, sizeof(m_nTrace));
	m_nTrace.m_nPlayerMaskMG = -1;
}

sRadarBlipSync::sRadarBlipSync(sRadarTrace* blipTrace) : sElementSync()
{
	DECLARE_SYNC_CONSTRUCT(this);
	m_nTrace.m_nColor = blipTrace->m_nColor;
	m_nTrace.m_eBlipType = blipTrace->m_eBlipType;
	m_nTrace.m_nEntityHandle = blipTrace->m_nEntityHandle;
	m_nTrace.m_vecPos = blipTrace->m_vecPos;
	// Flags
	m_nTrace.bMultiplayerState = blipTrace->bMultiplayerState;
	m_nTrace.bDim = blipTrace->bDim;
	m_nTrace.bInUse = blipTrace->bInUse;
	m_nTrace.bShortRange = blipTrace->bShortRange;
	m_nTrace.bUnused = blipTrace->bUnused;
	m_nTrace.eBlipDisplay = blipTrace->eBlipDisplay;

	m_nTrace.m_Radius = blipTrace->m_Radius;
	m_nTrace.m_Scale = blipTrace->m_Scale;
	m_nTrace.m_eRadarSprite = blipTrace->m_eRadarSprite;
	m_nTrace.m_BlipIndex = blipTrace->m_BlipIndex;
	m_nTrace.m_nPlayerMaskMG = blipTrace->m_nPlayerMaskMG;
	m_nTrace.bMultiplayerState = false;
}

sRadarBlipSync::sRadarBlipSync(const sRadarBlipSync& other) : sElementSync(other)
{
	DECLARE_SYNC_CONSTRUCT(this);
	m_nTrace.m_nColor = other.m_nTrace.m_nColor;
	m_nTrace.m_eBlipType = other.m_nTrace.m_eBlipType;
	m_nTrace.m_nEntityHandle = other.m_nTrace.m_nEntityHandle;
	m_nTrace.m_vecPos = other.m_nTrace.m_vecPos;
	// Flags
	m_nTrace.bMultiplayerState = other.m_nTrace.bMultiplayerState;
	m_nTrace.bDim = other.m_nTrace.bDim;
	m_nTrace.bInUse = other.m_nTrace.bInUse;
	m_nTrace.bShortRange = other.m_nTrace.bShortRange;
	m_nTrace.bUnused = other.m_nTrace.bUnused;
	m_nTrace.eBlipDisplay = other.m_nTrace.eBlipDisplay;

	m_nTrace.m_Radius = other.m_nTrace.m_Radius;
	m_nTrace.m_Scale = other.m_nTrace.m_Scale;
	m_nTrace.m_eRadarSprite = other.m_nTrace.m_eRadarSprite;
	m_nTrace.m_BlipIndex = other.m_nTrace.m_BlipIndex;
	m_nTrace.m_nPlayerMaskMG = other.m_nTrace.m_nPlayerMaskMG;
}

sRadarBlipSync::~sRadarBlipSync() {
	DECLARE_SYNC_DESTRUCT(this);
}

// not checks: bMultiplayerState
bool sRadarBlipSync::Compare(const sRadarBlipSync& other)
{
	if (m_nTrace.m_nColor != other.m_nTrace.m_nColor)
		return false;
	if (m_nTrace.m_eBlipType != other.m_nTrace.m_eBlipType)
		return false;
	if (m_nTrace.m_nEntityHandle != other.m_nTrace.m_nEntityHandle)
		return false;
	if (m_nTrace.m_vecPos != other.m_nTrace.m_vecPos)
		return false;
	if (m_nTrace.m_BlipIndex != other.m_nTrace.m_BlipIndex)
		return false;

	// Flags
	if (m_nTrace.bDim != other.m_nTrace.bDim)
		return false;
	if (m_nTrace.bInUse != other.m_nTrace.bInUse)
		return false;
	if (m_nTrace.bShortRange != other.m_nTrace.bShortRange)
		return false;
	if (m_nTrace.bUnused != other.m_nTrace.bUnused)
		return false;

	if (m_nTrace.m_Radius != other.m_nTrace.m_Radius)
		return false;
	if (m_nTrace.m_Scale != other.m_nTrace.m_Scale)
		return false;
	if (m_nTrace.eBlipDisplay != other.m_nTrace.eBlipDisplay)
		return false;
	if (m_nTrace.m_eRadarSprite != other.m_nTrace.m_eRadarSprite)
		return false;
	if (m_nTrace.m_nPlayerMaskMG != other.m_nTrace.m_nPlayerMaskMG)
		return false;

	return true;
}

#if !defined(FINAL) && !defined(MASTER)
void sRadarBlipSync::Dump()
{
	sElementSync::Dump();

	printf("=== sRadarBlipSync Dump ===\n");
	printf("Blip Type:       %u (0x%08X)\n", m_nTrace.m_eBlipType, m_nTrace.m_eBlipType);
	printf("Color:           %u (0x%08X)\n", m_nTrace.m_nColor, m_nTrace.m_nColor);
	printf("Entity Handle:   %d (0x%08X)\n", m_nTrace.m_nEntityHandle, m_nTrace.m_nEntityHandle);
	printf("Position:        (%.3f, %.3f, %.3f)\n", m_nTrace.m_vecPos.x, m_nTrace.m_vecPos.y, m_nTrace.m_vecPos.z);
	printf("Radius:          %.3f\n", m_nTrace.m_Radius);
	printf("Scale:           %d\n", m_nTrace.m_Scale);
	printf("Radar Sprite:    %u\n", m_nTrace.m_eRadarSprite);
	printf("Blip Index:      %u\n", m_nTrace.m_BlipIndex);
	printf("Player Mask MG:  0x%02X\n", m_nTrace.m_nPlayerMaskMG);
	printf("Flags:\n");
	printf("  bMultiplayerState: %d\n", m_nTrace.bMultiplayerState);
	printf("  bDim:              %d\n", m_nTrace.bDim);
	printf("  bInUse:            %d\n", m_nTrace.bInUse);
	printf("  bShortRange:       %d\n", m_nTrace.bShortRange);
	printf("  bUnused:           %d\n", m_nTrace.bUnused);
	printf("  eBlipDisplay:      %d\n", m_nTrace.eBlipDisplay);
	printf("================================\n");
}
#endif


sRadarBlip::sRadarBlip() {
	DECLARE_ELEMENT_CONSTRUCT(this, true, false);
	m_nBlipIndex = -1;
	DECLARE_ELEMENT_CONSTRUCT(this, false, false);
}

sRadarBlip::sRadarBlip(int32 nBlipIndex) {
	DECLARE_ELEMENT_CONSTRUCT(this, true, true);
	m_nBlipIndex = TheRadar->GetActualBlipArrayIndex(nBlipIndex);
	RegisterSelf();
	AttachSync(m_nTime, new sRadarBlipSync(&TheRadar->ms_RadarTrace[m_nBlipIndex]));
	DECLARE_ELEMENT_CONSTRUCT(this, false, true);
}


ElementCapability sRadarBlip::GetCapability()
{
	return sRadarBlip::Capability();
}

bool sRadarBlip::HasCapability(ElementCapability capability)
{
	if (sRadarBlip::Capability() == capability)
		return true;
	if (sElement::Capability() == capability)
		return true;
	return false;
}

sRadarBlip::~sRadarBlip()
{
	DECLARE_ELEMENT_DESTRUCT(this);
	ClearBlip();

	sElement::PurgeAttached();
	// ~sElement
}

sElementSync* sRadarBlip::CreateSync()
{
	return new sRadarBlipSync();
}

void sRadarBlip::DisposeSync(sElementSync* pSync)
{
	if(pSync)
		delete ((sRadarBlipSync*)pSync);
}

sElementSync* sRadarBlip::CreateSyncFromOther(sElementSync* pSync)
{
	sRadarBlipSync& sync = *(sRadarBlipSync*)pSync;
	return new sRadarBlipSync(sync);
}

bool sRadarBlip::HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB)
{
	sRadarBlipSync& syncA = *(sRadarBlipSync*)pSyncA;
	sRadarBlipSync& syncB = *(sRadarBlipSync*)pSyncB;
	return syncA.Compare(syncB);
}

void sRadarBlip::ApplyClientSync(uint16 nTime)
{
	sElement::ApplyClientSync(nTime);
	if (cMultiGame::Instance().IsElementOwnerLocalPlayer(this))
		return;

	sRadarBlipSync* pSync = FindSync(nTime, nil).radarblip;
	if (pSync->m_nTrace.bInUse)
	{
		if (m_nBlipIndex == -1)
		{
			// Find Free index
			for (m_nBlipIndex = 0; m_nBlipIndex < NUMRADARBLIPS; m_nBlipIndex++)
				if (!TheRadar->ms_RadarTrace[m_nBlipIndex].bInUse)
					break;
			assert(m_nBlipIndex < NUMRADARBLIPS);
		}
		sRadarTrace& trace = TheRadar->ms_RadarTrace[m_nBlipIndex];

		trace.m_nColor = pSync->m_nTrace.m_nColor;
		trace.m_eBlipType = pSync->m_nTrace.m_eBlipType;
		trace.m_nEntityHandle = pSync->m_nTrace.m_nEntityHandle;
		trace.m_vecPos = pSync->m_nTrace.m_vecPos;

		// Flags
		trace.bMultiplayerState = pSync->m_nTrace.bMultiplayerState;
		trace.bDim = pSync->m_nTrace.bDim;
		trace.bInUse = pSync->m_nTrace.bInUse;
		trace.bShortRange = pSync->m_nTrace.bShortRange;
		trace.bUnused = pSync->m_nTrace.bUnused;
		trace.eBlipDisplay = pSync->m_nTrace.eBlipDisplay;

		trace.m_Radius = pSync->m_nTrace.m_Radius;
		trace.m_Scale = pSync->m_nTrace.m_Scale;
		trace.m_eRadarSprite = pSync->m_nTrace.m_eRadarSprite;
		trace.m_BlipIndex = pSync->m_nTrace.m_BlipIndex;
		trace.m_nPlayerMaskMG = pSync->m_nTrace.m_nPlayerMaskMG;
	}
	else if (m_nBlipIndex != -1)
		ClearBlip();
}

void sRadarBlip::Update(uint16 nTime)
{
	AttachSync(nTime, new sRadarBlipSync(&TheRadar->ms_RadarTrace[m_nBlipIndex]));
}

bool sRadarBlip::WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime)
{
	sRadarBlipSync* pSync = FindSync(nSyncWriteTime, nil).radarblip;
	if (static_cast<int16>(nSyncLastTime - m_vSync.front().m_nTime) >= 0)
		return WriteSyncDelta(pSyncStream, pSync, GetSyncWithTime(nSyncLastTime).radarblip);

	PerformWriteSync(pSyncStream, pSync, eRadarBlipSyncDelta::MP_PKTD_RADARBLIP_FULL);
	return true;
}

void sRadarBlip::ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync)
{
	sRadarBlipSync& sync = *(sRadarBlipSync*)pOutSync;

	uint8 nDiffMask = pSyncStream->ReadU8();
	sync.m_nTrace.m_nFlags = pSyncStream->ReadU8();

	if (nDiffMask & eRadarBlipSyncDelta::MP_PKTD_RADARBLIP_POSITION) 
	{
		sync.m_nTrace.m_nColor = pSyncStream->ReadU32();
		sync.m_nTrace.m_eBlipType = static_cast<uint32>(pSyncStream->ReadU8());
		sync.m_nTrace.m_nEntityHandle = pSyncStream->ReadI32();
		sync.m_nTrace.m_vecPos = pSyncStream->ReadVector();
		sync.m_nTrace.m_Scale = pSyncStream->ReadI8();
		sync.m_nTrace.m_eRadarSprite = pSyncStream->ReadU8();
	}

	if (nDiffMask & eRadarBlipSyncDelta::MP_PKTD_RADARBLIP_RADIUS)
		sync.m_nTrace.m_Radius = pSyncStream->ReadFloat();

	if (nDiffMask & eRadarBlipSyncDelta::MP_PKTD_RADARBLIP_PLAYERMASK)
		sync.m_nTrace.m_nPlayerMaskMG = pSyncStream->ReadU8();
}

inline uint8 sRadarBlip::CompareSyncState(sRadarBlipSync* pSync, sRadarBlipSync* pLastSync) {
	uint8 nDiffMask = eRadarBlipSyncDelta::MP_PKTD_RADARBLIP_EQUAL;

	if (pSync->m_nTrace.m_vecPos != pLastSync->m_nTrace.m_vecPos)
		nDiffMask |= eRadarBlipSyncDelta::MP_PKTD_RADARBLIP_POSITION;
	if (pSync->m_nTrace.m_Radius != pLastSync->m_nTrace.m_Radius)
		nDiffMask |= eRadarBlipSyncDelta::MP_PKTD_RADARBLIP_RADIUS;
	if (pSync->m_nTrace.m_nPlayerMaskMG != pLastSync->m_nTrace.m_nPlayerMaskMG)
		nDiffMask |= eRadarBlipSyncDelta::MP_PKTD_RADARBLIP_PLAYERMASK;

	return nDiffMask;
}

bool sRadarBlip::WriteSyncDelta(sWriteSyncStream* pSyncStream, sRadarBlipSync* pSync, sRadarBlipSync* pLastSync) {
	uint8 nDiffMask = CompareSyncState(pSync, pLastSync);

	if (nDiffMask == eRadarBlipSyncDelta::MP_PKTD_RADARBLIP_EQUAL)
		return false;

	PerformWriteSync(pSyncStream, pSync, nDiffMask);
	return true;
}

void sRadarBlip::PerformWriteSync(sWriteSyncStream* pSyncStream, sRadarBlipSync* pSync, uint8 nDiffMask) {
	pSyncStream->WriteU8(nDiffMask);

	pSyncStream->WriteU8(pSync->m_nTrace.m_nFlags);

	if (nDiffMask & eRadarBlipSyncDelta::MP_PKTD_RADARBLIP_POSITION)
	{
		pSyncStream->WriteU32(pSync->m_nTrace.m_nColor);
		pSyncStream->WriteU8(static_cast<uint8>(pSync->m_nTrace.m_eBlipType));
		pSyncStream->WriteI32(pSync->m_nTrace.m_nEntityHandle);
		pSyncStream->WriteVector(pSync->m_nTrace.m_vecPos);
		pSyncStream->WriteI8(pSync->m_nTrace.m_Scale);
		pSyncStream->WriteU8(pSync->m_nTrace.m_eRadarSprite);
	}

	if (nDiffMask & eRadarBlipSyncDelta::MP_PKTD_RADARBLIP_RADIUS)
		pSyncStream->WriteFloat(pSync->m_nTrace.m_Radius);

	if (nDiffMask & eRadarBlipSyncDelta::MP_PKTD_RADARBLIP_PLAYERMASK)
		pSyncStream->WriteU8(pSync->m_nTrace.m_nPlayerMaskMG);
}


void sRadarBlip::ClearBlip()
{
	if (m_nBlipIndex == -1)
		return;

#ifdef FIX_BUGS
	TheRadar->ms_RadarTrace[m_nBlipIndex].bMultiplayerState = false;
#endif
	TheRadar->ms_RadarTrace[m_nBlipIndex].bInUse = false;
	TheRadar->ms_RadarTrace[m_nBlipIndex].m_eBlipType = eBlipType::BLIP_NONE;
	TheRadar->ms_RadarTrace[m_nBlipIndex].eBlipDisplay = eBlipDisplay::BLIP_DISPLAY_NEITHER;
	TheRadar->ms_RadarTrace[m_nBlipIndex].bUnused = false;
	TheRadar->ms_RadarTrace[m_nBlipIndex].m_nPlayerMaskMG = 0xFF; //?
	TheRadar->ms_RadarTrace[m_nBlipIndex].m_eRadarSprite = eRadarSprite::RADAR_SPRITE_NONE;
	m_nBlipIndex = -1;
}