/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "multiplayer/MultiGame.h"
#include "multiplayer/elements/sSyncStream.h"
#include "multiplayer/elements/sAutomobile.h"
#include "Object.h"
#include "World.h"
#include "VisibilityPlugins.h"
#include "Streaming.h"
#include "Particle.h"

sDamageManager::sDamageManager()
{
	ResetDamageStatus();
}

sDamageManager::sDamageManager(CDamageManager* damage)
{
	m_fWheelDamageEffect = damage->m_fWheelDamageEffect;
	m_lightStatus = damage->m_lightStatus;
	m_panelStatus = damage->m_panelStatus;
	for (uint32 i = 0; i < ARRAY_SIZE(m_wheelStatus); i++) {
		m_wheelStatus[i] = damage->m_wheelStatus[i];
	}
	for (uint32 i = 0; i < ARRAY_SIZE(m_doorStatus); i++) {
		m_doorStatus[i] = damage->m_doorStatus[i];
	}
#ifdef FIX_BUGS // idk pad [2] or m_bSmashedDoorDoesntClose m_engineStatus, but we copy it
	m_bSmashedDoorDoesntClose = damage->m_bSmashedDoorDoesntClose;
	m_engineStatus = damage->m_engineStatus;
#endif
}

sDamageManager::sDamageManager(const sDamageManager& other)
{
	m_fWheelDamageEffect = other.m_fWheelDamageEffect;
	for (uint32 i = 0; i < ARRAY_SIZE(m_wheelStatus); i++) {
		m_wheelStatus[i] = other.m_wheelStatus[i];
	}
	for (uint32 i = 0; i < ARRAY_SIZE(m_doorStatus); i++) {
		m_doorStatus[i] = other.m_doorStatus[i];
	}
#ifdef FIX_BUGS // idk pad [2] or m_bSmashedDoorDoesntClose m_engineStatus, but we copy it
	m_bSmashedDoorDoesntClose = other.m_bSmashedDoorDoesntClose;
	m_engineStatus = other.m_engineStatus;
#endif
	m_lightStatus = other.m_lightStatus;
	m_panelStatus = other.m_panelStatus;
}

bool sDamageManager::Compare(const sDamageManager& other)
{
	if (m_fWheelDamageEffect != other.m_fWheelDamageEffect)
		return false;

	for (uint32 i = 0; i < ARRAY_SIZE(m_wheelStatus); i++) {
		if (m_wheelStatus[i] != other.m_wheelStatus[i])
			return false;
	}

	for (uint32 i = 0; i < ARRAY_SIZE(m_doorStatus); i++) {
		if (m_doorStatus[i] != other.m_doorStatus[i])
			return false;
	}

	if (m_bSmashedDoorDoesntClose != other.m_bSmashedDoorDoesntClose)
		return false;

	if (m_engineStatus != other.m_engineStatus)
		return false;

	if (m_lightStatus != other.m_lightStatus)
		return false;

	if (m_panelStatus != other.m_panelStatus)
		return false;

	return true;
}

#if !defined(FINAL) && !defined(MASTER)
void sDamageManager::Dump()
{
	printf("Damage Manager:\n");
	printf("  WheelDamageEffect: %.2f (0x%08X)\n", m_fWheelDamageEffect, *(uint32*)&m_fWheelDamageEffect);
	printf("  Wheel Status:\n");
	for (uint32 i = 0; i < ARRAY_SIZE(m_wheelStatus); i++) {
		printf("    Wheel[%d]: %u (0x%02X)\n", i, m_wheelStatus[i], m_wheelStatus[i]);
	}
	printf("  Door Status:\n");
	for (uint32 i = 0; i < ARRAY_SIZE(m_doorStatus); i++) {
		printf("    Door[%d]: %u (0x%02X)\n", i, m_doorStatus[i], m_doorStatus[i]);
	}
	printf("  SmashedDoorDoesntClose: %s\n", m_bSmashedDoorDoesntClose ? "true" : "false");
	printf("  EngineStatus: %u (0x%02X)\n", m_engineStatus, m_engineStatus);
	printf("  LightStatus:  %u (0x%08X)\n", m_lightStatus, m_lightStatus);
	printf("  PanelStatus:  %u (0x%08X)\n", m_panelStatus, m_panelStatus);
}
#endif


#ifdef GTA_LIBERTY
sAutomobileSync::sAutomobileSync() : sVehicleSync()
#else
sAutomobileSync::sAutomobileSync() : sAutomobileBaseSync()
#endif
{
	DECLARE_SYNC_CONSTRUCT(this);
	Damage = sDamageManager();
	Damage.m_fWheelDamageEffect = 0.75f;
}

#ifdef GTA_LIBERTY
sAutomobileSync::sAutomobileSync(CAutomobile* pAutomobile) : sVehicleSync(pAutomobile)
#else
sAutomobileSync::sAutomobileSync(CAutomobile* pAutomobile) : sAutomobileBaseSync(pAutomobile)
#endif
{
	DECLARE_SYNC_CONSTRUCT(this);
	Damage = sDamageManager(&pAutomobile->Damage);
	m_fCarGunLR = pAutomobile->m_fCarGunLR;
}

#ifdef GTA_LIBERTY
sAutomobileSync::sAutomobileSync(const sAutomobileSync& other) : sVehicleSync(other)
#else
sAutomobileSync::sAutomobileSync(const sAutomobileSync& other) : sAutomobileBaseSync(other)
#endif
{
	DECLARE_SYNC_CONSTRUCT(this);
#ifdef FIX_BUGS
	Damage = sDamageManager(other.Damage);
	m_fCarGunLR = other.m_fCarGunLR;
	field_30C = other.field_30C; // unused?
#endif
}

sAutomobileSync::~sAutomobileSync() {
	DECLARE_SYNC_DESTRUCT(this);
}

bool sAutomobileSync::Compare(const sAutomobileSync& other)
{
	if (!sAutomobileBaseSync::Compare(other))
		return false;

#ifdef FIX_BUGS
	if (!Damage.Compare(other.Damage))
		return false;

	if (m_fCarGunLR != other.m_fCarGunLR)
		return false;

	if (field_30C != other.field_30C)
		return false;
#endif

	return true;
}

#if !defined(FINAL) && !defined(MASTER)
void sAutomobileSync::Dump()
{
	sAutomobileBaseSync::Dump();

	printf("=== sAutomobileSync Dump ===\n");
	printf("sAutomobileSync:\n");
	Damage.Dump();
	printf("  CarGunLR: %.2f (0x%08X)\n", m_fCarGunLR, *(uint32*)&m_fCarGunLR);
	printf("  field_30C: %d (0x%08X)\n", field_30C, field_30C);
	printf("================================\n");
}
#endif


#ifdef GTA_LIBERTY
cAutomobileMG::cAutomobileMG(sElement* elem) : cVehicleMG(elem)
#else
cAutomobileMG::cAutomobileMG(sElement* elem) : cAutomobileBaseMG(elem)
#endif
{
	m_vehType = VEHICLE_TYPE_CAR;
}

cAutomobileMG::~cAutomobileMG()
{

}

void cAutomobileMG::PreRender(void)
{
	cMultiGame& Game = cMultiGame::Instance();
	sAutomobile* pAutomobile = GetElement().automobile;
	cAutomobileBaseMG::PreRender();

	uint16 nTime = GetElement().automobile->m_nTime - static_cast<uint16>(Game.m_nLagValue);
	sAutomobileSync* automobile = pAutomobile->FindSync(nTime, nil).automobile;
	CVehicleModelInfo* mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex());

	CMatrix mat;
	CVector pos;

#ifdef FIX_BUGS
	if (GetModelIndex() == MI_RHINO && pAutomobile->m_aCarNodes[CAR_WINDSCREEN])
#else
	if (GetModelIndex() == MI_RHINO)
#endif
	{
		mat.Attach(RwFrameGetMatrix(pAutomobile->m_aCarNodes[CAR_WINDSCREEN]));
		pos = mat.GetPosition();
		mat.SetRotateZ(automobile->m_fCarGunLR);
#ifndef FIX_BUGS // i think it's pastebug from cAutomobileBaseMG::PreRenderWheels or etc, idk
		mat.Scale(mi->GetWheelScale(true));
#endif
		mat.Translate(pos);
		mat.UpdateRW();
	}

	PreRenderWheels(automobile, mi);
	pAutomobile->SetPanelDamage(automobile, CAR_WING_LF, VEHPANEL_FRONT_LEFT);
	pAutomobile->SetPanelDamage(automobile, CAR_WING_RF, VEHPANEL_FRONT_RIGHT);
	pAutomobile->SetPanelDamage(automobile, CAR_WING_LR, VEHPANEL_REAR_LEFT);
	pAutomobile->SetPanelDamage(automobile, CAR_WING_RR, VEHPANEL_REAR_RIGHT);
	pAutomobile->SetPanelDamage(automobile, CAR_WINDSCREEN, VEHPANEL_WINDSCREEN);
	pAutomobile->SetBumperDamage(automobile, CAR_BUMP_FRONT, VEHBUMPER_FRONT);
	pAutomobile->SetBumperDamage(automobile, CAR_BUMP_REAR, VEHBUMPER_REAR);
	pAutomobile->SetDoorDamage(automobile, CAR_BONNET, DOOR_BONNET);
	pAutomobile->SetDoorDamage(automobile, CAR_BOOT, DOOR_BOOT);
	pAutomobile->SetDoorDamage(automobile, CAR_DOOR_LF, DOOR_FRONT_LEFT);
	pAutomobile->SetDoorDamage(automobile, CAR_DOOR_RF, DOOR_FRONT_RIGHT);
	pAutomobile->SetDoorDamage(automobile, CAR_DOOR_LR, DOOR_REAR_LEFT);
	pAutomobile->SetDoorDamage(automobile, CAR_DOOR_RR, DOOR_REAR_RIGHT);


	 // Wheel particles

    if (GetModelIndex() == MI_DODO)
    {
        ; // nothing
    }
    //else if (GetModelIndex() == MI_RCBANDIT)
    //{
    //    for (uint32 i = 0; i < 4; i++)
    //    {
    //        // Game has same code three times here
    //        switch (m_aWheelState[i])
    //        {
    //            case WHEEL_STATE_SPINNING:
    //            case WHEEL_STATE_SKIDDING:
    //            case WHEEL_STATE_FIXED:
    //                CParticle::AddParticle(PARTICLE_RUBBER_SMOKE,
    //                    m_aWheelColPoints[i].point + CVector(0.0f, 0.0f, 0.05f),
    //                    CVector(0.0f, 0.0f, 0.0f), nil, 0.1f);
    //                break;
    //            default:
    //                break;
    //        }
    //    }
    //}
    else
    {
		float fwdSpeed = Abs(DotProduct(m_vecMoveSpeed, GetForward()));
        int32 drawParticles = Abs(fwdSpeed) < 90.0f;
        if (pAutomobile->GetSync().automobile->m_nStatus == STATUS_PHYSICS || pAutomobile->GetSync().automobile->m_nStatus == STATUS_PLAYER ||
			pAutomobile->GetSync().automobile->m_nStatus == STATUS_PLAYER_PLAYBACKFROMBUFFER || pAutomobile->GetSync().automobile->m_nStatus == STATUS_SIMPLE)
        {
            bool rearSkidding = false;
            if (pAutomobile->GetSync().automobile->m_aWheelState[CARWHEEL_REAR_LEFT] == WHEEL_STATE_SKIDDING ||
				pAutomobile->GetSync().automobile->m_aWheelState[CARWHEEL_REAR_RIGHT] == WHEEL_STATE_SKIDDING)
                rearSkidding = true;

            for (uint32 i = 0; i < 4; i++)
            {
                if (pAutomobile->GetSync().automobile->m_aSuspensionSpringRatio[i] < 1.0f &&
					pAutomobile->GetSync().automobile->m_aWheelColPoints[i].surfaceB != SURFACE_WATER)
				{
					bool bIsBloody = (pAutomobile->GetSync().automobile->m_nSkidmarkFlags & BIT(i)); // m_aWheelSkidmarkBloody[i]
					bool bIsUnk = (pAutomobile->GetSync().automobile->m_nSkidmarkFlags & BIT(i + 4));    // m_aWheelSkidmarkUnk[i]

                    switch (pAutomobile->GetSync().automobile->m_aWheelState[i])
                    {
                        case WHEEL_STATE_SPINNING:
                            if (AddWheelDirtAndWater(&pAutomobile->GetSync().automobile->m_aWheelColPoints[i], drawParticles))
                            {
                                CParticle::AddParticle(PARTICLE_BURNINGRUBBER_SMOKE,
									pAutomobile->GetSync().automobile->m_aWheelColPoints[i].point + CVector(0.0f, 0.0f, 0.25f),
                                    CVector(0.0f, 0.0f, 0.0f));

                                CParticle::AddParticle(PARTICLE_BURNINGRUBBER_SMOKE,
									pAutomobile->GetSync().automobile->m_aWheelColPoints[i].point + CVector(0.0f, 0.0f, 0.25f),
                                    CVector(0.0f, 0.0f, 0.05f));
                            }

                            CParticle::AddParticle(PARTICLE_RUBBER_SMOKE,
								pAutomobile->GetSync().automobile->m_aWheelColPoints[i].point + CVector(0.0f, 0.0f, 0.25f),
                                CVector(0.0f, 0.0f, 0.0f));

                            if (pAutomobile->GetSync().automobile->m_aWheelTimer[i] > 0.0f)
                                CSkidmarks::RegisterOne((uintptr)this + i, pAutomobile->GetSync().automobile->m_aWheelColPoints[i].point,
                                    GetForward().x, GetForward().y,
									pAutomobile->GetSync().automobile->m_aWheelSkidmarkType[i], &bIsBloody);
                            break;

                        case WHEEL_STATE_SKIDDING:
                            if (i == CARWHEEL_REAR_LEFT || i == CARWHEEL_REAR_RIGHT || rearSkidding)
                            {
                                // same as below

                                if (Abs(fwdSpeed) > 5.0f)
                                {
                                    AddWheelDirtAndWater(&pAutomobile->GetSync().automobile->m_aWheelColPoints[i], drawParticles);

                                    CParticle::AddParticle(PARTICLE_RUBBER_SMOKE,
										pAutomobile->GetSync().automobile->m_aWheelColPoints[i].point + CVector(0.0f, 0.0f, 0.25f),
                                        CVector(0.0f, 0.0f, 0.0f));
                                }

                                if (pAutomobile->GetSync().automobile->m_aWheelTimer[i] > 0.0f)
                                    CSkidmarks::RegisterOne((uintptr)this + i, pAutomobile->GetSync().automobile->m_aWheelColPoints[i].point,
                                        GetForward().x, GetForward().y,
										pAutomobile->GetSync().automobile->m_aWheelSkidmarkType[i], &bIsBloody);
                            }
                            break;

                        case WHEEL_STATE_FIXED:
                            if (Abs(fwdSpeed) > 5.0f)
                            {
                                AddWheelDirtAndWater(&pAutomobile->GetSync().automobile->m_aWheelColPoints[i], drawParticles);

                                CParticle::AddParticle(PARTICLE_RUBBER_SMOKE,
									pAutomobile->GetSync().automobile->m_aWheelColPoints[i].point + CVector(0.0f, 0.0f, 0.25f),
                                    CVector(0.0f, 0.0f, 0.0f));
                            }

                            if (pAutomobile->GetSync().automobile->m_aWheelTimer[i] > 0.0f)
                                CSkidmarks::RegisterOne((uintptr)this + i, pAutomobile->GetSync().automobile->m_aWheelColPoints[i].point,
                                    GetForward().x, GetForward().y,
									pAutomobile->GetSync().automobile->m_aWheelSkidmarkType[i], &bIsBloody);
                            break;

                        default:
                            if (Abs(fwdSpeed) > 5.0f)
                                AddWheelDirtAndWater(&pAutomobile->GetSync().automobile->m_aWheelColPoints[i], drawParticles);
                            if ((bIsBloody || bIsUnk) && pAutomobile->GetSync().automobile->m_aWheelTimer[i] > 0.0f)
                                CSkidmarks::RegisterOne((uintptr)this + i, pAutomobile->GetSync().automobile->m_aWheelColPoints[i].point,
                                    GetForward().x, GetForward().y,
									pAutomobile->GetSync().automobile->m_aWheelSkidmarkType[i], &bIsBloody);
                    }
					}

                // Sparks for friction of burst wheels
                if (pAutomobile->GetSync().automobile->Damage.GetWheelStatus((eCarWheel)i) == WHEEL_STATUS_BURST && pAutomobile->GetSync().automobile->m_aSuspensionSpringRatio[i] < 1.0f)
                {
                    static float speedSq;
                    speedSq = m_vecMoveSpeed.MagnitudeSqr();
                    if (speedSq > SQR(0.1f) &&
                        pAutomobile->GetSync().automobile->m_aWheelColPoints[i].surfaceB != SURFACE_GRASS &&
                        pAutomobile->GetSync().automobile->m_aWheelColPoints[i].surfaceB != SURFACE_MUD_DRY &&
                        pAutomobile->GetSync().automobile->m_aWheelColPoints[i].surfaceB != SURFACE_SAND &&
                        pAutomobile->GetSync().automobile->m_aWheelColPoints[i].surfaceB != SURFACE_SAND_BEACH &&
                        pAutomobile->GetSync().automobile->m_aWheelColPoints[i].surfaceB != SURFACE_WATER)
                    {
                        CVector normalSpeed = pAutomobile->GetSync().automobile->m_aWheelColPoints[i].normal *
							DotProduct(pAutomobile->GetSync().automobile->m_aWheelColPoints[i].normal, m_vecMoveSpeed);
                        CVector frictionSpeed = m_vecMoveSpeed - normalSpeed;
                        if (i == CARWHEEL_FRONT_LEFT || i == CARWHEEL_REAR_LEFT)
                            frictionSpeed -= 0.05f * GetRight();
                        else
                            frictionSpeed += 0.05f * GetRight();
                        CVector unusedRight = 0.15f * GetRight();
                        CVector sparkDir = 0.25f * frictionSpeed;
                        CParticle::AddParticle(PARTICLE_SPARK_SMALL, pAutomobile->GetSync().automobile->m_aWheelColPoints[i].point, sparkDir);

                        if (speedSq > 0.04f)
                            CParticle::AddParticle(PARTICLE_SPARK_SMALL, pAutomobile->GetSync().automobile->m_aWheelColPoints[i].point, sparkDir);
                        if (speedSq > 0.16f)
                        {
                            CParticle::AddParticle(PARTICLE_SPARK_SMALL, pAutomobile->GetSync().automobile->m_aWheelColPoints[i].point, sparkDir);
                            CParticle::AddParticle(PARTICLE_SPARK_SMALL, pAutomobile->GetSync().automobile->m_aWheelColPoints[i].point, sparkDir);
                        }
                    }
                }
            }
        }
    }

	AddDamagedVehicleParticles();
}

void cAutomobileMG::Render(void)
{
#if defined GTA_LIBERTY || !defined FIX_BUGS
	cVehicleMG::Render();
#else
	cAutomobileBaseMG::Render();
#endif
}

static float fSpeedMult[] = {
	0.8f,
	0.75f,
	0.85f,
	0.9f,
	0.85f,
	0.85f
};
static float fDamagePosSpeedShift = 0.4f;
static CVector vecDAMAGE_ENGINE_POS_SMALL(-0.1f, -0.1f, 0.0f);
static CVector vecDAMAGE_ENGINE_POS_BIG(-0.5f, -0.3f, 0.0f);

void cAutomobileMG::AddDamagedVehicleParticles(void)
{
	sAutomobile* pAutomobile = GetElement().automobile;
	sAutomobileSync* automobile = pAutomobile->FindSync(pAutomobile->m_nTime, nil).automobile;
	float fHealth = automobile->m_fHealth;

	int32 i, n;
    if ( (CTimer::GetFrameCounter() + m_randomSeed) & 1)
        return;
    if (fHealth >= 650.0f)
        return;

    CVector direction = fSpeedMult[5] * m_vecMoveSpeed;
    CVector damagePos = ((CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex()))->m_positions[CAR_POS_HEADLIGHTS];

    switch (automobile->Damage.GetDoorStatus(DOOR_BONNET))
    {
        case DOOR_STATUS_OK:
        case DOOR_STATUS_SMASHED:
            // Bonnet is still there, smoke comes out at the edge
            damagePos += vecDAMAGE_ENGINE_POS_SMALL;
            break;
        case DOOR_STATUS_SWINGING:
        case DOOR_STATUS_MISSING:
            // Bonnet is gone, smoke comes out at the engine
            damagePos += vecDAMAGE_ENGINE_POS_BIG;
            break;
    }
    damagePos.z += fDamagePosSpeedShift * (GetColModel()->boundingBox.max.z - damagePos.z) * DotProduct(GetForward(), m_vecMoveSpeed);
    damagePos = GetMatrix() * damagePos;
    damagePos.z += 0.15f;

    if (fHealth < 320.0f && fHealth > 1.0f)
    {
        direction = 0.85f * m_vecMoveSpeed;
        direction += GetRight() * CGeneral::GetRandomNumberInRange(0.0f, 0.04f) * (1.0f - 2.0f * m_vecMoveSpeed.Magnitude());
        direction.z += 0.001f;
        n = (CGeneral::GetRandomNumber() & 7) + 2;
        for (i = 0; i < n; i++)
            CParticle::AddParticle(PARTICLE_SPARK_SMALL, damagePos, direction);
        if (((CTimer::GetFrameCounter() + m_randomSeed) & 7) == 0)
            CParticle::AddParticle(PARTICLE_ENGINE_SMOKE2, damagePos, 0.8f * m_vecMoveSpeed, nil, 0.1f, 0, 0, 0, 1000);
    }
    else if (fHealth < 460.0f)
    {
        direction = 0.85f * m_vecMoveSpeed;
        direction += GetRight() * CGeneral::GetRandomNumberInRange(0.0f, 0.04f) * (1.0f - 2.0f * m_vecMoveSpeed.Magnitude());
        direction.z += 0.001f;
        n = (CGeneral::GetRandomNumber() & 3) + 1;
        for (i = 0; i < n; i++)
            CParticle::AddParticle(PARTICLE_SPARK_SMALL, damagePos, direction);
        if (((CTimer::GetFrameCounter() + m_randomSeed) & 0xF) == 0)
            CParticle::AddParticle(PARTICLE_ENGINE_SMOKE, damagePos, 0.8f * m_vecMoveSpeed, nil, 0.1f, 0, 0, 0, 1000);
    }
    else if (fHealth < 250.0f)
    {
        // nothing
    }
    else if (fHealth < 320.0f)
    {
        CParticle::AddParticle(PARTICLE_ENGINE_SMOKE2, damagePos, fSpeedMult[0] * direction);
    }
    else if (fHealth < 390.0f)
    {
        CParticle::AddParticle(PARTICLE_ENGINE_STEAM, damagePos, fSpeedMult[1] * direction);
        CParticle::AddParticle(PARTICLE_ENGINE_SMOKE, damagePos, fSpeedMult[2] * direction);
    }
    else if (fHealth < 460.0f)
    {
        if (((CTimer::GetFrameCounter() + m_randomSeed) & 3) == 0 ||
            ((CTimer::GetFrameCounter() + m_randomSeed) & 3) == 2)
            CParticle::AddParticle(PARTICLE_ENGINE_STEAM, damagePos, fSpeedMult[3] * direction);
    }
	else
	{
		int32 rnd = CTimer::GetFrameCounter() + m_randomSeed;
		if (rnd < 10 ||
			rnd < 70 && rnd > 25 ||
			rnd < 160 && rnd > 100 ||
			rnd < 200 && rnd > 175 ||
			rnd > 235)
			return;
		direction.z += 0.05f * Max(1.0f - 1.6f * m_vecMoveSpeed.Magnitude(), 0.0f);
		if (TheCamera.GetLookDirection() != LOOKING_FORWARD)
			CParticle::AddParticle(PARTICLE_ENGINE_STEAM, damagePos, direction);
		else if (((CTimer::GetFrameCounter() + m_randomSeed) & 1) == 0)
			CParticle::AddParticle(PARTICLE_ENGINE_STEAM, damagePos, fSpeedMult[4] * m_vecMoveSpeed);
	}
}


#ifdef GTA_LIBERTY
sAutomobile::sAutomobile() : sVehicle()
#else
sAutomobile::sAutomobile() : sAutomobileBase()
#endif
{
	DECLARE_ELEMENT_CONSTRUCT(this, true, false);
	Damage = sDamageManager();
	Damage.m_fWheelDamageEffect = 0.75f;
#ifdef GTA_LIBERTY
	SetPhysical(new cVehicleMG(this));
#else
	SetPhysical(new cAutomobileMG(this));
#endif
	DECLARE_ELEMENT_CONSTRUCT(this, false, false);
}

#ifdef GTA_LIBERTY
sAutomobile::sAutomobile(CAutomobile* pAutomobile) : sVehicle(/*pAutomobile*/)
#else
sAutomobile::sAutomobile(CAutomobile* pAutomobile) : sAutomobileBase(/*pAutomobile*/)
#endif
{
	DECLARE_ELEMENT_CONSTRUCT(this, true, true);
	Damage = sDamageManager();
	Damage.m_fWheelDamageEffect = 0.75f;
	cMultiGame::Instance().AttachEntity(this, pAutomobile);
	SetEntity(pAutomobile);
#ifdef GTA_LIBERTY
	SetPhysical(new cVehicleMG(this));
#else
	SetPhysical(new cAutomobileMG(this));
#endif
	sAutomobileBase::Initialise();

	// sVehicle
	m_fTraction = pAutomobile->m_fTraction;
	//memcpy(m_aSuspensionLineLength, pAutomobile->m_aSuspensionLineLength, sizeof(pAutomobile->m_aSuspensionLineLength)); // gcc unroll?
	//memcpy(m_aSuspensionSpringLength, pAutomobile->m_aSuspensionSpringLength, sizeof(pAutomobile->m_aSuspensionSpringLength));
	for (uint32 i = 0; i < ARRAY_SIZE(pAutomobile->m_aSuspensionLineLength); i++) // probably compiler loop optimization upper
		m_aSuspensionLineLength[i] = pAutomobile->m_aSuspensionLineLength[i];
	for (uint32 i = 0; i < ARRAY_SIZE(pAutomobile->m_aSuspensionSpringLength); i++)
		m_aSuspensionSpringLength[i] = pAutomobile->m_aSuspensionSpringLength[i];
#ifdef FIX_BUGS // already in Initialise, but still here to copy sVehicle all info
	m_aColours[VEHICLE_COLOUR_PRIMARY] = pAutomobile->m_aColours[VEHICLE_COLOUR_PRIMARY];
	m_aColours[VEHICLE_COLOUR_SECONDARY] = pAutomobile->m_aColours[VEHICLE_COLOUR_SECONDARY];
#endif
	SetupDamage();
	AttachSync(m_nTime, new sAutomobileSync(pAutomobile));
	TransferZone();

	// CAutomobile
#ifdef FIX_BUGS
	sAutomobileSync* pSync = GetSync().automobile;
	for (uint32 i = 0; i < ARRAY_SIZE(pAutomobile->m_aSuspensionSpringRatio); i++)
		m_aSuspensionSpringRatio[i] = pAutomobile->m_aSuspensionSpringRatio[i];
#else
	for (uint32 i = 0; i < ARRAY_SIZE(pAutomobile->m_aSuspensionSpringRatio); i++)
		m_aSuspensionSpringRatio[i] = GetSync().automobile->m_aSuspensionSpringRatio[i];
#endif
	DECLARE_ELEMENT_CONSTRUCT(this, false, true);
}


ElementCapability sAutomobile::GetCapability()
{
	return sAutomobile::Capability();
}

bool sAutomobile::HasCapability(ElementCapability capability)
{
	if (sAutomobile::Capability() == capability)
		return true;
#ifndef GTA_LIBERTY
	if (sAutomobileBase::Capability() == capability)
		return true;
#endif
	if (sVehicle::Capability() == capability)
		return true;
	if (sElementPhysical::Capability() == capability)
		return true;
	if (sElement::Capability() == capability)
		return true;
	return false;
}

sAutomobile::~sAutomobile()
{
	DECLARE_ELEMENT_DESTRUCT(this);
	sElement::PurgeAttached();
	// ~sElement
}

sElementSync* sAutomobile::CreateSync() {
	return new sAutomobileSync();
}

void sAutomobile::DisposeSync(sElementSync* pSync) {
	if (pSync)
		delete ((sAutomobileSync*)pSync);
}

sElementSync* sAutomobile::CreateSyncFromOther(sElementSync* pSync) {
	sAutomobileSync& sync = *(sAutomobileSync*)pSync;
#ifdef FIX_BUGS
	return new sAutomobileSync(sync);
#else
	sAutomobileSync* pNewSync = new sAutomobileSync(sync);
	// Base
	pNewSync->field_220 = sync.field_220;
	for (uint32 i = 0; i < NUM_DOORS; i++) {
		pNewSync->Doors[i] = sDoorSync(sync.Doors[i]);
	}

	// Auto
	pNewSync->Damage = sDamageManager(sync.Damage);
	pNewSync->m_fCarGunLR = sync.m_fCarGunLR;
	//pNewSync->field_30C = sync.field_30C; // unused?
	return pNewSync;
#endif
}

bool sAutomobile::HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) {
	sAutomobileSync& syncA = *(sAutomobileSync*)pSyncA;
	sAutomobileSync& syncB = *(sAutomobileSync*)pSyncB;
	return syncA.Compare(syncB);
}

void sAutomobile::ApplyClientSync(uint16 nTime) {
#ifdef GTA_LIBERTY
	sVehicle::ApplyClientSync(nTime);
#else
	sAutomobileBase::ApplyClientSync(nTime);
#endif

#ifdef FIX_BUGS
	sAutomobileSync* pSync = GetSync().automobile;
	for (uint32 i = 0; i < ARRAY_SIZE(pSync->m_aSuspensionSpringRatio); i++)
		m_aSuspensionSpringRatio[i] = pSync->m_aSuspensionSpringRatio[i];
#else
	for (uint32 i = 0; i < ARRAY_SIZE(pSync->m_aSuspensionSpringRatio); i++)
		m_aSuspensionSpringRatio[i] = GetSync().automobile->m_aSuspensionSpringRatio[i];
#endif
}

void sAutomobile::Update(uint16 nTime) {
	if (GetEntity() != nil)
		AttachSync(nTime, new sAutomobileSync((CAutomobile*)GetEntity()));
	else
		delete this; // ?
}

bool sAutomobile::WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) {
	if (static_cast<int16>(nSyncLastTime - m_vSync.front().m_nTime) >= 0) // FindSync? not GetSyncWithTime?
		return WriteSyncDelta(pSyncStream, FindSync(nSyncWriteTime, nil).automobile, GetSyncWithTime(nSyncLastTime).automobile, (nSyncWriteTime - nSyncLastTime));

	tAutomobileSyncsDeltas automobileDeltaManager{};
	automobileDeltaManager.SetDifference(); // FindSync? not GetSyncWithTime?
	PerformWriteSync(pSyncStream, FindSync(nSyncWriteTime, nil).automobile, &automobileDeltaManager); // max diff
	return true;
}

void sAutomobile::ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) {
	sAutomobileSync& sync = *(sAutomobileSync*)pOutSync;
	uint32 nDiffMask = pSyncStream->ReadU32();

	if (nDiffMask & eAutomobileSync::MP_PKTD_AUTO_AUTOBASE)
		sAutomobileBase::ReadSyncFromStreamAutomobileBase(pSyncStream, (sAutomobileSync*)pOutSync);

	if (nDiffMask & eAutomobileSync::MP_PKTD_AUTO_DAMAGE)
		ReadDamageSyncFromStream(pSyncStream, &sync.Damage);

	if (nDiffMask & eAutomobileSync::MP_PKTD_AUTO_CAR_GUN_LR)
		sync.m_fCarGunLR = pSyncStream->ReadFloat();
}

void sAutomobile::ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) {
	cMultiGame& Game = cMultiGame::Instance();
	if (nOwner != Game.LocalPlayerID()) {
		sAutomobileBase::ReceiveEntity(nOwner, nID, nTime);
		return;
	}

	sAutomobileSync* pSync = GetSync().automobile;
	assert(GetPhysical());
	CAutomobile* automobile = new CAutomobile(GetPhysical()->GetModelIndex(), pSync->VehicleCreatedBy);
	Game.AttachEntity(this, automobile);
	SetEntity(automobile);

	// decomp do memmove here, i guess its gcc loop unroll or smth like this

	for (uint32 i = 0; i < ARRAY_SIZE(automobile->Damage.m_doorStatus); i++)
		automobile->Damage.m_doorStatus[i] = pSync->Damage.m_doorStatus[i];
	
	for (uint32 i = 0; i < ARRAY_SIZE(automobile->Damage.m_wheelStatus); i++)
		automobile->Damage.m_wheelStatus[i] = pSync->Damage.m_wheelStatus[i];

	automobile->Damage.m_fWheelDamageEffect = pSync->Damage.m_fWheelDamageEffect;
	automobile->Damage.m_lightStatus = pSync->Damage.m_lightStatus;
	automobile->Damage.m_panelStatus = pSync->Damage.m_panelStatus;

#ifdef FIX_BUGS
	automobile->Damage.m_bSmashedDoorDoesntClose = pSync->Damage.m_bSmashedDoorDoesntClose;
	automobile->Damage.m_engineStatus = pSync->Damage.m_engineStatus;
#endif

	for (uint32 i = 0; i < ARRAY_SIZE(automobile->m_aWheelTimer); i++)
		automobile->m_aWheelTimer[i] = pSync->m_aWheelTimer[i];

	for (uint32 i = 0; i < ARRAY_SIZE(automobile->m_aWheelState); i++)
		automobile->m_aWheelState[i] = pSync->m_aWheelState[i];

	for (uint32 i = 0; i < ARRAY_SIZE(automobile->m_aWheelRotation); i++)
		automobile->m_aWheelRotation[i] = pSync->m_aWheelRotation[i];

	for (uint32 i = 0; i < ARRAY_SIZE(automobile->m_aWheelPosition); i++)
		automobile->m_aWheelPosition[i] = pSync->m_aWheelPosition[i];

	for (uint32 i = 0; i < ARRAY_SIZE(automobile->m_aWheelSpeed); i++)
		automobile->m_aWheelSpeed[i] = pSync->m_aWheelSpeed[i];

	for (uint32 i = 0; i < ARRAY_SIZE(automobile->m_aSuspensionSpringRatio); i++)
		automobile->m_aSuspensionSpringRatio[i] = pSync->m_aSuspensionSpringRatio[i];

	for (uint32 i = 0; i < ARRAY_SIZE(automobile->m_aWheelColPoints); i++)
		automobile->m_aWheelColPoints[i] = pSync->m_aWheelColPoints[i];

	for (uint32 i = 0; i < ARRAY_SIZE(automobile->m_aWheelSkidmarkType); i++)
		automobile->m_aWheelSkidmarkType[i] = pSync->m_aWheelSkidmarkType[i];

	// Skidmark flags
	for (uint32 i = 0; i < ARRAY_SIZE(automobile->m_aWheelSkidmarkBloody); i++) {
		automobile->m_aWheelSkidmarkBloody[i] = (pSync->m_nSkidmarkFlags & BIT(i));
		automobile->m_aWheelSkidmarkUnk[i] = (pSync->m_nSkidmarkFlags & BIT(i + 4));
	}

	sAutomobileBase::ReceiveEntity(nOwner, nID, nTime);
	automobile->ResetSuspension();
	AttachSync(m_nTime, new sAutomobileSync(automobile));
}

void sAutomobile::SetupModel() {
	sAutomobileBase::SetupModel();
	assert(GetPhysical());
	CVehicleModelInfo* mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(GetPhysical()->GetModelIndex());
	tHandlingData* pHandling = GET_HANDLING(GetPhysical()->GetModelIndex());

	SetupDoors();
	SetupDamage();

	CColModel* colModel = mi->GetColModel();

	//bool adjustColModel;
	if (colModel->lines == nil)
	{
		//adjustColModel = true;
		colModel->numLines = 4;
#ifdef FIX_BUGS // in idb CBike line allocator and sBike are different, i guess sBike bugly RwMalloc (delete[])
		colModel->lines = new CColLine[4];
#else
		colModel->lines = (CColLine*)RwMalloc(4 * sizeof(CColLine));
#endif
	}
	//else
	//	adjustColModel = false;

	CVector posn;
	// Each suspension line starts at the uppermost wheel position
	// and extends down to the lowermost point on the tyre
	for (uint32 i = 0; i < 4; i++)
	{
		mi->GetWheelPosn(i, posn);
		//m_aWheelPosition[i] = posn.z;

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

	// Compress spring somewhat to get normal height on road
	float fHeightAboveRoad = m_aSuspensionSpringLength[0] * (1.0f - 1.0f / (4.0f * pHandling->fSuspensionForceLevel)) - colModel->lines[0].p0.z + mi->m_wheelScale * 0.5f;
	//for (uint32 i = 0; i < 4; i++)
	//	m_aWheelPosition[i] = mi->m_wheelScale * 0.5f - m_fHeightAboveRoad;

	// adjust col model to include suspension lines
	if (colModel->boundingBox.min.z > colModel->lines[0].p1.z)
		colModel->boundingBox.min.z = colModel->lines[0].p1.z;
	float radius = Max(colModel->boundingBox.min.Magnitude(), colModel->boundingBox.max.Magnitude());
	if (colModel->boundingSphere.radius < radius)
		colModel->boundingSphere.radius = radius;

	//if (GetPhysical()->GetModelIndex() == MI_RCBANDIT)
	//{
	//	colModel->boundingSphere.radius = 2.0f;
	//	for (i = 0; i < colModel->numSpheres; i++)
	//		colModel->spheres[i].radius = 0.3f;
	//}

	if (pHandling->IsHandlingFlagSet(eHandlingFlags::HANDLING_FORCE_GRND_CLR)/* && adjustColModel*/)
	{
		// 0.25 is the min distance between ground and col spheres, everything above it is safe
		float safePos = 0.25f - fHeightAboveRoad;
		for (uint32 i = 0; i < colModel->numSpheres; i++)
		{
			CColSphere* sph = &colModel->spheres[i];
			if (sph->center.z - sph->radius < safePos)
			{
				// sphere extends too far down, so move it up
				// or decrease the radius for bigger spheres
				if (radius > 0.4f)
					sph->radius = Max(sph->center.z - safePos, 0.4f);
				sph->center.z = safePos + sph->radius;
			}
		}
	}

	m_nHandlingFlags = pHandling->handlingFlags;

	if (pHandling->IsModelFlagSet(eHandlingModelFlags::HANDLING_MODEL_IS_HELI)) { // if (IsRealHeli()) {
		RpAtomicSetFlags((RpAtomic*)GetFirstObject(m_aCarNodes[CAR_WHEEL_LF]), 0);
		RpAtomicSetFlags((RpAtomic*)GetFirstObject(m_aCarNodes[CAR_WHEEL_RF]), 0);
		RpAtomicSetFlags((RpAtomic*)GetFirstObject(m_aCarNodes[CAR_WHEEL_LB]), 0);
		RpAtomicSetFlags((RpAtomic*)GetFirstObject(m_aCarNodes[CAR_WHEEL_RB]), 0);
	}
}

void sAutomobile::CompareSyncState(sAutomobileSync* pSync, sAutomobileSync* pLastSync, uint32 nDelta, tAutomobileSyncsDeltas* pDiff) {
	sAutomobileBase::CompareSyncState(pSync, pLastSync, nDelta, &pDiff->tAutomobileBaseDiff);

	if (pDiff->tAutomobileBaseDiff.nAutomobileBaseDiff != eAutomobileBaseSync::MP_PKTD_AUTOBASE_EQUAL)
		pDiff->nAutomobileDiff |= eAutomobileSync::MP_PKTD_AUTO_AUTOBASE;

	// Damage
	pDiff->nDamageDiff = CompareDamageSyncState(&pSync->Damage, &pLastSync->Damage);
	if (pDiff->nDamageDiff != eDamageSync::MP_PKTD_DAM_EQUAL)
		pDiff->nAutomobileDiff |= eAutomobileSync::MP_PKTD_AUTO_DAMAGE;

	// Car Gun LR
	if (FLT_EPS_NOT_EQ(pLastSync->m_fCarGunLR, pSync->m_fCarGunLR))
		pDiff->nAutomobileDiff |= eAutomobileSync::MP_PKTD_AUTO_CAR_GUN_LR;
#ifdef FIX_BUGS
	else // !!---- UPD!!
		pSync->m_fCarGunLR = pLastSync->m_fCarGunLR;
#endif
}

void sAutomobile::PerformWriteSync(sWriteSyncStream* pSyncStream, sAutomobileSync* pSync, tAutomobileSyncsDeltas* pDiff) {
	pSyncStream->WriteU32(pDiff->nAutomobileDiff);

	if (pDiff->nAutomobileDiff & eAutomobileSync::MP_PKTD_AUTO_AUTOBASE)
		sAutomobileBase::PerformWriteSync(pSyncStream, pSync, &pDiff->tAutomobileBaseDiff);

	if (pDiff->nAutomobileDiff & eAutomobileSync::MP_PKTD_AUTO_DAMAGE)
		PerformWriteDamageSync(pSyncStream, &pSync->Damage, pDiff->nDamageDiff);

	if (pDiff->nAutomobileDiff & eAutomobileSync::MP_PKTD_AUTO_CAR_GUN_LR)
		pSyncStream->WriteFloat(pSync->m_fCarGunLR);
}

bool sAutomobile::WriteSyncDelta(sWriteSyncStream* pSyncStream, sAutomobileSync* pSync, sAutomobileSync* pLastSync, uint32 nDelta) {
	tAutomobileSyncsDeltas automobileDeltaManager{};
	automobileDeltaManager.SetEqual();
	CompareSyncState(pSync, pLastSync, nDelta, &automobileDeltaManager);

	if (automobileDeltaManager.nAutomobileDiff == eAutomobileSync::MP_PKTD_AUTO_EQUAL) // main delta
		return false;

	PerformWriteSync(pSyncStream, pSync, &automobileDeltaManager);
	return true;
}

uint8 sAutomobile::CompareDamageSyncState(sDamageManager* pSync, sDamageManager* pLastSync) {
	uint8 nDiffMask = eDamageSync::MP_PKTD_DAM_EQUAL;

	// m_fWheelDamageEffect
	if (pLastSync->m_fWheelDamageEffect != pSync->m_fWheelDamageEffect)
		nDiffMask |= eDamageSync::MP_PKTD_DAM_WHEEL_DAMAGE_EFFECT;

	// m_wheelStatus[CARWHEEL_FRONT_LEFT], m_wheelStatus[CARWHEEL_REAR_LEFT]
	if (pLastSync->m_wheelStatus[CARWHEEL_FRONT_LEFT] != pSync->m_wheelStatus[CARWHEEL_FRONT_LEFT] ||
		pLastSync->m_wheelStatus[CARWHEEL_REAR_LEFT] != pSync->m_wheelStatus[CARWHEEL_REAR_LEFT])
		nDiffMask |= eDamageSync::MP_PKTD_DAM_WHEEL_STATUS_LEFT;

	// m_wheelStatus[CARWHEEL_FRONT_RIGHT], m_wheelStatus[CARWHEEL_REAR_RIGHT]
	if (pLastSync->m_wheelStatus[CARWHEEL_FRONT_RIGHT] != pSync->m_wheelStatus[CARWHEEL_FRONT_RIGHT] ||
		pLastSync->m_wheelStatus[CARWHEEL_REAR_RIGHT] != pSync->m_wheelStatus[CARWHEEL_REAR_RIGHT])
		nDiffMask |= eDamageSync::MP_PKTD_DAM_WHEEL_STATUS_RIGHT;

	// m_doorStatus[DOOR_BONNET], m_doorStatus[DOOR_BOOT]
	if (pLastSync->m_doorStatus[DOOR_BONNET] != pSync->m_doorStatus[DOOR_BONNET] ||
		pLastSync->m_doorStatus[DOOR_BOOT] != pSync->m_doorStatus[DOOR_BOOT])
		nDiffMask |= eDamageSync::MP_PKTD_DAM_DOOR_STATUS_BON_BOOT;

	// m_doorStatus[DOOR_FRONT_LEFT], m_doorStatus[DOOR_FRONT_RIGHT]
	if (pLastSync->m_doorStatus[DOOR_FRONT_LEFT] != pSync->m_doorStatus[DOOR_FRONT_LEFT] ||
		pLastSync->m_doorStatus[DOOR_FRONT_RIGHT] != pSync->m_doorStatus[DOOR_FRONT_RIGHT])
		nDiffMask |= eDamageSync::MP_PKTD_DAM_DOOR_STATUS_FRONT;

	// m_doorStatus[DOOR_REAR_LEFT], m_doorStatus[DOOR_REAR_RIGHT]
	if (pLastSync->m_doorStatus[DOOR_REAR_LEFT] != pSync->m_doorStatus[DOOR_REAR_LEFT] ||
		pLastSync->m_doorStatus[DOOR_REAR_RIGHT] != pSync->m_doorStatus[DOOR_REAR_RIGHT])
		nDiffMask |= eDamageSync::MP_PKTD_DAM_DOOR_STATUS_REAR;

	// m_lightStatus
	if (pLastSync->m_lightStatus != pSync->m_lightStatus)
		nDiffMask |= eDamageSync::MP_PKTD_DAM_LIGHT_STATUS;

	// m_panelStatus
	if (pLastSync->m_panelStatus != pSync->m_panelStatus)
		nDiffMask |= eDamageSync::MP_PKTD_DAM_PANEL_STATUS;

	return nDiffMask;
}

void sAutomobile::PerformWriteDamageSync(sWriteSyncStream* pSyncStream, sDamageManager* pSync, uint8 nDiffMask) {
	pSyncStream->WriteU8(nDiffMask);

	if (nDiffMask & eDamageSync::MP_PKTD_DAM_WHEEL_DAMAGE_EFFECT)
		pSyncStream->WriteFloat(pSync->m_fWheelDamageEffect);

	if (nDiffMask & eDamageSync::MP_PKTD_DAM_WHEEL_STATUS_LEFT)
	{
		pSyncStream->WriteU8(pSync->m_wheelStatus[CARWHEEL_FRONT_LEFT]);
		pSyncStream->WriteU8(pSync->m_wheelStatus[CARWHEEL_REAR_LEFT]);
	}

	if (nDiffMask & eDamageSync::MP_PKTD_DAM_WHEEL_STATUS_RIGHT)
	{
		pSyncStream->WriteU8(pSync->m_wheelStatus[CARWHEEL_FRONT_RIGHT]);
		pSyncStream->WriteU8(pSync->m_wheelStatus[CARWHEEL_REAR_RIGHT]);
	}

	if (nDiffMask & eDamageSync::MP_PKTD_DAM_DOOR_STATUS_BON_BOOT)
	{
		pSyncStream->WriteU8(pSync->m_doorStatus[DOOR_BONNET]);
		pSyncStream->WriteU8(pSync->m_doorStatus[DOOR_BOOT]);
	}

	if (nDiffMask & eDamageSync::MP_PKTD_DAM_DOOR_STATUS_FRONT)
	{
		pSyncStream->WriteU8(pSync->m_doorStatus[DOOR_FRONT_LEFT]);
		pSyncStream->WriteU8(pSync->m_doorStatus[DOOR_FRONT_RIGHT]);
	}

	if (nDiffMask & eDamageSync::MP_PKTD_DAM_DOOR_STATUS_REAR)
	{
		pSyncStream->WriteU8(pSync->m_doorStatus[DOOR_REAR_LEFT]);
		pSyncStream->WriteU8(pSync->m_doorStatus[DOOR_REAR_RIGHT]);
	}

	if (nDiffMask & eDamageSync::MP_PKTD_DAM_LIGHT_STATUS)
		pSyncStream->WriteU32(pSync->m_lightStatus);

	if (nDiffMask & eDamageSync::MP_PKTD_DAM_PANEL_STATUS)
		pSyncStream->WriteU32(pSync->m_panelStatus);
}

void sAutomobile::ReadDamageSyncFromStream(sReadSyncStream* pSyncStream, sDamageManager* pOutSync) {
	uint8 diffMask = pSyncStream->ReadU8();

	if (diffMask & eDamageSync::MP_PKTD_DAM_WHEEL_DAMAGE_EFFECT)
		pOutSync->m_fWheelDamageEffect = pSyncStream->ReadFloat();

	if (diffMask & eDamageSync::MP_PKTD_DAM_WHEEL_STATUS_LEFT)
	{
		pOutSync->m_wheelStatus[CARWHEEL_FRONT_LEFT] = pSyncStream->ReadU8();
		pOutSync->m_wheelStatus[CARWHEEL_REAR_LEFT] = pSyncStream->ReadU8();
	}

	if (diffMask & eDamageSync::MP_PKTD_DAM_WHEEL_STATUS_RIGHT)
	{
		pOutSync->m_wheelStatus[CARWHEEL_FRONT_RIGHT] = pSyncStream->ReadU8();
		pOutSync->m_wheelStatus[CARWHEEL_REAR_RIGHT] = pSyncStream->ReadU8();
	}

	if (diffMask & eDamageSync::MP_PKTD_DAM_DOOR_STATUS_BON_BOOT)
	{
		pOutSync->m_doorStatus[DOOR_BONNET] = pSyncStream->ReadU8();
		pOutSync->m_doorStatus[DOOR_BOOT] = pSyncStream->ReadU8();
	}

	if (diffMask & eDamageSync::MP_PKTD_DAM_DOOR_STATUS_FRONT)
	{
		pOutSync->m_doorStatus[DOOR_FRONT_LEFT] = pSyncStream->ReadU8();
		pOutSync->m_doorStatus[DOOR_FRONT_RIGHT] = pSyncStream->ReadU8();
	}

	if (diffMask & eDamageSync::MP_PKTD_DAM_DOOR_STATUS_REAR)
	{
		pOutSync->m_doorStatus[DOOR_REAR_LEFT] = pSyncStream->ReadU8();
		pOutSync->m_doorStatus[DOOR_REAR_RIGHT] = pSyncStream->ReadU8();
	}

	if (diffMask & eDamageSync::MP_PKTD_DAM_LIGHT_STATUS)
		pOutSync->m_lightStatus = pSyncStream->ReadU32();

	if (diffMask & eDamageSync::MP_PKTD_DAM_PANEL_STATUS)
		pOutSync->m_panelStatus = pSyncStream->ReadU32();
}

void sAutomobile::SetupDamage() {
	assert(GetPhysical());
	tHandlingData* pHandling = GET_HANDLING(GetPhysical()->GetModelIndex());
	if (pHandling->IsModelFlagSet(eHandlingModelFlags::HANDLING_MODEL_NO_DOORS))
	{
		Damage.SetDoorStatus(DOOR_FRONT_LEFT, DOOR_STATUS_MISSING);
		Damage.SetDoorStatus(DOOR_FRONT_RIGHT, DOOR_STATUS_MISSING);
		Damage.SetDoorStatus(DOOR_REAR_LEFT, DOOR_STATUS_MISSING);
		Damage.SetDoorStatus(DOOR_REAR_RIGHT, DOOR_STATUS_MISSING);
	}
}

void sAutomobile::Fix() {
	assert(GetPhysical());
	Damage.ResetDamageStatus();
	RpClumpForAllAtomics(GetPhysical()->GetClump(), CVehicleModelInfo::HideAllComponentsAtomicCB, (void*)ATOMIC_FLAG_DAM);

	for (int32 component = CAR_BUMP_FRONT; component < NUM_CAR_NODES; component++)
	{
		if (m_aCarNodes[component])
		{
			CMatrix mat(RwFrameGetMatrix(m_aCarNodes[component]));
			mat.SetTranslate(mat.GetPosition());
			mat.UpdateRW();
		}
	}
}

void sAutomobile::SetPanelDamage(sAutomobileSync* pSync, eCarNodes component, ePanels panel) {
	int32 nSyncStatus = pSync->Damage.GetPanelStatus(panel);
	int32 nAutoStatus = Damage.GetPanelStatus(panel);

	if (nSyncStatus == nAutoStatus)
		return;

	Damage.SetPanelStatus(panel, nSyncStatus);

	if (m_aCarNodes[component] == nil)
		return;

	if (nSyncStatus == PANEL_STATUS_SMASHED1) {
		// show damaged part
		SetComponentVisibility(m_aCarNodes[component], ATOMIC_FLAG_DAM);
	}
	else if (nSyncStatus == PANEL_STATUS_MISSING) {
		// hide both
		SetComponentVisibility(m_aCarNodes[component], ATOMIC_FLAG_NONE);
	}
}

void sAutomobile::SetBumperDamage(sAutomobileSync* pSync, eCarNodes component, ePanels panel) {
	SetPanelDamage(pSync, component, panel);
}

void sAutomobile::SetDoorDamage(sAutomobileSync* pSync, eCarNodes component, eDoors door) {
	eDoorStatus nSyncStatus = pSync->Damage.GetDoorStatus(door);
	eDoorStatus nAutoStatus = Damage.GetDoorStatus(door);
	if (nSyncStatus == nAutoStatus)
		return;

	Damage.SetDoorStatus(door, nSyncStatus);

	if (m_aCarNodes[component] == nil)
	{
#ifdef DEBUG_MULTIGAME_IMPROVEMENTS
		assert(GetPhysical());
		printf("Trying to damage component %d of %s\n",
			component, CModelInfo::GetModelInfo(GetPhysical()->GetModelIndex())->GetModelName());
#endif
		return;
	}

	switch (nSyncStatus)
	{
		case DOOR_STATUS_SMASHED:
		{
			// show damaged part
			SetComponentVisibility(m_aCarNodes[component], ATOMIC_FLAG_DAM);
			break;
		}
		case DOOR_STATUS_SWINGING:
		{
			// turn off angle cull for swinging doors
			RwFrameForAllObjects(m_aCarNodes[component], CVehicleModelInfo::SetAtomicFlagCB, (void*)ATOMIC_FLAG_NOCULL);
			break;
		}
		case DOOR_STATUS_MISSING:
		{
			//RpAtomic* okdam[2] = { nil, nil };
			//RwFrameForAllObjects(m_aCarNodes[component], GetOkAndDamagedAtomicCB, okdam); // !! VCS use new GetOkAndDamagedAtomicCB for 1 atomic, array (my GetOkOrDamagedAtomicCB)
			RpAtomic* okdam = nil;
			RwFrameForAllObjects(m_aCarNodes[component], GetOkOrDamagedAtomicCB, &okdam);
			assert(okdam && RwObjectGetType((RwObject*)okdam) == rpATOMIC);
			if (okdam != nil && (RpAtomicGetFlags(okdam) & rpATOMICRENDER)) {
				SetComponentVisibility(m_aCarNodes[component], ATOMIC_FLAG_DAM);
				RwFrameForAllObjects(m_aCarNodes[component], CVehicleModelInfo::SetAtomicFlagCB, (void*)ATOMIC_FLAG_NOCULL);
				Damage.SetDoorStatus(door, DOOR_STATUS_SWINGING);
			}
			else {
				// hide both
				SetComponentVisibility(m_aCarNodes[component], ATOMIC_FLAG_NONE);
			}
			break;
		}
	}
}

CObject* sAutomobile::SpawnFlyingComponent(eCarNodes component, uint32 type, int32 unused, CVector vecMoveSpeed, CVector vecTurnSpeed, bool bRenderScorched)
{
	RpAtomic* atomic;
	RwFrame* frame;
	RwMatrix* matrix;
	CObject* obj;

	if (CObject::nNoTempObjects >= NUMTEMPOBJECTS)
		return nil;

	atomic = nil;
	RwFrameForAllObjects(m_aCarNodes[component], GetCurrentAtomicObjectCB, &atomic);
	if (atomic == nil)
		return nil;

	obj = new CObject();
	if (obj == nil)
		return nil;

	obj->bPhys_lvcs_unk_1 = false;
	if (component == CAR_WINDSCREEN)
	{
		obj->SetModelIndexNoCreate(MI_CAR_BONNET);
	}
	else
		switch (type)
		{
			case COMPGROUP_BUMPER:
				obj->SetModelIndexNoCreate(MI_CAR_BUMPER);
				break;
			case COMPGROUP_WHEEL:
				obj->SetModelIndexNoCreate(MI_CAR_WHEEL);
				break;
			case COMPGROUP_DOOR:
				obj->SetModelIndexNoCreate(MI_CAR_DOOR);
				obj->SetCenterOfMass(0.0f, -0.5f, 0.0f);
				obj->bDrawLast = true;
				break;
			case COMPGROUP_BONNET:
				obj->SetModelIndexNoCreate(MI_CAR_BONNET);
				obj->SetCenterOfMass(0.0f, 0.4f, 0.0f);
				break;
			case COMPGROUP_BOOT:
				obj->SetModelIndexNoCreate(MI_CAR_BOOT);
				obj->SetCenterOfMass(0.0f, -0.3f, 0.0f);
				break;
			case COMPGROUP_PANEL:
			default:
				obj->SetModelIndexNoCreate(MI_CAR_PANEL);
				break;
		}

#ifdef FIX_BUGS
	// object needs base model
	assert(GetPhysical());
	obj->RefModelInfo(GetPhysical()->GetModelIndex());
#endif

	// create new atomic
	matrix = RwFrameGetLTM(m_aCarNodes[component]);
	frame = RwFrameCreate();
	atomic = RpAtomicClone(atomic);
	*RwFrameGetMatrix(frame) = *matrix;
	RpAtomicSetFrame(atomic, frame);
	CVisibilityPlugins::SetAtomicRenderCallback(atomic, nil);
	CStreaming::RegisterInstance(atomic, nil);
	obj->AttachToRwObject((RwObject*)atomic);
	obj->bDontStream = true;

	// init object
	obj->m_fMass = 10.0f;
	obj->m_fTurnMass = 25.0f;
	obj->m_fAirResistance = 0.97f;
	obj->m_fElasticity = 0.1f;
	obj->m_fBuoyancy = obj->m_fMass * GRAVITY / 0.75f;
	obj->ObjectCreatedBy = TEMP_OBJECT;
	obj->SetIsStatic(false);
	obj->bIsPickup = false;
	obj->bUseVehicleColours = true;
	obj->m_aColours[VEHICLE_COLOUR_PRIMARY] = m_aColours[VEHICLE_COLOUR_PRIMARY];
	obj->m_aColours[VEHICLE_COLOUR_SECONDARY] = m_aColours[VEHICLE_COLOUR_SECONDARY];

	// life time - the more objects the are, the shorter this one will live
	CObject::nNoTempObjects++;
	if (CObject::nNoTempObjects > 20)
		obj->m_nEndOfLifeTime = CTimer::GetTimeInMilliseconds() + 20000 / 5.0f;
	else if (CObject::nNoTempObjects > 10)
		obj->m_nEndOfLifeTime = CTimer::GetTimeInMilliseconds() + 20000 / 2.0f;
	else
		obj->m_nEndOfLifeTime = CTimer::GetTimeInMilliseconds() + 20000;

	obj->SetMoveSpeed(vecMoveSpeed);
	obj->m_vecTurnSpeed = vecTurnSpeed;

	if (bRenderScorched)
		obj->bRenderScorched = true;

	CWorld::Add(obj);

	return obj;
}