/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "common.h"

#include "multiplayer/elements/sAutomobile.h"
#ifndef GTA_LIBERTY
#include "QuadBike.h"

enum eQuadBikeSync
{
	MP_PKTD_QUAD_EQUAL    = 0,
	MP_PKTD_QUAD_AUTOBASE = BIT(0), // sAutomobileBaseSync
	MP_PKTD_QUAD_FULL     = -1,     // full diff
};

struct sQuadBikeSync : sAutomobileBaseSync
{
public:

	sQuadBikeSync();
	sQuadBikeSync(CQuadBike* pQuadBike);
	sQuadBikeSync(const sQuadBikeSync& other);
	~sQuadBikeSync() override;

	bool Compare(const sQuadBikeSync& other); // inlined

#if !defined(FINAL) && !defined(MASTER)
	void Dump();
#endif

#ifdef MULTIGAME_ELEMENTS_IMPROVEMENTS
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_QUADBIKE; }
#endif
};

#pragma pack(push, 1)
struct tQuadBikeSyncsDeltas
{
	uint32 nQuadBikeDiff; // eQuadBikeSync
	tAutomobileBaseSyncsDeltas tAutomobileBaseDiff;

	inline void SetEqual()
	{
		tAutomobileBaseDiff.SetEqual(); // useless
		//nQuadBikeDiff = eQuadBikeSync::MP_PKTD_QUAD_EQUAL;
		memset(this, eQuadBikeSync::MP_PKTD_QUAD_EQUAL, sizeof(tQuadBikeSyncsDeltas));
	}

	inline void SetDifference()
	{
		tAutomobileBaseDiff.SetDifference(); // useless
		//nQuadBikeDiff = eQuadBikeSync::MP_PKTD_QUAD_FULL;
		memset(this, static_cast<uint32>(eQuadBikeSync::MP_PKTD_QUAD_FULL), sizeof(tQuadBikeSyncsDeltas)); // 0xFFFFFFFF
	}
};
static_assert(sizeof(tQuadBikeSyncsDeltas) == 36, "sizeof(tQuadBikeSyncsDeltas)");
#pragma pack(pop)

class cQuadBikeMG : public cAutomobileBaseMG {
public:

	cQuadBikeMG(sElement* elem);

	~cQuadBikeMG() override;
	void PreRender(void) override;
	void Render(void) override;
};


struct sQuadBike : sAutomobileBase {
public:
//#ifdef FIX_BUGS
//	float m_aSuspensionSpringRatio[4];
//#endif

	sQuadBike();
	sQuadBike(CQuadBike* pQuadBike);

	static ElementCapability Capability() { return reinterpret_cast<ElementCapability>(sQuadBike::Capability); }

	ElementCapability GetCapability() override;
	bool HasCapability(ElementCapability capability) override;
	~sQuadBike() override;
	sElementSync* CreateSync() override;
	void DisposeSync(sElementSync* pSync) override;
	sElementSync* CreateSyncFromOther(sElementSync* pSync) override;
	bool HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) override;
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_QUADBIKE; }
	void ApplyClientSync(uint16 nTime) override;
	void Update(uint16 nTime) override;
	bool WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) override;
	void ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) override;
	void ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) override;
	void Initialise() override;
	void SetupModel() override;

	void CompareSyncState(sQuadBikeSync* pSync, sQuadBikeSync* pLastSync, uint32 nDelta, tQuadBikeSyncsDeltas* pDiff);
	void PerformWriteSync(sWriteSyncStream* pSyncStream, sQuadBikeSync* pSync, tQuadBikeSyncsDeltas* pDiff);
	bool WriteSyncDelta(sWriteSyncStream* pSyncStream, sQuadBikeSync* pSync, sQuadBikeSync* pLastSync, uint32 nDelta);

#ifdef FIX_BUGS
	void Fix();
#endif
};
#endif