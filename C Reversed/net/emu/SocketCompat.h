/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once
#if defined(_MSC_VER) && _MSC_VER < 1700
#error You need a newer version of Visual Studio
#else
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x601 // Compile for Win7 on Visual Studio 2012 and above
#undef WINVER
#define WINVER 0x601
#endif
#undef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#ifdef _WIN32
#pragma warning(disable:4091)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#undef min
#undef max
#undef DrawText
#endif

#include <io.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <fcntl.h>
#include <errno.h>

#pragma comment(lib, "Ws2_32.lib")
// or premake
//filter{ "system:windows" }
//	links{ "ws2_32", "winmm" }

#undef ESHUTDOWN
#undef ECONNABORTED
#undef ECONNRESET
#undef ECONNREFUSED
#undef ENETUNREACH
#undef ENOTCONN
#undef EBADF
#undef EAGAIN
#undef EINPROGRESS
#undef EISCONN
#undef EALREADY
#undef ETIMEDOUT
#undef EOPNOTSUPP
#undef ENOTSOCK
#undef EPROTONOSUPPORT
#undef ESOCKTNOSUPPORT
#undef EPFNOSUPPORT
#undef EAFNOSUPPORT
#undef EINTR
#undef EACCES
#undef EFAULT
#undef EINVAL
#undef ENOSPC
#undef EHOSTDOWN
#undef EADDRINUSE
#undef EADDRNOTAVAIL
#undef ENETUNREACH
#undef EHOSTUNREACH
#undef ENETDOWN
#define socket_errno WSAGetLastError()
#define ESHUTDOWN WSAESHUTDOWN
#define ECONNABORTED WSAECONNABORTED
#define ECONNRESET WSAECONNRESET
#define ECONNREFUSED WSAECONNREFUSED
#define ENETUNREACH WSAENETUNREACH
#define ENOTCONN WSAENOTCONN
#define EBADF WSAEBADF
#define EAGAIN WSAEWOULDBLOCK
#define EINPROGRESS WSAEWOULDBLOCK
#define EISCONN WSAEISCONN
#define EALREADY WSAEALREADY
#define ETIMEDOUT WSAETIMEDOUT
#define EOPNOTSUPP WSAEOPNOTSUPP
#define ENOTSOCK WSAENOTSOCK
#define EPROTONOSUPPORT WSAEPROTONOSUPPORT
#define ESOCKTNOSUPPORT WSAESOCKTNOSUPPORT
#define EPFNOSUPPORT WSAEPFNOSUPPORT
#define EAFNOSUPPORT WSAEAFNOSUPPORT
#define EINTR WSAEINTR
#define EACCES WSAEACCES
#define EFAULT WSAEFAULT
#define EINVAL WSAEINVAL
#define ENOSPC ERROR_INVALID_PARAMETER
#define EHOSTDOWN WSAEHOSTDOWN
#define EADDRINUSE WSAEADDRINUSE
#define EADDRNOTAVAIL WSAEADDRNOTAVAIL
#define ENETUNREACH WSAENETUNREACH
#define EHOSTUNREACH WSAEHOSTUNREACH
#define ENETDOWN WSAENETDOWN
inline bool connectInProgress(int errcode) { return (errcode == WSAEWOULDBLOCK || errcode == WSAEINPROGRESS || errcode == WSAEALREADY || errcode == WSAEINVAL); } // WSAEINVAL should be treated as WSAEALREADY during connect for backward-compatibility with Winsock 1.1

#ifndef MSG_NOSIGNAL
// Default value to 0x00 (do nothing) in systems where it's not supported.
#define MSG_NOSIGNAL 0x00
#endif

#undef SendMessage