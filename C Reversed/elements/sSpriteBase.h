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
struct sSpriteBaseSync : sElementSync {
public:
	CRGBA m_color;
	CVector2D m_BasePos; // rename todo m_PosOld
	CVector2D m_Size;
	float m_fOrder; // kek real float, asm as f0 reg

	sSpriteBaseSync();
	sSpriteBaseSync(float posX, float posY, float width, float height);
	sSpriteBaseSync(const sSpriteBaseSync& other);
	~sSpriteBaseSync() override;

	bool Compare(const sSpriteBaseSync& other);
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
	void SetColour(CRGBA value);
	void SetPos(int x, int y, float scale);
	CVector2D* GetPos();
	void SetPos(CVector2D pos);
	CVector2D* GetSize();
	void SetSize(CVector2D size);
	float GetOrder();
	void SetOrder(float fValue);
};

void mp_update_sprites();
#endif
