/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "common.h"

#include "multiplayer/elements/sVehicle.h"
#include "Boat.h"

#ifndef GTA_LIBERTY
struct sBoatSync : sVehicleSync
{
public:

	sBoatSync();
	sBoatSync(CBoat* pBoat);
	sBoatSync(const sBoatSync& other);
	~sBoatSync() override;
};

class cBoatMG : public cVehicleMG {
public:
	// todo
	cBoatMG(sElement* elem);
};


struct sBoat : sVehicle {
public:

	sBoat();
	sBoat(CBoat* pBoat);

	static ElementCapability Capability() { return reinterpret_cast<ElementCapability>(sBoat::Capability); }

	ElementCapability GetCapability() override;
	bool HasCapability(ElementCapability capability) override;
	~sBoat() override;
	sElementSync* CreateSync() override;
	void DisposeSync(sElementSync* pSync) override;
	sElementSync* CreateSyncFromOther(sElementSync* pSync) override;
	bool HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) override;
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_BOAT; }
	void ApplyClientSync(uint16 time) override;
	void Update(uint16 time) override;
	bool WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) override;
	void ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) override;
	void ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) override;
	void SetupModel() override;
};
#endif