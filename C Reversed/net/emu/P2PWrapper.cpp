/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once
#include "SocketCompat.h"
#include "proAdhoc.h"
#include "NetAdhocCommon.h"
#include "PSPErrorCodes.h"
#include "MultiGame.h"
#include <vector>
#include <chrono>
#include <thread>
#include <cstring>

#ifdef ADHOC_PTP_PDP_WRAPPERS
uint64_t getCurrentTimeUS() {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}
// Constants
const uint8_t CONN_PDP = 0;
const uint8_t CONN_PTP = 1;
const uint16_t SYN_SIZE = 0;
const uint16_t ACK_SIZE = 65535;
// Common state
struct Datagram {
    std::vector<uint8_t> data;
    uint16_t sender_port;
};
struct ChatConnState {
    SceNetEtherAddr mac; // remote mac
    uint8_t connType;
    uint16_t port; // local port
    uint16_t sender_port; // remote port
    std::vector<uint8_t> pending; // accumulating
    uint16_t expected;
    uint16_t received;
    bool isDone;
    bool isNew; // for PTP accept
    bool connected;
    bool connecting; // for PTP client
    std::vector<Datagram> datagrams; // for PDP
    std::vector<uint8_t> stream; // for PTP
};

std::vector<ChatConnState> chatConnStates;

// PDP Sockets
struct PdpSocket {
    int id;
    SceNetEtherAddr mac; // optional
    uint16_t port;
    int bufferSize;
    uint32_t flag;
};
std::vector<PdpSocket> pdpSockets;
int nextPdpId = 1;

// PTP Sockets
struct PtpSocket {
    int id;
    int state; // 0: closed, 1: open, 2: connecting, 3: connected, 4: listen
    SceNetEtherAddr srcmac;
    uint16_t sport;
    SceNetEtherAddr dstmac;
    uint16_t dport;
    int bufsize;
    int rexmt_int;
    int rexmt_cnt;
    int backlog; // for listen
    int flag;
};

std::vector<PtpSocket> ptpSockets;
int nextPtpId = 1;

// Functions
bool sendP2PBuffer(const uint8_t* pBuffer, uint16_t bufferSize, SceNetEtherAddr* mac, uint16_t port, uint8_t connType, uint16_t sender_port) {
    if (!friendFinderRunning) return false;
    SceNetAdhocctlPDPPTPPacketC2S header = {};
    header.base.opcode = OPCODE_MZHK;
    SceNetEtherAddr localMac;
    getLocalMac(&localMac);
    memcpy(&header.sender_mac, &localMac, sizeof(SceNetEtherAddr));
    header.connectiontype = connType;
    memcpy(&header.target_mac, mac, sizeof(SceNetEtherAddr));
    header.target_port = port;
    header.sender_port = sender_port;
    header.size = bufferSize;
    size_t packetSize = sizeof(header) + ((bufferSize > 0 && bufferSize < 65535) ? bufferSize : 0);
    std::vector<uint8_t> packet(packetSize);
    memcpy(packet.data(), &header, sizeof(header));
    if (bufferSize > 0 && bufferSize < 65535) memcpy(packet.data() + sizeof(header), pBuffer, bufferSize);
    if (IsSocketReady(metasocket, false, true) > 0) {
        int sent = send(metasocket, (const char*)packet.data(), (int)packet.size(), MSG_NOSIGNAL);
        if (sent == (int)packet.size()) {
            NOTICE_LOG(sceNet, "Sent P2P buffer (size=%u, connType=%u)", bufferSize, connType);
            return true;
        }
        ERROR_LOG(sceNet, "Send failed: %d", sent);
    }
    else {
        ERROR_LOG(sceNet, "Socket not ready");
    }
    return false;
}

void onRecvP2PBuffer(const uint8_t* pBuffer, int len) {
    if (len < sizeof(SceNetAdhocctlPDPPTPPacketS2C)) return;
    const SceNetAdhocctlPDPPTPPacketS2C* packet = (const SceNetAdhocctlPDPPTPPacketS2C*)pBuffer;
    if (packet->base.base.opcode != OPCODE_MZHK) return;
    size_t offset = sizeof(SceNetAdhocctlPacketBase);
    SceNetEtherAddr sender_mac;
    memcpy(&sender_mac, pBuffer + offset, sizeof(SceNetEtherAddr)); offset += sizeof(SceNetEtherAddr);
    uint8_t connType = pBuffer[offset++];
    SceNetEtherAddr target_mac;
    memcpy(&target_mac, pBuffer + offset, sizeof(SceNetEtherAddr)); offset += sizeof(SceNetEtherAddr);
    uint16_t target_port = *(uint16_t*)(pBuffer + offset); offset += sizeof(uint16_t);
    uint16_t sender_port = *(uint16_t*)(pBuffer + offset); offset += sizeof(uint16_t);
    uint16_t msg_size = *(uint16_t*)(pBuffer + offset); offset += sizeof(uint16_t);
    if (!isLocalMAC(&target_mac)) return;
    if (len != offset + ((msg_size > 0 && msg_size < 65535) ? msg_size : 0)) return;
    std::vector<uint8_t> chunk;
    if (msg_size > 0 && msg_size < 65535) chunk.assign(pBuffer + offset, pBuffer + offset + msg_size);
    // Find or create state
    auto it = std::find_if(chatConnStates.begin(), chatConnStates.end(), [&](const ChatConnState& s) {
        return memcmp(&s.mac, &sender_mac, sizeof(SceNetEtherAddr)) == 0 && s.connType == connType && s.port == target_port;
        });
    ChatConnState* state = (it != chatConnStates.end()) ? &*it : nullptr;
    if (msg_size == SYN_SIZE || msg_size == ACK_SIZE) {
        // Handle control
        if (msg_size == SYN_SIZE) {
            if (connType == CONN_PTP) {
                // Find listener
                auto lit = std::find_if(ptpSockets.begin(), ptpSockets.end(), [&](const PtpSocket& sock) {
                    return sock.state == 4 && sock.sport == target_port;
                    });
                if (lit != ptpSockets.end()) {
                    // Count pending
                    int pending = 0;
                    for (const auto& cs : chatConnStates) {
                        if (cs.connType == CONN_PTP && cs.port == target_port && cs.isNew) ++pending;
                    }
                    if (pending >= lit->backlog) return; // Full
                    if (!state) {
                        ChatConnState newState{};
                        newState.mac = sender_mac;
                        newState.connType = connType;
                        newState.port = target_port;
                        newState.sender_port = sender_port;
                        newState.isNew = true;
                        newState.connected = false;
                        newState.connecting = false;
                        newState.isDone = false;
                        newState.expected = 0;
                        newState.received = 0;
                        chatConnStates.push_back(newState);
                        state = &chatConnStates.back();
                    }
                    else {
                        state->isNew = true;
                    }
                }
            }
        }
        else if (msg_size == ACK_SIZE) {
            if (state && connType == CONN_PTP) {
                state->connected = true;
                state->connecting = false;
            }
        }
        return;
    }
    // Data
    if (msg_size == 0 || msg_size >= 65535) return;
    if (!state) {
        ChatConnState newState{};
        newState.mac = sender_mac;
        newState.connType = connType;
        newState.port = target_port;
        newState.sender_port = sender_port;
        newState.isNew = (connType == CONN_PTP);
        newState.connected = false;
        newState.connecting = false;
        newState.isDone = false;
        newState.expected = 0;
        newState.received = 0;
        chatConnStates.push_back(newState);
        state = &chatConnStates.back();
    }
    if (state->received == 0 || state->isDone) {
        state->expected = msg_size;
        state->pending.clear();
        state->received = 0;
        state->isDone = false;
    }
    else if (state->expected != msg_size) {
        ERROR_LOG(sceNet, "Size mismatch: expected %u, got %u", state->expected, msg_size);
        state->pending.clear();
        state->received = 0;
        state->isDone = false;
        return;
    }
    state->pending.insert(state->pending.end(), chunk.begin(), chunk.end());
    state->received += (uint16_t)chunk.size();
    if (state->received > state->expected) {
        ERROR_LOG(sceNet, "Overflow: received %u > expected %u", state->received, state->expected);
        state->pending.clear();
        state->received = 0;
        state->isDone = false;
    }
    else if (state->received == state->expected) {
        state->isDone = true;
        if (state->connType == CONN_PDP) {
            Datagram dg;
            dg.data = std::move(state->pending);
            dg.sender_port = state->sender_port;
            state->datagrams.push_back(std::move(dg));
        }
        else if (state->connType == CONN_PTP) {
            state->stream.insert(state->stream.end(), state->pending.begin(), state->pending.end());
        }
        state->pending.clear();
        NOTICE_LOG(sceNet, "Complete buffer %u for connType %u, port %u", state->expected, state->connType, state->port);
        state->received = 0;
        state->expected = 0;
        state->isDone = false;
    }
}

#ifdef ADHOC_PTP_PDP_CHAT_EMU
bool sendP2PChatBuffer(const uint8_t* pBuffer, uint16_t bufferSize, SceNetEtherAddr* mac, uint16_t port, uint8_t connType, uint16_t sender_port) {
    if (!friendFinderRunning) return false;
    bool isControl = (bufferSize == SYN_SIZE || bufferSize == ACK_SIZE);
    uint16_t remaining = isControl ? 0 : bufferSize;
    const uint8_t* ptr = pBuffer;
    std::vector<SceNetAdhocctlChatPDPPTPPacketC2S> chats;
    SceNetAdhocctlChatPDPPTPPacketC2S chat = {};
    chat.base.opcode = OPCODE_CHAT;
    chat.subopcode = OPCODE_MZHK;
    SceNetEtherAddr localMac;
    getLocalMac(&localMac);
    do {
        const size_t payload_size = sizeof(chat.connectiontype) + sizeof(chat.mac) + sizeof(chat.port) + sizeof(chat.realmessagesize) + sizeof(chat.buffer);
        const uint16_t max_binary = payload_size / 2;
        const uint16_t header_size = sizeof(uint8_t) + 2 * sizeof(SceNetEtherAddr) + 2 * sizeof(uint16_t) + sizeof(uint16_t); // connType1 + sender_mac6 + target_mac6 + target_port2 + sender_port2 + bufferSize2 =19
        const uint16_t max_chunk = max_binary - header_size;
        uint16_t chunkSize = isControl ? 0 : std::min(remaining, max_chunk);
        std::vector<uint8_t> binary(header_size + chunkSize);
        size_t off = 0;
        binary[off++] = connType;
        memcpy(&binary[off], &localMac, sizeof(SceNetEtherAddr)); off += sizeof(SceNetEtherAddr);
        memcpy(&binary[off], mac, sizeof(SceNetEtherAddr)); off += sizeof(SceNetEtherAddr);
        memcpy(&binary[off], &port, sizeof(uint16_t)); off += sizeof(uint16_t);
        memcpy(&binary[off], &sender_port, sizeof(uint16_t)); off += sizeof(uint16_t);
        memcpy(&binary[off], &bufferSize, sizeof(uint16_t)); off += sizeof(uint16_t);
        if (!isControl) memcpy(&binary[off], ptr, chunkSize);
        // Hex encode
        char* payload = (char*)&chat.connectiontype;
        memset(payload, ' ', payload_size);
        const char* hex_digits = "0123456789ABCDEF";
        for (size_t i = 0; i < binary.size(); ++i) {
            uint8_t b = binary[i];
            payload[i * 2] = hex_digits[b >> 4];
            payload[i * 2 + 1] = hex_digits[b & 0xF];
        }
        chats.push_back(chat);
        if (!isControl) {
            ptr += chunkSize;
            remaining -= chunkSize;
        }
    } while ((!isControl && remaining > 0) || (isControl && chats.empty()));
    if (!chats.empty()) {
        for (const auto& c : chats) {
            if (IsSocketReady(metasocket, false, true) <= 0) {
                ERROR_LOG(sceNet, "Socket not ready");
                return false;
            }
            int sent = send(metasocket, (const char*)&c, sizeof(c), MSG_NOSIGNAL);
            if (sent != sizeof(c)) {
                ERROR_LOG(sceNet, "Send failed: %d", sent);
                return false;
            }
        }
        NOTICE_LOG(sceNet, "Sent Chat P2P buffer (size=%u, connType=%u)", bufferSize, connType);
        return true;
    }
    return false;
}

void onRecvP2PChatBuffer(const uint8_t* pBuffer) {
    SceNetAdhocctlChatPDPPTPPacketS2C* packet = (SceNetAdhocctlChatPDPPTPPacketS2C*)pBuffer;
    if (packet->base.subopcode != OPCODE_MZHK) return;
    const size_t payload_size = sizeof(packet->base.connectiontype) + sizeof(packet->base.mac) + sizeof(packet->base.port) + sizeof(packet->base.realmessagesize) + sizeof(packet->base.buffer);
    const char* hex_str = (const char*)&packet->base.connectiontype;
    std::vector<uint8_t> binary;
    for (size_t i = 0; i < payload_size; i += 2) {
        if (hex_str[i] == 0 || hex_str[i + 1] == 0) break;
        if (!isxdigit(hex_str[i]) || !isxdigit(hex_str[i + 1])) break;
        char hex[3] = { hex_str[i], hex_str[i + 1], 0 };
        uint8_t byte = (uint8_t)strtoul(hex, nullptr, 16);
        binary.push_back(byte);
    }
    const uint16_t header_size = sizeof(uint8_t) + 2 * sizeof(SceNetEtherAddr) + 2 * sizeof(uint16_t) + sizeof(uint16_t); // 19
    if (binary.size() < header_size) return;
    size_t off = 0;
    uint8_t connType = binary[off++];
    SceNetEtherAddr sender_mac;
    memcpy(&sender_mac, &binary[off], sizeof(SceNetEtherAddr)); off += sizeof(SceNetEtherAddr);
    SceNetEtherAddr target_mac;
    memcpy(&target_mac, &binary[off], sizeof(SceNetEtherAddr)); off += sizeof(SceNetEtherAddr);
    uint16_t target_port = *(uint16_t*)&binary[off]; off += sizeof(uint16_t);
    uint16_t sender_port = *(uint16_t*)&binary[off]; off += sizeof(uint16_t);
    uint16_t msg_size = *(uint16_t*)&binary[off]; off += sizeof(uint16_t);
    std::vector<uint8_t> chunk(binary.begin() + off, binary.end());
    if (!isLocalMAC(&target_mac)) return;
    // Same as onRecvP2PBuffer logic
    auto it = std::find_if(chatConnStates.begin(), chatConnStates.end(), [&](const ChatConnState& s) {
        return memcmp(&s.mac, &sender_mac, sizeof(SceNetEtherAddr)) == 0 && s.connType == connType && s.port == target_port;
        });
    ChatConnState* state = (it != chatConnStates.end()) ? &*it : nullptr;
    if (msg_size == SYN_SIZE || msg_size == ACK_SIZE) {
        // Handle control same as above
        if (msg_size == SYN_SIZE) {
            if (connType == CONN_PTP) {
                auto lit = std::find_if(ptpSockets.begin(), ptpSockets.end(), [&](const PtpSocket& sock) {
                    return sock.state == 4 && sock.sport == target_port;
                    });
                if (lit != ptpSockets.end()) {
                    int pending = 0;
                    for (const auto& cs : chatConnStates) {
                        if (cs.connType == CONN_PTP && cs.port == target_port && cs.isNew) ++pending;
                    }
                    if (pending >= lit->backlog) return;
                    if (!state) {
                        ChatConnState newState{};
                        newState.mac = sender_mac;
                        newState.connType = connType;
                        newState.port = target_port;
                        newState.sender_port = sender_port;
                        newState.isNew = true;
                        newState.connected = false;
                        newState.connecting = false;
                        newState.isDone = false;
                        newState.expected = 0;
                        newState.received = 0;
                        chatConnStates.push_back(newState);
                        state = &chatConnStates.back();
                    }
                    else {
                        state->isNew = true;
                    }
                }
            }
        }
        else if (msg_size == ACK_SIZE) {
            if (state && connType == CONN_PTP) {
                state->connected = true;
                state->connecting = false;
            }
        }
        return;
    }
    // Data same
    if (msg_size == 0 || msg_size >= 65535) return;
    if (!state) {
        ChatConnState newState{};
        newState.mac = sender_mac;
        newState.connType = connType;
        newState.port = target_port;
        newState.sender_port = sender_port;
        newState.isNew = (connType == CONN_PTP);
        newState.connected = false;
        newState.connecting = false;
        newState.isDone = false;
        newState.expected = 0;
        newState.received = 0;
        chatConnStates.push_back(newState);
        state = &chatConnStates.back();
    }
    if (state->received == 0 || state->isDone) {
        state->expected = msg_size;
        state->pending.clear();
        state->received = 0;
        state->isDone = false;
    }
    else if (state->expected != msg_size) {
        ERROR_LOG(sceNet, "Size mismatch: expected %u, got %u", state->expected, msg_size);
        state->pending.clear();
        state->received = 0;
        state->isDone = false;
        return;
    }
    state->pending.insert(state->pending.end(), chunk.begin(), chunk.end());
    state->received += (uint16_t)chunk.size();
    if (state->received > state->expected) {
        ERROR_LOG(sceNet, "Overflow: received %u > expected %u", state->received, state->expected);
        state->pending.clear();
        state->received = 0;
        state->isDone = false;
    }
    else if (state->received == state->expected) {
        state->isDone = true;
        if (state->connType == CONN_PDP) {
            Datagram dg;
            dg.data = std::move(state->pending);
            dg.sender_port = state->sender_port;
            state->datagrams.push_back(std::move(dg));
        }
        else if (state->connType == CONN_PTP) {
            state->stream.insert(state->stream.end(), state->pending.begin(), state->pending.end());
        }
        state->pending.clear();
        NOTICE_LOG(sceNet, "Complete buffer %u for connType %u, port %u", state->expected, state->connType, state->port);
        state->received = 0;
        state->expected = 0;
        state->isDone = false;
    }
}
#endif

// Wrappers
inline int GetAdhocWDefaultError() { 
    return SCE_NET_ADHOC_ERROR_WOULD_BLOCK;
    return SCE_NET_ADHOC_ERROR_TIMEOUT;
    return -1;
}

int sceNetAdhocPdpCreate_W(unsigned char* mac, int port, int bufferSize, int flag) {
    PdpSocket sock{};
    sock.id = nextPdpId++;
    if (mac) memcpy(&sock.mac, mac, sizeof(SceNetEtherAddr));
    sock.port = (uint16_t)port;
    sock.bufferSize = bufferSize;
    sock.flag = flag;
    pdpSockets.push_back(sock);
    return sock.id;
}

int sceNetAdhocPdpDelete_W(int id, int unknown) {
    auto it = std::find_if(pdpSockets.begin(), pdpSockets.end(), [id](const PdpSocket& s) { return s.id == id; });
    if (it == pdpSockets.end()) return GetAdhocWDefaultError(); // ERROR
    pdpSockets.erase(it);
    return 0;
}

int sceNetAdhocPdpSend_W(int id, unsigned char* mac, int port, void* data, int len, int timeout, int flag) {
    auto it = std::find_if(pdpSockets.begin(), pdpSockets.end(), [id](const PdpSocket& s) { return s.id == id; });
    if (it == pdpSockets.end()) return -1;
    PdpSocket& sock = *it;
    SceNetEtherAddr targetMac;
    memcpy(&targetMac, mac, sizeof(SceNetEtherAddr));
    uint16_t targetPort = (uint16_t)port;
    const uint8_t* pBuffer = (const uint8_t*)data;
    uint16_t bufferSize = (uint16_t)len;
    bool success = false;
#ifdef ADHOC_PTP_PDP_CHAT_EMU
    if (g_Config.bPtpPdpDedicatedEmuChatChannel)
        success = sendP2PChatBuffer(pBuffer, bufferSize, &targetMac, targetPort, CONN_PDP, sock.port);
    else
#endif
    success =  sendP2PBuffer(pBuffer, bufferSize, &targetMac, targetPort, CONN_PDP, sock.port);
    return success ? 0 : GetAdhocWDefaultError();
}

int sceNetAdhocPdpRecv_W(int id, void* addr, void* port, void* buf, void* dataLength, int timeout, int flag) {
    auto it = std::find_if(pdpSockets.begin(), pdpSockets.end(), [id](const PdpSocket& s) { return s.id == id; });
    if (it == pdpSockets.end()) return -1;
    PdpSocket& sock = *it;
    int* lenPtr = (int*)dataLength;
    int maxLen = *lenPtr;
    *lenPtr = 0;

    // https://github.com/hrydgard/ppsspp/issues/13452?timeline_page=1
    //uint64_t start = getCurrentTimeUS();
    //uint64_t endTime = start + (uint64_t)timeout;
    //bool nonblock = flag != 0;
    //if (nonblock) endTime = start;

    uint64_t start = getCurrentTimeUS();
    bool nonblock = flag != 0;
    uint64_t endTime;
    if (nonblock) endTime = start;
    else if (timeout == 0) endTime = (uint64_t)-1;
    else endTime = start + (uint64_t)timeout;

    while (true) {
        bool found = false;
        for (auto& state : chatConnStates) {
            if (state.connType == CONN_PDP && state.port == sock.port && !state.datagrams.empty()) {
                Datagram& dg = state.datagrams.front();
                int copyLen = std::min((int)dg.data.size(), maxLen);
                memcpy(buf, dg.data.data(), copyLen);
                *lenPtr = copyLen;
                memcpy(addr, &state.mac, sizeof(SceNetEtherAddr));
                *(uint16_t*)port = dg.sender_port;
                state.datagrams.erase(state.datagrams.begin());
                found = true;
                break;
            }
        }
        if (found) return 0;
        uint64_t now = getCurrentTimeUS();
        if (now >= endTime) return GetAdhocWDefaultError(); // timeout or no data
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return 0;
}

int sceNetAdhocPtpOpen_W(unsigned char* srcmac, int sport, unsigned char* dstmac, int dport, int bufsize, int rexmt_int, int rexmt_cnt, int flag) {
    PtpSocket sock{};
    sock.id = nextPtpId++;
    memcpy(&sock.srcmac, srcmac, sizeof(SceNetEtherAddr));
    sock.sport = (uint16_t)sport;
    memcpy(&sock.dstmac, dstmac, sizeof(SceNetEtherAddr));
    sock.dport = (uint16_t)dport;
    sock.bufsize = bufsize;
    sock.rexmt_int = rexmt_int;
    sock.rexmt_cnt = rexmt_cnt;
    sock.flag = flag;
    sock.state = 1; // open
    ptpSockets.push_back(sock);
    // Create state for client
    ChatConnState newState{};
    newState.mac = sock.dstmac;
    newState.connType = CONN_PTP;
    newState.port = sock.sport;
    newState.sender_port = sock.dport;
    newState.isNew = false;
    newState.connected = false;
    newState.connecting = false;
    newState.isDone = false;
    newState.expected = 0;
    newState.received = 0;
    chatConnStates.push_back(newState);
    return sock.id;
}

int sceNetAdhocPtpConnect_W(int id, int timeout, int flag) {
    auto it = std::find_if(ptpSockets.begin(), ptpSockets.end(), [id](const PtpSocket& s) { return s.id == id; });
    if (it == ptpSockets.end()) return GetAdhocWDefaultError();
    PtpSocket& sock = *it;
    if (sock.state != 1) return GetAdhocWDefaultError();
    // Find state
    auto sit = std::find_if(chatConnStates.begin(), chatConnStates.end(), [&](const ChatConnState& s) {
        return memcmp(&s.mac, &sock.dstmac, sizeof(SceNetEtherAddr)) == 0 && s.connType == CONN_PTP && s.port == sock.sport;
        });
    if (sit == chatConnStates.end()) return GetAdhocWDefaultError();
    ChatConnState& state = *sit;
    // Send SYN
    bool success = false;
#ifdef ADHOC_PTP_PDP_CHAT_EMU
    if (g_Config.bPtpPdpDedicatedEmuChatChannel)
        success = sendP2PChatBuffer(nullptr, SYN_SIZE, &sock.dstmac, sock.dport, CONN_PTP, sock.sport);
    else
#endif
    success = sendP2PBuffer(nullptr, SYN_SIZE, &sock.dstmac, sock.dport, CONN_PTP, sock.sport);
    if (!success) return GetAdhocWDefaultError();
    state.connecting = true;
    sock.state = 2; // connecting

    // https://github.com/hrydgard/ppsspp/issues/13452?timeline_page=1
    //bool nonblock = flag != 0;
    //uint64_t start = getCurrentTimeUS();
    //uint64_t endTime = start + (uint64_t)timeout;
    //if (nonblock) return 0;
    //if (timeout == 0) endTime = (uint64_t)-1; // infinite

    bool nonblock = flag != 0;
    uint64_t start = getCurrentTimeUS();
    uint64_t endTime;
    if (nonblock) {
        return 0;
    }
    else if (timeout == 0) {
        endTime = (uint64_t)-1;
    }
    else {
        endTime = start + (uint64_t)timeout;
    }

    while (true) {
        if (state.connected) {
            sock.state = 3;
            return 0;
        }
        uint64_t now = getCurrentTimeUS();
        if (now >= endTime) return GetAdhocWDefaultError();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return GetAdhocWDefaultError();
}

int sceNetAdhocPtpListen_W(unsigned char* srcmac, int sport, int bufsize, int rexmt_int, int rexmt_cnt, int backlog, int flag) {
    PtpSocket sock{};
    sock.id = nextPtpId++;
    memcpy(&sock.srcmac, srcmac, sizeof(SceNetEtherAddr));
    sock.sport = (uint16_t)sport;
    memset(&sock.dstmac, 0, sizeof(SceNetEtherAddr));
    sock.dport = 0;
    sock.bufsize = bufsize;
    sock.rexmt_int = rexmt_int;
    sock.rexmt_cnt = rexmt_cnt;
    sock.backlog = backlog;
    sock.flag = flag;
    sock.state = 4; // listen
    ptpSockets.push_back(sock);
    return sock.id;
}

int sceNetAdhocPtpAccept_W(int id, unsigned char* peerMacAddrPtr, void* peerPortPtr, int timeout, int flag) {
    auto it = std::find_if(ptpSockets.begin(), ptpSockets.end(), [id](const PtpSocket& s) { return s.id == id; });
    if (it == ptpSockets.end() || it->state != 4) return GetAdhocWDefaultError();
    PtpSocket& listener = *it;

    // https://github.com/hrydgard/ppsspp/issues/13452?timeline_page=1
    //bool nonblock = flag != 0;
    //uint64_t start = getCurrentTimeUS();
    //uint64_t endTime = start + (uint64_t)timeout;
    //if (nonblock) endTime = start;
    //if (timeout == 0 && !nonblock) endTime = (uint64_t)-1;

    bool nonblock = flag != 0;
    uint64_t start = getCurrentTimeUS();
    uint64_t endTime;
    if (nonblock) endTime = start;
    else if (timeout == 0) endTime = (uint64_t)-1;
    else endTime = start + (uint64_t)timeout;

    while (true) {
        for (auto sit = chatConnStates.begin(); sit != chatConnStates.end(); ++sit) {
            if (sit->connType == CONN_PTP && sit->port == listener.sport && sit->isNew) {
                PtpSocket newSock{};
                newSock.id = nextPtpId++;
                newSock.srcmac = listener.srcmac;
                newSock.sport = listener.sport;
                newSock.dstmac = sit->mac;
                newSock.dport = sit->sender_port;
                newSock.bufsize = listener.bufsize;
                newSock.rexmt_int = listener.rexmt_int;
                newSock.rexmt_cnt = listener.rexmt_cnt;
                newSock.flag = listener.flag;
                newSock.state = 3; // connected
                ptpSockets.push_back(newSock);
                // Send ACK
                bool success = false;
#ifdef ADHOC_PTP_PDP_CHAT_EMU
                if (g_Config.bPtpPdpDedicatedEmuChatChannel)
                    success = sendP2PChatBuffer(nullptr, ACK_SIZE, &newSock.dstmac, newSock.dport, CONN_PTP, newSock.sport);
                else
#endif
                success = sendP2PBuffer(nullptr, ACK_SIZE, &newSock.dstmac, newSock.dport, CONN_PTP, newSock.sport);
                if (success) {
                    sit->connected = true;
                    sit->isNew = false;
                }
                if (peerMacAddrPtr) memcpy(peerMacAddrPtr, &sit->mac, sizeof(SceNetEtherAddr));
                if (peerPortPtr) *((uint16_t*)peerPortPtr) = sit->sender_port;
                return newSock.id;
            }
        }
        uint64_t now = getCurrentTimeUS();
        if (now >= endTime) return GetAdhocWDefaultError();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return GetAdhocWDefaultError();
}

int sceNetAdhocPtpSend_W(int id, void* dataAddr, int* dataSizeAddr, int timeout, int flag) {
    auto it = std::find_if(ptpSockets.begin(), ptpSockets.end(), [id](const PtpSocket& s) { return s.id == id; });
    if (it == ptpSockets.end() || it->state != 3) return -1;
    PtpSocket& sock = *it;
    auto sit = std::find_if(chatConnStates.begin(), chatConnStates.end(), [&](const ChatConnState& s) {
        return memcmp(&s.mac, &sock.dstmac, sizeof(SceNetEtherAddr)) == 0 && s.connType == CONN_PTP && s.port == sock.sport;
        });
    if (sit == chatConnStates.end() || !sit->connected) return GetAdhocWDefaultError();
    int len = *dataSizeAddr;
    const uint8_t* pBuffer = (const uint8_t*)dataAddr;
    bool success = false;
#ifdef ADHOC_PTP_PDP_CHAT_EMU
    if (g_Config.bPtpPdpDedicatedEmuChatChannel)
        success = sendP2PChatBuffer(pBuffer, (uint16_t)len, &sock.dstmac, sock.dport, CONN_PTP, sock.sport);
    else
#endif
    success = sendP2PBuffer(pBuffer, (uint16_t)len, &sock.dstmac, sock.dport, CONN_PTP, sock.sport);
    if (success) return 0;
    *dataSizeAddr = 0;
    return GetAdhocWDefaultError();
}

int sceNetAdhocPtpRecv_W(int id, void* dataAddr, int* dataSizeAddr, int timeout, int flag) {
    auto it = std::find_if(ptpSockets.begin(), ptpSockets.end(), [id](const PtpSocket& s) { return s.id == id; });
    if (it == ptpSockets.end() || it->state != 3) return GetAdhocWDefaultError();
    PtpSocket& sock = *it;
    auto sit = std::find_if(chatConnStates.begin(), chatConnStates.end(), [&](const ChatConnState& s) {
        return memcmp(&s.mac, &sock.dstmac, sizeof(SceNetEtherAddr)) == 0 && s.connType == CONN_PTP && s.port == sock.sport;
        });
    if (sit == chatConnStates.end() || !sit->connected) return GetAdhocWDefaultError();
    ChatConnState& state = *sit;
    int maxLen = *dataSizeAddr;
    *dataSizeAddr = 0;

    // https://github.com/hrydgard/ppsspp/issues/13452?timeline_page=1
    //bool nonblock = flag != 0;
    //uint64_t start = getCurrentTimeUS();
    //uint64_t endTime = start + (uint64_t)timeout;
    //if (nonblock) endTime = start;
    //if (timeout == 0 && !nonblock) endTime = (uint64_t)-1;

    bool nonblock = flag != 0;
    uint64_t start = getCurrentTimeUS();
    uint64_t endTime;
    if (nonblock) endTime = start;
    else if (timeout == 0) endTime = (uint64_t)-1;
    else endTime = start + (uint64_t)timeout;

    while (true) {
        if (!state.stream.empty()) {
            int copyLen = std::min((int)state.stream.size(), maxLen);
            memcpy(dataAddr, state.stream.data(), copyLen);
            state.stream.erase(state.stream.begin(), state.stream.begin() + copyLen);
            *dataSizeAddr = copyLen;
            return 0;
        }
        uint64_t now = getCurrentTimeUS();
        if (now >= endTime) return GetAdhocWDefaultError();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int sceNetAdhocPtpClose_W(int id, int unknown) {
    auto it = std::find_if(ptpSockets.begin(), ptpSockets.end(), [id](const PtpSocket& s) { return s.id == id; });
    if (it == ptpSockets.end()) return GetAdhocWDefaultError();
    PtpSocket& sock = *it;
    if (sock.state == 4) { // listen
        // No state
    }
    else { // connection
        auto sit = std::find_if(chatConnStates.begin(), chatConnStates.end(), [&](const ChatConnState& s) {
            return memcmp(&s.mac, &sock.dstmac, sizeof(SceNetEtherAddr)) == 0 && s.connType == CONN_PTP && s.port == sock.sport;
            });
        if (sit != chatConnStates.end()) chatConnStates.erase(sit);
    }
    ptpSockets.erase(it);
    return 0;
}

int sceNetAdhocPtpFlush_W(int id, int timeout, int flag) {
    // No op
    return 0;
}
#endif