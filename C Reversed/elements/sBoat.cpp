/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "multiplayer/MultiGame.h"
#include "multiplayer/elements/sBoat.h"


#ifndef GTA_LIBERTY
sBoatSync::sBoatSync()
{

}

sBoatSync::sBoatSync(CBoat* pBoat)
{

}

sBoatSync::sBoatSync(const sBoatSync& other)
{

}

sBoatSync::~sBoatSync()
{

}


cBoatMG::cBoatMG(sElement* elem) : cVehicleMG(elem)
{
	m_vehType = eVehicleType::VEHICLE_TYPE_BOAT;
	// todo 2 wake shit
	//CBoatWake();
}


sBoat::sBoat()
{
	SetPhysical(new cBoatMG(this));
}

sBoat::sBoat(CBoat* pBoat)
{
	cMultiGame::Instance().AttachEntity(this, pBoat);
	SetEntity(pBoat);
	SetPhysical(new cBoatMG(this));
	Initialise();
	AttachSync(m_nTime, new sBoatSync(pBoat));
	//sub_888F9A4(this);
	TODO();
}



ElementCapability sBoat::GetCapability()
{
	return sBoat::Capability();
}

bool sBoat::HasCapability(ElementCapability capability)
{
	if (sBoat::Capability() == capability)
		return true;
	if (sVehicle::Capability() == capability)
		return true;
	if (sElementPhysical::Capability() == capability)
		return true;
	if (sElement::Capability() == capability)
		return true;
	return false;
}

sBoat::~sBoat() {

}

sElementSync* sBoat::CreateSync() {
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	return nil;
}

void sBoat::DisposeSync(sElementSync* pSync) {

}

sElementSync* sBoat::CreateSyncFromOther(sElementSync* pSync) {
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	return nil;
}

bool sBoat::HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) {
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	return false;
}

/* TODO(MP): stub */
void sBoat::ApplyClientSync(uint16 time) {

}

/* TODO(MP): stub */
void sBoat::Update(uint16 time) {

}

bool sBoat::WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) {
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	return false;
}

void sBoat::ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) {

}

void sBoat::ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) {

}

void sBoat::SetupModel() {
	;
}
#endif