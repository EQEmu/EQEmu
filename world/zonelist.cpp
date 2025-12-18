/*	EQEMu: Everquest Server Emulator
Copyright (C) 2001-2005 EQEMu Development Team (http://eqemulator.net)

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

#include "zonelist.h"

#include "common/content/world_content_service.h"
#include "common/event_sub.h"
#include "common/events/player_event_logs.h"
#include "common/json/json.h"
#include "common/misc_functions.h"
#include "common/patches/patches.h"
#include "common/random.h"
#include "common/repositories/buyer_repository.h"
#include "common/repositories/trader_repository.h"
#include "common/servertalk.h"
#include "common/skill_caps.h"
#include "common/strings.h"
#include "common/zone_store.h"
#include "world/clientlist.h"
#include "world/dynamic_zone_manager.h"
#include "world/queryserv.h"
#include "world/shared_task_manager.h"
#include "world/ucs.h"
#include "world/web_interface.h"
#include "world/world_boot.h"
#include "world/world_config.h"
#include "world/worlddb.h"
#include "world/zoneserver.h"

extern uint32 numzones;
volatile bool UCSServerAvailable_ = false;
void CatchSignal(int sig_num);

ZSList::ZSList()
{
	NextID = 1;
	CurGroupID = 1;
	memset(pLockedZones, 0, sizeof(pLockedZones));

	m_tick = std::make_unique<EQ::Timer>(5000, true, std::bind(&ZSList::OnTick, this, std::placeholders::_1));
}

ZSList::~ZSList() {
}

void ZSList::ShowUpTime(WorldTCPConnection* con, const char* adminname) {
	uint32 ms = Timer::GetCurrentTime();
	std::string time_string = Strings::MillisecondsToTime(ms);
	con->SendEmoteMessage(
		adminname,
		0,
		AccountStatus::Player,
		Chat::White,
		fmt::format(
			"Worldserver Uptime | {}",
			time_string
		).c_str()
	);
}

void ZSList::Add(ZoneServer* zoneserver) {
	zone_server_list.emplace_back(std::unique_ptr<ZoneServer>(zoneserver));
	zoneserver->SendGroupIDs();
}

void ZSList::Remove(const std::string &uuid)
{
	auto iter = zone_server_list.begin();
	while (iter != zone_server_list.end()) {
		if ((*iter)->GetUUID().compare(uuid) == 0) {
			auto port = (*iter)->GetCPort();
			(*iter)->CheckToClearTraderAndBuyerTables();

			zone_server_list.erase(iter);

			if (port != 0) {
				m_ports_free.push_back(port);
			}
			return;
		}
		iter++;
	}
}

void ZSList::KillAll() {
	auto iterator = zone_server_list.begin();
	while (iterator != zone_server_list.end()) {
		(*iterator)->Disconnect();
		iterator = zone_server_list.erase(iterator);
	}
}

void ZSList::Process() {

	if (shutdowntimer && shutdowntimer->Check()) {
		LogInfo("Shutdown timer has expired. Telling all zones to shut down and exiting. (fake sigint)");
		auto pack2 = new ServerPacket;
		pack2->opcode = ServerOP_ShutdownAll;
		pack2->size = 0;
		SendPacket(pack2);
		safe_delete(pack2);
		Process();
		CatchSignal(2);
	}

	if (reminder && reminder->Check() && shutdowntimer) {
		SendEmoteMessage(
			0,
			0,
			AccountStatus::Player,
			Chat::System,
			fmt::format(
				"[SYSTEM] World will be shutting down in {} minutes.",
				((shutdowntimer->GetRemainingTime() / 1000) / 60)
			).c_str()
		);
	}

	if (!m_queued_reloads.empty()) {
		m_queued_reloads_mutex.lock();
		for (auto &type : m_queued_reloads) {
			LogInfo("Sending reload of type [{}] to zones", ServerReload::GetName(type));
			SendServerReload(type, nullptr);
		}
		m_queued_reloads.clear();
		m_queued_reloads_mutex.unlock();
	}
}

bool ZSList::SendPacket(ServerPacket* pack) {
	auto iterator = zone_server_list.begin();
	while (iterator != zone_server_list.end()) {
		(*iterator)->SendPacket(pack);
		iterator++;
	}
	return true;
}

bool ZSList::SendPacket(uint32 ZoneID, ServerPacket* pack) {
	auto iterator = zone_server_list.begin();
	while (iterator != zone_server_list.end()) {
		if ((*iterator)->GetZoneID() == ZoneID) {
			ZoneServer* tmp = (*iterator).get();
			tmp->SendPacket(pack);
			return true;
		}
		iterator++;
	}
	return(false);
}

bool ZSList::SendPacket(uint32 ZoneID, uint16 instanceID, ServerPacket* pack) {
	if (instanceID != 0)
	{
		auto iterator = zone_server_list.begin();
		while (iterator != zone_server_list.end()) {
			if ((*iterator)->GetInstanceID() == instanceID) {
				ZoneServer* tmp = (*iterator).get();
				tmp->SendPacket(pack);
				return true;
			}
			iterator++;
		}
	}
	else
	{
		auto iterator = zone_server_list.begin();
		while (iterator != zone_server_list.end()) {
			if ((*iterator)->GetZoneID() == ZoneID
				&& (*iterator)->GetInstanceID() == 0) {
				ZoneServer* tmp = (*iterator).get();
				tmp->SendPacket(pack);
				return true;
			}
			iterator++;
		}
	}
	return(false);
}

ZoneServer* ZSList::FindByName(const char* zonename) {
	auto iterator = zone_server_list.begin();
	while (iterator != zone_server_list.end()) {
		if (strcasecmp((*iterator)->GetZoneName(), zonename) == 0) {
			ZoneServer* tmp = (*iterator).get();
			return tmp;
		}
		iterator++;
	}
	return 0;
}

ZoneServer* ZSList::FindByID(uint32 ZoneID) {
	auto iterator = zone_server_list.begin();
	while (iterator != zone_server_list.end()) {
		if ((*iterator)->GetID() == ZoneID) {
			ZoneServer* tmp = (*iterator).get();
			return tmp;
		}
		iterator++;
	}
	return 0;
}

ZoneServer* ZSList::FindByZoneID(uint32 ZoneID) {
	auto iterator = zone_server_list.begin();
	while (iterator != zone_server_list.end()) {
		ZoneServer* tmp = (*iterator).get();
		if (tmp->GetZoneID() == ZoneID && tmp->GetInstanceID() == 0) {
			return tmp;
		}
		iterator++;
	}
	return 0;
}

ZoneServer* ZSList::FindByPort(uint16 port) {
	auto iterator = zone_server_list.begin();
	while (iterator != zone_server_list.end()) {
		if ((*iterator)->GetCPort() == port) {
			ZoneServer* tmp = (*iterator).get();
			return tmp;
		}
		iterator++;
	}
	return 0;
}

ZoneServer* ZSList::FindByInstanceID(uint32 InstanceID)
{
	auto iterator = zone_server_list.begin();
	while (iterator != zone_server_list.end()) {
		if ((*iterator)->GetInstanceID() == InstanceID) {
			ZoneServer* tmp = (*iterator).get();
			return tmp;
		}
		iterator++;
	}
	return 0;
}

bool ZSList::SetLockedZone(uint16 iZoneID, bool iLock) {
	for (auto &zone : pLockedZones) {
		if (iLock) {
			if (zone == 0) {
				zone = iZoneID;
				return true;
			}
		}
		else {
			if (zone == iZoneID) {
				zone = 0;
				return true;
			}
		}
	}
	return false;
}

void ZSList::Init()
{
	const WorldConfig* Config = WorldConfig::get();
	for (uint16 i = Config->ZonePortLow; i <= Config->ZonePortHigh; ++i) {
		m_ports_free.push_back(i);
	}
}

bool ZSList::IsZoneLocked(uint16 iZoneID) {
	for (auto &zone : pLockedZones) {
		if (zone == iZoneID)
			return true;
	}
	return false;
}

void ZSList::ListLockedZones(const char* to, WorldTCPConnection* connection) {
	int zone_count = 0;
	for (const auto& zone_id : pLockedZones) {
		if (zone_id) {
			int zone_number = (zone_count + 1);
			connection->SendEmoteMessageRaw(
				to,
				0,
				AccountStatus::Player,
				Chat::White,
				fmt::format(
					"Zone {} | Name: {} ({}) ID: {}",
					zone_number,
					ZoneLongName(zone_id),
					ZoneName(zone_id),
					zone_id
				).c_str()
			);
			zone_count++;
		}
	}

	std::string zone_message = (
		zone_count ?
		fmt::format("{} Zones are locked.", zone_count) :
		"There are no zones locked."
	);
	connection->SendEmoteMessage(
		to,
		0,
		AccountStatus::Player,
		Chat::White,
		zone_message.c_str()
	);
}

void ZSList::SendZoneStatus(const char* to, int16 admin, WorldTCPConnection* connection) {
	char locked[4];
	if (WorldConfig::get()->Locked == true) {
		strcpy(locked, "Yes");
	}
	else {
		strcpy(locked, "No");
	}

	auto out = fmt::memory_buffer();

	if (connection->IsConsole()) {
		fmt::format_to(std::back_inserter(out), "World Locked: {}\r\n", locked);
	}
	else {
		fmt::format_to(std::back_inserter(out), "World Locked: {}^", locked);
	}
	if (connection->IsConsole()) {
		fmt::format_to(std::back_inserter(out), "Zoneservers online:\r\n");
	}
	else {
		fmt::format_to(std::back_inserter(out), "Zoneservers online:^");
	}

	int v = 0, w = 0, x = 0, y = 0, z = 0;
	char is_static_string[2] = { 0, 0 }, zone_data_string[64];
	memset(zone_data_string, 0, sizeof(zone_data_string));

	ZoneServer* zone_server_data = 0;

	auto iterator = zone_server_list.begin();
	while (iterator != zone_server_list.end()) {
		zone_server_data = (*iterator).get();
		auto addr = zone_server_data->GetIP();

		if (zone_server_data->IsStaticZone()) {
			z++;
		}
		else if (zone_server_data->GetZoneID() != 0) {
			w++;
		}
		else if (zone_server_data->GetZoneID() == 0 && !zone_server_data->IsBootingUp()) {
			v++;
		}

		if (zone_server_data->IsStaticZone())
			is_static_string[0] = 'S';
		else
			is_static_string[0] = 'D';

		if (admin >= AccountStatus::GMLeadAdmin) {
			if (zone_server_data->GetZoneID()) {
				snprintf(zone_data_string, sizeof(zone_data_string), "%s (%i)", zone_server_data->GetZoneName(), zone_server_data->GetZoneID());
			}
			else if (zone_server_data->IsBootingUp()) {
				strcpy(zone_data_string, "...");
			}
			else {
				zone_data_string[0] = 0;
			}

			fmt::format_to(std::back_inserter(out),
				"#{:<3} :: {} :: {}:{:<5} :: {:2} :: {}:{} :: {} :: ({})",
				zone_server_data->GetID(),
				is_static_string,
				addr.c_str(),
				zone_server_data->GetPort(),
				zone_server_data->NumPlayers(),
				zone_server_data->GetCAddress(),
				zone_server_data->GetCPort(),
				zone_data_string,
				zone_server_data->GetZoneOSProcessID()
				);

			if (out.size() >= 3584) {
				connection->SendEmoteMessageRaw(
					to,
					0,
					AccountStatus::Player,
					Chat::NPCQuestSay,
					out.data()
				);
				out.clear();
			}
			else {
				if (connection->IsConsole())
					fmt::format_to(std::back_inserter(out), "\r\n");
				else
					fmt::format_to(std::back_inserter(out), "^");
			}
			x++;
		}
		else if (zone_server_data->GetZoneID() != 0) {
			if (zone_server_data->GetZoneID())
				strcpy(zone_data_string, zone_server_data->GetZoneName());
			else
				zone_data_string[0] = 0;
			fmt::format_to(std::back_inserter(out), "  #{} {}  {}", zone_server_data->GetID(), is_static_string, zone_data_string);
			if (out.size() >= 3584) {
				connection->SendEmoteMessageRaw(
					to,
					0,
					AccountStatus::Player,
					Chat::NPCQuestSay,
					out.data()
				);
				out.clear();
			}
			else {
				if (connection->IsConsole()) {
					fmt::format_to(std::back_inserter(out), "\r\n");
				}
				else {
					fmt::format_to(std::back_inserter(out), "^");
				}
			}
			x++;
		}
		y++;
		iterator++;
	}

	if (connection->IsConsole()) {
		fmt::format_to(std::back_inserter(out), "{} servers listed. {} servers online.\r\n", x, y);
	}
	else {
		fmt::format_to(std::back_inserter(out), "{} servers listed. {} servers online.^", x, y);
	}

	fmt::format_to(std::back_inserter(out), "{} zones are static zones, {} zones are booted zones, {} zones available.", z, w, v);

	connection->SendEmoteMessageRaw(
		to,
		0,
		AccountStatus::Player,
		Chat::NPCQuestSay,
		out.data()
	);
}

void ZSList::SendChannelMessage(const char* from, const char* to, uint8 chan_num, uint8 language, const char* message, ...) {
	if (!message)
		return;
	va_list argptr;
	char buffer[1024];

	va_start(argptr, message);
	vsnprintf(buffer, sizeof(buffer), message, argptr);
	va_end(argptr);

	SendChannelMessageRaw(from, to, chan_num, language, buffer);
}

void ZSList::SendChannelMessageRaw(const char* from, const char* to, uint8 chan_num, uint8 language, const char* message) {
	if (!message)
		return;
	auto pack = new ServerPacket;

	pack->opcode = ServerOP_ChannelMessage;
	pack->size = sizeof(ServerChannelMessage_Struct) + strlen(message) + 1;
	pack->pBuffer = new uchar[pack->size];
	memset(pack->pBuffer, 0, pack->size);
	ServerChannelMessage_Struct* scm = (ServerChannelMessage_Struct*)pack->pBuffer;
	if (from == 0) {
		strcpy(scm->from, "WServer");
		scm->noreply = true;
	}
	else if (from[0] == 0) {
		strcpy(scm->from, "WServer");
		scm->noreply = true;
	}
	else
		strcpy(scm->from, from);
	if (to != 0) {
		strcpy((char *)scm->to, to);
		strcpy((char *)scm->deliverto, to);
	}
	else {
		scm->to[0] = 0;
		scm->deliverto[0] = 0;
	}

	scm->language = language;
	scm->lang_skill = 100;
	scm->chan_num = chan_num;
	strcpy(&scm->message[0], message);

	SendPacket(pack);
	delete pack;
}


void ZSList::SendEmoteMessage(const char* to, uint32 to_guilddbid, int16 to_minstatus, uint32 type, const char* message, ...) {
	if (!message)
		return;
	va_list argptr;
	char buffer[1024];

	va_start(argptr, message);
	vsnprintf(buffer, sizeof(buffer), message, argptr);
	va_end(argptr);

	SendEmoteMessageRaw(to, to_guilddbid, to_minstatus, type, buffer);
}

void ZSList::SendEmoteMessageRaw(
	const char *to,
	uint32 to_guilddbid,
	int16 to_minstatus,
	uint32 type,
	const char *message
)
{
	if (!message) {
		return;
	}
	auto pack = new ServerPacket;

	pack->opcode  = ServerOP_EmoteMessage;
	pack->size    = sizeof(ServerEmoteMessage_Struct) + strlen(message) + 1;
	pack->pBuffer = new uchar[pack->size];
	memset(pack->pBuffer, 0, pack->size);
	ServerEmoteMessage_Struct *sem = (ServerEmoteMessage_Struct *) pack->pBuffer;

	if (to) {
		strcpy((char *) sem->to, to);
	}
	else {
		sem->to[0] = 0;
	}

	sem->guilddbid = to_guilddbid;
	sem->minstatus = to_minstatus;
	sem->type      = type;
	strcpy(&sem->message[0], message);
	char tempto[64] = {0};
	if (to) {
		strn0cpy(tempto, to, 64);
	}

	if (tempto[0] == 0) {
		if (to_guilddbid > 0) {
			SendPacketToZonesWithGuild(to_guilddbid, pack);
		}
		else if (to_minstatus > 0) {
			SendPacketToZonesWithGMs(pack);
		} else {
			SendPacket(pack);
		}
	}
	else {
		ZoneServer *zs = FindByName(to);
		if (zs) {
			zs->SendPacket(pack);
		}
		else if (to_guilddbid > 0) {
			SendPacketToZonesWithGuild(to_guilddbid, pack);
		}
		else if (to_minstatus > 0) {
			SendPacketToZonesWithGMs(pack);
		}
		else {
			SendPacket(pack);
		}
	}
	delete pack;
}

void ZSList::SendTimeSync() {
	auto pack = new ServerPacket(ServerOP_SyncWorldTime, sizeof(eqTimeOfDay));
	eqTimeOfDay* tod = (eqTimeOfDay*)pack->pBuffer;
	tod->start_eqtime = worldclock.getStartEQTime();
	tod->start_realtime = worldclock.getStartRealTime();
	SendPacket(pack);
	delete pack;
}

void ZSList::NextGroupIDs(uint32 &start, uint32 &end) {
	start = CurGroupID;
	CurGroupID += 1000;	//hand them out 1000 at a time...
	if (CurGroupID < start) {	//handle overflow
		start = 1;
		CurGroupID = 1001;
	}
	end = CurGroupID - 1;
}

void ZSList::SOPZoneBootup(const char* adminname, uint32 ZoneServerID, const char* zonename, bool iMakeStatic) {
	ZoneServer* zs = 0;
	ZoneServer* zs2 = 0;
	uint32 zoneid;
	if (!(zoneid = ZoneID(zonename))) {
		SendEmoteMessage(
			adminname,
			0,
			AccountStatus::Player,
			Chat::White,
			fmt::format(
				"Error: SOP_ZoneBootup: Zone '{}' not found in 'zone' table.",
				zonename
			).c_str()
		);
	} else {
		if (ZoneServerID != 0) {
			zs = FindByID(ZoneServerID);
		} else {
			SendEmoteMessage(
				adminname,
				0,
				AccountStatus::Player,
				Chat::White,
				"Error: SOP_ZoneBootup: Server ID must be specified."
			);
		}

		if (!zs) {
			SendEmoteMessage(
				adminname,
				0,
				AccountStatus::Player,
				Chat::White,
				"Error: SOP_ZoneBootup: Zoneserver not found."
			);
		} else {
			zs2 = FindByName(zonename);
			if (zs2 != 0)
				SendEmoteMessage(
					adminname,
					0,
					AccountStatus::Player,
					Chat::White,
					fmt::format(
						"Error: SOP_ZoneBootup: Zone '{}' already being hosted by Zoneserver ID {}.",
						zonename,
						zs2->GetID()
					).c_str()
				);
			else {
				zs->TriggerBootup(zoneid, 0, adminname, iMakeStatic);
			}
		}
	}
}

void ZSList::RebootZone(const char* ip1, uint16 port, const char* ip2, uint32 skipid, uint32 zoneid) {
	// get random zone
	uint32 x = 0;
	auto iterator = zone_server_list.begin();
	while (iterator != zone_server_list.end()) {
		x++;
		iterator++;
	}
	if (x == 0)
		return;
	auto tmp = new ZoneServer *[x];
	uint32 y = 0;

	iterator = zone_server_list.begin();
	while (iterator != zone_server_list.end()) {
		if (!strcmp((*iterator)->GetCAddress(), ip2) && !(*iterator)->IsBootingUp() && (*iterator)->GetID() != skipid) {
			tmp[y++] = (*iterator).get();
		}
		iterator++;
	}
	if (y == 0) {
		safe_delete_array(tmp);
		return;
	}
	uint32 z = EQ::Random::Instance()->Int(0, y - 1);

	auto pack = new ServerPacket(ServerOP_ZoneReboot, sizeof(ServerZoneReboot_Struct));
	ServerZoneReboot_Struct* s = (ServerZoneReboot_Struct*)pack->pBuffer;
	//	strcpy(s->ip1,ip1);
	strcpy(s->ip2, ip2);
	s->port = port;
	s->zoneid = zoneid;
	if (zoneid != 0)
		LogInfo("Rebooting static zone with the ID of: [{}]", zoneid);
	tmp[z]->SendPacket(pack);
	delete pack;
	safe_delete_array(tmp);
}

uint16 ZSList::GetAvailableZonePort()
{
	if (m_ports_free.empty()) {
		return 0;
	}

	auto first = m_ports_free.front();
	m_ports_free.pop_front();
	return first;
}

uint32 ZSList::TriggerBootup(uint32 iZoneID, uint32 iInstanceID) {
	if (iInstanceID > 0)
	{
		auto iterator = zone_server_list.begin();
		while (iterator != zone_server_list.end()) {
			if ((*iterator)->GetInstanceID() == iInstanceID)
			{
				return (*iterator)->GetID();
			}
			iterator++;
		}

		iterator = zone_server_list.begin();
		while (iterator != zone_server_list.end()) {
			if ((*iterator)->GetZoneID() == 0 && !(*iterator)->IsBootingUp()) {
				ZoneServer* zone = (*iterator).get();
				zone->TriggerBootup(iZoneID, iInstanceID);
				return zone->GetID();
			}
			iterator++;
		}
		return 0;
	}
	else
	{
		auto iterator = zone_server_list.begin();
		while (iterator != zone_server_list.end()) {
			if ((*iterator)->GetZoneID() == iZoneID && (*iterator)->GetInstanceID() == 0)
			{
				return (*iterator)->GetID();
			}
			iterator++;
		}

		iterator = zone_server_list.begin();
		while (iterator != zone_server_list.end()) {
			if ((*iterator)->GetZoneID() == 0 && !(*iterator)->IsBootingUp()) {
				ZoneServer* zone = (*iterator).get();
				zone->TriggerBootup(iZoneID);
				return zone->GetID();
			}
			iterator++;
		}
		return 0;
	}
}

void ZSList::SendLSZones() {
	auto iterator = zone_server_list.begin();
	while (iterator != zone_server_list.end()) {
		ZoneServer* zs = (*iterator).get();
		zs->LSBootUpdate(zs->GetZoneID(), true);
		iterator++;
	}
}

int ZSList::GetZoneCount() {
	return(zone_server_list.size());
}

void ZSList::GetZoneIDList(std::vector<uint32> &zones) {
	auto iterator = zone_server_list.begin();
	while (iterator != zone_server_list.end()) {
		ZoneServer* zs = (*iterator).get();
		zones.push_back(zs->GetID());
		iterator++;
	}
}

void ZSList::UpdateUCSServerAvailable(bool ucss_available) {
	UCSServerAvailable_ = ucss_available;
	auto outapp = new ServerPacket(ServerOP_UCSServerStatusReply, sizeof(UCSServerStatus_Struct));
	auto ucsss = (UCSServerStatus_Struct*)outapp->pBuffer;
	ucsss->available = (ucss_available ? 1 : 0);
	ucsss->timestamp = Timer::GetCurrentTime();
	SendPacket(outapp);
	safe_delete(outapp);
}

void ZSList::WorldShutDown(uint32 time, uint32 interval)
{
	if (time > 0) {
		SendEmoteMessage(
			0,
			0,
			AccountStatus::Player,
			Chat::System,
			fmt::format(
				"[SYSTEM] World will be shutting down in {} minutes.",
				(time / 60)
			).c_str()
		);

		time *= 1000;
		interval *= 1000;
		if (interval < 5000) { interval = 5000; }

		shutdowntimer->SetTimer(time);
		reminder->SetTimer(interval - 1000);
		reminder->SetAtTrigger(interval);
		shutdowntimer->Start();
		reminder->Start();
	}
	else {
		SendEmoteMessage(
			0,
			0,
			AccountStatus::Player,
			Chat::Yellow,
			"[SYSTEM] World is shutting down."
		);
		auto pack = new ServerPacket;
		pack->opcode = ServerOP_ShutdownAll;
		pack->size = 0;
		SendPacket(pack);
		safe_delete(pack);
		Process();
		CatchSignal(2);
	}
}

void ZSList::DropClient(uint32 lsid, ZoneServer *ignore_zoneserver) {
	ServerPacket packet(ServerOP_DropClient, sizeof(ServerZoneDropClient_Struct));
	auto drop = (ServerZoneDropClient_Struct*)packet.pBuffer;
	drop->lsid = lsid;

	for (auto &zs : zone_server_list) {
		if (zs.get() != ignore_zoneserver) {
			zs->SendPacket(&packet);
		}
	}
}

void ZSList::OnTick(EQ::Timer *t)
{
	if (!EventSubscriptionWatcher::Get()->IsSubscribed("EQW::ZoneUpdate")) {
		return;
	}

	Json::Value out;
	out["event"] = "EQW::ZoneUpdate";
	out["data"] = Json::Value();

	for (auto &zone : zone_server_list)
	{
		Json::Value outzone;

		outzone["CAddress"] = zone->GetCAddress();
		outzone["CLocalAddress"] = zone->GetCLocalAddress();
		outzone["CompileTime"] = zone->GetCompileTime();
		outzone["CPort"] = zone->GetCPort();
		outzone["ID"] = zone->GetID();
		outzone["InstanceID"] = zone->GetInstanceID();
		outzone["IP"] = zone->GetIP();
		outzone["LaunchedName"] = zone->GetLaunchedName();
		outzone["LaunchName"] = zone->GetLaunchName();
		outzone["Port"] = zone->GetPort();
		outzone["PrevZoneID"] = zone->GetPrevZoneID();
		outzone["UUID"] = zone->GetUUID();
		outzone["ZoneID"] = zone->GetZoneID();
		outzone["ZoneLongName"] = zone->GetZoneLongName();
		outzone["ZoneName"] = zone->GetZoneName();
		outzone["ZoneOSProcessID"] = zone->GetZoneOSProcessID();
		outzone["NumPlayers"] = zone->NumPlayers();
		outzone["BootingUp"] = zone->IsBootingUp();
		outzone["StaticZone"] = zone->IsStaticZone();

		out["data"].append(outzone);
	}

	WebInterfaceList::Instance()->SendEvent(out);
}

const std::list<std::unique_ptr<ZoneServer>> &ZSList::getZoneServerList() const
{
	return zone_server_list;
}

bool ZSList::SendPacketToBootedZones(ServerPacket* pack)
{
	for (auto const& z : zone_server_list) {
		auto r = z.get();
		if (r && r->GetZoneID() > 0) {
			r->SendPacket(pack);
		}
	}

	return true;
}

bool ZSList::SendPacketToZonesWithGuild(uint32 guild_id, ServerPacket* pack)
{
	auto servers = ClientList::Instance()->GetGuildZoneServers(guild_id);
	for (auto const& z : zone_server_list) {
		for (auto const& server_id : servers) {
			if (z->GetID() == server_id && z->GetZoneID() > 0) {
				z->SendPacket(pack);
			}
		}
	}

	return true;
}

bool ZSList::SendPacketToZonesWithGMs(ServerPacket* pack)
{
	auto servers = ClientList::Instance()->GetZoneServersWithGMs();
	for (auto const &z: zone_server_list) {
		for (auto const &server_id: servers) {
			if (z->GetID() == server_id && z->GetZoneID() > 0) {
				z->SendPacket(pack);
			}
		}
	}

	return true;
}

void ZSList::SendServerReload(ServerReload::Type type, uchar *packet)
{
	static auto pack = ServerPacket(ServerOP_ServerReloadRequest, sizeof(ServerReload::Request));
	auto        r    = (ServerReload::Request *) pack.pBuffer;

	// Copy the packet data if it exists
	if (packet) {
		memcpy(pack.pBuffer, packet, sizeof(ServerReload::Request));
	}

	r->type                 = type;
	r->requires_zone_booted = true;

	LogInfo("Sending reload to all zones for type [{}]", ServerReload::GetName(type));

	static const std::unordered_set<ServerReload::Type> no_zone_boot_required = {
		ServerReload::Type::Opcodes,
		ServerReload::Type::Rules,
		ServerReload::Type::ContentFlags,
		ServerReload::Type::Logs,
		ServerReload::Type::Commands,
		ServerReload::Type::PerlExportSettings,
		ServerReload::Type::DataBucketsCache,
		ServerReload::Type::Quests,
		ServerReload::Type::QuestsTimerReset,
		ServerReload::Type::WorldRepop,
		ServerReload::Type::WorldWithRespawn
	};

	// Set requires_zone_booted flag before executing reload logic
	if (no_zone_boot_required.contains(type)) {
		r->requires_zone_booted = false;
	}

	// reload at the world level
	if (type == ServerReload::Type::Opcodes) {
		ReloadAllPatches();
	} else if (type == ServerReload::Type::Rules) {
		RuleManager::Instance()->LoadRules(&database, RuleManager::Instance()->GetActiveRuleset(), true);
	} else if (type == ServerReload::Type::SkillCaps) {
		SkillCaps::Instance()->ReloadSkillCaps();
	} else if (type == ServerReload::Type::ContentFlags) {
		WorldContentService::Instance()->SetExpansionContext()->ReloadContentFlags();
	} else if (type == ServerReload::Type::Logs) {
		EQEmuLogSys::Instance()->LoadLogDatabaseSettings();
		PlayerEventLogs::Instance()->ReloadSettings();
		UCSConnection::Instance()->SendPacket(&pack);
		QueryServConnection::Instance()->SendPacket(&pack);
	} else if (type == ServerReload::Type::Tasks) {
		SharedTaskManager::Instance()->LoadTaskData();
	} else if (type == ServerReload::Type::DzTemplates) {
		dynamic_zone_manager.LoadTemplates();
	}

	// Send the packet to all zones with staggered delays
	// to prevent all zones from reloading at the same time
	// and causing a massive spike in CPU usage
	// This is especially important for large servers
	// with many zones
	// we reload 10 zones every second
	int counter = 0;

	for (auto &z: zone_server_list) {
		bool is_local = r->zone_server_id != 0;

		// if the zone reload is local to a specific zone
		if (r->zone_server_id != 0 && r->zone_server_id != z->GetID()) {
			continue;
		}

		// if the reload is local, we don't need to stagger the reloads
		r->reload_at_unix = is_local ? 0 : (std::time(nullptr) + 1) + (counter / 10);
		z->SendPacket(&pack);
		++counter;
	}
}

void ZSList::QueueServerReload(ServerReload::Type &type)
{
	m_queued_reloads_mutex.lock();
	m_queued_reloads.emplace_back(type);
	m_queued_reloads_mutex.unlock();
}
