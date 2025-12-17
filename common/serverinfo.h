/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
#pragma once

#include <cstdint>
#include <string>
#include <vector>

typedef struct eq_cpu_info_s {
	std::string model;
	double speed;
	uint64_t time_user;
	uint64_t time_nice;
	uint64_t time_sys;
	uint64_t time_idle;
	uint64_t time_irq;
} eq_cpu_info_t;

typedef struct eq_utsname_s {
	std::string sysname;
	std::string release;
	std::string version;
	std::string machine;
} eq_utsname_t;

namespace EQ
{
	size_t GetRSS();
	double GetUptime();
	size_t GetPID();
	std::vector<eq_cpu_info_t> GetCPUs();
	eq_utsname_t GetOS();
}
