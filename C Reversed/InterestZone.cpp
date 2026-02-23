/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "multiplayer/InterestZone.h"
#include "multiplayer/elements/sSyncStream.h"

#include "multiplayer/elements/sElement.h"			//
#include "multiplayer/elements/sElementPhysical.h"	//
#include "multiplayer/elements/sPlayer.h"			// 0
#include "multiplayer/elements/sPed.h"				// 1
#include "multiplayer/elements/sAutomobile.h"		// 2
#include "multiplayer/elements/sBike.h"				// 3
#include "multiplayer/elements/sHeli.h"				// 4
#include "multiplayer/elements/sRadarBlip.h"		// 5
#include "multiplayer/elements/sTextSprite.h"		// 6
#include "multiplayer/elements/sPickup.h"			// 7
#ifndef GTA_LIBERTY
#include "multiplayer/elements/sSpriteBase.h"		//
#include "multiplayer/elements/sBoat.h"				// 8
#include "multiplayer/elements/sPlane.h"			// 9
#include "multiplayer/elements/sBmx.h"				// 10
#include "multiplayer/elements/sQuadBike.h"			// 11
#include "multiplayer/elements/sNetMeter2d.h"		// 12
#endif
#ifdef MULTIGAME_IMPROVEMENTS
#include "multiplayer/elements/sObject.h"			// 13
#endif

#include "World.h"
#include "multiplayer/MultiGame.h"
#include "multiplayer/natives/public.h"
#include <algorithm>
#include <iterator>

// Recv
#define MP_ZONE_SEQ_FLAG			(0x80)		// BIT(7)
#define MP_ZONE_SEQ_MASK			(0x7F)		// 0b01111111
#define MP_ENTITY_CREATE_FLAG		(0x8000U)	// BIT(15)
#define MP_ENTITY_CREATE_MASK		(0x7FFFU)
#define MP_ENTITY_DESTROY_TYPE		(0xFF)
#define MP_ENTITY_TRANSFER_FLAG		(0x80)		// BIT(7)
#define MP_ENTITY_TRANSFER_MASK		(0x7F)

// Send
#define MP_PACKET_SPLIT_SIZE		(1024)	// trigger to stop add sync into a buffer, start into new packet (memmove)
#define MP_PICKUP_SEND_TIME			(2000)
#define MP_PICKUP_SEND_MASK			(0x3F)		// 0b00111111
#define MP_ENTITY_FINAL_FLAG		(0x80000000U)

#define MP_DISPOSE_SYNC_DELTA		(60) // NET_SESSION_60 ?

cInterestZone::cInterestZone(int16 nID) {
	m_nID = nID;
	m_nCurTime = 0;
	m_bHasPos = false;
	m_nBasis = 0;
	m_vPeers = std::vector<tZonePeer>();
	m_vElements = std::vector<sElement*>();
	m_vEntities = std::vector<tElementEntry>();
	m_vAck = std::vector<tAck>();
}

cInterestZone::~cInterestZone() {
	cMultiGame& Game = cMultiGame::Instance();
	gb_mp_will_destroy_elem = true;
	DiscardElement();
	gb_mp_will_destroy_elem = false;
	Game.m_ZoneManager.RemoveZone(GetID());
}

//#define TESTNET
#ifdef TESTNET
void SendGameStateW(bool bIsInRange, cInterestZone* pZ) {
	//if (!TheAdhoc.IsHost())
	//	return;

	static bool seeded = false;
	if (!seeded) { seeded = true; std::srand((unsigned)time(nullptr)); }

	uint16_t dataSize = (uint16_t)(std::rand() % 1001);

	std::vector<uint8_t> payload;
	payload.resize(dataSize);
	for (uint16_t i = 0; i < dataSize; ++i) payload[i] = (uint8_t)(std::rand() & 0xFF);

	uint32_t dataHash = fast_hash32(payload.data(), dataSize);

	sWriteSyncStream stream;
	net::pckt_game_state& packet = *(net::pckt_game_state*)&stream;
	packet.pckt_size = sizeof(net::pckt_game_state); // after pckt_game_state
	packet.pckt_id = gtMP_PacketIDs.game_state.pckt_id;
	packet.sequence = 0;
	packet.zone = pZ->m_nID;
	stream.WriteU16(dataSize);
	stream.WriteU32(dataHash);
	if (dataSize) {
		for (uint16_t i = 0; i < dataSize; ++i) stream.WriteU8(payload[i]);
	}
	DUMP_PACKET(packet, "");
	debug("[SEND TEST] game_state -> size=%d hash=0x%08X total=%u\n", dataSize, dataHash, (unsigned)packet.pckt_size);

//#ifdef FIX_BUGS
//	if (m_vPeers.size() == 1) {
//#else
//	if (m_vPeers.size() < 2) {
//#endif
//		cMultiGame::Instance().SendMessage(packet, m_vPeers[0].nPeerID);
//	}
//	else
	{
		cMultiGame::Instance().SendMessage(packet, BROADCAST_PEER_GROUPID);
		//cMultiGame::Instance().SendMessage(packet, 1);
	}
}

void ReceiveGameStateW(uint32 nPeerID, uint16 nState, sReadSyncStream* syncStream) {
	//if (TheAdhoc.IsHost())
	//	return;

	uint16 dataSize = syncStream->ReadU16();
	uint32 receivedHash = syncStream->ReadU32();
	uint8* dataStart = syncStream->Tellg();
	if (dataSize) syncStream->AddLength(dataSize);
	uint32 calcHash = fast_hash32(dataStart, dataSize);

	if (receivedHash != calcHash) {
		debug("[RECV TEST P%u] HASH MISMATCH: recv=0x%08X calc=0x%08X size=%u\n",
			(unsigned)nPeerID, receivedHash, calcHash, (unsigned)dataSize);

		int32 dump = Min(32, dataSize);
		if (dump > 0) {
			debug("[RECV TEST P%u] first %d bytes: ", (unsigned)nPeerID, dump);
			for (int32 i = 0; i < dump; ++i) debug("%02X ", dataStart[i]);
			debug("\n");
		}

		assert(false && "ReceiveGameState TEST: hash mismatch");
		return;
	}

	debug("[RECV TEST P%u] OK size=%u hash=0x%08X\n", (unsigned)nPeerID, (unsigned)dataSize, receivedHash);
}
#endif

void cInterestZone::SendGameState(bool bIsInRange) {
#ifdef TESTNET
	SendGameStateW(bIsInRange, this); return;
#endif
#if !defined(FINAL) && !defined(MASTER)
	//INTEREST_ZONE_LOG(1б "cInterestZone::SendGameState(Z%d B%d F%d)\n", m_nID, m_nBasis, m_nCurTime);
#endif
	cMultiGame& Game = cMultiGame::Instance();
	cAdhoc& Adhoc = cAdhoc::Instance();
	if (Adhoc.HadError()) {
		debug("AdhocConnection is boned, no multiplayer update\n");
		return;
	}
	if (!Game.m_bIsConnected) {
#ifdef FIX_BUGS
		debug("Not connected, no send\n");
#else
		debug("Not connected, no send");
#endif
		return;
	}

#ifdef GTA_LIBERTY
	float fPosX = 0.0f, fPosY = 0.0f;
	if (m_bHasPos) {
		// m_nPosX normalized 0.0f - 1.0f
		fPosX = WORLD_MIN_X + (float)m_nPosX * WORLD_SIZE_X;
		fPosY = WORLD_MIN_Y + (float)m_nPosY * WORLD_SIZE_Y;
	}
#endif

	if (!bIsInRange)
		DiscardElement();

	uint16 nCurFrame = Game.m_pNetSession->m_nCurTime;
#ifdef GTA_LIBERTY
	float delta = CTimer::GetTimeStep() * 200.0f;
#else
	float delta = CTimer::GetTimeStep() * 400.0f;
#endif
	float fCurTime = ((float)m_nCurTime) - delta;
	m_nCurTime = (uint32)fCurTime;

	if (fCurTime < 0.0f)
	{
		m_nCurTime = 0;
		sWriteSyncStream stream;
		net::pckt_game_state& packet = *(net::pckt_game_state*)&stream;
		packet.pckt_size = sizeof(net::pckt_game_state);
		packet.pckt_id = gtMP_PacketIDs.game_state.pckt_id;
		packet.sequence = 0;
		packet.zone = m_nID;
		m_nBasis = nCurFrame;

		// Collect intersection of frames from all peers acks
		std::vector<uint16> combinedFrames;
		if (!m_vPeers.empty())
		{
			combinedFrames = m_vPeers[0].acks;
			for (int32 i = 1; i < m_vPeers.size(); ++i)
			{
				std::vector<uint16> newCombined;
				std::set_intersection(
					combinedFrames.begin(), combinedFrames.end(),
					m_vPeers[i].acks.begin(), m_vPeers[i].acks.end(),
					std::back_inserter(newCombined)
				);
				combinedFrames = std::move(newCombined);
				if (combinedFrames.empty()) break;
			}

			if (!combinedFrames.empty())
			{
				uint16 maxFrame = combinedFrames.back();
				m_nBasis = maxFrame;
				if (static_cast<int16>(nCurFrame - maxFrame) > MP_DISPOSE_SYNC_DELTA) {
					debug("discarding old deltas: %i for time == %d\n", nCurFrame - maxFrame, nCurFrame);
					m_nBasis = nCurFrame;
					for (auto& ent : m_vEntities) {
						ent.nEntityID &= ~MP_ENTITY_FINAL_FLAG;
					}
				}
			}

			// Now remove old acks from each peer
			for (int32 i = 0; i < m_vPeers.size(); ++i)
			{
				auto& peer = m_vPeers[i];
				auto& acks = peer.acks;
				if (acks.empty()) {
					debug("Peer %i : Last ack FUCK UP\n", i);
					debug("Peer %i : Last ack FUCK UP\n", i);
					continue;
				}
				auto it = std::lower_bound(acks.begin(), acks.end(), m_nBasis);
				if (it == acks.end()) {
					it = acks.end() - 1;
				}
				acks.erase(acks.begin(), it);
			}
		}

		// Calculate lastBasis
		uint16 lagValue = Game.m_nLagValue;
		uint16 lastBasis = nCurFrame - lagValue;
		if (Game.m_bIsConnected && m_nBasis < lastBasis) {
			lastBasis = m_nBasis;
		}

		// Write basis
		stream.WriteU16(m_nBasis);
#ifdef FIX_BUGS
		INTEREST_ZONE_LOG(1, "FRAME %i BASIS %i : Zone %i\n", nCurFrame, m_nBasis, m_nID);
#else
		INTEREST_ZONE_LOG(1, "FRAME %i BASIS %i : Zone %i", nCurFrame, m_nBasis, m_nID);
#endif
		// --mazahaka: запоминаем позицию перед ack data чтобы в 1+ seq могли ack перезаписать нахуй, а в recv парсим первый seq
		uint32 pos_ack_count = stream.pckt_size;  // Position after basis, before ack count // data start

		// Write ack count
		uint8 ackCount = static_cast<uint8>(m_vAck.size());
		stream.WriteU8(ackCount);
#if !defined(FINAL) && !defined(MASTER)
		if (!ackCount && m_vPeers.size() != 0) { // bad stuff when i didnt have acks for all the receipts in recv (no new states?)
			SetConsoleColor(0);
			debug("[SEND] SYKAAA PIZDA NAHUY NET MOIH ACK CHUJIH STATE (NE SOBRAL V ReceiveGameState) curTime %d Z%d\n", nCurFrame, m_nID);
			SetConsoleColor(6);
		}
#endif
		for (const auto& ack : m_vAck) {
			stream.WriteU8(ack.nPeerID);
			stream.WriteU16(ack.nFrame);
#ifdef FIX_BUGS
			INTEREST_ZONE_LOG(1, " I am send Ack peer %i frame %i\n", ack.nPeerID, ack.nFrame);
#else
			INTEREST_ZONE_LOG(1, " Ack peer %i frame %i", ack.nPeerID, ack.nFrame);
#endif
		}

		// Clear m_vAck
		m_vAck.clear();

		// Process elements
		auto elemIt = m_vElements.begin();
		while (elemIt != m_vElements.end())
		{
			sElement* pElem = *elemIt;
			pElem->DisposeAttached(lastBasis);
			if (m_vPeers.empty() || pElem->m_bWasTransfered) {
				++elemIt;
				continue;
			}

			uint32 startSize = stream.pckt_size;
			bool wrote = false;
			uint16 elemTime = pElem->m_nLastSentFrame;
			bool isPickup = (pElem->GetType() == eElementType::ELEMENT_TYPE_PICKUP);
			uint32 sendInterval = isPickup ? MP_PICKUP_SEND_TIME + ((pElem->GetID() & MP_PICKUP_SEND_MASK) << 6) : MP_PICKUP_SEND_TIME;

			//if (m_nBasis < nCurFrame && m_nBasis >= syncTime && (nCurFrame - elemTime) < sendInterval)
			if (static_cast<int16>(m_nBasis - nCurFrame) < 0 && // m_nBasis < nCurFrame
				static_cast<int16>(m_nBasis - pElem->m_vSync.front().m_nTime) >= 0 && //  m_nBasis >= syncTime
				static_cast<int16>(nCurFrame - elemTime) < static_cast<int16>(sendInterval))
			{
				// Delta update
				INTEREST_ZONE_LOG(1, " delta sent for entity %i type %i\n", pElem->GetID(), pElem->GetType()); // custom
				uint16 elemId = pElem->GetID() & MP_ENTITY_CREATE_MASK;
				stream.WriteU16(elemId);
				pElem->m_nPrevOwnerID = -1;
				wrote = pElem->WriteSyncToStream(&stream, nCurFrame, m_nBasis);
				if (wrote) pElem->m_nLastSentFrame = nCurFrame;
			}
			else
			{
				// Full create/transfer
				INTEREST_ZONE_LOG(1, " Full state sent for entity %i type %i\n", pElem->GetID(), pElem->GetType());
				uint16 elemId = pElem->GetID() | MP_ENTITY_CREATE_FLAG;
				stream.WriteU16(elemId);
				uint8 elemType = static_cast<uint8>(pElem->GetType());
				if (pElem->m_nPrevOwnerID != -1)
					elemType |= MP_ENTITY_TRANSFER_FLAG;
				stream.WriteU8(elemType);
				if (elemType & MP_ENTITY_TRANSFER_FLAG) {
					stream.WriteI8(pElem->m_nPrevOwnerID);
					stream.WriteI16(pElem->m_nPrevID);
				}
				////int16 syncTime = static_cast<int16>(pElem->m_vSync.front().m_nTime - 1); // Warn! 0 - 1 -> u16 max
				////assert(syncTime < pElem->m_vSync.front().m_nTime);
				////wrote = pElem->WriteSyncToStream(&stream, nCurFrame, static_cast<uint16>(syncTime));
				uint16 syncTime = pElem->m_vSync.front().m_nTime - 1; // Warn! 0 - 1 -> u16 max
				wrote = pElem->WriteSyncToStream(&stream, nCurFrame, syncTime);
				pElem->m_nLastSentFrame = nCurFrame;
			}

			const char* aEntityNames[] = {
				"plyr",
				"ped ",
				"car ",
				"bike",
				"heli",
				"blip",
				"text",
				"pick",
#ifndef GTA_LIBERTY
				"boat",
				"plne",
				"bmx ",
				"quad",
				"nm2d",
#endif
#ifdef MULTIGAME_IMPROVEMENTS
				"obj ",
#endif
			};


			eElementType type = pElem->GetType();
			if (wrote) {
#ifdef FIX_BUGS
				// probably diff logger
				INTEREST_ZONE_LOG(1, " Entity %i %s Size %i\n", pElem->GetID(), aEntityNames[static_cast<uint32>(type)], packet.pckt_size - startSize);
				INTEREST_ZONE_LOG(1, " Oldest state %i\n", lastBasis);
				INTEREST_ZONE_LOG(1, "\n");
#else
				INTEREST_ZONE_LOG(1, " Entity %i %s Size %i", pElem->GetID(), aEntityNames[static_cast<uint32>(type)], packet.pckt_size - startSize);
				INTEREST_ZONE_LOG(1, " Oldest state %i", lastBasis);
#endif
#if !defined(FINAL) && !defined(MASTER)
				//INTEREST_ZONE_LOG(1, "SendGameState BF: 0x%p AF: 0x%p TT: %d\n", stream.CalcPos(startSize), stream.Tellg(), stream.pckt_size - startSize);
#endif
			}
			else {
#if !defined(FINAL) && !defined(MASTER)
				INTEREST_ZONE_LOG(1, " Skip send sync: wrote is false\n");
#endif
				pElem->DisposeFrame(nCurFrame);
				packet.pckt_size = startSize; // reset entity id and other header when delta is 0x0
			}

			// send split msg without finale flag 0x80 MP_ZONE_SEQ_FLAG
			// pos_ack_count - payload start, startSize - prev valid end sync for entity (< LIM)
			// смотри прикол split разбивает не по 1024 это тригер на разбиение, а по ласт размеру sync, тоесть отправит например 2 entity из 4х
			// и в recv здать целостность не нужно
			assert(packet.pckt_size < SYNC_WRITER_BUFFER_SIZE);
			if (packet.pckt_size > MP_PACKET_SPLIT_SIZE) // split per sync (entity)
			{
				uint32 nCurSz = packet.pckt_size;
				packet.pckt_size = startSize;
				m_nCurTime += startSize; // kek
#ifdef FIX_BUGS
				INTEREST_ZONE_LOG(1, " == message split point %i ==\n", packet.sequence + 1);
#else
				INTEREST_ZONE_LOG(1, " == message split point %i ==", packet.sequence + 1);
#endif
				//DUMP_PACKET(packet, "SENDSPLIT", 1); // -----------------------------
				Game.SendMessage(packet, BROADCAST_PEER_GROUPID);
				packet.pckt_size = pos_ack_count + (nCurSz - startSize);
				// [ACK] EH1[SYN1] EH2[SYN2] .. --->> [ACK] EH2[SYN2] ...
				// [HDR] [BASIS] [ACKCNT] [ACK1] [ACK2] ...[ACKN][PAYLOAD:EH1[SYN1] EH2[SYN2] EH3[SYN3] ...]
				//	^				 ^								 ^
				//	|                |                               |
				//	buf start     pos_ack_count                  buf end(nCurSz)
				memmove(((uint8*)&packet) + pos_ack_count, ((uint8*)&packet) + startSize, nCurSz - startSize); // rewrite ack data + entity id type sync
				++packet.sequence;
			}
			++elemIt;
		}

		// Process remove entries
        auto removeIt = m_vEntities.begin();
        while (removeIt != m_vEntities.end()) {
			uint32 entityId = removeIt->nEntityID;
            uint16 basis = removeIt->nBasis;
			bool isFinal = static_cast<int16>(basis - lastBasis) < 0;
#ifdef FIX_BUGS
			INTEREST_ZONE_LOG(1, " Deleted entity %i %s\n", (entityId & MP_ENTITY_CREATE_MASK), isFinal ? "FINAL" : "");
            //debug(" Deleted entity %i %s\n", entityId, isFinal ? "FINAL" : "");
#else
			INTEREST_ZONE_LOG(1, " Deleted entity %i %s", (entityId & MP_ENTITY_CREATE_MASK), isFinal ? "FINAL" : "");
#endif

            uint16 writeId = (entityId & MP_ENTITY_CREATE_MASK) | MP_ENTITY_CREATE_FLAG; // ?
            stream.WriteU16(writeId);
            stream.WriteU8(MP_ENTITY_DESTROY_TYPE);
			bool canRemove = (entityId & MP_ENTITY_FINAL_FLAG) != 0;
			if (!canRemove) {
				removeIt->nBasis = nCurFrame;
				removeIt->nEntityID |= MP_ENTITY_FINAL_FLAG;
				++removeIt;
			}
			else if (isFinal) {
				removeIt = m_vEntities.erase(removeIt);
			}
			else {
				++removeIt;
			}
        }

		// Final send!!

        m_nCurTime += stream.pckt_size; // kek 2
		packet.sequence |= MP_ZONE_SEQ_FLAG; // final packet flags &0x7F- seq index, &0x80 finale flag

		//DUMP_PACKET(packet, "FINALESEND", 1); // -----------------------------
#ifdef FIX_BUGS
        if (m_vPeers.size() == 1) {
#else
        if (m_vPeers.size() < 2) {
#endif
            Game.SendMessage(packet, m_vPeers[0].nPeerID);
        } else {
            Game.SendMessage(packet, BROADCAST_PEER_GROUPID);
        }
    } // DT < 0
	else if (!bIsInRange && m_vElements.empty() && m_vEntities.empty()) {
        delete this;
    }
}

void cInterestZone::ReceiveGameState(uint32 nPeerID, uint16 nState, sReadSyncStream* syncStream) {
#ifdef TESTNET
	ReceiveGameStateW(nPeerID, nState, syncStream); return;
#endif
#if !defined(FINAL) && !defined(MASTER)
	INTEREST_ZONE_LOG(1, "start cInterestZone::ReceiveGameState( stream -> 0x%p  streamEnd -> 0x%p)\n", syncStream->m_pBuffer, syncStream->m_pBufferEnd);
	INTEREST_ZONE_LOG(1, "cInterestZone::ReceiveGameState(Z%d B%d F%d)\n", m_nID, m_nBasis, m_nCurTime);
#define LOG_ON_END() INTEREST_ZONE_LOG(1, "end cInterestZone::ReceiveGameState( stream -> 0x%p  streamEnd -> 0x%p)\n", syncStream->m_pBuffer, syncStream->m_pBufferEnd);
//#define ON_ACK_COLLECT(peer, frame) debug("ON_ACK_COLLECT(P%d F%d Z%d, T%p)\n", peer, frame, m_nID, this)
#define ON_ACK_COLLECT(peer, frame)
#else
#define ON_ACK_COLLECT(peer, frame)
//#define LOG_ON_END() // cleanup
#endif
	cMultiGame& Game = cMultiGame::Instance();
	cAdhoc& Adhoc = cAdhoc::Instance();
	if (Adhoc.HadError()) {
		debug("AdhocConnection or Multigame is boned, no multiplayer update\n");
		LOG_ON_END();
		return;
	}
	if (!Game.IsOpen()) {
		debug("AdhocConnection or Multigame is boned, no multiplayer update\n");
		LOG_ON_END();
		return;
	}

	uint16 basis = syncStream->ReadU16();
#ifdef FIX_BUGS
	SetConsoleColor(3); // custom
	INTEREST_ZONE_LOG(1, "P%i ZONE %i state %i (basis %i)\n", nPeerID, m_nID, nState, basis);
	SetConsoleColor(6); // custom
#else
	INTEREST_ZONE_LOG(1, "P%i ZONE %i state %i (basis %i)", nPeerID, m_nID, nState, basis);
#endif
	int32 peerFromIndex = -1;
	for (int32 i = 0; i < (int32)m_vPeers.size(); ++i) {
		if (m_vPeers[i].nPeerID == nPeerID) {
			peerFromIndex = i;
			break;
		}
	}

	if (peerFromIndex < 0) {
		debug("peer not subscribed to interest zone..(%d)(%d) uhoh\n", peerFromIndex, m_vPeers.size());
		return;
	}
#ifdef GTA_LIBERTY
	if (!Game.m_pNetSession->IsPeerConnected(m_vPeers[peerFromIndex].nPeerID))
#else
	if (!PeerManager.IsPeerConnected(m_vPeers[peerFromIndex].nPeerID))
#endif
	{
		debug("Ignoring interestzone update for peer %d.\n", m_vPeers[peerFromIndex].nPeerID);
		LOG_ON_END();
		return;
	}

	tZonePeer& zonePeerFrom = m_vPeers[peerFromIndex];
	if (zonePeerFrom.nState < nState) { // if (static_cast<int16>(zonePeerFrom.nState - nState) < 0)
		zonePeerFrom.nState = nState;
		zonePeerFrom.nSeqBitmask = 0x0;
		zonePeerFrom.nExpectedSeqMask = 0xFFFFFFFF;
	}

	uint8 seqFlag = syncStream->sequence;
	uint8 seqIndex = seqFlag & MP_ZONE_SEQ_MASK;
	if (nState == zonePeerFrom.nState && !(zonePeerFrom.nSeqBitmask & (1 << seqIndex)))
	{
		// MP_ZONE_SEQ_FLAG 0b10000000  BIT(7)
		if (seqFlag & MP_ZONE_SEQ_FLAG) { // LAST FRAGMENT
			zonePeerFrom.nExpectedSeqMask = (1 << (seqIndex + 1)) - 1;
#if !defined(FINAL) && !defined(MASTER)
			INTEREST_ZONE_LOG(1, "GOT FINALE PACKET, lets go parse\n");
#endif
		}
#if !defined(FINAL) && !defined(MASTER)
		else {
			INTEREST_ZONE_LOG(1, "GOT SPLIT PACKET %d, lest go parse until end\n", seqIndex);
		}
#endif
		zonePeerFrom.nSeqBitmask |= (1 << seqIndex);

		if (seqIndex == 0) // 1st split/full packet
		{
			uint8 ackCount = syncStream->ReadU8();
			for (uint8 ackIdx = 0; ackIdx < ackCount; ++ackIdx) {
				uint8 ackPeer = syncStream->ReadU8();
				uint16 ackFrame = syncStream->ReadU16();
				if (ackPeer == Game.LocalPlayerID()) {
					std::vector<uint16>& acks = zonePeerFrom.acks;
					acks.push_back(ackFrame);
					//std::sort(acks.begin(), acks.end());

					auto it = zonePeerFrom.acks.end() - 1; // Last element
					while (it != zonePeerFrom.acks.begin()) {
						auto prev = it - 1;
						if (*it >= *prev) break; // Already in order
						//if (static_cast<int16>(*it - *prev) >= 0) break;
						std::iter_swap(it, prev);
						--it;
					}

					INTEREST_ZONE_LOG(1, " ACK %i", ackFrame);
					Game.OnAckRecv(nPeerID);
				}
			}
		}
	}

	zonePeerFrom.nBasis = basis;
	if (syncStream->IsEmpty())
	{
		if (zonePeerFrom.nSeqBitmask == zonePeerFrom.nExpectedSeqMask) // all packets received
		{
			bool found = false;
			for (auto& ack : m_vAck) {
				if (ack.nPeerID == nPeerID) {
					found = true;
					//if (static_cast<int16>(it->nFrame - nState) < 0) it->nFrame = nState;
					if (ack.nFrame < nState) ack.nFrame = nState;
					ON_ACK_COLLECT(nPeerID, nState); // upd
					break;
				}
			}

			if (!found) {
				tAck newAck;
				newAck.nPeerID = nPeerID;
				newAck.nFrame = nState;
				m_vAck.push_back(newAck);
				ON_ACK_COLLECT(nPeerID, nState); // new
			}
		}
		LOG_ON_END();
		return;
	}

	while (!syncStream->IsEmpty())
	{
#if !defined(FINAL) && !defined(MASTER)
		INTEREST_ZONE_LOG(1, "entityId: 0x%p, peek 0x%X / %d\n", syncStream->Tellg(), syncStream->PeekU16(), syncStream->PeekU16());
#endif
		uint16 entityId = syncStream->ReadU16();

		// if create flag in id - create entity
		if (entityId & MP_ENTITY_CREATE_FLAG)
		{
			uint8 type = syncStream->ReadU8();
			entityId &= MP_ENTITY_CREATE_MASK;

			if (type == MP_ENTITY_DESTROY_TYPE)
			{
				debug(" Destroy %i", entityId);
				gb_mp_will_destroy_elem = true;
				sElement* pElem = Game.GetEntityForHandle(nPeerID, entityId);
				if (pElem) delete pElem;
				gb_mp_will_destroy_elem = false;
				continue;
			}

			int8 owner = nPeerID;
			int16 transferId = entityId;
			if (type & MP_ENTITY_TRANSFER_FLAG)
			{
				owner = syncStream->ReadI8();
				transferId = syncStream->ReadI16();
#ifdef FIX_BUGS
				debug("Destroy %i (own %i) for transfer\n", transferId, owner);
#else
				debug("Destroy %i (own %i) for transfer", transferId, owner);
#endif
				gb_mp_will_destroy_elem = true;
				type &= MP_ENTITY_TRANSFER_MASK;
				sElement* pElem = Game.GetEntityForHandle(owner, transferId);
				if (pElem) delete pElem;
				gb_mp_will_destroy_elem = false;
			}

			if (!Game.GetEntityForHandle(nPeerID, entityId))
			{

				sElement* pNewElem = nil;
				SetConsoleColor(3); // custom
				switch ((eElementType)type)
				{
					case eElementType::ELEMENT_TYPE_PLAYER:
					{
						//debug(" Create Player peer=%d ent=%d", nPeerID, entityId); // added \n
						debug(" Create Player peer=%d ent=%d\n", nPeerID, entityId);
						pNewElem = new sPlayer();
						break;
					}
					case eElementType::ELEMENT_TYPE_PED:
					{
						//debug(" Create Ped peer=%d ent=%d", nPeerID, entityId); // added \n
						debug(" Create Ped peer=%d ent=%d\n", nPeerID, entityId);
						pNewElem = new sPed();
						break;
					}
					case eElementType::ELEMENT_TYPE_AUTOMOBILE:
					{
						//debug(" Create Car peer=%d ent=%d", nPeerID, entityId); // added \n
						debug(" Create Car peer=%d ent=%d\n", nPeerID, entityId);
						pNewElem = new sAutomobile();
						assert(false && "unimplemented sync reader");
						break;
					}
					case eElementType::ELEMENT_TYPE_BIKE:
					{
						//debug(" Create Bike peer=%d ent=%d", nPeerID, entityId); // added \n
						debug(" Create Bike peer=%d ent=%d\n", nPeerID, entityId);
						pNewElem = new sBike();
						assert(false && "unimplemented sync reader");
						break;
					}
#ifndef GTA_LIBERTY
					case eElementType::ELEMENT_TYPE_HELI:
					{
						//debug(" Create Heli peer=%d ent=%d", nPeerID, entityId); // added \n
						debug(" Create Heli peer=%d ent=%d\n", nPeerID, entityId);
						pNewElem = new sHeli();
						assert(false && "unimplemented sync reader");
						break;
					}
#endif
					case eElementType::ELEMENT_TYPE_RADAR_BLIP:
					{
						//debug(" Create Blip peer=%d ent=%d", nPeerID, entityId); // added \n
						debug(" Create Blip peer=%d ent=%d\n", nPeerID, entityId);
						pNewElem = new sRadarBlip();
						break;
					}
					case eElementType::ELEMENT_TYPE_TEXT:
					{
						//debug(" Create Text Sprite peer=%d ent=%d", nPeerID, entityId); // added \n
						debug(" Create Text Sprite peer=%d ent=%d\n", nPeerID, entityId);
						pNewElem = new sTextSprite();
						assert(false && "unimplemented sync reader");
						break;
					}
					case eElementType::ELEMENT_TYPE_PICKUP:
					{
						//debug(" Create Pickup peer=%d ent=%d", nPeerID, entityId); // added \n
						debug(" Create Pickup peer=%d ent=%d\n", nPeerID, entityId);
						pNewElem = new sPickup();
						break;
					}
#ifndef GTA_LIBERTY
					case eElementType::ELEMENT_TYPE_BOAT:
					{
						//debug(" Create Boat peer=%d ent=%d", nPeerID, entityId); // added \n
						debug(" Create Boat peer=%d ent=%d\n", nPeerID, entityId);
						pNewElem = new sBoat();
						assert(false && "unimplemented sync reader");
						break;
					}
					case eElementType::ELEMENT_TYPE_PLANE:
					{
						//debug(" Create Plane peer=%d ent=%d", nPeerID, entityId); // added \n
						debug(" Create Plane peer=%d ent=%d\n", nPeerID, entityId);
						pNewElem = new sPlane();
						assert(false && "unimplemented sync reader");
						break;
					}
					case eElementType::ELEMENT_TYPE_BMX:
					{
						//debug(" Create Bmx peer=%d ent=%d", nPeerID, entityId); // added \n
						debug(" Create Bmx peer=%d ent=%d\n", nPeerID, entityId);
						pNewElem = new sBmx();
						assert(false && "unimplemented sync reader");
						break;
					}
					case eElementType::ELEMENT_TYPE_QUADBIKE:
					{
						//debug(" Create QuadBike peer=%d ent=%d", nPeerID, entityId); // added \n
						debug(" Create QuadBike peer=%d ent=%d\n", nPeerID, entityId);
						pNewElem = new sQuadBike();
						assert(false && "unimplemented sync reader");
						break;
					}
					case eElementType::ELEMENT_TYPE_NETMETER2D:
					{
						//debug(" Create NetMeter2d peer=%d ent=%d", nPeerID, entityId); // added \n
						debug(" Create NetMeter2d peer=%d ent=%d\n", nPeerID, entityId);
						pNewElem = new sNetMeter2d();
						assert(false && "unimplemented sync reader");
						break;
					}
#endif
#ifdef MULTIGAME_IMPROVEMENTS
					case eElementType::ELEMENT_TYPE_OBJECT:
					{
						//debug(" Create Object peer=%d ent=%d", nPeerID, entityId); // added \n
						debug(" Create Object peer=%d ent=%d\n", nPeerID, entityId);
						pNewElem = new sObject();
						break;
					}
#endif
					default:
					{
						debug(" Unknown element type=%d 0x%X peer=%d ent=%d 0x%X\n", type, type, nPeerID, entityId, entityId);
						assert(false && "pizda");
						LOG_ON_END();
						return;
					}
				}
				SetConsoleColor(6); // custom

				pNewElem->RegisterSelfWithOwner(nPeerID, entityId);
#ifndef GTA_LIBERTY
				sElementPhysical* pElemPhys = pNewElem->HasCapability(sElementPhysical::Capability()) ? (sElementPhysical*)pNewElem : nil;
				if (pElemPhys)
				{
					net::pckt_ack_entity_create packet;
					packet.pckt_size = sizeof(net::pckt_ack_entity_create);
					packet.pckt_id = gtMP_PacketIDs.ack_entity_create.pckt_id;
					packet.entityId = entityId;
					Game.SendMessagePriority(packet, BROADCAST_PEER_GROUPID);
				}
#endif
				mp_lsc_transfer_entity(owner, transferId, pNewElem->GetOwner(), pNewElem->GetID());
				pNewElem->m_pZone = this;
				uint8* pBufferBeforeParse = syncStream->Tellg();
				sElementSync* pSync = pNewElem->GetSyncWithTime2(nState, nState).element;
				pNewElem->ReadSyncFromStream(syncStream, pSync); // << ------------------  read sync for new element [elem header] [elem full sync]
				//assert(syncStream->Tellg() - pBufferBeforeParse == GetSyncSizeByElement(pNewElem)); // full state [leeds fakap full diff kek]
				assert(syncStream->Tellg() - pBufferBeforeParse > 0); // detect unimpremented sync reader // full delta не может быть 0

				const char* aEntityNames[] = {
					"plyr",
					"ped ",
					"car ",
					"bike",
					"heli",
					"blip",
					"text",
					"pick",
#if !defined(GTA_LIBERTY) && defined(FIX_BUGS)
					"boat",
					"plne",
					"bmx ",
					"quad",
					"nm2d",
#endif
#ifdef MULTIGAME_IMPROVEMENTS
					"obj ",
#endif
				};

				eElementType elemType = pNewElem->GetType();
#ifdef FIX_BUGS
				debug(" FULL STATE READ for new element Entity %i ZONE %d Type %s (%i bytes)\n", entityId, m_nID, aEntityNames[(uint32)elemType],
					syncStream->Tellg() - pBufferBeforeParse);
#else
				debug(" Entity %i ZONE %d Type %s (%i bytes)", entityId, m_nID, aEntityNames[(uint32)elemType],
					syncStream->Tellg() - pBufferBeforeParse);
#endif
#if !defined(FINAL) && !defined(MASTER)
				debug("ReadSyncFromStream BF: 0x%p AF: 0x%p TT: %d\n", pBufferBeforeParse, syncStream->Tellg(), syncStream->Tellg() - pBufferBeforeParse);
#endif
				pNewElem->ApplyClientSync(Game.m_pNetSession->m_nCurTime);
				if (elemType == eElementType::ELEMENT_TYPE_AUTOMOBILE
					|| elemType == eElementType::ELEMENT_TYPE_BIKE
					|| elemType == eElementType::ELEMENT_TYPE_PED
#if !defined(GTA_LIBERTY) && defined(FIX_BUGS)
					|| elemType == eElementType::ELEMENT_TYPE_HELI
					|| elemType == eElementType::ELEMENT_TYPE_BOAT
					|| elemType == eElementType::ELEMENT_TYPE_PLANE
					|| elemType == eElementType::ELEMENT_TYPE_BMX
					|| elemType == eElementType::ELEMENT_TYPE_QUADBIKE
#endif
#ifdef MULTIGAME_IMPROVEMENTS
					|| elemType == eElementType::ELEMENT_TYPE_OBJECT
#endif
					)
				{
					if (((sElementPhysical*)pNewElem)->GetPhysical()->bHasBlip)
						TheRadar->AddMultiplayerMarker(pNewElem->GetOwner(), pNewElem->GetID());
				}
				continue; // sync was read already for new element // <<----------------- !!
			} // end nil entity
		} // end create flag



		// NOTE!! leeds packets, MP_ENTITY_CREATE_FLAG read entity, if exists read basis here, if not exists create + near read entity flag?
		// NOT IN 0x8000 FLAG, can't merge with if(!pEnt)
		// non create, update
		uint8* pBufferBeforeParse = syncStream->Tellg();
		sElement* pElem = Game.GetEntityForHandle(nPeerID, entityId);
		if (!pElem) {
			assert(false && "wait, we non create flag element id (already created), but pElem is nil");
			debug("***MULTIPLAYER COCKUP*** TRYING TO ACCESS ENTITY %d %d\n", nPeerID, entityId);
			debug("multiplayer cockup !!! TRYING TO ACCESS ENTITY %d %d\n", nPeerID, entityId);
			Adhoc.SetHasError();
			LOG_ON_END();
			return;
		}
		sElementSync* pSync = pElem->GetSyncWithTime2(nState, basis).element;
		pElem->ReadSyncFromStream(syncStream, pSync); // << ------------------  read sync for exists element [elem delta/full sync]

		const char* aEntityNames[] = {
			"plyr",
			"ped ",
			"car ",
			"bike",
			"heli",
			"blip",
			"text",
			"pick",
#if !defined(GTA_LIBERTY) && defined(FIX_BUGS)
			"boat",
			"plne",
			"bmx ",
			"quad",
			"nm2d",
#endif
#ifdef MULTIGAME_IMPROVEMENTS
			"obj ",
#endif
		};

		eElementType elemType = pElem->GetType();
#ifdef FIX_BUGS
		INTEREST_ZONE_LOG(1, " DELTA STATE READ for new element Entity %i ZONE %d Type %s (%i bytes)\n", entityId, m_nID, aEntityNames[(uint32)elemType],
			syncStream->Tellg() - pBufferBeforeParse);
#else
		INTEREST_ZONE_LOG(1, " Entity %i ZONE %d Type %s (%i bytes)", entityId, m_nID, aEntityNames[(uint32)elemType],
			syncStream->Tellg() - pBufferBeforeParse);
#endif
		pElem->ApplyClientSync(Game.m_pNetSession->m_nCurTime);
		debug("DT %d, SYN %d\n", syncStream->Tellg() - pBufferBeforeParse, GetSyncSizeByElement(pElem));

	} // end sync stream loop


	if (zonePeerFrom.nSeqBitmask == zonePeerFrom.nExpectedSeqMask)
	{
		bool found = false;
		for (auto& entry : m_vAck) {
			if (entry.nPeerID == nPeerID) {
				found = true;
				//if (static_cast<int16>(it->nFrame - nState) < 0) it->nFrame = nState;
				if (entry.nFrame < nState) entry.nFrame = nState;
				ON_ACK_COLLECT(nPeerID, nState); // upd frame
				break;
			}
		}
		if (!found) {
			tAck newEntry;
			newEntry.nPeerID = nPeerID;
			newEntry.nFrame = nState;
			m_vAck.push_back(newEntry);
			ON_ACK_COLLECT(nPeerID, nState); // new
		}
	}
	LOG_ON_END();
}

void cInterestZone::RegisterElement(sElement* pElement) {
	m_vElements.push_back(pElement);
	pElement->RegisterZone(this);
}

void cInterestZone::RemoveElement(sElement* pElement) {
	tElementEntry elem;
	elem.nBasis = TheMPGame.m_pNetSession->m_nCurTime;
	elem.nEntityID = pElement->GetID();
	m_vEntities.push_back(elem);
	pElement->RegisterZone(nil);
#if 0
	auto it = std::find(m_vElements.begin(), m_vElements.end(), pElement);
	if (it != m_vElements.end()) {
		for (auto next = it + 1; next != m_vElements.end(); ++next) {
			if (*next != pElement) { // ?
				*it = *next;
				++it;
			}
		}
		m_vElements.resize(it - m_vElements.begin());
	}
#else
	m_vElements.erase(std::remove(m_vElements.begin(), m_vElements.end(), pElement), m_vElements.end());
#endif
}

void cInterestZone::DisconnectPeer(uint8 nPeerID) {
	debug("cInterestZone::DisconnectPeer %d\n", nPeerID);
	auto it = std::find_if(m_vPeers.begin(), m_vPeers.end(), [nPeerID](const tZonePeer& peer) {
		return peer.nPeerID == nPeerID;
		});
	if (it != m_vPeers.end()) {
		m_vPeers.erase(it);
	}
}

uint16 cInterestZone::PeerLastAck(uint8 nPeerID) {
	cMultiGame& Game = cMultiGame::Instance();
	for (const auto& peer : m_vPeers) {
		if (peer.nPeerID == nPeerID) {
			if (!peer.acks.empty()) {
				return peer.acks.back();
			}
			break;
		}
	}

	// tmp
	//for (std::vector<tZonePeer>::iterator it = m_vPeers.begin(); it != m_vPeers.end(); it++) {
	//	tZonePeer& peer = *it;
	//	if (peer.nPeerID != nPeerID) continue;
	//	return peer.acks.at(peer.acks.size() - 1);
	//}
	return Game.m_pNetSession->m_nCurTime;
}

bool cInterestZone::PeerLastAckEmpty(uint8 nPeerID) {
	for (const auto& peer : m_vPeers) {
		if (peer.nPeerID == nPeerID) {
			return peer.acks.empty();
		}
	}

	// tmp
	//for (std::vector<tZonePeer>::iterator it = m_vPeers.begin(); it != m_vPeers.end(); it++) {
	//	tZonePeer& peer = *it;
	//	if (peer.nPeerID == nPeerID) return peer.acks.empty();
	//}
	debug("cInterestZone::PeerLastAckEmpty, Peer id not found");
	return true;
}

void cInterestZone::AddPeer(uint8 nPeerID) {
	if (HasPeer(nPeerID))
		return;

	tZonePeer newPeer{};
	m_vPeers.push_back(newPeer);
	m_vPeers.back().nPeerID = nPeerID;
}

bool cInterestZone::HasPeer(uint8 nPeerId) {
	for (const auto& peer : m_vPeers) {
		if (peer.nPeerID == nPeerId) {
			return true;
		}
	}
	return false;
}

void cInterestZone::DiscardElement() {
	while (!m_vElements.empty()) {
		auto it = m_vElements.begin();
		sElement* pElement = (*it);
		if (pElement) {
			delete pElement;
		}
	}

	// tmp
	// crash modify m_vElements in dtor sElement
	//////for (auto* pElement : m_vElements) {
	//////	if (pElement) {
	//////		delete pElement;
	//////	}
	//////}
	//////m_vElements.clear();
}


#ifndef GTA_LIBERTY
cInterestZoneWrapper::cInterestZoneWrapper(int16 nID) : cInterestZone(nID) {
	;
}

cInterestZoneWrapper::~cInterestZoneWrapper() {
	;
}

void cInterestZoneWrapper::SendGameState(bool bIsInRange) {
	cInterestZone::SendGameState(bIsInRange);
}

void cInterestZoneWrapper::ReceiveGameState(uint32 nPeerID, uint16 nTime, sReadSyncStream* pSyncStream) {
	cMultiGame& Game = cMultiGame::Instance();
	PeerManager.GetPeerById(nPeerID)->UpdateAck();
	if (Game.IsSameGroup(GetID(), Game.LocalPlayerID()))
		cInterestZone::ReceiveGameState(nPeerID, nTime, pSyncStream);
}

bool cInterestZoneWrapper::GetNotInRange() {
	return false;
}

void cInterestZoneWrapper::Update() {
	if (m_vPeers.size() != (PeerManager.m_vPlayers.size() - 1))
		UpdatePlayers();
}

void cInterestZoneWrapper::UpdatePlayers() {
	cPeerManager& PeerMgr = PeerManager;
	cMultiGame& Game = cMultiGame::Instance();
	for (uint32 i = 0; i < PeerMgr.m_vPlayers.size(); i++) {
		sPeerState* peer = PeerMgr.GetPeerAt(i);
		if (peer->m_nID == Game.LocalPlayerID()) continue;
		if (!HasPeer(peer->m_nID)) AddPeer(peer->m_nID);
	}
}
#endif


cInterestZone* cInterestZoneManager::GetZoneByPeer(int32 nPeerID) // nPeerID can be -1 (MP_HOST_INDEX)
{
	auto it = m_vZones.find(static_cast<uint16>(nPeerID));  // BST search
	if (it != m_vZones.end()) {
		return it->second;
	}

	// Not found: create new
	cInterestZone* pNewZone = new cInterestZoneWrapper(static_cast<uint16>(nPeerID));
	auto inserted = m_vZones.insert({ static_cast<uint16>(nPeerID), pNewZone });
	//m_vZones[static_cast<uint16>(nPeerID)] = pNewZone;
	pNewZone->Update();
	return pNewZone;
}

void cInterestZoneManager::RemovePeerFromAllZones(int32 nPeerID) {
	debug("cInterestZoneManager::RemovePeerFromAllZones()\n");
	for (auto& pair : m_vZones) {
		pair.second->DisconnectPeer(nPeerID);
	}
}

void cInterestZoneManager::Terminate() {
	while (!m_vZones.empty()) {
		auto it = m_vZones.begin();
		cInterestZone* pZone = it->second;
		if (pZone) {
			m_vZones.erase(it);
			delete pZone;
		}
	}

	// tmp
	//for (auto& pair : m_vZones) {
	//	cInterestZone* pZone = pair.second;
	//	if (pZone)
	//		delete pZone;
	//}
	//m_vZones.clear();
}

#ifdef GTA_LIBERTY
void cInterestZoneManager::UpdatePeer(int32 nPeerID, int32 x, int32 y) {
	TODO();
	TODO();
	debug("cInterestZoneManager::UpdatePeer() is disconnecting a peer\n");
	TODO();
}
#else
void cInterestZoneManager::UpdatePeer(int32 nPeerID) {
	for (auto& pair : m_vZones) {
		cInterestZone* pZone = pair.second;
		if (pZone) {
			PeerManager.UpdateTeamPeerGroups();
			pZone->Update();
		}
	}
}
#endif

bool cInterestZoneManager::RemoveZone(uint16 nID) {
	auto it = m_vZones.find(nID);
	if (it == m_vZones.end()) return false;
	cInterestZone* pZone = it->second;
	m_vZones.erase(it);
	return true;
}

/* TODO: LCS */
#ifdef GTA_LIBERTY
void cInterestZoneManager::UpdatePlayer(int32 posX, int32 posY) {
	TODO();
	TODO();
	TODO();
}
#else
void cInterestZoneManager::UpdatePlayer() {
	for (auto& pair : m_vZones)
		pair.second->SendGameState(!pair.second->GetNotInRange());
}
#endif

cInterestZoneManager::~cInterestZoneManager() {
	Terminate();
	m_vZones.clear();
}
