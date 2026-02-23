/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"
#include "World.h"
#include "PlayerPed.h"
#include "Pickups.h"
#include "Messages.h"
#include "Text.h"
#include "Darkel.h"
#include "AudioManager.h"
#include "Object.h"
#include "sampman.h"

#include "multiplayer/MultiGame.h"
#include "multiplayer/elements/sPickup.h"
#include "multiplayer/elements/sSyncStream.h"
#include "multiplayer/LScript.h"


sPickupSync::sPickupSync() : sElementSync()
{
	m_vecPos = CVector(0.0f, 0.0f, 0.0f);
#if defined(GTA_LIBERTY) && !defined(FIX_BUGS)
	m_bRemoved = false;
#else
	m_bRemoved = true;
#endif
	m_nPickupBy = -1;
}

sPickupSync::sPickupSync(CPickup* pSrc) : sElementSync()
{
	m_vecPos = pSrc->m_vecPos;
	m_bRemoved = pSrc->m_bRemoved;
	m_nPickupBy = (int8)pSrc->m_nNetPickedUpBy;
}

// inlined
sPickupSync::sPickupSync(const sPickupSync& other) : sElementSync(other)
{
	m_vecPos = other.m_vecPos;
	m_bRemoved = other.m_bRemoved;
	m_nPickupBy = other.m_nPickupBy;
}

sPickupSync::~sPickupSync() { }

// not checks: m_nPickupBy
bool sPickupSync::Compare(const sPickupSync& other)
{
	if (m_vecPos != other.m_vecPos)
		return false;
	if (m_bRemoved != other.m_bRemoved)
		return false;

	return true;
}

#if !defined(FINAL) && !defined(MASTER)
void sPickupSync::Dump()
{
	sElementSync::Dump();

	printf("=== sPickupSync Dump ===\n");
	printf("Position:        (%.3f, %.3f, %.3f)\n", m_vecPos.x, m_vecPos.y, m_vecPos.z);
	printf("Removed:         %s\n", m_bRemoved ? "true" : "false");
	printf("Picked Up By:    %d\n", m_nPickupBy);
	printf("================================\n");
}
#endif


sPickup::sPickup() {
	m_nPickHandle = -1;
	m_bPickupedUp = false;
	m_nTimeUnk = 0;
	m_nTime = 0;
#ifdef FIX_BUGS
	m_nPickupBy = -1;
#endif
}

sPickup::sPickup(int32 handle) {
	m_nPickHandle = handle;
	m_bPickupedUp = false;
	m_nTimeUnk = 0;
	m_nTime = 0;
	RegisterSelf();
	CPickup& pick = CPickups::aPickUps[CPickups::GetActualPickupIndex(m_nPickHandle)];
	pick.m_pNetworkElem = this;
	m_eType = pick.m_eType;
	m_eModelIndex = pick.m_eModelIndex;
	m_nQuantity = (uint8)pick.m_nQuantity;
	AttachSync(m_nTime, new sPickupSync(&pick));
#ifdef FIX_BUGS
	m_nPickupBy = -1;
#endif
}


ElementCapability sPickup::GetCapability()
{
	return sPickup::Capability();
}

bool sPickup::HasCapability(ElementCapability capability)
{
	if (sPickup::Capability() == capability)
		return true;
	if (sElement::Capability() == capability)
		return true;
	return false;
}

sPickup::~sPickup() {
	if (m_nPickHandle != -1)
		CPickups::RemovePickUp(m_nPickHandle);

	sElement::PurgeAttached();
	// ~sElement
}

sElementSync* sPickup::CreateSync() {
	return new sPickupSync();
}

void sPickup::DisposeSync(sElementSync* pSync) {
	delete (sPickupSync*)pSync;
}

sElementSync* sPickup::CreateSyncFromOther(sElementSync* pSync)
{
	sPickupSync& sync = *(sPickupSync*)pSync;
	return new sPickupSync(sync);
}

bool sPickup::HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB)
{
	sPickupSync& syncA = *(sPickupSync*)pSyncA;
	sPickupSync& syncB = *(sPickupSync*)pSyncB;
	return syncA.Compare(syncB);
}

void sPickup::ApplyClientSync(uint16 time) {
	sElement::ApplyClientSync(time);
	sPickupSync* pSync = FindSync(time, nil).pickup;
	if (m_nPickHandle == -1) {
		m_nPickHandle = CPickups::GenerateNewOne(pSync->GetPosition(), m_eModelIndex, m_eType, m_nQuantity, 0, false);
	}
	CPickup& pick = CPickups::aPickUps[CPickups::GetActualPickupIndex(m_nPickHandle)];
	pick.m_pNetworkElem = this;
	cMultiGame& Game = cMultiGame::Instance();
	uint16 nSelfID = Game.LocalPlayerID();
	if (pick.m_bRemoved != pSync->m_bRemoved && pSync->m_bRemoved == false) {
		if (m_nOwnerID != nSelfID) {
			m_bPickupedUp = false;
			debug("Setting picked up to FALSE (1)\n");
		}
	}
	if (pSync->m_bRemoved == true) {
		if (pick.m_pObject) {
			debug("~~~~ Last Picked Up By %d (My player id %d)\n", pSync->m_nPickupBy, nSelfID);
			if (pSync->m_nPickupBy != nSelfID) pick.GetRidOfObjects();
		}
	}
	pick.m_bRemoved = pSync->m_bRemoved;
}

ePowerupType inline GetPowerUpType(CPickup& pick) {
	if(pick.m_eModelIndex == MI_PICKUP_MEGADAMAGE)
		return ePowerupType::QUAD_DAMAGE;
	else if (pick.m_eModelIndex == MI_PICKUP_REGENERATOR)
		return ePowerupType::REGENRATOR;
	else if (pick.m_eModelIndex == MI_PICKUP_INVISIBLE)
		return ePowerupType::INVISIBLE;
	else if (pick.m_eModelIndex == MI_PICKUP_KILLFRENZY)
		return ePowerupType::KILL_FENZY;
#ifndef GTA_LIBERTY
	else if (pick.m_eModelIndex == MI_PICKUP_BRIEFCASE)
		return ePowerupType::FLAGBALL;
#endif
	return ePowerupType::POWERUP_NONE;
}

void sPickup::Update(uint16 time) {
	cMultiGame& Game = cMultiGame::Instance();
	CPickup& pick = CPickups::aPickUps[CPickups::GetActualPickupIndex(m_nPickHandle)];

	if (m_bPickupedUp) debug("%d %d\n", time, m_nTimeUnk);
	if (m_bPickupedUp && ((time - m_nTimeUnk) >= 0)) {
		debug("PICKED UP %i:%i by(%d)\n", m_nOwnerID, m_nID, m_nPickupBy);
		m_bPickupedUp = false;
		debug("Setting picked up to FALSE (2)\n");
		if (pick.m_bRemoved == false) {
			debug("~~~~ The pickup is NOT off\n");
			if (pick.m_eType == ePickupType::PICKUP_NETWORK_1 || pick.m_eType == ePickupType::PICKUP_NETWORK_2) {
				net::pckt_powerup_collected packet{};
				packet.pckt_size = sizeof(net::pckt_powerup_collected);
				packet.pckt_id = gtMP_PacketIDs.powerup_collected.pckt_id;
				packet.powerup_type = (uint8)GetPowerUpType(pick);
				packet.amount = (uint8)pick.m_nQuantity;
				packet.player = (uint8)m_nPickupBy;
				on_recv_powerup_collected(packet, m_nPickupBy, Game.m_pNetSession->m_nCurTime, true);
				Game.SendMessagePriority(packet, BROADCAST_PEER_GROUPID);
				pick.m_nNetPickedUpBy = m_nPickupBy;
#ifdef GTA_LIBERTY
	#ifdef FIX_BUGS
				pick.PickupTheDamnPickup(nil, nil, -1, m_nPickupBy);
	#else
				pick.PickupTheDamnPickup(nil, nil, -1, 100); // bug?
	#endif
#else
	#ifdef FIX_BUGS
				pick.PickupTheDamnPickup(nil, nil, -1, m_nPickupBy, -1);
	#else
				pick.PickupTheDamnPickup(nil, nil, -1, 100, -1); // bug?
	#endif
#endif
			}
			else {
				pick.m_nNetPickedUpBy = m_nPickupBy;
				net::pckt_pickup_collected packet{};
				packet.pckt_size = sizeof(net::pckt_pickup_collected);
				packet.pckt_id = gtMP_PacketIDs.pickup_collected.pckt_id;
				packet.elem = GetID();
				packet.remove = false;
#ifndef GTA_LIBERTY
				packet.modelIndex = pick.m_pObject ? pick.m_pObject->GetModelIndex() : -1;
#endif
				if (m_nPickupBy == Game.LocalPlayerID()) {
					debug("~~~~ It's picked up by this machine\n");
					on_recv_pickup_collected(packet, m_nPickupBy, Game.m_pNetSession->m_nCurTime, true);
				}
				else {
					debug("~~~~ It's NOT picked up by this machine\n");
#ifdef GTA_LIBERTY
					if (pick.PickupTheDamnPickup(nil, nil, -1, m_nPickupBy))
#else
					if (pick.PickupTheDamnPickup(nil, nil, -1, m_nPickupBy, -1))
#endif
					{
						debug("~~~~ Sending a guaranteed message to say it was picked up by %d\n", m_nPickupBy);
						Game.SendMessagePriority(packet, m_nPickupBy);
					}
				}
			}
		}
	}
	AttachSync(time, new sPickupSync(&pick));
}

bool sPickup::WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime)
{
	sPickupSync* pSync = FindSync(nSyncWriteTime, nil).pickup;
	if (static_cast<int16>(nSyncLastTime - m_vSync.front().m_nTime) >= 0)
		return WriteSyncDelta(pSyncStream, pSync, GetSyncWithTime(nSyncLastTime).pickup);

	PerformWriteSync(pSyncStream, pSync, ePickupSync::MP_PKTD_PICKUP_FULL_DIFF); // max diff
	return true;
}

void sPickup::ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync)
{
	sPickupSync& sync = *(sPickupSync*)pOutSync;
	uint8 nDiffMask = pSyncStream->ReadU8();

	if (nDiffMask & ePickupSync::MP_PKTD_PICKUP_BASE)
	{
		m_eModelIndex = pSyncStream->ReadI16();
		m_nQuantity = pSyncStream->ReadU8();
		m_eType = pSyncStream->ReadU8();
	}

	if (nDiffMask & ePickupSync::MP_PKTD_PICKUP_POSITION)
		sync.GetPosition() = pSyncStream->ReadVector();

	if (nDiffMask & ePickupSync::MP_PKTD_PICKUP_REMOVED)
		sync.m_bRemoved = pSyncStream->ReadBool();

	if (nDiffMask & ePickupSync::MP_PKTD_PICKUP_PICKUP_BY)
		sync.m_nPickupBy = pSyncStream->ReadI8();
}


void sPickup::RequestCollect(uint16 time, int32 who)
{
	debug("~~~~ Request collect\n");
	if (GetSync().pickup->m_bRemoved == true) {
		debug("~~~~ Request collect but PICKUP STATE IS OFF\n");
		return;
	}

	cMultiGame& Game = cMultiGame::Instance();
	if (m_nOwnerID != Game.LocalPlayerID())
	{
		if (m_bPickupedUp)
			debug("Is NOT locally owned and is has already been picked up\n");
		else
		{
			debug("Setting picked up to TRUE (1)\n");
			m_bPickupedUp = true;
			m_nPickupBy = who;
			debug("~~~~ sMessagePickupRequest Sent Id %d Owner %d\n", m_nID, m_nOwnerID);
			net::pckt_pickup_request packet{};
			packet.pckt_size = sizeof(net::pckt_pickup_request);
			packet.pckt_id = gtMP_PacketIDs.pickup_request.pckt_id;
			packet.elem = m_nID;
			Game.SendMessagePriority(packet, m_nOwnerID);
		}
	}
	else
	{
		if (m_bPickupedUp && (time - m_nTime) < 0)
		{
			debug("~~~~ No bloody idea what's happening here...\n");
			m_nTime = time;
			m_nOwnerID = who;
		}
		else if (!m_bPickupedUp)
		{
			float fMaxTime = 0.0f;
			for (int32 nPlayerID = 0; nPlayerID < Game.m_pNetSession->m_vPeers.size(); nPlayerID++)
			{
				sPlayer* pPlayer = Game.GetPlayer(nPlayerID);
				if (pPlayer == nil) continue;
				float fDistSqr = (pPlayer->GetPosition() - GetSync().pickup->GetPosition()).MagnitudeSqr();
				if (fDistSqr < SQR(5.0f))
				{
					sPeerState* peer = PeerManager.GetPeerAt(nPlayerID);
					float avg_latency = peer->m_bufferIndex ? peer->m_latencySum / peer->m_bufferIndex : 0.0f;
					fMaxTime = Max(fMaxTime, avg_latency);
				}
			}
			sPeerState* requester_peer = PeerManager.GetPeerById(who);
			float fSelfTime = requester_peer->m_bufferIndex ? requester_peer->m_latencySum / requester_peer->m_bufferIndex : 0.0f;
			m_nTimeUnk = (time - fSelfTime) + fMaxTime;
			m_nTime = time;
			m_nPickupBy = who;
			m_bPickupedUp = true;
			debug("~~~~ Picked up set to true\n");
		}
	}
}

#ifndef GTA_LIBERTY
void sPickup::Collect(int16 modelIndex)
#else
void sPickup::Collect()
#endif
{
	CPlayerPed* pPed = CWorld::Players[CWorld::PlayerInFocus].m_pPed;
	CPickup& pick = CPickups::aPickUps[CPickups::GetActualPickupIndex(m_nPickHandle)];

	debug("~~~~ COLLECTED PICKUP %i:%i\n", GetOwner(), GetID());
#ifndef GTA_LIBERTY
	CVehicle* pVeh = pPed->InVehicle() ? pPed->m_pMyVehicle : nil;
	if (pick.PickupTheDamnPickup(pPed, pVeh, CWorld::PlayerInFocus, 100, modelIndex))
#else
	CVehicle* pVeh = pPed->bInVehicle ? pPed->m_pMyVehicle : nil;
	if (pick.PickupTheDamnPickup(pPed, pVeh, CWorld::PlayerInFocus, 100))
#endif
	{
		debug("~~~~ Pickupthedamnpickup has returned true\n");
		if (m_bPickupedUp) debug(".... AND IS MARKED AS PICKED UP\n");
		else debug(".... AND IS *NOT* MARKED AS PICKED UP\n");
	}
}

inline uint8 sPickup::CompareSyncState(sPickupSync* pSync, sPickupSync* pLastSync)
{
	uint8 nDiffMask = ePickupSync::MP_PKTD_PICKUP_EQUAL;

	if (pSync->GetPosition() != pLastSync->GetPosition())
		nDiffMask |= ePickupSync::MP_PKTD_PICKUP_POSITION;
	if (pSync->m_bRemoved != pLastSync->m_bRemoved)
		nDiffMask |= ePickupSync::MP_PKTD_PICKUP_REMOVED;
	if (pSync->m_nPickupBy != pLastSync->m_nPickupBy)
		nDiffMask |= ePickupSync::MP_PKTD_PICKUP_PICKUP_BY;

	return nDiffMask;
}

bool sPickup::WriteSyncDelta(sWriteSyncStream* pSyncStream, sPickupSync* pSync, sPickupSync* pLastSync)
{
	uint8 nDiffMask = CompareSyncState(pSync, pLastSync);

	if (nDiffMask == ePickupSync::MP_PKTD_PICKUP_EQUAL)
		return false;

	PerformWriteSync(pSyncStream, pSync, nDiffMask);
	return true;
}

void sPickup::PerformWriteSync(sWriteSyncStream* pSyncStream, sPickupSync* pSync, uint8 nDiffMask)
{
	pSyncStream->WriteU8(nDiffMask);

	if (nDiffMask & ePickupSync::MP_PKTD_PICKUP_BASE)
	{
		pSyncStream->WriteI16(m_eModelIndex);
		pSyncStream->WriteU8(m_nQuantity);
		pSyncStream->WriteU8(m_eType);
	}

	if (nDiffMask & ePickupSync::MP_PKTD_PICKUP_POSITION)
		pSyncStream->WriteVector(pSync->GetPosition());

	if (nDiffMask & ePickupSync::MP_PKTD_PICKUP_REMOVED)
		pSyncStream->WriteBool(pSync->m_bRemoved);

	if (nDiffMask & ePickupSync::MP_PKTD_PICKUP_PICKUP_BY)
		pSyncStream->WriteI8(pSync->m_nPickupBy);
}

void mp_pickup_send_on_collected(sElement* elem) {
	net::pckt_pickup_collected packet{};
	packet.pckt_size = sizeof(net::pckt_pickup_collected);
	packet.pckt_id = gtMP_PacketIDs.pickup_collected.pckt_id;
	packet.elem = elem->GetID();
	packet.remove = true;
#ifndef GTA_LIBERTY
	packet.modelIndex = -1;
#endif
	cMultiGame::Instance().SendMessagePriority(packet, elem->GetOwner());
}