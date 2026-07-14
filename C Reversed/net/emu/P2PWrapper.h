/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once
#include "SocketCompat.h"
#include "proAdhoc.h"
#include "NetAdhocCommon.h"

//// C2S Chat PDP PTP Emu (Custom compat aemu) // socom and similar hack to spread message from cgnat
//typedef struct {
//	SceNetAdhocctlPacketBase base; // OPCODE_CHAT
//	uint8_t subopcode; // OPCODE_MZHK // to detect real chat or buffer hack
//	uint8_t connectiontype; // pdp ptp, can some flags
//	SceNetEtherAddr mac; // sender to recv
//	uint8_t port; // pdp ptp port
//	uint16_t realmessagesize; // hello, 136 / 53 ~x3 chat message buffer sizeof(data)
//	uint8_t buffer[53]; // remaining space
//} PACK SceNetAdhocctlChatPDPPTPPacketC2S;
//
// C2S PDP PTP
//typedef struct {
//	SceNetAdhocctlPacketBase base; // OPCODE_CHAT
//	SceNetEtherAddr sender_mac; // sender mac
//	uint8_t connectiontype; // pdp ptp, can some flags
//	SceNetEtherAddr target_mac; // target mac
//	uint16_t target_port; // target port
//	uint16_t sender_port; // sender port
//	uint16_t size; // data size anter this header
//} PACK SceNetAdhocctlPDPPTPPacketC2S;
//
//// S2C Chat PDP PTP Emu (Custom compat aemu) // socom and similar hack to spread message from cgnat
//typedef struct {
//	SceNetAdhocctlChatPDPPTPPacketC2S base;
//	SceNetAdhocctlNickname name;
//} PACK SceNetAdhocctlChatPDPPTPPacketS2C;
//
//// S2C PDP PTP
//typedef struct {
//	SceNetAdhocctlPDPPTPPacketC2S base;
//} PACK SceNetAdhocctlPDPPTPPacketS2C;

//Extra stucts into proAdhoc.h
bool sendP2PBuffer(const uint8_t* pBuffer, uint16_t bufferSize, SceNetEtherAddr* mac, uint16_t port, uint8_t connType, uint16_t sender_port);
void onRecvP2PBuffer(const uint8_t* pBuffer, int len);

#ifdef ADHOC_PTP_PDP_CHAT_EMU
// subopcode, need decode from strncpy
bool sendP2PChatBuffer(const uint8_t* pBuffer, uint16_t bufferSize, SceNetEtherAddr* mac, uint16_t port, uint8_t connType, uint16_t sender_port);
void onRecvP2PChatBuffer(const uint8_t* pBuffer);
#endif

// Wrappers
int sceNetAdhocPdpCreate_W(unsigned char* mac, int port, int bufferSize, int flag);
int sceNetAdhocPdpSend_W(int id, unsigned char* mac, int port, void* data, int len, int timeout, int flag);
int sceNetAdhocPdpRecv_W(int id, void* addr, void* port, void* buf, void* dataLength, int timeout, int flag);
int sceNetAdhocPdpDelete_W(int id, int unknown);

int sceNetAdhocPtpClose_W(int id, int unknown);
int sceNetAdhocPtpSend_W(int id, void* dataAddr, int* dataSizeAddr, int timeout, int flag);
int sceNetAdhocPtpOpen_W(unsigned char* srcmac, int sport, unsigned char* dstmac, int dport, int bufsize, int rexmt_int, int rexmt_cnt, int flag);
int sceNetAdhocPtpRecv_W(int id, void* dataAddr, int* dataSizeAddr, int timeout, int flag);
int sceNetAdhocPtpAccept_W(int id, unsigned char* peerMacAddrPtr, void* peerPortPtr, int timeout, int flag);
int sceNetAdhocPtpListen_W(unsigned char* srcmac, int sport, int bufsize, int rexmt_int, int rexmt_cnt, int backlog, int flag); // mac SceNetEtherAddr*
int sceNetAdhocPtpConnect_W(int id, int timeout, int flag);
int sceNetAdhocPtpFlush_W(int id, int timeout, int flag);
