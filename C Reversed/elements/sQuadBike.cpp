/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "multiplayer/MultiGame.h"
#include "multiplayer/elements/sQuadBike.h"


#ifndef GTA_LIBERTY
sQuadBikeSync::sQuadBikeSync()
{

}

sQuadBikeSync::sQuadBikeSync(CQuadBike* pQuadbike)
{

}

sQuadBikeSync::sQuadBikeSync(const sQuadBikeSync& other)
{

}

sQuadBikeSync::~sQuadBikeSync()
{

}


cQuadBikeMG::cQuadBikeMG(sElement* elem) : cAutomobileBaseMG(elem)
{
	m_vehType = eVehicleType::VEHICLE_TYPE_QUAD;
}


sQuadBike::sQuadBike()
{
	SetPhysical(new cQuadBikeMG(this));
}


sQuadBike::sQuadBike(CQuadBike* pQuadbike)
{
	cMultiGame::Instance().AttachEntity(this, pQuadbike);
	SetEntity(pQuadbike);
	SetPhysical(new cQuadBikeMG(this));
	Initialise();
	AttachSync(m_nTime, new sQuadBikeSync(pQuadbike));
	//sub_888F9A4(this);
	TODO();
}

ElementCapability sQuadBike::GetCapability()
{
	return sQuadBike::Capability();
}

bool sQuadBike::HasCapability(ElementCapability capability)
{
	if (sQuadBike::Capability() == capability)
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

sQuadBike::~sQuadBike() {

}

sElementSync* sQuadBike::CreateSync() {
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	return nil;
}

void sQuadBike::DisposeSync(sElementSync* pSync) {

}

sElementSync* sQuadBike::CreateSyncFromOther(sElementSync* pSync) {
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	return nil;
}

bool sQuadBike::HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) {
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	return false;
}

/* TODO(MP): stub */
void sQuadBike::ApplyClientSync(uint16 time) {

}

/* TODO(MP): stub */
void sQuadBike::Update(uint16 time) {

}

bool sQuadBike::WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) {
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	return false;
}

void sQuadBike::ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) {

}

void sQuadBike::ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) {

}

void sQuadBike::Initialise() {
	sAutomobileBase::Initialise();
}

void sQuadBike::SetupModel() {
	sAutomobileBase::SetupModel();
}
#endif