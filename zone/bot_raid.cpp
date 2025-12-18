/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.org)

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

#include "common/data_verification.h"
#include "zone/bot_command.h"
#include "zone/bot.h"
#include "zone/client.h"
#include "zone/doors.h"
#include "zone/object.h"
#include "zone/quest_parser_collection.h"
#include "zone/raids.h"

std::vector<RaidMember> Raid::GetRaidGroupMembers(uint32 gid)
{
	std::vector<RaidMember> raid_group_members;
	raid_group_members.clear();

	for (const auto& m : members)
	{
		if (m.member && m.group_number == gid)
		{
			raid_group_members.push_back(m);
		}
	}
	return raid_group_members;
}

// Returns Bot members that are in the raid
// passing in owner will return all Bots that have the same owner.
std::vector<Bot*> Raid::GetRaidBotMembers(uint32 owner)
{
	std::vector<Bot*> raid_members_bots;
	raid_members_bots.clear();

	for (const auto& m : members) {
		if (
			m.member &&
			m.member->IsBot()
			) {
			auto b_member = m.member->CastToBot();
			if (owner && b_member->GetBotOwnerCharacterID() == owner) {
				raid_members_bots.emplace_back(b_member);
			} else if (!owner) {
				raid_members_bots.emplace_back(b_member);
			}
		}
	}

	return raid_members_bots;
}

// Returns Bot members that are in the group specified
// passing in owner will return only Bots that have the same owner.
std::vector<Bot*> Raid::GetRaidGroupBotMembers(uint32 gid)
{
	std::vector<Bot*> raid_members_bots;
	raid_members_bots.clear();

	for (const auto& m : members) {
		if (
			m.member &&
			m.member->IsBot() &&
			m.group_number == gid
		) {
			auto b_member = m.member->CastToBot();
			raid_members_bots.emplace_back(b_member);
			raid_members_bots.emplace_back(b_member);
		}
	}

	return raid_members_bots;
}

void Raid::HandleBotGroupDisband(uint32 owner, uint32 gid)
{
	auto raid_members_bots = gid != RAID_GROUPLESS ? GetRaidGroupBotMembers(gid) : GetRaidBotMembers(owner);

	// If any of the bots are a group leader then re-create the botgroup on disband, dropping any clients
	for (const auto& b: raid_members_bots) {

		// Remove the entire BOT group in this case
		if (b && gid != RAID_GROUPLESS && IsRaidMember(b->GetName()) && IsGroupLeader(b->GetName())) {
			auto r_group_members = GetRaidGroupMembers(GetGroup(b->GetName()));

			for (auto m: r_group_members) {
				if (m.member->IsBot()) {
					auto b_member = m.member->CastToBot();
					Bot::RemoveBotFromRaid(b_member);
				}
			}
		} else {
			Bot::RemoveBotFromRaid(b);
		}
	}
}

// we need to cleanup any camped/offline bots when the owner leaves the Raid
void Raid::HandleOfflineBots(uint32 owner) {
	std::list<BotsAvailableList> bots_list;
	if (!database.botdb.LoadBotsList(owner, bots_list)) {
		return;
	}

	for (const auto& b: bots_list) {
		if (IsRaidMember(b.bot_name)) {
			for (const auto& m: members) {
				if (m.is_bot && strcmp(m.member_name, b.bot_name) == 0) {
					uint32 gid = GetGroup(m.member_name);
					SendRaidGroupRemove(m.member_name, gid);
					RemoveMember(m.member_name);
					GroupUpdate(gid);
					if (!RaidCount()) {
						DisbandRaid();
					}
				}
			}
		}
	}
}

void Bot::ProcessRaidInvite(Mob* invitee, Client* invitor, bool group_invite) {

	if (!invitee || !invitor) {
		return;
	}

	if (invitee->IsBot() &&
		invitor->HasRaid() &&
		invitee->GetOwner()->HasRaid() &&
		invitor->GetRaid()->IsRaidMember(invitee->GetOwner()->GetName())
	) {
		// If the Bot Owner is in our raid we need to be able to invite their Bots
	}
	else if (invitee->IsBot() && (invitee->CastToBot()->GetBotOwnerCharacterID() != invitor->CharacterID())) {
		invitor->Message(Chat::Red, "%s's owner needs to be in your raid to be able to invite them.", invitee->GetCleanName());
		return;
	}

	Raid* raid = entity_list.GetRaidByClient(invitor);

	if (raid && raid->RaidCount() >= MAX_RAID_MEMBERS) {
		invitor->MessageString(Chat::Red, RAID_IS_FULL);
		return;
	}
	Bot::CreateBotRaid(invitee, invitor, group_invite, raid);
}

void Bot::CreateBotRaid(Mob* invitee, Client* invitor, bool group_invite, Raid* raid) {

	Group* g_invitee = invitee->GetGroup();
	Group* g_invitor = invitor->GetGroup();

	if (g_invitee && invitor->IsClient() && !g_invitee->IsLeader(invitee)) {
		if (g_invitee->GetLeader()) {
			invitor->Message(
				Chat::Red,
				fmt::format(
					"You can only invite group leaders or ungrouped bots. Try {} instead.",
					g_invitee->GetLeader()->GetCleanName()
				).c_str()
			);
			return;
		} else {
			invitor->Message(Chat::Red, "You can only invite group leaders or ungrouped bots.");
			return;
		}
	}

	bool new_raid = false;
	if (!raid) {
		new_raid = true;
		raid = new Raid(invitor);
		entity_list.AddRaid(raid);
		raid->SetRaidDetails();
	}

	// Add Invitor to new raid
	if (new_raid) {
		if (g_invitor) {
			ProcessBotGroupAdd(g_invitor, raid, nullptr, true, true);
		} else {
			raid->SendRaidCreate(invitor);
			raid->AddMember(invitor, 0xFFFFFFFF, true, false, true);
			raid->SendMakeLeaderPacketTo(invitor->GetName(), invitor);
			if (raid->IsLocked()) {
				raid->SendRaidLockTo(invitor);
			}
		}
	}

	// Add Bot Group or Client Bot Group to raid
	if (g_invitee) {
		ProcessBotGroupAdd(g_invitee, raid);

	// Add individual client to raid
	} else if (invitee->IsClient()) {
		ProcessBotGroupAdd(g_invitee, raid, invitee->CastToClient());

	// Add individual bot to raid
	} else {
		auto gid = raid->GetGroup(invitor->GetName());
		auto b = invitee->CastToBot();

		// gives us a choice to either invite directly into the clients Raid Group, or just into the Raid
		if (group_invite && raid->GroupCount(gid) < MAX_GROUP_MEMBERS) {
			raid->AddBot(b, gid);
		} else {
			raid->AddBot(b);
		}
	}
}

void Bot::ProcessBotGroupAdd(Group* group, Raid* raid, Client* client, bool new_raid, bool initial) {

	uint32 raid_free_group_id = raid->GetFreeGroup();
	if (group) {
		for (const auto& m : group->members) {
			if (m) {
				if (m && m->IsBot()) {
					raid->AddBot(m->CastToBot(), raid_free_group_id, false, !raid->GroupCount(raid_free_group_id), false);
				} else if (m && m->IsClient()) {
					auto c = m->CastToClient();
					raid->SendRaidCreate(c);
					raid->AddMember(
						c,
						raid_free_group_id,
						new_raid,
						!raid->GroupCount(raid_free_group_id),
						false
					);
					raid->SendMakeLeaderPacketTo(raid->leadername, c);
					raid->SendBulkRaid(c);
				}
			}
		}
		group->JoinRaidXTarget(raid, initial);
		group->DisbandGroup(true);
	} else if (client) {
		raid->SendRaidCreate(client);
		raid->AddMember(client, RAID_GROUPLESS, false, false, true);
		raid->SendMakeLeaderPacketTo(raid->leadername, client);
		raid->SendBulkRaid(client);
		if (raid->IsLocked()) {
			raid->SendRaidLockTo(client);
		}
	}

	raid->GroupUpdate(raid_free_group_id);
}

void Client::SpawnRaidBotsOnConnect(Raid* raid) {
	std::list<BotsAvailableList> bots_list;
	if (!database.botdb.LoadBotsList(CharacterID(), bots_list) || bots_list.empty()) {
		return;
	}

	std::vector<RaidMember> r_members = raid->GetMembers();
	for (const auto& m: r_members) {
		if (strlen(m.member_name) != 0) {

			for (const auto& b : bots_list) {
				if (strcmp(m.member_name, b.bot_name) == 0) {
					std::string buffer = "^spawn ";
					buffer.append(m.member_name);
					std::string silent = " silent";
					buffer.append(silent);
					bot_command_real_dispatch(this, buffer.c_str());
					auto bot = entity_list.GetBotByBotName(m.member_name);

					if (bot) {
						bot->SetRaidGrouped(true);
						bot->SetStoredRaid(raid);
						bot->p_raid_instance = raid;
						bot->SetVerifiedRaid(false);
					}
				}
			}
		}
	}
}

