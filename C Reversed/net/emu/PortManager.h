/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

//#ifdef USE_SYSTEM_MINIUPNPC
//#include <miniupnpc/miniwget.h>
//#include <miniupnpc/miniupnpc.h>
//#include <miniupnpc/upnpcommands.h>
//#else
//#ifndef MINIUPNP_STATICLIB
//#define MINIUPNP_STATICLIB
//#endif
//#include "ext/miniupnp/miniupnpc/include/miniwget.h"
//#include "ext/miniupnp/miniupnpc/include/miniupnpc.h"
//#include "ext/miniupnp/miniupnpc/include/upnpcommands.h"
//#endif

#include <string>
#include <deque>

struct UPnPArgs {
	int cmd;
	std::string protocol;
	unsigned short port;
	unsigned short intport;
};

#define IP_PROTOCOL_TCP	"TCP"
#define IP_PROTOCOL_UDP	"UDP"
#define UPNP_INITSTATE_NONE	0
#define UPNP_INITSTATE_BUSY	1
#define UPNP_INITSTATE_DONE	2

#define UPNP_CMD_ADD	0
#define UPNP_CMD_REMOVE	1

struct UPNPUrls;
struct IGDdatas;

struct PortMap {
	bool taken;
	std::string protocol;
	std::string extPort_str;
	std::string intPort_str;
	std::string lanip;
	std::string remoteHost;
	std::string desc;
	std::string duration;
	std::string enabled;
};

//class PortManager {
//public:
//	PortManager();
//	~PortManager();
//
//	// Initialize UPnP
//	// timeout: milliseconds to wait for a router to respond (default = 2000 ms)
//	bool Initialize(const unsigned int timeout = 2000);
//
//	// Get UPnP Initialization status
//	int GetInitState();
//
//	// Add a port & protocol (TCP, UDP or vendor-defined) to map for forwarding (intport = 0 : same as [external] port)
//	bool Add(const char* protocol, unsigned short port, unsigned short intport = 0);
//
//	// Remove a port mapping (external port)
//	bool Remove(const char* protocol, unsigned short port);
//
//	// Call on exit. Does a full shutdown.
//	void Shutdown();
//
//private:
//	// Retrieves port lists mapped by PPSSPP for current LAN IP & other's applications
//	bool RefreshPortList();
//
//	// Removes any lingering mapped ports created by PPSSPP (including from previous crashes)
//	bool Clear();
//
//	// Restore ports mapped by others that were taken by PPSSPP, better used after Clear()
//	bool Restore();
//
//	// Uninitialize/Reset the state
//	void Terminate();
//
//	struct UPNPUrls* urls = nullptr;
//	struct IGDdatas* datas = nullptr;
//
//	int m_InitState = UPNP_INITSTATE_NONE;
//	int m_LocalPort = UPNP_LOCAL_PORT_ANY;
//	std::string m_lanip;
//	std::string m_defaultDesc;
//	std::string m_leaseDuration = "43200"; // range(0-604800) in seconds (0 = Indefinite/permanent). Some routers doesn't support non-zero value
//	std::deque<std::pair<std::string, std::string>> m_portList;
//	std::deque<PortMap> m_otherPortList;
//};
//
//extern PortManager g_PortManager;

void __UPnPInit(const int timeout_ms);
void __UPnPShutdown();

// Add a port & protocol (TCP, UDP or vendor-defined) to map for forwarding (intport = 0 : same as [external] port)
void UPnP_Add(const char* protocol, unsigned short port, unsigned short intport = 0);

// Remove a port mapping (external port)
void UPnP_Remove(const char* protocol, unsigned short port);
