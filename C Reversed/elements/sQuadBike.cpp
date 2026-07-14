/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "multiplayer/MultiGame.h"
#include "multiplayer/elements/sSyncStream.h"
#include "multiplayer/elements/sQuadBike.h"


#ifndef GTA_LIBERTY
sQuadBikeSync::sQuadBikeSync() : sAutomobileBaseSync() {
	DECLARE_SYNC_CONSTRUCT(this);
}

sQuadBikeSync::sQuadBikeSync(CQuadBike* pQuadBike) : sAutomobileBaseSync(pQuadBike) {
	DECLARE_SYNC_CONSTRUCT(this);
	// kek copypaste CQuadBike <-> CHeli
	// useless, already in sAutomobileBaseSync(pQuadBike)
	for (uint32 i = 0; i < ARRAY_SIZE(pQuadBike->m_aWheelColPoints); i++)
		m_aWheelColPoints[i] = pQuadBike->m_aWheelColPoints[i];
}

sQuadBikeSync::sQuadBikeSync(const sQuadBikeSync& other) : sAutomobileBaseSync(other) {
	DECLARE_SYNC_CONSTRUCT(this);
}

sQuadBikeSync::~sQuadBikeSync() {
	DECLARE_SYNC_DESTRUCT(this);
}

bool sQuadBikeSync::Compare(const sQuadBikeSync& other)
{
	if (!sAutomobileBaseSync::Compare(other))
		return false;

	return true;
}

#if !defined(FINAL) && !defined(MASTER)
void sQuadBikeSync::Dump()
{
	sAutomobileBaseSync::Dump();

	printf("=== sQuadBikeSync Dump ===\n");
	printf("================================\n");
}
#endif


cQuadBikeMG::cQuadBikeMG(sElement* elem) : cAutomobileBaseMG(elem)
{
	m_vehType = eVehicleType::VEHICLE_TYPE_QUAD;
}

cQuadBikeMG::~cQuadBikeMG()
{

}

void cQuadBikeMG::PreRender(void)
{
	cMultiGame& Game = cMultiGame::Instance();
	cAutomobileBaseMG::PreRender();

	sQuadBike* pQuadBike = GetElement().quadbike;
	uint16 nTime = pQuadBike->m_nTime - static_cast<uint16>(Game.m_nLagValue);
	sQuadBikeSync* quadbike = pQuadBike->FindSync(nTime, nil).quadbike;

	CVehicleModelInfo* mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex());
	PreRenderWheels(quadbike, mi);


	CVector wheelPos;
	mi->GetWheelPosn(CARWHEEL_REAR_LEFT, wheelPos);
	CVehicle::SetTransmissionRotation(
		pQuadBike->m_aCarNodes[QUAD_REAR_AXLE],
		quadbike->m_aWheelPosition[CARWHEEL_REAR_LEFT],
		quadbike->m_aWheelPosition[CARWHEEL_REAR_RIGHT],
		wheelPos,
		false
	);

	CVector wheelFrontLeftPos;
	mi->GetWheelPosn(CARWHEEL_FRONT_LEFT, wheelFrontLeftPos);

	// Original code saves position of each matrix (because calls to SetRotation set the pos. to 0), then restores it
	// We just use SetRotateYOnly which doesn't modify the position

	CMatrix mat;

	if (pQuadBike->m_aCarNodes[QUAD_SUSPENSION_LF]) {
		mat.Attach(RwFrameGetMatrix(pQuadBike->m_aCarNodes[QUAD_SUSPENSION_LF]), false);
		mat.SetRotateYOnly(atan2(quadbike->m_aWheelPosition[CARWHEEL_FRONT_LEFT] - wheelFrontLeftPos.z, fabs(wheelFrontLeftPos.x)));
		mat.UpdateRW();
	}

	if (pQuadBike->m_aCarNodes[QUAD_SUSPENSION_RF]) {
		mat.Attach(RwFrameGetMatrix(pQuadBike->m_aCarNodes[QUAD_SUSPENSION_RF]), false);
		mat.SetRotateYOnly(-atan2(quadbike->m_aWheelPosition[CARWHEEL_FRONT_RIGHT] - wheelFrontLeftPos.z, fabs(wheelFrontLeftPos.x)));
		mat.UpdateRW();
	}
}

void cQuadBikeMG::Render(void)
{
	cAutomobileBaseMG::Render();
}


sQuadBike::sQuadBike() : sAutomobileBase()
{
	DECLARE_ELEMENT_CONSTRUCT(this, true, false);
	SetPhysical(new cQuadBikeMG(this));
	DECLARE_ELEMENT_CONSTRUCT(this, false, false);
}


sQuadBike::sQuadBike(CQuadBike* pQuadBike) : sAutomobileBase(/*pQuadBike*/)
{
	DECLARE_ELEMENT_CONSTRUCT(this, true, true);
	cMultiGame::Instance().AttachEntity(this, pQuadBike);
	SetEntity(pQuadBike);
	SetPhysical(new cQuadBikeMG(this));
	Initialise();
#ifdef FIX_BUGS
	// sVehicle
	m_fTraction = pQuadBike->m_fTraction;
	for (uint32 i = 0; i < ARRAY_SIZE(pQuadBike->m_aSuspensionLineLength); i++) // probably compiler loop optimization upper
		m_aSuspensionLineLength[i] = pQuadBike->m_aSuspensionLineLength[i];
	for (uint32 i = 0; i < ARRAY_SIZE(pQuadBike->m_aSuspensionSpringLength); i++)
		m_aSuspensionSpringLength[i] = pQuadBike->m_aSuspensionSpringLength[i];
	// already in Initialise, but still here to copy sVehicle all info
	m_aColours[VEHICLE_COLOUR_PRIMARY] = pQuadBike->m_aColours[VEHICLE_COLOUR_PRIMARY];
	m_aColours[VEHICLE_COLOUR_SECONDARY] = pQuadBike->m_aColours[VEHICLE_COLOUR_SECONDARY];
#endif
	AttachSync(m_nTime, new sQuadBikeSync(pQuadBike));
	TransferZone();
	DECLARE_ELEMENT_CONSTRUCT(this, false, true);
}

ElementCapability sQuadBike::GetCapability()
{
	return sQuadBike::Capability();
}

bool sQuadBike::HasCapability(ElementCapability capability)
{
	if (sQuadBike::Capability() == capability)
		return true;
	if (sAutomobileBase::Capability() == capability)
		return true;
	if (sVehicle::Capability() == capability)
		return true;
	if (sElementPhysical::Capability() == capability)
		return true;
	if (sElement::Capability() == capability)
		return true;
	return false;
}

sQuadBike::~sQuadBike()
{
	DECLARE_ELEMENT_DESTRUCT(this);
	sElement::PurgeAttached();
	// ~sElement
}

sElementSync* sQuadBike::CreateSync() {
	return new sQuadBikeSync();
}

void sQuadBike::DisposeSync(sElementSync* pSync) {
	if (pSync)
		delete ((sQuadBikeSync*)pSync);
}

sElementSync* sQuadBike::CreateSyncFromOther(sElementSync* pSync) {
	sQuadBikeSync& sync = *(sQuadBikeSync*)pSync;
#ifdef FIX_BUGS
	return new sQuadBikeSync(sync);
#else
	sQuadBikeSync* pNewSync = new sQuadBikeSync(sync);
	// Base
	pNewSync->field_220 = sync.field_220;
	for (uint32 i = 0; i < NUM_DOORS; i++) {
		pNewSync->Doors[i] = sDoorSync(sync.Doors[i]);
	}
	return pNewSync;
#endif
}

bool sQuadBike::HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) {
	sQuadBikeSync& syncA = *(sQuadBikeSync*)pSyncA;
	sQuadBikeSync& syncB = *(sQuadBikeSync*)pSyncB;
	return syncA.Compare(syncB);
}

void sQuadBike::ApplyClientSync(uint16 nTime) {
	sAutomobileBase::ApplyClientSync(nTime);

//#ifdef FIX_BUGS
//	sQuadBikeSync* pSync = GetSync().quadbike;
//	for (uint32 i = 0; i < ARRAY_SIZE(pSync->m_aSuspensionSpringRatio); i++)
//		m_aSuspensionSpringRatio[i] = pSync->m_aSuspensionSpringRatio[i];
//#endif
}

void sQuadBike::Update(uint16 nTime) {
	if (GetEntity() != nil)
		AttachSync(nTime, new sQuadBikeSync((CQuadBike*)GetEntity()));
	else
		delete this; // ?
}

bool sQuadBike::WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) {
	if (static_cast<int16>(nSyncLastTime - m_vSync.front().m_nTime) >= 0) // FindSync? not GetSyncWithTime?
		return WriteSyncDelta(pSyncStream, FindSync(nSyncWriteTime, nil).quadbike, GetSyncWithTime(nSyncLastTime).quadbike, (nSyncWriteTime - nSyncLastTime));

	tQuadBikeSyncsDeltas quadBikeDeltaManager{};
	quadBikeDeltaManager.SetDifference(); // FindSync? not GetSyncWithTime?
	PerformWriteSync(pSyncStream, FindSync(nSyncWriteTime, nil).quadbike, &quadBikeDeltaManager); // max diff
	return true;
}

void sQuadBike::ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) {
	//sQuadBikeSync& sync = *(sQuadBikeSync*)pOutSync;
	uint32 nDiffMask = pSyncStream->ReadU32();

	if (nDiffMask & eQuadBikeSync::MP_PKTD_QUAD_AUTOBASE)
		sAutomobileBase::ReadSyncFromStreamAutomobileBase(pSyncStream, (sQuadBikeSync*)pOutSync);
}

void sQuadBike::ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) {
	cMultiGame& Game = cMultiGame::Instance();
	if (nOwner != Game.LocalPlayerID()) {
		sAutomobileBase::ReceiveEntity(nOwner, nID, nTime);
		return;
	}

	sQuadBikeSync* pSync = GetSync().quadbike;
	assert(GetPhysical());
	CQuadBike* quadbike = new CQuadBike(GetPhysical()->GetModelIndex(), pSync->VehicleCreatedBy);
	Game.AttachEntity(this, quadbike);
	SetEntity(quadbike);

#ifdef FIX_BUGS
	for (uint32 i = 0; i < ARRAY_SIZE(quadbike->m_aWheelTimer); i++)
		quadbike->m_aWheelTimer[i] = pSync->m_aWheelTimer[i];

	for (uint32 i = 0; i < ARRAY_SIZE(quadbike->m_aWheelState); i++)
		quadbike->m_aWheelState[i] = pSync->m_aWheelState[i];

	for (uint32 i = 0; i < ARRAY_SIZE(quadbike->m_aWheelRotation); i++)
		quadbike->m_aWheelRotation[i] = pSync->m_aWheelRotation[i];

	for (uint32 i = 0; i < ARRAY_SIZE(quadbike->m_aWheelPosition); i++)
		quadbike->m_aWheelPosition[i] = pSync->m_aWheelPosition[i];

	for (uint32 i = 0; i < ARRAY_SIZE(quadbike->m_aWheelSpeed); i++)
		quadbike->m_aWheelSpeed[i] = pSync->m_aWheelSpeed[i];

	for (uint32 i = 0; i < ARRAY_SIZE(quadbike->m_aSuspensionSpringRatio); i++)
		quadbike->m_aSuspensionSpringRatio[i] = pSync->m_aSuspensionSpringRatio[i];

	for (uint32 i = 0; i < ARRAY_SIZE(quadbike->m_aWheelColPoints); i++)
		quadbike->m_aWheelColPoints[i] = pSync->m_aWheelColPoints[i];

	for (uint32 i = 0; i < ARRAY_SIZE(quadbike->m_aWheelSkidmarkType); i++)
		quadbike->m_aWheelSkidmarkType[i] = pSync->m_aWheelSkidmarkType[i];

	// Skidmark flags
	for (uint32 i = 0; i < ARRAY_SIZE(quadbike->m_aWheelSkidmarkBloody); i++) {
		quadbike->m_aWheelSkidmarkBloody[i] = (pSync->m_nSkidmarkFlags & BIT(i));
		quadbike->m_aWheelSkidmarkUnk[i] = (pSync->m_nSkidmarkFlags & BIT(i + 4));
	}
#endif

	sAutomobileBase::ReceiveEntity(nOwner, nID, nTime);
#ifdef FIX_BUGS
	quadbike->ResetSuspension();
#endif
	AttachSync(m_nTime, new sQuadBikeSync(quadbike));
}

void sQuadBike::Initialise() {
	sAutomobileBase::Initialise();
}

void sQuadBike::SetupModel() {
	sAutomobileBase::SetupModel();
#ifdef FIX_BUGS
	SetupDoors();
#endif
}

void sQuadBike::CompareSyncState(sQuadBikeSync* pSync, sQuadBikeSync* pLastSync, uint32 nDelta, tQuadBikeSyncsDeltas* pDiff) {
	sAutomobileBase::CompareSyncState(pSync, pLastSync, nDelta, &pDiff->tAutomobileBaseDiff);

#ifdef FIX_BUGS
	if (pDiff->tAutomobileBaseDiff.nAutomobileBaseDiff != eAutomobileBaseSync::MP_PKTD_AUTOBASE_EQUAL)
#endif
		pDiff->nQuadBikeDiff |= eQuadBikeSync::MP_PKTD_QUAD_AUTOBASE;
}

void sQuadBike::PerformWriteSync(sWriteSyncStream* pSyncStream, sQuadBikeSync* pSync, tQuadBikeSyncsDeltas* pDiff) {
	pSyncStream->WriteU32(pDiff->nQuadBikeDiff);

	if (pDiff->nQuadBikeDiff & eQuadBikeSync::MP_PKTD_QUAD_AUTOBASE)
		sAutomobileBase::PerformWriteSync(pSyncStream, pSync, &pDiff->tAutomobileBaseDiff);
}

bool sQuadBike::WriteSyncDelta(sWriteSyncStream* pSyncStream, sQuadBikeSync* pSync, sQuadBikeSync* pLastSync, uint32 nDelta) {
	tQuadBikeSyncsDeltas quadBikeDeltaManager{};
	quadBikeDeltaManager.SetEqual();
	CompareSyncState(pSync, pLastSync, nDelta, &quadBikeDeltaManager);

#ifdef FIX_BUGS
	if (quadBikeDeltaManager.nQuadBikeDiff == eQuadBikeSync::MP_PKTD_QUAD_EQUAL) // main delta
		return false;
#endif

	PerformWriteSync(pSyncStream, pSync, &quadBikeDeltaManager);
	return true;
}

#ifdef FIX_BUGS
void sQuadBike::Fix() {
	;
}
#endif
#endif