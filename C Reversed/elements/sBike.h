/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "common.h"
#include "Bike.h"

#include "multiplayer/elements/sVehicle.h"

enum eBikeSync
{
	MP_PKTD_BIKE_EQUAL           = 0,
	MP_PKTD_BIKE_VEHICLE	     = BIT(0), // sVehicleSync
	MP_PKTD_BIKE_BAR_STEER_ANGLE = BIT(1), // m_fBarSteerAngle // руль
	MP_PKTD_BIKE_WHEEL_STATUS_0  = BIT(2), // m_wheelStatus[0]
	MP_PKTD_BIKE_WHEEL_STATUS_1  = BIT(3), // m_wheelStatus[1]
	MP_PKTD_BIKE_LEAN_ANGLE      = BIT(4), // m_fLeanAngle // лево/право
	MP_PKTD_BIKE_FULL               = -1,  // full diff
};

struct sBikeSync : sVehicleSync
{
public:
	float m_fBarSteerAngle;
	float m_fLeanAngle;
	uint8 m_wheelStatus[2];
	uint8 field_22A[2]; // pad
	int32 field_22C;

	sBikeSync();
	sBikeSync(CBike* bike);
	sBikeSync(const sBikeSync& other);
	~sBikeSync() override;

	bool Compare(const sBikeSync& other); // inlined

#if !defined(FINAL) && !defined(MASTER)
	void Dump();
#endif

#ifdef MULTIGAME_ELEMENTS_IMPROVEMENTS
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_BIKE; }
#endif
};

#pragma pack(push, 1)
struct tBikeSyncsDeltas/* : tVehicleSyncsDeltas*/
{
	tVehicleSyncsDeltas tVehicleDiff; // It's similar to inheritance, but in sBmx the fields are the other way around, so manual diff
	uint32 nBikeDiff;   // eBikeSync

	inline void SetEqual()
	{
		tVehicleDiff.SetEqual(); // useless
		//nBikeDiff = eBikeSync::MP_PKTD_BIKE_EQUAL;
		memset(this, eBikeSync::MP_PKTD_BIKE_EQUAL, sizeof(tBikeSyncsDeltas));
	}

	inline void SetDifference()
	{
		tVehicleDiff.SetDifference(); // useless
		//nBikeDiff = eBikeSync::MP_PKTD_BIKE_FULL;
		memset(this, static_cast<uint32>(eBikeSync::MP_PKTD_BIKE_FULL), sizeof(tBikeSyncsDeltas)); // 0xFFFFFFFF
	}
};
static_assert(sizeof(tBikeSyncsDeltas) == 32, "sizeof(tBikeSyncsDeltas)");
#pragma pack(pop)

class cBikeMG : public cVehicleMG {
public:

	cBikeMG(sElement* elem);

	~cBikeMG() override;
	void PreRender(void) override;
//#ifdef FIX_BUGS
//	void Render(void) override;
//#endif
};


struct sBike : sVehicle {
public:
	float m_aSuspensionSpringRatio[4];
	RwFrame* m_aBikeNodes[MAX_BIKE_NODES];
	float m_aWheelBasePosition[2];
	float m_fRearForkLength;
	float m_fFrontForkY;
	float m_fFrontForkZ;
	float m_fFrontForkSlope;


	sBike();
	sBike(CBike* pBike);

	static ElementCapability Capability() { return reinterpret_cast<ElementCapability>(sBike::Capability); }

	ElementCapability GetCapability() override;
	bool HasCapability(ElementCapability capability) override;
	~sBike() override;
	sElementSync* CreateSync() override;
	void DisposeSync(sElementSync* pSync) override;
	sElementSync* CreateSyncFromOther(sElementSync* pSync) override;
	bool HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) override;
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_BIKE; }
	void ApplyClientSync(uint16 nTime) override;
	void Update(uint16 nTime) override;
	bool WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) override;
	void ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) override;
	void ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) override;
	void SetupModel() override;

	void CompareSyncState(sBikeSync* pSync, sBikeSync* pLastSync, uint32 nDelta, tBikeSyncsDeltas* pDiff);
	void PerformWriteSync(sWriteSyncStream* pSyncStream, sBikeSync* pSync, tBikeSyncsDeltas* pDiff);
	bool WriteSyncDelta(sWriteSyncStream* pSyncStream, sBikeSync* pSync, sBikeSync* pLastSync, uint32 nDelta);

	void Fix();
};