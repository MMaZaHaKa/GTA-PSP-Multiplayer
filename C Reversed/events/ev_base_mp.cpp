/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once
#include "multiplayer/MultiGame.h"
#include "multiplayer/net/packet.h"
#include "multiplayer/natives/public.h"
#include "multiplayer/events/public.h"
#include "multiplayer/elements/sSyncStream.h"
#include "multiplayer/net/emu/Utils.h"
#ifndef GTA_PSP
#include "multiplayer/net/emu/sceNetAdhoc.h"
#endif

#include "common.h"
#include "World.h"
#include "Messages.h"
#include "Text.h"

void dump_packet_data(net::pckt_base& packet, const char* str, int32 colour)
{
	std::string packethex;
	DataToHexString(10, (uintptr_t)&packet, (uint8_t*)&packet, packet.pckt_size, &packethex); // packet dump
	//NOTICE_LOG(Log::sceNet, "(%s) HASH: 0x%X PACKET Dump %d [\"%s\"] (packet:0x%p  bytes:%d):\n%s\n", str, packet.CalcHash(), packet.pckt_id,
	//	gtMP_PacketIDs.GetPacketName(packet.pckt_id), &packet, packet.pckt_size, packethex.c_str());
	//MULTIGAME_SLOT_LOG(1, 2, "(%s) HASH: 0x%X PACKET Dump %d [\"%s\"] (packet:0x%p  bytes:%d):\n%s\n", str, packet.CalcHash(), packet.pckt_id,
	//	gtMP_PacketIDs.GetPacketName(packet.pckt_id), &packet, packet.pckt_size, packethex.c_str());
	SetConsoleColor(colour);
	printf("(%s) HASH: 0x%X PACKET Dump %d [\"%s\"] (packet:0x%p  bytes:%d):\n%s\n", str, packet.CalcHash(), packet.pckt_id,
		gtMP_PacketIDs.GetPacketName(packet.pckt_id), &packet, packet.pckt_size, packethex.c_str());
	SetConsoleColor(6);
}

// old on_recv_main [from cNetSession::DispatchMessages]
void cMultiGame::FireMessageHandler(net::pckt_base& packet, int sender, uint16 time, bool bFromRing)
{
	sPeerState* peer = PeerManager.GetPeerById(sender);
	assert(peer && packet.pckt_id < net::packet_id_list_t::snPacketCount);
	if (!m_bIsConnected || !peer->IsConnected())
		return;

	cPacketDispatcherBase* dispatcher = m_tPacketsEventsCB[packet.pckt_id];
#if !defined(FINAL) && !defined(MASTER)
	MULTIGAME_SLOT_LOG(1, 1, "cMultiGame::FireMessageHandler(packet: \"%s\" [%d], sender:%d, time:%d, bFromRing:%d)\n",
		gtMP_PacketIDs.GetPacketName(packet.pckt_id), packet.pckt_id, sender, time, bFromRing);
#endif
	//assert(dispatcher); // triger on unsub info
	if (dispatcher)
		dispatcher->PerformDispatchPacket(packet, sender, time, bFromRing);
	else {
		SetConsoleColor(0);
		MULTIGAME_LOG(1, "AHUETb!! no dispatcher for cMultiGame::FireMessageHandler(packet: \"%s\" [%d], sender:%d, time:%d, bFromRing:%d)\n",
			gtMP_PacketIDs.GetPacketName(packet.pckt_id), packet.pckt_id, sender, time, bFromRing);
		SetConsoleColor(6);
	}
}

//void on_recv_ack(net::pckt_ack& packet, int sender, uint16 time, bool bFromRing) // ID 1
//{
//	MULTIGAME_UNIMPLEMENTED_EVENT();
//}

//void on_recv_info(net::pckt_info& packet, int sender, uint16 time, bool bFromRing) // ID 2
//{
//	MULTIGAME_UNIMPLEMENTED_EVENT();
//}

void MultigameKickPlayer(uint8 nID) // custom
{
	cMultiGame& Game = cMultiGame::Instance();
	if (!gIsMultiplayerGame || Game.m_pNetSession == nil) return;
	assert(nID != Game.LocalPlayerID() && "are u serious?");
	net::pckt_kick_player kick_packet{};
	kick_packet.pckt_size = sizeof(net::pckt_kick_player);
	kick_packet.pckt_id = gtMP_PacketIDs.kick_player.pckt_id;
	kick_packet.peer_id = nID;
	Game.SendMessagePriority(kick_packet, BROADCAST_PEER_GROUPID);
	Game.RemovePlayerFromGame(kick_packet.peer_id);
	//Game.m_pNetSession->UpdateSend(); // pdp spread buffer // cMultiGame::PerformInitialConnection()
}

void on_recv_kick_player(net::pckt_kick_player& packet, int sender, uint16 time, bool bFromRing) // ID 4
{
	debug("Server tells me to kick peer %d\n", packet.peer_id);
	cMultiGame::Instance().RemovePlayerFromGame(packet.peer_id);
}

void MultigameRequestKickPlayer(uint8 nID) // custom, guess leeds send in cMultiGame
{
	cMultiGame& Game = cMultiGame::Instance();
	if (!gIsMultiplayerGame || Game.m_pNetSession == nil) return;
	assert(nID != Game.LocalPlayerID() && "are u serious?");
	net::pckt_request_kick_player packet{};
	packet.pckt_size = sizeof(net::pckt_request_kick_player);
	packet.pckt_id = gtMP_PacketIDs.request_kick_player.pckt_id;
	packet.peer_id = nID;
	if (cAdhoc::Instance().IsHost())
		on_recv_request_kick_player(packet, Game.LocalPlayerID(), 0, true);
	else
		Game.SendMessagePriority(packet, FindPlayerHostID());
	//Game.SendMessagePriority(packet, BROADCAST_PEER_GROUPID); // append packet to queue
	//Game.m_pNetSession->UpdateSend(); // pdp spread buffer // cMultiGame::PerformInitialConnection()
}

void on_recv_request_kick_player(net::pckt_request_kick_player& packet, int sender, uint16 time, bool bFromRing) // ID 5
{
	cMultiGame& Game = cMultiGame::Instance();
	debug("Handling request kick player\n");
	if (!cAdhoc::Instance().IsHost()) return;
	debug("Client %d requests the server to kick peer %d\n", sender, packet.peer_id);
	net::pckt_kick_player kick_packet{};
	kick_packet.pckt_size = sizeof(net::pckt_kick_player);
	kick_packet.pckt_id = gtMP_PacketIDs.kick_player.pckt_id;
	kick_packet.peer_id = packet.peer_id;
	Game.SendMessagePriority(kick_packet, BROADCAST_PEER_GROUPID);
	Game.RemovePlayerFromGame(packet.peer_id);
}

bool bNoChangeTeamScore = false; // guessed
void on_recv_set_team_score(net::pckt_set_team_score& packet, int sender, uint16 time, bool bFromRing) // ID 6
{
	cMultiGame& Game = cMultiGame::Instance();
	if (!bNoChangeTeamScore)
		Game.m_anTeamScore[packet.team_id] = packet.score;
}

void on_recv_send_game_event(net::pckt_send_game_event& packet, int sender, uint16 time, bool bFromRing) // ID 7
{
	// TODO: this maybe was implemented inline by orig authors :/
	TheMPGame.m_pEventStack->push(packet.event);
}


// old on_recv_game_state
void cMultiGame::OnGameStateChange(net::pckt_game_state& packet, int sender, uint16 time, bool bFromRing) // ID 14
{
	//DUMP_PACKET(packet, "RECV", 3);

	bool bOverflow = false;
#ifdef GTA_LIBERTY
	if (!m_pNetSession->m_vPeers.empty() && m_pNetSession->m_vPeers[LocalPlayerID()] != nil) {
		bOverflow = m_pNetSession->m_vPeers[LocalPlayerID()]->bSendOverflow;
	}
	if (bOverflow || !m_pNetSession->IsPeerConnected(LocalPlayerID()))
#else
	if (!m_pNetSession->m_vPeers.empty() && m_pNetSession->m_vPeers[LocalPlayerID()] != nil) {
		bOverflow = PeerManager.GetPeerById(LocalPlayerID())->bSendOverflow;
	}
	if (bOverflow || !PeerManager.IsPeerConnected(LocalPlayerID()))
#endif
	{
		debug("We are gone do NOT update !!!");
		return;
	}

	sReadSyncStream readStream;
	readStream.sequence = packet.sequence;
	readStream.m_pBuffer = (uint8*)((&packet) + 1); // after packet
	readStream.m_pBufferEnd = ((uint8*)&packet) + packet.pckt_size; // from packet start, not m_pBuffer
	cInterestZone* pZone = m_ZoneManager.FindZone(packet.zone); // inline stl tool
	if (pZone) pZone->ReceiveGameState(sender, time, &readStream); // read all syncs
}

//void on_recv_heart_beat(net::pckt_heart_beat& packet, int sender, uint16 time, bool bFromRing) // ID 16
//{
//	MULTIGAME_UNIMPLEMENTED_EVENT();
//}

// old on_recv_transfer_entity
#ifdef GTA_LIBERTY
// TODO: RemoveElement InsertElement
void cMultiGame::OnTransferEntity(net::pckt_transfer_entity& packet, int sender, uint16 time, bool bFromRing) // ID 17
{
	if (packet.src == LocalPlayerID()) {
		sElement* pElem = nil;
		auto& elems = m_vPlayers[LocalPlayerID()]->m_vElements;
		std::map<uint16, sElement*>::iterator it = elems.find(packet.elem);
		if (it != elems.end()) pElem = it->second;
		if (pElem && !pElem->m_bWasTransfered)
			pElem->TransferEntity(packet.dest);
	}
	else if (packet.src == sender && packet.dest == LocalPlayerID()) {
		TODO();
		TODO();
		TODO();
		if (m_vPlayers[packet.src] != nil)
		{
			int32 newElemId = GetNextElementID();
			//if (newElemId != -1) {
			//	sElement* pElem = nil;
			//	auto& elems = m_vPlayers[packet.src]->m_vElements;
			//	std::map<uint16, sElement*>::iterator it = elems.find(packet.elem);
			//	if (it != elems.end()) pElem = it->second;
			//	srcPeer->RemoveElement(pElem);
			//	if (pElem) {
			//		pElem->m_nPrevOwnerID = pElem->GetOwner();
			//		pElem->m_nPrevID = pElem->GetID();
			//		pElem->ReceiveEntity(packet.dest, newElemId, m_pNetSession->m_nCurTime);
			//		mp_lsc_transfer_entity(pElem->m_nPrevOwnerID, pElem->m_nPrevID, pElem->GetOwner(), pElem->GetID());
			//	}
			//	PeerMgr.GetSelfPeer()->InsertElement(pElem);
			//}
		}
	}
}
#else
void cMultiGame::OnTransferEntity(net::pckt_transfer_entity& packet, int sender, uint16 time, bool bFromRing) // ID 17
{
	cPeerManager& PeerMgr = PeerManager;
	if (packet.src == LocalPlayerID()) { // i need send my entity
		sElement* pElem = PeerMgr.GetSelfPeer()->FindElement(packet.elem);
		if (pElem && !pElem->m_bWasTransfered)
			pElem->TransferEntity(packet.dest);
	}
	else if (packet.src == sender && packet.dest == LocalPlayerID()) { // this entity for me
		if (PeerMgr.IsPeerConnected(packet.src))
		{
			int32 newElemId = GetNextElementID(); // LCS check -1
			sPeerState* srcPeer = PeerMgr.GetPeerById(packet.src);
			sElement* pElem = srcPeer->FindElement(packet.elem);
			srcPeer->RemoveElement(pElem);
			if (pElem) {
				pElem->m_nPrevOwnerID = pElem->GetOwner();
				pElem->m_nPrevID = pElem->GetID();
				pElem->ReceiveEntity(packet.dest, newElemId, m_pNetSession->m_nCurTime);
				lsc_transfer_tracked_entity(pElem->m_nPrevOwnerID, pElem->m_nPrevID, pElem->GetOwner(), pElem->GetID());
			}
			PeerMgr.GetSelfPeer()->InsertElement(pElem);
		}
	}
}
#endif

//void on_recv_clock(net::pckt_clock& packet, int sender, uint16 time, bool bFromRing) // ID 18
//{
//	MULTIGAME_UNIMPLEMENTED_EVENT();
//}

void on_recv_game_time(net::pckt_game_time& packet, int sender, uint16 time, bool bFromRing) // ID 19
{
	cMultiGame& Game = cMultiGame::Instance();
	Game.SetTimeMinutes(packet.time.min);
	Game.SetTimeSeconds(packet.time.sec);
	Game.SetGameElapsedMs(packet.elapsedMs);
	Game.SetTeamGameTime(static_cast<int32>(eGameTeam::TEAM_A), packet.nTeamATime);
	Game.SetTeamGameTime(static_cast<int32>(eGameTeam::TEAM_B), packet.nTeamBTime);
	Game.m_bTimeHasSync = true;
}

void on_recv_print_now(net::pckt_print_now& packet, int sender, uint16 time, bool bFromRing) // ID 51
{
	wchar* pText = TheText.Get(packet.key);
	CMessages::AddMessageJumpQ(pText, packet.time, packet.flag);
}

// OLD mp_pkt_recv_sync_cutscene
void on_recv_msg_ready_for_cutscene(net::pckt_msg_ready_for_cutscene& packet, int sender, uint16 time, bool bFromRing) // ID 59
{
	debug("***** Player %d is ready to start cutscene\n", sender);
	TheMPGame.SetCutscenePlaying(sender);
}

#ifndef GTA_LIBERTY
void on_recv_ack_entity_create(net::pckt_ack_entity_create& packet, int sender, uint16 time, bool bFromRing) // ID 60
{
	cMultiGame& Game = cMultiGame::Instance();
	sElement* pElement = Game.GetEntityForHandle(Game.LocalPlayerID(), packet.entityId);
	if (!pElement) return;
#ifdef FIX_BUGS
	if (pElement->HasCapability(sElementPhysical::Capability())) ((sElementPhysical*)pElement)->AddAckPeerID(sender);
#else
	sElementPhysical* pElementPhy = pElement->HasCapability(sElementPhysical::Capability()) ? (sElementPhysical*)pElement : nil;
	pElementPhy->AddAckPeerID(sender);
#endif
}

void on_recv_sync_peer_group(net::pckt_sync_peer_group& packet, int sender, uint16 time, bool bFromRing) // ID 61
{
	cMultiGame& Game = cMultiGame::Instance();
	cPeerManager& PeerMgr = PeerManager;
	Game.m_pNetSession->CreatePeerGroup(packet.group);
	for (uint32 i = 0; i < MP_MAX_NUM_PEERS; i++)
	{
		if ((packet.peer_mask & (1 << i)))
		{
			if (PeerMgr.IsPeerConnected(i))
				Game.m_pNetSession->RegisterGroupPeer(packet.group, i);
		}
	}
	Game.m_ZoneManager.GetZoneByPeer(packet.group)->Update();
	Game.m_pNetSession->PrintAllPeerGroups();
}

#if !defined(FINAL) && !defined(MASTER)
void DebugMultigameTriggerError() // snd from peer where error, guess leeds send in cMultiGame
{
	cMultiGame& Game = cMultiGame::Instance();
	if (!gIsMultiplayerGame || Game.m_pNetSession == nil) return;
	debug("DebugMultigameTriggerError()\n");
	net::pckt_debug_break packet{};
	packet.pckt_size = sizeof(net::pckt_debug_break);
	packet.pckt_id = gtMP_PacketIDs.debug_break.pckt_id;
	Game.SendMessagePriority(packet, BROADCAST_PEER_GROUPID); // append packet to queue
	Game.m_pNetSession->UpdateSend(); // pdp spread buffer // cMultiGame::PerformInitialConnection()
}
#endif

void on_recv_debug_break(net::pckt_debug_break& packet, int sender, uint16 time, bool bFromRing) // ID 62
{
#ifdef DEBUG
	SetConsoleColor(0);
	//debug("[MP] on_recv_debug_break(S%d) [put a breakpoint here]\n", sender); // custom
	debug("[MP] i got pckt_debug_break, STOPPING THE GAME!!\n"); // custom, lets read last log (point to stop coreloop, save logs to journal, probably)
	//__debugbreak(); // in release crash
#ifndef GTA_PSP
	__NetAdhocServerShutdown(); // optional
#endif
	std::this_thread::sleep_for(std::chrono::hours(100)); // i think pckt_debug_break use for stop sending on non error device for read last logs
	SetConsoleColor(6);
#else
	// PSP 0D 00 00 00 _break(0, 0);
#if defined(_MSC_VER)
	__debugbreak();
#elif defined(__GNUC__) || defined(__clang__)
	__builtin_trap();
#endif
#endif
}

void on_recv_msg_create_lua_object(net::pckt_msg_create_lua_object& packet, int sender, uint16 time, bool bFromRing) // ID 64
{
	cMultiGame& pGame = cMultiGame::Instance();
	if (packet.isDestroy)
		pGame.DestroyLuaObject();
	if (packet.isCreate) // 1 && 1 recreate
		pGame.CreateLuaObject(packet.object_id, packet.pos);
}

void on_recv_msg_server_ready_to_go(net::pckt_msg_server_ready_to_go& packet, int sender, uint16 time, bool bFromRing) // ID 65
{
	cMultiGame& Game = cMultiGame::Instance();
	Game.SetIsServerReadyToGo(packet.bIsServerReadyToGo);
}
#endif

