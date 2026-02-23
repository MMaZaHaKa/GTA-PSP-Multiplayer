/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once
#include "common.h"
#include <string>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <chrono>
#include <iomanip>

enum LogLevel : int {
	LNOTICE = 0, // min info
	LERROR,
	LWARNING,
	LINFO,
	LDEBUG,
	LVERBOSE, // max info

	LLEVEL_COUNT
};
enum Log : int {
	sceNet = 0,
	sceUtility,
	LMODULE_COUNT
};
extern const char* LogLevelNames[LLEVEL_COUNT];
void LogPrint(Log mdl, LogLevel level, const char* fmt, ...);
#if 1
//#define GENERIC_LOG(t, v, ...) \
//    do { \
//        printf(__VA_ARGS__); \
//        printf("\n"); \
//    } while (false);
#define GENERIC_LOG(t, v, ...) \
    do { \
        LogPrint(t, v, __VA_ARGS__); \
    } while (false);
#else
#define GENERIC_LOG(t, v, ...)
#endif
#define NOTICE_LOG(t,...)  do { GENERIC_LOG(t, LogLevel::LNOTICE,  __VA_ARGS__) } while (false) // small info
#define ERROR_LOG(t,...)   do { GENERIC_LOG(t, LogLevel::LERROR,   __VA_ARGS__) } while (false)
#define WARN_LOG(t,...)    do { GENERIC_LOG(t, LogLevel::LWARNING, __VA_ARGS__) } while (false)
#define INFO_LOG(t,...)    do { GENERIC_LOG(t, LogLevel::LINFO,    __VA_ARGS__) } while (false)
#define DEBUG_LOG(t,...)   do { GENERIC_LOG(t, LogLevel::LDEBUG,   __VA_ARGS__) } while (false)
#define VERBOSE_LOG(t,...) do { GENERIC_LOG(t, LogLevel::LVERBOSE, __VA_ARGS__) } while (false) // spam


enum eJournalSlots
{
    NET_SESSION = 0,
    INTEREST_ZONE,
    MULTIGAME,
    SPED,
    LUA,
    MAIN,
    DEV,
};

#define JOURNAL_LOG(slot, param, fmt, ...) (Journal::Instance().log((slot), (param), (fmt), ##__VA_ARGS__))

// tmp solution for spam switch todo normal + cleanup
void net_session_log(int level, const char* fmt, ...);
//#define NET_SESSION_LOG(level, ...) net_session_log(level, __VA_ARGS__)
#define NET_SESSION_LOG(level, fmt, ...) JOURNAL_LOG(NET_SESSION, 0, fmt, __VA_ARGS__)
void interest_zone_log(int level, const char* fmt, ...);
//#define INTEREST_ZONE_LOG(level, ...) interest_zone_log(level, __VA_ARGS__)
#define INTEREST_ZONE_LOG(level, fmt, ...) JOURNAL_LOG(INTEREST_ZONE, 0, fmt, __VA_ARGS__)
void multigame_log(int level, const char* fmt, ...);
//#define MULTIGAME_LOG(level, ...) multigame_log(level, __VA_ARGS__)
#define MULTIGAME_LOG(level, fmt, ...) JOURNAL_LOG(MULTIGAME, 0, fmt, __VA_ARGS__)
#define MULTIGAME_SLOT_LOG(level, slot, fmt, ...) JOURNAL_LOG(MULTIGAME, slot, fmt, __VA_ARGS__)
void main_log(int level, const char* fmt, ...);
//#define MAIN_LOG(level, ...) main_log(level, __VA_ARGS__)
#define MAIN_LOG(level, fmt, ...) JOURNAL_LOG(MAIN, 0, fmt, __VA_ARGS__)
void lua_log(int level, const char* fmt, ...);
//#define LUA_LOG(level, ...) lua_log(level, __VA_ARGS__)
#define LUA_LOG(level, fmt, ...) JOURNAL_LOG(LUA, 0, fmt, __VA_ARGS__)
#define PED_LOG(level, fmt, ...) JOURNAL_LOG(SPED, 0, fmt, __VA_ARGS__)


class Journal
{
public:
    static Journal& Instance() {
        static Journal inst;
        return inst;
    }

    void log(uint32 slot, uint32 param, const char* fmt, ...);
    std::string dumpSlot(uint32 slot);
    std::string dumpSlotParam(uint32 slot, uint32 param);
    bool saveSlotToFile(uint32 slot, const std::string& filename);

private:
    Journal() {}
    ~Journal() {}

    Journal(const Journal&) = delete;
    Journal& operator=(const Journal&) = delete;

    struct Slot {
        std::mutex mutex;
        std::unordered_map<uint32, std::vector<std::string>> params;
    };

    std::unordered_map<uint32, Slot> slots;
    std::mutex slotsMutex;

    Slot& getOrCreateSlot(uint32 slot);

    static std::string vformat(const char* fmt, va_list args);
    static std::string timestamp();
};