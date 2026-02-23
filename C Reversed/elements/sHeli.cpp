/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "multiplayer/MultiGame.h"
#include "multiplayer/elements/sHeli.h"


sHeliSync::sHeliSync()
{

}

sHeliSync::sHeliSync(CHeli* pHeli)
{

}

sHeliSync::sHeliSync(const sHeliSync& other)
{

}

sHeliSync::~sHeliSync()
{

}


cHeliMG::cHeliMG(sElement* elem) : cAutomobileBaseMG(elem)
{
	// todo
	//field_180 = 0.0;
	//field_184 = 0.0;
	m_vehType = eVehicleType::VEHICLE_TYPE_HELI;
}


sHeli::sHeli()
{
	SetPhysical(new cHeliMG(this));
}

sHeli::sHeli(CHeli* pHeli)
{
	cMultiGame::Instance().AttachEntity(this, pHeli);
	SetEntity(pHeli);
	SetPhysical(new cHeliMG(this));
	Initialise();
	AttachSync(m_nTime, new sHeliSync(pHeli));
	//sub_888F9A4(this);
	TODO();
}


ElementCapability sHeli::GetCapability()
{
	return sHeli::Capability();
}

bool sHeli::HasCapability(ElementCapability capability)
{
	if (sHeli::Capability() == capability)
		return true;
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

sHeli::~sHeli() {

}

sElementSync* sHeli::CreateSync() {
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	return nil;
}

void sHeli::DisposeSync(sElementSync* pSync) {

}

sElementSync* sHeli::CreateSyncFromOther(sElementSync* pSync) {
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	return nil;
}

bool sHeli::HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) {
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	return false;
}

/* TODO(MP): stub */
void sHeli::ApplyClientSync(uint16 time) {

}

/* TODO(MP): stub */
void sHeli::Update(uint16 time) {

}

bool sHeli::WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) {
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	return false;
}

void sHeli::ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) {

}

void sHeli::ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) {

}

void sHeli::Initialise() {
	sAutomobileBase::Initialise();
}

/* TODO(MP): stub */
void sHeli::SetupModel() {
	sAutomobileBase::SetupModel();
	// TODO frame flag zero
	TODO();
}
