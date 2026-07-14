/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "multiplayer/MultiGame.h"
#include "multiplayer/elements/sSyncStream.h"
#include "multiplayer/elements/sPlane.h"


#ifndef GTA_LIBERTY
sPlaneSync::sPlaneSync() : sAutomobileBaseSync()
{
	DECLARE_SYNC_CONSTRUCT(this);
	m_fAttackLift = 0.0f;
	m_fGearDownLiftMult = 0.0f;
	m_fEngineSpeed = 0.0f;
	m_fYawControl = 0.0f;
	m_fPitchControl = 0.0f;
	m_fRollControl = 0.0f;
	m_fThrottleControl = 0.0f;
	m_fLGearAngle = 0.0f;
}

sPlaneSync::sPlaneSync(CPlane* pPlane) : sAutomobileBaseSync(pPlane)
{
	DECLARE_SYNC_CONSTRUCT(this);
	assert(pPlane->m_pFlyingHandling);
	m_fAttackLift = pPlane->m_pFlyingHandling->fAttackLift;
	m_fGearDownLiftMult = pPlane->m_pFlyingHandling->fGearDownLiftMult;
	m_fEngineSpeed = pPlane->m_fEngineSpeed;
	m_fYawControl = pPlane->m_fYawControl;
	m_fPitchControl = pPlane->m_fPitchControl;
	m_fRollControl = pPlane->m_fRollControl;
	m_fThrottleControl = pPlane->m_fThrottleControl;
	m_fLGearAngle = pPlane->m_fLGearAngle;
}

sPlaneSync::sPlaneSync(const sPlaneSync& other) : sAutomobileBaseSync(other)
{
	DECLARE_SYNC_CONSTRUCT(this);
#ifdef FIX_BUGS
	m_fAttackLift = other.m_fAttackLift;
	m_fGearDownLiftMult = other.m_fGearDownLiftMult;
	m_fEngineSpeed = other.m_fEngineSpeed;
	m_fYawControl = other.m_fYawControl;
	m_fPitchControl = other.m_fPitchControl;
	m_fRollControl = other.m_fRollControl;
	m_fThrottleControl = other.m_fThrottleControl;
	m_fLGearAngle = other.m_fLGearAngle;
#endif
}

sPlaneSync::~sPlaneSync() {
	DECLARE_SYNC_DESTRUCT(this);
}

bool sPlaneSync::Compare(const sPlaneSync& other)
{
	if (!sAutomobileBaseSync::Compare(other))
		return false;

#ifdef FIX_BUGS
	if (m_fAttackLift != other.m_fAttackLift)
		return false;

	if (m_fGearDownLiftMult != other.m_fGearDownLiftMult)
		return false;

	if (m_fEngineSpeed != other.m_fEngineSpeed)
		return false;

	if (m_fYawControl != other.m_fYawControl)
		return false;

	if (m_fPitchControl != other.m_fPitchControl)
		return false;

	if (m_fRollControl != other.m_fRollControl)
		return false;

	if (m_fThrottleControl != other.m_fThrottleControl)
		return false;

	if (m_fLGearAngle != other.m_fLGearAngle)
		return false;
#endif

	return true;
}

#if !defined(FINAL) && !defined(MASTER)
void sPlaneSync::Dump()
{
	sAutomobileBaseSync::Dump();

	printf("=== sPlaneSync Dump ===\n");
	printf("Plane Controls:\n");
	printf("  AttackLift:        %.2f (0x%08X)\n", m_fAttackLift, *(uint32*)&m_fAttackLift);
	printf("  GearDownLiftMult:  %.2f (0x%08X)\n", m_fGearDownLiftMult, *(uint32*)&m_fGearDownLiftMult);
	printf("  EngineSpeed:       %.2f (0x%08X)\n", m_fEngineSpeed, *(uint32*)&m_fEngineSpeed);
	printf("  YawControl:        %.2f (0x%08X)\n", m_fYawControl, *(uint32*)&m_fYawControl);
	printf("  PitchControl:      %.2f (0x%08X)\n", m_fPitchControl, *(uint32*)&m_fPitchControl);
	printf("  RollControl:       %.2f (0x%08X)\n", m_fRollControl, *(uint32*)&m_fRollControl);
	printf("  ThrottleControl:   %.2f (0x%08X)\n", m_fThrottleControl, *(uint32*)&m_fThrottleControl);
	printf("  LGearAngle:        %.2f (0x%08X)\n", m_fLGearAngle, *(uint32*)&m_fLGearAngle);
	printf("================================\n");
}
#endif


cPlaneMG::cPlaneMG(sElement* elem) : cAutomobileBaseMG(elem)
{
	m_fPropellerAngle = 0.0f;
	m_vehType = eVehicleType::VEHICLE_TYPE_PLANE;
}

cPlaneMG::~cPlaneMG()
{

}

void cPlaneMG::PreRender(void)
{
	cMultiGame& Game = cMultiGame::Instance();
	cAutomobileBaseMG::PreRender();

	sPlane* pPlane = GetElement().plane;
	uint16 nTime = pPlane->m_nTime - static_cast<uint16>(Game.m_nLagValue);
	sPlaneSync* plane = pPlane->FindSync(nTime, nil).plane;

	CVehicleModelInfo* mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex());
	CColModel* colModel = mi->GetColModel();

	if (mi->b27C_40)
		DoHeliDustEffect();

	m_fPropellerAngle += plane->m_fEngineSpeed * CTimer::GetTimeStep();

	while (m_fPropellerAngle > TWOPI)
		m_fPropellerAngle -= TWOPI;

	int32 rotorAlpha = (1.5f - Min(1.7f * Max(plane->m_fEngineSpeed, 0.0f) / 0.22f, 1.5f)) * 255.0f;
	rotorAlpha = Min(rotorAlpha, 255);


	if (pPlane->GetOwner() != Game.LocalPlayerID())
	{
		TODO();
		TODO();
		TODO();
		TODO();
	}


	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();


	// todo recheck CAutomobile::PreRender
	//if (IsRealPlane())
	//{
	//	// Front wheel
	//	mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RF]));
	//	pos = mat.GetPosition();
	//	pos.z = m_aWheelPosition[CARWHEEL_FRONT_RIGHT];
	//	if (Damage.GetWheelStatus(CARWHEEL_FRONT_RIGHT) == WHEEL_STATUS_BURST)
	//		mat.SetRotate(m_aWheelRotation[CARWHEEL_FRONT_RIGHT], 0.0f, m_fSteerAngle + 0.3f * Sin(m_aWheelRotation[CARWHEEL_FRONT_RIGHT]));
	//	else
	//		mat.SetRotate(m_aWheelRotation[CARWHEEL_FRONT_RIGHT], 0.0f, m_fSteerAngle);
	//	mat.Scale(mi->GetWheelScale(true));
	//	mat.Translate(pos);
	//	mat.UpdateRW();

	//	// Rotate propeller
	//	if (m_aCarNodes[CAR_WINDSCREEN])
	//	{
	//		mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WINDSCREEN]));
	//		pos = mat.GetPosition();
	//		mat.SetRotateY(m_fPropellerRotation);
	//		mat.Translate(pos);
	//		mat.UpdateRW();

	//		m_fPropellerRotation += m_fGasPedal != 0.0f ? TWOPI / 13.0f : TWOPI / 26.0f;
	//		if (m_fPropellerRotation > TWOPI)
	//			m_fPropellerRotation -= TWOPI;
	//	}

	//	// Rudder
	//	if (Damage.GetDoorStatus(DOOR_BOOT) != DOOR_STATUS_MISSING && m_aCarNodes[CAR_BOOT])
	//	{
	//		mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_BOOT]));
	//		pos = mat.GetPosition();
	//		mat.SetRotate(0.0f, 0.0f, -m_fSteerAngle);
	//		mat.Rotate(0.0f, Sin(m_fSteerAngle) * DEGTORAD(22.0f), 0.0f);
	//		mat.Translate(pos);
	//		mat.UpdateRW();
	//	}
	//}


}

void cPlaneMG::Render(void)
{
	cAutomobileBaseMG::Render();
}

void cPlaneMG::DoHeliDustEffect(void)
{
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();

	// CVehicle::HeliDustGenerate()

	//int i;
	//float angle;
	//CColPoint point;
	//CEntity* entity;
	//uint8 r, g, b;

	//if (heli == nil)
	//	return;

	//uint8 surface = SURFACE_TARMAC;
	//int frm = CTimer::GetFrameCounter() & 7;
	//float testLowZ = ground - 10.0f;
	//float dustSize = 0.0f;
	//float baseSize = 1.0f;
	//float offset = 1.0f; // when heli is tilted
	//float particleZ = -101.0f;
	//int n = 0;

	//if (heli->GetModelIndex() == MI_RCGOBLIN || heli->GetModelIndex() == MI_RCRAIDER)
	//{
	//	radius = 3.0f;
	//	dustSize = 0.04f;
	//	baseSize = 0.07f;
	//	offset = 0.3f;
	//}

	//CVector heliPos = heli->GetPosition();

	//if (heli->IsVehicle() && ((CVehicle*)heli)->IsCar())
	//{
	//	heliPos.x -= (heliPos.z - ground) * heli->GetUp().x * offset * 0.5f;
	//	heliPos.y -= (heliPos.z - ground) * heli->GetUp().y * offset * 0.5f;
	//}

	//float steamSize = 0.25f * radius * baseSize;
	//float splashSize = 0.3f * radius * baseSize;

	//i = 0;
	//for (i = 0; i < 32 + rnd; i++)
	//{
	//	angle = i * TWOPI / 32.0f;
	//	CVector pos(radius * Cos(angle), radius * Sin(angle), 0.0f);
	//	CVector dir = CVector(pos.x, pos.y, 1.0f) * 0.01f;
	//	pos += heliPos;

	//	if (i < 32 && i == 4 * frm)
	//	{
	//		if (CWorld::ProcessVerticalLine(pos, testLowZ, point, entity, true, false, false, false, true, false, nil))
	//		{
	//			n = rnd;
	//			particleZ = point.point.z;
	//			surface = point.surfaceB;
	//		}
	//		else
	//			n = 0;

	//		float waterLevel = 0.0f;
	//		if (CWaterLevel::GetWaterLevel(pos, &waterLevel, false) && waterLevel > particleZ)
	//		{
	//			surface = SURFACE_WATER;
	//			n = rnd;
	//			particleZ = waterLevel;
	//		}
	//	}

	//	if (n)
	//	{
	//		pos.z = particleZ;
	//		if (surface == SURFACE_WATER)
	//		{
	//			float red = (0.3 * CTimeCycle::GetDirectionalRed() + CTimeCycle::GetAmbientRed_Obj()) * 255.0f / 4.0f;
	//			float green = (0.3 * CTimeCycle::GetDirectionalGreen() + CTimeCycle::GetAmbientGreen_Obj()) * 255.0f / 4.0f;
	//			float blue = (0.3 * CTimeCycle::GetDirectionalBlue() + CTimeCycle::GetAmbientBlue_Obj()) * 255.0f / 4.0f;
	//			r = Clamp(red, 0.0f, 255.0f);
	//			g = Clamp(green, 0.0f, 255.0f);
	//			b = Clamp(blue, 0.0f, 255.0f);
	//			RwRGBA col1 = { r, g, b, (RwUInt8)CGeneral::GetRandomNumberInRange(8, 32) };
	//			RwRGBA col2 = { 255, 255, 255, 32 };

	//			if (n & 1)
	//				CParticle::AddParticle(PARTICLE_STEAM_NY_SLOWMOTION, pos, dir, nil, steamSize, col2);
	//			else
	//				CParticle::AddParticle(PARTICLE_CAR_SPLASH, pos, dir, nil, splashSize, col1,
	//					CGeneral::GetRandomNumberInRange(0.0f, 10.0f),
	//					CGeneral::GetRandomNumberInRange(0.0f, 90.0f), 1);
	//		}
	//		else
	//		{
	//			switch (surface)
	//			{
	//			default:
	//			case SURFACE_TARMAC:
	//				r = 10;
	//				g = 10;
	//				b = 10;
	//				break;
	//			case SURFACE_GRASS:
	//				r = 10;
	//				g = 10;
	//				b = 3;
	//				break;
	//			case SURFACE_GRAVEL:
	//				r = 10;
	//				g = 8;
	//				b = 7;
	//				break;
	//			case SURFACE_MUD_DRY:
	//				r = 10;
	//				g = 6;
	//				b = 3;
	//				break;
	//			case SURFACE_SAND:
	//			case SURFACE_SAND_BEACH:
	//				r = 10;
	//				g = 10;
	//				b = 7;
	//				break;
	//			}
	//			RwRGBA col = { r, g, b, 32 };
	//			if (heliPos.z - pos.z < 20.0f)
	//				CParticle::AddParticle(PARTICLE_HELI_DUST, pos, dir, nil, dustSize, col);
	//		}

	//		n--;
	//	}
	//}
}


sPlane::sPlane() : sAutomobileBase()
{
	DECLARE_ELEMENT_CONSTRUCT(this, true, false);
	SetPhysical(new cPlaneMG(this));
	DECLARE_ELEMENT_CONSTRUCT(this, false, false);
}

sPlane::sPlane(CPlane* pPlane) : sAutomobileBase(/*pPlane*/)
{
	DECLARE_ELEMENT_CONSTRUCT(this, true, true);
	cMultiGame::Instance().AttachEntity(this, pPlane);
	SetEntity(pPlane);
	SetPhysical(new cPlaneMG(this));
	Initialise();
#ifdef FIX_BUGS
	// sVehicle
	m_fTraction = pPlane->m_fTraction;
	for (uint32 i = 0; i < ARRAY_SIZE(pPlane->m_aSuspensionLineLength); i++) // probably compiler loop optimization upper
		m_aSuspensionLineLength[i] = pPlane->m_aSuspensionLineLength[i];
	for (uint32 i = 0; i < ARRAY_SIZE(pPlane->m_aSuspensionSpringLength); i++)
		m_aSuspensionSpringLength[i] = pPlane->m_aSuspensionSpringLength[i];
	// already in Initialise, but still here to copy sVehicle all info
	m_aColours[VEHICLE_COLOUR_PRIMARY] = pPlane->m_aColours[VEHICLE_COLOUR_PRIMARY];
	m_aColours[VEHICLE_COLOUR_SECONDARY] = pPlane->m_aColours[VEHICLE_COLOUR_SECONDARY];
#endif
	AttachSync(m_nTime, new sPlaneSync(pPlane));
	TransferZone();
	DECLARE_ELEMENT_CONSTRUCT(this, false, true);
}

ElementCapability sPlane::GetCapability()
{
	return sPlane::Capability();
}

bool sPlane::HasCapability(ElementCapability capability)
{
	if (sPlane::Capability() == capability)
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

sPlane::~sPlane()
{
	DECLARE_ELEMENT_DESTRUCT(this);
	sElement::PurgeAttached();
	// ~sElement
}

sElementSync* sPlane::CreateSync() {
	return new sPlaneSync();
}

void sPlane::DisposeSync(sElementSync* pSync) {
	if (pSync)
		delete ((sPlaneSync*)pSync);
}

sElementSync* sPlane::CreateSyncFromOther(sElementSync* pSync) {
	sPlaneSync& sync = *(sPlaneSync*)pSync;
#ifdef FIX_BUGS
	return new sPlaneSync(sync);
#else
	sPlaneSync* pNewSync = new sPlaneSync(sync);
	// Base
	pNewSync->field_220 = sync.field_220;
	for (uint32 i = 0; i < NUM_DOORS; i++) {
		pNewSync->Doors[i] = sDoorSync(sync.Doors[i]);
	}

	// Plane
	pNewSync->m_fAttackLift = sync.m_fAttackLift;
	pNewSync->m_fGearDownLiftMult = sync.m_fGearDownLiftMult;
	pNewSync->m_fEngineSpeed = sync.m_fEngineSpeed;
	pNewSync->m_fYawControl = sync.m_fYawControl;
	pNewSync->m_fPitchControl = sync.m_fPitchControl;
	pNewSync->m_fRollControl = sync.m_fRollControl;
	pNewSync->m_fThrottleControl = sync.m_fThrottleControl;
	pNewSync->m_fLGearAngle = sync.m_fLGearAngle;
	return pNewSync;
#endif
}

bool sPlane::HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) {
	sPlaneSync& syncA = *(sPlaneSync*)pSyncA;
	sPlaneSync& syncB = *(sPlaneSync*)pSyncB;
	return syncA.Compare(syncB);
}

void sPlane::ApplyClientSync(uint16 nTime) {
	sAutomobileBase::ApplyClientSync(nTime);
}

void sPlane::Update(uint16 nTime) {
	if (GetEntity() != nil)
		AttachSync(nTime, new sPlaneSync((CPlane*)GetEntity()));
	else
		delete this; // ?
}

bool sPlane::WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) {
	if (static_cast<int16>(nSyncLastTime - m_vSync.front().m_nTime) >= 0) // FindSync? not GetSyncWithTime?
		return WriteSyncDelta(pSyncStream, FindSync(nSyncWriteTime, nil).plane, GetSyncWithTime(nSyncLastTime).plane, (nSyncWriteTime - nSyncLastTime));

	tPlaneSyncsDeltas planeDeltaManager{};
	planeDeltaManager.SetDifference(); // FindSync? not GetSyncWithTime?
	PerformWriteSync(pSyncStream, FindSync(nSyncWriteTime, nil).plane, &planeDeltaManager); // max diff
	return true;
}

void sPlane::ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) {
	sPlaneSync& sync = *(sPlaneSync*)pOutSync;
	uint32 nDiffMask = pSyncStream->ReadU32();

	if (nDiffMask & ePlaneSync::MP_PKTD_PLANE_AUTOBASE)
		sAutomobileBase::ReadSyncFromStreamAutomobileBase(pSyncStream, (sPlaneSync*)pOutSync);

	if (nDiffMask & ePlaneSync::MP_PKTD_PLANE_ATTACK_LIFT)
		sync.m_fAttackLift = pSyncStream->ReadFloat();

	if (nDiffMask & ePlaneSync::MP_PKTD_PLANE_GEAR_DOWN_LIFT_MULT)
		sync.m_fGearDownLiftMult = pSyncStream->ReadFloat();

	if (nDiffMask & ePlaneSync::MP_PKTD_PLANE_ENGINE_SPEED)
		sync.m_fEngineSpeed = pSyncStream->ReadFloat();

	if (nDiffMask & ePlaneSync::MP_PKTD_PLANE_YAW_CONTROL)
		sync.m_fYawControl = pSyncStream->ReadFloat();

	if (nDiffMask & ePlaneSync::MP_PKTD_PLANE_PITCH_CONTROL)
		sync.m_fPitchControl = pSyncStream->ReadFloat();

	if (nDiffMask & ePlaneSync::MP_PKTD_PLANE_ROLL_CONTROL)
		sync.m_fRollControl = pSyncStream->ReadFloat();

	if (nDiffMask & ePlaneSync::MP_PKTD_PLANE_THROTTLE_CONTROL)
		sync.m_fThrottleControl = pSyncStream->ReadFloat();

	if (nDiffMask & ePlaneSync::MP_PKTD_PLANE_LGEAR_ANGLE)
		sync.m_fLGearAngle = pSyncStream->ReadFloat();
}

void sPlane::ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) {
	cMultiGame& Game = cMultiGame::Instance();
	if (nOwner != Game.LocalPlayerID()) {
		sAutomobileBase::ReceiveEntity(nOwner, nID, nTime);
		return;
	}

	sPlaneSync* pSync = GetSync().plane;
	assert(GetPhysical());
	CPlane* plane = new CPlane(GetPhysical()->GetModelIndex(), pSync->VehicleCreatedBy);
	Game.AttachEntity(this, plane);
	SetEntity(plane);

#ifdef FIX_BUGS
	for (uint32 i = 0; i < ARRAY_SIZE(plane->m_aWheelTimer); i++)
		plane->m_aWheelTimer[i] = pSync->m_aWheelTimer[i];

	for (uint32 i = 0; i < ARRAY_SIZE(plane->m_aWheelState); i++)
		plane->m_aWheelState[i] = pSync->m_aWheelState[i];

	for (uint32 i = 0; i < ARRAY_SIZE(plane->m_aWheelRotation); i++)
		plane->m_aWheelRotation[i] = pSync->m_aWheelRotation[i];

	for (uint32 i = 0; i < ARRAY_SIZE(plane->m_aWheelPosition); i++)
		plane->m_aWheelPosition[i] = pSync->m_aWheelPosition[i];

	for (uint32 i = 0; i < ARRAY_SIZE(plane->m_aWheelSpeed); i++)
		plane->m_aWheelSpeed[i] = pSync->m_aWheelSpeed[i];

	for (uint32 i = 0; i < ARRAY_SIZE(plane->m_aSuspensionSpringRatio); i++)
		plane->m_aSuspensionSpringRatio[i] = pSync->m_aSuspensionSpringRatio[i];

	for (uint32 i = 0; i < ARRAY_SIZE(plane->m_aWheelColPoints); i++)
		plane->m_aWheelColPoints[i] = pSync->m_aWheelColPoints[i];

	for (uint32 i = 0; i < ARRAY_SIZE(plane->m_aWheelSkidmarkType); i++)
		plane->m_aWheelSkidmarkType[i] = pSync->m_aWheelSkidmarkType[i];

	// Skidmark flags
	for (uint32 i = 0; i < ARRAY_SIZE(plane->m_aWheelSkidmarkBloody); i++) {
		plane->m_aWheelSkidmarkBloody[i] = (pSync->m_nSkidmarkFlags & BIT(i));
		plane->m_aWheelSkidmarkUnk[i] = (pSync->m_nSkidmarkFlags & BIT(i + 4));
	}
#endif

	sAutomobileBase::ReceiveEntity(nOwner, nID, nTime);
#ifdef FIX_BUGS
	plane->ResetSuspension();
#endif
	AttachSync(m_nTime, new sPlaneSync(plane));
}

void sPlane::Initialise() {
	sAutomobileBase::Initialise();
}

void sPlane::SetupModel() {
	sAutomobileBase::SetupModel();
#ifdef FIX_BUGS
	SetupDoors();
#endif

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

void sPlane::CompareSyncState(sPlaneSync* pSync, sPlaneSync* pLastSync, uint32 nDelta, tPlaneSyncsDeltas* pDiff) {
	sAutomobileBase::CompareSyncState(pSync, pLastSync, nDelta, &pDiff->tAutomobileBaseDiff);

	// paste CPlane <-> CHeli
#ifdef FIX_BUGS
	if (pDiff->tAutomobileBaseDiff.nAutomobileBaseDiff != eAutomobileBaseSync::MP_PKTD_AUTOBASE_EQUAL)
#else
	if (pDiff->tAutomobileBaseDiff.nAutomobileBaseDiff != eAutomobileBaseSync::MP_PKTD_AUTOBASE_EQUAL ||
		pDiff->tAutomobileBaseDiff.tVehicleDiff.nVehicleDiff != eVehicleSync::MP_PKTD_VEH_EQUAL) // veh dif cmp useless, its already in nAutomobileBaseDiff
#endif
		pDiff->nPlaneDiff |= ePlaneSync::MP_PKTD_PLANE_AUTOBASE;

	if (FLT_EPS_NOT_EQ(pLastSync->m_fAttackLift, pSync->m_fAttackLift))
		pDiff->nPlaneDiff |= ePlaneSync::MP_PKTD_PLANE_ATTACK_LIFT;
#ifdef FIX_BUGS
	else // !!---- UPD!!
		pSync->m_fAttackLift = pLastSync->m_fAttackLift;
#endif

	if (FLT_EPS_NOT_EQ(pLastSync->m_fGearDownLiftMult, pSync->m_fGearDownLiftMult))
		pDiff->nPlaneDiff |= ePlaneSync::MP_PKTD_PLANE_GEAR_DOWN_LIFT_MULT;
#ifdef FIX_BUGS
	else // !!---- UPD!!
		pSync->m_fGearDownLiftMult = pLastSync->m_fGearDownLiftMult;
#endif

	if (FLT_EPS_NOT_EQ(pLastSync->m_fEngineSpeed, pSync->m_fEngineSpeed))
		pDiff->nPlaneDiff |= ePlaneSync::MP_PKTD_PLANE_ENGINE_SPEED;
#ifdef FIX_BUGS
	else // !!---- UPD!!
		pSync->m_fEngineSpeed = pLastSync->m_fEngineSpeed;
#endif

	if (FLT_EPS_NOT_EQ(pLastSync->m_fYawControl, pSync->m_fYawControl))
		pDiff->nPlaneDiff |= ePlaneSync::MP_PKTD_PLANE_YAW_CONTROL;
#ifdef FIX_BUGS
	else // !!---- UPD!!
		pSync->m_fYawControl = pLastSync->m_fYawControl;
#endif

	if (FLT_EPS_NOT_EQ(pLastSync->m_fPitchControl, pSync->m_fPitchControl))
		pDiff->nPlaneDiff |= ePlaneSync::MP_PKTD_PLANE_PITCH_CONTROL;
#ifdef FIX_BUGS
	else // !!---- UPD!!
		pSync->m_fPitchControl = pLastSync->m_fPitchControl;
#endif

	if (FLT_EPS_NOT_EQ(pLastSync->m_fRollControl, pSync->m_fRollControl))
		pDiff->nPlaneDiff |= ePlaneSync::MP_PKTD_PLANE_ROLL_CONTROL;
#ifdef FIX_BUGS
	else // !!---- UPD!!
		pSync->m_fRollControl = pLastSync->m_fRollControl;
#endif

	if (FLT_EPS_NOT_EQ(pLastSync->m_fThrottleControl, pSync->m_fThrottleControl))
		pDiff->nPlaneDiff |= ePlaneSync::MP_PKTD_PLANE_THROTTLE_CONTROL;
#ifdef FIX_BUGS
	else // !!---- UPD!!
		pSync->m_fThrottleControl = pLastSync->m_fThrottleControl;
#endif

	if (FLT_EPS_NOT_EQ(pLastSync->m_fLGearAngle, pSync->m_fLGearAngle))
		pDiff->nPlaneDiff |= ePlaneSync::MP_PKTD_PLANE_LGEAR_ANGLE;
#ifdef FIX_BUGS
	else // !!---- UPD!!
		pSync->m_fLGearAngle = pLastSync->m_fLGearAngle;
#endif
}

void sPlane::PerformWriteSync(sWriteSyncStream* pSyncStream, sPlaneSync* pSync, tPlaneSyncsDeltas* pDiff) {
	pSyncStream->WriteU32(pDiff->nPlaneDiff);

	if (pDiff->nPlaneDiff & ePlaneSync::MP_PKTD_PLANE_AUTOBASE)
		sAutomobileBase::PerformWriteSync(pSyncStream, pSync, &pDiff->tAutomobileBaseDiff);

	if (pDiff->nPlaneDiff & ePlaneSync::MP_PKTD_PLANE_ATTACK_LIFT)
		pSyncStream->WriteFloat(pSync->m_fAttackLift);

	if (pDiff->nPlaneDiff & ePlaneSync::MP_PKTD_PLANE_GEAR_DOWN_LIFT_MULT)
		pSyncStream->WriteFloat(pSync->m_fGearDownLiftMult);

	if (pDiff->nPlaneDiff & ePlaneSync::MP_PKTD_PLANE_ENGINE_SPEED)
		pSyncStream->WriteFloat(pSync->m_fEngineSpeed);

	if (pDiff->nPlaneDiff & ePlaneSync::MP_PKTD_PLANE_YAW_CONTROL)
		pSyncStream->WriteFloat(pSync->m_fYawControl);

	if (pDiff->nPlaneDiff & ePlaneSync::MP_PKTD_PLANE_PITCH_CONTROL)
		pSyncStream->WriteFloat(pSync->m_fPitchControl);

	if (pDiff->nPlaneDiff & ePlaneSync::MP_PKTD_PLANE_ROLL_CONTROL)
		pSyncStream->WriteFloat(pSync->m_fRollControl);

	if (pDiff->nPlaneDiff & ePlaneSync::MP_PKTD_PLANE_THROTTLE_CONTROL)
		pSyncStream->WriteFloat(pSync->m_fThrottleControl);

	if (pDiff->nPlaneDiff & ePlaneSync::MP_PKTD_PLANE_LGEAR_ANGLE)
		pSyncStream->WriteFloat(pSync->m_fLGearAngle);
}

bool sPlane::WriteSyncDelta(sWriteSyncStream* pSyncStream, sPlaneSync* pSync, sPlaneSync* pLastSync, uint32 nDelta) {
	tPlaneSyncsDeltas planeDeltaManager{};
	planeDeltaManager.SetEqual();
	CompareSyncState(pSync, pLastSync, nDelta, &planeDeltaManager);

#ifdef FIX_BUGS
	if (planeDeltaManager.nPlaneDiff == ePlaneSync::MP_PKTD_PLANE_EQUAL) // main delta
		return false;
#endif

	PerformWriteSync(pSyncStream, pSync, &planeDeltaManager);
	return true;
}

#ifdef FIX_BUGS
void sPlane::Fix() {
	;
}
#endif
#endif
