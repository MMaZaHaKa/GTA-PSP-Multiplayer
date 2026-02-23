/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once
#include "common.h"
#include "Entity.h"
#include <deque>
#include "multiplayer/public.h"
#include "multiplayer/InterestZone.h"
#include "multiplayer/net/public.h" // hash


enum class eElementType { // https://prnt.sc/jO0XbAwdsaIk
	ELEMENT_TYPE_PLAYER = 0,
	ELEMENT_TYPE_PED,
	ELEMENT_TYPE_AUTOMOBILE,
	ELEMENT_TYPE_BIKE,
	ELEMENT_TYPE_HELI,
	ELEMENT_TYPE_RADAR_BLIP,
	ELEMENT_TYPE_TEXT,
	ELEMENT_TYPE_PICKUP,
#ifndef GTA_LIBERTY
	ELEMENT_TYPE_BOAT,
	ELEMENT_TYPE_PLANE,
	ELEMENT_TYPE_BMX,
	ELEMENT_TYPE_QUADBIKE,
	ELEMENT_TYPE_NETMETER2D,
#endif
#ifdef MULTIGAME_IMPROVEMENTS
	ELEMENT_TYPE_OBJECT,
#endif
	NUM_ELEMENT_TYPES
};


// forward decl
class sPeerState;

struct sPlayer;         // 0
struct sPlayerSync;     // 0 S
struct sPed;            // 1
struct sPedSync;        // 1 S
struct sAutomobile;     // 2
struct sAutomobileSync; // 2 S
struct sBike;           // 3
struct sBikeSync;       // 3 S
struct sHeli;           // 4
struct sHeliSync;       // 4 S
struct sRadarBlip;      // 5
struct sRadarBlipSync;  // 5 S
struct sTextSprite;     // 6
struct sTextSpriteSync; // 6 S
struct sPickup;         // 7
struct sPickupSync;     // 7 S

#ifndef GTA_LIBERTY
struct sBoat;           // 8
struct sBoatSync;       // 8 S
struct sPlane;          // 9
struct sPlaneSync;      // 9 S
struct sBmx;            // 10
struct sBmxSync;        // 10 S
struct sQuadBike;       // 11
struct sQuadBikeSync;   // 11 S
struct sNetMeter2d;     // 12
struct sNetMeter2dSync; // 12 S
#endif
#ifdef MULTIGAME_IMPROVEMENTS
struct sObject;     // 13
struct sObjectSync; // 13 S
#endif

struct sElement;
struct sElementSync;
struct sElementPhysical;
struct sElementPhysicalSync;
struct sVehicle;
struct sVehicleSync;
#ifndef GTA_LIBERTY
struct sAutomobileBase;
struct sAutomobileBaseSync;
struct sSpriteBase;
struct sSpriteBaseSync;
#endif

union uElement
{
	void* base;
	sPlayer* player;
	sPed* ped;
	sAutomobile* automobile;
	sBike* bike;
	sHeli* heli;
	sRadarBlip* radarblip;
	sTextSprite* text;
	sPickup* pickup;
#ifndef GTA_LIBERTY
	sBoat* boat;
	sPlane* plane;
	sBmx* bmx;
	sQuadBike* quadbike;
	sNetMeter2d* netmeter;
#endif
#ifdef MULTIGAME_IMPROVEMENTS
	sObject* object;
#endif
	sElement* element;
	sElementPhysical* elementphysical;
	sVehicle* vehicle;
#ifndef GTA_LIBERTY
	sAutomobileBase* automobilebase;
	sSpriteBase* sprite;
#endif
};

union uElementSync
{
	void* base;
	sPlayerSync* player;
	sPedSync* ped;
	sAutomobileSync* automobile;
	sBikeSync* bike;
	sHeliSync* heli;
	sRadarBlipSync* radarblip;
	sTextSpriteSync* text;
	sPickupSync* pickup;
#ifndef GTA_LIBERTY
	sBoatSync* boat;
	sPlaneSync* plane;
	sBmxSync* bmx;
	sQuadBikeSync* quadbike;
	sNetMeter2dSync* netmeter;
#endif
#ifdef MULTIGAME_IMPROVEMENTS
	sObjectSync* object;
#endif
	sElementSync* element;
	sElementPhysicalSync* elementphysical;
	sVehicleSync* vehicle;
#ifndef GTA_LIBERTY
	sAutomobileBaseSync* automobilebase;
	sSpriteBaseSync* sprite;
#endif
};

struct tSyncEntry {
public:
	uint16 m_nTime;
	//int8 m_pad0[2];
	uElementSync m_pAttachedElement; // m_pSync
};

// old: // TODO(MP): this somehow is extending CEntity? probably psp matrix, probably aligned
typedef CMatrix CMGMatrix; // pad https://prnt.sc/ucGw4s7JyeOt, lcs printf log size https://prnt.sc/F2oK5wZMQsug

struct sElementSync {
	bool m_bUnk; // for prev syncs
	//uint8 m_nPad0[3];
	inline sElementSync() { m_bUnk = 0; }
	inline sElementSync(const sElementSync& other) { m_bUnk = other.m_bUnk; }
//#ifdef GTA_LIBERTY // TODO: LCS ifdefs override
//	~sElementSync() {}
//#else
	virtual ~sElementSync() {} // vcs vft : 1
//#endif
#if !defined(FINAL) && !defined(MASTER)
	void Dump() { debug("=== sElementSync Dump ===\nm_bUnk: %d\n", m_bUnk); }
#endif
};

using ElementCapability = void* (*)(void);

struct sElement
{
public:

	// Flags
	union
	{
		struct
		{
			uint8 bPhyUnk_1 : 1; // (xref:  CPhysical::SendMuliVehicleCollision)
			uint8 b0_2 : 1; // xref: cMultiGame::SendTransferEntityMsg ????
			uint8 b0_4 : 1;
			uint8 b0_8 : 1;
			uint8 b0_10 : 1;
			uint8 b0_20 : 1;
			uint8 b0_40 : 1;
			uint8 b0_80 : 1;
		};
		uint8 m_nFlags;
	};

	bool m_bWasTransfered;
	int8 m_nPrevOwnerID; // i8 in ctor -1, m_nOwnerID i8?
	uint8 m_nOwnerID;
	bool m_bIsNewSync;
	//int8 m_pad0;
	int16 m_nPrevID; // i16 in ctor -1, m_nID i16?
	uint16 m_nID;
	uint16 m_nTime;
	uint16 m_nDeltaTime;
	uint16 m_nLastSentFrame;
	std::deque<tSyncEntry> m_vSync;
	std::deque<tSyncEntry> m_vSyncB;
	uElementSync m_pSync;
	cInterestZone* m_pZone;
	CEntity* m_pEntity; // from who created (CPed, CAutomobile, etc), native gta entity

	sElement();

	static ElementCapability Capability() { return reinterpret_cast<ElementCapability>(sElement::Capability); }

	// 0 nil
	virtual ElementCapability GetCapability(); // vcs vft : 1
	virtual bool HasCapability(ElementCapability capability); // vcs vft : 2
	virtual ~sElement(); // vcs vft : 3
	virtual sElementSync* CreateSync() = 0; // vcs vft : 4 __purecall
	virtual void DisposeSync(sElementSync* pSync) = 0; // vcs vft : 5 __purecall // old DeleteSync
	virtual sElementSync* CreateSyncFromOther(sElementSync* pSync) = 0; // vcs vft : 6 __purecall // old CopyInfo, CopySync, CreateSyncFromOther
	virtual bool HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) = 0; // vcs vft : 7 __purecall // old IsSyncEqual, CompareSync, because it doesn't check all sync fields
	virtual eElementType GetType() = 0; // vcs vft : 8 __purecall
	virtual void ApplyClientSync(uint16 time); // vcs vft: 9 // old OnClientCreate
	virtual void Update(uint16 time); // vcs vft: 10 // from sElement::FindSync
	virtual bool WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) = 0; // vcs vft: 11 __purecall // time can be -1 (0 - 1)
	virtual void ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) = 0; // vcs vft: 12 __purecall
	virtual void ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime); // vcs vft: 13 // CreateFromSync, ReceiveEntity, OnReceiveTransfer, mk vehice, ped etc
	virtual void UpdateDelta(sElementSync* pSync, uint16 nTimeDelta); // vcs vft: 14 // [nTime ticks when > 0] // todo check u16/float
	virtual void ApplyDeltaState(sElementSync* pSync, uint16 nTimeDelta); // vcs vft: 15 // [NO PREV SYNC] // todo check u16/float
	virtual void InterpolateDeltaState(sElementSync* pSyncA, sElementSync* pSyncB, float fDelta); // vcs vft: 16 // UpdateDeltaState [WITH PREV SYNC] pSyncA curr for mod, pSyncB our target
	virtual bool IsTransferable(void) { return true; } // vcs vft: 17
	virtual void TransferEntity(int16 nDestPlayer); // vcs vft: 18
	virtual void RegisterSelf(); // vcs vft: 19
	virtual void RegisterSelfWithOwner(uint8 nOwner, uint16 nID); // name hiding avoid // vcs vft: 20 // last sElement

	void PurgeAttached(); // TODO: its in scalar double vft, recheck impl

	void AttachSync(uint16 nTime, sElementSync* pSync);
	uElementSync FindSync(uint16 nTime, bool* bIsNewSync); // get or create
	uElementSync GetSyncWithTime(uint16 nTime);
	void RegisterZone(cInterestZone* pZone);
	sPlayer* GetOwnerPlayer();
	void ClearSyncs();
	void DisposeAttachedDelta(uint16 nNetTime, uint16 nZoneTime);
	void DisposeAttached(uint16 nTime);
	uElementSync GetSyncWithTime2(uint16 nState, uint16 nBasis);
	void DisposeFrame(uint16 nTime);

	inline bool WasTransfered(void) { return m_bWasTransfered; }
	inline uElementSync GetSync() {
		if (m_pSync.element == nil) {
			bool bIsNewSync = false;
			m_pSync = FindSync(m_nTime, &bIsNewSync);
			m_bIsNewSync = bIsNewSync;
		}
		return m_pSync;
	}
	inline uint16 GetOwner() { return m_nOwnerID; };
	inline void SetOwner(uint16 owner) { m_nOwnerID = owner; };
	inline uint16 GetID() { return m_nID; };
	inline void SetID(uint16 id) { m_nID = id; }
	inline CEntity* GetEntity() { return m_pEntity; };
	inline void SetEntity(CEntity* pEntity) { m_pEntity = pEntity; };
	/*inline*/ sPeerState* GetPeer(); // custom
	/*inline*/ uint32 GetSyncCount(bool owned); // custom

	inline tSyncEntry* FindSyncEntry(sElementSync* pSync, bool* bFromMainBuffer = nil) {
		if (!pSync) return nil;
		if (bFromMainBuffer) *bFromMainBuffer = false;
		for (auto& sM : m_vSync) {
			if (sM.m_pAttachedElement.element == pSync) {
				if (bFromMainBuffer) *bFromMainBuffer = true;
				return &sM;
			}
		}
		for (auto& sM : m_vSyncB) {
			if (sM.m_pAttachedElement.element == pSync)
				return &sM;
		}
		return nil;
	}

	inline uint32 CalcSyncHash() {
		uint32 GetSyncSizeByElement(sElement* pElement);
		return fast_hash32(GetSync().element, GetSyncSizeByElement(this));
	}
};

extern bool gb_mp_will_destroy_elem; // leftover
