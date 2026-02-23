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


#ifdef GTA_LIBERTY
struct sAutomobileSync : sVehicleSync
#else
struct sAutomobileSync : sAutomobileBaseSync
#endif
{
public:

	sAutomobileSync();
	sAutomobileSync(CAutomobile* pAutomobile);
	sAutomobileSync(const sAutomobileSync& other);
	~sAutomobileSync() override;
};


#ifdef GTA_LIBERTY
class cAutomobileMG : public cVehicleMG
#else
struct cAutomobileMG : public cAutomobileBaseMG
#endif
{
public:

	cAutomobileMG(sElement* elem);
};

#ifdef GTA_LIBERTY
struct sAutomobile : sVehicle
#else
struct sAutomobile : sAutomobileBase
#endif
{
public:

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
	void ApplyClientSync(uint16 time) override;
	void Update(uint16 time) override;
	bool WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) override;
	void ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) override;
	void ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) override;
	void SetupModel() override;

	void Fix();
};