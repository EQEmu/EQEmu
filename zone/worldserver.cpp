/*	EQEMu: Everquest Server Emulator
Copyright (C) 2001-2016 EQEMu Development Team (http://eqemu.org)

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

#include "common/eq_packet_structs.h"
#include "common/events/player_event_logs.h"
#include "common/misc_functions.h"
#include "common/patches/patches.h"
#include "common/profanity_manager.h"
#include "common/repositories/guild_tributes_repository.h"
#include "common/rulesys.h"
#include "common/say_link.h"
#include "common/server_reload_types.h"
#include "common/servertalk.h"
#include "common/shared_tasks.h"
#include "common/skill_caps.h"
#include "zone/bot_command.h"
#include "zone/client.h"
#include "zone/command.h"
#include "zone/corpse.h"
#include "zone/dialogue_window.h"
#include "zone/dynamic_zone.h"
#include "zone/entity.h"
#include "zone/guild_mgr.h"
#include "zone/mob.h"
#include "zone/petitions.h"
#include "zone/queryserv.h"
#include "zone/quest_parser_collection.h"
#include "zone/raids.h"
#include "zone/shared_task_zone_messaging.h"
#include "zone/string_ids.h"
#include "zone/titles.h"
#include "zone/zone_config.h"
#include "zone/zone.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <iostream>

extern EntityList             entity_list;
extern Zone                  *zone;
extern volatile bool          is_zone_loaded;
extern WorldServer            worldserver;
extern uint32                 numclients;
extern volatile bool          RunLoops;
extern QuestParserCollection *parse;
extern QueryServ             *QServ;

void Shutdown();

// QuestParserCollection *parse = 0;

WorldServer::WorldServer()
{
	cur_groupid = 0;
	last_groupid = 0;
	oocmuted = false;
}

WorldServer::~WorldServer() {
}

void WorldServer::Process()
{
	if (!m_reload_queue.empty()) {
		m_reload_mutex.lock();
		for (auto it = m_reload_queue.begin(); it != m_reload_queue.end(); ) {
			if (it->second.reload_at_unix < std::time(nullptr)) {
				ProcessReload(it->second);
				it = m_reload_queue.erase(it);
				break;
			} else {
				++it;
			}
		}
		m_reload_mutex.unlock();
	}
}

void WorldServer::Connect()
{
	m_connection = std::make_unique<EQ::Net::ServertalkClient>(Config->WorldIP, Config->WorldTCPPort, false, "Zone", Config->SharedKey);
	m_connection->OnConnect([this](EQ::Net::ServertalkClient *client) {
		OnConnected();
	});

	m_connection->OnMessage(std::bind(&WorldServer::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
}

bool WorldServer::SendPacket(ServerPacket *pack)
{
	m_connection->SendPacket(pack);
	return true;
}

std::string WorldServer::GetIP() const
{
	return m_connection->Handle()->RemoteIP();
}

uint16 WorldServer::GetPort() const
{
	return m_connection->Handle()->RemotePort();
}

bool WorldServer::Connected() const
{
	return m_connection->Connected();
}

void WorldServer::SetZoneData(uint32 iZoneID, uint32 iInstanceID) {
	auto pack = new ServerPacket(ServerOP_SetZone, sizeof(SetZone_Struct));
	SetZone_Struct* szs = (SetZone_Struct*)pack->pBuffer;
	szs->zoneid = iZoneID;
	szs->instanceid = iInstanceID;
	if (zone) {
		szs->staticzone = zone->IsStaticZone();
	}
	SendPacket(pack);
	safe_delete(pack);
}

void WorldServer::OnConnected() {
	ServerPacket* pack;

	/* Tell the launcher what our information is */
	pack = new ServerPacket(ServerOP_SetLaunchName, sizeof(LaunchName_Struct));
	LaunchName_Struct* ln = (LaunchName_Struct*)pack->pBuffer;
	strn0cpy(ln->launcher_name, m_launcherName.c_str(), 32);
	strn0cpy(ln->zone_name, m_launchedName.c_str(), 16);
	SendPacket(pack);
	safe_delete(pack);

	/* Tell the Worldserver basic information about this zone process */
	pack = new ServerPacket(ServerOP_SetConnectInfo, sizeof(ServerConnectInfo));
	ServerConnectInfo* sci = (ServerConnectInfo*)pack->pBuffer;

	auto config = ZoneConfig::get();
	sci->port = ZoneConfig::get()->ZonePort;
	if (config->WorldAddress.length() > 0) {
		strn0cpy(sci->address, config->WorldAddress.c_str(), 250);
	}
	if (config->LocalAddress.length() > 0) {
		strn0cpy(sci->local_address, config->LocalAddress.c_str(), 250);
	}

	/* Fetch process ID */
	if (getpid()) {
		sci->process_id = getpid();
	}
	else {
		sci->process_id = 0;
	}

	SendPacket(pack);
	safe_delete(pack);

	if (is_zone_loaded) {
		SetZoneData(zone->GetZoneID(), zone->GetInstanceID());
		entity_list.UpdateWho(true);
		SendEmoteMessage(
			0,
			0,
			Chat::Yellow,
			fmt::format(
				"Zone Connected | {}",
				zone->GetZoneDescription()
			).c_str()
		);
		zone->GetTimeSync();
	}
	else {
		SetZoneData(0);
	}

	pack = new ServerPacket(ServerOP_LSZoneBoot, sizeof(ZoneBoot_Struct));
	ZoneBoot_Struct* zbs = (ZoneBoot_Struct*)pack->pBuffer;
	strcpy(zbs->compile_time, LAST_MODIFIED);
	SendPacket(pack);
	safe_delete(pack);
}

/* Zone Process Packets from World */
void WorldServer::HandleMessage(uint16 opcode, const EQ::Net::Packet &p)
{
	ServerPacket tpack(opcode, p);
	ServerPacket *pack = &tpack;

	switch (opcode) {
	case 0:
	case ServerOP_KeepAlive: {
		break;
	}
	case ServerOP_SetConnectInfo: {
		if (pack->size != sizeof(ServerConnectInfo))
			break;
		ServerConnectInfo* sci = (ServerConnectInfo*)pack->pBuffer;

		if (sci->port == 0) {
			LogError("World did not have a port to assign from this server, the port range was not large enough.");
			Shutdown();
		}
		else {
			LogInfo("World assigned Port [{}] for this zone", sci->port);
			ZoneConfig::SetZonePort(sci->port);

			EQEmuLogSys::Instance()->SetDiscordHandler(&Zone::DiscordWebhookMessageHandler);
		}
		break;
	}
	case ServerOP_ChannelMessage: {
		if (!is_zone_loaded)
			break;
		ServerChannelMessage_Struct* scm = (ServerChannelMessage_Struct*)pack->pBuffer;
		if (scm->deliverto[0] == 0) {
			entity_list.ChannelMessageFromWorld(scm->from, scm->to, scm->chan_num, scm->guilddbid, scm->language, scm->lang_skill, scm->message);
		}
		else {
			Client* client = entity_list.GetClientByName(scm->deliverto);
			if (client && client->Connected()) {
				if (scm->chan_num == ChatChannel_TellEcho) {
					if (scm->queued == 1) // tell was queued
						client->Tell_StringID(QUEUED_TELL, scm->to, scm->message);
					else if (scm->queued == 2) // tell queue was full
						client->Tell_StringID(QUEUE_TELL_FULL, scm->to, scm->message);
					else if (scm->queued == 3) // person was offline
						client->MessageString(Chat::EchoTell, TOLD_NOT_ONLINE, scm->to);
					else // normal tell echo "You told Soanso, 'something'"
							// tell echo doesn't use language, so it looks normal to you even if nobody can understand your tells
						client->ChannelMessageSend(scm->from, scm->to, scm->chan_num, Language::CommonTongue, Language::MaxValue, scm->message);
				}
				else if (scm->chan_num == ChatChannel_Tell) {
					client->ChannelMessageSend(scm->from, scm->to, scm->chan_num, scm->language, scm->lang_skill, scm->message);
					if (scm->queued == 0) { // this is not a queued tell
						// if it's a tell, echo back to acknowledge it and make it show on the sender's client
						scm->chan_num = ChatChannel_TellEcho;
						memset(scm->deliverto, 0, sizeof(scm->deliverto));
						strcpy(scm->deliverto, scm->from);
						SendPacket(pack);
					}
				}
				else {
					client->ChannelMessageSend(scm->from, scm->to, scm->chan_num, scm->language, scm->lang_skill, scm->message);
				}
			}
		}
		break;
	}
	case ServerOP_VoiceMacro: {
		if (!is_zone_loaded)
			break;

		ServerVoiceMacro_Struct* svm = (ServerVoiceMacro_Struct*)pack->pBuffer;

		auto outapp = new EQApplicationPacket(OP_VoiceMacroOut, sizeof(VoiceMacroOut_Struct));
		VoiceMacroOut_Struct* vmo = (VoiceMacroOut_Struct*)outapp->pBuffer;

		strcpy(vmo->From, svm->From);
		vmo->Type = svm->Type;
		vmo->Voice = svm->Voice;
		vmo->MacroNumber = svm->MacroNumber;

		switch (svm->Type) {
			case VoiceMacroTell: {
				Client* c = entity_list.GetClientByName(svm->To);
				if (!c)
					break;

				c->QueuePacket(outapp);
				break;
			}

			case VoiceMacroGroup: {
				Group* g = entity_list.GetGroupByID(svm->GroupID);

				if (!g)
					break;

				for (unsigned int i = 0; i < MAX_GROUP_MEMBERS; i++) {
					if (g->members[i] && g->members[i]->IsClient())
						g->members[i]->CastToClient()->QueuePacket(outapp);

				}
				break;
			}

			case VoiceMacroRaid: {
				Raid *r = entity_list.GetRaidByID(svm->RaidID);

				if (!r) {
					break;
				}

				for (const auto& m: r->members) {
					if (m.is_bot) {
						continue;
					}

					if (m.member) {
						m.member->QueuePacket(outapp);
					}
				}

				break;
			}
		}
		safe_delete(outapp);
		break;
	}
	case ServerOP_SpawnCondition: {
		if (pack->size != sizeof(ServerSpawnCondition_Struct))
			break;
		if (!is_zone_loaded)
			break;
		ServerSpawnCondition_Struct* ssc = (ServerSpawnCondition_Struct*)pack->pBuffer;

		zone->spawn_conditions.SetCondition(zone->GetShortName(), zone->GetInstanceID(), ssc->condition_id, ssc->value, true);
		break;
	}
	case ServerOP_SpawnEvent: {
		if (pack->size != sizeof(ServerSpawnEvent_Struct))
			break;
		if (!is_zone_loaded)
			break;
		ServerSpawnEvent_Struct* sse = (ServerSpawnEvent_Struct*)pack->pBuffer;

		zone->spawn_conditions.ReloadEvent(sse->event_id);

		break;
	}
	case ServerOP_AcceptWorldEntrance: {
		if (pack->size != sizeof(WorldToZone_Struct))
			break;
		if (!is_zone_loaded)
			break;
		WorldToZone_Struct* wtz = (WorldToZone_Struct*)pack->pBuffer;

		if (zone->GetMaxClients() != 0 && numclients >= zone->GetMaxClients())
			wtz->response = -1;
		else
			wtz->response = 1;

		SendPacket(pack);
		break;
	}
	case ServerOP_ZoneToZoneRequest: {
		if (pack->size != sizeof(ZoneToZone_Struct))
			break;
		if (!is_zone_loaded)
			break;
		ZoneToZone_Struct* ztz = (ZoneToZone_Struct*)pack->pBuffer;

		LogZoning(
			"ZoneToZone client [{}] guild_id [{}] requested_zone [{}] requested_zone_id [{}] requested_instance_id [{}] current_zone [{}] current_zone_id [{}] current_instance_id [{}] response [{}] admin [{}] ignorerestrictions [{}]",
			ztz->name,
			ztz->guild_id,
			ZoneName(ztz->requested_zone_id),
			ztz->requested_zone_id,
			ztz->requested_instance_id,
			ZoneName(ztz->current_zone_id),
			ztz->current_zone_id,
			ztz->current_instance_id,
			ztz->response,
			ztz->admin,
			ztz->ignorerestrictions
		);

		// the client was rejected by the world server
		// zone was not ready for some reason
		if (ztz->current_zone_id == zone->GetZoneID() && ztz->current_instance_id == zone->GetInstanceID()) {
			// it's a response
			Entity* entity = entity_list.GetClientByName(ztz->name);
			if (entity == 0)
				break;

			EQApplicationPacket *outapp;
			outapp = new EQApplicationPacket(OP_ZoneChange, sizeof(ZoneChange_Struct));
			ZoneChange_Struct* zc2 = (ZoneChange_Struct*)outapp->pBuffer;

			if (ztz->response <= 0) {
				zc2->success = ZoningMessage::ZoneNotReady;
				entity->CastToMob()->SetZone(ztz->current_zone_id, ztz->current_instance_id);
				entity->CastToClient()->SetZoning(false);
				entity->CastToClient()->SetLockSavePosition(false);

				LogZoning("ZoneToZone (ZoneNotReady) client [{}] guild_id [{}] requested_zone [{}] requested_zone_id [{}] requested_instance_id [{}] current_zone [{}] current_zone_id [{}] current_instance_id [{}] response [{}] admin [{}] ignorerestrictions [{}] ",
					ztz->name,
					ztz->guild_id,
					ZoneName(ztz->requested_zone_id),
					ztz->requested_zone_id,
					ztz->requested_instance_id,
					ZoneName(ztz->current_zone_id),
					ztz->current_zone_id,
					ztz->current_instance_id,
					ztz->response,
					ztz->admin,
					ztz->ignorerestrictions
				);
			}
			else {
				entity->CastToClient()->UpdateWho(1);
				strn0cpy(zc2->char_name, entity->CastToMob()->GetName(), 64);
				zc2->zoneID = ztz->requested_zone_id;
				zc2->instanceID = ztz->requested_instance_id;
				zc2->success = 1;

				// This block is necessary to clean up any merc objects owned by a Client. Maybe we should do this for bots, too?
				if (entity->CastToClient()->GetMerc() != nullptr)
				{
					entity->CastToClient()->GetMerc()->ProcessClientZoneChange(entity->CastToClient());
				}

				entity->CastToMob()->SetZone(ztz->requested_zone_id, ztz->requested_instance_id);

				if (ztz->ignorerestrictions == 3)
					entity->CastToClient()->GoToSafeCoords(ztz->requested_zone_id, ztz->requested_instance_id);
			}

			outapp->priority = 6;
			entity->CastToClient()->QueuePacket(outapp);
			safe_delete(outapp);

			switch (ztz->response)
			{
			case -2: {
				entity->CastToClient()->Message(Chat::Red, "You do not own the required locations to enter this zone.");
				break;
			}
			case -1: {
				entity->CastToClient()->Message(Chat::Red, "The zone is currently full, please try again later.");
				break;
			}
			case 0: {
				entity->CastToClient()->Message(Chat::Red, "All zone servers are taken at this time, please try again later.");
				break;
			}
			}
		}
		// the client was accepted by the world server
		else {
			// it's a request
			ztz->response = 0;

			if (zone->GetMaxClients() != 0 && numclients >= zone->GetMaxClients())
				ztz->response = -1;
			else {
				ztz->response = 1;
			}

			SendPacket(pack);
			break;
		}
		break;
	}
	case ServerOP_WhoAllReply: {
		if (!is_zone_loaded)
			break;


		WhoAllReturnStruct* wars = (WhoAllReturnStruct*)pack->pBuffer;
		if (wars && wars->id != 0 && wars->id<0xFFFFFFFF) {
			Client* client = entity_list.GetClientByID(wars->id);
			if (client) {
				if (pack->size == 64)//no results
					client->MessageString(Chat::White, WHOALL_NO_RESULTS);
				else {
					auto outapp = new EQApplicationPacket(OP_WhoAllResponse, pack->size);
					memcpy(outapp->pBuffer, pack->pBuffer, pack->size);
					client->QueuePacket(outapp);
					safe_delete(outapp);
				}
			}
			else {
				LogDebug("[CLIENT] id=[{}], playerineqstring=[{}], playersinzonestring=[{}]. Dumping WhoAllReturnStruct:",
					wars->id, wars->playerineqstring, wars->playersinzonestring);
			}
		}
		else
			LogError("WhoAllReturnStruct: Could not get return struct!");
		break;
	}
	case ServerOP_EmoteMessage: {
		if (!is_zone_loaded)
			break;
		ServerEmoteMessage_Struct* sem = (ServerEmoteMessage_Struct*)pack->pBuffer;
		if (sem->to[0] != 0) {
			if (strcasecmp(sem->to, zone->GetShortName()) == 0)
				entity_list.MessageStatus(
					sem->guilddbid,
					sem->minstatus,
					sem->type,
					(char*)sem->message
				);
			else {
				Client* client = entity_list.GetClientByName(sem->to);
				if (client) {
					char* newmessage = 0;
					if (strstr(sem->message, "^") == 0)
						client->Message(sem->type, (char*)sem->message);
					else {
						for (newmessage = strtok((char*)sem->message, "^"); newmessage != nullptr; newmessage = strtok(nullptr, "^"))
							client->Message(sem->type, newmessage);
					}
				}
			}
		}
		else {
			char* newmessage = 0;
			if (strstr(sem->message, "^") == 0) {
				entity_list.MessageStatus(
					sem->guilddbid,
					sem->minstatus,
					sem->type,
					sem->message
				);
			} else {
				for (newmessage = strtok((char*)sem->message, "^"); newmessage != nullptr; newmessage = strtok(nullptr, "^")) {
					entity_list.MessageStatus(
						sem->guilddbid,
						sem->minstatus,
						sem->type,
						newmessage
					);
				}
			}
		}
		break;
	}
	case ServerOP_Motd: {
		if (pack->size != sizeof(ServerMotd_Struct))
			break;

		ServerMotd_Struct *smotd = (ServerMotd_Struct *)pack->pBuffer;
		SerializeBuffer buf(100);
		buf.WriteString(smotd->motd);

		auto outapp = std::make_unique<EQApplicationPacket>(OP_MOTD, buf);

		entity_list.QueueClients(0, outapp.get());
		break;
	}
	case ServerOP_ShutdownAll: {
		entity_list.Save();
		Shutdown();
		break;
	}
	case ServerOP_ZoneShutdown: {
		if (pack->size != sizeof(ServerZoneStateChange_Struct)) {
			LogError("Wrong size on ServerOP_ZoneShutdown. Got: [{}] Expected: [{}]", pack->size, sizeof(ServerZoneStateChange_Struct));
			break;
		}

		if (!is_zone_loaded) {
			SetZoneData(0);
		} else {
			SendEmoteMessage(
				0,
				0,
				Chat::Yellow,
				fmt::format(
					"Zone Shutdown | {}",
					zone->GetZoneDescription()
				).c_str()
			);

			auto *s = (ServerZoneStateChange_Struct *) pack->pBuffer;
			LogInfo("Zone shutdown by {}.", s->admin_name);
			zone->Shutdown();
		}
		break;
	}
	case ServerOP_ZoneBootup: {
		if (pack->size != sizeof(ServerZoneStateChange_Struct)) {
			LogError("Wrong size on ServerOP_ZoneShutdown. Got: [{}] Expected: [{}]", pack->size, sizeof(ServerZoneStateChange_Struct));
			break;
		}

		auto *s = (ServerZoneStateChange_Struct *) pack->pBuffer;
		if (is_zone_loaded) {
			SetZoneData(zone->GetZoneID(), zone->GetInstanceID());
			if (s->zone_id != zone->GetZoneID()) {
				SendEmoteMessage(
					s->admin_name,
					0,
					Chat::White,
					fmt::format(
						"Zone Bootup Failed | {} Already running",
						zone->GetZoneDescription()
					).c_str()
				);
			}
			break;
		}

		if (s->admin_name[0] != 0) {
			LogInfo("Zone bootup by {}.", s->admin_name);
		}

		Zone::Bootup(s->zone_id, s->instance_id, s->is_static);
		if (zone) {
			zone->SetZoneServerId(s->zone_server_id);
		}

		break;
	}
	case ServerOP_ZoneIncClient: {
		if (pack->size != sizeof(ServerZoneIncomingClient_Struct)) {
			std::cout << "Wrong size on ServerOP_ZoneIncClient. Got: " << pack->size << ", Expected: " << sizeof(ServerZoneIncomingClient_Struct) << std::endl;
			break;
		}
		ServerZoneIncomingClient_Struct* szic = (ServerZoneIncomingClient_Struct*)pack->pBuffer;
		if (is_zone_loaded) {
			SetZoneData(zone->GetZoneID(), zone->GetInstanceID());

			if (szic->zoneid == zone->GetZoneID()) {
				auto client = entity_list.GetClientByLSID(szic->lsid);
				if (client) {
					client->Kick("Dropped by world CLE subsystem");
					client->Save();
				}

				zone->RemoveAuth(szic->lsid);
				zone->AddAuth(szic);
			}
		}
		else {
			if ((Zone::Bootup(szic->zoneid, szic->instanceid))) {
				zone->AddAuth(szic);
			}
		}
		break;
	}
	case ServerOP_DropClient: {
		if (pack->size != sizeof(ServerZoneDropClient_Struct)) {
			break;
		}

		ServerZoneDropClient_Struct* drop = (ServerZoneDropClient_Struct*)pack->pBuffer;
		if (zone) {
			zone->RemoveAuth(drop->lsid);

			auto client = entity_list.GetClientByLSID(drop->lsid);
			if (client) {
				client->Kick("Dropped by world CLE subsystem");
				client->Save();
			}
		}
		break;
	}
	case ServerOP_ZonePlayer: {
		ServerZonePlayer_Struct* szp = (ServerZonePlayer_Struct*)pack->pBuffer;
		auto client = entity_list.GetClientByName(szp->name);
		if (client) {
			if (!strcasecmp(szp->adminname, szp->name)) {
				client->Message(
					Chat::White,
					fmt::format(
						"Zoning to {} ({}).",
						ZoneLongName(
							ZoneID(szp->zone)
						),
						ZoneID(szp->zone)
					).c_str()
				);
			} else if (client->GetAnon() == 1 && client->Admin() > szp->adminrank) {
				break;
			} else {
				std::string name = Strings::ToLower(szp->name);
				name[0] = toupper(name[0]);

				SendEmoteMessage(
					szp->adminname,
					0,
					Chat::White,
					fmt::format(
						"Summoning {} to {:.2f}, {:.2f}, {:.2f} in {} ({}).",
						name,
						szp->x_pos,
						szp->y_pos,
						szp->z_pos,
						ZoneLongName(
							ZoneID(szp->zone)
						),
						ZoneID(szp->zone)
					).c_str()
				);
			}

			if (!szp->instance_id) {
				client->MovePC(ZoneID(szp->zone), szp->instance_id, szp->x_pos, szp->y_pos, szp->z_pos, client->GetHeading(), szp->ignorerestrictions, GMSummon);
			} else {
				if (database.GetInstanceID(client->CharacterID(), ZoneID(szp->zone))) {
					client->RemoveFromInstance(database.GetInstanceID(client->CharacterID(), ZoneID(szp->zone)));
				}

				client->AssignToInstance(szp->instance_id);
				client->MovePC(ZoneID(szp->zone), szp->instance_id, szp->x_pos, szp->y_pos, szp->z_pos, client->GetHeading(), szp->ignorerestrictions, GMSummon);
			}
		}
		break;
	}
	case ServerOP_KickPlayer: {
		ServerKickPlayer_Struct* skp = (ServerKickPlayer_Struct*)pack->pBuffer;
		Client* client;
		if (strlen(skp->name)) {
			client = entity_list.GetClientByName(skp->name);
		} else if (skp->account_id) {
			client = entity_list.GetClientByAccID(skp->account_id);
		}

		if (client) {
			if (skp->adminrank >= client->Admin()) {
				client->WorldKick();
				SendEmoteMessage(
					skp->adminname,
					0,
					Chat::White,
					fmt::format(
						"Remote Kick | {} booted{}.",
						skp->name,
						is_zone_loaded ?
						fmt::format(
							" in {}",
							zone->GetZoneDescription()
						) :
						""
					).c_str()
				);
			} else if (client->GetAnon() != 1) {
				SendEmoteMessage(
					skp->adminname,
					0,
					Chat::White,
					fmt::format(
						"Remote Kick | Your Status Level is not high enough to kick {}.",
						skp->name
					).c_str()
				);
			}
		}
		break;
	}
	case ServerOP_KillPlayer: {
		ServerKillPlayer_Struct* skp = (ServerKillPlayer_Struct*)pack->pBuffer;
		Client* client = entity_list.GetClientByName(skp->target);
		if (client) {
			if (skp->admin >= client->Admin()) {
				client->GMKill();
				SendEmoteMessage(
					skp->gmname,
					0,
					Chat::White,
					fmt::format(
						"Remote Kill | {} killed{}",
						skp->target,
						is_zone_loaded ?
						fmt::format(
							"in {}",
							zone->GetZoneDescription()
						) :
						""
					).c_str()
				);
			} else if (client->GetAnon() != 1) {
				SendEmoteMessage(
					skp->gmname,
					0,
					Chat::White,
					fmt::format(
						"Remote Kill | Your Status Level is not high enough to kill {}",
						skp->target
					).c_str()
				);
			}
		}
		break;
	}
	case ServerOP_OnlineGuildMembersResponse:
	case ServerOP_RefreshGuild:
	case ServerOP_DeleteGuild:
	case ServerOP_GuildCharRefresh:
	case ServerOP_GuildMemberUpdate:
	case ServerOP_GuildPermissionUpdate:
	case ServerOP_GuildRankUpdate:
	case ServerOP_GuildRankNameChange:
	case ServerOP_LFGuildUpdate:
	case ServerOP_GuildMemberLevelUpdate:
	case ServerOP_GuildMemberPublicNote:
	case ServerOP_GuildChannel:
	case ServerOP_GuildURL:
	case ServerOP_GuildMemberRemove:
	case ServerOP_GuildMemberAdd:
	case ServerOP_GuildSendGuildList:
	case ServerOP_GuildMembersList:
	{
		guild_mgr.ProcessWorldPacket(pack);
		break;
	}
	case ServerOP_FlagUpdate: {
		Client* client = entity_list.GetClientByAccID(*((uint32*)pack->pBuffer));
		if (client) {
			client->UpdateAdmin();
		}
		break;
	}
	case ServerOP_GMGoto: {
		if (pack->size != sizeof(ServerGMGoto_Struct)) {
			std::cout << "Wrong size on ServerOP_GMGoto. Got: " << pack->size << ", Expected: " << sizeof(ServerGMGoto_Struct) << std::endl;
			break;
		}
		if (!is_zone_loaded)
			break;
		ServerGMGoto_Struct* gmg = (ServerGMGoto_Struct*)pack->pBuffer;
		Client* client = entity_list.GetClientByName(gmg->gotoname);
		if (client) {
			SendEmoteMessage(
				gmg->myname,
				0,
				Chat::Red,
				fmt::format(
					"Summoning you to {} in {} at {:.2f}, {:.2f}, {:.2f}",
					client->GetCleanName(),
					zone->GetZoneDescription(),
					client->GetX(),
					client->GetY(),
					client->GetZ()
				).c_str()
			);
			auto outpack = new ServerPacket(ServerOP_ZonePlayer, sizeof(ServerZonePlayer_Struct));
			ServerZonePlayer_Struct* szp = (ServerZonePlayer_Struct*)outpack->pBuffer;
			strcpy(szp->adminname, gmg->myname);
			strcpy(szp->name, gmg->myname);
			strcpy(szp->zone, zone->GetShortName());
			szp->instance_id = zone->GetInstanceID();
			szp->x_pos = client->GetX();
			szp->y_pos = client->GetY();
			szp->z_pos = client->GetZ();
			SendPacket(outpack);
			safe_delete(outpack);
		}
		else {
			SendEmoteMessage(
				gmg->myname,
				0,
				Chat::Red,
				fmt::format(
					"Error: {} not found.",
					gmg->gotoname
				).c_str()
			);
		}
		break;
	}
	case ServerOP_MultiLineMsg: {
		ServerMultiLineMsg_Struct* mlm = (ServerMultiLineMsg_Struct*)pack->pBuffer;
		Client* client = entity_list.GetClientByName(mlm->to);
		if (client) {
			auto outapp = new EQApplicationPacket(OP_MultiLineMsg, strlen(mlm->message));
			strcpy((char*)outapp->pBuffer, mlm->message);
			client->QueuePacket(outapp);
			safe_delete(outapp);
		}
		break;
	}
	case ServerOP_Uptime: {
		if (pack->size != sizeof(ServerUptime_Struct)) {
			std::cout << "Wrong size on ServerOP_Uptime. Got: " << pack->size << ", Expected: " << sizeof(ServerUptime_Struct) << std::endl;
			break;
		}
		ServerUptime_Struct* sus = (ServerUptime_Struct*)pack->pBuffer;
		uint32 ms = Timer::GetCurrentTime();
		std::string time_string = Strings::MillisecondsToTime(ms);
		SendEmoteMessage(
			sus->adminname,
			0,
			Chat::White,
			fmt::format(
				"Zoneserver {} | Uptime: {}",
				sus->zoneserverid,
				time_string
			).c_str()
		);
	}
	case ServerOP_Petition: {
		std::cout << "Got Server Requested Petition List Refresh" << std::endl;
		ServerPetitionUpdate_Struct* sus = (ServerPetitionUpdate_Struct*)pack->pBuffer;
		// this was typoed to = instead of ==, not that it acts any different now though..
		if (sus->status == 0) PetitionList::Instance()->ReadDatabase();
		else if (sus->status == 1) PetitionList::Instance()->ReadDatabase(); // Until I fix this to be better....
		break;
	}
	case ServerOP_RezzPlayer: {
		RezzPlayer_Struct* srs = (RezzPlayer_Struct*)pack->pBuffer;
		if (srs->rezzopcode == OP_RezzRequest)
		{
			// The Rezz request has arrived in the zone the player to be rezzed is currently in,
			// so we send the request to their client which will bring up the confirmation box.
			Client* client = entity_list.GetClientByName(srs->rez.your_name);
			if (client)
			{
				if (client->IsRezzPending())
				{
					auto Response = new ServerPacket(ServerOP_RezzPlayerReject,
						strlen(srs->rez.rezzer_name) + 1);

					char *Buffer = (char *)Response->pBuffer;
					sprintf(Buffer, "%s", srs->rez.rezzer_name);
					worldserver.SendPacket(Response);
					safe_delete(Response);
					break;
				}
				//pendingrezexp is the amount of XP on the corpse. Setting it to a value >= 0
				//also serves to inform Client::OPRezzAnswer to expect a packet.
				client->SetPendingRezzData(srs->exp, srs->dbid, srs->rez.spellid, srs->rez.corpse_name);
				LogSpells("[WorldServer::HandleMessage] OP_RezzRequest in zone [{}] for [{}] spellid [{}]",
					zone->GetShortName(), client->GetName(), srs->rez.spellid);
				auto outapp = new EQApplicationPacket(OP_RezzRequest,
					sizeof(Resurrect_Struct));
				memcpy(outapp->pBuffer, &srs->rez, sizeof(Resurrect_Struct));
				client->QueuePacket(outapp);
				safe_delete(outapp);
				break;
			}
		}
		if (srs->rezzopcode == OP_RezzComplete) {
			// We get here when the Rezz complete packet has come back via the world server
			// to the zone that the corpse is in.
			Corpse* corpse = entity_list.GetCorpseByName(srs->rez.corpse_name);
			if (corpse && corpse->IsCorpse()) {
				LogSpells("[WorldServer::HandleMessage] OP_RezzComplete received in zone [{}] for corpse [{}]",
					zone->GetShortName(), srs->rez.corpse_name);

				LogSpells("[WorldServer::HandleMessage] Found corpse. Marking corpse as rezzed if needed");
				// I don't know why Rezzed is not set to true in CompleteRezz().
				if (!IsEffectInSpell(srs->rez.spellid, SpellEffect::SummonToCorpse)) {
					corpse->IsRezzed(true);
					corpse->CompleteResurrection();
				}
			}
		}

		break;
	}
	case ServerOP_RezzPlayerReject:
	{
		char *Rezzer = (char *)pack->pBuffer;

		Client* c = entity_list.GetClientByName(Rezzer);

		if (c)
			c->MessageString(Chat::SpellWornOff, REZZ_ALREADY_PENDING);

		break;
	}
	case ServerOP_ZoneReboot: {
		std::cout << "Got Server Requested Zone reboot" << std::endl;
		ServerZoneReboot_Struct* zb = (ServerZoneReboot_Struct*)pack->pBuffer;
		break;
	}
	case ServerOP_SyncWorldTime: {
		if (zone != 0 && !zone->is_zone_time_localized) {
			LogInfo("[{}] Received Message SyncWorldTime", __FUNCTION__);

			eqTimeOfDay* newtime = (eqTimeOfDay*)pack->pBuffer;
			zone->zone_time.SetCurrentEQTimeOfDay(newtime->start_eqtime, newtime->start_realtime);
			auto outapp = new EQApplicationPacket(OP_TimeOfDay, sizeof(TimeOfDay_Struct));
			TimeOfDay_Struct* time_of_day = (TimeOfDay_Struct*)outapp->pBuffer;
			zone->zone_time.GetCurrentEQTimeOfDay(time(0), time_of_day);
			entity_list.QueueClients(0, outapp, false);
			safe_delete(outapp);

			char time_message[255];
			time_t current_time = time(nullptr);
			TimeOfDay_Struct eq_time{};
			zone->zone_time.GetCurrentEQTimeOfDay(current_time, &eq_time);

			sprintf(time_message, "EQTime [%02d:%s%d %s]",
				((eq_time.hour - 1) % 12) == 0 ? 12 : ((eq_time.hour - 1) % 12),
				(eq_time.minute < 10) ? "0" : "",
				eq_time.minute,
				(eq_time.hour >= 13) ? "pm" : "am"
				);

			LogInfo("Time Broadcast Packet: {}", time_message);
			zone->SetZoneHasCurrentTime(true);

		}
		if (zone && zone->is_zone_time_localized) {
			LogInfo("Received request to sync time from world, but our time is localized currently");
		}
		break;
	}
	case ServerOP_RefreshCensorship: {
		if (!EQ::ProfanityManager::LoadProfanityList(&database))
			LogError("Received request to refresh the profanity list..but, the action failed");
		break;
	}
	case ServerOP_ChangeWID: {
		if (pack->size != sizeof(ServerChangeWID_Struct)) {
			std::cout << "Wrong size on ServerChangeWID_Struct. Got: " << pack->size << ", Expected: " << sizeof(ServerChangeWID_Struct) << std::endl;
			break;
		}
		ServerChangeWID_Struct* scw = (ServerChangeWID_Struct*)pack->pBuffer;
		Client* client = entity_list.GetClientByCharID(scw->charid);
		if (client)
			client->SetWID(scw->newwid);
		break;
	}
	case ServerOP_OOCMute: {
		auto o = (ServerOOCMute_Struct *) pack->pBuffer;
		oocmuted = o->is_muted;
		break;
	}
	case ServerOP_Revoke: {
		RevokeStruct* rev = (RevokeStruct*)pack->pBuffer;
		Client* client = entity_list.GetClientByName(rev->name);
		if (client) {
			SendEmoteMessage(
				rev->adminname,
				0,
				Chat::White,
				fmt::format(
					"Zone {} | {} {}.",
					zone->GetZoneDescription(),
					rev->toggle ? "Revoking" : "Unrevoking",
					client->GetCleanName()
				).c_str()
			);
			client->SetRevoked(rev->toggle);
		}
		break;
	}
	case ServerOP_GroupIDReply: {
		ServerGroupIDReply_Struct* ids = (ServerGroupIDReply_Struct*)pack->pBuffer;
		cur_groupid = ids->start;
		last_groupid = ids->end;
		break;
	}
	case ServerOP_GroupLeave: {
		ServerGroupLeave_Struct* gl = (ServerGroupLeave_Struct*)pack->pBuffer;
		if (zone) {
			if (gl->zoneid == zone->GetZoneID() && gl->instance_id == zone->GetInstanceID())
				break;

			entity_list.SendGroupLeave(gl->gid, gl->member_name);
		}
		break;
	}
	case ServerOP_GroupInvite: {
		// A player in another zone invited a player in this zone to join their group.
		GroupInvite_Struct* gis = (GroupInvite_Struct*)pack->pBuffer;

		Mob *Invitee = entity_list.GetMob(gis->invitee_name);

		if (Invitee && Invitee->IsClient() && Invitee->CastToClient()->MercOnlyOrNoGroup() && !Invitee->IsRaidGrouped())
		{
			auto outapp = new EQApplicationPacket(OP_GroupInvite, sizeof(GroupInvite_Struct));
			memcpy(outapp->pBuffer, gis, sizeof(GroupInvite_Struct));
			Invitee->CastToClient()->QueuePacket(outapp);
			safe_delete(outapp);
		}

		break;
	}
	case ServerOP_GroupFollow: {
		// Player in another zone accepted a group invitation from a player in this zone.
		ServerGroupFollow_Struct* sgfs = (ServerGroupFollow_Struct*)pack->pBuffer;

		Mob* Inviter = entity_list.GetClientByName(sgfs->gf.name1);

		if (Inviter && Inviter->IsClient())
		{
			Group* group = entity_list.GetGroupByClient(Inviter->CastToClient());

			if (!group)
			{
				//Make new group
				group = new Group(Inviter);

				if (!group)
				{
					break;
				}

				entity_list.AddGroup(group);

				if (group->GetID() == 0) {
					Inviter->Message(Chat::Red, "Unable to get new group id. Cannot create group.");
					break;
				}

				group->AddToGroup(Inviter);
				database.SetGroupLeaderName(group->GetID(), Inviter->GetName());
				group->UpdateGroupAAs();

				if (Inviter->CastToClient()->ClientVersion() < EQ::versions::ClientVersion::SoD)
				{
					auto outapp =
						new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
					auto outgj = (GroupJoin_Struct*)outapp->pBuffer;
					strcpy(outgj->membername, Inviter->GetName());
					strcpy(outgj->yourname, Inviter->GetName());
					outgj->action = groupActInviteInitial; // 'You have formed the group'.
					group->GetGroupAAs(&outgj->leader_aas);
					Inviter->CastToClient()->QueuePacket(outapp);
					safe_delete(outapp);
				}
				else
				{
					// SoD and later
					Inviter->CastToClient()->SendGroupCreatePacket();
					Inviter->CastToClient()->SendGroupLeaderChangePacket(Inviter->GetName());
					Inviter->CastToClient()->SendGroupJoinAcknowledge();
				}

				group->GetXTargetAutoMgr()->merge(*Inviter->CastToClient()->GetXTargetAutoMgr());
				Inviter->CastToClient()->GetXTargetAutoMgr()->clear();
				Inviter->CastToClient()->SetXTargetAutoMgr(group->GetXTargetAutoMgr());
			}

			if (!group)
			{
				break;
			}

			auto outapp = new EQApplicationPacket(OP_GroupFollow, sizeof(GroupGeneric_Struct));
			GroupGeneric_Struct *gg = (GroupGeneric_Struct *)outapp->pBuffer;
			strn0cpy(gg->name1, sgfs->gf.name1, sizeof(gg->name1));
			strn0cpy(gg->name2, sgfs->gf.name2, sizeof(gg->name2));
			Inviter->CastToClient()->QueuePacket(outapp);
			safe_delete(outapp);

			if (!group->AddMember(nullptr, sgfs->gf.name2, sgfs->CharacterID))
				break;

			if (Inviter->CastToClient()->IsLFP())
				Inviter->CastToClient()->UpdateLFP();

			auto pack2 = new ServerPacket(ServerOP_GroupJoin, sizeof(ServerGroupJoin_Struct));
			auto gj = (ServerGroupJoin_Struct*)pack2->pBuffer;
			gj->gid = group->GetID();
			gj->zoneid = zone->GetZoneID();
			gj->instance_id = zone->GetInstanceID();
			strn0cpy(gj->member_name, sgfs->gf.name2, sizeof(gj->member_name));
			worldserver.SendPacket(pack2);
			safe_delete(pack2);



			// Send acknowledgement back to the Invitee to let them know we have added them to the group.
			auto pack3 =
				new ServerPacket(ServerOP_GroupFollowAck, sizeof(ServerGroupFollowAck_Struct));
			ServerGroupFollowAck_Struct* sgfas = (ServerGroupFollowAck_Struct*)pack3->pBuffer;
			strn0cpy(sgfas->Name, sgfs->gf.name2, sizeof(sgfas->Name));
			worldserver.SendPacket(pack3);
			safe_delete(pack3);
		}
		break;
	}
	case ServerOP_GroupFollowAck: {
		// The Inviter (in another zone) has successfully added the Invitee (in this zone) to the group.
		ServerGroupFollowAck_Struct* sgfas = (ServerGroupFollowAck_Struct*)pack->pBuffer;

		Client *client = entity_list.GetClientByName(sgfas->Name);

		if (!client) {
			break;
		}

		uint32 groupid = database.GetGroupID(client->GetName());

		Group* group = nullptr;

		if (groupid > 0)
		{
			group = entity_list.GetGroupByID(groupid);

			if (!group)
			{	//nobody from our group is here... start a new group
				group = new Group(groupid);

				if (group->GetID() != 0)
					entity_list.AddGroup(group, groupid);
				else
					safe_delete(group);
			}

			if (group)
				group->UpdatePlayer(client);
			else
			{
				if (client->GetMerc()) {
					Group::RemoveFromGroup(client->GetMerc());
				}

				Group::RemoveFromGroup(client);	//cannot re-establish group, kill it
			}

		}

		if (group)
		{
			if (client->GetMerc())
			{
				client->GetMerc()->MercJoinClientGroup();
			}
			database.RefreshGroupFromDB(client);

			group->SendHPManaEndPacketsTo(client);

			// If the group leader is not set, pull the group leader information from the database.
			if (!group->GetLeader())
			{
				char ln[64];
				char MainTankName[64];
				char AssistName[64];
				char PullerName[64];
				char NPCMarkerName[64];
				char mentoree_name[64];
				int mentor_percent;
				GroupLeadershipAA_Struct GLAA;
				memset(ln, 0, 64);
				database.GetGroupLeadershipInfo(group->GetID(), ln, MainTankName, AssistName, PullerName, NPCMarkerName, mentoree_name, &mentor_percent, &GLAA);
				Client *lc = entity_list.GetClientByName(ln);
				if (lc)
					group->SetLeader(lc);

				group->SetMainTank(MainTankName);
				group->SetMainAssist(AssistName);
				group->SetPuller(PullerName);
				group->SetNPCMarker(NPCMarkerName);
				group->SetGroupAAs(&GLAA);
				group->SetGroupMentor(mentor_percent, mentoree_name);
				client->JoinGroupXTargets(group);
			}
		}
		else if (client->GetMerc())
		{
			client->GetMerc()->MercJoinClientGroup();
		}
		break;

	}
	case ServerOP_GroupCancelInvite: {

		GroupCancel_Struct* sgcs = (GroupCancel_Struct*)pack->pBuffer;

		Mob* Inviter = entity_list.GetClientByName(sgcs->name1);

		if (Inviter && Inviter->IsClient())
		{
			auto outapp = new EQApplicationPacket(OP_GroupCancelInvite, sizeof(GroupCancel_Struct));
			memcpy(outapp->pBuffer, sgcs, sizeof(GroupCancel_Struct));
			Inviter->CastToClient()->QueuePacket(outapp);
			safe_delete(outapp);
		}
		break;
	}
	case ServerOP_GroupJoin: {
		auto gj = (ServerGroupJoin_Struct*)pack->pBuffer;
		if (zone) {
			if (gj->zoneid == zone->GetZoneID() && gj->instance_id == zone->GetInstanceID())
				break;

			Group* g = entity_list.GetGroupByID(gj->gid);
			if (g)
				g->AddMember(gj->member_name);

			entity_list.SendGroupJoin(gj->gid, gj->member_name);
		}
		break;
	}
	case ServerOP_ForceGroupUpdate: {
		ServerForceGroupUpdate_Struct* fgu = (ServerForceGroupUpdate_Struct*)pack->pBuffer;
		if (zone) {
			if (fgu->origZoneID == zone->GetZoneID() && fgu->instance_id == zone->GetInstanceID())
				break;

			entity_list.ForceGroupUpdate(fgu->gid);
		}
		break;
	}
	case ServerOP_IsOwnerOnline: {
		auto o = (ServerIsOwnerOnline_Struct*)pack->pBuffer;
		if (zone) {
			if (o->zone_id != zone->GetZoneID()) {
				break;
			}

			Corpse* c = entity_list.GetCorpseByID(o->corpse_id);
			if (c && o->online) {
				c->SetOwnerOnline(true);
			} else if (c) {
				c->SetOwnerOnline(false);
			}
		}
		break;
	}
	case ServerOP_OOZGroupMessage: {
		ServerGroupChannelMessage_Struct* gcm = (ServerGroupChannelMessage_Struct*)pack->pBuffer;
		if (zone) {
			if (gcm->zoneid == zone->GetZoneID() && gcm->instanceid == zone->GetInstanceID())
				break;

			entity_list.GroupMessage(gcm->groupid, gcm->from, gcm->message);
		}
		break;
	}
	case ServerOP_DisbandGroup: {
		ServerDisbandGroup_Struct* sd = (ServerDisbandGroup_Struct*)pack->pBuffer;
		if (zone) {
			if (sd->zoneid == zone->GetZoneID() && sd->instance_id == zone->GetInstanceID())
				break;

			Group *g = entity_list.GetGroupByID(sd->groupid);
			if (g)
				g->DisbandGroup();
		}
		break;
	}
	case ServerOP_RaidAdd: {
		auto rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
		if (zone) {
			if (rga->zoneid == zone->GetZoneID() && rga->instance_id == zone->GetInstanceID())
				break;

			Raid *r = entity_list.GetRaidByID(rga->rid);
			if (r) {
				r->LearnMembers();
				r->VerifyRaid();
				r->SendRaidAddAll(rga->playername);
			}
		}
		break;
	}
	case ServerOP_RaidRemove: {
		auto rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
		if (zone) {
			if (rga->zoneid == zone->GetZoneID() && rga->instance_id == zone->GetInstanceID())
				break;

			Raid *r = entity_list.GetRaidByID(rga->rid);
			if (r) {
				r->SendRaidRemoveAll(rga->playername);
				Client *rem = entity_list.GetClientByName(rga->playername);
				if (rem) {
					rem->LeaveRaidXTargets(r);
					r->SendRaidDisband(rem);
				}
				r->LearnMembers();
				r->VerifyRaid();
			}
		}
		break;
	}
	case ServerOP_RaidDisband: {
		auto rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
		if (zone) {
			if (rga->zoneid == zone->GetZoneID() && rga->instance_id == zone->GetInstanceID())
				break;

			Raid *r = entity_list.GetRaidByID(rga->rid);
			if (r) {
				r->SendRaidDisbandAll();
				r->LearnMembers();
				r->VerifyRaid();
			}
		}
		break;
	}
	case ServerOP_RaidLockFlag: {
		auto rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
		if (zone) {
			if (rga->zoneid == zone->GetZoneID() && rga->instance_id == zone->GetInstanceID())
				break;

			Raid *r = entity_list.GetRaidByID(rga->rid);
			if (r) {
				r->GetRaidDetails(); //update our details
				if (rga->gid)
					r->SendRaidLock();
				else
					r->SendRaidUnlock();
			}
		}
		break;
	}
	case ServerOP_RaidChangeGroup: {
		auto rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
		if (zone) {
			if (rga->zoneid == zone->GetZoneID() && rga->instance_id == zone->GetInstanceID())
				break;

			Raid *r = entity_list.GetRaidByID(rga->rid);
			if (r) {
				r->LearnMembers();
				r->VerifyRaid();
				Client *c = entity_list.GetClientByName(rga->playername);
				if (c) {
					r->SendRaidDisband(c);
					r->SendRaidRemoveAll(rga->playername);
					r->SendRaidCreate(c);
					r->SendMakeLeaderPacketTo(r->leadername, c);
					r->SendBulkRaid(c);
					r->SendRaidAddAll(rga->playername);
					if (r->IsLocked()) { r->SendRaidLockTo(c); }
				}
				else {
					r->SendRaidRemoveAll(rga->playername);
					r->SendRaidAddAll(rga->playername);
				}
			}
		}
		break;
	}
	case ServerOP_UpdateGroup: {
		auto rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
		if (zone) {
			if (rga->zoneid == zone->GetZoneID() && rga->instance_id == zone->GetInstanceID())
				break;

			Raid *r = entity_list.GetRaidByID(rga->rid);
			if (r) {
				r->GroupUpdate(rga->gid, false);
			}
		}
		break;
	}
	case ServerOP_RaidGroupLeader: {
		auto rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
		if (zone) {
			if (rga->zoneid == zone->GetZoneID() && rga->instance_id == zone->GetInstanceID())
				break;
		}
		break;
	}
	case ServerOP_RaidLeader: {
		auto rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
		if (zone) {
			if (rga->zoneid == zone->GetZoneID() && rga->instance_id == zone->GetInstanceID())
				break;

			Raid *r = entity_list.GetRaidByID(rga->rid);
			if (r) {
				Client *c = entity_list.GetClientByName(rga->playername);
				strn0cpy(r->leadername, rga->playername, 64);
				if (c) {
					r->SetLeader(c);
				}
				r->LearnMembers();
				r->VerifyRaid();
				r->SendMakeLeaderPacket(rga->playername);
			}
		}
		break;
	}
	case ServerOP_DetailsChange: {
		auto rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
		if (zone) {
			if (rga->zoneid == zone->GetZoneID() && rga->instance_id == zone->GetInstanceID())
				break;

			Raid *r = entity_list.GetRaidByID(rga->rid);
			if (r) {
				r->GetRaidDetails();
				r->LearnMembers();
				r->VerifyRaid();
			}
		}
		break;
	}
	case ServerOP_RaidGroupDisband: {
		auto rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
		if (zone) {
			if (rga->zoneid == zone->GetZoneID() && rga->instance_id == zone->GetInstanceID())
				break;

			Client *c = entity_list.GetClientByName(rga->playername);
			if (c)
			{
				auto outapp =
					new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupUpdate_Struct));
				auto gu = (GroupUpdate_Struct*)outapp->pBuffer;
				gu->action = groupActDisband;
				strn0cpy(gu->leadersname, c->GetName(), 64);
				strn0cpy(gu->yourname, c->GetName(), 64);
				c->FastQueuePacket(&outapp);
			}
		}
		break;
	}
	case ServerOP_RaidGroupAdd: {
		auto rga = (ServerRaidGroupAction_Struct*)pack->pBuffer;
		if (zone) {
			Raid *r = entity_list.GetRaidByID(rga->rid);
			if (r) {
				r->LearnMembers();
				r->VerifyRaid();
				auto outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
				auto gj = (GroupJoin_Struct*)outapp->pBuffer;
				strn0cpy(gj->membername, rga->membername, 64);
				gj->action = groupActJoin;

				for (const auto& m : r->members) {
					if (m.is_bot) {
						continue;
					}

					if (m.member && strcmp(m.member->GetName(), rga->membername) != 0) {
						if ((rga->gid < MAX_RAID_GROUPS) && rga->gid == m.group_number) {
							strn0cpy(gj->yourname, m.member->GetName(), 64);
							m.member->QueuePacket(outapp);
						}
					}
				}
				safe_delete(outapp);
			}
		}
		break;
	}
	case ServerOP_RaidGroupRemove: {
		auto rga = (ServerRaidGroupAction_Struct*)pack->pBuffer;
		if (zone) {
			Raid *r = entity_list.GetRaidByID(rga->rid);
			if (r) {
				r->LearnMembers();
				r->VerifyRaid();
				auto outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
				auto gj = (GroupJoin_Struct*)outapp->pBuffer;
				strn0cpy(gj->membername, rga->membername, 64);
				gj->action = groupActLeave;

				for (const auto& m : r->members) {
					if (m.is_bot) {
						continue;
					}

					if (m.member && strcmp(m.member->GetName(), rga->membername) != 0) {
						if ((rga->gid < MAX_RAID_GROUPS) && rga->gid == m.group_number) {
							strn0cpy(gj->yourname, m.member->GetName(), 64);
							m.member->QueuePacket(outapp);
						}
					}
				}
				safe_delete(outapp);
			}
		}
		break;
	}
	case ServerOP_RaidGroupSay: {
		auto rmsg = (ServerRaidMessage_Struct*)pack->pBuffer;
		if (zone) {
			Raid *r = entity_list.GetRaidByID(rmsg->rid);
			if (r) {
				for (const auto& m :r->members) {
					if (m.is_bot) {
						continue;
					}

					if (m.member && strcmp(m.member->GetName(), rmsg->from) != 0) {
						if (m.group_number == rmsg->gid) {
							if (m.member->GetFilter(FilterGroupChat) != 0) {
								m.member->ChannelMessageSend(rmsg->from, m.member->GetName(), ChatChannel_Group, rmsg->language, rmsg->lang_skill, rmsg->message);
							}
						}
					}
				}
			}
		}
		break;
	}
	case ServerOP_RaidSay: {
		auto rmsg = (ServerRaidMessage_Struct*)pack->pBuffer;
		if (zone) {
			Raid *r = entity_list.GetRaidByID(rmsg->rid);

			if (r) {
				for (const auto& m :r->members) {
					if (m.is_bot) {
						continue;
					}

					if (m.member) {
						if (strcmp(rmsg->from, m.member->GetName()) != 0) {
							if (!m.is_bot && m.member->GetFilter(FilterGroupChat) != 0) {
								m.member->ChannelMessageSend(rmsg->from, m.member->GetName(), ChatChannel_Raid, rmsg->language, rmsg->lang_skill, rmsg->message);
							}
						}
					}
				}
			}
		}
		break;
	}
	case ServerOP_RaidMOTD: {
		ServerRaidMOTD_Struct *rmotd = (ServerRaidMOTD_Struct *)pack->pBuffer;
		if (!zone)
			break;
		Raid *r = entity_list.GetRaidByID(rmotd->rid);
		if (!r)
			break;
		r->SetRaidMOTD(std::string(rmotd->motd));
		r->SendRaidMOTD();
		break;
	}
	case ServerOP_RaidNote: {
		auto snote = (ServerRaidNote_Struct*)pack->pBuffer;
		if (snote->rid > 0) {
			Raid* r = entity_list.GetRaidByID(snote->rid);
			if (r) {
				r->SendRaidNotes();
			}
		}
		break;
	}
	case ServerOP_SpawnPlayerCorpse: {
		auto   *s = (SpawnPlayerCorpse_Struct *) pack->pBuffer;
		Corpse *c = database.LoadCharacterCorpse(s->player_corpse_id);
		if (c) {
			c->Spawn();
		}
		else {
			LogError("Unable to load player corpse id [{}] for zone [{}]", s->player_corpse_id, zone->GetShortName());
		}

		break;
	}
	case ServerOP_Consent: {
		ServerOP_Consent_Struct* s = (ServerOP_Consent_Struct*)pack->pBuffer;

		bool found_corpse = false;
		for (auto const& it : entity_list.GetCorpseList()) {
			if (it.second->IsPlayerCorpse() && strcmp(it.second->GetOwnerName(), s->ownername) == 0) {
				if (s->consent_type == EQ::consent::Normal) {
					if (s->permission == 1) {
						it.second->AddConsentName(s->grantname);
					}
					else {
						it.second->RemoveConsentName(s->grantname);
					}
				}
				else if (s->consent_type == EQ::consent::Group) {
					it.second->SetConsentGroupID(s->consent_id);
				}
				else if (s->consent_type == EQ::consent::Raid) {
					it.second->SetConsentRaidID(s->consent_id);
				}
				else if (s->consent_type == EQ::consent::Guild) {
					it.second->SetConsentGuildID(s->consent_id);
				}
				found_corpse = true;
			}
		}

		if (found_corpse) {
			// forward the grant/deny message for this zone to both owner and granted
			auto outapp = new ServerPacket(ServerOP_Consent_Response, sizeof(ServerOP_Consent_Struct));
			ServerOP_Consent_Struct* scs = (ServerOP_Consent_Struct*)outapp->pBuffer;
			memcpy(outapp->pBuffer, s, sizeof(ServerOP_Consent_Struct));
			if (zone) {
				strn0cpy(scs->zonename, zone->GetLongName(), sizeof(scs->zonename));
			}
			worldserver.SendPacket(outapp);
			safe_delete(outapp);
		}
		break;
	}
	case ServerOP_Consent_Response: {
		ServerOP_Consent_Struct* s = (ServerOP_Consent_Struct*)pack->pBuffer;
		Client* owner_client = entity_list.GetClientByName(s->ownername);
		Client* grant_client = nullptr;
		if (s->consent_type == EQ::consent::Normal) {
			grant_client = entity_list.GetClientByName(s->grantname);
		}
		if (owner_client || grant_client) {
			auto outapp = new EQApplicationPacket(OP_ConsentResponse, sizeof(ConsentResponse_Struct));
			ConsentResponse_Struct* crs = (ConsentResponse_Struct*)outapp->pBuffer;
			strn0cpy(crs->grantname, s->grantname, sizeof(crs->grantname));
			strn0cpy(crs->ownername, s->ownername, sizeof(crs->ownername));
			crs->permission = s->permission;
			strn0cpy(crs->zonename, s->zonename, sizeof(crs->zonename));
			if (owner_client) {
				owner_client->QueuePacket(outapp); // confirmation message to the owner
			}
			if (grant_client) {
				grant_client->QueuePacket(outapp); // message to the client being granted/denied
			}
			safe_delete(outapp);
		}
		break;
	}
	case ServerOP_LFGMatches: {
		HandleLFGMatches(pack);
		break;
	}
	case ServerOP_LFPMatches: {
		HandleLFPMatches(pack);
		break;
	}
	case ServerOP_UpdateSpawn: {
		if (zone)
		{
			UpdateSpawnTimer_Struct *ust = (UpdateSpawnTimer_Struct*)pack->pBuffer;
			LinkedListIterator<Spawn2*> iterator(zone->spawn2_list);
			iterator.Reset();
			while (iterator.MoreElements())
			{
				if (iterator.GetData()->GetID() == ust->id)
				{
					if (!iterator.GetData()->NPCPointerValid())
					{
						iterator.GetData()->SetTimer(ust->duration);
					}
					break;
				}
				iterator.Advance();
			}
		}
		break;
	}
	case ServerOP_InstanceUpdateTime:
	{
		ServerInstanceUpdateTime_Struct *iut = (ServerInstanceUpdateTime_Struct*)pack->pBuffer;
		if (zone)
		{
			if (zone->GetInstanceID() == iut->instance_id)
			{
				zone->SetInstanceTimer(iut->new_duration);
			}
		}
		break;
	}
	case ServerOP_DepopAllPlayersCorpses:
	{
		ServerDepopAllPlayersCorpses_Struct *sdapcs = (ServerDepopAllPlayersCorpses_Struct *)pack->pBuffer;

		if (zone && !((zone->GetZoneID() == sdapcs->ZoneID) && (zone->GetInstanceID() == sdapcs->InstanceID)))
			entity_list.RemoveAllCorpsesByCharID(sdapcs->CharacterID);

		break;

	}
	case ServerOP_DepopPlayerCorpse:
	{
		ServerDepopPlayerCorpse_Struct *sdpcs = (ServerDepopPlayerCorpse_Struct *)pack->pBuffer;

		if (zone && !((zone->GetZoneID() == sdpcs->ZoneID) && (zone->GetInstanceID() == sdpcs->InstanceID)))
			entity_list.RemoveCorpseByDBID(sdpcs->DBID);

		break;

	}
	case ServerOP_SpawnStatusChange:
	{
		if (zone)
		{
			ServerSpawnStatusChange_Struct *ssc = (ServerSpawnStatusChange_Struct*)pack->pBuffer;
			if (ssc->instance_id != zone->GetInstanceID()) {
				break;
			}

			LinkedListIterator<Spawn2 *> iterator(zone->spawn2_list);
			iterator.Reset();
			Spawn2 *found_spawn = nullptr;
			while (iterator.MoreElements()) {
				Spawn2 *cur = iterator.GetData();
				if (cur->GetID() == ssc->id) {
					found_spawn = cur;
					break;
				}
				iterator.Advance();
			}

			if (found_spawn) {
				if (ssc->new_status == 0) {
					found_spawn->Disable();
				}
				else {
					found_spawn->Enable();
				}
			}
		}
		break;
	}
	case ServerOP_QGlobalUpdate:
	{
		if (pack->size != sizeof(ServerQGlobalUpdate_Struct))
		{
			break;
		}

		if (zone)
		{
			ServerQGlobalUpdate_Struct *qgu = (ServerQGlobalUpdate_Struct*)pack->pBuffer;
			if (qgu->from_zone_id != zone->GetZoneID() || qgu->from_instance_id != zone->GetInstanceID())
			{
				QGlobal temp;
				temp.npc_id = qgu->npc_id;
				temp.char_id = qgu->char_id;
				temp.zone_id = qgu->zone_id;
				temp.expdate = qgu->expdate;
				temp.name.assign(qgu->name);
				temp.value.assign(qgu->value);
				entity_list.UpdateQGlobal(qgu->id, temp);
				zone->UpdateQGlobal(qgu->id, temp);
			}
		}
		break;
	}
	case ServerOP_QGlobalDelete:
	{
		if (pack->size != sizeof(ServerQGlobalDelete_Struct))
		{
			break;
		}

		if (zone)
		{
			ServerQGlobalDelete_Struct *qgd = (ServerQGlobalDelete_Struct*)pack->pBuffer;
			if (qgd->from_zone_id != zone->GetZoneID() || qgd->from_instance_id != zone->GetInstanceID())
			{
				entity_list.DeleteQGlobal(std::string((char*)qgd->name), qgd->npc_id, qgd->char_id, qgd->zone_id);
				zone->DeleteQGlobal(std::string((char*)qgd->name), qgd->npc_id, qgd->char_id, qgd->zone_id);
			}
		}
		break;
	}
	case ServerOP_AdventureRequestAccept:
	{
		ServerAdventureRequestAccept_Struct *ars = (ServerAdventureRequestAccept_Struct*)pack->pBuffer;
		Client *c = entity_list.GetClientByName(ars->leader);
		if (c)
		{
			c->NewAdventure(ars->id, ars->theme, ars->text, ars->member_count, (const char*)(pack->pBuffer + sizeof(ServerAdventureRequestAccept_Struct)));
			c->ClearPendingAdventureRequest();
		}
		break;
	}
	case ServerOP_AdventureRequestDeny:
	{
		ServerAdventureRequestDeny_Struct *ars = (ServerAdventureRequestDeny_Struct*)pack->pBuffer;
		Client *c = entity_list.GetClientByName(ars->leader);
		if (c)
		{
			c->SendAdventureError(ars->reason);
			c->ClearPendingAdventureRequest();
		}
		break;
	}
	case ServerOP_AdventureCreateDeny:
	{
		Client *c = entity_list.GetClientByName((const char*)pack->pBuffer);
		if (c)
		{
			c->ClearPendingAdventureData();
			c->ClearPendingAdventureCreate();
		}
		break;
	}
	case ServerOP_AdventureData:
	{
		Client *c = entity_list.GetClientByName((const char*)pack->pBuffer);
		if (c)
		{
			c->ClearAdventureData();
			auto adv_data = new char[pack->size];
			memcpy(adv_data, pack->pBuffer, pack->size);
			c->SetAdventureData(adv_data);
			c->ClearPendingAdventureData();
			c->ClearPendingAdventureCreate();
			c->SendAdventureDetails();
		}
		break;
	}
	case ServerOP_AdventureDataClear:
	{
		Client *c = entity_list.GetClientByName((const char*)pack->pBuffer);
		if (c)
		{
			if (c->HasAdventureData())
			{
				c->ClearAdventureData();
				c->SendAdventureError("You are not currently assigned to an adventure.");
			}
		}
		break;
	}
	case ServerOP_AdventureClickDoorReply:
	{
		ServerPlayerClickedAdventureDoorReply_Struct *adr = (ServerPlayerClickedAdventureDoorReply_Struct*)pack->pBuffer;
		Client *c = entity_list.GetClientByName(adr->player);
		if (c)
		{
			c->ClearPendingAdventureDoorClick();
			c->MovePC(adr->zone_id, adr->instance_id, adr->x, adr->y, adr->z, adr->h, 0);
		}
		break;
	}
	case ServerOP_AdventureClickDoorError:
	{
		Client *c = entity_list.GetClientByName((const char*)pack->pBuffer);
		if (c)
		{
			c->ClearPendingAdventureDoorClick();
			c->MessageString(Chat::Red, 5141);
		}
		break;
	}
	case ServerOP_AdventureLeaveReply:
	{
		Client *c = entity_list.GetClientByName((const char*)pack->pBuffer);
		if (c)
		{
			c->ClearPendingAdventureLeave();
			c->ClearCurrentAdventure();
		}
		break;
	}
	case ServerOP_AdventureLeaveDeny:
	{
		Client *c = entity_list.GetClientByName((const char*)pack->pBuffer);
		if (c)
		{
			c->ClearPendingAdventureLeave();
			c->Message(Chat::Red, "You cannot leave this adventure at this time.");
		}
		break;
	}
	case ServerOP_AdventureCountUpdate:
	{
		ServerAdventureCountUpdate_Struct *ac = (ServerAdventureCountUpdate_Struct*)pack->pBuffer;
		Client *c = entity_list.GetClientByName(ac->player);
		if (c)
		{
			c->SendAdventureCount(ac->count, ac->total);
		}
		break;
	}
	case ServerOP_AdventureZoneData:
	{
		if (zone)
		{
			safe_delete(zone->adv_data);
			zone->adv_data = new char[pack->size];
			memcpy(zone->adv_data, pack->pBuffer, pack->size);
			ServerZoneAdventureDataReply_Struct* ds = (ServerZoneAdventureDataReply_Struct*)zone->adv_data;
		}
		break;
	}
	case ServerOP_AdventureFinish:
	{
		ServerAdventureFinish_Struct *af = (ServerAdventureFinish_Struct*)pack->pBuffer;
		Client *c = entity_list.GetClientByName(af->player);
		if (c)
		{
			c->AdventureFinish(af->win, af->theme, af->points);
		}
		break;
	}
	case ServerOP_AdventureLeaderboard:
	{
		Client *c = entity_list.GetClientByName((const char*)pack->pBuffer);
		if (c)
		{
			auto outapp = new EQApplicationPacket(OP_AdventureLeaderboardReply,
				sizeof(AdventureLeaderboard_Struct));
			memcpy(outapp->pBuffer, pack->pBuffer + 64, sizeof(AdventureLeaderboard_Struct));
			c->FastQueuePacket(&outapp);
		}
		break;
	}
	case ServerOP_ServerReloadRequest:
	{
		auto o = (ServerReload::Request*) pack->pBuffer;
		QueueReload(*o);
		break;
	}
	case ServerOP_CameraShake:
	{
		if (zone)
		{
			ServerCameraShake_Struct *scss = (ServerCameraShake_Struct*)pack->pBuffer;
			entity_list.CameraEffect(scss->duration, scss->intensity);
		}
		break;
	}
	case ServerOP_QueryServGeneric:
	{
		pack->SetReadPosition(8);
		char From[64];
		pack->ReadString(From);

		Client *c = entity_list.GetClientByName(From);

		if (!c)
			return;

		uint32 Type = pack->ReadUInt32();;

		switch (Type)
		{
		case QSG_LFGuild:
		{
			c->HandleLFGuildResponse(pack);
			break;
		}

		default:
			break;
		}

		break;
	}
	case ServerOP_UCSServerStatusReply:
	{
		auto ucsss = (UCSServerStatus_Struct *) pack->pBuffer;
		if (zone) {
			zone->SetUCSServerAvailable((ucsss->available != 0), ucsss->timestamp);
			LogInfo("UCS Server is now [{}]", (ucsss->available == 1 ? "online" : "offline"));

			for (auto &e : entity_list.GetClientList()) {
				e.second->ReconnectUCS();
			}
		}
		break;
	}
	case ServerOP_CZDialogueWindow:
	{
		auto s = (CZDialogueWindow_Struct*) pack->pBuffer;

		if (s->update_type == CZUpdateType_Character) {
			Client* c = entity_list.GetClientByCharID(s->update_identifier);
			if (c) {
				DialogueWindow::Render(c, s->message);
			}
		} else if (s->update_type == CZUpdateType_Group) {
			Group* g = entity_list.GetGroupByID(s->update_identifier);
			if (g) {
				for (const auto& m : g->members) {
					if (m && m->IsClient()) {
						DialogueWindow::Render(m->CastToClient(), s->message);
					}
				}
			}
		} else if (s->update_type == CZUpdateType_Raid) {
			Raid* r = entity_list.GetRaidByID(s->update_identifier);
			if (r) {
				for (const auto& m : r->members) {
					if (m.member && m.member->IsClient()) {
						DialogueWindow::Render(m.member->CastToClient(), s->message);
					}
				}
			}
		} else if (s->update_type == CZUpdateType_Guild) {
			for (const auto& c : entity_list.GetClientList()) {
				if (c.second->IsInGuild(s->update_identifier)) {
					DialogueWindow::Render(c.second, s->message);
				}
			}
		} else if (s->update_type == CZUpdateType_Expedition) {
			for (const auto& c: entity_list.GetClientList()) {
				if (c.second->GetExpedition() && c.second->GetExpedition()->GetID() == s->update_identifier) {
					DialogueWindow::Render(c.second, s->message);
				}
			}
		} else if (s->update_type == CZUpdateType_ClientName) {
			Client* c = entity_list.GetClientByName(s->client_name);
			if (c) {
				DialogueWindow::Render(c, s->message);
			}
		}

		break;
	}
	case ServerOP_CZLDoNUpdate:
	{
		auto u = (CZLDoNUpdate_Struct*) pack->pBuffer;

		if (u->update_type == CZUpdateType_Character) {
			Client* c = entity_list.GetClientByCharID(u->update_identifier);
			if (c) {
				switch (u->update_subtype) {
					case CZLDoNUpdateSubtype_AddLoss:
						c->UpdateLDoNWinLoss(u->theme_id, false);
						break;
					case CZLDoNUpdateSubtype_AddPoints:
						c->UpdateLDoNPoints(u->theme_id, u->points);
						break;
					case CZLDoNUpdateSubtype_AddWin:
						c->UpdateLDoNWinLoss(u->theme_id, true);
						break;
					case CZLDoNUpdateSubtype_RemoveLoss:
						c->UpdateLDoNWinLoss(u->theme_id, false, true);
						break;
					case CZLDoNUpdateSubtype_RemoveWin:
						c->UpdateLDoNWinLoss(u->theme_id, true, true);
						break;
					default:
						break;
				}
			}
			break;
		} else if (u->update_type == CZUpdateType_Group) {
			Group* g = entity_list.GetGroupByID(u->update_identifier);
			if (g) {
				for (const auto& m : g->members) {
					if (m && m->IsClient()) {
						switch (u->update_subtype) {
							case CZLDoNUpdateSubtype_AddLoss:
								m->CastToClient()->UpdateLDoNWinLoss(u->theme_id, false);
								break;
							case CZLDoNUpdateSubtype_AddPoints:
								m->CastToClient()->UpdateLDoNPoints(u->theme_id, u->points);
								break;
							case CZLDoNUpdateSubtype_AddWin:
								m->CastToClient()->UpdateLDoNWinLoss(u->theme_id, true);
								break;
							case CZLDoNUpdateSubtype_RemoveLoss:
								m->CastToClient()->UpdateLDoNWinLoss(u->theme_id, false, true);
								break;
							case CZLDoNUpdateSubtype_RemoveWin:
								m->CastToClient()->UpdateLDoNWinLoss(u->theme_id, true, true);
								break;
							default:
								break;
						}
					}
				}
			}
		} else if (u->update_type == CZUpdateType_Raid) {
			Raid* r = entity_list.GetRaidByID(u->update_identifier);
			if (r) {
				for (const auto& m : r->members) {
					if (m.member && m.member->IsClient()) {
						switch (u->update_subtype) {
							case CZLDoNUpdateSubtype_AddLoss:
								m.member->UpdateLDoNWinLoss(u->theme_id, false);
								break;
							case CZLDoNUpdateSubtype_AddPoints:
								m.member->UpdateLDoNPoints(u->theme_id, u->points);
								break;
							case CZLDoNUpdateSubtype_AddWin:
								m.member->UpdateLDoNWinLoss(u->theme_id, true);
								break;
							case CZLDoNUpdateSubtype_RemoveLoss:
								m.member->UpdateLDoNWinLoss(u->theme_id, false, true);
								break;
							case CZLDoNUpdateSubtype_RemoveWin:
								m.member->UpdateLDoNWinLoss(u->theme_id, true, true);
								break;
							default:
								break;
						}
					}
				}
			}
		} else if (u->update_type == CZUpdateType_Guild) {
			for (const auto& c : entity_list.GetClientList()) {
				if (c.second->IsInGuild(u->update_identifier)) {
					switch (u->update_subtype) {
						case CZLDoNUpdateSubtype_AddLoss:
							c.second->UpdateLDoNWinLoss(u->theme_id, false);
							break;
						case CZLDoNUpdateSubtype_AddPoints:
							c.second->UpdateLDoNPoints(u->theme_id, u->points);
							break;
						case CZLDoNUpdateSubtype_AddWin:
							c.second->UpdateLDoNWinLoss(u->theme_id, true);
							break;
						case CZLDoNUpdateSubtype_RemoveLoss:
							c.second->UpdateLDoNWinLoss(u->theme_id, false, true);
							break;
						case CZLDoNUpdateSubtype_RemoveWin:
							c.second->UpdateLDoNWinLoss(u->theme_id, true, true);
							break;
						default:
							break;
					}
				}
			}
		} else if (u->update_type == CZUpdateType_Expedition) {
			for (const auto& c : entity_list.GetClientList()) {
				if (c.second->GetExpedition() && c.second->GetExpedition()->GetID() == u->update_identifier) {
					switch (u->update_subtype) {
						case CZLDoNUpdateSubtype_AddLoss:
							c.second->UpdateLDoNWinLoss(u->theme_id, false);
							break;
						case CZLDoNUpdateSubtype_AddPoints:
							c.second->UpdateLDoNPoints(u->theme_id, u->points);
							break;
						case CZLDoNUpdateSubtype_AddWin:
							c.second->UpdateLDoNWinLoss(u->theme_id, true);
							break;
						case CZLDoNUpdateSubtype_RemoveLoss:
							c.second->UpdateLDoNWinLoss(u->theme_id, false, true);
							break;
						case CZLDoNUpdateSubtype_RemoveWin:
							c.second->UpdateLDoNWinLoss(u->theme_id, true, true);
							break;
						default:
							break;
					}
				}
			}
		} else if (u->update_type == CZUpdateType_ClientName) {
			Client* c = entity_list.GetClientByName(u->client_name);
			if (c) {
				switch (u->update_subtype) {
					case CZLDoNUpdateSubtype_AddLoss:
						c->UpdateLDoNWinLoss(u->theme_id, false);
						break;
					case CZLDoNUpdateSubtype_AddPoints:
						c->UpdateLDoNPoints(u->theme_id, u->points);
						break;
					case CZLDoNUpdateSubtype_AddWin:
						c->UpdateLDoNWinLoss(u->theme_id, true);
						break;
					case CZLDoNUpdateSubtype_RemoveLoss:
						c->UpdateLDoNWinLoss(u->theme_id, false, true);
						break;
					case CZLDoNUpdateSubtype_RemoveWin:
						c->UpdateLDoNWinLoss(u->theme_id, true, true);
						break;
					default:
						break;
				}
			}
			break;
		}
		break;
	}
	case ServerOP_CZMarquee:
	{
		auto s = (CZMarquee_Struct*) pack->pBuffer;

		if (s->update_type == CZUpdateType_Character) {
			Client* c = entity_list.GetClientByCharID(s->update_identifier);
			if (c) {
				c->SendMarqueeMessage(s->type, s->priority, s->fade_in, s->fade_out, s->duration, s->message);
			}
		} else if (s->update_type == CZUpdateType_Group) {
			Group* g = entity_list.GetGroupByID(s->update_identifier);
			if (g) {
				for (const auto& m : g->members) {
					if (m && m->IsClient()) {
						m->CastToClient()->SendMarqueeMessage(
							s->type,
							s->priority,
							s->fade_in,
							s->fade_out,
							s->duration,
							s->message
						);
					}
				}
			}
		} else if (s->update_type == CZUpdateType_Raid) {
			Raid* r = entity_list.GetRaidByID(s->update_identifier);
			if (r) {
				for (const auto& m : r->members) {
					if (m.member && m.member->IsClient()) {
						m.member->CastToClient()->SendMarqueeMessage(
							s->type,
							s->priority,
							s->fade_in,
							s->fade_out,
							s->duration,
							s->message
						);
					}
				}
			}
		} else if (s->update_type == CZUpdateType_Guild) {
			for (const auto& c: entity_list.GetClientList()) {
				if (c.second->IsInGuild(s->update_identifier)) {
					c.second->SendMarqueeMessage(
						s->type,
						s->priority,
						s->fade_in,
						s->fade_out,
						s->duration,
						s->message
					);
				}
			}
		} else if (s->update_type == CZUpdateType_Expedition) {
			for (const auto& c: entity_list.GetClientList()) {
				if (c.second->GetExpedition() && c.second->GetExpedition()->GetID() == s->update_identifier) {
					c.second->SendMarqueeMessage(
						s->type,
						s->priority,
						s->fade_in,
						s->fade_out,
						s->duration,
						s->message
					);
				}
			}
		} else if (s->update_type == CZUpdateType_ClientName) {
			Client* c = entity_list.GetClientByName(s->client_name);
			if (c) {
				c->SendMarqueeMessage(s->type, s->priority, s->fade_in, s->fade_out, s->duration, s->message);
			}
		}

		break;
	}
	case ServerOP_CZMessage:
	{
		auto s = (CZMessage_Struct*) pack->pBuffer;

		if (s->update_type == CZUpdateType_Character) {
			Client* c = entity_list.GetClientByCharID(s->update_identifier);
			if (c) {
				c->Message(s->type, s->message);
			}
		} else if (s->update_type == CZUpdateType_Group) {
			Group* g = entity_list.GetGroupByID(s->update_identifier);
			if (g) {
				for (const auto& m : g->members) {
					if (m && m->IsClient()) {
						m->CastToClient()->Message(s->type, s->message);
					}
				}
			}
		} else if (s->update_type == CZUpdateType_Raid) {
			Raid* r = entity_list.GetRaidByID(s->update_identifier);
			if (r) {
				for (const auto& m : r->members) {
					if (m.member && m.member->IsClient()) {
						m.member->CastToClient()->Message(s->type, s->message);
					}
				}
			}
		} else if (s->update_type == CZUpdateType_Guild) {
			for (const auto& c : entity_list.GetClientList()) {
				if (c.second->IsInGuild(s->update_identifier)) {
					c.second->Message(s->type, s->message);
				}
			}
		} else if (s->update_type == CZUpdateType_Expedition) {
			for (const auto& c : entity_list.GetClientList()) {
				if (c.second->GetExpedition() && c.second->GetExpedition()->GetID() == s->update_identifier) {
					c.second->Message(s->type, s->message);
				}
			}
		} else if (s->update_type == CZUpdateType_ClientName) {
			Client* c = entity_list.GetClientByName(s->client_name);
			if (c) {
				c->Message(s->type, s->message);
			}
		}

		break;
	}
	case ServerOP_CZMove:
	{
		auto s = (CZMove_Struct*) pack->pBuffer;

		if (Strings::IsNumber(s->client_name) || Strings::IsNumber(s->zone_short_name)) {
			break;
		}

		if (s->update_type == CZUpdateType_Character) {
			Client* c = entity_list.GetClientByCharID(s->update_identifier);
			if (c) {
				switch (s->update_subtype) {
					case CZMoveUpdateSubtype_MoveZone:
						c->MoveZone(s->zone_short_name.c_str(), s->coordinates);
						break;
					case CZMoveUpdateSubtype_MoveZoneInstance:
						c->MoveZoneInstance(s->instance_id, s->coordinates);
						break;
				}
			}
		} else if (s->update_type == CZUpdateType_Group) {
			Group* g = entity_list.GetGroupByID(s->update_identifier);
			if (g) {
				for (const auto& m : g->members) {
					if (m && m->IsClient()) {
						switch (s->update_subtype) {
							case CZMoveUpdateSubtype_MoveZone:
								m->CastToClient()->MoveZone(s->zone_short_name.c_str(), s->coordinates);
								break;
							case CZMoveUpdateSubtype_MoveZoneInstance:
								m->CastToClient()->MoveZoneInstance(s->instance_id, s->coordinates);
								break;
						}
					}
				}
			}
		} else if (s->update_type == CZUpdateType_Raid) {
			Raid* r = entity_list.GetRaidByID(s->update_identifier);
			if (r) {
				for (const auto& m : r->members) {
					if (m.member && m.member->IsClient()) {
						switch (s->update_subtype) {
							case CZMoveUpdateSubtype_MoveZone:
								m.member->CastToClient()->MoveZone(s->zone_short_name.c_str(), s->coordinates);
								break;
							case CZMoveUpdateSubtype_MoveZoneInstance:
								m.member->CastToClient()->MoveZoneInstance(s->instance_id, s->coordinates);
								break;
						}
					}
				}
			}
		} else if (s->update_type == CZUpdateType_Guild) {
			for (const auto& c : entity_list.GetClientList()) {
				if (c.second->IsInGuild(s->update_identifier)) {
					switch (s->update_subtype) {
						case CZMoveUpdateSubtype_MoveZone:
							c.second->MoveZone(s->zone_short_name.c_str(), s->coordinates);
							break;
						case CZMoveUpdateSubtype_MoveZoneInstance:
							c.second->MoveZoneInstance(s->instance_id, s->coordinates);
							break;
					}
				}
			}
		} else if (s->update_type == CZUpdateType_Expedition) {
			for (const auto& c : entity_list.GetClientList()) {
				if (c.second->GetExpedition() && c.second->GetExpeditionID() == s->update_identifier) {
					switch (s->update_subtype) {
						case CZMoveUpdateSubtype_MoveZone:
							c.second->MoveZone(s->zone_short_name.c_str(), s->coordinates);
							break;
						case CZMoveUpdateSubtype_MoveZoneInstance:
							c.second->MoveZoneInstance(s->instance_id, s->coordinates);
							break;
					}
				}
			}
		} else if (s->update_type == CZUpdateType_ClientName) {
			Client* c = entity_list.GetClientByName(s->client_name.c_str());
			if (c) {
				switch (s->update_subtype) {
					case CZMoveUpdateSubtype_MoveZone:
						c->MoveZone(s->zone_short_name.c_str(), s->coordinates);
						break;
					case CZMoveUpdateSubtype_MoveZoneInstance:
						c->MoveZoneInstance(s->instance_id, s->coordinates);
						break;
				}
			}
		}

		break;
	}
	case ServerOP_CZSetEntityVariable:
	{
		auto s = (CZSetEntityVariable_Struct*) pack->pBuffer;

		if (s->update_type == CZUpdateType_Character) {
			Client* c = entity_list.GetClientByCharID(s->update_identifier);
			if (c) {
				c->SetEntityVariable(s->variable_name, s->variable_value);
			}
		} else if (s->update_type == CZUpdateType_Group) {
			Group* g = entity_list.GetGroupByID(s->update_identifier);
			if (g) {
				for (const auto& m : g->members) {
					if (m && m->IsClient()) {
						m->SetEntityVariable(s->variable_name, s->variable_value);
					}
				}
			}
		} else if (s->update_type == CZUpdateType_Raid) {
			Raid* r = entity_list.GetRaidByID(s->update_identifier);
			if (r) {
				for (const auto& m : r->members) {
					if (m.member && m.member->IsClient()) {
						m.member->CastToClient()->SetEntityVariable(s->variable_name, s->variable_value);
					}
				}
			}
		} else if (s->update_type == CZUpdateType_Guild) {
			for (const auto& c : entity_list.GetClientList()) {
				if (c.second->IsInGuild(s->update_identifier)) {
					c.second->SetEntityVariable(s->variable_name, s->variable_value);
				}
			}
		} else if (s->update_type == CZUpdateType_Expedition) {
			for (const auto& c : entity_list.GetClientList()) {
				if (c.second->GetExpedition() && c.second->GetExpedition()->GetID() == s->update_identifier) {
					c.second->SetEntityVariable(s->variable_name, s->variable_value);
				}
			}
		} else if (s->update_type == CZUpdateType_ClientName) {
			Client* c = entity_list.GetClientByName(s->client_name);
			if (c) {
				c->SetEntityVariable(s->variable_name, s->variable_value);
			}
		} else if (s->update_type == CZUpdateType_NPC) {
			NPC* n = entity_list.GetNPCByNPCTypeID(s->update_identifier);
			if (n) {
				n->SetEntityVariable(s->variable_name, s->variable_value);
			}
		}

		break;
	}
	case ServerOP_CZSignal:
	{
		auto s = (CZSignal_Struct*) pack->pBuffer;

		if (s->update_type == CZUpdateType_Character) {
			Client* c = entity_list.GetClientByCharID(s->update_identifier);
			if (c) {
				c->Signal(s->signal_id);
			}
		} else if (s->update_type == CZUpdateType_Group) {
			Group* g = entity_list.GetGroupByID(s->update_identifier);
			if (g) {
				for (const auto& m : g->members) {
					if (m && m->IsClient()) {
						m->CastToClient()->Signal(s->signal_id);
					}
				}
			}
		} else if (s->update_type == CZUpdateType_Raid) {
			Raid* r = entity_list.GetRaidByID(s->update_identifier);
			if (r) {
				for (const auto& m : r->members) {
					if (m.member && m.member->IsClient()) {
						m.member->CastToClient()->Signal(s->signal_id);
					}
				}
			}
		} else if (s->update_type == CZUpdateType_Guild) {
			for (const auto& c : entity_list.GetClientList()) {
				if (c.second->IsInGuild(s->update_identifier)) {
					c.second->Signal(s->signal_id);
				}
			}
		} else if (s->update_type == CZUpdateType_Expedition) {
			for (const auto& c : entity_list.GetClientList()) {
				if (c.second->GetExpedition() && c.second->GetExpedition()->GetID() == s->update_identifier) {
					c.second->Signal(s->signal_id);
				}
			}
		} else if (s->update_type == CZUpdateType_ClientName) {
			Client* c = entity_list.GetClientByName(s->client_name);
			if (c) {
				c->Signal(s->signal_id);
			}
		} else if (s->update_type == CZUpdateType_NPC) {
			NPC* n = entity_list.GetNPCByNPCTypeID(s->update_identifier);
			if (n) {
				n->SignalNPC(s->signal_id);
			}
		}

		break;
	}
	case ServerOP_CZSpell:
	{
		auto s = (CZSpell_Struct*) pack->pBuffer;

		if (s->update_type == CZUpdateType_Character) {
			Client* c = entity_list.GetClientByCharID(s->update_identifier);
			if (c) {
				switch (s->update_subtype) {
					case CZSpellUpdateSubtype_Cast:
						c->ApplySpellBuff(s->spell_id);

						if (RuleB(Zone, AllowCrossZoneSpellsOnBots)) {
							for (const auto& b : entity_list.GetBotListByCharacterID(c->CharacterID())) {
								b->ApplySpellBuff(s->spell_id);

								if (RuleB(Zone, AllowCrossZoneSpellsOnPets)) {
									if (b->HasPet()) {
										b->GetPet()->ApplySpellBuff(s->spell_id);
									}
								}
							}
						}

						if (RuleB(Zone, AllowCrossZoneSpellsOnMercs)) {
							if (c->GetMerc()) {
								c->GetMerc()->ApplySpellBuff(s->spell_id);
							}
						}

						if (RuleB(Zone, AllowCrossZoneSpellsOnPets)) {
							if (c->HasPet()) {
								c->GetPet()->ApplySpellBuff(s->spell_id);
							}
						}

						break;
					case CZSpellUpdateSubtype_Remove:
						c->BuffFadeBySpellID(s->spell_id);

						if (RuleB(Zone, AllowCrossZoneSpellsOnBots)) {
							for (const auto& b : entity_list.GetBotListByCharacterID(c->CharacterID())) {
								b->BuffFadeBySpellID(s->spell_id);

								if (RuleB(Zone, AllowCrossZoneSpellsOnPets)) {
									if (b->HasPet()) {
										b->GetPet()->BuffFadeBySpellID(s->spell_id);
									}
								}
							}
						}

						if (RuleB(Zone, AllowCrossZoneSpellsOnMercs)) {
							if (c->GetMerc()) {
								c->GetMerc()->BuffFadeBySpellID(s->spell_id);
							}
						}

						if (RuleB(Zone, AllowCrossZoneSpellsOnPets)) {
							if (c->HasPet()) {
								c->GetPet()->BuffFadeBySpellID(s->spell_id);
							}
						}

						break;
				}
			}
		} else if (s->update_type == CZUpdateType_Group) {
			Group* g = entity_list.GetGroupByID(s->update_identifier);
			if (g) {
				for (const auto& m : g->members) {
					if (
						m &&
						(
							m->IsClient() ||
							(m->IsBot() && RuleB(Zone, AllowCrossZoneSpellsOnBots)) ||
							(m->IsMerc() && RuleB(Zone, AllowCrossZoneSpellsOnMercs))
						)
					) {
						switch (s->update_subtype) {
							case CZSpellUpdateSubtype_Cast:
								m->ApplySpellBuff(s->spell_id);

								if (RuleB(Zone, AllowCrossZoneSpellsOnPets)) {
									if (m->HasPet()) {
										m->GetPet()->ApplySpellBuff(s->spell_id);
									}
								}

								break;
							case CZSpellUpdateSubtype_Remove:
								m->BuffFadeBySpellID(s->spell_id);

								if (RuleB(Zone, AllowCrossZoneSpellsOnPets)) {
									if (m->HasPet()) {
										m->GetPet()->BuffFadeBySpellID(s->spell_id);
									}
								}

								break;
						}
					}
				}
			}
		} else if (s->update_type == CZUpdateType_Raid) {
			Raid* r = entity_list.GetRaidByID(s->update_identifier);
			if (r) {
				for (const auto& m : r->members) {
					if (
						m.member &&
						(
							m.member->IsClient() ||
							(m.member->IsBot() && RuleB(Zone, AllowCrossZoneSpellsOnBots)) ||
							(m.member->IsMerc() && RuleB(Zone, AllowCrossZoneSpellsOnMercs))
						)
					){
						switch (s->update_subtype) {
							case CZSpellUpdateSubtype_Cast:
								m.member->ApplySpellBuff(s->spell_id);

								if (RuleB(Zone, AllowCrossZoneSpellsOnPets)) {
									if (m.member->HasPet()) {
										m.member->GetPet()->ApplySpellBuff(s->spell_id);
									}
								}

								break;
							case CZSpellUpdateSubtype_Remove:
								m.member->BuffFadeBySpellID(s->spell_id);

								if (RuleB(Zone, AllowCrossZoneSpellsOnPets)) {
									if (m.member->HasPet()) {
										m.member->GetPet()->BuffFadeBySpellID(s->spell_id);
									}
								}

								break;
						}
					}
				}
			}
		} else if (s->update_type == CZUpdateType_Guild) {
			for (const auto& c : entity_list.GetClientList()) {
				if (c.second->IsInGuild(s->update_identifier)) {
					switch (s->update_subtype) {
						case CZSpellUpdateSubtype_Cast:
							c.second->ApplySpellBuff(s->spell_id);

							if (RuleB(Zone, AllowCrossZoneSpellsOnBots)) {
								for (const auto& b : entity_list.GetBotListByCharacterID(c.second->CharacterID())) {
									b->ApplySpellBuff(s->spell_id);

									if (RuleB(Zone, AllowCrossZoneSpellsOnPets)) {
										if (b->HasPet()) {
											b->GetPet()->ApplySpellBuff(s->spell_id);
										}
									}
								}
							}

							if (RuleB(Zone, AllowCrossZoneSpellsOnMercs)) {
								if (c.second->GetMerc()) {
									c.second->GetMerc()->ApplySpellBuff(s->spell_id);
								}
							}

							if (RuleB(Zone, AllowCrossZoneSpellsOnPets)) {
								if (c.second->HasPet()) {
									c.second->GetPet()->ApplySpellBuff(s->spell_id);
								}
							}

							break;
						case CZSpellUpdateSubtype_Remove:
							c.second->BuffFadeBySpellID(s->spell_id);

							if (RuleB(Zone, AllowCrossZoneSpellsOnBots)) {
								for (const auto& b : entity_list.GetBotListByCharacterID(c.second->CharacterID())) {
									b->BuffFadeBySpellID(s->spell_id);

									if (RuleB(Zone, AllowCrossZoneSpellsOnPets)) {
										if (b->HasPet()) {
											b->GetPet()->BuffFadeBySpellID(s->spell_id);
										}
									}
								}
							}

							if (RuleB(Zone, AllowCrossZoneSpellsOnMercs)) {
								if (c.second->GetMerc()) {
									c.second->GetMerc()->BuffFadeBySpellID(s->spell_id);
								}
							}

							if (RuleB(Zone, AllowCrossZoneSpellsOnPets)) {
								if (c.second->HasPet()) {
									c.second->GetPet()->BuffFadeBySpellID(s->spell_id);
								}
							}

							break;
					}
				}
			}
		} else if (s->update_type == CZUpdateType_Expedition) {
			for (const auto& c : entity_list.GetClientList()) {
				if (c.second->GetExpedition() && c.second->GetExpedition()->GetID() == s->update_identifier) {
					switch (s->update_subtype) {
						case CZSpellUpdateSubtype_Cast:
							c.second->ApplySpellBuff(s->spell_id);

							if (RuleB(Zone, AllowCrossZoneSpellsOnBots)) {
								for (const auto& b : entity_list.GetBotListByCharacterID(c.second->CharacterID())) {
									b->ApplySpellBuff(s->spell_id);

									if (RuleB(Zone, AllowCrossZoneSpellsOnPets)) {
										if (b->HasPet()) {
											b->GetPet()->ApplySpellBuff(s->spell_id);
										}
									}
								}
							}

							if (RuleB(Zone, AllowCrossZoneSpellsOnMercs)) {
								if (c.second->GetMerc()) {
									c.second->GetMerc()->ApplySpellBuff(s->spell_id);
								}
							}

							if (RuleB(Zone, AllowCrossZoneSpellsOnPets)) {
								if (c.second->HasPet()) {
									c.second->GetPet()->ApplySpellBuff(s->spell_id);
								}
							}

							break;
						case CZSpellUpdateSubtype_Remove:
							c.second->BuffFadeBySpellID(s->spell_id);

							if (RuleB(Zone, AllowCrossZoneSpellsOnBots)) {
								for (const auto& b : entity_list.GetBotListByCharacterID(c.second->CharacterID())) {
									b->BuffFadeBySpellID(s->spell_id);

									if (RuleB(Zone, AllowCrossZoneSpellsOnPets)) {
										if (b->HasPet()) {
											b->GetPet()->BuffFadeBySpellID(s->spell_id);
										}
									}
								}
							}

							if (RuleB(Zone, AllowCrossZoneSpellsOnMercs)) {
								if (c.second->GetMerc()) {
									c.second->GetMerc()->BuffFadeBySpellID(s->spell_id);
								}
							}

							if (RuleB(Zone, AllowCrossZoneSpellsOnPets)) {
								if (c.second->HasPet()) {
									c.second->GetPet()->BuffFadeBySpellID(s->spell_id);
								}
							}

							break;
					}
				}
			}
		} else if (s->update_type == CZUpdateType_ClientName) {
			Client* c = entity_list.GetClientByName(s->client_name);
			if (c) {
				switch (s->update_subtype) {
					case CZSpellUpdateSubtype_Cast:
						c->ApplySpellBuff(s->spell_id);

						if (RuleB(Zone, AllowCrossZoneSpellsOnBots)) {
							for (const auto& b : entity_list.GetBotListByCharacterID(c->CharacterID())) {
								b->ApplySpellBuff(s->spell_id);

								if (RuleB(Zone, AllowCrossZoneSpellsOnPets)) {
									if (b->HasPet()) {
										b->GetPet()->ApplySpellBuff(s->spell_id);
									}
								}
							}
						}

						if (RuleB(Zone, AllowCrossZoneSpellsOnMercs)) {
							if (c->GetMerc()) {
								c->GetMerc()->ApplySpellBuff(s->spell_id);
							}
						}

						if (RuleB(Zone, AllowCrossZoneSpellsOnPets)) {
							if (c->HasPet()) {
								c->GetPet()->ApplySpellBuff(s->spell_id);
							}
						}

						break;
					case CZSpellUpdateSubtype_Remove:
						c->BuffFadeBySpellID(s->spell_id);

						if (RuleB(Zone, AllowCrossZoneSpellsOnBots)) {
							for (const auto& b : entity_list.GetBotListByCharacterID(c->CharacterID())) {
								b->BuffFadeBySpellID(s->spell_id);

								if (RuleB(Zone, AllowCrossZoneSpellsOnPets)) {
									if (b->HasPet()) {
										b->GetPet()->BuffFadeBySpellID(s->spell_id);
									}
								}
							}
						}

						if (RuleB(Zone, AllowCrossZoneSpellsOnMercs)) {
							if (c->GetMerc()) {
								c->GetMerc()->BuffFadeBySpellID(s->spell_id);
							}
						}

						if (RuleB(Zone, AllowCrossZoneSpellsOnPets)) {
							if (c->HasPet()) {
								c->GetPet()->BuffFadeBySpellID(s->spell_id);
							}
						}

						break;
				}
			}
		}
		break;
	}
	case ServerOP_CZTaskUpdate:
	{
		auto u = (CZTaskUpdate_Struct*) pack->pBuffer;

		if (u->update_type == CZUpdateType_Character) {
			Client* c = entity_list.GetClientByCharID(u->update_identifier);
			if (c) {
				switch (u->update_subtype) {
					case CZTaskUpdateSubtype_ActivityReset:
						c->ResetTaskActivity(u->task_identifier, u->task_subidentifier);
						break;
					case CZTaskUpdateSubtype_ActivityUpdate:
						c->UpdateTaskActivity(u->task_identifier, u->task_subidentifier, u->update_count);
						break;
					case CZTaskUpdateSubtype_AssignTask:
						c->AssignTask(u->task_identifier, u->task_subidentifier, u->enforce_level_requirement);
						break;
					case CZTaskUpdateSubtype_DisableTask:
						c->DisableTask(1, reinterpret_cast<int*>(&u->task_identifier));
						break;
					case CZTaskUpdateSubtype_EnableTask:
						c->EnableTask(1, reinterpret_cast<int*>(&u->task_identifier));
						break;
					case CZTaskUpdateSubtype_FailTask:
						c->FailTask(u->task_identifier);
						break;
					case CZTaskUpdateSubtype_RemoveTask:
						c->RemoveTaskByTaskID(u->task_identifier);
						break;
				}
			}
		} else if (u->update_type == CZUpdateType_Group) {
			auto client_group = entity_list.GetGroupByID(u->update_identifier);
			if (client_group) {
				for (int member_index = 0; member_index < MAX_GROUP_MEMBERS; member_index++) {
					if (client_group->members[member_index] && client_group->members[member_index]->IsClient()) {
						auto group_member = client_group->members[member_index]->CastToClient();
						switch (u->update_subtype) {
							case CZTaskUpdateSubtype_ActivityReset:
								group_member->ResetTaskActivity(u->task_identifier, u->task_subidentifier);
								break;
							case CZTaskUpdateSubtype_ActivityUpdate:
								group_member->UpdateTaskActivity(u->task_identifier, u->task_subidentifier, u->update_count);
								break;
							case CZTaskUpdateSubtype_AssignTask:
								group_member->AssignTask(u->task_identifier, u->task_subidentifier, u->enforce_level_requirement);
								break;
							case CZTaskUpdateSubtype_DisableTask:
								group_member->DisableTask(1, reinterpret_cast<int*>(&u->task_identifier));
								break;
							case CZTaskUpdateSubtype_EnableTask:
								group_member->EnableTask(1, reinterpret_cast<int*>(&u->task_identifier));
								break;
							case CZTaskUpdateSubtype_FailTask:
								group_member->FailTask(u->task_identifier);
								break;
							case CZTaskUpdateSubtype_RemoveTask:
								group_member->RemoveTaskByTaskID(u->task_identifier);
								break;
						}
					}
				}
			}
		} else if (u->update_type == CZUpdateType_Raid) {
			Raid* r = entity_list.GetRaidByID(u->update_identifier);
			if (r) {
				for (const auto& m : r->members) {
					if (m.member && m.member->IsClient()) {
						switch (u->update_subtype) {
							case CZTaskUpdateSubtype_ActivityReset:
								m.member->CastToClient()->ResetTaskActivity(u->task_identifier, u->task_subidentifier);
								break;
							case CZTaskUpdateSubtype_ActivityUpdate:
								m.member->CastToClient()->UpdateTaskActivity(u->task_identifier, u->task_subidentifier, u->update_count);
								break;
							case CZTaskUpdateSubtype_AssignTask:
								m.member->CastToClient()->AssignTask(u->task_identifier, u->task_subidentifier, u->enforce_level_requirement);
								break;
							case CZTaskUpdateSubtype_DisableTask:
								m.member->CastToClient()->DisableTask(1, reinterpret_cast<int*>(&u->task_identifier));
								break;
							case CZTaskUpdateSubtype_EnableTask:
								m.member->CastToClient()->EnableTask(1, reinterpret_cast<int*>(&u->task_identifier));
								break;
							case CZTaskUpdateSubtype_FailTask:
								m.member->CastToClient()->FailTask(u->task_identifier);
								break;
							case CZTaskUpdateSubtype_RemoveTask:
								m.member->CastToClient()->RemoveTaskByTaskID(u->task_identifier);
								break;
						}
					}
				}
			}
		} else if (u->update_type == CZUpdateType_Guild) {
			for (const auto& c : entity_list.GetClientList()) {
				if (c.second->IsInGuild(u->update_identifier)) {
					switch (u->update_subtype) {
						case CZTaskUpdateSubtype_ActivityReset:
							c.second->ResetTaskActivity(u->task_identifier, u->task_subidentifier);
							break;
						case CZTaskUpdateSubtype_ActivityUpdate:
							c.second->UpdateTaskActivity(u->task_identifier, u->task_subidentifier, u->update_count);
							break;
						case CZTaskUpdateSubtype_AssignTask:
							c.second->AssignTask(u->task_identifier, u->task_subidentifier, u->enforce_level_requirement);
							break;
						case CZTaskUpdateSubtype_DisableTask:
							c.second->DisableTask(1, reinterpret_cast<int*>(&u->task_identifier));
							break;
						case CZTaskUpdateSubtype_EnableTask:
							c.second->EnableTask(1, reinterpret_cast<int*>(&u->task_identifier));
							break;
						case CZTaskUpdateSubtype_FailTask:
							c.second->FailTask(u->task_identifier);
							break;
						case CZTaskUpdateSubtype_RemoveTask:
							c.second->RemoveTaskByTaskID(u->task_identifier);
							break;
					}
				}
			}
		} else if (u->update_type == CZUpdateType_Expedition) {
			for (const auto& c : entity_list.GetClientList()) {
				if (c.second->GetExpedition() && c.second->GetExpedition()->GetID() == u->update_identifier) {
					switch (u->update_subtype) {
						case CZTaskUpdateSubtype_ActivityReset:
							c.second->ResetTaskActivity(u->task_identifier, u->task_subidentifier);
							break;
						case CZTaskUpdateSubtype_ActivityUpdate:
							c.second->UpdateTaskActivity(u->task_identifier, u->task_subidentifier, u->update_count);
							break;
						case CZTaskUpdateSubtype_AssignTask:
							c.second->AssignTask(u->task_identifier, u->task_subidentifier, u->enforce_level_requirement);
							break;
						case CZTaskUpdateSubtype_DisableTask:
							c.second->DisableTask(1, reinterpret_cast<int*>(&u->task_identifier));
							break;
						case CZTaskUpdateSubtype_EnableTask:
							c.second->EnableTask(1, reinterpret_cast<int*>(&u->task_identifier));
							break;
						case CZTaskUpdateSubtype_FailTask:
							c.second->FailTask(u->task_identifier);
							break;
						case CZTaskUpdateSubtype_RemoveTask:
							c.second->RemoveTaskByTaskID(u->task_identifier);
							break;
					}
				}
			}
		} else if (u->update_type == CZUpdateType_ClientName) {
			Client* c = entity_list.GetClientByName(u->client_name);
			if (c) {
				switch (u->update_subtype) {
					case CZTaskUpdateSubtype_ActivityReset:
						c->ResetTaskActivity(u->task_identifier, u->task_subidentifier);
						break;
					case CZTaskUpdateSubtype_ActivityUpdate:
						c->UpdateTaskActivity(u->task_identifier, u->task_subidentifier, u->update_count);
						break;
					case CZTaskUpdateSubtype_AssignTask:
						c->AssignTask(u->task_identifier, u->task_subidentifier, u->enforce_level_requirement);
						break;
					case CZTaskUpdateSubtype_DisableTask:
						c->DisableTask(1, reinterpret_cast<int*>(&u->task_identifier));
						break;
					case CZTaskUpdateSubtype_EnableTask:
						c->EnableTask(1, reinterpret_cast<int*>(&u->task_identifier));
						break;
					case CZTaskUpdateSubtype_FailTask:
						c->FailTask(u->task_identifier);
						break;
					case CZTaskUpdateSubtype_RemoveTask:
						c->RemoveTaskByTaskID(u->task_identifier);
						break;
				}
			}
		}

		break;
	}
	case ServerOP_WWDialogueWindow:
	{
		auto s = (WWDialogueWindow_Struct*) pack->pBuffer;

		for (const auto& c : entity_list.GetClientList()) {
			if (
				c.second->Admin() >= s->min_status &&
				(c.second->Admin() <= s->max_status || s->max_status == AccountStatus::Player)
			) {
				DialogueWindow::Render(c.second, s->message);
			}
		}

		break;
	}
	case ServerOP_WWLDoNUpdate:
	{
		auto u = (WWLDoNUpdate_Struct*) pack->pBuffer;

		for (const auto& c : entity_list.GetClientList()) {
			if (
				c.second->Admin() >= u->min_status &&
				(c.second->Admin() <= u->max_status || u->max_status == AccountStatus::Player)
			) {
				switch (u->update_type) {
					case WWLDoNUpdateType_AddLoss:
						c.second->UpdateLDoNWinLoss(u->theme_id, false);
						break;
					case WWLDoNUpdateType_AddPoints:
						c.second->UpdateLDoNPoints(u->theme_id, u->points);
						break;
					case WWLDoNUpdateType_AddWin:
						c.second->UpdateLDoNWinLoss(u->theme_id, true);
						break;
					case WWLDoNUpdateType_RemoveLoss:
						c.second->UpdateLDoNWinLoss(u->theme_id, false, true);
						break;
					case WWLDoNUpdateType_RemoveWin:
						c.second->UpdateLDoNWinLoss(u->theme_id, true, true);
						break;
				}
			}
		}

		break;
	}
	case ServerOP_WWMarquee:
	{
		auto s = (WWMarquee_Struct*) pack->pBuffer;
		for (const auto& c : entity_list.GetClientList()) {
			if (
				c.second->Admin() >= s->min_status &&
				(c.second->Admin() <= s->max_status || s->max_status == AccountStatus::Player)
			) {
				c.second->SendMarqueeMessage(s->type, s->priority, s->fade_in, s->fade_out, s->duration, s->message);
			}
		}

		break;
	}
	case ServerOP_WWMessage:
	{
		auto s = (WWMessage_Struct*) pack->pBuffer;

		for (const auto& c : entity_list.GetClientList()) {
			if (
				c.second->Admin() >= s->min_status &&
				(c.second->Admin() <= s->max_status || s->max_status == AccountStatus::Player)
			) {
				c.second->Message(s->type, s->message);
			}
		}

		break;
	}
	case ServerOP_WWMove:
	{
		auto s = (WWMove_Struct*) pack->pBuffer;

		for (const auto& c : entity_list.GetClientList()) {
			if (
				c.second->Admin() >= s->min_status &&
				(c.second->Admin() <= s->max_status || s->max_status == AccountStatus::Player)
			) {
				switch (s->update_type) {
					case WWMoveUpdateType_MoveZone:
						c.second->MoveZone(s->zone_short_name);
						break;
					case WWMoveUpdateType_MoveZoneInstance:
						c.second->MoveZoneInstance(s->instance_id);
						break;
				}
			}
		}

		break;
	}
	case ServerOP_WWSetEntityVariable:
	{
		auto s = (WWSetEntityVariable_Struct*) pack->pBuffer;

		if (s->update_type == WWSetEntityVariableUpdateType_Character) {
			for (const auto& c : entity_list.GetClientList()) {
				if (
					c.second->Admin() >= s->min_status &&
					(c.second->Admin() <= s->max_status || s->max_status == AccountStatus::Player)
				) {
					c.second->SetEntityVariable(s->variable_name, s->variable_value);
				}
			}
		} else if (s->update_type == WWSetEntityVariableUpdateType_NPC) {
			for (const auto& n : entity_list.GetNPCList()) {
				n.second->SetEntityVariable(s->variable_name, s->variable_value);
			}
		}

		break;
	}
	case ServerOP_WWSignal:
	{
		auto s = (WWSignal_Struct*) pack->pBuffer;

		if (s->update_type == WWSignalUpdateType_Character) {
			for (const auto& c : entity_list.GetClientList()) {
				if (
					c.second->Admin() >= s->min_status &&
					(c.second->Admin() <= s->max_status || s->max_status == AccountStatus::Player)
				) {
					c.second->Signal(s->signal_id);
				}
			}
		} else if (s->update_type == WWSignalUpdateType_NPC) {
			for (const auto& n : entity_list.GetNPCList()) {
				n.second->SignalNPC(s->signal_id);
			}
		}
		break;
	}
	case ServerOP_WWSpell: {
		auto s = (WWSpell_Struct *) pack->pBuffer;

		for (const auto& c : entity_list.GetClientList()) {
			if (
				c.second->Admin() >= s->min_status &&
				(c.second->Admin() <= s->max_status || s->max_status == AccountStatus::Player)
			) {
				if (s->update_type == WWSpellUpdateType_Cast) {
					c.second->ApplySpellBuff(s->spell_id);

					if (RuleB(Zone, AllowCrossZoneSpellsOnBots)) {
						for (const auto& b : entity_list.GetBotListByCharacterID(c.second->CharacterID())) {
							b->ApplySpellBuff(s->spell_id);

							if (RuleB(Zone, AllowCrossZoneSpellsOnPets)) {
								if (b->HasPet()) {
									b->GetPet()->ApplySpellBuff(s->spell_id);
								}
							}
						}
					}

					if (RuleB(Zone, AllowCrossZoneSpellsOnMercs)) {
						if (c.second->GetMerc()) {
							c.second->GetMerc()->ApplySpellBuff(s->spell_id);
						}
					}

					if (RuleB(Zone, AllowCrossZoneSpellsOnPets)) {
						if (c.second->HasPet()) {
							c.second->GetPet()->ApplySpellBuff(s->spell_id);
						}
					}
				} else if (s->update_type == WWSpellUpdateType_Remove) {
					c.second->BuffFadeBySpellID(s->spell_id);

					if (RuleB(Zone, AllowCrossZoneSpellsOnBots)) {
						for (const auto& b : entity_list.GetBotListByCharacterID(c.second->CharacterID())) {
							b->BuffFadeBySpellID(s->spell_id);

							if (RuleB(Zone, AllowCrossZoneSpellsOnPets)) {
								if (b->HasPet()) {
									b->GetPet()->BuffFadeBySpellID(s->spell_id);
								}
							}
						}
					}

					if (RuleB(Zone, AllowCrossZoneSpellsOnMercs)) {
						if (c.second->GetMerc()) {
							c.second->GetMerc()->BuffFadeBySpellID(s->spell_id);
						}
					}

					if (RuleB(Zone, AllowCrossZoneSpellsOnPets)) {
						if (c.second->HasPet()) {
							c.second->GetPet()->BuffFadeBySpellID(s->spell_id);
						}
					}
				}
			}
		}
		break;
	}
	case ServerOP_WWTaskUpdate:
	{
		auto u = (WWTaskUpdate_Struct*) pack->pBuffer;

		for (const auto& c : entity_list.GetClientList()) {
			if (
				c.second->Admin() >= u->min_status &&
				(c.second->Admin() <= u->max_status || u->max_status == AccountStatus::Player)
			) {
				switch (u->update_type) {
					case WWTaskUpdateType_ActivityReset:
						c.second->ResetTaskActivity(u->task_identifier, u->task_subidentifier);
						break;
					case WWTaskUpdateType_ActivityUpdate:
						c.second->UpdateTaskActivity(u->task_identifier, u->task_subidentifier, u->update_count);
						break;
					case WWTaskUpdateType_AssignTask:
						c.second->AssignTask(u->task_identifier, u->task_subidentifier, u->enforce_level_requirement);
						break;
					case WWTaskUpdateType_DisableTask:
						c.second->DisableTask(1, reinterpret_cast<int*>(&u->task_identifier));
						break;
					case WWTaskUpdateType_EnableTask:
						c.second->EnableTask(1, reinterpret_cast<int*>(&u->task_identifier));
						break;
					case WWTaskUpdateType_FailTask:
						c.second->FailTask(u->task_identifier);
						break;
					case WWTaskUpdateType_RemoveTask:
						c.second->RemoveTaskByTaskID(u->task_identifier);
						break;
				}
			}
		}
		break;
	}
	case ServerOP_UpdateSchedulerEvents: {
		LogScheduler("Received signal from world to update");
		if (GetScheduler()) {
			m_zone_scheduler->LoadScheduledEvents();
		}

		break;
	}
	case ServerOP_HotReloadQuests:
	{
		if (!zone) {
			break;
		}

		auto *hot_reload_quests = (HotReloadQuestsStruct *) pack->pBuffer;

		LogHotReloadDetail(
			"Receiving request [HotReloadQuests] | request_zone [{}] current_zone [{}]",
			hot_reload_quests->zone_short_name,
			zone->GetShortName()
		);

		std::string request_zone_short_name = hot_reload_quests->zone_short_name;
		std::string local_zone_short_name   = zone->GetShortName();
		bool can_reload_global_script = (request_zone_short_name == "all" && RuleB(HotReload, QuestsAutoReloadGlobalScripts));

		if (request_zone_short_name == local_zone_short_name || can_reload_global_script) {
			zone->SetQuestHotReloadQueued(true);
		} else if (request_zone_short_name == "all") {
			std::string reload_quest_saylink = Saylink::Silent("#reload quest", "Locally");
			std::string reload_world_saylink = Saylink::Silent("#reload world 1", "Globally");
			for (const auto& [client_id, client] : entity_list.GetClientList()) {
				if (client->Admin() < AccountStatus::ApprenticeGuide) {
					continue;
				}

				client->Message(Chat::Yellow, fmt::format(
					"A quest, plugin, or global script has changed. Reload: [{}] [{}]",
					reload_quest_saylink,
					reload_world_saylink
				).c_str());
			}
		}
		break;
	}
	case ServerOP_ChangeSharedMem:
	{
		std::string hotfix_name = std::string((char*)pack->pBuffer);
		LogInfo("Loading items");
		if (!content_db.LoadItems(hotfix_name)) {
			LogError("Loading items failed!");
		}

		LogInfo("Loading spells");
		if (!content_db.LoadSpells(hotfix_name, &SPDAT_RECORDS, &spells)) {
			LogError("Loading spells failed!");
		}
		break;
	}
	case ServerOP_CZClientMessageString:
	{
		auto s = reinterpret_cast<CZClientMessageString_Struct*>(pack->pBuffer);

		Client* c = entity_list.GetClientByName(s->client_name);
		if (c) {
			c->MessageString(s);
		}

		break;
	}
	case ServerOP_DzCreated:
	case ServerOP_DzDeleted:
	case ServerOP_DzAddPlayer:
	case ServerOP_DzMakeLeader:
	case ServerOP_DzAddRemoveMember:
	case ServerOP_DzSwapMembers:
	case ServerOP_DzRemoveAllMembers:
	case ServerOP_DzDurationUpdate:
	case ServerOP_DzGetMemberStatuses:
	case ServerOP_DzGetBulkMemberStatuses:
	case ServerOP_DzSetCompass:
	case ServerOP_DzSetSafeReturn:
	case ServerOP_DzSetZoneIn:
	case ServerOP_DzSetSwitchID:
	case ServerOP_DzUpdateMemberStatus:
	case ServerOP_DzLeaderChanged:
	case ServerOP_DzExpireWarning:
	case ServerOP_DzMovePC:
	case ServerOP_DzLock:
	case ServerOP_DzReplayOnJoin:
	case ServerOP_DzLockout:
	case ServerOP_DzLockoutDuration:
	case ServerOP_DzCharacterLockout:
	{
		DynamicZone::HandleWorldMessage(pack);
		break;
	}
	case ServerOP_SharedTaskAcceptNewTask:
	case ServerOP_SharedTaskUpdate:
	case ServerOP_SharedTaskQuit:
	case ServerOP_SharedTaskMemberlist:
	case ServerOP_SharedTaskMemberChange:
	case ServerOP_SharedTaskInvitePlayer:
	case ServerOP_SharedTaskPurgeAllCommand:
	case ServerOP_SharedTaskFailed:
	{
		SharedTaskZoneMessaging::HandleWorldMessage(pack);
		break;
	}
	case ServerOP_GuildTributeUpdate: {
		GuildTributeUpdate* in = (GuildTributeUpdate*)pack->pBuffer;

		auto guild = guild_mgr.GetGuildByGuildID(in->guild_id);
		if (guild) {
			guild->tribute.favor          = in->favor;
			guild->tribute.id_1           = in->tribute_id_1;
			guild->tribute.id_2           = in->tribute_id_2;
			guild->tribute.id_1_tier      = in->tribute_id_1_tier;
			guild->tribute.id_2_tier      = in->tribute_id_2_tier;
			guild->tribute.time_remaining = in->time_remaining;
			guild->tribute.enabled        = in->enabled;
		}
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_GuildSendActiveTributes, sizeof(GuildTributeSendActive_Struct));
		GuildTributeSendActive_Struct* out = (GuildTributeSendActive_Struct*)outapp->pBuffer;

		out->not_used          = in->guild_id;
		out->guild_favor       = in->favor;
		out->tribute_enabled   = in->enabled;
		out->tribute_timer     = in->time_remaining;
		out->tribute_id_1      = in->tribute_id_1;
		out->tribute_id_2      = in->tribute_id_2;
		out->tribute_id_1_tier = in->tribute_id_1_tier;
		out->tribute_id_2_tier = in->tribute_id_2_tier;

		entity_list.QueueClientsGuild(outapp, in->guild_id);
		safe_delete(outapp);
		break;
	}
	case ServerOP_GuildTributeActivate: {
		GuildTributeUpdate* in = (GuildTributeUpdate*)pack->pBuffer;

		auto guild = guild_mgr.GetGuildByGuildID(in->guild_id);
		if (guild) {
			guild->tribute.favor = in->favor;
			guild->tribute.id_1 = in->tribute_id_1;
			guild->tribute.id_2 = in->tribute_id_2;
			guild->tribute.id_1_tier = in->tribute_id_1_tier;
			guild->tribute.id_2_tier = in->tribute_id_2_tier;
			guild->tribute.time_remaining = in->time_remaining;
			guild->tribute.enabled = in->enabled;
		}

		EQApplicationPacket* outapp = new EQApplicationPacket(OP_GuildTributeToggleReply, sizeof(GuildTributeSendActive_Struct));
		GuildTributeSendActive_Struct* out = (GuildTributeSendActive_Struct*)outapp->pBuffer;

		out->not_used = in->guild_id;
		out->guild_favor = in->favor;
		out->tribute_enabled = in->enabled;
		out->tribute_timer = in->time_remaining;
		out->tribute_id_1 = in->tribute_id_1;
		out->tribute_id_2 = in->tribute_id_2;
		out->tribute_id_1_tier = in->tribute_id_1_tier;
		out->tribute_id_2_tier = in->tribute_id_2_tier;

		entity_list.QueueClientsGuild(outapp, in->guild_id);
		safe_delete(outapp);

		for (auto& c : entity_list.GetClientList()) {
			if (c.second->IsInGuild(in->guild_id)) {
				c.second->DoGuildTributeUpdate();
			}
		}
		break;
	}
	case ServerOP_GuildTributeUpdateDonations: {
		auto in     = (GuildTributeUpdate *) pack->pBuffer;
		auto outapp = new EQApplicationPacket(OP_GuildOptInOut, sizeof(GuildTributeOptInOutReply_Struct));
		auto data   = (GuildTributeOptInOutReply_Struct *) outapp->pBuffer;

		data->guild_id              = in->guild_id;
		data->no_donations          = in->member_favor;
		data->tribute_toggle        = in->member_enabled ? true : false;
		data->tribute_trophy_toggle = 0; // not yet implemented
		data->time                  = in->member_time;
		data->command               = 1;
		strn0cpy(data->player_name, in->player_name, sizeof(data->player_name));

		entity_list.QueueClientsGuild(outapp, in->guild_id);
		safe_delete(outapp);

		outapp = new EQApplicationPacket(OP_GuildTributeToggleReply, sizeof(GuildTributeSendActive_Struct));
		auto out = (GuildTributeSendActive_Struct *) outapp->pBuffer;

		auto guild = guild_mgr.GetGuildByGuildID(in->guild_id);
		if (!guild) {
			safe_delete(outapp)
			return;
		}

		out->not_used          = in->guild_id;
		out->guild_favor       = guild->tribute.favor;
		out->tribute_enabled   = guild->tribute.enabled;
		out->tribute_timer     = guild->tribute.time_remaining;
		out->tribute_id_1      = guild->tribute.id_1;
		out->tribute_id_2      = guild->tribute.id_2;
		out->tribute_id_1_tier = guild->tribute.id_1_tier;
		out->tribute_id_2_tier = guild->tribute.id_2_tier;

		entity_list.QueueClientsGuild(outapp, in->guild_id);
		safe_delete(outapp)

		break;
	}
	case ServerOP_GuildTributeOptInToggle:
	{
		GuildTributeMemberToggle* in = (GuildTributeMemberToggle*)pack->pBuffer;

		EQApplicationPacket* outapp = new EQApplicationPacket(OP_GuildOptInOut, sizeof(GuildTributeOptInOutReply_Struct));
		GuildTributeOptInOutReply_Struct* data = (GuildTributeOptInOutReply_Struct*)outapp->pBuffer;

		data->guild_id = in->guild_id;
		strn0cpy(data->player_name, in->player_name, sizeof(data->player_name));
		data->no_donations = in->no_donations;
		data->tribute_toggle = in->tribute_toggle ? true : false;
		data->tribute_trophy_toggle = 0; //not yet implemented
		data->time = in->member_last_donated;
		data->command = in->command;

		entity_list.QueueClientsGuild(outapp, in->guild_id);
		safe_delete(outapp);

		auto guild = guild_mgr.GetGuildByGuildID(in->guild_id);
		auto client = entity_list.GetClientByCharID(in->char_id);
		if (guild && client) {
			client->SetGuildTributeOptIn(in->tribute_toggle ? true : false);

			EQApplicationPacket* outapp = new EQApplicationPacket(OP_GuildTributeToggleReply, sizeof(GuildTributeSendActive_Struct));
			GuildTributeSendActive_Struct* out = (GuildTributeSendActive_Struct*)outapp->pBuffer;

			out->not_used = in->guild_id;
			out->guild_favor = guild->tribute.favor;
			out->tribute_enabled = guild->tribute.enabled;
			out->tribute_timer = guild->tribute.time_remaining;
			out->tribute_id_1 = guild->tribute.id_1;
			out->tribute_id_2 = guild->tribute.id_2;
			out->tribute_id_1_tier = guild->tribute.id_1_tier;
			out->tribute_id_2_tier = guild->tribute.id_2_tier;
			client->QueuePacket(outapp);
			safe_delete(outapp);
			//send deactivate and then activate

			client->DoGuildTributeUpdate();
		}

		break;
	}
	case ServerOP_GuildTributeFavAndTimer:
	{
		GuildTributeFavorTimer_Struct* in = (GuildTributeFavorTimer_Struct*)pack->pBuffer;

		auto guild = guild_mgr.GetGuildByGuildID(in->guild_id);
		if (guild) {
			guild->tribute.favor = in->guild_favor;
			guild->tribute.time_remaining = in->tribute_timer;

			auto outapp = new EQApplicationPacket(OP_GuildTributeFavorAndTimer, sizeof(GuildTributeFavorTimer_Struct));
			GuildTributeFavorTimer_Struct* gtsa = (GuildTributeFavorTimer_Struct*)outapp->pBuffer;

			gtsa->guild_id = in->guild_id;
			gtsa->guild_favor = guild->tribute.favor;
			gtsa->tribute_timer = guild->tribute.time_remaining;
			gtsa->trophy_timer = 0; //not yet implemented

			entity_list.QueueClientsGuild(outapp, in->guild_id);
			safe_delete(outapp);
		}
		break;
	}
	case ServerOP_RequestGuildActiveTributes:
	{
		GuildTributeUpdate* in = (GuildTributeUpdate*)pack->pBuffer;
		auto guild = guild_mgr.GetGuildByGuildID(in->guild_id);

		if (guild) {
			auto outapp = new EQApplicationPacket(OP_GuildSendActiveTributes, sizeof(GuildTributeSendActive_Struct));
			GuildTributeSendActive_Struct* gtsa = (GuildTributeSendActive_Struct*)outapp->pBuffer;

			guild->tribute.enabled = in->enabled;
			guild->tribute.favor = in->favor;
			guild->tribute.id_1 = in->tribute_id_1;
			guild->tribute.id_2 = in->tribute_id_2;
			guild->tribute.id_1_tier = in->tribute_id_1_tier;
			guild->tribute.id_2_tier = in->tribute_id_2_tier;
			guild->tribute.time_remaining = in->time_remaining;

			gtsa->guild_favor = guild->tribute.favor;
			gtsa->tribute_timer = guild->tribute.time_remaining;
			gtsa->tribute_enabled = guild->tribute.enabled;
			gtsa->tribute_id_1 = guild->tribute.id_1;
			gtsa->tribute_id_1_tier = guild->tribute.id_1_tier;
			gtsa->tribute_id_2 = guild->tribute.id_2;
			gtsa->tribute_id_2_tier = guild->tribute.id_2_tier;

			entity_list.QueueClientsGuild(outapp, in->guild_id);
			safe_delete(outapp);
		}
		break;
	}
	case ServerOP_RequestGuildFavorAndTimer:
	{
		GuildTributeFavorTimer_Struct* in = (GuildTributeFavorTimer_Struct*)pack->pBuffer;

		auto guild = guild_mgr.GetGuildByGuildID(in->guild_id);
		if (guild) {
			guild->tribute.favor = in->guild_favor;
			guild->tribute.time_remaining = in->tribute_timer;

			auto outapp = new EQApplicationPacket(OP_GuildTributeFavorAndTimer, sizeof(GuildTributeFavorTimer_Struct));
			GuildTributeFavorTimer_Struct* gtsa = (GuildTributeFavorTimer_Struct*)outapp->pBuffer;

			gtsa->guild_id = in->guild_id;
			gtsa->guild_favor = guild->tribute.favor;
			gtsa->tribute_timer = guild->tribute.time_remaining;
			gtsa->trophy_timer = 0; //not yet implemented

			entity_list.QueueClientsGuild(outapp, in->guild_id);
			safe_delete(outapp);
		}
		break;
	}
		case ServerOP_ParcelDelivery: {
			auto in = (Parcel_Struct *) pack->pBuffer;

			if (strlen(in->send_to) == 0) {
				LogError(
					"ServerOP_ParcelDelivery pack received with incorrect character_name of {}.",
					in->send_to
				);
				return;
			}

			for (auto const &c: entity_list.GetClientList()) {
				if (strcasecmp(c.second->GetCleanName(), in->send_to) == 0) {
					c.second->MessageString(
						Chat::Yellow,
						PARCEL_DELIVERY_ARRIVED
					);
					c.second->SendParcelStatus();
					if (c.second->GetEngagedWithParcelMerchant()) {
						c.second->SendParcel(*in);
					}
					return;
				}
			}

			break;
		}
		case ServerOP_ParcelPrune: {
			for (auto const &c: entity_list.GetClientList()) {
				if (c.second->GetEngagedWithParcelMerchant()) {
					c.second->Message(
						Chat::Red,
						"Parcel data has been updated.  Please re-open the Merchant Window."
					);
					c.second->SetEngagedWithParcelMerchant(false);
					c.second->DoParcelCancel();

					auto out = new EQApplicationPacket(OP_ShopEndConfirm);
					c.second->QueuePacket(out);
					safe_delete(out);
					return;
				}
			}
			break;
		}
		case ServerOP_TraderMessaging: {
			auto            in = (TraderMessaging_Struct *) pack->pBuffer;
			for (auto const &c: entity_list.GetClientList()) {
				if (c.second->ClientVersion() >= EQ::versions::ClientVersion::RoF2) {
					auto outapp           = new EQApplicationPacket(OP_BecomeTrader, sizeof(BecomeTrader_Struct));
					auto out              = (BecomeTrader_Struct *) outapp->pBuffer;

					out->entity_id        = in->entity_id;
					out->zone_id          = in->zone_id;
					out->zone_instance_id = in->instance_id;
					out->trader_id        = in->trader_id;
					strn0cpy(out->trader_name, in->trader_name, sizeof(out->trader_name));

					switch (in->action) {
						case TraderOn: {
							out->action = AddTraderToBazaarWindow;
							if (c.second->GetTraderCount() <
								EQ::constants::StaticLookup(c.second->ClientVersion())->BazaarTraderLimit) {
								if (RuleB(Bazaar, UseAlternateBazaarSearch)) {
									if (out->zone_id == Zones::BAZAAR &&
										out->zone_instance_id == c.second->GetInstanceID()) {
										c.second->IncrementTraderCount();
										c.second->QueuePacket(outapp, true, Mob::CLIENT_CONNECTED);
									}
								}
								else {
									c.second->IncrementTraderCount();
									c.second->QueuePacket(outapp, true, Mob::CLIENT_CONNECTED);
								}
							}
							break;
						}
						case TraderOff: {
							out->action = RemoveTraderFromBazaarWindow;
							if (c.second->GetTraderCount() <=
								EQ::constants::StaticLookup(c.second->ClientVersion())->BazaarTraderLimit) {
								if (RuleB(Bazaar, UseAlternateBazaarSearch)) {
									if (out->zone_id == Zones::BAZAAR &&
										out->zone_instance_id == c.second->GetInstanceID()) {
										c.second->DecrementTraderCount();
										c.second->QueuePacket(outapp, true, Mob::CLIENT_CONNECTED);
									}
								}
								else {
									c.second->DecrementTraderCount();
									c.second->QueuePacket(outapp, true, Mob::CLIENT_CONNECTED);
								}
							}
							break;
						}
						default: {
							out->action = 0;
							c.second->QueuePacket(outapp, true, Mob::CLIENT_CONNECTED);
						}
					}

					safe_delete(outapp);
				}
				if (zone && zone->GetZoneID() == Zones::BAZAAR && in->instance_id == zone->GetInstanceID()) {
					if (in->action == TraderOn) {
						c.second->SendBecomeTrader(TraderOn, in->entity_id);
					}
					else {
						c.second->SendBecomeTrader(TraderOff, in->entity_id);
					}
				}
			}
			break;
		}
		case ServerOP_BazaarPurchase: {
			auto in        = (BazaarPurchaseMessaging_Struct *) pack->pBuffer;
			auto trader_pc = entity_list.GetClientByCharID(in->trader_buy_struct.trader_id);
			if (!trader_pc) {
				LogTrading("Request trader_id <red>[{}] could not be found in zone_id <red>[{}]",
						   in->trader_buy_struct.trader_id,
						   zone->GetZoneID()
				);
				return;
			}

			if (trader_pc->IsThereACustomer()) {
				auto customer = entity_list.GetClientByID(trader_pc->GetCustomerID());
				if (customer) {
					customer->CancelTraderTradeWindow();
				}
			}

			auto item_sn = Strings::ToUnsignedBigInt(in->trader_buy_struct.serial_number);
			auto outapp  = std::make_unique<EQApplicationPacket>(OP_Trader, static_cast<uint32>(sizeof(TraderBuy_Struct)));
			auto data    = (TraderBuy_Struct *) outapp->pBuffer;

			memcpy(data, &in->trader_buy_struct, sizeof(TraderBuy_Struct));

			if (trader_pc->ClientVersion() < EQ::versions::ClientVersion::RoF) {
				data->price = in->trader_buy_struct.price * in->trader_buy_struct.quantity;
			}

			TraderRepository::UpdateActiveTransaction(database, in->id, false);

			auto item = trader_pc->FindTraderItemBySerialNumber(item_sn);

			if (item && PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::TRADER_SELL)) {
				auto e = PlayerEvent::TraderSellEvent{
					.item_id              = item ? item->GetID() : 0,
					.augment_1_id         = item->GetAugmentItemID(0),
					.augment_2_id         = item->GetAugmentItemID(1),
					.augment_3_id         = item->GetAugmentItemID(2),
					.augment_4_id         = item->GetAugmentItemID(3),
					.augment_5_id         = item->GetAugmentItemID(4),
					.augment_6_id         = item->GetAugmentItemID(5),
					.item_name            = in->trader_buy_struct.item_name,
					.buyer_id             = in->buyer_id,
					.buyer_name           = in->trader_buy_struct.buyer_name,
					.price                = in->trader_buy_struct.price,
					.quantity             = in->trader_buy_struct.quantity,
					.charges              = item ? item->IsStackable() ? 1 : item->GetCharges() : 0,
					.total_cost           = (in->trader_buy_struct.price * in->trader_buy_struct.quantity),
					.player_money_balance = trader_pc->GetCarriedMoney(),
				};
				RecordPlayerEventLogWithClient(trader_pc, PlayerEvent::TRADER_SELL, e);
			}

			trader_pc->RemoveItemBySerialNumber(item_sn, in->trader_buy_struct.quantity);
			trader_pc->AddMoneyToPP(in->trader_buy_struct.price * in->trader_buy_struct.quantity, true);
			trader_pc->QueuePacket(outapp.get());

			break;
		}
		case ServerOP_BuyerMessaging: {
			auto in = (BuyerMessaging_Struct *) pack->pBuffer;

			switch (in->action) {
				case Barter_AddToBarterWindow: {
					auto outapp = std::make_unique<EQApplicationPacket>(
						OP_Barter,
						static_cast<uint32>(sizeof(BuyerAddBuyertoBarterWindow_Struct))
					);
					auto emu    = (BuyerAddBuyertoBarterWindow_Struct *) outapp->pBuffer;

					emu->action          = Barter_AddToBarterWindow;
					emu->buyer_entity_id = in->buyer_entity_id;
					emu->buyer_id        = in->buyer_id;
					emu->zone_id         = in->zone_id;
					strn0cpy(emu->buyer_name, in->buyer_name, sizeof(emu->buyer_name));

					entity_list.QueueClients(nullptr, outapp.get());

					break;
				}
				case Barter_RemoveFromBarterWindow: {
					auto outapp = std::make_unique<EQApplicationPacket>(
						OP_Barter,
						static_cast<uint32>(sizeof(BuyerRemoveBuyerFromBarterWindow_Struct))
					);
					auto emu    = (BuyerRemoveBuyerFromBarterWindow_Struct *) outapp->pBuffer;

					emu->action   = Barter_RemoveFromBarterWindow;
					emu->buyer_id = in->buyer_id;

					entity_list.QueueClients(nullptr, outapp.get());

					break;
				}
				case Barter_FailedTransaction: {
					auto seller = entity_list.GetClientByID(in->seller_entity_id);
					auto buyer  = entity_list.GetClientByID(in->buyer_entity_id);

					BuyerLineSellItem_Struct sell_line{};
					sell_line.item_id         = in->buy_item_id;
					sell_line.item_quantity   = in->buy_item_qty;
					sell_line.item_cost       = in->buy_item_cost;
					sell_line.seller_name     = in->seller_name;
					sell_line.buyer_name      = in->buyer_name;
					sell_line.seller_quantity = in->seller_quantity;
					sell_line.slot            = in->slot;
					sell_line.purchase_method = in->purchase_method;
					strn0cpy(sell_line.item_name, in->item_name, sizeof(sell_line.item_name));

					uint64 total_cost = (uint64) sell_line.item_cost * (uint64) sell_line.seller_quantity;
					std::unique_ptr<EQ::ItemInstance> inst(database.CreateItem(in->buy_item_id, in->seller_quantity));

					switch (in->sub_action) {
						case Barter_FailedBuyerChecks:
						case Barter_FailedSellerChecks: {
							if (seller) {
								LogTradingDetail("Significant barter transaction failure.");
								seller->Message(
									Chat::Red,
									"Significant barter transaction error.  Transaction rolled back."
								);
								seller->SendBarterBuyerClientMessage(
									sell_line,
									Barter_SellerTransactionComplete,
									Barter_Failure,
									Barter_Failure
								);

								if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::BARTER_TRANSACTION)) {
									PlayerEvent::BarterTransaction e{};
									e.status        = "Failed Barter Transaction";
									e.item_id       = sell_line.item_id;
									e.item_quantity = sell_line.seller_quantity;
									e.item_name     = sell_line.item_name;
									e.trade_items   = sell_line.trade_items;
									for (auto &i: e.trade_items) {
										i *= sell_line.seller_quantity;
									}
									e.total_cost  = (uint64) sell_line.item_cost * (uint64) in->seller_quantity;
									e.buyer_name  = sell_line.buyer_name;
									e.seller_name = sell_line.seller_name;
									RecordPlayerEventLogWithClient(seller, PlayerEvent::BARTER_TRANSACTION, e);
								}
							}

							if (buyer) {
								LogError("Significant barter transaction failure.  Replacing {} and {} {} to {}",
										 buyer->DetermineMoneyString(total_cost),
										 sell_line.seller_quantity,
										 sell_line.item_name,
										 buyer->GetCleanName());
								buyer->AddMoneyToPP(total_cost, true);
								buyer->RemoveItem(sell_line.item_id, sell_line.seller_quantity);

								buyer->Message(
									Chat::Red,
									"Significant barter transaction error.  Transaction rolled back."
								);
								buyer->SendBarterBuyerClientMessage(
									sell_line,
									Barter_BuyerTransactionComplete,
									Barter_Failure,
									Barter_Failure
								);

								if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::BARTER_TRANSACTION)) {
									PlayerEvent::BarterTransaction e{};
									e.status        = "Failed Barter Transaction";
									e.item_id       = sell_line.item_id;
									e.item_quantity = sell_line.seller_quantity;
									e.item_name     = sell_line.item_name;
									e.trade_items   = sell_line.trade_items;
									for (auto &i: e.trade_items) {
										i *= sell_line.seller_quantity;
									}
									e.total_cost  = (uint64) sell_line.item_cost * (uint64) in->seller_quantity;
									e.buyer_name  = sell_line.buyer_name;
									e.seller_name = sell_line.seller_name;
									RecordPlayerEventLogWithClient(buyer, PlayerEvent::BARTER_TRANSACTION, e);
								}
							}
							break;
						}
						default: {
							if (seller) {
								seller->SendBarterBuyerClientMessage(
									sell_line,
									Barter_SellerTransactionComplete,
									Barter_Failure,
									Barter_Failure
								);
							}

							if (buyer) {
								buyer->SendBarterBuyerClientMessage(
									sell_line,
									Barter_BuyerTransactionComplete,
									Barter_Failure,
									Barter_Failure
								);
							}
						}
					}
					break;
				}
				case Barter_SellItem: {
					auto buyer = entity_list.GetClientByID(in->buyer_entity_id);
					if (!buyer) {
						in->action     = Barter_FailedTransaction;
						in->sub_action = Barter_BuyerCouldNotBeFound;
						worldserver.SendPacket(pack);
						return;
					}
					if (buyer->IsThereACustomer()) {
						auto customer = entity_list.GetClientByID(buyer->GetCustomerID());
						if (customer) {
							customer->CancelBuyerTradeWindow();
						}
					}

					BuyerLineSellItem_Struct sell_line{};
					sell_line.item_id         = in->buy_item_id;
					sell_line.item_quantity   = in->buy_item_qty;
					sell_line.item_cost       = in->buy_item_cost;
					sell_line.seller_name     = in->seller_name;
					sell_line.buyer_name      = in->buyer_name;
					sell_line.buyer_entity_id = in->buyer_entity_id;
					sell_line.seller_quantity = in->seller_quantity;
					sell_line.slot            = in->slot;
					strn0cpy(sell_line.item_name, in->item_name, sizeof(sell_line.item_name));

					if (!buyer->DoBarterBuyerChecks(sell_line)) {
						in->action     = Barter_FailedTransaction;
						in->sub_action = Barter_FailedBuyerChecks;
						worldserver.SendPacket(pack);
						break;
					}

					BuyerLineSellItem_Struct blis{};
					blis.enabled         = 1;
					blis.item_toggle     = 1;
					blis.item_cost       = in->buy_item_cost;
					blis.item_id         = in->buy_item_id;
					blis.item_quantity   = in->buy_item_qty;
					blis.item_icon       = in->buy_item_icon;
					blis.slot            = in->slot;
					blis.seller_quantity = in->seller_quantity;
					blis.buyer_entity_id = in->buyer_entity_id;
					strn0cpy(blis.item_name, in->item_name, sizeof(blis.item_name));

					uint64 total_cost = (uint64) sell_line.item_cost * (uint64) sell_line.seller_quantity;
					std::unique_ptr<EQ::ItemInstance> inst(database.CreateItem(in->buy_item_id, in->seller_quantity));

					if (inst->IsStackable()) {
						if (!buyer->PutItemInInventoryWithStacking(inst.get())) {
							buyer->Message(Chat::Red, "Error putting item in your inventory.");
							buyer->AddMoneyToPP(total_cost, true);
							in->action     = Barter_FailedTransaction;
							in->sub_action = Barter_FailedBuyerChecks;
							worldserver.SendPacket(pack);
							break;
						}
					}
					else {
						for (int i = 1; i <= sell_line.seller_quantity; i++) {
							inst->SetCharges(1);
							if (!buyer->PutItemInInventoryWithStacking(inst.get())) {
								buyer->Message(Chat::Red, "Error putting item in your inventory.");
								buyer->AddMoneyToPP(total_cost, true);
								in->action     = Barter_FailedTransaction;
								in->sub_action = Barter_FailedBuyerChecks;
								worldserver.SendPacket(pack);
								goto exit_loop;
							}
						}
					}

					if (!buyer->TakeMoneyFromPP(total_cost, false)) {
						in->action     = Barter_FailedTransaction;
						in->sub_action = Barter_FailedBuyerChecks;
						worldserver.SendPacket(pack);
						break;
					}

					buyer->SendWindowUpdatesToSellerAndBuyer(blis);
					buyer->SendBarterBuyerClientMessage(
						sell_line,
						Barter_BuyerTransactionComplete,
						Barter_Success,
						Barter_Success
					);

					if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::BARTER_TRANSACTION)) {
						PlayerEvent::BarterTransaction e{};
						e.status        = "Successful Barter Transaction";
						e.item_id       = sell_line.item_id;
						e.item_quantity = sell_line.seller_quantity;
						e.item_name     = sell_line.item_name;
						e.trade_items   = sell_line.trade_items;
						for (auto &i: e.trade_items) {
							i *= sell_line.seller_quantity;
						}
						e.total_cost  = (uint64) sell_line.item_cost * (uint64) in->seller_quantity;
						e.buyer_name  = sell_line.buyer_name;
						e.seller_name = sell_line.seller_name;
						RecordPlayerEventLogWithClient(buyer, PlayerEvent::BARTER_TRANSACTION, e);
					}

					in->action = Barter_BuyerTransactionComplete;
					worldserver.SendPacket(pack);

					exit_loop:
					break;
				}
				case Barter_BuyerTransactionComplete: {
					auto seller = entity_list.GetClientByID(in->seller_entity_id);
					if (!seller) {
						in->action     = Barter_FailedTransaction;
						in->sub_action = Barter_SellerCouldNotBeFound;
						worldserver.SendPacket(pack);
						return;
					}

					BuyerLineSellItem_Struct sell_line{};
					sell_line.item_id         = in->buy_item_id;
					sell_line.item_quantity   = in->buy_item_qty;
					sell_line.item_cost       = in->buy_item_cost;
					sell_line.seller_name     = in->seller_name;
					sell_line.buyer_name      = in->buyer_name;
					sell_line.seller_quantity = in->seller_quantity;
					sell_line.slot            = in->slot;
					strn0cpy(sell_line.item_name, in->item_name, sizeof(sell_line.item_name));

					if (!seller->DoBarterSellerChecks(sell_line)) {
						in->action = Barter_FailedTransaction;
						in->action = Barter_FailedSellerChecks;
						worldserver.SendPacket(pack);
						return;
					}

					uint64 total_cost = (uint64) sell_line.item_cost * (uint64) sell_line.seller_quantity;
					seller->RemoveItem(in->buy_item_id, in->seller_quantity);
					seller->AddMoneyToPP(total_cost, false);
					seller->SendBarterBuyerClientMessage(
						sell_line,
						Barter_SellerTransactionComplete,
						Barter_Success,
						Barter_Success
					);

					if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::BARTER_TRANSACTION)) {
						PlayerEvent::BarterTransaction e{};
						e.status        = "Successful Barter Transaction";
						e.item_id       = sell_line.item_id;
						e.item_quantity = sell_line.seller_quantity;
						e.item_name     = sell_line.item_name;
						e.trade_items   = sell_line.trade_items;
						for (auto &i: e.trade_items) {
							i *= sell_line.seller_quantity;
						}
						e.total_cost  = (uint64) sell_line.item_cost * (uint64) in->seller_quantity;
						e.buyer_name  = sell_line.buyer_name;
						e.seller_name = sell_line.seller_name;
						RecordPlayerEventLogWithClient(seller, PlayerEvent::BARTER_TRANSACTION, e);
					}

					break;
				}
			}
		}
		default: {
			LogInfo("Unknown ZS Opcode [{}] size [{}]", (int) pack->opcode, pack->size);
			break;
		}
	}
}

bool WorldServer::SendChannelMessage(Client* from, const char* to, uint8 chan_num, uint32 guilddbid, uint8 language, uint8 lang_skill, const char* message, ...) {
	if (!worldserver.Connected()) {
		return false;
	}

	va_list argptr;
	auto length = strlen(message) + 1;
	char* buffer = new char[length];

	va_start(argptr, message);
	vsnprintf(buffer, length, message, argptr);
	va_end(argptr);
	buffer[length - 1] = '\0';

	auto pack = new ServerPacket(ServerOP_ChannelMessage, sizeof(ServerChannelMessage_Struct) + length);
	ServerChannelMessage_Struct* scm = (ServerChannelMessage_Struct*)pack->pBuffer;

	if (from == 0) {
		strcpy(scm->from, "ZServer");
		scm->fromadmin = AccountStatus::Player;
	}
	else {
		strcpy(scm->from, from->GetName());
		scm->fromadmin = from->Admin();
	}
	if (to == 0) {
		scm->to[0] = 0;
		scm->deliverto[0] = '\0';
	}
	else {
		strn0cpy(scm->to, to, sizeof(scm->to));
		strn0cpy(scm->deliverto, to, sizeof(scm->deliverto));
	}
	scm->noreply = false;
	scm->chan_num = chan_num;
	scm->guilddbid = guilddbid;
	scm->language = language;
	scm->lang_skill = lang_skill;
	scm->queued = 0;
	strcpy(scm->message, buffer);

	bool ret = SendPacket(pack);
	safe_delete(pack);
	safe_delete_array(buffer);
	return ret;
}

bool WorldServer::SendEmoteMessage(const char* to, uint32 to_guilddbid, uint32 type, const char* message, ...) {
	va_list argptr;
	char buffer[4096] = { 0 };

	va_start(argptr, message);
	vsnprintf(buffer, sizeof(buffer) - 1, message, argptr);
	va_end(argptr);

	return SendEmoteMessage(
		to,
		to_guilddbid,
		AccountStatus::Player,
		type,
		buffer
	);
}

bool WorldServer::SendEmoteMessage(const char* to, uint32 to_guilddbid, int16 to_minstatus, uint32 type, const char* message, ...) {
	va_list argptr;
	char buffer[4096] = { 0 };

	va_start(argptr, message);
	vsnprintf(buffer, sizeof(buffer) - 1, message, argptr);
	va_end(argptr);

	if (!Connected() && to == 0) {
		entity_list.MessageStatus(
			to_guilddbid,
			to_minstatus,
			type,
			buffer
		);
		return false;
	}

	auto pack = new ServerPacket(ServerOP_EmoteMessage, sizeof(ServerEmoteMessage_Struct) + strlen(buffer) + 1);
	ServerEmoteMessage_Struct* sem = (ServerEmoteMessage_Struct*)pack->pBuffer;
	sem->type = type;
	if (to != 0)
		strcpy(sem->to, to);
	sem->guilddbid = to_guilddbid;
	sem->minstatus = to_minstatus;
	strcpy(sem->message, buffer);

	bool ret = SendPacket(pack);
	safe_delete(pack);
	return ret;
}

bool WorldServer::SendVoiceMacro(Client* From, uint32 Type, char* Target, uint32 MacroNumber, uint32 GroupOrRaidID) {

	if (!worldserver.Connected() || !From)
		return false;

	auto pack = new ServerPacket(ServerOP_VoiceMacro, sizeof(ServerVoiceMacro_Struct));

	ServerVoiceMacro_Struct* svm = (ServerVoiceMacro_Struct*)pack->pBuffer;

	strcpy(svm->From, From->GetName());

	switch (Type) {

	case VoiceMacroTell:
		strcpy(svm->To, Target);
		break;

	case VoiceMacroGroup:
		svm->GroupID = GroupOrRaidID;
		break;

	case VoiceMacroRaid:
		svm->RaidID = GroupOrRaidID;
		break;
	}

	svm->Type = Type;

	uint16 player_race = GetPlayerRaceValue(From->GetRace());

	if (player_race == Race::Doug) {
		player_race = From->GetBaseRace();
	}

	svm->Voice = (player_race * 2) + From->GetGender();

	svm->MacroNumber = MacroNumber;

	bool Ret = SendPacket(pack);

	safe_delete(pack);

	return Ret;
}

bool WorldServer::RezzPlayer(EQApplicationPacket* rpack, uint32 rezzexp, uint32 dbid, uint16 opcode)
{
	LogSpells("rezzexp is [{}] (0 is normal for RezzComplete", rezzexp);
	auto pack = new ServerPacket(ServerOP_RezzPlayer, sizeof(RezzPlayer_Struct));
	RezzPlayer_Struct* sem = (RezzPlayer_Struct*)pack->pBuffer;
	sem->rezzopcode = opcode;
	sem->rez = *(Resurrect_Struct*)rpack->pBuffer;
	sem->exp = rezzexp;
	sem->dbid = dbid;
	bool ret = SendPacket(pack);
	if (ret)
		LogSpells("Sending player rezz packet to world spellid:[{}]", sem->rez.spellid);
	else
		LogSpells("NOT Sending player rezz packet to world");

	safe_delete(pack);
	return ret;
}

void WorldServer::SendReloadTasks(uint8 reload_type, uint32 task_id) {
	SendReload(ServerReload::Type::Tasks);
}

uint32 WorldServer::NextGroupID() {
	//this system wastes a lot of potential group IDs (~5%), but
	//if you are creating 2 billion groups in 1 run of the emu,
	//something else is wrong...
	if (cur_groupid >= last_groupid) {
		//this is an error... This means that 50 groups were created before
		//1 packet could make the zone->world->zone trip... so let it error.
		LogError("Ran out of group IDs before the server sent us more");
		return(0);
	}
	if (cur_groupid > (last_groupid - /*50*/995)) {
		//running low, request more
		auto pack = new ServerPacket(ServerOP_GroupIDReq);
		SendPacket(pack);
		safe_delete(pack);
	}
	printf("Handing out new group id %d\n", cur_groupid);
	return(cur_groupid++);
}

void WorldServer::UpdateLFP(uint32 LeaderID, uint8 Action, uint8 MatchFilter, uint32 FromLevel, uint32 ToLevel, uint32 Classes,
	const char *Comments, GroupLFPMemberEntry *LFPMembers) {

	auto pack = new ServerPacket(ServerOP_LFPUpdate, sizeof(ServerLFPUpdate_Struct));
	ServerLFPUpdate_Struct* sus = (ServerLFPUpdate_Struct*)pack->pBuffer;

	sus->LeaderID = LeaderID;
	sus->Action = Action;
	sus->MatchFilter = MatchFilter;
	sus->FromLevel = FromLevel;
	sus->ToLevel = ToLevel;
	sus->Classes = Classes;
	strcpy(sus->Comments, Comments);
	memcpy(sus->Members, LFPMembers, sizeof(sus->Members));
	SendPacket(pack);
	safe_delete(pack);

}

void WorldServer::UpdateLFP(uint32 LeaderID, GroupLFPMemberEntry *LFPMembers) {

	UpdateLFP(LeaderID, LFPMemberUpdate, 0, 0, 0, 0, "", LFPMembers);
}

void WorldServer::StopLFP(uint32 LeaderID) {

	GroupLFPMemberEntry LFPMembers[MAX_GROUP_MEMBERS];
	UpdateLFP(LeaderID, LFPOff, 0, 0, 0, 0, "", LFPMembers);
}

void WorldServer::HandleLFGMatches(ServerPacket *pack) {

	char *Buffer = (char *)pack->pBuffer;

	int PacketLength = 4;

	int Entries = (pack->size - 4) / sizeof(ServerLFGMatchesResponse_Struct);

	uint32 EntityID = VARSTRUCT_DECODE_TYPE(uint32, Buffer);

	Client* client = entity_list.GetClientByID(EntityID);

	if (client) {
		ServerLFGMatchesResponse_Struct* smrs = (ServerLFGMatchesResponse_Struct*)Buffer;

		for (int i = 0; i<Entries; i++) {
			PacketLength = PacketLength + 12 + strlen(smrs->Name) + strlen(smrs->Comments);
			smrs++;
		}

		auto outapp = new EQApplicationPacket(OP_LFGGetMatchesResponse, PacketLength);

		smrs = (ServerLFGMatchesResponse_Struct*)Buffer;

		char *OutBuffer = (char *)outapp->pBuffer;

		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0x00074af); // Unknown

		for (int i = 0; i<Entries; i++) {
			VARSTRUCT_ENCODE_STRING(OutBuffer, smrs->Comments);
			VARSTRUCT_ENCODE_STRING(OutBuffer, smrs->Name);
			VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, smrs->Class_);
			VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, smrs->Level);
			VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, smrs->Zone);
			VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, smrs->GuildID);
			VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, smrs->Anon);
			smrs++;
		}
		client->QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void WorldServer::HandleLFPMatches(ServerPacket *pack) {

	char *Buffer = (char *)pack->pBuffer;

	int PacketLength = 4;

	int Entries = (pack->size - 4) / sizeof(ServerLFPMatchesResponse_Struct);

	uint32 EntityID = VARSTRUCT_DECODE_TYPE(uint32, Buffer);

	ServerLFPMatchesResponse_Struct* smrs = (ServerLFPMatchesResponse_Struct*)Buffer;

	Client* client = entity_list.GetClientByID(EntityID);

	if (client) {
		for (int i = 0; i<Entries; i++) {
			PacketLength += strlen(smrs->Comments) + 11;
			for (unsigned int j = 0; j<MAX_GROUP_MEMBERS; j++) {

				if (smrs->Members[j].Name[0] != '\0')
					PacketLength += strlen(smrs->Members[j].Name) + 9;
			}
			smrs++;
		}
		auto outapp = new EQApplicationPacket(OP_LFPGetMatchesResponse, PacketLength);

		smrs = (ServerLFPMatchesResponse_Struct*)Buffer;

		char *OutBuffer = (char *)outapp->pBuffer;

		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0x00074af); // Unknown

		for (int i = 0; i<Entries; i++) {

			int MemberCount = 0;

			for (unsigned int j = 0; j<MAX_GROUP_MEMBERS; j++)
				if (smrs->Members[j].Name[0] != '\0')
					MemberCount++;

			VARSTRUCT_ENCODE_STRING(OutBuffer, smrs->Comments);
			VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, smrs->FromLevel);
			VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, smrs->ToLevel);
			VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, smrs->Classes);
			VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, MemberCount);

			for (unsigned int j = 0; j<MAX_GROUP_MEMBERS; j++) {
				if (smrs->Members[j].Name[0] != '\0') {
					VARSTRUCT_ENCODE_STRING(OutBuffer, smrs->Members[j].Name);
					VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, smrs->Members[j].Class);
					VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, smrs->Members[j].Level);
					VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, smrs->Members[j].Zone);
					VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, smrs->Members[j].GuildID);
				}
			}
			smrs++;
		}
		client->QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void WorldServer::RequestTellQueue(const char *who)
{
	if (!who)
		return;

	auto pack = new ServerPacket(ServerOP_RequestTellQueue, sizeof(ServerRequestTellQueue_Struct));
	ServerRequestTellQueue_Struct* rtq = (ServerRequestTellQueue_Struct*)pack->pBuffer;

	strn0cpy(rtq->name, who, sizeof(rtq->name));

	SendPacket(pack);
	safe_delete(pack);
	return;
}

ZoneEventScheduler *WorldServer::GetScheduler() const
{
	return m_zone_scheduler;
}

void WorldServer::SetScheduler(ZoneEventScheduler *scheduler)
{
	WorldServer::m_zone_scheduler = scheduler;
}

void WorldServer::SendReload(ServerReload::Type type, bool is_global)
{
	static auto pack = ServerPacket(ServerOP_ServerReloadRequest, sizeof(ServerReload::Request));
	auto reload = (ServerReload::Request*) pack.pBuffer;
	reload->type = type;
	reload->zone_server_id = 0;
	if (!is_global && zone && zone->IsLoaded()) {
		reload->zone_server_id = zone->GetZoneServerId();
	}

	SendPacket(&pack);
}

void WorldServer::QueueReload(ServerReload::Request r)
{
	m_reload_mutex.lock();
	int64_t reload_at = r.reload_at_unix - std::time(nullptr);

	LogInfo(
		"Queuing reload for [{}] ({}) to reload in [{}]",
		ServerReload::GetName(r.type),
		r.type,
		reload_at > 0 ? Strings::SecondsToTime(reload_at) : "Now"
	);

	m_reload_queue[r.type] = r;
	m_reload_mutex.unlock();
}

void WorldServer::ProcessReload(const ServerReload::Request& request)
{
	LogInfo(
		"Reloading [{}] ({}) zone booted required [{}]",
		ServerReload::GetName(request.type),
		request.type,
		request.requires_zone_booted
	);

	if (request.requires_zone_booted) {
		if (!zone || (zone && !zone->IsLoaded())) {
			LogInfo("Zone not booted, skipping reload for [{}] ({})", ServerReload::GetName(request.type), request.type);
			return;
		}
	}

	zone->SendReloadMessage(ServerReload::GetName(request.type));

	switch (request.type) {
		case ServerReload::Type::AAData:
			zone->LoadAlternateAdvancement();
			entity_list.SendAlternateAdvancementStats();
			break;

		case ServerReload::Type::Opcodes:
			ReloadAllPatches();
			break;

		case ServerReload::Type::AlternateCurrencies:
			zone->LoadAlternateCurrencies();
			break;

		case ServerReload::Type::BaseData:
			zone->ReloadBaseData();
			break;

		case ServerReload::Type::BlockedSpells:
			zone->LoadZoneBlockedSpells();
			break;

		case ServerReload::Type::Commands:
			command_init();
			if (RuleB(Bots, Enabled) && database.DoesTableExist("bot_command_settings")) {
				bot_command_init();
			}
			break;

		case ServerReload::Type::ContentFlags:
			WorldContentService::Instance()->SetExpansionContext()->ReloadContentFlags();
			break;

		case ServerReload::Type::DzTemplates:
			zone->LoadDynamicZoneTemplates();
			break;

		case ServerReload::Type::Factions:
			content_db.LoadFactionData();
			zone->ReloadNPCFactions();
			zone->ReloadFactionAssociations();
			break;

		case ServerReload::Type::LevelEXPMods:
			zone->LoadLevelEXPMods();
			break;

		case ServerReload::Type::Logs:
			EQEmuLogSys::Instance()->LoadLogDatabaseSettings();
			PlayerEventLogs::Instance()->ReloadSettings();
			break;

		case ServerReload::Type::Loot:
			zone->ReloadLootTables();
			break;

		case ServerReload::Type::Maps:
			zone->ReloadMaps();
			break;

		case ServerReload::Type::Merchants:
			entity_list.ReloadMerchants();
			break;

		case ServerReload::Type::NPCEmotes:
			zone->LoadNPCEmotes(&zone->npc_emote_list);
			break;

		case ServerReload::Type::NPCSpells:
			content_db.ClearNPCSpells();
			for (auto &e: entity_list.GetNPCList()) {
				e.second->ReloadSpells();
			}
			break;

		case ServerReload::Type::PerlExportSettings:
			parse->LoadPerlEventExportSettings(parse->perl_event_export_settings);
			break;

		case ServerReload::Type::Rules:
			RuleManager::Instance()->LoadRules(&database, RuleManager::Instance()->GetActiveRuleset(), true);
			break;

		case ServerReload::Type::SkillCaps:
			SkillCaps::Instance()->ReloadSkillCaps();
			break;

		case ServerReload::Type::DataBucketsCache:
			DataBucket::ClearCache();
			break;

		case ServerReload::Type::StaticZoneData:
		case ServerReload::Type::Doors:
		case ServerReload::Type::GroundSpawns:
		case ServerReload::Type::Objects:
			zone->ReloadStaticData();
			break;

		case ServerReload::Type::Tasks:
			if (RuleB(Tasks, EnableTaskSystem)) {
				entity_list.SaveAllClientsTaskState();
				TaskManager::Instance()->LoadTasks();
				entity_list.ReloadAllClientsTaskState();
				TaskManager::Instance()->LoadTaskSets();
			}
			break;

		case ServerReload::Type::Quests:
			entity_list.ClearAreas();
			parse->ReloadQuests(false);
			break;

		case ServerReload::Type::QuestsTimerReset:
			entity_list.ClearAreas();
			parse->ReloadQuests(true);
			break;

		case ServerReload::Type::Titles:
			title_manager.LoadTitles();
			break;

		case ServerReload::Type::Traps:
			entity_list.UpdateAllTraps(true, true);
			break;

		case ServerReload::Type::Variables:
			database.LoadVariables();
			break;

		case ServerReload::Type::VeteranRewards:
			zone->LoadVeteranRewards();
			break;

		case ServerReload::Type::WorldRepop:
			parse->ReloadQuests();
			if (zone && zone->IsLoaded()) {
				entity_list.ClearAreas();
				zone->Repop();
			}
			break;

		case ServerReload::Type::WorldWithRespawn:
			parse->ReloadQuests();
			if (zone && zone->IsLoaded()) {
				entity_list.ClearAreas();
				zone->Repop();
				zone->ClearSpawnTimers();
			}
			break;

		case ServerReload::Type::ZonePoints:
			content_db.LoadStaticZonePoints(&zone->zone_point_list, zone->GetShortName(), zone->GetInstanceVersion());
			break;

		case ServerReload::Type::ZoneData:
			ZoneStore::Instance()->LoadZones(content_db);
			zone->LoadZoneCFG(zone->GetShortName(), zone->GetInstanceVersion());
			break;

		default:
			break;
	}

	LogInfo("Reloaded [{}] ({})", ServerReload::GetName(request.type), request.type);
}
