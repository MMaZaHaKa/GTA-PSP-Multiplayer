/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "common.h"
#include "Bike.h"

#include "multiplayer/elements/sVehicle.h"

struct sBikeSync : sVehicleSync
{
public:
	float m_fWheelieRoll; //  up angle,  temp for test, todo meaf CVehicle/CBike size

	sBikeSync();
	sBikeSync(CBike* bike);
	sBikeSync(const sBikeSync& that);
	~sBikeSync() override;
};

class cBikeMG : public cVehicleMG {
public:
	// todo
	cBikeMG(sElement* elem);
};


struct sBike : sVehicle {
public:

	sBike();
	sBike(CBike* bike);

	static ElementCapability Capability() { return reinterpret_cast<ElementCapability>(sBike::Capability); }

	ElementCapability GetCapability() override;
	bool HasCapability(ElementCapability capability) override;
	~sBike() override;
	sElementSync* CreateSync() override;
	void DisposeSync(sElementSync* pSync) override;
	sElementSync* CreateSyncFromOther(sElementSync* pSync) override;
	bool HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) override;
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_BIKE; }
	void ApplyClientSync(uint16 time) override;
	void Update(uint16 time) override;
	bool WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) override;
	void ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) override;
	void ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) override;
	void SetupModel() override;

	void Fix();
};