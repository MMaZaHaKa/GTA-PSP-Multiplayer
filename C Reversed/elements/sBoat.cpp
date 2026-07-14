/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "multiplayer/MultiGame.h"
#include "multiplayer/elements/sSyncStream.h"
#include "multiplayer/elements/sBoat.h"
#include "VisibilityPlugins.h"
#include "Particle.h"
#include "ParticleObject.h"
#include "WaterLevel.h"
#include "TimeCycle.h"


#ifndef GTA_LIBERTY
sBoatSync::sBoatSync() : sVehicleSync() {
	DECLARE_SYNC_CONSTRUCT(this);
}

sBoatSync::sBoatSync(CBoat* pBoat) : sVehicleSync(pBoat) {
	DECLARE_SYNC_CONSTRUCT(this);
}

sBoatSync::sBoatSync(const sBoatSync& other) : sVehicleSync(other) {
	DECLARE_SYNC_CONSTRUCT(this);
}

sBoatSync::~sBoatSync() {
	DECLARE_SYNC_DESTRUCT(this);
}

bool sBoatSync::Compare(const sBoatSync& other)
{
	if (!sVehicleSync::Compare(other))
		return false;

	return true;
}

#if !defined(FINAL) && !defined(MASTER)
void sBoatSync::Dump()
{
	sVehicleSync::Dump();

	printf("=== sBoatSync Dump ===\n");
	printf("================================\n");
}
#endif


cBoatMG::cBoatMG(sElement* elem) : cVehicleMG(elem)
{
	m_vehType = eVehicleType::VEHICLE_TYPE_BOAT;
	m_pLeftWake = new CWaterWake();
	m_pRightWake = new CWaterWake();
}

cBoatMG::~cBoatMG()
{
	delete m_pLeftWake;
	delete m_pRightWake;
}

void cBoatMG::PreRender(void)
{
	cMultiGame& Game = cMultiGame::Instance();
	sBoat* pBoat = GetElement().boat;
	cVehicleMG::PreRender();
	m_pLeftWake->Process();
	m_pRightWake->Process();

	uint16 nTime = pBoat->m_nTime - static_cast<uint16>(Game.m_nLagValue);
	sBoatSync* bike = pBoat->FindSync(nTime, nil).boat;

#if 0 // re3
	RwRGBA dropColor = { 0, 0, 0, 0 };
	int32 r, g, b;
	RwRGBA splashColor, jetColor;
	r = 127.5f * (CTimeCycle::GetAmbientRed_Obj() + 0.5f * CTimeCycle::GetDirectionalRed());
	g = 127.5f * (CTimeCycle::GetAmbientGreen_Obj() + 0.5f * CTimeCycle::GetDirectionalGreen());
	b = 127.5f * (CTimeCycle::GetAmbientBlue_Obj() + 0.5f * CTimeCycle::GetDirectionalBlue());
	r = Clamp(r, 0, 255);
	g = Clamp(g, 0, 255);
	b = Clamp(b, 0, 255);
	splashColor.red = r;
	splashColor.green = g;
	splashColor.blue = b;
	splashColor.alpha = CGeneral::GetRandomNumberInRange(160, 196);

	// 229.5? 153.0f with 0.6f
	r = 229.5f * (CTimeCycle::GetAmbientRed() + 0.85f * CTimeCycle::GetDirectionalRed());
	g = 229.5f * (CTimeCycle::GetAmbientGreen() + 0.85f * CTimeCycle::GetDirectionalGreen());
	b = 229.5f * (CTimeCycle::GetAmbientBlue() + 0.85f * CTimeCycle::GetDirectionalBlue());
	r = Clamp(r, 0, 255);
	g = Clamp(g, 0, 255);
	b = Clamp(b, 0, 255);
	jetColor.red = r;
	jetColor.green = g;
	jetColor.blue = b;
	jetColor.alpha = CGeneral::GetRandomNumberInRange(196, 228);
#else
	CRGBA dropColor = { 0, 0, 0, 0 };
	CRGBA splashColor = CTimeCycle::CalculateColor(160, 196, 0.5f, 0.5f);
	CRGBA jetColor = CTimeCycle::CalculateColor(196, 228, 0.6f, 0.85f); // 255 * 0.9f = 229.5 ??
#endif

	CVehicleModelInfo* mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex());
	tBoatHandlingData* pBoatHandling = GET_BOAT_HANDLING(GetModelIndex());

	TODO();
	TODO();
	TODO();
	TODO();


	// TODO this, tmp paste from CBoat
//    // Damage particles
//    if (m_fHealth <= 460.0f && GetStatus() != STATUS_WRECKED &&
//        Abs(GetPosition().x - TheCamera.GetPosition().x) < 200.0f &&
//        Abs(GetPosition().y - TheCamera.GetPosition().y) < 200.0f)
//    {
//        float speedSq = m_vecMoveSpeed.MagnitudeSqr();
//        CVector smokeDir = 0.8f * m_vecMoveSpeed;
//        CVector smokePos;
//        switch (GetModelIndex())
//        {
//        case MI_SPEEDER:
//            smokePos = CVector(0.4f, -2.4f, 0.8f);
//            smokeDir += 0.05f * GetRight();
//            smokeDir.z += 0.2f * m_vecMoveSpeed.z;
//            break;
//        case MI_REEFER:
//            smokePos = CVector(2.0f, -1.0f, 0.5f);
//            smokeDir += 0.07f * GetRight();
//            break;
//        case MI_PREDATOR:
//        default:
//            smokePos = CVector(-1.5f, -0.5f, 1.2f);
//            smokeDir += -0.08f * GetRight();
//            break;
//        }
//
//        smokePos = GetMatrix() * smokePos;
//
//        // On fire
//        if (m_fHealth < 250.0f)
//        {
//            CParticle::AddParticle(PARTICLE_CARFLAME, smokePos,
//                CVector(0.0f, 0.0f, CGeneral::GetRandomNumberInRange(2.25f / 200.0f, 0.09f)),
//                nil, 0.9f);
//            CVector smokePos2 = smokePos;
//            smokePos2.x += CGeneral::GetRandomNumberInRange(-2.25f / 4.0f, 2.25f / 4.0f);
//            smokePos2.y += CGeneral::GetRandomNumberInRange(-2.25f / 4.0f, 2.25f / 4.0f);
//            smokePos2.z += CGeneral::GetRandomNumberInRange(2.25f / 4.0f, 2.25f);
//            CParticle::AddParticle(PARTICLE_ENGINE_SMOKE2, smokePos2, CVector(0.0f, 0.0f, 0.0f));
//
//            m_fDamage += CTimer::GetTimeStepInMilliseconds();
//            if (m_fDamage > 5000.0f)
//                BlowUpCar(m_pSetOnFireEntity);
//        }
//
//        if (speedSq < 0.25f && (CTimer::GetFrameCounter() + m_randomSeed) & 1)
//            CParticle::AddParticle(PARTICLE_ENGINE_STEAM, smokePos, smokeDir);
//        if (speedSq < 0.25f && m_fHealth <= 390.0f)
//            CParticle::AddParticle(PARTICLE_ENGINE_SMOKE, smokePos, 1.25f * smokeDir);
//    }
//
//    bool bSeparateTurnForce = bHasHitWall;
//    CPhysical::ProcessControl();
//
//    CVector buoyanceImpulse(0.0f, 0.0f, 0.0f);
//    CVector buoyancePoint(0.0f, 0.0f, 0.0f);
//    if (mod_Buoyancy.ProcessBuoyancyBoat(this, m_pHandling->fBuoyancy, &buoyancePoint, &buoyanceImpulse, bSeparateTurnForce))
//    {
//        // Process boat in water
//        if (0.1f * m_fMass * GRAVITY * CTimer::GetTimeStep() < buoyanceImpulse.z)
//        {
//            bBoatInWater = true;
//            bIsInWater = true;
//            if (GetUp().z < -0.6f && Abs(GetMoveSpeed().x) < 0.05 && Abs(GetMoveSpeed().y) < 0.05)
//            {
//                bIsDrowning = true;
//                if (pDriver)
//                {
//                    pDriver->bTouchingWater = true;
//                    pDriver->InflictDamage(nil, WEAPONTYPE_DROWNING, CTimer::GetTimeStep(), PEDPIECE_TORSO, 0);
//                }
//            }
//            else
//                bIsDrowning = false;
//        }
//        else
//        {
//            bBoatInWater = false;
//            bIsInWater = false;
//            bIsDrowning = false;
//        }
//
//        m_fVolumeUnderWater = mod_Buoyancy.m_volumeUnderWater;
//        if (GetModelIndex() == MI_SKIMMER && GetUp().z < -0.5f && Abs(m_vecMoveSpeed.x) < 0.2f && Abs(m_vecMoveSpeed.y) < 0.2f)
//            ApplyMoveForce(0.03f * buoyanceImpulse);
//        else
//            ApplyMoveForce(buoyanceImpulse);
//        if (bSeparateTurnForce)
//            ApplyTurnForce(0.4f * buoyanceImpulse, buoyancePoint);
//
//        // TODO: what is this?
//        if (GetModelIndex() == MI_SKIMMER)
//            if (m_skimmerThingTimer != 0.0f ||
//                GetForward().z < -0.5f && GetUp().z > -0.5f && m_vecMoveSpeed.z < -0.15f &&
//                buoyanceImpulse.z > 0.01f * m_fMass * GRAVITY * CTimer::GetTimeStep() &&
//                buoyanceImpulse.z < 0.4f * m_fMass * GRAVITY * CTimer::GetTimeStep())
//            {
//                float turnImpulse = -0.00017f * GetForward().z * buoyanceImpulse.z * m_fMass * CTimer::GetTimeStep();
//                ApplyTurnForce(turnImpulse * GetForward(), GetUp());
//                bBoatInWater = false;
//                // BUG? aren't we forgetting the timestep here?
//                float moveImpulse = -0.5f * DotProduct(m_vecMoveSpeed, GetForward()) * m_fMass;
//                ApplyMoveForce(moveImpulse * GetForward());
//                if (m_skimmerThingTimer == 0.0f)
//                    m_skimmerThingTimer = CTimer::GetTimeInMilliseconds() + 300.0f;
//                else if (m_skimmerThingTimer < CTimer::GetTimeInMilliseconds())
//                    m_skimmerThingTimer = 0.0f;
//            }
//
//        if (!onLand && bBoatInWater && GetUp().z > 0.0f)
//        {
//            float impulse = m_vecMoveSpeed.MagnitudeSqr() * m_pHandlingBoat->fAqPlaneForce * buoyanceImpulse.z * CTimer::GetTimeStep() * 0.5f;
//            if (GetModelIndex() == MI_SKIMMER)
//                impulse *= 1.0f + m_fGasPedal;
//            else if (m_fGasPedal > 0.05f)
//                impulse *= m_fGasPedal;
//            else
//                impulse = 0.0f;
//            impulse = Min(impulse, GRAVITY * m_pHandlingBoat->fAqPlaneLimit * m_fMass * CTimer::GetTimeStep());
//            ApplyMoveForce(impulse * GetUp());
//            ApplyTurnForce(impulse * GetUp(), buoyancePoint - m_pHandlingBoat->fAqPlaneOffset * GetForward());
//        }
//
//        // Handle boat moving forward
//        float fwdSpeed = 1.0f;
//        if (Abs(m_fGasPedal) > 0.05f || (fwdSpeed = m_vecMoveSpeed.Magnitude2D()) > 0.01f)
//        {
//            if (bBoatInWater && fwdSpeed > 0.05f)
//                AddWakePoint(GetPosition());
//
//            float steerFactor = 1.0f;
//            if (GetStatus() == STATUS_PLAYER)
//            {
//                float steerLoss = DotProduct(m_vecMoveSpeed, GetForward()) * m_pHandling->fTractionBias;
//                if (CPad::GetPad(0)->GetHandBrake())
//                    steerLoss *= 0.5f;
//                steerFactor -= steerLoss;
//                steerFactor = Clamp(steerFactor, 0.0f, 1.0f);
//            }
//
//            CVector boundMin = GetColModel()->boundingBox.min;
//            CVector propeller(0.0f, boundMin.y * m_pHandlingBoat->fThrustY, boundMin.z * m_pHandlingBoat->fThrustZ);
//            propeller = Multiply3x3(GetMatrix(), propeller);
//            CVector propellerWorld = GetPosition() + propeller;
//
//            float steerSin = Sin(-m_fSteerAngle * steerFactor);
//            float steerCos = Cos(-m_fSteerAngle * steerFactor);
//            float waterLevel;
//            CWaterLevel::GetWaterLevel(propellerWorld, &waterLevel, true);
//            if (propellerWorld.z - 0.5f < waterLevel)
//            {
//                float propellerDepth = waterLevel - (propellerWorld.z - 0.5f);
//                if (propellerDepth > 1.0f)
//                    propellerDepth = 1.0f;
//                else
//                    propellerDepth = SQR(propellerDepth);
//                bPropellerInWater = true;
//
//                bool bSlowAhead = false;
//                if (Abs(m_fGasPedal) > 0.01f && GetModelIndex() != MI_SKIMMER)
//                {
//                    if (Abs(m_fGasPedal) < 0.05f)
//                        bSlowAhead = true;
//
//                    CVector forceDir = Multiply3x3(GetMatrix(), CVector(-steerSin, steerCos, -Abs(m_fSteerAngle)));
//                    CVector force = propellerDepth * m_fGasPedal * 40.0f * m_pHandling->Transmission.fEngineAcceleration * m_pHandling->fMass * forceDir;
//                    if (force.z > 0.2f)
//                        force.z = SQR(1.2f - force.z) + 0.2f;
//                    if (onLand)
//                    {
//                        if (m_fGasPedal < 0.0f)
//                        {
//                            force.x *= 5.0f;
//                            force.y *= 5.0f;
//                        }
//                        if (force.z < 0.0f)
//                            force.z = 0.0f;
//                        ApplyMoveForce(force * CTimer::GetTimeStep());
//                    }
//                    else
//                    {
//                        ApplyMoveForce(force * CTimer::GetTimeStep());
//                        ApplyTurnForce(force * CTimer::GetTimeStep(), propeller - m_pHandlingBoat->fThrustAppZ * GetUp());
//                        float rightForce = -DotProduct(GetRight(), force) * m_pHandling->fTractionMultiplier;
//                        ApplyTurnForce(rightForce * GetRight() * CTimer::GetTimeStep(), GetUp());
//                    }
//
//                    // Spray some particles
//                    CVector jetDir = -0.04f * force;
//                    if (m_fGasPedal > 0.0f)
//                    {
//                        if (GetStatus() == STATUS_PLAYER)
//                        {
//                            CVector sternPos = GetColModel()->boundingBox.min;
//                            sternPos.x = 0.0f;
//                            sternPos.z = 0.0f;
//                            sternPos = Multiply3x3(GetMatrix(), sternPos);
//
//                            CVector wakePos = GetPosition() + sternPos;
//                            // no actual particles for player...
//                        }
//                        else if (IsVisible() && ((CTimer::GetFrameCounter() + m_randomSeed) & 1) &&
//                            CVisibilityPlugins::GetDistanceSquaredFromCamera(&propellerWorld) < SQR(70.0f * TheCamera.GenerationDistMultiplier))
//                        {
//                            jetDir.z = 0.015f;
//                            jetDir.x *= 3.5f;
//                            jetDir.y *= 3.5f;
//                            propellerWorld.z += 0.5f;
//
//                            CParticle::AddParticle(PARTICLE_BOAT_SPLASH, propellerWorld, jetDir, nil, 1.25f, jetColor,
//                                CGeneral::GetRandomNumberInRange(0, 5),
//                                CGeneral::GetRandomNumberInRange(0, 90), 1, 500);
//
//                            CParticle::AddParticle(PARTICLE_CAR_SPLASH, propellerWorld, 0.75f * jetDir, nil, 0.5f, splashColor,
//                                CGeneral::GetRandomNumberInRange(0, 30),
//                                CGeneral::GetRandomNumberInRange(0, 45), 3, 500);
//                        }
//                    }
//                }
//                else
//                    bSlowAhead = true;
//
//                if (!onLand && bSlowAhead)
//                {
//                    float force = m_pHandling->fTractionLoss * DotProduct(m_vecMoveSpeed, GetForward());
//                    force = Min(force, 0.01f * m_fTurnMass);
//                    if (m_fGasPedal > 0.01f)
//                    {
//                        if (GetStatus() == STATUS_PLAYER)
//                            force *= (0.55f - Abs(m_fGasPedal)) * 1.3f;
//                        else
//                            force *= (0.55f - Abs(m_fGasPedal)) * 2.5f;
//                    }
//                    if (m_fGasPedal < 0.0f && force > 0.0f || m_fGasPedal > 0.0f && force < 0.0f)
//                        force *= -1.0f;
//                    CVector propellerForce = propellerDepth * Multiply3x3(GetMatrix(), force * CVector(-steerSin, 0.0f, 0.0f));
//                    ApplyMoveForce(propellerForce * CTimer::GetTimeStep());
//                    ApplyTurnForce(propellerForce * CTimer::GetTimeStep(), propeller);
//                    float rightForce = -steerSin * force * 0.75f / steerFactor * Max(CTimer::GetTimeStep(), 0.01f);
//                    ApplyTurnForce(GetRight() * rightForce, GetUp());
//                }
//            }
//            else
//                bPropellerInWater = false;
//
//            if (m_pHandling->fSuspensionBias != 0.0f)
//            {
//                CVector right = CrossProduct(GetForward(), CVector(0.0f, 0.0f, 1.0f));
//                float rightSpeed = DotProduct(m_vecMoveSpeed, right);
//                float impulse = 0.1f * m_pHandling->fSuspensionBias * m_fMass * m_fVolumeUnderWater * rightSpeed * CTimer::GetTimeStep();
//                ApplyMoveForce(right - impulse * 0.3f * CVector(-right.y, right.x, 0.0f));
//            }
//
//            if (GetStatus() == STATUS_PLAYER && CPad::GetPad(0)->GetHandBrake())
//            {
//                float fwdSpeed = DotProduct(m_vecMoveSpeed, GetForward());
//                if (fwdSpeed > 0.0f)
//                {
//                    float impulse = -0.1f * m_pHandling->fSuspensionLowerLimit * m_fMass * m_fVolumeUnderWater * fwdSpeed * CTimer::GetTimeStep();
//                    ApplyMoveForce(impulse * GetForward());
//                }
//            }
//        }
//
//        // Slow down or push down boat as it approaches the world limits
//        m_vecMoveSpeed.x = Min(m_vecMoveSpeed.x, -(GetPosition().x - (WORLD_MAX_X - 100.0f)) * 0.01f); // east
//        m_vecMoveSpeed.x = Max(m_vecMoveSpeed.x, -(GetPosition().x - (WORLD_MIN_X + 100.0f)) * 0.01f); // west
//        m_vecMoveSpeed.y = Min(m_vecMoveSpeed.y, -(GetPosition().y - (WORLD_MAX_Y - 100.0f)) * 0.01f); // north
//        m_vecMoveSpeed.y = Max(m_vecMoveSpeed.y, -(GetPosition().y - (WORLD_MIN_Y + 100.0f)) * 0.01f); // south
//
//        if (!onLand && bBoatInWater && !bSeparateTurnForce)
//            ApplyWaterResistance();
//
//        if ((GetModelIndex() != MI_SKIMMER || m_skimmerThingTimer == 0.0f) && !bSeparateTurnForce)
//        {
//            // No idea what exactly is going on here besides drag in YZ
//            float fx = Pow(m_pHandlingBoat->vecTurnRes.x, CTimer::GetTimeStep());
//            float fy = Pow(m_pHandlingBoat->vecTurnRes.y, CTimer::GetTimeStep());
//            float fz = Pow(m_pHandlingBoat->vecTurnRes.z, CTimer::GetTimeStep());
//            m_vecTurnSpeed = Multiply3x3(m_vecTurnSpeed, GetMatrix()); // invert - to local space
//            // TODO: figure this out
//            float magic = 1.0f / (1000.0f * SQR(m_vecTurnSpeed.x) + 1.0f) * fx;
//            m_vecTurnSpeed.y *= fy;
//            m_vecTurnSpeed.z *= fz;
//            float forceUp = (magic - 1.0f) * m_vecTurnSpeed.x * m_fTurnMass;
//            m_vecTurnSpeed = Multiply3x3(GetMatrix(), m_vecTurnSpeed); // back to world
//            CVector com = Multiply3x3(GetMatrix(), m_vecCentreOfMass);
//            ApplyTurnForce(forceUp * GetUp(), com + GetForward());
//        }
//
//        int16 nDeltaVolumeUnderWater = (m_fVolumeUnderWater - m_fPrevVolumeUnderWater) * 10000;
//
//        // Falling into water
//        if (!onLand && bBoatInWater && GetUp().z > 0.0f)
//        {
//            float splashVol = nDeltaVolumeUnderWater * m_pHandlingBoat->fWaveAudioMult;
//            if (splashVol > 200.0f)
//                DMAudio.PlayOneShot(m_audioEntityId, SOUND_CAR_SPLASH, splashVol);
//
//            if (nDeltaVolumeUnderWater > 200)
//            {
//                float speedUp = m_vecMoveSpeed.MagnitudeSqr() * nDeltaVolumeUnderWater * 0.001f;
//                if (speedUp + m_vecMoveSpeed.z > m_pHandling->fBrakeDeceleration)
//                    speedUp = m_pHandling->fBrakeDeceleration - m_vecMoveSpeed.z;
//                if (speedUp < 0.0f)
//                    speedUp = 0.0f;
//                float speedFwd = DotProduct(m_vecMoveSpeed, GetForward());
//                speedFwd *= -nDeltaVolumeUnderWater * 0.01f * m_pHandling->fBrakeBias;
//                CVector speed = speedFwd * GetForward() + CVector(0.0f, 0.0f, speedUp);
//                CVector splashImpulse = speed * m_fMass;
//                ApplyMoveForce(splashImpulse);
//                ApplyTurnForce(splashImpulse, buoyancePoint);
//            }
//        }
//
//        // Splashes
//        float speed = m_vecMoveSpeed.Magnitude();
//        if (speed > 0.05f && GetUp().x > 0.0f && !TheCamera.GetLookingForwardFirstPerson() && IsVisible() &&
//            (AutoPilot.m_nCarMission != MISSION_CRUISE || (CTimer::GetFrameCounter() & 2) == 0))
//        {
//            CVector splashPos, splashDir;
//            float splashSize, front, waterLevel;
//
//            switch (GetModelIndex())
//            {
//            case MI_RIO:
//                splashSize = speed;
//                front = 0.9f * GetColModel()->boundingBox.max.y;
//                splashDir = -0.5f * m_vecMoveSpeed;
//                splashDir.z += 0.25f * speed;
//                splashDir += 0.35f * speed * GetRight();
//                splashPos = GetPosition() + 1.85f * GetRight() + front * GetForward();
//                splashPos.z += 0.5f;
//                break;
//            case MI_SQUALO:
//                splashSize = speed;
//                front = 0.75f * GetColModel()->boundingBox.max.y;
//                splashDir = -0.125f * m_vecMoveSpeed;
//                splashDir.z += 0.15f * speed;
//                splashDir += 0.25f * speed * GetRight();
//                splashPos = GetPosition() + buoyancePoint + 0.5f * GetRight() + front * GetForward();
//                splashPos.z += 0.5f;
//                break;
//            case MI_REEFER:
//                splashSize = speed;
//                front = 0.75f * GetColModel()->boundingBox.max.y;
//                splashDir = -0.5f * m_vecMoveSpeed;
//                splashDir.z += 0.15f * speed;
//                splashDir += 0.5f * speed * GetRight();
//                splashPos = GetPosition() + buoyancePoint + 1.3f * GetRight() + front * GetForward();
//                break;
//            case MI_COASTG:
//                splashSize = 0.25f * speed;
//                front = 0.8f * GetColModel()->boundingBox.max.y;
//                splashDir = 0.165f * m_vecMoveSpeed;
//                splashDir.z += 0.25f * speed;
//                splashDir += 0.15f * speed * GetRight();
//                splashPos = GetPosition() + 0.65f * GetRight() + front * GetForward();
//                splashPos.z += 0.5f;
//                break;
//            case MI_DINGHY:
//                splashSize = 0.25f * speed;
//                front = 0.9f * GetColModel()->boundingBox.max.y;
//                splashDir = 0.35f * m_vecMoveSpeed;
//                splashDir.z += 0.25f * speed;
//                splashDir += 0.25f * speed * GetRight();
//                splashPos = GetPosition() + 0.6f * GetRight() + front * GetForward();
//                splashPos.z += 0.5f;
//                break;
//            default:
//                splashSize = speed;
//                front = 0.9f * GetColModel()->boundingBox.max.y;
//                splashDir = -0.5f * m_vecMoveSpeed;
//                splashDir.z += 0.25f * speed;
//                splashDir += 0.35f * speed * GetRight();
//                splashPos = GetPosition() + buoyancePoint + 0.5f * GetRight() + front * GetForward();
//                break;
//            }
//            if (splashSize > 0.75f)
//                splashSize = 0.75f;
//            if (AutoPilot.m_nCarMission == MISSION_CRUISE)
//                splashDir *= 1.5f;
//            static float lifeMult = 1000.0f;
//            static float lifeBase = 300.0f;
//            splashDir.z += 0.0003f * nDeltaVolumeUnderWater;
//            CWaterLevel::GetWaterLevel(splashPos, &waterLevel, true);
//            if (splashPos.z - waterLevel < 3.0f &&
//                CVisibilityPlugins::GetDistanceSquaredFromCamera(&splashPos) < SQR(70.0f * TheCamera.GenerationDistMultiplier))
//            {
//                splashPos.z = waterLevel + 0.1f;
//                CParticle::AddParticle(PARTICLE_CAR_SPLASH, splashPos, 0.75f * splashDir, nil, splashSize + 0.1f, splashColor,
//                    CGeneral::GetRandomNumberInRange(0.0f, 10.0f), CGeneral::GetRandomNumberInRange(0.0f, 90.0f),
//                    1, lifeBase + splashDir.z * lifeMult);
//                CParticle::AddParticle(PARTICLE_BOAT_SPLASH, splashPos, splashDir, nil, splashSize, jetColor,
//                    CGeneral::GetRandomNumberInRange(0.0f, 0.4f), CGeneral::GetRandomNumberInRange(0.0f, 45.0f),
//                    0, lifeBase + splashDir.z * lifeMult);
//            }
//
//            switch (GetModelIndex())
//            {
//            case MI_RIO:
//                splashDir = -0.5f * m_vecMoveSpeed;
//                splashDir.z += 0.25f * speed;
//                splashDir -= 0.35f * speed * GetRight();
//                splashPos = GetPosition() - 1.85f * GetRight() + front * GetForward();
//                splashPos.z += 0.5f;
//                break;
//            case MI_SQUALO:
//                splashDir = -0.125f * m_vecMoveSpeed;
//                splashDir.z += 0.15f * speed;
//                splashDir -= 0.25f * speed * GetRight();
//                splashPos = GetPosition() + buoyancePoint - 0.5f * GetRight() + front * GetForward();
//                splashPos.z += 0.5f;
//                break;
//            case MI_REEFER:
//                splashDir = -0.5f * m_vecMoveSpeed;
//                splashDir.z += 0.15f * speed;
//                splashDir -= 0.5f * speed * GetRight();
//                splashPos = GetPosition() + buoyancePoint - 1.3f * GetRight() + front * GetForward();
//                break;
//            case MI_COASTG:
//                splashDir = 0.165f * m_vecMoveSpeed;
//                splashDir.z += 0.25f * speed;
//                splashDir -= 0.15f * speed * GetRight();
//                splashPos = GetPosition() - 0.65f * GetRight() + front * GetForward();
//                splashPos.z += 0.5f;
//                break;
//            case MI_DINGHY:
//                splashDir = 0.35f * m_vecMoveSpeed;
//                splashDir.z += 0.25f * speed;
//                splashDir -= 0.25f * speed * GetRight();
//                splashPos = GetPosition() - 0.6f * GetRight() + front * GetForward();
//                splashPos.z += 0.5f;
//                break;
//            default:
//                splashDir = -0.5f * m_vecMoveSpeed;
//                splashDir.z += 0.25f * speed;
//                splashDir -= 0.35f * speed * GetRight();
//                splashPos = GetPosition() + buoyancePoint - 0.5f * GetRight() + front * GetForward();
//                break;
//            }
//            if (AutoPilot.m_nCarMission == MISSION_CRUISE)
//                splashDir *= 1.5f;
//            splashDir.z += 0.0003f * nDeltaVolumeUnderWater;
//            CWaterLevel::GetWaterLevel(splashPos, &waterLevel, true);
//            if (splashPos.z - waterLevel < 3.0f &&
//                CVisibilityPlugins::GetDistanceSquaredFromCamera(&splashPos) < SQR(70.0f * TheCamera.GenerationDistMultiplier))
//            {
//                splashPos.z = waterLevel + 0.1f;
//                CParticle::AddParticle(PARTICLE_CAR_SPLASH, splashPos, 0.75f * splashDir, nil, splashSize + 0.1f, splashColor,
//                    CGeneral::GetRandomNumberInRange(0.0f, 10.0f), CGeneral::GetRandomNumberInRange(0.0f, 90.0f),
//                    1, lifeBase + splashDir.z * lifeMult);
//                CParticle::AddParticle(PARTICLE_BOAT_SPLASH, splashPos, splashDir, nil, splashSize, jetColor,
//                    CGeneral::GetRandomNumberInRange(0.0f, 0.4f), CGeneral::GetRandomNumberInRange(0.0f, 45.0f),
//                    0, lifeBase + splashDir.z * lifeMult);
//            }
//        }
//
//        // Spray waterdrops on screen
//        /*
//        if(TheCamera.GetLookingForwardFirstPerson() && FindPlayerVehicle() && FindPlayerVehicle()->IsBoat() &&
//           m_nDeltaVolumeUnderWater > 0 && numWaterDropOnScreen < 20){
//            CVector dropPos;
//            CVector dropDir(CGeneral::GetRandomNumberInRange(-0.25f, 0.25f), CGeneral::GetRandomNumberInRange(1.0f, 0.75f), 0.0f);
//
//            int frm = CGeneral::GetRandomNumber() & 1;
//            if(TheCamera.m_CameraAverageSpeed < 0.35f){
//                dropPos.x = CGeneral::GetRandomNumberInRange(50, (int)SCREEN_WIDTH-50);
//                dropPos.y = CGeneral::GetRandomNumberInRange(50, (int)SCREEN_HEIGHT-50);
//            }else{
//                dropPos.x = CGeneral::GetRandomNumberInRange(200, (int)SCREEN_WIDTH-200);
//                dropPos.y = CGeneral::GetRandomNumberInRange(150, (int)SCREEN_HEIGHT-150);
//            }
//            dropPos.z = 1.0f;
//
//            if(TheCamera.m_CameraAverageSpeed > 0.35f){
//                if((int)SCREEN_WIDTH / 2 < dropPos.x)
//                    dropPos.x += CGeneral::GetRandomNumberInRange(0.35f, TheCamera.m_CameraAverageSpeed)*7.5f;
//                else
//                    dropPos.x -= CGeneral::GetRandomNumberInRange(0.35f, TheCamera.m_CameraAverageSpeed)*7.5f;
//
//                if((int)SCREEN_HEIGHT / 2 < dropPos.y)
//                    dropPos.y += CGeneral::GetRandomNumberInRange(0.35f, TheCamera.m_CameraAverageSpeed)*7.5f;
//                else
//                    dropPos.y -= CGeneral::GetRandomNumberInRange(0.35f, TheCamera.m_CameraAverageSpeed)*7.5f;
//            }
//
//            if(CParticle::AddParticle(PARTICLE_WATERDROP, dropPos, dropDir, nil,
//                    CGeneral::GetRandomNumberInRange(0.1f, 0.15f), dropColor, 0, 0, frm))
//                numWaterDropOnScreen++;
//        }*/
//
//        if (m_fPrevVolumeUnderWater == 0.0f && m_fVolumeUnderWater > 0.0f && GetModelIndex() == MI_SKIMMER)
//        {
//            CVector splashDir(0.0f, 0.0f, 0.25f * speed);
//            CVector splashPos = GetPosition();
//            float level;
//            CWaterLevel::GetWaterLevel(splashPos, &level, true);
//            splashPos.z = level;
//            CParticleObject::AddObject(POBJECT_CAR_WATER_SPLASH, splashPos, splashDir, 0.0f, 65, splashColor, true);
//        }
//
//        m_fPrevVolumeUnderWater = m_fVolumeUnderWater;
//    }
//    else
//    {
//        bBoatInWater = false;
//        bIsInWater = false;
//#ifdef FIX_BUGS
//        bIsDrowning = false;
//#endif
//    }
//
//    if (m_modelIndex == MI_SKIMMER && CTimer::GetTimeStep() > 0.0f)
//    {
//        if (GetStatus() == STATUS_PLAYER)
//        {
//            if (m_fMovingSpeed < 0.22f)
//                m_fMovingSpeed += 0.001f * CTimer::GetTimeStep();
//            FlyingControl(FLIGHT_MODEL_SEAPLANE);
//        }
//        else
//        {
//            if (m_fMovingSpeed > 0.0005f * CTimer::GetTimeStep())
//                m_fMovingSpeed -= 0.0005f * CTimer::GetTimeStep();
//            else
//                m_fMovingSpeed = 0.0f;
//        }
//    }
//    else if (bCheat8)
//        FlyingControl(FLIGHT_MODEL_PLANE);
//
//    if (bIsAnchored)
//    {
//        m_vecMoveSpeed.x = 0.0f;
//        m_vecMoveSpeed.y = 0.0f;
//
//        if (m_fOrientation == INVALID_ORIENTATION)
//        {
//            m_fOrientation = GetForward().Heading();
//        }
//        else
//        {
//            // is this some inlined CPlaceable method?
//            CVector pos = GetPosition();
//            GetMatrix().RotateZ(m_fOrientation - GetForward().Heading());
//            GetMatrix().SetTranslateOnly(pos);
//        }
//    }
//
//    ProcessDelayedExplosion();

}

void cBoatMG::Render(void)
{
	m_pLeftWake->Render();
	m_pRightWake->Render();
	cVehicleMG::Render();
}


sBoat::sBoat() : sVehicle()
{
	DECLARE_ELEMENT_CONSTRUCT(this, true, false);
	SetPhysical(new cBoatMG(this));
	DECLARE_ELEMENT_CONSTRUCT(this, false, false);
}

sBoat::sBoat(CBoat* pBoat) : sVehicle()
{
	DECLARE_ELEMENT_CONSTRUCT(this, true, true);
	cMultiGame::Instance().AttachEntity(this, pBoat);
	SetEntity(pBoat);
	SetPhysical(new cBoatMG(this));
	Initialise();
	AttachSync(m_nTime, new sBoatSync(pBoat));
	TransferZone();

//	// sVehicle
//	m_fTraction = pBoat->m_fTraction;
//	//memcpy(m_aSuspensionLineLength, pBike->m_aSuspensionLineLength, sizeof(pBike->m_aSuspensionLineLength)); // gcc unroll?
//	//memcpy(m_aSuspensionSpringLength, pBike->m_aSuspensionSpringLength, sizeof(pBike->m_aSuspensionSpringLength));
//	for (uint32 i = 0; i < ARRAY_SIZE(pBoat->m_aSuspensionLineLength); i++) // probably compiler loop optimization upper
//		m_aSuspensionLineLength[i] = pBoat->m_aSuspensionLineLength[i];
//	for (uint32 i = 0; i < ARRAY_SIZE(pBoat->m_aSuspensionSpringLength); i++)
//		m_aSuspensionSpringLength[i] = pBoat->m_aSuspensionSpringLength[i];
//#ifdef FIX_BUGS // already in Initialise, but still here to copy sVehicle all info
//	m_aColours[VEHICLE_COLOUR_PRIMARY] = pBoat->m_aColours[VEHICLE_COLOUR_PRIMARY];
//	m_aColours[VEHICLE_COLOUR_SECONDARY] = pBoat->m_aColours[VEHICLE_COLOUR_SECONDARY];
//#endif
	DECLARE_ELEMENT_CONSTRUCT(this, false, true);
}


ElementCapability sBoat::GetCapability()
{
	return sBoat::Capability();
}

bool sBoat::HasCapability(ElementCapability capability)
{
	if (sBoat::Capability() == capability)
		return true;
	if (sVehicle::Capability() == capability)
		return true;
	if (sElementPhysical::Capability() == capability)
		return true;
	if (sElement::Capability() == capability)
		return true;
	return false;
}

sBoat::~sBoat()
{
	DECLARE_ELEMENT_DESTRUCT(this);
	sElement::PurgeAttached();
	// ~sElement
}

sElementSync* sBoat::CreateSync() {
	return new sBoatSync();
}

void sBoat::DisposeSync(sElementSync* pSync) {
	if (pSync)
		delete ((sBoatSync*)pSync);
}

sElementSync* sBoat::CreateSyncFromOther(sElementSync* pSync) {
	sBoatSync& sync = *(sBoatSync*)pSync;
	return new sBoatSync(sync);
}

bool sBoat::HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) {
	sBoatSync& syncA = *(sBoatSync*)pSyncA;
	sBoatSync& syncB = *(sBoatSync*)pSyncB;
	return syncA.Compare(syncB);
}

void sBoat::ApplyClientSync(uint16 nTime) {
	sVehicle::ApplyClientSync(nTime);
}

void sBoat::Update(uint16 nTime) {
	if (GetEntity() != nil)
		AttachSync(nTime, new sBoatSync((CBoat*)GetEntity()));
	else
		delete this; // ?
}

bool sBoat::WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) {
	if (static_cast<int16>(nSyncLastTime - m_vSync.front().m_nTime) >= 0) // FindSync? not GetSyncWithTime?
		return WriteSyncDelta(pSyncStream, FindSync(nSyncWriteTime, nil).boat, GetSyncWithTime(nSyncLastTime).boat, (nSyncWriteTime - nSyncLastTime));

	tBoatSyncsDeltas boatDeltaManager{};
	boatDeltaManager.SetDifference(); // FindSync? not GetSyncWithTime?
	PerformWriteSync(pSyncStream, FindSync(nSyncWriteTime, nil).boat, &boatDeltaManager); // max diff
	return true;
}

void sBoat::ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) {
	sBoatSync& sync = *(sBoatSync*)pOutSync;
	uint8 nDiffMask = (uint8)pSyncStream->ReadU32(); // 32 -> 8

	if (nDiffMask & eBoatSync::MP_PKTD_BOAT_VEHICLE)
		sVehicle::ReadSyncFromStreamVehicle(pSyncStream, (sBoatSync*)pOutSync);
}

void sBoat::ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) {
	cMultiGame& Game = cMultiGame::Instance();
	if (nOwner != Game.LocalPlayerID()) {
		sVehicle::ReceiveEntity(nOwner, nID, nTime);
		return;
	}

	sBoatSync* pSync = GetSync().boat;
	assert(GetPhysical());
	CBoat* boat = new CBoat(GetPhysical()->GetModelIndex(), pSync->VehicleCreatedBy);
	Game.AttachEntity(this, boat);
	SetEntity(boat);

	sVehicle::ReceiveEntity(nOwner, nID, nTime);
	AttachSync(m_nTime, new sBoatSync(boat));
}

void sBoat::SetupModel() {
	;
}

void sBoat::CompareSyncState(sBoatSync* pSync, sBoatSync* pLastSync, uint32 nDelta, tBoatSyncsDeltas* pDiff) {
	sVehicle::CompareSyncState(pSync, pLastSync, nDelta, &pDiff->tVehicleDiff);

	if (pDiff->tVehicleDiff.nVehicleDiff != eVehicleSync::MP_PKTD_VEH_EQUAL)
		pDiff->nBoatDiff |= eBoatSync::MP_PKTD_BOAT_VEHICLE;
}

void sBoat::PerformWriteSync(sWriteSyncStream* pSyncStream, sBoatSync* pSync, tBoatSyncsDeltas* pDiff) {
	pSyncStream->WriteU32((uint32)pDiff->nBoatDiff); // 8 -> 32

	if (pDiff->nBoatDiff & eBoatSync::MP_PKTD_BOAT_VEHICLE)
		sVehicle::PerformWriteSync(pSyncStream, pSync, &pDiff->tVehicleDiff);
}

bool sBoat::WriteSyncDelta(sWriteSyncStream* pSyncStream, sBoatSync* pSync, sBoatSync* pLastSync, uint32 nDelta) {
	tBoatSyncsDeltas boatDeltaManager{};
	boatDeltaManager.SetEqual();
	CompareSyncState(pSync, pLastSync, nDelta, &boatDeltaManager);

#ifdef FIX_BUGS
	if (boatDeltaManager.nBoatDiff == eBoatSync::MP_PKTD_BOAT_EQUAL) // main delta
		return false;
#endif

	PerformWriteSync(pSyncStream, pSync, &boatDeltaManager);
	return true;
}

#ifdef FIX_BUGS
void sBoat::Fix() {
	;
}
#endif
#endif