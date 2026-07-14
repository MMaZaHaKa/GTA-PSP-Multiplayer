/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "common.h"

#include "leeds/base/stringt.h"
#include "multiplayer/elements/sSpriteBase.h"
#include "Hud.h"

#ifndef GTA_LIBERTY

enum eNetMeter2dSync
{
	MP_PKTD_NETMTR_EQUAL = 0,
	MP_PKTD_NETMTR_BASE         = BIT(0), // base
	MP_PKTD_NETMTR_ALPHA        = BIT(1), // m_Alpha
	MP_PKTD_NETMTR_FILL_RATIO   = BIT(2), // m_fFillRatio
	MP_PKTD_NETMTR_IS_FLASHING  = BIT(3), // m_bIsFlashing
	MP_PKTD_NETMTR_IS_USE_TITLE = BIT(4), // m_bIsUseTitle
	MP_PKTD_NETMTR_TITLE_KEY    = BIT(5), // m_sTitleKey
	MP_PKTD_NETMTR_FULL         = -1,	  // full diff
};

struct sNetMeter2dSync : public sSpriteBaseSync
{
public:
	uint8 m_Alpha;
	//uint8 field_21[3]; // pad
	float m_fFillRatio;
	bool m_bIsFlashing;
	bool m_bIsUseTitle;
	//uint8 field_2A[2]; // pad
	base::string m_sTitleKey;

	sNetMeter2dSync();
	sNetMeter2dSync(float posX, float posY, float width, float height);
	sNetMeter2dSync(const sNetMeter2dSync& other);
	~sNetMeter2dSync() override;

	bool Compare(const sNetMeter2dSync& other);

#if !defined(FINAL) && !defined(MASTER)
	void Dump();
#endif

#ifdef MULTIGAME_ELEMENTS_IMPROVEMENTS
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_NETMETER2D; }
#endif
};

#pragma pack(push, 1)
struct tNetMeter2dSyncsDeltas
{
	uint16 nNetMeterDiff;   // eNetMeter2dSync
	uint16 nBaseSpriteDiff; // eSpriteBaseSync

	inline void SetEqualNetMeter()
	{
		memset(this, eNetMeter2dSync::MP_PKTD_NETMTR_EQUAL, sizeof(tNetMeter2dSyncsDeltas));
		//nNetMeterDiff = eNetMeter2dSync::MP_PKTD_NETMTR_EQUAL;
		//nBaseSpriteDiff = eSpriteBaseSync::MP_PKTD_SPR_BASE_EQUAL;
	}

	inline void SetDifferenceNetMeter()
	{
		memset(this, static_cast<uint32>(eNetMeter2dSync::MP_PKTD_NETMTR_FULL), sizeof(tNetMeter2dSyncsDeltas)); // 0xFFFFFFFF
		//nNetMeterDiff = eNetMeter2dSync::MP_PKTD_NETMTR_FULL;
		//nBaseSpriteDiff = eSpriteBaseSync::MP_PKTD_SPR_BASE_FULL;
	}
};
static_assert(sizeof(tNetMeter2dSyncsDeltas) == 4, "sizeof(tNetMeter2dSyncsDeltas)");
#pragma pack(pop)


struct sNetMeter2d : sSpriteBase // (VIP, HUNTERATTACK) bar
{
public:
	cHudBar m_HudBar;

	sNetMeter2d();
	sNetMeter2d(int32 nPeerID, float posX, float posY, float width, float height);

	static ElementCapability Capability() { return reinterpret_cast<ElementCapability>(sNetMeter2d::Capability); }

	ElementCapability GetCapability() override;
	bool HasCapability(ElementCapability capability) override;
	~sNetMeter2d() override;
	sElementSync* CreateSync() override;
	void DisposeSync(sElementSync* pSync) override;
	sElementSync* CreateSyncFromOther(sElementSync* pSync) override;
	bool HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) override;
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_NETMETER2D; }
	void ApplyClientSync(uint16 time) override;
	bool WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) override;
	void ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) override;
	void OnHudPrint() override;


	void CompareSyncState(sNetMeter2dSync* pSync, sNetMeter2dSync* pLastSync, tNetMeter2dSyncsDeltas* pDiff);
	void PerformWriteSync(sWriteSyncStream* pSyncStream, sNetMeter2dSync* pSync, tNetMeter2dSyncsDeltas diff); // not pDiff kek
	bool WriteSyncDelta(sWriteSyncStream* pSyncStream, sNetMeter2dSync* pSync, sNetMeter2dSync* pLastSync);

	uint8 GetAlpha();
	void SetAlpha(uint8 alpha);
	float GetFillRatio();
	void SetFillRatio(float fillRatio);
	bool GetFlashing();
	void SetFlashing(bool flashing);
	bool GetUseTitle();
	void SetUseTitle(bool isUse);
	void SetTitleKey(const char* titleKey);
};
#endif