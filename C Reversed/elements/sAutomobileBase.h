/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "multiplayer/elements/sVehicle.h"
#include "Automobile.h"

#ifndef GTA_LIBERTY
struct sAutomobileBaseSync : sVehicleSync
{
public:

	sAutomobileBaseSync();
	sAutomobileBaseSync(CAutomobile* pAutomobile);
	~sAutomobileBaseSync() override;
};


struct cAutomobileBaseMG : public cVehicleMG {
	// todo vcs
	cAutomobileBaseMG(sElement* elem);
};


struct sAutomobileBase : sVehicle {
public:

	sAutomobileBase();

	static ElementCapability Capability() { return reinterpret_cast<ElementCapability>(sAutomobileBase::Capability); }

	ElementCapability GetCapability() override;
	bool HasCapability(ElementCapability capability) override;
	~sAutomobileBase() override;
	void ApplyClientSync(uint16 time) override;
	void ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) override;
	bool IsTransferable(void) override;
	void TransferEntity(int16 nDestPlayer) override;
	void Initialise() override;
	void SetupModel() override;
};
#endif
