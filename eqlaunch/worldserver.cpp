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

#include "worldserver.h"

#include "common/eqemu_config.h"
#include "common/eqemu_logsys.h"
#include "common/servertalk.h"
#include "common/strings.h"
#include "eqlaunch/zone_launch.h"

WorldServer::WorldServer(std::map<std::string, ZoneLaunch *> &zones, const char *name, const EQEmuConfig *config)
	: m_name(name),
	m_config(config),
	m_zones(zones)
{
	m_connection = std::make_unique<EQ::Net::ServertalkClient>(config->WorldIP, config->WorldTCPPort, false, "Launcher", config->SharedKey);
	m_connection->OnConnect([this](EQ::Net::ServertalkClient *client) {
		OnConnected();
	});

	m_connection->OnMessage(std::bind(&WorldServer::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
}

WorldServer::~WorldServer() {
}

void WorldServer::OnConnected() {

	auto pack = new ServerPacket(ServerOP_LauncherConnectInfo, sizeof(LauncherConnectInfo));
	LauncherConnectInfo* sci = (LauncherConnectInfo*)pack->pBuffer;
	strn0cpy(sci->name, m_name, sizeof(sci->name));
	m_connection->SendPacket(pack);
	safe_delete(pack);

	//send status for all zones...
	std::map<std::string, ZoneLaunch *>::iterator cur, end;
	cur = m_zones.begin();
	end = m_zones.end();
	for (; cur != end; ++cur) {
		cur->second->SendStatus();
	}
}

void WorldServer::HandleMessage(uint16 opcode, EQ::Net::Packet &p) {
	ServerPacket tpack(opcode, p);
	ServerPacket *pack = &tpack;

	switch (opcode) {
	case 0: {
		break;
	}
	case ServerOP_EmoteMessage:
	case ServerOP_KeepAlive: {
		// ignore this
		break;
	}
	case ServerOP_LauncherZoneRequest: {
		if (pack->size != sizeof(LauncherZoneRequest)) {
			Log(Logs::Detail, Logs::Launcher, "Invalid size of LauncherZoneRequest: %d", pack->size);
			break;
		}
		const LauncherZoneRequest *lzr = (const LauncherZoneRequest *)pack->pBuffer;

		switch (ZoneRequestCommands(lzr->command)) {
		case ZR_Start: {
			if (m_zones.find(lzr->short_name) != m_zones.end()) {
				Log(Logs::Detail, Logs::Launcher, "World told us to start zone %s, but it is already running.", lzr->short_name);
			}
			else {
				Log(Logs::Detail, Logs::Launcher, "World told us to start zone %s.", lzr->short_name);
				auto l = new ZoneLaunch(this, m_name, lzr->short_name, lzr->port, m_config);
				m_zones[lzr->short_name] = l;
			}
			break;
		}
		case ZR_Restart: {
			auto res = m_zones.find(lzr->short_name);
			if (res == m_zones.end()) {
				Log(Logs::Detail, Logs::Launcher, "World told us to restart zone %s, but it is not running.", lzr->short_name);
			}
			else {
				Log(Logs::Detail, Logs::Launcher, "World told us to restart zone %s.", lzr->short_name);
				res->second->Restart();
			}
			break;
		}
		case ZR_Stop: {
			auto res = m_zones.find(lzr->short_name);
			if (res == m_zones.end()) {
				Log(Logs::Detail, Logs::Launcher, "World told us to stop zone %s, but it is not running.", lzr->short_name);
			}
			else {
				Log(Logs::Detail, Logs::Launcher, "World told us to stop zone %s.", lzr->short_name);
				res->second->Stop();
			}
			break;
		}
		}
		break;
	}
	case ServerOP_GroupIDReply: {
		//ignore this, world is still being dumb
		break;
	}

	default: {
		Log(Logs::Detail, Logs::Launcher, "Unknown opcode 0x%x from World of len %d", pack->opcode, pack->size);
		break;
	}
	}
}



void WorldServer::SendStatus(const char *short_name, uint32 start_count, bool running) {
	auto pack = new ServerPacket(ServerOP_LauncherZoneStatus, sizeof(LauncherZoneStatus));
	LauncherZoneStatus* it = (LauncherZoneStatus*)pack->pBuffer;

	strn0cpy(it->short_name, short_name, 32);
	it->start_count = start_count;
	it->running = running ? 1 : 0;

	m_connection->SendPacket(pack);
	safe_delete(pack);
}
