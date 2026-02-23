/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "multiplayer/MultiGame.h"
#include "multiplayer/elements/sAutomobile.h"


sAutomobileSync::sAutomobileSync()
{

}

sAutomobileSync::sAutomobileSync(CAutomobile* pAutomobile)
{

}

sAutomobileSync::sAutomobileSync(const sAutomobileSync& other)
{

}

sAutomobileSync::~sAutomobileSync()
{

}


cAutomobileMG::cAutomobileMG(sElement* elem) : cAutomobileBaseMG(elem)
{
	m_vehType = VEHICLE_TYPE_CAR;
}


sAutomobile::sAutomobile() {
	// TODO
	//memset(&sAutomobileBase.sVehicle.field_16C, 0, 24);
	//sAutomobileBase.sVehicle.field_16C = 0.75f;
#ifdef GTA_LIBERTY
	SetPhysical(new cVehicleMG(this));
#else
	SetPhysical(new cAutomobileMG(this));
#endif
}

sAutomobile::sAutomobile(CAutomobile* pAutomobile) {
	// TODO
	//memset(&sAutomobileBase.sVehicle.field_16C, 0, 24);
	//sAutomobileBase.sVehicle.field_16C = 0.75f;
	cMultiGame::Instance().AttachEntity(this, pAutomobile);
	SetEntity(pAutomobile);
#ifdef GTA_LIBERTY
	SetPhysical(new cVehicleMG(this));
#else
	SetPhysical(new cAutomobileMG(this));
#endif
	Initialise();
	//m_fTraction = pAutomobile->m_fTraction;
	//memcpy(this->sAutomobileBase.sVehicle.field_8C, &pAutomobile->field_564[308], 16);
	//memcpy(&this->sAutomobileBase.sVehicle.field_8C[16], &pAutomobile->field_564[292], 16);
	//sub_89F7AFC(this);
	AttachSync(m_nTime, new sAutomobileSync(pAutomobile));
	//sub_888F9A4(this);
	// sync
	TODO();
}


ElementCapability sAutomobile::GetCapability()
{
	return sAutomobile::Capability();
}

bool sAutomobile::HasCapability(ElementCapability capability)
{
	if (sAutomobile::Capability() == capability)
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

sAutomobile::~sAutomobile() {

}

sElementSync* sAutomobile::CreateSync() {
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	return nil;
}

void sAutomobile::DisposeSync(sElementSync* pSync) {

}

sElementSync* sAutomobile::CreateSyncFromOther(sElementSync* pSync) {
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	return nil;
}

bool sAutomobile::HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) {
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	return false;
}

/* TODO(MP): stub */
void sAutomobile::ApplyClientSync(uint16 time) {

}

/* TODO(MP): stub */
void sAutomobile::Update(uint16 time) {

}

bool sAutomobile::WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) {
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	return false;
}

void sAutomobile::ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) {

}

void sAutomobile::ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) {

}

/* TODO(MP): stub */
void sAutomobile::SetupModel() {
	sAutomobileBase::SetupModel();
	// TODO frame flag zero
	TODO();
}

/* TODO(MP): stub */
void sAutomobile::Fix() {

}