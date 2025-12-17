/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2019 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
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
	static std::string DNSLookupSync(
		const std::string &addr,
		int port
	);
	static bool IsIPAddress(const std::string &ip_address);
	static bool IsPortInUse(const std::string& ip, int port);

};
