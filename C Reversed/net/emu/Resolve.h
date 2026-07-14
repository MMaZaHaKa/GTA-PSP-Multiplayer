/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <future>

namespace net
{

enum class DNSType {
	ANY = 0,
	IPV4 = 1,
	IPV6 = 2,
};

// Strictly only required on Win32, but all platforms should call it.
void Init();
void Shutdown();
bool HostPortExists(const std::string& host, int port, int timeout_ms, int connType); // SOCK_STREAM TCP
std::vector<std::string> GetLocalAddresses(); // get local interface ip
std::string GetOutboundIP(); // get local interface ip // like getLocalIp()
std::future<std::vector<std::string>> ScanOutboundSubnetAsync(int port, int connType, int timeout_ms = 200); // SOCK_STREAM TCP
void ScanOutboundSubnetWithCallback(int port, int connType, std::function<void(std::vector<std::string>)> on_complete, int timeout_ms = 200);
bool DNSResolve(const std::string &host, const std::string &service, struct addrinfo **res, std::string &error, DNSType type = DNSType::ANY);
void DNSResolveFree(struct addrinfo* res);
}  // namespace net
