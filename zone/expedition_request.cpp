/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2020 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include "expedition_request.h"

#include "common/repositories/character_expedition_lockouts_repository.h"
#include "zone/client.h"
#include "zone/groups.h"
#include "zone/raids.h"
#include "zone/string_ids.h"

ExpeditionRequest::ExpeditionRequest(const DynamicZone& dz, Client& client, bool silent)
	: m_dz(&dz), m_requester(&client), m_silent(silent)
{
}

bool ExpeditionRequest::Validate()
{
	// a message is sent to leader for every member that fails a requirement

	BenchTimer benchmark;

	bool requirements_met = false;

	if (Raid* raid = m_requester->GetRaid())
	{
		m_is_raid = true;
		requirements_met = CanRaidRequest(raid);
	}
	else if (Group* group = m_requester->GetGroup())
	{
		requirements_met = CanGroupRequest(group);
	}
	else // solo request
	{
		m_leader = m_requester;
		m_leader_id = m_requester->CharacterID();
		m_leader_name = m_requester->GetName();
		requirements_met = CanMembersJoin({m_leader_name});
	}

	auto elapsed = benchmark.elapsed();
	LogExpeditions("Create validation for [{}] members took [{}s]", m_members.size(), elapsed);

	return requirements_met;
}

bool ExpeditionRequest::CanRaidRequest(Raid* raid)
{
	m_leader = raid->GetLeader();
	m_leader_name = raid->leadername;
	m_leader_id = m_leader ? m_leader->CharacterID() : database.GetCharacterID(raid->leadername);

	// live (as of September 16, 2020) supports creation even if raid count exceeds
	// expedition max. members are added up to the max ordered by group number.
	auto raid_members = raid->GetMembers();

	if (raid_members.size() > m_dz->GetMaxPlayers())
	{
		// leader first then raid group, order within a raid group may not be what is displayed
		std::sort(raid_members.begin(), raid_members.end(),
			[&](const RaidMember& lhs, const RaidMember& rhs) {
				if (m_leader_name == lhs.member_name) { return true; }
				if (m_leader_name == rhs.member_name) { return false; }
				return lhs.group_number < rhs.group_number;
			});
	}

	// live still performs conflict checks for all members even those beyond max
	std::vector<std::string> member_names;
	for (int i = 0; i < raid_members.size(); ++i)
	{
		member_names.emplace_back(raid_members[i].member_name);
	}

	return CanMembersJoin(member_names);
}

bool ExpeditionRequest::CanGroupRequest(Group* group)
{
	m_leader = nullptr;
	if (group->GetLeader() && group->GetLeader()->IsClient())
	{
		m_leader = group->GetLeader()->CastToClient();
	}

	// Group::GetLeaderName() is broken if group formed across zones, ask database instead
	m_leader_name = m_leader ? m_leader->GetName() : group->GetLeaderName();
	m_leader_id = m_leader ? m_leader->CharacterID() : database.GetCharacterID(m_leader_name);

	std::vector<std::string> member_names;
	member_names.emplace_back(m_leader_name); // leader always added first

	for (int i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if (group->membername[i][0] && m_leader_name != group->membername[i])
		{
			member_names.emplace_back(group->membername[i]);
		}
	}

	return CanMembersJoin(member_names);
}

bool ExpeditionRequest::CanMembersJoin(const std::vector<std::string>& member_names)
{
	if (member_names.empty())
	{
		return false;
	}

	bool requirements_met = true;

	if (CheckMembersForConflicts(member_names))
	{
		requirements_met = false;
	}

	// live only checks player count requirement after other expensive checks pass (?)
	// maybe it's done intentionally as a way to preview lockout conflicts
	if (requirements_met)
	{
		requirements_met = IsPlayerCountValidated();
	}

	return requirements_met;
}

bool ExpeditionRequest::SaveLeaderLockouts(const std::vector<DzLockout>& lockouts)
{
	bool has_replay_lockout = false;

	for (const auto& lockout : lockouts)
	{
		if (!lockout.IsExpired())
		{
			// db prevents duplicate event names
			m_lockouts.push_back(lockout);

			if (lockout.IsReplay())
			{
				has_replay_lockout = true;
			}
		}
	}

	return has_replay_lockout;
}

bool ExpeditionRequest::CheckMembersForConflicts(const std::vector<std::string>& member_names)
{
	// order of member_names is preserved by queries for use with max member truncation
	auto entries = DynamicZonesRepository::GetCharactersWithDz(database, member_names, static_cast<int>(DynamicZoneType::Expedition));
	if (entries.empty())
	{
		LogExpeditions("Failed to load members for expedition request");
		return true;
	}

	bool is_solo = (member_names.size() == 1);
	bool has_conflicts = false;

	std::vector<uint32_t> char_ids;
	for (const auto& character : entries)
	{
		if (is_solo && character.dz_id != 0)
		{
			// live doesn't bother checking replay lockout here
			SendLeaderMemberInExpedition(character.name, is_solo);
			return true;
		}

		m_members.emplace_back(character.id, character.name, DynamicZoneMemberStatus::Online);
		char_ids.push_back(character.id);
	}

	auto lockouts = CharacterExpeditionLockoutsRepository::GetLockouts(database, char_ids, m_dz->GetName());

	// on live if leader has a replay lockout it never checks for event conflicts
	bool leader_replay = false;
	auto it = lockouts.find(m_leader_id);
	if (it != lockouts.end())
	{
		leader_replay = SaveLeaderLockouts(it->second);
	}

	for (const auto& character : entries)
	{
		if (character.dz_id != 0)
		{
			has_conflicts = true;
			SendLeaderMemberInExpedition(character.name, is_solo);
		}

		auto it = lockouts.find(character.id);
		if (it != lockouts.end())
		{
			for (const auto& lockout : it->second)
			{
				if (!lockout.IsExpired())
				{
					auto is_event = [&](const auto& l) { return l.IsEvent(lockout.Event()); };

					// replay timers were sorted by query so they show up before event conflicts
					if (lockout.IsReplay())
					{
						has_conflicts = true;
						SendLeaderMemberReplayLockout(character.name, lockout, is_solo);
					}
					else if (!leader_replay && character.id != m_leader_id && std::ranges::none_of(m_lockouts, is_event))
					{
						// leader doesn't have this lockout
						has_conflicts = true;
						SendLeaderMemberEventLockout(character.name, lockout);
					}
				}
			}
		}
	}

	return has_conflicts;
}

void ExpeditionRequest::SendLeaderMessage(uint16_t chat_type, uint32_t string_id, std::initializer_list<std::string> args)
{
	if (!m_silent)
	{
		Client::SendCrossZoneMessageString(m_leader, m_leader_name, chat_type, string_id, args);
	}
}

void ExpeditionRequest::SendLeaderMemberInExpedition(const std::string& name, bool is_solo)
{
	if (m_silent)
	{
		return;
	}

	if (is_solo)
	{
		SendLeaderMessage(Chat::Red, DZ_YOU_BELONG);
	}
	else if (m_requester)
	{
		// message string 9265 (not in emu clients)
		Client::SendCrossZoneMessage(m_leader, m_leader_name, Chat::Red, fmt::format(
			"{} attempted to create an expedition but {} already belongs to one.", m_requester->GetName(), name));
	}
}

void ExpeditionRequest::SendLeaderMemberReplayLockout(const std::string& name, const DzLockout& lockout, bool is_solo)
{
	if (m_silent)
	{
		return;
	}

	auto time = lockout.GetTimeRemainingStrs();
	if (is_solo)
	{
		SendLeaderMessage(Chat::Red, DZ_REPLAY_YOU, { time.days, time.hours, time.mins });
	}
	else
	{
		SendLeaderMessage(Chat::Red, DZ_REPLAY_OTHER, { name, time.days, time.hours, time.mins });
	}
}

void ExpeditionRequest::SendLeaderMemberEventLockout(const std::string& name, const DzLockout& lockout)
{
	if (m_silent)
	{
		return;
	}

	auto time = lockout.GetTimeRemainingStrs();
	SendLeaderMessage(Chat::Red, DZ_EVENT_TIMER, { name, lockout.Event(), time.days, time.hours, time.mins });
}

bool ExpeditionRequest::IsPlayerCountValidated()
{
	// note: offline group members count towards requirement but not added to expedition
	bool requirements_met = true;

	auto bypass_status = RuleI(Expedition, MinStatusToBypassPlayerCountRequirements);
	auto gm_bypass = (m_requester && m_requester->GetGM() && m_requester->Admin() >= bypass_status);

	if (!gm_bypass && m_members.size() < m_dz->GetMinPlayers())
	{
		requirements_met = false;

		SendLeaderMessage(Chat::System, DZ_PLAYER_COUNT, {
			fmt::format_int(m_members.size()).str(),
			fmt::format_int(m_dz->GetMinPlayers()).str(),
			fmt::format_int(m_dz->GetMaxPlayers()).str()
		});
	}

	if (gm_bypass) {
		m_requester->Message(Chat::White, "Your GM Status allows you to bypass expedition minimum and maximum player restrictions.");
	}

	return requirements_met;
}
