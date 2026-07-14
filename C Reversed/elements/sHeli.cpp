/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "multiplayer/MultiGame.h"
#include "multiplayer/elements/sSyncStream.h"
#include "multiplayer/elements/sHeli.h"
#include "PlayerInfo.h"

#ifndef GTA_LIBERTY
sHeliSync::sHeliSync() : sAutomobileBaseSync() {
	DECLARE_SYNC_CONSTRUCT(this);
}

sHeliSync::sHeliSync(CHeli* pHeli) : sAutomobileBaseSync(pHeli) {
	DECLARE_SYNC_CONSTRUCT(this);
	// kek copypaste CQuadBike <-> CHeli
	// useless, already in sAutomobileBaseSync(pHeli)
	for (uint32 i = 0; i < ARRAY_SIZE(pHeli->m_aWheelColPoints); i++)
		m_aWheelColPoints[i] = pHeli->m_aWheelColPoints[i];
}

sHeliSync::sHeliSync(const sHeliSync& other) : sAutomobileBaseSync(other) {
	DECLARE_SYNC_CONSTRUCT(this);
}

sHeliSync::~sHeliSync() {
	DECLARE_SYNC_DESTRUCT(this);
}

bool sHeliSync::Compare(const sHeliSync& other)
{
	if (!sAutomobileBaseSync::Compare(other))
		return false;

	return true;
}

#if !defined(FINAL) && !defined(MASTER)
void sHeliSync::Dump()
{
	sAutomobileBaseSync::Dump();

	printf("=== sHeliSync Dump ===\n");
	printf("================================\n");
}
#endif


cHeliMG::cHeliMG(sElement* elem) : cAutomobileBaseMG(elem)
{
	m_fMainRotorAngle = 0.0f;
	m_fRearRotorAngle = 0.0f;
	m_vehType = eVehicleType::VEHICLE_TYPE_HELI;
}

cHeliMG::~cHeliMG()
{

}

void cHeliMG::PreRender(void)
{
	cMultiGame& Game = cMultiGame::Instance();
	cAutomobileBaseMG::PreRender();

	sHeli* pHeli = GetElement().heli;
	uint16 nTime = pHeli->m_nTime - static_cast<uint16>(Game.m_nLagValue);
	sHeliSync* heli = pHeli->FindSync(nTime, nil).heli;

	// CAutomobile::ProcessControl(void)
	// Heli dust
	if (GetModelIndex() != MI_RCRAIDER && GetModelIndex() != MI_RCGOBLIN && heli->m_aWheelSpeed[1] > 0.1125f /*&& GetPosition().z < 30.0f*/)
	{
		CVector pos = FindPlayerCoors();
		float radius = ((((GetPosition().z - pos.z) - 1.0f) - 10.0f) * 0.3f) + 10.0f;
		float rnd = Max(16.0f - 4.0f * CTimer::GetTimeStep(), 2.0f);
		CVehicle::HeliDustGenerate(this, radius, pos.z, Ceil(rnd));
	}

	if (GetModelIndex() == MI_SPARROW ||
		GetModelIndex() == MI_SEASPAR ||
		GetModelIndex() == MI_MAVERICK ||
		GetModelIndex() == MI_VCNMAV ||
		GetModelIndex() == MI_POLMAV)
	{
		// Looks like LCS actually uses fmodf for the angles but VC has a loop...
		// top rotor
		m_fMainRotorAngle -= heli->m_aWheelSpeed[1] * 1.66f * CTimer::GetTimeStep();
	}
	else {
		// top rotor
		m_fMainRotorAngle -= heli->m_aWheelSpeed[1] * CTimer::GetTimeStep();
	}

	while (m_fMainRotorAngle > TWOPI)
		m_fMainRotorAngle -= TWOPI;

	if (GetModelIndex() == -978) {
		// rear rotor
		m_fRearRotorAngle -= heli->m_aWheelSpeed[1] * 2.0f * CTimer::GetTimeStep();
	}
	else {
		// rear rotor
		m_fRearRotorAngle -= heli->m_aWheelSpeed[1] * 2.3f * CTimer::GetTimeStep();
	}

	while (m_fRearRotorAngle > TWOPI)
		m_fRearRotorAngle -= TWOPI;

	// CAutomobile::PreRender(void)
	CMatrix mat;
	CVector pos;

	// Top rotor
	if (pHeli->m_aCarNodes[HELI_STATIC_ROTOR])
	{
		mat.Attach(RwFrameGetMatrix(pHeli->m_aCarNodes[HELI_STATIC_ROTOR]));
		pos = mat.GetPosition();
		mat.SetRotateZ(m_fMainRotorAngle);
		mat.Translate(pos);
		mat.UpdateRW();
	}

	// Blurred top rotor
	if (pHeli->m_aCarNodes[HELI_MOVING_ROTOR])
	{
		mat.Attach(RwFrameGetMatrix(pHeli->m_aCarNodes[HELI_MOVING_ROTOR]));
		pos = mat.GetPosition();
		mat.SetRotateZ(m_fMainRotorAngle); // not -?
		mat.Translate(pos);
		mat.UpdateRW();
	}

	// Rear rotor
	if (pHeli->m_aCarNodes[HELI_STATIC_ROTOR_2])
	{
		mat.Attach(RwFrameGetMatrix(pHeli->m_aCarNodes[HELI_STATIC_ROTOR_2]));
		pos = mat.GetPosition();
		mat.SetRotateX(m_fRearRotorAngle);
		mat.Translate(pos);
		mat.UpdateRW();
	}

	// Blurred rear rotor
	if (pHeli->m_aCarNodes[HELI_MOVING_ROTOR_2])
	{
		mat.Attach(RwFrameGetMatrix(pHeli->m_aCarNodes[HELI_MOVING_ROTOR_2]));
		pos = mat.GetPosition();
		mat.SetRotateX(m_fRearRotorAngle); // not -?
		mat.Translate(pos);
		mat.UpdateRW();
	}
}

// CAutomobile::Render(void)
void cHeliMG::Render(void)
{
	cMultiGame& Game = cMultiGame::Instance();

	sHeli* pHeli = GetElement().heli;
	uint16 nTime = pHeli->m_nTime - static_cast<uint16>(Game.m_nLagValue);
	sHeliSync* heli = pHeli->FindSync(nTime, nil).heli;

	RpAtomic* atomic = nil;
	int32 rotorAlpha = (1.5f - Min(1.7f * Max(heli->m_aWheelSpeed[1], 0.0f) / 0.22f, 1.5f)) * 255.0f;
	rotorAlpha = Min(rotorAlpha, 255);
	int32 blurAlpha = Max(1.5f * heli->m_aWheelSpeed[1] / 0.22f - 0.4f, 0.0f) * 250.0f;
	blurAlpha = Min(blurAlpha, 250);

	// Top rotor
	if (pHeli->m_aCarNodes[HELI_STATIC_ROTOR])
	{
		RwFrameForAllObjects(pHeli->m_aCarNodes[HELI_STATIC_ROTOR], GetCurrentAtomicObjectCB, &atomic);
		if (atomic)
			SetComponentAtomicAlpha(atomic, rotorAlpha);
	}
	atomic = nil;

	// Rear rotor
	if (pHeli->m_aCarNodes[HELI_STATIC_ROTOR_2])
	{
		RwFrameForAllObjects(pHeli->m_aCarNodes[HELI_STATIC_ROTOR_2], GetCurrentAtomicObjectCB, &atomic);
		if (atomic)
			SetComponentAtomicAlpha(atomic, rotorAlpha);
	}
	atomic = nil;

	// Blurred top rotor
	if (pHeli->m_aCarNodes[HELI_MOVING_ROTOR])
	{
		RwFrameForAllObjects(pHeli->m_aCarNodes[HELI_MOVING_ROTOR], GetCurrentAtomicObjectCB, &atomic);
		if (atomic)
			SetComponentAtomicAlpha(atomic, blurAlpha);
	}
	atomic = nil;

	// Blurred rear rotor
	if (pHeli->m_aCarNodes[HELI_MOVING_ROTOR_2])
	{
		RwFrameForAllObjects(pHeli->m_aCarNodes[HELI_MOVING_ROTOR_2], GetCurrentAtomicObjectCB, &atomic);
		if (atomic)
			SetComponentAtomicAlpha(atomic, blurAlpha);
	}

#ifdef MULTIGAME_ELEMENTS_IMPROVEMENTS
	if (CVehicle::bWheelsOnlyCheat)
	{
		RpAtomicRender((RpAtomic*)GetFirstObject(pHeli->m_aCarNodes[CAR_WHEEL_RB]));
		RpAtomicRender((RpAtomic*)GetFirstObject(pHeli->m_aCarNodes[CAR_WHEEL_LB]));
		RpAtomicRender((RpAtomic*)GetFirstObject(pHeli->m_aCarNodes[CAR_WHEEL_RF]));
		RpAtomicRender((RpAtomic*)GetFirstObject(pHeli->m_aCarNodes[CAR_WHEEL_LF]));
		if (pHeli->m_aCarNodes[CAR_WHEEL_RM])
			RpAtomicRender((RpAtomic*)GetFirstObject(pHeli->m_aCarNodes[CAR_WHEEL_RM]));
		if (pHeli->m_aCarNodes[CAR_WHEEL_LM])
			RpAtomicRender((RpAtomic*)GetFirstObject(pHeli->m_aCarNodes[CAR_WHEEL_LM]));
	}
	else
#endif
		cAutomobileBaseMG::Render();
}


sHeli::sHeli() : sAutomobileBase()
{
	DECLARE_ELEMENT_CONSTRUCT(this, true, false);
	SetPhysical(new cHeliMG(this));
	DECLARE_ELEMENT_CONSTRUCT(this, false, false);
}

sHeli::sHeli(CHeli* pHeli) : sAutomobileBase(/*pHeli*/)
{
	DECLARE_ELEMENT_CONSTRUCT(this, true, true);
	cMultiGame::Instance().AttachEntity(this, pHeli);
	SetEntity(pHeli);
	SetPhysical(new cHeliMG(this));
	Initialise();
#ifdef FIX_BUGS
	// sVehicle
	m_fTraction = pHeli->m_fTraction;
	for (uint32 i = 0; i < ARRAY_SIZE(pHeli->m_aSuspensionLineLength); i++) // probably compiler loop optimization upper
		m_aSuspensionLineLength[i] = pHeli->m_aSuspensionLineLength[i];
	for (uint32 i = 0; i < ARRAY_SIZE(pHeli->m_aSuspensionSpringLength); i++)
		m_aSuspensionSpringLength[i] = pHeli->m_aSuspensionSpringLength[i];
	// already in Initialise, but still here to copy sVehicle all info
	m_aColours[VEHICLE_COLOUR_PRIMARY] = pHeli->m_aColours[VEHICLE_COLOUR_PRIMARY];
	m_aColours[VEHICLE_COLOUR_SECONDARY] = pHeli->m_aColours[VEHICLE_COLOUR_SECONDARY];
#endif
	AttachSync(m_nTime, new sHeliSync(pHeli));
	TransferZone();
	DECLARE_ELEMENT_CONSTRUCT(this, false, true);
}


ElementCapability sHeli::GetCapability()
{
	return sHeli::Capability();
}

bool sHeli::HasCapability(ElementCapability capability)
{
	if (sHeli::Capability() == capability)
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

sHeli::~sHeli()
{
	DECLARE_ELEMENT_DESTRUCT(this);
	sElement::PurgeAttached();
	// ~sElement
}

sElementSync* sHeli::CreateSync() {
	return new sHeliSync();
}

void sHeli::DisposeSync(sElementSync* pSync) {
	if (pSync)
		delete ((sHeliSync*)pSync);
}

sElementSync* sHeli::CreateSyncFromOther(sElementSync* pSync) {
	sHeliSync& sync = *(sHeliSync*)pSync;
#ifdef FIX_BUGS
	return new sHeliSync(sync);
#else
	sHeliSync* pNewSync = new sHeliSync(sync);
	// Base
	pNewSync->field_220 = sync.field_220;
	for (uint32 i = 0; i < NUM_DOORS; i++) {
		pNewSync->Doors[i] = sDoorSync(sync.Doors[i]);
	}
	return pNewSync;
#endif
}

bool sHeli::HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) {
	sHeliSync& syncA = *(sHeliSync*)pSyncA;
	sHeliSync& syncB = *(sHeliSync*)pSyncB;
	return syncA.Compare(syncB);
}

void sHeli::ApplyClientSync(uint16 nTime) {
	sAutomobileBase::ApplyClientSync(nTime);
}

void sHeli::Update(uint16 nTime) {
	if (GetEntity() != nil)
		AttachSync(nTime, new sHeliSync((CHeli*)GetEntity()));
	else
		delete this; // ?
}

bool sHeli::WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) {
	if (static_cast<int16>(nSyncLastTime - m_vSync.front().m_nTime) >= 0) // FindSync? not GetSyncWithTime?
		return WriteSyncDelta(pSyncStream, FindSync(nSyncWriteTime, nil).heli, GetSyncWithTime(nSyncLastTime).heli, (nSyncWriteTime - nSyncLastTime));

	tHeliSyncsDeltas heliDeltaManager{};
	heliDeltaManager.SetDifference(); // FindSync? not GetSyncWithTime?
	PerformWriteSync(pSyncStream, FindSync(nSyncWriteTime, nil).heli, &heliDeltaManager); // max diff
	return true;
}

void sHeli::ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) {
	//sHeliSync& sync = *(sHeliSync*)pOutSync;
	uint32 nDiffMask = pSyncStream->ReadU32();

	if (nDiffMask & eHeliSync::MP_PKTD_HELI_AUTOBASE)
		sAutomobileBase::ReadSyncFromStreamAutomobileBase(pSyncStream, (sHeliSync*)pOutSync);
}

void sHeli::ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) {
	cMultiGame& Game = cMultiGame::Instance();
	if (nOwner != Game.LocalPlayerID()) {
		sAutomobileBase::ReceiveEntity(nOwner, nID, nTime);
		return;
	}

	sHeliSync* pSync = GetSync().heli;
	assert(GetPhysical());
	CHeli* heli = new CHeli(GetPhysical()->GetModelIndex(), pSync->VehicleCreatedBy);
	Game.AttachEntity(this, heli);
	SetEntity(heli);

#ifdef FIX_BUGS
	for (uint32 i = 0; i < ARRAY_SIZE(heli->m_aWheelTimer); i++)
		heli->m_aWheelTimer[i] = pSync->m_aWheelTimer[i];

	for (uint32 i = 0; i < ARRAY_SIZE(heli->m_aWheelState); i++)
		heli->m_aWheelState[i] = pSync->m_aWheelState[i];

	for (uint32 i = 0; i < ARRAY_SIZE(heli->m_aWheelRotation); i++)
		heli->m_aWheelRotation[i] = pSync->m_aWheelRotation[i];

	for (uint32 i = 0; i < ARRAY_SIZE(heli->m_aWheelPosition); i++)
		heli->m_aWheelPosition[i] = pSync->m_aWheelPosition[i];

	for (uint32 i = 0; i < ARRAY_SIZE(heli->m_aWheelSpeed); i++)
		heli->m_aWheelSpeed[i] = pSync->m_aWheelSpeed[i];

	for (uint32 i = 0; i < ARRAY_SIZE(heli->m_aSuspensionSpringRatio); i++)
		heli->m_aSuspensionSpringRatio[i] = pSync->m_aSuspensionSpringRatio[i];

	for (uint32 i = 0; i < ARRAY_SIZE(heli->m_aWheelColPoints); i++)
		heli->m_aWheelColPoints[i] = pSync->m_aWheelColPoints[i];

	for (uint32 i = 0; i < ARRAY_SIZE(heli->m_aWheelSkidmarkType); i++)
		heli->m_aWheelSkidmarkType[i] = pSync->m_aWheelSkidmarkType[i];

	// Skidmark flags
	for (uint32 i = 0; i < ARRAY_SIZE(heli->m_aWheelSkidmarkBloody); i++) {
		heli->m_aWheelSkidmarkBloody[i] = (pSync->m_nSkidmarkFlags & BIT(i));
		heli->m_aWheelSkidmarkUnk[i] = (pSync->m_nSkidmarkFlags & BIT(i + 4));
	}
#endif

	sAutomobileBase::ReceiveEntity(nOwner, nID, nTime);
#ifdef FIX_BUGS
	heli->ResetSuspension();
#endif
	AttachSync(m_nTime, new sHeliSync(heli));
}

void sHeli::Initialise() {
	sAutomobileBase::Initialise();
}

void sHeli::SetupModel() {
	sAutomobileBase::SetupModel();
	SetupDoors();

	RpAtomic* pLF = (RpAtomic*)GetFirstObject(m_aCarNodes[PLANE_WHEEL_LF]);
	RpAtomic* pRF = (RpAtomic*)GetFirstObject(m_aCarNodes[PLANE_WHEEL_RF]);
	RpAtomic* pLB = (RpAtomic*)GetFirstObject(m_aCarNodes[PLANE_WHEEL_LB]);
	RpAtomic* pRB = (RpAtomic*)GetFirstObject(m_aCarNodes[PLANE_WHEEL_RB]);
	// mid wheels flag reset missing bug??
	if (pLF)
		RpAtomicSetFlags(pLF, 0);
	if (pRF)
		RpAtomicSetFlags(pRF, 0);
	if (pLB)
		RpAtomicSetFlags(pLB, 0);
	if (pRB)
		RpAtomicSetFlags(pRB, 0);
}

void sHeli::CompareSyncState(sHeliSync* pSync, sHeliSync* pLastSync, uint32 nDelta, tHeliSyncsDeltas* pDiff) {
	sAutomobileBase::CompareSyncState(pSync, pLastSync, nDelta, &pDiff->tAutomobileBaseDiff);

	// paste CPlane <-> CHeli
#ifdef FIX_BUGS
	if (pDiff->tAutomobileBaseDiff.nAutomobileBaseDiff != eAutomobileBaseSync::MP_PKTD_AUTOBASE_EQUAL)
#else
	if (pDiff->tAutomobileBaseDiff.nAutomobileBaseDiff != eAutomobileBaseSync::MP_PKTD_AUTOBASE_EQUAL ||
		pDiff->tAutomobileBaseDiff.tVehicleDiff.nVehicleDiff != eVehicleSync::MP_PKTD_VEH_EQUAL) // veh dif cmp useless, its already in nAutomobileBaseDiff
#endif
		pDiff->nHeliDiff |= eHeliSync::MP_PKTD_HELI_AUTOBASE;
}

void sHeli::PerformWriteSync(sWriteSyncStream* pSyncStream, sHeliSync* pSync, tHeliSyncsDeltas* pDiff) {
	pSyncStream->WriteU32(pDiff->nHeliDiff);

	if (pDiff->nHeliDiff & eHeliSync::MP_PKTD_HELI_AUTOBASE)
		sAutomobileBase::PerformWriteSync(pSyncStream, pSync, &pDiff->tAutomobileBaseDiff);
}

bool sHeli::WriteSyncDelta(sWriteSyncStream* pSyncStream, sHeliSync* pSync, sHeliSync* pLastSync, uint32 nDelta) {
	tHeliSyncsDeltas heliDeltaManager{};
	heliDeltaManager.SetEqual();
	CompareSyncState(pSync, pLastSync, nDelta, &heliDeltaManager);

#ifdef FIX_BUGS
	if (heliDeltaManager.nHeliDiff == eHeliSync::MP_PKTD_HELI_EQUAL) // main delta
		return false;
#endif

	PerformWriteSync(pSyncStream, pSync, &heliDeltaManager);
	return true;
}

#ifdef FIX_BUGS
void sHeli::Fix() {
	;
}
#endif
#endif
