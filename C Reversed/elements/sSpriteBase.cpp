/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"

#include "multiplayer/elements/sSpriteBase.h"


#ifndef GTA_LIBERTY
sSpriteBaseSync::sSpriteBaseSync() : sElementSync()
{
	m_color = CRGBA(255, 255, 255, 255);
	m_BasePos = CVector2D(0.0f, 0.0f);
	m_Size = CVector2D(0.0f, 0.0f);
	m_fOrder = 0.0f;
}

sSpriteBaseSync::sSpriteBaseSync(float posX, float posY, float width, float height) : sElementSync()
{
	m_color = CRGBA(255, 255, 255, 255);
	m_BasePos = CVector2D(posX, posY);
	m_Size = CVector2D(width, height);
	m_fOrder = 0.0f;
}

#ifdef FIX_BUGS
sSpriteBaseSync::sSpriteBaseSync(const sSpriteBaseSync& other) : sElementSync(other)
#else
sSpriteBaseSync::sSpriteBaseSync(const sSpriteBaseSync& other) : sElementSync()
#endif
{
	m_color = other.m_color;
	m_BasePos = other.m_BasePos;
	m_Size = other.m_Size;
	m_fOrder = other.m_fOrder;
}

sSpriteBaseSync::~sSpriteBaseSync()
{

}

bool sSpriteBaseSync::Compare(const sSpriteBaseSync& other)
{
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	return false;
}


sSpriteBase::sSpriteBase()
{
	ms_vItems.push_back(this);
}


ElementCapability sSpriteBase::GetCapability()
{
	return sSpriteBase::Capability();
}

bool sSpriteBase::HasCapability(ElementCapability capability)
{
	if (sSpriteBase::Capability() == capability)
		return true;
	if (sElement::Capability() == capability)
		return true;
	return false;
}

sSpriteBase::~sSpriteBase()
{
	for (std::vector<sSpriteBase*>::iterator it = ms_vItems.begin(); it != ms_vItems.end(); it++) {
		if (*it == this) {
			ms_vItems.erase(it);
			break;
		}
	}
}

void sSpriteBase::ApplyClientSync(uint16 time) {
	sElement::ApplyClientSync(time);
}

/* TODO(MP): stub */
void sSpriteBase::ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) {

}

void sSpriteBase::Terminate()
{
	for (std::vector<sSpriteBase*>::iterator it = ms_vItems.begin(); it != ms_vItems.end(); it++) {
		delete* it;
	}
	ms_vItems.clear();
}

// inlined
void sSpriteBase::UpdateAll() // sNetMeter2d + sTextSprite
{
//#ifdef DEBUG_MULTIGAME
//	int nCount = 0;
//	for (std::vector<sSpriteBase*>::iterator it = ms_vItems.begin(); it != ms_vItems.end(); it++) {
//		(*it)->OnHudPrint();
//		++nCount;
//	}
//	debug("sSpriteBase::UpdateAll: printed %d elements\n", nCount);
//#else
	for (std::vector<sSpriteBase*>::iterator it = ms_vItems.begin(); it != ms_vItems.end(); it++) {
		(*it)->OnHudPrint();
	}
//#endif
}


CRGBA* sSpriteBase::GetColour() {
	return &GetSync().sprite->m_color;
}

void sSpriteBase::SetColour(CRGBA value) {
	GetSync().sprite->m_color = value;
}

CVector2D* sSpriteBase::GetPos() {
	return &GetSync().sprite->m_BasePos;
}

void sSpriteBase::SetPos(CVector2D pos) {
	GetSync().sprite->m_BasePos = pos;
}

CVector2D* sSpriteBase::GetSize() {
	return &GetSync().sprite->m_Size;
}

void sSpriteBase::SetSize(CVector2D size) {
	GetSync().sprite->m_Size = size;
}

float sSpriteBase::GetOrder() {
	return GetSync().sprite->m_fOrder;
}

void sSpriteBase::SetOrder(float fValue) {
	GetSync().sprite->m_fOrder = fValue;
}
#endif
