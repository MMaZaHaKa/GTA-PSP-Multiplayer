/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "NetAdhocCommon.h"
#include "GameInstance.h"
#ifdef WITH_UPNP
#include "PortManager.h"
#endif
#include "Utils.h"
#include "proAdhoc.h"
#include "proAdhocServer.h"

#include "NetAdhocCommon.h"
#include "sceNetAdhocMatching.h"
#include "sceNet.h"
#include "sceNetAdhoc.h"
#include "Resolve.h"
#include "MultiGame.h" // config

int netAdhocEnterGameModeTimeout = 15000000; // 15 sec as default timeout, to wait for all players to join
std::recursive_mutex adhocEvtMtx;
bool g_adhocServerConnected = false;

std::atomic<bool> g_bNetInited = false;
void AdhocEmu_NativeInit() // On Mp Start
{
	if (g_bNetInited)
		return;

	net::Init();  // This needs to happen before we load the config. So on Windows we also run it in Main. It's fine to call multiple times.

	g_Config.Init(); // can use wsa stuff!

#ifdef WITH_UPNP
	// Probably an excessive timeout. it only causes delays on shutdown, though.
	__UPnPInit(2000);
#endif

	//-------__KernelLoadReset-----
	//-------__KernelInit-----
	__NetInit();
	__NetAdhocInit();
	__NetAdhocMatchingInit();

	g_bNetInited = true;
}

void AdhocEmu_NetUpdate() // callbacks updater from native thread // upd always, ppsspp its hack thread
{
	if (!g_bNetInited)
		return;

#ifndef ADHOC_ASYNC_UPDATER
	// Fake function for PPSSPP's use.
	__NetMatchingCallbacks(); // sceNetAdhocMatching.cpp processMatchingEvents()
	__NetTriggerCallbacks(); // sceNetAdhoc.cpp processAdhocctlEvents()
#else
	// https://github.com/hrydgard/ppsspp/issues/13452?timeline_page=1
	static bool s_started = false;
	if (!s_started) {
		s_started = true;
		std::thread([] {
			while (true) {
				__NetMatchingCallbacks();
				__NetTriggerCallbacks();
				//std::this_thread::sleep_for(std::chrono::milliseconds(10));
				std::this_thread::sleep_for(std::chrono::microseconds(adhocDefaultEatDelay));
			}
			}).detach();
	}
#endif
}

void AdhocEmu_NativeShutdown() // On Mp End
{
	if (!g_bNetInited)
		return;

	//-------__KernelShutdown-----
	__NetAdhocShutdown();
	__NetAdhocMatchingShutdown();
	__NetShutdown();

#ifdef WITH_UPNP
	__UPnPShutdown();

	//g_PortManager.Shutdown(); // moved into __UPnPShutdown
#endif

	net::Shutdown();

	g_bNetInited = false;
}
