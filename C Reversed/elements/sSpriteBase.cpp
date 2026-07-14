/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"

#include "multiplayer/elements/sSyncStream.h"
#include "multiplayer/elements/sSpriteBase.h"


#ifndef GTA_LIBERTY
sSpriteBaseSync::sSpriteBaseSync() : sElementSync()
{
	m_color = CRGBA(255, 255, 255, 255);
	m_posOld = CVector2D(0.0f, 0.0f);
	m_Size = CVector2D(0.0f, 0.0f);
	m_fOrder = 0.0f;
}

sSpriteBaseSync::sSpriteBaseSync(float posX, float posY, float width, float height) : sElementSync()
{
	m_color = CRGBA(255, 255, 255, 255);
	m_posOld = CVector2D(posX, posY);
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
	m_posOld = other.m_posOld;
	m_Size = other.m_Size;
	m_fOrder = other.m_fOrder;
}

sSpriteBaseSync::~sSpriteBaseSync() { }

#ifdef MULTIGAME_ELEMENTS_IMPROVEMENTS
bool sSpriteBaseSync::Compare(const sSpriteBaseSync& other)
{
	if (m_color != other.m_color)
		return false;
	if (m_posOld.x != other.m_posOld.x)
		return false;
	if (m_posOld.y != other.m_posOld.y)
		return false;
	if (m_Size.x != other.m_Size.x)
		return false;
	if (m_fOrder != other.m_fOrder)
		return false;

	return true;
}
#endif

#if !defined(FINAL) && !defined(MASTER)
void sSpriteBaseSync::Dump()
{
	sElementSync::Dump();

	printf("=== sSpriteBaseSync Dump ===\n");
	printf("Sprite Info:\n");
	printf("  Color: R=%u G=%u B=%u A=%u (0x%08X)\n", m_color.red, m_color.green, m_color.blue, m_color.alpha, *(uint32*)&m_color);
	printf("  Position Old: X=%.2f Y=%.2f\n", m_posOld.x, m_posOld.y);
	printf("  Size: Width=%.2f Height=%.2f\n", m_Size.x, m_Size.y);
	printf("  Order: %.2f (0x%08X)\n", m_fOrder, *(uint32*)&m_fOrder);
	printf("================================\n");
}
#endif


sSpriteBase::sSpriteBase() : sElement()
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

void sSpriteBase::ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) {
	sSpriteBaseSync& sync = *(sSpriteBaseSync*)pOutSync;

	uint16 nDiffMask = pSyncStream->ReadU16();

	if (nDiffMask & eSpriteBaseSync::MP_PKTD_SPR_BASE_POS)
		sync.m_posOld = pSyncStream->ReadVector2D();

	if (nDiffMask & eSpriteBaseSync::MP_PKTD_SPR_BASE_SIZE_X)
		sync.m_Size.x = pSyncStream->ReadFloat();

	if (nDiffMask & eSpriteBaseSync::MP_PKTD_SPR_BASE_SIZE_Y)
		sync.m_Size.y = pSyncStream->ReadFloat();

	if (nDiffMask & eSpriteBaseSync::MP_PKTD_SPR_BASE_COLOR)
		sync.m_color = pSyncStream->ReadColour();

	if (nDiffMask & eSpriteBaseSync::MP_PKTD_SPR_BASE_ORDER)
		sync.m_fOrder = pSyncStream->ReadFloat();
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

void sSpriteBase::SetColour(CRGBA colour) {
	GetSync().sprite->m_color = colour;
}

float sSpriteBase::GetOrder() {
	return GetSync().sprite->m_fOrder;
}

void sSpriteBase::SetOrder(float fOrder) {
	GetSync().sprite->m_fOrder = fOrder;
}

void sSpriteBase::CompareSyncState(sSpriteBaseSync* pSync, sSpriteBaseSync* pLastSync, uint16* pDiffMask)
{
	if (FLT_EPS_NOT_EQ(pLastSync->m_posOld.x, pSync->m_posOld.x) ||
		FLT_EPS_NOT_EQ(pLastSync->m_posOld.y, pSync->m_posOld.y))
	{
		*pDiffMask |= eSpriteBaseSync::MP_PKTD_SPR_BASE_POS;
	}

	if (FLT_EPS_NOT_EQ(pLastSync->m_Size.x, pSync->m_Size.x))
		*pDiffMask |= eSpriteBaseSync::MP_PKTD_SPR_BASE_SIZE_X;

	if (FLT_EPS_NOT_EQ(pLastSync->m_Size.y, pSync->m_Size.y))
		*pDiffMask |= eSpriteBaseSync::MP_PKTD_SPR_BASE_SIZE_Y;

	if (pLastSync->m_color != pSync->m_color)
	{
		*pDiffMask |= eSpriteBaseSync::MP_PKTD_SPR_BASE_COLOR;
	}

	if (pLastSync->m_fOrder != pSync->m_fOrder)
		*pDiffMask |= eSpriteBaseSync::MP_PKTD_SPR_BASE_ORDER;
}

void sSpriteBase::PerformWriteSync(sWriteSyncStream* pSyncStream, sSpriteBaseSync* pSync, uint16 nDiffMask)
{
	pSyncStream->WriteU16(nDiffMask);

	if (nDiffMask & eSpriteBaseSync::MP_PKTD_SPR_BASE_POS)
		pSyncStream->WriteVector2D(pSync->m_posOld);

	if (nDiffMask & eSpriteBaseSync::MP_PKTD_SPR_BASE_SIZE_X)
		pSyncStream->WriteFloat(pSync->m_Size.x);

	if (nDiffMask & eSpriteBaseSync::MP_PKTD_SPR_BASE_SIZE_Y)
		pSyncStream->WriteFloat(pSync->m_Size.y);

	if (nDiffMask & eSpriteBaseSync::MP_PKTD_SPR_BASE_COLOR)
		pSyncStream->WriteColour(pSync->m_color);

	if (nDiffMask & eSpriteBaseSync::MP_PKTD_SPR_BASE_ORDER)
		pSyncStream->WriteFloat(pSync->m_fOrder);
}

#ifdef MULTIGAME_ELEMENTS_IMPROVEMENTS
CVector2D* sSpriteBase::GetPosOld() {
	return &GetSync().sprite->m_posOld;
}

void sSpriteBase::SetPosOld(CVector2D pos) {
	GetSync().sprite->m_posOld = pos;
}

CVector2D* sSpriteBase::GetSize() {
	return &GetSync().sprite->m_Size;
}

void sSpriteBase::SetSize(CVector2D size) {
	GetSync().sprite->m_Size = size;
}
#endif
#endif
