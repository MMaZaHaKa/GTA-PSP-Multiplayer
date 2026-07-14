/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"
#include "World.h"
#include "Radar.h"

#include "multiplayer/elements/sElementPhysical.h"
#include "multiplayer/elements/sSyncStream.h"
#include "multiplayer/MultiGame.h"

sElementPhysicalSync::sElementPhysicalSync() : sElementSync()
{
	m_matrix = CMatrix();
	m_vecMoveSpeed = CVector(0, 0, 0);
	m_vecTurnSpeed = CVector(0, 0, 0);
	m_vecMoveFriction = CVector(0, 0, 0);
	m_vecTurnFriction = CVector(0, 0, 0);
	m_nPhys_lvcs_unk_flagsB = 0;
	m_matrix.SetUnity();
}

sElementPhysicalSync::sElementPhysicalSync(CPhysical* source) : sElementSync()
{
	m_matrix = CMatrix(source->GetMatrix());
	m_vecMoveSpeed = source->m_vecMoveSpeed;
	m_vecTurnSpeed = source->m_vecTurnSpeed;
	m_vecMoveFriction = source->m_vecMoveFriction;
	m_vecTurnFriction = source->m_vecTurnFriction;
	m_nPhys_lvcs_unk_flagsB = source->m_nPhys_lvcs_unk_flagsB;
	if (source->b4E_8)
		b154_20 = true;
}

// inlined
sElementPhysicalSync::sElementPhysicalSync(const sElementPhysicalSync& other) : sElementSync(other)
{
	m_matrix = CMatrix(other.m_matrix);
	m_vecMoveSpeed = other.m_vecMoveSpeed;
	m_vecTurnSpeed = other.m_vecTurnSpeed;
	m_vecMoveFriction = other.m_vecMoveFriction;
	m_vecTurnFriction = other.m_vecTurnFriction;
	m_nPhys_lvcs_unk_flagsB = other.m_nPhys_lvcs_unk_flagsB;
}

sElementPhysicalSync::~sElementPhysicalSync() { }

bool sElementPhysicalSync::Compare(const sElementPhysicalSync& other)
{
	if (m_matrix != other.m_matrix)
		return false;
	if (m_vecMoveSpeed != other.m_vecMoveSpeed)
		return false;
	if (m_vecTurnSpeed != other.m_vecTurnSpeed)
		return false;
	if (m_vecMoveFriction != other.m_vecMoveFriction)
		return false;
	if (m_vecTurnFriction != other.m_vecTurnFriction)
		return false;
	if (m_nPhys_lvcs_unk_flagsB != other.m_nPhys_lvcs_unk_flagsB)
		return false;

	return true;
}

#if !defined(FINAL) && !defined(MASTER)
void sElementPhysicalSync::Dump()
{
	sElementSync::Dump();

	printf("=== sElementPhysicalSync Dump ===\n");
	printf("Matrix:\n");
	printf("  Right:  (%.3f, %.3f, %.3f)\n", m_matrix.GetRight().x, m_matrix.GetRight().y, m_matrix.GetRight().z);
	printf("  Up:     (%.3f, %.3f, %.3f)\n", m_matrix.GetUp().x, m_matrix.GetUp().y, m_matrix.GetUp().z);
	printf("  At:     (%.3f, %.3f, %.3f)\n", m_matrix.GetForward().x, m_matrix.GetForward().y, m_matrix.GetForward().z);
	printf("  Pos:    (%.3f, %.3f, %.3f)\n", m_matrix.GetPosition().x, m_matrix.GetPosition().y, m_matrix.GetPosition().z);
	printf("Move Speed:     (%.3f, %.3f, %.3f)\n", m_vecMoveSpeed.x, m_vecMoveSpeed.y, m_vecMoveSpeed.z);
	printf("Turn Speed:     (%.3f, %.3f, %.3f)\n", m_vecTurnSpeed.x, m_vecTurnSpeed.y, m_vecTurnSpeed.z);
	printf("Move Friction:  (%.3f, %.3f, %.3f)\n", m_vecMoveFriction.x, m_vecMoveFriction.y, m_vecMoveFriction.z);
	printf("Turn Friction:  (%.3f, %.3f, %.3f)\n", m_vecTurnFriction.x, m_vecTurnFriction.y, m_vecTurnFriction.z);
	printf("Phys LVCS Flags B: 0x%02X\n", m_nPhys_lvcs_unk_flagsB);
	printf("  b154_1:     %d\n", b154_1);
	printf("  bHasBlipPhys:%d\n", bHasBlipPhys);
	printf("  b154_4:     %d\n", b154_4);
	printf("  bNoRadarForEnemy:%d\n", bNoRadarForEnemy);
	printf("  b154_10:    %d\n", b154_10);
	printf("  b154_20:    %d\n", b154_20);
	printf("  b154_40:    %d\n", b154_40);
	printf("  b154_80:    %d\n", b154_80);
	printf("================================\n");
}
#endif


cPhysicalMG::cPhysicalMG(sElement* elem) : CPhysical() {
	m_pElem.element = elem;
	m_type = eEntityType::ENTITY_TYPE_MULTIPLAYER;
	bInfiniteMass = true;
	m_phy_flagA08 = true;
	bUsesCollision = true;
	AddToMovingList();
#ifdef MULTIGAME_ELEMENTS_IMPROVEMENTS
	bIsObject = false;
#endif
}

//#ifdef FIX_BUGS
//cPhysicalMG::cPhysicalMG(CPhysical* pPhysical) {
//	m_type = eEntityType::ENTITY_TYPE_MULTIPLAYER;
//	//bUsesCollision = true;
//
//	SetMatrix(pPhysical->GetMatrix());
//	SetMoveSpeed(pPhysical->m_vecMoveSpeed);
//	m_vecTurnSpeed = pPhysical->m_vecTurnSpeed;
//	m_vecMoveFriction = pPhysical->m_vecMoveFriction;
//	m_vecTurnFriction = pPhysical->m_vecTurnFriction;
//}
//#endif

cPhysicalMG::~cPhysicalMG() {
	TheRadar->RemoveMultiplayerMarker(GetElement().element->GetOwner(), GetElement().element->GetID());
}

void cPhysicalMG::ProcessControl(void) {
	CPhysical::ProcessControl();
}

void cPhysicalMG::Render(void) {
	sElementPhysicalSync* pSync = GetElement().element->GetSync().elementphysical;
	SetMatrix(pSync->GetMatrix());
	CEntity::UpdateRwFrame();
	CEntity::Render();
}

void cPhysicalMG::ApplyMoveSpeed(void) {
	sElementPhysicalSync* pSync = GetElement().element->GetSync().elementphysical;
	CVector vPos = pSync->GetMatrix().GetPosition();
	SetPosition(vPos.x, vPos.y, vPos.z);
}

void cPhysicalMG::ApplyTurnSpeed(void) {
	sElementPhysicalSync* pSync = GetElement().element->GetSync().elementphysical;
	GetMatrix().GetRight() = pSync->GetMatrix().GetRight();
	GetMatrix().GetUp() = pSync->GetMatrix().GetUp();
	GetMatrix().GetForward() = pSync->GetMatrix().GetForward();
}

int32 cPhysicalMG::ProcessEntityCollision(CEntity* ent, CColPoint* colpoints) {
	CModelInfo::GetModelInfo(GetModelIndex())->GetColModel()->numLines = 0;
	return CPhysical::ProcessEntityCollision(ent, colpoints);
}


sElementPhysical::sElementPhysical() : sElement() {
	m_pPhyElem = nil;
	m_bWasPhyTransfered = false;
#ifndef GTA_LIBERTY
	m_nAcks = 0;
#endif
}


ElementCapability sElementPhysical::GetCapability()
{
	return sElementPhysical::Capability();
}

bool sElementPhysical::HasCapability(ElementCapability capability)
{
	if (sElementPhysical::Capability() == capability)
		return true;
	if (sElement::Capability() == capability)
		return true;
	return false;
}

sElementPhysical::~sElementPhysical() {
	if (m_pPhyElem) {
		CWorld::Remove(m_pPhyElem, eWorldRemoveType::WORLD_REMOVE_WITH_CLEANUP_VEHICLES);
		m_pPhyElem->RemoveFromMovingList();
		delete m_pPhyElem;
#ifdef FIX_BUGS
		SetPhysical(nil);
#endif
	}
}

void sElementPhysical::ApplyClientSync(uint16 nTime)
{
	sElement::ApplyClientSync(nTime);
	sElementPhysicalSync* pSync = GetSync().elementphysical;

	assert(GetPhysical());
	if ((!cMultiGame::Instance().IsElementOwnerLocalPlayer(this)) && GetPhysical()->m_entryInfoList.first) // IsMultiplayerPointerValid check?
	{
		CVector vZero = CVector(0.0f, 0.0f, 0.0f);
		GetPhysical()->SetMoveSpeed(vZero);
		GetPhysical()->m_vecTurnSpeed = vZero;
		GetPhysical()->m_vecMoveFriction = vZero;
		GetPhysical()->m_vecTurnFriction = vZero;
		if (GetPhysical()->GetStatus() == eEntityStatus::STATUS_SIMPLE)
			GetPhysical()->SetMatrix(pSync->GetMatrix());
	}
	else
	{
		GetPhysical()->SetMatrix(pSync->GetMatrix());
		GetPhysical()->SetMoveSpeed(pSync->GetMoveSpeed());
		GetPhysical()->m_vecTurnSpeed = pSync->GetTurnSpeed();
		GetPhysical()->UpdateRwFrame();
		GetPhysical()->RemoveAndAdd();
	}

	TransferZone();
	if (GetPhysical()->GetStatus() == eEntityStatus::STATUS_PHYSICS && GetPhysical()->bUsesCollision)
		++ms_nNumberOfSyncedPhysicals;

	if (!cMultiGame::Instance().IsElementOwnerLocalPlayer(this) && IsTransferable() &&
		GetPhysical()->GetStatus() == eEntityStatus::STATUS_PHYSICS && !m_bWasPhyTransfered)
		TransferPhysicalEntity();
}

void sElementPhysical::ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) {
#ifndef GTA_LIBERTY
	m_nAcks = 0;
#endif
	sElementPhysicalSync* pSync = GetSync().elementphysical;
	sElement::ReceiveEntity(nOwner, nID, nTime);

	assert(GetPhysical());
	if (!cMultiGame::Instance().IsElementOwnerLocalPlayer(this))
	{
		GetPhysical()->bInfiniteMass = true;
		GetPhysical()->m_phy_flagA08 = true;
		GetPhysical()->bUsesCollision = true;
		GetPhysical()->AddToMovingList();
	}
	else
	{
		GetPhysical()->bUsesCollision = false;
		GetPhysical()->RemoveFromMovingList();
		if (GetEntity())
			((CPhysical*)GetEntity())->m_nPhys_lvcs_unk_flagsB = pSync->GetFlags();
		if(GetPhysical()) // what??
			GetPhysical()->m_nPhys_lvcs_unk_flagsB = pSync->GetFlags();
	}
	TransferZone();
}

void sElementPhysical::ApplyDeltaState(sElementSync* pSync, uint16 nTimeDelta) {
	;
}

void sElementPhysical::InterpolateDeltaState(sElementSync* pSyncA, sElementSync* pSyncB, float fDelta) {	
	sElementPhysicalSync& syncA = *(sElementPhysicalSync*)pSyncA;
	sElementPhysicalSync& syncB = *(sElementPhysicalSync*)pSyncB;
	float alpha = 1.0f - fDelta;

	CVector& posA = syncA.GetMatrix().GetPosition();
	const CVector& posB = syncB.GetMatrix().GetPosition();
	posA = posA * alpha + posB * fDelta;

	CVector& forwardA = syncA.GetMatrix().GetForward();
	const CVector& forwardB = syncB.GetMatrix().GetForward();
	forwardA = forwardA * alpha + forwardB * fDelta;

	CVector& rightA = syncA.GetMatrix().GetRight();
	const CVector& rightB = syncB.GetMatrix().GetRight();
	rightA = rightA * alpha + rightB * fDelta;

	if (forwardA.MagnitudeSqr() <= 0.0001f || rightA.MagnitudeSqr() <= 0.0001f)
		syncA.GetMatrix().SetUnity();

	syncA.GetMatrix().Reorthogonalise();
}

void sElementPhysical::RegisterSelf() {
	GetPhysical()->bUsesCollision = false;
	sElement::RegisterSelf();
}

void sElementPhysical::TransferPhysicalEntity() {
	m_bWasPhyTransfered = true;
	cMultiGame::Instance().TransferEntity(this);
}


uint8 sElementPhysical::CompareSyncState(sElementPhysicalSync* pSync, sElementPhysicalSync* pLastSync) {
	// maybe smth for FLT_EPS_NOT_EQ?
	const float MATRIX_THRESHOLD_SQR = SQR(0.01f); // 0.0001f
	const float POSITION_THRESHOLD_SQR = 0.001f;
	const float SPEED_THRESHOLD_SQR = 0.00001f;

	uint8 nDiffMask = ePhysicalSync::MP_PKTD_PHY_EQUAL;

	if ((pLastSync->GetMatrix().GetUp() - pSync->GetMatrix().GetUp()).MagnitudeSqr() > MATRIX_THRESHOLD_SQR)
		nDiffMask |= ePhysicalSync::MP_PKTD_PHY_MATRIX;
	else if ((pLastSync->GetMatrix().GetForward() - pSync->GetMatrix().GetForward()).MagnitudeSqr() > MATRIX_THRESHOLD_SQR)
		nDiffMask |= ePhysicalSync::MP_PKTD_PHY_MATRIX;
	else { // !!---- UPD!!
		pSync->GetMatrix().GetRight() = pLastSync->GetMatrix().GetRight();
		pSync->GetMatrix().GetForward() = pLastSync->GetMatrix().GetForward();
		pSync->GetMatrix().GetUp() = pLastSync->GetMatrix().GetUp();
	}

	if ((pLastSync->GetMatrix().GetPosition() - pSync->GetMatrix().GetPosition()).MagnitudeSqr() > POSITION_THRESHOLD_SQR)
		nDiffMask |= ePhysicalSync::MP_PKTD_PHY_POSITION;
	else // !!---- UPD!!
		pSync->GetMatrix().GetPosition() = pLastSync->GetMatrix().GetPosition();

	if ((pLastSync->GetMoveSpeed() - pSync->GetMoveSpeed()).MagnitudeSqr() > SPEED_THRESHOLD_SQR)
		nDiffMask |= ePhysicalSync::MP_PKTD_PHY_MOVE_SPEED;
	else // !!---- UPD!!
		pSync->SetMoveSpeed(pLastSync->GetMoveSpeed());

	if ((pLastSync->GetTurnSpeed() - pSync->GetTurnSpeed()).MagnitudeSqr() > SPEED_THRESHOLD_SQR)
		nDiffMask |= ePhysicalSync::MP_PKTD_PHY_TURN_SPEED;
	else // !!---- UPD!!
		pSync->SetTurnSpeed(pLastSync->GetTurnSpeed());

	if (pLastSync->GetMoveFriction() != pSync->GetMoveFriction())
		nDiffMask |= ePhysicalSync::MP_PKTD_PHY_MOVE_FRICT;

	if (pLastSync->GetTurnFriction() != pSync->GetTurnFriction())
		nDiffMask |= ePhysicalSync::MP_PKTD_PHY_TURN_FRICT;

	if (GetPhysical()) {
		if (pLastSync->GetFlags() != pSync->GetFlags())
			nDiffMask |= ePhysicalSync::MP_PKTD_PHY_FLAGS;
	}

	return nDiffMask;
}

void sElementPhysical::PerformWriteSync(sWriteSyncStream* pSyncStream, sElementPhysicalSync* pSync, uint8 nDiffMask) {
	pSyncStream->WriteU8(nDiffMask);

	if (nDiffMask & ePhysicalSync::MP_PKTD_PHY_MATRIX) {
		pSyncStream->WriteVector(pSync->GetMatrix().GetRight());
		pSyncStream->WriteVector(pSync->GetMatrix().GetForward());
		pSyncStream->WriteVector(pSync->GetMatrix().GetUp());
	}

	if (nDiffMask & ePhysicalSync::MP_PKTD_PHY_POSITION)
		pSyncStream->WriteVector(pSync->GetMatrix().GetPosition());

	if (nDiffMask & ePhysicalSync::MP_PKTD_PHY_MOVE_SPEED)
		pSyncStream->WriteVector(pSync->GetMoveSpeed());

	if (nDiffMask & ePhysicalSync::MP_PKTD_PHY_TURN_SPEED)
		pSyncStream->WriteVector(pSync->GetTurnSpeed());

	if (nDiffMask & ePhysicalSync::MP_PKTD_PHY_MOVE_FRICT)
		pSyncStream->WriteVector(pSync->GetMoveFriction());

	if (nDiffMask & ePhysicalSync::MP_PKTD_PHY_TURN_FRICT)
		pSyncStream->WriteVector(pSync->GetTurnFriction());

	if (nDiffMask & ePhysicalSync::MP_PKTD_PHY_PHYELEM && GetPhysical()) {
		pSyncStream->WriteFloat(GetPhysical()->m_fMass);
		pSyncStream->WriteFloat(GetPhysical()->m_fTurnMass);
		pSyncStream->WriteFloat(GetPhysical()->m_fForceMultiplier);
		pSyncStream->WriteFloat(GetPhysical()->m_fAirResistance);
		pSyncStream->WriteFloat(GetPhysical()->m_fElasticity);
		pSyncStream->WriteFloat(GetPhysical()->m_fBuoyancy);
		pSyncStream->WriteVector(GetPhysical()->m_vecCentreOfMass);
	}

	if (nDiffMask & ePhysicalSync::MP_PKTD_PHY_FLAGS)
		pSyncStream->WriteU8(pSync->GetFlags());
}

void sElementPhysical::ReadSyncFromStreamPhysical(sReadSyncStream* pSyncStream, sElementPhysicalSync* pOutSync) {
	sElementPhysicalSync& sync = *pOutSync;

	uint8 nDiffMask = pSyncStream->ReadU8();

	if (nDiffMask & ePhysicalSync::MP_PKTD_PHY_MATRIX) {
		sync.GetMatrix().GetRight() = pSyncStream->ReadVector();
		sync.GetMatrix().GetForward() = pSyncStream->ReadVector();
		sync.GetMatrix().GetUp() = pSyncStream->ReadVector();
	}

	if (nDiffMask & ePhysicalSync::MP_PKTD_PHY_POSITION)
		sync.GetMatrix().GetPosition() = pSyncStream->ReadVector();

	if (nDiffMask & ePhysicalSync::MP_PKTD_PHY_MOVE_SPEED)
		sync.SetMoveSpeed(pSyncStream->ReadVector());

	if (nDiffMask & ePhysicalSync::MP_PKTD_PHY_TURN_SPEED)
		sync.SetTurnSpeed(pSyncStream->ReadVector());

	if (nDiffMask & ePhysicalSync::MP_PKTD_PHY_MOVE_FRICT)
		sync.SetMoveFriction(pSyncStream->ReadVector());

	if (nDiffMask & ePhysicalSync::MP_PKTD_PHY_TURN_FRICT)
		sync.SetTurnFriction(pSyncStream->ReadVector());

	if (nDiffMask & ePhysicalSync::MP_PKTD_PHY_PHYELEM && GetPhysical()) {
		GetPhysical()->m_fMass = pSyncStream->ReadFloat();
		GetPhysical()->m_fTurnMass = pSyncStream->ReadFloat();
		GetPhysical()->m_fForceMultiplier = pSyncStream->ReadFloat();
		GetPhysical()->m_fAirResistance = pSyncStream->ReadFloat();
		GetPhysical()->m_fElasticity = pSyncStream->ReadFloat();
		GetPhysical()->m_fBuoyancy = pSyncStream->ReadFloat();
		GetPhysical()->m_vecCentreOfMass = pSyncStream->ReadVector();
	}

	if (nDiffMask & ePhysicalSync::MP_PKTD_PHY_FLAGS) {
		uint8 flagsB = pSyncStream->ReadU8();
		assert(GetPhysical());
		GetPhysical()->m_nPhys_lvcs_unk_flagsB = flagsB;
		GetPhysical()->b4E_8 = false;

		if (GetOwner() == cMultiGame::Instance().LocalPlayerID()) {
			assert(GetEntity());
			((CPhysical*)GetEntity())->m_nPhys_lvcs_unk_flagsB = flagsB;
		}
		sync.SetFlags(flagsB);
	}
}

bool sElementPhysical::HasAckPeerID(uint32 nPeerID) {
	if (nPeerID == TheMPGame.LocalPlayerID())
		return true;

	assert(nPeerID < MP_NUM_PEERS);
	return (m_nAcks & (1 << (nPeerID & 0x1F))) != 0;
}

bool sElementPhysical::HasAcksFromAllPeers() {
	for (int32 nPlayerID = 0; nPlayerID < PeerManager.m_vPlayers.size(); ++nPlayerID)
	{
		sPeerState* peer = PeerManager.GetPeerAt(nPlayerID);
		if (!HasAckPeerID(peer->m_nID))
			return false;
	}
	return true;
}

void sElementPhysical::AddAckPeerID(uint32 nPeerID) {
	assert(nPeerID < MP_NUM_PEERS);
	m_nAcks |= (1 << (nPeerID & 0x1F));
}

void sElementPhysical::TransferZone() {
	if (!cMultiGame::Instance().IsElementOwnerLocalPlayer(this) || m_pZone)
		return;

	cInterestZone* pZone = cMultiGame::Instance().m_ZoneManager.GetZoneByPeer(MP_HOST_INDEX);
	if (pZone)
		pZone->RegisterElement(this);
}

int32 sElementPhysical::ms_nNumberOfSyncedPhysicals = 0;
