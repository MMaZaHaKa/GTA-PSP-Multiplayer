/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "common.h"

#include "multiplayer/elements/sVehicle.h"
#ifndef GTA_LIBERTY
#include "Bmx.h"

struct sBmxSync : sVehicleSync
{
public:

	sBmxSync();
	sBmxSync(CBmx* pBmx);
	sBmxSync(const sBmxSync& other);
	~sBmxSync() override;
};

class cBmxMG : public cVehicleMG {
public:
	// todo
	cBmxMG(sElement* elem);
};


struct sBmx : sVehicle {
public:

	sBmx();
	sBmx(CBmx* pBmx);

	static ElementCapability Capability() { return reinterpret_cast<ElementCapability>(sBmx::Capability); }

	ElementCapability GetCapability() override;
	bool HasCapability(ElementCapability capability) override;
	~sBmx() override;
	sElementSync* CreateSync() override;
	void DisposeSync(sElementSync* pSync) override;
	sElementSync* CreateSyncFromOther(sElementSync* pSync) override;
	bool HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) override;
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_BMX; }
	void ApplyClientSync(uint16 time) override;
	void Update(uint16 time) override;
	bool WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) override;
	void ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) override;
	void ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) override;
	void Initialise() override;
	void SetupModel() override;
};
#endif