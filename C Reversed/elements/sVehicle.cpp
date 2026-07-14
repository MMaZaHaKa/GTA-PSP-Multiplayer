/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"
#include "AnimBlendAssociation.h"
#include "Automobile.h"
#include "Ped.h"
#include "Vehicle.h"
#include "Garages.h"
#include "DMAudio.h"
#include "VisibilityPlugins.h"
#include "Streaming.h"
#include "AnimManager.h"
#include "Renderer.h"
#include "Lights.h"
#include "PointLights.h"
#include "Particle.h"
#include "PlayerPed.h"
#include "PlayerInfo.h"
#include "Weather.h"
#include "Zones.h"
#include "World.h"
#include "AudioManager.h"

#include "multiplayer/MultiGame.h"
#include "multiplayer/elements/sSyncStream.h"
#include "multiplayer/elements/sVehicle.h"

// Audio
#ifndef GTA_LIBERTY
#include "multiplayer/elements/sBmx.h"
#include "multiplayer/elements/sBoat.h"
#include "multiplayer/elements/sPlane.h"
#include "multiplayer/elements/sHeli.h"
#include "multiplayer/elements/sQuadBike.h"
#endif
#include "multiplayer/elements/sBike.h"
#include "multiplayer/elements/sAutomobile.h"

int32 sVehicle::ms_nNumberOfSyncedVehicles = 0;

sAutoPilotSync::sAutoPilotSync(CAutoPilot* pAutoPilot)
{
	// Node Info
#ifdef GTA_LIBERTY
	m_nNextPathNodeInfo = pAutoPilot->m_nNextPathNodeInfo;
	m_nCurrentPathNodeInfo = pAutoPilot->m_nCurrentPathNodeInfo;
#else
	m_nNextPathNodeInfo = pAutoPilot->m_aRouteNodes[ROUTE_NODE_NEXT].m_nPathNodeInfo;
	m_nCurrentPathNodeInfo = pAutoPilot->m_aRouteNodes[ROUTE_NODE_CURRENT].m_nPathNodeInfo;
	m_nPreviousPathNodeInfo = pAutoPilot->m_aRouteNodes[ROUTE_NODE_PREV].m_nPathNodeInfo;
	m_nNextNextPathNodeInfo = pAutoPilot->m_aRouteNodes[ROUTE_NODE_NEXT_NEXT].m_nPathNodeInfo;
#endif

	// Time
	m_nTimeToSpendOnCurrentCurve = pAutoPilot->m_nTimeToSpendOnCurrentCurve;
	m_nTimeEnteredCurve = pAutoPilot->m_nTimeEnteredCurve;

	// Route Node
#ifdef GTA_LIBERTY
	m_nNextRouteNode = pAutoPilot->m_nNextRouteNode;
	m_nCurrentRouteNode = pAutoPilot->m_nCurrentRouteNode;
#else
	m_nNextRouteNode = pAutoPilot->m_aRouteNodes[ROUTE_NODE_NEXT].m_nRouteNode;
	m_nCurrentRouteNode = pAutoPilot->m_aRouteNodes[ROUTE_NODE_CURRENT].m_nRouteNode;
	m_nPrevRouteNode = pAutoPilot->m_aRouteNodes[ROUTE_NODE_PREV].m_nRouteNode;
	m_nNextNextRouteNode = pAutoPilot->m_aRouteNodes[ROUTE_NODE_NEXT_NEXT].m_nRouteNode;
#endif

	// Lane
#ifdef GTA_LIBERTY
	m_nNextLane = pAutoPilot->m_nNextLane;
	m_nCurrentLane = pAutoPilot->m_nCurrentLane;
#else
	m_nNextLane = pAutoPilot->m_aRouteNodes[ROUTE_NODE_NEXT].m_nLane;
	m_nCurrentLane = pAutoPilot->m_aRouteNodes[ROUTE_NODE_CURRENT].m_nLane;
	m_nPrevLane = pAutoPilot->m_aRouteNodes[ROUTE_NODE_PREV].m_nLane;
	m_nNextNextLane = pAutoPilot->m_aRouteNodes[ROUTE_NODE_NEXT_NEXT].m_nLane;
#endif

	// Direction
#ifdef GTA_LIBERTY
	m_nNextDirection = pAutoPilot->m_nNextDirection;
	m_nCurrentDirection = pAutoPilot->m_nCurrentDirection;
#else
	m_nNextDirection = pAutoPilot->m_aRouteNodes[ROUTE_NODE_NEXT].m_nDirection;
	m_nNextNextDirection = pAutoPilot->m_aRouteNodes[ROUTE_NODE_NEXT_NEXT].m_nDirection;
	m_nCurrentDirection = pAutoPilot->m_aRouteNodes[ROUTE_NODE_CURRENT].m_nDirection;
	m_nPreviousDirection = pAutoPilot->m_aRouteNodes[ROUTE_NODE_PREV].m_nDirection;
#endif

	m_nCruiseSpeed = pAutoPilot->m_nCruiseSpeed;
	m_nMaxTrafficSpeed = pAutoPilot->m_fMaxTrafficSpeed;
	m_nCarMission = pAutoPilot->m_nCarMission;
	m_nDrivingStyle = pAutoPilot->m_nDrivingStyle;

	// what about field_28/lcs field_18 ?
}

sAutoPilotSync::sAutoPilotSync(const sAutoPilotSync& other)
{
	// Node Info
	m_nNextPathNodeInfo = other.m_nNextPathNodeInfo;
	m_nCurrentPathNodeInfo = other.m_nCurrentPathNodeInfo;
#ifndef GTA_LIBERTY
	m_nPreviousPathNodeInfo = other.m_nPreviousPathNodeInfo;
	m_nNextNextPathNodeInfo = other.m_nNextNextPathNodeInfo;
#endif

	// Time
	m_nTimeToSpendOnCurrentCurve = other.m_nTimeToSpendOnCurrentCurve;
	m_nTimeEnteredCurve = other.m_nTimeEnteredCurve;

	// Route Node
	m_nNextRouteNode = other.m_nNextRouteNode;
	m_nCurrentRouteNode = other.m_nCurrentRouteNode;
#ifndef GTA_LIBERTY
	m_nPrevRouteNode = other.m_nPrevRouteNode;
	m_nNextNextRouteNode = other.m_nNextNextRouteNode;
#endif

	field_28 = other.field_28;
	m_nNextLane = other.m_nNextLane;
	m_nNextDirection = other.m_nNextDirection;
	m_nCruiseSpeed = other.m_nCruiseSpeed;

#ifdef FIX_BUGS
	// next already lane
	m_nCurrentLane = other.m_nCurrentLane;
#ifndef GTA_LIBERTY
	m_nPrevLane = other.m_nPrevLane;
	m_nNextNextLane = other.m_nNextNextLane;
#endif

	// next already direction
	m_nCurrentDirection = other.m_nCurrentDirection;
#ifndef GTA_LIBERTY
	m_nPreviousDirection = other.m_nPreviousDirection;
	m_nNextNextDirection = other.m_nNextNextDirection;
#endif

	m_nMaxTrafficSpeed = other.m_nMaxTrafficSpeed;
	m_nCarMission = other.m_nCarMission;
	m_nDrivingStyle = other.m_nDrivingStyle;
#endif
}

#if !defined(FINAL) && !defined(MASTER)
void sAutoPilotSync::Dump()
{
	printf("=== sAutoPilotSync Dump ===\n");
	printf("Node Info:\n");
	printf("  NextPathNodeInfo:     %u (0x%08X)\n", m_nNextPathNodeInfo, m_nNextPathNodeInfo);
	printf("  CurrentPathNodeInfo:  %u (0x%08X)\n", m_nCurrentPathNodeInfo, m_nCurrentPathNodeInfo);
#ifndef GTA_LIBERTY
	printf("  PreviousPathNodeInfo: %u (0x%08X)\n", m_nPreviousPathNodeInfo, m_nPreviousPathNodeInfo);
	printf("  NextNextPathNodeInfo: %u (0x%08X)\n", m_nNextNextPathNodeInfo, m_nNextNextPathNodeInfo);
#endif
	printf("\n");
	printf("Timing:\n");
	printf("  TimeToSpendOnCurrentCurve: %d\n", m_nTimeToSpendOnCurrentCurve);
	printf("  TimeEnteredCurve:          %d\n", m_nTimeEnteredCurve);
	printf("\n");
	printf("Route Node:\n");
	printf("  NextRouteNode:     %d\n", m_nNextRouteNode);
	printf("  CurrentRouteNode:  %d\n", m_nCurrentRouteNode);
#ifndef GTA_LIBERTY
	printf("  PrevRouteNode:     %d\n", m_nPrevRouteNode);
	printf("  NextNextRouteNode: %d\n", m_nNextNextRouteNode);
#endif
	printf("  field_28:          %d (0x%08X)\n", field_28, field_28);
	printf("\n");
	printf("Lane:\n");
	printf("  NextLane:     %d\n", m_nNextLane);
	printf("  CurrentLane:  %d\n", m_nCurrentLane);
#ifndef GTA_LIBERTY
	printf("  PrevLane:     %d\n", m_nPrevLane);
	printf("  NextNextLane: %d\n", m_nNextNextLane);
#endif
	printf("\n");
	printf("Direction:\n");
	printf("  NextDirection:        %d\n", m_nNextDirection);
#ifndef GTA_LIBERTY
	printf("  NextNextDirection:    %d\n", m_nNextNextDirection);
#endif
	printf("  CurrentDirection:     %d\n", m_nCurrentDirection);
#ifndef GTA_LIBERTY
	printf("  PreviousDirection:    %d\n", m_nPreviousDirection);
#endif
	printf("\n");
	printf("Settings:\n");
	printf("  CruiseSpeed:       %u (0x%02X)\n", m_nCruiseSpeed, m_nCruiseSpeed);
	printf("  MaxTrafficSpeed:   %u (0x%02X)\n", m_nMaxTrafficSpeed, m_nMaxTrafficSpeed);
	printf("  CarMission:        %u (0x%02X)\n", m_nCarMission, m_nCarMission);
	printf("  DrivingStyle:      %u (0x%02X)\n", m_nDrivingStyle, m_nDrivingStyle);
	printf("================================\n");
}
#endif

sVehicleSync::sVehicleSync() : sElementPhysicalSync(), AutoPilot()
{
	m_bSirenOrAlarm = false;
	m_fSteerAngle = 0.0f;
	m_fGasPedal = 0.0f;
	m_fBrakePedal = 0.0f;
	m_bIsHandbrakeOn = false;
	m_nCarJacker = 0;
	m_nStatus = eEntityStatus::STATUS_PLAYER;
	m_nCarHornTimer = 0;
#ifdef FIX_BUGS
	bHasDriver = false;
	bEngineOn = false;
#endif
	bIsDrivenByPlayer = false;
	m_nCurrentGear = 0;
	m_fChangeGearTime = 0.0f;
	bIsDrowning = false;
	m_nDriverTeam = -1;
	m_nDriverID = -1;
	m_nPlayerID = -1;
	m_nLastDriverID = -1;
	m_nSkidmarkFlags = 0;
	field_1C4 = -1;
	m_fLastDamageAmount = 0.0f;
	m_fHealth = 1000.0f;
	m_nLastDamagePlayerID = 0; // not -1?
	VehicleCreatedBy = RANDOM_VEHICLE;

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

sVehicleSync::sVehicleSync(CVehicle* pVeh) : sElementPhysicalSync(pVeh), AutoPilot(&pVeh->AutoPilot)
{
	m_fLastDamageAmount = pVeh->m_nDamageAmountMG;
	m_nLastDamagePlayerID = pVeh->m_nDamagedByPeerID;
	//AutoPilot = sAutoPilotSync(&pVeh->AutoPilot);
	VehicleCreatedBy = pVeh->VehicleCreatedBy;
	m_nDoorLock = pVeh->m_nDoorLock;
	m_fSteerAngle = pVeh->m_fSteerAngle;
	m_fGasPedal = pVeh->m_fGasPedal;
	m_fBrakePedal = pVeh->m_fBrakePedal;
	m_bIsHandbrakeOn = pVeh->bIsHandbrakeOn;
	m_nDriverID = pVeh->m_nDriverIdMG;
	m_nStatus = pVeh->GetStatus();
	m_nCurrentGear = pVeh->m_nCurrentGear;
	m_nCarJacker = pVeh->m_vehLCS_258;
	m_bSirenOrAlarm = pVeh->bSirenOrAlarm;
	m_fChangeGearTime = pVeh->m_fChangeGearTime;
	m_fHealth = pVeh->m_fHealth;
	bIsDrivenByPlayer = pVeh->pDriver && pVeh->pDriver->IsPlayer();
	m_nDriverTeam = pVeh->pDriver ? pVeh->pDriver->GetTeamID() : -1;
	bHasDriver = pVeh->pDriver != nil;
	bEngineOn = pVeh->bEngineOn;
	bIsDrowning = pVeh->bIsDrowning;
	m_nCarHornTimer = pVeh->m_nCarHornTimer;
	m_fHealth = pVeh->m_fHealth; // duplicate
	m_nPlayerID = pVeh->m_nPlayerID;
}

sVehicleSync::sVehicleSync(const sVehicleSync& other) : sElementPhysicalSync(other), AutoPilot(other.AutoPilot)
{
	m_bSirenOrAlarm = other.m_bSirenOrAlarm;
	m_fSteerAngle = other.m_fSteerAngle;
	m_fGasPedal = other.m_fGasPedal;
	m_fBrakePedal = other.m_fBrakePedal;
	m_bIsHandbrakeOn = other.m_bIsHandbrakeOn;
	m_nCarJacker = other.m_nCarJacker;
	m_nStatus = other.m_nStatus;
	m_nCarHornTimer = other.m_nCarHornTimer;
	m_nCurrentGear = other.m_nCurrentGear;
	m_fChangeGearTime = other.m_fChangeGearTime;
	bHasDriver = other.bHasDriver;
	bIsDrivenByPlayer = other.bIsDrivenByPlayer;
	bEngineOn = other.bEngineOn;
	bIsDrowning = other.bIsDrowning;
	m_nDriverTeam = other.m_nDriverTeam;
	m_nDriverID = other.m_nDriverID;
	m_nPlayerID = other.m_nPlayerID;
	m_nLastDriverID = other.m_nLastDriverID;

	for (uint32 i = 0; i < ARRAY_SIZE(m_aWheelTimer); i++)
		m_aWheelTimer[i] = other.m_aWheelTimer[i];
	for (uint32 i = 0; i < ARRAY_SIZE(m_aWheelState); i++)
		m_aWheelState[i] = other.m_aWheelState[i];
	for (uint32 i = 0; i < ARRAY_SIZE(m_aWheelRotation); i++)
		m_aWheelRotation[i] = other.m_aWheelRotation[i];
	for (uint32 i = 0; i < ARRAY_SIZE(m_aWheelPosition); i++)
		m_aWheelPosition[i] = other.m_aWheelPosition[i];
	for (uint32 i = 0; i < ARRAY_SIZE(m_aWheelSpeed); i++)
		m_aWheelSpeed[i] = other.m_aWheelSpeed[i];
	for (uint32 i = 0; i < ARRAY_SIZE(m_aSuspensionSpringRatio); i++)
		m_aSuspensionSpringRatio[i] = other.m_aSuspensionSpringRatio[i];
	for (uint32 i = 0; i < ARRAY_SIZE(m_aWheelColPoints); i++)
		m_aWheelColPoints[i] = other.m_aWheelColPoints[i];
	for (uint32 i = 0; i < ARRAY_SIZE(m_aWheelSkidmarkType); i++)
		m_aWheelSkidmarkType[i] = other.m_aWheelSkidmarkType[i];

	m_nSkidmarkFlags = other.m_nSkidmarkFlags;
	field_1C4 = other.field_1C4;
	m_fHealth = other.m_fHealth;
	m_fLastDamageAmount = other.m_fLastDamageAmount;
	m_nLastDamagePlayerID = other.m_nLastDamagePlayerID;
	//AutoPilot = sAutoPilotSync(other.AutoPilot);
	VehicleCreatedBy = other.VehicleCreatedBy;
	m_nDoorLock = other.m_nDoorLock;
}

sVehicleSync::~sVehicleSync() { }

// not checks: m_bSirenOrAlarm
bool sVehicleSync::Compare(const sVehicleSync& other)
{
	if (!sElementPhysicalSync::Compare(other))
		return false;

	// its bad idea, because: malloc non memset0 memory,
	// padding not init in ctor(vft can't memset 0, non POD struct), and we compare random padding malloc trash
	// TODO: compare each fields after m_fSteerAngle
	if (memcmp(&m_fSteerAngle, &other.m_fSteerAngle, BLOCK_SIZE_END(m_fSteerAngle, sVehicleSync)) != 0)
		return false;

	return true;
}

#if !defined(FINAL) && !defined(MASTER)
void sVehicleSync::Dump()
{
	sElementPhysicalSync::Dump();

	printf("=== sVehicleSync Dump ===\n");
	printf("Controls:\n");
	printf("  SteerAngle:    %.2f\n", m_fSteerAngle);
	printf("  GasPedal:      %.2f\n", m_fGasPedal);
	printf("  BrakePedal:    %.2f\n", m_fBrakePedal);
	printf("  IsHandbrakeOn: %s\n", m_bIsHandbrakeOn ? "true" : "false");
	printf("  CarHornTimer:  %u (0x%02X)\n", m_nCarHornTimer, m_nCarHornTimer);
	printf("  CurrentGear:   %u (0x%02X)\n", m_nCurrentGear, m_nCurrentGear);
	printf("  ChangeGearTime: %.2f\n", m_fChangeGearTime);
	printf("\n");

	printf("Status:\n");
	printf("  SirenOrAlarm:  %s\n", m_bSirenOrAlarm ? "true" : "false");
	printf("  CarJacker:     %d\n", m_nCarJacker);
	printf("  Status:        %u (0x%02X)\n", m_nStatus, m_nStatus);
	printf("\n");

	printf("Flags (0xCC):\n");
	printf("  HasDriver:        %u\n", bHasDriver);
	printf("  IsDrivenByPlayer: %u\n", bIsDrivenByPlayer);
	printf("  EngineOn:         %u\n", bEngineOn);
	printf("  IsDrowning:       %u\n", bIsDrowning);
	printf("  CC_10:            %u\n", bCC_10);
	printf("  CC_20:            %u\n", bCC_20);
	printf("  CC_40:            %u\n", bCC_40);
	printf("  CC_80:            %u\n", bCC_80);
	printf("\n");

	printf("Drivers:\n");
	printf("  DriverTeam:      %d\n", m_nDriverTeam);
	printf("  DriverID:        %d\n", m_nDriverID);
	printf("  m_nPlayerID:      %d\n", m_nPlayerID);
	printf("  LastDriverID:    %d\n", m_nLastDriverID);
	printf("\n");

	printf("Wheels:\n");
	printf("  WheelTimer[0]: %u (0x%02X)\n", m_aWheelTimer[0], m_aWheelTimer[0]);
	printf("  WheelTimer[1]: %u (0x%02X)\n", m_aWheelTimer[1], m_aWheelTimer[1]);
	printf("  WheelTimer[2]: %u (0x%02X)\n", m_aWheelTimer[2], m_aWheelTimer[2]);
	printf("  WheelTimer[3]: %u (0x%02X)\n", m_aWheelTimer[3], m_aWheelTimer[3]);
	printf("  SkidmarkFlags: %u (0x%02X)\n", m_nSkidmarkFlags, m_nSkidmarkFlags);
	printf("\n");

	printf("Damage & Health:\n");
	printf("  Health:            %.2f\n", m_fHealth);
	printf("  LastDamageAmount:  %.2f\n", m_fLastDamageAmount);
	printf("  LastDamagePlayerID: %u (0x%02X)\n", m_nLastDamagePlayerID, m_nLastDamagePlayerID);
	printf("  field_1C4:         %d (0x%08X)\n", field_1C4, field_1C4);
	printf("\n");

	printf("Vehicle Info:\n");
	printf("  VehicleCreatedBy: %u (0x%02X)\n", VehicleCreatedBy, VehicleCreatedBy);
	printf("  DoorLock:         %d (0x%08X)\n", m_nDoorLock, m_nDoorLock);
	printf("  field_214:        %d (0x%08X)\n", field_214, field_214);
	printf("  field_218:        %d (0x%08X)\n", field_218, field_218);
	printf("  field_21C:        %d (0x%08X)\n", field_21C, field_21C);
	printf("\n");

	printf("AutoPilot:\n");
	AutoPilot.Dump();

	printf("================================\n");
}
#endif


cVehicleMG::cVehicleMG(sElement* elem) : cPhysicalMG(elem) {
#ifdef FIX_BUGS
	m_vehType = eVehicleType::VEHICLE_TYPE_CAR; // 0
#endif
	m_pFire = nil;
	bIsVehicle = true;
	SetStatus(STATUS_SIMPLE);
	m_audioEntityId = DMAudio.CreateEntity(AUDIOTYPE_PHYSICAL, this);
	if (m_audioEntityId >= 0)
		DMAudio.SetEntityStatus(m_audioEntityId, TRUE);
	m_fVelocityChangeForAudio = 0.0f;
	m_fGasPedalAudio = 0.0f;
}

cVehicleMG::~cVehicleMG()
{
	DMAudio.DestroyEntity(m_audioEntityId);
}

void cVehicleMG::SetModelIndex(uint32 id)
{
	if (GetModelIndex() == id)
		return;

	if (!CStreaming::HasModelLoaded(id))
	{
		CStreaming::FlushRequestList();
		CStreaming::RequestModel(id, STREAMFLAGS_NOFADE | STREAMFLAGS_DEPENDENCY | STREAMFLAGS_SCRIPTOWNED, nil);
		CStreaming::LoadAllRequestedModels(false);
	}
	CEntity::SetModelIndex(id);
	GetElement().vehicle->SetupModel();
	tHandlingData* handling = GET_HANDLING(id);
	m_fMass = handling->fMass;
	m_fTurnMass = handling->fTurnMass;
	m_vecCentreOfMass = handling->CentreOfMass;
	GetElement().vehicle->m_fTraction = 1.0f;
	m_fForceMultiplier = 1.0f;
}

void cVehicleMG::PreRender(void)
{
	;
}

void cVehicleMG::Render(void)
{
	CVehicleModelInfo* mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex());
#ifdef GTA_LIBERTY
	mi->SetVehicleColour(&GetElement().vehicle->m_currentColour1, &GetElement().vehicle->m_currentColour2);
#else
	mi->SetVehicleColour(&GetElement().vehicle->m_aColours[VEHICLE_COLOUR_PRIMARY], &GetElement().vehicle->m_aColours[VEHICLE_COLOUR_SECONDARY]);
#endif
	cPhysicalMG::Render();
}

bool cVehicleMG::SetupLighting(void)
{
	ActivateDirectional();
	SetAmbientColoursForPedsCarsAndObjects();
	//gbForceEnvOff = false;
	// PSP gif g_GifCMDBufferPtr++ = 0xC9010100;
	if (bRenderScorched)
	{
		WorldReplaceNormalLightsWithScorched(Scene.world, 0.1f);
		//gbForceEnvOff = true;
	}
	else
	{
		CVector coors = GetPosition();
		float lighting = CPointLights::GenerateLightsAffectingObject(&coors);
		if (lighting != 1.0f)
		{
			SetAmbientAndDirectionalColours(lighting);
			return true;
		}
	}

	return false;
}

void cVehicleMG::RemoveLighting(bool reset)
{
	CRenderer::RemoveVehiclePedLights(this, reset);
	SetAmbientColours();
	DeActivateDirectional();
}

RpMaterial* SetMgCompAlphaCB(RpMaterial* material, void* data)
{
	uint32 alpha = (uint32)(uintptr)data;
	RwRGBA* col = (RwRGBA*)RpMaterialGetColor(material); // get rid of const
	col->alpha = alpha;
	return material;
}

void cVehicleMG::SetComponentAtomicAlpha(RpAtomic* atomic, int32 alpha)
{
	RpGeometry* geo = RpAtomicGetGeometry(atomic);
	RpGeometrySetFlags(geo, RpGeometryGetFlags(geo) | rpGEOMETRYMODULATEMATERIALCOLOR);
	RpGeometryForAllMaterials(geo, SetMgCompAlphaCB, RWVOIDRS(alpha));
}

int32 cVehicleMG::AddWheelDirtAndWater(CColPoint* colpoint, uint32 belowEffectSpeed) {
    int32 i;
    CVector dir;
    static RwRGBA grassCol = { 8, 24, 8, 255 };
    static RwRGBA gravelCol = { 64, 64, 64, 255 };
    static RwRGBA mudCol = { 64, 32, 16, 255 };
    static RwRGBA sandCol = { 170, 165, 140, 255 };
    static RwRGBA waterCol = { 48, 48, 64, 0 };

    if (!belowEffectSpeed && colpoint->surfaceB != SURFACE_SAND && colpoint->surfaceB != SURFACE_SAND_BEACH)
        return 0;

    switch (colpoint->surfaceB)
    {
        case SURFACE_GRASS:
            dir.x = -0.05f * m_vecMoveSpeed.x;
            dir.y = -0.05f * m_vecMoveSpeed.y;
            for (i = 0; i < 4; i++)
            {
                dir.z = CGeneral::GetRandomNumberInRange(0.03f, 0.06f);
                CParticle::AddParticle(PARTICLE_WHEEL_DIRT, colpoint->point, dir, nil,
                    CGeneral::GetRandomNumberInRange(0.02f, 0.1f), grassCol);
            }
            return 0;
        case SURFACE_GRAVEL:
            dir.x = -0.05f * m_vecMoveSpeed.x;
            dir.y = -0.05f * m_vecMoveSpeed.y;
            for (i = 0; i < 4; i++)
            {
                dir.z = CGeneral::GetRandomNumberInRange(0.03f, 0.06f);
                CParticle::AddParticle(PARTICLE_WHEEL_DIRT, colpoint->point, dir, nil,
                    CGeneral::GetRandomNumberInRange(0.05f, 0.09f), gravelCol);
            }
            return 1;
        case SURFACE_MUD_DRY:
            dir.x = -0.05f * m_vecMoveSpeed.x;
            dir.y = -0.05f * m_vecMoveSpeed.y;
            for (i = 0; i < 4; i++)
            {
                dir.z = CGeneral::GetRandomNumberInRange(0.03f, 0.06f);
                CParticle::AddParticle(PARTICLE_WHEEL_DIRT, colpoint->point, dir, nil,
                    CGeneral::GetRandomNumberInRange(0.02f, 0.06f), mudCol);
            }
            return 0;
        case SURFACE_SAND:
        case SURFACE_SAND_BEACH:
            if (CTimer::GetFrameCounter() & 2 ||
                CWeather::WetRoads > 0.0f && CGeneral::GetRandomNumberInRange(CWeather::WetRoads, 1.01f) > 0.5f)
                return 0;
            dir.x = 0.5f * m_vecMoveSpeed.x;
            dir.y = 0.5f * m_vecMoveSpeed.y;
            for (i = 0; i < 1; i++)
            {
                dir.z = CGeneral::GetRandomNumberInRange(0.02f, 0.055f);
                CParticle::AddParticle(PARTICLE_SAND, colpoint->point, dir, nil,
                    2.0f * m_vecMoveSpeed.Magnitude(), sandCol);
            }
            return 0;
        default:
            if (CWeather::WetRoads > 0.01f)
            {
                if (CTimer::GetFrameCounter() & 1)
                    CParticle::AddParticle(
                        PARTICLE_WATERSPRAY,
                        colpoint->point + CVector(0.0f, 0.0f, 0.25f + 0.25f),
                        CVector(0.0f, 0.0f, CGeneral::GetRandomNumberInRange(0.005f, 0.04f)),
                        nil,
                        CGeneral::GetRandomNumberInRange(0.1f, 0.5f), waterCol);
                return 0;
            }
            return 1;
    }
}


sVehicle::sVehicle() : sElementPhysical() {
	m_pPed = nil;
	++ms_nNumberOfSyncedVehicles;
}


ElementCapability sVehicle::GetCapability()
{
	return sVehicle::Capability();
}

bool sVehicle::HasCapability(ElementCapability capability)
{
	if (sVehicle::Capability() == capability)
		return true;
	if (sElementPhysical::Capability() == capability)
		return true;
	if (sElement::Capability() == capability)
		return true;
	return false;
}

sVehicle::~sVehicle() {
	for (int32 nIndex = 0; nIndex < CGarages::NumGarages; nIndex++) {
		CGarage& garage = CGarages::aGarages[nIndex];
		if (garage.m_pSSVehicle == this)
			garage.m_pSSVehicle = nil;
	}
	--ms_nNumberOfSyncedVehicles;

#ifdef FIX_BUGS
	if (GetEntity()) {
		CVehicle* pVeh = (CVehicle*)GetEntity();
		assert(pVeh->GetNumPassengers() == 0); // delete entity in sElement with passengers
	}
#endif

	//sElement::PurgeAttached(); // not sVehicle purge
	// ~sElementPhysical
}

// TODO: recheck particle floats
void sVehicle::ApplyClientSync(uint16 nTime) {
	sElementPhysical::ApplyClientSync(nTime);
	m_pPed = nil;

	m_pPed = (GetSync().vehicle->m_nPlayerID == cMultiGame::Instance().LocalPlayerID()) ? FindPlayerPed() : nil;
	assert(GetPhysical());
	CVector damagePos = ((CVehicleModelInfo*)CModelInfo::GetModelInfo(GetPhysical()->GetModelIndex()))->m_positions[CAR_POS_HEADLIGHTS];

	damagePos = GetSync().vehicle->GetMatrix() * damagePos;
	damagePos.z += 0.15f;

	if (GetSync().vehicle->m_fHealth < DAMAGE_HEALTH_TO_CATCH_FIRE && GetSync().vehicle->m_nStatus != eEntityStatus::STATUS_WRECKED)
	{
		// Car is on fire

		CParticle::AddParticle(PARTICLE_CARFLAME, damagePos,
			CVector(0.0f, 0.0f, CGeneral::GetRandomNumberInRange(0.01125f, 0.09f)),
			nil, 0.63f);

		CVector coors = damagePos;
		coors.x += CGeneral::GetRandomNumberInRange(-0.5625f, 0.5625f),
			coors.y += CGeneral::GetRandomNumberInRange(-0.5625f, 0.5625f),
			coors.z += CGeneral::GetRandomNumberInRange(0.5625f, 2.25f);
		CParticle::AddParticle(PARTICLE_CARFLAME_SMOKE, coors, CVector(0.0f, 0.0f, 0.0f));

		CParticle::AddParticle(PARTICLE_ENGINE_SMOKE2, damagePos, CVector(0.0f, 0.0f, 0.0f), nil, 0.5f);
	}

	if (GetSync().vehicle->m_nStatus == eEntityStatus::STATUS_WRECKED)
		GetPhysical()->bRenderScorched = true;
}

void sVehicle::ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) {
	if (nOwner != cMultiGame::Instance().LocalPlayerID()) {
		sElementPhysical::ReceiveEntity(nOwner, nID, nTime);
		return;
	}

	sVehicleSync* pSync = GetSync().vehicle;
	CVehicle* pVehicle = (CVehicle*)GetEntity();
	assert(pVehicle);

	// why it not in sElementPhysical::ReceiveEntity??
	pVehicle->SetMatrix(pSync->GetMatrix());
	pVehicle->SetMoveSpeed(pSync->GetMoveSpeed());
	pVehicle->m_vecTurnSpeed = pSync->GetTurnSpeed();
	pVehicle->m_vecMoveFriction = pSync->GetMoveFriction();
	pVehicle->m_vecTurnFriction = pSync->GetTurnFriction();

	pVehicle->m_fSteerAngle = pSync->m_fSteerAngle;
	pVehicle->m_fGasPedal = pSync->m_fGasPedal;
	pVehicle->m_fBrakePedal = pSync->m_fBrakePedal;
	pVehicle->bIsHandbrakeOn = pSync->m_bIsHandbrakeOn;
	pVehicle->m_nCurrentGear = pSync->m_nCurrentGear;
	pVehicle->m_fChangeGearTime = pSync->m_fChangeGearTime;
	pVehicle->m_vehLCS_258 = pSync->m_nCarJacker;
	pVehicle->m_nDriverIdMG = pSync->m_nDriverID;
	pVehicle->m_nDoorLock = pSync->m_nDoorLock;
	
	if (pSync->m_nStatus == eEntityStatus::STATUS_WRECKED) {
		pVehicle->bRenderScorched = true;
		pVehicle->SetStatus(pSync->m_nStatus); // eEntityStatus::STATUS_WRECKED
	}
	else if (pSync->m_nStatus == eEntityStatus::STATUS_SIMPLE)
		pVehicle->SetStatus(eEntityStatus::STATUS_SIMPLE);
	else if (pSync->m_nStatus == eEntityStatus::STATUS_PHYSICS)
		pVehicle->SetStatus(eEntityStatus::STATUS_PHYSICS);
	else
		pVehicle->SetStatus(eEntityStatus::STATUS_ABANDONED);

	if (pSync->m_nStatus != STATUS_WRECKED)
	{
		CCarCtrl::JoinCarWithRoadSystem(pVehicle);
		pVehicle->AutoPilot.m_aRouteNodes[ROUTE_NODE_NEXT].m_nPathNodeInfo = pSync->AutoPilot.m_nNextPathNodeInfo;
		pVehicle->AutoPilot.m_aRouteNodes[ROUTE_NODE_NEXT].m_nRouteNode = pSync->AutoPilot.m_nNextRouteNode;
		pVehicle->AutoPilot.m_aRouteNodes[ROUTE_NODE_NEXT].m_nDirection = pSync->AutoPilot.m_nNextDirection;
		pVehicle->AutoPilot.m_aRouteNodes[ROUTE_NODE_NEXT].m_nLane = pSync->AutoPilot.m_nNextLane;

		pVehicle->AutoPilot.m_aRouteNodes[ROUTE_NODE_CURRENT].m_nPathNodeInfo = pSync->AutoPilot.m_nCurrentPathNodeInfo;
		pVehicle->AutoPilot.m_aRouteNodes[ROUTE_NODE_CURRENT].m_nRouteNode = pSync->AutoPilot.m_nCurrentRouteNode;
		pVehicle->AutoPilot.m_aRouteNodes[ROUTE_NODE_CURRENT].m_nDirection = pSync->AutoPilot.m_nCurrentDirection;
		pVehicle->AutoPilot.m_aRouteNodes[ROUTE_NODE_CURRENT].m_nLane = pSync->AutoPilot.m_nCurrentLane;

		pVehicle->AutoPilot.m_aRouteNodes[ROUTE_NODE_PREV].m_nPathNodeInfo = pSync->AutoPilot.m_nPreviousPathNodeInfo;
		pVehicle->AutoPilot.m_aRouteNodes[ROUTE_NODE_PREV].m_nRouteNode = pSync->AutoPilot.m_nPrevRouteNode;
		pVehicle->AutoPilot.m_aRouteNodes[ROUTE_NODE_PREV].m_nDirection = pSync->AutoPilot.m_nPreviousDirection;
		pVehicle->AutoPilot.m_aRouteNodes[ROUTE_NODE_PREV].m_nLane = pSync->AutoPilot.m_nPrevLane;

		pVehicle->AutoPilot.m_aRouteNodes[ROUTE_NODE_NEXT_NEXT].m_nPathNodeInfo = pSync->AutoPilot.m_nNextNextPathNodeInfo;
		pVehicle->AutoPilot.m_aRouteNodes[ROUTE_NODE_NEXT_NEXT].m_nRouteNode = pSync->AutoPilot.m_nNextNextRouteNode;
		pVehicle->AutoPilot.m_aRouteNodes[ROUTE_NODE_NEXT_NEXT].m_nDirection = pSync->AutoPilot.m_nNextNextDirection;
		pVehicle->AutoPilot.m_aRouteNodes[ROUTE_NODE_NEXT_NEXT].m_nLane = pSync->AutoPilot.m_nNextNextLane;

		pVehicle->AutoPilot.m_nTimeToSpendOnCurrentCurve = pSync->AutoPilot.m_nTimeToSpendOnCurrentCurve;
		pVehicle->AutoPilot.m_nTimeEnteredCurve = pSync->AutoPilot.m_nTimeEnteredCurve;
		pVehicle->AutoPilot.m_nCarMission = pSync->AutoPilot.m_nCarMission;
		pVehicle->AutoPilot.m_nTempAction = eCarTempAction::TEMPACT_NONE;
		pVehicle->AutoPilot.m_nDrivingStyle = pSync->AutoPilot.m_nDrivingStyle;
		pVehicle->AutoPilot.m_nCruiseSpeed = pSync->AutoPilot.m_nCruiseSpeed;
		pVehicle->AutoPilot.m_fMaxTrafficSpeed = pSync->AutoPilot.m_nMaxTrafficSpeed;
		pVehicle->AutoPilot.UpdatePathSegment();
	}

	pVehicle->bEngineOn = false;
	//pVehicle->m_level = CTheZones::GetLevelFromPosition(&pSync->GetMatrix().GetPosition()); // need? FIX_BUGS
	pVehicle->m_nZoneLevel = CTheZones::GetLevelFromPosition(&pSync->GetMatrix().GetPosition());
	pVehicle->bHasBeenOwnedByPlayer = true;
	pVehicle->m_aColours[VEHICLE_COLOUR_PRIMARY] = m_aColours[VEHICLE_COLOUR_PRIMARY];
	pVehicle->m_aColours[VEHICLE_COLOUR_SECONDARY] = m_aColours[VEHICLE_COLOUR_SECONDARY];
	pVehicle->m_fHealth = pSync->m_fHealth;
	CWorld::Add(pVehicle);

	sElementPhysical::ReceiveEntity(nOwner, nID, nTime);
}

void sVehicle::UpdateDelta(sElementSync* pSync, uint16 nTimeDelta) {
	sElement::UpdateDelta(pSync, nTimeDelta);
}

bool sVehicle::IsTransferable(void) {
	return GetSync().vehicle->bIsDrivenByPlayer == false;
}

void sVehicle::TransferEntity(int16 nDestPlayer) {
	debug("Transfer Vehicle %d", GetID());
	CVehicle* pVeh = (CVehicle*)GetEntity();
	CPed* pDriver = pVeh->pDriver;
	if (pDriver != nil && pDriver->IsPlayer()) {
		if (pDriver->m_nPedState != PED_DRIVING) {
			debug("Car transfer prevented by driver state %d\n", pDriver->m_nPedState);
			return;
		}
		int16 nAnimID = pDriver->m_pVehicleAnim->animId;
		bool bHasSitAnim = nAnimID == ANIM_STD_CAR_SIT || nAnimID == ANIM_BIKE_RIDE || nAnimID == ANIM_STD_CAR_SIT_LO;
		if (!bHasSitAnim) {
			debug("Car transfer prevented by driver anim %d\n", nAnimID);
			return;
		}
	}
	sElement::TransferEntity(nDestPlayer);
	TransferOccupants(nDestPlayer);
}

void sVehicle::Initialise() {
	CVehicle* pVeh = (CVehicle*)GetEntity();
	m_pPhyElem->SetModelIndex(pVeh->GetModelIndex());
	m_pPhyElem->m_fAirResistance = pVeh->m_fAirResistance;
	m_pPhyElem->m_fMass = pVeh->m_fMass;
	m_pPhyElem->m_fTurnMass = pVeh->m_fTurnMass;
	m_pPhyElem->m_fForceMultiplier = pVeh->m_fForceMultiplier;
	m_pPhyElem->m_vecCentreOfMass = pVeh->m_vecCentreOfMass;
	m_pPhyElem->bUsesCollision = false;
	m_pPed = nil;
#ifdef GTA_LIBERTY
	m_currentColour1 = pVeh->m_currentColour1;
	m_currentColour2 = pVeh->m_currentColour2;
#else
	m_aColours[VEHICLE_COLOUR_PRIMARY] = pVeh->m_aColours[VEHICLE_COLOUR_PRIMARY];
	m_aColours[VEHICLE_COLOUR_SECONDARY] = pVeh->m_aColours[VEHICLE_COLOUR_SECONDARY];
#endif
	RegisterSelf();
}

bool sVehicle::TransferOccupants(int16 nDestPlayer) {
	debug("Transfer Occupants\n");
	CVehicle* pVeh = (CVehicle*)GetEntity();
	if (pVeh == nil)
		return true;
	CPed* pDriver = pVeh->pDriver;
	if (pDriver != nil && pDriver->IsPlayer()) {
		pDriver->SetPlayerToFollow(nDestPlayer);
#ifdef GTA_LIBERTY
		pDriver->m_pMyVehicle = nil;
		pDriver->bInVehicle = false;
		pVeh->pDriver = nil;
#else
		if (pDriver->InVehicle())
			pDriver->SetPedState(PedState::PED_IDLE);
		pDriver->SetInVehicle(false);
		pDriver->SetMyVehicle(nil);
		GetEntity()->SetStatus(eEntityStatus::STATUS_ABANDONED);
		pVeh->SetDriverPed(nil);
#endif
		return true;
	}
	sElement* pElem = cMultiGame::Instance().GetElementFromEntity(pDriver);
	if (pElem != nil && !pElem->WasTransfered())
		pElem->TransferEntity(nDestPlayer);
	return true;
}

void sVehicle::SetComponentVisibility(RwFrame* frame, uint32 flags)
{
	//HideAllComps(); // native
	//bIsDamaged = true; // native
	RwFrameForAllObjects(frame, SetVehicleAtomicVisibilityCB, RWVOIDRS(flags));
}

void sVehicle::CompareSyncState(sVehicleSync* pSync, sVehicleSync* pLastSync, uint32 nDelta, tVehicleSyncsDeltas* pDiff) {
	// Physical sync
	pDiff->nPhysicalDiff = sElementPhysical::CompareSyncState(pSync, pLastSync);
	if (pDiff->nPhysicalDiff != ePhysicalSync::MP_PKTD_PHY_EQUAL)
		pDiff->nVehicleDiff |= eVehicleSync::MP_PKTD_VEH_PHYSICAL;

	// AutoPilot sync
	pDiff->nAutoPilotDiff = CompareAutoPilotSyncState(&pSync->AutoPilot, &pLastSync->AutoPilot);
	if (pDiff->nAutoPilotDiff != eAutoPilotSync::MP_PKTD_VEH_AP_EQUAL)
		pDiff->nVehicleDiff |= eVehicleSync::MP_PKTD_VEH_AUTOPILOT;

	// Wheels
	for (uint32 i = 0; i < NUM_MULTIPLAYER_SYNC_WHEELS; i++) {
		pDiff->aWheelDiff[i] = CompareWheelSyncState(pSync, pLastSync, i);
		if (pDiff->aWheelDiff[i] != eWheelSync::MP_PKTD_WHEEL_EQUAL)
			pDiff->nVehicleDiff |= MP_PKTD_VEH_WHEEL_MASK(i);
	}

	// Skidmark
	if (pLastSync->m_nSkidmarkFlags != pSync->m_nSkidmarkFlags)
		pDiff->nVehicleDiff |= eVehicleSync::MP_PKTD_VEH_SKIDMARK_FLAGS;

	// Steer angle
	if (FLT_EPS_NOT_EQ(pLastSync->m_fSteerAngle, pSync->m_fSteerAngle))
		pDiff->nVehicleDiff |= eVehicleSync::MP_PKTD_VEH_STEER_ANGLE;
	else // !!---- UPD!!
		pSync->m_fSteerAngle = pLastSync->m_fSteerAngle;

	// Gas pedal
	if (FLT_EPS_NOT_EQ(pLastSync->m_fGasPedal, pSync->m_fGasPedal))
		pDiff->nVehicleDiff |= eVehicleSync::MP_PKTD_VEH_GAS_PEDAL;
	else // !!---- UPD!!
		pSync->m_fGasPedal = pLastSync->m_fGasPedal;

	// Brake pedal
	if (FLT_EPS_NOT_EQ(pLastSync->m_fBrakePedal, pSync->m_fBrakePedal))
		pDiff->nVehicleDiff |= eVehicleSync::MP_PKTD_VEH_BRAKE_PEDAL;
	else // !!---- UPD!!
		pSync->m_fBrakePedal = pLastSync->m_fBrakePedal;

	// Handbrake and horn
	if (pLastSync->m_bIsHandbrakeOn != pSync->m_bIsHandbrakeOn || pLastSync->m_nCarHornTimer != pSync->m_nCarHornTimer)
		pDiff->nVehicleDiff |= eVehicleSync::MP_PKTD_VEH_HANDBRAKE_HORN;

	// SirenOrAlarm
	if (pLastSync->m_bSirenOrAlarm != pSync->m_bSirenOrAlarm)
		pDiff->nVehicleDiff |= eVehicleSync::MP_PKTD_VEH_SIREN_ALARM;

	// Status
	if (pLastSync->m_nStatus != pSync->m_nStatus)
		pDiff->nVehicleDiff |= eVehicleSync::MP_PKTD_VEH_STATUS;

	// ChangeGearTime
	if (pLastSync->m_fChangeGearTime != pSync->m_fChangeGearTime)
		pDiff->nVehicleDiff |= eVehicleSync::MP_PKTD_VEH_CHANGE_GEAR_TIME;

	// Current gear and flags
	if (pLastSync->m_nCurrentGear != pSync->m_nCurrentGear ||
		pLastSync->bIsDrivenByPlayer != pSync->bIsDrivenByPlayer ||
		pLastSync->m_nCarJacker != pSync->m_nCarJacker ||
		pLastSync->bHasDriver != pSync->bHasDriver ||
#ifdef FIX_BUGS
		pLastSync->bIsDrowning != pSync->bIsDrowning ||
#endif
		pLastSync->bEngineOn != pSync->bEngineOn)
	{
		pDiff->nVehicleDiff |= eVehicleSync::MP_PKTD_VEH_CURRENT_GEAR_FLAGS;
	}

	// DriverID
	if (pLastSync->m_nDriverID != pSync->m_nDriverID)
		pDiff->nVehicleDiff |= eVehicleSync::MP_PKTD_VEH_DRIVER_ID;

	// Health
	if (pLastSync->m_fHealth != pSync->m_fHealth)
		pDiff->nVehicleDiff |= eVehicleSync::MP_PKTD_VEH_HEALTH;

	// PlayerID
	if (pLastSync->m_nPlayerID != pSync->m_nPlayerID)
		pDiff->nVehicleDiff |= eVehicleSync::MP_PKTD_VEH_PLAYER_ID;

#ifdef FIX_BUGS
	// Colours
	if (!CRGBA_CMP_24(pLastSync->m_aColours[VEHICLE_COLOUR_PRIMARY], pSync->m_aColours[VEHICLE_COLOUR_PRIMARY]) ||
		!CRGBA_CMP_24(pLastSync->m_aColours[VEHICLE_COLOUR_SECONDARY], pSync->m_aColours[VEHICLE_COLOUR_SECONDARY])) // + Physical ModelIndex
		pDiff->nVehicleDiff |= eVehicleSync::MP_PKTD_VEH_COLORS;
#endif

	// DriverTeam
	if (pLastSync->m_nDriverTeam != pSync->m_nDriverTeam)
		pDiff->nVehicleDiff |= eVehicleSync::MP_PKTD_VEH_DRIVER_TEAM;

	// LastDamage
#ifdef FIX_BUGS
	if (pLastSync->m_fLastDamageAmount != pSync->m_fLastDamageAmount || pLastSync->m_nLastDamagePlayerID != pSync->m_nLastDamagePlayerID)
#else
	if (pLastSync->m_fLastDamageAmount != pSync->m_fLastDamageAmount)
#endif
		pDiff->nVehicleDiff |= eVehicleSync::MP_PKTD_VEH_LAST_DAMAGE;

	// VehicleCreatedBy
	if (pLastSync->VehicleCreatedBy != pSync->VehicleCreatedBy)
		pDiff->nVehicleDiff |= eVehicleSync::MP_PKTD_VEH_CREATED_BY;

	// DoorLock
	if (pLastSync->m_nDoorLock != pSync->m_nDoorLock)
		pDiff->nVehicleDiff |= eVehicleSync::MP_PKTD_VEH_DOOR_LOCK;
}

void sVehicle::PerformWriteSync(sWriteSyncStream* pSyncStream, sVehicleSync* pSync, tVehicleSyncsDeltas* pDiff) {
	pSyncStream->WriteU32(pDiff->nVehicleDiff);

	// Physical sync
	if (pDiff->nVehicleDiff & eVehicleSync::MP_PKTD_VEH_PHYSICAL)
		sElementPhysical::PerformWriteSync(pSyncStream, pSync, pDiff->nPhysicalDiff);

	// AutoPilot sync
	if (pDiff->nVehicleDiff & eVehicleSync::MP_PKTD_VEH_AUTOPILOT)
		PerformWriteAutoPilotSync(pSyncStream, &pSync->AutoPilot, pDiff->nAutoPilotDiff);

	// Wheels
	for (uint32 i = 0; i < NUM_MULTIPLAYER_SYNC_WHEELS; i++) {
		if (pDiff->nVehicleDiff & MP_PKTD_VEH_WHEEL_MASK(i))
			PerformWriteWheelSync(pSyncStream, pSync, i, pDiff->aWheelDiff[i]);
	}

	// Skidmark
	if (pDiff->nVehicleDiff & eVehicleSync::MP_PKTD_VEH_SKIDMARK_FLAGS)
		pSyncStream->WriteU8(pSync->m_nSkidmarkFlags);

	// Steer angle
	if (pDiff->nVehicleDiff & eVehicleSync::MP_PKTD_VEH_STEER_ANGLE)
		pSyncStream->WriteFloat(pSync->m_fSteerAngle);

	// Gas pedal
	if (pDiff->nVehicleDiff & eVehicleSync::MP_PKTD_VEH_GAS_PEDAL)
		pSyncStream->WriteFloat(pSync->m_fGasPedal);

	// Brake pedal
	if (pDiff->nVehicleDiff & eVehicleSync::MP_PKTD_VEH_BRAKE_PEDAL)
		pSyncStream->WriteFloat(pSync->m_fBrakePedal);

	// Handbrake and horn (packed)
	if (pDiff->nVehicleDiff & eVehicleSync::MP_PKTD_VEH_HANDBRAKE_HORN)
	{
		uint8 packed = (pSync->m_bIsHandbrakeOn ? 0x80 : 0) | (pSync->m_nCarHornTimer & 0x7F);
		pSyncStream->WriteU8(packed);
	}

	// SirenOrAlarm
	if (pDiff->nVehicleDiff & eVehicleSync::MP_PKTD_VEH_SIREN_ALARM)
		pSyncStream->WriteBool(pSync->m_bSirenOrAlarm);

	// Status
	if (pDiff->nVehicleDiff & eVehicleSync::MP_PKTD_VEH_STATUS)
		pSyncStream->WriteU8(pSync->m_nStatus);

	// Change gear time
	if (pDiff->nVehicleDiff & eVehicleSync::MP_PKTD_VEH_CHANGE_GEAR_TIME)
		pSyncStream->WriteFloat(pSync->m_fChangeGearTime);

	// Current gear and flags
	if (pDiff->nVehicleDiff & eVehicleSync::MP_PKTD_VEH_CURRENT_GEAR_FLAGS)
	{
		uint16 packed = (pSync->m_nCurrentGear & 0x3F) | // 00111111
			(pSync->bHasDriver ? 0x40 : 0) |
			(pSync->bIsDrivenByPlayer ? 0x80 : 0) |
			(pSync->m_nCarJacker ? 0x100 : 0) |
			(pSync->bEngineOn ? 0x200 : 0) |
			(pSync->bIsDrowning ? 0x400 : 0);
		pSyncStream->WriteU16(packed);
	}

	// DriverID
	if (pDiff->nVehicleDiff & eVehicleSync::MP_PKTD_VEH_DRIVER_ID)
		pSyncStream->WriteI8(pSync->m_nDriverID);

	// Health
	if (pDiff->nVehicleDiff & eVehicleSync::MP_PKTD_VEH_HEALTH)
		pSyncStream->WriteFloat(pSync->m_fHealth);

	// PlayerID
	if (pDiff->nVehicleDiff & eVehicleSync::MP_PKTD_VEH_PLAYER_ID)
		pSyncStream->WriteI8(pSync->m_nPlayerID);

	// Colours // m_aColours not exists in sync
	if (pDiff->nVehicleDiff & eVehicleSync::MP_PKTD_VEH_COLORS)
	{
		// This writes vehicle model and colors from this, not from pSync!
		assert(GetPhysical());
		pSyncStream->WriteI16(GetPhysical()->GetModelIndex());
		pSyncStream->WriteColour24(m_aColours[VEHICLE_COLOUR_PRIMARY]);
		pSyncStream->WriteColour24(m_aColours[VEHICLE_COLOUR_SECONDARY]);
	}

	// DriverTeam
	if (pDiff->nVehicleDiff & eVehicleSync::MP_PKTD_VEH_DRIVER_TEAM)
		pSyncStream->WriteI8(pSync->m_nDriverTeam);

	// LastDamage
	if (pDiff->nVehicleDiff & eVehicleSync::MP_PKTD_VEH_LAST_DAMAGE)
	{
		pSyncStream->WriteFloat(pSync->m_fLastDamageAmount);
		pSyncStream->WriteU8(pSync->m_nLastDamagePlayerID);
	}

	// VehicleCreatedBy
	if (pDiff->nVehicleDiff & eVehicleSync::MP_PKTD_VEH_CREATED_BY)
		pSyncStream->WriteU8(pSync->VehicleCreatedBy);

	// DoorLock
	if (pDiff->nVehicleDiff & eVehicleSync::MP_PKTD_VEH_DOOR_LOCK)
		pSyncStream->WriteU32(pSync->m_nDoorLock);
}

void sVehicle::ReadSyncFromStreamVehicle(sReadSyncStream* pSyncStream, sVehicleSync* pOutSync) {
	uint32 nDiffMask = pSyncStream->ReadU32();

	// Physical sync
	if (nDiffMask & eVehicleSync::MP_PKTD_VEH_PHYSICAL)
		sElementPhysical::ReadSyncFromStreamPhysical(pSyncStream, pOutSync);

	// AutoPilot sync
	if (nDiffMask & eVehicleSync::MP_PKTD_VEH_AUTOPILOT)
		ReadAutoPilotSyncFromStream(pSyncStream, &pOutSync->AutoPilot);

	// Wheels
	for (uint32 i = 0; i < NUM_MULTIPLAYER_SYNC_WHEELS; i++) {
		if (nDiffMask & MP_PKTD_VEH_WHEEL_MASK(i))
			ReadWheelSyncFromStream(pSyncStream, pOutSync, i);
	}

	// Skidmark
	if (nDiffMask & eVehicleSync::MP_PKTD_VEH_SKIDMARK_FLAGS)
		pOutSync->m_nSkidmarkFlags = pSyncStream->ReadU8();

	// Steer angle
	if (nDiffMask & eVehicleSync::MP_PKTD_VEH_STEER_ANGLE)
		pOutSync->m_fSteerAngle = pSyncStream->ReadFloat();

	// Gas pedal
	if (nDiffMask & eVehicleSync::MP_PKTD_VEH_GAS_PEDAL)
		pOutSync->m_fGasPedal = pSyncStream->ReadFloat();

	// Brake pedal
	if (nDiffMask & eVehicleSync::MP_PKTD_VEH_BRAKE_PEDAL)
		pOutSync->m_fBrakePedal = pSyncStream->ReadFloat();

	// Handbrake and horn
	if (nDiffMask & eVehicleSync::MP_PKTD_VEH_HANDBRAKE_HORN)
	{
		uint8 packed = pSyncStream->ReadU8();
		pOutSync->m_bIsHandbrakeOn = (packed & 0x80) != 0;
		pOutSync->m_nCarHornTimer = packed & 0x7F;
	}

	// SirenOrAlarm
	if (nDiffMask & eVehicleSync::MP_PKTD_VEH_SIREN_ALARM)
		pOutSync->m_bSirenOrAlarm = pSyncStream->ReadBool();

	// Status
	if (nDiffMask & eVehicleSync::MP_PKTD_VEH_STATUS)
		pOutSync->m_nStatus = pSyncStream->ReadU8();

	// Change gear time
	if (nDiffMask & eVehicleSync::MP_PKTD_VEH_CHANGE_GEAR_TIME)
		pOutSync->m_fChangeGearTime = pSyncStream->ReadFloat();

	// Current gear and flags
	if (nDiffMask & eVehicleSync::MP_PKTD_VEH_CURRENT_GEAR_FLAGS)
	{
		uint16 packed = pSyncStream->ReadU16();
		pOutSync->m_nCurrentGear = packed & 0x3F;
		pOutSync->bHasDriver = (packed & 0x40) ? 1 : 0;
		pOutSync->bIsDrivenByPlayer = (packed & 0x80) ? 1 : 0;
		pOutSync->m_nCarJacker = (packed & 0x100) ? 1 : 0;
		pOutSync->bEngineOn = (packed & 0x200) ? 1 : 0;
		pOutSync->bIsDrowning = (packed & 0x400) ? 1 : 0;
	}

	// DriverID
	if (nDiffMask & eVehicleSync::MP_PKTD_VEH_DRIVER_ID)
	{
		pOutSync->m_nDriverID = pSyncStream->ReadI8();
		if (pOutSync->m_nDriverID >= 0)
			pOutSync->m_nLastDriverID = pOutSync->m_nDriverID;
	}

	// Health
	if (nDiffMask & eVehicleSync::MP_PKTD_VEH_HEALTH)
		pOutSync->m_fHealth = pSyncStream->ReadFloat();

	// PlayerID
	if (nDiffMask & eVehicleSync::MP_PKTD_VEH_PLAYER_ID)
		pOutSync->m_nPlayerID = pSyncStream->ReadI8();

	// Colours // m_aColours not exists in sync
	if (nDiffMask & eVehicleSync::MP_PKTD_VEH_COLORS)
	{
		int16 nModelIndex = pSyncStream->ReadI16();
		assert(GetPhysical());
		//assert(CStreaming::HasModelLoaded(nModelIndex)); // load in cVehicleMG::SetModelIndex
		GetPhysical()->SetModelIndex(nModelIndex); // set u32, but get i16
		pSyncStream->ReadColour24(m_aColours[VEHICLE_COLOUR_PRIMARY]);
		pSyncStream->ReadColour24(m_aColours[VEHICLE_COLOUR_SECONDARY]);
		// what about default alpha // VCS still 0, vehicle didn't use alpha (CVehicleModelInfo::SetVehicleColour)
#ifdef FIX_BUGS // its fine
		m_aColours[VEHICLE_COLOUR_PRIMARY].alpha = 255;
		m_aColours[VEHICLE_COLOUR_SECONDARY].alpha = 255;
#endif
	}

	// DriverTeam
	if (nDiffMask & eVehicleSync::MP_PKTD_VEH_DRIVER_TEAM)
		pOutSync->m_nDriverTeam = pSyncStream->ReadI8();

	// LastDamage
	if (nDiffMask & eVehicleSync::MP_PKTD_VEH_LAST_DAMAGE)
	{
		pOutSync->m_fLastDamageAmount = pSyncStream->ReadFloat();
		pOutSync->m_nLastDamagePlayerID = pSyncStream->ReadU8();
	}

	// VehicleCreatedBy
	if (nDiffMask & eVehicleSync::MP_PKTD_VEH_CREATED_BY)
		pOutSync->VehicleCreatedBy = pSyncStream->ReadU8();

	// DoorLock
	if (nDiffMask & eVehicleSync::MP_PKTD_VEH_DOOR_LOCK)
		pOutSync->m_nDoorLock = (eCarLock)pSyncStream->ReadU32();
}

uint8 sVehicle::CompareAutoPilotSyncState(sAutoPilotSync* pSync, sAutoPilotSync* pLastSync) {
	uint8 nDiffMask = eAutoPilotSync::MP_PKTD_VEH_AP_EQUAL;

#if defined(FIX_BUGS) && !defined(GTA_LIBERTY) // forgot upd
	if (pSync->m_nNextDirection != pLastSync->m_nNextDirection
		|| pSync->m_nCurrentDirection != pLastSync->m_nCurrentDirection
		|| pSync->m_nPreviousDirection != pLastSync->m_nPreviousDirection // vcs
		|| pSync->m_nNextNextDirection != pLastSync->m_nNextNextDirection // vcs
		|| pSync->m_nNextPathNodeInfo != pLastSync->m_nNextPathNodeInfo
		|| pSync->m_nCurrentPathNodeInfo != pLastSync->m_nCurrentPathNodeInfo
		|| pSync->m_nPreviousPathNodeInfo != pLastSync->m_nPreviousPathNodeInfo // vcs
		|| pSync->m_nNextNextPathNodeInfo != pLastSync->m_nNextNextPathNodeInfo) // vcs
		nDiffMask |= eAutoPilotSync::MP_PKTD_VEH_AP_PATH_NODES;
#else
	if (pSync->m_nNextDirection != pLastSync->m_nNextDirection
		|| pSync->m_nCurrentDirection != pLastSync->m_nCurrentDirection
		|| pSync->m_nNextPathNodeInfo != pLastSync->m_nNextPathNodeInfo
		|| pSync->m_nCurrentPathNodeInfo != pLastSync->m_nCurrentPathNodeInfo)
		nDiffMask |= eAutoPilotSync::MP_PKTD_VEH_AP_PATH_NODES;
#endif

	if (pSync->m_nTimeToSpendOnCurrentCurve != pLastSync->m_nTimeToSpendOnCurrentCurve
		|| pSync->m_nTimeEnteredCurve != pLastSync->m_nTimeEnteredCurve)
		nDiffMask |= eAutoPilotSync::MP_PKTD_VEH_AP_TIME_CURVE;

	if (pSync->m_nNextRouteNode != pLastSync->m_nNextRouteNode
		|| pSync->m_nCurrentRouteNode != pLastSync->m_nCurrentRouteNode)
		nDiffMask |= eAutoPilotSync::MP_PKTD_VEH_AP_ROUTE_NODES;

	if (pSync->m_nNextLane != pLastSync->m_nNextLane || pSync->m_nCurrentLane != pLastSync->m_nCurrentLane)
		nDiffMask |= eAutoPilotSync::MP_PKTD_VEH_AP_LANES;

	if (pSync->m_nCruiseSpeed != pLastSync->m_nCruiseSpeed || pSync->m_nMaxTrafficSpeed != pLastSync->m_nMaxTrafficSpeed)
		nDiffMask |= eAutoPilotSync::MP_PKTD_VEH_AP_SPEEDS;

	if (pSync->m_nCarMission != pLastSync->m_nCarMission)
		nDiffMask |= eAutoPilotSync::MP_PKTD_VEH_AP_CAR_MISSION;

	if (pSync->m_nDrivingStyle != pLastSync->m_nDrivingStyle)
		nDiffMask |= eAutoPilotSync::MP_PKTD_VEH_AP_DRIVING_STYLE;

	return nDiffMask;
}

void sVehicle::PerformWriteAutoPilotSync(sWriteSyncStream* pSyncStream, sAutoPilotSync* pSync, uint8 nDiffMask) {
	pSyncStream->WriteU8(nDiffMask);

	if (nDiffMask & eAutoPilotSync::MP_PKTD_VEH_AP_PATH_NODES)
	{
		pSyncStream->WriteU32(pSync->m_nNextPathNodeInfo);
		pSyncStream->WriteU32(pSync->m_nCurrentPathNodeInfo);
#ifndef GTA_LIBERTY
		pSyncStream->WriteU32(pSync->m_nNextNextPathNodeInfo);
		pSyncStream->WriteU32(pSync->m_nPreviousPathNodeInfo);
#endif
		pSyncStream->WriteI8(pSync->m_nNextDirection);
		pSyncStream->WriteI8(pSync->m_nCurrentDirection);
#ifndef GTA_LIBERTY
		pSyncStream->WriteI8(pSync->m_nNextNextDirection);
		pSyncStream->WriteI8(pSync->m_nPreviousDirection);
#endif
	}

	if (nDiffMask & eAutoPilotSync::MP_PKTD_VEH_AP_TIME_CURVE)
	{
		pSyncStream->WriteI32(pSync->m_nTimeToSpendOnCurrentCurve);

		// Write relative time (current time - CTimer)
		int32 relativeTime = pSync->m_nTimeEnteredCurve - CTimer::GetTimeInMilliseconds();
		pSyncStream->WriteI32(relativeTime);
	}

	if (nDiffMask & eAutoPilotSync::MP_PKTD_VEH_AP_ROUTE_NODES)
	{
		pSyncStream->WriteI32(pSync->m_nNextRouteNode);
		pSyncStream->WriteI32(pSync->m_nCurrentRouteNode);
#ifndef GTA_LIBERTY
		pSyncStream->WriteI32(pSync->m_nNextNextRouteNode);
		pSyncStream->WriteI32(pSync->m_nPrevRouteNode);
#endif
	}

	if (nDiffMask & eAutoPilotSync::MP_PKTD_VEH_AP_LANES)
	{
		pSyncStream->WriteI8(pSync->m_nNextLane);
		pSyncStream->WriteI8(pSync->m_nCurrentLane);
#ifndef GTA_LIBERTY
		pSyncStream->WriteI8(pSync->m_nNextNextLane);
		pSyncStream->WriteI8(pSync->m_nPrevLane);
#endif
	}

	if (nDiffMask & eAutoPilotSync::MP_PKTD_VEH_AP_SPEEDS)
	{
		pSyncStream->WriteU8(pSync->m_nCruiseSpeed);
		pSyncStream->WriteU8(pSync->m_nMaxTrafficSpeed);
	}

	if (nDiffMask & eAutoPilotSync::MP_PKTD_VEH_AP_CAR_MISSION)
		pSyncStream->WriteU8(pSync->m_nCarMission);

	if (nDiffMask & eAutoPilotSync::MP_PKTD_VEH_AP_DRIVING_STYLE)
		pSyncStream->WriteU8(pSync->m_nDrivingStyle);
}

void sVehicle::ReadAutoPilotSyncFromStream(sReadSyncStream* pSyncStream, sAutoPilotSync* pOutSync) {
	uint8 nDiffMask = pSyncStream->ReadU8();

	if (nDiffMask & eAutoPilotSync::MP_PKTD_VEH_AP_PATH_NODES)
	{
		pOutSync->m_nNextPathNodeInfo = pSyncStream->ReadU32();
		pOutSync->m_nCurrentPathNodeInfo = pSyncStream->ReadU32();
#ifndef GTA_LIBERTY
		pOutSync->m_nNextNextPathNodeInfo = pSyncStream->ReadU32();
		pOutSync->m_nPreviousPathNodeInfo = pSyncStream->ReadU32();
#endif
		pOutSync->m_nNextDirection = pSyncStream->ReadI8();
		pOutSync->m_nCurrentDirection = pSyncStream->ReadI8();
#ifndef GTA_LIBERTY
		pOutSync->m_nNextNextDirection = pSyncStream->ReadI8();
		pOutSync->m_nPreviousDirection = pSyncStream->ReadI8();
#endif
	}

	if (nDiffMask & eAutoPilotSync::MP_PKTD_VEH_AP_TIME_CURVE)
	{
		pOutSync->m_nTimeToSpendOnCurrentCurve = pSyncStream->ReadI32();
		int32 relativeTime = pSyncStream->ReadI32();
		pOutSync->m_nTimeEnteredCurve = relativeTime + CTimer::GetTimeInMilliseconds();
	}

	if (nDiffMask & eAutoPilotSync::MP_PKTD_VEH_AP_ROUTE_NODES)
	{
		pOutSync->m_nNextRouteNode = pSyncStream->ReadI32();
		pOutSync->m_nCurrentRouteNode = pSyncStream->ReadI32();
#ifndef GTA_LIBERTY
		pOutSync->m_nNextNextRouteNode = pSyncStream->ReadI32();
		pOutSync->m_nPrevRouteNode = pSyncStream->ReadI32();
#endif
	}

	if (nDiffMask & eAutoPilotSync::MP_PKTD_VEH_AP_LANES)
	{
		pOutSync->m_nNextLane = pSyncStream->ReadI8();
		pOutSync->m_nCurrentLane = pSyncStream->ReadI8();
#ifndef GTA_LIBERTY
		pOutSync->m_nNextNextLane = pSyncStream->ReadI8();
		pOutSync->m_nPrevLane = pSyncStream->ReadI8();
#endif
	}

	if (nDiffMask & eAutoPilotSync::MP_PKTD_VEH_AP_SPEEDS)
	{
		pOutSync->m_nCruiseSpeed = pSyncStream->ReadU8();
		pOutSync->m_nMaxTrafficSpeed = pSyncStream->ReadU8();
	}

	if (nDiffMask & eAutoPilotSync::MP_PKTD_VEH_AP_CAR_MISSION)
		pOutSync->m_nCarMission = pSyncStream->ReadU8();

	if (nDiffMask & eAutoPilotSync::MP_PKTD_VEH_AP_DRIVING_STYLE)
		pOutSync->m_nDrivingStyle = pSyncStream->ReadU8();
}

uint32 sVehicle::CompareWheelSyncState(sVehicleSync* pSync, sVehicleSync* pLastSync, uint32 nWheel) {
	const float POSITION_EPSILON = 0.001f;

	uint32 nDiffMask = eWheelSync::MP_PKTD_WHEEL_EQUAL;

	if (GetType() == eElementType::ELEMENT_TYPE_BOAT)
		return eWheelSync::MP_PKTD_WHEEL_EQUAL;

	// Wheel timer
	if (pLastSync->m_aWheelTimer[nWheel] != pSync->m_aWheelTimer[nWheel])
		nDiffMask |= eWheelSync::MP_PKTD_WHEEL_TIMER;

	// Wheel state
	if (pLastSync->m_aWheelState[nWheel] != pSync->m_aWheelState[nWheel])
		nDiffMask |= eWheelSync::MP_PKTD_WHEEL_STATE;

	// Wheel rotation
	if (FLT_EPS_NOT_EQ(pLastSync->m_aWheelRotation[nWheel], pSync->m_aWheelRotation[nWheel]))
		nDiffMask |= eWheelSync::MP_PKTD_WHEEL_ROTATION;
	else // !!---- UPD!!
		pSync->m_aWheelRotation[nWheel] = pLastSync->m_aWheelRotation[nWheel];

	// Wheel position
	if (fabsf(pLastSync->m_aWheelPosition[nWheel] - pSync->m_aWheelPosition[nWheel]) > POSITION_EPSILON)
		nDiffMask |= eWheelSync::MP_PKTD_WHEEL_POSITION;
	else // !!---- UPD!!
		pSync->m_aWheelPosition[nWheel] = pLastSync->m_aWheelPosition[nWheel];

	// Wheel speed
	if (FLT_EPS_NOT_EQ(pLastSync->m_aWheelSpeed[nWheel], pSync->m_aWheelSpeed[nWheel]))
		nDiffMask |= eWheelSync::MP_PKTD_WHEEL_SPEED;
	else // !!---- UPD!!
		pSync->m_aWheelSpeed[nWheel] = pLastSync->m_aWheelSpeed[nWheel];

	// Suspension spring ratio
	if (fabsf(pLastSync->m_aSuspensionSpringRatio[nWheel] - pSync->m_aSuspensionSpringRatio[nWheel]) > POSITION_EPSILON)
		nDiffMask |= eWheelSync::MP_PKTD_WHEEL_SUSPENSION;
	else // !!---- UPD!!
		pSync->m_aSuspensionSpringRatio[nWheel] = pLastSync->m_aSuspensionSpringRatio[nWheel];


	if (!GetEntity() || !((CVehicle*)GetEntity())->IsDriver(FindPlayerPed()))
		return nDiffMask;

	// Skidmark type
	if (pLastSync->m_aWheelSkidmarkType[nWheel] != pSync->m_aWheelSkidmarkType[nWheel])
		nDiffMask |= eWheelSync::MP_PKTD_WHEEL_SKIDMARK;

	// CColPoint
	CColPoint* pLastPoint = &pLastSync->m_aWheelColPoints[nWheel];
	CColPoint* pPoint = &pSync->m_aWheelColPoints[nWheel];

	// Point X
	if (fabsf(pLastPoint->point.x - pPoint->point.x) > POSITION_EPSILON)
		nDiffMask |= eWheelSync::MP_PKTD_WHEEL_POINT_X;
	else // !!---- UPD!!
		pPoint->point.x = pLastPoint->point.x;

	// Point Y
	if (fabsf(pLastPoint->point.y - pPoint->point.y) > POSITION_EPSILON)
		nDiffMask |= eWheelSync::MP_PKTD_WHEEL_POINT_Y;
	else // !!---- UPD!!
		pPoint->point.y = pLastPoint->point.y;

	// Point Z
	if (fabsf(pLastPoint->point.z - pPoint->point.z) > POSITION_EPSILON)
		nDiffMask |= eWheelSync::MP_PKTD_WHEEL_POINT_Z;
	else // !!---- UPD!!
		pPoint->point.z = pLastPoint->point.z;

	// Normal X
	if (fabsf(pLastPoint->normal.x - pPoint->normal.x) > POSITION_EPSILON)
		nDiffMask |= eWheelSync::MP_PKTD_WHEEL_NORMAL_X;
	else // !!---- UPD!!
		pPoint->normal.x = pLastPoint->normal.x;

	// Normal Y
	if (fabsf(pLastPoint->normal.y - pPoint->normal.y) > POSITION_EPSILON)
		nDiffMask |= eWheelSync::MP_PKTD_WHEEL_NORMAL_Y;
	else // !!---- UPD!!
		pPoint->normal.y = pLastPoint->normal.y;

	// Normal Z
	if (fabsf(pLastPoint->normal.z - pPoint->normal.z) > POSITION_EPSILON)
		nDiffMask |= eWheelSync::MP_PKTD_WHEEL_NORMAL_Z;
	else // !!---- UPD!!
		pPoint->normal.z = pLastPoint->normal.z;

	// Point W
	if (fabsf(pLastPoint->point.w - pPoint->point.w) > POSITION_EPSILON)
		nDiffMask |= eWheelSync::MP_PKTD_WHEEL_POINT_W;
	else // !!---- UPD!!
		pPoint->point.w = pLastPoint->point.w;

#ifdef FIX_BUGS
	// Surface A
	if (pLastPoint->surfaceA != pPoint->surfaceA)
		nDiffMask |= eWheelSync::MP_PKTD_WHEEL_SURFACE_A;

	// Piece A
	if (pLastPoint->pieceA != pPoint->pieceA)
		nDiffMask |= eWheelSync::MP_PKTD_WHEEL_PIECE_A;

	// Surface B
	if (pLastPoint->surfaceB != pPoint->surfaceB)
		nDiffMask |= eWheelSync::MP_PKTD_WHEEL_SURFACE_B;

	// Piece B
	if (pLastPoint->pieceB != pPoint->pieceB)
		nDiffMask |= eWheelSync::MP_PKTD_WHEEL_PIECE_B;
#else // what??
	// Surface A
	if (pLastPoint->surfaceA != pPoint->surfaceA)
		nDiffMask |= eWheelSync::MP_PKTD_WHEEL_SURFACE_A;
	else // !!---- UPD!!
		pPoint->pieceA = pLastPoint->pieceA; // pastebug? surfaceA + useless

	// Piece A
	if (pLastPoint->pieceA != pPoint->pieceA)
		nDiffMask |= eWheelSync::MP_PKTD_WHEEL_PIECE_A;
	else // !!---- UPD!!
		pPoint->pieceA = pLastPoint->pieceA; // useless

	// Surface B
	if (pLastPoint->surfaceB != pPoint->surfaceB)
		nDiffMask |= eWheelSync::MP_PKTD_WHEEL_SURFACE_B;
	else // !!---- UPD!!
		pPoint->pieceB = pLastPoint->pieceB; // pastebug? surfaceB + useless

	// Piece B
	if (pLastPoint->pieceB != pPoint->pieceB)
		nDiffMask |= eWheelSync::MP_PKTD_WHEEL_PIECE_B;
	else // !!---- UPD!!
		pPoint->pieceB = pLastPoint->pieceB; // useless
#endif

	return nDiffMask;
}

void sVehicle::PerformWriteWheelSync(sWriteSyncStream* pSyncStream, sVehicleSync* pSync, uint32 nWheel, uint32 nDiffMask) {
	if (GetType() == eElementType::ELEMENT_TYPE_BOAT)
		return;

	pSyncStream->WriteU32(nDiffMask);

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_TIMER)
		pSyncStream->WriteU8(pSync->m_aWheelTimer[nWheel]);

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_STATE)
		pSyncStream->WriteU8((uint8)pSync->m_aWheelState[nWheel]);

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_ROTATION)
		pSyncStream->WriteFloat(pSync->m_aWheelRotation[nWheel]);

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_POSITION)
		pSyncStream->WriteFloat(pSync->m_aWheelPosition[nWheel]);

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_SPEED)
		pSyncStream->WriteFloat(pSync->m_aWheelSpeed[nWheel]);

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_SUSPENSION)
		pSyncStream->WriteFloat(pSync->m_aSuspensionSpringRatio[nWheel]);

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_SKIDMARK)
		pSyncStream->WriteU8((uint8)pSync->m_aWheelSkidmarkType[nWheel]);

	CColPoint* pPoint = &pSync->m_aWheelColPoints[nWheel];

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_POINT_X)
		pSyncStream->WriteFloat(pPoint->point.x);

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_POINT_Y)
		pSyncStream->WriteFloat(pPoint->point.y);

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_POINT_Z)
		pSyncStream->WriteFloat(pPoint->point.z);

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_NORMAL_X)
		pSyncStream->WriteFloat(pPoint->normal.x);

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_NORMAL_Y)
		pSyncStream->WriteFloat(pPoint->normal.y);

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_NORMAL_Z)
		pSyncStream->WriteFloat(pPoint->normal.z);

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_POINT_W) // ?
		pSyncStream->WriteFloat(pPoint->point.w);

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_SURFACE_A)
		pSyncStream->WriteU8(pPoint->surfaceA);

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_PIECE_A)
		pSyncStream->WriteU8(pPoint->pieceA);

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_SURFACE_B)
		pSyncStream->WriteU8(pPoint->surfaceB);

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_PIECE_B)
		pSyncStream->WriteU8(pPoint->pieceB);
}

void sVehicle::ReadWheelSyncFromStream(sReadSyncStream* pSyncStream, sVehicleSync* pOutSync, uint32 nWheel) {
	if (GetType() == eElementType::ELEMENT_TYPE_BOAT)
		return;

	uint32 nDiffMask = pSyncStream->ReadU32();

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_TIMER)
		pOutSync->m_aWheelTimer[nWheel] = pSyncStream->ReadU8();

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_STATE)
		pOutSync->m_aWheelState[nWheel] = (tWheelState)pSyncStream->ReadU8();

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_ROTATION)
		pOutSync->m_aWheelRotation[nWheel] = pSyncStream->ReadFloat();

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_POSITION)
		pOutSync->m_aWheelPosition[nWheel] = pSyncStream->ReadFloat();

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_SPEED)
		pOutSync->m_aWheelSpeed[nWheel] = pSyncStream->ReadFloat();

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_SUSPENSION)
		pOutSync->m_aSuspensionSpringRatio[nWheel] = pSyncStream->ReadFloat();

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_SKIDMARK)
		pOutSync->m_aWheelSkidmarkType[nWheel] = (eSkidmarkType)pSyncStream->ReadU8();

	CColPoint* pPoint = &pOutSync->m_aWheelColPoints[nWheel];

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_POINT_X)
		pPoint->point.x = pSyncStream->ReadFloat();

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_POINT_Y)
		pPoint->point.y = pSyncStream->ReadFloat();

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_POINT_Z)
		pPoint->point.z = pSyncStream->ReadFloat();

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_NORMAL_X)
		pPoint->normal.x = pSyncStream->ReadFloat();

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_NORMAL_Y)
		pPoint->normal.y = pSyncStream->ReadFloat();

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_NORMAL_Z)
		pPoint->normal.z = pSyncStream->ReadFloat();

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_POINT_W) // ?
		pPoint->point.w = pSyncStream->ReadFloat();

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_SURFACE_A)
		pPoint->surfaceA = (eSurfaceType)pSyncStream->ReadU8();

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_PIECE_A)
		pPoint->pieceA = pSyncStream->ReadU8();

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_SURFACE_B)
		pPoint->surfaceB = (eSurfaceType)pSyncStream->ReadU8();

	if (nDiffMask & eWheelSync::MP_PKTD_WHEEL_PIECE_B)
		pPoint->pieceB = pSyncStream->ReadU8();
}

// TODO: LCS compat types + TODO new types forgotten by leeds + fixbugs new functions for new types
// Audio
enum
{
	CAR_HELI_MAX_DIST = 250,
	CAR_HELI_ENGINE_MAX_DIST = 140,
	CAR_HELI_ENGINE_START_MAX_DIST = 30,
	CAR_HELI_ENGINE_START_VOLUME = 70,
	CAR_HELI_SEAPLANE_MAX_DIST = 20,
	CAR_HELI_SEAPLANE_VOLUME = 100,
	CAR_HELI_REAR_MAX_DIST = 27,
	CAR_HELI_REAR_VOLUME = 25,

	FORKLIFT_FORKS_MAX_DIST = 60,
	BICYCLE_MAX_DIST = 50,

	FLAT_TYRE_MAX_DIST = 60,
	FLAT_TYRE_VOLUME = 100,


	RAIN_ON_VEHICLE_MAX_DIST = 22,
	RAIN_ON_VEHICLE_VOLUME = 30,

	REVERSE_GEAR_MAX_DIST = 30,
	REVERSE_GEAR_VOLUME = 24,

	MODEL_CAR_ENGINE_MAX_DIST = 35,
	MODEL_CAR_ENGINE_VOLUME = 90,
	MODEL_HELI_ENGINE_VOLUME = 70,

	VEHICLE_ROAD_NOISE_MAX_DIST = 95,
	VEHICLE_ROAD_NOISE_VOLUME = 30,

	WET_ROAD_NOISE_MAX_DIST = 30,
	WET_ROAD_NOISE_VOLUME = 23,

	VEHICLE_ENGINE_MAX_DIST = 50,
	VEHICLE_ENGINE_BASE_VOLUME = 75,
	VEHICLE_ENGINE_FULL_VOLUME = 90,

	CESNA_IDLE_MAX_DIST = 200,
	CESNA_REV_MAX_DIST = 90,
	CESNA_VOLUME = 80,

	PLAYER_VEHICLE_ENGINE_VOLUME = 120,

	VEHICLE_SKIDDING_MAX_DIST = 40,
	VEHICLE_SKIDDING_VOLUME = 50,

	VEHICLE_HORN_MAX_DIST = 40,
	VEHICLE_HORN_VOLUME = 80,

	VEHICLE_SIREN_MAX_DIST = 110,
	VEHICLE_SIREN_VOLUME = 80,

	VEHICLE_REVERSE_WARNING_MAX_DIST = 50,
	VEHICLE_REVERSE_WARNING_VOLUME = 60,

	VEHICLE_DOORS_MAX_DIST = 40,
	VEHICLE_DOORS_VOLUME = 100,

	AIR_BRAKES_MAX_DIST = 30,
	AIR_BRAKES_VOLUME = 70,

	ENGINE_DAMAGE_MAX_DIST = 40,
	ENGINE_DAMAGE_VOLUME = 6,
	ENGINE_DAMAGE_ON_FIRE_VOLUME = 60,

	CAR_BOMB_TICK_MAX_DIST = 40,
	CAR_BOMB_TICK_VOLUME = 60,

	VEHICLE_ONE_SHOT_HELI_BLADE_MAX_DIST = 35,
	VEHICLE_ONE_SHOT_HELI_BLADE_VOLUME = 70,

	VEHICLE_ONE_SHOT_CAR_TYRE_POP_MAX_DIST = 60,
	VEHICLE_ONE_SHOT_CAR_TYRE_POP_VOLUME = 117,

	VEHICLE_ONE_SHOT_DOOR_MAX_DIST = 50,
	VEHICLE_ONE_SHOT_DOOR_OPEN_VOLUME = 122,
	VEHICLE_ONE_SHOT_DOOR_CLOSE_VOLUME = 117,

	VEHICLE_ONE_SHOT_WINDSHIELD_CRACK_MAX_DIST = 40,
	VEHICLE_ONE_SHOT_WINDSHIELD_CRACK_VOLUME = 80,

	VEHICLE_ONE_SHOT_CAR_JUMP_MAX_DIST = 35,
	VEHICLE_ONE_SHOT_CAR_JUMP_VOLUME = 80,

	VEHICLE_ONE_SHOT_CAR_ENGINE_START_MAX_DIST = 40,
	VEHICLE_ONE_SHOT_CAR_ENGINE_START_VOLUME = 60,

	VEHICLE_ONE_SHOT_CAR_LIGHT_BREAK_VOLUME = 30,

	VEHICLE_ONE_SHOT_CAR_HYDRAULIC_MAX_DIST = 35,
	VEHICLE_ONE_SHOT_CAR_HYDRAULIC_VOLUME = 55,

	VEHICLE_ONE_SHOT_CAR_SPLASH_MAX_DIST = 60,
	VEHICLE_ONE_SHOT_CAR_SPLASH_VOLUME = 35,

	VEHICLE_ONE_SHOT_BOAT_SLOWDOWN_MAX_DIST = 50,

	VEHICLE_ONE_SHOT_TRAIN_DOOR_MAX_DIST = 35,
	VEHICLE_ONE_SHOT_TRAIN_DOOR_VOLUME = 70,

	VEHICLE_ONE_SHOT_CAR_TANK_TURRET_MAX_DIST = 40,
	VEHICLE_ONE_SHOT_CAR_TANK_TURRET_VOLUME = 90,

	VEHICLE_ONE_SHOT_CAR_BOMB_TICK_MAX_DIST = 30,
	VEHICLE_ONE_SHOT_CAR_BOMB_TICK_VOLUME = CAR_BOMB_TICK_VOLUME,

	VEHICLE_ONE_SHOT_PLANE_ON_GROUND_MAX_DIST = 180,
	VEHICLE_ONE_SHOT_PLANE_ON_GROUND_VOLUME = 75,

	VEHICLE_ONE_SHOT_WEAPON_SHOT_FIRED_MAX_DIST = 120,
	VEHICLE_ONE_SHOT_WEAPON_SHOT_FIRED_VOLUME = 65,

	VEHICLE_ONE_SHOT_WEAPON_HIT_VEHICLE_MAX_DIST = 40,
	VEHICLE_ONE_SHOT_WEAPON_HIT_VEHICLE_VOLUME = 90,

	VEHICLE_ONE_SHOT_BOMB_ARMED_MAX_DIST = 50,
	VEHICLE_ONE_SHOT_BOMB_ARMED_VOLUME = 50,

	VEHICLE_ONE_SHOT_WATER_FALL_MAX_DIST = 40,
	VEHICLE_ONE_SHOT_WATER_FALL_VOLUME = 90,

	VEHICLE_ONE_SHOT_SPLATTER_MAX_DIST = 40,
	VEHICLE_ONE_SHOT_SPLATTER_VOLUME = 55,

	VEHICLE_ONE_SHOT_CAR_PED_COLLISION_MAX_DIST = 40,

	TRAIN_NOISE_FAR_MAX_DIST = 140,
	TRAIN_NOISE_NEAR_MAX_DIST = 70,
	TRAIN_NOISE_VOLUME = 70,

	FERRY_NOISE_MAX_DIST = 70,
	FERRY_NOISE_WATER_VOLUME = 30,
	FERRY_NOISE_ENGINE_MAX_DIST = 160,
	FERRY_NOISE_ENGINE_VOLUME = 40,

	BOAT_ENGINE_MAX_DIST = 90,
	BOAT_ENGINE_JETSKI_MAX_DIST = 120,
	BOAT_ENGINE_REEFER_IDLE_VOLUME = 80,

	BOAT_ENGINE_LOW_ACCEL_VOLUME = 45,
	BOAT_ENGINE_HIGH_ACCEL_MIN_VOLUME = 15,
	BOAT_ENGINE_HIGH_ACCEL_VOLUME_MULT = 105,

	BOAT_MOVING_OVER_WATER_MAX_DIST = 50,
	BOAT_MOVING_OVER_WATER_VOLUME = 30,

	JUMBO_MAX_DIST = 440,
	JUMBO_RUMBLE_SOUND_MAX_DIST = 240,
	JUMBO_ENGINE_SOUND_MAX_DIST = 180,
	JUMBO_WHINE_SOUND_MAX_DIST = 170,
};

enum eVehicleModel
{
	SIXATV,
	ADMIRAL,
	CHEETAH,
	AUTOGYRO,
	BAGGAGE,
	BANSHEE,
	PEREN,
	BLISTAC,
	BMXBOY,
	BMXGIRL,
	BOBCAT,
	BULLDOZE,
	BURRITO,
	CABBIE,
	CADDY,
	SPEEDER2,
	PIMP,
	DELUXO,
	HUEY,
	HUEYHOSP,
	ELECTRAG,
	ELECTRAP,
	ESPERANT,
	FBICAR,
	FIRETRUCK,
	GLENDALE,
	GREENWOO,
	HERMES,
	HOVERCR,
	IDAHO,
	LANDSTAL,
	MANANA,
	MOP50,
	OCEANIC,
	VICECHEE,
	SANCHEZ,
	STALLION,
	POLICE,
	BOBO,
	PATRIOT,
	PONY,
	SENTINEL,
	PCJ600,
	MAVERICK,
	REEFER,
	SPEEDER,
	LINERUN,
	WALTON,
	BARRACKS,
	PREDATOR,
	FLATBED,
	AMMOTRUK,
	BIPLANE,
	MOONBEAM,
	RUMPO,
	YOLA,
	TAXI,
	AMBULAN,
	STRETCH,
	FAGGIO,
	QUAD,
	ANGEL,
	FREEWAY,
	JETSKI,
	ENFORCER,
	BOXVILLE,
	BENSON,
	COACH,
	MULE,
	VOODOO,
	SECURICA,
	TRASH,
	TOPFUN,
	YANKEE,
	MRWHOOP,
	SANDKING,
	RHINO,
	DINGHY,
	MARQUIS,
	RIO,
	TROPIC,
	FORKLIFT,
	STREETFI,
	VIRGO,
	STINGER,
	BFINJECT,
	PHEONIX,
	SQUALO,
	JETMAX,
	MESA,
	VCNMAV,
	POLMAV,
	SPARROW,
	SEASPAR,
	SCARAB,
	CHOLLO,
	COMET,
	CUBAN,
	FBIRANCH,
	GANGBUR,
	INFERNUS,
	REGINA,
	SABRE,
	SABRETUR,
	SENTXS,
	HUNTER,
	WASHING,
	COASTG,
	SKIMMER,
	CHOPPER,
	AIRTRAIN,
	MAX_CARS,
};

struct tEngineSounds
{
	int32 idleId;
	int32 revId;
};
const tEngineSounds aEngineSoundsMG[24] = {
	{ SFX_CAR_IDLE_PONT,        SFX_CAR_REV_PONT },        // 0
	{ SFX_CAR_IDLE_PORSHE,      SFX_CAR_REV_PORSHE },      // 1
	{ SFX_CAR_IDLE_SPIDER,      SFX_CAR_REV_SPIDER },      // 2
	{ SFX_CAR_IDLE_MERC,        SFX_CAR_REV_MERC },        // 3
	{ SFX_CAR_IDLE_TRUCK,       SFX_CAR_REV_TRUCK },       // 4
	{ SFX_CAR_IDLE_HOTROD,      SFX_CAR_REV_HOTROD },      // 5
	{ SFX_CAR_IDLE_COBRA,       SFX_CAR_REV_COBRA },       // 6
#ifdef PSP_SFX_IDS
	{ SFX_CAR_IDLE_PONT2,       SFX_CAR_REV_PONT2 },       // 7
	{ SFX_CAR_IDLE_CADI,        SFX_CAR_REV_CADI },        // 8
	{ SFX_CAR_IDLE_PATHFINDER,  SFX_CAR_REV_PATHFINDER },  // 9
	{ SFX_CAR_IDLE_PACARD,      SFX_CAR_REV_PACARD },      // 10
#else
	{ SFX_CAR_IDLE_PONT,        SFX_CAR_REV_PONT },        // 7
	{ SFX_CAR_IDLE_MERC,        SFX_CAR_REV_MERC },        // 8
	{ SFX_CAR_IDLE_PATHFINDER,  SFX_CAR_REV_PATHFINDER },  // 9
	{ SFX_CAR_IDLE_MERC,        SFX_CAR_REV_MERC },        // 10
#endif
	{ SFX_CAR_IDLE_GOLFCART,    SFX_CAR_REV_GOLFCART },    // 11
	{ SFX_CAR_IDLE_GOLFCART,    SFX_CAR_REV_GOLFCART },    // 12
	{ SFX_CAR_IDLE_GOLFCART,    SFX_CAR_REV_GOLFCART },    // 13
	{ SFX_CAR_IDLE_GOLFCART,    SFX_CAR_REV_GOLFCART },    // 14
	{ SFX_CAR_IDLE_GOLFCART,    SFX_CAR_REV_GOLFCART },    // 15
	{ SFX_CAR_IDLE_VTWI,        SFX_CAR_REV_VTWI },        // 16
	{ SFX_MOPED_IDLE,           SFX_MOPED_REV },           // 17
	{ SFX_CAR_IDLE_HONDA,       SFX_CAR_REV_HONDA },       // 18
	{ SFX_CAR_IDLE_SPORTCAR,    SFX_CAR_REV_SPORTCAR },    // 19
#ifdef PSP_SFX_IDS
	{ SFX_CAR_IDLE_UNUSED1,     SFX_CAR_REV_UNUSED1 },     // 20 // diff <<--- \/
	{ SFX_CAR_IDLE_UNUSED2,		SFX_CAR_REV_UNUSED2 },     // 21
	{ SFX_CAR_IDLE_UNUSED3,     SFX_CAR_REV_UNUSED3 },     // 22
	{ SFX_CAR_IDLE_UNUSED4,     SFX_CAR_REV_UNUSED4 },     // 23
#else
	{ SFX_CAR_IDLE_UNUSED1,     SFX_CAR_REV_HOVERCRAFT },  // 20
	{ SFX_CAR_IDLE_STREET_BIKE, SFX_CAR_REV_STREET_BIKE }, // 21
	{ SFX_CAR_IDLE_UNUSED1,     SFX_CAR_REV_UNUSED1 },     // 22
	{ SFX_CAR_IDLE_UNUSED1,     SFX_CAR_REV_UNUSED1 },     // 23
#endif
};

void cAudioManager::ProcessMultiplayerVehicle(cVehicleMG* vehicle)
{
	if (cMultiGame::Instance().IsElementOwnerLocalPlayer(vehicle->GetElement().element))
		return;

    cVehicleParams params;

    switch (CGame::currArea)
    {
        case AREA_MAIN_MAP:
        case AREA_MANSION:
        case AREA_MALL:
        case AREA_EVERYWHERE:
        case AREA_DIRT:
        case AREA_BLOOD:
        case AREA_OVALRING:
            break;
        default:
            return;
    }

	tHandlingData* pHandling = GET_HANDLING(vehicle->GetModelIndex());
    m_sQueueSample.m_vecPos = vehicle->GetPosition();
    params.m_bDistanceCalculated = FALSE;
    params.m_pNetVehicle = vehicle;
    params.m_fDistance = GetDistanceSquared(m_sQueueSample.m_vecPos);
    params.m_pTransmission = pHandling != nil ? &pHandling->Transmission : nil;
    params.m_nIndex = vehicle->m_modelIndex - MI_FIRST_VEHICLE;
    //if (vehicle->GetStatus() == STATUS_SIMPLE)
    //    params.m_fVelocityChange = vehicle->AutoPilot.m_fMaxTrafficSpeed * 0.02f;
    //else
    //    params.m_fVelocityChange = DotProduct(vehicle->m_vecMoveSpeed, vehicle->GetForward());
    //params.m_VehicleType = vehicle->m_vehType; // why? seems fine

	sVehicle* pVeh = vehicle->GetElement().vehicle;
    params.m_fVelocityChange = DotProduct(pVeh->GetSync().elementphysical->GetMoveSpeed(), vehicle->GetForward());

	if (pVeh->HasCapability(sAutomobile::Capability()))
		params.m_VehicleType = eVehicleType::VEHICLE_TYPE_CAR;
	else if (pVeh->HasCapability(sBike::Capability()))
		params.m_VehicleType = eVehicleType::VEHICLE_TYPE_BIKE;
#ifndef GTA_LIBERTY
	else if (pVeh->HasCapability(sBoat::Capability()))
		params.m_VehicleType = eVehicleType::VEHICLE_TYPE_BOAT;
	else if (pVeh->HasCapability(sHeli::Capability()))
		params.m_VehicleType = eVehicleType::VEHICLE_TYPE_HELI;
	else if (pVeh->HasCapability(sPlane::Capability()))
		params.m_VehicleType = eVehicleType::VEHICLE_TYPE_PLANE;
	else if (pVeh->HasCapability(sBmx::Capability()))
		params.m_VehicleType = eVehicleType::VEHICLE_TYPE_BMX;
	else if (pVeh->HasCapability(sQuadBike::Capability()))
		params.m_VehicleType = eVehicleType::VEHICLE_TYPE_QUAD;

#ifdef FIX_BUGS
	if (pVeh->HasCapability(sBoat::Capability()) && vehicle->GetModelIndex() == MI_JETSKI)
		params.m_VehicleType = eVehicleType::VEHICLE_TYPE_JETSKI;
#endif
#endif

    //if (CGame::currArea == AREA_MALL && FindVehicleOfPlayer() != vehicle)
    //{
    //    ProcessVehicleOneShots(params);
    //    ProcessVehicleSirenOrAlarm(params);
    //    ProcessEngineDamage(params);
    //    return;
    //}

    switch (params.m_VehicleType)
    {
        case VEHICLE_TYPE_CAR:
#ifdef FIX_BUGS
        case VEHICLE_TYPE_QUAD:
#endif
            UpdateMultiplayerGasPedalAudio(vehicle, params.m_VehicleType);
            //if (vehicle->m_modelIndex == MI_RCBANDIT || vehicle->m_modelIndex == MI_RCBARON)
            //{
            //    ProcessMultiplayerModelVehicle(params);
            //    ProcessMultiplayerEngineDamage(params);
            //}
            //else if (vehicle->m_modelIndex == MI_RCRAIDER || vehicle->m_modelIndex == MI_RCGOBLIN)
            //{
            //    ProcessMultiplayerModelHeliVehicle(params);
            //    ProcessMultiplayerEngineDamage(params);
            //}
            //else
            //{
            //    switch (vehicle->GetVehicleAppearance())
            //    {
            //        case VEHICLE_APPEARANCE_HELI:
            //            ProcessMultiplayerCarHeli(params);
            //            ProcessMultiplayerVehicleFlatTyre(params);
            //            ProcessMultiplayerEngineDamage(params);
            //            break;
            //        case VEHICLE_APPEARANCE_BOAT:
            //        case VEHICLE_APPEARANCE_PLANE:
            //            break;
            //        default:
            //            if (vehicle->GetModelIndex() == MI_FORKLIFT) // fine be fix bugs but mp didnt sync fork height, so
            //                ProcessMultiplayerVehicleForkliftForksNoise(params);
                        if (ProcessMultiplayerVehicleRoadNoise(params))
                        {
                            ProcessMultiplayerReverseGear(params);
                            //if (CWeather::WetRoads > 0.0f)
                            //    ProcessMultiplayerWetRoadNoise(params);
                            ProcessMultiplayerVehicleSkidding(params);
                            ProcessMultiplayerVehicleFlatTyre(params);
                            ProcessMultiplayerVehicleHorn(params);
                            ProcessMultiplayerVehicleSirenOrAlarm(params);
                            //if (UsesMultiplayerReverseWarning(params.m_nIndex))
                            //    ProcessMultiplayerVehicleReverseWarning(params);
                            //if (HasMultiplayerAirBrakes(params.m_nIndex))
                            //    ProcessMultiplayerAirBrakes(params);
                            //ProcessMultiplayerCarBombTick(params);
                            ProcessMultiplayerVehicleEngine(params);
                            ProcessMultiplayerEngineDamage(params);
                            ProcessMultiplayerVehicleDoors(params);
                        }
            //            break;
            //    }
            //}
            //ProcessMultiplayerVehicleOneShots(params);
            vehicle->m_fVelocityChangeForAudio = params.m_fVelocityChange;
            break;
//        case VEHICLE_TYPE_BOAT:
//            if (vehicle->m_modelIndex == MI_SKIMMER)
//                ProcessMultiplayerCarHeli(params);
//            else
//                ProcessMultiplayerBoatEngine(params);
//            ProcessMultiplayerBoatMovingOverWater(params);
//            ProcessMultiplayerVehicleOneShots(params);
//            break;
////#ifdef GTA_TRAIN
////        case VEHICLE_TYPE_TRAIN:
////            ProcessMultiplayerTrainNoise(params);
////            ProcessMultiplayerVehicleOneShots(params);
////            break;
////#endif
//        case VEHICLE_TYPE_JETSKI:
//            ProcessMultiplayerJetskiEngine(params);
//            ProcessMultiplayerVehicleOneShots(params);
//            break;
//        case VEHICLE_TYPE_HELI:
//            ProcessMultiplayerCarHeli(params);
//            ProcessMultiplayerVehicleOneShots(params);
//            break;
//        case VEHICLE_TYPE_PLANE:
//            ProcessMultiplayerPlane(params);
//            ProcessMultiplayerVehicleOneShots(params);
//            ProcessMultiplayerVehicleFlatTyre(params);
//            break;
        case VEHICLE_TYPE_BIKE:
#ifdef FIX_BUGS
            UpdateMultiplayerGasPedalAudio(vehicle, params.m_VehicleType);
#else
            UpdateMultiplayerGasPedalAudio(vehicle, VEHICLE_TYPE_BIKE);
#endif
            if (ProcessMultiplayerVehicleRoadNoise(params))
            {
                //if (CWeather::WetRoads > 0.0f)
                //    ProcessMultiplayerWetRoadNoise(params);
                ProcessMultiplayerVehicleSkidding(params);
                ProcessMultiplayerVehicleHorn(params);
                ProcessMultiplayerVehicleSirenOrAlarm(params);
                //ProcessMultiplayerCarBombTick(params);
                ProcessMultiplayerEngineDamage(params);
                ProcessMultiplayerVehicleEngine(params);
                ProcessMultiplayerVehicleFlatTyre(params);
            }
            //ProcessMultiplayerVehicleOneShots(params);
            vehicle->m_fVelocityChangeForAudio = params.m_fVelocityChange;
            break;
        ////case VEHICLE_TYPE_FERRY:
        ////    ProcessMultiplayerFerryNoise(params);
        ////    ProcessMultiplayerVehicleOneShots(params);
        ////    break;
        //case VEHICLE_TYPE_BMX:
        //    UpdateMultiplayerGasPedalAudio(veh, params.m_VehicleType);
        //    ProcessMultiplayerVehicleSkidding(params);
        //    ProcessMultiplayerVehicleHorn(params);
        //    ProcessMultiplayerBicycle(params);
        //    ((CBike*)vehicle)->m_fVelocityChangeForAudio = params.m_fVelocityChange;
        //    break;
        default:
            break;
    }

//#ifdef FIX_BUGS
//    if(params.m_VehicleType != VEHICLE_TYPE_BIKE && params.m_VehicleType != VEHICLE_TYPE_BMX)
//#endif
    ProcessRainOnVehicle(params);
}

bool8 cAudioManager::UsesMultiplayerSiren(cVehicleParams& params)
{
	switch (params.m_pNetVehicle->GetModelIndex())
	{
		case MI_ELECTRAP:
		case MI_FBICAR:
		case MI_FIRETRUCK:
		case MI_VICECHEE:
		case MI_POLICE:
		case MI_PREDATOR:
		case MI_AMBULAN:
		case MI_ENFORCER:
		case MI_MRWHOOP:
		case MI_FBIRANCH:
			// vcs: -992
			// vcs: -935
			// vcs: -934
			return true;
		default:
			return false;
	}
}

void cAudioManager::UpdateMultiplayerGasPedalAudio(cVehicleMG* veh, int vehType)
{
	float gasPedal = Abs(veh->GetElement().vehicle->GetSync().vehicle->m_fGasPedal);
	if (veh->m_fGasPedalAudio < gasPedal)
		veh->m_fGasPedalAudio = Min(veh->m_fGasPedalAudio + 0.09f, gasPedal);
	else
		veh->m_fGasPedalAudio = Max(veh->m_fGasPedalAudio - 0.07f, gasPedal);
}

bool8 cAudioManager::ProcessMultiplayerReverseGear(cVehicleParams& params)
{
	cVehicleMG* veh;
	uint8 Vol;
	float modificator;

	if (params.m_fDistance < SQR(REVERSE_GEAR_MAX_DIST))
	{
		veh = params.m_pNetVehicle;
		if (veh->GetModelIndex() == MI_CADDY)
			return TRUE;
		if (
			//veh->GetElement().vehicle->GetSync().vehicle->bEngineOn &&
			(veh->GetElement().vehicle->GetSync().vehicle->m_fGasPedal < 0.0f ||
				veh->GetElement().vehicle->GetSync().vehicle->m_nCurrentGear == 0))
		{
			CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
			//if (veh->m_nDriveWheelsOnGround > 0)
				modificator = params.m_fVelocityChange / params.m_pTransmission->fMaxReverseVelocity;
			//else
			//{
			//	if (veh->m_nDriveWheelsOnGroundPrev > 0)
			//		veh->m_fGasPedalAudio *= 0.4f;
			//	modificator = veh->m_fGasPedalAudio;
			//}
			modificator = ABS(modificator);
			Vol = ((float)REVERSE_GEAR_VOLUME * modificator);
			m_sQueueSample.m_nVolume = ComputeVolume(Vol, REVERSE_GEAR_MAX_DIST, m_sQueueSample.m_fDistance);
			if (m_sQueueSample.m_nVolume > 0)
			{
				if (veh->GetElement().vehicle->GetSync().vehicle->m_fGasPedal < 0.0f)
				{
					m_sQueueSample.m_nCounter = 61;
					m_sQueueSample.m_nSampleIndex = SFX_REVERSE_GEAR;
				}
				else
				{
					m_sQueueSample.m_nCounter = 62;
					m_sQueueSample.m_nSampleIndex = SFX_REVERSE_GEAR_2;
				}
				m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				m_sQueueSample.m_bIs2D = FALSE;
				m_sQueueSample.m_nPriority = 3;
				m_sQueueSample.m_nFrequency = (6000 * modificator) + 7000;
				m_sQueueSample.m_nLoopCount = 0;
				SET_EMITTING_VOLUME(Vol);
				SET_LOOP_OFFSETS(m_sQueueSample.m_nSampleIndex)
					m_sQueueSample.m_fSpeedMultiplier = 3.0f;
				m_sQueueSample.m_MaxDistance = REVERSE_GEAR_MAX_DIST;
				m_sQueueSample.m_bStatic = FALSE;
				m_sQueueSample.m_nFramesToPlay = 5;
				SET_SOUND_REVERB(TRUE);
				SET_SOUND_REFLECTION(FALSE);
				AddSampleToRequestedQueue();
			}
		}
		return TRUE;
	}
	return FALSE;
}

bool8 cAudioManager::ProcessMultiplayerEngineDamage(cVehicleParams& params)
{
	uint8 Vol;

	if (params.m_fDistance < SQR(ENGINE_DAMAGE_MAX_DIST))
	{
		if (params.m_pNetVehicle->m_modelIndex == MI_CADDY)
			return TRUE;
		if (params.m_pNetVehicle->GetElement().vehicle->GetSync().vehicle->m_nStatus == STATUS_WRECKED ||
			params.m_pNetVehicle->GetElement().vehicle->GetSync().vehicle->m_fHealth >= 390.0f)
			return TRUE;
		if (params.m_pNetVehicle->GetElement().vehicle->GetSync().vehicle->m_fHealth < 250.0f)
		{
			Vol = 60;
			m_sQueueSample.m_nSampleIndex = SFX_CAR_ON_FIRE;
			m_sQueueSample.m_nPriority = 7;
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CAR_ON_FIRE);
		}
		else
		{
			Vol = 30;
			m_sQueueSample.m_nSampleIndex = SFX_PALM_TREE_LO;
			m_sQueueSample.m_nPriority = 7;
			m_sQueueSample.m_nFrequency = 27000;
		}
		CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
		//if (params.m_pNetVehicle->bIsDrowning)
		//	Vol >>= 2;
		m_sQueueSample.m_nVolume = ComputeVolume(Vol, ENGINE_DAMAGE_MAX_DIST, m_sQueueSample.m_fDistance);
		if (m_sQueueSample.m_nVolume > 0)
		{
			m_sQueueSample.m_nCounter = 28;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_bIs2D = FALSE;
			m_sQueueSample.m_nLoopCount = 0;
			SET_EMITTING_VOLUME(Vol);
			SET_LOOP_OFFSETS(m_sQueueSample.m_nSampleIndex)
				m_sQueueSample.m_fSpeedMultiplier = 2.0f;
			m_sQueueSample.m_MaxDistance = ENGINE_DAMAGE_MAX_DIST;
			m_sQueueSample.m_bStatic = FALSE;
			m_sQueueSample.m_nFramesToPlay = 3;
			SET_SOUND_REVERB(TRUE);
			SET_SOUND_REFLECTION(FALSE);
			AddSampleToRequestedQueue();
		}
		return TRUE;
	}
	return FALSE;
}

bool8 cAudioManager::ProcessMultiplayerVehicleDoors(cVehicleParams& params)
{
	cVehicleMG* vehicle = params.m_pNetVehicle;
	//int8 doorState;
	uint8 Vol;
	float velocity;

	if (params.m_fDistance < SQR(VEHICLE_DOORS_MAX_DIST))
	{
		sAutomobile* pAuto = vehicle->GetElement().automobile;
		CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
		for (uint8 i = 0; i < ARRAY_SIZE(pAuto->Doors); i++)
		{
			if (pAuto->GetSync().automobile->Damage.GetDoorStatus((eDoors)i) == DOOR_STATUS_SWINGING)
			{
				//doorState = pAuto->Doors[i].m_nDoorState;
				//if (doorState == DOORST_OPEN || doorState == DOORST_CLOSED)

				float fAngle = pAuto->GetSync().automobile->Doors[i].m_fAngle;
				if (fAngle > pAuto->Doors[i].m_fMaxAngle || fAngle < pAuto->Doors[i].m_fMinAngle)
				{
					velocity = Min(0.3f, Abs(pAuto->GetSync().automobile->Doors[i].m_fAngVel));
					if (velocity > 0.0035f)
					{
						Vol = ((float)VEHICLE_DOORS_VOLUME * velocity / 0.3f);
						m_sQueueSample.m_nVolume = ComputeVolume(Vol, VEHICLE_DOORS_MAX_DIST, m_sQueueSample.m_fDistance);
						if (m_sQueueSample.m_nVolume > 0)
						{
							m_sQueueSample.m_nCounter = i + 6;
							m_sQueueSample.m_nSampleIndex = m_anRandomTable[1] % 6 + SFX_COL_CAR_PANEL_1;
							m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex) + RandomDisplacement(1000);
							m_sQueueSample.m_nBankIndex = SFX_BANK_0;
							m_sQueueSample.m_bIs2D = FALSE;
							m_sQueueSample.m_nPriority = 10;
							m_sQueueSample.m_nLoopCount = 1;
							SET_EMITTING_VOLUME(Vol);
							RESET_LOOP_OFFSETS
							m_sQueueSample.m_fSpeedMultiplier = 1.0f;
							m_sQueueSample.m_MaxDistance = VEHICLE_DOORS_MAX_DIST;
							m_sQueueSample.m_bStatic = TRUE;
							SET_SOUND_REVERB(TRUE);
							SET_SOUND_REFLECTION(TRUE);
							AddSampleToRequestedQueue();
						}
					}
				}
			}
		}
		return TRUE;
	}
	return FALSE;
}

bool8 cAudioManager::ProcessMultiplayerVehicleHorn(cVehicleParams& params)
{
	if (params.m_fDistance < SQR(VEHICLE_HORN_MAX_DIST))
	{
		if (params.m_pNetVehicle->GetModelIndex() == MI_MRWHOOP || params.m_pNetVehicle->GetModelIndex() == MI_AMBULAN)
			return TRUE;

		sVehicle* pVeh = params.m_pNetVehicle->GetElement().vehicle;
		uint32 nCarHornPattern = 0;
		if (pVeh->GetSync().vehicle->m_nCarHornTimer > 0)
		{
			if (pVeh->GetSync().vehicle->m_nStatus != STATUS_PLAYER)
			{
				pVeh->GetSync().vehicle->m_nCarHornTimer = Min(44, pVeh->GetSync().vehicle->m_nCarHornTimer);
				if (pVeh->GetSync().vehicle->m_nCarHornTimer == 44)
					nCarHornPattern = (m_FrameCounter + m_sQueueSample.m_nEntityIndex) & 7;

				if (!HornPattern[nCarHornPattern][44 - pVeh->GetSync().vehicle->m_nCarHornTimer])
					return TRUE;
			}

			CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
#ifdef FIX_BUGS
			m_sQueueSample.m_nVolume = ComputeVolume(pVeh->GetSync().vehicle->bIsDrowning ? VEHICLE_HORN_VOLUME / 4 : VEHICLE_HORN_VOLUME, VEHICLE_HORN_MAX_DIST, m_sQueueSample.m_fDistance);
#else
			m_sQueueSample.m_nVolume = ComputeVolume(VEHICLE_HORN_VOLUME, VEHICLE_HORN_MAX_DIST, m_sQueueSample.m_fDistance);
#endif
			const auto& sampleData = ((CVehicleModelInfo*)CModelInfo::GetModelInfo(params.m_pNetVehicle->GetModelIndex()))->m_SampleData;
			if (m_sQueueSample.m_nVolume > 0)
			{
				m_sQueueSample.m_nCounter = 4;
				m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				m_sQueueSample.m_nSampleIndex = sampleData.m_nHornSample;
				m_sQueueSample.m_nFramesToPlay = 4;
				m_sQueueSample.m_nLoopCount = 0;
				m_sQueueSample.m_nFrequency = sampleData.m_nHornFrequency;
				SET_SOUND_REFLECTION(FALSE);
				m_sQueueSample.m_bIs2D = FALSE;
				m_sQueueSample.m_bStatic = FALSE;
				m_sQueueSample.m_nPriority = 2;
				m_sQueueSample.m_fSpeedMultiplier = 5.0f;
				m_sQueueSample.m_MaxDistance = VEHICLE_HORN_MAX_DIST;

#ifdef FIX_BUGS
				SET_EMITTING_VOLUME(pVeh->GetSync().vehicle->bIsDrowning ? VEHICLE_HORN_VOLUME / 4 : VEHICLE_HORN_VOLUME);
#else
				SET_EMITTING_VOLUME(VEHICLE_HORN_VOLUME);
#endif
				SET_LOOP_OFFSETS(m_sQueueSample.m_nSampleIndex)
				SET_SOUND_REVERB(TRUE);
				AddSampleToRequestedQueue();
			}
		}
		return TRUE;
	}
	return FALSE;
}

float cAudioManager::GetMultiplayerVehicleDriveWheelSkidValue(cVehicleMG* veh, tWheelState wheelState, float gasPedalAudio, cTransmission* transmission, float velocityChange)
{
	float relativeVelChange = 0.0f;
	float velChange;
	float relativeVel;

	switch (wheelState)
	{
		case WHEEL_STATE_SPINNING:
			if (gasPedalAudio > 0.4f)
				relativeVelChange = (gasPedalAudio - 0.4f) * (5.0f / 3.0f) * 0.75f;
			break;
		case WHEEL_STATE_SKIDDING:
			relativeVelChange = Min(1.0f, Abs(velocityChange) / transmission->fMaxVelocity);
			break;
		case WHEEL_STATE_FIXED:
			relativeVel = gasPedalAudio;
			if (relativeVel > 0.4f)
				relativeVel = (gasPedalAudio - 0.4f) * (5.0f / 3.0f);

			velChange = Abs(velocityChange);
			if (velChange > 0.04f)
				relativeVelChange = Min(1.0f, velChange / transmission->fMaxVelocity);
			if (relativeVel > relativeVelChange)
				relativeVelChange = relativeVel;

			break;
		default:
			break;
	}

	return Max(relativeVelChange, Min(1.0f, Abs(veh->GetElement().vehicle->GetSync().vehicle->GetTurnSpeed().z) * 20.0f));
}

float cAudioManager::GetMultiplayerVehicleNonDriveWheelSkidValue(cVehicleMG* veh, tWheelState wheelState, cTransmission* transmission, float velocityChange)
{
	float relativeVelChange = 0.0f;

	if (wheelState == WHEEL_STATE_SKIDDING)
		relativeVelChange = Min(1.0f, Abs(velocityChange) / transmission->fMaxVelocity);

	return Max(relativeVelChange, Min(1.0f, Abs(veh->GetElement().vehicle->GetSync().vehicle->GetTurnSpeed().z) * 20.0f));
}

bool8 cAudioManager::ProcessMultiplayerVehicleEngine(cVehicleParams& params)
{
	float relativeGearChange;
#ifdef FIX_BUGS
	uint32 freq = 0; // uninitialized variable
#else
	uint32 freq;
#endif
	uint8 Vol;
	cTransmission* transmission;
	uint8 currentGear;
	float modificator;
	uint8 wheelsOnGround;
	float* gasPedalAudioPtr;

	bool8 isMoped = FALSE;
	bool8 isGolfCart = FALSE;
	float traction = 0.0f;
	tVehicleSampleData& sampleData = ((CVehicleModelInfo*)CModelInfo::GetModelInfo(params.m_pNetVehicle->GetModelIndex()))->m_SampleData;
	if (params.m_fDistance < SQR(VEHICLE_ENGINE_MAX_DIST))
	{
		transmission = params.m_pTransmission;
		sVehicle* pVeh = params.m_pNetVehicle->GetElement().vehicle;
		if (transmission != nil)
		{
			switch (params.m_pNetVehicle->GetModelIndex())
			{
				case MI_PIZZABOY:
				case MI_FAGGIO:
					isMoped = TRUE;
					currentGear = transmission->nNumberOfGears;
					break;
				case MI_CADDY:
					currentGear = transmission->nNumberOfGears;
					isGolfCart = TRUE;
					break;
				default:
					currentGear = pVeh->GetSync().vehicle->m_nCurrentGear;
					break;
			}

			switch (params.m_VehicleType)
			{
				case VEHICLE_TYPE_CAR:
#ifdef FIX_BUGS
				case VEHICLE_TYPE_QUAD:
#endif
					wheelsOnGround = 4;
					gasPedalAudioPtr = &params.m_pNetVehicle->m_fGasPedalAudio;
					break;
				case VEHICLE_TYPE_BIKE:
					wheelsOnGround = 2;
					gasPedalAudioPtr = &params.m_pNetVehicle->m_fGasPedalAudio;
					break;
				default:
					debug(" ** AUDIOLOG: Unrecognised vehicle type %d in ProcessVehicleEngine() * \n", params.m_VehicleType);
					return TRUE;
			}

			if (pVeh->GetSync().vehicle->m_bIsHandbrakeOn && (!isMoped || !isGolfCart))
			{ // what a weird check
				if (params.m_fVelocityChange == 0.0f)
					traction = 0.9f;
			}
			else if (pVeh->GetSync().vehicle->m_nStatus == STATUS_SIMPLE || isMoped || isGolfCart)
			{
				traction = 0.0f;
			}
			else
			{
				switch (transmission->nDriveType)
				{
					case '4':
						if (params.m_VehicleType == VEHICLE_TYPE_BIKE)
						{
							for (uint8 i = 0; i < 2; i++)
								if (pVeh->GetSync().vehicle->m_aWheelState[i] == WHEEL_STATE_SPINNING)
									traction += 0.1f;
						}
						else
						{
							for (uint8 i = 0; i < 4; i++)
								if (pVeh->GetSync().vehicle->m_aWheelState[i] == WHEEL_STATE_SPINNING)
									traction += 0.05f;
						}
						break;
					case 'F':
						if (params.m_VehicleType == VEHICLE_TYPE_BIKE)
						{
							if (pVeh->GetSync().vehicle->m_aWheelState[BIKEWHEEL_FRONT] == WHEEL_STATE_SPINNING)
								traction += 0.2f;
						}
						else
						{
							if (pVeh->GetSync().vehicle->m_aWheelState[CARWHEEL_FRONT_LEFT] == WHEEL_STATE_SPINNING)
								traction += 0.1f;
							if (pVeh->GetSync().vehicle->m_aWheelState[CARWHEEL_FRONT_RIGHT] == WHEEL_STATE_SPINNING)
								traction += 0.1f;
						}
						break;
					case 'R':
						if (params.m_VehicleType == VEHICLE_TYPE_BIKE)
						{
							if (pVeh->GetSync().vehicle->m_aWheelState[BIKEWHEEL_REAR] == WHEEL_STATE_SPINNING)
								traction += 0.2f;
						}
						else
						{
							if (pVeh->GetSync().vehicle->m_aWheelState[CARWHEEL_REAR_LEFT] == WHEEL_STATE_SPINNING)
								traction += 0.1f;
							if (pVeh->GetSync().vehicle->m_aWheelState[CARWHEEL_REAR_RIGHT] == WHEEL_STATE_SPINNING)
								traction += 0.1f;
						}
						break;
					default:
						break;
				}
			}

			if (transmission->fMaxVelocity > 0.0f)
			{
				if (isMoped || isGolfCart)
					modificator = Min(1.0f, Abs(params.m_fVelocityChange / transmission->fMaxVelocity > 1.0f));
				else
				{
					if (currentGear > 0)
					{
						relativeGearChange = Min(1.0f,
							params.m_fVelocityChange - transmission->Gears[currentGear].fShiftDownVelocity) /
							transmission->fMaxVelocity * 2.5f;
						if (traction == 0.0f && pVeh->GetSync().vehicle->m_nStatus != STATUS_SIMPLE &&
							params.m_fVelocityChange < transmission->Gears[1].fShiftUpVelocity)
							traction = 0.7f;
						modificator = traction * *gasPedalAudioPtr * 0.95f + (1.0f - traction) * relativeGearChange;
					}
					else
					{
						modificator = Min(1.0f,
							1.0f - Abs((params.m_fVelocityChange - transmission->Gears[0].fShiftDownVelocity) / transmission->fMaxReverseVelocity));
					}
				}
			}
			else
				modificator = 0.0f;

			if (currentGear == 0 || wheelsOnGround > 0)
			{
				if (params.m_VehicleType == VEHICLE_TYPE_BIKE)
					freq = 22050;
				else
					freq = 13000 * modificator + 14000;
			}
			else
				freq = 1200 * currentGear + 18000 * modificator + 14000;
			if (modificator < 0.75f)
				Vol = modificator / 0.75f * ((float)VEHICLE_ENGINE_FULL_VOLUME - (float)VEHICLE_ENGINE_BASE_VOLUME) + (float)VEHICLE_ENGINE_BASE_VOLUME;
			else
				Vol = VEHICLE_ENGINE_FULL_VOLUME;
		}
		else
		{
			modificator = 0.0f;
			Vol = VEHICLE_ENGINE_BASE_VOLUME;
		}

		//if (params.m_pNetVehicle->bIsDrowning)
		//	Vol >>= 2;
		if (isGolfCart)
		{
			Vol = 100 * modificator;
			freq = 2130 * modificator + 4270;
			m_sQueueSample.m_nCounter = 2;
		}
		m_sQueueSample.m_nVolume = ComputeVolume(Vol, VEHICLE_ENGINE_MAX_DIST, m_sQueueSample.m_fDistance);
		if (m_sQueueSample.m_nVolume > 0)
		{
			if (!isGolfCart)
			{
				if (pVeh->GetSync().vehicle->m_nStatus == STATUS_SIMPLE)
				{
					if (modificator < 0.02f)
					{
						m_sQueueSample.m_nSampleIndex = aEngineSoundsMG[sampleData.m_nBank - CAR_SFX_BANKS_OFFSET].idleId;
						freq = 10000 * modificator + 22050;
						m_sQueueSample.m_nCounter = 52;
					}
					else
					{
						m_sQueueSample.m_nSampleIndex = sampleData.m_nAccelerationSampleIndex;
						m_sQueueSample.m_nCounter = 2;
					}
				}
				else
				{
					if (pVeh->GetSync().vehicle->m_fGasPedal < 0.02f)
					{
						m_sQueueSample.m_nSampleIndex = aEngineSoundsMG[sampleData.m_nBank - CAR_SFX_BANKS_OFFSET].idleId;
						freq = 10000 * modificator + 22050;
						m_sQueueSample.m_nCounter = 52;
					}
					else
					{
						m_sQueueSample.m_nSampleIndex = sampleData.m_nAccelerationSampleIndex;
						m_sQueueSample.m_nCounter = 2;
					}
				}
			}
			if (isGolfCart)
			{
				//if (FindVehicleOfPlayer() == params.m_pNetVehicle)
				//	m_sQueueSample.m_nSampleIndex = SFX_CAR_AFTER_ACCEL_12;
				//else
					m_sQueueSample.m_nSampleIndex = SFX_CAR_REV_GOLFCART;
				m_sQueueSample.m_nBankIndex = SFX_BANK_0; // TODO: PS2 bank
				m_sQueueSample.m_nFrequency = freq + 20 * m_sQueueSample.m_nEntityIndex % 100;
			}
			else
			{
				m_sQueueSample.m_nBankIndex = SFX_BANK_0; // TODO: PS2 bank
				m_sQueueSample.m_nFrequency = freq + 100 * m_sQueueSample.m_nEntityIndex % 1000;
			}
			m_sQueueSample.m_bIs2D = FALSE;
			m_sQueueSample.m_nPriority = 3;
			if (m_sQueueSample.m_nSampleIndex == SFX_CAR_IDLE_TRUCK || m_sQueueSample.m_nSampleIndex == SFX_CAR_REV_TRUCK)
				m_sQueueSample.m_nFrequency >>= 1;
			m_sQueueSample.m_nLoopCount = 0;
			SET_EMITTING_VOLUME(Vol);
			SET_LOOP_OFFSETS(m_sQueueSample.m_nSampleIndex)
				m_sQueueSample.m_fSpeedMultiplier = 6.0f;
			m_sQueueSample.m_MaxDistance = VEHICLE_ENGINE_MAX_DIST;
			m_sQueueSample.m_bStatic = FALSE;
			m_sQueueSample.m_nFramesToPlay = 8;
			SET_SOUND_REVERB(TRUE);
			SET_SOUND_REFLECTION(FALSE);
			AddSampleToRequestedQueue();
		}
		return TRUE;
	}
	return FALSE;
}

bool8 cAudioManager::ProcessMultiplayerVehicleSkidding(cVehicleParams& params)
{
    uint8 numWheels;
    uint8 wheelsOnGround = 0;
    float gasPedalAudio;

    cTransmission* transmission;
    uint8 Vol;
    float newSkidVal = 0.0f;
    float skidVal = 0.0f;
	sVehicle* pVeh = params.m_pNetVehicle->GetElement().vehicle;

    if (params.m_fDistance < SQR(VEHICLE_SKIDDING_MAX_DIST))
    {
        switch (params.m_VehicleType)
        {
            case VEHICLE_TYPE_CAR:
#ifdef FIX_BUGS
            case VEHICLE_TYPE_QUAD:
#endif
				for (uint32 i = 0; i < 4; i++) {
					if (pVeh->GetSync().vehicle->m_aWheelTimer[i] != 0)
						++wheelsOnGround;
				}
                gasPedalAudio = params.m_pNetVehicle->m_fGasPedalAudio;
                numWheels = 4;
                break;
            case VEHICLE_TYPE_BIKE:
#ifdef FIX_BUGS
            case VEHICLE_TYPE_BMX:
#endif
				for (uint32 i = 0; i < 2; i++) {
					if (pVeh->GetSync().vehicle->m_aWheelTimer[i] != 0)
						++wheelsOnGround;
				}
                gasPedalAudio = params.m_pNetVehicle->m_fGasPedalAudio;
                numWheels = 2;
                break;
            default:
                debug("\n * AUDIOLOG:  ProcessMultiplayerVehicleSkidding() Unsupported vehicle type %d * \n", params.m_VehicleType);
                return TRUE;
        }

        if (wheelsOnGround > 0)
        {
            CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);

            for (uint8 i = 0; i < numWheels; i++)
            {
				tWheelState wheelState = pVeh->GetSync().vehicle->m_aWheelState[i];
                if (wheelState == WHEEL_STATE_NORMAL)
                    continue;
                transmission = params.m_pTransmission;
                switch (transmission->nDriveType)
                {
                    case '4':
                        newSkidVal = GetMultiplayerVehicleDriveWheelSkidValue(params.m_pNetVehicle, wheelState, gasPedalAudio, transmission, params.m_fVelocityChange);
                        break;
                    case 'F':
                        if (i == CARWHEEL_FRONT_LEFT || i == CARWHEEL_FRONT_RIGHT)
                            newSkidVal = GetMultiplayerVehicleDriveWheelSkidValue(params.m_pNetVehicle, wheelState, gasPedalAudio, transmission, params.m_fVelocityChange);
                        else
                            newSkidVal = GetMultiplayerVehicleNonDriveWheelSkidValue(params.m_pNetVehicle, wheelState, transmission, params.m_fVelocityChange);
                        break;
                    case 'R':
                        if (i == CARWHEEL_REAR_LEFT || i == CARWHEEL_REAR_RIGHT)
                            newSkidVal = GetMultiplayerVehicleDriveWheelSkidValue(params.m_pNetVehicle, wheelState, gasPedalAudio, transmission, params.m_fVelocityChange);
                        else
                            newSkidVal = GetMultiplayerVehicleNonDriveWheelSkidValue(params.m_pNetVehicle, wheelState, transmission, params.m_fVelocityChange);
                        break;
                    default:
                        break;
                }
                skidVal = Max(skidVal, newSkidVal);
            }

            if (skidVal > 0.0f)
            {
                Vol = (float)VEHICLE_SKIDDING_VOLUME * skidVal;
                m_sQueueSample.m_nVolume = ComputeVolume(Vol, VEHICLE_SKIDDING_MAX_DIST, m_sQueueSample.m_fDistance);
                if (m_sQueueSample.m_nVolume > 0)
                {
                    m_sQueueSample.m_nCounter = 3;
                    switch (pVeh->GetSync().vehicle->m_aWheelColPoints[0].surfaceA)
                    {
                        case SURFACE_GRASS:
                        case SURFACE_HEDGE:
                            m_sQueueSample.m_nSampleIndex = SFX_RAIN;
                            Vol >>= 2;
                            m_sQueueSample.m_nFrequency = 13000 * skidVal + 35000;
                            m_sQueueSample.m_nVolume >>= 2;
                            if (m_sQueueSample.m_nVolume == 0)
                                return TRUE;
                            break;
                        case SURFACE_GRAVEL:
                        case SURFACE_MUD_DRY:
                        case SURFACE_SAND:
                        case SURFACE_WATER:
                        case SURFACE_SAND_BEACH:
                            m_sQueueSample.m_nSampleIndex = SFX_GRAVEL_SKID;
                            m_sQueueSample.m_nFrequency = 6000 * skidVal + 10000;
                            break;

                        default:
                            m_sQueueSample.m_nSampleIndex = SFX_SKID;
                            m_sQueueSample.m_nFrequency = 5000 * skidVal + 11000;
                            if (params.m_VehicleType == VEHICLE_TYPE_BIKE)
                                m_sQueueSample.m_nFrequency += 2000;
                            //if (params.m_VehicleType == VEHICLE_TYPE_BMX)
                            //{
                            //    m_sQueueSample.m_nSampleIndex = SFX_RAIN;
                            //    Vol >>= 2;
                            //    if (pVeh->GetSync().vehicle->m_aWheelColPoints[0].surfaceA == SURFACE_GRASS)
                            //        Vol >>= 1;
                            //}
                            break;
                    }

                    m_sQueueSample.m_nBankIndex = SFX_BANK_0;
                    m_sQueueSample.m_bIs2D = FALSE;
                    m_sQueueSample.m_nPriority = 8;
                    m_sQueueSample.m_nLoopCount = 0;
                    SET_EMITTING_VOLUME(Vol);
                    SET_LOOP_OFFSETS(m_sQueueSample.m_nSampleIndex)
                    m_sQueueSample.m_fSpeedMultiplier = 3.0f;
                    m_sQueueSample.m_MaxDistance = VEHICLE_SKIDDING_MAX_DIST;
                    m_sQueueSample.m_bStatic = FALSE;
                    m_sQueueSample.m_nFramesToPlay = 3;
                    SET_SOUND_REVERB(TRUE);
                    SET_SOUND_REFLECTION(FALSE);
                    AddSampleToRequestedQueue();
                }
            }
        }
        return TRUE;
    }
    return FALSE;
}

bool8 cAudioManager::ProcessMultiplayerVehicleRoadNoise(cVehicleParams& params)
{
	uint8 Vol;
    uint32 freq;
    float multiplier;
    int sampleFreq;
    float velocity;
    uint8 wheelsOnGround = 0;
	sVehicle* pVeh = params.m_pNetVehicle->GetElement().vehicle;

    if (params.m_fDistance < SQR(VEHICLE_ROAD_NOISE_MAX_DIST))
    {
        switch (params.m_VehicleType)
        {
            case VEHICLE_TYPE_CAR:
#ifdef FIX_BUGS
            case VEHICLE_TYPE_QUAD:
#endif
				for (uint32 i = 0; i < 4; i++) {
					if (pVeh->GetSync().vehicle->m_aWheelTimer[i] != 0)
						++wheelsOnGround;
				}
                break;
            case VEHICLE_TYPE_BIKE:
#ifdef FIX_BUGS
            case VEHICLE_TYPE_BMX:
#endif
				for (uint32 i = 0; i < 2; i++) {
					if (pVeh->GetSync().vehicle->m_aWheelTimer[i] != 0)
						++wheelsOnGround;
				}
                break;
            default:
                wheelsOnGround = 4;
                break;
        }

        if ((params.m_pTransmission != nil) && (wheelsOnGround > 0))
        {
            velocity = Abs(params.m_fVelocityChange);
            if (velocity > 0.0f)
            {
                CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
                Vol = (float)VEHICLE_ROAD_NOISE_VOLUME * Min(1.0f, velocity / (0.5f * params.m_pTransmission->fMaxVelocity));
                m_sQueueSample.m_nVolume = ComputeVolume(Vol, VEHICLE_ROAD_NOISE_MAX_DIST, m_sQueueSample.m_fDistance);
                if (m_sQueueSample.m_nVolume > 0)
                {
                    m_sQueueSample.m_nCounter = 0;
                    m_sQueueSample.m_nBankIndex = SFX_BANK_0;
                    m_sQueueSample.m_bIs2D = FALSE;
                    m_sQueueSample.m_nPriority = 3;
                    if (pVeh->GetSync().vehicle->m_aWheelColPoints[0].surfaceA == SURFACE_WATER)
                    {
                        m_sQueueSample.m_nSampleIndex = SFX_BOAT_WATER_LOOP;
                        freq = 6050 * Vol / VEHICLE_ROAD_NOISE_VOLUME + 16000;
                    }
                    else
                    {
                        m_sQueueSample.m_nSampleIndex = SFX_ROAD_NOISE;
                        multiplier = (m_sQueueSample.m_fDistance / (float)VEHICLE_ROAD_NOISE_MAX_DIST) * 0.5f;
                        sampleFreq = SampleManager.GetSampleBaseFrequency(SFX_ROAD_NOISE);
                        freq = (sampleFreq * multiplier) + ((3 * sampleFreq) >> 2);
                    }
                    m_sQueueSample.m_nFrequency = freq;
                    m_sQueueSample.m_nLoopCount = 0;
                    SET_EMITTING_VOLUME(Vol);
                    SET_LOOP_OFFSETS(m_sQueueSample.m_nSampleIndex)
                    m_sQueueSample.m_fSpeedMultiplier = 6.0f;
                    m_sQueueSample.m_MaxDistance = VEHICLE_ROAD_NOISE_MAX_DIST;
                    m_sQueueSample.m_bStatic = FALSE;
                    m_sQueueSample.m_nFramesToPlay = 4;
                    SET_SOUND_REVERB(TRUE);
                    SET_SOUND_REFLECTION(FALSE);
                    AddSampleToRequestedQueue();
                }
            }
        }
        return TRUE;
    }
    return FALSE;
}

bool8 cAudioManager::ProcessMultiplayerVehicleFlatTyre(cVehicleParams& params)
{
    bool8 wheelBurst;
    uint8 Vol;
    float modifier;
	sVehicle* pVeh = params.m_pNetVehicle->GetElement().vehicle;

    if (params.m_fDistance < SQR(FLAT_TYRE_MAX_DIST))
    {
        switch (params.m_VehicleType)
        {
            case VEHICLE_TYPE_CAR:
#ifdef FIX_BUGS
            case VEHICLE_TYPE_QUAD:
#endif
                wheelBurst = FALSE;
                for (int32 i = 0; i < 4; i++)
#ifdef FIX_BUGS
                    if (pVeh->GetSync().automobile->Damage.m_wheelStatus[i] == WHEEL_STATUS_BURST && pVeh->GetSync().vehicle->m_aWheelTimer[i] > 0.0f)
#else
                    if (pVeh->GetSync().vehicle->m_aWheelState[i] == WHEEL_STATE_SPINNING && pVeh->GetSync().vehicle->m_aWheelTimer[i] > 0.0f) // what?
#endif
                        wheelBurst = TRUE;
                if (!wheelBurst)
                    return TRUE;
                break;
            case VEHICLE_TYPE_BIKE:
                wheelBurst = FALSE;
                for (int32 i = 0; i < 2; i++)
#ifdef FIX_BUGS
                    if (pVeh->GetSync().bike->m_wheelStatus[i] == WHEEL_STATUS_BURST && pVeh->GetSync().vehicle->m_aWheelTimer[i] > 0.0f)
#else
                    if (pVeh->GetSync().vehicle->m_aWheelState[i] == WHEEL_STATE_SPINNING && pVeh->GetSync().vehicle->m_aWheelTimer[i] > 0.0f) // what?
#endif
                        wheelBurst = TRUE;
                if (!wheelBurst)
                    return TRUE;
                break;
            default:
                return TRUE;
        }
        modifier = Min(1.0f, Abs(params.m_fVelocityChange) / (0.3f * params.m_pTransmission->fMaxVelocity));
        if (modifier > 0.01f)
        {
            Vol = ((float)FLAT_TYRE_VOLUME * modifier);
            CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
            m_sQueueSample.m_nVolume = ComputeVolume(Vol, FLAT_TYRE_MAX_DIST, m_sQueueSample.m_fDistance);
            if (m_sQueueSample.m_nVolume > 0)
            {
                m_sQueueSample.m_nCounter = 95;
                m_sQueueSample.m_nBankIndex = SFX_BANK_0;
                m_sQueueSample.m_bIs2D = FALSE;
                m_sQueueSample.m_nPriority = 5;
                m_sQueueSample.m_nSampleIndex = SFX_TYRE_BURST_L;
                m_sQueueSample.m_nFrequency = (5500.0f * modifier) + 8000;
                m_sQueueSample.m_nLoopCount = 0;
                SET_EMITTING_VOLUME(Vol);
                SET_LOOP_OFFSETS(SFX_TYRE_BURST_L)
                m_sQueueSample.m_fSpeedMultiplier = 2.0f;
                m_sQueueSample.m_MaxDistance = FLAT_TYRE_MAX_DIST;
                m_sQueueSample.m_bStatic = FALSE;
                m_sQueueSample.m_nFramesToPlay = 3;
                SET_SOUND_REVERB(TRUE);
                SET_SOUND_REFLECTION(FALSE);
                AddSampleToRequestedQueue();
            }
        }
        return TRUE;
    }
    return FALSE;
}

bool8 cAudioManager::ProcessMultiplayerVehicleSirenOrAlarm(cVehicleParams& params)
{
	uint8 Vol;
	sVehicle* pVeh = params.m_pNetVehicle->GetElement().vehicle;

	if (params.m_fDistance < SQR(VEHICLE_SIREN_MAX_DIST))
	{
		if (pVeh->GetSync().vehicle->m_bSirenOrAlarm /*|| veh->IsAlarmOn()*/)
		{
			//if (veh->IsAlarmOn())
			//{
			//	if (CTimer::GetTimeInMilliseconds() > veh->m_nCarHornTimer)
			//		veh->m_nCarHornTimer = CTimer::GetTimeInMilliseconds() + 750;

			//	if (veh->m_nCarHornTimer < CTimer::GetTimeInMilliseconds() + 375)
			//		return TRUE;
			//}

			CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
			Vol = pVeh->GetSync().vehicle->bIsDrowning ? VEHICLE_SIREN_VOLUME / 4 : VEHICLE_SIREN_VOLUME;
			m_sQueueSample.m_nVolume = ComputeVolume(Vol, VEHICLE_SIREN_MAX_DIST, m_sQueueSample.m_fDistance);
			if (m_sQueueSample.m_nVolume > 0)
			{
				tVehicleSampleData& sampleData = ((CVehicleModelInfo*)CModelInfo::GetModelInfo(params.m_pNetVehicle->GetModelIndex()))->m_SampleData;
				m_sQueueSample.m_nCounter = 5;
				if (UsesMultiplayerSiren(params))
				{
#ifdef FIX_BUGS
					if (pVeh->GetSync().vehicle->m_nStatus == STATUS_ABANDONED)
						return TRUE;
#else
					if (params.m_pNetVehicle->GetStatus() == STATUS_ABANDONED)
						return TRUE;
#endif

					if (pVeh->GetSync().vehicle->m_nCarHornTimer > 0 && params.m_nIndex != FIRETRUCK && params.m_nIndex != MRWHOOP)
					{
						m_sQueueSample.m_nSampleIndex = SFX_SIREN_FAST;
						if (params.m_nIndex == FBICAR)
							m_sQueueSample.m_nFrequency = 12668;
						else
							m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_SIREN_FAST);
						m_sQueueSample.m_nCounter = 60;
					}
					else if (params.m_nIndex == VICECHEE)
					{
						m_sQueueSample.m_nSampleIndex = SFX_POLICE_SIREN_SLOW;
						m_sQueueSample.m_nFrequency = 11440;
					}
					else
					{
						m_sQueueSample.m_nSampleIndex = sampleData.m_nSirenOrAlarmSample;
						m_sQueueSample.m_nFrequency = sampleData.m_nSirenOrAlarmFrequency;
					}
				}
				else
				{
					m_sQueueSample.m_nSampleIndex = sampleData.m_nHornSample;
					m_sQueueSample.m_nFrequency = sampleData.m_nHornFrequency;
				}
				m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				m_sQueueSample.m_bIs2D = FALSE;
				m_sQueueSample.m_nPriority = 1;
				m_sQueueSample.m_nLoopCount = 0;
				SET_EMITTING_VOLUME(Vol);
				SET_LOOP_OFFSETS(m_sQueueSample.m_nSampleIndex)
					m_sQueueSample.m_fSpeedMultiplier = 7.0f;
				m_sQueueSample.m_MaxDistance = VEHICLE_SIREN_MAX_DIST;
				m_sQueueSample.m_bStatic = FALSE;
				m_sQueueSample.m_nFramesToPlay = 5;
				SET_SOUND_REVERB(TRUE);
				SET_SOUND_REFLECTION(FALSE);
				AddSampleToRequestedQueue();
			}
		}
		return TRUE;
	}
	return FALSE;
}

#ifdef FIX_BUGS
// TODO new VCS vehicle types audio process
// TODO new VCS vehicle types audio process
// TODO new VCS vehicle types audio process
#endif