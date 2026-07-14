/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once
#ifndef PSP_THREAD_ATTR_KERNEL
#define PSP_THREAD_ATTR_KERNEL 0x00001000
#endif
#ifndef PSP_THREAD_ATTR_USER
#define PSP_THREAD_ATTR_USER 0x80000000
#endif

extern bool g_netInited;

void __NetInit();
void __NetShutdown();


// API ==================================================================== (4)
int sceNetInit(uint32_t poolSize, uint32_t calloutPri, uint32_t calloutStack, uint32_t netinitPri, uint32_t netinitStack);
uint32_t sceNetTerm();
uint32_t sceNetGetLocalEtherAddr(unsigned char* addrAddr);
void sceNetEtherNtostr(unsigned char* macPtr, char* bufferPtr); // Probably a void function, but often returns a useful value.