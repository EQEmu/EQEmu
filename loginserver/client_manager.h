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

#include "common/net/eqstream.h"
#include "loginserver/client.h"

#include <list>

class ClientManager {
public:
	ClientManager();
	~ClientManager();
	void Process();
	void RemoveExistingClient(unsigned int c, const std::string &loginserver);
	Client *GetClient(unsigned int c, const std::string &loginserver);
private:
	void ProcessDisconnect();

	std::list<Client *>      m_clients;
	OpcodeManager            *m_titanium_ops;
	EQ::Net::EQStreamManager *m_titanium_stream;
	OpcodeManager            *m_sod_ops;
	EQ::Net::EQStreamManager *m_sod_stream;
	OpcodeManager            *m_larion_ops;
	EQ::Net::EQStreamManager *m_larion_stream;
};
