/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "multiplayer/MultiGame.h"
#include "multiplayer/elements/sSyncStream.h"
#include "multiplayer/elements/sBike.h"
#include "Bike.h"
#include "Coronas.h"
#include "Particle.h"
#include "Clock.h"
#include "Camera.h"
#include "SpecialFX.h"
#include "PlayerPed.h"
#include "PlayerInfo.h"
#include "PointLights.h"
#include "Shadows.h"
#include "DamageManager.h"
#include "Weather.h"


sBikeSync::sBikeSync() : sVehicleSync() {
    DECLARE_SYNC_CONSTRUCT(this);
#ifdef FIX_BUGS
    m_fBarSteerAngle = 0.0f;
    m_fLeanAngle = 0.0f;
    for (uint32 i = 0; i < ARRAY_SIZE(m_wheelStatus); i++)
        m_wheelStatus[i] = 0;
    field_22C = 0;
#endif
}

sBikeSync::sBikeSync(CBike* bike) : sVehicleSync(bike) {
    DECLARE_SYNC_CONSTRUCT(this);
#if 0 // orig do compiler optimization loop unroll
	if (&bike->m_wheelStatus[ARRAY_SIZE(m_wheelStatus)] != &bike->m_wheelStatus[0])
		memmove(m_wheelStatus, bike->m_wheelStatus, sizeof(bike->m_wheelStatus));
	if (&bike->m_aWheelTimer[ARRAY_SIZE(m_aWheelTimer)] != &bike->m_aWheelTimer[0])
		memmove(m_aWheelTimer, bike->m_aWheelTimer, sizeof(bike->m_aWheelTimer));
	if (&bike->m_aWheelState[ARRAY_SIZE(m_aWheelState)] != &bike->m_aWheelState[0])
		memmove(m_aWheelState, bike->m_aWheelState, sizeof(bike->m_aWheelState));
	if (&bike->m_aWheelRotation[ARRAY_SIZE(m_aWheelRotation)] != &bike->m_aWheelRotation[0])
		memmove(m_aWheelRotation, bike->m_aWheelRotation, sizeof(bike->m_aWheelRotation));
	if (&bike->m_aWheelPosition[ARRAY_SIZE(m_aWheelPosition)] != &bike->m_aWheelPosition[0])
		memmove(m_aWheelPosition, bike->m_aWheelPosition, sizeof(bike->m_aWheelPosition));
	if (&bike->m_aWheelSpeed[ARRAY_SIZE(m_aWheelSpeed)] != &bike->m_aWheelSpeed[0])
		memmove(m_aWheelSpeed, bike->m_aWheelSpeed, sizeof(bike->m_aWheelSpeed));
	if (&bike->m_aSuspensionSpringRatio[ARRAY_SIZE(m_aSuspensionSpringRatio)] != &bike->m_aSuspensionSpringRatio[0])
		memmove(m_aSuspensionSpringRatio, bike->m_aSuspensionSpringRatio, sizeof(bike->m_aSuspensionSpringRatio));
#else
	for (uint32 i = 0; i < ARRAY_SIZE(bike->m_wheelStatus); i++)
		m_wheelStatus[i] = bike->m_wheelStatus[i];

	for (uint32 i = 0; i < ARRAY_SIZE(bike->m_aWheelTimer); i++)
		m_aWheelTimer[i] = bike->m_aWheelTimer[i];

	for (uint32 i = 0; i < ARRAY_SIZE(bike->m_aWheelState); i++)
		m_aWheelState[i] = bike->m_aWheelState[i]; // 4 - 2

	for (uint32 i = 0; i < ARRAY_SIZE(bike->m_aWheelRotation); i++)
		m_aWheelRotation[i] = bike->m_aWheelRotation[i]; // 4 - 2

	for (uint32 i = 0; i < ARRAY_SIZE(bike->m_aWheelPosition); i++)
		m_aWheelPosition[i] = bike->m_aWheelPosition[i]; // 4 - 2

	for (uint32 i = 0; i < ARRAY_SIZE(bike->m_aWheelSpeed); i++)
		m_aWheelSpeed[i] = bike->m_aWheelSpeed[i]; // 4 - 2

	for (uint32 i = 0; i < ARRAY_SIZE(bike->m_aSuspensionSpringRatio); i++)
		m_aSuspensionSpringRatio[i] = bike->m_aSuspensionSpringRatio[i];
#endif

	for (uint32 i = 0; i < ARRAY_SIZE(bike->m_aWheelColPoints); i++)
		m_aWheelColPoints[i] = bike->m_aWheelColPoints[i];

	for (uint32 i = 0; i < ARRAY_SIZE(bike->m_aWheelSkidmarkType); i++)
		m_aWheelSkidmarkType[i] = bike->m_aWheelSkidmarkType[i];

	m_fBarSteerAngle = bike->GetRideAnimData()->m_fBarSteerAngle;
	m_nSkidmarkFlags = 0x0;
	m_fLeanAngle = bike->GetRideAnimData()->m_fLeanAngle;

	for (uint32 i = 0; i < ARRAY_SIZE(CBike::m_aWheelSkidmarkBloody); i++) {
		if (bike->m_aWheelSkidmarkBloody[i])
			m_nSkidmarkFlags |= BIT(i);
		else
			m_nSkidmarkFlags &= ~BIT(i);

		if (bike->m_aWheelSkidmarkUnk[i])
			m_nSkidmarkFlags |= BIT(i + 4);
		else
			m_nSkidmarkFlags &= ~BIT(i + 4);
	}
}

sBikeSync::sBikeSync(const sBikeSync& other) : sVehicleSync(other) {
    DECLARE_SYNC_CONSTRUCT(this);
#ifdef FIX_BUGS // its right, but in orig its init after creation obj
	m_fBarSteerAngle = other.m_fBarSteerAngle;
	m_fLeanAngle = other.m_fLeanAngle;
	for (uint32 i = 0; i < ARRAY_SIZE(m_wheelStatus); i++)
		m_wheelStatus[i] = other.m_wheelStatus[i];
	field_22C = other.field_22C;
#endif
}

sBikeSync::~sBikeSync() {
    DECLARE_SYNC_DESTRUCT(this);
}

// inlined
// not checks: m_fBarSteerAngle, m_fLeanAngle, m_wheelStatus
bool sBikeSync::Compare(const sBikeSync& other)
{
	if (!sVehicleSync::Compare(other))
		return false;

#ifdef FIX_BUGS
	if (m_fBarSteerAngle != other.m_fBarSteerAngle)
		return false;
	if (m_fLeanAngle != other.m_fLeanAngle)
		return false;
	if (memcmp(&m_wheelStatus, &other.m_wheelStatus, sizeof(m_wheelStatus)) != 0)
		return false;
#endif

	return true;
}

#if !defined(FINAL) && !defined(MASTER)
void sBikeSync::Dump()
{
	sVehicleSync::Dump();

	printf("=== sBikeSync Dump ===\n");
	printf("Bike:\n");
	printf("  BarSteerAngle: %.2f\n", m_fBarSteerAngle);
	printf("  LeanAngle:     %.2f\n", m_fLeanAngle);
	printf("  WheelStatus[0]: %u (0x%02X)\n", m_wheelStatus[0], m_wheelStatus[0]);
	printf("  WheelStatus[1]: %u (0x%02X)\n", m_wheelStatus[1], m_wheelStatus[1]);
	printf("  field_22C:      %d (0x%08X)\n", field_22C, field_22C);
	printf("================================\n");
}
#endif


cBikeMG::cBikeMG(sElement* elem) : cVehicleMG(elem)
{
	m_vehType = eVehicleType::VEHICLE_TYPE_BIKE;
}

cBikeMG::~cBikeMG()
{

}

// MISSED: shouldLightsBeOn, Coronas::RegisterCorona lights, CBrightLights::RegisterOne, CShadows::StoreCarLightShadow, CPointLights::AddLight
// MISSED: electrap siren, less skidmark code, WHEEL_STATUS_BURST, AddWheelDirtAndWater???, AddDamagedVehicleParticles??, StoreShadowForVehicle??
void cBikeMG::PreRender(void)
{
    cMultiGame& Game = cMultiGame::Instance();
    sBike* pBike = GetElement().bike;
    if (Game.IsElementOwnerLocalPlayer(pBike)) {
        CEntity* pEntity = pBike->GetEntity();
        assert(pEntity);
        pEntity->PreRender();
    }
    cVehicleMG::PreRender();

    uint16 nTime = pBike->m_nTime - static_cast<uint16>(Game.m_nLagValue);
    sBikeSync* bike = pBike->FindSync(nTime, nil).bike;

    if (!Game.IsElementOwnerLocalPlayer(pBike))
    {
        if (pBike->GetSync().bike->m_nStatus == eEntityStatus::STATUS_PHYSICS ||
            pBike->GetSync().bike->m_nStatus == eEntityStatus::STATUS_PLAYER ||
            pBike->GetSync().bike->m_nStatus == eEntityStatus::STATUS_PLAYER_PLAYBACKFROMBUFFER ||
            pBike->GetSync().bike->m_nStatus == eEntityStatus::STATUS_SIMPLE)
        {
            // Wheel particles

            bool bIsBloody = (bike->m_nSkidmarkFlags & BIT(BIKEWHEEL_REAR)); // m_aWheelSkidmarkBloody[BIKEWHEEL_REAR]
            if (bike->m_aWheelState[BIKEWHEEL_REAR] != WHEEL_STATE_NORMAL &&
                bike->m_aWheelColPoints[BIKESUSP_R2].surfaceB != SURFACE_WATER && bike->m_aWheelTimer[BIKESUSP_R2] > 0.0f)
            {
                static float smokeSize = 0.2f;
                CVector groundPos = bike->m_aWheelColPoints[BIKESUSP_R2].point;
                if (pBike->m_aSuspensionSpringRatio[BIKESUSP_R1] < 1.0f) // m_aSuspensionSpringRatioPrev?
                    groundPos = (groundPos + bike->m_aWheelColPoints[BIKESUSP_R1].point) / 2.0f;
                groundPos += Sin(bike->m_fLeanAngle) * 0.8f * GetColModel()->boundingBox.min.z * GetRight();
                CParticle::AddParticle(PARTICLE_RUBBER_SMOKE,
                    groundPos + CVector(0.0f, 0.0f, 0.25f), CVector(0.0f, 0.0f, 0.0f),
                    nil, smokeSize);

                CSkidmarks::RegisterOne((uintptr)this, groundPos, GetForward().x, GetForward().y,
                    bike->m_aWheelSkidmarkType[BIKEWHEEL_REAR], &bIsBloody);

                if (bike->m_aWheelState[BIKEWHEEL_REAR] == WHEEL_STATE_SPINNING &&
                    (CSurfaceTable::GetAdhesionGroup(bike->m_aWheelColPoints[BIKESUSP_R2].surfaceB) == ADHESIVE_HARD ||
                        CSurfaceTable::GetAdhesionGroup(bike->m_aWheelColPoints[BIKESUSP_R2].surfaceB) == ADHESIVE_ROAD))
                {
                    CParticle::AddParticle(PARTICLE_BURNINGRUBBER_SMOKE,
                        groundPos + CVector(0.0f, 0.0f, 0.25f),
                        CVector(0.0f, 0.0f, 0.0f));
                    CParticle::AddParticle(PARTICLE_BURNINGRUBBER_SMOKE,
                        groundPos + CVector(0.0f, 0.0f, 0.25f),
                        CVector(0.0f, 0.0f, 0.05f));
                }
            }
            else if ((bike->m_nSkidmarkFlags & BIT(BIKEWHEEL_REAR)) || (bike->m_nSkidmarkFlags & BIT(BIKEWHEEL_REAR + 4))) // m_aWheelSkidmarkBloody || m_aWheelSkidmarkUnk
            {
                CVector groundPos = bike->m_aWheelColPoints[BIKESUSP_R2].point;
                groundPos += Sin(bike->m_fLeanAngle) * 0.8f * GetColModel()->boundingBox.min.z * GetRight();

                CSkidmarks::RegisterOne((uintptr)this, groundPos, GetForward().x, GetForward().y,
                    bike->m_aWheelSkidmarkType[BIKEWHEEL_REAR], &bIsBloody);
            }
        }
    }

    CVehicleModelInfo* mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex());
    CColModel* colModel = mi->GetColModel();
    CMatrix mat;
    CVector pos;

    // Front fork
    if (pBike->m_aBikeNodes[BIKE_FORKS_FRONT])
    {
        mat.Attach(RwFrameGetMatrix(pBike->m_aBikeNodes[BIKE_FORKS_FRONT]));
        pos = mat.GetPosition();

        RwMatrix rwrot;
        // TODO: this looks like some weird ctor we don't have
        CMatrix rot;
        rot.m_attachment = &rwrot;
        rot.SetUnity();
        rot.UpdateRW();

        // Make rotation matrix with front fork as axis
        CVector forkAxis(0.0f, Sin(DEGTORAD(mi->m_bikeSteerAngle)), -Cos(DEGTORAD(mi->m_bikeSteerAngle)));
        forkAxis.Normalise(); // as if that's not already the case
        CQuaternion quat;
        quat.Set(&forkAxis, -bike->m_fBarSteerAngle);
        quat.Get(rot.m_attachment);
        rot.Update();

        // Transform fork
        mat.SetUnity();
        mat = mat * rot;
        mat.Translate(pos);
        mat.UpdateRW();

        if (pBike->m_aBikeNodes[BIKE_HANDLEBARS])
        {
            // Transform handle
            mat.Attach(RwFrameGetMatrix(pBike->m_aBikeNodes[BIKE_HANDLEBARS]));
            pos = mat.GetPosition();
            if (GetStatus() == STATUS_ABANDONED || GetStatus() == STATUS_WRECKED)
            {
                mat.SetUnity();
                mat = mat * rot;
                mat.Translate(pos);
            }
            else
                mat.SetTranslate(mat.GetPosition());
            mat.UpdateRW();
        }
    }

    // Rear fork
    if (pBike->m_aBikeNodes[BIKE_FORKS_REAR])
    {
        float sine = (bike->m_aWheelPosition[BIKEWHEEL_REAR] - pBike->m_aWheelBasePosition[BIKEWHEEL_REAR]) / pBike->m_fRearForkLength;
        mat.Attach(RwFrameGetMatrix(pBike->m_aBikeNodes[BIKE_FORKS_REAR]));
        pos = mat.GetPosition();
        mat.SetRotate(-Asin(sine), 0.0f, 0.0f);
        mat.Translate(pos);
        mat.UpdateRW();
    }

    // Front wheel
    mat.Attach(RwFrameGetMatrix(pBike->m_aBikeNodes[BIKE_WHEEL_FRONT]));
    pos.x = mat.GetPosition().x;
    pos.z = bike->m_aWheelPosition[BIKEWHEEL_FRONT] - pBike->m_fFrontForkZ;
    float y = (colModel->lines[BIKESUSP_F1].p0.y + colModel->lines[BIKESUSP_F2].p0.y) / 2.0f - pBike->m_fFrontForkY;
    pos.y = y - (bike->m_aWheelPosition[BIKEWHEEL_FRONT] - pBike->m_aWheelBasePosition[BIKEWHEEL_FRONT]) * pBike->m_fFrontForkSlope;
    if (bike->m_wheelStatus[BIKEWHEEL_FRONT] == WHEEL_STATUS_BURST)
        mat.SetRotate(bike->m_aWheelRotation[BIKEWHEEL_FRONT], 0.0f, 0.05f * Sin(bike->m_aWheelRotation[BIKEWHEEL_FRONT]));
    else
        mat.SetRotateX(bike->m_aWheelRotation[BIKEWHEEL_FRONT]);
    mat.Translate(pos);
    mat.UpdateRW();
    // and mudguard
    mat.Attach(RwFrameGetMatrix(pBike->m_aBikeNodes[BIKE_MUDGUARD]));
    mat.SetTranslateOnly(pos);
    mat.UpdateRW();

    // Rear wheel
    mat.Attach(RwFrameGetMatrix(pBike->m_aBikeNodes[BIKE_WHEEL_REAR]));
    pos = mat.GetPosition();
    if (bike->m_wheelStatus[BIKEWHEEL_REAR] == WHEEL_STATUS_BURST)
        mat.SetRotate(bike->m_aWheelRotation[BIKEWHEEL_REAR], 0.0f, 0.07f * Sin(bike->m_aWheelRotation[BIKEWHEEL_REAR]));
    else
        mat.SetRotateX(bike->m_aWheelRotation[BIKEWHEEL_REAR]);
    mat.Translate(pos);
    mat.UpdateRW();

    // Chassis
    if (pBike->m_aBikeNodes[BIKE_CHASSIS])
    {
        mat.Attach(RwFrameGetMatrix(pBike->m_aBikeNodes[BIKE_CHASSIS]));
        pos = mat.GetPosition();
        pos.z = (1.0f - Cos(bike->m_fLeanAngle)) * (0.9f * colModel->boundingBox.min.z);
        mat.SetRotateX(-0.05f * Abs(bike->m_fLeanAngle));
        mat.RotateY(bike->m_fLeanAngle);
        mat.Translate(pos);
        mat.UpdateRW();
    }

    // Exhaust smoke
    tHandlingData* pHandlingData = GET_HANDLING(GetModelIndex());
    float fwdSpeed = DotProduct(bike->GetMoveSpeed(), GetForward()) * 180.0f;
    if (bike->bEngineOn && !(pHandlingData->IsModelFlagSet(eHandlingModelFlags::HANDLING_MODEL_NO_EXHAUST)) && fwdSpeed < 130.0f)
    {
        CVector exhaustPos = mi->m_positions[BIKE_POS_EXHAUST];
        CVector pos1, pos2, dir;

        if (exhaustPos != CVector(0.0f, 0.0f, 0.0f))
        {
            dir.z = 0.0f;
            if (fwdSpeed < 10.0f)
            {
                CVector steerFwd(-Sin(bike->m_fSteerAngle), Cos(bike->m_fSteerAngle), 0.0f);
                steerFwd = Multiply3x3(GetMatrix(), steerFwd);
                float r = CGeneral::GetRandomNumberInRange(-0.06f, -0.03f);
                dir.x = steerFwd.x * r;
                dir.y = steerFwd.y * r;
            }
            else
            {
                dir.x = m_vecMoveSpeed.x;
                dir.y = m_vecMoveSpeed.y;
            }

            bool dblExhaust = false;
            pos1 = GetMatrix() * exhaustPos;
            if (pHandlingData->IsModelFlagSet(eHandlingModelFlags::HANDLING_MODEL_DBL_EXHAUST))
            {
                dblExhaust = true;
                pos2 = exhaustPos;
                pos2.x = -pos2.x;
                pos2 = GetMatrix() * pos2;
            }

            static float fumesLimit = 2.0f;
            if (CGeneral::GetRandomNumberInRange(1.0f, 3.0f) * (bike->m_fGasPedal + 1.1f) > fumesLimit)
            {
                CParticle::AddParticle(PARTICLE_EXHAUST_FUMES, pos1, dir);
                if (dblExhaust)
                    CParticle::AddParticle(PARTICLE_EXHAUST_FUMES, pos2, dir);

                //if (GetStatus() == STATUS_PLAYER && (CTimer::GetFrameCounter() & 3) == 0 &&
                //    CWeather::Rain == 0.0f)
                //{
                //    CVector camDist = GetPosition() - TheCamera.GetPosition();
                //    if (DotProduct(GetForward(), camDist) > 0.0f ||
                //        TheCamera.GetLookDirection() == LOOKING_LEFT ||
                //        TheCamera.GetLookDirection() == LOOKING_RIGHT)
                //    {
                //        if (dblExhaust)
                //            pos1 = 0.5f * pos1 + 0.5f * pos2;

                //        if (TheCamera.GetLookDirection() == LOOKING_LEFT ||
                //            TheCamera.GetLookDirection() == LOOKING_RIGHT)
                //            pos1 -= 0.2f * GetForward();

                //        // CParticle::AddParticle(PARTICLE_HEATHAZE, pos1, CVector(0.0f, 0.0f, 0.0f));
                //    }
                //}
            }
        }
    }

#ifdef MULTIGAME_ELEMENTS_IMPROVEMENTS
    // MISSED: shouldLightsBeOn, Coronas::RegisterCorona lights, CBrightLights::RegisterOne, CShadows::StoreCarLightShadow, CPointLights::AddLight
    // MISSED: electrap siren, less skidmark code, WHEEL_STATUS_BURST, AddWheelDirtAndWater???, AddDamagedVehicleParticles??, StoreShadowForVehicle??
#endif
}

//#ifdef FIX_BUGS // guessed compiler override optimization, guessed by cBmxMG
//void cBikeMG::Render(void)
//{
//    cVehicleMG::Render();
//}
//#endif


sBike::sBike() : sVehicle()
{
    DECLARE_ELEMENT_CONSTRUCT(this, true, false);
#ifdef FIX_BUGS
	for (uint32 i = 0; i < ARRAY_SIZE(m_aSuspensionSpringRatio); i++)
		m_aSuspensionSpringRatio[i] = 0.0f;
	for (uint32 i = 0; i < ARRAY_SIZE(m_aBikeNodes); i++)
		m_aBikeNodes[i] = nil;
#endif
	SetPhysical(new cBikeMG(this));
    DECLARE_ELEMENT_CONSTRUCT(this, false, false);
}

sBike::sBike(CBike* pBike) : sVehicle()
{
    DECLARE_ELEMENT_CONSTRUCT(this, true, true);
#ifdef FIX_BUGS
    //for (uint32 i = 0; i < ARRAY_SIZE(m_aSuspensionSpringRatio); i++)
    //    m_aSuspensionSpringRatio[i] = 0.0f;
    for (uint32 i = 0; i < ARRAY_SIZE(m_aBikeNodes); i++)
        m_aBikeNodes[i] = nil;
#endif
	cMultiGame::Instance().AttachEntity(this, pBike);
	SetEntity(pBike);
	SetPhysical(new cBikeMG(this)); // vcs inlined ctor cBikeMG
	Initialise();

    // sVehicle
	m_fTraction = pBike->m_fTraction;
	//memcpy(m_aSuspensionLineLength, pBike->m_aSuspensionLineLength, sizeof(pBike->m_aSuspensionLineLength)); // gcc unroll?
	//memcpy(m_aSuspensionSpringLength, pBike->m_aSuspensionSpringLength, sizeof(pBike->m_aSuspensionSpringLength));
	for (uint32 i = 0; i < ARRAY_SIZE(pBike->m_aSuspensionLineLength); i++) // probably compiler loop optimization upper
		m_aSuspensionLineLength[i] = pBike->m_aSuspensionLineLength[i];
	for (uint32 i = 0; i < ARRAY_SIZE(pBike->m_aSuspensionSpringLength); i++)
		m_aSuspensionSpringLength[i] = pBike->m_aSuspensionSpringLength[i];
#ifdef FIX_BUGS // already in Initialise, but still here to copy sVehicle all info
    m_aColours[VEHICLE_COLOUR_PRIMARY] = pBike->m_aColours[VEHICLE_COLOUR_PRIMARY];
    m_aColours[VEHICLE_COLOUR_SECONDARY] = pBike->m_aColours[VEHICLE_COLOUR_SECONDARY];
#endif

	AttachSync(m_nTime, new sBikeSync(pBike));
	TransferZone();

    // CBike
#ifdef FIX_BUGS
	sBikeSync* pSync = GetSync().bike;
	for (uint32 i = 0; i < ARRAY_SIZE(pBike->m_aSuspensionSpringRatio); i++)
		m_aSuspensionSpringRatio[i] = pSync->m_aSuspensionSpringRatio[i];
#else
	for (uint32 i = 0; i < ARRAY_SIZE(pBike->m_aSuspensionSpringRatio); i++)
		m_aSuspensionSpringRatio[i] = GetSync().bike->m_aSuspensionSpringRatio[i];
#endif
    DECLARE_ELEMENT_CONSTRUCT(this, false, true);
}


ElementCapability sBike::GetCapability()
{
	return sBike::Capability();
}

bool sBike::HasCapability(ElementCapability capability)
{
	if (sBike::Capability() == capability)
		return true;
	if (sVehicle::Capability() == capability)
		return true;
	if (sElementPhysical::Capability() == capability)
		return true;
	if (sElement::Capability() == capability)
		return true;
	return false;
}

sBike::~sBike()
{
    DECLARE_ELEMENT_DESTRUCT(this);
	sElement::PurgeAttached();
	// ~sVehicle
}

sElementSync* sBike::CreateSync() {
	return new sBikeSync();
}

void sBike::DisposeSync(sElementSync* pSync) {
	if(pSync)
		delete ((sBikeSync*)pSync);
}

sElementSync* sBike::CreateSyncFromOther(sElementSync* pSync) {
	sBikeSync& sync = *(sBikeSync*)pSync;
#ifdef FIX_BUGS
    return new sBikeSync(sync);
#else
    sBikeSync* pNewSync = new sBikeSync(sync);
    pNewSync->m_fBarSteerAngle = sync.m_fBarSteerAngle;
    pNewSync->m_fLeanAngle = sync.m_fLeanAngle;
    for (uint32 i = 0; i < ARRAY_SIZE(sBikeSync::m_wheelStatus); i++)
        pNewSync->m_wheelStatus[i] = sync.m_wheelStatus[i];
    //pNewSync->field_22C = sync.field_22C; // unused?
    return pNewSync;
#endif
}

bool sBike::HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) {
	sBikeSync& syncA = *(sBikeSync*)pSyncA;
	sBikeSync& syncB = *(sBikeSync*)pSyncB;
	return syncA.Compare(syncB);
}

void sBike::ApplyClientSync(uint16 nTime) {
	sVehicle::ApplyClientSync(nTime);

#ifdef FIX_BUGS
	sBikeSync* pSync = GetSync().bike;
	for (uint32 i = 0; i < ARRAY_SIZE(pSync->m_aSuspensionSpringRatio); i++)
		m_aSuspensionSpringRatio[i] = pSync->m_aSuspensionSpringRatio[i];
#else
	for (uint32 i = 0; i < ARRAY_SIZE(pSync->m_aSuspensionSpringRatio); i++)
		m_aSuspensionSpringRatio[i] = GetSync().bike->m_aSuspensionSpringRatio[i];
#endif
}

void sBike::Update(uint16 nTime) {
	if (GetEntity() != nil)
		AttachSync(nTime, new sBikeSync((CBike*)GetEntity()));
	else
		delete this; // ?
}

bool sBike::WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) {
	if (static_cast<int16>(nSyncLastTime - m_vSync.front().m_nTime) >= 0) // FindSync? not GetSyncWithTime?
		return WriteSyncDelta(pSyncStream, FindSync(nSyncWriteTime, nil).bike, GetSyncWithTime(nSyncLastTime).bike, (nSyncWriteTime - nSyncLastTime));

	tBikeSyncsDeltas bikeDeltaManager{};
	bikeDeltaManager.SetDifference(); // FindSync? not GetSyncWithTime?
	PerformWriteSync(pSyncStream, FindSync(nSyncWriteTime, nil).bike, &bikeDeltaManager); // max diff
	return true;
}

void sBike::ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) {
	sBikeSync& sync = *(sBikeSync*)pOutSync;
	uint16 nDiffMask = pSyncStream->ReadU16();

	if (nDiffMask & eBikeSync::MP_PKTD_BIKE_VEHICLE)
		sVehicle::ReadSyncFromStreamVehicle(pSyncStream, (sBikeSync*)pOutSync);

	if (nDiffMask & eBikeSync::MP_PKTD_BIKE_BAR_STEER_ANGLE)
		sync.m_fBarSteerAngle = pSyncStream->ReadFloat();

	if (nDiffMask & eBikeSync::MP_PKTD_BIKE_WHEEL_STATUS_0)
		sync.m_wheelStatus[0] = pSyncStream->ReadU8();

	if (nDiffMask & eBikeSync::MP_PKTD_BIKE_WHEEL_STATUS_1)
		sync.m_wheelStatus[1] = pSyncStream->ReadU8();

	if (nDiffMask & eBikeSync::MP_PKTD_BIKE_LEAN_ANGLE)
		sync.m_fLeanAngle = pSyncStream->ReadFloat();
}

void sBike::ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) {
	cMultiGame& Game = cMultiGame::Instance();
	if (nOwner != Game.LocalPlayerID()) {
		sVehicle::ReceiveEntity(nOwner, nID, nTime);
		return;
	}

	sBikeSync* pSync = GetSync().bike;
	assert(GetPhysical());
	CBike* bike = new CBike(GetPhysical()->GetModelIndex(), pSync->VehicleCreatedBy);
	Game.AttachEntity(this, bike);
	SetEntity(bike);

	// decomp do memmove here, i guess its gcc loop unroll or smth like this

	for (uint32 i = 0; i < ARRAY_SIZE(bike->m_aWheelTimer); i++)
		bike->m_aWheelTimer[i] = pSync->m_aWheelTimer[i];

	for (uint32 i = 0; i < ARRAY_SIZE(bike->m_aWheelState); i++)
		bike->m_aWheelState[i] = pSync->m_aWheelState[i]; // 2 - 4

	for (uint32 i = 0; i < ARRAY_SIZE(bike->m_aWheelRotation); i++)
		bike->m_aWheelRotation[i] = pSync->m_aWheelRotation[i]; // 2 - 4

	for (uint32 i = 0; i < ARRAY_SIZE(bike->m_aWheelPosition); i++)
		bike->m_aWheelPosition[i] = pSync->m_aWheelPosition[i]; // 2 - 4

	for (uint32 i = 0; i < ARRAY_SIZE(bike->m_aWheelSpeed); i++)
		bike->m_aWheelSpeed[i] = pSync->m_aWheelSpeed[i]; // 2 - 4

	for (uint32 i = 0; i < ARRAY_SIZE(bike->m_aSuspensionSpringRatio); i++)
		bike->m_aSuspensionSpringRatio[i] = pSync->m_aSuspensionSpringRatio[i];

	for (uint32 i = 0; i < ARRAY_SIZE(bike->m_aWheelColPoints); i++)
		bike->m_aWheelColPoints[i] = pSync->m_aWheelColPoints[i];

	for (uint32 i = 0; i < ARRAY_SIZE(bike->m_aWheelSkidmarkType); i++)
		bike->m_aWheelSkidmarkType[i] = pSync->m_aWheelSkidmarkType[i]; // 2 - 4

	for (uint32 i = 0; i < ARRAY_SIZE(bike->m_wheelStatus); i++)
		bike->m_wheelStatus[i] = pSync->m_wheelStatus[i];


	// Ride Anim Data
	bike->GetRideAnimData()->m_fLeanAngle = pSync->m_fLeanAngle;
	bike->GetRideAnimData()->m_fBarSteerAngle = pSync->m_fBarSteerAngle;

	// Skidmark flags
	for (uint32 i = 0; i < ARRAY_SIZE(bike->m_aWheelSkidmarkBloody); i++) {
		bike->m_aWheelSkidmarkBloody[i] = (pSync->m_nSkidmarkFlags & BIT(i));
		bike->m_aWheelSkidmarkUnk[i] = (pSync->m_nSkidmarkFlags & BIT(i + 4));
	}

	sVehicle::ReceiveEntity(nOwner, nID, nTime);
	bike->ResetSuspension();
	AttachSync(m_nTime, new sBikeSync(bike));
}

// CBike::CBike
// CBike::SetModelIndex -> CBike::SetupModelNodes
// CBike::SetupSuspensionLines
void sBike::SetupModel() {
	assert(GetPhysical());
	CVehicleModelInfo* mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(GetPhysical()->GetModelIndex());
	tHandlingData* pHandling = GET_HANDLING(GetPhysical()->GetModelIndex());

	// CBike::SetupModelNodes
	for (uint32 i = 0; i < ARRAY_SIZE(m_aBikeNodes); i++)
		m_aBikeNodes[i] = nil;
	CClumpModelInfo::FillFrameArray(GetPhysical()->GetClump(), m_aBikeNodes);

	CColModel* colModel = mi->GetColModel();
	if (colModel->lines == nil)
	{
		colModel->numLines = 4;
#ifdef FIX_BUGS // in idb CBike line allocator and sBike are different, i guess sBike bugly RwMalloc (delete[])
		colModel->lines = new CColLine[4];
#else
        colModel->lines = (CColLine*)RwMalloc(4 * sizeof(CColLine));
#endif
	}
	
	//colModel->lines[0].p0.z = FAKESUSPENSION; // BUG? this would make more sense in the if above

	m_fFrontForkSlope = Tan(DEGTORAD(mi->m_bikeSteerAngle));

	// CBike::SetupSuspensionLines
    CVector posn;
    float suspOffset = 0.0f;
    RwFrame* node = nil;
    RwMatrix* mat = RwMatrixCreate();

    //bool initialized = colModel->lines[0].p0.z != FAKESUSPENSION;

    for (int32 i = 0; i < 4; i++)
    {
        //if (initialized)
        //{
        //    posn = colModel->lines[i].p0;
        //    if (i < 2)
        //        posn.z = m_aWheelBasePosition[0];
        //    else
        //        posn.z = m_aWheelBasePosition[1];
        //}
        //else
        {
            switch (i)
            {
                case BIKESUSP_F1:
                    node = m_aBikeNodes[BIKE_WHEEL_FRONT];
                    suspOffset = 0.25f * mi->m_wheelScale;
                    break;
                case BIKESUSP_F2:
                    node = m_aBikeNodes[BIKE_WHEEL_FRONT];
                    suspOffset = -0.25f * mi->m_wheelScale;
                    break;
                case BIKESUSP_R1:
                    node = m_aBikeNodes[BIKE_WHEEL_REAR];
                    suspOffset = 0.25f * mi->m_wheelScale;
                    break;
                case BIKESUSP_R2:
                    node = m_aBikeNodes[BIKE_WHEEL_REAR];
                    suspOffset = -0.25f * mi->m_wheelScale;
                    break;
            }

            GetRelativeMatrix(mat, node, node);
            posn = *RwMatrixGetPos(mat);
            if (i == BIKESUSP_F1)
                m_aWheelBasePosition[BIKEWHEEL_FRONT] = posn.z;
            else if (i == BIKESUSP_R1)
            {
                m_aWheelBasePosition[BIKEWHEEL_REAR] = posn.z;

                GetRelativeMatrix(mat, m_aBikeNodes[BIKE_FORKS_REAR], m_aBikeNodes[BIKE_FORKS_REAR]);
                float dz = posn.z - RwMatrixGetPos(mat)->z;
                float dy = posn.y - RwMatrixGetPos(mat)->y;
                m_fRearForkLength = Sqrt(SQR(dy) + SQR(dz));
                assert(m_fRearForkLength != 0.0f); // we want to divide by this
            }
            posn.y += suspOffset;
        }

        // uppermost wheel position
        posn.z += pHandling->fSuspensionUpperLimit;
        colModel->lines[i].p0 = posn;

        // lowermost wheel position
        posn.z += pHandling->fSuspensionLowerLimit - pHandling->fSuspensionUpperLimit;
        // lowest point on tyre
        posn.z -= mi->m_wheelScale * 0.5f;
        colModel->lines[i].p1 = posn;

        // this is length of the spring at rest
        m_aSuspensionSpringLength[i] = pHandling->fSuspensionUpperLimit - pHandling->fSuspensionLowerLimit;
        m_aSuspensionLineLength[i] = colModel->lines[i].p0.z - colModel->lines[i].p1.z;
    }

    //if (!initialized)
    {
        GetRelativeMatrix(mat, m_aBikeNodes[BIKE_FORKS_FRONT], m_aBikeNodes[BIKE_FORKS_FRONT]);
        m_fFrontForkY = RwMatrixGetPos(mat)->y;
        m_fFrontForkZ = RwMatrixGetPos(mat)->z;
    }

    //// Compress spring somewhat to get normal height on road
    //m_fHeightAboveRoad = m_aSuspensionSpringLength[0] * (1.0f - 1.0f / (4.0f * m_pHandling->fSuspensionForceLevel)) - colModel->lines[0].p0.z + mi->m_wheelScale * 0.5f;
    //for (i = 0; i < 2; i++)
    //    m_aWheelPosition[i] = mi->m_wheelScale * 0.5f - m_fHeightAboveRoad;

    // adjust col model to include suspension lines
    if (colModel->boundingBox.min.z > colModel->lines[0].p1.z)
        colModel->boundingBox.min.z = colModel->lines[0].p1.z;
    float radius = Max(colModel->boundingBox.min.Magnitude(), colModel->boundingBox.max.Magnitude());
    if (colModel->boundingSphere.radius < radius)
        colModel->boundingSphere.radius = radius;

#ifdef FIX_BUGS
    RwMatrixDestroy(mat);
#endif
}

void sBike::CompareSyncState(sBikeSync* pSync, sBikeSync* pLastSync, uint32 nDelta, tBikeSyncsDeltas* pDiff) {
	sVehicle::CompareSyncState(pSync, pLastSync, nDelta, &pDiff->tVehicleDiff);

	if (pDiff->tVehicleDiff.nVehicleDiff != eVehicleSync::MP_PKTD_VEH_EQUAL)
		pDiff->nBikeDiff |= eBikeSync::MP_PKTD_BIKE_VEHICLE;

	// Bar steer angle
	if (FLT_EPS_NOT_EQ(pLastSync->m_fBarSteerAngle, pSync->m_fBarSteerAngle))
		pDiff->nBikeDiff |= eBikeSync::MP_PKTD_BIKE_BAR_STEER_ANGLE;
	else // !!---- UPD!!
		pSync->m_fBarSteerAngle = pLastSync->m_fBarSteerAngle;

	// Wheel status 0
	if (pLastSync->m_wheelStatus[0] != pSync->m_wheelStatus[0])
		pDiff->nBikeDiff |= eBikeSync::MP_PKTD_BIKE_WHEEL_STATUS_0;

	// Wheel status 1
	if (pLastSync->m_wheelStatus[1] != pSync->m_wheelStatus[1])
		pDiff->nBikeDiff |= eBikeSync::MP_PKTD_BIKE_WHEEL_STATUS_1;

	// Lean angle
	if (pLastSync->m_fLeanAngle != pSync->m_fLeanAngle) // idb cmp as int?
		pDiff->nBikeDiff |= eBikeSync::MP_PKTD_BIKE_LEAN_ANGLE;
}


void sBike::PerformWriteSync(sWriteSyncStream* pSyncStream, sBikeSync* pSync, tBikeSyncsDeltas* pDiff) {
	pSyncStream->WriteU16(pDiff->nBikeDiff);

	if (pDiff->nBikeDiff & eBikeSync::MP_PKTD_BIKE_VEHICLE)
		sVehicle::PerformWriteSync(pSyncStream, pSync, &pDiff->tVehicleDiff);

	if (pDiff->nBikeDiff & eBikeSync::MP_PKTD_BIKE_BAR_STEER_ANGLE)
		pSyncStream->WriteFloat(pSync->m_fBarSteerAngle);

	if (pDiff->nBikeDiff & eBikeSync::MP_PKTD_BIKE_WHEEL_STATUS_0)
		pSyncStream->WriteU8(pSync->m_wheelStatus[0]);

	if (pDiff->nBikeDiff & eBikeSync::MP_PKTD_BIKE_WHEEL_STATUS_1)
		pSyncStream->WriteU8(pSync->m_wheelStatus[1]);

	if (pDiff->nBikeDiff & eBikeSync::MP_PKTD_BIKE_LEAN_ANGLE)
		pSyncStream->WriteFloat(pSync->m_fLeanAngle);
}

bool sBike::WriteSyncDelta(sWriteSyncStream* pSyncStream, sBikeSync* pSync, sBikeSync* pLastSync, uint32 nDelta) {
	tBikeSyncsDeltas bikeDeltaManager{};
	bikeDeltaManager.SetEqual();
	CompareSyncState(pSync, pLastSync, nDelta, &bikeDeltaManager);

	if (bikeDeltaManager.nBikeDiff == eBikeSync::MP_PKTD_BIKE_EQUAL) // main delta
		return false;

	PerformWriteSync(pSyncStream, pSync, &bikeDeltaManager);
	return true;
}

void sBike::Fix() {
	;
}