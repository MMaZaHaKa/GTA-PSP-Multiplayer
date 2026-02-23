/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "GameInstance.h"

#ifdef _WIN32
#include "Windows.h"
static HANDLE hIDMapFile = nil;
static HANDLE mapLock = nil;
#else
static int hIDMapFile = -1;
static long BUF_SIZE = 4096;
#endif

uint8_t GAME_ID = 0;

struct InstanceInfo {
	uint8_t pad[2];
	uint8_t next;
	uint8_t total;
};

// proAdhoc.cpp // Don't need to connect if AdhocServer IP is the same with this instance localhost IP and having AdhocServer disabled
// need shift ip from sema in ppsspp to connect into localhost without bEnableAdhocServer (debug with ppsspp)
//#if !defined(FINAL) && !defined(MASTER)
#define ID_SHM_NAME "/PPSSPP_ID"
#define ID_MUTEX_NAME L"PPSSPP_ID_mutex"
//#else // ppsspp compat local multiinstances
//#define ID_SHM_NAME "/RE3_ID"
//#define ID_MUTEX_NAME L"RE3_ID_mutex"
//#endif

static bool UpdateInstanceCounter(void (*callback)(volatile InstanceInfo *)) {
#ifdef _WIN32
	if (!hIDMapFile) {
		return false;
	}
	InstanceInfo *buf = (InstanceInfo *)MapViewOfFile(hIDMapFile,   // handle to map object
		FILE_MAP_ALL_ACCESS, // read/write permission
		0,
		0,
		sizeof(InstanceInfo));

	if (!buf) {
		auto err = GetLastError();
		//printf("Could not map view of file %s, %08x %s", ID_SHM_NAME, (uint32_t)err, GetStringErrorMsg(err).c_str());
		return false;
	}

	bool result = false;
	if (!mapLock || WaitForSingleObject(mapLock, INFINITE) == 0) {
		callback(buf);
		if (mapLock) {
			ReleaseMutex(mapLock);
		}
		result = true;
	}
	UnmapViewOfFile(buf);

	return result;
#else
	// todo unix?
	return false;
#endif
}

// Get current number of instance running.
int GetInstancePeerCount() {
	static int c = 0;
	UpdateInstanceCounter([](volatile InstanceInfo *buf) {
		c = buf->total;
	});
	return c;
}

// Must be called only once during init.
void InitInstanceCounter() {
#ifdef _WIN32
	uint32_t BUF_SIZE = 4096;
	SYSTEM_INFO sysInfo;

	GetSystemInfo(&sysInfo);
	int gran = sysInfo.dwAllocationGranularity ? sysInfo.dwAllocationGranularity : 0x10000;
	BUF_SIZE = (BUF_SIZE + gran - 1) & ~(gran - 1);

	mapLock = CreateMutexW(nullptr, FALSE, ID_MUTEX_NAME);

	hIDMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,    // use paging file
		NULL,                    // default security
		PAGE_READWRITE,          // read/write access
		0,                       // maximum object size (high-order DWORD)
		BUF_SIZE,                // maximum object size (low-order DWORD)
		TEXT(ID_SHM_NAME));       // name of mapping object

	DWORD lasterr = GetLastError();
	if (!hIDMapFile) {
		// "Could not create %s file mapping object, %08x %s"
		GAME_ID = 1;
		return;
	}
#else
	// Create shared memory object
	hIDMapFile = shm_open(ID_SHM_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	BUF_SIZE = (BUF_SIZE < sysconf(_SC_PAGE_SIZE)) ? sysconf(_SC_PAGE_SIZE) : BUF_SIZE;

	if (hIDMapFile < 0 || (ftruncate(hIDMapFile, BUF_SIZE)) == -1) {    // Set the size
		ERROR_LOG(Log::sceNet, "ftruncate(%s) failure.", ID_SHM_NAME);
		GAME_ID = 1;
		return;
	}
#endif

	bool success = UpdateInstanceCounter([](volatile InstanceInfo *buf) {
		GAME_ID = ++buf->next;
		buf->total++;
	});
	if (!success) {
		GAME_ID = 1;
	}
}

void ShutdownInstanceCounter() {
	UpdateInstanceCounter([](volatile InstanceInfo *buf) {
		buf->total--;
	});

#ifdef _WIN32
	if (hIDMapFile) {
		CloseHandle(hIDMapFile); // If program exited(or crashed?) or the last handle reference closed the shared memory object will be deleted.
		hIDMapFile = nil;
	}
	if (mapLock) {
		CloseHandle(mapLock);
		mapLock = nil;
	}
#else
	if (hIDMapFile >= 0) {
		close(hIDMapFile);
		shm_unlink(ID_SHM_NAME);     // If program exited or crashed before unlinked the shared memory object and it's contents will persist.
		hIDMapFile = -1;
	}
#endif
}
