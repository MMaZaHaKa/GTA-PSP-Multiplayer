/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "multiplayer/elements/sVehicle.h"
#include "Automobile.h"

#ifndef GTA_LIBERTY
enum eAutomobileBaseSync
{
	MP_PKTD_AUTOBASE_EQUAL                 = 0,
	MP_PKTD_AUTOBASE_VEHICLE               = BIT(0),  // sVehicleSync
	MP_PKTD_AUTOBASE_DOOR_ANGLE_0          = BIT(1),  // Doors[0].m_fAngle
	MP_PKTD_AUTOBASE_DOOR_ANGLE_1          = BIT(2),  // Doors[1].m_fAngle
	MP_PKTD_AUTOBASE_DOOR_ANGLE_2          = BIT(3),  // Doors[2].m_fAngle
	MP_PKTD_AUTOBASE_DOOR_ANGLE_3          = BIT(4),  // Doors[3].m_fAngle
	MP_PKTD_AUTOBASE_DOOR_ANGLE_4          = BIT(5),  // Doors[4].m_fAngle
	MP_PKTD_AUTOBASE_DOOR_ANGLE_5          = BIT(6),  // Doors[5].m_fAngle
	MP_PKTD_AUTOBASE_DOOR_ANGLE_VELOCITY_0 = BIT(7),  // Doors[0].m_fAngVel
	MP_PKTD_AUTOBASE_DOOR_ANGLE_VELOCITY_1 = BIT(8),  // Doors[1].m_fAngVel
	MP_PKTD_AUTOBASE_DOOR_ANGLE_VELOCITY_2 = BIT(9),  // Doors[2].m_fAngVel
	MP_PKTD_AUTOBASE_DOOR_ANGLE_VELOCITY_3 = BIT(10), // Doors[3].m_fAngVel
	MP_PKTD_AUTOBASE_DOOR_ANGLE_VELOCITY_4 = BIT(11), // Doors[4].m_fAngVel
	MP_PKTD_AUTOBASE_DOOR_ANGLE_VELOCITY_5 = BIT(12), // Doors[5].m_fAngVel
	MP_PKTD_AUTOBASE_UNKNOWN               = BIT(13), // field_220
	MP_PKTD_AUTOBASE_FULL                  = -1,      // full diff
};
#define MP_PKTD_AUTOBASE_DOOR_ANGLE_MASK(doorIndex) (BIT(1) << (doorIndex))
#define MP_PKTD_AUTOBASE_DOOR_ANGLE_VELOCITY_MASK(doorIndex) (BIT(7) << (doorIndex))

struct sDoor {
	float m_fMaxAngle;
	float m_fMinAngle;
	// direction of rotation for air resistance
	int8 m_nDirn;
	// axis in which this door rotates
	int8 m_nAxis;
	int8 m_nDoorState; // eDoorState
	int8 field_B; // pad

	sDoor()
	{
		m_nDirn = 0;
		m_fMaxAngle = 0.0f;
		m_fMinAngle = 0.0f;
		m_nAxis = 0;
#ifdef FIX_BUGS
		m_nDoorState = eDoorState::DOORST_SWINGING;
#endif
	}

	inline void Init(float minAngle, float maxAngle, int8 dir, int8 axis)
	{
		m_fMinAngle = minAngle;
		m_fMaxAngle = maxAngle;
		m_nDirn = dir;
		m_nAxis = axis;
	}
};

struct sDoorSync {
	float m_fAngle;
	float m_fPrevAngle;
	float m_fAngVel;
	uint8 field_C[4]; // pad
	CVector m_vecSpeed;

	sDoorSync();
	sDoorSync(const sDoorSync& other);

	bool Compare(const sDoorSync& other);

#if !defined(FINAL) && !defined(MASTER)
	void Dump();
#endif
};

struct sAutomobileBaseSync : sVehicleSync
{
public:

	uint16 field_220; // used, unknown
	uint8 field_222[14]; // pad
	sDoorSync Doors[NUM_DOORS];

	sAutomobileBaseSync();
	sAutomobileBaseSync(CAutomobile* pAutomobile);
	sAutomobileBaseSync(const sAutomobileBaseSync& other);
	~sAutomobileBaseSync() override;

	bool Compare(const sAutomobileBaseSync& other); // inlined

#if !defined(FINAL) && !defined(MASTER)
	void Dump();
#endif
};

#pragma pack(push, 1)
struct tAutomobileBaseSyncsDeltas
{
	uint32 nAutomobileBaseDiff;   // eAutomobileBaseSync
	tVehicleSyncsDeltas tVehicleDiff;

	inline void SetEqual()
	{
		tVehicleDiff.SetEqual(); // useless
		//nAutomobileBaseDiff = eAutomobileBaseSync::MP_PKTD_AUTOBASE_EQUAL;
		memset(this, eAutomobileBaseSync::MP_PKTD_AUTOBASE_EQUAL, sizeof(tAutomobileBaseSyncsDeltas));
	}

	inline void SetDifference()
	{
		tVehicleDiff.SetDifference(); // useless
		//nAutomobileBaseDiff = eAutomobileBaseSync::MP_PKTD_AUTOBASE_FULL;
		memset(this, static_cast<uint32>(eAutomobileBaseSync::MP_PKTD_AUTOBASE_FULL), sizeof(tAutomobileBaseSyncsDeltas)); // 0xFFFFFFFF
	}
};
static_assert(sizeof(tAutomobileBaseSyncsDeltas) == 32, "sizeof(tAutomobileBaseSyncsDeltas)");
#pragma pack(pop)


struct cAutomobileBaseMG : public cVehicleMG {
	cAutomobileBaseMG(sElement* elem);

	~cAutomobileBaseMG() override;
	void PreRender(void) override;
	void Render(void) override;

	void PreRenderWheels(sAutomobileBaseSync* pSync, CVehicleModelInfo* pModelInfo);
};


struct sAutomobileBase : sVehicle {
public:
	RwFrame* m_aCarNodes[MAX_CAR_NODES];
	sDoor Doors[NUM_DOORS];

	sAutomobileBase();

	static ElementCapability Capability() { return reinterpret_cast<ElementCapability>(sAutomobileBase::Capability); }

	ElementCapability GetCapability() override;
	bool HasCapability(ElementCapability capability) override;
	~sAutomobileBase() override;
	void ApplyClientSync(uint16 time) override;
	void ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) override;
	bool IsTransferable(void) override;
	void TransferEntity(int16 nDestPlayer) override;
	void Initialise() override;
	void SetupModel() override;

	void CompareSyncState(sAutomobileBaseSync* pSync, sAutomobileBaseSync* pLastSync, uint32 nDelta, tAutomobileBaseSyncsDeltas* pDiff);
	void PerformWriteSync(sWriteSyncStream* pSyncStream, sAutomobileBaseSync* pSync, tAutomobileBaseSyncsDeltas* pDiff);
	void ReadSyncFromStreamAutomobileBase(sReadSyncStream* pSyncStream, sAutomobileBaseSync* pOutSync);

	void SetupDoors();
	void PreRenderDoor(sAutomobileBaseSync* pSync, eCarNodes component, eDoors nDoor);
};
#endif
