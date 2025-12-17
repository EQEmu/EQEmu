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

#include "serverinfo.h"

#include "uv.h"

size_t EQ::GetRSS()
{
	size_t rss = 0;

	if (0 != uv_resident_set_memory(&rss)) {
		return 0;
	}

	return rss;
}

double EQ::GetUptime()
{
	double uptime = 0.0;

	if (0 != uv_uptime(&uptime)) {
		return 0.0;
	}

	return uptime;
}

size_t EQ::GetPID()
{
	return uv_os_getpid();
}

std::vector<eq_cpu_info_t> EQ::GetCPUs()
{
	std::vector<eq_cpu_info_t> ret;
	uv_cpu_info_t *cpu_info = nullptr;
	int count = 0;

	if (0 != uv_cpu_info(&cpu_info, &count)) {
		return ret;
	}

	ret.reserve(count);
	for (int i = 0; i < count; ++i) {
		eq_cpu_info_t r;
		auto &entry = cpu_info[i];

		r.model = entry.model;
		r.speed = entry.speed / 1000.0;
		r.time_user = entry.cpu_times.user;
		r.time_sys = entry.cpu_times.sys;
		r.time_idle = entry.cpu_times.idle;
		r.time_nice = entry.cpu_times.nice;
		r.time_irq = entry.cpu_times.irq;

		ret.push_back(r);
	}

	uv_free_cpu_info(cpu_info, count);
	return ret;
}

eq_utsname_t EQ::GetOS()
{
	eq_utsname_t ret;
	uv_utsname_t name;

	if (0 != uv_os_uname(&name)) {
		return ret;
	}

	ret.machine = name.machine;
	ret.release = name.release;
	ret.sysname = name.sysname;
	ret.version = name.version;

	return ret;
}
