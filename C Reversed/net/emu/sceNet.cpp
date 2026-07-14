/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include <mutex>
#include <string>
#include <algorithm>

#include "Resolve.h"
#include "SocketCompat.h"
#include "PSPErrorCodes.h"
#include "NetAdhocCommon.h"
#ifdef WITH_UPNP
#include "PortManager.h"
#endif
#include "GameInstance.h"
#include "Utils.h"
#include "proAdhoc.h"
#include "sceNetAdhoc.h"
#include "sceNetAdhocMatching.h"
#include "sceNet.h"
#include "MultiGame.h" // config

bool g_netInited;

static uint32_t netDropRate = 0;
static uint32_t netDropDuration = 0;
static void* netPoolAddr = 0;
static void* netThread1Addr = 0;
static void* netThread2Addr = 0;

static struct SceNetMallocStat netMallocStat;
static uint32_t Net_Term();


void InitLocalhostIP() {
	// The entire 127.*.*.* is reserved for loopback
	uint32_t localIP = 0x7F000001 + GAME_ID - 1;

	g_localhostIP.in.sin_family = AF_INET;
	g_localhostIP.in.sin_addr.s_addr = htonl(localIP);
	g_localhostIP.in.sin_port = 0;

	std::string serverStr(StripSpaces(g_Config.sProAdhocServer));
	isLocalServer = (!strcasecmp(serverStr.c_str(), "localhost") || serverStr.find("127.") == 0);
}

// __CreateHLELoop // mips gen code for fake thread

static void __ResetInitNetLib() {
	g_netInited = false;

	memset(&netMallocStat, 0, sizeof(netMallocStat));
	memset(&parameter, 0, sizeof(parameter));
}

void __NetCallbackInit() {
	// __NetTriggerCallbacks __NetMatchingCallbacks while true code for thread
	// MipsCall cleaners init
}

void __NetInit() {
	INFO_LOG(Log::sceNet, "__NetInit");

	// Windows: Assuming WSAStartup already called beforehand
	portOffset = g_Config.iPortOffset;
	isOriPort = g_Config.bEnableUPnP && g_Config.bUPnPUseOriginalPort;
	minSocketTimeoutUS = g_Config.iMinTimeout * 1000UL;

	// Init Default AdhocServer struct
	g_adhocServerIP.in.sin_family = AF_INET;
	g_adhocServerIP.in.sin_port = htons(ADHOC_SERVER_PORT); //27312 // Maybe read this from config too
	g_adhocServerIP.in.sin_addr.s_addr = INADDR_NONE;

	dummyPeekBuf64k = (char*)malloc(dummyPeekBuf64kSize);
	InitLocalhostIP();

	SceNetEtherAddr mac;
	getLocalMac(&mac);
	INFO_LOG(Log::sceNet, "LocalHost IP will be %s [%s]", ip2str(g_localhostIP.in.sin_addr).c_str(), mac2str(&mac).c_str());

	// For libretro we don't have a better place. On other platforms, we just init/shutdown it with the rest of the emu (NativeInit / NativeShutdown).
#ifdef __LIBRETRO__
	__UPnPInit(2000);
#endif

	__ResetInitNetLib();
	__NetCallbackInit();
}

void __NetShutdown() {
	INFO_LOG(Log::sceNet, "__NetShutdown");

	// Network Cleanup
	Net_Term();
	__ResetInitNetLib();
	if (dummyPeekBuf64k) {
		free(dummyPeekBuf64k);
		dummyPeekBuf64k = NULL;
	}
}

static inline void* AllocUser(uint32_t size, bool fromTop, const char *name) {
	return malloc(size);
}

static inline void FreeUser(void*& addr) {
	if (addr != NULL)
		free(addr);
	addr = NULL;
}

uint32_t Net_Term() {
	// May also need to Terminate netAdhocctl and netAdhoc to free some resources & threads, since the game (ie. GTA:VCS, Wipeout Pulse, etc)
	//might not have called
	// them before calling sceNetTerm and causing them to behave strangely on the next sceNetInit & sceNetAdhocInit
	NetAdhocctl_Term();
	NetAdhocMatching_Term();
	NetAdhoc_Term();

	// Library is initialized
	if (g_netInited) {
		// Delete Adhoc Sockets
		deleteAllAdhocSockets();
	}

	FreeUser(netPoolAddr);
	FreeUser(netThread1Addr);
	FreeUser(netThread2Addr);
	g_netInited = false;

	return 0;
}

/*
Parameters:
	poolsize	- Memory pool size (appears to be for the whole of the networking library).
	calloutprio	- Priority of the SceNetCallout thread.
	calloutstack	- Stack size of the SceNetCallout thread (defaults to 4096 on non 1.5 firmware regardless of what value is passed).
	netintrprio	- Priority of the SceNetNetintr thread.
	netintrstack	- Stack size of the SceNetNetintr thread (defaults to 4096 on non 1.5 firmware regardless of what value is passed).
*/
int sceNetInit(uint32_t poolSize, uint32_t calloutPri, uint32_t calloutStack, uint32_t netinitPri, uint32_t netinitStack)  {
	// TODO: Create Network Threads using given priority & stack
	// TODO: The correct behavior is actually to allocate more and leak the other threads/pool.
	// But we reset here for historic reasons (GTA:VCS potentially triggers this.)
	if (g_netInited) {
		// This cleanup attempt might not worked when SaveState were loaded in the middle of multiplayer game and re-entering multiplayer, thus causing memory leaks & wasting binded ports.
		// Maybe we shouldn't save/load "Inited" vars on SaveState?
		Net_Term();
	}

	if (poolSize == 0) {
		return SCE_KERNEL_ERROR_ILLEGAL_MEMSIZE;
	} else if (calloutPri < 0x08 || calloutPri > 0x77) {
		return SCE_KERNEL_ERROR_ILLEGAL_PRIORITY;
	} else if (netinitPri < 0x08 || netinitPri > 0x77) {
		return SCE_KERNEL_ERROR_ILLEGAL_PRIORITY;
	}

	// TODO: Should also start the threads, probably?  For now, let's just allocate.
	// TODO: Respect the stack size if firmware set to 1.50?
	uint32_t stackSize = 4096;
	netThread1Addr = AllocUser(stackSize, true, "netstack1");
	if (netThread1Addr == 0) {
		return SCE_KERNEL_ERROR_NO_MEMORY;
	}
	netThread2Addr = AllocUser(stackSize, true, "netstack2");
	if (netThread2Addr == 0) {
		FreeUser(netThread1Addr);
		return SCE_KERNEL_ERROR_NO_MEMORY;
	}

	netPoolAddr = AllocUser(poolSize, false, "netpool");
	if (netPoolAddr == 0) {
		FreeUser(netThread1Addr);
		FreeUser(netThread2Addr);
		return SCE_KERNEL_ERROR_NO_MEMORY;
	}

	INFO_LOG(Log::sceNet, "sceNetInit(poolsize=%d, calloutpri=%i, calloutstack=%d, netintrpri=%i, netintrstack=%d)", poolSize, calloutPri, calloutStack, netinitPri, netinitStack);
	
	netMallocStat.pool = poolSize - 0x20; // On Vantage Master Portable this is slightly (32 bytes) smaller than the poolSize arg when tested with JPCSP + prx files
	netMallocStat.maximum = 0x4050; // Dummy maximum foot print
	netMallocStat.free = netMallocStat.pool; // Dummy free size, we should set this high enough to prevent any issue (ie. Vantage Master Portable), this is probably the only field being checked by games?

	// Clear Socket Translator Memory
	memset(&adhocSockets, 0, sizeof(adhocSockets));

	g_netInited = true;

	return 0;
}

uint32_t sceNetTerm() {
	uint32_t retval = Net_Term();

	// mazhaka -> 1 millisecond (ms) = 1,000 microseconds (µs) 1 microsecond (µs) = 1,000 nanoseconds (ns)
	// Give time to make sure everything are cleaned up
	hleEatMicro(adhocDefaultEatDelay);
	return retval;
}

uint32_t sceNetGetLocalEtherAddr(unsigned char* addrAddr) {
	if (adhocctlCurrentMode == ADHOCCTL_MODE_NONE)
		return 0x80410180; //  "address not available?"

	if (addrAddr == NULL) {
		// More correctly, it should crash.
		return SCE_KERNEL_ERROR_ILLEGAL_ADDR;
	}

	uint8_t* addr = (uint8_t*)addrAddr;
#ifndef MAZAHAKA_PC // generate instance mac in config init
	if (GAME_ID > 1) {
		memset(addrAddr, GAME_ID, 6);
		// Making sure the 1st 2-bits on the 1st byte of OUI are zero to prevent issue with some games (ie. Gran Turismo)
		addr[0] &= 0xfc;
	}
	else
#endif
	{
		// Read MAC Address from config
		if (!ParseMacAddress(g_Config.sMACAddress, addr)) {
			ERROR_LOG(Log::sceNet, "Error parsing mac address %s", g_Config.sMACAddress.c_str());
			memset(addrAddr, 0, 6);
		}
	}
	//NotifyMemInfo(MemBlockFlags::WRITE, addrAddr, 6, "WlanEtherAddr");

	hleEatMicro(200);
	return 0;
}

// Probably a void function, but often returns a useful value.
void sceNetEtherNtostr(unsigned char* macPtr, char* bufferPtr) {
	DEBUG_LOG(Log::sceNet, "sceNetEtherNtostr(%08x, %08x)", macPtr, bufferPtr);

	if (macPtr != NULL && bufferPtr != NULL)
	{
		char *buffer = (char *)(bufferPtr);
		const uint8_t *mac = macPtr;

		// MAC address is always 6 bytes / 48 bits.
		sprintf(buffer, "%02x:%02x:%02x:%02x:%02x:%02x",
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

		VERBOSE_LOG(Log::sceNet, "sceNetEtherNtostr - [%s]", buffer);
	}
}
