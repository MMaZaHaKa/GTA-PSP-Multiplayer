/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "proAdhoc.h"

// PSP (x32 pointer)
//#ifdef _MSC_VER
//#pragma pack(push,1)
//#endif
//typedef struct MatchingArgs {
//	uint32_t data[6]; // ContextID, EventID, bufAddr[ to MAC], OptLen, OptAddr[, EntryPoint]
//} PACK MatchingArgs;
//#ifdef _MSC_VER
//#pragma pack(pop)
//#endif

// Struct for MatchingArgs with uintptr_t for pointers
struct MatchingArgs {
    uint32_t id;
    uint32_t event;
    uintptr_t bufAddr;
    uint32_t optlen;
    uintptr_t optAddr;
    uintptr_t handler;
};

#include <vector>
#include <deque>
extern std::vector<SceUID> matchingThreads;
extern std::deque<MatchingArgs> matchingEvents;

int NetAdhocMatching_Term();

void __NetAdhocMatchingInit();
void __NetMatchingCallbacks(); // Fake function for PPSSPP's use.
void __NetAdhocMatchingShutdown();

extern bool netAdhocMatchingInited;
extern int adhocMatchingEventDelay; //30000


// API ==================================================================== (9)
int sceNetAdhocMatchingInit(uint32_t memsize);
int sceNetAdhocMatchingTerm();
int sceNetAdhocMatchingCreate(int mode, int maxnum, int port, int rxbuflen, int hello_int, int keepalive_int, int init_count, int rexmt_int, pspAdhocMatchingCallback callback);
int sceNetAdhocMatchingStart(int matchingId, int evthPri, int evthStack, int inthPri, int inthStack, int optLen, void* optDataAddr);
int sceNetAdhocMatchingStop(int matchingId);
int sceNetAdhocMatchingDelete(int matchingId);
int sceNetAdhocMatchingSelectTarget(int matchingId, unsigned char* macAddress, int optLen, void* optDataPtr);
int sceNetAdhocMatchingCancelTarget(int matchingId, unsigned char* macAddress);
int sceNetAdhocMatchingSetHelloOpt(int matchingId, int optLenAddr, void* optDataAddr);
