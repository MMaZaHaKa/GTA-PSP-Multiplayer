/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"
#include "sceNetAdhocMatching.h"
#include "sceNetAdhoc.h"
#include "NetAdhocCommon.h"
#include "Utils.h"
#include "MultiGame.h" // config

#include <deque>
#include <algorithm>

#include "PSPErrorCodes.h"

std::vector<SceUID> matchingThreads;
std::deque<MatchingArgs> matchingEvents;

bool netAdhocMatchingInited;
int adhocMatchingEventDelay = 30000; //30000

int netAdhocMatchingStarted = 0;


void __UpdateMatchingHandler(const MatchingArgs& ArgsPtr) {
	std::lock_guard<std::recursive_mutex> adhocGuard(adhocEvtMtx);
	matchingEvents.push_back(ArgsPtr);
}

//!!!! PC modified
// Matching callback is void function: typedef void(*SceNetAdhocMatchingHandler)(int id, int event, SceNetEtherAddr * peer, int optlen, void * opt);
// Important! The MIPS call need to be fully executed before the next MIPS call invoked, as the game (ie. DBZ Tag Team) may need to prepare something for the next callback event to use
// Note: Must not lock peerlock within this function to prevent race-condition with other thread whos owning peerlock and trying to lock context->eventlock owned by this thread
void notifyMatchingHandler(SceNetAdhocMatchingContext* context, ThreadMessage* msg, void* opt, uint32_t& bufAddr, uint32_t& bufLen, uint32_t* args) {
	// Don't share buffer address space with other mipscall in the queue since mipscalls aren't immediately executed
	MatchingArgs argsNew = { 0 };
	uintptr_t dataBufLen = msg->optlen + 8; //max(bufLen, msg->optlen + 8);
	void* dataBuf = malloc(dataBufLen);	// We will free this memory after returning from mipscall. FIXME: Are these buffers supposed to be taken/pre-allocated from the memory pool during sceNetAdhocMatchingInit?
	argsNew.bufAddr = (uintptr_t)dataBuf;
	uint8_t* dataPtr = (uint8_t*)dataBuf;
	if (dataPtr) {
		//memcpy(dataPtr, &msg->mac, sizeof(msg->mac)); // ppsspp
		memcpy(dataPtr, &msg->mac, sizeof(SceNetEtherAddr));
		if (msg->optlen > 0)
			memcpy(dataPtr + 8, opt, msg->optlen);
		//argsNew.data[1] = msg->opcode;
		//argsNew.data[2] = dataBufAddr;
		//argsNew.data[3] = msg->optlen;
		//argsNew.data[4] = dataBufAddr + 8; // OptData Addr

		// PC
		argsNew.event = msg->opcode;
		argsNew.optlen = msg->optlen;
		argsNew.optAddr = (uintptr_t)(dataPtr + 8); // OptData Addr
	}
	else {
		//argsNew.data[1] = PSP_ADHOC_MATCHING_EVENT_ERROR;
		////argsNew.data[2] = dataBufAddr; // FIXME: Is the MAC address mandatory (ie. can't be null pointer) even for EVENT_ERROR? Where should we put this MAC data in the case we failed to allocate the memory? may be on the memory pool?
	
		// PC
		argsNew.event = PSP_ADHOC_MATCHING_EVENT_ERROR;
	}
	//argsNew.data[0] = context->id;
	//argsNew.data[5] = (uint32_t)context->handler.entryPoint; //not part of callback argument, just borrowing a space to store callback address so i don't need to search the context first later
	
	// PC
	argsNew.id = context->id;
	argsNew.handler = (uintptr_t)context->handler.entryPoint; //not part of callback argument, just borrowing a space to store callback address so i don't need to search the context first later

	// ScheduleEvent_Threadsafe_Immediate seems to get mixed up with interrupt (returning from mipscall inside an interrupt) and getting invalid address before returning from interrupt
	__UpdateMatchingHandler(argsNew);
}

// Using matchingId = -1 to delete all matching events
//void deleteMatchingEvents(const int matchingId = -1) {
//	for (auto it = matchingEvents.begin(); it != matchingEvents.end(); ) {
//		if (matchingId < 0 || it->data[0] == matchingId) {
//			if (it->data[2] != 0)
//				free((void*)it->data[2]);
//			it = matchingEvents.erase(it);
//		}
//		else
//			++it;
//	}
//}

// PC
void deleteMatchingEvents(const int matchingId = -1) {
	for (auto it = matchingEvents.begin(); it != matchingEvents.end(); ) {
		if (matchingId < 0 || it->id == matchingId) {
			if (it->bufAddr != 0)
				free((void*)it->bufAddr);
			it = matchingEvents.erase(it);
		}
		else
			++it;
	}
}

/**
* Broadcast Ping Message to other Matching Users
* @param context Matching Context Pointer
*/
void broadcastPingMessage(SceNetAdhocMatchingContext* context) {
	// Ping Opcode
	uint8_t ping = PSP_ADHOC_MATCHING_PACKET_PING;

	// Lock the peer
	std::lock_guard<std::recursive_mutex> peer_guard(peerlock);

	// Send Broadcast
	// FIXME: Not sure whether this PING supposed to be sent only to AdhocMatching members or to everyone in Adhocctl Group, since we already pinging the AdhocServer to avoid getting kicked out of Adhocctl Group
	auto peer = friends; // Use context->peerlist if only need to send to AdhocMatching members
	for (; peer != NULL; peer = peer->next) {
		// Skipping soon to be removed peer
		if (peer->last_recv == 0)
			continue;

		uint16_t port = context->port;
		auto it = (*context->peerPort).find(peer->mac_addr);
		if (it != (*context->peerPort).end())
			port = it->second;

		context->socketlock->lock();
		sceNetAdhocPdpSend(context->socket, (unsigned char*)&peer->mac_addr, port, &ping, (uint32_t)sizeof(ping), 0, ADHOC_F_NONBLOCK);
		context->socketlock->unlock();
	}
}

/**
* Broadcast Hello Message to other Matching Users
* @param context Matching Context Pointer
*/
void broadcastHelloMessage(SceNetAdhocMatchingContext* context) {
	static uint8_t* hello = NULL;
	static int32_t len = -5;

	// Allocate Hello Message Buffer, reuse when necessary
	if ((int32_t)context->hellolen > len) {
		uint8_t* tmp = (uint8_t*)realloc(hello, 5LL + context->hellolen);
		if (tmp != NULL) {
			hello = tmp;
			len = context->hellolen;
		}
	}

	if (hello == NULL) {
		// Failed to allocate the Hello Message Buffer
		return;
	}

	// Hello Opcode
	hello[0] = PSP_ADHOC_MATCHING_PACKET_HELLO;

	// Hello Data Length (have to memcpy this to avoid cpu alignment crash)
	memcpy(hello + 1, &context->hellolen, sizeof(context->hellolen));

	// FIXME: When using JPCSP + prx files the data being sent have a header of 12 bytes instead of 5 bytes: 
	// [01(always 1? size of the next data? or combined with next byte as U16_BE opcode?) 01(matching opcode, or combined with previous byte as U16_BE opcode?) 01 E0(size of next data + hello data in big-endian/U16_BE) 00 0F 42 40(U32_BE? time?) 00 0F 42 40(U32_BE? time?)], 
	// followed by hello data (0x1D8 bytes of opt data, based on Ys vs. Sora no Kiseki), and followed by 16 bytes of (optional?) footer [01 00 00 .. 00 00](footer doesn't exist if the size after opcode is 00 00)

	// Copy Hello Data
	if (context->hellolen > 0) memcpy(hello + 5, context->hello, context->hellolen);

	std::string hellohex;
	DataToHexString(10, 0, context->hello, context->hellolen, &hellohex); // broadcast
	DEBUG_LOG(Log::sceNet, "HELLO Dump (%d bytes):\n%s", context->hellolen, hellohex.c_str());

	// Send Broadcast, so everyone know we have a room here
	peerlock.lock();
	SceNetAdhocctlPeerInfo* peer = friends;
	for (; peer != NULL; peer = peer->next) {
		// Skipping soon to be removed peer
		if (peer->last_recv == 0)
			continue;

		uint16_t port = context->port;
		auto it = (*context->peerPort).find(peer->mac_addr);
		if (it != (*context->peerPort).end())
			port = it->second;

		context->socketlock->lock();
		sceNetAdhocPdpSend(context->socket, (unsigned char*)&peer->mac_addr, port, hello, 5 + context->hellolen, 0, ADHOC_F_NONBLOCK);
		context->socketlock->unlock();
	}
	peerlock.unlock();
}

/**
* Send Accept Packet to Player
* @param context Matching Context Pointer
* @param mac Target Player MAC
* @param optlen Optional Data Length
* @param opt Optional Data
*/
void sendAcceptPacket(SceNetAdhocMatchingContext* context, SceNetEtherAddr* mac, int optlen, void* opt) {
	// Lock the peer
	std::lock_guard<std::recursive_mutex> peer_guard(peerlock);

	// Find Peer
	SceNetAdhocMatchingMemberInternal* peer = findPeer(context, mac);

	if (peer == NULL || (peer->state != PSP_ADHOC_MATCHING_PEER_CHILD && peer->state != PSP_ADHOC_MATCHING_PEER_P2P)) {
		// Not found
		return;
	}

	// Required Sibling Buffer
	uint32_t siblingbuflen = 0;

	// Parent Mode
	if (context->mode == PSP_ADHOC_MATCHING_MODE_PARENT) siblingbuflen = (uint32_t)sizeof(SceNetEtherAddr) * (countConnectedPeers(context) - 2);

	// Sibling Count
	int siblingcount = siblingbuflen / sizeof(SceNetEtherAddr);

	// Allocate Accept Message Buffer
	uint8_t* accept = (uint8_t*)malloc(9LL + optlen + siblingbuflen);

	if (accept == NULL) {
		// Failed to allocate the Accept Message Buffer
		return;
	}

	// Accept Opcode
	accept[0] = PSP_ADHOC_MATCHING_PACKET_ACCEPT;

	// Optional Data Length
	memcpy(accept + 1, &optlen, sizeof(optlen));

	// Sibling Count
	memcpy(accept + 5, &siblingcount, sizeof(siblingcount));

	// Copy Optional Data
	if (optlen > 0) memcpy(accept + 9, opt, optlen);

	// Parent Mode Extra Data required
	if (context->mode == PSP_ADHOC_MATCHING_MODE_PARENT && siblingcount > 0) {
		// Create MAC Array Pointer
		uint8_t* siblingmacs = (uint8_t*)(accept + 9 + optlen);

		// MAC Writing Pointer
		int i = 0;

		// Iterate Peer List
		SceNetAdhocMatchingMemberInternal* item = context->peerlist;
		for (; item != NULL; item = item->next) {
			// Ignore Target
			if (item == peer) continue;

			// Copy Child MAC
			if (item->state == PSP_ADHOC_MATCHING_PEER_CHILD) {
				// Clone MAC the stupid memcpy way to shut up PSP CPU
				memcpy(siblingmacs + sizeof(SceNetEtherAddr) * i++, &item->mac, sizeof(SceNetEtherAddr));
			}
		}
	}

	// Send Data
	context->socketlock->lock();
	sceNetAdhocPdpSend(context->socket, (unsigned char*)mac, (*context->peerPort)[*mac], accept, 9 + optlen + siblingbuflen, 0, ADHOC_F_NONBLOCK);
	context->socketlock->unlock();

	// Free Memory
	free(accept);

	// Spawn Local Established Event
	spawnLocalEvent(context, PSP_ADHOC_MATCHING_EVENT_ESTABLISHED, mac, 0, NULL);
}

/**
* Send Join Packet to Player
* @param context Matching Context Pointer
* @param mac Target Player MAC
* @param optlen Optional Data Length
* @param opt Optional Data
*/
void sendJoinPacket(SceNetAdhocMatchingContext* context, SceNetEtherAddr* mac, int optlen, void* opt) {
	// Lock the peer
	std::lock_guard<std::recursive_mutex> peer_guard(peerlock);

	// Find Peer
	SceNetAdhocMatchingMemberInternal* peer = findPeer(context, mac);

	if (peer == NULL || peer->state != PSP_ADHOC_MATCHING_PEER_OUTGOING_REQUEST) {
		// Valid peer not found
		return;
	}

	// Allocate Join Message Buffer
	uint8_t* join = (uint8_t*)malloc(5LL + optlen);

	if (join == NULL) {
		// Failed to allocate the Join Message Buffer
		return;
	}

	// Join Opcode
	join[0] = PSP_ADHOC_MATCHING_PACKET_JOIN;

	// Optional Data Length
	memcpy(join + 1, &optlen, sizeof(optlen));

	// Copy Optional Data
	if (optlen > 0) memcpy(join + 5, opt, optlen);

	// Send Data
	context->socketlock->lock();
	sceNetAdhocPdpSend(context->socket, (unsigned char*)mac, (*context->peerPort)[*mac], join, 5 + optlen, 0, ADHOC_F_NONBLOCK);
	context->socketlock->unlock();

	// Free Memory
	free(join);
}

/**
* Send Cancel Packet to Player
* @param context Matching Context Pointer
* @param mac Target Player MAC
* @param optlen Optional Data Length
* @param opt Optional Data
*/
void sendCancelPacket(SceNetAdhocMatchingContext* context, SceNetEtherAddr* mac, int optlen, void* opt) {
	// Lock the peer
	std::lock_guard<std::recursive_mutex> peer_guard(peerlock);

	// Allocate Cancel Message Buffer
	uint8_t* cancel = (uint8_t*)malloc(5LL + optlen);

	// Allocated Cancel Message Buffer
	if (cancel != NULL) {
		// Cancel Opcode
		cancel[0] = PSP_ADHOC_MATCHING_PACKET_CANCEL;

		// Optional Data Length
		memcpy(cancel + 1, &optlen, sizeof(optlen));

		// Copy Optional Data
		if (optlen > 0) memcpy(cancel + 5, opt, optlen);

		// Send Data
		context->socketlock->lock();
		sceNetAdhocPdpSend(context->socket, (unsigned char*)mac, (*context->peerPort)[*mac], cancel, 5 + optlen, 0, ADHOC_F_NONBLOCK);
		context->socketlock->unlock();

		// Free Memory
		free(cancel);
	}

	// Find Peer
	SceNetAdhocMatchingMemberInternal* peer = findPeer(context, mac);

	if (peer == NULL) {
		// Peer not found
		return;
	}

	// Child Mode Fallback - Delete All
	if (context->mode == PSP_ADHOC_MATCHING_MODE_CHILD) {
		// Delete Peer List
		clearPeerList(context);
	}

	// Delete Peer
	else {
		// Instead of removing peer immediately, We should give a little time before removing the peer and let it timed out? so it can send the BYE packet when stopping AdhocMatching after Canceling it
		peer->lastping = CoreTiming::GetGlobalTimeUsScaled();
	}
}

/**
* Send Bulk Data Packet to Player
* @param context Matching Context Pointer
* @param mac Target Player MAC
* @param datalen Data Length
* @param data Data
*/
void sendBulkDataPacket(SceNetAdhocMatchingContext* context, SceNetEtherAddr* mac, int datalen, void* data) {
	// Lock the peer
	std::lock_guard<std::recursive_mutex> peer_guard(peerlock);

	// Find Peer
	SceNetAdhocMatchingMemberInternal* peer = findPeer(context, mac);

	if (peer == NULL) {
		// Invalid Peer
		return;
	}

	// Don't send if it's aborted
	//if (peer->sending == 0) return;

	// Allocate Send Message Buffer
	uint8_t* send = (uint8_t*)malloc(5LL + datalen);

	if (send == NULL) {
		// Failed to allocate the Send Message Buffer
		return;
	}

	// Send Opcode
	send[0] = PSP_ADHOC_MATCHING_PACKET_BULK;

	// Data Length
	memcpy(send + 1, &datalen, sizeof(datalen));

	// Copy Data
	memcpy(send + 5, data, datalen);

	// Send Data
	context->socketlock->lock();
	sceNetAdhocPdpSend(context->socket, (unsigned char*)mac, (*context->peerPort)[*mac], send, 5 + datalen, 0, ADHOC_F_NONBLOCK);
	context->socketlock->unlock();

	// Free Memory
	free(send);

	// Remove Busy Bit from Peer
	peer->sending = 0;

	// Spawn Data Event
	spawnLocalEvent(context, PSP_ADHOC_MATCHING_EVENT_DATA_ACK, mac, 0, NULL);
}

/**
* Tell Established Peers of new Child
* @param context Matching Context Pointer
* @param mac New Child's MAC
*/
void sendBirthPacket(SceNetAdhocMatchingContext* context, SceNetEtherAddr* mac) {
	// Lock the peer
	std::lock_guard<std::recursive_mutex> peer_guard(peerlock);

	// Find Newborn Child
	SceNetAdhocMatchingMemberInternal* newborn = findPeer(context, mac);

	if (newborn == NULL) {
		// Did not find Newborn Child
		return;
	}

	// Packet Buffer
	uint8_t packet[7];

	// Set Opcode
	packet[0] = PSP_ADHOC_MATCHING_PACKET_BIRTH;

	// Set Newborn MAC
	memcpy(packet + 1, mac, sizeof(SceNetEtherAddr));

	// Iterate Peers
	SceNetAdhocMatchingMemberInternal* peer = context->peerlist;
	for (; peer != NULL; peer = peer->next) {
		// Skip Newborn Child
		if (peer == newborn) continue;

		// Send only to children
		if (peer->state != PSP_ADHOC_MATCHING_PEER_CHILD) {
			continue;
		}

		// Send Packet
		context->socketlock->lock();
		int sent = sceNetAdhocPdpSend(context->socket, (unsigned char*)&peer->mac, (*context->peerPort)[peer->mac], packet, (uint32_t)sizeof(packet), 0, ADHOC_F_NONBLOCK);
		context->socketlock->unlock();

		// Log Send Success
		if (sent >= 0)
			INFO_LOG(Log::sceNet, "InputLoop: Sending BIRTH [%s] to %s", mac2str(mac).c_str(), mac2str(&peer->mac).c_str());
		else
			WARN_LOG(Log::sceNet, "InputLoop: Failed to Send BIRTH [%s] to %s", mac2str(mac).c_str(), mac2str(&peer->mac).c_str());
	}
}

/**
* Tell Established Peers of new Child
* @param context Matching Context Pointer
* @param mac New Child's MAC
*/
void sendDeathPacket(SceNetAdhocMatchingContext* context, SceNetEtherAddr* mac) {
	// Lock the peer
	std::lock_guard<std::recursive_mutex> peer_guard(peerlock);

	// Find abandoned Child
	SceNetAdhocMatchingMemberInternal* deadkid = findPeer(context, mac);

	if (deadkid == NULL) {
		// Did not find abandoned Child
		return;
	}

	// Packet Buffer
	uint8_t packet[7];

	// Set abandoned Child MAC
	memcpy(packet + 1, mac, sizeof(SceNetEtherAddr));

	// Iterate Peers
	SceNetAdhocMatchingMemberInternal* peer = context->peerlist;
	for (; peer != NULL; peer = peer->next) {
		// Skip dead Child? Or May be we should also tells the disconnected Child, that they have been disconnected from the Host (in the case they were disconnected because they went to PPSSPP settings for too long)
		if (peer == deadkid) {
			// Set Opcode
			packet[0] = PSP_ADHOC_MATCHING_PACKET_BYE;

			// Send Bye Packet
			context->socketlock->lock();
			sceNetAdhocPdpSend(context->socket, (unsigned char*)&peer->mac, (*context->peerPort)[peer->mac], packet, (uint32_t)sizeof(packet[0]), 0, ADHOC_F_NONBLOCK);
			context->socketlock->unlock();
		}
		else {
			// Send to other children
			if (peer->state == PSP_ADHOC_MATCHING_PEER_CHILD) {
				// Set Opcode
				packet[0] = PSP_ADHOC_MATCHING_PACKET_DEATH;

				// Send Death Packet
				context->socketlock->lock();
				sceNetAdhocPdpSend(context->socket, (unsigned char*)&peer->mac, (*context->peerPort)[peer->mac], packet, (uint32_t)sizeof(packet), 0, ADHOC_F_NONBLOCK);
				context->socketlock->unlock();
			}
		}
	}

	// Delete Peer
	deletePeer(context, deadkid);
}

/**
* Tell Established Peers that we're shutting the Networking Layer down
* @param context Matching Context Pointer
*/
void sendByePacket(SceNetAdhocMatchingContext* context) {
	// Lock the peer
	std::lock_guard<std::recursive_mutex> peer_guard(peerlock);

	// Iterate Peers
	SceNetAdhocMatchingMemberInternal* peer = context->peerlist;
	for (; peer != NULL; peer = peer->next) {
		// Peer of Interest
		if (peer->state == PSP_ADHOC_MATCHING_PEER_PARENT || peer->state == PSP_ADHOC_MATCHING_PEER_CHILD || peer->state == PSP_ADHOC_MATCHING_PEER_P2P || peer->state == PSP_ADHOC_MATCHING_PEER_CANCEL_IN_PROGRESS) {
			// Bye Opcode
			uint8_t opcode = PSP_ADHOC_MATCHING_PACKET_BYE;

			// Send Bye Packet
			context->socketlock->lock();
			sceNetAdhocPdpSend(context->socket, (unsigned char*)&peer->mac, (*context->peerPort)[peer->mac], &opcode, (uint32_t)sizeof(opcode), 0, ADHOC_F_NONBLOCK);
			context->socketlock->unlock();
		}
	}
}

/**
* Handle Ping Packet
* @param context Matching Context Pointer
* @param sendermac Packet Sender MAC
*/
void actOnPingPacket(SceNetAdhocMatchingContext* context, SceNetEtherAddr* sendermac) {
	// Find Peer
	SceNetAdhocMatchingMemberInternal* peer = findPeer(context, sendermac);

	// Found Peer
	if (peer != NULL) {
		// Update Receive Timer
		peer->lastping = CoreTiming::GetGlobalTimeUsScaled(); //time_now_d()*1000000.0;
	}
}

/**
* Handle Hello Packet
* @param context Matching Context Pointer
* @param sendermac Packet Sender MAC
* @param length Packet Length
*/
void actOnHelloPacket(SceNetAdhocMatchingContext* context, SceNetEtherAddr* sendermac, int32_t length) {
	// Interested in Hello Data
	if ((context->mode == PSP_ADHOC_MATCHING_MODE_CHILD && findParent(context) == NULL) || (context->mode == PSP_ADHOC_MATCHING_MODE_P2P && findP2P(context) == NULL)) {
		if (length < 5) {
			// Incomplete Packet Header
			return;
		}

		// Extract Optional Data Length
		int optlen = 0; memcpy(&optlen, context->rxbuf + 1, sizeof(optlen));

		if (optlen < 0 || length < (5 + optlen)) {
			// Invalid packet
			return;
		}

		// Set Default Null Data
		void* opt = NULL;

		// Extract Optional Data Pointer
		if (optlen > 0) opt = context->rxbuf + 5;

		// Find Peer
		SceNetAdhocMatchingMemberInternal* peer = findPeer(context, sendermac);

		// Peer not found
		if (peer == NULL) {
			// Allocate Memory
			peer = (SceNetAdhocMatchingMemberInternal*)malloc(sizeof(SceNetAdhocMatchingMemberInternal));

			// Allocated Memory
			if (peer != NULL) {
				// Clear Memory
				memset(peer, 0, sizeof(SceNetAdhocMatchingMemberInternal));

				// Copy Sender MAC
				peer->mac = *sendermac;

				// Set Peer State
				peer->state = PSP_ADHOC_MATCHING_PEER_OFFER;

				// Initialize Ping Timer
				peer->lastping = CoreTiming::GetGlobalTimeUsScaled(); //time_now_d()*1000000.0;

				peerlock.lock();
				// Link Peer into List
				peer->next = context->peerlist;
				context->peerlist = peer;
				peerlock.unlock();
			}
		}

		// Peer available now
		if (peer != NULL && peer->state != PSP_ADHOC_MATCHING_PEER_OUTGOING_REQUEST && peer->state != PSP_ADHOC_MATCHING_PEER_INCOMING_REQUEST) {
			std::string hellohex;
			DataToHexString(10, 0, (uint8_t*)opt, optlen, &hellohex); // actOnHelloPacket
			DEBUG_LOG(Log::sceNet, "HELLO Dump (%d bytes):\n%s", optlen, hellohex.c_str());

			// Spawn Hello Event. FIXME: HELLO event should not be triggered in the middle of joining? This will cause Bleach 7 to Cancel the join request
			spawnLocalEvent(context, PSP_ADHOC_MATCHING_EVENT_HELLO, sendermac, optlen, opt);
		}
	}
}

/**
* Handle Join Packet
* @param context Matching Context Pointer
* @param sendermac Packet Sender MAC
* @param length Packet Length
*/
void actOnJoinPacket(SceNetAdhocMatchingContext* context, SceNetEtherAddr* sendermac, int32_t length) {
	if (context->mode == PSP_ADHOC_MATCHING_MODE_CHILD) {
		// Child mode context
		return;
	}

	// We still got a unoccupied slot in our room (Parent / P2P)
	if ((context->mode == PSP_ADHOC_MATCHING_MODE_PARENT && countChildren(context) < (context->maxpeers - 1)) || (context->mode == PSP_ADHOC_MATCHING_MODE_P2P && findP2P(context) == NULL)) {
		// Complete Packet Header available
		if (length >= 5) {
			// Extract Optional Data Length
			int optlen = 0; memcpy(&optlen, context->rxbuf + 1, sizeof(optlen));

			// Complete Valid Packet available
			if (optlen >= 0 && length >= (5 + optlen)) {
				// Set Default Null Data
				void* opt = NULL;

				// Extract Optional Data Pointer
				if (optlen > 0) opt = context->rxbuf + 5;

				// Find Peer
				SceNetAdhocMatchingMemberInternal* peer = findPeer(context, sendermac);

				// If we got the peer in the table already and are a parent, there is nothing left to be done.
				// This is because the only way a parent can know of a child is via a join request...
				// If we thus know of a possible child, then we already had a previous join request thus no need for double tapping.
				if (peer != NULL && peer->lastping != 0 && context->mode == PSP_ADHOC_MATCHING_MODE_PARENT) {
					WARN_LOG(Log::sceNet, "Join Event(2) Ignored");
					return;
				}

				// New Peer
				if (peer == NULL) {
					// Allocate Memory
					peer = (SceNetAdhocMatchingMemberInternal*)malloc(sizeof(SceNetAdhocMatchingMemberInternal));

					// Allocated Memory
					if (peer != NULL) {
						// Clear Memory
						memset(peer, 0, sizeof(SceNetAdhocMatchingMemberInternal));

						// Copy Sender MAC
						peer->mac = *sendermac;

						// Set Peer State
						peer->state = PSP_ADHOC_MATCHING_PEER_INCOMING_REQUEST;

						// Initialize Ping Timer
						peer->lastping = CoreTiming::GetGlobalTimeUsScaled(); //time_now_d()*1000000.0;

						peerlock.lock();
						// Link Peer into List
						peer->next = context->peerlist;
						context->peerlist = peer;
						peerlock.unlock();

						// Spawn Request Event
						spawnLocalEvent(context, PSP_ADHOC_MATCHING_EVENT_REQUEST, sendermac, optlen, opt);

						// Return Success
						return;
					}
				}

				// Existing Peer (this case is only reachable for P2P mode)
				else {
					// Set Peer State
					peer->state = PSP_ADHOC_MATCHING_PEER_INCOMING_REQUEST;

					// Initialize Ping Timer
					peer->lastping = CoreTiming::GetGlobalTimeUsScaled();

					// Spawn Request Event
					spawnLocalEvent(context, PSP_ADHOC_MATCHING_EVENT_REQUEST, sendermac, optlen, opt);

					// Return Success
					return;
				}
			}
		}
	}

	WARN_LOG(Log::sceNet, "Join Event(2) Rejected");
	// Auto-Reject Player
	sendCancelPacket(context, sendermac, 0, NULL);
}

/**
* Handle Accept Packet
* @param context Matching Context Pointer
* @param sendermac Packet Sender MAC
* @param length Packet Length
*/
void actOnAcceptPacket(SceNetAdhocMatchingContext* context, SceNetEtherAddr* sendermac, uint32_t length) {
	if (context->mode == PSP_ADHOC_MATCHING_MODE_PARENT) {
		// Parent context
		return;
	}

	// Don't have a master yet
	if ((context->mode == PSP_ADHOC_MATCHING_MODE_CHILD && findParent(context) == NULL) || (context->mode == PSP_ADHOC_MATCHING_MODE_P2P && findP2P(context) == NULL)) {
		if (length < 9) {
			// Incomplete Packet Header
			return;
		}

		// Extract Optional Data Length
		int optlen = 0; memcpy(&optlen, context->rxbuf + 1, sizeof(optlen));

		// Extract Sibling Count
		int siblingcount = 0; memcpy(&siblingcount, context->rxbuf + 5, sizeof(siblingcount));

		if (optlen < 0 || length < (9LL + optlen + static_cast<long long>(sizeof(SceNetEtherAddr)) * siblingcount)) {
			// Invalid packet
			return;
		}

		// Set Default Null Data
		void* opt = NULL;

		// Extract Optional Data Pointer
		if (optlen > 0) opt = context->rxbuf + 9;

		// Sibling MAC Array Null Data
		SceNetEtherAddr* siblings = NULL;

		// Extract Optional Sibling MAC Array
		if (siblingcount > 0) siblings = (SceNetEtherAddr*)(context->rxbuf + 9 + optlen);

		// Find Outgoing Request
		SceNetAdhocMatchingMemberInternal* request = findOutgoingRequest(context);

		// We are waiting for an answer to our request...
		if (request == NULL) {
			return;
		}

		// Find Peer
		SceNetAdhocMatchingMemberInternal* peer = findPeer(context, sendermac);

		if (request != peer) {
			// It's not the answer we wanted!
			return;
		}

		// Change Peer State
		peer->state = (context->mode == PSP_ADHOC_MATCHING_MODE_CHILD) ? (PSP_ADHOC_MATCHING_PEER_PARENT) : (PSP_ADHOC_MATCHING_PEER_P2P);

		// Remove Unneeded Peer Information
		postAcceptCleanPeerList(context);

		// Add Sibling Peers
		if (context->mode == PSP_ADHOC_MATCHING_MODE_CHILD) {
			// Add existing siblings
			postAcceptAddSiblings(context, siblingcount, siblings);

			// Add Self Peer to the following position (using peer->state = 0 to identify as Self)
			addMember(context, &context->mac);
		}

		// IMPORTANT! The Event Order here is ok!
		// Internally the Event Stack appends to the front, so the order will be switched around.

		// Spawn Established Event
		spawnLocalEvent(context, PSP_ADHOC_MATCHING_EVENT_ESTABLISHED, sendermac, 0, NULL);

		// Spawn Accept Event
		spawnLocalEvent(context, PSP_ADHOC_MATCHING_EVENT_ACCEPT, sendermac, optlen, opt);
	}
}

/**
* Handle Cancel Packet
* @param context Matching Context Pointer
* @param sendermac Packet Sender MAC
* @param length Packet Length
*/
void actOnCancelPacket(SceNetAdhocMatchingContext* context, SceNetEtherAddr* sendermac, int32_t length) {
	// Find Peer
	SceNetAdhocMatchingMemberInternal* peer = findPeer(context, sendermac);

	if (peer == NULL) {
		// Interest Condition not fulfilled
		return;
	}

	if (length < 5) {
		// Incomplete Packet Header
		return;
	}

	// Extract Optional Data Length
	int optlen = 0; memcpy(&optlen, context->rxbuf + 1, sizeof(optlen));

	if (optlen < 0 || length < (5 + optlen)) {
		// Invalid packet
		return;
	}

	// Set Default Null Data
	void* opt = NULL;

	// Extract Optional Data Pointer
	if (optlen > 0) opt = context->rxbuf + 5;

	// Get Outgoing Join Request
	SceNetAdhocMatchingMemberInternal* request = findOutgoingRequest(context);

	// Child Mode
	if (context->mode == PSP_ADHOC_MATCHING_MODE_CHILD) {
		// Get Parent
		SceNetAdhocMatchingMemberInternal* parent = findParent(context);

		// Join Request denied
		if (request == peer) {
			// Spawn Deny Event
			spawnLocalEvent(context, PSP_ADHOC_MATCHING_EVENT_DENY, sendermac, optlen, opt);

			// Delete Peer from List
			//deletePeer(context, peer);
			peer->lastping = 0;
		}

		// Kicked from Room
		else if (parent == peer) {
			// Iterate Peers
			SceNetAdhocMatchingMemberInternal* item = context->peerlist;
			for (; item != NULL; item = item->next) {
				// Established Peer
				if (item->state == PSP_ADHOC_MATCHING_PEER_CHILD || item->state == PSP_ADHOC_MATCHING_PEER_PARENT) {
					// Spawn Leave / Kick Event
					spawnLocalEvent(context, PSP_ADHOC_MATCHING_EVENT_LEAVE, &item->mac, optlen, opt);
				}
			}

			// Delete Peer from List
			clearPeerList(context);
		}
	}

	// Parent Mode
	else if (context->mode == PSP_ADHOC_MATCHING_MODE_PARENT) {
		// Cancel Join Request
		if (peer->state == PSP_ADHOC_MATCHING_PEER_INCOMING_REQUEST) {
			// Spawn Request Cancel Event
			spawnLocalEvent(context, PSP_ADHOC_MATCHING_EVENT_CANCEL, sendermac, optlen, opt);

			// Delete Peer from List
			//deletePeer(context, peer);
			peer->lastping = 0;
		}

		// Leave Room
		else if (peer->state == PSP_ADHOC_MATCHING_PEER_CHILD) {
			// Spawn Leave / Kick Event
			spawnLocalEvent(context, PSP_ADHOC_MATCHING_EVENT_LEAVE, sendermac, optlen, opt);

			// Delete Peer from List
			//deletePeer(context, peer);
			peer->lastping = 0;
		}
	}

	// P2P Mode
	else {
		// Get P2P Partner
		SceNetAdhocMatchingMemberInternal* p2p = findP2P(context);

		// Join Request denied
		if (request == peer) {
			// Spawn Deny Event
			spawnLocalEvent(context, PSP_ADHOC_MATCHING_EVENT_DENY, sendermac, optlen, opt);

			// FIXME: Delete Peer from List?
			// Instead of removing the peer immediately, we should let it timedout, otherwise inviter in Crazy Taxi will wait forever without getting timedout, since handleTimeout need the peer data to exist.
			peer->lastping = 0;
		}

		// Kicked from Room
		else if (p2p == peer) {
			// Spawn Leave / Kick Event
			spawnLocalEvent(context, PSP_ADHOC_MATCHING_EVENT_LEAVE, sendermac, optlen, opt);

			// Delete Peer from List
			//deletePeer(context, peer);
			peer->lastping = 0;
		}

		// Cancel Join Request
		else if (peer->state == PSP_ADHOC_MATCHING_PEER_INCOMING_REQUEST) {
			// Spawn Request Cancel Event
			spawnLocalEvent(context, PSP_ADHOC_MATCHING_EVENT_CANCEL, sendermac, optlen, opt);

			// Delete Peer from List
			//deletePeer(context, peer);
			peer->lastping = 0;
		}
	}
}

/**
* Handle Bulk Data Packet
* @param context Matching Context Pointer
* @param sendermac Packet Sender MAC
* @param length Packet Length
*/
void actOnBulkDataPacket(SceNetAdhocMatchingContext* context, SceNetEtherAddr* sendermac, int32_t length) {
	// Find Peer
	SceNetAdhocMatchingMemberInternal* peer = findPeer(context, sendermac);

	// Established Peer
	if (peer != NULL && (
		(context->mode == PSP_ADHOC_MATCHING_MODE_PARENT && peer->state == PSP_ADHOC_MATCHING_PEER_CHILD) ||
		(context->mode == PSP_ADHOC_MATCHING_MODE_CHILD && (peer->state == PSP_ADHOC_MATCHING_PEER_CHILD || peer->state == PSP_ADHOC_MATCHING_PEER_PARENT)) ||
		(context->mode == PSP_ADHOC_MATCHING_MODE_P2P && peer->state == PSP_ADHOC_MATCHING_PEER_P2P)))
	{
		// Complete Packet Header available
		if (length > 5) {
			// Extract Data Length
			int datalen = 0; memcpy(&datalen, context->rxbuf + 1, sizeof(datalen));

			// Complete Valid Packet available
			if (datalen > 0 && length >= (5 + datalen)) {
				// Extract Data
				void* data = context->rxbuf + 5;

				// Spawn Data Event
				spawnLocalEvent(context, PSP_ADHOC_MATCHING_EVENT_DATA, sendermac, datalen, data);
			}
		}
	}
}

/**
* Handle Birth Packet
* @param context Matching Context Pointer
* @param sendermac Packet Sender MAC
* @param length Packet Length
*/
void actOnBirthPacket(SceNetAdhocMatchingContext* context, SceNetEtherAddr* sendermac, uint32_t length) {
	// Find Peer
	SceNetAdhocMatchingMemberInternal* peer = findPeer(context, sendermac);

	if (peer == NULL || context->mode != PSP_ADHOC_MATCHING_MODE_CHILD || peer != findParent(context)) {
		// Invalid Circumstances
		return;
	}
	if (length < (1 + sizeof(SceNetEtherAddr))) {
		// Incomplete packet
		return;
	}

	// Extract Child MAC
	SceNetEtherAddr mac;
	memcpy(&mac, context->rxbuf + 1, sizeof(SceNetEtherAddr));

	// Allocate Memory
	SceNetAdhocMatchingMemberInternal* sibling = (SceNetAdhocMatchingMemberInternal*)malloc(sizeof(SceNetAdhocMatchingMemberInternal));

	if (sibling == NULL) {
		// Failed to allocate memory
		return;
	}

	// Clear Memory
	memset(sibling, 0, sizeof(SceNetAdhocMatchingMemberInternal));

	// Save MAC Address
	sibling->mac = mac;

	// Set Peer State
	sibling->state = PSP_ADHOC_MATCHING_PEER_CHILD;

	// Initialize Ping Timer
	sibling->lastping = CoreTiming::GetGlobalTimeUsScaled(); //time_now_d()*1000000.0;

	peerlock.lock();

	// Link Peer
	sibling->next = context->peerlist;
	context->peerlist = sibling;

	peerlock.unlock();

	// Spawn Established Event. FIXME: ESTABLISHED event should only be triggered for Parent/P2P peer?
	//spawnLocalEvent(context, PSP_ADHOC_MATCHING_EVENT_ESTABLISHED, &sibling->mac, 0, NULL);
}

/**
* Handle Death Packet
* @param context Matching Context Pointer
* @param sendermac Packet Sender MAC
* @param length Packet Length
*/
void actOnDeathPacket(SceNetAdhocMatchingContext* context, SceNetEtherAddr* sendermac, uint32_t length) {
	// Find Peer
	SceNetAdhocMatchingMemberInternal* peer = findPeer(context, sendermac);

	if (peer == NULL || context->mode != PSP_ADHOC_MATCHING_MODE_CHILD || peer != findParent(context)) {
		// Invalid Circumstances
		return;
	}

	if (length < (1 + sizeof(SceNetEtherAddr))) {
		// Incomplete packet
		return;
	}

	// Extract Child MAC
	SceNetEtherAddr mac;
	memcpy(&mac, context->rxbuf + 1, sizeof(SceNetEtherAddr));

	// Find Peer
	SceNetAdhocMatchingMemberInternal* deadkid = findPeer(context, &mac);

	if (deadkid->state != PSP_ADHOC_MATCHING_PEER_CHILD) {
		// Invalid Sibling
		return;
	}

	// Spawn Leave Event
	spawnLocalEvent(context, PSP_ADHOC_MATCHING_EVENT_LEAVE, &mac, 0, NULL);

	// Delete Peer
	deletePeer(context, deadkid);
}

/**
* Handle Bye Packet
* @param context Matching Context Pointer
* @param sendermac Packet Sender MAC
*/
void actOnByePacket(SceNetAdhocMatchingContext* context, SceNetEtherAddr* sendermac) {
	// Find Peer
	SceNetAdhocMatchingMemberInternal* peer = findPeer(context, sendermac);

	if (peer == NULL) {
		// We don't know this guy
		return;
	}

	// P2P or Child Bye. FIXME: Should we allow BYE Event to intervene joining process of Parent-Child too just like P2P Mode? (ie. Crazy Taxi uses P2P Mode)
	if ((context->mode == PSP_ADHOC_MATCHING_MODE_PARENT && peer->state == PSP_ADHOC_MATCHING_PEER_CHILD) ||
		(context->mode == PSP_ADHOC_MATCHING_MODE_CHILD && peer->state == PSP_ADHOC_MATCHING_PEER_CHILD) ||
		(context->mode == PSP_ADHOC_MATCHING_MODE_P2P &&
			(peer->state == PSP_ADHOC_MATCHING_PEER_P2P || peer->state == PSP_ADHOC_MATCHING_PEER_OFFER || peer->state == PSP_ADHOC_MATCHING_PEER_INCOMING_REQUEST || peer->state == PSP_ADHOC_MATCHING_PEER_OUTGOING_REQUEST || peer->state == PSP_ADHOC_MATCHING_PEER_CANCEL_IN_PROGRESS)))
	{
		if (context->mode != PSP_ADHOC_MATCHING_MODE_CHILD) {
			// Spawn Leave / Kick Event. FIXME: DISCONNECT event should only be triggered on Parent/P2P mode and for Parent/P2P peer?
			spawnLocalEvent(context, PSP_ADHOC_MATCHING_EVENT_BYE, sendermac, 0, NULL);
		}

		// Delete Peer
		deletePeer(context, peer);
		// Instead of removing peer immediately, We should give a little time before removing the peer and let it timed out? just in case the game is in the middle of communicating with the peer on another thread so it won't recognize it as Unknown peer
		//peer->lastping = CoreTiming::GetGlobalTimeUsScaled();
	}

	// Parent Bye
	else if (context->mode == PSP_ADHOC_MATCHING_MODE_CHILD && peer->state == PSP_ADHOC_MATCHING_PEER_PARENT) {
		// Spawn Leave / Kick Event. FIXME: DISCONNECT event should only be triggered on Parent/P2P mode and for Parent/P2P peer?
		spawnLocalEvent(context, PSP_ADHOC_MATCHING_EVENT_BYE, sendermac, 0, NULL);

		// Delete Peer from List
		clearPeerList(context);
	}
}

/**
* TODO: This really should be a callback or event!
* Matching Event Dispatcher Thread
* @param args sizeof(SceNetAdhocMatchingContext *)
* @param argp SceNetAdhocMatchingContext *
* @return Exit Point is never reached...
*/
int matchingEventThread(int matchingId) {
	SetCurrentThreadName("MatchingEvent");
	// Multithreading Lock
	peerlock.lock();
	// Cast Context
	SceNetAdhocMatchingContext* context = findMatchingContext(matchingId);
	// Multithreading Unlock
	peerlock.unlock();

	// Log Startup
	INFO_LOG(Log::sceNet, "EventLoop: Begin of EventLoop[%i] Thread", matchingId);

	// Run while needed...
	if (context != NULL) {
		uint32_t bufLen = context->rxbuflen; //0;
		uint32_t bufAddr = 0; //= userMemory.Alloc(bufLen); //context->rxbuf;
		uint32_t* args = context->handlerArgs; //MatchingArgs

		while (contexts != NULL && context->eventRunning) {
			// Multithreading Lock
			peerlock.lock();
			// Cast Context
			context = findMatchingContext(matchingId);
			// Multithreading Unlock
			peerlock.unlock();

			// Messages on Stack ready for processing
			while (context != NULL && context->event_stack != NULL) {
				// Claim Stack
				context->eventlock->lock();

				// Iterate Message List
				ThreadMessage* msg = context->event_stack;
				if (msg != NULL) {
					// Default Optional Data
					void* opt = NULL;

					// Grab Optional Data
					if (msg->optlen > 0) opt = ((uint8_t*)msg) + sizeof(ThreadMessage); //&msg[1]

					// Log Matching Events
					INFO_LOG(Log::sceNet, "EventLoop[%d]: Matching Event [%d=%s][%s] OptSize=%d", matchingId, msg->opcode, getMatchingEventStr(msg->opcode), mac2str(&msg->mac).c_str(), msg->optlen);

					// Unlock to prevent race-condition with other threads due to recursive lock
					//context->eventlock->unlock();
					// Call Event Handler
					//context->handler(context->id, msg->opcode, &msg->mac, msg->optlen, opt);
					// Notify Event Handlers
					notifyMatchingHandler(context, msg, opt, bufAddr, bufLen, args); // If we're using shared Buffer & Args for All Events We should wait for the Mipscall to be fully executed before processing the next event. GTA VCS need this delay/sleep.

					// Give some time before executing the next mipscall to prevent event ACCEPT(6)->ESTABLISH(7) getting reversed After Action ESTABLISH(7)->ACCEPT(6)
					// Must Not be delayed too long to prevent desync/disconnect. Not longer than the delays on callback's HLE?
					//sleep_ms(10); //sceKernelDelayThread(10000);

					// Lock again
					//context->eventlock->lock();

					// Pop event stack from front (this should be queue instead of stack?)
					context->event_stack = msg->next;
					free(msg);
					msg = NULL;
				}

				// Unlock Stack
				context->eventlock->unlock();
			}

			// Share CPU Time
			sleep_ms(10, "pro-adhoc-poll-3"); //1 //sceKernelDelayThread(10000);

			//// Don't do anything if it's paused, otherwise the log will be flooded
			//while (Core_IsStepping() && coreState != CORE_POWERDOWN && contexts != NULL && context->eventRunning)
			//	sleep_ms(10, "pro-adhoc-event-poll-3");
		}

		// Process Last Messages
		if (contexts != NULL && context->event_stack != NULL) {
			// Claim Stack
			context->eventlock->lock();

			// Iterate Message List
			int msg_count = 0;
			ThreadMessage* msg = context->event_stack;
			for (; msg != NULL; msg = msg->next) {
				// Default Optional Data
				void* opt = NULL;

				// Grab Optional Data
				if (msg->optlen > 0) opt = ((uint8_t*)msg) + sizeof(ThreadMessage); //&msg[1]

				INFO_LOG(Log::sceNet, "EventLoop[%d]: Matching Event [EVENT=%d]\n", matchingId, msg->opcode);

				//context->eventlock->unlock();
				// Original Call Event Handler
				//context->handler(context->id, msg->opcode, &msg->mac, msg->optlen, opt);
				// Notify Event Handlers
				notifyMatchingHandler(context, msg, opt, bufAddr, bufLen, args);
				//context->eventlock->lock();
				msg_count++;
			}

			// Clear Event Message Stack
			clearStack(context, PSP_ADHOC_MATCHING_EVENT_STACK);

			// Free Stack
			context->eventlock->unlock();
			INFO_LOG(Log::sceNet, "EventLoop[%d]: Finished (%d msg)", matchingId, msg_count);
		}

		// Free memory
		//if (Memory::IsValidAddress(bufAddr)) userMemory.Free(bufAddr);

		// Delete Pointer Reference (and notify caller about finished cleanup)
		//context->eventThread = NULL;
	}

	// Log Shutdown
	INFO_LOG(Log::sceNet, "EventLoop: End of EventLoop[%i] Thread", matchingId);

	// Return Zero to shut up Compiler
	return 0;
}

/**
* Matching IO Handler Thread
* @param args sizeof(SceNetAdhocMatchingContext *)
* @param argp SceNetAdhocMatchingContext *
* @return Exit Point is never reached...
*/
int matchingInputThread(int matchingId) { // TODO: The MatchingInput thread is using sceNetAdhocPdpRecv & sceNetAdhocPdpSend functions so it might be better to run this on PSP thread instead of real thread
	SetCurrentThreadName("MatchingInput");
	// Multithreading Lock
	peerlock.lock();
	// Cast Context
	SceNetAdhocMatchingContext* context = findMatchingContext(matchingId);
	// Multithreading Unlock
	peerlock.unlock();

	// Last Ping
	uint64_t lastping = 0;

	// Last Hello
	uint64_t lasthello = 0;

	uint64_t now;

	static SceNetEtherAddr sendermac;
	static uint32_t senderport;
	static int rxbuflen;

	// Log Startup
	INFO_LOG(Log::sceNet, "InputLoop: Begin of InputLoop[%i] Thread", matchingId);

	// Run while needed...
	if (context != NULL) {
		while (contexts != NULL && context->inputRunning) {
			// Multithreading Lock
			peerlock.lock();
			// Cast Context
			context = findMatchingContext(matchingId);
			// Multithreading Unlock
			peerlock.unlock();

			while (context != NULL && context->inputRunning/* && !Core_IsStepping()*/) {
				now = CoreTiming::GetGlobalTimeUsScaled(); //time_now_d()*1000000.0;

				// Hello Message Sending Context with unoccupied Slots
				if ((context->mode == PSP_ADHOC_MATCHING_MODE_PARENT && (countChildren(context) < (context->maxpeers - 1))) || (context->mode == PSP_ADHOC_MATCHING_MODE_P2P && findP2P(context) == NULL)) {
					// Hello Message Broadcast necessary because of Hello Interval
					if (context->hello_int > 0)
						if (static_cast<int64_t>(now - lasthello) >= static_cast<int64_t>(context->hello_int)) {
							// Broadcast Hello Message
							broadcastHelloMessage(context);

							// Update Hello Timer
							lasthello = now;
						}
				}

				// Ping Required
				if (context->keepalive_int > 0) {
					if (static_cast<int64_t>(now - lastping) >= static_cast<int64_t>(context->keepalive_int)) {
						// Handle Peer Timeouts
						handleTimeout(context);

						// Broadcast Ping Message
						broadcastPingMessage(context);

						// Update Ping Timer
						lastping = now;
					}
				}
				else {
					// FIXME: Should we checks for Timeout too when the game doesn't set the keep alive interval?
					handleTimeout(context);
				}

				// Messages on Stack ready for processing
				if (context->input_stack != NULL) {
					// Claim Stack
					context->inputlock->lock();

					// Iterate Message List
					ThreadMessage* msg = context->input_stack;
					while (msg != NULL) {
						// Default Optional Data
						void* opt = NULL;

						// Grab Optional Data
						if (msg->optlen > 0) opt = ((uint8_t*)msg) + sizeof(ThreadMessage);

						//context->inputlock->unlock(); // Unlock to prevent race condition when locking peerlock

						// Send Accept Packet
						if (msg->opcode == PSP_ADHOC_MATCHING_PACKET_ACCEPT) sendAcceptPacket(context, &msg->mac, msg->optlen, opt);

						// Send Join Packet
						else if (msg->opcode == PSP_ADHOC_MATCHING_PACKET_JOIN) sendJoinPacket(context, &msg->mac, msg->optlen, opt);

						// Send Cancel Packet
						else if (msg->opcode == PSP_ADHOC_MATCHING_PACKET_CANCEL) sendCancelPacket(context, &msg->mac, msg->optlen, opt);

						// Send Bulk Data Packet
						else if (msg->opcode == PSP_ADHOC_MATCHING_PACKET_BULK) sendBulkDataPacket(context, &msg->mac, msg->optlen, opt);

						// Send Birth Packet
						else if (msg->opcode == PSP_ADHOC_MATCHING_PACKET_BIRTH) sendBirthPacket(context, &msg->mac);

						// Send Death Packet
						else if (msg->opcode == PSP_ADHOC_MATCHING_PACKET_DEATH) sendDeathPacket(context, &msg->mac);

						// Cancel Bulk Data Transfer (does nothing as of now as we fire and forget anyway) // Do we need to check DeathPacket and ByePacket here?
						//else if(msg->opcode == PSP_ADHOC_MATCHING_PACKET_BULK_ABORT) sendAbortBulkDataPacket(context, &msg->mac, msg->optlen, opt);

						//context->inputlock->lock(); // Lock again

						// Pop input stack from front (this should be queue instead of stack?)
						context->input_stack = msg->next;
						free(msg);
						msg = context->input_stack;
					}

					// Free Stack
					context->inputlock->unlock();
				}

				// Receive PDP Datagram
				// FIXME: When using JPCSP + prx files, the "SceNetAdhocMatchingInput" thread is using blocking PdpRecv with infinite(0) timeout, which can be stopped/aborted using SetSocketAlert, while "SceNetAdhocMatchingEvent" thread is using non-blocking for sending
				rxbuflen = context->rxbuflen;
				senderport = 0;
				// Lock the peer first before locking the socket to avoid race condiion
				peerlock.lock();
				context->socketlock->lock();
				int recvresult = sceNetAdhocPdpRecv(context->socket, &sendermac, &senderport, context->rxbuf, &rxbuflen, 0, ADHOC_F_NONBLOCK);
				context->socketlock->unlock();
				peerlock.unlock();

				// Received Data from a Sender that interests us
				// Note: There are cases where the sender port might be re-mapped by router or ISP, so we shouldn't check the source port.
				if (recvresult == 0 && rxbuflen > 0) {
					// Log Receive Success
					if (context->rxbuf[0] > 1) {
						INFO_LOG(Log::sceNet, "InputLoop[%d]: Received %d Bytes (Opcode[%d]=%s)", matchingId, rxbuflen, context->rxbuf[0], getMatchingOpcodeStr(context->rxbuf[0]));
					}

					// Update Peer Timestamp
					peerlock.lock();
					SceNetAdhocctlPeerInfo* peer = findFriend(&sendermac);
					if (peer != NULL) {
						now = CoreTiming::GetGlobalTimeUsScaled();
						int64_t delta = now - peer->last_recv;
						DEBUG_LOG(Log::sceNet, "Timestamp LastRecv Delta: %lld (%llu - %llu) from %s", delta, now, peer->last_recv, mac2str(&sendermac).c_str());
						if (peer->last_recv != 0) peer->last_recv = std::max(peer->last_recv, now - defaultLastRecvDelta);
					}
					else {
						WARN_LOG(Log::sceNet, "InputLoop[%d]: Unknown Peer[%s:%u] (Recved=%i, Length=%i)", matchingId, mac2str(&sendermac).c_str(), senderport, recvresult, rxbuflen);
					}

					// Show a warning if other player is having their port being re-mapped, thus that other player may have issue with the communication. 
					// Note: That other player may need to switch side between host and join, or reboot their router to solve this issue.
					if (context->port != senderport && senderport != (*context->peerPort)[sendermac]) {
						char name[9] = {};
						if (peer != NULL)
							truncate_cpy(name, sizeof(name), (const char*)peer->nickname.data);
						WARN_LOG(Log::sceNet, "InputLoop[%d]: Unknown Source Port from [%s][%s:%u -> %u] (Recved=%i, Length=%i)", matchingId, name, mac2str(&sendermac).c_str(), senderport, context->port, recvresult, rxbuflen);
					}
					// Keep tracks of re-mapped peer's ports for further communication. 
					// Note: This will only works if this player were able to receives data on normal port from other players (ie. this player's port wasn't remapped)
					(*context->peerPort)[sendermac] = senderport;
					peerlock.unlock();

					// Ping Packet
					if (context->rxbuf[0] == PSP_ADHOC_MATCHING_PACKET_PING) actOnPingPacket(context, &sendermac);

					// Hello Packet
					else if (context->rxbuf[0] == PSP_ADHOC_MATCHING_PACKET_HELLO) actOnHelloPacket(context, &sendermac, rxbuflen);

					// Join Packet
					else if (context->rxbuf[0] == PSP_ADHOC_MATCHING_PACKET_JOIN) actOnJoinPacket(context, &sendermac, rxbuflen);

					// Accept Packet
					else if (context->rxbuf[0] == PSP_ADHOC_MATCHING_PACKET_ACCEPT) actOnAcceptPacket(context, &sendermac, rxbuflen);

					// Cancel Packet
					else if (context->rxbuf[0] == PSP_ADHOC_MATCHING_PACKET_CANCEL) actOnCancelPacket(context, &sendermac, rxbuflen);

					// Bulk Data Packet
					else if (context->rxbuf[0] == PSP_ADHOC_MATCHING_PACKET_BULK) actOnBulkDataPacket(context, &sendermac, rxbuflen);

					// Abort Bulk Data Packet
					//else if (context->rxbuf[0] == PSP_ADHOC_MATCHING_PACKET_BULK_ABORT) actOnAbortBulkDataPacket(context, &sendermac, rxbuflen);

					// Birth Packet
					else if (context->rxbuf[0] == PSP_ADHOC_MATCHING_PACKET_BIRTH) actOnBirthPacket(context, &sendermac, rxbuflen);

					// Death Packet
					else if (context->rxbuf[0] == PSP_ADHOC_MATCHING_PACKET_DEATH) actOnDeathPacket(context, &sendermac, rxbuflen);

					// Bye Packet
					else if (context->rxbuf[0] == PSP_ADHOC_MATCHING_PACKET_BYE) actOnByePacket(context, &sendermac);

					// Ignore Incoming Trash Data
				}
				else
					break;
			}
			// Share CPU Time
			sleep_ms(10, "pro-adhoc-4"); //1 //sceKernelDelayThread(10000);

			//// Don't do anything if it's paused, otherwise the log will be flooded
			//while (Core_IsStepping() && coreState != CORE_POWERDOWN && contexts != NULL && context->inputRunning)
			//	sleep_ms(10, "pro-adhoc-input-4");
		}

		if (contexts != NULL) {
			// Process Last Messages
			if (context->input_stack != NULL) {
				// Claim Stack
				context->inputlock->lock();

				// Iterate Message List
				int msg_count = 0;
				ThreadMessage* msg = context->input_stack;
				while (msg != NULL) {
					// Default Optional Data
					void* opt = NULL;

					// Grab Optional Data
					if (msg->optlen > 0) opt = ((uint8_t*)msg) + sizeof(ThreadMessage);

					// Send Accept Packet
					if (msg->opcode == PSP_ADHOC_MATCHING_PACKET_ACCEPT) sendAcceptPacket(context, &msg->mac, msg->optlen, opt);

					// Send Join Packet
					else if (msg->opcode == PSP_ADHOC_MATCHING_PACKET_JOIN) sendJoinPacket(context, &msg->mac, msg->optlen, opt);

					// Send Cancel Packet
					else if (msg->opcode == PSP_ADHOC_MATCHING_PACKET_CANCEL) sendCancelPacket(context, &msg->mac, msg->optlen, opt);

					// Send Bulk Data Packet
					else if (msg->opcode == PSP_ADHOC_MATCHING_PACKET_BULK) sendBulkDataPacket(context, &msg->mac, msg->optlen, opt);

					// Send Birth Packet
					else if (msg->opcode == PSP_ADHOC_MATCHING_PACKET_BIRTH) sendBirthPacket(context, &msg->mac);

					// Send Death Packet
					else if (msg->opcode == PSP_ADHOC_MATCHING_PACKET_DEATH) sendDeathPacket(context, &msg->mac);

					// Cancel Bulk Data Transfer (does nothing as of now as we fire and forget anyway) // Do we need to check DeathPacket and ByePacket here?
					//else if(msg->opcode == PSP_ADHOC_MATCHING_PACKET_BULK_ABORT) sendAbortBulkDataPacket(context, &msg->mac, msg->optlen, opt);

					// Pop input stack from front (this should be queue instead of stack?)
					context->input_stack = msg->next;
					free(msg);
					msg = context->input_stack;
					msg_count++;
				}

				// Free Stack
				context->inputlock->unlock();
				INFO_LOG(Log::sceNet, "InputLoop[%d]: Finished (%d msg)", matchingId, msg_count);
			}

			// Clear IO Message Stack
			clearStack(context, PSP_ADHOC_MATCHING_INPUT_STACK);

			// Send Bye Messages. FIXME: Official prx seems to be sending DEATH instead of BYE packet during MatchingStop? But DEATH packet doesn't works with DBZ Team Tag
			sendByePacket(context);

			// Free Peer List Buffer
			clearPeerList(context); //deleteAllMembers(context);

			// Delete Pointer Reference (and notify caller about finished cleanup)
			//context->inputThread = NULL;
		}
	}

	// Log Shutdown
	INFO_LOG(Log::sceNet, "InputLoop: End of InputLoop[%i] Thread", matchingId);

	// Terminate Thread
	//sceKernelExitDeleteThread(0);

	// Return Zero to shut up Compiler
	return 0;
}

void netAdhocMatchingValidateLoopMemory() { // ?????
	//if (!matchingThreadHackAddr || (matchingThreadHackAddr && strcmp("matchingThreadHack", kernelMemory.GetBlockTag(matchingThreadHackAddr)) != 0)) {
	//	uint32_t blockSize = sizeof(matchingThreadCode);
	//	matchingThreadHackAddr = kernelMemory.Alloc(blockSize, false, "matchingThreadHack");
	//	if (matchingThreadHackAddr) Memory::Memcpy(matchingThreadHackAddr, matchingThreadCode, sizeof(matchingThreadCode));
	//}
}

int NetAdhocMatching_Stop(int matchingId) {
	SceNetAdhocMatchingContext* item = findMatchingContext(matchingId);
	if (item == NULL) {
		return 0;
	}

	// This will cause using PdpRecv on this socket to return ERROR_NET_ADHOC_SOCKET_ALERTED (Based on Ys vs. Sora no Kiseki when tested with JPCSP + prx files). Is this used to abort inprogress socket activity?
	NetAdhoc_SetSocketAlert(item->socket, ADHOC_F_ALERTRECV);

	item->inputRunning = false;
	if (item->inputThread.joinable()) {
		item->inputThread.join();
	}

	item->eventRunning = false;
	if (item->eventThread.joinable()) {
		item->eventThread.join();
	}

	// Stop fake PSP Thread.
	// kernelObjects may already been cleared early during a Shutdown, thus trying to access it may generates Warning/Error in the log
	if (matchingThreads[item->matching_thid] > 0 /*&& strcmp(__KernelGetThreadName(matchingThreads[item->matching_thid]), "ERROR") != 0*/) {
		//__KernelStopThread(matchingThreads[item->matching_thid], SCE_KERNEL_ERROR_THREAD_TERMINATED, "AdhocMatching stopped");
		//__KernelDeleteThread(matchingThreads[item->matching_thid], SCE_KERNEL_ERROR_THREAD_TERMINATED, "AdhocMatching deleted");
	}
	matchingThreads[item->matching_thid] = 0;

	// Make sure nobody locking/using the socket
	item->socketlock->lock();
	// Delete the socket
	NetAdhocPdp_Delete(item->socket, 0); // item->connected = (sceNetAdhocPdpDelete(item->socket, 0) < 0);
	item->socketlock->unlock();

	// Multithreading Lock
	peerlock.lock();

	// Remove your own MAC, or All members, or don't remove at all or we should do this on MatchingDelete ?
	clearPeerList(item); //deleteAllMembers(item);

	item->running = 0;
	netAdhocMatchingStarted--;

	// Multithreading Unlock
	peerlock.unlock();

	return 0;
}

int sceNetAdhocMatchingStop(int matchingId) {
	WARN_LOG(Log::sceNet, "UNTESTED sceNetAdhocMatchingStop(%i)", matchingId);

	return NetAdhocMatching_Stop(matchingId);
}

int NetAdhocMatching_Delete(int matchingId) {
	// Multithreading Lock
	std::lock_guard<std::recursive_mutex> peer_guard(peerlock);

	// Previous Context Reference
	SceNetAdhocMatchingContext* prev = NULL;

	// Context Pointer
	SceNetAdhocMatchingContext* item = contexts;

	// Iterate contexts
	for (; item != NULL; item = item->next) {
		// Found matching ID
		if (item->id == matchingId) {
			// Unlink Left (Beginning)
			if (prev == NULL) contexts = item->next;

			// Unlink Left (Other)
			else prev->next = item->next;

			// Stop it first if it's still running
			if (item->running) {
				NetAdhocMatching_Stop(matchingId);
			}
			// Delete the Fake PSP Thread
			//__KernelDeleteThread(item->matching_thid, SCE_KERNEL_ERROR_THREAD_TERMINATED, "AdhocMatching deleted");
			//delete item->matchingThread;

			// Free allocated memories
			free(item->hello);
			free(item->rxbuf);
			clearPeerList(item); //deleteAllMembers(item);
			(*item->peerPort).clear();
			delete item->peerPort;
			// Destroy locks
			item->eventlock->lock(); // Make sure it's not locked when being deleted
			item->eventlock->unlock();
			delete item->eventlock;
			item->inputlock->lock(); // Make sure it's not locked when being deleted
			item->inputlock->unlock();
			delete item->inputlock;
			item->socketlock->lock(); // Make sure it's not locked when being deleted
			item->socketlock->unlock();
			delete item->socketlock;
			// Free item context memory
			free(item);
			item = NULL;

			// Making sure there are no leftover matching events from this session which could cause a crash on the next session
			deleteMatchingEvents(matchingId);

			// Stop Search
			break;
		}

		// Set Previous Reference
		prev = item;
	}

	return 0;
}

int sceNetAdhocMatchingDelete(int matchingId) {
	// WLAN might be disabled in the middle of successfull multiplayer, but we still need to cleanup right?

	NetAdhocMatching_Delete(matchingId);

	WARN_LOG(Log::sceNet, "UNTESTED sceNetAdhocMatchingDelete(%i)", matchingId);

	// Give a little time to make sure everything are cleaned up before the following AdhocMatchingCreate, Not too long tho, otherwise Naruto Ultimate Ninja Heroes 3 will have an issue
	//hleDelayResult(0, "give time to init/cleanup", adhocExtraPollDelayMS * 1000);
	return 0;
}

int sceNetAdhocMatchingInit(uint32_t memsize) {
	WARN_LOG(Log::sceNet, "sceNetAdhocMatchingInit(%d)", memsize);

	// Uninitialized Library
	if (netAdhocMatchingInited)
		return SCE_NET_ADHOC_MATCHING_ERROR_ALREADY_INITIALIZED;

	// Save Fake Pool Size
	fakePoolSize = memsize;

	// Initialize Library
	deleteMatchingEvents();
	netAdhocMatchingInited = true;

	// Return Success
	return 0;
}

int NetAdhocMatching_Term() {
	if (netAdhocMatchingInited) {
		// Delete all Matching contexts
		SceNetAdhocMatchingContext* next = NULL;
		SceNetAdhocMatchingContext* context = contexts;
		while (context != NULL) {
			next = context->next;
			//if (context->running) NetAdhocMatching_Stop(context->id);
			NetAdhocMatching_Delete(context->id);
			context = next;
		}
		contexts = NULL;
		matchingThreads.clear();
	}

	return 0;
}

int sceNetAdhocMatchingTerm() {
	WARN_LOG(Log::sceNet, "UNTESTED sceNetAdhocMatchingTerm()");
	// Should we cleanup all created matching contexts first? just in case there are games that doesn't delete them before calling this
	NetAdhocMatching_Term();

	netAdhocMatchingInited = false;
	return 0;
}

int sceNetAdhocMatchingCreate(int mode, int maxnum, int port, int rxbuflen, int hello_int, int keepalive_int, int init_count, int rexmt_int, pspAdhocMatchingCallback callback) {
	WARN_LOG(Log::sceNet, "sceNetAdhocMatchingCreate(mode=%i, maxnum=%i, port=%i, rxbuflen=%i, hello=%i, keepalive=%i, initcount=%i, rexmt=%i, callbackAddr=%08x)", mode, maxnum, port, rxbuflen, hello_int, keepalive_int, init_count, rexmt_int, callback);
	if (!g_Config.bEnableWlan) {
		return -1;
	}

	SceNetAdhocMatchingHandler handler;
	handler.entryPoint = callback;

	if (!netAdhocMatchingInited) {
		// Uninitialized Library
		return SCE_NET_ADHOC_MATCHING_ERROR_NOT_INITIALIZED;
	}

	if (maxnum <= 1 || maxnum > 16) {
		// Invalid Member Limit
		return SCE_NET_ADHOC_MATCHING_ERROR_INVALID_MAXNUM;
	}

	if (rxbuflen < 1) {
		// Invalid Receive Buffer Size
		return SCE_NET_ADHOC_MATCHING_ERROR_RXBUF_TOO_SHORT;
	}

	if (mode < 1 || mode > 3) {
		// InvalidERROR_NET_Arguments
		return SCE_NET_ADHOC_MATCHING_ERROR_INVALID_ARG;
	}

	// Iterate Matching Contexts
	SceNetAdhocMatchingContext* item = contexts;
	for (; item != NULL; item = item->next) {
		// Port Match found
		if (item->port == port)
			return SCE_NET_ADHOC_MATCHING_ERROR_PORT_IN_USE;
	}

	// Allocate Context Memory
	SceNetAdhocMatchingContext* context = (SceNetAdhocMatchingContext*)malloc(sizeof(SceNetAdhocMatchingContext));

	// Allocated Memory
	if (context != NULL) {
		// Create PDP Socket
		SceNetEtherAddr localmac;
		getLocalMac(&localmac);

		// Clear Memory
		memset(context, 0, sizeof(SceNetAdhocMatchingContext));

		// Allocate Receive Buffer
		context->rxbuf = (uint8_t*)malloc(rxbuflen);

		// Allocated Memory
		if (context->rxbuf != NULL) {
			// Clear Memory
			memset(context->rxbuf, 0, rxbuflen);

			// Fill in Context Data
			context->id = findFreeMatchingID();
			context->mode = mode;
			context->maxpeers = maxnum;
			context->port = port;
			context->rxbuflen = rxbuflen;
			context->resendcounter = init_count;
			context->resend_int = rexmt_int; // used as ack timeout on lost packet (ie. not receiving anything after sending)?
			context->hello_int = hello_int; // client might set this to 0
			if (keepalive_int < 1) context->keepalive_int = PSP_ADHOCCTL_PING_TIMEOUT; else context->keepalive_int = keepalive_int; // client might set this to 0
			context->keepalivecounter = init_count; // used to multiply keepalive_int as timeout
			context->timeout = (((uint64_t)(keepalive_int)+(uint64_t)rexmt_int) * (uint64_t)init_count);
			context->timeout += 500000; // For internet play we need higher timeout than what the game wanted
			context->handler = handler;
			context->peerPort = new std::map<SceNetEtherAddr, uint16_t>();

			// Fill in Selfpeer
			context->mac = localmac;

			// Create locks
			context->socketlock = new std::recursive_mutex;
			context->eventlock = new std::recursive_mutex;
			context->inputlock = new std::recursive_mutex;

			// Multithreading Lock
			peerlock.lock(); //contextlock.lock();

			// Add Callback Handler
			context->handler.entryPoint = callback;
			context->matching_thid = static_cast<int>(matchingThreads.size());
			matchingThreads.push_back(0);

			// Link Context
			//context->connected = true;
			context->next = contexts;
			contexts = context;

			// Multithreading UnLock
			peerlock.unlock(); //contextlock.unlock();

			// Just to make sure Adhoc is already connected
			//hleDelayResult(context->id, "give time to init/cleanup", adhocEventDelayMS * 1000);

			// Return Matching ID
			return context->id;
		}

		// Free Memory
		free(context);
	}

	// Out of Memory
	return SCE_NET_ADHOC_MATCHING_ERROR_NO_SPACE;
}

int NetAdhocMatching_Start(int matchingId, int evthPri, int evthPartitionId, int evthStack, int inthPri, int inthPartitionId, int inthStack, int optLen, void* optDataAddr) {
	// Multithreading Lock
	std::lock_guard<std::recursive_mutex> peer_guard(peerlock);

	SceNetAdhocMatchingContext* item = findMatchingContext(matchingId);

	if (item == NULL) {
		// return SCE_NET_ADHOC_MATCHING_ERROR_INVALID_ID; //Faking success to prevent GTA:VCS from stuck unable to choose host/join menu
		return 0;
	}

	//sceNetAdhocMatchingSetHelloOpt(matchingId, optLen, optDataAddr); //SetHelloOpt only works when context is running
	if ((optLen > 0) && optDataAddr != NULL) {
		// Allocate the memory and copy the content
		free(item->hello);
		item->hello = (uint8_t*)malloc(optLen);
		if (item->hello != NULL) {
			memcpy(item->hello, optDataAddr, optLen);
			item->hellolen = optLen;
			item->helloAddr = optDataAddr;
		}
		//else return SCE_NET_ADHOC_MATCHING_ERROR_NO_SPACE; //Faking success to prevent GTA:VCS from stuck unable to choose host/join menu
	}
	//else return SCE_NET_ADHOC_MATCHING_ERROR_INVALID_ARG; // SCE_NET_ADHOC_MATCHING_ERROR_INVALID_OPTLEN; // Returning Not Success will cause GTA:VC stuck unable to choose host/join menu

	// Create PDP Socket
	int sock = sceNetAdhocPdpCreate((unsigned char*)&item->mac, static_cast<int>(item->port), item->rxbuflen, ADHOC_F_BLOCK);
	item->socket = sock;
	if (sock < 1) {
		return SCE_NET_ADHOC_MATCHING_ERROR_PORT_IN_USE;
	}

	// Create & Start the Fake PSP Thread ("matching_ev%d" and "matching_io%d")
	netAdhocMatchingValidateLoopMemory();
	std::string thrname = std::string("MatchingThr") + std::to_string(matchingId);
	//matchingThreads[item->matching_thid] = hleCall(ThreadManForUser, int, sceKernelCreateThread, thrname.c_str(), matchingThreadHackAddr, evthPri, evthStack, 0, 0);
	////item->matchingThread = new HLEHelperThread(thrname.c_str(), "sceNetAdhocMatching", "__NetMatchingCallbacks", inthPri, inthStack);
	//if (matchingThreads[item->matching_thid] > 0) {
	//	hleCall(ThreadManForUser, int, sceKernelStartThread, matchingThreads[item->matching_thid], 0, 0); //sceKernelStartThread(context->event_thid, sizeof(context), &context);
	//	//item->matchingThread->Start(matchingId, 0);
	//}

	matchingThreads[item->matching_thid] = 0; // PC fake

	//Create the threads
	if (!item->eventRunning) {
		item->eventRunning = true;
		item->eventThread = std::thread(matchingEventThread, matchingId);
	}
	if (!item->inputRunning) {
		item->inputRunning = true;
		item->inputThread = std::thread(matchingInputThread, matchingId);
	}

	item->running = 1;
	netAdhocMatchingStarted++;

	return 0;
}

#define KERNEL_PARTITION_ID 1
#define USER_PARTITION_ID 2
#define VSHELL_PARTITION_ID 5
// This should be similar with sceNetAdhocMatchingStart2 but using USER_PARTITION_ID (2) for PartitionId params
int sceNetAdhocMatchingStart(int matchingId, int evthPri, int evthStack, int inthPri, int inthStack, int optLen, void* optDataAddr) {
	WARN_LOG(Log::sceNet, "UNTESTED sceNetAdhocMatchingStart(%i, %i, %i, %i, %i, %i, %08x)", matchingId, evthPri, evthStack, inthPri, inthStack, optLen, optDataAddr);
	if (!g_Config.bEnableWlan) {
		return -1;
	}

	int retval = NetAdhocMatching_Start(matchingId, evthPri, USER_PARTITION_ID, evthStack, inthPri, USER_PARTITION_ID, inthStack, optLen, optDataAddr);
	// Give a little time to make sure matching Threads are ready before the game use the next sceNet functions, should've checked for status instead of guessing the time?
	hleEatMicro(adhocMatchingEventDelay);
	return retval;
}

int sceNetAdhocMatchingSelectTarget(int matchingId, unsigned char* macAddress, int optLen, void* optDataPtr) {
	WARN_LOG(Log::sceNet, "UNTESTED sceNetAdhocMatchingSelectTarget(%i, %s, %i, %08x)", matchingId, mac2str((SceNetEtherAddr*)macAddress).c_str(), optLen, optDataPtr);
	if (!g_Config.bEnableWlan) {
		return -1;
	}

	if (!netAdhocMatchingInited) {
		// Uninitialized Library
		return SCE_NET_ADHOC_MATCHING_ERROR_NOT_INITIALIZED;
	}

	if (macAddress == NULL) {
		// Invalid Arguments
		return SCE_NET_ADHOC_MATCHING_ERROR_INVALID_ARG;
	}

	SceNetEtherAddr* target = (SceNetEtherAddr*)macAddress;

	// Find Matching Context for ID
	SceNetAdhocMatchingContext* context = findMatchingContext(matchingId);

	if (context == NULL) {
		// Invalid Matching ID
		return SCE_NET_ADHOC_MATCHING_ERROR_INVALID_ID;
	}

	if (!context->running) {
		// Idle Context
		return SCE_NET_ADHOC_MATCHING_ERROR_NOT_RUNNING;
	}

	// Search Result
	SceNetAdhocMatchingMemberInternal* peer = findPeer(context, (SceNetEtherAddr*)target);

	if (peer == NULL) {
		// Peer not found
		return SCE_NET_ADHOC_MATCHING_ERROR_UNKNOWN_TARGET;
	}

	if ((optLen != 0) && (optLen <= 0 || optDataPtr == 0)) {
		// Invalid Optional Data Length
		return SCE_NET_ADHOC_MATCHING_ERROR_INVALID_OPTLEN;
	}

	void* opt = optDataPtr;
	// Host Mode
	if (context->mode == PSP_ADHOC_MATCHING_MODE_PARENT) {
		// Already Connected
		if (peer->state == PSP_ADHOC_MATCHING_PEER_CHILD) return SCE_NET_ADHOC_MATCHING_ERROR_ALREADY_ESTABLISHED;

		// Not enough space
		if (countChildren(context) == (context->maxpeers - 1)) return SCE_NET_ADHOC_MATCHING_ERROR_EXCEED_MAXNUM;

		// Requesting Peer
		if (peer->state == PSP_ADHOC_MATCHING_PEER_INCOMING_REQUEST) {
			// Accept Peer in Group
			peer->state = PSP_ADHOC_MATCHING_PEER_CHILD;

			// Sending order may need to be reversed since Stack appends to the front, so the order will be switched around.

			// Tell Children about new Sibling
			sendBirthMessage(context, peer);

			// Spawn Established Event
			//spawnLocalEvent(context, PSP_ADHOC_MATCHING_EVENT_ESTABLISHED, target, 0, NULL);

			// Send Accept Confirmation to Peer
			sendAcceptMessage(context, peer, optLen, opt);

			// Return Success
			return 0;
		}
	}

	// Client Mode
	else if (context->mode == PSP_ADHOC_MATCHING_MODE_CHILD) {
		// Already connected
		if (findParent(context) != NULL) return SCE_NET_ADHOC_MATCHING_ERROR_ALREADY_ESTABLISHED;

		// Outgoing Request in Progress
		if (findOutgoingRequest(context) != NULL) return SCE_NET_ADHOC_MATCHING_ERROR_REQUEST_IN_PROGRESS;

		// Valid Offer
		if (peer->state == PSP_ADHOC_MATCHING_PEER_OFFER) {
			// Switch into Join Request Mode
			peer->state = PSP_ADHOC_MATCHING_PEER_OUTGOING_REQUEST;

			// Send Join Request to Peer
			sendJoinRequest(context, peer, optLen, opt);

			// Return Success
			return 0;
		}
	}

	// P2P Mode
	else {
		// Already connected
		if (findP2P(context) != NULL) return SCE_NET_ADHOC_MATCHING_ERROR_ALREADY_ESTABLISHED;

		// Outgoing Request in Progress
		if (findOutgoingRequest(context) != NULL) return SCE_NET_ADHOC_MATCHING_ERROR_REQUEST_IN_PROGRESS;

		// Join Request Mode
		if (peer->state == PSP_ADHOC_MATCHING_PEER_OFFER) {
			// Switch into Join Request Mode
			peer->state = PSP_ADHOC_MATCHING_PEER_OUTGOING_REQUEST;

			// Send Join Request to Peer
			sendJoinRequest(context, peer, optLen, opt);

			// Return Success
			return 0;
		}

		// Requesting Peer
		else if (peer->state == PSP_ADHOC_MATCHING_PEER_INCOMING_REQUEST) {
			// Accept Peer in Group
			peer->state = PSP_ADHOC_MATCHING_PEER_P2P;

			// Tell Children about new Sibling
			//sendBirthMessage(context, peer);
			// Send Accept Confirmation to Peer
			sendAcceptMessage(context, peer, optLen, opt);

			// Return Success
			return 0;
		}
	}

	// How did this happen?! It shouldn't!
	return SCE_NET_ADHOC_MATCHING_ERROR_TARGET_NOT_READY;
}

int NetAdhocMatching_CancelTargetWithOpt(int matchingId, unsigned char* macAddress, int optLen, void* optDataPtr) {
	if (!netAdhocMatchingInited) {
		// Uninitialized Library
		return SCE_NET_ADHOC_MATCHING_ERROR_NOT_INITIALIZED;
	}

	SceNetEtherAddr* target = (SceNetEtherAddr*)macAddress;
	void* opt = optDataPtr;

	if (target == NULL || ((optLen != 0) && (optLen <= 0 || opt == NULL))) {
		// Invalid Arguments
		return SCE_NET_ADHOC_MATCHING_ERROR_INVALID_ARG;
	}

	// Find Matching Context
	SceNetAdhocMatchingContext* context = findMatchingContext(matchingId);

	if (context == NULL) {
		// Invalid Matching ID
		return SCE_NET_ADHOC_MATCHING_ERROR_INVALID_ID;
	}

	if (!context->running) {
		// Context not running
		return SCE_NET_ADHOC_MATCHING_ERROR_NOT_RUNNING;
	}

	// Find Peer
	SceNetAdhocMatchingMemberInternal* peer = findPeer(context, (SceNetEtherAddr*)target);

	if (peer == NULL) {
		// Peer not found
		//return hleLogError(Log::sceNet, SCE_NET_ADHOC_MATCHING_ERROR_UNKNOWN_TARGET, "adhocmatching unknown target");
		// Faking success to prevent the game (ie. Soul Calibur) to repeatedly calling this function when the other player is disconnected
		return 0;
	}

	// Valid Peer Mode
	if ((context->mode == PSP_ADHOC_MATCHING_MODE_CHILD && (peer->state == PSP_ADHOC_MATCHING_PEER_PARENT || peer->state == PSP_ADHOC_MATCHING_PEER_OUTGOING_REQUEST)) ||
		(context->mode == PSP_ADHOC_MATCHING_MODE_PARENT && (peer->state == PSP_ADHOC_MATCHING_PEER_CHILD || peer->state == PSP_ADHOC_MATCHING_PEER_INCOMING_REQUEST)) ||
		(context->mode == PSP_ADHOC_MATCHING_MODE_P2P && (peer->state == PSP_ADHOC_MATCHING_PEER_P2P || peer->state == PSP_ADHOC_MATCHING_PEER_INCOMING_REQUEST)))
	{
		// Notify other Children of Death
		if (context->mode == PSP_ADHOC_MATCHING_MODE_PARENT && peer->state == PSP_ADHOC_MATCHING_PEER_CHILD && countConnectedPeers(context) > 1) {
			// Send Death Message
			sendDeathMessage(context, peer);
		}

		// Mark Peer as Canceled
		peer->state = PSP_ADHOC_MATCHING_PEER_CANCEL_IN_PROGRESS;

		// Send Cancel Event to Peer
		sendCancelMessage(context, peer, optLen, opt);

		// Delete Peer from List
		// Can't delete here, Threads still need this data.
		// deletePeer(context, peer);
		// Marking peer to be timedout instead of deleting immediately
		peer->lastping = 0;

		hleEatCycles(adhocDefaultEatDelay);
		// Return Success
		return 0;
	}

	// Peer not found
	//return hleLogError(Log::sceNet, SCE_NET_ADHOC_MATCHING_ERROR_UNKNOWN_TARGET, "adhocmatching unknown target");
	// Faking success to prevent the game (ie. Soul Calibur) to repeatedly calling this function when the other player is disconnected
	return 0;
}

int sceNetAdhocMatchingCancelTarget(int matchingId, unsigned char* macAddress) {
	WARN_LOG(Log::sceNet, "UNTESTED sceNetAdhocMatchingCancelTarget(%i, %s)", matchingId, mac2str((SceNetEtherAddr*)macAddress).c_str());
	if (!g_Config.bEnableWlan) {
		return -1;
	}
	return NetAdhocMatching_CancelTargetWithOpt(matchingId, macAddress, 0, 0);
}

int sceNetAdhocMatchingSetHelloOpt(int matchingId, int optLenAddr, void* optDataAddr) {
	if (!g_Config.bEnableWlan) {
		return -1;
	}

	if (!netAdhocMatchingInited)
		return SCE_NET_ADHOC_MATCHING_ERROR_NOT_INITIALIZED;

	// Multithreading Lock
	peerlock.lock();

	SceNetAdhocMatchingContext* context = findMatchingContext(matchingId);

	// Multithreading Unlock
	peerlock.unlock();

	// Context not found
	if (context == NULL)
		return SCE_NET_ADHOC_MATCHING_ERROR_INVALID_ID;

	// Invalid Matching Mode (Child)
	if (context->mode == PSP_ADHOC_MATCHING_MODE_CHILD)
		return SCE_NET_ADHOC_MATCHING_ERROR_INVALID_MODE;

	// Context not running
	if (!context->running)
		return SCE_NET_ADHOC_MATCHING_ERROR_NOT_RUNNING;

	// Invalid Optional Data Length
	if ((optLenAddr != 0) && (optDataAddr == 0))
		return SCE_NET_ADHOC_MATCHING_ERROR_INVALID_OPTLEN; //SCE_NET_ADHOC_MATCHING_ERROR_INVALID_ARG

	// Grab Existing Hello Data
	void* hello = context->hello;

	// Free Previous Hello Data, or Reuse it
	//free(hello);

	// Allocation Required
	if (optLenAddr > 0) {
		// Allocate Memory
		if (optLenAddr > context->hellolen) {
			hello = realloc(hello, optLenAddr);
		}

		// Out of Memory
		if (hello == NULL) {
			context->hellolen = 0;
			return SCE_NET_ADHOC_MATCHING_ERROR_NO_SPACE;
		}

		// Clone Hello Data
		memcpy(hello, optDataAddr, optLenAddr);

		// Set Hello Data
		context->hello = (uint8_t*)hello;
		context->hellolen = optLenAddr;
		context->helloAddr = optDataAddr;
	}
	else {
		// Delete Hello Data
		context->hellolen = 0;
		context->helloAddr = 0;
		//free(context->hello); // Doesn't need to free it since it will be reused later
		//context->hello = NULL;
	}

	// Return Success
	return 0;
}

void __NetMatchingCallbacks() { //(int matchingId)
	std::lock_guard<std::recursive_mutex> adhocGuard(adhocEvtMtx);
	//hleSkipDeadbeef();
	int delayus = 3000;

	auto params = matchingEvents.begin();
	if (params != matchingEvents.end()) {
		//uint32_t args[6];
		//memcpy(args, params->data, sizeof(args));
		//auto context = findMatchingContext(args[0]);

		//if (actionAfterMatchingMipsCall < 0) {
		//	actionAfterMatchingMipsCall = __KernelRegisterActionType(AfterMatchingMipsCall::Create);
		//}
		//DEBUG_LOG(Log::sceNet, "AdhocMatching - Remaining Events: %zu", matchingEvents.size());
		//SceNetAdhocMatchingMemberInternal* peer = findPeer(context, (SceNetEtherAddr*)Memory::GetPointer(args[2]));
		//// Discard HELLO Events when in the middle of joining, as some games (ie. Super Pocket Tennis) might tried to join again (TODO: Need to confirm whether sceNetAdhocMatchingSelectTarget supposed to be blocking the current thread or not)
		//if (peer == NULL || (args[1] != PSP_ADHOC_MATCHING_EVENT_HELLO || (peer->state != PSP_ADHOC_MATCHING_PEER_OUTGOING_REQUEST && peer->state != PSP_ADHOC_MATCHING_PEER_INCOMING_REQUEST && peer->state != PSP_ADHOC_MATCHING_PEER_CANCEL_IN_PROGRESS))) {
		//	DEBUG_LOG(Log::sceNet, "AdhocMatchingCallback: [ID=%i][EVENT=%i][%s]", args[0], args[1], mac2str((SceNetEtherAddr*)Memory::GetPointer(args[2])).c_str());

		//	AfterMatchingMipsCall* after = (AfterMatchingMipsCall*)__KernelCreateAction(actionAfterMatchingMipsCall);
		//	after->SetData(args[0], args[1], args[2]);
		//	hleEnqueueCall(args[5], 5, args, after);
		//	matchingEvents.pop_front();
		//}
		//else {
		//	DEBUG_LOG(Log::sceNet, "AdhocMatching - Discarding Callback: [ID=%i][EVENT=%i][%s]", args[0], args[1], mac2str((SceNetEtherAddr*)Memory::GetPointer(args[2])).c_str());
		//	matchingEvents.pop_front();
		//}

		// PC
		//DEBUG_LOG(Log::sceNet, "AdhocMatching - Remaining Events: %zu", matchingEvents.size());
		//SceNetAdhocMatchingMemberInternal* peer = findPeer(context, (SceNetEtherAddr*)((uint8_t*)args[2]));
		// Discard HELLO Events when in the middle of joining, as some games (ie. Super Pocket Tennis) might tried to join again (TODO: Need to confirm whether sceNetAdhocMatchingSelectTarget supposed to be blocking the current thread or not)
		//if (peer == NULL || (args[1] != PSP_ADHOC_MATCHING_EVENT_HELLO || (peer->state != PSP_ADHOC_MATCHING_PEER_OUTGOING_REQUEST && peer->state != PSP_ADHOC_MATCHING_PEER_INCOMING_REQUEST && peer->state != PSP_ADHOC_MATCHING_PEER_CANCEL_IN_PROGRESS))) {
		//	DEBUG_LOG(Log::sceNet, "AdhocMatchingCallback: [ID=%i][EVENT=%i][%s]", args[0], args[1], mac2str((SceNetEtherAddr*)((uint8_t*)args[2])).c_str());

		//	// ???

		//	matchingEvents.pop_front();
		//}
		//else {
		//	DEBUG_LOG(Log::sceNet, "AdhocMatching - Discarding Callback: [ID=%i][EVENT=%i][%s]", args[0], args[1], mac2str((SceNetEtherAddr*)((uint8_t*)args[2])).c_str());
		//	matchingEvents.pop_front();
		//}

		MatchingArgs Params = matchingEvents.front();
		SceNetAdhocMatchingContext* context = findMatchingContext(Params.id);
		if (context) {
			pspAdhocMatchingCallback handler = (pspAdhocMatchingCallback)Params.handler;
			if (handler) {
				if (Params.event == PSP_ADHOC_MATCHING_EVENT_ERROR) {
					handler(Params.id, Params.event, NULL, 0, NULL);
				}
				else {
					void* buf = (void*)Params.bufAddr;
					if (buf != NULL) {
						handler(Params.id, Params.event, (unsigned char*)buf, Params.optlen, (void*)Params.optAddr);
					}
				}
			}
		}
		if (Params.bufAddr != NULL)
			free((void*)Params.bufAddr);
		matchingEvents.pop_front();

		/////////??????
		//SceNetEtherAddr* peer = (SceNetEtherAddr*)((uint8_t*)args[2]);
		//int event = args[1];
		//int optlen = args[3];
		//void* opt = (void*)args[4];
		//if (context->handler.entryPoint)
		//	((void (*)(int, int, SceNetEtherAddr*, int, void*))context->handler.entryPoint)(args[0], event, peer, optlen, opt);
		//free((void*)args[2]);
		//matchingEvents.pop_front();

		// Must be delayed long enough whenever there is a pending callback. Should it be 10-100ms for Matching Events? or Not Less than the delays on sceNetAdhocMatching HLE?
		std::this_thread::sleep_for(std::chrono::microseconds(delayus));
	}

}

void __NetAdhocMatchingInit() {
	netAdhocMatchingInited = false;
}

void __NetAdhocMatchingShutdown() {
	NetAdhocMatching_Term();
}
