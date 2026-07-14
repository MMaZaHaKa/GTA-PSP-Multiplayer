/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once
#include "multiplayer/MultiGame.h"
#include "multiplayer/net/packet.h"
#include "multiplayer/natives/public.h"
#include "multiplayer/events/public.h"
#include "multiplayer/elements/sVehicle.h"

#include "common.h"
#include "Ped.h"
#include "PlayerPed.h"
#include "PlayerInfo.h"
#include "Vehicle.h"
#include "Pad.h"
#include "World.h"
#include "Particle.h"
#include "DMAudio.h"
#include "Vehicle.h"
#include "RpAnimBlend.h"


void cMultiGame::OnPlayerKill(net::pckt_player_kill& packet, int sender, uint16 time, bool bFromRing) // ID 3
{
	lua_State* vm = cLWrapper::Instance().m_luaVM;
	lua_pushstring(vm, "RegisterPlayerKill");
	lua_gettable(vm, LUA_GLOBALSINDEX);
	if (!lua_isnil(vm, -1)) {
		lsn_push_player_id(vm, sender); // who got killed
		lsn_push_player_id(vm, packet.assassin);
		lsc_call(vm, 2, false);
	}
}

void on_recv_target_player(net::pckt_target_player& packet, int sender, uint16 time, bool bFromRing) // ID 20
{
	cMultiGame& Game = cMultiGame::Instance();
	Game.SetTargetPlayer(packet.player, false);
}

// its LCS logic for VCS for now, unfinished vcs fighting
void on_recv_fight_hit_ped(net::pckt_fight_hit_ped& packet, int sender, uint16 time, bool bFromRing) // ID 27
{
	extern FightMove tFightMoves[NUM_FIGHTMOVES];
	cMultiGame& Game = cMultiGame::Instance();
	sElement* pVictimElem = Game.GetEntityForHandle(Game.LocalPlayerID(), packet.victim_id);
	if (!pVictimElem)
		return;

	CEntity* pVictimEntity = pVictimElem->GetEntity();
	if (!pVictimEntity)
		return;

	CPed* pVictimPed = (CPed*)pVictimEntity;

	sPed* pAttacker = (sPed*)Game.GetEntityForHandle(sender, packet.attacker_id);
	if (!pAttacker)
		return;

	CPed* pAttackerPhy = (CPed*)pAttacker->GetPhysical();

	CWeaponInfo* pWepInfo = CWeaponInfo::GetWeaponInfo((eWeaponType)packet.weapon);
	if (!pWepInfo) return;

	// CPed::FightHitPed
	bool fightingWithWeapon = (pWepInfo->m_Flags & WEAPONFLAG_FIGHTMODE) != 0;
	//CVector bloodPos = packet.blood_pos;
	CVector bloodPos = packet.move_end_pos; // recheck

	if (pWepInfo->m_AnimToPlay == ASSOCGRP_KNIFE && packet.cur_fight_move >= FIGHTMOVE_MELEE1 && pVictimEntity->GetIsOnScreen())
	{
		//CVector dir = packet.dir; // LCS
		CVector dir = (packet.move_end_pos - packet.move_pos); // recheck

		static float particleRightLen = 0.05f;
		static float particleUpLen = 0.05f;

		// Just for particles. We will restore it below.
		dir /= (20.0f * dir.Magnitude());
		if (packet.cur_fight_move == FIGHTMOVE_MELEE1)
		{
			float rightMult = -particleRightLen;
			dir += particleUpLen * pAttackerPhy->GetUp() + rightMult * pAttackerPhy->GetRight();
		}
		else if (packet.cur_fight_move == FIGHTMOVE_MELEE2)
		{
			float upMult = 2.0f * particleUpLen;
			dir += particleRightLen * pAttackerPhy->GetRight() + upMult * pAttackerPhy->GetUp();
		}
		CParticle::AddParticle(PARTICLE_BLOOD_SPURT, bloodPos, dir, nil, 0.0f, 0, 0, 0, 0);
		CParticle::AddParticle(PARTICLE_BLOOD_SPURT, bloodPos, dir, nil, 0.0f, 0, 0, 0, 0);
		if (pAttacker->GetID() <= eElementID::MG_ELEMENT_PLAYER_PED_ID)
		{
			CParticle::AddParticle(PARTICLE_BLOOD_SPURT, bloodPos, dir, nil, 0.0f, 0, 0, 0, 0);
			CParticle::AddParticle(PARTICLE_BLOOD_SPURT, bloodPos, dir, nil, 0.0f, 0, 0, 0, 0);
		}
		if (!(CGeneral::GetRandomNumber() & 3))
		{
			CParticle::AddParticle(PARTICLE_TEST, bloodPos, dir, nil, 0.0f, 0, 0, 0, 0);
		}
	}
	else if ((tFightMoves[packet.cur_fight_move].hitLevel > HITLEVEL_MEDIUM || fightingWithWeapon) && pVictimEntity->GetIsOnScreen())
	{
		//CVector dir = packet.dir; // LCS
		CVector dir = (packet.move_end_pos - packet.move_pos); // recheck

		// Just for particles. We will restore it below.
		dir /= (10.0f * dir.Magnitude());
		for (int i = 0; i < 4; i++)
		{
			CParticle::AddParticle(PARTICLE_BLOOD, bloodPos, dir, nil, 0.0f, 0, 0, 0, 0);
		}
	}

	bool brassKnucklePunch = (packet.weapon == WEAPONTYPE_BRASSKNUCKLE);
	pVictimPed->ReactToAttack(pAttackerPhy);

	if (packet.weapon != WEAPONTYPE_UNARMED) {
		if(!pVictimPed->IsPlayer())
			pVictimPed->StartFightDefend(packet.local_dir, tFightMoves[packet.cur_fight_move].hitLevel, 0);
	}
	else {
		pVictimPed->StartFightDefend(packet.local_dir, tFightMoves[packet.cur_fight_move].hitLevel, 0);
	}

	if (!pVictimPed->DyingOrDead())
	{
		//if (fightingWithWeapon)
		//	pVictimPed->InflictDamage(this, GetWeapon()->m_eWeaponType, damageMult, (ePedPieceTypes)piece, direction);
		//else
			//pVictimPed->InflictDamage(pAttackerPhy, (eWeaponType)packet.weapon, packet.damage_mult, (ePedPieceTypes)packet.ped_piece, packet.local_dir);
			pVictimPed->InflictDamage(pAttackerPhy, WEAPONTYPE_UNARMED, packet.damage_mult, (ePedPieceTypes)packet.ped_piece, packet.local_dir); // not packet.weapon?
	}

	if (tFightMoves[packet.cur_fight_move].hitLevel > HITLEVEL_MEDIUM && pVictimPed->m_nPedState == PED_DIE)
	{
		//CVector dir = packet.dir; // LCS
		CVector dir = (packet.move_end_pos - packet.move_pos); // recheck

		// Just for particles. We will restore it below.
		dir /= (10.0f * dir.Magnitude());
		for (int i = 0; i < 4; i++)
		{
			CParticle::AddParticle(PARTICLE_BLOOD, bloodPos, dir, nil, 0.0f, 0, 0, 0, 0);
		}
	}

	float oldVictimHealth = pVictimPed->m_fHealth;

	if (!fightingWithWeapon)
	{
		if (!pVictimPed->OnGround())
		{
			float curVictimHealth = pVictimPed->m_fHealth;
			if (curVictimHealth > 0.0f && (curVictimHealth < 30.0f && oldVictimHealth > 30.0f ||
				packet.weapon != WEAPONTYPE_UNARMED && packet.weapon != WEAPONTYPE_BRASSKNUCKLE && pVictimPed->IsPlayer() ||
					pVictimPed->m_pedStats->m_flags & STAT_ONE_HIT_KNOCKDOWN || brassKnucklePunch))
			{

				pVictimPed->SetFall(0, (AnimationId)(packet.local_dir + ANIM_STD_HIGHIMPACT_FRONT), 0);
				if (pVictimPed->m_nPedState == PED_FALL)
					pVictimPed->bIsStanding = false;
			}
		}
	}

	if (pVictimPed->m_nPedState == PED_DIE || !pVictimPed->bIsStanding)
	{
		CVector dir = (pVictimPed->GetPosition() - pAttacker->GetSync().ped->GetMatrix().GetPosition()); // recheck
		dir.Normalise();
		dir.z = 1.0f;
		pVictimPed->bIsStanding = false;

		float moveMult;
		if (fightingWithWeapon)
		{
			moveMult = Min(packet.damage_mult * 0.02f, 1.0f);
		}
		else if (packet.cur_fight_move == FIGHTMOVE_GROUNDKICK)
		{
			moveMult = Min(packet.damage_mult * 0.6f, 4.0f);
		}
		else
		{
			if (pVictimPed->m_nPedState != PED_DIE || packet.damage_mult >= 20)
			{
				moveMult = packet.damage_mult;
			}
			else
			{
				moveMult = Min(packet.damage_mult * 2.0f, 14.0f);
			}
		}

		pVictimPed->ApplyMoveForce(moveMult * 0.6 * dir);
	}

	enum {
		S37 = SOUND_FIGHT_37,
		S38 = SOUND_FIGHT_38,
		S39 = SOUND_FIGHT_39,
		S40 = SOUND_FIGHT_40,
		S41 = SOUND_FIGHT_41,
		S42 = SOUND_FIGHT_42,
		S43 = SOUND_FIGHT_43,
		S44 = SOUND_FIGHT_44,
		S45 = SOUND_FIGHT_45,
		S46 = SOUND_FIGHT_46,
		S47 = SOUND_FIGHT_47,
		S48 = SOUND_FIGHT_48,
		NO_SND = SOUND_NO_SOUND
	};
	const uint16 hitSoundsByFightMoves[17][10] = {
	  { S37, S46, S41, S41, S46, S46, S40, S41, S43, S40 },
	  { NO_SND, NO_SND, NO_SND, NO_SND, NO_SND, NO_SND, NO_SND, NO_SND, NO_SND, NO_SND },
	  { NO_SND, NO_SND, NO_SND, NO_SND, NO_SND, NO_SND, NO_SND, NO_SND, NO_SND, NO_SND },
	  { S46, S46, S46, S46, S37, S47, S37, S38, S43, S38 },
	  { S46, S46, S46, S46, S46, S46, S40, S41, S43, S46 },
	  { S46, S46, S46, S46, S46, S46, S40, S41, S43, S40 },
	  { S46, S46, S46, S46, S46, S46, S40, S41, S43, S40 },
	  { S46, S46, S37, S46, S37, S47, S40, S47, S43, S37 },
	  { S46, S46, S46, S46, S46, S46, S43, S44, S43, S43 },
	  { S37, S46, S46, S46, S38, S47, S40, S38, S43, S46 },
	  { S46, S37, S46, S37, S39, S46, S40, S39, S43, S37 },
	  { S46, S37, S46, S46, S38, S47, S40, S38, S43, S46 },
	  { S37, S37, S46, S46, S38, S47, S48, S38, S43, S37 },
	  { S46, S46, S46, S46, S37, S46, S40, S38, S43, S46 },
	  { S46, S46, S46, S37, S39, S46, S40, S39, S43, S46 },
	  { S37, S46, S46, S46, S37, S46, S40, S37, S43, S46 },
	  { S43, S43, S43, S43, S43, S43, S43, S43, S43, S43 }
	};

	if (pWepInfo->m_AnimToPlay == ASSOCGRP_KNIFE) {
		if (packet.cur_fight_move >= FIGHTMOVE_MELEE1) {
			if (packet.cur_fight_move == FIGHTMOVE_MELEE3) {
				DMAudio.PlayOneShot(pAttackerPhy->m_audioEntityId, SOUND_WEAPON_BAT_ATTACK, (packet.weapon << 8) | ENTITY_TYPE_PED);
			}
			else {
				DMAudio.PlayOneShot(pAttackerPhy->m_audioEntityId, SOUND_WEAPON_KNIFE_ATTACK, (packet.weapon << 8) | ENTITY_TYPE_PED);
			}
			return;
		}
	}
	// This is why first dimension is between FightMove 1 and 17.
	else if (packet.cur_fight_move > FIGHTMOVE_NULL || packet.cur_fight_move < FIGHTMOVE_HITFRONT)
	{
		uint16 nHitSound;
		if (pVictimPed->m_nPedState == PED_DEAD || pVictimPed->UseGroundColModel()) {
			nHitSound = hitSoundsByFightMoves[packet.cur_fight_move][FIGHTMOVE_HITFRONT - FIGHTMOVE_HITFRONT];
		}
		else {
			nHitSound = hitSoundsByFightMoves[packet.cur_fight_move][FIGHTMOVE_HITONFLOOR - FIGHTMOVE_HITFRONT];
		}

		if (nHitSound != SOUND_NO_SOUND)
		{
			DMAudio.SetEntityStatus(pAttackerPhy->m_audioEntityId, TRUE);
			DMAudio.PlayOneShot(pAttackerPhy->m_audioEntityId, nHitSound, 1.0f);
		}
	}
}

void on_recv_shot_ped(net::pckt_shot_ped& packet, int sender, uint16 time, bool bFromRing) // ID 28
{
	cMultiGame& Game = cMultiGame::Instance();
	sPed* pPed = (sPed*)Game.GetEntityForHandle(Game.LocalPlayerID(), packet.victim_id);
	CPed* pVictimPed = pPed ? (CPed*)pPed->GetEntity() : nil;
	sPed* pShooterPed = (sPed*)Game.GetEntityForHandle(sender, packet.shooter_id);
	if (!pVictimPed || !pShooterPed) return;

	pVictimPed->ReactToAttack(pShooterPed->GetPhysical());
	//CWeaponInfo* info = CWeaponInfo::GetWeaponInfo((eWeaponType)packet.weapon); // damage

	// CWeapon::DoBulletImpact
	if (!pVictimPed->IsPedInControl() || pVictimPed->bIsDucking)
	{
		pVictimPed->InflictDamage(pPed->GetPhysical(), (eWeaponType)packet.weapon, packet.damage_mult, (ePedPieceTypes)packet.ped_piece, packet.local_dir);
	}
	else
	{
		if (pVictimPed->bCanBeShotInVehicle && (CWeapon::IsShotgun(packet.weapon) ||
			(!pVictimPed->IsPlayer() && (packet.weapon == WEAPONTYPE_HELICANNON || packet.weapon == WEAPONTYPE_M60 || packet.weapon == WEAPONTYPE_PYTHON))))
		{
			CVector2D posOffset = CVector2D(packet.pos_offset.x, packet.pos_offset.y);
			posOffset.Normalise();
			pVictimPed->bIsStanding = false;

			pVictimPed->ApplyMoveForce(posOffset.x * -5.0f, posOffset.y * -5.0f, 5.0f);
			pVictimPed->SetFall(1500, AnimationId(ANIM_STD_HIGHIMPACT_FRONT + packet.local_dir), false);

			pVictimPed->InflictDamage(pPed->GetPhysical(), (eWeaponType)packet.weapon, packet.damage_mult, (ePedPieceTypes)packet.ped_piece, packet.local_dir);
		}
		else
		{
			if (pVictimPed->IsPlayer())
			{
				CPlayerPed* victimPlayer = (CPlayerPed*)pVictimPed;
				bool bAttack = false;
				TODO();
				TODO();
				TODO();
				if ((/*pVictimPed->m_weapons[pVictimPed->m_currentWeapon] != 0xFFFFFA8C*/ true // recheck ?WEAPONTYPE_TEARGAS?, todo
					&& (pVictimPed->m_weapons[pVictimPed->m_currentWeapon].m_eWeaponType == WEAPONTYPE_GRENADE
						|| pVictimPed->m_weapons[pVictimPed->m_currentWeapon].m_eWeaponType == WEAPONTYPE_DETONATOR_GRENADE
						|| pVictimPed->m_weapons[pVictimPed->m_currentWeapon].m_eWeaponType == WEAPONTYPE_MOLOTOV
						|| pVictimPed->m_weapons[pVictimPed->m_currentWeapon].m_eWeaponType == WEAPONTYPE_LANDMINE)
					&&
						(RpAnimBlendClumpGetAssociation(pVictimPed->GetClump(), ANIM_ATTACK_1) ||
							RpAnimBlendClumpGetAssociation(pVictimPed->GetClump(), ANIM_ATTACK_2) ||
							RpAnimBlendClumpGetAssociation(pVictimPed->GetClump(), ANIM_ATTACK_3) ||
							RpAnimBlendClumpGetAssociation(pVictimPed->GetClump(), ANIM_ATTACK_EXTRA1) ||
							RpAnimBlendClumpGetAssociation(pVictimPed->GetClump(), ANIM_ATTACK_EXTRA2)
							)) )
				{
					bAttack = true;
				}

				if (!bAttack && victimPlayer->m_nHitAnimDelayTimer < CTimer::GetTimeInMilliseconds())
				{
					pVictimPed->ClearAttackByRemovingAnim();

					if (!TheAnimManager->IsAnimPlaying(pVictimPed->GetClump(), AnimationId(ANIM_STD_HITBYGUN_FRONT + packet.local_dir)))
					{
						CAnimBlendAssociation* pAnim = TheAnimManager->AddAnimation(pVictimPed->GetClump(), ASSOCGRP_STD, AnimationId(ANIM_STD_HITBYGUN_FRONT + packet.local_dir));
						pAnim->blendAmount = 0.0f;
						pAnim->blendDelta = 8.0f;
					}

					if (packet.weapon == WEAPONTYPE_M4)
						victimPlayer->m_nHitAnimDelayTimer = CTimer::GetTimeInMilliseconds() + 2500;
					else
						victimPlayer->m_nHitAnimDelayTimer = CTimer::GetTimeInMilliseconds() + 1000;
				}
			}
			else
			{
				pVictimPed->ClearAttackByRemovingAnim();

				if (!TheAnimManager->IsAnimPlaying(pVictimPed->GetClump(), AnimationId(ANIM_STD_HITBYGUN_FRONT + packet.local_dir)))
				{
					CAnimBlendAssociation* pAnim = TheAnimManager->AddAnimation(pVictimPed->GetClump(), ASSOCGRP_STD, AnimationId(ANIM_STD_HITBYGUN_FRONT + packet.local_dir));
					pAnim->blendAmount = 0.0f;
					pAnim->blendDelta = 8.0f;
				}
			}

			pVictimPed->InflictDamage(pPed->GetPhysical(), (eWeaponType)packet.weapon, packet.damage_mult, (ePedPieceTypes)packet.ped_piece, packet.local_dir);
		}
	}

	if (CGame::nastyGame)
	{
		uint8 bloodAmount = 8;
		if (CWeapon::IsShotgun(packet.weapon) || packet.weapon == WEAPONTYPE_HELICANNON)
			bloodAmount = 32;

		CVector hitPoint = packet.point;
		CVector localPos = pVictimPed->GetPosition();
		CVector dir = (hitPoint - localPos) * 0.01f;
		dir.z = 0.01f;

		if (pVictimPed->GetIsOnScreen())
		{
			for (uint8 i = 0; i < bloodAmount; i++)
				CParticle::AddParticle(PARTICLE_BLOOD_SMALL, hitPoint, dir);
		}

		if (packet.weapon == WEAPONTYPE_MINIGUN)
		{
			CParticle::AddParticle(PARTICLE_TEST, hitPoint, CVector(0.f, 0.f, 0.f), nil, 0.f, 0, 0, 0, 0);
			CParticle::AddParticle(PARTICLE_TEST, hitPoint + CVector(0.2f, -0.2f, 0.f), CVector(0.f, 0.f, 0.f), nil, 0.f, 0, 0, 0, 0);
			CParticle::AddParticle(PARTICLE_TEST, hitPoint + CVector(-0.2f, 0.2f, 0.f), CVector(0.f, 0.f, 0.f), nil, 0.f, 0, 0, 0, 0);
		}
	}
}

void on_recv_shot_ped_from_car(net::pckt_shot_ped_from_car& packet, int sender, uint16 time, bool bFromRing) // ID 35
{
	cMultiGame& Game = cMultiGame::Instance();
	sPed* pPed = (sPed*)Game.GetEntityForHandle(Game.LocalPlayerID(), packet.victim_id);
	CPed* pVictimPed = pPed ? (CPed*)pPed->GetEntity() : nil;
	if (!pVictimPed) return;

	sVehicle* pVehicle = (sVehicle*)Game.GetEntityForHandle(sender, packet.shooter_id);
	if (!pVehicle) return;

	pVictimPed->ReactToAttack(FindPlayerPed());

	// FireOneInstantHitRound
	if (pVictimPed->CanHitReact() && (pVictimPed->m_pMyVehicle == nil || !pVictimPed->m_pMyVehicle->IsBike())) // VCS +Bmx?
	{
		pVictimPed->ClearAttackByRemovingAnim();
		if (!TheAnimManager->IsAnimPlaying(pVictimPed->GetClump(), AnimationId(ANIM_STD_HITBYGUN_FRONT + packet.direction)))
		{
			CAnimBlendAssociation* pAnim = TheAnimManager->AddAnimation(pVictimPed->GetClump(), ASSOCGRP_STD, AnimationId(ANIM_STD_HITBYGUN_FRONT + packet.direction));
			pAnim->blendAmount = 0.0f;
			pAnim->blendDelta = 8.0f;
		}
	}

	if (pVehicle->GetSync().vehicle->bIsDrivenByPlayer)
	{
		sPed* pPed = (sPed*)cMultiGame::Instance().GetEntityForHandle(sender, eElementID::MG_ELEMENT_PLAYER_PED_ID); // cMultiGame::GetPlayerPed
		assert(pPed && pPed->GetPhysical());
		pVictimPed->InflictDamage(pPed->GetPhysical(), WEAPONTYPE_UZI_DRIVEBY, packet.damage, (ePedPieceTypes)packet.ped_piece, packet.direction);
	}

	if (pVictimPed->GetIsOnScreen())
	{
		if (CGame::nastyGame)
		{
			for (int32 i = 0; i < 4; i++)
			{
				CVector hitPos = packet.pos;
				CVector dir;
				dir.x = CGeneral::GetRandomNumberInRange(-0.1f, 0.1f);
				dir.y = CGeneral::GetRandomNumberInRange(-0.1f, 0.1f);
				dir.z = CGeneral::GetRandomNumberInRange(-0.1f, 0.1f);

				CParticle::AddParticle(PARTICLE_BLOOD, hitPos, dir);
			}
		}
	}
}

void on_recv_kill_player_ped(net::pckt_kill_player_ped& packet, int sender, uint16 time, bool bFromRing) // ID 37
{
	if (packet.player_id == cMultiGame::Instance().LocalPlayerID())
		FindPlayerPed()->InflictDamage(nil, eWeaponType::WEAPONTYPE_FALL, 1000.0f, ePedPieceTypes::PEDPIECE_TORSO, 0);
}

void on_recv_melee(net::pckt_melee& packet, int sender, uint16 time, bool bFromRing) // ID 39
{
	debug("multi melee hit\n");

	cMultiGame& Game = cMultiGame::Instance();
	sElement* pVictimElem = Game.GetEntityForHandle(Game.LocalPlayerID(), packet.victim_id);
	if (!pVictimElem) return;
	CEntity* pVictimEntity = pVictimElem->GetEntity();

	if (pVictimEntity && pVictimEntity->IsPed())
	{
		CPed* pVictimPed = (CPed*)pVictimEntity;
		if (!pVictimPed) // useless double check
			return; // skip veh block bellow?

		sPed* pShooterPed = (sPed*)Game.GetEntityForHandle(sender, packet.shooter_id);
		if (!pShooterPed)
			return; // skip veh block bellow?

		eWeaponType nWeaponType = (eWeaponType)packet.weapon;
		CVector bloodPos = packet.blood_pos;
		CVector collisionDist = packet.collision_dist;

		bool isHeavy = packet.bIsHeavy;
		bool anim2Playing = packet.bAnim2Playing;
		CPed* shooterPed = (CPed*)pShooterPed->GetPhysical();

		if (!pVictimPed->DyingOrDead())
			pVictimPed->ReactToAttack(pShooterPed->GetPhysical());

#ifdef GTA_LIBERTY
		uint8 hitLevel = HITLEVEL_HIGH;
		if (isHeavy && (pVictimPed->OnGround() || pVictimPed->m_nWaitState == WAITSTATE_SUN_BATHE_IDLE))
			hitLevel = HITLEVEL_GROUND;
		pVictimPed->StartFightDefend(localDir, hitLevel, 10);
#endif

		DMAudio.PlayOneShot(pVictimPed->m_audioEntityId, SOUND_WEAPON_BAT_ATTACK, 1.0f);
		CWeaponInfo* info = CWeaponInfo::GetWeaponInfo(nWeaponType);

		if (!pVictimPed->DyingOrDead())
		{
			//if (shooterPed->IsPlayer() && isHeavy && anim2Playing)
			//	pVictimPed->InflictDamage(shooterPed, nWeaponType, 100.0f, PEDPIECE_TORSO, localDir);
			//else if (shooterPed->IsPlayer() && ((CPlayerPed*)shooterPed)->bAdrenalineActive)
			//	pVictimPed->InflictDamage(shooterPed, nWeaponType, 3.5f * info->m_nDamage, PEDPIECE_TORSO, localDir);
			//else
			//{
			//	if (pVictimPed->IsPlayer() && isHeavy) // wtf, it's not fair
			//		pVictimPed->InflictDamage(shooterPed, nWeaponType, 2.0f * info->m_nDamage, PEDPIECE_TORSO, localDir);
			//	else
			//		pVictimPed->InflictDamage(shooterPed, nWeaponType, info->m_nDamage, PEDPIECE_TORSO, localDir);
			//}

			pVictimPed->InflictDamage(shooterPed, nWeaponType, packet.damage, PEDPIECE_TORSO, packet.localDir);
		}

		if (CGame::nastyGame && pVictimPed->GetIsOnScreen())
		{
			CVector dir = collisionDist * RecipSqrt(1.0f, 10.0f * collisionDist.MagnitudeSqr());

			CParticle::AddParticle(PARTICLE_BLOOD, bloodPos, dir);
			CParticle::AddParticle(PARTICLE_BLOOD, bloodPos, dir);
			CParticle::AddParticle(PARTICLE_BLOOD, bloodPos, dir);

			if (isHeavy)
			{
				dir.x += CGeneral::GetRandomNumberInRange(-0.05f, 0.05f);
				dir.y += CGeneral::GetRandomNumberInRange(-0.05f, 0.05f);
				CParticle::AddParticle(PARTICLE_BLOOD, bloodPos, dir);

				dir.x += CGeneral::GetRandomNumberInRange(-0.05f, 0.05f);
				dir.y += CGeneral::GetRandomNumberInRange(-0.05f, 0.05f);
				CParticle::AddParticle(PARTICLE_BLOOD, bloodPos, dir);
			}

			if (nWeaponType == WEAPONTYPE_CHAINSAW)
			{
				if (pVictimPed->m_nPedState != PED_DEAD && !((CTimer::GetFrameCounter() + 17) & 1) || pVictimPed->m_nPedState == PED_DEAD && !((CTimer::GetFrameCounter() + 17) & 3))
				{
					CParticle::AddParticle(PARTICLE_TEST, bloodPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.2f);
				}
				CVector newDir(dir);
				newDir.z += 0.2f;
				CParticle::AddParticle(PARTICLE_BLOOD_SMALL, bloodPos, newDir);
				CParticle::AddParticle(PARTICLE_BLOOD, bloodPos, newDir);
				newDir.z = dir.z + 0.1f;
				CParticle::AddParticle(PARTICLE_BLOOD, bloodPos, newDir);
				newDir.x = 0.0f;
				newDir.y = 0.0f;
				newDir.z = 0.01f;
				CParticle::AddParticle(PARTICLE_DEBRIS2, bloodPos, newDir);

				CVector dropDir(CGeneral::GetRandomNumberInRange(-0.15f, 0.15f), CGeneral::GetRandomNumberInRange(0.1f, 0.35f), 0.f);
				CVector dropPos(CGeneral::GetRandomNumberInRange(SCREEN_STRETCH_X(50.0f), SCREEN_STRETCH_FROM_RIGHT(50.0f)),
					CGeneral::GetRandomNumberInRange(SCREEN_STRETCH_Y(50.0f), SCREEN_STRETCH_FROM_BOTTOM(50.0f)), 1.f);
				/*CParticle::AddParticle(PARTICLE_BLOODDROP, dropPos, dropDir, nil, CGeneral::GetRandomNumberInRange(0.1f, 0.15f),
					CRGBA(0, 0, 0, 0), 0, 0, CGeneral::GetRandomNumber() & 1, 0);
				*/
			}
			if (info->m_AnimToPlay == ASSOCGRP_KNIFE)
			{
				dir += 0.1f * shooterPed->GetUp() + 0.05f * shooterPed->GetRight();
				CParticle::AddParticle(PARTICLE_BLOOD_SPURT, bloodPos, dir);
				CParticle::AddParticle(PARTICLE_BLOOD_SPURT, bloodPos, dir);
				CParticle::AddParticle(PARTICLE_BLOOD_SPURT, bloodPos, dir);
			}
		}

		CVector2D posOffset(shooterPed->GetPosition().x - pVictimPed->GetPosition().x, shooterPed->GetPosition().y - pVictimPed->GetPosition().y);
		if (!pVictimPed->OnGround())
		{
			if (pVictimPed->m_fHealth > 0.0f && (pVictimPed->m_fHealth < 30.0f && pVictimPed->m_fHealth > 30.0f ||
				(isHeavy || nWeaponType == WEAPONTYPE_BRASSKNUCKLE) && !pVictimPed->IsPlayer()))
			{
				posOffset.Normalise();
				pVictimPed->bIsStanding = false;
				if (nWeaponType == WEAPONTYPE_CHAINSAW)
					pVictimPed->ApplyMoveForce(posOffset.x * -2.0f, posOffset.y * -2.0f, 2.0f);
				else
					pVictimPed->ApplyMoveForce(posOffset.x * -5.0f, posOffset.y * -5.0f, 3.0f);

				if (isHeavy && pVictimPed->IsPlayer())
					pVictimPed->SetFall(3000, AnimationId(ANIM_STD_HIGHIMPACT_FRONT + packet.localDir), false);
				else
					pVictimPed->SetFall(1500, AnimationId(ANIM_STD_HIGHIMPACT_FRONT + packet.localDir), false);

				//shooterPed->m_pSeekTarget = pVictimPed;
				//shooterPed->m_pSeekTarget->RegisterReference(&shooterPed->m_pSeekTarget);
			}
		}
		else if (pVictimPed->Dying() && !anim2Playing)
		{
			posOffset.Normalise();
			pVictimPed->bIsStanding = false;
			if (nWeaponType == WEAPONTYPE_CHAINSAW)
				pVictimPed->ApplyMoveForce(posOffset.x * -1.0f, posOffset.y * -1.0f, 1.0f);
			else
				pVictimPed->ApplyMoveForce(posOffset.x * -5.0f, posOffset.y * -5.0f, 3.0f);
		}
	}
#ifdef FIX_BUGS
	else if (pVictimEntity && pVictimEntity->IsVehicle())
#else
	else if (pVictimEntity->IsVehicle())
#endif
	{
		// stupid block, use pVictimEntity
		sElement* pElem = Game.GetEntityForHandle(Game.LocalPlayerID(), packet.victim_id); // why?? pVictimEntity
		if (!pElem) return;

		CVehicle* pVictimVeh = (CVehicle*)pElem->GetEntity();
		if (!pVictimVeh)
			return;

		eWeaponType nWeaponType = (eWeaponType)packet.weapon;

#ifdef GTA_LIBERTY
		if (!pVictimVeh->IsCar()) // non bike?
			return;
#else
		if (!pVictimVeh->IsCar() && !pVictimVeh->IsBike() && !pVictimVeh->IsBoat()) // what about quadbike, bmx?
			return;
#endif

		float oldHealth = pVictimVeh->m_fHealth;
		//CWeaponInfo* info = CWeaponInfo::GetWeaponInfo(nWeaponType);
		pVictimVeh->VehicleDamage(packet.damage * (0.01f * pVictimVeh->m_pHandling->fMass), packet.pieceB);

		if (pVictimVeh->m_fHealth < oldHealth)
		{
			pVictimVeh->m_nLastWeaponToDamage = nWeaponType;
			//pVictimVeh->m_pLastDamageEntity = shooterPed;
		}
		//if (shooterPed->m_fDamageImpulse == 0.0f)
		//{
		//	shooterPed->m_pDamageEntity = nearCar;
		//	nearCar->RegisterReference(&shooterPed->m_pDamageEntity);
		//}
		//damageEntityRegistered = 2;
		//if (FindPlayerPed()->GetWeapon() == this && nearCar->VehicleCreatedBy != MISSION_VEHICLE)
		//{
			if (pVictimVeh->AutoPilot.m_nDrivingStyle != DRIVINGSTYLE_PLOUGH_THROUGH &&
				(CGeneral::GetRandomTrueFalse() || pVictimVeh->AutoPilot.m_nCarMission != MISSION_CRUISE))
			{
				int leaveCarDelay = 200;
				CPed* driver = pVictimVeh->pDriver;
				if (driver && driver->CharCreatedBy != MISSION_CHAR)
				{
					if (driver->m_pedStats->m_temper <= driver->m_pedStats->m_fear)
					{
						driver->SetObjective(OBJECTIVE_FLEE_ON_FOOT_TILL_SAFE);
					}
					else
					{
						driver->SetObjective(OBJECTIVE_KILL_CHAR_ON_FOOT, FindPlayerPed());
						driver->m_objectiveTimer = CTimer::GetTimeInMilliseconds() + 10000;
						driver->m_prevObjective = OBJECTIVE_KILL_CHAR_ON_FOOT;
					}
					driver->m_leaveCarTimer = CTimer::GetTimeInMilliseconds() + 200;
					leaveCarDelay = 400;
				}
				for (int32 j = 0; j < pVictimVeh->m_nNumMaxPassengers; ++j)
				{
					CPed* passenger = pVictimVeh->GetPassenger(j);
					if (passenger && passenger->CharCreatedBy != MISSION_CHAR)
					{
						passenger->SetObjective(OBJECTIVE_FLEE_ON_FOOT_TILL_SAFE);
						passenger->m_leaveCarTimer = CTimer::GetTimeInMilliseconds() + leaveCarDelay;
						leaveCarDelay += 200;
					}
				}
			}
			else
			{
				CPed* driver = pVictimVeh->pDriver;
				if (driver)
				{
					if (driver->m_objective != OBJECTIVE_LEAVE_CAR && driver->m_objective != OBJECTIVE_KILL_CHAR_ON_FOOT &&
						driver->m_objective != OBJECTIVE_FLEE_ON_FOOT_TILL_SAFE)
					{
						if (pVictimVeh->AutoPilot.m_nDrivingStyle != DRIVINGSTYLE_PLOUGH_THROUGH)
							pVictimVeh->AutoPilot.m_nCruiseSpeed = pVictimVeh->AutoPilot.m_nCruiseSpeed * 1.5f;

						pVictimVeh->AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_PLOUGH_THROUGH;
					}
				}
			}
		//}
	}
}

void on_recv_player_been_hit(net::pckt_player_been_hit& packet, int sender, uint16 time, bool bFromRing) // ID 40
{
	debug("you hit player %d: damage %d\n", packet.player, packet.impact);
	sPlayer* pPlayer = cMultiGame::Instance().GetPlayer(packet.player);
	if (!pPlayer) return;
	CVector pos = pPlayer->GetPosition();
	CParticle::AddParticle(PARTICLE_MULTIPLAYER_HIT, pos, CVector(0.0f, 0.0f, 0.03f));
}

void on_recv_player_control(net::pckt_player_control& packet, int sender, uint16 time, bool bFromRing) // ID 41
{
	CPad* pPad = CPad::GetPad(0);

	if (!packet.player_control_toggle_type) {
		if (packet.player_control_toggle_value)
			pPad->DisablePlayerControls &= ~PLAYERCONTROL_PLAYERINFO; // enable control
		else
			pPad->DisablePlayerControls |= PLAYERCONTROL_PLAYERINFO; // disable control
	}
	else {
		pPad->field_9C = !packet.player_control_toggle_value;
	}
}

void on_recv_player_set_position(net::pckt_set_position& packet, int sender, uint16 time, bool bFromRing) // ID 42
{
	CPlayerPed* pPed = FindPlayerPed();
	CVector pos = packet.pos;
	pos.z = CWorld::FindGroundZFor3DCoord(pos.x, pos.y, pos.z, nil) + 1.0f;
#ifdef GTA_LIBERTY
	if (pPed->bInVehicle && pPed->m_pMyVehicle)
#else
	if (pPed->InVehicle() && pPed->m_pMyVehicle)
#endif
		pPed->m_pMyVehicle->SetPosition(pos);
	else
		pPed->Teleport(pos);
}


void on_recv_player_set_heading(net::pckt_set_heading& packet, int sender, uint16 time, bool bFromRing) // ID 43
{
	CPlayerPed* pPed = FindPlayerPed();
	const float fRot = DEGTORAD(packet.heading);
#ifdef GTA_LIBERTY
	if (pPed->bInVehicle && pPed->m_pMyVehicle)
#else
	if (pPed->InVehicle() && pPed->m_pMyVehicle)
#endif
		pPed->m_pMyVehicle->SetHeading(fRot);
	else {
		pPed->m_fRotationCur = fRot;
		pPed->m_fRotationDest = fRot;
		pPed->SetHeading(fRot);
	}
}

void on_recv_set_player_blip_visible_state(net::pckt_set_player_blip_visible_state& packet, int sender, uint16 time, bool bFromRing) // ID 48
{
	cMultiGame& Game = cMultiGame::Instance();
	if (packet.player_id != Game.LocalPlayerID()) return;
	CPlayerPed* pPed = FindPlayerPed();
#ifdef GTA_LIBERTY
	if (packet.visible)
#if defined(GTA_LIBERTY) && !defined(FIX_BUGS)
		pPed->m_nPowerups = 0x0; // what? // check mp_lsn_RemoveLocalPlayerNoRadarForEnemy
#else
		pPed->bNoRadarForEnemy = false;
#endif
	else
		pPed->bNoRadarForEnemy = true; // lcs 0x20
#else
	if (packet.visible)
		pPed->bNoRadarForEnemy = false;
	else
		pPed->bNoRadarForEnemy = true;
#endif
}

void on_recv_player_respawn(net::pckt_player_respawn& packet, int sender, uint16 time, bool bFromRing) // ID 49
{
	cMultiGame& Game = cMultiGame::Instance();
	Game.SetPlayerSpawned(sender);
	sPed* pPed = Game.GetPlayerPed(sender);
	if (pPed) pPed->Respawn();
}

