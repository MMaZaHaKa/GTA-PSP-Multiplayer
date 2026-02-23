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
// 0x2C 44 2len + 4mask  https://prnt.sc/Kd3hUh5c4nWB
struct sNetMeter2dSync : public sSpriteBaseSync
{
public:
	uint8 m_Alpha;
	float m_fFillRatio;
	bool m_bIsFlashing;
	bool m_bIsUseTitle;
	base::string m_sTitleKey;

	sNetMeter2dSync();
	sNetMeter2dSync(float posX, float posY, float width, float height);
	sNetMeter2dSync(const sNetMeter2dSync& other);
	~sNetMeter2dSync() override;
};


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