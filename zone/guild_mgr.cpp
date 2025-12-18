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

#include "guild_mgr.h"

#include "common/emu_versions.h"
#include "common/repositories/guild_bank_repository.h"
#include "common/repositories/guild_ranks_repository.h"
#include "common/servertalk.h"
#include "common/strings.h"
#include "zone/client.h"
#include "zone/string_ids.h"
#include "zone/worldserver.h"
#include "zone/zonedb.h"


ZoneGuildManager guild_mgr;
GuildBankManager *GuildBanks;

extern WorldServer worldserver;
extern volatile bool is_zone_loaded;
extern EntityList entity_list;

void ZoneGuildManager::SendGuildRefresh(uint32 guild_id, bool name, bool motd, bool rank, bool relation) {
	LogGuilds("Sending guild refresh for [{}] to world, changes: name=[{}], motd=[{}], rank=d, relation=[{}]", guild_id, name, motd, rank, relation);
	auto pack = new ServerPacket(ServerOP_RefreshGuild, sizeof(ServerGuildRefresh_Struct));
	ServerGuildRefresh_Struct *s = (ServerGuildRefresh_Struct *) pack->pBuffer;
	s->guild_id = guild_id;
	s->name_change = name;
	s->motd_change = motd;
	s->rank_change = rank;
	s->relation_change = relation;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void ZoneGuildManager::SendCharRefresh(uint32 old_guild_id, uint32 guild_id, uint32 charid) {
	if(guild_id == 0) {
		LogGuilds("Guild lookup for char [{}] when sending char refresh", charid);

		CharGuildInfo gci;
		if(!GetCharInfo(charid, gci)) {
			guild_id = GUILD_NONE;
		} else {
			guild_id = gci.guild_id;
		}
	}

	LogGuilds("Sending char refresh for [{}] from guild [{}] to world", charid, guild_id);

	auto pack = new ServerPacket(ServerOP_GuildCharRefresh, sizeof(ServerGuildCharRefresh_Struct));
	ServerGuildCharRefresh_Struct *s = (ServerGuildCharRefresh_Struct *) pack->pBuffer;
	s->guild_id = guild_id;
	s->old_guild_id = old_guild_id;
	s->char_id = charid;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void ZoneGuildManager::SendRankUpdate(uint32 CharID)
{
	CharGuildInfo gci;
	if(!GetCharInfo(CharID, gci)) {
		return;
	}

	auto pack  = new ServerPacket(ServerOP_GuildRankUpdate, sizeof(ServerGuildRankUpdate_Struct));
	auto sgrus = (ServerGuildRankUpdate_Struct *) pack->pBuffer;

	sgrus->guild_id  = gci.guild_id;
	sgrus->rank      = gci.rank;
	sgrus->banker    = gci.banker + (gci.alt * 2);
	sgrus->no_update = true;
	strn0cpy(sgrus->member_name, gci.char_name.c_str(), sizeof(sgrus->member_name));

	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void ZoneGuildManager::SendGuildDelete(uint32 guild_id) {
	LogGuilds("Sending guild delete for guild [{}] to world", guild_id);
	auto pack = new ServerPacket(ServerOP_DeleteGuild, sizeof(ServerGuildID_Struct));
	auto s    = (ServerGuildID_Struct *) pack->pBuffer;
	s->guild_id = guild_id;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void ZoneGuildManager::ListGuilds(Client *c, uint32 guild_id) const {
	if (m_guilds.size()) {
		const auto& g = m_guilds.find(guild_id);
		if (g == m_guilds.end()) {
			c->Message(
				Chat::White,
				fmt::format(
					"Guild ID {} does not exist or is invalid.",
					guild_id
				).c_str()
			);
			return;
		}

		const auto leader_name = database.GetCharNameByID(g->second->leader);
		c->Message(
			Chat::White,
			fmt::format(
				"Guild {} | {}Name: {}",
				g->first,
				(
					!leader_name.empty() ?
					fmt::format(
						"Leader: {} ({}) ",
						leader_name,
						g->second->leader
					) :
					""
				),
				g->second->name
			).c_str()
		);
	} else {
		c->Message(Chat::White, "There are no Guilds to list.");
	}
}

void ZoneGuildManager::ListGuilds(Client *c, std::string search_criteria) const {
	if (m_guilds.size()) {
		if (search_criteria.empty()) {
			c->Message(
				Chat::White,
				fmt::format(
					"Listing {} Guild{}.",
					m_guilds.size(),
					m_guilds.size() != 1 ? "s" : ""
				).c_str()
			);
		}

		auto found_count = 0;

		for (const auto& guild : m_guilds) {
			if (
				!search_criteria.empty() &&
				!Strings::Contains(
					Strings::ToLower(guild.second->name),
					Strings::ToLower(search_criteria)
				)
			) {
				continue;
			}

			const auto leader_name = database.GetCharNameByID(guild.second->leader);
			c->Message(
				Chat::White,
				fmt::format(
					"Guild {} | {}Name: {}",
					guild.first,
					(
						!leader_name.empty() ?
						fmt::format(
							"Leader: {} ({}) ",
							leader_name,
							guild.second->leader
						) :
						""
					),
					guild.second->name
				).c_str()
			);

			found_count++;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Found {} Guild{}{}.",
				found_count,
				found_count != 1 ? "s" : "",
				(
					!search_criteria.empty() ?
					fmt::format(
						" matching '{}'",
						search_criteria
					) :
					""
				)
			).c_str()
		);
	} else {
		c->Message(Chat::White, "There are no Guilds to list.");
	}
}


void ZoneGuildManager::DescribeGuild(Client* c, uint32 guild_id) const
{
    std::map<uint32, GuildInfo *>::const_iterator res;
	res = m_guilds.find(guild_id);
	if (res == m_guilds.end()) {
		c->Message(
			Chat::White,
			fmt::format(
				"Guild ID {} could not be found.",
				guild_id
			).c_str()
		);
		return;
	}

	const GuildInfo *info = res->second;
    auto membership = GuildMembersRepository::GetGuildMembershipStats(*m_db, guild_id);

	auto leader_name = database.GetCharNameByID(info->leader);
	std::string popup_text = "<table>";
	popup_text += fmt::format(
		"<tr><td>Name</td><td><c \"#00FF00\">{}</c></td><td>Guild ID</td><td><c \"#00FF00\">{}</c></td></tr>",
        info->name,
		guild_id
	);
	popup_text += fmt::format(
		"<tr><td>Leader</td><td><c \"#F62217\">{}</c></td><td>Character ID</td><td><c \"#00FF00\">{}</c></td></tr>",
		leader_name,
		info->leader
	);

    popup_text += "<tr><td>.</td></tr>";
    popup_text += "<tr><td>Ranks</td><td>Quantity</td></tr>";
    popup_text += fmt::format("<tr><td>{}</td><td><c \"#00FF00\">{}</c></td></tr>", info->rank_names[1].c_str(), membership.leaders);
    popup_text += fmt::format("<tr><td>{}</td><td><c \"#00FF00\">{}</c></td></tr>", info->rank_names[2].c_str(), membership.senior_officers);
    popup_text += fmt::format("<tr><td>{}</td><td><c \"#00FF00\">{}</c></td></tr>", info->rank_names[3].c_str(), membership.officers);
    popup_text += fmt::format("<tr><td>{}</td><td><c \"#00FF00\">{}</c></td></tr>", info->rank_names[4].c_str(), membership.senior_members);
    popup_text += fmt::format("<tr><td>{}</td><td><c \"#00FF00\">{}</c></td></tr>", info->rank_names[5].c_str(), membership.members);
    popup_text += fmt::format("<tr><td>{}</td><td><c \"#00FF00\">{}</c></td></tr>", info->rank_names[6].c_str(), membership.junior_members);
    popup_text += fmt::format("<tr><td>{}</td><td><c \"#00FF00\">{}</c></td></tr>", info->rank_names[7].c_str(), membership.initates);
    popup_text += fmt::format("<tr><td>{}</td><td><c \"#00FF00\">{}</c></td></tr>", info->rank_names[8].c_str(), membership.recruits);

    popup_text += "<tr><td>.</td></tr>";
    popup_text += fmt::format("<tr><td>Tribute On</td><td><c \"#F62217\">{}</c></td></tr>", membership.tribute_enabled);
    popup_text += fmt::format("<tr><td>Total Tribute</td><td><c \"#F62217\">{}</c></td></tr>", info->tribute.favor);

    popup_text += "</table>";

    auto text = new char[4096];
    strn0cpy(text, popup_text.c_str(), 4096);

	c->SendPopupToClient(
		"Guild Information",
		text
	);
    safe_delete(text);
}

//in theory, we could get a pile of unused entries in this array, but only if
//we had a malicious client sending controlled packets, plus its like 10 bytes per entry.
void ZoneGuildManager::RecordInvite(uint32 char_id, uint32 guild_id, uint8 rank) {
	m_inviteQueue[char_id] = std::pair<uint32, uint8>(guild_id, rank);
}

bool ZoneGuildManager::VerifyAndClearInvite(uint32 char_id, uint32 guild_id, uint8 rank) {
	std::map<uint32, std::pair<uint32, uint8> >::iterator res;
	res = m_inviteQueue.find(char_id);
	if(res == m_inviteQueue.end())
		return(false);	//no entry...
	bool valid = false;
	if(res->second.first == guild_id && res->second.second == rank) {
		valid = true;
	}
	m_inviteQueue.erase(res);
	return(valid);
}

void ZoneGuildManager::ProcessWorldPacket(ServerPacket *pack)
{
	switch (pack->opcode) {
		case ServerOP_RefreshGuild: {
			if (pack->size != sizeof(ServerGuildRefresh_Struct)) {
				LogError("Received ServerOP_RefreshGuild of incorrect size [{}], expected [{}]",
						 pack->size,
						 sizeof(ServerGuildRefresh_Struct));
				return;
			}

			ServerGuildRefresh_Struct *s = (ServerGuildRefresh_Struct *) pack->pBuffer;
			LogDebug(
				"Received guild refresh from world for guild id [{}] changes: name=[{}] motd=[{}] rank=[{}] relation=[{}]",
				s->guild_id,
				s->name_change,
				s->motd_change,
				s->rank_change,
				s->relation_change
			);

			RefreshGuild(s->guild_id);

			if (s->motd_change) {
				//resend guild MOTD to all guild members in this zone.
				entity_list.SendGuildMOTD(s->guild_id);
			}

			if (s->name_change) {
				//until we figure out the guild update packet, we resend the whole guild list.
				entity_list.SendGuildList();
			}

			if (s->rank_change) {
				//we need to send spawn appearance packets for all members of this guild in the zone, to everybody.
				entity_list.SendGuildSpawnAppearance(s->guild_id);
			}

			if (s->relation_change) {
				//unknown until we implement guild relations.
			}

			break;
		}

		case ServerOP_GuildCharRefresh: {
			if (pack->size != sizeof(ServerGuildCharRefresh_Struct)) {
				LogError("Received ServerOP_RefreshGuild of incorrect size [{}], expected [{}]",
						 pack->size,
						 sizeof(ServerGuildCharRefresh_Struct));
				return;
			}
			ServerGuildCharRefresh_Struct *s = (ServerGuildCharRefresh_Struct *) pack->pBuffer;
			LogDebug("Received guild member refresh from world for char [{}] from guild [{}]",
					 s->char_id,
					 s->guild_id
			);
			RefreshGuild(s->guild_id);

			Client *c = entity_list.GetClientByCharID(s->char_id);

			if (c) {
				//this reloads the char's guild info from the database and sends appearance updates
				c->RefreshGuildInfo();
			}

			//it would be nice if we had the packet to send just a one-person update
			if (s->guild_id == GUILD_NONE) {
				if (c) {
					c->SendGuildMOTD();
				}
			}
			else {
				entity_list.SendGuildMembers(s->guild_id);        //even send GUILD_NONE (empty)

			}

			if (s->old_guild_id != 0 && s->old_guild_id != GUILD_NONE && s->old_guild_id != s->guild_id) {
				entity_list.SendGuildMembers(s->old_guild_id);
			}
			else if (c && s->guild_id != GUILD_NONE) {
				//char is in zone, and has changed into a new guild, send MOTD.
				c->SendGuildMOTD();
				if (c->ClientVersion() >= EQ::versions::ClientVersion::RoF) {
					c->SendGuildRanks();
				}
			}
			break;
		}

		case ServerOP_GuildRankUpdate: {
			auto sgrus = (ServerGuildRankUpdate_Struct *) pack->pBuffer;

			if (is_zone_loaded) {
				entity_list.SendGuildMemberRankAltBanker(sgrus->guild_id, sgrus->rank, sgrus->member_name, sgrus->alt, sgrus->banker);
			}
			break;
		}

		case ServerOP_DeleteGuild: {
			if (pack->size != sizeof(ServerGuildID_Struct)) {
				LogError("Received ServerOP_DeleteGuild of incorrect size [{}], expected [{}]",
						 pack->size,
						 sizeof(ServerGuildID_Struct));
				return;
			}

			if (is_zone_loaded) {
				ServerGuildID_Struct *s = (ServerGuildID_Struct *) pack->pBuffer;

				LogGuilds("Received guild delete from world for guild [{}]", s->guild_id);

				auto      clients = entity_list.GetClientList();
				for (auto &c: clients) {
					if (c.second->GuildID() == s->guild_id) {
						c.second->SetGuildID(GUILD_NONE);
						c.second->SetGuildRank(GUILD_RANK_NONE);
						c.second->SetGuildTributeOptIn(false);
						c.second->SendGuildActiveTributes(c.second->GuildID());
						c.second->SendGuildDeletePacket(s->guild_id);
						c.second->RefreshGuildInfo();
						c.second->MessageString(Chat::Guild, GUILD_DISBANDED);
						c.second->SendGuildList();
					}
				}

				auto res = m_guilds.find(s->guild_id);
				if (res != m_guilds.end()) {
					delete res->second;
					m_guilds.erase(res);
				}
			}
			break;
		}

		case ServerOP_GuildMemberUpdate: {
			auto sgmus = (ServerGuildMemberUpdate_Struct *) pack->pBuffer;

			if (is_zone_loaded) {
				auto outapp = new EQApplicationPacket(OP_GuildMemberUpdate, sizeof(GuildMemberUpdate_Struct));
				auto gmus   = (GuildMemberUpdate_Struct *) outapp->pBuffer;

				gmus->GuildID    = sgmus->guild_id;
				gmus->ZoneID     = sgmus->zone_id;
				gmus->InstanceID = 0;
				gmus->LastSeen   = sgmus->last_seen;
				strn0cpy(gmus->MemberName, sgmus->member_name, sizeof(gmus->MemberName));

				entity_list.QueueClientsGuild(outapp, sgmus->guild_id);
				safe_delete(outapp);
			}
			break;
		}
		case ServerOP_OnlineGuildMembersResponse: {
			if (is_zone_loaded) {
				char *Buffer = (char *) pack->pBuffer;

				uint32 FromID = VARSTRUCT_DECODE_TYPE(uint32, Buffer);
				uint32 Count = VARSTRUCT_DECODE_TYPE(uint32, Buffer);
				Client *c = entity_list.GetClientByCharID(FromID);

				if (!c || !c->IsInAGuild()) {
					LogGuilds("Invalid Client or not in guild. ID=[{}]", FromID);
					break;
				}
				LogGuilds("Processing ServerOP_OnlineGuildMembersResponse");
				auto                     outapp = new EQApplicationPacket(
					OP_GuildMemberUpdate,
					sizeof(GuildMemberUpdate_Struct));
				GuildMemberUpdate_Struct *gmus  = (GuildMemberUpdate_Struct *) outapp->pBuffer;
				char                     Name[64];
				gmus->LastSeen   = time(nullptr);
				gmus->InstanceID = 1;
				gmus->GuildID    = c->GuildID();
				for (int i = 0; i < Count; i++) {
					// Just make the packet once and swap out name/zone and send
					VARSTRUCT_DECODE_STRING(Name, Buffer);
					strn0cpy(gmus->MemberName, Name, sizeof(gmus->MemberName));
					gmus->ZoneID = VARSTRUCT_DECODE_TYPE(uint32, Buffer);
					LogGuilds("Sending OP_GuildMemberUpdate to [{}]. Name=[{}] ZoneID=[{}]",
							  FromID,
							  Name,
							  gmus->ZoneID);
					c->QueuePacket(outapp);
				}
				safe_delete(outapp);

			}
			break;
		}
		case ServerOP_LFGuildUpdate: {
			if (is_zone_loaded) {
				char   GuildName[33];
				char   Comments[257];
				uint32 FromLevel, ToLevel, Classes, AACount, TimeZone, TimePosted, Toggle;

				pack->ReadString(GuildName);
				pack->ReadString(Comments);
				FromLevel  = pack->ReadUInt32();
				ToLevel    = pack->ReadUInt32();
				Classes    = pack->ReadUInt32();
				AACount    = pack->ReadUInt32();
				TimeZone   = pack->ReadUInt32();
				TimePosted = pack->ReadUInt32();
				Toggle     = pack->ReadUInt32();

				uint32 GuildID = GetGuildIDByName(GuildName);

				if (GuildID == GUILD_NONE) {
					break;
				}

				auto outapp = new EQApplicationPacket(OP_LFGuild, sizeof(LFGuild_GuildToggle_Struct));

				auto *gts = (LFGuild_GuildToggle_Struct *) outapp->pBuffer;
				gts->Command = 1;
				strcpy(gts->Comment, Comments);
				gts->FromLevel  = FromLevel;
				gts->ToLevel    = ToLevel;
				gts->Classes    = Classes;
				gts->AACount    = AACount;
				gts->TimeZone   = TimeZone;
				gts->Toggle     = Toggle;
				gts->TimePosted = TimePosted;
				gts->Name[0] = 0;
				entity_list.QueueClientsGuild(outapp, GuildID);
				safe_delete(outapp);
			}
			break;
		}
		case ServerOP_GuildPermissionUpdate: {
			if (is_zone_loaded) {
                auto *sgpus = (ServerGuildPermissionUpdate_Struct *)pack->pBuffer;
                auto  guild = GetGuildByGuildID(sgpus->guild_id);
                if (!guild) {
                    return;
                }

                if (sgpus->function_value) {
                    guild->functions[sgpus->function_id].perm_value |= (1UL << (8 - sgpus->rank));
                }
                else {
                    guild->functions[sgpus->function_id].perm_value &= ~(1UL << (8 - sgpus->rank));
                }

				auto outapp  = new EQApplicationPacket(OP_GuildUpdate, sizeof(GuildPermission_Struct));
				auto *guuacs = (GuildPermission_Struct *) outapp->pBuffer;
				guuacs->Action      = GuildUpdatePermissions;
				guuacs->rank        = sgpus->rank;
				guuacs->function_id = sgpus->function_id;
				guuacs->value       = sgpus->function_value;

				entity_list.QueueClientsGuild(outapp, sgpus->guild_id);
				LogDebug(
					"Zone Received guild permission update from world for rank {} function id [{}] and value [{}]",
					guuacs->rank        = sgpus->rank,
					guuacs->function_id = sgpus->function_id,
					guuacs->value       = sgpus->function_value
				);
				safe_delete(outapp);

				if (sgpus->function_id == GUILD_ACTION_DISPLAY_GUILD_NAME) {
					entity_list.SendGuildSpawnAppearance(sgpus->guild_id);
				}

				//for backwards compatibility with guild bank functionality
				//if the four permissions (deposit, promote, withdraw and view) exist for a rank, turn on the banker flag for pre RoF clients
				if (IsActionABankAction((GuildAction) sgpus->function_id) &&
					GetGuildBankerStatus(sgpus->guild_id, sgpus->rank)) {
					entity_list.GuildSetPreRoFBankerFlag(sgpus->guild_id, sgpus->rank, true);
				}
				else if (IsActionABankAction((GuildAction) sgpus->function_id) &&
						 !GetGuildBankerStatus(sgpus->guild_id, sgpus->rank)) {
					entity_list.GuildSetPreRoFBankerFlag(sgpus->guild_id, sgpus->rank, false);
				}
			}
			break;
		}
		case ServerOP_GuildRankNameChange: {
			if (is_zone_loaded) {
				auto *s = (ServerGuildRankNameChange *) pack->pBuffer;
				LogGuilds(
					"Received guild rank name change from world for rank [{}] from guild [{}]",
					s->rank,
					s->guild_id
				);

				auto guild = guild_mgr.GetGuildByGuildID(s->guild_id);
				if (guild) {
					guild->rank_names[s->rank] = s->rank_name;
					auto outapp = new EQApplicationPacket(OP_GuildUpdate,sizeof(GuildUpdateUCPStruct));
					auto *gucp  = (GuildUpdateUCPStruct *) outapp->pBuffer;
					gucp->payload.rank_name.rank = s->rank;
					strn0cpy(gucp->payload.rank_name.rank_name, s->rank_name, sizeof(gucp->payload.rank_name.rank_name));
					gucp->action = GuildUpdateRanks;

					entity_list.QueueClientsGuild(outapp, s->guild_id);
					safe_delete(outapp);
				}
			}
			break;
		}
		case ServerOP_GuildMemberLevelUpdate: {
			if (is_zone_loaded) {
				auto s_in = (ServerOP_GuildMessage_Struct *) pack->pBuffer;
				entity_list.SendGuildMemberLevel(s_in->guild_id, s_in->player_level, s_in->player_name);
			}
			break;
		}
		case ServerOP_GuildMemberPublicNote: {
			if (is_zone_loaded) {
				auto s_in = (ServerOP_GuildMessage_Struct *) pack->pBuffer;
				entity_list.SendGuildMemberPublicNote(s_in->guild_id, s_in->player_name, s_in->note);
			}
			break;
		}
		case ServerOP_GuildSendGuildList: {
			if (is_zone_loaded) {
				entity_list.SendGuildList();
			}
			break;
		}
		case ServerOP_GuildChannel: {
			if (is_zone_loaded) {
				auto s_in = (ServerOP_GuildMessage_Struct *) pack->pBuffer;
				entity_list.SendGuildChannel(s_in->guild_id);
			}
			break;
		}
		case ServerOP_GuildMembersList: {
			if (is_zone_loaded) {
				auto s_in = (ServerOP_GuildMessage_Struct *) pack->pBuffer;
				entity_list.SendGuildMembersList(s_in->guild_id);
			}
			break;
		}
		case ServerOP_GuildURL: {
			if (is_zone_loaded) {
				auto s_in = (ServerOP_GuildMessage_Struct *) pack->pBuffer;
				entity_list.SendGuildURL(s_in->guild_id);
			}
			break;
		}
		case ServerOP_GuildMemberRemove: {
			if (is_zone_loaded) {
				auto s_in = (ServerOP_GuildMessage_Struct *) pack->pBuffer;
				entity_list.SendGuildMemberRemove(s_in->guild_id, s_in->player_name);
			}
			break;
		}
		case ServerOP_GuildMemberAdd: {
			if (is_zone_loaded) {
				auto s_in = (ServerOP_GuildMessage_Struct *) pack->pBuffer;
				entity_list.SendGuildMemberAdd(
					s_in->guild_id,
					s_in->player_level,
					s_in->player_class,
					s_in->player_rank,
					0,
					s_in->player_zone_id,
					s_in->player_name
				);
			}
			break;
		}
	}
}

void ZoneGuildManager::SendGuildMemberUpdateToWorld(const char *MemberName, uint32 GuildID, uint16 ZoneID, uint32 LastSeen)
{
	auto pack = new ServerPacket(ServerOP_GuildMemberUpdate, sizeof(ServerGuildMemberUpdate_Struct));

	ServerGuildMemberUpdate_Struct *sgmus = (ServerGuildMemberUpdate_Struct*)pack->pBuffer;
	sgmus->guild_id = GuildID;
	strn0cpy(sgmus->member_name, MemberName, sizeof(sgmus->member_name));
	sgmus->zone_id = ZoneID;
	sgmus->last_seen = LastSeen;
	worldserver.SendPacket(pack);

	safe_delete(pack);
}

void ZoneGuildManager::RequestOnlineGuildMembers(uint32 FromID, uint32 GuildID)
{
	auto pack =
	    new ServerPacket(ServerOP_RequestOnlineGuildMembers, sizeof(ServerRequestOnlineGuildMembers_Struct));
	ServerRequestOnlineGuildMembers_Struct *srogm = (ServerRequestOnlineGuildMembers_Struct*)pack->pBuffer;

	srogm->FromID = FromID;
	srogm->GuildID = GuildID;
	worldserver.SendPacket(pack);

	safe_delete(pack);
}

ZoneGuildManager::~ZoneGuildManager()
{
	ClearGuilds();
}

GuildBankManager::~GuildBankManager()
{

}

void GuildBankManager::Load(uint32 guild_id)
{
	auto results = GuildBankRepository::GetWhere(
		database,
		fmt::format("`guild_id` = '{}' ORDER BY `area`, `slot`", guild_id)
	);

	auto bank          = std::make_shared<GuildBank>();
	bank->guild_id     = guild_id;
	int32 deposit_slot = 0;
	int32 main_slot    = 0;

	for (auto &it: results) {
		if (it.area == GuildBankMainArea) {
			it.slot = deposit_slot;
			bank->items.main_area.emplace(deposit_slot, it);
			deposit_slot++;
		}
		else {
			it.slot = main_slot;
			bank->items.deposit_area.emplace(main_slot, it);
			main_slot++;
		}
	}

    banks.push_back(bank);
}

bool GuildBankManager::IsLoaded(uint32 guild_id)
{
	return GetGuildBank(guild_id) ? true : false;
}

void GuildBankManager::SendGuildBank(Client *c)
{
	if (!c || !c->IsInAGuild()) {
		return;
	}

	if (!IsLoaded(c->GuildID())) {
		Load(c->GuildID());
	}

	auto guild_bank = GetGuildBank(c->GuildID());
	if (!guild_bank) {
		LogError("Unable to find guild bank for guild ID <red>[{}]", c->GuildID());
		return;
	}

	// RoF+ uses a bulk list packet -- This is also how the Action 0 of older clients basically works
	if (c->ClientVersionBit() & EQ::versions::maskRoFAndLater) {

		auto outapp = new EQApplicationPacket(OP_GuildBankItemList, sizeof(GuildBankItemListEntry_Struct) * 240);
		for (int i = 0; i < c->GetInv().GetLookup()->InventoryTypeSize.GuildBankDeposit; i++) {
			const EQ::ItemData *item = database.GetItem(guild_bank->items.deposit_area[i].item_id);
			if (item) {
				outapp->WriteUInt8(1);
				outapp->WriteUInt32(guild_bank->items.deposit_area[i].permissions);
				outapp->WriteString(guild_bank->items.deposit_area[i].who_for.c_str());
				outapp->WriteString(guild_bank->items.deposit_area[i].donator.c_str());
				outapp->WriteUInt32(item->ID);
				outapp->WriteUInt32(item->Icon);
				if (item->Stackable) {
					outapp->WriteUInt32(guild_bank->items.deposit_area[i].quantity);
					outapp->WriteUInt8(item->StackSize == guild_bank->items.deposit_area[i].quantity ? 1 : 1);
				}
				else {
					outapp->WriteUInt32(1);
					outapp->WriteUInt8(0);
				}
				outapp->WriteUInt8(item->IsEquipable(c->GetBaseRace(), c->GetBaseClass()) ? 1 : 0);
				outapp->WriteString(item->Name);
			}
			else {
				outapp->WriteUInt8(0); // empty
			}
		}

		for (int i = 0; i < c->GetInv().GetLookup()->InventoryTypeSize.GuildBankMain; ++i) {
			const EQ::ItemData *Item = database.GetItem(guild_bank->items.main_area[i].item_id);
			if (Item) {
				outapp->WriteUInt8(1);
				outapp->WriteUInt32(guild_bank->items.main_area[i].permissions);
				outapp->WriteString(guild_bank->items.main_area[i].who_for.c_str());
				outapp->WriteString(guild_bank->items.main_area[i].donator.c_str());
				outapp->WriteUInt32(Item->ID);
				outapp->WriteUInt32(Item->Icon);
				if (Item->Stackable) {
					outapp->WriteUInt32(guild_bank->items.main_area[i].quantity);
					outapp->WriteUInt8(Item->StackSize == guild_bank->items.main_area[i].quantity ? 1 : 1);
				}
				else {
					outapp->WriteUInt32(1);
					outapp->WriteUInt8(0);
				}
				outapp->WriteUInt8(Item->IsEquipable(c->GetBaseRace(), c->GetBaseClass()) ? 1 : 0);
				outapp->WriteString(Item->Name);
			}
			else {
				outapp->WriteUInt8(0); // empty
			}
		}

		outapp->size = outapp->GetWritePosition(); // truncate to used size
		c->FastQueuePacket(&outapp);
		return;
	}

	for (int i = 0; i < c->GetInv().GetLookup()->InventoryTypeSize.GuildBankDeposit; ++i) {
		if (guild_bank->items.deposit_area[i].item_id > 0) {
			const EQ::ItemData *item = database.GetItem(guild_bank->items.deposit_area[i].item_id);
			if (!item) {
				continue;
			}

			auto outapp = new EQApplicationPacket(OP_GuildBank, sizeof(GuildBankItemUpdate_Struct));
			auto gbius  = (GuildBankItemUpdate_Struct *) outapp->pBuffer;

			if (!item->Stackable) {
				gbius->Init(
					GuildBankItemUpdate,
					1,
					i,
					GuildBankDepositArea,
					1,
					item->ID,
					item->Icon,
					1,
					guild_bank->items.deposit_area[i].permissions,
					0,
					false
				);
			}
			else {
				if (guild_bank->items.deposit_area[i].quantity == item->StackSize) {
					gbius->Init(
						GuildBankItemUpdate,
						1,
						i,
						GuildBankDepositArea,
						1,
						item->ID,
						item->Icon,
						guild_bank->items.deposit_area[i].quantity,
						guild_bank->items.deposit_area[i].permissions,
						1,
						false
					);
				}
				else {
					gbius->Init(
						GuildBankItemUpdate,
						1,
						i,
						GuildBankDepositArea,
						1,
						item->ID,
						item->Icon,
						guild_bank->items.deposit_area[i].quantity,
						guild_bank->items.deposit_area[i].permissions,
						1,
						false
					);
				}
			}

			strn0cpy(gbius->item_name, item->Name, sizeof(gbius->item_name));
			strn0cpy(gbius->donator, guild_bank->items.deposit_area[i].donator.c_str(), sizeof(gbius->donator));
			strn0cpy(gbius->who_for, guild_bank->items.deposit_area[i].who_for.c_str(), sizeof(gbius->who_for));

			c->FastQueuePacket(&outapp);
		}
	}

	for (int i = 0; i < c->GetInv().GetLookup()->InventoryTypeSize.GuildBankMain; ++i) {
		if (guild_bank->items.main_area[i].item_id > 0) {
			const EQ::ItemData *item = database.GetItem(guild_bank->items.main_area[i].item_id);

			if (!item) {
				continue;
			}

			bool useable = item->IsEquipable(c->GetBaseRace(), c->GetBaseClass());
			auto outapp  = new EQApplicationPacket(OP_GuildBank, sizeof(GuildBankItemUpdate_Struct));
			auto gbius   = (GuildBankItemUpdate_Struct *) outapp->pBuffer;

			if (!item->Stackable) {
				gbius->Init(
					GuildBankItemUpdate,
					1,
					i,
					GuildBankMainArea,
					1,
					item->ID,
					item->Icon,
					1,
					guild_bank->items.main_area[i].permissions,
					0,
					useable
				);
			}
			else {
				if (guild_bank->items.main_area[i].quantity == item->StackSize) {
					gbius->Init(
						GuildBankItemUpdate,
						1,
						i,
						GuildBankMainArea,
						1,
						item->ID,
						item->Icon,
						guild_bank->items.main_area[i].quantity,
						guild_bank->items.main_area[i].permissions,
						1,
						useable
					);
				}
				else {
					gbius->Init(
						GuildBankItemUpdate,
						1,
						i,
						GuildBankMainArea,
						1,
						item->ID,
						item->Icon,
						guild_bank->items.main_area[i].quantity,
						guild_bank->items.main_area[i].permissions,
						1,
						useable
					);
				}
			}

			strn0cpy(gbius->item_name, item->Name, sizeof(gbius->item_name));
			strn0cpy(gbius->donator, guild_bank->items.main_area[i].donator.c_str(), sizeof(gbius->donator));
			strn0cpy(gbius->who_for, guild_bank->items.main_area[i].who_for.c_str(), sizeof(gbius->who_for));

			c->FastQueuePacket(&outapp);
		}
	}
}

bool GuildBankManager::IsAreaFull(uint32 guild_id, uint16 area)
{
	return NextFreeBankSlot(guild_id, area) == -1;
}

bool GuildBankManager::AddItem(GuildBankRepository::GuildBank &guild_bank_item, Client* c)
{
	auto guild_bank = GetGuildBank(guild_bank_item.guild_id);
	if (!guild_bank) {
		LogError("Unable to find guild bank for guild ID [{}]", guild_bank_item.guild_id);
		return false;
	}

	switch (guild_bank_item.area) {
		case GuildBankMainArea: {
			for (auto &[key, item]: guild_bank->items.main_area) {
				if (item.item_id == 0) {
					item.slot            = NextFreeBankSlot(guild_bank_item.guild_id, GuildBankMainArea);
					guild_bank_item.slot = item.slot;
					auto e               = GuildBankRepository::InsertOne(database, guild_bank_item);
					if (!e.id) {
						return false;
					}

					guild_bank_item.id = e.id;
					item               = guild_bank_item;
					break;
				}
			}
			break;
		}
		case GuildBankDepositArea: {
			for (auto &[key, item]: guild_bank->items.deposit_area) {
				if (item.item_id == 0) {
					item.slot            = NextFreeBankSlot(guild_bank_item.guild_id, GuildBankDepositArea);
					guild_bank_item.slot = item.slot;
					auto e               = GuildBankRepository::InsertOne(database, guild_bank_item);
					if (!e.id) {
						return false;
					}
					guild_bank_item.id = e.id;
					item               = guild_bank_item;
					break;
				}
			}
			break;
		}
		default: {
			return false;
		}
	}

	SendGuildBankItemUpdate(guild_bank_item.guild_id, guild_bank_item.slot, guild_bank_item.area, true, c);

	return true;
}

int GuildBankManager::Promote(uint32 guild_id, int slot_id, Client* c)
{
	if (slot_id < 0) {
		return -1;
	}

	auto guild_bank = GetGuildBank(guild_id);
	if (!guild_bank) {
		return -1;
	}

	auto it_deposit = guild_bank->items.deposit_area.find(slot_id);
	if (it_deposit == std::end(guild_bank->items.deposit_area)) {
		return -1;
	}

	auto new_item             = GuildBankRepository::NewEntity();
	new_item.id               = it_deposit->second.id;
	new_item.guild_id         = it_deposit->second.guild_id;
	new_item.area             = GuildBankMainArea;
	new_item.slot             = NextFreeBankSlot(guild_id, GuildBankMainArea);
	new_item.item_id          = it_deposit->second.item_id;
	new_item.augment_one_id   = it_deposit->second.augment_one_id;
	new_item.augment_two_id   = it_deposit->second.augment_two_id;
	new_item.augment_three_id = it_deposit->second.augment_three_id;
	new_item.augment_four_id  = it_deposit->second.augment_four_id;
	new_item.augment_five_id  = it_deposit->second.augment_five_id;
	new_item.augment_six_id   = it_deposit->second.augment_six_id;
	new_item.quantity         = it_deposit->second.quantity;
	new_item.donator          = it_deposit->second.donator;
	new_item.who_for          = it_deposit->second.who_for;
	new_item.permissions      = it_deposit->second.permissions;

	GuildBankRepository::UpdateOne(database, new_item);

	SendGuildBankItemUpdate(it_deposit->second.guild_id, it_deposit->second.slot, it_deposit->second.area, false, c);
	it_deposit->second                         = std::move(GuildBankRepository::NewEntity());
	guild_bank->items.main_area[new_item.slot] = new_item;
	SendGuildBankItemUpdate(new_item.guild_id, new_item.slot, new_item.area, true, c);

	return new_item.slot;
}

void GuildBankManager::SetPermissions(uint32 guild_id, uint16 slot_id, uint32 permissions, const char *member_name, Client* c)
{
	auto guild_bank = GetGuildBank(guild_id);
	if (!guild_bank) {
		return;
	}

	auto item         = &guild_bank->items.main_area[slot_id];
	item->permissions = permissions;
	item->who_for.clear();

	if (permissions == GuildBankSingleMember) {
		item->who_for = member_name;
	}

	GuildBankRepository::UpdateOne(database, *item);
	SendGuildBankItemUpdate(item->guild_id, item->slot, item->area, true, c);
}

std::unique_ptr<EQ::ItemInstance> GuildBankManager::GetItem(uint32 guild_id, uint16 area, uint16 slot_id, uint32 quantity)
{
	auto guild_bank = GetGuildBank(guild_id);
	if (!guild_bank) {
		return nullptr;
	}

	std::unique_ptr<EQ::ItemInstance> inst;

	if (area == GuildBankDepositArea) {
		if (slot_id > guild_bank->items.deposit_area.size()) {
			return nullptr;
		}

		inst.reset(database.CreateItem(
			guild_bank->items.deposit_area[slot_id].item_id,
			guild_bank->items.deposit_area[slot_id].quantity,
			guild_bank->items.deposit_area[slot_id].augment_one_id,
			guild_bank->items.deposit_area[slot_id].augment_two_id,
			guild_bank->items.deposit_area[slot_id].augment_three_id,
			guild_bank->items.deposit_area[slot_id].augment_four_id,
			guild_bank->items.deposit_area[slot_id].augment_five_id,
			guild_bank->items.deposit_area[slot_id].augment_six_id)
		);

		if (!inst) {
			return nullptr;
		}

		if (!inst->IsStackable()) {
			inst->SetCharges(guild_bank->items.deposit_area[slot_id].quantity);
		}
		else {
			if (quantity <= guild_bank->items.deposit_area[slot_id].quantity) {
				inst->SetCharges(quantity);
			}
			else {
				inst->SetCharges(guild_bank->items.deposit_area[slot_id].quantity);
			}
		}
	}
	else {
		if (slot_id > guild_bank->items.main_area.size()) {
			return nullptr;
		}

		inst.reset(database.CreateItem(
			guild_bank->items.main_area[slot_id].item_id,
			guild_bank->items.main_area[slot_id].quantity,
			guild_bank->items.main_area[slot_id].augment_one_id,
			guild_bank->items.main_area[slot_id].augment_two_id,
			guild_bank->items.main_area[slot_id].augment_three_id,
			guild_bank->items.main_area[slot_id].augment_four_id,
			guild_bank->items.main_area[slot_id].augment_five_id,
			guild_bank->items.main_area[slot_id].augment_six_id)
		);

		if (!inst) {
			return nullptr;
		}

		if (!inst->IsStackable()) {
			inst->SetCharges(guild_bank->items.main_area[slot_id].quantity);
		}
		else {
			if (quantity <= guild_bank->items.main_area[slot_id].quantity) {
				inst->SetCharges(quantity);
			}
			else {
				inst->SetCharges(guild_bank->items.main_area[slot_id].quantity);
			}
		}
	}

	return inst;
}

bool GuildBankManager::HasItem(uint32 guild_id, uint32 item_id)
{
	auto guild_bank = GetGuildBank(guild_id);
	if (!guild_bank) {
		return false;
	}

	for (auto const &[slot, item]: guild_bank->items.deposit_area) {
		if (item.item_id == item_id) {
			return true;
		}
	}

	for (auto const &[slot, item]: guild_bank->items.main_area) {
		if (item.item_id == item_id) {
			return true;
		}
	}

	return false;
}

std::shared_ptr<GuildBank> GuildBankManager::GetGuildBank(const uint32 guild_id)
{
	if (!GuildBanks) {
		return nullptr;
	}

	for (auto &b:banks) {
		if (b->guild_id == guild_id) {
			return b;
		}
	}

	return nullptr;
}

bool GuildBankManager::DeleteItem(uint32 guild_id, uint16 area, uint16 slot_id, uint32 quantity, Client* c)
{
	auto guild_bank = GetGuildBank(guild_id);
	if (!guild_bank) {
		return false;
	}

	auto item = &guild_bank->items.deposit_area[slot_id];

	if (area == GuildBankMainArea) {
		item = &guild_bank->items.main_area[slot_id];
	}

	if (item->item_id == 0) {
		return false;
	}

	// delete the item
	if (item->quantity == quantity) {
		SendGuildBankItemUpdate(item->guild_id, item->slot, item->area, false, c);
		GuildBankRepository::DeleteOne(database, item->id);

		item->item_id     = 0;
		item->id          = 0;
		item->guild_id    = 0;
		item->slot        = 0;
		item->quantity    = 0;
		item->permissions = 0;
		item->donator.clear();
		item->who_for.clear();

		return true;
	}

	//otherwise update the quantity
	item->quantity -= quantity;
	GuildBankRepository::UpdateOne(database, *item);
	SendGuildBankItemUpdate(item->guild_id, item->slot, item->area, true, c);

	return true;
}

bool GuildBankManager::MergeStacks(uint32 guild_id, uint16 slot_id, Client* c)
{
	auto guild_bank = GetGuildBank(guild_id);
	if (!guild_bank) {
		return false;
	}

	auto       merge_bank_item = &guild_bank->items.main_area[slot_id];
	const auto merge_item      = database.GetItem(merge_bank_item->item_id);

	if (merge_bank_item->item_id == 0 || !merge_item) {
		return false;
	}

	const int16                                   stack_size = merge_item->StackSize;
	std::vector<GuildBankRepository::GuildBank *> queue{};
	std::vector<std::string>                      queue_delete{};
	std::vector<GuildBankRepository::GuildBank>   queue_updates{};

	for (auto &[key, item]: guild_bank->items.main_area) {
		if (item.item_id == merge_bank_item->item_id && item.slot != slot_id) {
			queue.push_back(&item);
		}
	}

	if (queue.empty()) {
		return false;
	}

	std::ranges::sort(queue.begin(), queue.end(), [](auto &a, auto &b) { return a->quantity < b->quantity; });

	for (auto const &i: queue) {
		const int16 remaining_stack_size = stack_size - static_cast<int16>(merge_bank_item->quantity);
		merge_bank_item->quantity += i->quantity;

		if (merge_bank_item->quantity <= stack_size) {
			i->item_id = 0;
			queue_delete.push_back(std::to_string(i->id));
			queue_updates.push_back(*i);
			continue;
		}

		if (merge_bank_item->quantity > stack_size) {
			merge_bank_item->quantity = stack_size;
			i->quantity              -= remaining_stack_size;
			queue_updates.push_back(*i);
			queue_updates.push_back(*merge_bank_item);
			break;
		}
	}

	if (merge_bank_item->quantity < stack_size) {
		queue_updates.push_back(*merge_bank_item);
		queue.push_back(merge_bank_item);
	}

	GuildBankRepository::ReplaceMany(database, queue_updates);
	if (!queue_delete.empty()) {
		GuildBankRepository::DeleteWhere(
			database, fmt::format("`id` IN({})", Strings::Implode(",", queue_delete)).c_str()
		);
	}

	for (auto const &i: queue_updates) {
		if (i.item_id == 0) {
			SendGuildBankItemUpdate(i.guild_id, i.slot, GuildBankMainArea, false, c);
			continue;
		}

		SendGuildBankItemUpdate(i.guild_id, i.slot, GuildBankMainArea, true, c);
	}

	return true;
}

bool GuildBankManager::SplitStack(uint32 guild_id, uint16 slot_id, uint32 quantity, Client* c)
{
	auto guild_bank = GetGuildBank(guild_id);
	if (!guild_bank) {
		return false;
	}

	if (IsAreaFull(guild_id, GuildBankMainArea)) {
		return false;
	}

	auto       split_bank_item = &guild_bank->items.main_area[slot_id];
	const auto split_item      = database.GetItem(split_bank_item->item_id);

	if (split_bank_item->item_id == 0 || !split_item) {
		return false;
	}

	auto new_item     = GuildBankRepository::NewEntity();
	new_item          = *split_bank_item;
	new_item.id       = 0;
	new_item.slot     = NextFreeBankSlot(guild_id, GuildBankMainArea);
	new_item.quantity = quantity;
	split_bank_item->quantity -= quantity;

	GuildBankRepository::UpdateOne(database, *split_bank_item);
	auto e                                     = GuildBankRepository::InsertOne(database, new_item);
	new_item.id                                = e.id;
	guild_bank->items.main_area[new_item.slot] = new_item;

	SendGuildBankItemUpdate(split_bank_item->guild_id, split_bank_item->slot, split_bank_item->area, true, c);
	SendGuildBankItemUpdate(new_item.guild_id, new_item.slot, new_item.area, true, c);

	return true;
}

int32 GuildBankManager::NextFreeBankSlot(uint32 guild_id, uint32 area)
{
	auto guild_bank = GetGuildBank(guild_id);
	if (!guild_bank) {
		return -1;
	}

	if (area == GuildBankMainArea) {
		auto it =
			std::ranges::find_if(guild_bank->items.main_area.begin(), guild_bank->items.main_area.end(), [&](auto &m) {
				return m.second.item_id == 0;
			}
		);
		if (it == guild_bank->items.main_area.end()) {
			return -1;
		}

		return it->first;
	}

	auto it = std::ranges::find_if(
		guild_bank->items.deposit_area.begin(), guild_bank->items.deposit_area.end(), [&](auto &m) {
			return m.second.item_id == 0;
		}
	);
	if (it == guild_bank->items.deposit_area.end()) {
		return -1;
	}

	return it->first;
}

void ZoneGuildManager::UpdateRankPermission(uint32 gid, uint32 charid, uint32 fid, uint32 rank, uint32 value)
{
    auto guild = GetGuildByGuildID(gid);
    if (!guild) {
        return;
    }

    if (value) {
        guild->functions[fid].perm_value |= (1UL << (8 - rank));
    }
    else {
        guild->functions[fid].perm_value &= ~(1UL << (8 - rank));
    }
    auto query   = fmt::format("UPDATE guild_permissions SET permission = {} WHERE perm_id = {} AND guild_id = {};",
                               guild->functions[fid].perm_value, fid, gid);
    auto results = m_db->QueryDatabase(query);
}

void ZoneGuildManager::SendPermissionUpdate(uint32 guild_id, uint32 rank, uint32 function_id, uint32 value)
{

	auto pack = new ServerPacket(ServerOP_GuildPermissionUpdate, sizeof(ServerGuildPermissionUpdate_Struct));
	auto *sgpus = (ServerGuildPermissionUpdate_Struct *) pack->pBuffer;

	sgpus->guild_id       = guild_id;
	sgpus->rank           = rank;
	sgpus->function_id    = function_id;
	sgpus->function_value = value;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void ZoneGuildManager::UpdateRankName(uint32 guild_id, uint32 rank, std::string rank_name)
{
	GuildRanksRepository::UpdateTitle(*m_db, guild_id, rank, rank_name);
}

void ZoneGuildManager::SendRankName(uint32 guild_id, uint32 rank, std::string rank_name)
{
	auto pack = new ServerPacket(ServerOP_GuildRankNameChange, sizeof(ServerGuildRankNameChange));
	auto *sgpus = (ServerGuildRankNameChange *) pack->pBuffer;

	sgpus->guild_id = guild_id;
	sgpus->rank     = rank;
	strn0cpy(sgpus->rank_name, rank_name.c_str(), sizeof(sgpus->rank_name));
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void ZoneGuildManager::SendAllRankNames(uint32 guild_id, uint32 char_id)
{
    auto guild = GetGuildByGuildID(guild_id);
    auto c     = entity_list.GetClientByCharID(char_id);
    if (guild && c) {
        auto                  outapp = new EQApplicationPacket(OP_GuildUpdate, sizeof(GuildUpdateUCPStruct));
        GuildUpdateUCPStruct *gucp   = (GuildUpdateUCPStruct *)outapp->pBuffer;
        for (int i = GUILD_LEADER; i <= GUILD_RECRUIT; i++) {
            gucp->payload.rank_name.rank = i;
            strn0cpy(gucp->payload.rank_name.rank_name, guild->rank_names[i].c_str(),
                     sizeof(gucp->payload.rank_name.rank_name));
            gucp->action = GuildUpdateRanks;
            c->QueuePacket(outapp);
        }
        safe_delete(outapp);
    }
}

BaseGuildManager::GuildInfo* ZoneGuildManager::GetGuildByGuildID(uint32 guild_id)
{
	auto guild = m_guilds.find(guild_id);
	if (guild != m_guilds.end()) {
		return guild->second;
	}
	return nullptr;
}

uint8* ZoneGuildManager::MakeGuildMembers(uint32 guild_id, const char* prefix_name, uint32& length) {
	uint8* retbuffer;

	std::vector<CharGuildInfo*> members;
	if (!GetEntireGuild(guild_id, members))
		return(nullptr);

	//figure out the actual packet length.
	uint32 fixed_length = sizeof(Internal_GuildMembers_Struct) + members.size() * sizeof(Internal_GuildMemberEntry_Struct);
	std::vector<CharGuildInfo*>::iterator cur, end;
	CharGuildInfo* ci;
	cur = members.begin();
	end = members.end();
	uint32 name_len = 0;
	uint32 note_len = 0;
	for (; cur != end; ++cur) {
		ci = *cur;
		name_len += ci->char_name.length();
		note_len += ci->public_note.length();
	}

	//calc total length.
	length = fixed_length + name_len + note_len + members.size() * 2;	//string data + null terminators

	//make our nice buffer
	retbuffer = new uint8[length];

	auto gms = (Internal_GuildMembers_Struct*)retbuffer;

	//fill in the global header
	strcpy(gms->player_name, prefix_name);
	gms->count       = members.size();
	gms->name_length = name_len;
	gms->note_length = note_len;

	char *name_buf = (char *) (retbuffer + fixed_length);
	char *note_buf = (char *) (name_buf + name_len + members.size());

	//fill in each member's entry.
	Internal_GuildMemberEntry_Struct* e = gms->member;

	cur = members.begin();
	end = members.end();
	for (; cur != end; ++cur) {
		ci = *cur;

		//the order we set things here must match the struct

//nice helper macro
#define SlideStructString(field, str) \
		strcpy(field, str.c_str()); \
		field += str.length() + 1
#define PutField(field) \
		e->field = ci->field

		SlideStructString(name_buf, ci->char_name);
		PutField(level);
		e->banker = ci->banker + (ci->alt * 2);	// low bit is banker flag, next bit is 'alt' flag.
		PutField(class_);
		auto c = entity_list.GetClientByID(ci->char_id);
		if (c && c->ClientVersion() < EQ::versions::ClientVersion::RoF) {
			switch (ci->rank) {
				case GUILD_RECRUIT:
				case GUILD_INITIATE:
				case GUILD_JUNIOR_MEMBER:
				case GUILD_MEMBER:
				case GUILD_SENIOR_MEMBER: {
					ci->rank = GUILD_MEMBER_TI;
					break;
				}
				case GUILD_OFFICER:
				case GUILD_SENIOR_OFFICER: {
					ci->rank = GUILD_OFFICER_TI;
					break;
				}
				case GUILD_LEADER: {
					ci->rank = GUILD_LEADER_TI;
					break;
				}
				default: {
					ci->rank = GUILD_RANK_NONE_TI;
					break;
				}
			}
		}
		PutField(rank);
		PutField(time_last_on);
		PutField(tribute_enable);
		PutField(total_tribute);
		PutField(last_tribute);
		SlideStructString(note_buf, ci->public_note);
		//e->zoneinstance = 0;
		if (ci->online) {
			e->zone_id = ci->zone_id;	//This routine, if there is a zone_id, will update the entire guild window (roster, notes, tribute) for online characters.
		}
		else {
			e->zone_id = 0;				//If zone_id is 0 and we rely on the current world routine, the notes/tribute tabs are not updated for online characters.
		}

#undef SlideStructString
#undef PutFieldN

		delete* cur;

		e++;
	}

	return(retbuffer);
}

void ZoneGuildManager::SendToWorldMemberLevelUpdate(uint32 guild_id, uint32 level, std::string player_name)
{
	auto s_outapp = new ServerPacket(ServerOP_GuildMemberLevelUpdate, sizeof(ServerOP_GuildMessage_Struct));
	auto *s_out = (ServerOP_GuildMessage_Struct *) s_outapp->pBuffer;

	s_out->guild_id     = guild_id;
	s_out->player_level = level;
	strn0cpy(s_out->player_name, player_name.c_str(), sizeof(s_out->player_name));

	worldserver.SendPacket(s_outapp);
	safe_delete(s_outapp);
}

void ZoneGuildManager::SendToWorldMemberPublicNote(uint32 guild_id, std::string player_name, std::string public_note)
{
	auto s_outapp = new ServerPacket(ServerOP_GuildMemberPublicNote, sizeof(ServerOP_GuildMessage_Struct));
	auto *s_out = (ServerOP_GuildMessage_Struct *) s_outapp->pBuffer;

	s_out->guild_id = guild_id;
	strn0cpy(s_out->player_name, player_name.c_str(), sizeof(s_out->player_name));
	strn0cpy(s_out->note, public_note.c_str(), sizeof(s_out->note));

	worldserver.SendPacket(s_outapp);
	safe_delete(s_outapp);
}

void ZoneGuildManager::SendToWorldGuildChannel(uint32 guild_id, std::string channel)
{
	auto s_outapp = new ServerPacket(ServerOP_GuildChannel, sizeof(ServerOP_GuildMessage_Struct));
	auto *s_out = (ServerOP_GuildMessage_Struct *) s_outapp->pBuffer;

	s_out->guild_id = guild_id;
	strn0cpy(s_out->channel, channel.c_str(), sizeof(s_out->channel));

	worldserver.SendPacket(s_outapp);
	safe_delete(s_outapp);
}

void ZoneGuildManager::SendToWorldGuildURL(uint32 guild_id, std::string url)
{
	auto s_outapp = new ServerPacket(ServerOP_GuildURL, sizeof(ServerOP_GuildMessage_Struct));
	auto *s_out = (ServerOP_GuildMessage_Struct *) s_outapp->pBuffer;

	s_out->guild_id = guild_id;
	strn0cpy(s_out->url, url.c_str(), sizeof(s_out->url));

	worldserver.SendPacket(s_outapp);
	safe_delete(s_outapp);
}

void ZoneGuildManager::SendToWorldMemberRemove(uint32 guild_id, std::string player_name)
{
	auto s_outapp = new ServerPacket(ServerOP_GuildMemberRemove, sizeof(ServerOP_GuildMessage_Struct));
	ServerOP_GuildMessage_Struct* s_out = (ServerOP_GuildMessage_Struct*)s_outapp->pBuffer;

    s_out->guild_id = guild_id;
	strn0cpy(s_out->player_name, player_name.c_str(), sizeof(s_out->player_name));

    worldserver.SendPacket(s_outapp);
	safe_delete(s_outapp);
}

void ZoneGuildManager::SendToWorldMemberAdd(uint32 guild_id, uint32 char_id, uint32 level, uint32 _class, uint32 rank, uint32 zone_id, std::string player_name)
{
	auto s_outapp = new ServerPacket(ServerOP_GuildMemberAdd, sizeof(ServerOP_GuildMessage_Struct));
	auto s_out = (ServerOP_GuildMessage_Struct*)s_outapp->pBuffer;

    s_out->guild_id       = guild_id;
	s_out->player_level   = level;
	s_out->player_rank    = rank;
	s_out->player_zone_id = zone_id;
	s_out->player_class   = _class;
	strn0cpy(s_out->player_name, player_name.c_str(), sizeof(s_out->player_name));

    worldserver.SendPacket(s_outapp);
	safe_delete(s_outapp);
}

void ZoneGuildManager::SendToWorldSendGuildList()
{
	auto s_outapp = new ServerPacket(ServerOP_GuildSendGuildList, sizeof(ServerOP_GuildMessage_Struct));
	auto s_out = (ServerOP_GuildMessage_Struct*)s_outapp->pBuffer;

	worldserver.SendPacket(s_outapp);
	safe_delete(s_outapp);
}

bool ZoneGuildManager::RemoveMember(uint32 guild_id, uint32 char_id, std::string player_name)
{
	GuildMembersRepository::DeleteOne(*m_db, char_id);
	SendToWorldMemberRemove(guild_id, player_name);
	return true;
}

void ZoneGuildManager::MemberAdd(uint32 guild_id, uint32 char_id, uint32 level, uint32 _class, uint32 rank, uint32 zone_id, std::string player_name)
{
	GuildMembersRepository::GuildMembers m;
	m.alt            = 0;
	m.banker         = rank == GUILD_LEADER ? 1 : 0;
	m.last_tribute   = 0;
	m.total_tribute  = 0;
	m.tribute_enable = 0;
	m.rank_          = rank;
	m.char_id        = char_id;
	m.guild_id       = guild_id;
	m.online         = 1;
	m.public_note.clear();

	GuildMembersRepository::ReplaceOne(*m_db, m);
	SendToWorldMemberAdd(guild_id, char_id, level, _class, rank, zone_id, player_name);
}

bool ZoneGuildManager::IsActionABankAction(GuildAction action)
{
	return action == GUILD_ACTION_BANK_DEPOSIT_ITEMS ||
		   action == GUILD_ACTION_BANK_PROMOTE_ITEMS ||
		   action == GUILD_ACTION_BANK_VIEW_ITEMS ||
		   action == GUILD_ACTION_BANK_WITHDRAW_ITEMS;
}

void ZoneGuildManager::SendToWorldMemberRankUpdate(uint32 guild_id, uint32 rank, uint32 banker, uint32 alt, bool no_update, const char *player_name)
{
	auto outapp = new ServerPacket(ServerOP_GuildRankUpdate, sizeof(ServerGuildRankUpdate_Struct));
	auto sr     = (ServerGuildRankUpdate_Struct *) outapp->pBuffer;

	sr->guild_id  = guild_id;
	sr->rank      = rank;
	sr->banker    = banker;
	sr->alt       = alt;
	sr->no_update = no_update;
	strn0cpy(sr->member_name, player_name, sizeof(sr->member_name));

	worldserver.SendPacket(outapp);
	safe_delete(outapp)
}

bool ZoneGuildManager::MemberRankUpdate(uint32 guild_id, uint32 rank, uint32 banker, uint32 alt, bool no_update, const char *player_name)
{
	SendToWorldMemberRankUpdate(guild_id, rank, banker, alt, no_update, player_name);
	return true;
}

void ZoneGuildManager::SendToWorldSendGuildMembersList(uint32 guild_id)
{
	auto outapp = new ServerPacket(ServerOP_GuildMembersList, sizeof(ServerOP_GuildMessage_Struct));
	auto sr     = (ServerOP_GuildMessage_Struct *) outapp->pBuffer;

	sr->guild_id  = guild_id;

	worldserver.SendPacket(outapp);
	safe_delete(outapp)
}

void GuildBankManager::SendGuildBankItemUpdate(uint32 guild_id, int32 slot_id, uint32 area, bool display, Client* c)
{
	auto guild_bank = GetGuildBank(guild_id);
	if (!guild_bank) {
		return;
	}

	auto item = guild_bank->items.deposit_area[slot_id];
	if (area == GuildBankMainArea) {
		item = guild_bank->items.main_area[slot_id];
	}

	const auto item_data = database.GetItem(item.item_id);

	if (item_data && (item_data->MaxCharges > 0 || item_data->MaxCharges < 0)) {
		item.quantity = 1;
	}

	GuildBankItemUpdate_Struct gbius{};
	gbius.action      = GuildBankItemUpdate;
	gbius.slot_id     = item.slot;
	gbius.area        = item.area;
	gbius.item_id     = item.item_id;
	gbius.permissions = item.permissions;
	gbius.icon_id     = item_data ? item_data->Icon : 0;
	gbius.display     = display;
	gbius.allow_merge = item_data ? item_data->Stackable : false;
	gbius.is_useable  = item_data ? item_data->IsEquipable(c->GetRace(), c->GetClass()) : false;
	gbius.quantity    = item.quantity;

	strn0cpy(gbius.item_name, item_data ? item_data->Name : "", sizeof(gbius.item_name));
	strn0cpy(gbius.donator, item.donator.empty() ? "" : item.donator.c_str(), sizeof(gbius.donator));
	strn0cpy(gbius.who_for, item.who_for.empty() ? "" : item.who_for.c_str(), sizeof(gbius.who_for));

	entity_list.QueueClientsGuildBankItemUpdate(&gbius, guild_id);
}

bool ZoneGuildManager::SetGuild(Client *client, uint32 guild_id, uint8 rank)
{
	if (!client || rank > GUILD_MAX_RANK || !GetGuildByGuildID(guild_id)) {
		return false;
	}

	if (rank <= GUILD_RANK_NONE) {
		rank = GUILD_RECRUIT;
	}

	const uint32 current_guild_id = client->GuildID();
	if (current_guild_id == guild_id) {
		return false;
	}

	if (current_guild_id != guild_id && current_guild_id != GUILD_NONE) {
		guild_mgr.RemoveMember(client->GuildID(), client->CharacterID(), std::string(client->GetCleanName()));
	}

	client->SetGuildID(guild_id);
	client->SetGuildRank(rank);
	MemberAdd(
		guild_id,
		client->CharacterID(),
		client->GetLevel(),
		client->GetClass(),
		rank,
		client->GetZoneID(),
		client->GetName()
	);

	client->SendGuildSpawnAppearance();
	return true;
}
