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

#include "world_tcp_connection.h"

#include "common/net/servertalk_server.h"
#include "common/event/timer.h"
#include "common/timer.h"
#include "common/emu_constants.h"
#include "world/console.h"

#include <cstring>
#include <string>

class Client;
class ServerPacket;

class ZoneServer : public WorldTCPConnection {
public:
	ZoneServer(std::shared_ptr<EQ::Net::ServertalkServerConnection> in_connection, EQ::Net::ConsoleServer *in_console);
	~ZoneServer();
	virtual inline bool IsZoneServer() { return true; }

	void        SendPacket(ServerPacket* pack) { tcpc->SendPacket(pack); }
	void		SendEmoteMessage(const char* to, uint32 to_guilddbid, int16 to_minstatus, uint32 type, const char* message, ...);
	void		SendEmoteMessageRaw(const char* to, uint32 to_guilddbid, int16 to_minstatus, uint32 type, const char* message);
	void		SendKeepAlive();
	bool		SetZone(uint32 in_zone_id, uint32 in_instance_id = 0, bool in_is_static_zone = false);
	void		TriggerBootup(uint32 in_zone_id = 0, uint32 in_instance_id = 0, const char* admin_name = 0, bool is_static_zone = false);
	void		Disconnect() { auto handle = tcpc->Handle(); if (handle) { handle->Disconnect(); } }
	void		IncomingClient(Client* client);
	void		LSBootUpdate(uint32 zone_id, uint32 instance_id = 0, bool startup = false);
	void		LSSleepUpdate(uint32 zone_id);
	uint32		GetPrevZoneID() { return zone_server_previous_zone_id; }
	void		ChangeWID(uint32 iCharID, uint32 iWID);
	void		SendGroupIDs();
	void        HandleMessage(uint16 opcode, const EQ::Net::Packet &p);

	inline const char*	GetZoneName() const	{ return zone_name; }
	inline const char*	GetZoneLongName() const	{ return long_name; }
	inline std::string  GetCurrentVersion() const { return CURRENT_VERSION; }
	void                CheckToClearTraderAndBuyerTables();
	inline std::string  GetCompileDate() const { return COMPILE_DATE; }
	const char*			GetCompileTime() const{ return compiled; }
	void				SetCompile(char* in_compile){ strcpy(compiled,in_compile); }
	inline uint32		GetZoneID() const	{ return zone_server_zone_id; }
	inline bool         IsConnected() const { return tcpc->Handle() ? tcpc->Handle()->IsConnected() : false; }
	inline std::string	GetIP() const		{ return tcpc->Handle() ? tcpc->Handle()->RemoteIP() : ""; }
	inline uint16		GetPort() const		{ return tcpc->Handle() ? tcpc->Handle()->RemotePort() : 0; }
	inline const char*	GetCAddress() const	{ return client_address; }
	inline const char*	GetCLocalAddress() const { return client_local_address; }
	inline uint16		GetCPort() const	{ return client_port; }
	inline uint32		GetID() const		{ return zone_server_id; }
	inline bool			IsBootingUp() const	{ return is_booting_up; }
	inline bool			IsStaticZone() const{ return is_static_zone; }
	inline uint32		NumPlayers() const	{ return zone_player_count; }
	inline void			AddPlayer()			{ zone_player_count++; }
	inline void			RemovePlayer()		{ zone_player_count--; }
	inline const char * GetLaunchName() const { return(launcher_name.c_str()); }
	inline const char * GetLaunchedName() const { return(launched_name.c_str()); }
	std::string         GetUUID() const { return tcpc->GetUUID(); }

	inline uint32		GetInstanceID() { return instance_id; }

	inline uint32		GetZoneOSProcessID() { return zone_os_process_id; }

private:
	std::shared_ptr<EQ::Net::ServertalkServerConnection> tcpc;
	std::unique_ptr<EQ::Timer> boot_timer_obj;

	uint32	zone_server_id;
	char	client_address[250];
	char	client_local_address[250];
	uint16	client_port;
	bool	is_booting_up;
	bool	is_static_zone;
	uint32	zone_player_count;
	char	compiled[25];
	char	zone_name[32];
	char	long_name[256];
	uint32	zone_server_zone_id;
	uint32	zone_server_previous_zone_id;
	Timer	zone_boot_timer;
	uint32	instance_id;	//instance ids contain a zone id, and a zone version
	uint32  zone_os_process_id;
	std::string launcher_name;	//the launcher which started us
	std::string launched_name;	//the name of the zone we launched.
	EQ::Net::ConsoleServer *console;
};
