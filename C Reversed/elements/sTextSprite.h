/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "common.h"

#include "leeds/base/stringt.h"
#ifdef GTA_LIBERTY
#include "multiplayer/elements/sElement.h"
#else
#include "multiplayer/elements/sSpriteBase.h"
#endif
#include <vector>


#ifdef GTA_LIBERTY
struct sTextSpriteSync : sElementSync
#else
struct sTextSpriteSync : sSpriteBaseSync
#endif
{
public:
#ifdef GTA_LIBERTY
	CRGBA m_color;
	CVector2D m_posOld;
	int m_nOrder;
#endif
	base::string m_sText;
	uint8 m_nStyle;
	CVector2D m_scale;
	int m_nWrapX;
	uint8 m_eAlign; // vcs 0, 1, 2
	CVector2D m_posScale;
	CVector2D m_pos;
	int m_nFlash;
	bool m_bCanShow;

	sTextSpriteSync();
	sTextSpriteSync(uint8 align, float posX, float posY);
	// TODO dtor + copy from !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO dtor + copy from !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO dtor + copy from !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	void Print();
};

#ifdef GTA_LIBERTY
struct sTextSprite : sElement
#else
struct sTextSprite : sSpriteBase
#endif
{
#ifdef GTA_LIBERTY
private:
	static std::vector<sTextSprite*> ms_vItems;
#endif
public:

	sTextSprite();
	sTextSprite(int32 nPeerID, uint8 align, float x, float y);

	static ElementCapability Capability() { return reinterpret_cast<ElementCapability>(sTextSprite::Capability); }

	ElementCapability GetCapability() override;
	bool HasCapability(ElementCapability capability) override;
	~sTextSprite() override;
	sElementSync* CreateSync() override;
	void DisposeSync(sElementSync* pSync) override;
	sElementSync* CreateSyncFromOther(sElementSync* pSync) override;
	bool HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) override;
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_TEXT; }
	void ApplyClientSync(uint16 time) override;
	bool WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) override;
	void ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) override;
	void UpdateDelta(sElementSync* pSync, uint16 nTimeDelta) override;
	void OnHudPrint() override;


#ifdef GTA_LIBERTY
	CRGBA* GetColour();
	void SetColour(CRGBA value);
	int GetOrder();
	void SetOrder(int value);
#endif
	int GetFlash();
	void SetFlash(int value);
	CVector2D* GetScale();
	void SetScale(float x, float y);
	uint8 GetStyle();
	void SetStyle(uint8 value);
	int GetWrapX();
	void SetWrapX(int value);
	uint8 GetAlign();
	void SetAlign(uint8 value);
	base::string* GetText();
	void SetText(base::string& value);
	void SetPos(int x, int y, float scale);
	CVector2D* GetPos();
	
#ifdef GTA_LIBERTY
	static void Terminate();
	static void UpdateAll();
#endif
};

#ifdef GTA_LIBERTY
void mp_update_sprites();
#endif
