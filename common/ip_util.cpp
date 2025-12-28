/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include "ip_util.h"

#include "common/eqemu_logsys.h"
#include "common/event/event_loop.h"
#include "common/event/task_scheduler.h"
#include "common/http/httplib.h"
#include "common/http/uri.h"
#include "common/net/dns.h"

#include "fmt/format.h"
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#endif


uint32_t IpUtil::IPToUInt(const std::string &ip)
{
	int      a, b, c, d;
	uint32_t addr = 0;

	if (sscanf(ip.c_str(), "%d.%d.%d.%d", &a, &b, &c, &d) != 4) {
		return 0;
	}

	addr = a << 24;
	addr |= b << 16;
	addr |= c << 8;
	addr |= d;
	return addr;
}

bool IpUtil::IsIpInRange(const std::string &ip, const std::string &network, const std::string &mask)
{
	uint32_t ip_addr      = IpUtil::IPToUInt(ip);
	uint32_t network_addr = IpUtil::IPToUInt(network);
	uint32_t mask_addr    = IpUtil::IPToUInt(mask);

	uint32_t net_lower = (network_addr & mask_addr);
	uint32_t net_upper = (net_lower | (~mask_addr));

	return ip_addr >= net_lower && ip_addr <= net_upper;
}

bool IpUtil::IsIpInPrivateRfc1918(const std::string &ip)
{
	return (
		IpUtil::IsIpInRange(ip, "10.0.0.0", "255.0.0.0") ||
		IpUtil::IsIpInRange(ip, "172.16.0.0", "255.240.0.0") ||
		IpUtil::IsIpInRange(ip, "192.168.0.0", "255.255.0.0")
	);
}

std::string IpUtil::GetLocalIPAddress()
{
#ifdef _WIN32
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		return "";
	}
#endif

	char my_ip_address[INET_ADDRSTRLEN];
	struct sockaddr_in server_address{};
	struct sockaddr_in my_address{};
	int sockfd;

	// Create a UDP socket
#ifdef _WIN32
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == INVALID_SOCKET) {
		WSACleanup();
		return "";
	}
#else
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		return "";
	}
#endif

	// Set server_addr (dummy address)
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr("8.8.8.8");  // Google DNS
	server_address.sin_port = htons(53);  // DNS port

	// Perform a dummy connection to the server (UDP)
	connect(sockfd, (struct sockaddr *) &server_address, sizeof(server_address));

	// Get my IP address
	memset(&my_address, 0, sizeof(my_address));
	socklen_t len = sizeof(my_address);
	getsockname(sockfd, (struct sockaddr *) &my_address, &len);
	inet_ntop(AF_INET, &my_address.sin_addr, my_ip_address, sizeof(my_ip_address));

#ifdef _WIN32
	closesocket(sockfd);
    WSACleanup();
#else
	close(sockfd);
#endif

	LogInfo("Local IP Address [{}]", my_ip_address);

	return std::string(my_ip_address);
}


/**
 * Gets public address
 * Uses various websites as options to return raw public IP back to the client
 * @return
 */
std::string IpUtil::GetPublicIPAddress()
{
	std::vector<std::string> endpoints = {
		"http://ifconfig.me",
		"http://api.ipify.org",
		"http://ipinfo.io/ip",
		"http://ipecho.net/plain",
	};

	for (auto &s: endpoints) {
		// http get request
		uri u(s);

		httplib::Client r(
			fmt::format(
				"{}://{}",
				u.get_scheme(),
				u.get_host()
			).c_str()
		);

		httplib::Headers headers = {
			{"Content-type", "text/plain; charset=utf-8"},
			{"User-Agent",   "curl/7.81.0"}
		};

		r.set_connection_timeout(1, 0);
		r.set_read_timeout(1, 0);
		r.set_write_timeout(1, 0);

		if (auto res = r.Get(fmt::format("/{}", u.get_path()).c_str(), headers)) {
			if (res->status == 200) {
				if (res->body.find('.') != std::string::npos) {
					return res->body;
				}
			}
		}
	}

	return {};
}

bool IpUtil::IsIPAddress(const std::string &ip_address)
{
	sockaddr_in sa{};
	int result = inet_pton(AF_INET, ip_address.c_str(), &(sa.sin_addr));
	return result != 0;
}


bool IpUtil::IsPortInUse(const std::string& ip, int port)
{
	bool in_use = false;

#ifdef _WIN32
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "WSAStartup failed\n";
		return true; // Assume in use on failure
	}
#endif

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
#ifdef _WIN32
		WSACleanup();
#endif
		return true; // Assume in use on failure
	}

#ifdef _WIN32
	int opt = 1;
	setsockopt(sock, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char*)&opt, sizeof(opt)); // Windows-specific
#else
	int opt = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // Linux/macOS
#endif

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	// Convert IP address from string to binary format
	if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0) {
		std::cerr << "Invalid IP address format: " << ip << std::endl;
#ifdef _WIN32
		closesocket(sock);
		WSACleanup();
#else
		close(sock);
#endif
		return true; // Assume in use on failure
	}

	if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
		in_use = true; // Bind failed, port is in use
	}

#ifdef _WIN32
	closesocket(sock);
	WSACleanup();
#else
	close(sock);
#endif

	return in_use;
}
