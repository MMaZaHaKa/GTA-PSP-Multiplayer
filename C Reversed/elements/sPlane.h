/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "common.h"

#include "multiplayer/elements/sAutomobile.h"
#ifndef GTA_LIBERTY
#include "Plane.h"

enum ePlaneSync
{
	MP_PKTD_PLANE_EQUAL               = 0,
	MP_PKTD_PLANE_AUTOBASE            = BIT(0), // sAutomobileBaseSync
	MP_PKTD_PLANE_ATTACK_LIFT         = BIT(1), // m_fAttackLift
	MP_PKTD_PLANE_GEAR_DOWN_LIFT_MULT = BIT(2), // m_fGearDownLiftMult
	MP_PKTD_PLANE_ENGINE_SPEED        = BIT(3), // m_fEngineSpeed
	MP_PKTD_PLANE_YAW_CONTROL         = BIT(4), // m_fYawControl
	MP_PKTD_PLANE_PITCH_CONTROL       = BIT(5), // m_fPitchControl
	MP_PKTD_PLANE_ROLL_CONTROL        = BIT(6), // m_fRollControl
	MP_PKTD_PLANE_THROTTLE_CONTROL    = BIT(7), // m_fThrottleControl
	MP_PKTD_PLANE_LGEAR_ANGLE         = BIT(8), // m_fLGearAngle
	MP_PKTD_PLANE_FULL                = -1,     // full diff
};

struct sPlaneSync : sAutomobileBaseSync
{
public:
	float m_fAttackLift;
	float m_fGearDownLiftMult;
	float m_fEngineSpeed;
	float m_fYawControl;
	float m_fPitchControl;
	float m_fRollControl;
	float m_fThrottleControl;
	float m_fLGearAngle;

	sPlaneSync();
	sPlaneSync(CPlane* pPlane);
	sPlaneSync(const sPlaneSync& other);
	~sPlaneSync() override;

	bool Compare(const sPlaneSync& other); // inlined

#if !defined(FINAL) && !defined(MASTER)
	void Dump();
#endif

#ifdef MULTIGAME_ELEMENTS_IMPROVEMENTS
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_PLANE; }
#endif
};

#pragma pack(push, 1)
struct tPlaneSyncsDeltas
{
	uint32 nPlaneDiff; // ePlaneSync
	tAutomobileBaseSyncsDeltas tAutomobileBaseDiff;

	inline void SetEqual()
	{
		tAutomobileBaseDiff.SetEqual(); // useless
		nPlaneDiff = ePlaneSync::MP_PKTD_PLANE_EQUAL;
		memset(this, ePlaneSync::MP_PKTD_PLANE_EQUAL, sizeof(tPlaneSyncsDeltas));
	}

	inline void SetDifference()
	{
		tAutomobileBaseDiff.SetDifference(); // useless
		nPlaneDiff = ePlaneSync::MP_PKTD_PLANE_FULL;
		memset(this, static_cast<uint32>(ePlaneSync::MP_PKTD_PLANE_FULL), sizeof(tPlaneSyncsDeltas)); // 0xFFFFFFFF
	}
};
static_assert(sizeof(tPlaneSyncsDeltas) == 36, "sizeof(tPlaneSyncsDeltas)");
#pragma pack(pop)

class cPlaneMG : public cAutomobileBaseMG {
public:
	float m_fPropellerAngle;

	cPlaneMG(sElement* elem);

	~cPlaneMG() override;
	void PreRender(void) override;
	void Render(void) override;

	void DoHeliDustEffect(void);
};


struct sPlane : sAutomobileBase {
public:

	sPlane();
	sPlane(CPlane* pPlane);

	static ElementCapability Capability() { return reinterpret_cast<ElementCapability>(sPlane::Capability); }

	ElementCapability GetCapability() override;
	bool HasCapability(ElementCapability capability) override;
	~sPlane() override;
	sElementSync* CreateSync() override;
	void DisposeSync(sElementSync* pSync) override;
	sElementSync* CreateSyncFromOther(sElementSync* pSync) override;
	bool HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) override;
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_PLANE; }
	void ApplyClientSync(uint16 nTime) override;
	void Update(uint16 nTime) override;
	bool WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) override;
	void ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) override;
	void ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) override;
	void Initialise() override;
	void SetupModel() override;

	void CompareSyncState(sPlaneSync* pSync, sPlaneSync* pLastSync, uint32 nDelta, tPlaneSyncsDeltas* pDiff);
	void PerformWriteSync(sWriteSyncStream* pSyncStream, sPlaneSync* pSync, tPlaneSyncsDeltas* pDiff);
	bool WriteSyncDelta(sWriteSyncStream* pSyncStream, sPlaneSync* pSync, sPlaneSync* pLastSync, uint32 nDelta);

#ifdef FIX_BUGS
	void Fix();
#endif
};
#endif
