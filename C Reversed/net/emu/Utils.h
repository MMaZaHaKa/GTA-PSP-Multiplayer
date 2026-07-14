/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include <string>
#include <string_view>
#include <map>
#include <vector>

#include "NetAdhocCommon.h"

bool ParseMacAddress(const std::string& str, uint8_t macAddr[6]);
std::string MacToString(const uint8_t mac[SCE_NET_ETHER_ADDR_LEN]);
std::string_view StripSpaces(std::string_view str);
void DataToHexString(const uint8_t* data, size_t size, std::string* output, bool lineBreaks = true);
void DataToHexString(int indent, uintptr_t startAddr, const uint8_t* data, size_t size, std::string* output);
std::string ToCase(const std::string& s, bool Upper);
bool IsUnCaseContains(const std::string& haystack, const std::string& needle);
size_t truncate_cpy(char* dest, size_t destSize, const char* src);
size_t truncate_cpy(char* dest, size_t destSize, std::string_view src);
void ConvertUTF8ToWString(wchar_t* dest, size_t destSize, std::string_view source);
std::string CreateRandMAC();
void sleep_ms(int ms, const char* reason);
void hleEatCycles(int cycles);
void hleEatMicro(int usec);
//void hleDelayResult(int usec);

void SetCurrentThreadName(const char* threadName);
#define __KernelGetCurThread() (0) // dummy

namespace CoreTiming { uint64_t GetGlobalTimeUsScaled(); }
double time_now_d();


// API ==================================================================== (2)
bool sceWlanGetSwitchState();
int sceUtilityGetSystemParamString(int id, char* destAddr, int destSize);
