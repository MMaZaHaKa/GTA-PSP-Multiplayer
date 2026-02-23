/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "common.h"

#include "multiplayer/elements/sAutomobile.h"
#ifndef GTA_LIBERTY
#include "QuadBike.h"

struct sQuadBikeSync : sAutomobileBaseSync
{
public:

	sQuadBikeSync();
	sQuadBikeSync(CQuadBike* pQuadbike);
	sQuadBikeSync(const sQuadBikeSync& other);
	~sQuadBikeSync() override;
};

class cQuadBikeMG : public cAutomobileBaseMG {
public:
	// todo
	cQuadBikeMG(sElement* elem);
};


struct sQuadBike : sAutomobileBase {
public:

	sQuadBike();
	sQuadBike(CQuadBike* pQuadbike);

	static ElementCapability Capability() { return reinterpret_cast<ElementCapability>(sQuadBike::Capability); }

	ElementCapability GetCapability() override;
	bool HasCapability(ElementCapability capability) override;
	~sQuadBike() override;
	sElementSync* CreateSync() override;
	void DisposeSync(sElementSync* pSync) override;
	sElementSync* CreateSyncFromOther(sElementSync* pSync) override;
	bool HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) override;
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_QUADBIKE; }
	void ApplyClientSync(uint16 time) override;
	void Update(uint16 time) override;
	bool WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) override;
	void ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) override;
	void ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) override;
	void Initialise() override;
	void SetupModel() override;
};
#endif