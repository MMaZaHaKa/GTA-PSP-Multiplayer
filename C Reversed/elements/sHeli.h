/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "common.h"

#include "multiplayer/elements/sAutomobile.h"
#include "Heli.h"


struct sHeliSync : sAutomobileBaseSync
{
public:

	sHeliSync();
	sHeliSync(CHeli* pHeli);
	sHeliSync(const sHeliSync& other);
	~sHeliSync() override;
};

class cHeliMG : public cAutomobileBaseMG {
public:
	// todo
	cHeliMG(sElement* elem);
};


struct sHeli : sAutomobileBase {
public:

	sHeli();
	sHeli(CHeli* pHeli);

	static ElementCapability Capability() { return reinterpret_cast<ElementCapability>(sHeli::Capability); }

	ElementCapability GetCapability() override;
	bool HasCapability(ElementCapability capability) override;
	~sHeli() override;
	sElementSync* CreateSync() override;
	void DisposeSync(sElementSync* pSync) override;
	sElementSync* CreateSyncFromOther(sElementSync* pSync) override;
	bool HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) override;
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_HELI; }
	void ApplyClientSync(uint16 time) override;
	void Update(uint16 time) override;
	bool WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) override;
	void ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) override;
	void ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) override;
	void Initialise() override;
	void SetupModel() override;
};
