/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"
#include "DMAudio.h"
#include "WeaponType.h"
#include "WeaponEffects.h"
#include "ProjectileInfo.h"
#include "Ped.h"
#include "PlayerPed.h"
#include "PlayerInfo.h"
#include "Camera.h"
#include "RpAnimBlend.h"
#include "VisibilityPlugins.h"
#include "Streaming.h"
#include "AnimManager.h"
#include "Renderer.h"
#include "Lights.h"
#include "PointLights.h"
#include "CopPed.h"
#include "Population.h"
#include "Particle.h"
#include "Shadows.h"
#include "TimeCycle.h"
#include "Pad.h"
#include "Pools.h"
#include "AudioManager.h"

#include "multiplayer/elements/sPed.h"
#include "multiplayer/elements/sVehicle.h"
#include "multiplayer/elements/sAutomobile.h"
#include "multiplayer/elements/sBike.h"
#ifndef GTA_LIBERTY
#include "multiplayer/elements/sBmx.h"
#endif
#include "multiplayer/elements/sSyncStream.h"
#include "multiplayer/MultiGame.h"


sPedIKSync::sPedIKSync(CPedIK* pPedIK)
{
	m_flags = pPedIK->m_flags;

	if (m_flags & CPedIK::PED_IK_HEAD_ORIENT_FLAG)
	{
		m_headOrient.pitch = PackFloatRad(pPedIK->m_headOrient.pitch);
		m_headOrient.yaw = PackFloatRad(pPedIK->m_headOrient.yaw);
	}

	if (m_flags & CPedIK::PED_IK_UPPER_ARM_ORIENT_FLAG)
	{
		m_upperArmOrient.pitch = PackFloatRad(pPedIK->m_upperArmOrient.pitch);
		m_upperArmOrient.yaw = PackFloatRad(pPedIK->m_upperArmOrient.yaw);
		m_nUaRoll = PackFloatRad(pPedIK->m_fUaRoll);
		m_nHandRoll = PackFloatRad(pPedIK->m_fHandRoll);
	}

	if (m_flags & CPedIK::PED_IK_LOWER_ARM_ORIENT_FLAG)
	{
		m_lowerArmOrient.pitch = PackFloatRad(pPedIK->m_lowerArmOrient.pitch);
		m_lowerArmOrient.yaw = PackFloatRad(pPedIK->m_lowerArmOrient.yaw);
	}

	if (m_flags & (CPedIK::PED_IK_TORSO_ORIENT_FLAG_2 | CPedIK::PED_IK_TORSO_ORIENT_FLAG_1))
	{
		m_torsoOrient.pitch = PackFloatRad(pPedIK->m_torsoOrient.pitch);
		m_torsoOrient.yaw = PackFloatRad(pPedIK->m_torsoOrient.yaw);
	}
}

// Not checks: m_nUaRoll, m_nHandRoll
uint16 sPedIKSync::CompareSyncState(sPedIKSync* pSync, sPedIKSync* pLastSync)
{
	uint16 mask = ePedIKSync::MP_PKTD_PED_IK_EQUAL;

	if (pSync->m_flags & CPedIK::PED_IK_HEAD_ORIENT_FLAG)
	{
		if (pLastSync->m_headOrient.yaw != pSync->m_headOrient.yaw)
			mask |= ePedIKSync::MP_PKTD_PED_IK_HEAD_YAW;
		if (pLastSync->m_headOrient.pitch != pSync->m_headOrient.pitch)
			mask |= ePedIKSync::MP_PKTD_PED_IK_HEAD_PITCH;
	}

	if (pSync->m_flags & CPedIK::PED_IK_UPPER_ARM_ORIENT_FLAG)
	{
		if (pLastSync->m_upperArmOrient.yaw != pSync->m_upperArmOrient.yaw)
			mask |= ePedIKSync::MP_PKTD_PED_IK_UPPER_ARM_YAW;
		if (pLastSync->m_upperArmOrient.pitch != pSync->m_upperArmOrient.pitch)
			mask |= ePedIKSync::MP_PKTD_PED_IK_UPPER_ARM_PITCH;
	}

	if (pSync->m_flags & CPedIK::PED_IK_LOWER_ARM_ORIENT_FLAG)
	{
		if (pLastSync->m_lowerArmOrient.yaw != pSync->m_lowerArmOrient.yaw)
			mask |= ePedIKSync::MP_PKTD_PED_IK_LOWER_ARM_YAW;
		if (pLastSync->m_lowerArmOrient.pitch != pSync->m_lowerArmOrient.pitch)
			mask |= ePedIKSync::MP_PKTD_PED_IK_LOWER_ARM_PITCH;
	}

	if (pSync->m_flags & CPedIK::PED_IK_TORSO_ORIENT_FLAG_2)
	{
		if (pLastSync->m_torsoOrient.yaw != pSync->m_torsoOrient.yaw)
			mask |= ePedIKSync::MP_PKTD_PED_IK_TORSO_YAW;
		if (pLastSync->m_torsoOrient.pitch != pSync->m_torsoOrient.pitch)
			mask |= ePedIKSync::MP_PKTD_PED_IK_TORSO_PITCH;
	}

	return mask;
}

void sPedIKSync::PerformWriteSync(sWriteSyncStream* pSyncStream, sPedIKSync* pSync, uint16 nDiffMaskA, uint16 nDiffMask)
{
	pSyncStream->WriteU16(nDiffMask);

	if (nDiffMask & ePedIKSync::MP_PKTD_PED_IK_HEAD_PITCH)
		pSyncStream->WriteU16(pSync->m_headOrient.pitch);

	if (nDiffMask & ePedIKSync::MP_PKTD_PED_IK_HEAD_YAW)
		pSyncStream->WriteU16(pSync->m_headOrient.yaw);

	if (nDiffMask & ePedIKSync::MP_PKTD_PED_IK_TORSO_PITCH)
		pSyncStream->WriteU16(pSync->m_torsoOrient.pitch);

	if (nDiffMask & ePedIKSync::MP_PKTD_PED_IK_TORSO_YAW)
		pSyncStream->WriteU16(pSync->m_torsoOrient.yaw);

	if (nDiffMask & ePedIKSync::MP_PKTD_PED_IK_UPPER_ARM_PITCH)
		pSyncStream->WriteU16(pSync->m_upperArmOrient.pitch);

	if (nDiffMask & ePedIKSync::MP_PKTD_PED_IK_UPPER_ARM_YAW)
		pSyncStream->WriteU16(pSync->m_upperArmOrient.yaw);

	if (nDiffMask & ePedIKSync::MP_PKTD_PED_IK_LOWER_ARM_PITCH)
		pSyncStream->WriteU16(pSync->m_lowerArmOrient.pitch);

	if (nDiffMask & ePedIKSync::MP_PKTD_PED_IK_LOWER_ARM_YAW)
		pSyncStream->WriteU16(pSync->m_lowerArmOrient.yaw);

	if (nDiffMask & ePedIKSync::MP_PKTD_PED_IK_UA_ROLL)
		pSyncStream->WriteU16(pSync->m_nUaRoll);

	if (nDiffMask & ePedIKSync::MP_PKTD_PED_IK_HAND_ROLL)
		pSyncStream->WriteU16(pSync->m_nHandRoll);
}

void sPedIKSync::ReadSyncFromStream(sReadSyncStream* pSyncStream, sPedIKSync* pOutSync)
{
	uint16 nDiffMask = pSyncStream->ReadU16();

	if (nDiffMask & ePedIKSync::MP_PKTD_PED_IK_HEAD_PITCH)
		pOutSync->m_headOrient.pitch = pSyncStream->ReadU16();

	if (nDiffMask & ePedIKSync::MP_PKTD_PED_IK_HEAD_YAW)
		pOutSync->m_headOrient.yaw = pSyncStream->ReadU16();

	if (nDiffMask & ePedIKSync::MP_PKTD_PED_IK_TORSO_PITCH)
		pOutSync->m_torsoOrient.pitch = pSyncStream->ReadU16();

	if (nDiffMask & ePedIKSync::MP_PKTD_PED_IK_TORSO_YAW)
		pOutSync->m_torsoOrient.yaw = pSyncStream->ReadU16();

	if (nDiffMask & ePedIKSync::MP_PKTD_PED_IK_UPPER_ARM_PITCH)
		pOutSync->m_upperArmOrient.pitch = pSyncStream->ReadU16();

	if (nDiffMask & ePedIKSync::MP_PKTD_PED_IK_UPPER_ARM_YAW)
		pOutSync->m_upperArmOrient.yaw = pSyncStream->ReadU16();

	if (nDiffMask & ePedIKSync::MP_PKTD_PED_IK_LOWER_ARM_PITCH)
		pOutSync->m_lowerArmOrient.pitch = pSyncStream->ReadU16();

	if (nDiffMask & ePedIKSync::MP_PKTD_PED_IK_LOWER_ARM_YAW)
		pOutSync->m_lowerArmOrient.yaw = pSyncStream->ReadU16();

	if (nDiffMask & ePedIKSync::MP_PKTD_PED_IK_UA_ROLL)
		pOutSync->m_nUaRoll = pSyncStream->ReadU16();

	if (nDiffMask & ePedIKSync::MP_PKTD_PED_IK_HAND_ROLL)
		pOutSync->m_nHandRoll = pSyncStream->ReadU16();
}

// TODO: merge CPedIK::PointGunInDirection PointGunInDirectionUsingArm, m_pedIK.PointGunInDirection(m_fLookDirection, ((CPlayerPed*)this)->m_fFPSMoveHeading);
// its rotate RtQuat directly for native hidden ped
// todo? check pedstate? aiming? calc from camera direction yaw and pitch and rotate RtQuat for aiming
// see cPedMG::PreRender
void sPedIKSync::OnPreRender(sPed* pPed)
{
	static const RwV3d Xaxis = { 1.0f, 0.0f, 0.0f };
	static const RwV3d Yaxis = { 0.0f, 1.0f, 0.0f };
	static const RwV3d Zaxis = { 0.0f, 0.0f, 1.0f };

	if (m_flags & CPedIK::PED_IK_HEAD_ORIENT_FLAG)
	{
		RtQuat* headQuat = &pPed->m_pFrames[PED_HEAD]->hanimFrame->q;
		RtQuatRotate(headQuat, &Zaxis, sPedIKSync::UnpackFloatDeg(m_headOrient.yaw), rwCOMBINEREPLACE);
		RtQuatRotate(headQuat, &Xaxis, sPedIKSync::UnpackFloatDeg(m_headOrient.pitch), rwCOMBINEPRECONCAT);
	}

	if (m_flags & CPedIK::PED_IK_UPPER_ARM_ORIENT_FLAG)
	{
		RtQuat* upperArmQuat = &pPed->m_pFrames[PED_UPPERARMR]->hanimFrame->q;
		RtQuatRotate(upperArmQuat, &Xaxis, sPedIKSync::UnpackFloatDeg(m_nUaRoll), rwCOMBINEREPLACE);
		RtQuatRotate(upperArmQuat, &Yaxis, -sPedIKSync::UnpackFloatDeg(m_upperArmOrient.yaw), rwCOMBINEPOSTCONCAT);
		RtQuatRotate(upperArmQuat, &Zaxis, -sPedIKSync::UnpackFloatDeg(m_upperArmOrient.pitch) - 90.0f, rwCOMBINEPOSTCONCAT);
		RtQuat* handQuat = &pPed->m_pFrames[PED_HANDR]->hanimFrame->q;
		RtQuatRotate(handQuat, &Xaxis, sPedIKSync::UnpackFloatDeg(m_nHandRoll), rwCOMBINEPRECONCAT);
	}

	if (m_flags & CPedIK::PED_IK_LOWER_ARM_ORIENT_FLAG)
	{
		// CPed::TransformToNode
		assert(pPed->GetPhysical() && RwObjectGetType(pPed->GetPhysical()->m_rwObject) == rpCLUMP);
		RpClump* pClump = (RpClump*)pPed->GetPhysical()->m_rwObject;
		RpHAnimHierarchy* hier = GetAnimHierarchyFromSkinClump(pClump);
		int32 idx = RpHAnimIDGetIndex(hier, pPed->m_pFrames[PED_FOREARMR]->nodeID);
		//RwMatrix* lowerArm = &RpHAnimHierarchyGetMatrixArray(hier)[idx];
		CAnimBlendClumpData* clumpData = *RPANIMBLENDCLUMPDATA(pClump);
		RtQuat* lowerArmQuat = &clumpData->frames[idx].hanimFrame->q;
		RtQuatRotate(lowerArmQuat, &Zaxis, -sPedIKSync::UnpackFloatDeg(m_lowerArmOrient.pitch), rwCOMBINEREPLACE);
		RtQuatRotate(lowerArmQuat, &Xaxis, -sPedIKSync::UnpackFloatDeg(m_lowerArmOrient.yaw), rwCOMBINEPOSTCONCAT);
	}

	if (m_flags & CPedIK::PED_IK_TORSO_ORIENT_FLAG_2)
	{
		// CPed::TransformToNode
		assert(pPed->GetPhysical() && RwObjectGetType(pPed->GetPhysical()->m_rwObject) == rpCLUMP);
		RpClump* pClump = (RpClump*)pPed->GetPhysical()->m_rwObject;
		RpHAnimHierarchy* hier = GetAnimHierarchyFromSkinClump(pClump);
		RwMatrix* matrixArray = RpHAnimHierarchyGetMatrixArray(hier);
		float atX = -matrixArray[2].at.x;
		float atY = -matrixArray[2].at.y;
		float walkAngle;
		if (atX == 0.0f && atY == 0.0f)
			walkAngle = 0.0f;
		else
			walkAngle = atan2f(atY, atX);
		float deltaAngle = CGeneral::LimitRadianAngle(walkAngle - pPed->GetSync().ped->m_fRotationCur);
		RwV3d customAxis = { 0.0f, sinf(deltaAngle), cosf(deltaAngle) };
		RtQuat* torsoQuat = &pPed->m_pFrames[PED_MID]->hanimFrame->q;
		RtQuatRotate(torsoQuat, &customAxis, sPedIKSync::UnpackFloatDeg(m_torsoOrient.yaw), rwCOMBINEPOSTCONCAT);
		RtQuatRotate(torsoQuat, &Xaxis, sPedIKSync::UnpackFloatDeg(m_torsoOrient.pitch), rwCOMBINEPOSTCONCAT);
	}
	else if (m_flags & CPedIK::PED_IK_TORSO_ORIENT_FLAG_1)
	{
		RtQuat* torsoQuat = &pPed->m_pFrames[PED_MID]->hanimFrame->q;
		RtQuatRotate(torsoQuat, &Xaxis, sPedIKSync::UnpackFloatDeg(m_torsoOrient.pitch), rwCOMBINEREPLACE);
		RtQuatRotate(torsoQuat, &Zaxis, sPedIKSync::UnpackFloatDeg(m_torsoOrient.yaw), rwCOMBINEPRECONCAT);
	}

	// remove it
	// check CPedIK::PointGunInDirection CPedIK::PointGunInDirectionUsingArm new ik flags, probably sync arm
#ifdef FIX_BUGS // copy rotation from native PointGunInDirection PointGunInDirectionUsingArm
	// todo its bug arm leg to up
	//CPed* pNativePed = (CPed*)pPed->GetEntity();
	//if (!pNativePed || npc)
	//	return;

	// todo return if not player (npc)

	//for (int32 i = 0; i < PED_NODE_MAX; i++)
	//{
	//	AnimBlendFrameData* pFrame = pNativePed->m_pFrames[i];

	//	if (pFrame)
	//	{
	//		RpHAnimStdInterpFrame* a = pFrame->hanimFrame;
	//		if (a)
	//			pPed->m_pFrames[i]->hanimFrame->q = a->q;
	//	}
	//}


	// some test
	//// fine get from sync cam/yawpitch
	//// rotate
	//// IsMultiplayerPlayer()
	//if (GetElement().ped->GetID() == eElementID::MG_ELEMENT_PLAYER_PED_ID &&
	//	GetElement().ped->GetOwner() == TheMPGame.LocalPlayerID() &&
	//	GetElement().ped->GetEntity())
	//{
	//	//CPlayerPed* player = FindPlayerPed();
	//	CPlayerPed* player = (CPlayerPed*)GetElement().ped->GetEntity();
	//	assert(player && player->IsPlayer());

	//	// CPed::AimGun()
	//	//m_pedIK.PointGunInDirection(player->m_fLookDirection, player->m_fFPSMoveHeading);
	//	//sPlayer* pPlayer = (sPlayer*)cMultiGame::Instance().GetEntityForHandle(GetElement().element->GetOwner(), eElementID::MG_ELEMENT_PLAYER_ID);
	//	//if (pPlayer) {
	//	//	CVector& vecFront = pPlayer->GetSync().player->m_vCamFront;
	//	//}
	//}
#endif
}

#if !defined(FINAL) && !defined(MASTER)
void sPedIKSync::Dump()
{
	float headYaw = UnpackFloatDeg(m_headOrient.yaw);
	float headPitch = UnpackFloatDeg(m_headOrient.pitch);
	float torsoYaw = UnpackFloatDeg(m_torsoOrient.yaw);
	float torsoPitch = UnpackFloatDeg(m_torsoOrient.pitch);
	float upperArmYaw = UnpackFloatDeg(m_upperArmOrient.yaw);
	float upperArmPitch = UnpackFloatDeg(m_upperArmOrient.pitch);
	float lowerArmYaw = UnpackFloatDeg(m_lowerArmOrient.yaw);
	float lowerArmPitch = UnpackFloatDeg(m_lowerArmOrient.pitch);

	printf("=== sPedIKSync Dump ===\n");
	printf("Flags:           0x%02X\n", m_flags);
	printf("Head Orientation:\n");
	printf("  Yaw:   %u (%.2f) (0x%04X)\n", m_headOrient.yaw, headYaw, m_headOrient.yaw);
	printf("  Pitch: %u (%.2f) (0x%04X)\n", m_headOrient.pitch, headPitch, m_headOrient.pitch);
	printf("Torso Orientation:\n");
	printf("  Yaw:   %u (%.2f) (0x%04X)\n", m_torsoOrient.yaw, torsoYaw, m_torsoOrient.yaw);
	printf("  Pitch: %u (%.2f) (0x%04X)\n", m_torsoOrient.pitch, torsoPitch, m_torsoOrient.pitch);
	printf("Upper Arm Orientation:\n");
	printf("  Yaw:   %u (%.2f) (0x%04X)\n", m_upperArmOrient.yaw, upperArmYaw, m_upperArmOrient.yaw);
	printf("  Pitch: %u (%.2f) (0x%04X)\n", m_upperArmOrient.pitch, upperArmPitch, m_upperArmOrient.pitch);
	printf("Lower Arm Orientation:\n");
	printf("  Yaw:   %u (%.2f) (0x%04X)\n", m_lowerArmOrient.yaw, lowerArmYaw, m_lowerArmOrient.yaw);
	printf("  Pitch: %u (%.2f) (0x%04X)\n", m_lowerArmOrient.pitch, lowerArmPitch, m_lowerArmOrient.pitch);
	printf("Upper Arm Roll:  %u (0x%04X)\n", m_nUaRoll, m_nUaRoll);
	printf("Hand Roll:       %u (0x%04X)\n", m_nHandRoll, m_nHandRoll);
	printf("================================\n");
}
#endif


sPedSync::sPedSync() : sElementPhysicalSync(), m_CompressedPedIK()
{
	DECLARE_SYNC_CONSTRUCT(this);
	m_nVehicleID = -1;
	m_fRotationCur = 0.0f;
	m_fRotationDest = 0.0f;
	m_nAnimCount = 0;
	m_bBodyPartJustCameOff = false;
	m_bIsPedDieAnimPlaying = false;
	m_nPlayerTeam = 0;
	m_bPhys_lvcs_unk_1 = false;
	m_nPeerLockOnMG = -1;
	m_nSurfaceTouched = eSurfaceType::SURFACE_DEFAULT;
	m_nMoveState = eMoveState::PEDMOVE_NONE;
	m_eCurWeaponState = eWeaponState::WEAPONSTATE_READY;
	m_nPedPhys_lvcs_unk_flagsB = 0;
}

sPedSync::sPedSync(CPed* ped) : sElementPhysicalSync(ped), m_CompressedPedIK(&ped->m_pedIK)
{
	DECLARE_SYNC_CONSTRUCT(this);
	m_nHealth = ped->m_fHealth;
	m_nPedState = ped->m_nPedState;
	m_nObjective = ped->m_objective;
	m_nPedType = ped->m_nPedType;
	m_eCurWeaponType = ped->GetWeapon()->m_eWeaponType;
	m_fRotationCur = ped->m_fRotationCur;
	m_fRotationDest = ped->m_fRotationDest;
	//m_CompressedPedIK = sPedIKSync(&ped->m_pedIK);
	m_bBodyPartJustCameOff = ped->bBodyPartJustCameOff;
	m_bIsPedDieAnimPlaying = ped->bIsPedDieAnimPlaying;
	m_nPlayerTeam = ped->m_nTeamIdMG;
	m_bPhys_lvcs_unk_1 = ped->bPhys_lvcs_unk_1;
	m_nPeerLockOnMG = ped->m_nPeerLockOnMG;
	m_nSurfaceTouched = ped->m_nSurfaceTouched;
	m_nMoveState = ped->m_nMoveState;
	m_eCurWeaponState = ped->GetWeapon()->m_eWeaponState;
	m_nPedPhys_lvcs_unk_flagsB = ped->m_nPhys_lvcs_unk_flagsB;
	if (ped->b4E_8)
		b154_20 = true;
	CopyAnimations(ped->GetClump());
	if (m_nAnimCount == 0)
		CopyAnimations(ped->GetClump()); // huh?
	if (ped->m_holdPositionState > 0)
		SetMatrix(ped->m_storedHoldPositionMG.GetMatrix());
	m_nVehicleID = -1;
#ifdef GTA_LIBERTY
	if (ped->bInVehicle)
#else
	if (ped->InVehicle())
#endif
	{
		sElement* pElem = cMultiGame::Instance().GetElementFromEntity((CEntity*)ped->m_pMyVehicle);
		if (pElem != nil)
			m_nVehicleID = pElem->GetID();
	}
#if !defined(FINAL) && !defined(MASTER)
	//if (TheMPGame.FindPlayerPedMG()->GetEntity() == ped)
	//debug("sPedSync::sPedSync 0x%p hash: 0x%X\n", this, fast_hash32(this, sizeof(sPedSync)));
#endif
}

sPedSync::sPedSync(const sPedSync& other) : sElementPhysicalSync(other), m_CompressedPedIK()
{
	DECLARE_SYNC_CONSTRUCT(this);
	m_nVehicleID = other.m_nVehicleID;
	m_nHealth = other.m_nHealth;
	m_nPedState = other.m_nPedState;
	m_nObjective = other.m_nObjective;
	m_nPedType = other.m_nPedType;
	m_eCurWeaponType = other.m_eCurWeaponType;
	m_fRotationCur = other.m_fRotationCur;
	m_fRotationDest = other.m_fRotationDest;

	m_CompressedPedIK.m_flags = other.m_CompressedPedIK.m_flags;
	m_CompressedPedIK.m_headOrient.yaw = other.m_CompressedPedIK.m_headOrient.yaw;
	m_CompressedPedIK.m_headOrient.pitch = other.m_CompressedPedIK.m_headOrient.pitch;
	m_CompressedPedIK.m_torsoOrient.yaw = other.m_CompressedPedIK.m_torsoOrient.yaw;
	m_CompressedPedIK.m_torsoOrient.pitch = other.m_CompressedPedIK.m_torsoOrient.pitch;
	m_CompressedPedIK.m_upperArmOrient.yaw = other.m_CompressedPedIK.m_upperArmOrient.yaw;
	m_CompressedPedIK.m_upperArmOrient.pitch = other.m_CompressedPedIK.m_upperArmOrient.pitch;
	m_CompressedPedIK.m_lowerArmOrient.yaw = other.m_CompressedPedIK.m_lowerArmOrient.yaw;
	m_CompressedPedIK.m_lowerArmOrient.pitch = other.m_CompressedPedIK.m_lowerArmOrient.pitch;
	m_CompressedPedIK.m_nUaRoll = other.m_CompressedPedIK.m_nUaRoll;
	m_CompressedPedIK.m_nHandRoll = other.m_CompressedPedIK.m_nHandRoll;

	m_nAnimCount = other.m_nAnimCount;
	for (int32 i = 0; i < NUM_MULTIPLAYER_SYNC_ANIMS; i++)
	{
		m_aPedAnims[i].m_fBlendAmount = other.m_aPedAnims[i].m_fBlendAmount;
		m_aPedAnims[i].m_fBlendDelta = other.m_aPedAnims[i].m_fBlendDelta;
		m_aPedAnims[i].m_fCurrentTime = other.m_aPedAnims[i].m_fCurrentTime;
		m_aPedAnims[i].m_fSpeed = other.m_aPedAnims[i].m_fSpeed;
		m_aPedAnims[i].m_nGroupId = other.m_aPedAnims[i].m_nGroupId;
		m_aPedAnims[i].m_nAnimId = other.m_aPedAnims[i].m_nAnimId;
		m_aPedAnims[i].m_nFlags = other.m_aPedAnims[i].m_nFlags;
	}

	m_bBodyPartJustCameOff = other.m_bBodyPartJustCameOff;
	m_bIsPedDieAnimPlaying = other.m_bIsPedDieAnimPlaying;
	m_nPlayerTeam = other.m_nPlayerTeam;
	m_bPhys_lvcs_unk_1 = other.m_bPhys_lvcs_unk_1;
	m_nPeerLockOnMG = other.m_nPeerLockOnMG;
	m_nSurfaceTouched = other.m_nSurfaceTouched;
	m_nMoveState = other.m_nMoveState;
	m_eCurWeaponState = other.m_eCurWeaponState;
	m_nPedPhys_lvcs_unk_flagsB = other.m_nPedPhys_lvcs_unk_flagsB;
}

sPedSync::~sPedSync() {
	DECLARE_SYNC_DESTRUCT(this);
}

// not checks: m_CompressedPedIK, m_bBodyPartJustCameOff, m_bIsPedDieAnimPlaying, m_nPlayerTeam, m_nPeerLockOnMG
bool sPedSync::Compare(const sPedSync& other)
{
	if (!sElementPhysicalSync::Compare(other))
		return false;

	if (m_nVehicleID != other.m_nVehicleID)
		return false;
	if (m_bPhys_lvcs_unk_1 != other.m_bPhys_lvcs_unk_1)
		return false;
	if (m_nHealth != other.m_nHealth)
		return false;
	if (m_nPedState != other.m_nPedState)
		return false;
	if (m_nObjective != other.m_nObjective)
		return false;
	if (m_nPedType != other.m_nPedType)
		return false;
	if (m_eCurWeaponType != other.m_eCurWeaponType)
		return false;
	if (m_eCurWeaponState != other.m_eCurWeaponState)
		return false;
	if (m_fRotationCur != other.m_fRotationCur)
		return false;
	if (m_fRotationDest != other.m_fRotationDest)
		return false;

	if (m_nAnimCount != other.m_nAnimCount)
		return false;

	if (memcmp(m_aPedAnims, other.m_aPedAnims, sizeof(sPedAnimSync) * m_nAnimCount) != 0)
		return false;

	if (m_nSurfaceTouched != other.m_nSurfaceTouched)
		return false;
	if (m_nMoveState != other.m_nMoveState)
		return false;
	if (m_nPedPhys_lvcs_unk_flagsB != other.m_nPedPhys_lvcs_unk_flagsB)
		return false;

	return true;
}

#ifdef MG_PED_ORIGINAL_SEQUENCE
// custom
void sPedSync::CopyAnimations(RpClump* clump)
{
	assert(clump);
	CAnimBlendClumpData* clumpData = *RPANIMBLENDCLUMPDATA(clump);
	CAnimBlendAssociation* assocs[NUM_MULTIPLAYER_SYNC_ANIMS];
	int32 nAssocCount = 0;
	for (CAnimBlendLink* link = clumpData->link.next; link; link = link->next)
	{
		CAnimBlendAssociation* assoc = CAnimBlendAssociation::FromLink(link);
		if (assoc->isDeletableMG) {
			assoc->isDeletableMG = false;
		}
		else {
			if (nAssocCount < NUM_MULTIPLAYER_SYNC_ANIMS) {
				assocs[nAssocCount++] = assoc;
			}
		}
	}
	m_nAnimCount = nAssocCount;
	for (int32 i = 0; i < nAssocCount; ++i) {
		CAnimBlendAssociation* assoc = assocs[nAssocCount - 1 - i];
		m_aPedAnims[i].m_fBlendAmount = assoc->blendAmount;
		m_aPedAnims[i].m_fBlendDelta = assoc->blendDelta;
		m_aPedAnims[i].m_fCurrentTime = assoc->currentTime;
		m_aPedAnims[i].m_fSpeed = assoc->speed;
		m_aPedAnims[i].m_nGroupId = assoc->groupId;
		m_aPedAnims[i].m_nAnimId = assoc->animId;
		m_aPedAnims[i].m_nFlags = assoc->flags;
	}
	for (CAnimBlendLink* link = clumpData->link.next; link;)
	{
		CAnimBlendAssociation* assoc = CAnimBlendAssociation::FromLink(link);
		if (assoc->RemoveRequested())
			link = clumpData->link.next; // reset
		else
			link = link->next;
	}
}
#else
// original
void sPedSync::CopyAnimations(RpClump* clump)
{
	assert(clump);

	int32 nAnimCount = 0;
	CAnimBlendClumpData* clumpData = *RPANIMBLENDCLUMPDATA(clump);
	for (CAnimBlendLink* link = clumpData->link.next; link && nAnimCount < NUM_MULTIPLAYER_SYNC_ANIMS; link = link->next)
	{
		CAnimBlendAssociation* assoc = CAnimBlendAssociation::FromLink(link);
		if (assoc->isDeletableMG) {
			assoc->isDeletableMG = false;
		}
		else {
			m_aPedAnims[nAnimCount].m_fBlendAmount = assoc->blendAmount;
			m_aPedAnims[nAnimCount].m_fBlendDelta = assoc->blendDelta;
			m_aPedAnims[nAnimCount].m_fCurrentTime = assoc->currentTime;
#if !defined(FINAL) && !defined(MASTER)
			//if (TheMPGame.FindPlayerPedMG()->GetEntity()->GetClump() == clump)
			//debug("------------- GOT TIME anim CPedClump->SYNC CopyAnimations %f %f %f  [%d]\n", m_aPedAnims[nAnimCount].m_fCurrentTime, m_aPedAnims[nAnimCount].m_fBlendAmount, m_aPedAnims[nAnimCount].m_fBlendDelta, assoc->animId);
#endif
			m_aPedAnims[nAnimCount].m_fSpeed = assoc->speed;
			m_aPedAnims[nAnimCount].m_nGroupId = assoc->groupId;
			m_aPedAnims[nAnimCount].m_nAnimId = assoc->animId;
			m_aPedAnims[nAnimCount].m_nFlags = assoc->flags;
			if (m_aPedAnims[nAnimCount].m_fCurrentTime < -100.0f || m_aPedAnims[nAnimCount].m_fCurrentTime > 100.0f)
			{
				int16 nGroupID = m_aPedAnims[nAnimCount].m_nGroupId;
				int16 nAnimID = m_aPedAnims[nAnimCount].m_nAnimId;
				float fTime = m_aPedAnims[nAnimCount].m_fCurrentTime;
				debug("%d) Group %d Id %d Time %f\n", nAnimCount, nGroupID, nAnimID, fTime);
			}
			++nAnimCount;
		}
	}

	m_nAnimCount = nAnimCount;

	for (CAnimBlendLink* link = clumpData->link.next; link;)
	{
		CAnimBlendAssociation* assoc = CAnimBlendAssociation::FromLink(link);
		if (assoc->RemoveRequested())
			link = clumpData->link.next; // reset
		else
			link = link->next;
	}
}
#endif

#if !defined(FINAL) && !defined(MASTER)
void sPedSync::Dump()
{
	sElementPhysicalSync::Dump();

	printf("=== sPedSync Dump ===\n");
	printf("\nPed Information:\n");
	printf("  Vehicle ID:        %d %s\n", m_nVehicleID, InVehicle() ? "[IN VEHICLE]" : "[ON FOOT]");
	printf("  Health:            %d\n", m_nHealth);
	printf("  Ped State:         %d (", m_nPedState);
	switch (m_nPedState) {
		case PedState::PED_DEAD: printf("DEAD"); break;
		case PedState::PED_DIE: printf("DYING"); break;
		default: printf("ALIVE"); break;
	}
	printf(") %s\n", DyingOrDead() ? "[DEAD/ DYING]" : "");

	printf("  Objective:         %d\n", m_nObjective);
	printf("  Ped Type:          %d\n", m_nPedType);
	printf("  Current Weapon:    %d\n", m_eCurWeaponType);
	printf("  Current Rotation:  %.3f\n", m_fRotationCur);
	printf("  Dest Rotation:     %.3f\n", m_fRotationDest);

	printf("\nFlags and States:\n");
	printf("  BodyPartCameOff:   %s\n", m_bBodyPartJustCameOff ? "YES" : "NO");
	printf("  DieAnimPlaying:    %s\n", m_bIsPedDieAnimPlaying ? "YES" : "NO");
	printf("  Player Team:       %d\n", m_nPlayerTeam);
	printf("  Phys_lvcs_unk_1:   0x%02X\n", m_bPhys_lvcs_unk_1);
	printf("  Peer Lock On MG:   %d\n", m_nPeerLockOnMG);
	printf("  Surface Touched:   %d\n", m_nSurfaceTouched);
	printf("  Move State:        %d\n", m_nMoveState);
	printf("  Weapon State:      %d\n", m_eCurWeaponState);

	printf("  Ped LVCS Flags B:  0x%02X\n", m_nPedPhys_lvcs_unk_flagsB);
	printf("    b154_1:          %d\n", b154_1);
	printf("    bHasBlipPhys:        %d\n", bHasBlipPhys);
	printf("    b154_4:          %d\n", b154_4);
	printf("    bNoRadarForEnemy:%d\n", bNoRadarForEnemy);
	printf("    b154_10:         %d\n", b154_10);
	printf("    b154_20:         %d\n", b154_20);
	printf("    b154_40:         %d\n", b154_40);
	printf("    b154_80:         %d\n", b154_80);

	m_CompressedPedIK.Dump();
	//printf("\nIK Sync:\n");
	//printf("  IK Flags:          0x%02X\n", m_CompressedPedIK.m_flags);
	//printf("  Head:              Yaw=%d, Pitch=%d\n", m_CompressedPedIK.m_headOrient.yaw, m_CompressedPedIK.m_headOrient.pitch);
	//printf("  Torso:             Yaw=%d, Pitch=%d\n", m_CompressedPedIK.m_torsoOrient.yaw, m_CompressedPedIK.m_torsoOrient.pitch);
	//printf("  Upper Arm:         Yaw=%d, Pitch=%d\n", m_CompressedPedIK.m_upperArmOrient.yaw, m_CompressedPedIK.m_upperArmOrient.pitch);
	//printf("  Lower Arm:         Yaw=%d, Pitch=%d\n", m_CompressedPedIK.m_lowerArmOrient.yaw, m_CompressedPedIK.m_lowerArmOrient.pitch);
	//printf("  UA Roll:           %d\n", m_CompressedPedIK.m_nUaRoll);
	//printf("  Hand Roll:         %d\n", m_CompressedPedIK.m_nHandRoll);

	printf("\nAnimations (Count: %d):\n", m_nAnimCount);
	for (int32 i = 0; i < m_nAnimCount && i < NUM_MULTIPLAYER_SYNC_ANIMS; i++) {
		const sPedAnimSync& anim = m_aPedAnims[i];
		printf("  Anim[%d]:\n", i);
		printf("    Group:        %d\n", anim.m_nGroupId);
		printf("    ID:           %d\n", anim.m_nAnimId);
		printf("    BlendAmount:  %.3f\n", anim.m_fBlendAmount);
		printf("    BlendDelta:   %.3f\n", anim.m_fBlendDelta);
		printf("    CurrentTime:  %.3f\n", anim.m_fCurrentTime);
		printf("    Speed:        %.3f\n", anim.m_fSpeed);
		printf("    Flags:        0x%08X\n", anim.m_nFlags);
	}
	printf("================================\n");
}
#endif


cPedMG::cPedMG(sElement* elem) : cPhysicalMG(elem)
{
	m_storedWeapon = eWeaponType::WEAPONTYPE_UNARMED;
	m_pWeaponModel = nil;
	m_pFire = nil;
	field_17C = 0.0f;
	//
	m_nTime = 0; // kek
	bIsPed = true;
	SetStatus(eEntityStatus::STATUS_SIMPLE);
	m_audioEntityId = DMAudio.CreateEntity(eAudioType::AUDIOTYPE_PHYSICAL, this);
	field_1A4 = 0.0f;
	m_nTime = cMultiGame::Instance().m_pNetSession->m_nCurTime;
#ifdef DEBUG_MULTIGAME
	for (uint32 i = 0; i < ARRAY_SIZE(szDebugMessages); i++)
		szDebugMessages[i] = nil;
#endif
}

cPedMG::~cPedMG()
{
	DMAudio.DestroyEntity(m_audioEntityId);
}

void cPedMG::SetModelIndex(uint32 id)
{
	if (GetModelIndex() == id)
		return;

	if (!CStreaming::HasModelLoaded(id))
	{
		CStreaming::RequestModel(id, STREAMFLAGS_DEPENDENCY);
		CStreaming::LoadAllRequestedModels(false);
	}
	CEntity::SetModelIndex(id);
	RpAnimBlendClumpInit(GetClump());
	RpAnimBlendClumpFillFrameArray(GetClump(), GetElement().ped->m_pFrames);
	//CPedModelInfo* modelInfo = (CPedModelInfo*)CModelInfo::GetModelInfo(GetModelIndex()); // orig
	CPedModelInfo* modelInfo = (CPedModelInfo*)CModelInfo::GetModelInfo(id);
	GetElement().ped->m_pedStats = CPedStats::ms_apPedStats[modelInfo->m_pedStatType];
	GetElement().ped->m_headingRate = GetElement().ped->m_pedStats->m_headingChangeRate;
	GetElement().ped->m_animGroup = (AssocGroupId)modelInfo->m_animGroup;
	TheAnimManager->AddAnimation(GetClump(), GetElement().ped->m_animGroup, ANIM_STD_IDLE); // Warn! vcs use for CPed custom CPed::AddAnimation()
	//if (!CanUseTorsoWhenLooking()) // CPed::SetModelIndex but not in cPedMG::SetModelIndex
	//	m_pedIK.m_flags |= CPedIK::LOOKAROUND_HEAD_ONLY;
	(*RPANIMBLENDCLUMPDATA(m_rwObject))->velocity2d = &GetElement().ped->m_vecAnimMoveDelta;
	//if (modelInfo->GetHitColModel() == nil) // CPed::SetModelIndex but not in cPedMG::SetModelIndex
	//	modelInfo->CreateHitColModelSkinned(GetClump());
#ifndef GTA_LIBERTY
	modelInfo->ChoosePedColour(m_aColors); // not store to m_nActiveColorVariation
#endif
	//UpdateRpHAnim(); // in CPed::SetModelIndex but not in cPedMG::SetModelIndex
}

// TODO: merge CPedIK::PointGunInDirection PointGunInDirectionUsingArm, m_pedIK.PointGunInDirection(m_fLookDirection, ((CPlayerPed*)this)->m_fFPSMoveHeading);
// its rotate RtQuat directly for native hidden ped
void cPedMG::PreRender(void)
{
	uint16 syncTime = GetElement().ped->m_nTime - static_cast<uint16>(TheMPGame.m_nLagValue);
	GetElement().ped->OnPreRender(syncTime);
	UpdateRpHAnim();
	sPedSync* sync = GetElement().ped->FindSync(syncTime, nil).ped;

#if !defined(FINAL) && !defined(MASTER)
	//if (TheMPGame.FindPlayerPedMG()->GetPhysical() == this)
	//debug("cPedMG::PreRender sync 0x%p hash: 0x%X\n", sync, fast_hash32(sync, sizeof(sPedSync)));
#endif

	if (sync->m_bBodyPartJustCameOff)
	{
		RpHAnimHierarchy* hier = GetAnimHierarchyFromSkinClump(GetClump());
		int32 idx = RpHAnimIDGetIndex(hier, GetElement().ped->m_pFrames[PED_HEAD]->nodeID);
		RwMatrix* matArray = RpHAnimHierarchyGetMatrixArray(hier);
		RwV3d scale = { 0.0f, 0.0f, 0.0f };
		RwMatrixScale(&matArray[idx], &scale, rwCOMBINEPRECONCAT);
	}

	if (sync->m_bBodyPartJustCameOff && sync->m_bIsPedDieAnimPlaying && ((CTimer::GetFrameCounter() & 7) >= 4))
	{
		RpHAnimHierarchy* hier = GetAnimHierarchyFromSkinClump(GetClump());
		int32 idx = RpHAnimIDGetIndex(hier, GetElement().ped->m_pFrames[PED_HEAD]->nodeID);
		RwV3d localPos = { 0.0f, 0.0f, 0.0f };
		RwV3d dir = { 0.0f, 0.0f, 0.0f };
		RwMatrix* matArray = RpHAnimHierarchyGetMatrixArray(hier);
		RwV3dTransformPoints(&localPos, &localPos, 1, &matArray[idx]);
		CVector up = GetMatrix().GetUp();
		RwV3dScale(&up, &up, 0.1f);
		RwV3dAdd(&localPos, &localPos, &up);
		for (int32 i = 0; i < 4; ++i)
			CParticle::AddParticle(PARTICLE_BLOOD_SPURT, localPos, dir, nil, 0.0f, 0, 0, 0, 0);
	}

#ifdef MULTIGAME_ELEMENTS_IMPROVEMENTS
	if (bIsVisible && !(sync->InVehicle() && sync->m_nPedState != PED_DRAG_FROM_CAR && sync->m_nPedState != PED_EXIT_CAR))
	{
		if (CTimeCycle::GetShadowStrength() != 0)
		{
#ifdef USE_CUTSCENE_SHADOW_FOR_PED
			CCutsceneShadow* pShadow = m_pRTShadow;
			if (pShadow)
			{
				if (pShadow->IsInitialized())
					pShadow->UpdateForCutscene();
				CShadows::StoreShadowForCutscenePedObject(this,
					CTimeCycle::m_fShadowDisplacementX[CTimeCycle::m_CurrentStoredValue], CTimeCycle::m_fShadowDisplacementY[CTimeCycle::m_CurrentStoredValue],
					CTimeCycle::m_fShadowFrontX[CTimeCycle::m_CurrentStoredValue], CTimeCycle::m_fShadowFrontY[CTimeCycle::m_CurrentStoredValue],
					CTimeCycle::m_fShadowSideX[CTimeCycle::m_CurrentStoredValue], CTimeCycle::m_fShadowSideY[CTimeCycle::m_CurrentStoredValue]);
			}
			return;
#endif

			CShadows::StoreShadowForPedObject(this,
				CTimeCycle::m_fShadowDisplacementX[CTimeCycle::m_CurrentStoredValue], CTimeCycle::m_fShadowDisplacementY[CTimeCycle::m_CurrentStoredValue],
				CTimeCycle::m_fShadowFrontX[CTimeCycle::m_CurrentStoredValue], CTimeCycle::m_fShadowFrontY[CTimeCycle::m_CurrentStoredValue],
				CTimeCycle::m_fShadowSideX[CTimeCycle::m_CurrentStoredValue], CTimeCycle::m_fShadowSideY[CTimeCycle::m_CurrentStoredValue]);
		}
	}
#endif
}

void cPedMG::Render(void)
{
	cMultiGame& Game = cMultiGame::Instance();
	if (GetElement().element->GetID() != eElementID::MG_ELEMENT_PLAYER_PED_ID || !FindPlayerPed() || !FindPlayerPed()->m_pMyVehicle ||
		FindPlayerPed()->m_pMyVehicle->GetVehicleAppearance() != VEHICLE_APPEARANCE_HELI ||
		TheCamera.Cams[TheCamera.ActiveCam].Mode != CCam::MODE_1STPERSON || TheCamera.Cams[TheCamera.ActiveCam].DirectionWasLooking)
	{
		if (GetElement().ped->GetID() == eElementID::MG_ELEMENT_PLAYER_PED_ID)
		{
			sPlayer* pPlayer = Game.GetPlayer(GetElement().ped->GetOwner());
			if (pPlayer)
				pPlayer->OnPlayerRender();
		}

		uint16 nTime = GetElement().ped->m_nTime - static_cast<uint16>(Game.m_nLagValue);

		CPed* pPed = (CPed*)GetElement().ped->GetEntity();

		int32 Mode = TheCamera.Cams[TheCamera.ActiveCam].Mode;
		if (Mode == CCam::MODE_1STPERSON && TheCamera.m_fAvoidTheGeometryProbsTimer > 0.0f)
			Mode = CCam::MODE_WHEELCAM;

		if ((!IsLocalPlayerPed() || !pPed || pPed->m_nPedState != PedState::PED_DRIVING ||
			TheCamera.GetLookDirection() != LOOKING_FORWARD || Mode != CCam::MODE_1STPERSON) &&
			(!pPed || !pPed->b4E_8))
		{
#ifdef DEBUG_MULTIGAME
			eElementSyncType foundType = eElementSyncType::SYNC_TYPE_NONE;
			sPedSync* sync = GetElement().ped->FindSync(nTime, nil, &foundType).ped;
#else
			sPedSync* sync = GetElement().ped->FindSync(nTime, nil).ped;
#endif

#ifdef DEBUG_MULTIGAME
			char dbuff[80];
			char dbuff2[80];
			cInterestZone* zone = TheMPGame.GetElementOwnerZone(GetElement().element);
			sprintf(dbuff, "time %d zoneId %d m_nBasis %d m_nCurTime %d S: %s",
				nTime, zone ? zone->m_nID : 0, zone ? zone->m_nBasis : 0, zone ? zone->m_nCurTime : 0, GetSyncTypeName(foundType));
			GetDebugMessages()[0+1] = dbuff;
			sprintf(dbuff2, "%f %f %f ik 0x%X", GetPosition().x, GetPosition().y, GetPosition().z, sync->m_CompressedPedIK.m_flags);
			GetDebugMessages()[1+1] = dbuff2;
#endif

			if (!sync->b154_20 && (!IsLocalPlayerPed() || !TheCamera.Using1stPersonWeaponMode()))
			{
#ifndef GTA_LIBERTY
				CPedModelInfo* mi = (CPedModelInfo*)CModelInfo::GetModelInfo(GetModelIndex());
				mi->SetPedColour(m_aColors);
#endif

				cPhysicalMG::Render();

				eWeaponType eCurrent = (eWeaponType)sync->m_eCurWeaponType;
				eWeaponType eStored = m_storedWeapon;
				if (eStored != eCurrent)
				{
					if (eStored)
						RemoveWeaponModel(CWeaponInfo::GetWeaponInfo(eStored)->m_nModelId);
					if (eCurrent)
						AddWeaponModel(CWeaponInfo::GetWeaponInfo(eCurrent)->m_nModelId);
					m_storedWeapon = eCurrent;
				}

				if (m_pWeaponModel)
				{
					RpClump* clump = GetClump();
					RpHAnimHierarchy* hier = GetAnimHierarchyFromSkinClump(clump);
					int32 idx = RpHAnimIDGetIndex(hier, GetElement().ped->m_pFrames[PED_HANDR]->nodeID);
					RwMatrix* mat = RpHAnimHierarchyGetMatrixArray(hier) + idx;
					RwFrame* frame = RpAtomicGetFrame(m_pWeaponModel);
					RwMatrix* frameMat = RwFrameGetMatrix(frame);
					*frameMat = *mat;
					RwFrameUpdateObjects(frame);
					RpAtomicRender(m_pWeaponModel);
				}
			}
		}
	}
}

void cPedMG::UpdateAnim(void)
{
	if (!bOffscreen) bOffscreen = !GetIsOnScreen();

	uint16 nPedTime = GetElement().ped->m_nTime;
	if (nPedTime == m_nTime) {
#if !defined(FINAL) && !defined(MASTER)
		SetConsoleColor(0);
		PED_LOG(1, "cPedMG::UpdateAnim SKIP UPDATE ANIM COND: sPed nPedTime %d == cPedMG m_nTime %d\n", nPedTime, m_nTime);
		SetConsoleColor(6);
#endif
		return;
	}

	if (GetElement().ped->FindSync(nPedTime, nil).ped->m_nAnimCount != 0) {
		GetElement().ped->UpdateAnim(nPedTime, 1.0f); // Set anim data from sync to physical clump cPedMG
		RpAnimBlendClumpUpdateAnimations(GetClump(), 0.0f, !bOffscreen); // 0.0f do not recalc new D/A, apply only to skelet
	}
	else {
		// CEntity delta CTimer::ms_fTimeStep / 50.0f GetTimeStepInSeconds nPedTime - m_nTime timestep
		RpAnimBlendClumpUpdateAnimations(GetClump(), static_cast<int16>(nPedTime - m_nTime) / NET_SESSION_60, !bOffscreen); // to sec
	}

	m_nTime = GetElement().ped->m_nTime;
}

// probably in CRenderer cpp file
bool cPedMG::SetupLighting(void)
{
	ActivateDirectional();
	SetAmbientColoursForPedsCarsAndObjects();
	//gbForceEnvOff = false;
	// PSP gif *(g_GifCMDBufferPtr++) = 0xC9010100;
	if (bRenderScorched)
	{
		WorldReplaceNormalLightsWithScorched(Scene.world, 0.1f);
		//gbForceEnvOff = true;
	}
	else
	{
		// Note that this lightMult is only affected by LIGHT_DARKEN. If there's no LIGHT_DARKEN, it will be 1.0.
		float lightMult = CPointLights::GenerateLightsAffectingObject(&GetPosition());
		if (lightMult != 1.0f)
		{
			SetAmbientAndDirectionalColours(lightMult);
			return true;
		}
	}
	return false;
}

void cPedMG::RemoveLighting(bool reset)
{
#if 1 // VC code
	if (!bRenderScorched)
	{
		CRenderer::RemoveVehiclePedLights(this, reset);
		if (reset)
			ReSetAmbientAndDirectionalColours();
	}
#else // LVCS code
	CRenderer::RemoveVehiclePedLights(this, reset);
#endif
	SetAmbientColours();
	DeActivateDirectional();
}


void cPedMG::AddWeaponModel(int32 model) {
	if (model == -1)
		return;

	CSimpleModelInfo* mi = (CSimpleModelInfo*)CModelInfo::GetModelInfo(model);
	if (m_pWeaponModel)
		RemoveWeaponModel(-1);
	m_pWeaponModel = (RpAtomic*)mi->CreateInstance();
	mi->AddRef();
}

void cPedMG::RemoveWeaponModel(int32 model) {
	if (!m_pWeaponModel)
		return;

	assert((RwObjectGetType((RwObject*)m_pWeaponModel) == rpATOMIC));

	CSimpleModelInfo* mi = CVisibilityPlugins::GetAtomicModelInfo(m_pWeaponModel);
	CSimpleModelInfo* miReq = (CSimpleModelInfo*)CModelInfo::GetModelInfo(model);
	if (model == -1 || mi == miReq)
	{
		mi->RemoveRef();
		RwFrame* f = RpAtomicGetFrame(m_pWeaponModel);
		RpAtomicDestroy(m_pWeaponModel);
		RwFrameDestroy(f);
		m_pWeaponModel = nil;
	}
}

bool cPedMG::IsLocalPlayerPed() {
	cMultiGame& pGame = cMultiGame::Instance();
	return GetElement().element->GetOwner() == pGame.LocalPlayerID() && GetElement().ped->GetID() == eElementID::MG_ELEMENT_PLAYER_PED_ID;
}

PedState GetPedState(CPhysical* pPed) {
	if (pPed->IsPed())
		return ((CPed*)pPed)->GetPedState();
	assert(pPed->IsMultiplayer() && pPed->bIsPed && ((cPedMG*)pPed)->GetElement().ped != nil);
	return (PedState)((cPedMG*)pPed)->GetElement().ped->GetSync().ped->m_nPedState;
}



sPed::sPed() : sElementPhysical() {
	DECLARE_ELEMENT_CONSTRUCT(this, true, false);
	m_nPedTime = 0;
	m_bHasReload = false;
	m_bHasShot = false;
//#ifdef FIX_BUGS
//	m_nTeamID = -1; // where is it
//#endif
	m_vecAnimMoveDelta = CVector2D(0.0f, 0.0f);
	++ms_nNumberOfSyncedPeds;
	SetPhysical(new cPedMG(this));
	GetPhysical()->m_fMass = 70.0f;
	GetPhysical()->m_fTurnMass = 100.0f;
	GetPhysical()->m_fAirResistance = 0.4 / GetPhysical()->m_fMass;
	ms_peds.push_back(this);
	DECLARE_ELEMENT_CONSTRUCT(this, false, false);
}

sPed::sPed(CPed* ped) : sElementPhysical() {
	DECLARE_ELEMENT_CONSTRUCT(this, true, true);
	m_nPedTime = 0;
	m_bHasReload = false;
	m_bHasShot = false;
	m_vecAnimMoveDelta = CVector2D(0.0f, 0.0f);
	++ms_nNumberOfSyncedPeds;
	cMultiGame::Instance().AttachEntity(this, ped);
	SetEntity(ped);
	SetPhysical(new cPedMG(this));
	GetPhysical()->m_fMass = 70.0f;
	GetPhysical()->m_fTurnMass = 100.0f;
	GetPhysical()->m_fAirResistance = 0.4 / GetPhysical()->m_fMass;
	GetPhysical()->SetModelIndex(ped->GetModelIndex());
	RegisterSelf();
	SetTeamID(ped->m_nTeamIdMG);
	AttachSync(m_nTime, new sPedSync(ped));
	DECLARE_ELEMENT_CONSTRUCT(this, false, true);
}


ElementCapability sPed::GetCapability()
{
	return sPed::Capability();
}

bool sPed::HasCapability(ElementCapability capability)
{
	if (sPed::Capability() == capability)
		return true;
	if (sElementPhysical::Capability() == capability)
		return true;
	if (sElement::Capability() == capability)
		return true;
	return false;
}

sPed::~sPed() {
	DECLARE_ELEMENT_DESTRUCT(this);
	--ms_nNumberOfSyncedPeds;
	if (GetID() == eElementID::MG_ELEMENT_PLAYER_PED_ID) {
#ifdef FIX_BUGS
		debug("trying to delete player ped!!!!!\n");
#else
		debug("trying to delete player ped!!!!!\n");
		debug("trying to delete player ped!!!!!\n");
		debug("trying to delete player ped!!!!!\n");
		debug("trying to delete player ped!!!!!\n");
		debug("trying to delete player ped!!!!!\n");
		debug("trying to delete player ped!!!!!\n");
		debug("trying to delete player ped!!!!!\n");
		debug("trying to delete player ped!!!!!\n");
		debug("trying to delete player ped!!!!!\n");
		debug("Trying to delete player ped");
#endif
	}

	if (GetOwner() != cMultiGame::Instance().LocalPlayerID()) {
		std::vector<sPed*>::iterator loc = std::find(ms_peds.begin(), ms_peds.end(), this);
		if (loc != ms_peds.end()) ms_peds.erase(loc);
	}

	if (GetPhysical() == CWeaponEffects::GetTargetEntity())
		CWeaponEffects::ClearCrossHair();
	CProjectileInfo::RemoveAllProjectilesOwnedBy(GetPhysical());

	int32 i = CPools::GetPedPool()->GetSize();
	while (i-- > 0)
	{
		CPed* pPed = CPools::GetPedPool()->GetSlot(i);
		if (!pPed) continue;
		for (int32 j = 0; j < pPed->m_numNearPedsMG; j++)
		{
			if (pPed->m_nearPedsMG[j] == this)
			{
				for (int32 k = j; k < pPed->m_numNearPedsMG - 1; ++k)
					pPed->m_nearPedsMG[k] = pPed->m_nearPedsMG[k + 1];
				--pPed->m_numNearPedsMG;
				pPed->m_nearPedsMG[pPed->m_numNearPedsMG] = nil;
				--j;
			}
		}
	}

	sElement::PurgeAttached();
	// ~sElement
}

sElementSync* sPed::CreateSync() {
	return new sPedSync();
}

void sPed::DisposeSync(sElementSync* pSync) {
	if(pSync)
		delete ((sPedSync*)pSync);
}

sElementSync* sPed::CreateSyncFromOther(sElementSync* pSync)
{
	sPedSync& sync = *(sPedSync*)pSync;
	return new sPedSync(sync);
}

bool sPed::HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB)
{
	sPedSync& syncA = *(sPedSync*)pSyncA;
	sPedSync& syncB = *(sPedSync*)pSyncB;
	return syncA.Compare(syncB);
}

void sPed::ApplyClientSync(uint16 nTime) {
	sElementPhysical::ApplyClientSync(nTime);
	if (GetOwner() == TheMPGame.LocalPlayerID())
		return;

	int32 reloadAnimIndex = -1;
	int32 shootAnimIndex = -1;

	sPedSync* pSync = GetSync().ped;
	uint8 animCount = pSync->m_nAnimCount;

	for (uint32 i = 0; i < animCount; i++) {
		sPedAnimSync* animSync = &pSync->m_aPedAnims[i];
		if (animSync->m_nFlags & eAnimAssocFlags::ASSOC_RUNNING) {
			CWeaponInfo* weaponInfo = CWeaponInfo::GetWeaponInfo((eWeaponType)pSync->m_eCurWeaponType);
			if (weaponInfo && weaponInfo->IsFlagSet(WEAPONFLAG_RELOAD)) {
				if (animSync->m_nAnimId == AnimationId::ANIM_ATTACK_EXTRA1) {
					reloadAnimIndex = i;
					break;
				}
			}
		}
	}

	if (reloadAnimIndex == -1) {
		if (m_bHasReload)
			m_bHasReload = false;
	}
	else
	{
		if (!m_bHasReload && GetPhysical())
		{
			DMAudio.PlayOneShot(GetPhysical()->m_audioEntityId, SOUND_WEAPON_RELOAD, /*pSync->m_eCurWeaponType,*/ 1.0f); // 1.0f vol
			m_bHasReload = true;
		}
	}

	for (uint32 i = 0; i < animCount; i++) {
		sPedAnimSync* animSync = &pSync->m_aPedAnims[i];
		if (animSync->m_nFlags & eAnimAssocFlags::ASSOC_RUNNING) {
#ifdef FIX_BUGS
			if (animSync->m_nGroupId > AssocGroupId::ASSOCGRP_BIKE_DIRT && animSync->m_nGroupId < AssocGroupId::ASSOCGRP_SUNBATHE) // recheck
#else
			if (animSync->m_nGroupId > AssocGroupId::ASSOCGRP_BIKE_DIRT && animSync->m_nGroupId < AssocGroupId::ASSOCGRP_ROCKETLAUNCHER)
#endif
			{
				shootAnimIndex = i;
				break;
			}
		}
	}

	if (shootAnimIndex == -1) {
		if (m_bHasShot) {
			if (GetPhysical())
				DMAudio.PlayOneShot(GetPhysical()->m_audioEntityId, SOUND_WEAPON_AK47_BULLET_ECHO, /*pSync->m_eCurWeaponType,*/ 1.0f);
			m_bHasShot = false;
		}
	}
	else {
		if (!m_bHasShot)
			m_bHasShot = true;
	}

	if (pSync->m_eCurWeaponType == eWeaponType::WEAPONTYPE_CHAINSAW) {
		if (!pSync->InVehicle()) {
			uint32 chainsawAnimIndex = 0;
			bool foundChainsawAnim = false;

			for (uint32 i = 0; i < animCount; i++) {
				sPedAnimSync* animSync = &pSync->m_aPedAnims[i];
				if (animSync->m_nAnimId == AnimationId::ANIM_WEAPON_FIRE) {
					chainsawAnimIndex = i;
					foundChainsawAnim = true;
					break;
				}
			}

			if (foundChainsawAnim) {
				DMAudio.SetEntityStatus(GetPhysical()->m_audioEntityId, true);
				sPedAnimSync* chainsawAnim = &pSync->m_aPedAnims[chainsawAnimIndex];
				if (pSync->m_eCurWeaponType == eWeaponType::WEAPONTYPE_CHAINSAW) // why?
				{
					if (chainsawAnim->m_fCurrentTime > -0.3f && (chainsawAnim->m_fCurrentTime - (CTimer::GetTimeStep() / 50.0f)) < 0.6f)
					{
						if (pSync->m_eCurWeaponState == eWeaponState::WEAPONSTATE_MELEE_MADECONTACT)
							DMAudio.PlayOneShot(GetPhysical()->m_audioEntityId, SOUND_WEAPON_CHAINSAW_MADECONTACT, 1.0f);
						else
							DMAudio.PlayOneShot(GetPhysical()->m_audioEntityId, SOUND_WEAPON_CHAINSAW_ATTACK, 1.0f);
					}
					else
						DMAudio.PlayOneShot(GetPhysical()->m_audioEntityId, SOUND_WEAPON_CHAINSAW_IDLE, 1.0f);
				}
			}
			else {
				DMAudio.SetEntityStatus(GetPhysical()->m_audioEntityId, true);
				DMAudio.PlayOneShot(GetPhysical()->m_audioEntityId, SOUND_WEAPON_CHAINSAW_IDLE, 1.0f);
			}
		}
	}
}

void sPed::Update(uint16 nTime) {
#ifdef DEBUG_MULTIGAME
	//debug("elem 0x%p %d %d\n", this, GetOwner(), GetID());
	assert(GetEntity() && GetEntity()->m_rwObject);
#endif
	GetEntity()->UpdateRpHAnim();
	AttachSync(nTime, new sPedSync((CPed*)GetEntity()));
}

bool sPed::WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) {
	if (static_cast<int16>(nSyncLastTime - m_vSync.front().m_nTime) >= 0)
		return WriteSyncDelta(pSyncStream, GetSyncWithTime(nSyncWriteTime).ped, GetSyncWithTime(nSyncLastTime).ped, (nSyncWriteTime - nSyncLastTime));

	tPedSyncsDeltas pedDeltaManager{};
	pedDeltaManager.SetDifference();
	PerformWriteSync(pSyncStream, GetSyncWithTime(nSyncWriteTime).ped, &pedDeltaManager); // max diff
	return true;
}

void sPed::ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) {
	sPedSync& sync = *(sPedSync*)pOutSync;

	uint32 nDiffMask = pSyncStream->ReadU32();
	uint32 nOldAnimCount = sync.m_nAnimCount;

	if (nDiffMask & ePedSync::MP_PKTD_PED_MODEL_TYPE)
	{
		int16 modelIndex = pSyncStream->ReadI16();
		if (GetPhysical())
			GetPhysical()->SetModelIndex(modelIndex);
		sync.m_nPedType = static_cast<ePedType>(pSyncStream->ReadU8());
		if (sync.m_nPedType == ePedType::PEDTYPE_COP)
			m_nCopModelIndex = pSyncStream->ReadU8();
	}

	if (nDiffMask & ePedSync::MP_PKTD_PED_HEALTH)
		sync.m_nHealth = pSyncStream->ReadU16();

	if (nDiffMask & ePedSync::MP_PKTD_PED_STATE)
		sync.m_nPedState = static_cast<PedState>(pSyncStream->ReadU8());

	if (nDiffMask & ePedSync::MP_PKTD_PED_OBJECTIVE)
		sync.m_nObjective = static_cast<eObjective>(pSyncStream->ReadU8());

	if (nDiffMask & ePedSync::MP_PKTD_PED_VEHICLE_ID)
		sync.m_nVehicleID = pSyncStream->ReadI16();

	if (nDiffMask & ePedSync::MP_PKTD_PED_POSITION_XY) {
		sync.GetMatrix().GetPosition().x = pSyncStream->ReadFloat();
		sync.GetMatrix().GetPosition().y = pSyncStream->ReadFloat();
	}

	if (nDiffMask & ePedSync::MP_PKTD_PED_POSITION_Z)
		sync.GetMatrix().GetPosition().z = pSyncStream->ReadFloat();

	if (nDiffMask & ePedSync::MP_PKTD_PED_ROTATION_CUR)
	{
		sync.m_fRotationCur = pSyncStream->ReadFloat();
		CVector pos = sync.GetMatrix().GetPosition();
		sync.GetMatrix().SetRotate(0.0f, 0.0f, sync.m_fRotationCur); // SetRotate reset pos
		sync.GetMatrix().Translate(pos);
	}

	if (nDiffMask & ePedSync::MP_PKTD_PED_ROTATION_DEST)
		sync.m_fRotationDest = pSyncStream->ReadFloat();

	if (nDiffMask & ePedSync::MP_PKTD_PED_ANIM_COUNT)
		sync.m_nAnimCount = pSyncStream->ReadU8();

	for (uint32 i = 0; i < sync.m_nAnimCount; i++) {
		if (i >= nOldAnimCount) {
			bool missing = false;
			if (!(nDiffMask & MP_PKTD_PED_ANIM_MASK(i)))
				missing = true;
			else if (pSyncStream->PeekU8() != static_cast<uint8>(ePedAnimSync::MP_PKTD_PED_ANIM_FULL))
				missing = true;

			if (missing) {
#ifdef FIX_BUGS
				debug("missing animation info in network message! This could cause an anim glitch !!!!!\n");
#else
				debug("missing animation info in network message! This could cause an anim glitch !!!!!");
#endif
			}
		}

		if (nDiffMask & MP_PKTD_PED_ANIM_MASK(i))
			ReadAnimSyncFromStream(pSyncStream, &sync.m_aPedAnims[i]);
	}

	for (uint32 i = sync.m_nAnimCount; i < NUM_MULTIPLAYER_SYNC_ANIMS; i++) { // m_nAnimCount -> NUM_MULTIPLAYER_SYNC_ANIMS
		if (nDiffMask & MP_PKTD_PED_ANIM_MASK(i))
			ReadAnimSyncFromStream(pSyncStream, &sync.m_aPedAnims[i]);
	}

	if (nDiffMask & ePedSync::MP_PKTD_PED_IK_FLAGS)
		sync.m_CompressedPedIK.m_flags = pSyncStream->ReadU8();

	if (nDiffMask & ePedSync::MP_PKTD_PED_IK_DATA)
		sPedIKSync::ReadSyncFromStream(pSyncStream, &sync.m_CompressedPedIK);

	if (nDiffMask & ePedSync::MP_PKTD_PED_WEAPON_TYPE)
		sync.m_eCurWeaponType = static_cast<eWeaponType>(pSyncStream->ReadU8());

	if (nDiffMask & ePedSync::MP_PKTD_PED_WEAPON_STATE)
		sync.m_eCurWeaponState = static_cast<eWeaponState>(pSyncStream->ReadU8());

	if (nDiffMask & ePedSync::MP_PKTD_PED_BODY_PART_CAME_OFF)
		sync.m_bBodyPartJustCameOff = pSyncStream->ReadBool();

	if (nDiffMask & ePedSync::MP_PKTD_PED_DIE_ANIM_PLAYING)
		sync.m_bIsPedDieAnimPlaying = pSyncStream->ReadBool();

	if (nDiffMask & ePedSync::MP_PKTD_PED_PLAYER_TEAM) {
		sync.m_nPlayerTeam = pSyncStream->ReadI16();
		SetTeamID(sync.m_nPlayerTeam);
	}

	if (nDiffMask & ePedSync::MP_PKTD_PED_PHYS_LVCS_UNK1)
		sync.m_bPhys_lvcs_unk_1 = pSyncStream->ReadBool();

	if (nDiffMask & ePedSync::MP_PKTD_PED_MOVE_SPEED)
		sync.SetMoveSpeed(pSyncStream->ReadVector());

	if (nDiffMask & ePedSync::MP_PKTD_PED_PEER_LOCK_ON)
		sync.m_nPeerLockOnMG = pSyncStream->ReadI8();

	if (nDiffMask & ePedSync::MP_PKTD_PED_SURFACE_TOUCHED)
		sync.m_nSurfaceTouched = pSyncStream->ReadI16();

	if (nDiffMask & ePedSync::MP_PKTD_PED_MOVE_STATE)
		sync.m_nMoveState = static_cast<eMoveState>(pSyncStream->ReadU8());

	if (!sync.InVehicle()) // non vehicle and update pos
	{
		if (nDiffMask & (ePedSync::MP_PKTD_PED_POSITION_XY | ePedSync::MP_PKTD_PED_POSITION_Z | ePedSync::MP_PKTD_PED_ROTATION_CUR))
		{
			CVector pos = sync.GetMatrix().GetPosition();
			sync.GetMatrix().SetRotate(0.0f, 0.0f, sync.m_fRotationCur); // SetRotate reset pos
			sync.GetMatrix().Translate(pos);
		}
	}
	else
	{
		sVehicle* vehicleElem = (sVehicle*)cMultiGame::Instance().GetEntityForHandle(GetOwner(), sync.m_nVehicleID);
		if (vehicleElem && vehicleElem->GetType() == eElementType::ELEMENT_TYPE_BIKE)
		{
			sBikeSync* pBikeSync = vehicleElem->GetSync().bike;

			// pitch up (HALFPI - atan(cot), cot = up.z / |up_xy|)
			float pitch = 0.0f;
			if (pBikeSync->GetMatrix().GetUp().z < 1.0f)
			{
				float upXY2 = (SQR(pBikeSync->GetMatrix().GetUp().x) + SQR(pBikeSync->GetMatrix().GetUp().y));
				if (upXY2 > 0.0f)
				{
					float xy = sqrt(upXY2);
#ifdef GTA_PC
					pitch = atan2(xy, pBikeSync->GetMatrix().GetUp().z);
#else
					TODO(); // probably pspsdk shit
					const float cot = pBikeSync->GetMatrix().GetUp().z / sqrt(upXY2);
					float abscot = cot < 0.0f ? -cot : cot;

					float a; // atan(cot)
					if (abscot < 0.15f) {
						//const float x2 = SQR(cot); // atan(x) ~ x + x^3*(-1/3) + x^5*(1/5) + x^7*(-1/7)
						//a = cot * (1.0f + x2 * (-0.33333f + x2 * (0.2f + x2 * (-0.14f))));
						//pitch = HALFPI - acosf(Clamp(pBikeSync->GetMatrix().GetUp().z, -1.0f, 1.0f)); // cz
						//a = atan_vfpu(cot);  // Emulated VFPU (handles sign internally)
					}
					else {
						a = (float)atan(cot);
						//a = atan_poly(cot);  // Polynomial (handles sign internally)
					}
					pitch = (HALFPI - a);
#endif
				}
			}

			// wheelie
			CVector pos = sync.GetMatrix().GetPosition();
			sync.GetMatrix().SetRotate(pitch, pBikeSync->m_fLeanAngle, sync.m_fRotationCur);
			sync.GetMatrix().Translate(pos);
		}
#ifdef MULTIGAME_BMX_IMPROVEMENTS
		if (vehicleElem && vehicleElem->GetType() == eElementType::ELEMENT_TYPE_BMX)
		{
			sBmxSync* pBmxSync = vehicleElem->GetSync().bmx;

			// pitch up (HALFPI - atan(cot), cot = up.z / |up_xy|)
			float pitch = 0.0f;
			if (pBmxSync->GetMatrix().GetUp().z < 1.0f)
			{
				float upXY2 = (SQR(pBmxSync->GetMatrix().GetUp().x) + SQR(pBmxSync->GetMatrix().GetUp().y));
				if (upXY2 > 0.0f)
				{
					float xy = sqrt(upXY2);
					pitch = atan2(xy, pBmxSync->GetMatrix().GetUp().z);
				}
			}

			// wheelie
			CVector pos = sync.GetMatrix().GetPosition();
			sync.GetMatrix().SetRotate(pitch, pBmxSync->m_fLeanAngle, sync.m_fRotationCur);
			sync.GetMatrix().Translate(pos);
		}
#endif
		else if (vehicleElem && vehicleElem->GetType() == eElementType::ELEMENT_TYPE_AUTOMOBILE)
		{
			sAutomobileSync* pAutomobile = vehicleElem->GetSync().automobile;
			cPhysicalMG* phy = vehicleElem->GetPhysical();
			if (phy) {
				CVector pos = sync.GetMatrix().GetPosition();

				// PitchFromForward
				const float xy2 = (SQR(phy->GetMatrix().GetForward().x) + SQR(phy->GetMatrix().GetForward().y));
				float pitch = (float)atan2(phy->GetMatrix().GetForward().z, sqrt(xy2 > 0.0f ? xy2 : 0.0f)); // [-pi/2..pi/2]

				float roll = 0.0f; // atan2(-right.z, up.z)
				const float a = -phy->GetMatrix().GetRight().z;
				const float b = phy->GetMatrix().GetUp().z;
				if (a != 0.0f || b != 0.0f)
					roll = (float)atan2(a, b);

				// wheelie
				sync.GetMatrix().SetRotate(pitch, roll, sync.m_fRotationCur);
				sync.GetMatrix().Translate(pos);
			}
		}
#ifndef GTA_LIBERTY
		// TODO? other forgotten types? plane, heli, quad, bmx, boat
		//else {
		//	CVector pos = sync.GetMatrix().GetPosition();
		//	sync.GetMatrix().SetRotate(0.0f, 0.0f, sync.m_fRotationCur);
		//	sync.GetMatrix().Translate(pos);
		//}
#endif
	}

	CVector pos = sync.GetMatrix().GetPosition();
	CVector fwd = sync.GetMatrix().GetForward();
	PED_LOG(1, "PedState (%f %f %f) (%f %f %f) car: %i heading: %f %f anims: %i",
		pos.x, pos.y, pos.z, fwd.x, fwd.y, fwd.z, sync.m_nVehicleID,
		sync.m_fRotationCur, sync.m_fRotationDest, sync.m_nAnimCount);

	for (uint32 i = 0; i < sync.m_nAnimCount; i++) {
		sPedAnimSync& anim = sync.m_aPedAnims[i];
		PED_LOG(1, " anim %i: blend %f %f time %f spd %f anm %i, %i flags %i",
			i, anim.m_fBlendAmount, anim.m_fBlendDelta, anim.m_fCurrentTime, anim.m_fSpeed,
			anim.m_nAnimId, anim.m_nGroupId, anim.m_nFlags);
	}

#ifndef GTA_LIBERTY // sElementPhysical part sElementPhysical::ReadSyncFromStreamPhysical
	if (nDiffMask & ePedSync::MP_PKTD_PED_PHYS_LVCS_FLAGS_B) {
		uint8 flagsB = pSyncStream->ReadU8();
		assert(GetPhysical());
		GetPhysical()->m_nPhys_lvcs_unk_flagsB = flagsB;
		GetPhysical()->b4E_8 = false;

		if (GetOwner() == cMultiGame::Instance().LocalPlayerID()) {
			assert(GetEntity());
			((CPhysical*)GetEntity())->m_nPhys_lvcs_unk_flagsB = flagsB;
		}
		sync.m_nPedPhys_lvcs_unk_flagsB = flagsB;
	}
#endif
}

void sPed::ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime)
{
	cMultiGame& Game = cMultiGame::Instance();
	if (nOwner != Game.LocalPlayerID()) {
		sElementPhysical::ReceiveEntity(nOwner, nID, nTime);
		return;
	}

	sPedSync* pSync = GetSync().ped;
	uint32 modelIndex = pSync->m_nPedType == ePedType::PEDTYPE_COP ? m_nCopModelIndex : GetPhysical()->GetModelIndex();
	CPed* ped = CPopulation::AddPed((ePedType)pSync->m_nPedType, modelIndex, pSync->GetMatrix().GetPosition(), 0, false);
	Game.AttachEntity(this, ped);
	SetEntity(ped);
	ped->SetMatrix(pSync->GetMatrix());
	ped->SetMoveSpeed(pSync->GetMoveSpeed());
	ped->m_vecTurnSpeed = pSync->GetTurnSpeed();
	ped->m_vecMoveFriction = pSync->GetMoveFriction();
	ped->m_vecTurnFriction = pSync->GetTurnFriction();
	ped->m_fHealth = pSync->m_nHealth;
	ped->bPhys_lvcs_unk_1 = pSync->m_bPhys_lvcs_unk_1;
	ped->m_fRotationCur = pSync->m_fRotationCur;
	ped->m_fRotationDest = pSync->m_fRotationDest;
	ped->m_currentWeapon = pSync->m_eCurWeaponType;
	if (ped->m_nPedType != pSync->m_nPedType) {
		CPopulation::UpdatePedCount(ped->m_nPedType, true);
		ped->m_nPedType = (ePedType)pSync->m_nPedType;
		CPopulation::UpdatePedCount(ped->m_nPedType, false);
	}
	ped->m_objective = (eObjective)pSync->m_nObjective;
	ped->bBodyPartJustCameOff = pSync->m_bBodyPartJustCameOff;
	ped->bIsPedDieAnimPlaying = pSync->m_bIsPedDieAnimPlaying;
	ped->m_nTeamIdMG = pSync->m_nPlayerTeam;
	if (pSync->m_nPedState == PedState::PED_WANDER_PATH)
	{
		float rotation = -((pSync->m_fRotationCur + M_PI) * (4.0f / M_PI));
		int32 adjustedDir = 5 - (-floor(rotation));
		int32 offset = (adjustedDir < 1) ? 8 : 0;
		ped->SetWanderPath(adjustedDir + offset);
	}

	if (pSync->InVehicle())
	{
		debug("))))))))))))))) Transferring ped with old id %d with car %d %d %d\n", m_nPrevID, nOwner, GetOwner(), pSync->m_nVehicleID);
		sVehicle* veh = (sVehicle*)Game.FindElement(nOwner, GetOwner(), pSync->m_nVehicleID);
		if (veh)
		{
			debug("))))))))))))))) Found car for multiplayer ped %d %d %d\n", nOwner, GetOwner(), pSync->m_nVehicleID);
			ped->SetMyVehicle((CVehicle*)veh->GetEntity());
			ped->m_pCollidingEntity = (CEntity*)ped->m_pMyVehicle;
			ped->bUsesCollision = false;
			ped->SetPedState((PedState)pSync->m_nPedState);
			ped->AddInCarAnims(ped->m_pMyVehicle, true);
			ped->m_pMyVehicle->SetDriverPed(ped);
		}
		else
		{
			debug("UNABLE TO FIND CAR FOR MULTIPLAYER PED\n was Looking for car %d %d %d at (%f %f %f)\n",
				nOwner, GetOwner(), pSync->m_nVehicleID,
				ped->GetMatrix().GetPosition().x, ped->GetMatrix().GetPosition().y, ped->GetMatrix().GetPosition().z);
		}
	}
	ped->SetPedState((PedState)pSync->m_nPedState);
	std::vector<sPed*>::iterator loc = std::find(ms_peds.begin(), ms_peds.end(), this);
	if (loc != ms_peds.end()) ms_peds.erase(loc);
	sElementPhysical::ReceiveEntity(nOwner, nID, nTime);
	AttachSync(m_nTime, new sPedSync(ped));
	GetPhysical()->AddToMovingList();
}

// update non native net ped (non self witout CPed)
void sPed::UpdateDelta(sElementSync* pSync, uint16 nTimeDelta) {
	sElement::UpdateDelta(pSync, nTimeDelta);
	sPedSync& sync = *(sPedSync*)pSync;
	for (int32 i = 0; i < sync.m_nAnimCount; i++) {
		if(sync.m_aPedAnims[i].m_nFlags & eAnimAssocFlags::ASSOC_RUNNING)
			sync.m_aPedAnims[i].m_fCurrentTime += (sync.m_aPedAnims[i].m_fSpeed * (nTimeDelta * NET_SESSION_R_60));
	}
}

void sPed::ApplyDeltaState(sElementSync* pSync, uint16 nTimeDelta) {
	;
}

void sPed::InterpolateDeltaState(sElementSync* pSyncA, sElementSync* pSyncB, float fDelta) {
	sElementPhysical::InterpolateDeltaState(pSyncA, pSyncB, fDelta);
	sPedSync& syncA = *(sPedSync*)pSyncA;
	sPedSync& syncB = *(sPedSync*)pSyncB;
#ifdef GTA_LIBERTY
	if (syncB.m_nAnimCount == 0 || syncB.m_nAnimCount == 0)
		return;

	for (int32 a = 0, b = 0; a < syncB.m_nAnimCount; ++a) {
		if (b >= syncB.m_nAnimCount)
			return;
		sPedAnimSync& animA = syncB.m_aPedAnims[a];
		if (animA.m_nGroupId == syncB.m_aPedAnims[b].m_nGroupId &&
			animA.m_nAnimId == syncB.m_aPedAnims[b].m_nAnimId)
		{
			if (animA.m_nAnimId < 229) { // ? todo
				CAnimBlendAssociation* pAssoc = CAnimManager::GetAnimAssociation(animA.m_nGroupId, animA.m_nAnimId);
				float fCurTimeB = syncB.m_aPedAnims[b].m_fCurrentTime;
				float fTotalLen = pAssoc->m_aHierarchy->m_fTotalLength; // ? todo
				float fSpeedA = animA.m_fSpeed;
				float fCurTimeA = animA.m_fCurrentTime;

				if (fSpeedA <= 0.0f) {
					if (fCurTimeA < fCurTimeB)
						fCurTimeB -= fTotalLen;
				}
				else {
					if (fCurTimeA > fCurTimeB)
						fCurTimeB += fTotalLen;
				}

				float fCurTime = (1.0f - fDelta) * fCurTimeA + fDelta * fCurTimeB;

				if ((animA.m_nFlags & ASSOC_REPEAT) != 0) {
					while (fCurTime < 0.0f) fCurTime += fTotalLen;
					while (fCurTime > fTotalLen) fCurTime -= fTotalLen;
				}
				else {
					if (fCurTime < 0.0f || fCurTime > fTotalLen)
						fCurTime = (fSpeedA > 0.0f) ? fTotalLen : 0.0f;
				}

				animA.m_fCurrentTime = fCurTime;
			}

			++b;
			continue;
		}

		if ((b + 1) < syncB.m_nAnimCount &&
			animA.m_nGroupId == syncB.m_aPedAnims[b + 1].m_nGroupId &&
			animA.m_nAnimId == syncB.m_aPedAnims[b + 1].m_nAnimId)
		{
			++b;
			if (animA.m_nAnimId < 229) {
				CAnimBlendAssociation* pAssoc = CAnimManager::GetAnimAssociation(animA.m_nGroupId, animA.m_nAnimId);
				float fCurTimeB = syncB.m_aPedAnims[b].m_fCurrentTime;
				float fTotalLen = pAssoc->m_aHierarchy->m_fTotalLength;
				float fSpeedA = animA.m_fSpeed;
				float fCurTimeA = animA.m_fCurrentTime;

				if (fSpeedA <= 0.0f) {
					if (fCurTimeA < fCurTimeB)
						fCurTimeB -= fTotalLen;
				}
				else {
					if (fCurTimeA > fCurTimeB)
						fCurTimeB += fTotalLen;
				}
				float fCurTime = (1.0f - fDelta) * fCurTimeA + fDelta * fCurTimeB;
				if ((animA.m_nFlags & ASSOC_REPEAT) != 0) {
					while (fCurTime < 0.0f) fCurTime += fTotalLen;
					while (fCurTime > fTotalLen) fCurTime -= fTotalLen;
				}
				else {
					if (fCurTime < 0.0f || fCurTime > fTotalLen)
						fCurTime = (fSpeedA > 0.0f) ? fTotalLen : 0.0f;
				}
				animA.m_fCurrentTime = fCurTime;
			}

			++b;
			continue;
		}
	}
#else
#if 0 // usless in vcs
	for (int32 a = 0, b = 0; a < syncA.m_nAnimCount; ++a) {
		if (syncA.m_aPedAnims[a].m_nGroupId == syncB.m_aPedAnims[b].m_nGroupId &&
			syncA.m_aPedAnims[a].m_nAnimId == syncB.m_aPedAnims[b].m_nAnimId) {
			++b;
			continue;
		}

		if (b + 1 < syncB.m_nAnimCount &&
			syncA.m_aPedAnims[a].m_nGroupId == syncB.m_aPedAnims[b + 1].m_nGroupId &&
			syncA.m_aPedAnims[a].m_nAnimId == syncB.m_aPedAnims[b + 1].m_nAnimId) {
			b += 2;
		}
	}
#endif
#endif
}

bool sPed::IsTransferable(void) {
	if (GetID() > eElementID::MG_ELEMENT_PLAYER_PED_ID)
	{
		sPedSync* pSync = GetSync().ped;
		sElement* pVeh = pSync->InVehicle() ? cMultiGame::Instance().GetEntityForHandle(GetOwner(), pSync->GetVehicleID()) : nil;
		if (pVeh != nil && pVeh->IsTransferable())
			return true; // usless?
		return true; // bug?
	}
	return false;
}

void sPed::TransferEntity(int16 nDestPlayer) {
	debug("Transfering Ped %d %x\n", GetID(), GetEntity());
	sPedSync* pSync = GetSync().ped;
	sElement* pVeh = pSync->InVehicle() ? cMultiGame::Instance().GetEntityForHandle(GetOwner(), pSync->GetVehicleID()) : nil;
	if (pVeh == nil || pVeh->WasTransfered()) {
		if (pVeh != nil)
			debug("Ped is in a car but car has been transferred already\n");
		else
			debug("Ped is on foot\n");
		sElement::TransferEntity(nDestPlayer);
	}
	debug("Ped is in vehicle, transfer vehicle\n");
	pVeh->TransferEntity(nDestPlayer);
}

void sPed::CompareSyncState(tPedSyncsDeltas* pDiff, sPedSync* pSync, sPedSync* pLastSync, uint32 nDelta) {
	// maybe smth for FLT_EPS_NOT_EQ?
	const float POSITION_EPSILON = 0.001f; // recheck if gcc shit
	const float ROTATION_EPSILON = 0.001f;
	const float MOVE_SPEED_EPSILON = 0.001f;

	pDiff->SetEqual();

	if (pLastSync->m_nVehicleID != pSync->m_nVehicleID)
		pDiff->nPedDiff |= ePedSync::MP_PKTD_PED_VEHICLE_ID;

	bool posXYChanged = false;
	if (fabsf(pLastSync->GetMatrix().GetPosition().x - pSync->GetMatrix().GetPosition().x) > POSITION_EPSILON)
		posXYChanged = true;
	else if (fabsf(pLastSync->GetMatrix().GetPosition().y - pSync->GetMatrix().GetPosition().y) > POSITION_EPSILON)
		posXYChanged = true;

	if (posXYChanged)
		pDiff->nPedDiff |= ePedSync::MP_PKTD_PED_POSITION_XY;
	else { // !!---- UPD!!
		pSync->GetMatrix().GetPosition().x = pLastSync->GetMatrix().GetPosition().x;
		pSync->GetMatrix().GetPosition().y = pLastSync->GetMatrix().GetPosition().y;
	}

	if (pLastSync->m_nHealth != pSync->m_nHealth)
		pDiff->nPedDiff |= ePedSync::MP_PKTD_PED_HEALTH;

	if (pLastSync->m_nPedState != pSync->m_nPedState)
		pDiff->nPedDiff |= ePedSync::MP_PKTD_PED_STATE;

	if (pLastSync->m_nObjective != pSync->m_nObjective)
		pDiff->nPedDiff |= ePedSync::MP_PKTD_PED_OBJECTIVE;

	if (fabsf(pLastSync->GetMatrix().GetPosition().z - pSync->GetMatrix().GetPosition().z) > POSITION_EPSILON)
		pDiff->nPedDiff |= ePedSync::MP_PKTD_PED_POSITION_Z;
	else // !!---- UPD!!
		pSync->GetMatrix().GetPosition().z = pLastSync->GetMatrix().GetPosition().z;

	if (fabsf(pLastSync->m_fRotationCur - pSync->m_fRotationCur) > ROTATION_EPSILON)
		pDiff->nPedDiff |= ePedSync::MP_PKTD_PED_ROTATION_CUR;
	else // !!---- UPD!!
		pSync->m_fRotationCur = pLastSync->m_fRotationCur;

	if (fabsf(pLastSync->m_fRotationDest - pSync->m_fRotationDest) > ROTATION_EPSILON)
		pDiff->nPedDiff |= ePedSync::MP_PKTD_PED_ROTATION_DEST;
	else // !!---- UPD!!
		pSync->m_fRotationDest = pLastSync->m_fRotationDest;

	if (pLastSync->m_nAnimCount != pSync->m_nAnimCount)
		pDiff->nPedDiff |= ePedSync::MP_PKTD_PED_ANIM_COUNT;

#if 1 // my vision
	for (int32 i = 0; i < pSync->m_nAnimCount; i++) {
		if (i < pLastSync->m_nAnimCount)
			pDiff->aAnimDiff[i] = CompareAnimSyncState(&pSync->m_aPedAnims[i], &pLastSync->m_aPedAnims[i], nDelta);
		else
			pDiff->aAnimDiff[i] = static_cast<uint8>(ePedAnimSync::MP_PKTD_PED_ANIM_FULL);

		if (pDiff->aAnimDiff[i] != ePedAnimSync::MP_PKTD_PED_ANIM_EQUAL) // write to ped diff what animation index does diff have
			pDiff->nPedDiff |= MP_PKTD_PED_ANIM_MASK(i);
	}
#else
	for (int32 i = 0; i < pSync->m_nAnimCount; i++) {
		if (i < pLastSync->m_nAnimCount)
			pDiff->nAnimDiff |= (CompareAnimSyncState(&pSync->m_aPedAnims[i], &pLastSync->m_aPedAnims[i], nDelta) << (i * 8));
		else
			pDiff->nAnimDiff |= (static_cast<uint8>(ePedAnimSync::MP_PKTD_PED_ANIM_FULL) << (i * 8)); // -1

		if ((pDiff->nAnimDiff >> (i * 8)) & 0xFF) // write to ped diff what animation index does diff have
			pDiff->nPedDiff |= MP_PKTD_PED_ANIM_MASK(i);
	}
#endif

	if (pLastSync->m_CompressedPedIK.m_flags != pSync->m_CompressedPedIK.m_flags)
		pDiff->nPedDiff |= ePedSync::MP_PKTD_PED_IK_FLAGS;

	pDiff->nIkDiff = sPedIKSync::CompareSyncState(&pSync->m_CompressedPedIK, &pLastSync->m_CompressedPedIK);
	if (pDiff->nIkDiff != ePedIKSync::MP_PKTD_PED_IK_EQUAL)
		pDiff->nPedDiff |= ePedSync::MP_PKTD_PED_IK_DATA;

	if (pLastSync->m_eCurWeaponType != pSync->m_eCurWeaponType)
		pDiff->nPedDiff |= ePedSync::MP_PKTD_PED_WEAPON_TYPE;

	if (pSync->m_eCurWeaponType == eWeaponType::WEAPONTYPE_CHAINSAW && pLastSync->m_eCurWeaponState != pSync->m_eCurWeaponState)
		pDiff->nPedDiff |= ePedSync::MP_PKTD_PED_WEAPON_STATE;

	if (pLastSync->m_bBodyPartJustCameOff != pSync->m_bBodyPartJustCameOff)
		pDiff->nPedDiff |= ePedSync::MP_PKTD_PED_BODY_PART_CAME_OFF;

	if (pLastSync->m_bIsPedDieAnimPlaying != pSync->m_bIsPedDieAnimPlaying)
		pDiff->nPedDiff |= ePedSync::MP_PKTD_PED_DIE_ANIM_PLAYING;

	if (pLastSync->m_nPlayerTeam != pSync->m_nPlayerTeam)
		pDiff->nPedDiff |= ePedSync::MP_PKTD_PED_PLAYER_TEAM;

	if (pLastSync->m_bPhys_lvcs_unk_1 != pSync->m_bPhys_lvcs_unk_1)
		pDiff->nPedDiff |= ePedSync::MP_PKTD_PED_PHYS_LVCS_UNK1;

	if (FindPlayerPed() == GetEntity())
	{
		bool moveSpeedChanged = false;
		if (fabsf(pLastSync->GetMoveSpeed().x - pSync->GetMoveSpeed().x) > MOVE_SPEED_EPSILON)
			moveSpeedChanged = true;
		else if (fabsf(pLastSync->GetMoveSpeed().y - pSync->GetMoveSpeed().y) > MOVE_SPEED_EPSILON)
			moveSpeedChanged = true;
		else if (fabsf(pLastSync->GetMoveSpeed().z - pSync->GetMoveSpeed().z) > MOVE_SPEED_EPSILON)
			moveSpeedChanged = true;

		if (moveSpeedChanged)
			pDiff->nPedDiff |= ePedSync::MP_PKTD_PED_MOVE_SPEED;
		else // !!---- UPD!!
			pSync->GetMoveSpeed() = pLastSync->GetMoveSpeed();
	}

	if (pLastSync->m_nPeerLockOnMG != pSync->m_nPeerLockOnMG)
		pDiff->nPedDiff |= ePedSync::MP_PKTD_PED_PEER_LOCK_ON;

	if (pLastSync->m_nSurfaceTouched != pSync->m_nSurfaceTouched)
		pDiff->nPedDiff |= ePedSync::MP_PKTD_PED_SURFACE_TOUCHED;

	if (pLastSync->m_nMoveState != pSync->m_nMoveState)
		pDiff->nPedDiff |= ePedSync::MP_PKTD_PED_MOVE_STATE;

	if (pLastSync->m_nPedPhys_lvcs_unk_flagsB != pSync->m_nPedPhys_lvcs_unk_flagsB)
		pDiff->nPedDiff |= ePedSync::MP_PKTD_PED_PHYS_LVCS_FLAGS_B;
}

bool sPed::WriteSyncDelta(sWriteSyncStream* pSyncStream, sPedSync* pSync, sPedSync* pLastSync, uint32 nDelta) {
	tPedSyncsDeltas pedDeltaManager{};
#ifdef FIX_BUGS
	pedDeltaManager.SetEqual();
#endif
	CompareSyncState(&pedDeltaManager, pSync, pLastSync, nDelta);

	if (pedDeltaManager.nPedDiff == ePedSync::MP_PKTD_PED_EQUAL) // main delta
		return false;

	PerformWriteSync(pSyncStream, pSync, &pedDeltaManager);
	return true;
}

void sPed::PerformWriteSync(sWriteSyncStream* pSyncStream, sPedSync* pSync, tPedSyncsDeltas* pDiff) {
	// clear bits unused anim slots
	for (uint32 i = pSync->m_nAnimCount; i < NUM_MULTIPLAYER_SYNC_ANIMS; i++)
		pDiff->nPedDiff &= ~MP_PKTD_PED_ANIM_MASK(i);

	pSyncStream->WriteU32(pDiff->nPedDiff);

	if (pDiff->nPedDiff & ePedSync::MP_PKTD_PED_MODEL_TYPE)
	{
		pSyncStream->WriteI16(GetPhysical()->GetModelIndex());
		pSyncStream->WriteU8(static_cast<uint8>(pSync->m_nPedType));
		if (pSync->m_nPedType == PEDTYPE_COP)
			pSyncStream->WriteU8(static_cast<uint8>(((CCopPed*)GetEntity())->m_nCopType));
	}

	if (pDiff->nPedDiff & ePedSync::MP_PKTD_PED_HEALTH)
		pSyncStream->WriteU16(pSync->m_nHealth);

	if (pDiff->nPedDiff & ePedSync::MP_PKTD_PED_STATE)
		pSyncStream->WriteU8(static_cast<uint8>(pSync->m_nPedState));

	if (pDiff->nPedDiff & ePedSync::MP_PKTD_PED_OBJECTIVE)
		pSyncStream->WriteU8(static_cast<uint8>(pSync->m_nObjective));

	if (pDiff->nPedDiff & ePedSync::MP_PKTD_PED_VEHICLE_ID)
		pSyncStream->WriteI16(pSync->m_nVehicleID);

	if (pDiff->nPedDiff & ePedSync::MP_PKTD_PED_POSITION_XY)
	{
		pSyncStream->WriteFloat(pSync->GetMatrix().GetPosition().x);
		pSyncStream->WriteFloat(pSync->GetMatrix().GetPosition().y);
	}

	if (pDiff->nPedDiff & ePedSync::MP_PKTD_PED_POSITION_Z)
		pSyncStream->WriteFloat(pSync->GetMatrix().GetPosition().z);

	if (pDiff->nPedDiff & ePedSync::MP_PKTD_PED_ROTATION_CUR)
		pSyncStream->WriteFloat(pSync->m_fRotationCur);

	if (pDiff->nPedDiff & ePedSync::MP_PKTD_PED_ROTATION_DEST)
		pSyncStream->WriteFloat(pSync->m_fRotationDest);

	if (pDiff->nPedDiff & ePedSync::MP_PKTD_PED_ANIM_COUNT)
		pSyncStream->WriteU8(pSync->m_nAnimCount);

#if 1 // my vision
	for (int32 i = 0; i < pSync->m_nAnimCount; i++) {
		if (pDiff->nPedDiff & MP_PKTD_PED_ANIM_MASK(i)) {
			PerformWriteAnimSync(pSyncStream, &pSync->m_aPedAnims[i], pDiff->aAnimDiff[i]);
		}
	}
#else
	for (int32 i = 0; i < pSync->m_nAnimCount; i++) {
		if (pDiff->nPedDiff & MP_PKTD_PED_ANIM_MASK(i)) {
			uint8 animDiffMask = static_cast<uint8>((pDiff->nAnimDiff >> (i * 8)) & 0xFF);
			PerformWriteAnimSync(pSyncStream, &pSync->m_aPedAnims[i], animDiffMask);
		}
	}
#endif

	if (pDiff->nPedDiff & ePedSync::MP_PKTD_PED_IK_FLAGS)
		pSyncStream->WriteU8(pSync->m_CompressedPedIK.m_flags);

	if (pDiff->nPedDiff & ePedSync::MP_PKTD_PED_IK_DATA) {
		sPedIKSync::PerformWriteSync(pSyncStream, &pSync->m_CompressedPedIK, pDiff->nIkDiff, ePedIKSync::MP_PKTD_PED_IK_FULL);
	}

	if (pDiff->nPedDiff & ePedSync::MP_PKTD_PED_WEAPON_TYPE)
		pSyncStream->WriteU8(static_cast<uint8>(pSync->m_eCurWeaponType));

	if (pDiff->nPedDiff & ePedSync::MP_PKTD_PED_WEAPON_STATE)
		pSyncStream->WriteU8(static_cast<uint8>(pSync->m_eCurWeaponState));

	if (pDiff->nPedDiff & ePedSync::MP_PKTD_PED_BODY_PART_CAME_OFF)
		pSyncStream->WriteBool(pSync->m_bBodyPartJustCameOff);

	if (pDiff->nPedDiff & ePedSync::MP_PKTD_PED_DIE_ANIM_PLAYING)
		pSyncStream->WriteBool(pSync->m_bIsPedDieAnimPlaying);

	if (pDiff->nPedDiff & ePedSync::MP_PKTD_PED_PLAYER_TEAM)
		pSyncStream->WriteI16(pSync->m_nPlayerTeam);

	if (pDiff->nPedDiff & ePedSync::MP_PKTD_PED_PHYS_LVCS_UNK1)
		pSyncStream->WriteBool(pSync->m_bPhys_lvcs_unk_1);

	if (pDiff->nPedDiff & ePedSync::MP_PKTD_PED_MOVE_SPEED)
		pSyncStream->WriteVector(pSync->GetMoveSpeed());

	if (pDiff->nPedDiff & ePedSync::MP_PKTD_PED_PEER_LOCK_ON)
		pSyncStream->WriteI8(pSync->m_nPeerLockOnMG);

	if (pDiff->nPedDiff & ePedSync::MP_PKTD_PED_SURFACE_TOUCHED)
		pSyncStream->WriteI16(pSync->m_nSurfaceTouched);

	if (pDiff->nPedDiff & ePedSync::MP_PKTD_PED_MOVE_STATE)
		pSyncStream->WriteU8(static_cast<uint8>(pSync->m_nMoveState));

#if !defined(FINAL) && !defined(MASTER)
	CVector pos = pSync->GetMatrix().GetPosition();
	CVector fwd = pSync->GetMatrix().GetForward();
	PED_LOG(1, " PedState (%f %f %f) (%f %f %f) car: %i heading: %f %f anims: %i",
		pos.x, pos.y, pos.z, fwd.x, fwd.y, fwd.z, pSync->m_nVehicleID,
		pSync->m_fRotationCur, pSync->m_fRotationDest, pSync->m_nAnimCount);

	for (uint32 i = 0; i < pSync->m_nAnimCount; i++) {
		sPedAnimSync& anim = pSync->m_aPedAnims[i];
		PED_LOG(1, " anim %i: blend %f %f time %f spd %f anm %i, %i flags %i",
			i, anim.m_fBlendAmount, anim.m_fBlendDelta, anim.m_fCurrentTime, anim.m_fSpeed,
			anim.m_nAnimId, anim.m_nGroupId, anim.m_nFlags);
	}
#endif

#ifndef GTA_LIBERTY
	if (pDiff->nPedDiff & ePedSync::MP_PKTD_PED_PHYS_LVCS_FLAGS_B)
		pSyncStream->WriteU8(pSync->m_nPedPhys_lvcs_unk_flagsB);
#endif
}

uint8 sPed::CompareAnimSyncState(sPedAnimSync* pSync, sPedAnimSync* pLastSync, int32 nDelta) {
	const float FLOAT_EPSILON = 0.000001f;
	const float CURRENT_TIME_EPSILON = 0.0099999998f; // 0.01f

	uint8 nDiffMask = ePedAnimSync::MP_PKTD_PED_ANIM_EQUAL;

	if (fabsf(pLastSync->m_fBlendAmount - pSync->m_fBlendAmount) > FLOAT_EPSILON)
		nDiffMask |= ePedAnimSync::MP_PKTD_PED_ANIM_BLEND_AMOUNT;
	else // !!---- UPD!!
		pSync->m_fBlendAmount = pLastSync->m_fBlendAmount;

	if (fabsf(pLastSync->m_fBlendDelta - pSync->m_fBlendDelta) > FLOAT_EPSILON)
		nDiffMask |= ePedAnimSync::MP_PKTD_PED_ANIM_BLEND_DELTA;
	else // !!---- UPD!!
		pSync->m_fBlendDelta = pLastSync->m_fBlendDelta;

	float predictedCurrentTime = pLastSync->m_fCurrentTime;
	if (pLastSync->m_nFlags & eAnimAssocFlags::ASSOC_RUNNING)
		predictedCurrentTime += (nDelta * NET_SESSION_R_60) * pLastSync->m_fSpeed;

	if (fabsf(predictedCurrentTime - pSync->m_fCurrentTime) > CURRENT_TIME_EPSILON)
		nDiffMask |= ePedAnimSync::MP_PKTD_PED_ANIM_CURRENT_TIME;
	else // !!---- UPD!!
		pSync->m_fCurrentTime = predictedCurrentTime;

	if (fabsf(pLastSync->m_fSpeed - pSync->m_fSpeed) > FLOAT_EPSILON)
		nDiffMask |= ePedAnimSync::MP_PKTD_PED_ANIM_SPEED;
	else // !!---- UPD!!
		pSync->m_fSpeed = pLastSync->m_fSpeed;

	if (pLastSync->m_nGroupId != pSync->m_nGroupId)
		nDiffMask |= (ePedAnimSync::MP_PKTD_PED_ANIM_GROUP_ID | ePedAnimSync::MP_PKTD_PED_ANIM_ANIM_ID);

	if (pLastSync->m_nAnimId != pSync->m_nAnimId)
		nDiffMask |= (ePedAnimSync::MP_PKTD_PED_ANIM_GROUP_ID | ePedAnimSync::MP_PKTD_PED_ANIM_ANIM_ID);

	if ((pLastSync->m_nFlags & 0x0000FFFF) != (pSync->m_nFlags & 0x0000FFFF))
		nDiffMask |= ePedAnimSync::MP_PKTD_PED_ANIM_FLAGS_LOW;

	if ((pLastSync->m_nFlags & 0xFFFF0000) != (pSync->m_nFlags & 0xFFFF0000))
		nDiffMask |= ePedAnimSync::MP_PKTD_PED_ANIM_FLAGS_HIGH;

	return nDiffMask;
}

void sPed::PerformWriteAnimSync(sWriteSyncStream* pSyncStream, sPedAnimSync* pSync, uint8 nDiffMask) {
	pSyncStream->WriteU8(nDiffMask);

	if (nDiffMask & ePedAnimSync::MP_PKTD_PED_ANIM_BLEND_AMOUNT)
		pSyncStream->WriteFloat(pSync->m_fBlendAmount);

	if (nDiffMask & ePedAnimSync::MP_PKTD_PED_ANIM_BLEND_DELTA)
		pSyncStream->WriteFloat(pSync->m_fBlendDelta);

	if (nDiffMask & ePedAnimSync::MP_PKTD_PED_ANIM_CURRENT_TIME)
		pSyncStream->WriteFloat(pSync->m_fCurrentTime);

	if (nDiffMask & ePedAnimSync::MP_PKTD_PED_ANIM_SPEED)
		pSyncStream->WriteFloat(pSync->m_fSpeed);

	if (nDiffMask & ePedAnimSync::MP_PKTD_PED_ANIM_GROUP_ID)
		pSyncStream->WriteI16(pSync->m_nGroupId);

	if (nDiffMask & ePedAnimSync::MP_PKTD_PED_ANIM_ANIM_ID)
		pSyncStream->WriteI16(pSync->m_nAnimId);

	if (nDiffMask & MP_PKTD_PED_ANIM_FLAGS_LOW)
		pSyncStream->WriteI16(static_cast<int16>(pSync->m_nFlags & 0xFFFF));

	if (nDiffMask & MP_PKTD_PED_ANIM_FLAGS_HIGH)
		pSyncStream->WriteI16(static_cast<int16>((pSync->m_nFlags >> 16) & 0xFFFF));
}

void sPed::ReadAnimSyncFromStream(sReadSyncStream* pSyncStream, sPedAnimSync* pOutSync) {
	uint8 nDiffMask = pSyncStream->ReadU8();

	if (nDiffMask & ePedAnimSync::MP_PKTD_PED_ANIM_BLEND_AMOUNT)
		pOutSync->m_fBlendAmount = pSyncStream->ReadFloat();

	if (nDiffMask & ePedAnimSync::MP_PKTD_PED_ANIM_BLEND_DELTA)
		pOutSync->m_fBlendDelta = pSyncStream->ReadFloat();

	if (nDiffMask & ePedAnimSync::MP_PKTD_PED_ANIM_CURRENT_TIME)
		pOutSync->m_fCurrentTime = pSyncStream->ReadFloat();

	if (nDiffMask & ePedAnimSync::MP_PKTD_PED_ANIM_SPEED)
		pOutSync->m_fSpeed = pSyncStream->ReadFloat();

	if (nDiffMask & ePedAnimSync::MP_PKTD_PED_ANIM_GROUP_ID)
		pOutSync->m_nGroupId = pSyncStream->ReadI16();

	if (nDiffMask & ePedAnimSync::MP_PKTD_PED_ANIM_ANIM_ID)
		pOutSync->m_nAnimId = pSyncStream->ReadI16();

	if (nDiffMask & ePedAnimSync::MP_PKTD_PED_ANIM_FLAGS_LOW) {
		uint16 flagsLow = pSyncStream->ReadU16();
		pOutSync->m_nFlags = (pOutSync->m_nFlags & 0xFFFF0000) | flagsLow;
	}

	if (nDiffMask & ePedAnimSync::MP_PKTD_PED_ANIM_FLAGS_HIGH) {
		uint16 flagsHigh = pSyncStream->ReadU16();
		pOutSync->m_nFlags = (pOutSync->m_nFlags & 0x0000FFFF) | (flagsHigh << 16);
	}
}

bool sPed::IsPedHeadAbovePos(float fZOffset) {
	return fZOffset + GetSync().ped->GetMatrix().GetPosition().z < ((CPed*)GetPhysical())->GetNodePosition(PED_HEAD).z;
}

bool sPed::GetPedBoneWorldPosition(CVector& pos, int32 nNode) {
	CVector Pos(0.0f, 0.0f, 0.0f);
	if (nNode > PED_TORSO && nNode < PED_NODE_MAX) {
		((CPed*)GetPhysical())->TransformToNode(Pos, nNode); // by m_pFrames
		pos = Pos; // :/
		return true;
	}
	pos = Pos;
	return false;
}

void sPed::OnPreRender(uint16 nTime) {
	FindSync(nTime, nil).ped->m_CompressedPedIK.OnPreRender(this);
}

// Set anim data from sync to physical clump
void sPed::UpdateAnim(uint16 nTime, float fDelta) {
	if (nTime == m_nPedTime)
		return;

	m_nPedTime = nTime;
	sPedSync* sync = FindSync(nTime, nil).ped;

	assert(GetPhysical() && RwObjectGetType(GetPhysical()->m_rwObject) == rpCLUMP);
	RpClump* pClump = (RpClump*)GetPhysical()->m_rwObject;
	// Array to track which sync animations have been matched to existing associations
	bool matched[NUM_MULTIPLAYER_SYNC_ANIMS] = { false, false, false, false, false };

	// First pass: Update or remove existing animations on the clump
	CAnimBlendClumpData* clumpData = *RPANIMBLENDCLUMPDATA(pClump);
	CAnimBlendLink* link = clumpData->link.next;
	while (link)
	{
		CAnimBlendLink* next_link = link->next; // avoid crash after deleting our assoc
		CAnimBlendAssociation* assoc = CAnimBlendAssociation::FromLink(link);
		bool found = false;

		for (uint32 i = 0; i < sync->m_nAnimCount; ++i) {
			if (!matched[i] && assoc->animId == sync->m_aPedAnims[i].m_nAnimId && assoc->groupId == sync->m_aPedAnims[i].m_nGroupId) {

				matched[i] = true;
				found = true;
				sPedAnimSync* syncAnim = &sync->m_aPedAnims[i];

				assoc->SetBlend(syncAnim->m_fBlendAmount, syncAnim->m_fBlendDelta);

				float syncTime = syncAnim->m_fCurrentTime;
#if !defined(FINAL) && !defined(MASTER)
				//if(TheMPGame.FindPlayerPedMG() == this)
				//debug("-------------GOT TIME anim SYNC->cPedMgClump sPed::UpdateAnim %f, %f %f [%d]\n", syncTime, syncAnim->m_fBlendAmount, syncAnim->m_fBlendDelta, assoc->animId);
#endif
				if (syncTime <= assoc->currentTime)
					assoc->SetCurrentTime(syncTime);
				else
					assoc->UpdateTime(fDelta * (syncTime - assoc->currentTime), 1.0f);

				assoc->flags = 0x0;
#ifdef GTA_LIBERTY
				assoc->flags |= syncAnim->m_nFlags;
#else
				assoc->flags = syncAnim->m_nFlags;
#endif
				assoc->speed = syncAnim->m_fSpeed;
				break;
			}
		}

		if (!found)
			delete assoc;

		link = next_link;
	}

	// Second pass: Add any new animations from sync that weren't matched
	// Warn!! Reversing of the sequence due to anim prepend A B C -> C B A (sPedSync::CopyAnimations)
	for (int32 i = 0; i < sync->m_nAnimCount; ++i) {
		if (!matched[i])
		{
			sPedAnimSync* syncAnim = &sync->m_aPedAnims[i];
#ifdef GTA_LIBERTY
			if (syncAnim->m_nAnimId < 229 || m_nOwnerID == TheMPGame.LocalPlayerID()) // todo 229
#endif
			{
#ifdef FIX_BUGS
				//int32 idleAnimBlockIndex = TheAnimManager->GetAnimationBlockIndex("playidles");
				//CStreaming::RequestAnim(idleAnimBlockIndex, STREAMFLAGS_DONT_REMOVE);
				// m_nGroupId to animblockindex + load
				TODO("if !load -> load anim id group id before add crash prevent"); // but not in psp, if req non loaded block it crash psp peer
#endif
				CAnimBlendAssociation* newAssoc = TheAnimManager->AddAnimation(pClump, (AssocGroupId)syncAnim->m_nGroupId, (AnimationId)syncAnim->m_nAnimId);
				assert(newAssoc);
				newAssoc->SetBlend(syncAnim->m_fBlendAmount, syncAnim->m_fBlendDelta);
				newAssoc->SetCurrentTime(syncAnim->m_fCurrentTime);
				newAssoc->flags = 0x0;
#ifdef GTA_LIBERTY
				newAssoc->flags |= syncAnim->m_nFlags;
#else
				newAssoc->flags = syncAnim->m_nFlags;
#endif
				newAssoc->speed = syncAnim->m_fSpeed;
			}
		}
	}
	if (!clumpData->link.next) {
		debug("Warning!!! ElementGroup has no animations\n");
	}
}

std::vector<sPed*> sPed::ms_peds;
int32 sPed::ms_nNumberOfSyncedPeds = 0;

// Audio
enum
{
	PED_ONE_SHOT_SHIRT_FLAP_MAX_DIST = 15,
	PED_ONE_SHOT_SHIRT_FLAP_VOLUME = 90,

	PED_ONE_SHOT_MINIGUN_MAX_DIST = 150,
	PED_ONE_SHOT_MINIGUN_VOLUME = MAX_VOLUME,

	PED_ONE_SHOT_SKATING_MAX_DIST = 20,
	PED_ONE_SHOT_SKATING_VOLUME = 70,

	PED_ONE_SHOT_STEP_MAX_DIST = 20,
	PED_ONE_SHOT_STEP_VOLUME = 45,

	PED_ONE_SHOT_FALL_MAX_DIST = 30,
	PED_ONE_SHOT_FALL_VOLUME = 80,

	PED_ONE_SHOT_PUNCH_MAX_DIST = 30,
	PED_ONE_SHOT_PUNCH_VOLUME = 100,

	PED_ONE_SHOT_WEAPON_COLT45_VOLUME = 90,
	PED_ONE_SHOT_WEAPON_UZI_VOLUME = 70,
	PED_ONE_SHOT_WEAPON_SHOTGUN_VOLUME = 100,
	PED_ONE_SHOT_WEAPON_M4_VOLUME = 90,
	PED_ONE_SHOT_WEAPON_M16_VOLUME = MAX_VOLUME,
	PED_ONE_SHOT_WEAPON_SNIPERRIFLE_VOLUME = 110,
	PED_ONE_SHOT_WEAPON_ROCKETLAUNCHER_VOLUME = 80,

	PED_ONE_SHOT_WEAPON_FLAMETHROWER_MAX_DIST = 60,
	PED_ONE_SHOT_WEAPON_FLAMETHROWER_VOLUME = 90,

	PED_ONE_SHOT_WEAPON_RELOAD_MAX_DIST = 30,
	PED_ONE_SHOT_WEAPON_RELOAD_VOLUME = 75,

	PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST = 120,
	PED_ONE_SHOT_WEAPON_BULLET_ECHO_VOLUME = 80,

	PED_ONE_SHOT_WEAPON_FLAMETHROWER_FIRE_MAX_DIST = 60,
	PED_ONE_SHOT_WEAPON_FLAMETHROWER_FIRE_VOLUME = 70,

	PED_ONE_SHOT_WEAPON_CHAINSAW_MAX_DIST = 60,
	PED_ONE_SHOT_WEAPON_CHAINSAW_IDLE_MAX_DIST = 50,
	PED_ONE_SHOT_WEAPON_CHAINSAW_VOLUME = 100,

	PED_ONE_SHOT_WEAPON_HIT_PED_MAX_DIST = 30,
	PED_ONE_SHOT_WEAPON_HIT_PED_VOLUME = 90,

	PED_ONE_SHOT_SPLASH_MAX_DIST = 40,
	PED_ONE_SHOT_SPLASH_PED_VOLUME = 70,

	PED_COMMENT_MAX_DIST = 40,
	PED_COMMENT_POLICE_HELI_MAX_DIST = 400,
};

void cAudioManager::ProcessMultiplayerPed(cPedMG* ped)
{
	cPedParams params;

	m_sQueueSample.m_vecPos = ped->GetPosition();

	params.m_bDistanceCalculated = FALSE;
	params.m_pNetPed = ped;
	params.m_fDistance = GetDistanceSquared(m_sQueueSample.m_vecPos);
	ProcessMultiplayerPedOneShots(params);
}

void cAudioManager::ProcessMultiplayerPedOneShots(cPedParams& params)
{
    uint8 Vol;
    uint32 sampleIndex;

    cPedMG* ped = params.m_pNetPed;
	sPed* pPed = params.m_pNetPed->GetElement().ped;

    bool8 narrowSoundRange;
    int16 sound;
    bool8 stereo;
    //CWeapon* weapon;
#ifdef FIX_BUGS
    float maxDist = 0.0f; // uninitialized variable
#else
    float maxDist;
#endif

    static uint8 iSound = 21;
    static uint32 iSplashFrame = 0;

	TODO();
	TODO();
	TODO(); // SFX_FIGHT_5
	TODO(); // SFX_FIGHT_2
	TODO();
	TODO();

    //weapon = params.m_pPed->GetWeapon();
    for (uint32 i = 0; i < m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_AudioEvents; i++)
    {
        stereo = FALSE;
        narrowSoundRange = FALSE;
        SET_SOUND_REFLECTION(FALSE);
        sound = m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_awAudioEvent[i];
        switch (sound)
        {
            case SOUND_STEP_START:
            case SOUND_STEP_END:
            {
                if (pPed->GetSync().ped->m_nPedState == PedState::PED_JUMP ||
#if !defined(GTA_LIBERTY) && defined(FIX_BUGS) // need?
					//pPed->GetSync().ped->m_nPedState == PedState::PED_LEDGE_JUMP ||
#endif
					pPed->GetSync().ped->m_nPedState == PedState::PED_FALL)
                    continue;

                Vol = m_anRandomTable[3] % 15 + PED_ONE_SHOT_STEP_VOLUME;
                //if (FindPlayerPed() != m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_pEntity)
                //    Vol >>= 1;
                maxDist = SQR(PED_ONE_SHOT_STEP_MAX_DIST);
                switch (pPed->GetSync().ped->m_nSurfaceTouched)
                {
					case SURFACE_GRASS:
						TODO(); // if else
						sampleIndex = m_anRandomTable[1] % 4 + SFX_FOOTSTEP_GRASS_1;
						break;
					case SURFACE_GRAVEL:
					case SURFACE_MUD_DRY:
						TODO(); // if else
						sampleIndex = m_anRandomTable[4] % 5 + SFX_FOOTSTEP_GRAVEL_1;
						break;
					case SURFACE_CAR:
					case SURFACE_GARAGE_DOOR:
					case SURFACE_CAR_PANEL:
					case SURFACE_THICK_METAL_PLATE:
					case SURFACE_SCAFFOLD_POLE:
					case SURFACE_LAMP_POST:
					case SURFACE_FIRE_HYDRANT:
					case SURFACE_GIRDER:
					case SURFACE_METAL_CHAIN_FENCE:
					case SURFACE_CONTAINER:
					case SURFACE_NEWS_VENDOR:
						sampleIndex = m_anRandomTable[0] % 5 + SFX_FOOTSTEP_METAL_1;
						break;
					case SURFACE_SAND:
						sampleIndex = m_anRandomTable[4] % 4 + SFX_FOOTSTEP_SAND_1;
						break;
					case SURFACE_WATER:
						sampleIndex = m_anRandomTable[3] % 4 + SFX_FOOTSTEP_WATER_1;
						break;
					case SURFACE_WOOD_CRATES:
					case SURFACE_WOOD_BENCH:
					case SURFACE_WOOD_SOLID:
						sampleIndex = m_anRandomTable[2] % 5 + SFX_FOOTSTEP_WOOD_1;
						break;
					case SURFACE_HEDGE:
						sampleIndex = m_anRandomTable[2] % 3 + SFX_COL_VEG_1;
						break;
					default:
						sampleIndex = m_anRandomTable[2] % 5 + SFX_FOOTSTEP_CONCRETE_1;
						break;
                }
                m_sQueueSample.m_nSampleIndex = sampleIndex;
                m_sQueueSample.m_nBankIndex = SFX_BANK_0;
                m_sQueueSample.m_nCounter = m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_awAudioEvent[i] - SOUND_STEP_START + 1;
                m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
                m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 17);
                switch (pPed->GetSync().ped->m_nMoveState)
                {
					case PEDMOVE_WALK:
						Vol >>= 2;
						m_sQueueSample.m_nFrequency = 9 * m_sQueueSample.m_nFrequency / 10;
						break;
					case PEDMOVE_RUN:
						Vol >>= 1;
						m_sQueueSample.m_nFrequency = 11 * m_sQueueSample.m_nFrequency / 10;
						break;
					case PEDMOVE_SPRINT:
						m_sQueueSample.m_nFrequency = 12 * m_sQueueSample.m_nFrequency / 10;
						break;
					default:
						break;
                }
                m_sQueueSample.m_nPriority = 5;
                m_sQueueSample.m_fSpeedMultiplier = 0.0f;
                m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_STEP_MAX_DIST;
                m_sQueueSample.m_nLoopCount = 1;
                RESET_LOOP_OFFSETS
                    SET_EMITTING_VOLUME(Vol);
                m_sQueueSample.m_bIs2D = FALSE;
                m_sQueueSample.m_bStatic = TRUE;
                SET_SOUND_REFLECTION(TRUE);
                break;
            }
            case SOUND_FALL_LAND:
            case SOUND_FALL_COLLAPSE:
            case SOUND_49:
            {
                //if (ped->bIsInTheAir)
                //    continue;
                //maxDist = SQR(PED_ONE_SHOT_FALL_MAX_DIST);
                //Vol = m_anRandomTable[3] % 20 + PED_ONE_SHOT_FALL_VOLUME;
                //if (ped->m_nSurfaceTouched == SURFACE_WATER)
                //    m_sQueueSample.m_nSampleIndex = (m_anRandomTable[3] % 4) + SFX_FOOTSTEP_WATER_1;
                //else if (sound == SOUND_FALL_LAND)
                //    m_sQueueSample.m_nSampleIndex = SFX_BODY_LAND;
                //else
                //    m_sQueueSample.m_nSampleIndex = SFX_BODY_LAND_AND_FALL;
                //m_sQueueSample.m_nBankIndex = SFX_BANK_0;
                //m_sQueueSample.m_nCounter = 1;
                //m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
                //m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 17);
                //m_sQueueSample.m_nPriority = 2;
                //m_sQueueSample.m_fSpeedMultiplier = 0.0f;
                //m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_FALL_MAX_DIST;
                //m_sQueueSample.m_nLoopCount = 1;
                //RESET_LOOP_OFFSETS
                //    SET_EMITTING_VOLUME(Vol);
                //m_sQueueSample.m_bIs2D = FALSE;
                //m_sQueueSample.m_bStatic = TRUE;
                //SET_SOUND_REFLECTION(TRUE);
                break;
            }
            case SOUND_FIGHT_37:
                m_sQueueSample.m_nSampleIndex = SFX_FIGHT_1;
                m_sQueueSample.m_nFrequency = 18000;
                goto AddFightSound;
            case SOUND_FIGHT_38:
                m_sQueueSample.m_nSampleIndex = SFX_FIGHT_1;
                m_sQueueSample.m_nFrequency = 16500;
                goto AddFightSound;
            case SOUND_FIGHT_39:
                m_sQueueSample.m_nSampleIndex = SFX_FIGHT_1;
                m_sQueueSample.m_nFrequency = 20000;
                goto AddFightSound;
            case SOUND_FIGHT_40:
            case SOUND_187:
                m_sQueueSample.m_nSampleIndex = SFX_FIGHT_2;
                m_sQueueSample.m_nFrequency = 18000;
                goto AddFightSound;
            case SOUND_FIGHT_41:
                m_sQueueSample.m_nSampleIndex = SFX_FIGHT_2;
                m_sQueueSample.m_nFrequency = 16500;
                goto AddFightSound;
            case SOUND_FIGHT_42:
                m_sQueueSample.m_nSampleIndex = SFX_FIGHT_2;
                m_sQueueSample.m_nFrequency = 20000;
                goto AddFightSound;
            case SOUND_FIGHT_43:
                m_sQueueSample.m_nSampleIndex = SFX_FIGHT_4;
                m_sQueueSample.m_nFrequency = 18000;
                goto AddFightSound;
            case SOUND_FIGHT_44:
                m_sQueueSample.m_nSampleIndex = SFX_FIGHT_4;
                m_sQueueSample.m_nFrequency = 16500;
                goto AddFightSound;
            case SOUND_FIGHT_45:
                m_sQueueSample.m_nSampleIndex = SFX_FIGHT_4;
                m_sQueueSample.m_nFrequency = 20000;
                goto AddFightSound;
            case SOUND_FIGHT_46:
            case SOUND_188:
                m_sQueueSample.m_nSampleIndex = SFX_FIGHT_5;
                m_sQueueSample.m_nFrequency = 18000;
                goto AddFightSound;
            case SOUND_FIGHT_47:
                m_sQueueSample.m_nSampleIndex = SFX_FIGHT_5;
                m_sQueueSample.m_nFrequency = 16500;
                goto AddFightSound;
            case SOUND_FIGHT_48:
                m_sQueueSample.m_nSampleIndex = SFX_FIGHT_5;
                m_sQueueSample.m_nFrequency = 20000;
            AddFightSound:
            {
                    uint32 soundParams = m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_afVolume[i]; // wtf? storing int as float
                    uint8 damagerType = soundParams & 0xFF;
                    uint32 weaponType = soundParams >> 8;

                    if (damagerType == ENTITY_TYPE_PED)
                    {
                        if (weaponType == WEAPONTYPE_BRASSKNUCKLE)
                        {
                            /*
                            cPedMG* ped = params.m_pNetPed;
                            uint32 fightMove = ped->m_curFightMove;
                            if (fightMove == FIGHTMOVE_BACKLEFT || fightMove == FIGHTMOVE_STDPUNCH || fightMove == FIGHTMOVE_PUNCH ||
                                ped->m_nPedState == PED_ATTACK) {
                                CEntity* damageEntity = ped->m_pDamageEntity;
                                if (!damageEntity)
                                    m_sQueueSample.m_nSampleIndex = m_anRandomTable[3] % 2 + SFX_HAMMER_HIT_1;
                                else if (damageEntity->GetType() != ENTITY_TYPE_PED)
                                    m_sQueueSample.m_nSampleIndex = m_anRandomTable[3] % 2 + SFX_HAMMER_HIT_1;
                                else if (((CPed*)damageEntity)->m_curFightMove != FIGHTMOVE_HITHEAD)
                                    m_sQueueSample.m_nSampleIndex = m_anRandomTable[3] % 2 + SFX_HAMMER_HIT_1;
                                else
                                    m_sQueueSample.m_nSampleIndex = SFX_HAMMER_HIT_1;
                            }
                            */
                        }
                    }
                    else
                    {
                        m_sQueueSample.m_nSampleIndex = m_anRandomTable[4] % 6 + SFX_COL_CAR_PANEL_1;
                        m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
                        m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 16);
                    }
            }
                m_sQueueSample.m_nBankIndex = SFX_BANK_0;
                m_sQueueSample.m_nCounter = iSound;
                narrowSoundRange = TRUE;
                iSound++;
                m_sQueueSample.m_nPriority = 3;
                m_sQueueSample.m_fSpeedMultiplier = 0.0f;
                m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_PUNCH_MAX_DIST;
                maxDist = SQR(PED_ONE_SHOT_PUNCH_MAX_DIST);
                m_sQueueSample.m_nLoopCount = 1;
                RESET_LOOP_OFFSETS
                    Vol = m_anRandomTable[3] % 26 + PED_ONE_SHOT_PUNCH_VOLUME;
                SET_EMITTING_VOLUME(Vol);
                m_sQueueSample.m_bIs2D = FALSE;
                m_sQueueSample.m_bStatic = TRUE;
                SET_SOUND_REFLECTION(TRUE);
                break;
            case SOUND_WEAPON_BAT_ATTACK:
            case SOUND_WEAPON_KNIFE_ATTACK:
            {
                uint32 soundParams = m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_afVolume[i]; // wtf? storing int as float
                uint8 damagerType = soundParams & 0xFF;
                uint32 weaponType = soundParams >> 8;
                if (damagerType == ENTITY_TYPE_PED)
                {
                    switch (weaponType)
                    {
						case WEAPONTYPE_SCREWDRIVER:
						case WEAPONTYPE_KNIFE:
						case WEAPONTYPE_CLEAVER:
						case WEAPONTYPE_MACHETE:
						case WEAPONTYPE_KATANA:
							if (sound == SOUND_WEAPON_KNIFE_ATTACK)
								m_sQueueSample.m_nSampleIndex = SFX_KNIFE_SLASH;
							else
								m_sQueueSample.m_nSampleIndex = SFX_KNIFE_STAB;
							m_sQueueSample.m_nBankIndex = SFX_BANK_0;
							m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
							m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency >> 5);
							break;
						case WEAPONTYPE_HAMMER:
							m_sQueueSample.m_nSampleIndex = m_anRandomTable[3] % 2 + SFX_HAMMER_HIT_1;
							m_sQueueSample.m_nBankIndex = SFX_BANK_0;
							m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
							m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency >> 5);
							break;
						default:
							m_sQueueSample.m_nSampleIndex = SFX_BAT_HIT_LEFT;
							m_sQueueSample.m_nBankIndex = SFX_BANK_0;
							m_sQueueSample.m_nFrequency = RandomDisplacement(2000) + 22000;
							stereo = TRUE;
							break;
                    }
                }
                else
                {
                    m_sQueueSample.m_nSampleIndex = m_anRandomTable[4] % 4 + SFX_COL_CAR_PANEL_1;
                    m_sQueueSample.m_nBankIndex = SFX_BANK_0;
                    m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
                    m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency >> 4);
                }
                m_sQueueSample.m_nCounter = iSound++;
                narrowSoundRange = TRUE;
                m_sQueueSample.m_nPriority = 3;
                m_sQueueSample.m_fSpeedMultiplier = 0.0f;
                m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_PUNCH_MAX_DIST;
                maxDist = SQR(PED_ONE_SHOT_PUNCH_MAX_DIST);
                m_sQueueSample.m_nLoopCount = 1;
                RESET_LOOP_OFFSETS
                    Vol = m_anRandomTable[2] % 20 + PED_ONE_SHOT_PUNCH_VOLUME;
                SET_EMITTING_VOLUME(Vol);
                m_sQueueSample.m_bIs2D = FALSE;
                m_sQueueSample.m_bStatic = TRUE;
                SET_SOUND_REFLECTION(TRUE);
                break;
            }
            case SOUND_WEAPON_CHAINSAW_IDLE:
                //if (FindVehicleOfPlayer()) // TODO PSP if ( !cSampleManager::IsSampleBankLoaded(SampleManager, SFX_BANK_CAR_CHAINSAW) )
                //    continue;
                m_sQueueSample.m_nSampleIndex = SFX_CAR_CHAINSAW_IDLE;
#ifdef GTA_PS2
                m_sQueueSample.m_nBankIndex = SFX_BANK_CAR_CHAINSAW;
#else
                m_sQueueSample.m_nBankIndex = SFX_BANK_0;
#endif
                m_sQueueSample.m_nCounter = 70;
                m_sQueueSample.m_nFrequency = 27000;
                m_sQueueSample.m_nPriority = 3;
                m_sQueueSample.m_fSpeedMultiplier = 3.0f;
                m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_CHAINSAW_IDLE_MAX_DIST;
                maxDist = SQR(PED_ONE_SHOT_WEAPON_CHAINSAW_IDLE_MAX_DIST);
                m_sQueueSample.m_nLoopCount = 0;
                Vol = PED_ONE_SHOT_WEAPON_CHAINSAW_VOLUME;
                SET_LOOP_OFFSETS(SFX_CAR_CHAINSAW_IDLE)
                    SET_EMITTING_VOLUME(PED_ONE_SHOT_WEAPON_CHAINSAW_VOLUME);
                m_sQueueSample.m_bIs2D = FALSE;
                m_sQueueSample.m_bStatic = FALSE;
                m_sQueueSample.m_nFramesToPlay = 5;
                break;
            case SOUND_WEAPON_CHAINSAW_ATTACK:
				//if (FindVehicleOfPlayer()) // TODO PSP if ( !cSampleManager::IsSampleBankLoaded(SampleManager, SFX_BANK_CAR_CHAINSAW) )
				//    continue;
                m_sQueueSample.m_nSampleIndex = SFX_CAR_CHAINSAW_ATTACK;
#ifdef GTA_PS2
                m_sQueueSample.m_nBankIndex = SFX_BANK_CAR_CHAINSAW;
#else
                m_sQueueSample.m_nBankIndex = SFX_BANK_0;
#endif
                m_sQueueSample.m_nCounter = 68;
                m_sQueueSample.m_nFrequency = 27000;
                m_sQueueSample.m_nPriority = 2;
                m_sQueueSample.m_fSpeedMultiplier = 3.0f;
                m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_CHAINSAW_MAX_DIST;
                maxDist = SQR(PED_ONE_SHOT_WEAPON_CHAINSAW_MAX_DIST);
                m_sQueueSample.m_nLoopCount = 0;
                Vol = PED_ONE_SHOT_WEAPON_CHAINSAW_VOLUME;
                SET_LOOP_OFFSETS(SFX_CAR_CHAINSAW_ATTACK)
                    SET_EMITTING_VOLUME(PED_ONE_SHOT_WEAPON_CHAINSAW_VOLUME);
                m_sQueueSample.m_bIs2D = FALSE;
                m_sQueueSample.m_bStatic = FALSE;
                m_sQueueSample.m_nFramesToPlay = 5;
                break;
            case SOUND_WEAPON_CHAINSAW_MADECONTACT:
				//if (FindVehicleOfPlayer()) // TODO PSP if ( !cSampleManager::IsSampleBankLoaded(SampleManager, SFX_BANK_CAR_CHAINSAW) )
				//    continue;
				// fix bugs?
                //if ((int32)m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_afVolume[i] != ENTITY_TYPE_PED)
                //    ReportCollision(params.m_pNetPed, params.m_pNetPed, SURFACE_CAR, SURFACE_TARMAC, 0.0f, 0.09f);
                m_sQueueSample.m_nSampleIndex = SFX_CAR_CHAINSAW_ATTACK;
#ifdef GTA_PS2
                m_sQueueSample.m_nBankIndex = SFX_BANK_CAR_CHAINSAW;
#else
                m_sQueueSample.m_nBankIndex = SFX_BANK_0;
#endif
                m_sQueueSample.m_nCounter = 68;
                m_sQueueSample.m_nFrequency = RandomDisplacement(500) + 22000;
                m_sQueueSample.m_nPriority = 2;
                m_sQueueSample.m_fSpeedMultiplier = 3.0f;
                m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_CHAINSAW_MAX_DIST;
                maxDist = SQR(PED_ONE_SHOT_WEAPON_CHAINSAW_MAX_DIST);
                m_sQueueSample.m_nLoopCount = 0;
                Vol = PED_ONE_SHOT_WEAPON_CHAINSAW_VOLUME;
                SET_LOOP_OFFSETS(SFX_CAR_CHAINSAW_ATTACK)
                    SET_EMITTING_VOLUME(PED_ONE_SHOT_WEAPON_CHAINSAW_VOLUME);
                m_sQueueSample.m_bIs2D = FALSE;
                m_sQueueSample.m_bStatic = FALSE;
                m_sQueueSample.m_nFramesToPlay = 5;
                break;
            case SOUND_WEAPON_SHOT_FIRED:
				//eWeaponType weapon = ped->m_storedWeapon;
                //if (!weapon) // ptr leftower?
                //    continue;
                switch (ped->m_storedWeapon)
                {
					case WEAPONTYPE_PYTHON:
						m_sQueueSample.m_nSampleIndex = SFX_PYTHON_LEFT;
						m_sQueueSample.m_nBankIndex = SFX_BANK_0;
						m_sQueueSample.m_nCounter = iSound++;
						narrowSoundRange = TRUE;
						m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_PYTHON_LEFT);
						m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency >> 5);
						m_sQueueSample.m_nPriority = 3;
						m_sQueueSample.m_fSpeedMultiplier = 0.0f;
						m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST;
						maxDist = SQR(PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST);
						m_sQueueSample.m_nLoopCount = 1;
						RESET_LOOP_OFFSETS
							Vol = MAX_VOLUME;
						SET_EMITTING_VOLUME(Vol);
						m_sQueueSample.m_bIs2D = FALSE;
						m_sQueueSample.m_bStatic = TRUE;
						SET_SOUND_REFLECTION(TRUE);
						stereo = TRUE;
						break;
					case WEAPONTYPE_COLT45:
						m_sQueueSample.m_nSampleIndex = SFX_COLT45_LEFT;
						m_sQueueSample.m_nBankIndex = SFX_BANK_0;
						m_sQueueSample.m_nCounter = iSound++;
						narrowSoundRange = TRUE;
						m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_COLT45_LEFT);
						m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency >> 5);
						m_sQueueSample.m_nPriority = 3;
						m_sQueueSample.m_fSpeedMultiplier = 0.0f;
						m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST;
						maxDist = SQR(PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST);
						m_sQueueSample.m_nLoopCount = 1;
						RESET_LOOP_OFFSETS
							Vol = m_anRandomTable[1] % 10 + PED_ONE_SHOT_WEAPON_COLT45_VOLUME;
						SET_EMITTING_VOLUME(Vol);
						m_sQueueSample.m_bIs2D = FALSE;
						m_sQueueSample.m_bStatic = TRUE;
						SET_SOUND_REFLECTION(TRUE);
						stereo = TRUE;
						break;
					case WEAPONTYPE_ROCKET:
					case WEAPONTYPE_ROCKETLAUNCHER:
						m_sQueueSample.m_nSampleIndex = SFX_ROCKET_LEFT;
						m_sQueueSample.m_nBankIndex = SFX_BANK_0;
						m_sQueueSample.m_nCounter = iSound++;
						narrowSoundRange = TRUE;
						m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_ROCKET_LEFT);
						m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency >> 5);
						m_sQueueSample.m_nPriority = 1;
						m_sQueueSample.m_fSpeedMultiplier = 0.0f;
						m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST;
						maxDist = SQR(PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST);
						m_sQueueSample.m_nLoopCount = 1;
						RESET_LOOP_OFFSETS
							Vol = m_anRandomTable[0] % 20 + PED_ONE_SHOT_WEAPON_ROCKETLAUNCHER_VOLUME;
						SET_EMITTING_VOLUME(Vol);
						m_sQueueSample.m_bIs2D = FALSE;
						m_sQueueSample.m_bStatic = TRUE;
						SET_SOUND_REFLECTION(TRUE);
						stereo = TRUE;
						break;
					case WEAPONTYPE_FLAMETHROWER:
						m_sQueueSample.m_nSampleIndex = SFX_FLAMETHROWER_LEFT;
						m_sQueueSample.m_nBankIndex = SFX_BANK_0;
						m_sQueueSample.m_nCounter = 9;
						Vol = PED_ONE_SHOT_WEAPON_FLAMETHROWER_VOLUME;
						m_sQueueSample.m_nFrequency = (10 * m_sQueueSample.m_nEntityIndex % 2048) + SampleManager.GetSampleBaseFrequency(SFX_FLAMETHROWER_LEFT);
						m_sQueueSample.m_nPriority = 3;
						m_sQueueSample.m_fSpeedMultiplier = 4.0f;
						m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_FLAMETHROWER_MAX_DIST;
						maxDist = SQR(PED_ONE_SHOT_WEAPON_FLAMETHROWER_MAX_DIST);
						m_sQueueSample.m_nLoopCount = 0;
						SET_LOOP_OFFSETS(m_sQueueSample.m_nSampleIndex)
							SET_EMITTING_VOLUME(PED_ONE_SHOT_WEAPON_FLAMETHROWER_VOLUME);
						m_sQueueSample.m_bIs2D = FALSE;
						m_sQueueSample.m_bStatic = FALSE;
						m_sQueueSample.m_nFramesToPlay = 6;
						stereo = TRUE;
						break;
					case WEAPONTYPE_M60:
					case WEAPONTYPE_HELICANNON:
						m_sQueueSample.m_nSampleIndex = SFX_M60_LEFT;
						m_sQueueSample.m_nBankIndex = SFX_BANK_0;
						m_sQueueSample.m_nCounter = iSound++;
						narrowSoundRange = TRUE;
						m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_M60_LEFT);
						m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency >> 5);
						m_sQueueSample.m_nPriority = 3;
						m_sQueueSample.m_fSpeedMultiplier = 0.0f;
						m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST;
						maxDist = SQR(PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST);
						m_sQueueSample.m_nLoopCount = 1;
						RESET_LOOP_OFFSETS
							Vol = PED_ONE_SHOT_WEAPON_M16_VOLUME;
						SET_EMITTING_VOLUME(PED_ONE_SHOT_WEAPON_M16_VOLUME);
						m_sQueueSample.m_bIs2D = FALSE;
						m_sQueueSample.m_bStatic = TRUE;
						stereo = TRUE;
						break;
					case WEAPONTYPE_MP5:
						m_sQueueSample.m_nSampleIndex = SFX_MP5_LEFT;
						m_sQueueSample.m_nBankIndex = SFX_BANK_0;
						m_sQueueSample.m_nCounter = iSound++;
						narrowSoundRange = TRUE;
						m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_MP5_LEFT);
						m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency >> 5);
						m_sQueueSample.m_nPriority = 3;
						m_sQueueSample.m_fSpeedMultiplier = 0.0f;
						m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST;
						maxDist = SQR(PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST);
						m_sQueueSample.m_nLoopCount = 1;
						RESET_LOOP_OFFSETS
							Vol = m_anRandomTable[3] % 15 + PED_ONE_SHOT_WEAPON_UZI_VOLUME;
						SET_EMITTING_VOLUME(Vol);
						m_sQueueSample.m_bIs2D = FALSE;
						m_sQueueSample.m_bStatic = TRUE;
						stereo = TRUE;
						break;
					case WEAPONTYPE_TEC9:
						m_sQueueSample.m_nSampleIndex = SFX_TEC_LEFT;
						m_sQueueSample.m_nBankIndex = SFX_BANK_0;
						m_sQueueSample.m_nCounter = iSound++;
						narrowSoundRange = TRUE;
						m_sQueueSample.m_nFrequency = RandomDisplacement(500) + 17000;
						m_sQueueSample.m_nPriority = 3;
						m_sQueueSample.m_fSpeedMultiplier = 0.0f;
						m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST;
						maxDist = SQR(PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST);
						m_sQueueSample.m_nLoopCount = 1;
						RESET_LOOP_OFFSETS
							Vol = m_anRandomTable[3] % 15 + PED_ONE_SHOT_WEAPON_UZI_VOLUME;
						SET_EMITTING_VOLUME(Vol);
						m_sQueueSample.m_bIs2D = FALSE;
						m_sQueueSample.m_bStatic = TRUE;
						stereo = TRUE;
						break;
					case WEAPONTYPE_SILENCED_INGRAM:
						m_sQueueSample.m_nSampleIndex = SFX_TEC_LEFT;
						m_sQueueSample.m_nBankIndex = SFX_BANK_0;
						m_sQueueSample.m_nCounter = iSound++;
						narrowSoundRange = TRUE;
						m_sQueueSample.m_nFrequency = RandomDisplacement(1000) + 34000;
						m_sQueueSample.m_nPriority = 3;
						m_sQueueSample.m_fSpeedMultiplier = 0.0f;
						m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST;
						maxDist = SQR(PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST);
						m_sQueueSample.m_nLoopCount = 1;
						RESET_LOOP_OFFSETS
							Vol = m_anRandomTable[3] % 15 + PED_ONE_SHOT_WEAPON_UZI_VOLUME;
						SET_EMITTING_VOLUME(Vol);
						m_sQueueSample.m_bIs2D = FALSE;
						m_sQueueSample.m_bStatic = TRUE;
						stereo = TRUE;
						break;
					case WEAPONTYPE_RUGER:
						m_sQueueSample.m_nSampleIndex = SFX_RUGER_LEFT;
						m_sQueueSample.m_nBankIndex = SFX_BANK_0;
						m_sQueueSample.m_nCounter = iSound++;
						narrowSoundRange = TRUE;
						m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_RUGER_LEFT);
						m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency >> 5);
						m_sQueueSample.m_nPriority = 3;
						m_sQueueSample.m_fSpeedMultiplier = 0.0f;
						m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST;
						maxDist = SQR(PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST);
						m_sQueueSample.m_nLoopCount = 1;
						RESET_LOOP_OFFSETS
							Vol = m_anRandomTable[3] % 15 + PED_ONE_SHOT_WEAPON_M4_VOLUME;
						SET_EMITTING_VOLUME(Vol);
						m_sQueueSample.m_bIs2D = FALSE;
						m_sQueueSample.m_bStatic = TRUE;
						stereo = TRUE;
						break;
					case WEAPONTYPE_M4:
						m_sQueueSample.m_nSampleIndex = SFX_RUGER_LEFT;
						m_sQueueSample.m_nBankIndex = SFX_BANK_0;
						m_sQueueSample.m_nCounter = iSound++;
						narrowSoundRange = TRUE;
						m_sQueueSample.m_nFrequency = RandomDisplacement(1000) + 43150;
						m_sQueueSample.m_nPriority = 3;
						m_sQueueSample.m_fSpeedMultiplier = 0.0f;
						m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST;
						maxDist = SQR(PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST);
						m_sQueueSample.m_nLoopCount = 1;
						RESET_LOOP_OFFSETS
							Vol = m_anRandomTable[3] % 15 + PED_ONE_SHOT_WEAPON_M4_VOLUME;
						SET_EMITTING_VOLUME(Vol);
						m_sQueueSample.m_bIs2D = FALSE;
						m_sQueueSample.m_bStatic = TRUE;
						stereo = TRUE;
						break;
					case WEAPONTYPE_UZI:
					case WEAPONTYPE_MINIGUN:
						m_sQueueSample.m_nSampleIndex = SFX_UZI_LEFT;
						m_sQueueSample.m_nBankIndex = SFX_BANK_0;
						m_sQueueSample.m_nCounter = iSound++;
						narrowSoundRange = TRUE;
						m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_UZI_LEFT);
						m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency >> 5);
						m_sQueueSample.m_nPriority = 3;
						m_sQueueSample.m_fSpeedMultiplier = 0.0f;
						m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST;
						maxDist = SQR(PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST);
						m_sQueueSample.m_nLoopCount = 1;
						RESET_LOOP_OFFSETS
							Vol = m_anRandomTable[3] % 15 + PED_ONE_SHOT_WEAPON_UZI_VOLUME;
						SET_EMITTING_VOLUME(Vol);
						m_sQueueSample.m_bIs2D = FALSE;
						m_sQueueSample.m_bStatic = TRUE;
						stereo = TRUE;
						break;
					case WEAPONTYPE_SNIPERRIFLE:
					case WEAPONTYPE_LASERSCOPE:
						m_sQueueSample.m_nSampleIndex = SFX_SNIPER_LEFT;
						m_sQueueSample.m_nBankIndex = SFX_BANK_0;
						m_sQueueSample.m_nCounter = iSound++;
						narrowSoundRange = TRUE;
						if (ped->m_storedWeapon == WEAPONTYPE_SNIPERRIFLE)
							m_sQueueSample.m_nFrequency = 25472;
						else
							m_sQueueSample.m_nFrequency = 20182;
						m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency >> 5);
						m_sQueueSample.m_nPriority = 3;
						m_sQueueSample.m_fSpeedMultiplier = 0.0f;
						m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST;
						maxDist = SQR(PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST);
						m_sQueueSample.m_nLoopCount = 1;
						RESET_LOOP_OFFSETS
							Vol = m_anRandomTable[4] % 10 + PED_ONE_SHOT_WEAPON_SNIPERRIFLE_VOLUME;
						SET_EMITTING_VOLUME(Vol);
						m_sQueueSample.m_bIs2D = FALSE;
						m_sQueueSample.m_bStatic = TRUE;
						SET_SOUND_REFLECTION(TRUE);
						stereo = TRUE;
						break;
					case WEAPONTYPE_SPAS12_SHOTGUN:
						m_sQueueSample.m_nSampleIndex = SFX_SPAS12_LEFT;
						m_sQueueSample.m_nBankIndex = SFX_BANK_0;
						m_sQueueSample.m_nCounter = iSound++;
						narrowSoundRange = TRUE;
						m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_SPAS12_LEFT);
						m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency >> 5);
						m_sQueueSample.m_nPriority = 3;
						m_sQueueSample.m_fSpeedMultiplier = 0.0f;
						m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST;
						maxDist = SQR(PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST);
						m_sQueueSample.m_nLoopCount = 1;
						RESET_LOOP_OFFSETS
							Vol = m_anRandomTable[2] % 10 + PED_ONE_SHOT_WEAPON_SHOTGUN_VOLUME;
						SET_EMITTING_VOLUME(Vol);
						m_sQueueSample.m_bIs2D = FALSE;
						m_sQueueSample.m_bStatic = TRUE;
						SET_SOUND_REFLECTION(TRUE);
						stereo = TRUE;
						break;
					case WEAPONTYPE_SHOTGUN:
					case WEAPONTYPE_STUBBY_SHOTGUN:
						m_sQueueSample.m_nSampleIndex = SFX_SHOTGUN_LEFT;
						m_sQueueSample.m_nBankIndex = SFX_BANK_0;
						m_sQueueSample.m_nCounter = iSound++;
						narrowSoundRange = TRUE;
						m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_SHOTGUN_LEFT);
						m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency >> 5);
						m_sQueueSample.m_nPriority = 3;
						m_sQueueSample.m_fSpeedMultiplier = 0.0f;
						m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST;
						maxDist = SQR(PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST);
						m_sQueueSample.m_nLoopCount = 1;
						RESET_LOOP_OFFSETS
							Vol = m_anRandomTable[2] % 10 + PED_ONE_SHOT_WEAPON_SHOTGUN_VOLUME;
						SET_EMITTING_VOLUME(Vol);
						m_sQueueSample.m_bIs2D = FALSE;
						m_sQueueSample.m_bStatic = TRUE;
						SET_SOUND_REFLECTION(TRUE);
						stereo = FALSE;
						break;
					default:
						continue;
                }
                break;
            case SOUND_WEAPON_RELOAD:
                switch ((int32)m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_afVolume[i])
                {
                    case WEAPONTYPE_COLT45:
                    case WEAPONTYPE_PYTHON:
                        m_sQueueSample.m_nSampleIndex = SFX_PISTOL_RELOAD;
                        m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_PISTOL_RELOAD) + RandomDisplacement(300);
                        break;
                    case WEAPONTYPE_TEC9:
                    case WEAPONTYPE_UZI:
                    case WEAPONTYPE_SILENCED_INGRAM:
                    case WEAPONTYPE_MP5:
                    case WEAPONTYPE_M4:
                    case WEAPONTYPE_M60:
                    case WEAPONTYPE_HELICANNON:
                        m_sQueueSample.m_nSampleIndex = SFX_AK47_RELOAD;
                        m_sQueueSample.m_nFrequency = 39243;
                        break;
                    case WEAPONTYPE_SHOTGUN:
                    case WEAPONTYPE_SPAS12_SHOTGUN:
                    case WEAPONTYPE_STUBBY_SHOTGUN:
                    case WEAPONTYPE_RUGER:
                        m_sQueueSample.m_nSampleIndex = SFX_AK47_RELOAD;
                        m_sQueueSample.m_nFrequency = 30290;
                        break;
                    case WEAPONTYPE_ROCKET:
                    case WEAPONTYPE_ROCKETLAUNCHER:
                        m_sQueueSample.m_nSampleIndex = SFX_ROCKET_RELOAD;
                        m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_ROCKET_RELOAD);
                        break;
                    case WEAPONTYPE_SNIPERRIFLE:
                    case WEAPONTYPE_LASERSCOPE:
                        m_sQueueSample.m_nSampleIndex = SFX_RIFLE_RELOAD;
                        m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_RIFLE_RELOAD);
                        break;
                    default:
                        continue;
                }
                Vol = PED_ONE_SHOT_WEAPON_RELOAD_VOLUME;
                m_sQueueSample.m_nCounter = iSound++;
                narrowSoundRange = TRUE;
                m_sQueueSample.m_nFrequency += RandomDisplacement(300);
                m_sQueueSample.m_nBankIndex = SFX_BANK_0;
                m_sQueueSample.m_nPriority = 5;
                m_sQueueSample.m_fSpeedMultiplier = 0.0f;
                m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_RELOAD_MAX_DIST;
                maxDist = SQR(PED_ONE_SHOT_WEAPON_RELOAD_MAX_DIST);
                m_sQueueSample.m_nLoopCount = 1;
                RESET_LOOP_OFFSETS
                SET_EMITTING_VOLUME(PED_ONE_SHOT_WEAPON_RELOAD_VOLUME);
                m_sQueueSample.m_bIs2D = FALSE;
                m_sQueueSample.m_bStatic = TRUE;
                SET_SOUND_REFLECTION(TRUE);
                break;
     //       case SOUND_WEAPON_AK47_BULLET_ECHO:
     //       {
     //           uint32 weaponType = m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_afVolume[i];
     //           switch (weaponType)
     //           {
					//case WEAPONTYPE_SPAS12_SHOTGUN:
					//	m_sQueueSample.m_nSampleIndex = SFX_SPAS12_TAIL_LEFT;
					//	break;
					//case WEAPONTYPE_M60:
					//case WEAPONTYPE_HELICANNON:
					//	m_sQueueSample.m_nSampleIndex = SFX_SPAS12_TAIL_LEFT;
					//case WEAPONTYPE_UZI:
					//case WEAPONTYPE_MP5:
					//	m_sQueueSample.m_nSampleIndex = SFX_UZI_END_LEFT;
					//	break;
					//case WEAPONTYPE_TEC9:
					//case WEAPONTYPE_SILENCED_INGRAM:
					//	m_sQueueSample.m_nSampleIndex = SFX_TEC_TAIL;
					//	break;
					//case WEAPONTYPE_M4:
					//case WEAPONTYPE_RUGER:
					//case WEAPONTYPE_SNIPERRIFLE:
					//case WEAPONTYPE_LASERSCOPE:
					//	m_sQueueSample.m_nSampleIndex = SFX_RUGER_TAIL;
					//	break;
					//	break;
					//default:
					//	continue;
     //           }
     //           m_sQueueSample.m_nBankIndex = SFX_BANK_0;
     //           m_sQueueSample.m_nCounter = iSound++;
     //           narrowSoundRange = TRUE;
     //           switch (weaponType)
     //           {
					//case WEAPONTYPE_SILENCED_INGRAM:
					//	m_sQueueSample.m_nFrequency = 26000;
					//	break;
					//case WEAPONTYPE_TEC9:
					//	m_sQueueSample.m_nFrequency = 13000;
					//	break;
					//case WEAPONTYPE_M4:
					//	m_sQueueSample.m_nFrequency = 15600;
					//	break;
					//case WEAPONTYPE_LASERSCOPE:
					//	m_sQueueSample.m_nFrequency = 7904;
					//	break;
					//case WEAPONTYPE_SNIPERRIFLE:
					//	m_sQueueSample.m_nFrequency = 9959;
					//	break;
					//default:
					//	m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
					//	break;
     //           }
     //           m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency >> 4);
     //           m_sQueueSample.m_nPriority = 3;
     //           m_sQueueSample.m_fSpeedMultiplier = 0.0f;
     //           m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST;
     //           maxDist = SQR(PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST);
     //           m_sQueueSample.m_nLoopCount = 1;
     //           RESET_LOOP_OFFSETS
     //               Vol = m_anRandomTable[4] % 10 + PED_ONE_SHOT_WEAPON_BULLET_ECHO_VOLUME;
     //           SET_EMITTING_VOLUME(Vol);
     //           m_sQueueSample.m_bIs2D = FALSE;
     //           m_sQueueSample.m_bStatic = TRUE;
     //           SET_SOUND_REFLECTION(TRUE);
     //           break;
     //       }
     //       case SOUND_WEAPON_FLAMETHROWER_FIRE:
     //           m_sQueueSample.m_nSampleIndex = SFX_FLAMETHROWER_START_LEFT;
     //           m_sQueueSample.m_nBankIndex = SFX_BANK_0;
     //           m_sQueueSample.m_nCounter = iSound++;
     //           m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_FLAMETHROWER_START_LEFT);
     //           m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency >> 4);
     //           m_sQueueSample.m_nPriority = 3;
     //           m_sQueueSample.m_fSpeedMultiplier = 4.0f;
     //           m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_FLAMETHROWER_FIRE_MAX_DIST;
     //           maxDist = SQR(PED_ONE_SHOT_WEAPON_FLAMETHROWER_FIRE_MAX_DIST);
     //           m_sQueueSample.m_nLoopCount = 1;
     //           RESET_LOOP_OFFSETS
     //               Vol = PED_ONE_SHOT_WEAPON_FLAMETHROWER_FIRE_VOLUME;
     //           SET_EMITTING_VOLUME(PED_ONE_SHOT_WEAPON_FLAMETHROWER_FIRE_VOLUME);
     //           m_sQueueSample.m_bIs2D = FALSE;
     //           m_sQueueSample.m_bStatic = TRUE;
     //           break;
     //       case SOUND_WEAPON_HIT_PED:
     //           m_sQueueSample.m_nSampleIndex = SFX_BULLET_PED;
     //           m_sQueueSample.m_nBankIndex = SFX_BANK_0;
     //           m_sQueueSample.m_nCounter = iSound++;
     //           narrowSoundRange = TRUE;
     //           m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_BULLET_PED);
     //           m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency >> 3);
     //           m_sQueueSample.m_nPriority = 7;
     //           m_sQueueSample.m_fSpeedMultiplier = 0.0f;
     //           m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_HIT_PED_MAX_DIST;
     //           maxDist = SQR(PED_ONE_SHOT_WEAPON_HIT_PED_MAX_DIST);
     //           m_sQueueSample.m_nLoopCount = 1;
     //           RESET_LOOP_OFFSETS
     //               Vol = m_anRandomTable[0] % 20 + PED_ONE_SHOT_WEAPON_HIT_PED_VOLUME;
     //           SET_EMITTING_VOLUME(Vol);
     //           m_sQueueSample.m_bIs2D = FALSE;
     //           m_sQueueSample.m_bStatic = TRUE;
     //           break;
     //       case SOUND_SPLASH:
     //           if (m_FrameCounter <= iSplashFrame)
     //               continue;
     //           iSplashFrame = m_FrameCounter + 6;
     //           m_sQueueSample.m_nSampleIndex = SFX_SPLASH_1;
     //           m_sQueueSample.m_nBankIndex = SFX_BANK_0;
     //           m_sQueueSample.m_nCounter = iSound++;
     //           narrowSoundRange = TRUE;
     //           m_sQueueSample.m_nFrequency = RandomDisplacement(1400) + 20000;
     //           m_sQueueSample.m_nPriority = 1;
     //           m_sQueueSample.m_fSpeedMultiplier = 0.0f;
     //           m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_SPLASH_MAX_DIST;
     //           maxDist = SQR(PED_ONE_SHOT_SPLASH_MAX_DIST);
     //           m_sQueueSample.m_nLoopCount = 1;
     //           RESET_LOOP_OFFSETS
     //               Vol = m_anRandomTable[2] % 30 + PED_ONE_SHOT_SPLASH_PED_VOLUME;
     //           SET_EMITTING_VOLUME(Vol);
     //           m_sQueueSample.m_bIs2D = FALSE;
     //           m_sQueueSample.m_bStatic = TRUE;
     //           SET_SOUND_REFLECTION(TRUE);
     //           break;
     //       case SOUND_FRONTEND_HURRICANE: // VCS TODO!!

     //           break;
     //       case SOUND_MELEE_ATTACK_START:
     //       {
     //           uint32 weaponType = ((uint32)m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_afVolume[i]) >> 8;
     //           switch (weaponType)
     //           {
     //           case WEAPONTYPE_SCREWDRIVER:
     //           case WEAPONTYPE_KNIFE:
     //           case WEAPONTYPE_CLEAVER:
     //           case WEAPONTYPE_MACHETE:
     //           case WEAPONTYPE_KATANA:
     //               m_sQueueSample.m_nSampleIndex = SFX_KNIFE_SWING;
     //               break;
     //           default:
     //               m_sQueueSample.m_nSampleIndex = SFX_GOLF_CLUB_SWING;
     //               break;
     //           }
     //           m_sQueueSample.m_nBankIndex = SFX_BANK_0;
     //           m_sQueueSample.m_nCounter = iSound++;
     //           narrowSoundRange = TRUE;
     //           m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
     //           m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency >> 4);
     //           m_sQueueSample.m_nPriority = 3;
     //           m_sQueueSample.m_fSpeedMultiplier = 0.0f;
     //           m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_HIT_PED_MAX_DIST;
     //           if (weaponType == WEAPONTYPE_UNARMED || weaponType == WEAPONTYPE_BRASSKNUCKLE)
     //               Vol = m_anRandomTable[1] % 10 + 35;
     //           else
     //               Vol = m_anRandomTable[2] % 20 + 70;
     //           maxDist = SQR(PED_ONE_SHOT_WEAPON_HIT_PED_MAX_DIST);
     //           m_sQueueSample.m_nLoopCount = 1;
     //           RESET_LOOP_OFFSETS
     //               SET_EMITTING_VOLUME(Vol);
     //           m_sQueueSample.m_bIs2D = FALSE;
     //           m_sQueueSample.m_bStatic = TRUE;
     //           SET_SOUND_REFLECTION(TRUE);
     //           break;
     //       }
     //       case SOUND_SKATING:
     //       {
     //           uint32 soundParams = m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_afVolume[i];
     //           uint8 param1 = soundParams & 0xFF;
     //           uint32 param2 = soundParams >> 8;
     //           m_sQueueSample.m_nSampleIndex = m_anRandomTable[3] % 2 + SFX_SKATE_1;
     //           m_sQueueSample.m_nBankIndex = SFX_BANK_0;
     //           m_sQueueSample.m_nCounter = iSound;
     //           stereo = TRUE;
     //           iSound++;
     //           m_sQueueSample.m_nFrequency = m_anRandomTable[1] % 1000 + 17000;
     //           if (param2 == 0)
     //               m_sQueueSample.m_nFrequency = (3 * m_sQueueSample.m_nFrequency) >> 2;
     //           m_sQueueSample.m_nPriority = 6;
     //           m_sQueueSample.m_fSpeedMultiplier = 3.0f;
     //           m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_SKATING_MAX_DIST;
     //           maxDist = SQR(PED_ONE_SHOT_SKATING_MAX_DIST);
     //           m_sQueueSample.m_nLoopCount = 1;
     //           RESET_LOOP_OFFSETS
     //               Vol = (m_anRandomTable[2] % 20 + PED_ONE_SHOT_SKATING_VOLUME) * param1 / MAX_VOLUME;
     //           SET_EMITTING_VOLUME(Vol);
     //           m_sQueueSample.m_bIs2D = FALSE;
     //           m_sQueueSample.m_bStatic = TRUE;
     //           SET_SOUND_REFLECTION(TRUE);
     //           break;
     //       }
     //       case SOUND_WEAPON_MINIGUN_ATTACK:
     //           m_sQueueSample.m_nSampleIndex = SFX_MINIGUN_FIRE_RIGHT;
     //           m_sQueueSample.m_nBankIndex = SFX_BANK_0;
     //           m_sQueueSample.m_nCounter = 68;
     //           m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_MINIGUN_FIRE_RIGHT);
     //           m_sQueueSample.m_nPriority = 2;
     //           m_sQueueSample.m_fSpeedMultiplier = 3.0f;
     //           m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_MINIGUN_MAX_DIST;
     //           Vol = PED_ONE_SHOT_MINIGUN_VOLUME;
     //           maxDist = SQR(PED_ONE_SHOT_MINIGUN_MAX_DIST);
     //           m_sQueueSample.m_nLoopCount = 0;
     //           SET_LOOP_OFFSETS(SFX_MINIGUN_FIRE_RIGHT)
     //           SET_EMITTING_VOLUME(PED_ONE_SHOT_MINIGUN_VOLUME);
     //           m_sQueueSample.m_bIs2D = FALSE;
     //           m_sQueueSample.m_bStatic = FALSE;
     //           m_sQueueSample.m_nFramesToPlay = 3;
     //           break;
     //       case SOUND_WEAPON_MINIGUN_2:
     //           m_sQueueSample.m_nSampleIndex = SFX_MINIGUN_FIRE_LEFT;
     //           m_sQueueSample.m_nBankIndex = SFX_BANK_0;
     //           m_sQueueSample.m_nCounter = 69;
     //           m_sQueueSample.m_nFrequency = 18569;
     //           m_sQueueSample.m_nPriority = 2;
     //           m_sQueueSample.m_fSpeedMultiplier = 3.0f;
     //           m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_MINIGUN_MAX_DIST;
     //           Vol = (float)PED_ONE_SHOT_MINIGUN_VOLUME * m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_afVolume[i];
     //           maxDist = SQR(PED_ONE_SHOT_MINIGUN_MAX_DIST);
     //           m_sQueueSample.m_nLoopCount = 0;
     //           SET_LOOP_OFFSETS(SFX_MINIGUN_FIRE_LEFT)
     //           SET_EMITTING_VOLUME(Vol);
     //           m_sQueueSample.m_bIs2D = FALSE;
     //           m_sQueueSample.m_bStatic = FALSE;
     //           m_sQueueSample.m_nFramesToPlay = 3;
     //           break;
     //       case SOUND_WEAPON_MINIGUN_3:
     //           m_sQueueSample.m_nSampleIndex = SFX_MINIGUN_STOP;
     //           m_sQueueSample.m_nBankIndex = SFX_BANK_0;
     //           m_sQueueSample.m_nCounter = 69;
     //           m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_MINIGUN_STOP);
     //           m_sQueueSample.m_nPriority = 4;
     //           m_sQueueSample.m_fSpeedMultiplier = 0.0f;
     //           m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_MINIGUN_MAX_DIST;
     //           maxDist = SQR(PED_ONE_SHOT_MINIGUN_MAX_DIST);
     //           m_sQueueSample.m_nLoopCount = 1;
     //           RESET_LOOP_OFFSETS
     //           Vol = PED_ONE_SHOT_MINIGUN_VOLUME;
     //           SET_EMITTING_VOLUME(PED_ONE_SHOT_MINIGUN_VOLUME);
     //           m_sQueueSample.m_bIs2D = FALSE;
     //           m_sQueueSample.m_bStatic = TRUE;
     //           SET_SOUND_REFLECTION(TRUE);
     //           break;
     //       case SOUND_SHIRT_WIND_FLAP:
     //       {
     //           if (params.m_pPed->IsPlayer() && params.m_pPed->m_pMyVehicle)
     //           {
     //               if (m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_afVolume[i] > 0.0f)
     //               {
     //                   if (m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_afVolume[i] > 1.0f)
     //                       m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_afVolume[i] = 1.0f;

     //                   Vol = (float)PED_ONE_SHOT_SHIRT_FLAP_VOLUME * m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_afVolume[i];

     //                   switch (params.m_pPed->m_pMyVehicle->GetModelIndex())
     //                   {
     //                   case MI_ANGEL:
     //                   case MI_FREEWAY:
     //                       m_sQueueSample.m_nSampleIndex = SFX_CAR_WIND_17;
     //                       break;
     //                   case MI_PCJ600:
     //                       m_sQueueSample.m_nSampleIndex = SFX_CAR_WIND_20;
     //                       break;
     //                   case MI_SANCHEZ:
     //                       m_sQueueSample.m_nSampleIndex = SFX_CAR_WIND_19;
     //                       break;
     //                   case MI_PIZZABOY:
     //                   case MI_FAGGIO:
     //                       m_sQueueSample.m_nSampleIndex = SFX_CAR_WIND_18;
     //                       break;
     //                   default:
     //                       continue;
     //                   };

     //                   m_sQueueSample.m_nBankIndex = SFX_BANK_0;
     //                   m_sQueueSample.m_nCounter = 71;
     //                   m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
     //                   m_sQueueSample.m_nPriority = 3;
     //                   m_sQueueSample.m_fSpeedMultiplier = 3.0f;
     //                   m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_SHIRT_FLAP_MAX_DIST;
     //                   maxDist = SQR(PED_ONE_SHOT_SHIRT_FLAP_MAX_DIST);
     //                   m_sQueueSample.m_nLoopCount = 0;
     //                   SET_LOOP_OFFSETS(m_sQueueSample.m_nSampleIndex)
     //                       SET_EMITTING_VOLUME(Vol);
     //                   m_sQueueSample.m_bIs2D = FALSE;
     //                   m_sQueueSample.m_bStatic = FALSE;
     //                   m_sQueueSample.m_nFramesToPlay = 3;
     //               }
     //           }
     //           continue;
     //       }
     //       default:
     //           SetupPedComments(params, sound);
     //           continue;
        }

        if (narrowSoundRange && iSound > 60)
            iSound = 21;
        if (params.m_fDistance < maxDist)
        {
            CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
            m_sQueueSample.m_nVolume = ComputeVolume(Vol, m_sQueueSample.m_MaxDistance, m_sQueueSample.m_fDistance);
            if (m_sQueueSample.m_nVolume > 0)
            {
                if (stereo)
                {
                    if (m_sQueueSample.m_fDistance < 0.2f * m_sQueueSample.m_MaxDistance)
                    {
                        m_sQueueSample.m_bIs2D = TRUE;
                        m_sQueueSample.m_nPan = 0;
                    }
                    else
                        stereo = FALSE;
                }
                SET_SOUND_REVERB(TRUE);
                AddSampleToRequestedQueue();
                if (stereo)
                {
                    m_sQueueSample.m_nPan = 127;
                    m_sQueueSample.m_nSampleIndex++;
                    if (m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_awAudioEvent[i] == SOUND_WEAPON_SHOT_FIRED &&
                        ped->m_storedWeapon == WEAPONTYPE_FLAMETHROWER)
                        m_sQueueSample.m_nCounter++;
                    else
                    {
                        m_sQueueSample.m_nCounter = iSound++;
                        if (iSound > 60)
                            iSound = 21;
                    }
                    AddSampleToRequestedQueue();
                }
            }
        }
    }
}
