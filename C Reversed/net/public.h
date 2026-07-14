/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "common.h"
#include "leeds/base/stringt.h"
#include "net/emu/NetAdhocCommon.h"
//#ifndef GTA_PSP
//#include "multiplayer/net/emu/proAdhoc.h" // SceNetEtherAddr
//#endif

//typedef int8 tMacAddr[6]; // temp
#define PSPSDKMACARG(pMac) ((unsigned char*)pMac)
#define PSPSDKPORTARG(pPort) ((unsigned short*)pPort)
#define MP_MACADDR_BROADCAST { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }

struct tMacAddr
{
public:
	uint8 bytes[SCE_NET_ETHER_ADDR_LEN];

	inline tMacAddr() { SetBroadcast(); }
	inline tMacAddr(uint8 b0, uint8 b1, uint8 b2, uint8 b3, uint8 b4, uint8 b5) {
		bytes[0] = b0; bytes[1] = b1; bytes[2] = b2; bytes[3] = b3; bytes[4] = b4; bytes[5] = b5;
	}
	inline void InitMacAddr() { SetBroadcast(); }
	inline void SetBroadcast() {
#if 1
		tMacAddr broadcast = MP_MACADDR_BROADCAST;
		*this = broadcast;
#elif 0
		memset(bytes, 0xFF, SCE_NET_ETHER_ADDR_LEN);
#else
		for (int32 i = 0; i < SCE_NET_ETHER_ADDR_LEN; ++i)
			bytes[i] = 0xFF;
#endif
	}
	inline bool IsBroadcast() const {
#if 1
		tMacAddr broadcast = MP_MACADDR_BROADCAST;
		return *this == broadcast;
#else
		for (int32 i = 0; i < SCE_NET_ETHER_ADDR_LEN; ++i)
			if (bytes[i] != 0xFF) return false;
		return true;
#endif
	}
	inline bool IsNonBroadcast() const { return !IsBroadcast(); }
	inline tMacAddr& operator=(const tMacAddr& other) {
#if 0
#else
		for (int32 i = 0; i < SCE_NET_ETHER_ADDR_LEN; ++i)
			bytes[i] = other.bytes[i];
#endif
		return *this;
	}
	inline bool operator==(const tMacAddr& other) const { // vcs vanila checks u16 pairs
#if 0
#else
		for (int32 i = 0; i < SCE_NET_ETHER_ADDR_LEN; ++i)
			if (bytes[i] != other.bytes[i]) return false;
		return true;
#endif
	}
	inline bool operator!=(const tMacAddr& other) const {
		return !(*this == other);
	}
	inline void ToString(char* buff) const { // char buffMac[18];  leeds using char buffMac[20];
		static const char hex[] = "0123456789ABCDEF";
		int pos = 0;
		for (int32 i = 0; i < 6; ++i) {
			uint8 v = bytes[i];
			buff[pos++] = hex[(v >> 4) & 0xF];
			buff[pos++] = hex[v & 0xF];
			if (i < 5) buff[pos++] = ':';
		}
		buff[pos] = '\0';
	}
	inline base::string ToString() {
		char buff[20];
		ToString(buff);
		return base::string(buff);
	}
	inline uint8* GetBytes() { return bytes; }
	inline unsigned char* GetBytesSCE() { return (unsigned char*)bytes; }
	//inline unsigned char* GetBytesSCESDK() { return (unsigned char*)bytes; }
#ifndef GTA_PSP
	inline SceNetEtherAddr* GetBytesProAdhoc() { return (SceNetEtherAddr*)bytes; } // ppsspp
#endif
	//inline const unsigned char* GetBytesSCE() const { return (const unsigned char*)bytes; }
};

// 6 matching rooms max in list, 6 players max in room, 37th instance can see 6 lobbies with 6 players 6*6=36
// old MP_MAX_PLAYERS = 6
enum
{
	MP_NUM_PEERS = 7,           // 7   size,   6 peds max in the lobby (1 host + 5 guests)  https://prnt.sc/zCGDYfqSR9ka
	MP_NUM_MATCHING_GROUPS = 7, // 7 (lobbies/rooms/join mode)
	MP_MAX_NUM_PEERS = 8,       // 8 bugly leeds max?
};

uint32_t fast_hash32(const void* key, uint32_t len, uint32_t seed = 0);
uint32_t hash_combine(uint32_t seed, uint32_t value);

#define MP_HOST_INDEX (-1)
#define BROADCAST_PEER_GROUPID (-1) // EVERYONE_PEER_GROUPID
#define BROADCAST_TEAM_A_GROUPID (-2)
#define BROADCAST_TEAM_B_GROUPID (-3)
#ifdef MULTIGAME_ELEMENTS_COMPAT_IMPROVEMENTS
#define BROADCAST_VANILLA_DEVICE_GROUPID (-4)
#define BROADCAST_CUSTOM_DEVICE_GROUPID (-5)
#endif

#ifdef GTA_LIBERTY
#define GTA_VER_V(lcsval, vcsval) (lcsval)
#else
#define GTA_VER_V(lcsval, vcsval) (vcsval)
#endif

//inline bool mp_netAddrAreEqual(tMacAddr* addrA, tMacAddr* addrB) {
//	int8* a = (int8*)addrA;
//	int8* b = (int8*)addrB;
//	return (
//		*((short*)(a)) == *((short*)(b)) &&
//		*((short*)(a + 2)) == *((short*)(b + 2)) &&
//		*((short*)(a + 4)) == *((short*)(b + 4))
//		);
//}
///* TODO(MP): stub */
//inline char* mp_netAddrToStr(tMacAddr* addrA) {
//	static char tmp[1] = "";
//	return tmp;
//}
//inline bool mp_netIsAddrBroadcast(tMacAddr* addr) {
//	tMacAddr broadcast = MP_MACADDR_BROADCAST;
//	return mp_netAddrAreEqual(addr, &broadcast);
//}
//inline void mp_netInitMacAddr(tMacAddr& addr) {
//	addr[0] = -1;
//	addr[1] = -1;
//	addr[2] = -1;
//	addr[3] = -1;
//	addr[4] = -1;
//	addr[5] = -1;
//}
//inline void mp_netAssignMacAddr(tMacAddr& dest, tMacAddr& src) {
//	memcpy(dest, src, sizeof(tMacAddr));
//}
