/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "Logger.h"
#include <stdarg.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <ctime>

void SetConsoleColor(int32 mode); // main.h
extern int32 gMPNetDebugLogLevel; // MultiGame.h

const char* LogLevelNames[LLEVEL_COUNT] = {
	"NOTICE",
	"ERROR",
	"WARNING",
	"INFO",
	"DEBUG",
	"VERBOSE"
};

const int LogLevelColorMode[LLEVEL_COUNT] = {
	1, // NOTICE  -> green
	0, // ERROR   -> red
	3, // WARNING -> yellow (red+green)
	4, // INFO    -> cyan (green+blue)
	5, // DEBUG   -> magenta (red+blue)
	6  // VERBOSE -> white
};

void LogPrint(Log mdl, LogLevel level, const char* fmt, ...)
{
	if ((int32)level > gMPNetDebugLogLevel)
		return;

	if (level < 0 || level >= LLEVEL_COUNT) level = LINFO;
	if (mdl < 0 || mdl >= LMODULE_COUNT) mdl = (Log)0;

	SetConsoleColor(LogLevelColorMode[level]);
	printf("[");
	printf("%s", LogLevelNames[level]);
	printf("] ");

	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	SetConsoleColor(6);

	printf("\n");
}


static void log_message(const char* prefix, int level, const char* fmt, va_list args) {
    //if (level > g_log_level) return;

    //std::lock_guard<std::mutex> lock(g_log_mutex);

    const char* level_str = "";
    switch (level) {
    case 0: level_str = "ERROR"; break;
    case 1: level_str = "WARN";  break;
    case 2: level_str = "INFO";  break;
    case 3: level_str = "DEBUG"; break;
    case 4: level_str = "TRACE"; break;
    default: level_str = "UNKNOWN"; break;
    }

    printf("[%s][%s] ", prefix, level_str);

    vprintf(fmt, args);
    //printf("\n");

    //if (level == LOG_LEVEL_ERROR) {
    //    fprintf(stderr, "[%s][%s] ", prefix, level_str);
    //    vfprintf(stderr, fmt, args);
    //    fprintf(stderr, "\n");
    //}
}

void net_session_log(int level, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_message("NET_SESSION", level, fmt, args);
    va_end(args);
}

void interest_zone_log(int level, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_message("INTEREST_ZONE", level, fmt, args);
    va_end(args);
}

void multigame_log(int level, const char* fmt, ...) {
    SetConsoleColor(3);
    va_list args;
    va_start(args, fmt);
    log_message("MULTIGAME", level, fmt, args);
    va_end(args);
    SetConsoleColor(6);
}

void main_log(int level, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_message("MAIN", level, fmt, args);
    va_end(args);
}

void lua_log(int level, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_message("LUA", level, fmt, args);
    va_end(args);
}


void Journal::log(uint32 slot, uint32 param, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    std::string msg = vformat(fmt, args);
    va_end(args);

    std::string entry = timestamp() + " " + msg;

    Slot& s = getOrCreateSlot(slot);

    {
        std::lock_guard<std::mutex> lk(s.mutex);
        s.params[param].push_back(std::move(entry));
    }
}

std::string Journal::dumpSlot(uint32 slot) {
    auto it = slots.find(slot);
    if (it == slots.end()) return std::string("Slot not found\n");

    Slot& s = it->second;
    std::lock_guard<std::mutex> lk(s.mutex);
    std::ostringstream oss;
    oss << "=== Journal Dump Slot " << slot << " ===\n";
    for (const auto& kv : s.params) {
        oss << "Param " << kv.first << ":\n";
        for (const auto& line : kv.second) {
            oss << "  " << line << "\n";
        }
    }
    oss << "=== End Dump Slot " << slot << " ===\n";
    return oss.str();
}

std::string Journal::dumpSlotParam(uint32 slot, uint32 param) {
    auto it = slots.find(slot);
    if (it == slots.end()) return std::string("Slot not found\n");

    Slot& s = it->second;
    std::lock_guard<std::mutex> lk(s.mutex);
    std::ostringstream oss;
    oss << "=== Journal Dump Slot " << slot << " Param " << param << " ===\n";
    auto pIt = s.params.find(param);
    if (pIt == s.params.end()) {
        oss << "(no entries)\n";
    }
    else {
        for (const auto& line : pIt->second) {
            oss << "  " << line << "\n";
        }
    }
    oss << "=== End ===\n";
    return oss.str();
}

bool Journal::saveSlotToFile(uint32 slot, const std::string& filename) {
    auto it = slots.find(slot);
    if (it == slots.end()) return false;

    Slot& s = it->second;
    std::lock_guard<std::mutex> lk(s.mutex);

    std::ofstream ofs(filename.c_str(), std::ios::out | std::ios::trunc);
    if (!ofs.is_open()) return false;

    ofs << "=== Slot " << slot << " dump ===\n";
    for (const auto& kv : s.params) {
        ofs << "Param " << kv.first << ":\n";
        for (const auto& line : kv.second) {
            ofs << line << "\n";
        }
    }
    ofs << "=== End ===\n";
    ofs.close();
    return true;
}

Journal::Slot& Journal::getOrCreateSlot(uint32 slot) {
    std::lock_guard<std::mutex> lk(slotsMutex);
    return slots[slot];
}

std::string Journal::vformat(const char* fmt, va_list args) {
    va_list tmp;
    va_copy(tmp, args);
    int needed = vsnprintf(nullptr, 0, fmt, tmp);
    va_end(tmp);
    if (needed <= 0) return std::string();

    std::vector<char> buf(static_cast<size_t>(needed) + 1);
    vsnprintf(buf.data(), buf.size(), fmt, args);
    return std::string(buf.data(), buf.data() + needed);
}

std::string Journal::timestamp() {
    using namespace std::chrono;
    auto now = system_clock::now();
    std::time_t t = system_clock::to_time_t(now);
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    char buf[64];
    std::tm tm;
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d.%03lld",
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec,
        static_cast<long long>(ms.count()));
    return std::string(buf);
}
