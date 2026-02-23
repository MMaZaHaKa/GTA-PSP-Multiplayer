/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "multiplayer/MultiGame.h"
#include "multiplayer/elements/sAutomobileBase.h"


#ifndef GTA_LIBERTY
sAutomobileBaseSync::sAutomobileBaseSync()
{
	// todo
}

sAutomobileBaseSync::sAutomobileBaseSync(CAutomobile* pAutomobile)
{
	// todo
}

sAutomobileBaseSync::~sAutomobileBaseSync()
{

}



cAutomobileBaseMG::cAutomobileBaseMG(sElement* elem) : cVehicleMG(elem) { }



/* TODO(MP): stub */
sAutomobileBase::sAutomobileBase() {
	TODO();
	//operator new[](this->sVehicle.tsVehicleUnk12_field_124, 6, 12, tsVehicleUnk12_ctor_sub_8AAC470);
}


ElementCapability sAutomobileBase::GetCapability()
{
	return sAutomobileBase::Capability();
}

bool sAutomobileBase::HasCapability(ElementCapability capability)
{
	if (sAutomobileBase::Capability() == capability)
		return true;
	if (sVehicle::Capability() == capability)
		return true;
	if (sElementPhysical::Capability() == capability)
		return true;
	if (sElement::Capability() == capability)
		return true;
	return false;
}

sAutomobileBase::~sAutomobileBase() { }

/* TODO(MP): stub */
void sAutomobileBase::ApplyClientSync(uint16 time) {

}

void sAutomobileBase::ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) {

}

bool sAutomobileBase::IsTransferable(void) {
	return sVehicle::IsTransferable();
}

void sAutomobileBase::TransferEntity(int16 nDestPlayer) {
	sVehicle::TransferEntity(nDestPlayer);
}


void sAutomobileBase::Initialise() {
	sVehicle::Initialise();
	//sAutomobileBase_init_sub_8AAD414(this); // todo
}

/* TODO(MP): stub */
void sAutomobileBase::SetupModel() {

}
#endif
