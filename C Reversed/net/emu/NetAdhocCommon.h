/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once
#include "common.h"
#include "Logger.h"
#include <mutex>

//#define ON_SOCKET_ERROR() assert(false && "Socket Error");
#define ON_SOCKET_ERROR()

// Valid values for PSP_SYSTEMPARAM_ID_INT_ADHOC_CHANNEL
#define PSP_SYSTEMPARAM_ADHOC_CHANNEL_AUTOMATIC     0
#define PSP_SYSTEMPARAM_ADHOC_CHANNEL_1             1
#define PSP_SYSTEMPARAM_ADHOC_CHANNEL_6             6
#define PSP_SYSTEMPARAM_ADHOC_CHANNEL_11            11


// pspsdk pspnet.h my
// https://pspdev.github.io/pspsdk/dir_427baa8a9a5be237f298d4545d0d1ce2.html
// https://pspdev.github.io/pspsdk/pspnet__adhocmatching_8h.html


// ppsspp common
#define ADHOCCTL_NICKNAME_LEN 128
#define ADHOCCTL_GROUPNAME_LEN 8
#define ETHER_ADDR_LEN 6
#define ADHOCCTL_MESSAGE_LEN 64

// --mazahaka: ident default user / re user
#define ADHOCCTL_USE_CUSTOM_IDENT
#ifdef ADHOCCTL_USE_CUSTOM_IDENT
	#define ADHOCCTL_CUSTOM_IDENT "CUSTOMUSER" // 0x00, 0x01, 0x00 ident is enough if your name is Ugwemubwem Ossas
	#define ADHOCCTL_CUSTOM_FLAG (1 << 7)
#endif

#ifndef GTA_PSP // vitasdk/pspsdk name compat
// https://docs.vitasdk.org/pspnet__adhocctl_8h.html
// https://github.com/vitasdk/vita-headers/blob/c25c07aaa6d91c4f4ced721485120fc8305192a3/include/psp2/pspnet_adhoc.h
// https://github.com/MMaZaHaKa/PSP_DOCS/blob/509eb63f4957032bb44222b2545c8e8e9d0cc43a/network/PSPNET_Adhocctl-Reference-English.htm#L175
#define SCE_NET_ADHOCCTL_NICKNAME_LEN   ADHOCCTL_NICKNAME_LEN
#define SCE_NET_ADHOCCTL_GROUPNAME_LEN  ADHOCCTL_GROUPNAME_LEN
#define SCE_NET_ETHER_ADDR_LEN          ETHER_ADDR_LEN
#endif

#define SCE_NET_PSPNET_ADHOCCTL_LEAST_STACK_SIZE 3072

#define WITH_UPNP
//#define ADHOC_ANTISPOOF // disable for fix for multiinstance (LAN IP -> 10053) TODO not work lobby figure out InitLocalhostIP() IP_ADD_MEMBERSHIP? SO_REUSEADDR?
#define MAZAHAKA_PC // detect pc port mods (test)
#define DUMP_AEMU_SERVER_INFO // debug stuff
#define ADHOC_PTP_PDP_WRAPPERS // no ppsspp direct bind port, CSC scheme
//#define ADHOC_ASYNC_UPDATER

#ifdef ADHOC_PTP_PDP_WRAPPERS
	#define ADHOC_PTP_PDP_CHAT_EMU // aemu api char buff concat [no custom opcode] kek
#endif

typedef int SceUID;
typedef unsigned int SceSize;
typedef int SceSSize;
typedef unsigned char SceUChar;
typedef unsigned int SceUInt;
typedef int SceMode;

struct SceNetEtherAddr;

void AdhocEmu_NativeInit();
void AdhocEmu_NetUpdate();
void AdhocEmu_NativeShutdown();

extern int netAdhocEnterGameModeTimeout;

// 1 second = 1 000 000 microseconds // 1sec * 1000 * 1000

// Old comments regarding the value of adhocDefaultTimeout:
// 3000000 usec
//2000000 usec // For some unknown reason, sometimes it tooks more than 2 seconds for Adhocctl Init to connect to AdhocServer on localhost (normally only 10 ms), and sometimes it tooks more than 1 seconds for built-in AdhocServer to be ready (normally only 1 ms)
constexpr int adhocDefaultTimeout = 5000000;

#if defined(_DEBUG) && !defined(FINAL) && !defined(MASTER)
constexpr int adhocDefaultWaitDelay = 100/*sec*/ * 1000 * 1000; //10000 (0.01 sec) old
#else
constexpr int adhocDefaultWaitDelay = 1 * 1000 * 1000; // 1sec
//constexpr int adhocDefaultWaitDelay = 100/*sec*/ * 1000 * 1000; // 100sec //10000 (0.01 sec) // *5 throw wait error, after adhoc init error WaitBlockingAdhocctlSocket
//constexpr int adhocDefaultWaitDelay = 10000*5; // 0.05sec //10000 (0.01 sec) // timeout retry if error socket  (fail if stack in debugger)
#endif
constexpr int adhocDefaultEatDelay = 10000; //10000 // timeout retry if error socket  (fail if stack in debugger)

constexpr int adhocExtraDelay = 20000; //20000
constexpr int adhocEventPollDelay = 100000; //100000; // Seems to be the same with PSP_ADHOCCTL_RECV_TIMEOUT

constexpr int adhocEventDelay = 2000000; //2000000 on real PSP ?

extern std::recursive_mutex adhocEvtMtx;

extern int IsAdhocctlInCB;

extern bool g_adhocServerConnected;

constexpr uint32_t defaultLastRecvDelta = 10000;


// Valid values for PSP_SYSTEMPARAM_ID_INT_LANGUAGE
#define PSP_SYSTEMPARAM_LANGUAGE_JAPANESE               0
#define PSP_SYSTEMPARAM_LANGUAGE_ENGLISH                1
#define PSP_SYSTEMPARAM_LANGUAGE_FRENCH                 2
#define PSP_SYSTEMPARAM_LANGUAGE_SPANISH                3
#define PSP_SYSTEMPARAM_LANGUAGE_GERMAN                 4
#define PSP_SYSTEMPARAM_LANGUAGE_ITALIAN                5
#define PSP_SYSTEMPARAM_LANGUAGE_DUTCH                  6
#define PSP_SYSTEMPARAM_LANGUAGE_PORTUGUESE             7
#define PSP_SYSTEMPARAM_LANGUAGE_RUSSIAN                8
#define PSP_SYSTEMPARAM_LANGUAGE_KOREAN                 9
#define PSP_SYSTEMPARAM_LANGUAGE_CHINESE_TRADITIONAL    10
#define PSP_SYSTEMPARAM_LANGUAGE_CHINESE_SIMPLIFIED     11

#define PSP_SYSTEMPARAM_TIME_FORMAT_24HR    0
#define PSP_SYSTEMPARAM_TIME_FORMAT_12HR    1

#define PSP_SYSTEMPARAM_ID_STRING_NICKNAME              1
#define PSP_SYSTEMPARAM_ID_INT_ADHOC_CHANNEL            2
#define PSP_SYSTEMPARAM_ID_INT_WLAN_POWERSAVE           3
#define PSP_SYSTEMPARAM_ID_INT_DATE_FORMAT              4
#define PSP_SYSTEMPARAM_ID_INT_TIME_FORMAT              5
//Timezone offset from UTC in minutes, (EST = -300 = -5 * 60)
#define PSP_SYSTEMPARAM_ID_INT_TIMEZONE                 6
#define PSP_SYSTEMPARAM_ID_INT_DAYLIGHTSAVINGS          7
#define PSP_SYSTEMPARAM_ID_INT_LANGUAGE                 8
#define PSP_SYSTEMPARAM_ID_INT_BUTTON_PREFERENCE        9
#define PSP_SYSTEMPARAM_ID_INT_LOCK_PARENTAL_LEVEL      10

// Return values for the SystemParam functions
#define PSP_SYSTEMPARAM_RETVAL_OK                       0

// Valid values for PSP_SYSTEMPARAM_ID_INT_ADHOC_CHANNEL
#define PSP_SYSTEMPARAM_ADHOC_CHANNEL_AUTOMATIC     0
#define PSP_SYSTEMPARAM_ADHOC_CHANNEL_1             1
#define PSP_SYSTEMPARAM_ADHOC_CHANNEL_6             6
#define PSP_SYSTEMPARAM_ADHOC_CHANNEL_11            11

// Valid values for PSP_SYSTEMPARAM_ID_INT_WLAN_POWERSAVE
#define PSP_SYSTEMPARAM_WLAN_POWERSAVE_OFF  0
#define PSP_SYSTEMPARAM_WLAN_POWERSAVE_ON   1

// Valid values for PSP_SYSTEMPARAM_ID_INT_DATE_FORMAT
#define PSP_SYSTEMPARAM_DATE_FORMAT_YYYYMMDD  0
#define PSP_SYSTEMPARAM_DATE_FORMAT_MMDDYYYY  1
#define PSP_SYSTEMPARAM_DATE_FORMAT_DDMMYYYY  2

// Valid values for PSP_SYSTEMPARAM_ID_INT_DAYLIGHTSAVINGS
#define PSP_SYSTEMPARAM_DAYLIGHTSAVINGS_STD    0
#define PSP_SYSTEMPARAM_DAYLIGHTSAVINGS_SAVING 1

// Valid values for PSP_SYSTEMPARAM_ID_INT_BUTTON_PREFERENCE
#define PSP_SYSTEMPARAM_BUTTON_CIRCLE  0
#define PSP_SYSTEMPARAM_BUTTON_CROSS   1

// Valid values for NetParam
#define PSP_NETPARAM_NAME               0 // string
#define PSP_NETPARAM_SSID               1 // string
#define PSP_NETPARAM_SECURE             2 // int
#define PSP_NETPARAM_WEPKEY             3 // string
#define PSP_NETPARAM_IS_STATIC_IP       4 // int
#define PSP_NETPARAM_IP                 5 // string
#define PSP_NETPARAM_NETMASK            6 // string
#define PSP_NETPARAM_ROUTE              7 // string
#define PSP_NETPARAM_MANUAL_DNS         8 // int
#define PSP_NETPARAM_PRIMARYDNS         9 // string
#define PSP_NETPARAM_SECONDARYDNS       10 // string
#define PSP_NETPARAM_PROXY_USER         11 // string
#define PSP_NETPARAM_PROXY_PASS         12 // string
#define PSP_NETPARAM_USE_PROXY          13 // int
#define PSP_NETPARAM_PROXY_SERVER       14 // string
#define PSP_NETPARAM_PROXY_PORT         15 // int
#define PSP_NETPARAM_VERSION            16 // int
#define PSP_NETPARAM_UNKNOWN            17 // int
#define PSP_NETPARAM_8021X_AUTH_TYPE    18 // int
#define PSP_NETPARAM_8021X_USER         19 // string
#define PSP_NETPARAM_8021X_PASS         20 // string
#define PSP_NETPARAM_WPA_TYPE           21 // int
#define PSP_NETPARAM_WPA_KEY            22 // string
#define PSP_NETPARAM_BROWSER            23 // int
#define PSP_NETPARAM_WIFI_CONFIG        24 // int

// X-Men Legends 2, and some homebrew may support up to 10 net config entries, but we currently only have 1 faked net config
#define PSP_NETPARAM_MAX_NUMBER_DUMMY_ENTRIES   1