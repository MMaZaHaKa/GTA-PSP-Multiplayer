/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"
#include "Font.h"
#include "Text.h"

#include "multiplayer/public.h"
#include "multiplayer/elements/sTextSprite.h"
#include "multiplayer/MultiGame.h"

#ifndef GTA_LIBERTY
sTextSpriteSync::sTextSpriteSync() : sSpriteBaseSync()
{
	m_sText = "";
}
#else
cTextSprite::cTextSprite() : sElementSync() { }
#endif

#ifndef GTA_LIBERTY
sTextSpriteSync::sTextSpriteSync(uint8 align, float posX, float posY) : sSpriteBaseSync(posX, posY, 0.0f, 0.0f)
#else
sTextSpriteSync::sTextSpriteSync(uint8 align, float posX, float posY) : sElementSync()
#endif
{
	m_sText = "";
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
	m_posScale = CVector2D(0, 0);
	m_pos = CVector2D(posX, posY);
#ifdef GTA_LIBERTY
	m_nOrder = 0;
#endif
	m_nFlash = 0;
	m_bCanShow = true;
}


void inline append_translated_text(base::string& dest, const base::string& key) {
	dest.Append(UnicodeToAscii(TheText.Get(key.c_str())));
}

size_t inline next_token(base::string& format, const char* token, const size_t& startPos) {
	size_t pos = format.Find(token, startPos); // -1
	return pos != base::string::npos ? pos : format.size();
}

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

void sTextSpriteSync::Print() {
	base::string text;
	format_text(text, m_sText);
	if (!m_nFlash || m_bCanShow) {
		uint32 len = (sizeof(wchar) * text.size()) + (sizeof(wchar) * 2); // this is bad
		wchar* buf = new wchar[len];
		if (buf) {
			memset(buf, 0, len);
			AsciiToUnicode(text.c_str(), buf);
			CFont::PrintString(buf, CVector2D(SCREEN_SCALE_X(m_BasePos.x), SCREEN_SCALE_Y(m_BasePos.y)), m_color, m_eAlign, m_nStyle,
				CVector2D(SCREEN_SCALE_X(m_scale.x), SCREEN_SCALE_Y(m_scale.y)), SCREEN_SCALE_X(m_nWrapX));
			delete[] buf;
		}
	}
	if (m_nFlash) {
		m_bCanShow = !m_bCanShow;
		m_nFlash--;
	}
}


#ifdef GTA_LIBERTY
sTextSprite::sTextSprite()
#else
sTextSprite::sTextSprite() : sSpriteBase()
#endif
{
	;
}

/* TODO#2 */
#ifdef GTA_LIBERTY
sTextSprite::sTextSprite(int32 nPeerID, uint8 align, float x, float y)
#else
sTextSprite::sTextSprite(int32 nPeerID, uint8 align, float x, float y) : sSpriteBase()
#endif
{
	RegisterSelf();
	AttachSync(m_nTime, new sTextSpriteSync(align, x, y));
	cInterestZone* pZone = cMultiGame::Instance().m_ZoneManager.GetZoneByPeer(nPeerID);
	pZone->RegisterElement(this);
#ifdef GTA_LIBERTY
	ms_vItems.push_back(this);
#endif
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
sTextSprite::~sTextSprite() {
	for (std::vector<sTextSprite*>::iterator it = ms_vItems.begin(); it != ms_vItems.end(); it++) {
		if (*it == this) {
			ms_vItems.erase(it);
			break;
}
	}
}
#else
sTextSprite::~sTextSprite() { }
#endif

sElementSync* sTextSprite::CreateSync() {
	return new sTextSpriteSync();
}

void sTextSprite::DisposeSync(sElementSync* pSync) {
	delete (sTextSpriteSync*)pSync;
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

void sTextSprite::ApplyClientSync(uint16 time) {
	sElement::ApplyClientSync(time);
}

bool sTextSprite::WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) {
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	return false;
}

void sTextSprite::ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) {

}

void sTextSprite::UpdateDelta(sElementSync* pSync, uint16 nTimeDelta) {

}

void sTextSprite::OnHudPrint() {
	int nPeer = m_pZone->GetID();
	if (cMultiGame::Instance().IsLocalPlayer(nPeer) && (!gbMP_DrawPauseScreen || GetOrder() < 0.0f)) {
		GetSync().text->Print();
	}
}


#ifdef GTA_LIBERTY
CRGBA* sTextSprite::GetColour() {
	return &GetSync().text->m_color;
}

void sTextSprite::SetColour(CRGBA value) {
	GetSync().text->m_color = value;
}

int sTextSprite::GetOrder() {
	return GetSync().text->m_nOrder;
}

void sTextSprite::SetOrder(int value) {
	GetSync().text->m_nOrder = value;
}
#endif

int sTextSprite::GetFlash() {
	return GetSync().text->m_nFlash;
}

void sTextSprite::SetFlash(int value) {
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

void sTextSprite::SetStyle(uint8 value) {
	GetSync().text->m_nStyle = value;
}

int sTextSprite::GetWrapX() {
	return GetSync().text->m_nWrapX;
}

void sTextSprite::SetWrapX(int value) {
	GetSync().text->m_nWrapX = value;
}

uint8 sTextSprite::GetAlign() {
	return GetSync().text->m_eAlign;
}

void sTextSprite::SetAlign(uint8 value) {
	GetSync().text->m_eAlign = value;
}

base::string* sTextSprite::GetText() {
	return &GetSync().text->m_sText;
}

void sTextSprite::SetText(base::string& value) {
	GetSync().text->m_sText = value;
}

void sTextSprite::SetPos(int x, int y, float scale) {
	sTextSpriteSync* pSpriteSync = GetSync().text;
	pSpriteSync->m_pos.x = x;
	pSpriteSync->m_pos.y = y;
	if (scale <= 0.0f) {
		pSpriteSync->m_posScale = CVector2D(0.0f, 0.0f);
		pSpriteSync->m_BasePos = pSpriteSync->m_pos;
	}
	else {
		float fDiffX = pSpriteSync->m_pos.x - pSpriteSync->m_BasePos.x;
		float fDiffY = pSpriteSync->m_pos.y - pSpriteSync->m_BasePos.y;
		pSpriteSync->m_posScale.x = (1.0f / scale) * fDiffX;
		pSpriteSync->m_posScale.y = (1.0f / scale) * fDiffY;
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
