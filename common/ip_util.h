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
#pragma once

#include "common/types.h"

#include <string>

class IpUtil {
public:

	static uint32_t IPToUInt(const std::string &ip);
	static bool IsIpInRange(const std::string &ip, const std::string &network, const std::string &mask);
	static bool IsIpInPrivateRfc1918(const std::string &ip);
	static std::string GetLocalIPAddress();
	static std::string GetPublicIPAddress();

	static bool IsIPAddress(const std::string &ip_address);
	static bool IsPortInUse(const std::string& ip, int port);

};
