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

#pragma once

#include "common/event/timer.h"
#include "common/net/servertalk_server_connection.h"
#include "common/timer.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

class ServerPacket;

class LauncherLink {
public:
	LauncherLink(int id, std::shared_ptr<EQ::Net::ServertalkServerConnection> tcpc);
	~LauncherLink();

	void Process(EQ::Timer *t);
	void ProcessMessage(uint16 opcode, EQ::Net::Packet &p);
	void SendPacket(ServerPacket* pack) { tcpc->SendPacket(pack); }

	int GetID() const { return(ID); }
	void Disconnect() { if (tcpc->Handle()) { tcpc->Handle()->Disconnect(); } }

	inline bool	HasName() const		{ return(m_name.length() > 0); }
	inline std::string GetIP() const { return tcpc->Handle() ? tcpc->Handle()->RemoteIP() : 0; }
	inline uint16 GetPort() const { return tcpc->Handle() ? tcpc->Handle()->RemotePort() : 0; }
	inline std::string GetUUID() const { return tcpc->GetUUID(); }
	inline const char * GetName() const		{ return(m_name.c_str()); }

	bool ContainsZone(const char *short_name) const;

	//commands
	void Shutdown();
	void BootZone(const char *short_name, uint16 port);
	void StartZone(const char *short_name);
	void StartZone(const char *short_name, uint16 port);
	void RestartZone(const char *short_name);
	void StopZone(const char *short_name);
	void BootDynamics(uint8 new_total);

	void GetZoneList(std::vector<std::string> &list);
	void GetZoneDetails(const char *short_name, std::map<std::string,std::string> &result);

protected:
	const int			ID;
	std::shared_ptr<EQ::Net::ServertalkServerConnection> tcpc;
	std::unique_ptr<EQ::Timer> m_process_timer;
	std::string			m_name;
	Timer				m_bootTimer;

	uint8 m_dynamicCount;

	typedef struct {
		bool up;
		uint32 starts;	//number of times this zone has started
		uint16 port;	//the port this zone wants to use (0=pick one)
	} ZoneState;
	std::map<std::string, ZoneState> m_states;
};
