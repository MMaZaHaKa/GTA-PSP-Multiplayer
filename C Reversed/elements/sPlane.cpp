/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "multiplayer/MultiGame.h"
#include "multiplayer/elements/sPlane.h"


#ifndef GTA_LIBERTY
sPlaneSync::sPlaneSync()
{

}

sPlaneSync::sPlaneSync(CPlane* pPlane)
{

}

sPlaneSync::sPlaneSync(const sPlaneSync& other)
{

}

sPlaneSync::~sPlaneSync()
{

}


cPlaneMG::cPlaneMG(sElement* elem) : cAutomobileBaseMG(elem)
{
	// todo 0.0f
	TODO();
	m_vehType = eVehicleType::VEHICLE_TYPE_PLANE;
}


sPlane::sPlane()
{
	SetPhysical(new cPlaneMG(this));
}

sPlane::sPlane(CPlane* pPlane)
{
	cMultiGame::Instance().AttachEntity(this, pPlane);
	SetEntity(pPlane);
	SetPhysical(new cPlaneMG(this));
	Initialise();
	AttachSync(m_nTime, new sPlaneSync(pPlane));
	//sub_888F9A4(this);
	TODO();
}

ElementCapability sPlane::GetCapability()
{
	return sPlane::Capability();
}

bool sPlane::HasCapability(ElementCapability capability)
{
	if (sPlane::Capability() == capability)
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

sPlane::~sPlane() {

}

sElementSync* sPlane::CreateSync() {
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	return nil;
}

void sPlane::DisposeSync(sElementSync* pSync) {

}

sElementSync* sPlane::CreateSyncFromOther(sElementSync* pSync) {
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	return nil;
}

bool sPlane::HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) {
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	return false;
}

/* TODO(MP): stub */
void sPlane::ApplyClientSync(uint16 time) {

}

/* TODO(MP): stub */
void sPlane::Update(uint16 time) {

}


bool sPlane::WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) {
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	return false;
}

void sPlane::ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) {

}

void sPlane::ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) {

}

void sPlane::Initialise() {
	sAutomobileBase::Initialise();
}

/* TODO(MP): stub */
void sPlane::SetupModel() {
	sAutomobileBase::SetupModel();
	// TODO frame flag zero
	TODO();
}
#endif