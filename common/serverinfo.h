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
