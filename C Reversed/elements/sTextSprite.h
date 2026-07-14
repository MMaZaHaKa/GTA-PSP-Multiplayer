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

enum eTextSpriteSync
{
	MP_PKTD_TXT_SPR_EQUAL      = 0,
	MP_PKTD_TXT_SPR_BASE       = BIT(0), // guessed, like netmeter in vcs
	MP_PKTD_TXT_SPR_TEXT       = BIT(1), // m_sText
	MP_PKTD_TXT_SPR_STYLE      = BIT(2), // m_nStyle
	MP_PKTD_TXT_SPR_SCALE      = BIT(3), // m_scale
	MP_PKTD_TXT_SPR_WRAP_X     = BIT(4), // m_nWrapX
	MP_PKTD_TXT_SPR_POS_SCALED = BIT(5), // m_posScaled
	MP_PKTD_TXT_SPR_POS        = BIT(6), // m_pos
	MP_PKTD_TXT_SPR_ALIGN      = BIT(7), // m_eAlign
	MP_PKTD_TXT_SPR_FLASH      = BIT(8), // m_nFlash
	MP_PKTD_TXT_SPR_FULL       = -1,	 // full diff
};

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
	int32 m_nOrder;
#endif
	base::string m_sText;
	uint8 m_nStyle;
	//uint8 field_29[3]; // pad
	CVector2D m_scale;
	int32 m_nWrapX;
	uint8 m_eAlign; // vcs 0, 1, 2
	//uint8 field_39[3]; // pad
	CVector2D m_posScaled;
	CVector2D m_pos;
	int32 m_nFlash;
	bool m_bCanShow;
	//uint8 field_51[3]; // pad

	sTextSpriteSync();
	sTextSpriteSync(uint8 align, float posX, float posY);
	sTextSpriteSync(const sTextSpriteSync& other); // inlined
	~sTextSpriteSync() override;

	bool Compare(const sTextSpriteSync& other);
	void UpdateDelta(uint16 nTimeDelta);
	void Print();

#if !defined(FINAL) && !defined(MASTER)
	void Dump();
#endif

#ifdef MULTIGAME_ELEMENTS_IMPROVEMENTS
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_TEXT; }
#endif
};

#pragma pack(push, 1)
struct tTextSpriteSyncsDeltas
{
	uint16 nTextSpriteDiff; // eTextSpriteSync
	uint16 nBaseSpriteDiff; // eSpriteBaseSync

	inline void SetEqualTextSprite()
	{
		memset(this, eTextSpriteSync::MP_PKTD_TXT_SPR_EQUAL, sizeof(tTextSpriteSyncsDeltas));
		//nTextSpriteDiff = eTextSpriteSync::MP_PKTD_TXT_SPR_EQUAL;
		//nBaseSpriteDiff = eSpriteBaseSync::MP_PKTD_SPR_BASE_EQUAL;
	}

	inline void SetDifferenceTextSprite()
	{
		memset(this, static_cast<uint32>(eTextSpriteSync::MP_PKTD_TXT_SPR_FULL), sizeof(tTextSpriteSyncsDeltas)); // 0xFFFFFFFF
		//nTextSpriteDiff = eTextSpriteSync::MP_PKTD_TXT_SPR_FULL;
		//nBaseSpriteDiff = eSpriteBaseSync::MP_PKTD_SPR_BASE_FULL;
	}
};
static_assert(sizeof(tTextSpriteSyncsDeltas) == 4, "sizeof(tTextSpriteSyncsDeltas)");
#pragma pack(pop)

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
	void ApplyClientSync(uint16 nTime) override;
	bool WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) override;
	void ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) override;
	void UpdateDelta(sElementSync* pSync, uint16 nTimeDelta) override;
	void OnHudPrint() override;


	void CompareSyncState(sTextSpriteSync* pSync, sTextSpriteSync* pLastSync, tTextSpriteSyncsDeltas* pDiff);
	void PerformWriteSync(sWriteSyncStream* pSyncStream, sTextSpriteSync* pSync, tTextSpriteSyncsDeltas diff); // not pDiff kek
	bool WriteSyncDelta(sWriteSyncStream* pSyncStream, sTextSpriteSync* pSync, sTextSpriteSync* pLastSync);

#ifdef GTA_LIBERTY
	CRGBA* GetColour();
	void SetColour(CRGBA value);
	int32 GetOrder();
	void SetOrder(int32 value);
#endif
	int32 GetFlash();
	void SetFlash(int32 value);
	CVector2D* GetScale();
	void SetScale(float x, float y);
	uint8 GetStyle();
	void SetStyle(uint8 nStyle);
	int32 GetWrapX();
	void SetWrapX(int32 nWrapX);
	uint8 GetAlign();
	void SetAlign(uint8 eAlign);
	base::string* GetText();
	void SetText(base::string& text);
	void SetPos(int32 x, int32 y, float scale);
	CVector2D* GetPos();
	
#ifdef GTA_LIBERTY
	static void Terminate();
	static void UpdateAll();
#endif
};

#ifdef GTA_LIBERTY
void mp_update_sprites();
#endif
