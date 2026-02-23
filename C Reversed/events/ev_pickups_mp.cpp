/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once
#include "multiplayer/MultiGame.h"
#include "multiplayer/net/packet.h"
#include "multiplayer/elements/sPickup.h"
#include "multiplayer/LScript.h"

#include "common.h"
#include "Ped.h"
#include "PlayerPed.h"
#include "PlayerInfo.h"
#include "Darkel.h"
#include "AudioManager.h"
#include "Messages.h"
#include "Text.h"


void on_recv_pickup_collected(net::pckt_pickup_collected& packet, int sender, uint16 time, bool bFromRing) // ID 23
{
	cMultiGame& Game = cMultiGame::Instance();
	debug("~~~~ Got message handler Pickup Collected\n");
	if (packet.remove)
	{
		debug("~~~~ Notifyremoval\n");
		sPickup* pElem = (sPickup*)Game.GetEntityForHandle(Game.LocalPlayerID(), packet.elem);
		if (pElem != nil)
		{
			Game.AttachEntity(pElem, nil);
			pElem->SetEntity(nil);
			delete pElem;
			debug("Notifyremoval DELETED PICKUP\n");
		}
	}
	else {
		debug("Not notifyremoval from %d pickup %d\n", sender, packet.elem);
		sPickup* pElem = (sPickup*)Game.GetEntityForHandle(sender, packet.elem);
		if (pElem != nil) {
#ifndef GTA_LIBERTY
			pElem->Collect(packet.modelIndex);
#else
			pElem->Collect();
#endif
		}
		else {
			debug("~~~~ Pickup handle not found\n");
		}
	}
}

void on_recv_pickup_request(net::pckt_pickup_request& packet, int sender, uint16 time, bool bFromRing) // ID 24
{
	cMultiGame& Game = cMultiGame::Instance();

	debug("~~~~ Received pickup request\n");
	sPickup* pElem = (sPickup*)Game.GetEntityForHandle(Game.LocalPlayerID(), packet.elem);
	if (pElem != nil) {
		pElem->RequestCollect(Game.AdjustSendTime(time, sender), sender);
		return;
	}
	debug("~~~~ Received pickup request - NO PICKUP!!!\n");
}

void on_recv_powerup_collected(net::pckt_powerup_collected& packet, int sender, uint16 time, bool bFromRing) // ID 25
{
	cMultiGame& Game = cMultiGame::Instance();
	bool bSendEvent = false;
	CPlayerPed* pPed = FindPlayerPed();
	net::pckt_play_remote_sound prs_packet{};
	prs_packet.pckt_size = sizeof(net::pckt_play_remote_sound);
	prs_packet.pckt_id = gtMP_PacketIDs.play_remote_sound.pckt_id;

	debug("POWERUP COLLECTED\n");
	if (packet.player == Game.LocalPlayerID())
	{
		if (pPed && !pPed->DyingOrDead())
		{
			debug("POWERUP type %d, time %d\n", packet.powerup_type, packet.amount);
			switch (static_cast<ePowerupType>(packet.powerup_type))
			{
				case ePowerupType::QUAD_DAMAGE:
				{
					if (!pPed->HasUberPickup()) {
						pPed->GiveQuadDamage(1000 * packet.amount);
						CMessages::AddBigMessage(TheText.Get("PUPMEGA"), 1000, 1);
						int32 nFreq = SampleManager.GetSampleBaseFrequency(SFX_ICE_CREAM_TUNE);
						prs_packet.bank = 0;
						prs_packet.frequency = nFreq;
						prs_packet.sample = SFX_ICE_CREAM_TUNE;
						prs_packet.counter = 0;
						prs_packet.frames = 20;
						AudioManager.DirectlyEnqueueSample(SFX_ICE_CREAM_TUNE, 0, 0, 1, nFreq, 127, 20, false);
						bSendEvent = true;
					}
					break;
				}
				case ePowerupType::REGENRATOR:
				{
					if (!pPed->HasUberPickup()) {
						pPed->GiveRegeneration(1000 * packet.amount);
						CMessages::AddBigMessage(TheText.Get("PUPREGN"), 1000, 1);
						int32 nFreq = SampleManager.GetSampleBaseFrequency(SFX_ICE_CREAM_TUNE);
						prs_packet.bank = 0;
						prs_packet.frequency = nFreq;
						prs_packet.sample = SFX_ICE_CREAM_TUNE;
						prs_packet.counter = 0;
						prs_packet.frames = 20;
						AudioManager.DirectlyEnqueueSample(SFX_ICE_CREAM_TUNE, 0, 0, 1, nFreq, 127, 20, false);
						bSendEvent = true;
					}
					break;
				}
				case ePowerupType::INVISIBLE:
				{
					if (!pPed->HasUberPickup()) {
						pPed->GiveInvisibility(1000 * packet.amount);
						CMessages::AddBigMessage(TheText.Get("PUPINVS"), 1000, 1);
						int32 nFreq = SampleManager.GetSampleBaseFrequency(SFX_ICE_CREAM_TUNE);
						prs_packet.bank = 0;
						prs_packet.frequency = nFreq;
						prs_packet.sample = SFX_ICE_CREAM_TUNE;
						prs_packet.counter = 0;
						prs_packet.frames = 20;
						AudioManager.DirectlyEnqueueSample(SFX_ICE_CREAM_TUNE, 0, 0, 1, nFreq, 127, 20, false);
						bSendEvent = true;
					}
					break;
				}
				case ePowerupType::KILL_FENZY:
				{
					if (!pPed->HasUberPickup()) {
						CDarkel::StartMultiplayerFrenzy(eWeaponType::WEAPONTYPE_ROCKETLAUNCHER, 1000 * packet.amount);
						CMessages::AddBigMessage(TheText.Get("PUPFREZ"), 1000, 1);
						int32 nFreq = SampleManager.GetSampleBaseFrequency(SFX_ICE_CREAM_TUNE);
						prs_packet.bank = 0;
						prs_packet.frequency = nFreq;
						prs_packet.sample = SFX_ICE_CREAM_TUNE;
						prs_packet.counter = 0;
						prs_packet.frames = 20;
						AudioManager.DirectlyEnqueueSample(SFX_ICE_CREAM_TUNE, 0, 0, 1, nFreq, 127, 20, false);
						bSendEvent = true;
					}
					break;
				}
#ifndef GTA_LIBERTY
				case ePowerupType::FLAGBALL:
				{
					pPed->GiveFlagBall();
					break;
				}
#endif
			}
		}
		if (bSendEvent) {
			prs_packet.priority = 1;
			prs_packet.volume = 127;
			Game.SendMessagePriority(prs_packet, BROADCAST_PEER_GROUPID);
		}
		return;
	}

	char buf[256];
#ifdef GTA_LIBERTY
	sprintf(buf, "main.game:Commentate('^S^%s ^T^PUPCOL')", Game.GetPlayerName(packet.player));
#else
	sprintf(buf, "coreLib:Commentate('^S^%s ^T^PUPCOL')", Game.GetPlayerName(packet.player));
#endif
	cLWrapper::Instance().ExecString(buf);
}
