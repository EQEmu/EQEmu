/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2017 EQEMu Development Team (http://eqemu.org)

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

#include "common/net/console_server_connection.h"
#include "world/world_tcp_connection.h"

class WorldConsoleTCPConnection : public WorldTCPConnection
{
public:
	WorldConsoleTCPConnection(EQ::Net::ConsoleServerConnection *c) { m_connection = c; }
	virtual ~WorldConsoleTCPConnection() { }
	virtual void SendEmoteMessage(const char* to, uint32 to_guilddbid, int16 to_minstatus, uint32 type, const char* message, ...);
	virtual void SendEmoteMessageRaw(const char* to, uint32 to_guilddbid, int16 to_minstatus, uint32 type, const char* message);

	virtual inline bool IsConsole() { return true; }
	virtual inline bool IsZoneServer() { return false; }
private:
	EQ::Net::ConsoleServerConnection *m_connection;
};
