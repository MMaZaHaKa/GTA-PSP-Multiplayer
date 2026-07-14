/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"
#include "Vehicle.h"
#include "World.h"
#include "PlayerInfo.h"
#include "PlayerPed.h"
#include "Draw.h"
#include "Pad.h"
#include "Camera.h"
#include "Radar.h"

#include "multiplayer/MultiGame.h"

#include "multiplayer/elements/sPlayer.h"
#include "multiplayer/elements/sPed.h"
#include "multiplayer/elements/sSyncStream.h"


sPlayerSync::sPlayerSync() : sElementSync()
{
	DECLARE_SYNC_CONSTRUCT(this);
#ifdef FIX_BUGS // recheck
	m_vPos = CVector(0.0f, 0.0f, 0.0f);
	m_nKeyPresses = 0;
	m_vCamFront = CVector(0.0f, 0.0f, 0.0f);
	m_vCamSource = CVector(0.0f, 0.0f, 0.0f);
	m_vCamUp = CVector(0.0f, 0.0f, 0.0f);
	m_fCamFov = 0.0f;
#endif
	m_eWBState = eWastedBustedState::WBSTATE_PLAYING;
	m_nPickups = 0;
}

sPlayerSync::sPlayerSync(CVector vPos, uint8 nKeyPresses, CVector vCamFront, CVector vCamSource,
	CVector vCamUp, uint8 nPickups, int8 eWBState, float fCamFov) : sElementSync()
{
	DECLARE_SYNC_CONSTRUCT(this);
	m_vPos = vPos;
	m_nKeyPresses = nKeyPresses;
	m_vCamFront = vCamFront;
	m_vCamSource = vCamSource;
	m_vCamUp = vCamUp;
	m_fCamFov = fCamFov;
	m_eWBState = eWBState;
	m_nPickups = nPickups;
}

// inlined, with copy sElementSync.unk1 = pSync->sElementSync.unk1
sPlayerSync::sPlayerSync(const sPlayerSync& other) : sElementSync(other)
{
	DECLARE_SYNC_CONSTRUCT(this);
	m_vPos = other.m_vPos;
	m_nKeyPresses = other.m_nKeyPresses;
	m_vCamFront = other.m_vCamFront;
	m_vCamSource = other.m_vCamSource;
	m_vCamUp = other.m_vCamUp;
	m_fCamFov = other.m_fCamFov;
	m_eWBState = other.m_eWBState;
	m_nPickups = other.m_nPickups;
}

sPlayerSync::~sPlayerSync() {
	DECLARE_SYNC_DESTRUCT(this);
}

// inlined // not checks: m_fCamFov, m_eWBState, m_nPickups
bool sPlayerSync::Compare(const sPlayerSync& other)
{
	if (m_vPos != other.m_vPos)
		return false;
	if (m_nKeyPresses != other.m_nKeyPresses)
		return false;
	if (m_vCamFront != other.m_vCamFront)
		return false;
	if (m_vCamSource != other.m_vCamSource)
		return false;
	if (m_vCamUp != other.m_vCamUp)
		return false;

	return true;
}

#if !defined(FINAL) && !defined(MASTER)
void sPlayerSync::Dump()
{
	sElementSync::Dump();

	printf("=== sPlayerSync Dump ===\n");
	printf("Position:        (%.3f, %.3f, %.3f)\n", m_vPos.x, m_vPos.y, m_vPos.z);
	printf("Key Presses:     0x%02X\n", m_nKeyPresses);
	printf("Camera Front:    (%.3f, %.3f, %.3f)\n", m_vCamFront.x, m_vCamFront.y, m_vCamFront.z);
	printf("Camera Source:   (%.3f, %.3f, %.3f)\n", m_vCamSource.x, m_vCamSource.y, m_vCamSource.z);
	printf("Camera Up:       (%.3f, %.3f, %.3f)\n", m_vCamUp.x, m_vCamUp.y, m_vCamUp.z);
	printf("Camera FOV:      %.3f\n", m_fCamFov);
	printf("Weapon State:    %d\n", m_eWBState);
	printf("Pickups:         0x%02X\n", m_nPickups);
	printf("================================\n");
}
#endif


sPlayer::sPlayer() {
	DECLARE_ELEMENT_CONSTRUCT(this, true, false);
	m_nBlipIndex = -1;
#ifdef FIX_BUGS
	m_fCamCosHFOV = 0.0f;
	m_fCamSinHFOV = 0.0f;
	m_fCamCosHFOVAspect = 0.0f;
	m_fCamSinHFOVAspect = 0.0f;
#endif
	DECLARE_ELEMENT_CONSTRUCT(this, false, false);
}


ElementCapability sPlayer::GetCapability()
{
	return sPlayer::Capability();
}

bool sPlayer::HasCapability(ElementCapability capability)
{
	if (sPlayer::Capability() == capability)
		return true;
	if (sElement::Capability() == capability)
		return true;
	return false;
}

sPlayer::~sPlayer() {
	DECLARE_ELEMENT_DESTRUCT(this);
	if (m_nBlipIndex != -1)
		TheRadar->ClearBlip(m_nBlipIndex);
	OnPlayerDelete();

	sElement::PurgeAttached();
	// ~sElement
}

sElementSync* sPlayer::CreateSync() {
	return new sPlayerSync();
}

void sPlayer::DisposeSync(sElementSync* pSync) {
	if(pSync)
		delete ((sPlayerSync*)pSync);
}

sElementSync* sPlayer::CreateSyncFromOther(sElementSync* pSync)
{
	sPlayerSync& sync = *(sPlayerSync*)pSync;
	return new sPlayerSync(sync);
}

bool sPlayer::HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB)
{
	sPlayerSync& syncA = *(sPlayerSync*)pSyncA;
	sPlayerSync& syncB = *(sPlayerSync*)pSyncB;
	return syncA.Compare(syncB);
}

void sPlayer::ApplyClientSync(uint16 nTime) {
	sElement::ApplyClientSync(nTime);

	m_nMatrix.GetRight() = CrossProduct(FindSync(nTime, nil).player->m_vCamFront, FindSync(nTime, nil).player->m_vCamUp);
	m_nMatrix.GetForward() = FindSync(nTime, nil).player->m_vCamFront;
	m_nMatrix.GetUp() = FindSync(nTime, nil).player->m_vCamUp;
	m_nMatrix.GetPosition() = FindSync(nTime, nil).player->m_vCamSource;

	// CCamera::CalculateDerivedValues()
	CMatrix cameraMatrix = Invert(m_nMatrix);
	m_nMatrix = cameraMatrix;
	float hfov = DEGTORAD(FindSync(nTime, nil).player->m_fCamFov) / 2.0f;
	float c = cosf(hfov);
	float s = sinf(hfov);
	m_fCamCosHFOV = c;
	m_fCamSinHFOV = s;
	CDraw::CalculateAspectRatio(); // not in psp
	c /= SCREEN_ASPECT_RATIO;
	s /= SCREEN_ASPECT_RATIO;
	m_fCamCosHFOVAspect = c;
	m_fCamSinHFOVAspect = s;
}

void sPlayer::Update(uint16 nTime) {
	CPlayerInfo& pInfo = CWorld::Players[CWorld::PlayerInFocus];
	uint8 nKeyPresses = 0;
	CPad* pPad = CPad::GetPad(0);
	if (pPad->GetHorn())
		nKeyPresses |= ePlayerPressKey::HORN;
	if (pPad->GetExitVehicleForScript())
		nKeyPresses |= ePlayerPressKey::EXIT_VEHICLE;
	if (pPad->GetLookLeft())
		nKeyPresses |= ePlayerPressKey::LOOK_LEFT;
	if (pPad->GetLookRight())
		nKeyPresses |= ePlayerPressKey::LOOK_RIGHT;
	CCam& pCam = TheCamera.Cams[TheCamera.ActiveCam];
	uint8 nPickups = *FindPlayerPed()->GetPickupsBeingCarried();
	AttachSync(nTime, new sPlayerSync(pInfo.GetPos(), nKeyPresses, pCam.Front, pCam.Source, pCam.Up, nPickups, pInfo.m_WBState, CDraw::GetFOV()));
}

bool sPlayer::WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime)
{
	sPlayerSync* pSync = FindSync(nSyncWriteTime, nil).player;
	if (static_cast<int16>(nSyncLastTime - m_vSync.front().m_nTime) >= 0) {
		return WriteSyncDelta(pSyncStream, pSync, GetSyncWithTime(nSyncLastTime).player);
	}

#ifdef FIX_BUGS
	PerformWriteSync(pSyncStream, pSync, ePlayerSync::MP_PKTD_PLR_FULL);
#else
	// missed MP_PKTD_PLR_PICK MP_PKTD_PLR_STAT (bit 8, 9)
	PerformWriteSync(pSyncStream, pSync, 0xFF); // max diff // bug 255, 0xFF, not 0xFFFF
#endif
	return true;
}

void sPlayer::ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync)
{
	sPlayerSync& sync = *(sPlayerSync*)pOutSync;

	uint16 nDiffMask = pSyncStream->ReadU16();

	if (nDiffMask & ePlayerSync::MP_PKTD_PLR_POSX)
		sync.m_vPos.x = pSyncStream->ReadFloat();

	if (nDiffMask & ePlayerSync::MP_PKTD_PLR_POSY)
		sync.m_vPos.y = pSyncStream->ReadFloat();

	if (nDiffMask & ePlayerSync::MP_PKTD_PLR_POSZ)
		sync.m_vPos.z = pSyncStream->ReadFloat();

	if (nDiffMask & ePlayerSync::MP_PKTD_PLR_KEYS)
		sync.m_nKeyPresses = pSyncStream->ReadU8();

	if (nDiffMask & ePlayerSync::MP_PKTD_PLR_CAMS)
		sync.m_vCamSource = pSyncStream->ReadVector();

	if (nDiffMask & ePlayerSync::MP_PKTD_PLR_CAMF)
		sync.m_vCamFront = pSyncStream->ReadVector();

	if (nDiffMask & ePlayerSync::MP_PKTD_PLR_CAMU)
		sync.m_vCamUp = pSyncStream->ReadVector();

	if (nDiffMask & ePlayerSync::MP_PKTD_PLR_CAMV)
		sync.m_fCamFov = pSyncStream->ReadFloat();

	if (nDiffMask & ePlayerSync::MP_PKTD_PLR_PICK)
		sync.m_nPickups = pSyncStream->ReadU8();

	if (nDiffMask & ePlayerSync::MP_PKTD_PLR_STAT)
		sync.m_eWBState = pSyncStream->ReadI16();
}

void sPlayer::RegisterSelfWithOwner(uint8 owner, uint16 id)
{
	cMultiGame& pGame = cMultiGame::Instance();
	sElement::RegisterSelfWithOwner(owner, id);
	if (pGame.IsElementOwnerLocalPlayer(this))
		return;

	m_nBlipIndex = TheRadar->SetEntityBlip(BLIP_MULTIGAME, (GetOwner() << 16), RADAR_TRACE_GREEN, BLIP_DISPLAY_BOTH); // handle
	if (pGame.GetGameType() == eGameType::HITPARADE) {
		TheRadar->SetBlipMultiplayerState(m_nBlipIndex, true);
	}
	TheRadar->SetBlipSprite(m_nBlipIndex, RADAR_SPRITE_MP_PLAYER);
	TheRadar->ChangeBlipScale(m_nBlipIndex, 3);
}


CVector& sPlayer::GetPosition() {
	cMultiGame& Game = cMultiGame::Instance();
	sElement* pElem = Game.GetEntityForHandle(GetOwner(), eElementID::MG_ELEMENT_PLAYER_PED_ID);
	return pElem ? pElem->GetSync().ped->GetMatrix().GetPosition() : GetSync().player->m_vPos;
}

bool sPlayer::isPressingHorn() {
	return (GetSync().player->m_nKeyPresses & ePlayerPressKey::HORN);
}

bool sPlayer::isPressingExitVehicle() {
	return (GetSync().player->m_nKeyPresses & ePlayerPressKey::EXIT_VEHICLE);
}

bool sPlayer::isPositionInRadius(const CVector& pos, float radius, float maxDist)
{
	float fMaxDistSqr = maxDist == 0.0f ? SQR(CDraw::GetFarClipZ()) : SQR(maxDist);
	CVector playerPos = GetPosition();
	float fDistSqr = (pos - playerPos).MagnitudeSqr();
	if (fDistSqr > fMaxDistSqr)
		return false;

	// CCamera::IsSphereVisible()
	CVector c;
#ifdef GTA_PS2
	TransformPoint(c, m_nMatrix, pos);
#else
	c = pos;
#ifdef FIX_BUGS
	c = m_nMatrix * pos;
#else
	RwV3dTransformPoints(&c, &c, 1, (RwMatrix*)&m_nMatrix);
#endif
#endif

	if (c.y + radius < CDraw::GetNearClipZ())
		return false;
	if (c.y - radius > CDraw::GetFarClipZ())
		return false;

	float dot1 = c.x * m_fCamCosHFOV + c.y * (-m_fCamSinHFOV);
	if (dot1 > radius)
		return false;

	float dot2 = c.x * (-m_fCamCosHFOV) + c.y * (-m_fCamSinHFOV);
	if (dot2 > radius)
		return false;

	float dot3 = c.y * (-m_fCamCosHFOVAspect) + c.z * (-m_fCamSinHFOVAspect);
	if (dot3 > radius)
		return false;

	float dot4 = c.y * (-m_fCamCosHFOVAspect) + c.z * m_fCamSinHFOVAspect;
	if (dot4 > radius)
		return false;

	return true;
}

bool sPlayer::IsEntityInRadius(CEntity* ent, float maxDist) {
	return isPositionInRadius(ent->GetPosition(), ent->GetColModel()->boundingSphere.radius, maxDist);
}

sPlayer* sPlayer::GetLockOnTarget()
{
	cMultiGame& Game = cMultiGame::Instance();
	int32 nOwnerID = GetOwnerID();
#ifdef GTA_LIBERTY
	for (int32 i = 0; i < Game.m_vPlayers.size(); ++i) {
#else
	for (int32 i = 0; i < PeerManager.m_vPlayers.size(); ++i) {
#endif
		sPeerState* peer = PeerManager.GetPeerAt(i);
		if (!peer) continue; // slot free
		sPlayer* player = Game.GetPlayer(peer->m_nID);
		if (!player) continue; // not have element
		if (peer->m_nID == Game.LocalPlayerID()) continue; // skip self
		sElement* entity = Game.GetEntityForHandle(player->GetOwnerID(), eElementID::MG_ELEMENT_PLAYER_PED_ID);
		if (!entity) continue;
		sPedSync* pedSync = entity->GetSync().ped;
		if (pedSync && pedSync->m_nPeerLockOnMG == nOwnerID)
			return player;
	}
	return nil;
}

inline uint16 sPlayer::CompareSyncState(sPlayerSync* pSync, sPlayerSync* pLastSync)
{
	// maybe smth for FLT_EPS_NOT_EQ?
	const float POSITION_THRESHOLD = 0.5f;
	const float CAM_DIR_THRESHOLD_SQR = SQR(0.05f);
	const float CAM_POS_THRESHOLD_SQR = SQR(0.2f);

	uint16 nDiffMask = ePlayerSync::MP_PKTD_PLR_EQUAL;

	if (fabsf(pLastSync->m_vPos.x - pSync->m_vPos.x) > POSITION_THRESHOLD)
		nDiffMask |= ePlayerSync::MP_PKTD_PLR_POSX;
	else // !!---- UPD!!
		pSync->m_vPos.x = pLastSync->m_vPos.x;

	if (fabsf(pLastSync->m_vPos.y - pSync->m_vPos.y) > POSITION_THRESHOLD)
		nDiffMask |= ePlayerSync::MP_PKTD_PLR_POSY;
	else // !!---- UPD!!
		pSync->m_vPos.y = pLastSync->m_vPos.y;

	if (fabsf(pLastSync->m_vPos.z - pSync->m_vPos.z) > POSITION_THRESHOLD)
		nDiffMask |= ePlayerSync::MP_PKTD_PLR_POSZ;
	else // !!---- UPD!!
		pSync->m_vPos.z = pLastSync->m_vPos.z;

	if (pLastSync->m_nKeyPresses != pSync->m_nKeyPresses)
		nDiffMask |= ePlayerSync::MP_PKTD_PLR_KEYS;

	if ((pLastSync->m_vCamFront - pSync->m_vCamFront).MagnitudeSqr() > CAM_DIR_THRESHOLD_SQR)
		nDiffMask |= ePlayerSync::MP_PKTD_PLR_CAMF;
	else // !!---- UPD!!
		pSync->m_vCamFront = pLastSync->m_vCamFront;

	if ((pLastSync->m_vCamUp - pSync->m_vCamUp).MagnitudeSqr() > CAM_DIR_THRESHOLD_SQR)
		nDiffMask |= ePlayerSync::MP_PKTD_PLR_CAMU;
	else // !!---- UPD!!
		pSync->m_vCamUp = pLastSync->m_vCamUp;

	if ((pLastSync->m_vCamSource - pSync->m_vCamSource).MagnitudeSqr() > CAM_POS_THRESHOLD_SQR)
		nDiffMask |= ePlayerSync::MP_PKTD_PLR_CAMS;
	else // !!---- UPD!!
		pSync->m_vCamSource = pLastSync->m_vCamSource;

	if (pLastSync->m_fCamFov != pSync->m_fCamFov)
		nDiffMask |= ePlayerSync::MP_PKTD_PLR_CAMV;

	if (pLastSync->m_nPickups != pSync->m_nPickups || FindPlayerPed()->HasUberPickup())
		nDiffMask |= ePlayerSync::MP_PKTD_PLR_PICK;

	if (pLastSync->m_eWBState != pSync->m_eWBState)
		nDiffMask |= ePlayerSync::MP_PKTD_PLR_STAT;

	return nDiffMask;
}

bool sPlayer::WriteSyncDelta(sWriteSyncStream* pSyncStream, sPlayerSync* pSync, sPlayerSync* pLastSync)
{
	uint16 nDiffMask = CompareSyncState(pSync, pLastSync);

	if (nDiffMask == ePlayerSync::MP_PKTD_PLR_EQUAL)
		return false;

	PerformWriteSync(pSyncStream, pSync, nDiffMask);
	return true;
}

void sPlayer::PerformWriteSync(sWriteSyncStream* pSyncStream, sPlayerSync* pSync, uint16 nDiffMask)
{
	pSyncStream->WriteU16(nDiffMask);

	if (nDiffMask & ePlayerSync::MP_PKTD_PLR_POSX)
		pSyncStream->WriteFloat(pSync->m_vPos.x);

	if (nDiffMask & ePlayerSync::MP_PKTD_PLR_POSY)
		pSyncStream->WriteFloat(pSync->m_vPos.y);

	if (nDiffMask & ePlayerSync::MP_PKTD_PLR_POSZ)
		pSyncStream->WriteFloat(pSync->m_vPos.z);

	if (nDiffMask & ePlayerSync::MP_PKTD_PLR_KEYS)
		pSyncStream->WriteU8(pSync->m_nKeyPresses);

	if (nDiffMask & ePlayerSync::MP_PKTD_PLR_CAMS)
		pSyncStream->WriteVector(pSync->m_vCamSource);

	if (nDiffMask & ePlayerSync::MP_PKTD_PLR_CAMF)
		pSyncStream->WriteVector(pSync->m_vCamFront);

	if (nDiffMask & ePlayerSync::MP_PKTD_PLR_CAMU)
		pSyncStream->WriteVector(pSync->m_vCamUp);

	if (nDiffMask & ePlayerSync::MP_PKTD_PLR_CAMV)
		pSyncStream->WriteFloat(pSync->m_fCamFov);

	if (nDiffMask & ePlayerSync::MP_PKTD_PLR_PICK)
		pSyncStream->WriteU8(pSync->m_nPickups);

	if (nDiffMask & ePlayerSync::MP_PKTD_PLR_STAT)
		pSyncStream->WriteI16(pSync->m_eWBState);
}
