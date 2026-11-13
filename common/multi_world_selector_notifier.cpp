/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.
*/
#include "common/multi_world_selector_notifier.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace EQ::Net::MultiWorldSelector {

bool Notify(const Config &config, const ControlSelection &selection, std::string &error)
{
	error.clear();
	if (!config.enabled) {
		return true;
	}

	if (!IsLoopbackIPv4(config.control_bind) || config.control_port == 0) {
		error = "control_bind must be a loopback IPv4 address and control_port must be non-zero";
		return false;
	}

	std::array<std::uint8_t, ControlPacketSize> packet{};
	if (!EncodeControlPacket(selection, packet)) {
		error = "invalid client IPv4 address, source port, or World short name";
		return false;
	}

#ifdef _WIN32
	WSADATA winsock_data{};
	if (WSAStartup(MAKEWORD(2, 2), &winsock_data) != 0) {
		error = "WSAStartup failed";
		return false;
	}
	const auto socket_handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socket_handle == INVALID_SOCKET) {
		error = "socket creation failed: " + std::to_string(WSAGetLastError());
		WSACleanup();
		return false;
	}
#else
	const auto socket_handle = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_handle < 0) {
		error = std::string("socket creation failed: ") + std::strerror(errno);
		return false;
	}
#endif

	sockaddr_in destination{};
	destination.sin_family = AF_INET;
	destination.sin_port   = htons(config.control_port);
	if (inet_pton(AF_INET, config.control_bind.c_str(), &destination.sin_addr) != 1) {
		error = "control_bind is not a numeric IPv4 address";
#ifdef _WIN32
		closesocket(socket_handle);
		WSACleanup();
#else
		close(socket_handle);
#endif
		return false;
	}

	const auto sent = sendto(
		socket_handle,
		reinterpret_cast<const char *>(packet.data()),
		static_cast<int>(packet.size()),
		0,
		reinterpret_cast<const sockaddr *>(&destination),
		static_cast<int>(sizeof(destination))
	);

#ifdef _WIN32
	if (sent == SOCKET_ERROR) {
		error = "sendto failed: " + std::to_string(WSAGetLastError());
	}
	closesocket(socket_handle);
	WSACleanup();
#else
	if (sent < 0) {
		error = std::string("sendto failed: ") + std::strerror(errno);
	}
	close(socket_handle);
#endif

	return sent == static_cast<decltype(sent)>(packet.size());
}

} // namespace EQ::Net::MultiWorldSelector
