/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include <deque>
#include "proAdhoc.h"

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct ProductStruct { // Similar to SceNetAdhocctlAdhocId ?
	int32_t unknown; // Unknown, set to 0 // Product Type ?
	char product[PRODUCT_CODE_LENGTH]; // Game ID (Example: ULUS10000)
} PACK ProductStruct;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct AdhocctlRequest {
	uint8_t opcode;
	SceNetAdhocctlGroupName group;
};

struct AdhocSendTarget {
	uint32_t ip;
	uint16_t port; // original port
	uint16_t portOffset; // port offset specific for this target IP
};

struct AdhocSendTargets {
	int length;
	std::deque<AdhocSendTarget> peers;
	bool isBroadcast;
};

struct AdhocSocketRequest {
	int type;
	int id; // PDP/PTP socket id
	void* buffer;
	int32_t* length;
	uint32_t timeout;
	uint64_t startTime;
	SceNetEtherAddr* remoteMAC;
	uint16_t* remotePort;
};

enum AdhocSocketRequestType : int
{
	PTP_CONNECT = 0,
	PTP_ACCEPT = 1,
	PTP_SEND = 2,
	PTP_RECV = 3,
	PTP_FLUSH = 4,
	PDP_SEND = 5,
	PDP_RECV = 6,
	ADHOC_POLL_SOCKET = 7,
};

// TODO: maybe move these to sceNetAdhocDiscover?
typedef struct SceNetAdhocDiscoverParam {
    uint32_t unknown1; // SleepMode? (ie. 0 on on Legend Of The Dragon, 1 on Dissidia 012)
    char   groupName[ADHOCCTL_GROUPNAME_LEN];
    uint32_t unknown2; // size of something? (ie. 0x3c on Legend Of The Dragon, 0x14 on Dissidia 012) // Note: the param size is 0x14 may be it can contains extra data too?
    uint32_t result; // inited to 0?
} PACK SceNetAdhocDiscoverParam;

enum AdhocDiscoverStatus : int
{
	NET_ADHOC_DISCOVER_STATUS_NONE = 0,
	NET_ADHOC_DISCOVER_STATUS_IN_PROGRESS = 1,
	NET_ADHOC_DISCOVER_STATUS_COMPLETED = 2,
};

enum AdhocDiscoverResult : int
{
	NET_ADHOC_DISCOVER_RESULT_NO_PEER_FOUND = 0, // Initial value
	NET_ADHOC_DISCOVER_RESULT_CANCELED = 1, // CANCELED or STOPPED?
	NET_ADHOC_DISCOVER_RESULT_PEER_FOUND = 2,
	NET_ADHOC_DISCOVER_RESULT_ABORTED = 3, // Internal Error occured?
};

#include <deque>
#include <map>
extern std::deque<std::pair<uint32_t, uint32_t>> adhocctlEvents;
extern std::map<int, AdhocctlHandler> adhocctlHandlers;

void __NetAdhocInit();
void __NetTriggerCallbacks(); // Fake function for PPSSPP's use.
void __NetAdhocServerShutdown();
void __NetAdhocShutdown();

void __UpdateAdhocctlHandlers(uint32_t flags, uint32_t error);
bool __NetAdhocConnected();

////////////////////////////

int NetAdhocctl_Term();
int NetAdhocctl_GetState();
int NetAdhocctl_Create(const char* groupName);

int NetAdhoc_Term();

// May need to use these from sceNet.cpp
extern bool netAdhocInited; // TODO: keep here
extern bool netAdhocctlInited; // TODO: move to sceNetAdhocctl



int NetAdhoc_SetSocketAlert(int id, int32_t flag);
int NetAdhocPdp_Delete(int id, int unknown);

// API ==================================================================== (10+8+4+2) (24)
// TODO: expose these via "sceNetAdhocctl.h"
// Called from netdialog (and from sceNetApctl)
// NOTE: use hleCall for sceNet* ones!
int sceNetAdhocctlScan();
int sceNetAdhocctlConnect(const char* groupName);
uint32_t sceNetAdhocctlAddHandler(void* handlerPtr, void* handlerArg);
int sceNetAdhocctlDisconnect();
int sceNetAdhocctlJoin(SceNetAdhocctlScanInfo* scanInfoAddr);
int sceNetAdhocctlGetScanInfo(int32_t* sizeAddr, SceNetAdhocctlScanInfo* bufAddr);
int sceNetAdhocctlGetNameByAddr(unsigned char* mac, SceNetAdhocctlNickname* nameAddr);
int sceNetAdhocctlTerm();
int sceNetAdhocctlInit(int stackSize, int prio, SceNetAdhocctlAdhocId* productAddr);
int sceNetAdhocctlCreate(const char* groupName);

uint32_t sceNetAdhocInit();
int sceNetAdhocTerm();

// Exposing those for the matching routines
// NOTE: use hleCall for sceNet* ones!
int sceNetAdhocPdpCreate(unsigned char* mac, int port, int bufferSize, int flag);
int sceNetAdhocPdpSend(int id, unsigned char* mac, int port, void* data, int len, int timeout, int flag);
int sceNetAdhocPdpRecv(int id, void* addr, void* port, void* buf, void* dataLength, int timeout, int flag);
int sceNetAdhocPdpDelete(int id, int unknown);

int sceNetAdhocPtpClose(int id, int unknown);
int sceNetAdhocPtpSend(int id, void* dataAddr, int* dataSizeAddr, int timeout, int flag);
int sceNetAdhocPtpOpen(unsigned char* srcmac, int sport, unsigned char* dstmac, int dport, int bufsize, int rexmt_int, int rexmt_cnt, int flag);
int sceNetAdhocPtpRecv(int id, void* dataAddr, int* dataSizeAddr, int timeout, int flag);
int sceNetAdhocPtpAccept(int id, unsigned char* peerMacAddrPtr, void* peerPortPtr, int timeout, int flag);
int sceNetAdhocPtpListen(unsigned char* srcmac, int sport, int bufsize, int rexmt_int, int rexmt_cnt, int backlog, int flag); // mac SceNetEtherAddr*
int sceNetAdhocPtpConnect(int id, int timeout, int flag);
int sceNetAdhocPtpFlush(int id, int timeout, int flag);
