/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "multiplayer/elements/sObject.h"
#include "multiplayer/elements/sSyncStream.h"
#include "multiplayer/MultiGame.h"
#include "World.h"
#include "Zones.h"

#ifdef MULTIGAME_ELEMENTS_IMPROVEMENTS

sObjectSync::sObjectSync() : sElementPhysicalSync() {
	DECLARE_SYNC_CONSTRUCT(this);
}

sObjectSync::sObjectSync(CObject* pObject) : sElementPhysicalSync(pObject) {
	DECLARE_SYNC_CONSTRUCT(this);
}

sObjectSync::sObjectSync(const sObjectSync& other) : sElementPhysicalSync(other) {
	DECLARE_SYNC_CONSTRUCT(this);
}

sObjectSync::~sObjectSync() {
	DECLARE_SYNC_DESTRUCT(this);
}

bool sObjectSync::Compare(const sObjectSync& other)
{
	if (!sElementPhysicalSync::Compare(other))
		return false;

	if (m_nHealth != other.m_nHealth)
		return false;

	return true;
}

#if !defined(FINAL) && !defined(MASTER)
void sObjectSync::Dump()
{
	sElementPhysicalSync::Dump();

	printf("=== sObjectSync Dump ===\n");
	printf("  Health:     %d\n", m_nHealth);
	printf("================================\n");
}
#endif

cObjectMG::cObjectMG(sElement* elem) : cPhysicalMG(elem)
{
	bIsObject = true;
}

cObjectMG::~cObjectMG()
{

}

void cObjectMG::PreRender(void)
{
	cPhysicalMG::PreRender();
}

void cObjectMG::Render(void)
{
	cPhysicalMG::Render();
}


sObject::sObject() : sElementPhysical()
{
	DECLARE_ELEMENT_CONSTRUCT(this, true, false);
	SetPhysical(new cObjectMG(this));
	bVanilaCompatible = false;
	DECLARE_ELEMENT_CONSTRUCT(this, false, false);
}

sObject::sObject(CObject* pObject) : sElementPhysical(/*pObject*/)
{
	DECLARE_ELEMENT_CONSTRUCT(this, true, true);
	cMultiGame::Instance().AttachEntity(this, pObject);
	SetEntity(pObject);
	SetPhysical(new cObjectMG(this));
	//Initialise();
	m_aColours[VEHICLE_COLOUR_PRIMARY] = pObject->m_aColours[VEHICLE_COLOUR_PRIMARY];
	m_aColours[VEHICLE_COLOUR_SECONDARY] = pObject->m_aColours[VEHICLE_COLOUR_SECONDARY];
	AttachSync(m_nTime, new sObjectSync(pObject));
	TransferZone();
	bVanilaCompatible = false;
	DECLARE_ELEMENT_CONSTRUCT(this, false, true);
}

ElementCapability sObject::GetCapability()
{
	return sObject::Capability();
}

bool sObject::HasCapability(ElementCapability capability)
{
	if (sObject::Capability() == capability)
		return true;
	if (sElementPhysical::Capability() == capability)
		return true;
	if (sElement::Capability() == capability)
		return true;
	return false;
}

sObject::~sObject()
{
	DECLARE_ELEMENT_DESTRUCT(this);
	sElement::PurgeAttached();
	// ~sElement
}

sElementSync* sObject::CreateSync() {
	return new sObjectSync();
}

void sObject::DisposeSync(sElementSync* pSync) {
	if (pSync)
		delete ((sObjectSync*)pSync);
}

sElementSync* sObject::CreateSyncFromOther(sElementSync* pSync)
{
	sObjectSync& sync = *(sObjectSync*)pSync;
	return new sObjectSync(sync);
}

bool sObject::HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB)
{
	sObjectSync& syncA = *(sObjectSync*)pSyncA;
	sObjectSync& syncB = *(sObjectSync*)pSyncB;
	return syncA.Compare(syncB);
}

void sObject::ApplyClientSync(uint16 nTime) {
	sElementPhysical::ApplyClientSync(nTime);
}

bool sObject::WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) {
	if (static_cast<int16>(nSyncLastTime - m_vSync.front().m_nTime) >= 0) // FindSync? not GetSyncWithTime?
		return WriteSyncDelta(pSyncStream, FindSync(nSyncWriteTime, nil).object, GetSyncWithTime(nSyncLastTime).object, (nSyncWriteTime - nSyncLastTime));

	tObjectSyncsDeltas objectDeltaManager{};
	objectDeltaManager.SetDifference(); // FindSync? not GetSyncWithTime?
	PerformWriteSync(pSyncStream, FindSync(nSyncWriteTime, nil).object, &objectDeltaManager); // max diff
	return true;
}

void sObject::ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) {
	if (bCurrDestPeerVanilaDevice) // useless, but ok
		return;

	sObjectSync& sync = *(sObjectSync*)pOutSync;
	uint32 nDiffMask = pSyncStream->ReadU32();

	// Physical sync
	if (nDiffMask & eObjectSyncDelta::MP_PKTD_OBJECT_PHYSICAL)
		sElementPhysical::ReadSyncFromStreamPhysical(pSyncStream, (sObjectSync*)pOutSync);

	// Health
	if (nDiffMask & eObjectSyncDelta::MP_PKTD_OBJECT_HEALTH)
		sync.m_nHealth = pSyncStream->ReadI8();
}

void sObject::ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) {
	if (nOwner != cMultiGame::Instance().LocalPlayerID()) {
		sElementPhysical::ReceiveEntity(nOwner, nID, nTime);
		return;
	}

	sObjectSync* pSync = GetSync().object;
	CObject* pObject = (CObject*)GetEntity();
	assert(pObject);

	// why it not in sElementPhysical::ReceiveEntity??
	pObject->SetMatrix(pSync->GetMatrix());
	pObject->SetMoveSpeed(pSync->GetMoveSpeed());
	pObject->m_vecTurnSpeed = pSync->GetTurnSpeed();
	pObject->m_vecMoveFriction = pSync->GetMoveFriction();
	pObject->m_vecTurnFriction = pSync->GetTurnFriction();

	//pObject->m_level = CTheZones::GetLevelFromPosition(&pSync->GetMatrix().GetPosition()); // need? FIX_BUGS
	pObject->m_nZoneLevel = CTheZones::GetLevelFromPosition(&pSync->GetMatrix().GetPosition());
	pObject->m_aColours[VEHICLE_COLOUR_PRIMARY] = m_aColours[VEHICLE_COLOUR_PRIMARY];
	pObject->m_aColours[VEHICLE_COLOUR_SECONDARY] = m_aColours[VEHICLE_COLOUR_SECONDARY];
	pObject->m_nHealth = pSync->m_nHealth;
	CWorld::Add(pObject);

	sElementPhysical::ReceiveEntity(nOwner, nID, nTime);
}

void sObject::UpdateDelta(sElementSync* pSync, uint16 nTimeDelta) {
	sElement::UpdateDelta(pSync, nTimeDelta);
}

bool sObject::IsTransferable(void) {
	return true;
}

void sObject::TransferEntity(int16 nDestPlayer) {
	debug("Transfer Object %d", GetID());
	sElement::TransferEntity(nDestPlayer);
}

void sObject::Update(uint16 nTime) {
	if (GetEntity() != nil)
		AttachSync(nTime, new sObjectSync((CObject*)GetEntity()));
	else
		delete this; // ?
}

void sObject::CompareSyncState(sObjectSync* pSync, sObjectSync* pLastSync, uint32 nDelta, tObjectSyncsDeltas* pDiff) {
	if (bCurrDestPeerVanilaDevice)
		return;

	// Physical sync
	pDiff->nPhysicalDiff = sElementPhysical::CompareSyncState(pSync, pLastSync);
	if (pDiff->nPhysicalDiff != ePhysicalSync::MP_PKTD_PHY_EQUAL)
		pDiff->nObjectDiff |= eObjectSyncDelta::MP_PKTD_OBJECT_PHYSICAL;

	// Health
	if (pLastSync->m_nHealth != pSync->m_nHealth)
		pDiff->nObjectDiff |= eObjectSyncDelta::MP_PKTD_OBJECT_HEALTH;
}

void sObject::PerformWriteSync(sWriteSyncStream* pSyncStream, sObjectSync* pSync, tObjectSyncsDeltas* pDiff) {
	if (bCurrDestPeerVanilaDevice)
		return;

	pSyncStream->WriteU32(pDiff->nObjectDiff);

	// Physical sync
	if (pDiff->nObjectDiff & eObjectSyncDelta::MP_PKTD_OBJECT_PHYSICAL)
		sElementPhysical::PerformWriteSync(pSyncStream, pSync, pDiff->nPhysicalDiff);

	// Health
	if (pDiff->nObjectDiff & eObjectSyncDelta::MP_PKTD_OBJECT_HEALTH)
		pSyncStream->WriteI8(pSync->m_nHealth);
}

bool sObject::WriteSyncDelta(sWriteSyncStream* pSyncStream, sObjectSync* pSync, sObjectSync* pLastSync, uint32 nDelta) {
	tObjectSyncsDeltas objectDeltaManager{};
	objectDeltaManager.SetEqual();
	CompareSyncState(pSync, pLastSync, nDelta, &objectDeltaManager);

	if (objectDeltaManager.nObjectDiff == eObjectSyncDelta::MP_PKTD_OBJECT_EQUAL) // main delta
		return false;

	PerformWriteSync(pSyncStream, pSync, &objectDeltaManager);
	return true;
}
#endif