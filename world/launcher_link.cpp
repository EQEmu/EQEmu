/*	EQEMu: Everquest Server Emulator
Copyright (C) 2001-2006 EQEMu Development Team (http://eqemulator.net)

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

#include "launcher_link.h"

#include "common/md5.h"
#include "common/misc_functions.h"
#include "common/packet_dump.h"
#include "common/servertalk.h"
#include "common/strings.h"
#include "world/eql_config.h"
#include "world/launcher_list.h"
#include "world/world_config.h"
#include "world/worlddb.h"

#include <string>
#include <vector>

extern LauncherList launcher_list;

LauncherLink::LauncherLink(int id, std::shared_ptr<EQ::Net::ServertalkServerConnection> c)
	: ID(id),
	tcpc(c),
	m_name(""),
	m_bootTimer(2000)
{
	m_dynamicCount = 0;
	m_bootTimer.Disable();

	tcpc->OnMessage(std::bind(&LauncherLink::ProcessMessage, this, std::placeholders::_1, std::placeholders::_2));
	m_process_timer = std::make_unique<EQ::Timer>(100, true, std::bind(&LauncherLink::Process, this, std::placeholders::_1));
}

LauncherLink::~LauncherLink() {
}

void LauncherLink::Process(EQ::Timer *t) {
	if (m_bootTimer.Check(false)) {
		//force a boot on any zone which isnt running.
		std::map<std::string, ZoneState>::iterator cur, end;
		cur = m_states.begin();
		end = m_states.end();
		for (; cur != end; ++cur) {
			if (!cur->second.up) {
				StartZone(cur->first.c_str(), cur->second.port);
			}
		}
		m_bootTimer.Disable();
	}
}

void LauncherLink::ProcessMessage(uint16 opcode, EQ::Net::Packet &p)
{
	ServerPacket tpack(opcode, p);
	ServerPacket *pack = &tpack;

	switch (opcode) {
	case 0:
		break;
	case ServerOP_KeepAlive: {
		// ignore this
		break;
	}
	case ServerOP_ZAAuth: {
		LogInfo("Got authentication from [{}] when they are already authenticated", m_name.c_str());
		break;
	}
	case ServerOP_LauncherConnectInfo: {
		const LauncherConnectInfo *it = (const LauncherConnectInfo *)pack->pBuffer;
		if (HasName()) {
			LogInfo("Launcher [{}] received an additional connect packet with name [{}]. Ignoring", m_name.c_str(), it->name);
			break;
		}
		m_name = it->name;

		EQLConfig *config = launcher_list.GetConfig(m_name.c_str());
		if (config == nullptr) {
			LogInfo("Unknown launcher [{}] connected. Disconnecting", it->name);
			Disconnect();
			break;
		}

		LogInfo("Launcher Identified itself as [{}]. Loading zone list", it->name);

		std::vector<LauncherZone> result;
		//database.GetLauncherZones(it->name, result);
		config->GetZones(result);

		std::vector<LauncherZone>::iterator cur, end;
		cur = result.begin();
		end = result.end();
		ZoneState zs;
		for (; cur != end; ++cur) {
			zs.port = cur->port;
			zs.up = false;
			zs.starts = 0;
			LogInfo("[{}] Loaded zone [{}] on port [{}]", m_name.c_str(), cur->name.c_str(), zs.port);
			m_states[cur->name] = zs;
		}

		//now we add all the dynamics.
		BootDynamics(config->GetDynamicCount());

		m_bootTimer.Start();

		break;
	}
	case ServerOP_LauncherZoneStatus: {
		const LauncherZoneStatus *it = (const LauncherZoneStatus *)pack->pBuffer;
		std::map<std::string, ZoneState>::iterator res;
		res = m_states.find(it->short_name);
		if (res == m_states.end()) {
			LogInfo("[{}] reported state for zone [{}] which it does not have", m_name.c_str(), it->short_name);
			break;
		}
		LogInfo("[{}] [{}] reported state [{}] ([{}] starts)", m_name.c_str(), it->short_name, it->running ? "STARTED" : "STOPPED", it->start_count);
		res->second.up = it->running;
		res->second.starts = it->start_count;
		break;
	}
	default:
	{
		LogInfo("Unknown ServerOPcode from launcher {:#04x}, size [{}]", pack->opcode, pack->size);
		DumpPacket(pack->pBuffer, pack->size);
		break;
	}
	}
}

bool LauncherLink::ContainsZone(const char *short_name) const {
	return(m_states.find(short_name) != m_states.end());
}

void LauncherLink::BootZone(const char *short_name, uint16 port) {
	ZoneState zs;
	zs.port = port;
	zs.up = false;
	zs.starts = 0;
	LogInfo("[{}] Loaded zone [{}] on port [{}]", m_name.c_str(), short_name, zs.port);
	m_states[short_name] = zs;

	StartZone(short_name, port);
}

void LauncherLink::StartZone(const char *short_name) {
	StartZone(short_name, 0);
}

void LauncherLink::StartZone(const char *short_name, uint16 port) {
	auto pack = new ServerPacket(ServerOP_LauncherZoneRequest, sizeof(LauncherZoneRequest));
	LauncherZoneRequest* s = (LauncherZoneRequest *)pack->pBuffer;

	strn0cpy(s->short_name, short_name, 32);
	s->command = ZR_Start;
	s->port = port;

	SendPacket(pack);
	delete pack;
}

void LauncherLink::RestartZone(const char *short_name) {
	auto pack = new ServerPacket(ServerOP_LauncherZoneRequest, sizeof(LauncherZoneRequest));
	LauncherZoneRequest* s = (LauncherZoneRequest *)pack->pBuffer;

	strn0cpy(s->short_name, short_name, 32);
	s->command = ZR_Restart;
	s->port = 0;

	SendPacket(pack);
	delete pack;
}

void LauncherLink::StopZone(const char *short_name) {
	auto pack = new ServerPacket(ServerOP_LauncherZoneRequest, sizeof(LauncherZoneRequest));
	LauncherZoneRequest* s = (LauncherZoneRequest *)pack->pBuffer;

	strn0cpy(s->short_name, short_name, 32);
	s->command = ZR_Stop;
	s->port = 0;

	SendPacket(pack);
	delete pack;
}

void LauncherLink::BootDynamics(uint8 new_count) {
	if (m_dynamicCount == new_count)
		return;

	ZoneState zs;
	if (m_dynamicCount < new_count) {
		//we are booting more dynamics.

		zs.port = 0;
		zs.up = false;
		zs.starts = 0;

		int r;
		char nbuf[20];
		uint8 index;
		//"for each zone we need to boot"
		for (r = m_dynamicCount; r < new_count; r++) {
			//find an idle ID
			for (index = m_dynamicCount + 1; index < 255; index++) {
				sprintf(nbuf, "dynamic_%02d", index);
				if (m_states.find(nbuf) != m_states.end())
					continue;
				m_states[nbuf] = zs;
				StartZone(nbuf);
				break;
			}
		}
		m_dynamicCount = new_count;
	}
	else if (new_count == 0) {
		//kill all zones...
		std::map<std::string, ZoneState>::iterator cur, end;
		cur = m_states.begin();
		end = m_states.end();
		for (; cur != end; ++cur) {
			StopZone(cur->first.c_str());
		}
	}
	else {
		//need to get rid of some zones...

		//quick and dirty way to do this.. should do better (like looking for idle zones)
		int found = 0;
		std::map<std::string, ZoneState>::iterator cur, end;
		cur = m_states.begin();
		end = m_states.end();
		for (; cur != end; ++cur) {
			if (cur->first.find("dynamic_") == 0) {
				if (found >= new_count) {
					//this zone exceeds the number of allowed booted zones.
					StopZone(cur->first.c_str());
				}
				else {
					found++;
				}
			}
		}

		m_dynamicCount = new_count;
	}

}


void LauncherLink::GetZoneList(std::vector<std::string> &l) {
	std::map<std::string, ZoneState>::iterator cur, end;
	cur = m_states.begin();
	end = m_states.end();
	for (; cur != end; ++cur) {
		l.push_back(cur->first.c_str());
	}
}

void LauncherLink::GetZoneDetails(const char *short_name, std::map<std::string, std::string> &res) {
	res.clear();

	std::map<std::string, ZoneState>::iterator r;
	r = m_states.find(short_name);
	if (r == m_states.end()) {
		res["error"] = "Zone Not Found";
		res["name"] = short_name;
		res["up"] = "0";
		res["starts"] = "0";
		res["port"] = "0";
	}
	else {
		res["name"] = r->first;
		res["up"] = r->second.up ? "1" : "0";
		res["starts"] = itoa(r->second.starts);
		res["port"] = itoa(r->second.port);
	}
}

void LauncherLink::Shutdown() {
	auto pack = new ServerPacket(ServerOP_ShutdownAll);
	SendPacket(pack);
	delete pack;
}
