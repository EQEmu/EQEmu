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

#include "wguild_mgr.h"

#include "common/eqemu_logsys.h"
#include "common/repositories/guild_bank_repository.h"
#include "common/repositories/guild_members_repository.h"
#include "common/repositories/guild_permissions_repository.h"
#include "common/repositories/guild_ranks_repository.h"
#include "common/repositories/guild_tributes_repository.h"
#include "common/repositories/guilds_repository.h"
#include "common/repositories/tribute_levels_repository.h"
#include "common/repositories/tributes_repository.h"
#include "common/servertalk.h"
#include "world/client.h"
#include "world/cliententry.h"
#include "world/clientlist.h"
#include "world/zonelist.h"
#include "world/zoneserver.h"

std::map<uint32, TributeData> tribute_list;

WorldGuildManager guild_mgr;

void WorldGuildManager::SendGuildRefresh(uint32 guild_id, bool name, bool motd, bool rank, bool relation) {
	LogGuilds("Broadcasting guild refresh for [{}], changes: name=[{}], motd=[{}], rank=d, relation=[{}]", guild_id, name, motd, rank, relation);
	auto pack = new ServerPacket(ServerOP_RefreshGuild, sizeof(ServerGuildRefresh_Struct));
	ServerGuildRefresh_Struct *s = (ServerGuildRefresh_Struct *) pack->pBuffer;
	s->guild_id = guild_id;
	s->name_change = name;
	s->motd_change = motd;
	s->rank_change = rank;
	s->relation_change = relation;
	ZSList::Instance()->SendPacketToZonesWithGuild(guild_id, pack);
	safe_delete(pack);
}

void WorldGuildManager::SendCharRefresh(uint32 old_guild_id, uint32 guild_id, uint32 charid) {
	LogGuilds("Broadcasting char refresh for [{}] from guild [{}] to world", charid, guild_id);
	auto pack = new ServerPacket(ServerOP_GuildCharRefresh, sizeof(ServerGuildCharRefresh_Struct));
	ServerGuildCharRefresh_Struct *s = (ServerGuildCharRefresh_Struct *) pack->pBuffer;
	s->guild_id = guild_id;
	s->old_guild_id = old_guild_id;
	s->char_id = charid;
	ZSList::Instance()->SendPacketToZonesWithGuild(guild_id, pack);
	safe_delete(pack);
}

void WorldGuildManager::SendGuildDelete(uint32 guild_id) {
	LogGuilds("Broadcasting guild delete for guild [{}] to world", guild_id);
	auto pack = new ServerPacket(ServerOP_DeleteGuild, sizeof(ServerGuildID_Struct));
	ServerGuildID_Struct *s = (ServerGuildID_Struct *) pack->pBuffer;
	s->guild_id = guild_id;
	ZSList::Instance()->SendPacketToZonesWithGuild(guild_id, pack);
	safe_delete(pack);
}

void WorldGuildManager::ProcessZonePacket(ServerPacket *pack) {
	switch(pack->opcode) {

	case ServerOP_RefreshGuild: {
		if(pack->size != sizeof(ServerGuildRefresh_Struct)) {
			LogGuilds("Received ServerOP_RefreshGuild of incorrect size [{}], expected [{}]", pack->size, sizeof(ServerGuildRefresh_Struct));
			return;
		}
		ServerGuildRefresh_Struct *s = (ServerGuildRefresh_Struct *) pack->pBuffer;
		LogGuilds("Received and broadcasting guild refresh for [{}], changes: name=[{}], motd=[{}], rank=d, relation=[{}]", s->guild_id, s->name_change, s->motd_change, s->rank_change, s->relation_change);

		//preform a local refresh.
		if(!RefreshGuild(s->guild_id)) {
			BaseGuildManager::RefreshGuild(s->guild_id);
		}

		//broadcast this packet to all zones.
		ZSList::Instance()->SendPacketToBootedZones(pack);

		break;
	}

	case ServerOP_GuildCharRefresh:
	{
		ServerGuildCharRefresh_Struct *s = (ServerGuildCharRefresh_Struct *) pack->pBuffer;
		LogGuilds("Received and broadcasting guild member refresh for char [{}] to all zones with members of guild [{}]", s->char_id, s->guild_id);

		RefreshGuild(s->guild_id);
		//preform the local update
		ClientList::Instance()->UpdateClientGuild(s->char_id, s->guild_id);

		//broadcast this update to any zone with a member in this guild.
		//because im sick of this not working, sending it to all zones, just spends a bit more bandwidth.
		ZSList::Instance()->SendPacketToZonesWithGuild(s->guild_id, pack);

		break;
	}

	case ServerOP_DeleteGuild:
	{
		if(pack->size != sizeof(ServerGuildID_Struct)) {
			LogGuilds("Received ServerOP_DeleteGuild of incorrect size [{}], expected [{}]", pack->size, sizeof(ServerGuildID_Struct));
			return;
		}

		ServerGuildID_Struct *s = (ServerGuildID_Struct *) pack->pBuffer;

		auto res = m_guilds.find(s->guild_id);
		if (res != m_guilds.end()) {
			delete res->second;
			m_guilds.erase(res);
		}

		LogGuilds("Received and broadcasting guild delete for guild [{}]", s->guild_id);

		//broadcast this packet to all zones.
		ZSList::Instance()->SendPacket(pack);

		break;
	}

	case ServerOP_GuildMemberUpdate:
	{
		if(pack->size != sizeof(ServerGuildMemberUpdate_Struct))
		{
			LogGuilds("Received ServerOP_GuildMemberUpdate of incorrect size [{}], expected [{}]", pack->size, sizeof(ServerGuildMemberUpdate_Struct));
			return;
		}

		auto s = (ServerGuildID_Struct *)pack->pBuffer;
		RefreshGuild(s->guild_id);

		ZSList::Instance()->SendPacketToZonesWithGuild(s->guild_id, pack);
		break;
	}
	case ServerOP_GuildPermissionUpdate:
	{
		if (pack->size != sizeof(ServerGuildPermissionUpdate_Struct))
		{
			LogGuilds("Received ServerOP_GuildPermissionUpdate of incorrect size [{}], expected [{}]", pack->size, sizeof(ServerGuildPermissionUpdate_Struct));
			return;
		}

		auto sg    = (ServerGuildPermissionUpdate_Struct *)pack->pBuffer;
		auto guild = GetGuildByGuildID(sg->guild_id);
		if (!guild) {
			guild_mgr.LoadGuilds();
			guild = GetGuildByGuildID(sg->guild_id);
		}

		if (guild) {
			if (sg->function_value) {
				guild->functions[sg->function_id].perm_value |= (1UL << (8 - sg->rank));
			}
			else {
				guild->functions[sg->function_id].perm_value &= ~(1UL << (8 - sg->rank));
			}

			LogGuilds("World Received ServerOP_GuildPermissionUpdate for guild [{}] function id {} with value of {}",
				sg->guild_id,
				sg->function_id,
				sg->function_value
			);

			ZSList::Instance()->SendPacketToZonesWithGuild(sg->guild_id, pack);
		}
		else {
			LogError("World Received ServerOP_GuildPermissionUpdate for guild [{}] function id {} with value of {} but guild could not be found.",
				sg->guild_id,
				sg->function_id,
				sg->function_value
			);
		}

		break;
	}
	case ServerOP_GuildRankNameChange:
	{
		if (pack->size != sizeof(ServerGuildRankNameChange))
		{
			LogGuilds("Received ServerOP_ServerGuildRankNameChange of incorrect size [{}], expected [{}]", pack->size, sizeof(ServerGuildPermissionUpdate_Struct));
			return;
		}

		auto rnc   = (ServerGuildRankNameChange*)pack->pBuffer;
		auto guild = GetGuildByGuildID(rnc->guild_id);
		if (!guild) {
			guild_mgr.LoadGuilds();
			guild = GetGuildByGuildID(rnc->guild_id);
		}

		if (guild) {
			guild->rank_names[rnc->rank] = rnc->rank_name;
			LogGuilds("World Received ServerOP_GuildRankNameChange from zone for guild [{}] rank id {} with new name of {}",
				rnc->guild_id,
				rnc->rank,
				rnc->rank_name
			);
			ZSList::Instance()->SendPacketToZonesWithGuild(rnc->guild_id, pack);
		}
		else {
			LogError("World Received ServerOP_GuildRankNameChange from zone for guild [{}] rank id {} with new name of {} but could not find guild.",
				rnc->guild_id,
				rnc->rank,
				rnc->rank_name
			);
		}

		break;
	}
	case ServerOP_GuildMemberLevelUpdate:
	case ServerOP_GuildMemberPublicNote:
	case ServerOP_GuildChannel:
	case ServerOP_GuildURL:
	case ServerOP_GuildMemberRemove:
	case ServerOP_GuildMembersList:
	{
		auto in = (ServerOP_GuildMessage_Struct *) pack->pBuffer;
		ZSList::Instance()->SendPacketToZonesWithGuild(in->guild_id, pack);
		break;
	}
    case ServerOP_GuildMemberAdd:
	{
        auto in    = (ServerOP_GuildMessage_Struct *)pack->pBuffer;
        auto guild = GetGuildByGuildID(in->guild_id);
        if (!guild) {
            BaseGuildManager::RefreshGuild(in->guild_id);
        }

        ZSList::Instance()->SendPacketToZonesWithGuild(in->guild_id, pack);
        break;
    }
	case ServerOP_GuildSendGuildList: {
		auto in = (ServerOP_GuildMessage_Struct *) pack->pBuffer;
		ZSList::Instance()->SendPacketToBootedZones(pack);
		break;
	}

	default:
		LogGuilds("Unknown packet {:#04x} received from zone??", pack->opcode);
		break;
	}
}

void WorldGuildManager::Process()
{
	for (auto &g: m_guilds) {
		if (!g.second->tribute.enabled) {
			continue;
		}
		else if (g.second->tribute.enabled && !g.second->tribute.timer.Enabled()) {
			g.second->tribute.timer.Start(g.second->tribute.time_remaining);
			LogGuilds(
				"Found a Guild Tribute Timer for guild [{}]. that was not started. Started it with [{}] time remaining before restart.",
				g.first,
				g.second->tribute.time_remaining
			);
		}
		else if (g.second->tribute.enabled && g.second->tribute.timer.Check()) {
			g.second->tribute.favor         -= GetGuildTributeCost(g.first);
			g.second->tribute.time_remaining = RuleI(Guild, TributeTime);
			g.second->tribute.timer.Start(RuleI(Guild, TributeTime));
			guild_mgr.UpdateDbGuildFavor(g.first, g.second->tribute.favor);
			guild_mgr.UpdateDbTributeTimeRemaining(g.first, RuleI(Guild, TributeTime));
			SendGuildTributeFavorAndTimer(g.first, g.second->tribute.favor, g.second->tribute.timer.GetRemainingTime());
		}
		else if (g.second->tribute.send_timer &&
				 ((g.second->tribute.timer.GetRemainingTime() / 1000) %
				  (RuleI(Guild, TributeTimeRefreshInterval) / 1000)) == 0 &&
				 !g.second->tribute.timer.Check()
			) {
			g.second->tribute.send_timer     = false;
			g.second->tribute.time_remaining = g.second->tribute.timer.GetRemainingTime();
			SendGuildTributeFavorAndTimer(g.first, g.second->tribute.favor, g.second->tribute.time_remaining);
			guild_mgr.UpdateDbTributeTimeRemaining(g.first, g.second->tribute.time_remaining);
			LogGuilds(
				"Timer Frequency [{}] ms hit sending time [{}] to guild clients",
				RuleI(Guild, TributeTimeRefreshInterval),
				g.second->tribute.time_remaining
			);
		}
		else if (!g.second->tribute.send_timer &&
				 ((g.second->tribute.timer.GetRemainingTime() / 1000) %
				  (RuleI(Guild, TributeTimeRefreshInterval) / 1000)) > 0 &&
				 !g.second->tribute.timer.Check()
			) {
			g.second->tribute.send_timer = true;
		}
	}
}

uint32 WorldGuildManager::GetGuildTributeCost(uint32 guild_id)
{
	auto guild_members = ClientList::Instance()->GetGuildClientsWithTributeOptIn(guild_id);
	auto total         = guild_members.size();
	auto total_cost    = 0;

	auto guild = guild_mgr.GetGuildByGuildID(guild_id);
	if (guild)
	{
		TributeData &d1  = tribute_list[guild->tribute.id_1];
		TributeData &d2  = tribute_list[guild->tribute.id_2];
		uint32 cost_id1  = d1.tiers[guild->tribute.id_1_tier].cost;
		uint32 cost_id2  = d2.tiers[guild->tribute.id_2_tier].cost;
		uint32 level_id1 = d2.tiers[guild->tribute.id_1_tier].level;
		uint32 level_id2 = d2.tiers[guild->tribute.id_2_tier].level;

		for (auto const &m: guild_members) {
			if (m.second->level() >= level_id1) {
				total_cost += cost_id1;
			}
			if (m.second->level() >= level_id2) {
				total_cost += cost_id2;
			}
		}
	}
	return total_cost;
}

bool WorldGuildManager::LoadTributes()
{
	TributeData td{};
	td.tier_count = 0;

	tribute_list.clear();

	auto tributes = TributesRepository::All(*m_content_db);
	for (auto const& t : tributes) {
		td.name            = t.name;
		td.description     = t.descr;
		td.unknown         = t.unknown;
		td.is_guild        = t.isguild == 0 ? false : true;
		tribute_list[t.id] = td;
	}

	LogInfo("Loaded [{}] tributes", Strings::Commify(tributes.size()));

	auto tribute_levels = TributeLevelsRepository::GetWhere(*m_content_db, "TRUE ORDER BY tribute_id, level");

	for (auto const& t : tribute_levels) {
		uint32 id = t.tribute_id;

		if (tribute_list.count(id) != 1) {
			LogError("Error in LoadTributes: unknown tribute [{}] in tribute_levels", (unsigned long)id);
			continue;
		}

		TributeData& cur = tribute_list[id];
		if (cur.tier_count >= MAX_TRIBUTE_TIERS) {
			LogError("Error in LoadTributes: on tribute [{}] more tiers defined than permitted", (unsigned long)id);
			continue;
		}

		TributeLevel_Struct& s = cur.tiers[cur.tier_count];

		s.level           = t.level;
		s.cost            = t.cost;
		s.tribute_item_id = t.item_id;

		cur.tier_count++;
	}

	LogInfo("Loaded [{}] tribute levels", Strings::Commify(tribute_levels.size()));

	return true;
}

bool WorldGuildManager::RefreshGuild(uint32 guild_id)
{
	auto temp_guild = GetGuildByGuildID(guild_id);
	if (!temp_guild) {
		return false;
	}

	BaseGuildManager::GuildInfo temp_guild_detail;

	if (temp_guild) {
		temp_guild_detail.tribute = temp_guild->tribute;
	}

	if (guild_id <= 0) {
		LogError("Requested to refresh guild id [{}] but id must be greater than 0.", guild_id);
		return false;
	}

	auto db_guild = GuildsRepository::FindOne(*m_db, guild_id);
	if (!db_guild.id) {
		LogGuilds("Guild ID [{}] not found in database.", db_guild.id);
		return false;
	}

	LogGuilds("Found guild id [{}].  Loading details.....", db_guild.id);
	_CreateGuild(db_guild.id, db_guild.name, db_guild.leader, db_guild.minstatus, db_guild.motd, db_guild.motd_setter, db_guild.channel, db_guild.url, db_guild.favor);
	auto guild = GetGuildByGuildID(guild_id);
	if (!guild) {
		LogError("Error refreshing guild id {}", guild_id);
		return false;
	}

	auto where_filter = fmt::format("guild_id = '{}'", guild_id);
	auto guild_ranks = GuildRanksRepository::GetWhere(*m_db, where_filter);
	for (auto const& r : guild_ranks) {
		guild->rank_names[r.rank_] = r.title;
	}

	where_filter = fmt::format("guild_id = '{}'", guild_id);
	auto guild_permissions = GuildPermissionsRepository::GetWhere(*m_db, where_filter);
	for (auto const& p : guild_permissions) {
		guild->functions[p.perm_id].id         = p.id;
		guild->functions[p.perm_id].guild_id   = p.guild_id;
		guild->functions[p.perm_id].perm_id    = p.perm_id;
		guild->functions[p.perm_id].perm_value = p.permission;
	}

	auto guild_tributes = GuildTributesRepository::FindOne(*m_db, guild_id);
	if (guild_tributes.guild_id) {
		guild->tribute.id_1           = guild_tributes.tribute_id_1;
		guild->tribute.id_2           = guild_tributes.tribute_id_2;
		guild->tribute.id_1_tier      = guild_tributes.tribute_id_1_tier;
		guild->tribute.id_2_tier      = guild_tributes.tribute_id_2_tier;
		guild->tribute.enabled        = guild_tributes.enabled;
		guild->tribute.time_remaining = guild_tributes.time_remaining;
	}
	if (temp_guild_detail.tribute.enabled == 1) {
		guild->tribute = temp_guild_detail.tribute;
	}

	LogGuilds("Successfully refreshed guild id [{}] from the [WORLD] database", guild_id);
	LogGuilds("Timer has [{}] time remaining from the [WORLD] refresh.", guild->tribute.time_remaining);

	return true;
}

void WorldGuildManager::SendGuildTributeFavorAndTimer(uint32 guild_id, uint32 favor, uint32 time)
{
	auto sp   = new ServerPacket(ServerOP_GuildTributeFavAndTimer, sizeof(GuildTributeFavorTimer_Struct));
	auto data = (GuildTributeFavorTimer_Struct *)sp->pBuffer;
	data->guild_id      = guild_id;
	data->guild_favor   = favor;
	data->tribute_timer = time;
	data->trophy_timer  = 0;

	ZSList::Instance()->SendPacketToZonesWithGuild(guild_id, sp);
	safe_delete(sp)
}
