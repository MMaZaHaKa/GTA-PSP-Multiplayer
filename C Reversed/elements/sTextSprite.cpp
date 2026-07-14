/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"
#include "Font.h"
#include "Text.h"

#include "multiplayer/public.h"
#include "multiplayer/elements/sSyncStream.h"
#include "multiplayer/elements/sTextSprite.h"
#include "multiplayer/MultiGame.h"

#ifndef GTA_LIBERTY
sTextSpriteSync::sTextSpriteSync() : sSpriteBaseSync()
{
	DECLARE_SYNC_CONSTRUCT(this);
	m_sText = base::string();
}
#else
sTextSpriteSync::sTextSpriteSync() : sElementSync() {
	DECLARE_SYNC_CONSTRUCT(this);
}
#endif

#ifndef GTA_LIBERTY
sTextSpriteSync::sTextSpriteSync(uint8 align, float posX, float posY) : sSpriteBaseSync(posX, posY, 0.0f, 0.0f)
#else
sTextSpriteSync::sTextSpriteSync(uint8 align, float posX, float posY) : sElementSync()
#endif
{
	DECLARE_SYNC_CONSTRUCT(this);
	m_sText = base::string();
	m_nStyle = FONT_BANK;
	m_scale = CVector2D(0.7f, 0.8f);
#ifdef GTA_LIBERTY
	m_color = CRGBA(255, 255, 255, 255);
	m_nWrapX = 640;
#else
	m_nWrapX = DEFAULT_SCREEN_WIDTH;
#endif
	m_eAlign = align;
#ifdef GTA_LIBERTY
	m_posOld = CVector2D(posX, posY);
#endif
	m_posScaled = CVector2D(0.0f, 0.0f);
	m_pos = CVector2D(posX, posY);
#ifdef GTA_LIBERTY
	m_nOrder = 0;
#endif
	m_nFlash = 0;
	m_bCanShow = true;
}

#ifndef GTA_LIBERTY
sTextSpriteSync::sTextSpriteSync(const sTextSpriteSync& other) : sSpriteBaseSync(other)
#else
sTextSpriteSync::sTextSpriteSync(const sTextSpriteSync& other) : sElementSync(other)
#endif
{
	DECLARE_SYNC_CONSTRUCT(this);
	m_sText = other.m_sText;
	m_nStyle = other.m_nStyle;
	m_scale = other.m_scale;
	m_nWrapX = other.m_nWrapX;
	m_eAlign = other.m_eAlign;
	m_posScaled = other.m_posScaled;
	m_pos = other.m_pos;
	m_nFlash = other.m_nFlash;
	m_bCanShow = other.m_bCanShow;
}

sTextSpriteSync::~sTextSpriteSync() {
	DECLARE_SYNC_DESTRUCT(this);
}

void sTextSpriteSync::UpdateDelta(uint16 nTimeDelta)
{
	float dx = m_posScaled.x;
	float dy = m_posScaled.y;

	m_posOld.x += dx;
	m_posOld.y += dy;

	bool reached =
		(dx < 0.0f && m_posOld.x <= m_pos.x) ||
		(dx > 0.0f && m_posOld.x >= m_pos.x) ||
		(dy < 0.0f && m_posOld.y <= m_pos.y) ||
		(dy > 0.0f && m_posOld.y >= m_pos.y);

	if (reached)
	{
		m_posOld.x = m_pos.x;
		m_posOld.y = m_pos.y;
		m_posScaled.x = 0.0f;
		m_posScaled.y = 0.0f;
	}
}

bool sTextSpriteSync::Compare(const sTextSpriteSync& other)
{
//#ifdef FIX_BUGS // need?
//	if (!sSpriteBaseSync::Compare(other))
//		return false;
//#endif

	if (m_nStyle != other.m_nStyle)
		return false;

	if (m_scale.x != other.m_scale.x)
		return false;

	if (m_scale.y != other.m_scale.y)
		return false;

	if (m_nWrapX != other.m_nWrapX)
		return false;

	if (m_eAlign != other.m_eAlign)
		return false;

	if (m_posScaled.x != other.m_posScaled.x)
		return false;

	if (m_posScaled.y != other.m_posScaled.y)
		return false;

	if (m_pos.x != other.m_pos.x)
		return false;

	if (m_pos.y != other.m_pos.y)
		return false;

	if (m_sText.size() != other.m_sText.size())
		return false;

	if (memcmp(m_sText.c_str(), other.m_sText.c_str(), m_sText.size()) != 0)
		return false;

	if (m_nFlash != other.m_nFlash)
		return false;

	return true;
}

#if !defined(FINAL) && !defined(MASTER)
void sTextSpriteSync::Dump()
{
#ifdef GTA_LIBERTY
	sElementSync::Dump();
#else
	sSpriteBaseSync::Dump();
#endif

	printf("=== sTextSpriteSync Dump ===\n");
#ifdef GTA_LIBERTY
	printf("Text Sprite Info (Liberty):\n");
	printf("  Color: R=%u G=%u B=%u A=%u (0x%08X)\n",
		m_color.red, m_color.green, m_color.blue, m_color.alpha,
		*(uint32*)&m_color);
	printf("  Position Old: X=%.2f Y=%.2f\n", m_posOld.x, m_posOld.y);
	printf("  Order: %d (0x%08X)\n", m_nOrder, m_nOrder);
#else
	printf("Text Sprite Info:\n");
#endif

	printf("  Text: \"%s\"\n", m_sText.c_str());
	printf("  Style: %u (0x%02X)\n", m_nStyle, m_nStyle);
	printf("  Scale: X=%.2f Y=%.2f\n", m_scale.x, m_scale.y);
	printf("  WrapX: %d (0x%08X)\n", m_nWrapX, m_nWrapX);
	printf("  Align: %u (0x%02X) [0=Left,1=Center,2=Right]\n", m_eAlign, m_eAlign);
	printf("  Position Scaled: X=%.2f Y=%.2f\n", m_posScaled.x, m_posScaled.y);
	printf("  Position: X=%.2f Y=%.2f\n", m_pos.x, m_pos.y);
	printf("  Flash: %d (0x%08X)\n", m_nFlash, m_nFlash);
	printf("  CanShow: %s\n", m_bCanShow ? "true" : "false");
	printf("================================\n");
}
#endif


void inline append_translated_text(base::string& dest, const base::string& key) {
	dest.Append(UnicodeToAscii(TheText.Get(key.c_str())));
}

// todo? pos after token
size_t next_token(base::string& format, const char* token, const size_t& startPos) {
	size_t pos = format.Find(token, startPos); // -1
	return pos != base::string::npos ? pos : format.size();
}

// ?
//size_t next_token(base::string& format, const char* token, const size_t& startPos) {
//	size_t pos = format.Find(token, startPos);
//	if (pos != base::string::npos)
//		return pos + strlen(token);
//	return format.size();
//}

/* TODO: stub */
void format_text(base::string& dest, base::string& format) {
	size_t found = format.Find("^"); // -1
	if (found == base::string::npos) {
		append_translated_text(dest, format);
		return;
	}
	if (found > 0) append_translated_text(dest, format.SubStr(0, found));
	for (size_t index = found + 1; index < format.size(); index++) {
		const char& token = format.At(index);
		if (token == 'T' || token == 't') {
			const size_t keyStart = index + 2;
			size_t keyEnd = next_token(format, "^", keyStart);
			const base::string& key = format.SubStr(keyStart, keyEnd - keyStart);
			append_translated_text(dest, key);
			index = keyEnd;
		}
		else if (token == 'S' || token == 's') {
			const size_t textStart = index + 2;
			size_t textEnd = next_token(format, "^", textStart);
			const base::string& text = format.SubStr(textStart, textEnd - textStart);
			dest.Append(text);
			index = textEnd;
		}
	}
}

// LCS text render, todo new vcs rect render when it's finished
void sTextSpriteSync::Print() {
	base::string text;
	format_text(text, m_sText);
	if (!m_nFlash || m_bCanShow) {
		uint32 len = (sizeof(wchar) * text.size()) + (sizeof(wchar) * 2); // this is bad
		wchar* buf = new wchar[len];
		if (buf) {
			memset(buf, 0, len);
			AsciiToUnicode(text.c_str(), buf);
//#ifdef GTA_LIBERTY
			CFont::PrintString(buf, CVector2D(SCREEN_SCALE_X(m_posOld.x), SCREEN_SCALE_Y(m_posOld.y)), m_color, m_eAlign, m_nStyle,
				CVector2D(SCREEN_SCALE_X(m_scale.x), SCREEN_SCALE_Y(m_scale.y)), SCREEN_SCALE_X(m_nWrapX));
//#else
//			if (m_eAlign == ALIGN_LEFT) {
//				CFont::PrintString(buf, CVector2D(SCREEN_SCALE_X(m_posOld.x), SCREEN_SCALE_Y(m_posOld.y)), m_color, m_eAlign, m_nStyle,
//					CVector2D(SCREEN_SCALE_X(m_scale.x), SCREEN_SCALE_Y(m_scale.y)), SCREEN_SCALE_X(m_nWrapX));
//			}
//			else if (m_eAlign == ALIGN_CENTER) {
//				float x = m_posOld.x;
//				float screenW = DEFAULT_SCREEN_WIDTH;
//				float leftBound = 5.0f;
//				float halfH = DEFAULT_SCREEN_HEIGHT / 2.0f;
//
//				if (x <= halfH)
//					x = leftBound;
//				else if (x >= screenW - halfH)
//					x = screenW - leftBound;
//
//				CFont::PrintString(buf, CVector2D(SCREEN_SCALE_X(x), SCREEN_SCALE_Y(m_posOld.y)), m_color, m_eAlign, m_nStyle,
//					CVector2D(SCREEN_SCALE_X(m_scale.x), SCREEN_SCALE_Y(m_scale.y)), SCREEN_SCALE_X(m_nWrapX));
//			}
//			else if (m_eAlign == ALIGN_RIGHT) {
//				CFont::PrintString(buf, CVector2D(SCREEN_SCALE_X(m_posOld.x), SCREEN_SCALE_Y(m_posOld.y)), m_color, m_eAlign, m_nStyle,
//					CVector2D(SCREEN_SCALE_X(m_scale.x), SCREEN_SCALE_Y(m_scale.y)), SCREEN_SCALE_X(m_nWrapX));
//			}
//#endif
			delete[] buf;
		}
	}
	if (m_nFlash) {
		m_bCanShow = !m_bCanShow;
		m_nFlash--;
	}
}


#ifdef GTA_LIBERTY
sTextSprite::sTextSprite() : sElement()
#else
sTextSprite::sTextSprite() : sSpriteBase()
#endif
{
	DECLARE_ELEMENT_CONSTRUCT(this, true, false);
	;
	DECLARE_ELEMENT_CONSTRUCT(this, false, false);
}

#ifdef GTA_LIBERTY
sTextSprite::sTextSprite(int32 nPeerID, uint8 align, float x, float y) : sElement()
#else
sTextSprite::sTextSprite(int32 nPeerID, uint8 align, float x, float y) : sSpriteBase()
#endif
{
	DECLARE_ELEMENT_CONSTRUCT(this, true, true);
	RegisterSelf();
	AttachSync(m_nTime, new sTextSpriteSync(align, x, y));
	cInterestZone* pZone = cMultiGame::Instance().m_ZoneManager.GetZoneByPeer(nPeerID);
	pZone->RegisterElement(this);
#ifdef GTA_LIBERTY
	ms_vItems.push_back(this);
#endif
	DECLARE_ELEMENT_CONSTRUCT(this, false, true);
}

ElementCapability sTextSprite::GetCapability()
{
	return sTextSprite::Capability();
}

bool sTextSprite::HasCapability(ElementCapability capability)
{
	if (sTextSprite::Capability() == capability)
		return true;
	if (sSpriteBase::Capability() == capability)
		return true;
	if (sElement::Capability() == capability)
		return true;
	return false;
}

#ifdef GTA_LIBERTY
sTextSprite::~sTextSprite()
{
	DECLARE_ELEMENT_DESTRUCT(this);
	for (std::vector<sTextSprite*>::iterator it = ms_vItems.begin(); it != ms_vItems.end(); it++) {
		if (*it == this) {
			ms_vItems.erase(it);
			break;
}
	}
}
#else
sTextSprite::~sTextSprite()
{
	DECLARE_ELEMENT_DESTRUCT(this);
	sElement::PurgeAttached();
	// ~sElement
}
#endif

sElementSync* sTextSprite::CreateSync() {
	return new sTextSpriteSync();
}

void sTextSprite::DisposeSync(sElementSync* pSync) {
	if (pSync)
		delete ((sTextSpriteSync*)pSync);
}

sElementSync* sTextSprite::CreateSyncFromOther(sElementSync* pSync) {
	sTextSpriteSync& sync = *(sTextSpriteSync*)pSync;
	return new sTextSpriteSync(sync);
}

bool sTextSprite::HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) {
	sTextSpriteSync& syncA = *(sTextSpriteSync*)pSyncA;
	sTextSpriteSync& syncB = *(sTextSpriteSync*)pSyncB;
	return syncA.Compare(syncB);
}

void sTextSprite::ApplyClientSync(uint16 nTime) {
	sElement::ApplyClientSync(nTime);
}

bool sTextSprite::WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) {
	if (static_cast<int16>(nSyncLastTime - m_vSync.front().m_nTime) >= 0) // FindSync? not GetSyncWithTime?
		return WriteSyncDelta(pSyncStream, FindSync(nSyncWriteTime, nil).text, GetSyncWithTime(nSyncLastTime).text);

	tTextSpriteSyncsDeltas textSpriteDeltaManager{};
	textSpriteDeltaManager.SetDifferenceTextSprite(); // FindSync? not GetSyncWithTime?
	PerformWriteSync(pSyncStream, FindSync(nSyncWriteTime, nil).text, textSpriteDeltaManager); // max diff
	return true;
}

void sTextSprite::ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) {
	sTextSpriteSync& sync = *(sTextSpriteSync*)pOutSync;
	uint16 nDiffMask = pSyncStream->ReadU16();

	//if (nDiffMask & eTextSpriteSync::MP_PKTD_TXT_SPR_BASE) // beta? FIX_BUGS?
		sSpriteBase::ReadSyncFromStream(pSyncStream, pOutSync);

	if (nDiffMask & eTextSpriteSync::MP_PKTD_TXT_SPR_TEXT)
		sync.m_sText = pSyncStream->ReadString();

	if (nDiffMask & eTextSpriteSync::MP_PKTD_TXT_SPR_SCALE)
		sync.m_scale = pSyncStream->ReadVector2D();

	if (nDiffMask & eTextSpriteSync::MP_PKTD_TXT_SPR_WRAP_X)
		sync.m_nWrapX = pSyncStream->ReadI32();

	if (nDiffMask & eTextSpriteSync::MP_PKTD_TXT_SPR_POS_SCALED)
		sync.m_posScaled = pSyncStream->ReadVector2D();

	if (nDiffMask & eTextSpriteSync::MP_PKTD_TXT_SPR_POS)
		sync.m_pos = pSyncStream->ReadVector2D();

	if (nDiffMask & eTextSpriteSync::MP_PKTD_TXT_SPR_ALIGN)
		sync.m_eAlign = pSyncStream->ReadU8();

	if (nDiffMask & eTextSpriteSync::MP_PKTD_TXT_SPR_STYLE)
		sync.m_nStyle = pSyncStream->ReadU8();

	if (nDiffMask & eTextSpriteSync::MP_PKTD_TXT_SPR_FLASH)
		sync.m_nFlash = pSyncStream->ReadI32();
}

void sTextSprite::UpdateDelta(sElementSync* pSync, uint16 nTimeDelta) {
	sTextSpriteSync& sync = *(sTextSpriteSync*)pSync;
	sync.UpdateDelta(nTimeDelta);
}

void sTextSprite::OnHudPrint() {
	int32 nPeer = m_pZone->GetID();
	if (cMultiGame::Instance().IsLocalPlayer(nPeer) && (!gbMP_DrawPauseScreen || GetOrder() < 0.0f)) {
		GetSync().text->Print();
	}
}

void sTextSprite::CompareSyncState(sTextSpriteSync* pSync, sTextSpriteSync* pLastSync, tTextSpriteSyncsDeltas* pDiff) {
	sSpriteBase::CompareSyncState(pSync, pLastSync, &pDiff->nBaseSpriteDiff);

	//if (pDiff->nBaseSpriteDiff != eSpriteBaseSync::MP_PKTD_SPR_BASE_EQUAL) // beta? FIX_BUGS?
	//	pDiff->nTextSpriteDiff |= eTextSpriteSync::MP_PKTD_TXT_SPR_BASE;

	if (pLastSync->m_sText.size() != pSync->m_sText.size() ||
		memcmp(pLastSync->m_sText.c_str(), pSync->m_sText.c_str(), pLastSync->m_sText.size()) != 0)
		pDiff->nTextSpriteDiff |= eTextSpriteSync::MP_PKTD_TXT_SPR_TEXT;

	if (pLastSync->m_scale.x != pSync->m_scale.x ||
		pLastSync->m_scale.y != pSync->m_scale.y)
		pDiff->nTextSpriteDiff |= eTextSpriteSync::MP_PKTD_TXT_SPR_SCALE;

	if (pLastSync->m_nWrapX != pSync->m_nWrapX)
		pDiff->nTextSpriteDiff |= eTextSpriteSync::MP_PKTD_TXT_SPR_WRAP_X;

	if (pLastSync->m_posScaled.x != pSync->m_posScaled.x ||
		pLastSync->m_posScaled.y != pSync->m_posScaled.y)
		pDiff->nTextSpriteDiff |= eTextSpriteSync::MP_PKTD_TXT_SPR_POS_SCALED;

	if (pLastSync->m_pos.x != pSync->m_pos.x ||
		pLastSync->m_pos.y != pSync->m_pos.y)
		pDiff->nTextSpriteDiff |= eTextSpriteSync::MP_PKTD_TXT_SPR_POS;

	if (pLastSync->m_eAlign != pSync->m_eAlign)
		pDiff->nTextSpriteDiff |= eTextSpriteSync::MP_PKTD_TXT_SPR_ALIGN;

	if (pLastSync->m_nStyle != pSync->m_nStyle)
		pDiff->nTextSpriteDiff |= eTextSpriteSync::MP_PKTD_TXT_SPR_STYLE;

	if (pLastSync->m_nFlash != pSync->m_nFlash)
		pDiff->nTextSpriteDiff |= eTextSpriteSync::MP_PKTD_TXT_SPR_FLASH;
}

void sTextSprite::PerformWriteSync(sWriteSyncStream* pSyncStream, sTextSpriteSync* pSync, tTextSpriteSyncsDeltas diff) {
	pSyncStream->WriteU16(diff.nTextSpriteDiff);

	//if (diff.nTextSpriteDiff & eTextSpriteSync::MP_PKTD_TXT_SPR_BASE) // beta? FIX_BUGS?
		sSpriteBase::PerformWriteSync(pSyncStream, pSync, diff.nBaseSpriteDiff);

	if (diff.nTextSpriteDiff & eTextSpriteSync::MP_PKTD_TXT_SPR_TEXT)
		pSyncStream->WriteString(pSync->m_sText);

	if (diff.nTextSpriteDiff & eTextSpriteSync::MP_PKTD_TXT_SPR_SCALE)
		pSyncStream->WriteVector2D(pSync->m_scale);

	if (diff.nTextSpriteDiff & eTextSpriteSync::MP_PKTD_TXT_SPR_WRAP_X)
		pSyncStream->WriteI32(pSync->m_nWrapX);

	if (diff.nTextSpriteDiff & eTextSpriteSync::MP_PKTD_TXT_SPR_POS_SCALED)
		pSyncStream->WriteVector2D(pSync->m_posScaled);

	if (diff.nTextSpriteDiff & eTextSpriteSync::MP_PKTD_TXT_SPR_POS)
		pSyncStream->WriteVector2D(pSync->m_pos);

	if (diff.nTextSpriteDiff & eTextSpriteSync::MP_PKTD_TXT_SPR_ALIGN)
		pSyncStream->WriteU8(pSync->m_eAlign);

	if (diff.nTextSpriteDiff & eTextSpriteSync::MP_PKTD_TXT_SPR_STYLE)
		pSyncStream->WriteU8(pSync->m_nStyle);

	if (diff.nTextSpriteDiff & eTextSpriteSync::MP_PKTD_TXT_SPR_FLASH)
		pSyncStream->WriteI32(pSync->m_nFlash);
}

bool sTextSprite::WriteSyncDelta(sWriteSyncStream* pSyncStream, sTextSpriteSync* pSync, sTextSpriteSync* pLastSync) {
	tTextSpriteSyncsDeltas textSpriteDeltaManager{};
	textSpriteDeltaManager.SetEqualTextSprite();
	CompareSyncState(pSync, pLastSync, &textSpriteDeltaManager);

	if (textSpriteDeltaManager.nTextSpriteDiff == eTextSpriteSync::MP_PKTD_TXT_SPR_EQUAL &&
		textSpriteDeltaManager.nBaseSpriteDiff == eSpriteBaseSync::MP_PKTD_SPR_BASE_EQUAL) // main delta
		return false;

	PerformWriteSync(pSyncStream, pSync, textSpriteDeltaManager);
	return true;
}


#ifdef GTA_LIBERTY
CRGBA* sTextSprite::GetColour() {
	return &GetSync().text->m_color;
}

void sTextSprite::SetColour(CRGBA value) {
	GetSync().text->m_color = value;
}

int32 sTextSprite::GetOrder() {
	return GetSync().text->m_nOrder;
}

void sTextSprite::SetOrder(int32 value) {
	GetSync().text->m_nOrder = value;
}
#endif

int32 sTextSprite::GetFlash() {
	return GetSync().text->m_nFlash;
}

void sTextSprite::SetFlash(int32 value) {
	GetSync().text->m_nFlash = value;
}

CVector2D* sTextSprite::GetScale() {
	return &GetSync().text->m_scale;
}

void sTextSprite::SetScale(float x, float y) {
	sTextSpriteSync* pSprite = GetSync().text;
	pSprite->m_scale.x = x;
	pSprite->m_scale.y = y;
}

uint8 sTextSprite::GetStyle() {
	return GetSync().text->m_nStyle;
}

void sTextSprite::SetStyle(uint8 nStyle) {
	GetSync().text->m_nStyle = nStyle;
}

int32 sTextSprite::GetWrapX() {
	return GetSync().text->m_nWrapX;
}

void sTextSprite::SetWrapX(int32 nWrapX) {
	GetSync().text->m_nWrapX = nWrapX;
}

uint8 sTextSprite::GetAlign() {
	return GetSync().text->m_eAlign;
}

void sTextSprite::SetAlign(uint8 eAlign) {
	GetSync().text->m_eAlign = eAlign;
}

base::string* sTextSprite::GetText() {
	return &GetSync().text->m_sText;
}

void sTextSprite::SetText(base::string& text) {
	GetSync().text->m_sText = text;
}

void sTextSprite::SetPos(int32 x, int32 y, float scale) {
	sTextSpriteSync* pSpriteSync = GetSync().text;
	pSpriteSync->m_pos.x = x;
	pSpriteSync->m_pos.y = y;
	if (scale <= 0.0f) {
		pSpriteSync->m_posScaled = CVector2D(0.0f, 0.0f);
		pSpriteSync->m_posOld = pSpriteSync->m_pos;
	}
	else {
		float fDiffX = pSpriteSync->m_pos.x - pSpriteSync->m_posOld.x;
		float fDiffY = pSpriteSync->m_pos.y - pSpriteSync->m_posOld.y;
		pSpriteSync->m_posScaled.x = (1.0f / scale) * fDiffX;
		pSpriteSync->m_posScaled.y = (1.0f / scale) * fDiffY;
	}
}

CVector2D* sTextSprite::GetPos() {
	return &GetSync().text->m_pos;
}

#ifdef GTA_LIBERTY
void sTextSprite::Terminate() {
	for (std::vector<sTextSprite*>::iterator it = ms_vItems.begin(); it != ms_vItems.end(); it++) {
		delete* it;
	}
	ms_vItems.clear();
}

void sTextSprite::UpdateAll() {
	for (std::vector<sTextSprite*>::iterator it = ms_vItems.begin(); it != ms_vItems.end(); it++) {
		(*it)->OnHudPrint();
	}
}
#endif


void mp_update_sprites() {
#ifdef GTA_LIBERTY
	sTextSprite::UpdateAll();
#else
	sSpriteBase::UpdateAll();
#endif
}

#ifdef GTA_LIBERTY
std::vector<sTextSprite*> sTextSprite::ms_vItems;
#else
std::vector<sSpriteBase*> sSpriteBase::ms_vItems;
#endif
