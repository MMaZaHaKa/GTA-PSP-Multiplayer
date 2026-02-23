/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "multiplayer/MultiGame.h"
#include "multiplayer/elements/sBike.h"


sBikeSync::sBikeSync()
{

}

sBikeSync::sBikeSync(CBike* bike)
{

}

sBikeSync::sBikeSync(const sBikeSync& that)
{

}

sBikeSync::~sBikeSync()
{

}


cBikeMG::cBikeMG(sElement* elem) : cVehicleMG(elem)
{
	m_vehType = eVehicleType::VEHICLE_TYPE_BIKE;
}



sBike::sBike() {
	m_pPhyElem = new cBikeMG(this);
}

sBike::sBike(CBike* bike) {
	cMultiGame::Instance().AttachEntity(this, bike);
	SetEntity(bike);
	SetPhysical(new cBikeMG(this)); // vcs inlined ctor cBikeMG
	Initialise();
	// TODO
	//this->sVehicle.m_fTraction = *&bike[1].AutoPilot[24];
	//memcpy(this->sVehicle.field_8C, &bike[1].AutoPilot[4], 16);
	//memcpy(&this->sVehicle.field_8C[16], &bike[1].m_pRightWakesInfo, 16);
	AttachSync(m_nTime, new sBikeSync(bike));
	//sub_888F9A4(this);
	// Sync
	TODO();
}


ElementCapability sBike::GetCapability()
{
	return sBike::Capability();
}

bool sBike::HasCapability(ElementCapability capability)
{
	if (sBike::Capability() == capability)
		return true;
	if (sVehicle::Capability() == capability)
		return true;
	if (sElementPhysical::Capability() == capability)
		return true;
	if (sElement::Capability() == capability)
		return true;
	return false;
}

sBike::~sBike()
{

}

sElementSync* sBike::CreateSync() {
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	return nil;
}

void sBike::DisposeSync(sElementSync* pSync) {

}

sElementSync* sBike::CreateSyncFromOther(sElementSync* pSync) {
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	return nil;
}

bool sBike::HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) {
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	return false;
}

/* TODO(MP): stub */
void sBike::ApplyClientSync(uint16 time) {

}

/* TODO(MP): stub */
void sBike::Update(uint16 time) {

}

bool sBike::WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) {
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	return false;
}

void sBike::ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) {

}

void sBike::ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) {

}

/* TODO(MP): stub */
void sBike::SetupModel() {

}


void sBike::Fix() {
	;
}