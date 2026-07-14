/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "common.h"

#include "leeds/base/stringt.h"
#include "multiplayer/elements/sElement.h"
#include <vector>


#ifndef GTA_LIBERTY

enum eSpriteBaseSync
{
	MP_PKTD_SPR_BASE_EQUAL  = 0,
	MP_PKTD_SPR_BASE_POS    = BIT(0), // m_posOld
	MP_PKTD_SPR_BASE_SIZE_X = BIT(1), // m_Size.x
	MP_PKTD_SPR_BASE_SIZE_Y = BIT(2), // m_Size.y
	MP_PKTD_SPR_BASE_COLOR  = BIT(3), // m_color
	MP_PKTD_SPR_BASE_ORDER  = BIT(4), // m_nOrder
	MP_PKTD_SPR_BASE_FULL   = -1,	  // full diff
};

struct sSpriteBaseSync : sElementSync {
public:
	CRGBA m_color;
	CVector2D m_posOld;
	CVector2D m_Size;
	float m_fOrder; // real float

	sSpriteBaseSync();
	sSpriteBaseSync(float posX, float posY, float width, float height);
	sSpriteBaseSync(const sSpriteBaseSync& other);
	~sSpriteBaseSync() override;

#ifdef MULTIGAME_ELEMENTS_IMPROVEMENTS
	bool Compare(const sSpriteBaseSync& other);
#endif
#if !defined(FINAL) && !defined(MASTER)
	void Dump();
#endif
};

struct sSpriteBase : sElement {
public:
	static std::vector<sSpriteBase*> ms_vItems;

	static ElementCapability Capability() { return reinterpret_cast<ElementCapability>(sSpriteBase::Capability); }

	sSpriteBase();

	ElementCapability GetCapability() override;
	bool HasCapability(ElementCapability capability) override;
	~sSpriteBase() override;
	void ApplyClientSync(uint16 time) override;
	void ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) override;
	virtual void OnHudPrint() = 0;

	static void Terminate();
	static void UpdateAll();


	CRGBA* GetColour();
	void SetColour(CRGBA colour);
	float GetOrder();
	void SetOrder(float fOrder);

	void CompareSyncState(sSpriteBaseSync* pSync, sSpriteBaseSync* pLastSync, uint16* pDiffMask);
	void PerformWriteSync(sWriteSyncStream* pSyncStream, sSpriteBaseSync* pSync, uint16 nDiffMask);

#ifdef MULTIGAME_ELEMENTS_IMPROVEMENTS
	CVector2D* GetPosOld();
	void SetPosOld(CVector2D pos);
	CVector2D* GetSize();
	void SetSize(CVector2D size);
#endif
};

void mp_update_sprites();
#endif
