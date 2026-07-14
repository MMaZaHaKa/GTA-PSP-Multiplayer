/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "common.h"
#include "Fire.h"
#include "Vehicle.h"
#include "Skidmarks.h"

#include "multiplayer/elements/sElementPhysical.h"

enum
{
	NUM_MULTIPLAYER_SYNC_WHEELS = 4,
};

enum eAutoPilotSync
{
	MP_PKTD_VEH_AP_EQUAL	     = 0,
	MP_PKTD_VEH_AP_ROUTE_NODES   = BIT(0), // m_nNextRouteNode, m_nCurrentRouteNode, m_nNextNextRouteNode, m_nPrevRouteNode
	MP_PKTD_VEH_AP_PATH_NODES    = BIT(1), // m_nNextPathNodeInfo, m_nCurrentPathNodeInfo, m_nNextNextPathNodeInfo, m_nPreviousPathNodeInfo + directions
	MP_PKTD_VEH_AP_LANES         = BIT(2), // m_nNextLane, m_nCurrentLane, m_nNextNextLane, m_nPrevLane
	MP_PKTD_VEH_AP_SPEEDS        = BIT(4), // m_nCruiseSpeed, m_nMaxTrafficSpeed
	MP_PKTD_VEH_AP_CAR_MISSION   = BIT(5), // m_nCarMission
	MP_PKTD_VEH_AP_DRIVING_STYLE = BIT(6), // m_nDrivingStyle
	MP_PKTD_VEH_AP_TIME_CURVE	 = BIT(7), // m_nTimeToSpendOnCurrentCurve, m_nTimeEnteredCurve
	MP_PKTD_VEH_AP_FULL          = -1,	   // full diff
};

enum eVehicleSync
{
	MP_PKTD_VEH_EQUAL              = 0,
	MP_PKTD_VEH_STEER_ANGLE        = BIT(0),  // m_fSteerAngle
	MP_PKTD_VEH_GAS_PEDAL          = BIT(1),  // m_fGasPedal
	MP_PKTD_VEH_BRAKE_PEDAL        = BIT(2),  // m_fBrakePedal
	MP_PKTD_VEH_HANDBRAKE_HORN     = BIT(3),  // m_bIsHandbrakeOn, m_nCarHornTimer
	MP_PKTD_VEH_STATUS             = BIT(4),  // m_nStatus
	MP_PKTD_VEH_CHANGE_GEAR_TIME   = BIT(5),  // m_fChangeGearTime
	MP_PKTD_VEH_CURRENT_GEAR_FLAGS = BIT(6),  // m_nCurrentGear, m_nFlags (car jacker, engine on, etc)
	MP_PKTD_VEH_WHEEL0             = BIT(7),  // 0x80
	MP_PKTD_VEH_WHEEL1             = BIT(8),  // 0x100
	MP_PKTD_VEH_WHEEL2             = BIT(9),  // 0x200
	MP_PKTD_VEH_WHEEL3             = BIT(10), // 0x400
	MP_PKTD_VEH_SKIDMARK_FLAGS     = BIT(11), // m_nSkidmarkFlags
	MP_PKTD_VEH_COLORS             = BIT(12), // model index + colors
	MP_PKTD_VEH_PHYSICAL           = BIT(13), // 0x2000
	MP_PKTD_VEH_HEALTH             = BIT(14), // m_fHealth
	MP_PKTD_VEH_AUTOPILOT          = BIT(15), // 0x8000
	MP_PKTD_VEH_DRIVER_ID          = BIT(16), // m_nDriverID
	MP_PKTD_VEH_PLAYER_ID          = BIT(17), // m_nPlayerID
	MP_PKTD_VEH_DRIVER_TEAM        = BIT(18), // m_nDriverTeam
	MP_PKTD_VEH_LAST_DAMAGE        = BIT(19), // m_fLastDamageAmount, m_nLastDamagePlayerID
	MP_PKTD_VEH_SIREN_ALARM        = BIT(20), // m_bSirenOrAlarm
	MP_PKTD_VEH_CREATED_BY         = BIT(21), // VehicleCreatedBy
	MP_PKTD_VEH_DOOR_LOCK          = BIT(22), // m_nDoorLock
	MP_PKTD_VEH_FULL               = -1,	  // full diff
};
#define MP_PKTD_VEH_WHEEL_MASK(wheelIndex) (BIT(7) << (wheelIndex))

enum eWheelSync
{
	MP_PKTD_WHEEL_EQUAL      = 0,
	MP_PKTD_WHEEL_TIMER      = BIT(0),  // m_aWheelTimer
	MP_PKTD_WHEEL_STATE      = BIT(1),  // m_aWheelState
	MP_PKTD_WHEEL_ROTATION   = BIT(2),  // m_aWheelRotation
	MP_PKTD_WHEEL_POSITION   = BIT(3),  // m_aWheelPosition
	MP_PKTD_WHEEL_SPEED      = BIT(4),  // m_aWheelSpeed
	MP_PKTD_WHEEL_SUSPENSION = BIT(5),  // m_aSuspensionSpringRatio
	MP_PKTD_WHEEL_SKIDMARK   = BIT(6),  // m_aWheelSkidmarkType
	MP_PKTD_WHEEL_POINT_X    = BIT(7),  // CColPoint.x (0x80)
	MP_PKTD_WHEEL_POINT_Y    = BIT(8),  // CColPoint.y (0x100)
	MP_PKTD_WHEEL_POINT_Z    = BIT(9),  // CColPoint.z (0x200)
	MP_PKTD_WHEEL_NORMAL_X   = BIT(10), // CColPoint.normal.x (0x400)
	MP_PKTD_WHEEL_NORMAL_Y   = BIT(11), // CColPoint.normal.y (0x800)
	MP_PKTD_WHEEL_NORMAL_Z   = BIT(12), // CColPoint.normal.z (0x1000)
	MP_PKTD_WHEEL_POINT_W    = BIT(13), // CColPoint.point.w (0x2000)
	MP_PKTD_WHEEL_SURFACE_A  = BIT(14), // CColPoint.surfaceA (0x4000)
	MP_PKTD_WHEEL_PIECE_A    = BIT(15), // CColPoint.pieceA (0x8000)
	MP_PKTD_WHEEL_SURFACE_B  = BIT(16), // CColPoint.surfaceB (0x10000)
	MP_PKTD_WHEEL_PIECE_B    = BIT(17), // CColPoint.pieceB (0x20000)
	MP_PKTD_WHEEL_FULL       = -1,	    // full diff
};

// TODO: LCS compat structs

struct sAutoPilotSync {
	// Node Info
	uint32 m_nNextPathNodeInfo;
	uint32 m_nCurrentPathNodeInfo;
#ifndef GTA_LIBERTY
	uint32 m_nPreviousPathNodeInfo;
	uint32 m_nNextNextPathNodeInfo;
#endif

	int32 m_nTimeToSpendOnCurrentCurve;
	int32 m_nTimeEnteredCurve;

	// Route Node
	int32 m_nNextRouteNode;
	int32 m_nCurrentRouteNode;
#ifndef GTA_LIBERTY
	int32 m_nPrevRouteNode;
	int32 m_nNextNextRouteNode;
#endif

	int32 field_28; // ?

	// Lane
	int8 m_nNextLane;
	int8 m_nCurrentLane;
#ifndef GTA_LIBERTY
	int8 m_nPrevLane;
	int8 m_nNextNextLane;
#endif

	// Direction
	int8 m_nNextDirection;
	int8 m_nNextNextDirection;
#ifndef GTA_LIBERTY
	int8 m_nCurrentDirection;
	int8 m_nPreviousDirection;
#endif

	uint8 m_nCruiseSpeed;
	uint8 m_nMaxTrafficSpeed; // AP float
	uint8 m_nCarMission;
	uint8 m_nDrivingStyle;

	inline sAutoPilotSync() {}
	sAutoPilotSync(CAutoPilot* pAutoPilot);
	sAutoPilotSync(const sAutoPilotSync& other); // inlined

#if !defined(FINAL) && !defined(MASTER)
	void Dump();
#endif
};

struct sVehicleSync : sElementPhysicalSync {
public:
	bool m_bSirenOrAlarm;
	uint8 field_B1[3]; // pad
	float m_fSteerAngle;
	float m_fGasPedal;
	float m_fBrakePedal;
	bool m_bIsHandbrakeOn;
	int8 m_nCarJacker; // bool?
	uint8 m_nStatus;
	uint8 m_nCarHornTimer;
	uint8 m_nCurrentGear;
	uint8 field_C5[3]; // pad
	float m_fChangeGearTime;

	// Flags 0xCC
	uint8 bHasDriver : 1;
	uint8 bIsDrivenByPlayer : 1;
	uint8 bEngineOn : 1;
	uint8 bIsDrowning : 1; // old bIsWreked
	uint8 bCC_10 : 1; // unused
	uint8 bCC_20 : 1; // unused
	uint8 bCC_40 : 1; // unused
	uint8 bCC_80 : 1; // unused

	int8 m_nDriverTeam;
	int8 m_nDriverID;
	int8 m_nPlayerID;
	int8 m_nLastDriverID;
	uint8 m_aWheelTimer[NUM_MULTIPLAYER_SYNC_WHEELS]; // set to 4.0 when wheel is touching ground, then decremented
	uint8 field_D5[3]; // pad
	tWheelState m_aWheelState[NUM_MULTIPLAYER_SYNC_WHEELS];
	float m_aWheelRotation[NUM_MULTIPLAYER_SYNC_WHEELS];
	float m_aWheelPosition[NUM_MULTIPLAYER_SYNC_WHEELS];
	float m_aWheelSpeed[NUM_MULTIPLAYER_SYNC_WHEELS];
	float m_aSuspensionSpringRatio[NUM_MULTIPLAYER_SYNC_WHEELS];
	CRGBA m_aColours[NUM_VEHICLE_COLOURS]; // guessed
	CColPoint m_aWheelColPoints[NUM_MULTIPLAYER_SYNC_WHEELS];
	eSkidmarkType m_aWheelSkidmarkType[NUM_MULTIPLAYER_SYNC_WHEELS];
	uint8 m_nSkidmarkFlags; // 1111 SkidmarkUnk(HI)   1111 SkidmarkBloody(LO)
	uint8 field_1C1[3]; // pad?
	int32 field_1C4;
	float m_fHealth;
	float m_fLastDamageAmount;
	uint8 m_nLastDamagePlayerID; // old m_nDamagedByPeerID
	uint8 field_1D1[3]; // pad?
	sAutoPilotSync AutoPilot;
	uint8 VehicleCreatedBy;
	uint8 field_20D[3]; // pad?
	eCarLock m_nDoorLock;
	int32 field_214; // m_aColours[2]?
	int32 field_218;
	int32 field_21C;

	sVehicleSync();
	sVehicleSync(CVehicle* pVeh);
	sVehicleSync(const sVehicleSync& other); // inlined
	~sVehicleSync() override;

	bool Compare(const sVehicleSync& other);
	bool HasDriver() { return bHasDriver; }

#if !defined(FINAL) && !defined(MASTER)
	void Dump();
#endif
};

#pragma pack(push, 1)
struct tVehicleSyncsDeltas
{
	uint32 nVehicleDiff;   // eVehicleSync
	uint32 nPhysicalDiff;  // ePhysicalSync
	uint32 nAutoPilotDiff; // eAutoPilotSync
	uint32 aWheelDiff[NUM_MULTIPLAYER_SYNC_WHEELS];  // eWheelSync

	inline void SetEqual()
	{
		memset(this, eVehicleSync::MP_PKTD_VEH_EQUAL, sizeof(tVehicleSyncsDeltas));
		//nVehicleDiff = eVehicleSync::MP_PKTD_VEH_EQUAL;
		//nPhysicalDiff = ePhysicalSync::MP_PKTD_PHY_EQUAL;
		//nAutoPilotDiff = eAutoPilotSync::MP_PKTD_VEH_AP_EQUAL;
		//for (uint32 i = 0; i < NUM_MULTIPLAYER_SYNC_WHEELS; i++)
		//	aWheelDiff[i] = eWheelSync::MP_PKTD_WHEEL_EQUAL;
	}

	inline void SetDifference()
	{
		memset(this, static_cast<uint32>(eVehicleSync::MP_PKTD_VEH_FULL), sizeof(tVehicleSyncsDeltas)); // 0xFFFFFFFF
		//nVehicleDiff = eVehicleSync::MP_PKTD_VEH_FULL;
		//nPhysicalDiff = ePhysicalSync::MP_PKTD_PHY_FULL;
		//nAutoPilotDiff = eAutoPilotSync::MP_PKTD_VEH_AP_FULL;
		//for (uint32 i = 0; i < NUM_MULTIPLAYER_SYNC_WHEELS; i++)
		//	aWheelDiff[i] = eWheelSync::MP_PKTD_WHEEL_FULL;
	}
};
static_assert(sizeof(tVehicleSyncsDeltas) == 28, "sizeof(tVehicleSyncsDeltas)");
#pragma pack(pop)

class cVehicleMG : public cPhysicalMG {
public:
	CFire* m_pFire;
	float m_fVelocityChangeForAudio;
	float m_fGasPedalAudio;
	eVehicleType m_vehType;

	cVehicleMG(sElement* elem);

	~cVehicleMG() override;
	void SetModelIndex(uint32 id) override;
	void PreRender(void) override;
	void Render(void) override;
	bool SetupLighting(void) override;
	void RemoveLighting(bool reset) override;

	void SetComponentAtomicAlpha(RpAtomic* atomic, int32 alpha);
	int32 AddWheelDirtAndWater(CColPoint* colpoint, uint32 belowEffectSpeed);

	inline bool IsCar(void) { return m_vehType == eVehicleType::VEHICLE_TYPE_CAR; }
	inline bool IsBoatOnly(void) { return m_vehType == eVehicleType::VEHICLE_TYPE_BOAT; }
	inline bool IsJetski(void) { return m_vehType == eVehicleType::VEHICLE_TYPE_JETSKI; }
	inline bool IsBoat(void) { return m_vehType == eVehicleType::VEHICLE_TYPE_BOAT || IsJetski(); }
	inline bool IsTrain(void) { return m_vehType == eVehicleType::VEHICLE_TYPE_TRAIN; }
	inline bool IsHeli(void) { return m_vehType == eVehicleType::VEHICLE_TYPE_HELI; }
	inline bool IsPlane(void) { return m_vehType == eVehicleType::VEHICLE_TYPE_PLANE; }
	inline bool IsBike(void) { return m_vehType == eVehicleType::VEHICLE_TYPE_BIKE; }
	inline bool IsFerry(void) { return m_vehType == eVehicleType::VEHICLE_TYPE_FERRY; }
	inline bool IsBmx(void) { return m_vehType == eVehicleType::VEHICLE_TYPE_BMX; }
	inline bool IsQuad(void) { return m_vehType == eVehicleType::VEHICLE_TYPE_QUAD; }
};

class CPed;

struct sVehicle : sElementPhysical {
public:
	static int32 ms_nNumberOfSyncedVehicles;

	CPed* m_pPed;
	uint8 field_80[4]; // pad?
#ifdef GTA_LIBERTY
	uint8 m_currentColour1;
	uint8 m_currentColour2;
#else
	CRGBA m_aColours[NUM_VEHICLE_COLOURS]; // 0x84 m_currentColour, vcs alpha read 0
#endif
	float m_aSuspensionSpringLength[NUM_MULTIPLAYER_SYNC_WHEELS];
	float m_aSuspensionLineLength[NUM_MULTIPLAYER_SYNC_WHEELS];
	float m_fTraction;

	sVehicle();
//#ifdef FIX_BUGS
//	sVehicle(CVehicle* pVehicle); // cant fixed because m_aSuspensionLineLength and etc in child structs CVehicle, so do it manual in every ctor's :/
//#endif

	static ElementCapability Capability() { return reinterpret_cast<ElementCapability>(sVehicle::Capability); }

	ElementCapability GetCapability() override;
	bool HasCapability(ElementCapability capability) override;
	~sVehicle() override;
	void ApplyClientSync(uint16 nTime) override;
	void ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) override;
	void UpdateDelta(sElementSync* pSync, uint16 nTimeDelta) override;
	bool IsTransferable(void) override;
	void TransferEntity(int16 nDestPlayer) override;
#ifndef GTA_LIBERTY
	virtual void Initialise();
#endif
	virtual void SetupModel() = 0; // old name SetupModelNodes, but its complex setup like vehicle ctor

	bool TransferOccupants(int16 nDestPlayer);
	void SetComponentVisibility(RwFrame* frame, uint32 flags);

	// Vehicle main
	void CompareSyncState(sVehicleSync* pSync, sVehicleSync* pLastSync, uint32 nDelta, tVehicleSyncsDeltas* pDiff);
	void PerformWriteSync(sWriteSyncStream* pSyncStream, sVehicleSync* pSync, tVehicleSyncsDeltas* pDiff);
	void ReadSyncFromStreamVehicle(sReadSyncStream* pSyncStream, sVehicleSync* pOutSync);

	// AutoPilot
	uint8 CompareAutoPilotSyncState(sAutoPilotSync* pSync, sAutoPilotSync* pLastSync);
	void PerformWriteAutoPilotSync(sWriteSyncStream* pSyncStream, sAutoPilotSync* pSync, uint8 nDiffMask);
	void ReadAutoPilotSyncFromStream(sReadSyncStream* pSyncStream, sAutoPilotSync* pOutSync);

	// Wheel
	uint32 CompareWheelSyncState(sVehicleSync* pSync, sVehicleSync* pLastSync, uint32 nWheel);
	void PerformWriteWheelSync(sWriteSyncStream* pSyncStream, sVehicleSync* pSync, uint32 nWheel, uint32 nDiffMask);
	void ReadWheelSyncFromStream(sReadSyncStream* pSyncStream, sVehicleSync* pOutSync, uint32 nWheel);
};
