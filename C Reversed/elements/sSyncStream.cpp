/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "sSyncStream.h"
#include "main.h" // debug

// Read 
void sReadSyncStream::ReadVector(CVector& vec) {
    // WARN! CVuVector in PSP PS2 16b, xyzw, but w missed as unnecessary (probably rw::V3d)
    //SYNC_STREAM_CAN_READ(sizeof(CVector)); // 16
    //LOG_READ_POINTER(sizeof(CVector)); // 16
    SYNC_STREAM_CAN_READ(sizeof(rw::V3d));
    LOG_READ_POINTER(sizeof(rw::V3d));
    ReadFloat(vec.x);
    ReadFloat(vec.y);
    ReadFloat(vec.z);
    // w missed
}

void sReadSyncStream::ReadVector2D(CVector2D& vec2D) {
    SYNC_STREAM_CAN_READ(sizeof(CVector2D));
    LOG_READ_POINTER(sizeof(CVector2D));
    ReadFloat(vec2D.x);
    ReadFloat(vec2D.y);
}

void sReadSyncStream::ReadString(base::string& string) {
    SYNC_STREAM_CAN_READ(sizeof(uint8));
    LOG_READ_POINTER(sizeof(uint8));
    uint8 len = 0;
    ReadU8(len);
    SYNC_STREAM_CAN_READ(len);
    LOG_READ_POINTER(len);
    char* buf = new char[len + 1];
    for (uint8 i = 0; i < len; ++i) {
        uint8 ch = 0;
        ReadU8(ch);
        buf[i] = static_cast<char>(ch);
    }
    buf[len] = '\0';
    string = base::string(buf);
    delete[] buf;
}

void sReadSyncStream::ReadColour(CRGBA& colour) {
    SYNC_STREAM_CAN_READ(sizeof(CRGBA));
    LOG_READ_POINTER(sizeof(CRGBA));
    ReadU8(colour.red);
    ReadU8(colour.green);
    ReadU8(colour.blue);
    ReadU8(colour.alpha);
}

void sReadSyncStream::ReadColour24(CRGBA& colour) {
    SYNC_STREAM_CAN_READ(sizeof(3));
    LOG_READ_POINTER(sizeof(3));
    ReadU8(colour.red);
    ReadU8(colour.green);
    ReadU8(colour.blue);
    //colour.alpha = 0;
}

void sReadSyncStream::ReadBool(bool& bVal) {
    //SYNC_STREAM_CAN_READ(sizeof(bool)); // ps2 bool 4b
    //LOG_READ_POINTER(sizeof(bool)); // ps2 bool 4b
    SYNC_STREAM_CAN_READ(sizeof(uint8));
    LOG_READ_POINTER(sizeof(uint8));
    uint8 uVal;
    ReadU8(uVal);
    bVal = !!static_cast<bool>(uVal);
}

void sReadSyncStream::ReadI8(int8& nVal) {
    SYNC_STREAM_CAN_READ(sizeof(int8));
    LOG_READ_POINTER(sizeof(int8));
    uint8 uVal;
    ReadU8(uVal);
    nVal = static_cast<int8>(uVal);
}

void sReadSyncStream::ReadU8(uint8& nVal) {
    SYNC_STREAM_CAN_READ(sizeof(uint8));
    LOG_READ_POINTER(sizeof(uint8));
    uint8* ptr = Tellg();
    nVal = *ptr;
    AddLength(1);
}

void sReadSyncStream::ReadI16(int16& nVal) {
    SYNC_STREAM_CAN_READ(sizeof(int16));
    LOG_READ_POINTER(sizeof(int16));
    uint16 uVal;
    ReadU16(uVal);
    nVal = static_cast<int16>(uVal);
}

void sReadSyncStream::ReadU16(uint16& nVal) {
    SYNC_STREAM_CAN_READ(sizeof(uint16));
    LOG_READ_POINTER(sizeof(uint16));
    uint8 low, high;
    ReadU8(low);
    ReadU8(high);
    nVal = static_cast<uint16>(low) | (static_cast<uint16>(high) << 8);
}

void sReadSyncStream::ReadI32(int32& nVal) {
    SYNC_STREAM_CAN_READ(sizeof(int32));
    LOG_READ_POINTER(sizeof(int32));
    uint32 uVal;
    ReadU32(uVal);
    nVal = static_cast<int32>(uVal);
}

void sReadSyncStream::ReadU32(uint32& nVal) {
    SYNC_STREAM_CAN_READ(sizeof(uint32));
    LOG_READ_POINTER(sizeof(uint32));
    uint8 b0, b1, b2, b3;
    ReadU8(b0);
    ReadU8(b1);
    ReadU8(b2);
    ReadU8(b3);
    nVal = static_cast<uint32>(b0) |
        (static_cast<uint32>(b1) << 8) |
        (static_cast<uint32>(b2) << 16) |
        (static_cast<uint32>(b3) << 24);
}

void sReadSyncStream::ReadI64(int64& nVal) {
    SYNC_STREAM_CAN_READ(sizeof(int64));
    LOG_READ_POINTER(sizeof(int64));
    uint64 uVal;
    ReadU64(uVal);
    nVal = static_cast<int64>(uVal);
}

void sReadSyncStream::ReadU64(uint64& nVal) {
    SYNC_STREAM_CAN_READ(sizeof(uint64));
    LOG_READ_POINTER(sizeof(uint64));
    uint8 b0, b1, b2, b3, b4, b5, b6, b7;
    ReadU8(b0);
    ReadU8(b1);
    ReadU8(b2);
    ReadU8(b3);
    ReadU8(b4);
    ReadU8(b5);
    ReadU8(b6);
    ReadU8(b7);
    nVal = static_cast<uint64>(b0) |
        (static_cast<uint64>(b1) << 8) |
        (static_cast<uint64>(b2) << 16) |
        (static_cast<uint64>(b3) << 24) |
        (static_cast<uint64>(b4) << 32) |
        (static_cast<uint64>(b5) << 40) |
        (static_cast<uint64>(b6) << 48) |
        (static_cast<uint64>(b7) << 56);
}

void sReadSyncStream::ReadFloat(float& fVal) {
    SYNC_STREAM_CAN_READ(sizeof(float));
    LOG_READ_POINTER(sizeof(float));
    uint32 u;
    ReadU32(u);
    union { float f; uint32 u; } conv;
    conv.u = u;
    fVal = conv.f;
}

void sReadSyncStream::ReadBuffer(uint8* pBuff, int32 nSize) {
    SYNC_STREAM_CAN_READ(nSize);
    LOG_READ_POINTER(nSize);
    for (int32 i = 0; i < nSize; ++i) {
        ReadU8(pBuff[i]);
    }
}

CVector sReadSyncStream::ReadVector() {
    CVector v;
    ReadVector(v);
    return v;
}

CVector2D sReadSyncStream::ReadVector2D() {
    CVector2D v;
    ReadVector2D(v);
    return v;
}

base::string sReadSyncStream::ReadString() {
    base::string s;
    ReadString(s);
    return s;
}

CRGBA sReadSyncStream::ReadColour() {
    CRGBA c;
    ReadColour(c);
    return c;
}

CRGBA sReadSyncStream::ReadColour24() {
    CRGBA c;
    ReadColour24(c);
    return c;
}

bool sReadSyncStream::ReadBool() {
    bool v;
    ReadBool(v);
    return v;
}

int8 sReadSyncStream::ReadI8() {
    int8 v;
    ReadI8(v);
    return v;
}

uint8 sReadSyncStream::ReadU8() {
    uint8 v;
    ReadU8(v);
    return v;
}

int16 sReadSyncStream::ReadI16() {
    int16 v;
    ReadI16(v);
    return v;
}

uint16 sReadSyncStream::ReadU16() {
    uint16 v;
    ReadU16(v);
    return v;
}

int32 sReadSyncStream::ReadI32() {
    int32 v;
    ReadI32(v);
    return v;
}

uint32 sReadSyncStream::ReadU32() {
    uint32 v;
    ReadU32(v);
    return v;
}

int64 sReadSyncStream::ReadI64() {
    int64 v;
    ReadI64(v);
    return v;
}

uint64 sReadSyncStream::ReadU64() {
    uint64 v;
    ReadU64(v);
    return v;
}

float sReadSyncStream::ReadFloat() {
    float f;
    ReadFloat(f);
    return f;
}

// PEEK
void sReadSyncStream::PeekVector(CVector& vec) {
    // WARN! CVuVector in PSP PS2 16b, xyzw, but w missed as unnecessary (probably rw::V3d)
    //SYNC_STREAM_CAN_READ(sizeof(CVector)); // 16
    //LOG_READ_POINTER(sizeof(CVector)); // 16
    uint8* saved = Tellg();
    ReadVector(vec);
    Seekg(saved);
    // w missed
}

void sReadSyncStream::PeekVector2D(CVector2D& vec2D) {
    SYNC_STREAM_CAN_READ(sizeof(CVector2D));
    LOG_READ_POINTER(sizeof(CVector2D));
    uint8* saved = Tellg();
    ReadVector2D(vec2D);
    Seekg(saved);
}

void sReadSyncStream::PeekString(base::string& string) {
    SYNC_STREAM_CAN_READ(sizeof(uint8));
    LOG_READ_POINTER(sizeof(uint8));
    uint8* saved = Tellg();
    ReadString(string);
    Seekg(saved);
}

void sReadSyncStream::PeekColour(CRGBA& colour) {
    SYNC_STREAM_CAN_READ(sizeof(CRGBA));
    LOG_READ_POINTER(sizeof(CRGBA));
    uint8* saved = Tellg();
    ReadColour(colour);
    Seekg(saved);
}

void sReadSyncStream::PeekBool(bool& bVal) {
    //SYNC_STREAM_CAN_READ(sizeof(bool)); // ps2 bool 4b
    //LOG_READ_POINTER(sizeof(bool)); // ps2 bool 4b
    SYNC_STREAM_CAN_READ(sizeof(uint8));
    LOG_READ_POINTER(sizeof(uint8));
    uint8* saved = Tellg();
    ReadBool(bVal);
    Seekg(saved);
}

void sReadSyncStream::PeekI8(int8& nVal) {
    SYNC_STREAM_CAN_READ(sizeof(int8));
    LOG_READ_POINTER(sizeof(int8));
    uint8* saved = Tellg();
    ReadI8(nVal);
    Seekg(saved);
}

void sReadSyncStream::PeekU8(uint8& nVal) {
    SYNC_STREAM_CAN_READ(sizeof(uint8));
    LOG_READ_POINTER(sizeof(uint8));
    uint8* saved = Tellg();
    ReadU8(nVal);
    Seekg(saved);
}

void sReadSyncStream::PeekI16(int16& nVal) {
    SYNC_STREAM_CAN_READ(sizeof(int16));
    LOG_READ_POINTER(sizeof(int16));
    uint8* saved = Tellg();
    ReadI16(nVal);
    Seekg(saved);
}

void sReadSyncStream::PeekU16(uint16& nVal) {
    SYNC_STREAM_CAN_READ(sizeof(uint16));
    LOG_READ_POINTER(sizeof(uint16));
    uint8* saved = Tellg();
    ReadU16(nVal);
    Seekg(saved);
}

void sReadSyncStream::PeekI32(int32& nVal) {
    SYNC_STREAM_CAN_READ(sizeof(int32));
    LOG_READ_POINTER(sizeof(int32));
    uint8* saved = Tellg();
    ReadI32(nVal);
    Seekg(saved);
}

void sReadSyncStream::PeekU32(uint32& nVal) {
    SYNC_STREAM_CAN_READ(sizeof(uint32));
    LOG_READ_POINTER(sizeof(uint32));
    uint8* saved = Tellg();
    ReadU32(nVal);
    Seekg(saved);
}

void sReadSyncStream::PeekI64(int64& nVal) {
    SYNC_STREAM_CAN_READ(sizeof(int64));
    LOG_READ_POINTER(sizeof(int64));
    uint8* saved = Tellg();
    ReadI64(nVal);
    Seekg(saved);
}

void sReadSyncStream::PeekU64(uint64& nVal) {
    SYNC_STREAM_CAN_READ(sizeof(uint64));
    LOG_READ_POINTER(sizeof(uint64));
    uint8* saved = Tellg();
    ReadU64(nVal);
    Seekg(saved);
}

void sReadSyncStream::PeekFloat(float& fVal) {
    SYNC_STREAM_CAN_READ(sizeof(float));
    LOG_READ_POINTER(sizeof(float));
    uint8* saved = Tellg();
    ReadFloat(fVal);
    Seekg(saved);
}

void sReadSyncStream::PeekBuffer(uint8* pBuff, int32 nSize) {
    SYNC_STREAM_CAN_READ(nSize);
    LOG_READ_POINTER(nSize);
    uint8* saved = Tellg();
    ReadBuffer(pBuff, nSize);
    Seekg(saved);
}

CVector sReadSyncStream::PeekVector() {
    CVector v;
    PeekVector(v);
    return v;
}

CVector2D sReadSyncStream::PeekVector2D() {
    CVector2D v;
    PeekVector2D(v);
    return v;
}

base::string sReadSyncStream::PeekString() {
    base::string s;
    PeekString(s);
    return s;
}

CRGBA sReadSyncStream::PeekColour() {
    CRGBA c;
    PeekColour(c);
    return c;
}

bool sReadSyncStream::PeekBool() {
    bool v;
    PeekBool(v);
    return v;
}

int8 sReadSyncStream::PeekI8() {
    int8 v;
    PeekI8(v);
    return v;
}

uint8 sReadSyncStream::PeekU8() {
    uint8 v;
    PeekU8(v);
    return v;
}

int16 sReadSyncStream::PeekI16() {
    int16 v;
    PeekI16(v);
    return v;
}

uint16 sReadSyncStream::PeekU16() {
    uint16 v;
    PeekU16(v);
    return v;
}

int32 sReadSyncStream::PeekI32() {
    int32 v;
    PeekI32(v);
    return v;
}

uint32 sReadSyncStream::PeekU32() {
    uint32 v;
    PeekU32(v);
    return v;
}

int64 sReadSyncStream::PeekI64() {
    int64 v;
    PeekI64(v);
    return v;
}

uint64 sReadSyncStream::PeekU64() {
    uint64 v;
    PeekU64(v);
    return v;
}

float sReadSyncStream::PeekFloat() {
    float v;
    PeekFloat(v);
    return v;
}


void sReadSyncStream::DumpContents() {
    // TODO: usage DataToHexString
    void OpenConsole();
    OpenConsole();
    uintptr remaining = m_pBufferEnd - m_pBuffer;
    printf("Remaining length: %zu\nDump:\n", remaining);
    uint8* data = m_pBuffer;
    for (uintptr i = 0; i < remaining; ++i) {
        printf("0x%02X ", data[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}

// Write
void sWriteSyncStream::WriteVector(const CVector& vec) {
    // WARN! CVuVector in PSP PS2 16b, xyzw, but w missed as unnecessary (probably rw::V3d)
    //SYNC_STREAM_CAN_WRITE(sizeof(CVector)); // 16
    //LOG_WRITE_POINTER(sizeof(CVector)); // 16
    SYNC_STREAM_CAN_WRITE(sizeof(rw::V3d));
    LOG_WRITE_POINTER(sizeof(rw::V3d));
    WriteFloat(vec.x);
    WriteFloat(vec.y);
    WriteFloat(vec.z);
}

void sWriteSyncStream::WriteVector2D(const CVector2D& vec2D) {
    SYNC_STREAM_CAN_WRITE(sizeof(CVector2D));
    LOG_WRITE_POINTER(sizeof(CVector2D));
    WriteFloat(vec2D.x);
    WriteFloat(vec2D.y);
}

void sWriteSyncStream::WriteString(const base::string& string) {
    SYNC_STREAM_CAN_WRITE(sizeof(uint8));
    LOG_WRITE_POINTER(sizeof(uint8));
    uint8 len = static_cast<uint8>(string.length());
    WriteU8(len);
    SYNC_STREAM_CAN_WRITE(len);
    LOG_WRITE_POINTER(len);
    for (int32 i = 0; i < static_cast<int32>(len); ++i) {
        WriteU8(static_cast<uint8>(string[i]));
    }
}

void sWriteSyncStream::WriteColour(const CRGBA& colour) {
    SYNC_STREAM_CAN_WRITE(sizeof(CRGBA));
    LOG_WRITE_POINTER(sizeof(CRGBA));
    WriteU8(colour.red);
    WriteU8(colour.green);
    WriteU8(colour.blue);
    WriteU8(colour.alpha);
}

void sWriteSyncStream::WriteColour24(const CRGBA& colour) {
    SYNC_STREAM_CAN_WRITE(3);
    LOG_WRITE_POINTER(3);
    WriteU8(colour.red);
    WriteU8(colour.green);
    WriteU8(colour.blue);
}

void sWriteSyncStream::WriteBool(bool bVal) {
    //SYNC_STREAM_CAN_WRITE(sizeof(bool)); // ps2 bool 4b
    //LOG_WRITE_POINTER(sizeof(bool)); // ps2 bool 4b
    SYNC_STREAM_CAN_WRITE(sizeof(uint8));
    LOG_WRITE_POINTER(sizeof(uint8));
    WriteU8(static_cast<uint8>(bVal));
}

void sWriteSyncStream::WriteI8(int8 nVal) {
    SYNC_STREAM_CAN_WRITE(sizeof(int8));
    LOG_WRITE_POINTER(sizeof(int8));
    WriteU8(static_cast<uint8>(nVal));
}

void sWriteSyncStream::WriteU8(uint8 nVal) {
    SYNC_STREAM_CAN_WRITE(sizeof(uint8));
    LOG_WRITE_POINTER(sizeof(uint8));
    uint8* ptr = Tellg();
    *ptr = nVal;
    AddLength(1);
}

void sWriteSyncStream::WriteI16(int16 nVal) {
    SYNC_STREAM_CAN_WRITE(sizeof(int16));
    LOG_WRITE_POINTER(sizeof(int16));
    WriteU16(static_cast<uint16>(nVal));
}

void sWriteSyncStream::WriteU16(uint16 nVal) {
    SYNC_STREAM_CAN_WRITE(sizeof(uint16));
    LOG_WRITE_POINTER(sizeof(uint16));
    WriteU8(nVal & 0xFF);
    WriteU8((nVal >> 8) & 0xFF);
}

void sWriteSyncStream::WriteI32(int32 nVal) {
    SYNC_STREAM_CAN_WRITE(sizeof(int32));
    LOG_WRITE_POINTER(sizeof(int32));
    WriteU32(static_cast<uint32>(nVal));
}

void sWriteSyncStream::WriteU32(uint32 nVal) {
    SYNC_STREAM_CAN_WRITE(sizeof(uint32));
    LOG_WRITE_POINTER(sizeof(uint32));
    WriteU8(nVal & 0xFF);
    WriteU8((nVal >> 8) & 0xFF);
    WriteU8((nVal >> 16) & 0xFF);
    WriteU8((nVal >> 24) & 0xFF);
}

void sWriteSyncStream::WriteI64(int64 nVal) {
    SYNC_STREAM_CAN_WRITE(sizeof(int64));
    LOG_WRITE_POINTER(sizeof(int64));
    WriteU64(static_cast<uint64>(nVal));
}

void sWriteSyncStream::WriteU64(uint64 nVal) {
    SYNC_STREAM_CAN_WRITE(sizeof(uint64));
    LOG_WRITE_POINTER(sizeof(uint64));
    WriteU8(static_cast<uint8>(nVal & 0xFF));
    WriteU8(static_cast<uint8>((nVal >> 8) & 0xFF));
    WriteU8(static_cast<uint8>((nVal >> 16) & 0xFF));
    WriteU8(static_cast<uint8>((nVal >> 24) & 0xFF));
    WriteU8(static_cast<uint8>((nVal >> 32) & 0xFF));
    WriteU8(static_cast<uint8>((nVal >> 40) & 0xFF));
    WriteU8(static_cast<uint8>((nVal >> 48) & 0xFF));
    WriteU8(static_cast<uint8>((nVal >> 56) & 0xFF));
}

void sWriteSyncStream::WriteFloat(float fVal) {
    SYNC_STREAM_CAN_WRITE(sizeof(float));
    LOG_WRITE_POINTER(sizeof(float));
    union { float f; uint32 u; } conv;
    conv.f = fVal;
    WriteU32(conv.u);
}

void sWriteSyncStream::WriteBuffer(uint8* pBuff, int32 nSize) {
    SYNC_STREAM_CAN_WRITE(nSize);
    LOG_WRITE_POINTER(nSize);
    for (int32 i = 0; i < nSize; ++i) {
        WriteU8(pBuff[i]);
    }
}

void sWriteSyncStream::DumpContents() {
    // TODO: usage DataToHexString
    void OpenConsole();
    OpenConsole();
    printf("Length: %u\nDump:\n", pckt_size);
    uint8* pDdata = GetBuffer();
    for (uint16 i = 0; i < pckt_size - sizeof(pckt_size); ++i) {
        printf("0x%02X ", pDdata[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}