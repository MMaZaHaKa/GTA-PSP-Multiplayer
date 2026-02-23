/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "common.h"
#include "Fire.h"
#include "Vehicle.h"

#include "multiplayer/elements/sElementPhysical.h"

struct sVehicleSync : sElementPhysicalSync {
public:

	//m_fSteerAngle
	//m_fGasPedal
	//m_fBrakePedal
	//field_C0
	//m_nCarJacker
	//m_nStatus
	//m_nCarHornTimer

	uint8 m_bHasDriver : 1;
	uint8 m_bIsDrivenByPlayer : 1;
	uint8 m_bIsWreked : 1;
	uint8 m_bCC_8 : 1;
	uint8 m_bCC_10 : 1;
	uint8 m_bCC_20 : 1;
	uint8 m_bCC_40 : 1;
	uint8 m_bCC_80 : 1;

	int8 m_nDriverTeam;
	int8 m_nDriverID;
	int8 m_vehLCS_2A3;  // enables 2A4
	int8 m_nLastDriverID;

	float m_fHealth;
	float m_fLastDamageAmount;
	uint8 m_nLastDamagePlayerID;

	uint8 VehicleCreatedBy;
	eCarLock m_nDoorLock;

	uint8 m_status;

	sVehicleSync();
	sVehicleSync(CVehicle* pVeh);
	//sVehicleSync(const sVehicleSync& other);
	~sVehicleSync() override;

	bool Compare(const sVehicleSync& other);
};

class cVehicleMG : public cPhysicalMG {
public:
	CFire* m_pFire;
	float field_174;
	float field_178;
	eVehicleType m_vehType;

	cVehicleMG(sElement* elem);

	~cVehicleMG() override;
	void SetModelIndex(uint32 id) override;
	void PreRender(void) override;
	void Render(void) override;
	bool SetupLighting(void) override;
	void RemoveLighting(bool reset) override;
};

class CPed;

struct sVehicle : sElementPhysical {
public:
	static int32 ms_nNumberOfSyncedVehicles;

	CPed* m_pPed;
#ifdef GTA_LIBERTY
	uint8 m_currentColour1;
	uint8 m_currentColour2;
#else
	CRGBA m_aColours[NUM_VEHICLE_COLOURS]; // 0x84 m_currentColour
#endif
	float field_AC;

	sVehicle();

	static ElementCapability Capability() { return reinterpret_cast<ElementCapability>(sVehicle::Capability); }

	ElementCapability GetCapability() override;
	bool HasCapability(ElementCapability capability) override;
	~sVehicle() override;
	void ApplyClientSync(uint16 time) override;
	void ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) override;
	void UpdateDelta(sElementSync* pSync, uint16 nTimeDelta) override;
	bool IsTransferable(void) override;
	void TransferEntity(int16 nDestPlayer) override;
#ifndef GTA_LIBERTY
	virtual void Initialise();
#endif
	virtual void SetupModel() = 0;

private:
	bool TransferOccupants(int16 nDestPlayer);
};
