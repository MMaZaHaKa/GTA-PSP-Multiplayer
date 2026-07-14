/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <array>

#include "SocketCompat.h" // WSA
#include "common.h" // assert
#include "Resolve.h"

#include <atomic>

#ifdef _WIN32
std::string ConvertWStringToUTF8(const wchar_t* wstr) {
	int len = (int)wcslen(wstr);
	int size = (int)WideCharToMultiByte(CP_UTF8, 0, wstr, len, 0, 0, NULL, NULL);
	std::string s;
	s.resize(size);
	if (size > 0) {
		WideCharToMultiByte(CP_UTF8, 0, wstr, len, &s[0], size, NULL, NULL);
	}
	return s;
}
#endif

namespace net {
    std::atomic<bool> gbWsaInited = false;
#ifdef _WIN32
#define CHECK_WSA_INIT() assert(gbWsaInited);
#else
#define CHECK_WSA_INIT() 
#endif

void Init()
{
#ifdef _WIN32
	// WSA does its own internal reference counting, no need to keep track of if we inited or not.
	WSADATA wsaData = {0};
	WSAStartup(MAKEWORD(2, 2), &wsaData);
    gbWsaInited = true;
#endif
}

void Shutdown()
{
#ifdef _WIN32
	WSACleanup();
    gbWsaInited = false;
#endif
}

// --mazahaka simple stack lifetime dispatcher
#ifdef _WIN32
struct WSAInit {
    WSAInit() {
        WSADATA w;
        WSAStartup(MAKEWORD(2, 2), &w);
        gbWsaInited = true;
    }
    ~WSAInit() {
        WSACleanup();
        gbWsaInited = false;
    }
};
#endif

bool HostPortExists(const std::string& host, int port, int timeout_ms, int connType) {
    if (host.empty() || (port <= 0 || port > 65535) || timeout_ms < 0) return false;

#ifdef _WIN32
    //// Ensure WSA is initialized for the lifetime of this call
    //WSAInit wsa;

    CHECK_WSA_INIT();
#endif

    addrinfo hints;
    addrinfo* res = nullptr;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = connType;
    //hints.ai_socktype = SOCK_STREAM; // TCP
    //hints.ai_socktype = SOCK_DGRAM; // UDP
    hints.ai_family = AF_UNSPEC;     // IPv4 or IPv6

    int gai = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &res);
    if (gai != 0) {
        // getaddrinfo failed (DNS resolve failed or bad port)
        return false;
    }

    bool ok = false;

    for (addrinfo* p = res; p != nullptr && !ok; p = p->ai_next) {
        // create socket
        int sockfd =
#ifdef _WIN32
        (int)socket(p->ai_family, p->ai_socktype, p->ai_protocol);
#else
            socket(p->ai_family, p->ai_socktype, p->ai_protocol);
#endif
        if (sockfd < 0) {
            continue;
        }

        // make non-blocking
#ifdef _WIN32
        unsigned long mode = 1;
        ioctlsocket((SOCKET)sockfd, FIONBIO, &mode);
#else
        int flags = fcntl(sockfd, F_GETFL, 0);
        if (flags == -1) flags = 0;
        fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
#endif

        // try connect
        int conn = connect(sockfd, p->ai_addr, (int)p->ai_addrlen);
#ifdef _WIN32
        if (conn == 0) {
            ok = true; // immediate success
        }
        else {
            int err = WSAGetLastError();
            if (err == WSAEWOULDBLOCK || err == WSAEINPROGRESS) {
                // fall through to select
            }
            else {
                // immediate failure
            }
        }
#else
        if (conn == 0) {
            ok = true; // immediate success
        }
        else {
            if (errno == EINPROGRESS) {
                // fall through to select
            }
            else {
                // immediate failure
            }
        }
#endif

        if (!ok) {
            // wait for writable with timeout
            fd_set writefds;
            FD_ZERO(&writefds);
#ifdef _WIN32
            FD_SET((SOCKET)sockfd, &writefds);
#else
            FD_SET(sockfd, &writefds);
#endif

            fd_set exceptfds;
            FD_ZERO(&exceptfds);
#ifdef _WIN32
            FD_SET((SOCKET)sockfd, &exceptfds);
#else
            FD_SET(sockfd, &exceptfds);
#endif

            timeval tv;
            tv.tv_sec = timeout_ms / 1000;
            tv.tv_usec = (timeout_ms % 1000) * 1000;

            int sel = select(
#ifdef _WIN32
                0,
#else
                sockfd + 1,
#endif
                nullptr, &writefds, &exceptfds, &tv);

            if (sel > 0) {
                // check for error on socket
                int sock_err = 0;
                socklen_t len = sizeof(sock_err);
#ifdef _WIN32
                int ret = getsockopt((SOCKET)sockfd, SOL_SOCKET, SO_ERROR, (char*)&sock_err, &len);
#else
                int ret = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &sock_err, &len);
#endif
#ifdef _WIN32
                bool writable = FD_ISSET(static_cast<SOCKET>(sockfd), &writefds) != 0;
#else
                bool writable = FD_ISSET(sockfd, &writefds) != 0;
#endif

                if (ret == 0 && sock_err == 0 && writable) {
                    ok = true;
                }
            }
            // else timeout or error -> try next addr
        }

        // close socket
#ifdef _WIN32
        closesocket((SOCKET)sockfd);
#else
        close(sockfd);
#endif
    }

    freeaddrinfo(res);
    return ok;
}

std::vector<std::string> GetLocalAddresses() {
#ifdef _WIN32
    CHECK_WSA_INIT();
#endif

    std::vector<std::string> outAddr;
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    struct hostent* host = gethostbyname(hostname);
    if (host != NULL) {
        for (int i = 0; host->h_addr_list[i] != NULL; i++) {
            struct in_addr addr;
            memcpy(&addr, host->h_addr_list[i], sizeof(struct in_addr));
            outAddr.push_back(std::string(inet_ntoa(addr)));
        }
    }
    return outAddr;
}

// like in int getLocalIp(sockaddr_in* SocketAddress) proAdhoc.cpp but without instance code
std::string GetOutboundIP()
{
#ifdef _WIN32
    CHECK_WSA_INIT();
#endif

    std::string dst = "8.8.8.8";
    std::string port = "53";

    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM; // UDP
    hints.ai_family = AF_UNSPEC;    // IPv4 or IPv6

    addrinfo* res = nullptr;
    int gai = getaddrinfo(dst.c_str(), port.c_str(), &hints, &res);
    if (gai != 0) {
        return std::string();
    }

    std::string result;

    for (addrinfo* p = res; p != nullptr; p = p->ai_next) {
        int sock =
#ifdef _WIN32
        (int)socket(p->ai_family, p->ai_socktype, p->ai_protocol);
#else
            socket(p->ai_family, p->ai_socktype, p->ai_protocol);
#endif
        if (sock < 0) continue;

        if (connect(sock, p->ai_addr, (int)p->ai_addrlen) != 0) {
#ifdef _WIN32
            closesocket(sock);
#else
            close(sock);
#endif
            continue;
        }

        sockaddr_storage localAddr;
        socklen_t localLen = sizeof(localAddr);
        if (getsockname(sock, reinterpret_cast<sockaddr*>(&localAddr), &localLen) == 0) {
            char hostbuf[NI_MAXHOST] = { 0 };
            int rc = getnameinfo(reinterpret_cast<sockaddr*>(&localAddr), localLen,
                hostbuf, sizeof(hostbuf), nullptr, 0, NI_NUMERICHOST);
            if (rc == 0) {
                result = std::string(hostbuf);
            }
        }

#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif

        if (!result.empty()) break;
    }

    freeaddrinfo(res);
    return result;

    // getLocalIp
    //// Socket doesn't "leak" to the game.
    //int sock = socket(AF_INET, SOCK_DGRAM, 0);
    //if (sock != SOCKET_ERROR) {
    //    const char* kGoogleDnsIp = "8.8.8.8"; // Needs to be an IP string so it can be resolved as fast as possible to IP, doesn't need to be reachable
    //    uint16_t kDnsPort = 53;
    //    struct sockaddr_in serv {};
    //    uint32_t ipv4 = INADDR_NONE; // inet_addr(kGoogleDnsIp); // deprecated?
    //    inet_pton(AF_INET, kGoogleDnsIp, &ipv4);
    //    serv.sin_family = AF_INET;
    //    serv.sin_addr.s_addr = ipv4;
    //    serv.sin_port = htons(kDnsPort);

    //    int err = connect(sock, (struct sockaddr*)&serv, sizeof(serv)); // connect should succeed even with SOCK_DGRAM
    //    if (err != SOCKET_ERROR) {
    //        struct sockaddr_in name {};
    //        socklen_t namelen = sizeof(name);
    //        err = getsockname(sock, (struct sockaddr*)&name, &namelen);
    //        if (err != SOCKET_ERROR) {
    //            SocketAddress->sin_addr = name.sin_addr; // May be we should cache this so it doesn't need to use connect all the time, or even better cache it when connecting to adhoc server to get an accurate IP
    //            closesocket(sock);
    //            return 0;
    //        }
    //    }
    //    closesocket(sock);
    //}
}

std::future<std::vector<std::string>> ScanOutboundSubnetAsync(int port, int connType, int timeout_ms)
{
    std::promise<std::vector<std::string>> prom;
    auto fut = prom.get_future();

    std::thread([prom = std::move(prom), port, connType, timeout_ms]() mutable {
        std::vector<std::string> result;

#ifdef _WIN32
        CHECK_WSA_INIT();
#endif

        std::string myip = GetOutboundIP();
        if (myip.empty()) {
            auto addrs = GetLocalAddresses();
            if (!addrs.empty()) myip = addrs.front();
        }

        std::array<int, 4> parts{};
        {
            std::istringstream iss(myip);
            std::string tok;
            int idx = 0;
            while (std::getline(iss, tok, '.') && idx < 4) {
                try { parts[idx++] = std::stoi(tok); }
                catch (...) { idx = 0; break; }
            }
            if (parts[0] < 0 || parts[0] > 255 || parts[1] < 0 || parts[1] > 255 ||
                parts[2] < 0 || parts[2] > 255 || parts[3] < 0 || parts[3] > 255) {
                prom.set_value(result);
                return;
            }
        }

        //  /24: a.b.c
        std::ostringstream baseOSS;
        baseOSS << parts[0] << "." << parts[1] << "." << parts[2];
        std::string base = baseOSS.str();
        int my_last = parts[3];

        unsigned int hw = std::thread::hardware_concurrency();
        if (hw == 0) hw = 4;
        unsigned int max_workers = std::min<unsigned int>(64, std::max<unsigned int>(2, hw));

        std::atomic<int> next_host(1); // 1..254
        std::mutex res_mtx;

        std::vector<std::thread> workers;
        workers.reserve(max_workers);

        for (unsigned int w = 0; w < max_workers; ++w) {
            workers.emplace_back([&]() {
                while (true) {
                    int h = next_host.fetch_add(1);
                    if (h > 254) break;
                    //if (h == my_last) continue; // self // we can host

                    std::ostringstream iposs;
                    iposs << base << "." << h;
                    std::string ip = iposs.str();

                    bool ok = HostPortExists(ip, port, timeout_ms, connType);
                    if (ok) {
                        std::lock_guard<std::mutex> lk(res_mtx);
                        result.push_back(ip);
                    }
                }
                });
        }

        for (auto& t : workers) if (t.joinable()) t.join();

        std::sort(result.begin(), result.end());

        prom.set_value(std::move(result));
    }).detach();

    return fut;
}

void ScanOutboundSubnetWithCallback(int port, int connType, std::function<void(std::vector<std::string>)> on_complete, int timeout_ms)
{
    if (!on_complete)
        return;

    std::thread([port, connType, timeout_ms, on_complete = std::move(on_complete)]() mutable {
        std::vector<std::string> result;
        try {
#ifdef _WIN32
            CHECK_WSA_INIT();
#endif
            std::string myip = GetOutboundIP();
            if (myip.empty()) {
                auto addrs = GetLocalAddresses();
                if (!addrs.empty()) myip = addrs.front();
            }

            if (myip.empty()) {
                on_complete(std::move(result));
                return;
            }

            int a = -1, b = -1, c = -1, d = -1;
            if (std::sscanf(myip.c_str(), "%d.%d.%d.%d", &a, &b, &c, &d) != 4) {
                on_complete(std::move(result));
                return;
            }
            if (a < 0 || a > 255 || b < 0 || b > 255 || c < 0 || c > 255 || d < 0 || d > 255) {
                on_complete(std::move(result));
                return;
            }

            // base /24
            std::ostringstream baseOSS;
            baseOSS << a << "." << b << "." << c;
            std::string base = baseOSS.str();
            int my_last = d;

            unsigned int hw = std::thread::hardware_concurrency();
            if (hw == 0) hw = 4;
            unsigned int max_workers = std::min<unsigned int>(64, std::max<unsigned int>(2, hw));

            std::atomic<int> next_host(1);
            std::mutex res_mtx;

            std::vector<std::thread> workers;
            workers.reserve(max_workers);

            for (unsigned int w = 0; w < max_workers; ++w) {
                workers.emplace_back([&]() {
                    while (true) {
                        int h = next_host.fetch_add(1);
                        if (h > 254) break;
                        //if (h == my_last) continue; // self // we can host

                        std::ostringstream iposs;
                        iposs << base << "." << h;
                        std::string ip = iposs.str();

                        bool ok = HostPortExists(ip, port, timeout_ms, connType);
                        if (ok) {
                            std::lock_guard<std::mutex> lk(res_mtx);
                            result.push_back(ip);
                        }
                    }
                    });
            }

            for (auto& t : workers) {
                if (t.joinable()) t.join();
            }

            std::sort(result.begin(), result.end());
        }
        catch (const std::exception& e) {
        }
        catch (...) {
        }

        on_complete(std::move(result));
    }).detach();
}

// NOTE: Due to the nature of getaddrinfo, this can block indefinitely. Not good.
bool DNSResolve(const std::string &host, const std::string &service, addrinfo **res, std::string &error, DNSType type) {
#ifdef _WIN32
    CHECK_WSA_INIT();
#endif

	addrinfo hints = {0};
	// TODO: Might be uses to lookup other values.
	hints.ai_socktype = SOCK_STREAM;
	// AI_V4MAPPED seems to have issues on some platforms, not sure we should include it:
	// http://stackoverflow.com/questions/1408030/what-is-the-purpose-of-the-ai-v4mapped-flag-in-getaddrinfo
	hints.ai_flags = /*AI_V4MAPPED |*/ AI_ADDRCONFIG;
	hints.ai_protocol = 0;
	if (type == DNSType::IPV4)
		hints.ai_family = AF_INET;
	else if (type == DNSType::IPV6)
		hints.ai_family = AF_INET6;

	const char *servicep = service.empty() ? nullptr : service.c_str();

	*res = nullptr;
	int result = getaddrinfo(host.c_str(), servicep, &hints, res);
	if (result == EAI_AGAIN) {
		// Temporary failure.  Since this already blocks, let's just try once more.
		Sleep(1);
		result = getaddrinfo(host.c_str(), servicep, &hints, res);
	}

	if (result != 0) {
#ifdef _WIN32
		//error = ConvertWStringToUTF8(gai_strerror(result));
		error = ConvertWStringToUTF8(gai_strerrorW(result));
#else
		error = gai_strerror(result);
#endif
		if (*res != nullptr)
			freeaddrinfo(*res);
		*res = nullptr;
		return false;
	}

	return true;
}

void DNSResolveFree(addrinfo* res)
{
	if (res)
		freeaddrinfo(res);
}

}  // namespace net
