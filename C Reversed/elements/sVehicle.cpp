/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"
#include "AnimBlendAssociation.h"
#include "Automobile.h"
#include "Ped.h"
#include "Vehicle.h"
#include "Garages.h"
#include "DMAudio.h"
#include "VisibilityPlugins.h"
#include "Streaming.h"
#include "AnimManager.h"
#include "Renderer.h"
#include "Lights.h"
#include "PointLights.h"

#include "multiplayer/MultiGame.h"
#include "multiplayer/elements/sVehicle.h"

int32 sVehicle::ms_nNumberOfSyncedVehicles = 0;

sVehicleSync::sVehicleSync() : sElementPhysicalSync()
{
	// TODO(MP): implement
	m_fHealth = 1000.0f;
	VehicleCreatedBy == RANDOM_VEHICLE;
	m_fLastDamageAmount = 0.0f;
	m_nLastDamagePlayerID = 0;
	m_bHasDriver = false;
	m_bIsDrivenByPlayer = false;
	m_bIsWreked = false;
	m_nDriverTeam = -1;
	m_nDriverID = -1;
	m_status = 0;
}

sVehicleSync::sVehicleSync(CVehicle* pVeh) : sElementPhysicalSync(pVeh)
{
	// TODO(MP): implement
	m_status = pVeh->GetStatus();
	VehicleCreatedBy = pVeh->VehicleCreatedBy;
	m_nDoorLock = pVeh->m_nDoorLock;
	m_nDriverID = pVeh->m_nDriverIdMG;
	m_fHealth = pVeh->m_fHealth;
	m_nDriverTeam = pVeh->pDriver ? pVeh->pDriver->GetTeamID() : -1;
	// TODO
	m_fHealth = pVeh->m_fHealth; // why kek
	m_vehLCS_2A3 = pVeh->m_vehLCS_2A3;
}

sVehicleSync::~sVehicleSync()
{

}

bool sVehicleSync::Compare(const sVehicleSync& other)
{
	TODO();
	TODO();
	TODO();
	TODO();
	TODO();
	return false;
}


cVehicleMG::cVehicleMG(sElement* elem) : cPhysicalMG(elem) {
	m_pFire = nil;
	bIsVehicle = true;
	SetStatus(STATUS_SIMPLE);
	m_audioEntityId = DMAudio.CreateEntity(AUDIOTYPE_PHYSICAL, this);
	if (m_audioEntityId >= 0)
		DMAudio.SetEntityStatus(m_audioEntityId, TRUE);
	field_174 = 0.0;
	field_178 = 0.0;
}


cVehicleMG::~cVehicleMG()
{
	DMAudio.DestroyEntity(m_audioEntityId);
}

void cVehicleMG::SetModelIndex(uint32 id)
{
	if (GetModelIndex() == id)
		return;

	if (!CStreaming::HasModelLoaded(id))
	{
		CStreaming::FlushRequestList();
		CStreaming::RequestModel(id, STREAMFLAGS_NOFADE | STREAMFLAGS_DEPENDENCY | STREAMFLAGS_SCRIPTOWNED);
		CStreaming::LoadAllRequestedModels(false);
	}
	CEntity::SetModelIndex(id);
	GetElement().vehicle->SetupModel();
	tHandlingData* handling = GET_HANDLING(id);
	m_fMass = handling->fMass;
	m_fTurnMass = handling->fTurnMass;
	m_vecCentreOfMass = handling->CentreOfMass;
	GetElement().vehicle->field_AC = 1.0f;
	m_fForceMultiplier = 1.0f;
}

void cVehicleMG::PreRender(void)
{
	;
}

void cVehicleMG::Render(void)
{
	CVehicleModelInfo* mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex());
#ifdef GTA_LIBERTY
	mi->SetVehicleColour(&GetElement().vehicle->m_currentColour1, &GetElement().vehicle->m_currentColour2);
#else
	mi->SetVehicleColour(&GetElement().vehicle->m_aColours[VEHICLE_COLOUR_PRIMARY], &GetElement().vehicle->m_aColours[VEHICLE_COLOUR_SECONDARY]);
#endif
	cPhysicalMG::Render();
}

bool cVehicleMG::SetupLighting(void)
{
	ActivateDirectional();
	SetAmbientColoursForPedsCarsAndObjects();
	//gbForceEnvOff = false;
	// PSP gif g_GifCMDBufferPtr++ = 0xC9010100;
	if (bRenderScorched)
	{
		WorldReplaceNormalLightsWithScorched(Scene.world, 0.1f);
		//gbForceEnvOff = true;
	}
	else
	{
		CVector coors = GetPosition();
		float lighting = CPointLights::GenerateLightsAffectingObject(&coors);
		if (lighting != 1.0f)
		{
			SetAmbientAndDirectionalColours(lighting);
			return true;
		}
	}

	return false;
}

void cVehicleMG::RemoveLighting(bool reset)
{
	CRenderer::RemoveVehiclePedLights(this, reset);
	SetAmbientColours();
	DeActivateDirectional();
}


sVehicle::sVehicle() {
	m_pPed = nil;
	++ms_nNumberOfSyncedVehicles;
}


ElementCapability sVehicle::GetCapability()
{
	return sVehicle::Capability();
}

bool sVehicle::HasCapability(ElementCapability capability)
{
	if (sVehicle::Capability() == capability)
		return true;
	if (sElementPhysical::Capability() == capability)
		return true;
	if (sElement::Capability() == capability)
		return true;
	return false;
}

sVehicle::~sVehicle() {
	for (int32 nIndex = 0; nIndex < CGarages::NumGarages; nIndex++) {
		CGarage& garage = CGarages::aGarages[nIndex];
		if (garage.m_pSSVehicle == this)
			garage.m_pSSVehicle = nil;
	}
	--ms_nNumberOfSyncedVehicles;
}

/* TODO(MP): stub */
void sVehicle::ApplyClientSync(uint16 time) {

}

/* TODO(MP): stub */
void sVehicle::ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) {

}

/* TODO(MP): stub */
void sVehicle::UpdateDelta(sElementSync* pSync, uint16 nTimeDelta) {

}

bool sVehicle::IsTransferable(void) {
	return GetSync().vehicle->m_bIsDrivenByPlayer == false;
}

void sVehicle::TransferEntity(int16 nDestPlayer) {
	TODO(); // vcs
	debug("Transfer Vehicle %d", GetID());
	CVehicle* pVeh = (CVehicle*)GetEntity();
	CPed* pDriver = pVeh->pDriver;
	if (pDriver != nil && pDriver->IsPlayer()) {
		if (pDriver->m_nPedState != PED_DRIVING) {
			debug("Car transfer prevented by driver state %d\n", pDriver->m_nPedState);
			return;
		}
		int16 nAnimID = pDriver->m_pVehicleAnim->animId;
		bool bHasSitAnim = nAnimID == ANIM_STD_CAR_SIT || nAnimID == ANIM_BIKE_RIDE || nAnimID == ANIM_STD_CAR_SIT_LO;
		if (!bHasSitAnim) {
			debug("Car transfer prevented by driver anim %d\n", nAnimID);
			return;
		}
	}
	sElement::TransferEntity(nDestPlayer);
	TransferOccupants(nDestPlayer);
}

/* TODO(MP): stub */
void sVehicle::Initialise() {
	CVehicle* pVeh = (CVehicle*)GetEntity();
	m_pPhyElem->SetModelIndex(pVeh->GetModelIndex());
	m_pPhyElem->m_fAirResistance = pVeh->m_fAirResistance;
	m_pPhyElem->m_fMass = pVeh->m_fMass;
	m_pPhyElem->m_fTurnMass = pVeh->m_fTurnMass;
	m_pPhyElem->m_fForceMultiplier = pVeh->m_fForceMultiplier;
	m_pPhyElem->m_vecCentreOfMass = pVeh->m_vecCentreOfMass;
	m_pPhyElem->bUsesCollision = false;
	m_pPed = nil;
#ifdef GTA_LIBERTY
	m_currentColour1 = pVeh->m_currentColour1;
	m_currentColour2 = pVeh->m_currentColour2;
#else
	m_aColours[VEHICLE_COLOUR_PRIMARY] = pVeh->m_aColours[VEHICLE_COLOUR_PRIMARY];
	m_aColours[VEHICLE_COLOUR_SECONDARY] = pVeh->m_aColours[VEHICLE_COLOUR_SECONDARY];
#endif
	RegisterSelf();
}

bool sVehicle::TransferOccupants(int16 nDestPlayer) {
	debug("Transfer Occupants\n");
	CVehicle* pVeh = (CVehicle*)GetEntity();
	if (pVeh == nil)
		return true;
	CPed* pDriver = pVeh->pDriver;
	if (pDriver != nil && pDriver->IsPlayer()) {
		pDriver->SetPlayerToFollow(nDestPlayer);
#ifdef GTA_LIBERTY
		pDriver->m_pMyVehicle = nil;
		pDriver->bInVehicle = false;
		pVeh->pDriver = nil;
#else
		if (pDriver->InVehicle())
			pDriver->SetPedState(PedState::PED_IDLE);
		pDriver->SetInVehicle(false);
		pDriver->SetMyVehicle(nil);
		GetEntity()->SetStatus(eEntityStatus::STATUS_ABANDONED);
		pVeh->SetDriverPed(nil);
#endif
		return true;
	}
	sElement* pElem = cMultiGame::Instance().GetElementFromEntity(pDriver);
	if (pElem != nil && !pElem->WasTransfered())
		pElem->TransferEntity(nDestPlayer);
	return true;
}
