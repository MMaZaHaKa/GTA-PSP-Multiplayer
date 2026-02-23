#include "Utils.h"
#include "NetAdhocCommon.h"
#include "PSPErrorCodes.h"
#include "MultiGame.h" // nick
#ifdef _WIN32
#include "Windows.h" // Sleep
#endif
#include <sstream>
#include <algorithm>

bool ParseMacAddress(const std::string& str, uint8_t macAddr[6]) {
	unsigned int mac[6];
	if (6 != sscanf(str.c_str(), "%02x:%02x:%02x:%02x:%02x:%02x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5])) {
		return false;
	}
	for (int i = 0; i < 6; i++) {
		macAddr[i] = mac[i];
	}
	return true;
}

std::string MacToString(const uint8_t mac[SCE_NET_ETHER_ADDR_LEN]) {
	char buf[18]; // "xx:xx:xx:xx:xx:xx" + '\0' = 18
	std::snprintf(buf, sizeof(buf),
		"%02x:%02x:%02x:%02x:%02x:%02x",
		static_cast<unsigned>(mac[0]),
		static_cast<unsigned>(mac[1]),
		static_cast<unsigned>(mac[2]),
		static_cast<unsigned>(mac[3]),
		static_cast<unsigned>(mac[4]),
		static_cast<unsigned>(mac[5]));
	return std::string(buf);
}

std::string_view StripSpaces(std::string_view str) {
	const size_t s = str.find_first_not_of(" \t\r\n");
	if (std::string::npos != s)
		return str.substr(s, str.find_last_not_of(" \t\r\n") - s + 1);
	else
		return "";
}

//void DataToHexString(const uint8_t* data, size_t size, std::string* output, bool lineBreaks) {
//	Buffer buffer;
//	for (size_t i = 0; i < size; i++) {
//		if (i && !(i & 15) && lineBreaks)
//			buffer.Printf("\n");
//		buffer.Printf("%02x ", data[i]);
//	}
//	buffer.TakeAll(output);
//}
//
//void DataToHexString(int indent, uint32_t startAddr, const uint8_t* data, size_t size, std::string* output) {
//	Buffer buffer;
//	size_t i = 0;
//	for (; i < size; i++) {
//		if (i && !(i & 15)) {
//			buffer.Printf(" ");
//			for (size_t j = i - 16; j < i; j++) {
//				buffer.Printf("%c", ((data[j] < 0x20) || (data[j] > 0x7e)) ? 0x2e : data[j]);
//			}
//			buffer.Printf("\n");
//		}
//		if (!(i & 15))
//			buffer.Printf("%*s%08x  ", indent, "", startAddr + i);
//		buffer.Printf("%02x ", data[i]);
//	}
//	if (size & 15) {
//		size_t padded_size = ((size - 1) | 15) + 1;
//		for (size_t j = size; j < padded_size; j++) {
//			buffer.Printf("   ");
//		}
//	}
//	if (size > 0) {
//		buffer.Printf(" ");
//		for (size_t j = (size - 1ULL) & ~UINT64_C(0xF); j < size; j++) {
//			buffer.Printf("%c", ((data[j] < 0x20) || (data[j] > 0x7e)) ? 0x2e : data[j]);
//		}
//	}
//	buffer.TakeAll(output);
//}

static inline bool is_printable_ascii(uint8_t c) {
	return c >= 0x20 && c <= 0x7e;
}

void DataToHexString(const uint8_t* data, size_t size, std::string* output, bool lineBreaks) {
	if (!output) return;
	output->clear();
	if (!data || size == 0) return;

	// reserve approximate size: 3 chars per byte + possible newlines
	output->reserve(size * 3 + (lineBreaks ? (size / 16) : 0));

	char buf[8];
	for (size_t i = 0; i < size; ++i) {
		if (i && (i % 16 == 0) && lineBreaks) {
			output->push_back('\n');
		}
		int n = std::snprintf(buf, sizeof(buf), "%02x ", data[i]);
		output->append(buf, static_cast<size_t>(n));
	}
}

void DataToHexString(int indent, uintptr_t startAddr, const uint8_t* data, size_t size, std::string* output) {
	if (!output) return;
	output->clear();
	if (!data || size == 0) return;

	// reserve approximate size: address + 3 chars per byte + ascii columns and newlines
	//output->reserve((size / 16 + 1) * (indent + 10 + 16 * 3 + 1 + 16));

	int addr_width = static_cast<int>(sizeof(uintptr_t) * 2);
	output->reserve((size / 16 + 1) * (indent + addr_width + 2 + 16 * 3 + 1 + 16));

	char buf[64];

	for (size_t i = 0; i < size; ++i) {
		if (i && (i % 16 == 0)) {
			// print ASCII for previous 16 bytes
			output->push_back(' ');
			size_t start = i - 16;
			for (size_t j = start; j < i; ++j) {
				output->push_back(is_printable_ascii(data[j]) ? static_cast<char>(data[j]) : '.');
			}
			output->push_back('\n');
		}

		//if ((i % 16) == 0) {
		//	// indent spaces
		//	if (indent > 0) output->append(std::string(static_cast<size_t>(indent), ' '));
		//	// address: 8 hex digits and two spaces after
		//	// cast to unsigned for printf
		//	unsigned addr_val = static_cast<unsigned>(startAddr + static_cast<uint32_t>(i));
		//	int n = std::snprintf(buf, sizeof(buf), "%08x  ", addr_val);
		//	output->append(buf, static_cast<size_t>(n));
		//}

		if ((i % 16) == 0) {
			// indent spaces
			if (indent > 0) output->append(std::string(static_cast<size_t>(indent), ' '));

			uintptr_t addr = static_cast<uintptr_t>(startAddr) + static_cast<uintptr_t>(i);
			std::string addr_str;
			addr_str.resize(static_cast<size_t>(addr_width), '0');

			uintptr_t tmp = addr;
			for (int pos = addr_width - 1; pos >= 0; --pos) {
				uint8_t nibble = static_cast<uint8_t>(tmp & 0xF);
				const char* hex_digits = "0123456789ABCDEF";
				addr_str[static_cast<size_t>(pos)] = hex_digits[nibble];
				tmp >>= 4;
			}
			output->append(addr_str);
			output->append("  ");
		}

		int n = std::snprintf(buf, sizeof(buf), "%02x ", data[i]);
		output->append(buf, static_cast<size_t>(n));
	}

	// If last line wasn't complete, add padding for hex columns
	if (size & 15) {
		size_t padded_size = ((size - 1) | 15) + 1;
		for (size_t j = size; j < padded_size; ++j) {
			output->append("   "); // three chars for each missing byte ("xx ")
		}
	}

	// Print ASCII for the final line (if any bytes exist)
	if (size > 0) {
		output->push_back(' ');
		size_t base = (size - 1) & ~static_cast<size_t>(0xF); // start index of the last 16-block
		for (size_t j = base; j < size; ++j) {
			output->push_back(is_printable_ascii(data[j]) ? static_cast<char>(data[j]) : '.');
		}
	}
}

std::string ToCase(const std::string& s, bool Upper) {
	std::string out = s;
	if (Upper)
		std::transform(out.begin(), out.end(), out.begin(), ::toupper);
	else
		std::transform(out.begin(), out.end(), out.begin(), ::tolower);
	return out;
}

bool IsUnCaseContains(const std::string& haystack, const std::string& needle) {
	if (needle.empty()) return true;
	std::string h = ToCase(haystack, false);
	std::string n = ToCase(needle, false);
	return h.find(n) != std::string::npos;
}

size_t truncate_cpy(char* dest, size_t destSize, const char* src) {
	size_t len = strlen(src);
	if (len >= destSize - 1) {
		memcpy(dest, src, destSize - 1);
		len = destSize - 1;
	}
	else {
		memcpy(dest, src, len);
	}
	dest[len] = '\0';
	return len;
}

size_t truncate_cpy(char* dest, size_t destSize, std::string_view src) {
	if (src.size() > destSize - 1) {
		memcpy(dest, src.data(), destSize - 1);
		dest[destSize - 1] = 0;
		return destSize - 1;
	}
	else {
		memcpy(dest, src.data(), src.size());
		dest[src.size()] = 0;
		return src.size();
	}
}

void ConvertUTF8ToWString(wchar_t* dest, size_t destSize, std::string_view source) {
	int len = (int)source.size();
	destSize -= 1;  // account for the \0.
	int size = (int)MultiByteToWideChar(CP_UTF8, 0, source.data(), len, NULL, 0);
	MultiByteToWideChar(CP_UTF8, 0, source.data(), len, dest, min((int)destSize, size));
	dest[size] = 0;
}

std::string CreateRandMAC() {
	std::stringstream randStream;
	srand(time(nullptr));
	for (int i = 0; i < 6; i++) {
		uint32_t value = rand() % 256;
		if (i == 0) {
			// Making sure the 1st 2-bits on the 1st byte of OUI are zero to prevent issue with some games (ie. Gran Turismo)
			value &= 0xfc;
		}
		if (value <= 15)
			randStream << '0' << std::hex << value;
		else
			randStream << std::hex << value;
		if (i < 5) {
			randStream << ':'; //we need a : between every octet
		}
	}
	return randStream.str();
}

void sleep_ms(int ms, const char* reason) {
	if (ms <= 0) {
		return;
	}
#if SLEEP_LOG_ENABLED
	INFO_LOG(Log::System, "Sleep %d ms: %s", ms, reason);
#endif
#ifdef _WIN32
	Sleep(ms);
#elif defined(HAVE_LIBNX)
	svcSleepThread(ms * 1000000);
#elif defined(__EMSCRIPTEN__)
	emscripten_sleep(ms);
#else
	usleep(ms * 1000);
#endif
}

// mazahaka -> 1 millisecond (ms) = 1,000 microseconds (µs) 1 microsecond (µs) = 1,000 nanoseconds (ns)
// time_now_d()*1000000.0
void hleEatCycles(int cycles) {
	hleEatMicro(cycles / 1000); // Approximate
}

void hleEatMicro(int usec) {
	if (usec > 0) std::this_thread::sleep_for(std::chrono::microseconds(usec));
}

//void hleDelayResult(int usec) {
//	//hleEatMicro(usec);
//}

#if defined(_WIN32)

void SetCurrentThreadNameThroughException(const char* threadName) {
	// Set the debugger-visible threadname through an unholy magic hack
	static const DWORD MS_VC_EXCEPTION = 0x406D1388;

#if defined(__MINGW32__)
	// Thread information for VS compatible debugger. -1 sets current thread.
	THREADNAME_INFO ti;
	ti.dwType = 0x1000;
	ti.szName = threadName;
	ti.dwThreadID = -1;

	// Push an exception handler to ignore all following exceptions
	NT_TIB* tib = ((NT_TIB*)NtCurrentTeb());
	EXCEPTION_REGISTRATION_RECORD rec;
	rec.Next = tib->ExceptionList;
	rec.Handler = ignore_handler;
	tib->ExceptionList = &rec;

	// Visual Studio and compatible debuggers receive thread names from the
	// program through a specially crafted exception
	RaiseException(MS_VC_EXCEPTION, 0, sizeof(ti) / sizeof(ULONG_PTR),
		(ULONG_PTR*)&ti);

	// Pop exception handler
	tib->ExceptionList = tib->ExceptionList->Next;
#else
#pragma pack(push,8)
	struct THREADNAME_INFO {
		DWORD dwType; // must be 0x1000
		LPCSTR szName; // pointer to name (in user addr space)
		DWORD dwThreadID; // thread ID (-1=caller thread)
		DWORD dwFlags; // reserved for future use, must be zero
	} info;
#pragma pack(pop)

	info.dwType = 0x1000;
	info.szName = threadName;
	info.dwThreadID = -1; //dwThreadID;
	info.dwFlags = 0;

	__try
	{
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}
	__except (EXCEPTION_CONTINUE_EXECUTION)
	{
	}
#endif
}
#endif

void SetCurrentThreadName(const char* threadName) {
#if defined(_WIN32)
	HMODULE hKernel32 = GetModuleHandleA("kernelbase.dll");
	if (hKernel32) {
		typedef HRESULT(WINAPI* TSetThreadDescription)(HANDLE, PCWSTR);
		TSetThreadDescription pSetThreadDescription = reinterpret_cast<TSetThreadDescription>(GetProcAddress(hKernel32, "SetThreadDescription"));
		if (pSetThreadDescription) {
			// Use the modern API
			wchar_t buffer[256];
			ConvertUTF8ToWString(buffer, ARRAY_SIZE(buffer), threadName);
			pSetThreadDescription(GetCurrentThread(), buffer);
		}
	}
	else {
		// Use the old exception hack.
		SetCurrentThreadNameThroughException(threadName);
	}
#endif
}

namespace CoreTiming {
	uint64_t GetGlobalTimeUsScaled() {
		auto now = std::chrono::steady_clock::now().time_since_epoch();
		return std::chrono::duration_cast<std::chrono::microseconds>(now).count();
	}
}

// seconds
double time_now_d() {
	auto now = std::chrono::steady_clock::now().time_since_epoch();
	return std::chrono::duration<double>(now).count();
}


// ==========================================================================
bool sceWlanGetSwitchState() { return g_Config.bEnableWlan; }

int sceUtilityGetSystemParamString(int id, char* destAddr, int destSize) {
	DEBUG_LOG(Log::sceUtility, "sceUtilityGetSystemParamString(%i, %08x, %i)", id, destAddr, destSize);
	char* buf = (char*)destAddr;
	switch (id) {
	case PSP_SYSTEMPARAM_ID_STRING_NICKNAME:
		// If there's not enough space for the string and null terminator, fail.
		if (destSize <= (int)g_Config.sNickName.length())
			return SCE_ERROR_UTILITY_STRING_TOO_LONG;
		// TODO: should we zero-pad the output as strncpy does? And what are the semantics for the terminating null if destSize == length?
		strncpy(buf, g_Config.sNickName.c_str(), destSize);
		break;

	default:
		return SCE_ERROR_UTILITY_INVALID_SYSTEM_PARAM_ID;
	}

	return 0;
}