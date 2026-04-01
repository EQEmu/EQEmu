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

#include "common/eq_packet_structs.h"
#include "common/mutex.h"
#include "common/queue.h"
#include "common/servertalk.h"
#include "common/timer.h"

#include <list>

class LoginServer;

class LoginServerList{
public:
	LoginServerList();
	~LoginServerList();

	void	Add(const char*, uint16, const char*, const char*, bool);
	bool	SendStatus();
	bool	SendPacket(ServerPacket *pack);
	bool	SendAccountUpdate(ServerPacket *pack);
	bool	Connected();
	size_t GetServerCount() const { return m_list.size(); }

	static LoginServerList* Instance()
	{
		static LoginServerList instance;
		return &instance;
	}

protected:
	std::list<std::unique_ptr<LoginServer>> m_list;
};
