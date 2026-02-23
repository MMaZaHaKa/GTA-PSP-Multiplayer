/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "multiplayer/MultiGame.h"
#include "multiplayer/elements/sBmx.h"


#ifndef GTA_LIBERTY
sBmxSync::sBmxSync()
{

}

sBmxSync::sBmxSync(CBmx* pBmx)
{

}

sBmxSync::sBmxSync(const sBmxSync& other)
{

}

sBmxSync::~sBmxSync()
{

}


cBmxMG::cBmxMG(sElement* elem) : cVehicleMG(elem)
{
	m_vehType = eVehicleType::VEHICLE_TYPE_BMX;
}


/* TODO(MP): stub */
sBmx::sBmx()
{
	// todo
	SetPhysical(new cBmxMG(this));
}

sBmx::sBmx(CBmx* pBmx)
{
	cMultiGame::Instance().AttachEntity(this, pBmx);
	SetEntity(pBmx);
	SetPhysical(new cBmxMG(this));
	Initialise();
	AttachSync(m_nTime, new sBmxSync(pBmx));
	//sub_888F9A4(this);
	TODO();
}

ElementCapability sBmx::GetCapability()
{
	return sBmx::Capability();
}

bool sBmx::HasCapability(ElementCapability capability)
{
	if (sBmx::Capability() == capability)
		return true;
	if (sVehicle::Capability() == capability)
		return true;
	if (sElementPhysical::Capability() == capability)
		return true;
	if (sElement::Capability() == capability)
		return true;
	return false;
}

sBmx::~sBmx()
{

}

sElementSync* sBmx::CreateSync() {
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	return nil;
}

void sBmx::DisposeSync(sElementSync* pSync) {

}

sElementSync* sBmx::CreateSyncFromOther(sElementSync* pSync) {
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	return nil;
}

bool sBmx::HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) {
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	return false;
}

/* TODO(MP): stub */
void sBmx::ApplyClientSync(uint16 time) {

}

/* TODO(MP): stub */
void sBmx::Update(uint16 time) {

}

bool sBmx::WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) {
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	return false;
}

void sBmx::ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) {

}

void sBmx::ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) {

}

void sBmx::Initialise() {
	sVehicle::Initialise();
}

/* TODO(MP): stub */
void sBmx::SetupModel() {

}

#endif