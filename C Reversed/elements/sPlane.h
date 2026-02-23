/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "common.h"

#include "multiplayer/elements/sAutomobile.h"
#ifndef GTA_LIBERTY
#include "Plane.h"

struct sPlaneSync : sAutomobileBaseSync
{
public:

	sPlaneSync();
	sPlaneSync(CPlane* pPlane);
	sPlaneSync(const sPlaneSync& other);
	~sPlaneSync() override;
};

class cPlaneMG : public cAutomobileBaseMG {
public:
	// todo
	cPlaneMG(sElement* elem);
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
	void ApplyClientSync(uint16 time) override;
	void Update(uint16 time) override;
	bool WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) override;
	void ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) override;
	void ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) override;
	void Initialise() override;
	void SetupModel() override;
};
#endif