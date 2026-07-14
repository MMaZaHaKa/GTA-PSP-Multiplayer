/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"
#include "Font.h"
#include "Text.h"

#include "multiplayer/public.h"
#include "multiplayer/elements/sSyncStream.h"
#include "multiplayer/elements/sNetMeter2d.h"
#include "multiplayer/elements/sTextSprite.h"
#include "multiplayer/MultiGame.h"
#include "Frontend.h"
#include "Hud.h"

#ifndef GTA_LIBERTY
sNetMeter2dSync::sNetMeter2dSync() : sSpriteBaseSync()
{
    DECLARE_SYNC_CONSTRUCT(this);
	m_bIsUseTitle = false;
	m_sTitleKey = " ";
}

sNetMeter2dSync::sNetMeter2dSync(float posX, float posY, float width, float height) : sSpriteBaseSync(posX, posY, width, height)
{
    DECLARE_SYNC_CONSTRUCT(this);
	m_bIsUseTitle = false;
	m_sTitleKey = " ";
}

sNetMeter2dSync::sNetMeter2dSync(const sNetMeter2dSync& other) : sSpriteBaseSync(other)
{
    DECLARE_SYNC_CONSTRUCT(this);
    m_Alpha = other.m_Alpha;
    m_fFillRatio = other.m_fFillRatio;
    m_bIsFlashing = other.m_bIsFlashing;
    m_bIsUseTitle = other.m_bIsUseTitle;
    m_sTitleKey = other.m_sTitleKey;
}

sNetMeter2dSync::~sNetMeter2dSync() {
    DECLARE_SYNC_DESTRUCT(this);
}

bool sNetMeter2dSync::Compare(const sNetMeter2dSync& other)
{
//#ifdef FIX_BUGS // need?
//	if (!sSpriteBaseSync::Compare(other))
//		return false;
//#endif

    if (m_Alpha != other.m_Alpha)
        return false;

    if (m_fFillRatio != other.m_fFillRatio)
        return false;

    if (m_bIsFlashing != other.m_bIsFlashing)
        return false;

    if (m_bIsUseTitle != other.m_bIsUseTitle)
        return false;

    if (m_sTitleKey.size() != other.m_sTitleKey.size())
        return false;

    if (memcmp(m_sTitleKey.c_str(), other.m_sTitleKey.c_str(), m_sTitleKey.size()) != 0)
        return false;

    return true;
}

#if !defined(FINAL) && !defined(MASTER)
void sNetMeter2dSync::Dump()
{
    sSpriteBaseSync::Dump();

    printf("=== sNetMeter2dSync Dump ===\n");
    printf("Net Meter Info:\n");
    printf("  Alpha: %u (0x%02X)\n", m_Alpha, m_Alpha);
    printf("  FillRatio: %.2f (0x%08X)\n", m_fFillRatio, *(uint32*)&m_fFillRatio);
    printf("  IsFlashing: %s\n", m_bIsFlashing ? "true" : "false");
    printf("  IsUseTitle: %s\n", m_bIsUseTitle ? "true" : "false");
    printf("  TitleKey: \"%s\"\n", m_sTitleKey.c_str());
    printf("================================\n");
}
#endif


sNetMeter2d::sNetMeter2d() : sSpriteBase()
{
    DECLARE_ELEMENT_CONSTRUCT(this, true, false);
    m_HudBar = cHudBar();
    m_HudBar.m_nFadeState = eHudBarFadeState::FADE_STATE_INACTIVE;
    DECLARE_ELEMENT_CONSTRUCT(this, false, false);
}

sNetMeter2d::sNetMeter2d(int32 nPeerID, float posX, float posY, float width, float height) : sSpriteBase()
{
    DECLARE_ELEMENT_CONSTRUCT(this, true, true);
    m_HudBar = cHudBar();
	RegisterSelf();
	AttachSync(m_nTime, new sNetMeter2dSync(posX, posY, width, height));
	cInterestZone* pZone = cMultiGame::Instance().m_ZoneManager.GetZoneByPeer(nPeerID);
	pZone->RegisterElement(this);
    m_HudBar.m_nFadeState = eHudBarFadeState::FADE_STATE_INACTIVE;
    DECLARE_ELEMENT_CONSTRUCT(this, false, true);
}

ElementCapability sNetMeter2d::GetCapability()
{
    return sNetMeter2d::Capability();
}

bool sNetMeter2d::HasCapability(ElementCapability capability)
{
    if (sNetMeter2d::Capability() == capability)
        return true;
    if (sSpriteBase::Capability() == capability)
        return true;
    if (sElement::Capability() == capability)
        return true;
    return false;
}

sNetMeter2d::~sNetMeter2d() {
    DECLARE_ELEMENT_DESTRUCT(this);
    sElement::PurgeAttached();
    // ~sElement
}

sElementSync* sNetMeter2d::CreateSync() {
    return new sNetMeter2dSync();
}

void sNetMeter2d::DisposeSync(sElementSync* pSync) {
    if(pSync)
        delete ((sNetMeter2dSync*)pSync);
}

sElementSync* sNetMeter2d::CreateSyncFromOther(sElementSync* pSync) {
    sNetMeter2dSync& sync = *(sNetMeter2dSync*)pSync;
    return new sNetMeter2dSync(sync);
}

bool sNetMeter2d::HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) {
    sNetMeter2dSync& syncA = *(sNetMeter2dSync*)pSyncA;
    sNetMeter2dSync& syncB = *(sNetMeter2dSync*)pSyncB;
    return syncA.Compare(syncB);
}

void sNetMeter2d::ApplyClientSync(uint16 time) {
    sElement::ApplyClientSync(time);
    m_HudBar.DoFade(BAR_DELTA);
}

bool sNetMeter2d::WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) {
    if (static_cast<int16>(nSyncLastTime - m_vSync.front().m_nTime) >= 0) // FindSync? not GetSyncWithTime?
        return WriteSyncDelta(pSyncStream, FindSync(nSyncWriteTime, nil).netmeter, GetSyncWithTime(nSyncLastTime).netmeter);

    tNetMeter2dSyncsDeltas netMeterDeltaManager{};
    netMeterDeltaManager.SetDifferenceNetMeter(); // FindSync? not GetSyncWithTime?
    PerformWriteSync(pSyncStream, FindSync(nSyncWriteTime, nil).netmeter, netMeterDeltaManager); // max diff
    return true;
}

void sNetMeter2d::ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) {
    sNetMeter2dSync& sync = *(sNetMeter2dSync*)pOutSync;
    uint16 nDiffMask = pSyncStream->ReadU16();

    if (nDiffMask & eNetMeter2dSync::MP_PKTD_NETMTR_BASE)
        sSpriteBase::ReadSyncFromStream(pSyncStream, pOutSync);

    if (nDiffMask & eNetMeter2dSync::MP_PKTD_NETMTR_ALPHA)
        sync.m_Alpha = pSyncStream->ReadU8();

    if (nDiffMask & eNetMeter2dSync::MP_PKTD_NETMTR_FILL_RATIO)
        sync.m_fFillRatio = pSyncStream->ReadFloat();

    if (nDiffMask & eNetMeter2dSync::MP_PKTD_NETMTR_IS_FLASHING)
        sync.m_bIsFlashing = pSyncStream->ReadBool();

    if (nDiffMask & eNetMeter2dSync::MP_PKTD_NETMTR_IS_USE_TITLE)
        sync.m_bIsUseTitle = pSyncStream->ReadBool();

    if (nDiffMask & eNetMeter2dSync::MP_PKTD_NETMTR_TITLE_KEY)
        sync.m_sTitleKey = pSyncStream->ReadString();
}

// LCS text render, todo new vcs rect render when it's finished
void sNetMeter2d::OnHudPrint() {
    cMultiGame& pGame = cMultiGame::Instance();
    if (pGame.GetGameType() == eGameType::VIP && pGame.LocalPlayerID() == pGame.m_nVipPeerID) // VIP doesn't need his bar, but srv send it
        return;

    if ((!pGame.IsLocalPlayer(m_pZone->GetID())) || (gbMP_DrawPauseScreen && GetOrder() >= 0.0f))
        return;

    sNetMeter2dSync* pSync = GetSync().netmeter;
    float fillRatio = pSync->m_fFillRatio;
    bool useTitle = pSync->m_bIsUseTitle;
    CRGBA baseColor = pSync->m_color;
    CVector2D size = pSync->m_Size; // unused? 48/15
    CRect pspBarRect = CRect(352.0f, 65.0f, 416.0f, 81.0f);
    CRect barRect(SCREEN_SCALE_FROM_RIGHT(HUD_BARS_X), SCREEN_SCALE_Y(HUD_BARS_Y + (HUD_BARS_SIZE_Y * 2.5)),
        SCREEN_SCALE_X(HUD_BARS_W), SCREEN_SCALE_Y(HUD_BARS_H)); // revcs calc


    if (pGame.GetGameType() == eGameType::HUNTERATTACK)
    {
        CRGBA color(150, 108, 65, 255);
        m_HudBar.Draw(&barRect, &TheHud->Sprites[HUD_BAR_INSIDE1], &TheHud->Sprites[HUD_BAR_INSIDE1DARK], &TheHud->Sprites[HUD_BAR_OUTLINE], &color, fillRatio);
    }
    else
    {
        if (baseColor.blue >= baseColor.red || baseColor.green >= baseColor.red)
            m_HudBar.Draw(&barRect, &TheHud->Sprites[HUD_BAR_INSIDE1], &TheHud->Sprites[HUD_BAR_INSIDE1DARK], &TheHud->Sprites[HUD_BAR_OUTLINE], &baseColor, fillRatio);
        else
            m_HudBar.Draw(&barRect, &TheHud->Sprites[HUD_BAR_INSIDE2], &TheHud->Sprites[HUD_BAR_INSIDE2DARK], &TheHud->Sprites[HUD_BAR_OUTLINE], &baseColor, fillRatio);
    }

    if (useTitle)
    {
        wchar* text = TheText.Get(GetSync().netmeter->m_sTitleKey.c_str());

        CFont::SetFontStyle(FONT_STANDARD);
        //CFont::ResetState();

        // custom
        {
            CFont::SetJustifyOff();
            CFont::SetCentreOff();
            CFont::SetRightJustifyOff();
            CFont::SetBackgroundOff();
            CFont::SetDropShadowPosition(1); // shadow custom
            CFont::SetPropOn();
            CFont::SetDropColor(CRGBA(0, 0, 0, 255));

            CFont::SetJustifyOn(); // vcs
        }

        CRGBA textColor;
        if (pGame.GetGameType() == eGameType::HUNTERATTACK) // leeds hotfix? kek
            textColor = CRGBA(74, 181, 160, 255);
        else
            textColor = baseColor;
        textColor.alpha = 255;

        // custom, psp CFont::DrawInRect
        {
            CRect pspTextRect = CRect(424.0f, 0.0f, 475.0f, 5.0f);
            float psp_x_pad = SCREEN_SCALE_X(8.0f); // pspTextRect.left - pspBarRect.right; // 424.0 - 416.0 = 8.0 x psp pad
            CFont::SetColor(textColor);
            //CFont::SetScale(SCREEN_SCALE_X(0.7f), SCREEN_SCALE_Y(0.7f));
            CFont::SetScale(SCREEN_SCALE_X(MEDIUMTEXT_X_SCALE), SCREEN_SCALE_Y(MEDIUMTEXT_Y_SCALE)); // custom
            CFont::SetWrapx(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH));
            //CFont::SetCentreSize(SCREEN_SCALE_X(260.0f));
            CFont::PrintString((barRect.left + barRect.right + psp_x_pad), barRect.top, text); // psp pad
        }
    }
}


void sNetMeter2d::CompareSyncState(sNetMeter2dSync* pSync, sNetMeter2dSync* pLastSync, tNetMeter2dSyncsDeltas* pDiff) {
    sSpriteBase::CompareSyncState(pSync, pLastSync, &pDiff->nBaseSpriteDiff);

    if (pDiff->nBaseSpriteDiff != eSpriteBaseSync::MP_PKTD_SPR_BASE_EQUAL)
        pDiff->nNetMeterDiff |= eNetMeter2dSync::MP_PKTD_NETMTR_BASE;

#ifdef FIX_BUGS
    if (pLastSync->m_Alpha != pSync->m_Alpha)
        pDiff->nNetMeterDiff |= eNetMeter2dSync::MP_PKTD_NETMTR_ALPHA;
#else
    if (FLT_EPS_NOT_EQ((float)pLastSync->m_Alpha, (float)pSync->m_Alpha)) // what?? pastebug from m_fFillRatio
        pDiff->nNetMeterDiff |= eNetMeter2dSync::MP_PKTD_NETMTR_ALPHA;
#endif

    if (FLT_EPS_NOT_EQ(pLastSync->m_fFillRatio, pSync->m_fFillRatio))
        pDiff->nNetMeterDiff |= eNetMeter2dSync::MP_PKTD_NETMTR_FILL_RATIO;

    if (pLastSync->m_bIsFlashing != pSync->m_bIsFlashing)
        pDiff->nNetMeterDiff |= eNetMeter2dSync::MP_PKTD_NETMTR_IS_FLASHING;

    if (pLastSync->m_bIsUseTitle != pSync->m_bIsUseTitle)
        pDiff->nNetMeterDiff |= eNetMeter2dSync::MP_PKTD_NETMTR_IS_USE_TITLE;

    if (pLastSync->m_sTitleKey.size() != pSync->m_sTitleKey.size() ||
        memcmp(pLastSync->m_sTitleKey.c_str(), pSync->m_sTitleKey.c_str(), pLastSync->m_sTitleKey.size()) != 0)
        pDiff->nNetMeterDiff |= eNetMeter2dSync::MP_PKTD_NETMTR_TITLE_KEY;
}

void sNetMeter2d::PerformWriteSync(sWriteSyncStream* pSyncStream, sNetMeter2dSync* pSync, tNetMeter2dSyncsDeltas diff) {
    pSyncStream->WriteU16(diff.nNetMeterDiff);

    if (diff.nNetMeterDiff & eNetMeter2dSync::MP_PKTD_NETMTR_BASE)
        sSpriteBase::PerformWriteSync(pSyncStream, pSync, diff.nBaseSpriteDiff);

    if (diff.nNetMeterDiff & eNetMeter2dSync::MP_PKTD_NETMTR_ALPHA)
        pSyncStream->WriteU8(pSync->m_Alpha);

    if (diff.nNetMeterDiff & eNetMeter2dSync::MP_PKTD_NETMTR_FILL_RATIO)
        pSyncStream->WriteFloat(pSync->m_fFillRatio);

    if (diff.nNetMeterDiff & eNetMeter2dSync::MP_PKTD_NETMTR_IS_FLASHING)
        pSyncStream->WriteBool(pSync->m_bIsFlashing);

    if (diff.nNetMeterDiff & eNetMeter2dSync::MP_PKTD_NETMTR_IS_USE_TITLE)
        pSyncStream->WriteBool(pSync->m_bIsUseTitle);

    if (diff.nNetMeterDiff & eNetMeter2dSync::MP_PKTD_NETMTR_TITLE_KEY)
        pSyncStream->WriteString(pSync->m_sTitleKey);
}

bool sNetMeter2d::WriteSyncDelta(sWriteSyncStream* pSyncStream, sNetMeter2dSync* pSync, sNetMeter2dSync* pLastSync) {
    tNetMeter2dSyncsDeltas netMeterDeltaManager{};
    netMeterDeltaManager.SetEqualNetMeter();
    CompareSyncState(pSync, pLastSync, &netMeterDeltaManager);

    if (netMeterDeltaManager.nNetMeterDiff == eNetMeter2dSync::MP_PKTD_NETMTR_EQUAL &&
        netMeterDeltaManager.nBaseSpriteDiff == eSpriteBaseSync::MP_PKTD_SPR_BASE_EQUAL) // main delta
        return false;

    PerformWriteSync(pSyncStream, pSync, netMeterDeltaManager);
    return true;
}

uint8 sNetMeter2d::GetAlpha() {
	return GetSync().netmeter->m_Alpha;
}

void sNetMeter2d::SetAlpha(uint8 alpha) {
	GetSync().netmeter->m_Alpha = alpha;
}

float sNetMeter2d::GetFillRatio() {
	return GetSync().netmeter->m_fFillRatio;
}

void sNetMeter2d::SetFillRatio(float fillRatio) {
	GetSync().netmeter->m_fFillRatio = fillRatio;
}

bool sNetMeter2d::GetFlashing() {
	return GetSync().netmeter->m_bIsFlashing;
}

void sNetMeter2d::SetFlashing(bool flashing) {
	GetSync().netmeter->m_bIsFlashing = flashing;
}

bool sNetMeter2d::GetUseTitle() {
	return GetSync().netmeter->m_bIsUseTitle;
}

void sNetMeter2d::SetUseTitle(bool isUse) {
	GetSync().netmeter->m_bIsUseTitle = isUse;
}

void sNetMeter2d::SetTitleKey(const char* titleKey) {
	GetSync().netmeter->m_sTitleKey = titleKey;
}
#endif