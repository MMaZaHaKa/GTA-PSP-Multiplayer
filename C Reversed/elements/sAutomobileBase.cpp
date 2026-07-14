/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "multiplayer/MultiGame.h"
#include "multiplayer/elements/sSyncStream.h"
#include "multiplayer/elements/sAutomobileBase.h"


#ifndef GTA_LIBERTY
sDoorSync::sDoorSync()
{
	m_fAngle = 0.0f;
	m_fPrevAngle = 0.0f;
	m_fAngVel = 0.0f;
	m_vecSpeed = CVector(0.0f, 0.0f, 0.0f);
}

sDoorSync::sDoorSync(const sDoorSync& other)
{
	m_fAngle = other.m_fAngle;
	m_fPrevAngle = other.m_fPrevAngle;
	m_fAngVel = other.m_fAngVel;
	m_vecSpeed = other.m_vecSpeed;
}

bool sDoorSync::Compare(const sDoorSync& other)
{
	if (m_fAngle != other.m_fAngle)
		return false;
	if (m_fPrevAngle != other.m_fPrevAngle)
		return false;
	if (m_fAngVel != other.m_fAngVel)
		return false;
	if (m_vecSpeed != other.m_vecSpeed)
		return false;

	return true;
}

#if !defined(FINAL) && !defined(MASTER)
void sDoorSync::Dump()
{
	printf("  Door Info:\n");
	printf("    Angle:     %.2f (0x%08X)\n", m_fAngle, *(uint32*)&m_fAngle);
	printf("    PrevAngle: %.2f (0x%08X)\n", m_fPrevAngle, *(uint32*)&m_fPrevAngle);
	printf("    AngVel:    %.2f (0x%08X)\n", m_fAngVel, *(uint32*)&m_fAngVel);
	printf("    Speed:     X=%.2f Y=%.2f Z=%.2f\n", m_vecSpeed.x, m_vecSpeed.y, m_vecSpeed.z);
}
#endif

sAutomobileBaseSync::sAutomobileBaseSync() : sVehicleSync()
{
	// default Doors ctor array
	// wtf? useless double init, it's already done in sVehicleSync(), but kek, okay
	for (uint32 i = 0; i < ARRAY_SIZE(m_aWheelTimer); i++)
		m_aWheelTimer[i] = 0;
	for (uint32 i = 0; i < ARRAY_SIZE(m_aWheelState); i++)
		m_aWheelState[i] = tWheelState::WHEEL_STATE_NORMAL;
	for (uint32 i = 0; i < ARRAY_SIZE(m_aWheelRotation); i++)
		m_aWheelRotation[i] = 0.0f;
	for (uint32 i = 0; i < ARRAY_SIZE(m_aWheelPosition); i++)
		m_aWheelPosition[i] = 0.0f;
	for (uint32 i = 0; i < ARRAY_SIZE(m_aWheelSpeed); i++)
		m_aWheelSpeed[i] = 0.0f;
	for (uint32 i = 0; i < ARRAY_SIZE(m_aSuspensionSpringRatio); i++)
		m_aSuspensionSpringRatio[i] = 0.0f;
	for (uint32 i = 0; i < ARRAY_SIZE(m_aWheelColPoints); i++) {
		//m_aWheelColPoints[i] = CColPoint();
		memset(&m_aWheelColPoints[i], 0, sizeof(CColPoint));
	}
	for (uint32 i = 0; i < ARRAY_SIZE(m_aWheelSkidmarkType); i++)
		m_aWheelSkidmarkType[i] = eSkidmarkType::SKIDMARK_NORMAL;
}

sAutomobileBaseSync::sAutomobileBaseSync(CAutomobile* pAutomobile) : sVehicleSync(pAutomobile)
{
	for (uint32 i = 0; i < ARRAY_SIZE(Doors); i++) {
		Doors[i] = sDoorSync();
	}

	// orig do compiler optimization loop unroll
	for (uint32 i = 0; i < ARRAY_SIZE(pAutomobile->m_aWheelColPoints); i++)
		m_aWheelColPoints[i] = pAutomobile->m_aWheelColPoints[i];

	for (uint32 i = 0; i < ARRAY_SIZE(pAutomobile->m_aSuspensionSpringRatio); i++)
		m_aSuspensionSpringRatio[i] = pAutomobile->m_aSuspensionSpringRatio[i];

	for (uint32 i = 0; i < ARRAY_SIZE(pAutomobile->m_aWheelTimer); i++)
		m_aWheelTimer[i] = pAutomobile->m_aWheelTimer[i];

	for (uint32 i = 0; i < ARRAY_SIZE(pAutomobile->m_aWheelSkidmarkType); i++)
		m_aWheelSkidmarkType[i] = pAutomobile->m_aWheelSkidmarkType[i];

	for (uint32 i = 0; i < ARRAY_SIZE(pAutomobile->m_aWheelRotation); i++)
		m_aWheelRotation[i] = pAutomobile->m_aWheelRotation[i];

	for (uint32 i = 0; i < ARRAY_SIZE(pAutomobile->m_aWheelPosition); i++)
		m_aWheelPosition[i] = pAutomobile->m_aWheelPosition[i];

	for (uint32 i = 0; i < ARRAY_SIZE(pAutomobile->m_aWheelSpeed); i++)
		m_aWheelSpeed[i] = pAutomobile->m_aWheelSpeed[i];

	for (uint32 i = 0; i < ARRAY_SIZE(pAutomobile->m_aWheelState); i++)
		m_aWheelState[i] = pAutomobile->m_aWheelState[i];

	//for (uint32 i = 0; i < ARRAY_SIZE(pAutomobile->m_wheelStatus); i++)
	//	m_wheelStatus[i] = pAutomobile->m_wheelStatus[i];

	m_nSkidmarkFlags = 0x0;

	for (uint32 i = 0; i < ARRAY_SIZE(CAutomobile::m_aWheelSkidmarkBloody); i++) {
		if (pAutomobile->m_aWheelSkidmarkBloody[i])
			m_nSkidmarkFlags |= BIT(i);
		else
			m_nSkidmarkFlags &= ~BIT(i);

		if (pAutomobile->m_aWheelSkidmarkUnk[i])
			m_nSkidmarkFlags |= BIT(i + 4);
		else
			m_nSkidmarkFlags &= ~BIT(i + 4);
	}

	for (uint32 i = 0; i < NUM_DOORS; i++) {
		Doors[i].m_fAngle = pAutomobile->Doors[i].m_fAngle;
		Doors[i].m_fAngVel = pAutomobile->Doors[i].m_fAngVel;
		Doors[i].m_fPrevAngle = pAutomobile->Doors[i].m_fPrevAngle;
		Doors[i].m_vecSpeed = pAutomobile->Doors[i].m_vecSpeed;
	}
}

sAutomobileBaseSync::sAutomobileBaseSync(const sAutomobileBaseSync& other) : sVehicleSync(other)
{
#ifdef FIX_BUGS
	field_220 = other.field_220;
	for (uint32 i = 0; i < ARRAY_SIZE(Doors); i++) {
		Doors[i] = sDoorSync(other.Doors[i]);
	}
#endif
}

bool sAutomobileBaseSync::Compare(const sAutomobileBaseSync& other)
{
	if (!sVehicleSync::Compare(other))
		return false;

	// need?
//#ifdef FIX_BUGS
//	for (uint32 i = 0; i < NUM_DOORS; i++) {
//		if (!Doors[i].Compare(other.Doors[i]))
//			return false;
//	}
//
//	if (field_220 != other.field_220)
//		return false;
//#endif

	return true;
}

sAutomobileBaseSync::~sAutomobileBaseSync() { }

#if !defined(FINAL) && !defined(MASTER)
void sAutomobileBaseSync::Dump()
{
	sVehicleSync::Dump();

	printf("=== sAutomobileBaseSync Dump ===\n");
	printf("sAutomobileBaseSync Info:\n");
	printf("  field_220: %u (0x%04X)\n", field_220, field_220);
	printf("\n");

	printf("Doors:\n");
	for (uint32 i = 0; i < NUM_DOORS; i++) {
		printf("  Door[%d]:\n", i);
		Doors[i].Dump();
	}
	printf("================================\n");
}
#endif



cAutomobileBaseMG::cAutomobileBaseMG(sElement* elem) : cVehicleMG(elem) {
}

cAutomobileBaseMG::~cAutomobileBaseMG()
{

}

void cAutomobileBaseMG::PreRender(void)
{
	cVehicleMG::PreRender();
	sAutomobileBase* pAutomobileBase = GetElement().automobilebase;
	uint16 nTime = pAutomobileBase->m_nTime - static_cast<uint16>(cMultiGame::Instance().m_nLagValue);
	sAutomobileBaseSync* pSync = pAutomobileBase->FindSync(nTime, nil).automobilebase;
	pAutomobileBase->PreRenderDoor(pSync, eCarNodes::CAR_DOOR_LF, eDoors::DOOR_FRONT_LEFT);
	pAutomobileBase->PreRenderDoor(pSync, eCarNodes::CAR_DOOR_RF, eDoors::DOOR_FRONT_RIGHT);
	pAutomobileBase->PreRenderDoor(pSync, eCarNodes::CAR_DOOR_LR, eDoors::DOOR_REAR_LEFT);
	pAutomobileBase->PreRenderDoor(pSync, eCarNodes::CAR_DOOR_RR, eDoors::DOOR_REAR_RIGHT);
	pAutomobileBase->PreRenderDoor(pSync, eCarNodes::CAR_BONNET, eDoors::DOOR_BONNET);
	pAutomobileBase->PreRenderDoor(pSync, eCarNodes::CAR_BOOT, eDoors::DOOR_BOOT);
}

void cAutomobileBaseMG::Render(void)
{
	cVehicleMG::Render();
}

void cAutomobileBaseMG::PreRenderWheels(sAutomobileBaseSync* pSync, CVehicleModelInfo* pModelInfo)
{
	CMatrix mat;
	CVector pos;

    sAutomobileBase* pAutomobileBase = GetElement().automobilebase;

    // Rear right wheel
    mat.Attach(RwFrameGetMatrix(pAutomobileBase->m_aCarNodes[CAR_WHEEL_RB]));
    pos = mat.GetPosition();
    pos.z = pSync->m_aWheelPosition[CARWHEEL_REAR_RIGHT];
    mat.SetRotateX(pSync->m_aWheelRotation[CARWHEEL_REAR_RIGHT]);
    mat.Scale(pModelInfo->GetWheelScale(true));
    mat.Translate(pos);
    mat.UpdateRW();

    // Rear left wheel
    mat.Attach(RwFrameGetMatrix(pAutomobileBase->m_aCarNodes[CAR_WHEEL_LB]));
    pos = mat.GetPosition();
    pos.z = pSync->m_aWheelPosition[CARWHEEL_REAR_LEFT];
    mat.SetRotate(-pSync->m_aWheelRotation[CARWHEEL_REAR_LEFT], 0.0f, PI);
    mat.Scale(pModelInfo->GetWheelScale(true));
    mat.Translate(pos);
    mat.UpdateRW();

    // Mid right wheel
    if (pAutomobileBase->m_aCarNodes[CAR_WHEEL_RM])
    {
        mat.Attach(RwFrameGetMatrix(pAutomobileBase->m_aCarNodes[CAR_WHEEL_RM]));
        pos = mat.GetPosition();
        pos.z = pSync->m_aWheelPosition[CARWHEEL_REAR_RIGHT];
        mat.SetRotateX(pSync->m_aWheelRotation[CARWHEEL_REAR_RIGHT]);
        mat.Scale(pModelInfo->GetWheelScale(true));
        mat.Translate(pos);
        mat.UpdateRW();
    }

    // Mid left wheel
    if (pAutomobileBase->m_aCarNodes[CAR_WHEEL_LM])
    {
        mat.Attach(RwFrameGetMatrix(pAutomobileBase->m_aCarNodes[CAR_WHEEL_LM]));
        pos = mat.GetPosition();
        pos.z = pSync->m_aWheelPosition[CARWHEEL_REAR_LEFT];
        mat.SetRotate(-pSync->m_aWheelRotation[CARWHEEL_REAR_LEFT], 0.0f, PI);
        mat.Scale(pModelInfo->GetWheelScale(true));
        mat.Translate(pos);
        mat.UpdateRW();
    }

    // Front right wheel
    mat.Attach(RwFrameGetMatrix(pAutomobileBase->m_aCarNodes[CAR_WHEEL_RF]));
    pos = mat.GetPosition();
    pos.z = pSync->m_aWheelPosition[CARWHEEL_FRONT_RIGHT];
    mat.SetRotate(pSync->m_aWheelRotation[CARWHEEL_FRONT_RIGHT], 0.0f, pSync->m_fSteerAngle);
    mat.Scale(pModelInfo->GetWheelScale(true));
    mat.Translate(pos);
    mat.UpdateRW();

    // Front left wheel
    mat.Attach(RwFrameGetMatrix(pAutomobileBase->m_aCarNodes[CAR_WHEEL_LF]));
    pos = mat.GetPosition();
    pos.z = pSync->m_aWheelPosition[CARWHEEL_FRONT_LEFT];
    mat.SetRotate(-pSync->m_aWheelRotation[CARWHEEL_FRONT_LEFT], 0.0f, PI + pSync->m_fSteerAngle);
    mat.Scale(pModelInfo->GetWheelScale(true));
    mat.Translate(pos);
    mat.UpdateRW();
}


sAutomobileBase::sAutomobileBase() : sVehicle() {
	for (uint32 i = 0; i < ARRAY_SIZE(Doors); i++) {
		Doors[i] = sDoor();
	}
#ifdef FIX_BUGS
	for (uint32 i = 0; i < ARRAY_SIZE(m_aCarNodes); i++)
		m_aCarNodes[i] = nil;
#endif
}


ElementCapability sAutomobileBase::GetCapability()
{
	return sAutomobileBase::Capability();
}

bool sAutomobileBase::HasCapability(ElementCapability capability)
{
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

sAutomobileBase::~sAutomobileBase() { }

void sAutomobileBase::ApplyClientSync(uint16 time) {
	sVehicle::ApplyClientSync(time);
}

void sAutomobileBase::ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) {
	sAutomobileBaseSync* automobilebase = GetSync().automobilebase;
	CAutomobile* pAutomobile = (CAutomobile*)GetEntity();
	assert(pAutomobile);
	for (uint32 i = 0; i < NUM_DOORS; i++) {
		pAutomobile->Doors[i].m_fAngle = automobilebase->Doors[i].m_fAngle;
		pAutomobile->Doors[i].m_fAngVel = automobilebase->Doors[i].m_fAngVel;
		//pAutomobile->Doors[i].m_fPrevAngle = automobilebase->Doors[i].m_fPrevAngle; // not need??
		//pAutomobile->Doors[i].m_vecSpeed = automobilebase->Doors[i].m_vecSpeed; // not need??
	}
	sVehicle::ReceiveEntity(nOwner, nID, nTime);
}

bool sAutomobileBase::IsTransferable(void) {
	return sVehicle::IsTransferable();
}

void sAutomobileBase::TransferEntity(int16 nDestPlayer) {
	sVehicle::TransferEntity(nDestPlayer);
}


void sAutomobileBase::Initialise() {
	sVehicle::Initialise();
	SetupDoors();
}

void sAutomobileBase::SetupModel() {
	assert(GetPhysical());

	// CAutomobile::SetupModelNodes
	for (uint32 i = 0; i < ARRAY_SIZE(m_aCarNodes); i++)
		m_aCarNodes[i] = nil;
	CClumpModelInfo::FillFrameArray(GetPhysical()->GetClump(), m_aCarNodes);
}

void sAutomobileBase::CompareSyncState(sAutomobileBaseSync* pSync, sAutomobileBaseSync* pLastSync, uint32 nDelta, tAutomobileBaseSyncsDeltas* pDiff) {
	sVehicle::CompareSyncState(pSync, pLastSync, nDelta, &pDiff->tVehicleDiff);

	if (pDiff->tVehicleDiff.nVehicleDiff != eVehicleSync::MP_PKTD_VEH_EQUAL)
		pDiff->nAutomobileBaseDiff |= eAutomobileBaseSync::MP_PKTD_AUTOBASE_VEHICLE;

	// ?
	if (FLT_EPS_NOT_EQ((float)pLastSync->field_220, (float)pSync->field_220)) // what? u16 as float? pastebug? FIX_BUGS? pLastSync->field_220 != pSync->field_220
		pDiff->nAutomobileBaseDiff |= eAutomobileBaseSync::MP_PKTD_AUTOBASE_UNKNOWN;

	// Doors
	for (uint32 i = 0; i < NUM_DOORS; i++) {
		// Angle
		if (FLT_EPS_NOT_EQ(pLastSync->Doors[i].m_fAngle, pSync->Doors[i].m_fAngle))
			pDiff->nAutomobileBaseDiff |= MP_PKTD_AUTOBASE_DOOR_ANGLE_MASK(i);
		else // !!---- UPD!!
			pSync->Doors[i].m_fAngle = pLastSync->Doors[i].m_fAngle;

		// Velocity
		if (FLT_EPS_NOT_EQ(pLastSync->Doors[i].m_fAngVel, pSync->Doors[i].m_fAngVel))
			pDiff->nAutomobileBaseDiff |= MP_PKTD_AUTOBASE_DOOR_ANGLE_VELOCITY_MASK(i);
		else // !!---- UPD!!
			pSync->Doors[i].m_fAngVel = pLastSync->Doors[i].m_fAngVel;
	}
}

void sAutomobileBase::PerformWriteSync(sWriteSyncStream* pSyncStream, sAutomobileBaseSync* pSync, tAutomobileBaseSyncsDeltas* pDiff) {
	pSyncStream->WriteU32(pDiff->nAutomobileBaseDiff);

	if (pDiff->nAutomobileBaseDiff & eAutomobileBaseSync::MP_PKTD_AUTOBASE_VEHICLE)
		sVehicle::PerformWriteSync(pSyncStream, pSync, &pDiff->tVehicleDiff);

	// ?
	if (pDiff->nAutomobileBaseDiff & eAutomobileBaseSync::MP_PKTD_AUTOBASE_UNKNOWN)
		pSyncStream->WriteU16(pSync->field_220);

	// Doors
	for (uint32 i = 0; i < NUM_DOORS; i++) {
		// Angle
		if (pDiff->nAutomobileBaseDiff & MP_PKTD_AUTOBASE_DOOR_ANGLE_MASK(i))
			pSyncStream->WriteFloat(pSync->Doors[i].m_fAngle);

		// Velocity
		if (pDiff->nAutomobileBaseDiff & MP_PKTD_AUTOBASE_DOOR_ANGLE_VELOCITY_MASK(i))
			pSyncStream->WriteFloat(pSync->Doors[i].m_fAngVel);
	}
}

void sAutomobileBase::ReadSyncFromStreamAutomobileBase(sReadSyncStream* pSyncStream, sAutomobileBaseSync* pOutSync) {
	uint32 nDiffMask = pSyncStream->ReadU32();

	if (nDiffMask & eAutomobileBaseSync::MP_PKTD_AUTOBASE_VEHICLE)
		sVehicle::ReadSyncFromStreamVehicle(pSyncStream, pOutSync);

	// ?
	if (nDiffMask & eAutomobileBaseSync::MP_PKTD_AUTOBASE_UNKNOWN)
		pOutSync->field_220 = pSyncStream->ReadU16();

	// Doors
	for (uint32 i = 0; i < NUM_DOORS; i++) {
		// Angle
		if (nDiffMask & MP_PKTD_AUTOBASE_DOOR_ANGLE_MASK(i))
			pOutSync->Doors[i].m_fAngle = pSyncStream->ReadFloat();

		// Velocity
		if (nDiffMask & MP_PKTD_AUTOBASE_DOOR_ANGLE_VELOCITY_MASK(i))
			pOutSync->Doors[i].m_fAngVel = pSyncStream->ReadFloat();
	}
}

void sAutomobileBase::SetupDoors() {
	assert(GetPhysical());
	tHandlingData* pHandling = GET_HANDLING(GetPhysical()->GetModelIndex());

	// Doors
	if (pHandling->IsModelFlagSet(eHandlingModelFlags::HANDLING_MODEL_IS_BUS))
	{
		Doors[DOOR_FRONT_LEFT].Init(-HALFPI, 0.0f, 0, 2);
		Doors[DOOR_FRONT_RIGHT].Init(0.0f, HALFPI, 1, 2);
	}
	else
	{
		Doors[DOOR_FRONT_LEFT].Init(-PI * 0.4f, 0.0f, 0, 2);
		Doors[DOOR_FRONT_RIGHT].Init(0.0f, PI * 0.4f, 1, 2);
	}
	if (pHandling->IsModelFlagSet(eHandlingModelFlags::HANDLING_MODEL_IS_VAN))
	{
		Doors[DOOR_REAR_LEFT].Init(-HALFPI, 0.0f, 1, 2);
		Doors[DOOR_REAR_RIGHT].Init(0.0f, HALFPI, 0, 2);
	}
	else
	{
		Doors[DOOR_REAR_LEFT].Init(-PI * 0.4f, 0.0f, 0, 2);
		Doors[DOOR_REAR_RIGHT].Init(0.0f, PI * 0.4f, 1, 2);
	}

	if (pHandling->IsModelFlagSet(eHandlingModelFlags::HANDLING_MODEL_REV_BONNET))
		Doors[DOOR_BONNET].Init(-PI * 0.3f, 0.0f, 1, 0);
	else
		Doors[DOOR_BONNET].Init(0.0f, PI * 0.3f, 1, 0);

	if (pHandling->IsModelFlagSet(eHandlingModelFlags::HANDLING_MODEL_HANGING_BOOT))
		Doors[DOOR_BOOT].Init(-PI * 0.4f, 0.0f, 0, 0);
	else if (pHandling->IsModelFlagSet(eHandlingModelFlags::HANDLING_MODEL_TAILGATE_BOOT))
		Doors[DOOR_BOOT].Init(0.0, HALFPI, 1, 0);
	else
		Doors[DOOR_BOOT].Init(-PI * 0.3f, 0.0f, 1, 0);
}

void sAutomobileBase::PreRenderDoor(sAutomobileBaseSync* pSync, eCarNodes component, eDoors nDoor) {
	if (!m_aCarNodes[component])
		return;

	CMatrix mat(RwFrameGetMatrix(m_aCarNodes[component]));
	CVector pos = mat.GetPosition();
	float axes[3] = { 0.0f, 0.0f, 0.0f };
	axes[Doors[nDoor].m_nAxis] = pSync->Doors[nDoor].m_fAngle;
	mat.SetRotate(axes[0], axes[1], axes[2]);
	mat.Translate(pos);
	mat.UpdateRW();
}
#endif
