/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#ifdef GTA_LIBERTY
#include "multiplayer/elements/sVehicle.h"
#else
#include "multiplayer/elements/sAutomobileBase.h"
#endif
#include "Automobile.h"

class CObject;

enum eAutomobileSync
{
	MP_PKTD_AUTO_EQUAL                 = 0,
	MP_PKTD_AUTO_DAMAGE                = BIT(0),  // Damage
	//MP_PKTD_AUTO_DOOR_ANGLE_0          = BIT(1),  // Doors[0].m_fAngle
	//MP_PKTD_AUTO_DOOR_ANGLE_1          = BIT(2),  // Doors[1].m_fAngle
	//MP_PKTD_AUTO_DOOR_ANGLE_2          = BIT(3),  // Doors[2].m_fAngle
	//MP_PKTD_AUTO_DOOR_ANGLE_3          = BIT(4),  // Doors[3].m_fAngle
	//MP_PKTD_AUTO_DOOR_ANGLE_4          = BIT(5),  // Doors[4].m_fAngle
	//MP_PKTD_AUTO_DOOR_ANGLE_5          = BIT(6),  // Doors[5].m_fAngle
	//MP_PKTD_AUTO_DOOR_ANGLE_VELOCITY_0 = BIT(7),  // Doors[0].m_fAngVel
	//MP_PKTD_AUTO_DOOR_ANGLE_VELOCITY_1 = BIT(8),  // Doors[1].m_fAngVel
	//MP_PKTD_AUTO_DOOR_ANGLE_VELOCITY_2 = BIT(9),  // Doors[2].m_fAngVel
	//MP_PKTD_AUTO_DOOR_ANGLE_VELOCITY_3 = BIT(10), // Doors[3].m_fAngVel
	//MP_PKTD_AUTO_DOOR_ANGLE_VELOCITY_4 = BIT(11), // Doors[4].m_fAngVel
	//MP_PKTD_AUTO_DOOR_ANGLE_VELOCITY_5 = BIT(12), // Doors[5].m_fAngVel
	MP_PKTD_AUTO_AUTOBASE              = BIT(13), // sAutomobileBaseSync
	MP_PKTD_AUTO_CAR_GUN_LR            = BIT(14), // m_fCarGunLR
	MP_PKTD_AUTO_FULL                  = -1,      // full diff
};

enum eDamageSync
{
	MP_PKTD_DAM_EQUAL                = 0,
	MP_PKTD_DAM_WHEEL_DAMAGE_EFFECT  = BIT(0), // m_fWheelDamageEffect
	MP_PKTD_DAM_WHEEL_STATUS_LEFT    = BIT(1), // m_wheelStatus[CARWHEEL_FRONT_LEFT], m_wheelStatus[CARWHEEL_REAR_LEFT]
	MP_PKTD_DAM_WHEEL_STATUS_RIGHT   = BIT(2), // m_wheelStatus[CARWHEEL_FRONT_RIGHT], m_wheelStatus[CARWHEEL_REAR_RIGHT]
	MP_PKTD_DAM_DOOR_STATUS_BON_BOOT = BIT(3), // m_doorStatus[DOOR_BONNET], m_doorStatus[DOOR_BOOT]
	MP_PKTD_DAM_DOOR_STATUS_FRONT    = BIT(4), // m_doorStatus[DOOR_FRONT_LEFT], m_doorStatus[DOOR_FRONT_RIGHT]
	MP_PKTD_DAM_DOOR_STATUS_REAR     = BIT(5), // m_doorStatus[DOOR_REAR_LEFT], m_doorStatus[DOOR_REAR_RIGHT]
	MP_PKTD_DAM_LIGHT_STATUS         = BIT(6), // m_lightStatus
	MP_PKTD_DAM_PANEL_STATUS         = BIT(7), // m_panelStatus
	MP_PKTD_DAM_FULL                 = -1,     // full diff
};

struct sDamageManager {
	float m_fWheelDamageEffect;
	uint8 m_wheelStatus[NUM_CARWHEELS]; // eWheelStatus // index eCarWheel
	uint8 m_doorStatus[NUM_DOORS];      // eDoorStatus  // index eDoors
	bool m_bSmashedDoorDoesntClose;          // unshure, probably pad
	uint8 m_engineStatus;   // eEngineStatus // unshure, probably pad
	uint32 m_lightStatus;   // eLightStatus
	uint32 m_panelStatus;   // ePanelStatus

	sDamageManager();
	sDamageManager(CDamageManager* damage); // inlined
	sDamageManager(const sDamageManager& other);

	bool Compare(const sDamageManager& other);

#if !defined(FINAL) && !defined(MASTER)
	void Dump();
#endif

	inline void ResetDamageStatus() { memset(this, 0, sizeof(sDamageManager)); }
	inline void SetDoorStatus(eDoors door, eDoorStatus status) { m_doorStatus[door] = status; }
	inline eDoorStatus GetDoorStatus(eDoors door) { return (eDoorStatus)m_doorStatus[door]; }
	inline void SetPanelStatus(int32 panel, uint32 status) { m_panelStatus = dpb(status, panel * 4, 4, m_panelStatus); }
	inline int32 GetPanelStatus(int32 panel) { return ldb(panel * 4, 4, m_panelStatus); }
	inline void SetWheelStatus(eCarWheel wheel, eWheelStatus status) { m_wheelStatus[wheel] = status; }
	inline eWheelStatus GetWheelStatus(eCarWheel wheel) { return (eWheelStatus)m_wheelStatus[wheel]; }
};

#ifdef GTA_LIBERTY
struct sAutomobileSync : sVehicleSync
#else
struct sAutomobileSync : sAutomobileBaseSync
#endif
{
public:
	sDamageManager Damage;
	float m_fCarGunLR;
	int32 field_30C;

	sAutomobileSync();
	sAutomobileSync(CAutomobile* pAutomobile);
	sAutomobileSync(const sAutomobileSync& other);
	~sAutomobileSync() override;

	bool Compare(const sAutomobileSync& other); // inlined

#if !defined(FINAL) && !defined(MASTER)
	void Dump();
#endif

#ifdef MULTIGAME_ELEMENTS_IMPROVEMENTS
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_AUTOMOBILE; }
#endif
};

#pragma pack(push, 1)
struct tAutomobileSyncsDeltas
{
	uint32 nAutomobileDiff; // eAutomobileSync
	uint32 nDamageDiff;     // eDamageSync
	tAutomobileBaseSyncsDeltas tAutomobileBaseDiff;

	inline void SetEqual()
	{
		tAutomobileBaseDiff.SetEqual(); // useless
		//nAutomobileDiff = eAutomobileSync::MP_PKTD_AUTO_EQUAL;
		//nDamageDiff = eDamageSync::MP_PKTD_DAM_EQUAL;
		memset(this, eAutomobileSync::MP_PKTD_AUTO_EQUAL, sizeof(tAutomobileSyncsDeltas));
	}

	inline void SetDifference()
	{
		tAutomobileBaseDiff.SetDifference(); // useless
		//nAutomobileDiff = eAutomobileSync::MP_PKTD_AUTO_FULL;
		//nDamageDiff = eDamageSync::MP_PKTD_DAM_EQUAL;
		memset(this, static_cast<uint32>(eAutomobileSync::MP_PKTD_AUTO_FULL), sizeof(tAutomobileSyncsDeltas)); // 0xFFFFFFFF
	}
};
static_assert(sizeof(tAutomobileSyncsDeltas) == 40, "sizeof(tAutomobileSyncsDeltas)");
#pragma pack(pop)


#ifdef GTA_LIBERTY
class cAutomobileMG : public cVehicleMG
#else
struct cAutomobileMG : public cAutomobileBaseMG
#endif
{
public:

	cAutomobileMG(sElement* elem);

	~cAutomobileMG() override;
	void PreRender(void) override;
	void Render(void) override;
	void AddDamagedVehicleParticles(void);
};

#ifdef GTA_LIBERTY
struct sAutomobile : sVehicle
#else
struct sAutomobile : sAutomobileBase
#endif
{
public:
	sDamageManager Damage;
	float m_aSuspensionSpringRatio[4];
	uint32 m_nHandlingFlags;

	sAutomobile();
	sAutomobile(CAutomobile* pAutomobile);

	static ElementCapability Capability() { return reinterpret_cast<ElementCapability>(sAutomobile::Capability); }

	ElementCapability GetCapability() override;
	bool HasCapability(ElementCapability capability) override;
	~sAutomobile() override;
	sElementSync* CreateSync() override;
	void DisposeSync(sElementSync* pSync) override;
	sElementSync* CreateSyncFromOther(sElementSync* pSync) override;
	bool HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) override;
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_AUTOMOBILE; }
	void ApplyClientSync(uint16 nTime) override;
	void Update(uint16 nTime) override;
	bool WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) override;
	void ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) override;
	void ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) override;
	void SetupModel() override;

	// Automobile main
	void CompareSyncState(sAutomobileSync* pSync, sAutomobileSync* pLastSync, uint32 nDelta, tAutomobileSyncsDeltas* pDiff);
	void PerformWriteSync(sWriteSyncStream* pSyncStream, sAutomobileSync* pSync, tAutomobileSyncsDeltas* pDiff);
	bool WriteSyncDelta(sWriteSyncStream* pSyncStream, sAutomobileSync* pSync, sAutomobileSync* pLastSync, uint32 nDelta);

	// Damage
	uint8 CompareDamageSyncState(sDamageManager* pSync, sDamageManager* pLastSync);
	void PerformWriteDamageSync(sWriteSyncStream* pSyncStream, sDamageManager* pSync, uint8 nDiffMask);
	void ReadDamageSyncFromStream(sReadSyncStream* pSyncStream, sDamageManager* pOutSync);

	void SetupDamage();
	void Fix();
	void SetPanelDamage(sAutomobileSync* pSync, eCarNodes component, ePanels panel);
	void SetBumperDamage(sAutomobileSync* pSync, eCarNodes component, ePanels panel);
	void SetDoorDamage(sAutomobileSync* pSync, eCarNodes component, eDoors door);
	CObject* SpawnFlyingComponent(eCarNodes component, uint32 type, int32 unused, CVector vecMoveSpeed, CVector vecTurnSpeed, bool bRenderScorched);
};