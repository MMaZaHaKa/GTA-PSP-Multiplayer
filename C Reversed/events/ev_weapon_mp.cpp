/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once
#include "multiplayer/MultiGame.h"
#include "multiplayer/net/packet.h"

#include "common.h"

#include "multiplayer/elements/sElementPhysical.h"
#include "multiplayer/elements/sElement.h"
#include "multiplayer/elements/sVehicle.h"
#include "multiplayer/elements/sPickup.h"
#include "multiplayer/events/public.h"
#include "PointLights.h"
#include "SpecialFX.h"
#include "Weapon.h"
#include "ProjectileInfo.h"
#include "BulletInfo.h"
#include "Explosion.h"
#include "Projectile.h"
#include "Particle.h"
#include "Weapon.h"
#include "ShotInfo.h"
#include "DMAudio.h"


void on_recv_start_fire(net::pckt_start_fire& packet, int sender, uint16 time, bool bFromRing) // ID 0
{
	debug("starting fire\n");
	cMultiGame& Game = cMultiGame::Instance();
	if (packet.entity == -1 && packet.source == -1) {
		gFireManager.StartFire(packet.pos, packet.strength, packet.propagation, false);
	}
	else {
		sPed* entityOnFire = (packet.entity != -1) ? (sPed*)Game.GetEntityForHandle(sender, packet.entity) : nil;
		if (entityOnFire) {
			sPed* fleeFrom = (packet.source != -1) ? (sPed*)Game.GetEntityForHandle(sender, packet.source) : nil;
			gFireManager.StartFire(entityOnFire->GetPhysical(), fleeFrom ? fleeFrom->GetPhysical() : nil, packet.strength, packet.propagation, false);
		}
	}
}

void on_recv_fire_instant_hit(net::pckt_fire_instant_hit& packet, int sender, uint16 time, bool bFromRing) // ID 29
{
	cMultiGame& Game = cMultiGame::Instance();
	sElementPhysical* pElem = (sElementPhysical*)Game.GetEntityForHandle(sender, packet.shooter_id);
	cPhysicalMG* pPhyMG = pElem != nil ? pElem->GetPhysical() : nil;
	if (pPhyMG != nil) {
		DMAudio.PlayOneShot(pPhyMG->m_audioEntityId, SOUND_WEAPON_SHOT_FIRED, 0.0f);
		DMAudio.SetEntityStatus(pPhyMG->m_audioEntityId, TRUE);
	}
	sElementPhysical* pElemShooter = (sElementPhysical*)Game.GetEntityForHandle(sender, packet.shooter_id);
	if (pElemShooter != nil) { // stupid twice call GetEntityForHandle
		CVector vSource = packet.fire_source;
		CVector vTarget = packet.target;
		CVector2D vMoveSpeed = CVector2D(packet.move_speed.x, packet.move_speed.y);
		FireInstantHitParticles(
			(eWeaponType)packet.weapon_type,
			&vSource,
			&vTarget,
			packet.firing_rate,
			pElemShooter->GetPhysical(),
			&vMoveSpeed,
			packet.changed_heading,
			packet.shooter_moving
		);
	}
}

void on_recv_fire_sniper(net::pckt_fire_sniper& packet, int sender, uint16 time, bool bFromRing) // ID 30
{
	cMultiGame& Game = cMultiGame::Instance();
	sElementPhysical* pElem = (sElementPhysical*)Game.GetEntityForHandle(sender, packet.shooter_id);
	cPhysicalMG* pPhyMG = pElem != nil ? pElem->GetPhysical() : nil;
	if (pPhyMG != nil) {
		DMAudio.PlayOneShot(pPhyMG->m_audioEntityId, SOUND_WEAPON_SHOT_FIRED, 0.0f);
		DMAudio.SetEntityStatus(pPhyMG->m_audioEntityId, TRUE);
	}
	sPlayer* pPlayer = Game.GetPlayer(sender);
	bool bHasQuadDamage = false;
	if (pPlayer)
		bHasQuadDamage = (pPlayer->GetSync().player->m_nPickups & ePowerupType::QUAD_DAMAGE);
	sElementPhysical* pElemShooter = (sElementPhysical*)Game.GetEntityForHandle(sender, packet.shooter_id);
	if (pElemShooter != nil) { // stupid twice call GetEntityForHandle, + no physical nil check ?
		CBulletInfo::AddBullet(pElemShooter->GetPhysical(), (eWeaponType)packet.weapon_type, packet.fire_source, packet.target, bHasQuadDamage);
	}

	CVector vTarget = packet.target;
	CWeaponInfo* weaponInfo = CWeaponInfo::GetWeaponInfo(WEAPONTYPE_SNIPERRIFLE);
	CVector vSource = packet.fire_source + (packet.target * 0.1f);
	assert(pPhyMG);
	CVector2D vMoveSpeed2D = CVector2D(pPhyMG->m_vecMoveSpeed.x, pPhyMG->m_vecMoveSpeed.y);

	FireInstantHitParticles(
		eWeaponType::WEAPONTYPE_SNIPERRIFLE,
		&vSource,
		&vTarget,
		weaponInfo->m_nFiringRate,
		pPhyMG,
		&vMoveSpeed2D,
		false,
		false);
}

void on_recv_fire_shotgun(net::pckt_fire_shotgun& packet, int sender, uint16 time, bool bFromRing) // ID 31
{
	cMultiGame& Game = cMultiGame::Instance();
	sElementPhysical* pElem = (sElementPhysical*)Game.GetEntityForHandle(sender, packet.shooter_id);
	cPhysicalMG* pPhyMG = pElem != nil ? pElem->GetPhysical() : nil;
	if (pPhyMG != nil) {
		DMAudio.PlayOneShot(pPhyMG->m_audioEntityId, SOUND_WEAPON_SHOT_FIRED, 0.0f);
		DMAudio.SetEntityStatus(pPhyMG->m_audioEntityId, TRUE);
	}
	sPlayer* pPlayer = Game.GetPlayer(sender);
	bool bHasQuadDamage = false;
	if (pPlayer)
		bHasQuadDamage = (pPlayer->GetSync().player->m_nPickups & ePowerupType::QUAD_DAMAGE);
	sElementPhysical* pElemShooter = (sElementPhysical*)Game.GetEntityForHandle(sender, packet.shooter_id);
	if (pElemShooter != nil && pElemShooter->GetPhysical() != nil) // stupid twice call GetEntityForHandle  if (pElem != nil && pPhyMG != nil)
	{
		CVector ppos = packet.pos;
		CVector* pPosVec = packet.has_attacker ? &ppos : nil;
		CVector fireSource = packet.fire_source;
		CWeapon::DoShotgunFire((eWeaponType)packet.weapon_type, pPhyMG, &fireSource, bHasQuadDamage, pPosVec, packet.angle);
	}
}

void on_recv_fire_projectile(net::pckt_fire_projectile& packet, int sender, uint16 time, bool bFromRing) // ID 32
{
	cMultiGame& Game = cMultiGame::Instance();
	CMatrix matrix;
	matrix.SetTranslate(packet.matr_pos);
	matrix.GetForward() = packet.matr_forward;
	matrix.GetRight() = packet.matr_right;
	matrix.GetUp() = packet.matr_up;
	sPlayer* pPlayer = Game.GetPlayer(sender);
	bool bHasQuadDamage = false;
	if (pPlayer)
		bHasQuadDamage = (pPlayer->GetSync().player->m_nPickups & ePowerupType::QUAD_DAMAGE);
	sElementPhysical* pElemShooter = (sElementPhysical*)Game.GetEntityForHandle(sender, packet.shooter_id);
	if (pElemShooter != nil && pElemShooter->GetPhysical() != nil) {
		CVector vecVelocity = packet.velocity;
		int32 nProj = CProjectileInfo::CreateProjectile((eWeaponType)packet.weapon_type, pElemShooter->GetPhysical(), matrix, vecVelocity, packet.gravity,
			CTimer::GetTimeInMilliseconds() + packet.time, packet.special_collision_response, bHasQuadDamage, packet.elasticity, sender);
		if (nProj != -1) {
			CProjectile* pEntry = CProjectileInfo::ms_apProjectile[nProj];
			CVector forward = matrix.GetForward();
			CVector prod;
			prod.x = forward.x * vecVelocity.x;
			prod.y = forward.y * vecVelocity.y;
			prod.z = forward.z * vecVelocity.z;
			pEntry->SetPosition(pEntry->GetPosition() + prod);
		}
	}
}

void on_recv_use_detonator(net::pckt_use_detonator& packet, int sender, uint16 time, bool bFromRing) // ID 33
{
	CProjectileInfo::RemoveDetonatorProjectiles(sender);
}

void on_recv_fire_area_effect(net::pckt_fire_area_effect& packet, int sender, uint16 time, bool bFromRing) // ID 34
{
	cMultiGame& Game = cMultiGame::Instance();
	sElementPhysical* pElemShooter = (sElementPhysical*)Game.GetEntityForHandle(sender, packet.shooter_id);
	if (pElemShooter != nil && pElemShooter->GetPhysical() != nil) {
		CShotInfo::AddShot(pElemShooter->GetPhysical(), (eWeaponType)packet.weapon_type, packet.fire_source, packet.target);
		CWeapon::GenerateFlameThrowerParticles(packet.fire_source, packet.direction);
	}
}

void on_recv_fire_instant_hit_car(net::pckt_fire_instant_hit_car& packet, int sender, uint16 time, bool bFromRing) // ID 36
{
	cMultiGame& Game = cMultiGame::Instance();
	sElementPhysical* pElemShooter = (sElementPhysical*)Game.GetEntityForHandle(sender, packet.shooter_id);
	if (pElemShooter == nil)
		return;

	sPed* lastDriver = Game.GetPlayerPed(pElemShooter->GetSync().vehicle->m_nLastDriverID);
	cPhysicalMG* pPhyMG = lastDriver != nil ? lastDriver->GetPhysical() : nil;
	if (pPhyMG != nil) {
		DMAudio.PlayOneShot(pPhyMG->m_audioEntityId, SOUND_WEAPON_SHOT_FIRED, 0.0f);
		DMAudio.SetEntityStatus(pPhyMG->m_audioEntityId, TRUE);
	}

	CVector vMoveSpeed = packet.move_speed * 1.6f;
	CVector vSource = CVector(packet.fire_source);
	CVector vTarget = CVector(packet.target);
	CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, vSource, vMoveSpeed, nil, 0.18f, 0, 0, 0, 0);
	CPointLights::AddLight(CPointLights::LIGHT_POINT, packet.move_speed, CVector(0.0f, 0.0f, 0.0f), 5.0f, 1.0f, 0.8f, 0.0f, 0, false);
	CBulletTraces::AddTrace(&vSource, &vTarget, eWeaponType::WEAPONTYPE_UZI_DRIVEBY, pPhyMG);
}

void on_recv_add_explosion(net::pckt_add_explosion& packet, int sender, uint16 time, bool bFromRing) // ID 50
{
	cMultiGame& Game = cMultiGame::Instance();
	sPlayer* pPlayer = Game.GetPlayer(sender);
	bool bHasQuadDamage = false;
	if (pPlayer)
		bHasQuadDamage = (pPlayer->GetSync().player->m_nPickups & ePowerupType::QUAD_DAMAGE);

	cPhysicalMG* pExplodingEnt = nil;
	cPhysicalMG* pCulpritEnt = nil;
	if (packet.cause != -1) {
		sElementPhysical* pExplodingElem = (sElementPhysical*)Game.GetEntityForHandle(sender, packet.cause);
		pExplodingEnt = pExplodingElem != nil ? pExplodingElem->GetPhysical() : nil;
	}
	if (packet.culprit != -1) {
		sElementPhysical* pCulpritElem = (sElementPhysical*)Game.GetEntityForHandle(sender, packet.culprit);
		pCulpritEnt = pCulpritElem != nil ? pCulpritElem->GetPhysical() : nil;
	}
	CExplosion::AddExplosion(pExplodingEnt, pCulpritEnt, (eExplosionType)packet.type, packet.pos, packet.lifetime, packet.hasSound, false, bHasQuadDamage);
}

