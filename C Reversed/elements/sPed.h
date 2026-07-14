/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include <vector>

#include "common.h"
#include "Ped.h"
#include "Fire.h"

#include "multiplayer/elements/sElementPhysical.h"

enum
{
	NUM_MULTIPLAYER_SYNC_ANIMS = 5,
};

enum ePedSync
{
	MP_PKTD_PED_EQUAL              = 0,
	MP_PKTD_PED_MODEL_TYPE         = BIT(0),  // m_pPhyElem->m_modelIndex, m_nPedType, (m_nCopModelIndex if cop)
	MP_PKTD_PED_VEHICLE_ID         = BIT(1),  // m_nVehicleID
	MP_PKTD_PED_POSITION_XY        = BIT(2),  // m_matrix.pos.x, m_matrix.pos.y
	MP_PKTD_PED_POSITION_Z         = BIT(3),  // m_matrix.pos.z
	MP_PKTD_PED_ROTATION_CUR       = BIT(4),  // m_fRotationCur
	MP_PKTD_PED_ROTATION_DEST      = BIT(5),  // m_fRotationDest
	MP_PKTD_PED_ANIM_COUNT         = BIT(6),  // m_nAnimCount
	MP_PKTD_PED_ANIM_0             = BIT(7),  // m_aPedAnims[0]
	MP_PKTD_PED_ANIM_1             = BIT(8),  // m_aPedAnims[1]
	MP_PKTD_PED_ANIM_2             = BIT(9),  // m_aPedAnims[2]
	MP_PKTD_PED_ANIM_3             = BIT(10), // m_aPedAnims[3]
	MP_PKTD_PED_ANIM_4             = BIT(11), // m_aPedAnims[4]
	MP_PKTD_PED_STATE              = BIT(12), // m_nPedState
	MP_PKTD_PED_HEALTH             = BIT(13), // m_nHealth
	MP_PKTD_PED_IK_FLAGS           = BIT(14), // m_CompressedPedIK.m_flags
	MP_PKTD_PED_IK_DATA            = BIT(15), // m_CompressedPedIK (yaw/pitch)
	MP_PKTD_PED_WEAPON_TYPE        = BIT(16), // m_eCurWeaponType
	MP_PKTD_PED_BODY_PART_CAME_OFF = BIT(17), // m_bBodyPartJustCameOff
	MP_PKTD_PED_DIE_ANIM_PLAYING   = BIT(18), // m_bIsPedDieAnimPlaying
	MP_PKTD_PED_PLAYER_TEAM        = BIT(19), // m_nPlayerTeam
	MP_PKTD_PED_PHYS_LVCS_UNK1     = BIT(20), // m_bPhys_lvcs_unk_1
	MP_PKTD_PED_MOVE_SPEED         = BIT(21), // m_vecMoveSpeed
	MP_PKTD_PED_PEER_LOCK_ON       = BIT(22), // m_nPeerLockOnMG
	MP_PKTD_PED_SURFACE_TOUCHED    = BIT(23), // m_nSurfaceTouched
	MP_PKTD_PED_MOVE_STATE         = BIT(24), // m_nMoveState
	MP_PKTD_PED_WEAPON_STATE       = BIT(25), // m_eCurWeaponState (m_eCurWeaponType == 11)
	MP_PKTD_PED_OBJECTIVE          = BIT(26), // m_nObjective
	MP_PKTD_PED_PHYS_LVCS_FLAGS_B  = BIT(27), // m_nPedPhys_lvcs_unk_flagsB
	MP_PKTD_PED_FULL               = -1,      // full diff
};
#define MP_PKTD_PED_ANIM_MASK(animIndex) (BIT(7) << (animIndex))


enum ePedIKSync
{
	MP_PKTD_PED_IK_EQUAL			= 0,
	MP_PKTD_PED_IK_HEAD_PITCH		= BIT(0),	// m_headOrient.pitch
	MP_PKTD_PED_IK_HEAD_YAW			= BIT(1),	// m_headOrient.yaw
	MP_PKTD_PED_IK_TORSO_PITCH		= BIT(2),	// m_torsoOrient.pitch
	MP_PKTD_PED_IK_TORSO_YAW		= BIT(3),	// m_torsoOrient.yaw
	MP_PKTD_PED_IK_UPPER_ARM_PITCH	= BIT(4),	// m_upperArmOrient.pitch
	MP_PKTD_PED_IK_UPPER_ARM_YAW	= BIT(5),	// m_upperArmOrient.yaw
	MP_PKTD_PED_IK_LOWER_ARM_PITCH	= BIT(6),	// m_lowerArmOrient.pitch
	MP_PKTD_PED_IK_LOWER_ARM_YAW	= BIT(7),	// m_lowerArmOrient.yaw
	MP_PKTD_PED_IK_HAND_ROLL		= BIT(8),	// m_nHandRoll
	MP_PKTD_PED_IK_UA_ROLL			= BIT(9),	// m_nUaRoll
	MP_PKTD_PED_IK_FULL				= -1,		// full diff
};

struct CompressedLimbOrientation {
public:
	uint16 pitch;
	uint16 yaw;
};
static_assert(sizeof(CompressedLimbOrientation) == 4, "error size CompressedLimbOrientation");

struct sPedIKSync {
public:
	uint8 m_flags; // CPedIK flags
	//int8 m_pad0;
	CompressedLimbOrientation m_headOrient;
	CompressedLimbOrientation m_torsoOrient;
	CompressedLimbOrientation m_upperArmOrient;
	CompressedLimbOrientation m_lowerArmOrient;
	uint16 m_nUaRoll; // recheck u16
	uint16 m_nHandRoll; // recheck u16

	inline sPedIKSync() {}
	sPedIKSync(CPedIK* pPedIK);
	~sPedIKSync() {}

	static uint16 CompareSyncState(sPedIKSync* pSync, sPedIKSync* pLastSync);
	static void PerformWriteSync(sWriteSyncStream* pSyncStream, sPedIKSync* pSync, uint16 nDiffMaskA, uint16 nDiffMask);
	static void ReadSyncFromStream(sReadSyncStream* pSyncStream, sPedIKSync* pOutSync);

	// 16384.0f BAMS
	static uint16 PackFloatRad(float f) { return f * (16384.0f / M_PI); } // 5215.189f
	static float UnpackFloatDeg(uint16 v) { return v * (180.0f / 16384.0f); } // 0.010986328f

	inline static uint16 PackFloatDeg(float f) { return f * (16384.0f / 180.0f); }
	inline static float UnpackFloatRad(uint16 v) { return v * (M_PI / 16384.0f); }
	inline static uint16 PackFloat(float f, float maxValue) { return f * (16384.0f / maxValue); }
	inline static float UnpackFloat(uint16 v, float maxValue) { return v * (maxValue / 16384.0f); }

	void OnPreRender(sPed* pPed);
#if !defined(FINAL) && !defined(MASTER)
	void Dump();
#endif
};
static_assert(sizeof(sPedIKSync) == 22, "error size sPedIKSync");


enum ePedAnimSync
{
	MP_PKTD_PED_ANIM_EQUAL          = 0,
	MP_PKTD_PED_ANIM_BLEND_AMOUNT   = BIT(0), // m_fBlendAmount
	MP_PKTD_PED_ANIM_BLEND_DELTA    = BIT(1), // m_fBlendDelta
	MP_PKTD_PED_ANIM_CURRENT_TIME   = BIT(2), // m_fCurrentTime
	MP_PKTD_PED_ANIM_SPEED          = BIT(3), // m_fSpeed
	MP_PKTD_PED_ANIM_GROUP_ID       = BIT(4), // m_nGroupId
	MP_PKTD_PED_ANIM_ANIM_ID        = BIT(5), // m_nAnimId
	MP_PKTD_PED_ANIM_FLAGS_LOW      = BIT(6), // m_nFlags low word
	MP_PKTD_PED_ANIM_FLAGS_HIGH     = BIT(7), // m_nFlags high word
	MP_PKTD_PED_ANIM_FULL           = -1,   // full diff
};

struct sPedAnimSync {
public:
	float m_fBlendAmount;
	float m_fBlendDelta; // how much blendAmount changes over time
	float m_fCurrentTime;
	float m_fSpeed;
	int16 m_nGroupId;  // ID of CAnimBlendAssocGroup this is in
	int16 m_nAnimId;
	int32 m_nFlags; // eAnimAssocFlags
};
static_assert(sizeof(sPedAnimSync) == 24, "error size sPedAnimSync");

struct sPedSync : sElementPhysicalSync {
public:
	int16 m_nVehicleID;
	uint16 m_nHealth;
	uint8 m_nPedState; // PedState
	uint8 m_nObjective; // eObjective
	uint8 m_nPedType; // ePedType
	uint8 m_eCurWeaponType; // eWeaponType
	float m_fRotationCur;
	float m_fRotationDest;
	sPedIKSync m_CompressedPedIK;
	uint8 m_nAnimCount;
	//uint8 field_D7; // pad
	sPedAnimSync m_aPedAnims[NUM_MULTIPLAYER_SYNC_ANIMS];
	bool m_bBodyPartJustCameOff;
	bool m_bIsPedDieAnimPlaying;
	int16 m_nPlayerTeam;
	uint8 m_bPhys_lvcs_unk_1; // bool?
	int8 m_nPeerLockOnMG;
	int16 m_nSurfaceTouched; // eSurfaceType
	uint8 m_nMoveState; // eMoveState
	uint8 m_eCurWeaponState; // eWeaponState

	// LVCS Flags B  wtf? This is stupid, sElementPhysicalSync already has it!
	union
	{
		struct
		{
			uint8 b154_1 : 1;
			uint8 bHasBlipPhys : 1;
			uint8 b154_4 : 1;
			uint8 bNoRadarForEnemy : 1;
			uint8 b154_10 : 1;
			uint8 b154_20 : 1; // has gta element?
			uint8 b154_40 : 1;
			uint8 b154_80 : 1;
		};
		uint8 m_nPedPhys_lvcs_unk_flagsB; // added prefix Ped
	};

	//uint8 field_15B[5]; // pad?

	sPedSync();
	sPedSync(CPed* pPed);
	sPedSync(const sPedSync& other);
	~sPedSync() override;

	bool Compare(const sPedSync& other);
	void CopyAnimations(RpClump* clump);
#if !defined(FINAL) && !defined(MASTER)
	void Dump();
#endif

#ifdef MULTIGAME_ELEMENTS_IMPROVEMENTS
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_PED; }
#endif

	inline int16 GetVehicleID() { return m_nVehicleID; }
	inline bool InVehicle() { return GetVehicleID() != -1; }
	inline bool Dead(void) { return m_nPedState == PedState::PED_DEAD; }
	inline bool Dying(void) { return m_nPedState == PedState::PED_DIE; }
	inline bool DyingOrDead(void) { return m_nPedState == PedState::PED_DIE || m_nPedState == PedState::PED_DEAD; }
};

#pragma pack(push, 1)
struct tPedSyncsDeltas
{
	uint32 nPedDiff;	// ePedSync 0-27 bits
	union
	{
		uint64 nAnimDiff;	// ePedAnimSync 8 bit * NUM_MULTIPLAYER_SYNC_ANIMS 5 = 40 bit
		uint8 aAnimDiff[NUM_MULTIPLAYER_SYNC_ANIMS]; // ePedAnimSync
	};
	uint16 nIkDiff;		// ePedIKSync 0-9 bits

	inline void SetEqual()
	{
		memset(this, ePedSync::MP_PKTD_PED_EQUAL, sizeof(tPedSyncsDeltas));
		//nPedDiff = ePedSync::MP_PKTD_PED_EQUAL;
		//nAnimDiff = ePedAnimSync::MP_PKTD_PED_ANIM_EQUAL;
		//for (uint32 i = 0; i < NUM_MULTIPLAYER_SYNC_ANIMS; i++)
		//	aAnimDiff[i] = ePedAnimSync::MP_PKTD_PED_ANIM_EQUAL;
		//nIkDiff = ePedIKSync::MP_PKTD_PED_IK_EQUAL;
	}

	inline void SetDifference()
	{
		memset(this, static_cast<uint32>(ePedSync::MP_PKTD_PED_FULL), sizeof(tPedSyncsDeltas)); // 0xFFFFFFFF
		//nPedDiff = ePedSync::MP_PKTD_PED_FULL;
		//nAnimDiff = ePedAnimSync::MP_PKTD_PED_ANIM_FULL;
		//for (uint32 i = 0; i < NUM_MULTIPLAYER_SYNC_ANIMS; i++)
		//	aAnimDiff[i] = ePedAnimSync::MP_PKTD_PED_ANIM_FULL;
		//nIkDiff = ePedIKSync::MP_PKTD_PED_IK_FULL;
	}
};
static_assert(sizeof(tPedSyncsDeltas) == 14, "sizeof(tPedSyncsDeltas)");
#pragma pack(pop)

class cPedMG : public cPhysicalMG {
public:
	eWeaponType m_storedWeapon;
	RpAtomic* m_pWeaponModel;
	CFire* m_pFire;
	float field_17C;
	//
	uint16 m_nTime;
	float field_1A4;
#ifndef GTA_LIBERTY
	CRGBA m_aColors[NUM_PED_COLOURS]; // 0x1A8
#endif
#ifdef DEBUG_MULTIGAME
	const char* szDebugMessages[10];
#endif

	cPedMG(sElement* elem);

	~cPedMG() override;
	void SetModelIndex(uint32 id) override;
	void PreRender(void) override;
	void Render(void) override;
	void UpdateAnim(void) override;
	bool SetupLighting(void) override;
	void RemoveLighting(bool reset) override;

	void AddWeaponModel(int32 model);
	void RemoveWeaponModel(int32 model);
	bool IsLocalPlayerPed();

	inline void Respawn() { if (m_pFire) m_pFire->Extinguish(); }
#ifdef DEBUG_MULTIGAME
	inline const char** GetDebugMessages() { return szDebugMessages; }
#endif
};

PedState GetPedState(CPhysical* pPed);

struct sPed : sElementPhysical {
public:
	uint16 m_nPedTime; // recheck
	//int8 m_pad1[2];
	CPedStats* m_pedStats;
	float m_headingRate;
	AssocGroupId m_animGroup;
	AnimBlendFrameData* m_pFrames[PED_NODE_MAX];
	//int8 m_unk1[2];
	int8 m_nCopModelIndex;
	//int8 m_unk2[1];
	int16 m_nTeamID;
	bool m_bHasReload;
	bool m_bHasShot;
	CVector2D m_vecAnimMoveDelta;

	static std::vector<sPed*> ms_peds;
	static int32 ms_nNumberOfSyncedPeds;

	sPed();
	sPed(CPed* ped);

	static ElementCapability Capability() { return reinterpret_cast<ElementCapability>(sPed::Capability); }

	ElementCapability GetCapability() override;
	bool HasCapability(ElementCapability capability) override;
	~sPed() override;
	sElementSync* CreateSync() override;
	void DisposeSync(sElementSync* pSync) override;
	sElementSync* CreateSyncFromOther(sElementSync* pSync) override;
	bool HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) override;
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_PED; }
	void ApplyClientSync(uint16 nTime) override;
	void Update(uint16 nTime) override;
	bool WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) override;
	void ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) override;
	void ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) override;
	void UpdateDelta(sElementSync* pSync, uint16 nTimeDelta) override;
	void ApplyDeltaState(sElementSync* pSync, uint16 nTimeDelta) override;
	void InterpolateDeltaState(sElementSync* pSyncA, sElementSync* pSyncB, float fDelta) override;
	bool IsTransferable(void) override;
	void TransferEntity(int16 nDestPlayer) override;

	// Ped main
	void CompareSyncState(tPedSyncsDeltas* pDiff, sPedSync* pSync, sPedSync* pLastSync, uint32 nDelta);
	bool WriteSyncDelta(sWriteSyncStream* pSyncStream, sPedSync* pSync, sPedSync* pLastSync, uint32 nDelta);
	void PerformWriteSync(sWriteSyncStream* pSyncStream, sPedSync* pSync, tPedSyncsDeltas* pDiff);

	// Anim
	uint8 CompareAnimSyncState(sPedAnimSync* pSync, sPedAnimSync* pLastSync, int32 nDelta); // old name GetAnimDelta()
	void PerformWriteAnimSync(sWriteSyncStream* pSyncStream, sPedAnimSync* pSync, uint8 nDiffMask);
	void ReadAnimSyncFromStream(sReadSyncStream* pSyncStream, sPedAnimSync* pOutSync);

	bool IsPedHeadAbovePos(float fZOffset);
	bool GetPedBoneWorldPosition(CVector& pos, int32 nNode); // PedNode
	int16 GetTeamID() { return m_nTeamID; }
	void SetTeamID(int16 id) { m_nTeamID = id; }
	void OnPreRender(uint16 nTime);
	void UpdateAnim(uint16 nTime, float fDelta);

	inline void Respawn() { if (m_pPhyElem) ((cPedMG*)m_pPhyElem)->Respawn(); }
};