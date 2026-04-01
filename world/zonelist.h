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

#include "common/eqtime.h"
#include "common/event/timer.h"
#include "common/server_reload_types.h"
#include "common/timer.h"
#include "common/types.h"

#include <deque>
#include <memory>
#include <mutex>
#include <vector>

class WorldTCPConnection;
class ServerPacket;
class ZoneServer;

class ZSList
{
public:
	enum { MaxLockedZones = 10 };

	static void ShowUpTime(WorldTCPConnection* con, const char* adminname = 0);

	ZSList();
	~ZSList();

	void Init();
	bool IsZoneLocked(uint16 iZoneID);
	bool SendPacket(ServerPacket *pack);
	bool SendPacket(uint32 zoneid, ServerPacket *pack);
	bool SendPacket(uint32 zoneid, uint16 instanceid, ServerPacket *pack);
	bool SendPacketToZonesWithGuild(uint32 guild_id, ServerPacket *pack);
	bool SendPacketToZonesWithGMs(ServerPacket *pack);
	bool SendPacketToBootedZones(ServerPacket* pack);
	bool SetLockedZone(uint16 iZoneID, bool iLock);

	EQTime worldclock;

	inline uint32 GetNextID() { return NextID++; }
	int GetZoneCount();

	Timer *reminder;
	Timer *shutdowntimer;

	uint16 GetAvailableZonePort();
	uint32 TriggerBootup(uint32 iZoneID, uint32 iInstanceID = 0);

	void Add(ZoneServer *zoneserver);
	void GetZoneIDList(std::vector<uint32> &zones);
	void KillAll();
	void ListLockedZones(const char *to, WorldTCPConnection *connection);
	void NextGroupIDs(uint32 &start, uint32 &end);
	void Process();
	void RebootZone(const char *ip1, uint16 port, const char *ip2, uint32 skipid, uint32 zoneid = 0);
	void Remove(const std::string &uuid);
	void SendChannelMessage(const char *from, const char *to, uint8 chan_num, uint8 language, const char *message, ...);
	void SendChannelMessageRaw(const char *from, const char *to, uint8 chan_num, uint8 language, const char *message);
	void SendEmoteMessage(const char *to, uint32 to_guilddbid, int16 to_minstatus, uint32 type, const char *message, ...);
	void SendEmoteMessageRaw(const char *to, uint32 to_guilddbid, int16 to_minstatus, uint32 type, const char *message);
	void SendLSZones();
	void SendTimeSync();
	void SendZoneStatus(const char *to, int16 admin, WorldTCPConnection *connection);
	void SOPZoneBootup(const char *adminname, uint32 ZoneServerID, const char *zonename, bool iMakeStatic = false);
	void UpdateUCSServerAvailable(bool ucss_available = true);
	void WorldShutDown(uint32 time, uint32 interval);
	void DropClient(uint32 lsid, ZoneServer *ignore_zoneserver);

	ZoneServer*	FindByPort(uint16 port);
	ZoneServer* FindByID(uint32 ZoneID);
	ZoneServer* FindByInstanceID(uint32 InstanceID);
	ZoneServer* FindByName(const char* zonename);
	ZoneServer* FindByZoneID(uint32 ZoneID);

	const std::list<std::unique_ptr<ZoneServer>> &getZoneServerList() const;
	inline uint32_t GetServerListCount() { return zone_server_list.size(); }
	void SendServerReload(ServerReload::Type type, uchar *packet = nullptr);
	std::mutex m_queued_reloads_mutex;
	std::vector<ServerReload::Type> m_queued_reloads = {};

	void QueueServerReload(ServerReload::Type &type);

	static ZSList* Instance()
	{
		static ZSList instance;
		return &instance;
	}

private:
	void OnTick(EQ::Timer *t);
	uint32 NextID;
	uint16	pLockedZones[MaxLockedZones];
	uint32 CurGroupID;
	std::deque<uint16> m_ports_free;
	std::unique_ptr<EQ::Timer> m_tick;
	std::unique_ptr<EQ::Timer> m_keepalive;

	std::list<std::unique_ptr<ZoneServer>> zone_server_list;
};
