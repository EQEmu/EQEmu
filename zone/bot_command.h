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

#pragma once

#include "common/types.h"
#include "bot.h"
#include "dialogue_window.h"

class Client;
class Seperator;


namespace
{
#define HP_RATIO_DELTA 5.0f

	enum { EffectIDFirst = 1, EffectIDLast = 12 };

#define VALIDATECLASSID(x) ((x >= Class::Warrior && x <= Class::Berserker) ? (x) : (0))

	// ActionableTarget action_type
#define FRIENDLY true
#define ENEMY false

	enum {
		AFT_None = 0,
		AFT_Value,
		AFT_GenderRace,
		AFT_Race
	};
}

namespace MyBots
{
	static bool IsMyBot(Client *bot_owner, Mob *my_bot) {
		if (!bot_owner || !my_bot || !my_bot->IsBot()) {
			return false;
		}

		auto test_bot = my_bot->CastToBot();
		if (!test_bot->GetOwner() || !test_bot->GetOwner()->IsClient() || test_bot->GetOwner()->CastToClient() != bot_owner) {
			return false;
		}

		return true;
	}

	static bool IsMyBotInTargetsGroup(Client *bot_owner, Mob *grouped_bot) {
		if (!bot_owner || !grouped_bot || !grouped_bot->GetGroup() || !IsMyBot(bot_owner, grouped_bot)) {
			return false;
		}

		auto target_mob = bot_owner->GetTarget();
		if (!target_mob) {
			return false;
		}

		if (!target_mob->GetGroup() || (!target_mob->IsClient() && !target_mob->IsBot())) {
			return false;
		}

		return (grouped_bot->GetGroup() == target_mob->GetGroup());
	}

	static bool IsMyBotInPlayerGroup(Client *bot_owner, Mob *grouped_bot, Client *grouped_player) {
		if (!bot_owner || !grouped_player || !grouped_player->GetGroup() || !grouped_bot || !grouped_bot->GetGroup() || !IsMyBot(bot_owner, grouped_bot)) {
			return false;
		}

		return (grouped_player->GetGroup() == grouped_bot->GetGroup());
	}

	static void UniquifySBL(std::vector<Bot*> &sbl) {
		sbl.erase(std::remove(sbl.begin(), sbl.end(), nullptr), sbl.end());
		std::sort(sbl.begin(), sbl.end());
		sbl.erase(std::unique(sbl.begin(), sbl.end()), sbl.end());
	}

	static void PopulateSBL_ByTargetedBot(Client *bot_owner, std::vector<Bot*> &sbl, bool clear_list = true) {
		if (clear_list) {
			sbl.clear();
		}

		if (IsMyBot(bot_owner, bot_owner->GetTarget())) {
			sbl.push_back(bot_owner->GetTarget()->CastToBot());
		}

		if (!clear_list) {
			UniquifySBL(sbl);
		}
	}

	static void PopulateSBL_ByNamedBot(Client *bot_owner, std::vector<Bot*> &sbl, const char* name, bool clear_list = true) {
		if (clear_list) {
			sbl.clear();
		}

		if (!bot_owner || !name) {
			return;
		}

		auto selectable_bot_list = entity_list.GetBotsByBotOwnerCharacterID(bot_owner->CharacterID());
		for (auto bot_iter : selectable_bot_list) {
			if (!strcasecmp(bot_iter->GetCleanName(), name)) {
				sbl.push_back(bot_iter);
				return;
			}
		}

		if (!clear_list) {
			UniquifySBL(sbl);
		}
	}

	static void PopulateSBL_ByMyGroupedBots(Client *bot_owner, std::vector<Bot*> &sbl, bool clear_list = true) {
		if (clear_list) {
			sbl.clear();
		}

		if (!bot_owner) {
			return;
		}

		if (!bot_owner->GetGroup() && !bot_owner->GetRaid()) {
			return;
		}

		if (bot_owner->IsRaidGrouped()) {
			Raid* raid = bot_owner->GetRaid();
			if (!raid) {
				return;
			}

			uint32 raid_group = raid->GetGroup(bot_owner);
			if (raid_group >= MAX_RAID_GROUPS) {
				return;
			}

			for (const auto& m : raid->members) {
				if (m.member && m.group_number == raid_group) {
					if (!MyBots::IsMyBot(bot_owner, m.member->CastToBot())) {
						continue;
					}

					sbl.push_back(m.member->CastToBot());
				}
			}
		}
		else {
			std::list<Bot*> group_list;
			bot_owner->GetGroup()->GetBotList(group_list);
			for (auto member_iter : group_list) {
				if (IsMyBot(bot_owner, member_iter)) {
					sbl.push_back(member_iter);
				}
			}
		}

		if (!clear_list) {
			UniquifySBL(sbl);
		}
	}

	static void PopulateSBL_ByMyRaidBots(Client* bot_owner, std::vector<Bot*>& sbl, bool clear_list = true) {
		if (clear_list) {
			sbl.clear();
		}

		if (!bot_owner) {
			return;
		}

		if (!bot_owner->GetRaid()) {
			return;
		}

		Raid* raid = bot_owner->GetRaid();
		if (!raid) {
			return;
		}

		for (const auto& m : raid->members) {
			if (m.member) {
				if (!MyBots::IsMyBot(bot_owner, m.member->CastToBot())) {
					continue;
				}

				sbl.push_back(m.member->CastToBot());
			}
		}

		if (!clear_list) {
			UniquifySBL(sbl);
		}
	}

	static void PopulateSBL_ByTargetsGroupedBots(Client *bot_owner, std::vector<Bot*> &sbl, bool clear_list = true) {
		if (clear_list) {
			sbl.clear();
		}

		if (!bot_owner) {
			return;
		}

		auto target_mob = bot_owner->GetTarget();
		if (!target_mob || (!target_mob->GetGroup() && !target_mob->GetRaid()) || (!target_mob->IsClient() && !target_mob->IsBot())) {
			return;
		}

		if (bot_owner->IsRaidGrouped()) {
			Raid* raid = bot_owner->GetRaid();
			if (!raid) {
				return;
			}

			if (MyBots::IsMyBot(bot_owner, target_mob)) {
				uint32 raid_group = raid->GetGroup(target_mob->CastToClient());
				if (raid_group >= MAX_RAID_GROUPS) {
					return;
				}

				for (const auto& m : raid->members) {
					if (m.member && m.group_number == raid_group) {
						if (!MyBots::IsMyBot(bot_owner, m.member->CastToBot())) {
							continue;
						}

						sbl.push_back(m.member->CastToBot());
					}
				}
			}
		}
		else {
			std::list<Bot*> group_list;
			bot_owner->GetGroup()->GetBotList(group_list);
			for (auto member_iter : group_list) {
				if (IsMyBot(bot_owner, member_iter)) {
					sbl.push_back(member_iter);
				}
			}
		}

		if (!clear_list) {
			UniquifySBL(sbl);
		}
	}

	static void PopulateSBL_ByNamesGroupedBots(Client *bot_owner, std::vector<Bot*> &sbl, const char* name, bool clear_list = true) {
		if (clear_list) {
			sbl.clear();
		}

		if (!bot_owner || !name) {
			return;
		}

		Mob* named_mob = nullptr;
		std::list<Mob*> l_mob_list;
		entity_list.GetMobList(l_mob_list);
		for (auto mob_iter : l_mob_list) {
			if (!strcasecmp(mob_iter->GetCleanName(), name)) {
				named_mob = mob_iter;
				break;
			}
		}

		if (!named_mob || !MyBots::IsMyBot(bot_owner, named_mob) || (!named_mob->GetGroup() && !named_mob->GetRaid()) || (!named_mob->IsClient() && !named_mob->IsBot())) {
			return;
		}

		if (bot_owner->IsRaidGrouped()) {
			Raid* raid = bot_owner->GetRaid();
			if (!raid) {
				return;
			}

			uint32 raid_group = raid->GetGroup(named_mob->CastToClient());
			if (raid_group >= MAX_RAID_GROUPS) {
				return;
			}

			for (const auto& m : raid->members) {
				if (m.member && m.group_number == raid_group) {
					if (!MyBots::IsMyBot(bot_owner, m.member->CastToBot())) {
						continue;
					}

					sbl.push_back(m.member->CastToBot());
				}
			}
		}
		else {
			std::list<Bot*> group_list;
			bot_owner->GetGroup()->GetBotList(group_list);
			for (auto member_iter : group_list) {
				if (IsMyBot(bot_owner, member_iter)) {
					sbl.push_back(member_iter);
				}
			}
		}

		if (!clear_list) {
			UniquifySBL(sbl);
		}
	}

	static void PopulateSBL_ByHealRotation(Client *bot_owner, std::vector<Bot*> &sbl, const char* name, bool clear_list = true) {
		if (clear_list) {
			sbl.clear();
		}

		if (!bot_owner || (!name && !bot_owner->GetTarget())) {
			return;
		}

		std::vector<Bot*> selectable_bot_list;
		if (name) {
			PopulateSBL_ByNamedBot(bot_owner, selectable_bot_list, name);
		}
		else {
			PopulateSBL_ByTargetedBot(bot_owner, selectable_bot_list);
		}

		if (selectable_bot_list.empty() || !selectable_bot_list.front()->IsHealRotationMember()) {
			return;
		}

		auto hrm = (*selectable_bot_list.front()->MemberOfHealRotation())->MemberList();
		for (auto hrm_iter : *hrm) {
			if (IsMyBot(bot_owner, hrm_iter)) {
				sbl.push_back(hrm_iter);
			}
		}

		auto hrt = (*selectable_bot_list.front()->MemberOfHealRotation())->TargetList();
		for (auto hrt_iter : *hrt) {
			if (IsMyBot(bot_owner, hrt_iter)) {
				sbl.push_back(hrt_iter->CastToBot());
			}
		}

		UniquifySBL(sbl);
	}

	static void PopulateSBL_ByHealRotationMembers(Client *bot_owner, std::vector<Bot*> &sbl, const char* name, bool clear_list = true) {
		if (clear_list) {
			sbl.clear();
		}

		if (!bot_owner || (!name && !bot_owner->GetTarget())) {
			return;
		}

		std::vector<Bot*> selectable_bot_list;
		if (name) {
			PopulateSBL_ByNamedBot(bot_owner, selectable_bot_list, name);
		}
		else {
			PopulateSBL_ByTargetedBot(bot_owner, selectable_bot_list);
		}

		if (selectable_bot_list.empty() || !selectable_bot_list.front()->IsHealRotationMember()) {
			return;
		}

		auto hrm = (*selectable_bot_list.front()->MemberOfHealRotation())->MemberList();
		for (auto hrm_iter : *hrm) {
			if (IsMyBot(bot_owner, hrm_iter)) {
				sbl.push_back(hrm_iter);
			}
		}

		if (!clear_list) {
			UniquifySBL(sbl);
		}
	}

	static void PopulateSBL_ByHealRotationTargets(Client *bot_owner, std::vector<Bot*> &sbl, const char* name, bool clear_list = true) {
		if (clear_list) {
			sbl.clear();
		}

		if (!bot_owner || (!name && !bot_owner->GetTarget())) {
			return;
		}

		std::vector<Bot*> selectable_bot_list;
		if (name) {
			PopulateSBL_ByNamedBot(bot_owner, selectable_bot_list, name);
		}
		else {
			PopulateSBL_ByTargetedBot(bot_owner, selectable_bot_list);
		}

		if (selectable_bot_list.empty() || !selectable_bot_list.front()->IsHealRotationMember()) {
			return;
		}

		auto hrm = (*selectable_bot_list.front()->MemberOfHealRotation())->TargetList();
		for (auto hrm_iter : *hrm) {
			if (IsMyBot(bot_owner, hrm_iter)) {
				sbl.push_back(static_cast<Bot*>(hrm_iter));
			}
		}

		if (!clear_list) {
			UniquifySBL(sbl);
		}
	}

	static void PopulateSBL_BySpawnedBots(Client *bot_owner, std::vector<Bot*> &sbl) { // should be used for most spell casting commands
		sbl.clear();
		if (!bot_owner) {
			return;
		}

		sbl = entity_list.GetBotsByBotOwnerCharacterID(bot_owner->CharacterID());
		sbl.erase(std::remove(sbl.begin(), sbl.end(), nullptr), sbl.end());
	}

	static void PopulateSBL_BySpawnedBotsClass(Client * bot_owner, std::vector<Bot*> &sbl, uint16 cls, bool clear_list = true) {
		if (clear_list) {
			sbl.clear();
		}

		if (!bot_owner || !cls) {
			return;
		}

		auto selectable_bot_list = entity_list.GetBotsByBotOwnerCharacterID(bot_owner->CharacterID());
		for (auto bot_iter : selectable_bot_list) {
			if (bot_iter->GetClass() != cls) {
				continue;
			}

			sbl.push_back(bot_iter);
		}

		if (!clear_list) {
			UniquifySBL(sbl);
		}
	}

	static void PopulateSBL_BySpawnedBotsRace(Client* bot_owner, std::vector<Bot*>& sbl, uint16 race, bool clear_list = true) {
		if (clear_list) {
			sbl.clear();
		}

		if (!bot_owner || !race) {
			return;
		}

		auto selectable_bot_list = entity_list.GetBotsByBotOwnerCharacterID(bot_owner->CharacterID());
		for (auto bot_iter : selectable_bot_list) {
			if (bot_iter->GetBaseRace() != race) {
				continue;
			}

			sbl.push_back(bot_iter);
		}

		if (!clear_list) {
			UniquifySBL(sbl);
		}
	}

	static void PopulateSBL_ByAtMMR(Client* bot_owner, std::vector<Bot*>& sbl, bool clear_list = true) {
		if (clear_list) {
			sbl.clear();
		}

		if (!bot_owner) {
			return;
		}

		auto selectable_bot_list = entity_list.GetBotsByBotOwnerCharacterID(bot_owner->CharacterID());
		for (auto bot_iter : selectable_bot_list) {
			if (!bot_iter->GetMaxMeleeRange()) {
				continue;
			}
			sbl.push_back(bot_iter);
		}
		if (!clear_list) {
			UniquifySBL(sbl);
		}
	}
};

namespace ActionableTarget
{
	static bool AmIInPlayerGroup(Client *bot_owner, Client *grouped_player) {
		if (!bot_owner || !grouped_player || !bot_owner->GetGroup() || !grouped_player->GetGroup())
			return false;

		return (bot_owner->GetGroup() == grouped_player->GetGroup());
	}

	static bool IsAttackable(Client *bot_owner, Mob* target_mob) {
		if (!bot_owner || !target_mob || bot_owner == target_mob)
			return false;

		return bot_owner->IsAttackAllowed(target_mob);
	}

	static Client* AsSingle_ByPlayer(Client *bot_owner, bool return_me_on_null_target = true) {
		if (!bot_owner)
			return nullptr;

		if (!bot_owner->GetTarget()) {
			if (return_me_on_null_target)
				return bot_owner;
			else
				return nullptr;
		}

		if (!bot_owner->GetTarget()->IsClient())
			return nullptr;

		return bot_owner->GetTarget()->CastToClient();
	}

	static Client* AsGroupMember_ByPlayer(Client *bot_owner, bool return_me_on_null_target = true) {
		if (!bot_owner)
			return nullptr;

		if (!bot_owner->GetTarget()) {
			if (return_me_on_null_target)
				return bot_owner;
			else
				return nullptr;
		}

		if (!bot_owner->GetTarget()->IsClient() || !AmIInPlayerGroup(bot_owner, bot_owner->GetTarget()->CastToClient()))
			return nullptr;

		return bot_owner->GetTarget()->CastToClient();
	}

	static Corpse* AsCorpse_ByPlayer(Client *bot_owner) {
		if (!bot_owner || !bot_owner->GetTarget() || !bot_owner->GetTarget()->IsPlayerCorpse())
			return nullptr;

		return bot_owner->GetTarget()->CastToCorpse();
	}

	static Mob* AsSingle_ByAttackable(Client *bot_owner) {
		if (!IsAttackable(bot_owner, bot_owner->GetTarget()))
			return nullptr;

		return bot_owner->GetTarget();
	}

	static bool IsFriendlyAllowed(Mob* target_mob) {
		if (!target_mob || target_mob->IsClient() || target_mob->IsBot() || (target_mob->IsPet() && target_mob->GetOwner() && (target_mob->GetOwner()->IsClient() || target_mob->GetOwner()->IsBot())) || target_mob->IsPlayerCorpse())
			return true;

		return false;
	}
}

namespace ActionableBots
{
	enum ABType {
		ABT_None = 0,
		ABT_Target,
		ABT_ByName,
		ABT_OwnerGroup,
		ABT_OwnerRaid,
		ABT_TargetGroup,
		ABT_NamesGroup,
		ABT_HealRotation,
		ABT_HealRotationMembers,
		ABT_HealRotationTargets,
		ABT_MMR,
		ABT_Class,
		ABT_Race,
		ABT_Spawned,
		ABT_All
	};

	enum ABMask {
		ABM_None = 0,
		ABM_Target = (1 << (ABT_Target - 1)),
		ABM_ByName = (1 << (ABT_ByName - 1)),
		ABM_OwnerGroup = (1 << (ABT_OwnerGroup - 1)),
		ABM_OwnerRaid = (1 << (ABT_OwnerRaid - 1)),
		ABM_TargetGroup = (1 << (ABT_TargetGroup - 1)),
		ABM_NamesGroup = (1 << (ABT_NamesGroup - 1)),
		ABM_HealRotation = (1 << (ABT_HealRotation - 1)),
		ABM_HealRotationMembers = (1 << (ABT_HealRotationMembers - 1)),
		ABM_HealRotationTargets = (1 << (ABT_HealRotationTargets - 1)),
		ABM_MMR = (1 << (ABT_MMR - 1)),
		ABM_Class = (1 << (ABT_Class - 1)),
		ABM_Race = (1 << (ABT_Race - 1)),
		ABM_Spawned = (1 << (ABT_Spawned - 1)),
		ABM_All = (1 << (ABT_All - 1)),
		ABM_Spawned_All = (3 << (ABT_Spawned - 1)),
		ABM_NoFilter = ~0,
		// grouped values
		ABM_Type1 = (ABM_Target | ABM_ByName | ABM_OwnerGroup | ABM_OwnerRaid | ABM_TargetGroup | ABM_NamesGroup | ABM_HealRotationTargets | ABM_Spawned | ABM_MMR | ABM_Class | ABM_Race),
		ABM_Type2 = (ABM_ByName | ABM_OwnerGroup | ABM_OwnerRaid | ABM_NamesGroup | ABM_HealRotation | ABM_Spawned | ABM_MMR | ABM_Class | ABM_Race)
	};

	// Populates 'sbl'
	static ABType PopulateSBL(Client* bot_owner, std::string ab_type_arg, std::vector<Bot*> &sbl, int ab_mask, const char* name = nullptr, uint16 classrace = 0, bool clear_list = true, bool suppress_message = false) {
		if (clear_list) {
			sbl.clear();
		}

		if (!bot_owner) {
			return ABT_None;
		}

		auto ab_type = ABT_None;
		if (!ab_type_arg.compare("target") || ab_type_arg.empty()) {
			ab_type = ABT_Target;
		}
		else if (!ab_type_arg.compare("byname")) {
			ab_type = ABT_ByName;
		}
		else if (!ab_type_arg.compare("ownergroup")) {
			ab_type = ABT_OwnerGroup;
		}
		else if (!ab_type_arg.compare("ownerraid")) {
			ab_type = ABT_OwnerRaid;
		}
		else if (!ab_type_arg.compare("targetgroup")) {
			ab_type = ABT_TargetGroup;
		}
		else if (!ab_type_arg.compare("namesgroup")) {
			ab_type = ABT_NamesGroup;
		}
		else if (!ab_type_arg.compare("healrotation")) {
			ab_type = ABT_HealRotation;
		}
		else if (!ab_type_arg.compare("healrotationmembers")) {
			ab_type = ABT_HealRotationMembers;
		}
		else if (!ab_type_arg.compare("healrotationtargets")) {
			ab_type = ABT_HealRotationTargets;
		}
		else if (!ab_type_arg.compare("mmr")) {
			ab_type = ABT_MMR;
		}
		else if (!ab_type_arg.compare("byclass")) {
			ab_type = ABT_Class;
		}
		else if (!ab_type_arg.compare("byrace")) {
			ab_type = ABT_Race;
		}
		else if (!ab_type_arg.compare("spawned")) {
			ab_type = ABT_Spawned;
		}
		else if (!ab_type_arg.compare("all")) {
			ab_type = ABT_All;
		}

		if (ab_type_arg.empty()) {
			ab_type_arg = "target";
		}

		switch (ab_type) {
			case ABT_Target:
				if (ab_mask & ABM_Target) {
					MyBots::PopulateSBL_ByTargetedBot(bot_owner, sbl, clear_list);
				}
				break;
			case ABT_ByName:
				if (ab_mask & ABM_ByName) {
					MyBots::PopulateSBL_ByNamedBot(bot_owner, sbl, name, clear_list);
				}
				break;
			case ABT_OwnerGroup:
				if (ab_mask & ABM_OwnerGroup) {
					MyBots::PopulateSBL_ByMyGroupedBots(bot_owner, sbl, clear_list);
				}
				break;
			case ABT_OwnerRaid:
				if (ab_mask & ABM_OwnerRaid) {
					MyBots::PopulateSBL_ByMyRaidBots(bot_owner, sbl, clear_list);
				}
				break;
			case ABT_TargetGroup:
				if (ab_mask & ABM_TargetGroup) {
					MyBots::PopulateSBL_ByTargetsGroupedBots(bot_owner, sbl, clear_list);
				}
				break;
			case ABT_NamesGroup:
				if (ab_mask & ABM_NamesGroup) {
					MyBots::PopulateSBL_ByNamesGroupedBots(bot_owner, sbl, name, clear_list);
				}
				break;
			case ABT_HealRotation:
				if (ab_mask & ABM_HealRotation) {
					MyBots::PopulateSBL_ByHealRotation(bot_owner, sbl, name, clear_list);
				}
				break;
			case ABT_HealRotationMembers:
				if (ab_mask & ABM_HealRotationMembers) {
					MyBots::PopulateSBL_ByHealRotationMembers(bot_owner, sbl, name, clear_list);
				}
				break;
			case ABT_HealRotationTargets:
				if (ab_mask & ABM_HealRotationTargets) {
					MyBots::PopulateSBL_ByHealRotationTargets(bot_owner, sbl, name, clear_list);
				}
				break;
			case ABT_MMR:
				if (ab_mask & ABM_MMR) {
					MyBots::PopulateSBL_ByAtMMR(bot_owner, sbl, clear_list);
				}
				break;
			case ABT_Class:
				if (ab_mask & ABM_Class) {
					MyBots::PopulateSBL_BySpawnedBotsClass(bot_owner, sbl, classrace, clear_list);
				}
				break;
			case ABT_Race:
				if (ab_mask & ABM_Race) {
					MyBots::PopulateSBL_BySpawnedBotsRace(bot_owner, sbl, classrace, clear_list);
				}
				break;
			case ABT_Spawned:
			case ABT_All:
				if (ab_mask & ABM_Spawned_All) {
					MyBots::PopulateSBL_BySpawnedBots(bot_owner, sbl);
				}
				break;
			default:
				break;
		}
		if (sbl.empty() && ab_type != ABT_All) {
			if (suppress_message) {
				return ABT_None;
			}

			if (!ab_mask) {
				bot_owner->Message(Chat::White, "Command passed null 'ActionableBot' criteria");
			}
			else if (ab_mask & ab_type) {
				if (classrace) {
					bot_owner->Message(Chat::White, "You have no spawned bots meeting this criteria - type: '%s', %s: '%i'", ab_type_arg.c_str(), ab_mask & ABM_Class ? "class" : ab_mask & ABM_Race ? "race" : "", classrace);
				}
				else {
					bot_owner->Message(Chat::White, "You have no spawned bots meeting this criteria - type: '%s', name: '%s'", ab_type_arg.c_str(), ((name) ? (name) : ("")));
				}
			}
			else {
				bot_owner->Message(Chat::White, "This command does not allow 'ActionableBot' criteria '%s'", ab_type_arg.c_str());
			}

			return ABT_None;
		}

		return ab_type;
	}

	static Bot* AsGroupMember_ByClass(Client *bot_owner, Client *bot_grouped_player, uint8 cls, bool petless = false) {
		if (!bot_owner || !bot_grouped_player) {
			return nullptr;
		}

		if (!bot_grouped_player->GetGroup() && !bot_grouped_player->GetRaid()) {
			return nullptr;
		}

		if (bot_owner->IsRaidGrouped()) {
			Raid* raid = bot_grouped_player->GetRaid();
			if (!raid) {
				return nullptr;
			}

			uint32 raid_group = raid->GetGroup(bot_grouped_player);
			if (raid_group >= MAX_RAID_GROUPS) {
				return nullptr;
			}

			for (const auto& m : raid->members) {
				if (m.member && m.group_number == raid_group) {
					if (!MyBots::IsMyBot(bot_owner, m.member->CastToBot())) {
						continue;
					}

					if (m.member->GetClass() != cls) {
						continue;
					}

					if (petless && m.member->GetPet()) {
						continue;
					}

					return static_cast<Bot*>(m.member->CastToBot());
				}
			}
		}
		else {
			std::list<Mob*> group_list;
			bot_grouped_player->GetGroup()->GetMemberList(group_list);
			for (auto member_iter : group_list) {
				if (!MyBots::IsMyBot(bot_owner, member_iter)) {
					continue;
				}

				if (member_iter->GetClass() != cls) {
					continue;
				}

				if (petless && member_iter->GetPet()) {
					continue;
				}

				return static_cast<Bot*>(member_iter);
			}
		}

		return nullptr;
	}

	static Bot* AsGroupMember_ByMinLevelAndClass(Client *bot_owner, Client *bot_grouped_player, uint8 minlvl, uint8 cls, bool petless = false) {
		// This function can be nixed if we can enforce bot level as owner level..and the level check can then be moved to the spell loop in the command function
		if (!bot_owner || !bot_grouped_player)
			return nullptr;
		if (!bot_grouped_player->GetGroup())
			return nullptr;

		std::list<Mob*> group_list;
		bot_grouped_player->GetGroup()->GetMemberList(group_list);
		for (auto member_iter : group_list) {
			if (!MyBots::IsMyBot(bot_owner, member_iter))
				continue;
			if (member_iter->GetLevel() < minlvl || member_iter->GetClass() != cls)
				continue;
			if (petless && member_iter->GetPet())
				continue;

			return static_cast<Bot*>(member_iter);
		}

		return nullptr;
	}

	static Bot* AsSpawned_ByClass(Client *bot_owner, std::vector<Bot*> &sbl, uint8 cls, bool petless = false) {
		if (!bot_owner) {
			return nullptr;
		}

		for (auto bot_iter : sbl) {
			if (!MyBots::IsMyBot(bot_owner, bot_iter)) {
				continue;
			}

			if (bot_iter->GetClass() != cls) {
				continue;
			}

			if (petless && bot_iter->GetPet()) {
				continue;
			}

			return bot_iter;
		}

		return nullptr;
	}

	static Bot* AsSpawned_ByMinLevelAndClass(Client *bot_owner, std::vector<Bot*> &sbl, uint8 minlvl, uint8 cls, bool petless = false) {
		// This function can be nixed if we can enforce bot level as owner level..and the level check can then be moved to the spell loop in the command function
		if (!bot_owner)
			return nullptr;

		for (auto bot_iter : sbl) {
			if (!MyBots::IsMyBot(bot_owner, bot_iter))
				continue;
			if (bot_iter->GetLevel() < minlvl || bot_iter->GetClass() != cls)
				continue;
			if (petless && bot_iter->GetPet())
				continue;

			return bot_iter;
		}

		return nullptr;
	}

	static Bot* AsTarget_ByBot(Client *bot_owner) {
		if (!bot_owner || !MyBots::IsMyBot(bot_owner, bot_owner->GetTarget()))
			return nullptr;

		return bot_owner->GetTarget()->CastToBot();
	}

	static Bot* AsNamed_ByBot(Client *bot_owner, std::string bot_name) {
		if (!bot_owner || bot_name.empty())
			return nullptr;

		std::vector<Bot*> selectable_bot_list;
		MyBots::PopulateSBL_BySpawnedBots(bot_owner, selectable_bot_list);
		for (auto bot_iter : selectable_bot_list) {
			if (!bot_name.compare(bot_iter->GetCleanName()))
				return bot_iter;
		}

		return nullptr;
	}

	static Bot* Select_ByClass(Client* bot_owner, int target_type, std::vector<Bot*>& sbl, uint8 cls, Mob* target_mob = nullptr, bool petless = false) {
		if (!bot_owner || sbl.empty())
			return nullptr;

		for (auto bot_iter : sbl) {
			if (!MyBots::IsMyBot(bot_owner, bot_iter))
				continue;
			if (bot_iter->GetClass() != cls)
				continue;
			if (petless && bot_iter->GetPet())
				continue;
			if (target_type == ST_GroupTeleport) {
				if (!target_mob)
					return nullptr;
				else if (bot_iter->GetGroup() != target_mob->GetGroup())
					continue;
			}

			return bot_iter;
		}

		return nullptr;
	}

	static Bot* Select_ByMinLevelAndClass(Client* bot_owner, int target_type, std::vector<Bot*>& sbl, uint8 minlvl, uint8 cls, Mob* target_mob = nullptr, bool petless = false) {
		if (!bot_owner || sbl.empty())
			return nullptr;

		for (auto bot_iter : sbl) {
			if (!MyBots::IsMyBot(bot_owner, bot_iter))
				continue;
			if (bot_iter->GetLevel() < minlvl || bot_iter->GetClass() != cls)
				continue;
			if (petless && bot_iter->GetPet())
				continue;
			if (target_type == ST_GroupTeleport) {
				if (!target_mob)
					return nullptr;
				else if (bot_iter->GetGroup() != target_mob->GetGroup())
					continue;
			}

			return bot_iter;
		}

		return nullptr;
	}

	// Filters actual 'sbl' list
	static void Filter_ByClasses(Client* bot_owner, std::vector<Bot*>& sbl, uint16 class_mask) {
		std::erase_if(sbl, [bot_owner](Bot* l) { return (!MyBots::IsMyBot(bot_owner, l)); });
		std::erase_if(sbl, [class_mask](Bot* l) { return (GetPlayerClassBit(l->GetClass()) & (~class_mask)); });
	}

	static void Filter_ByMinLevel(Client* bot_owner, std::vector<Bot*>& sbl, uint8 min_level) {
		std::erase_if(sbl, [bot_owner](Bot* l) { return (!MyBots::IsMyBot(bot_owner, l)); });
		std::erase_if(sbl, [min_level](Bot* l) { return (l->GetLevel() < min_level); });
	}

	static void Filter_ByRanged(Client* bot_owner, std::vector<Bot*>& sbl) {
		std::erase_if(sbl, [bot_owner](Bot* l) { return (!MyBots::IsMyBot(bot_owner, l)); });
		std::erase_if(sbl, [bot_owner](Bot* l) { return (l->IsBotRanged()); });
	}

	static void Filter_ByHighestSkill(Client* bot_owner, std::vector<Bot*>& sbl, EQ::skills::SkillType skill_type, float& skill_value) {
		std::erase_if(sbl, [bot_owner](Bot* l) { return (!MyBots::IsMyBot(bot_owner, l)); });
		skill_value = 0.0f;

		float mod_skill_value = 0.0f;
		const Bot* skilled_bot = nullptr;
		for (auto bot_iter : sbl) {
			float base_skill_value = bot_iter->GetSkill(skill_type);
			if (base_skill_value == 0.0f)
				continue;

			mod_skill_value = base_skill_value;
			for (int16 index = EQ::invslot::EQUIPMENT_BEGIN; index <= EQ::invslot::EQUIPMENT_END; ++index) {
				const EQ::ItemInstance* indexed_item = bot_iter->GetBotItem(index);
				if (indexed_item && indexed_item->GetItem()->SkillModType == skill_type)
					mod_skill_value += (base_skill_value * (((float)indexed_item->GetItem()->SkillModValue) / 100.0f));
			}

			if (!skilled_bot) {
				skill_value = mod_skill_value;
				skilled_bot = bot_iter;
			}
			else if (mod_skill_value > skill_value) {
				skill_value = mod_skill_value;
				skilled_bot = bot_iter;
			}
		}
		std::erase_if(sbl, [skilled_bot](Bot* l) { return (l != skilled_bot); });
	}

	static void Filter_ByHighestPickLock(Client* bot_owner, std::vector<Bot*>& sbl, float& pick_lock_value) {
		std::erase_if(sbl, [bot_owner](Bot* l) { return (!MyBots::IsMyBot(bot_owner, l)); });
		std::erase_if(sbl, [bot_owner](Bot* l) { return (l->GetClass() != Class::Rogue && l->GetClass() != Class::Bard); });
		std::erase_if(sbl, [bot_owner](Bot* l) { return (l->GetClass() == Class::Rogue && l->GetLevel() < 5); });
		std::erase_if(sbl, [bot_owner](Bot* l) { return (l->GetClass() == Class::Bard && l->GetLevel() < 40); });

		ActionableBots::Filter_ByHighestSkill(bot_owner, sbl, EQ::skills::SkillPickLock, pick_lock_value);
	}
}

#define	BOT_COMMAND_CHAR '^'

typedef void (*BotCmdFuncPtr)(Client *,const Seperator *);

typedef struct {
	int access;
	const char *desc;			// description of bot command
	BotCmdFuncPtr function;		// null means perl function
} BotCommandRecord;

extern int (*bot_command_dispatch)(Client *,char const*);
extern int bot_command_count;	// number of bot commands loaded

// Bot Command System:
int bot_command_init(void);
void bot_command_deinit(void);
int bot_command_add(std::string bot_command_name, const char *desc, int access, BotCmdFuncPtr function);
int bot_command_not_avail(Client *c, const char *message);
int bot_command_real_dispatch(Client *c, char const *message);

// Bot Commands
void bot_command_actionable(Client *c, const Seperator *sep);
void bot_command_apply_poison(Client *c, const Seperator *sep);
void bot_command_apply_potion(Client* c, const Seperator* sep);
void bot_command_attack(Client *c, const Seperator *sep);
void bot_command_behind_mob(Client* c, const Seperator* sep);
void bot_command_blocked_buffs(Client* c, const Seperator* sep);
void bot_command_blocked_pet_buffs(Client* c, const Seperator* sep);
void bot_command_bot(Client *c, const Seperator *sep);
void bot_command_bot_settings(Client* c, const Seperator* sep);
void bot_command_cast(Client* c, const Seperator* sep);
void bot_command_discipline(Client* c, const Seperator* sep);
void bot_command_distance_ranged(Client* c, const Seperator* sep);
void bot_command_class_race_list(Client* c, const Seperator* sep);
void bot_command_click_item(Client* c, const Seperator* sep);
void bot_command_copy_settings(Client* c, const Seperator* sep);
void bot_command_default_settings(Client* c, const Seperator* sep);
void bot_command_depart(Client *c, const Seperator *sep);
void bot_command_find_aliases(Client *c, const Seperator *sep);
void bot_command_follow(Client *c, const Seperator *sep);
void bot_command_guard(Client *c, const Seperator *sep);
void bot_command_heal_rotation(Client *c, const Seperator *sep);
void bot_command_help(Client *c, const Seperator *sep);
void bot_command_hold(Client *c, const Seperator *sep);
void bot_command_illusion_block(Client* c, const Seperator* sep);
void bot_command_inventory(Client *c, const Seperator *sep);
void bot_command_item_use(Client *c, const Seperator *sep);
void bot_command_max_melee_range(Client* c, const Seperator* sep);
void bot_command_owner_option(Client *c, const Seperator *sep);
void bot_command_pet(Client *c, const Seperator *sep);
void bot_command_pick_lock(Client *c, const Seperator *sep);
void bot_command_pickpocket(Client* c, const Seperator* sep);
void bot_command_precombat(Client* c, const Seperator* sep);
void bot_command_pull(Client *c, const Seperator *sep);
void bot_command_release(Client *c, const Seperator *sep);
void bot_command_sit_hp_percent(Client* c, const Seperator* sep);
void bot_command_sit_in_combat(Client* c, const Seperator* sep);
void bot_command_sit_mana_percent(Client* c, const Seperator* sep);
void bot_command_spell_aggro_checks(Client* c, const Seperator* sep);
void bot_command_spell_announce_cast(Client* c, const Seperator* sep);
void bot_command_spell_delays(Client* c, const Seperator* sep);
void bot_command_spell_engaged_priority(Client* c, const Seperator* sep);
void bot_command_spell_holds(Client* c, const Seperator* sep);
void bot_command_spell_idle_priority(Client* c, const Seperator* sep);
void bot_command_spell_max_hp_pct(Client* c, const Seperator* sep);
void bot_command_spell_max_mana_pct(Client* c, const Seperator* sep);
void bot_command_spell_max_thresholds(Client* c, const Seperator* sep);
void bot_command_spell_min_hp_pct(Client* c, const Seperator* sep);
void bot_command_spell_min_mana_pct(Client* c, const Seperator* sep);
void bot_command_spell_min_thresholds(Client* c, const Seperator* sep);
void bot_command_spell_pursue_priority(Client* c, const Seperator* sep);
void bot_command_spell_resist_limits(Client* c, const Seperator* sep);
void bot_command_spell_target_count(Client* c, const Seperator* sep);
void bot_command_spell_list(Client* c, const Seperator *sep);
void bot_command_spell_settings_add(Client* c, const Seperator *sep);
void bot_command_spell_settings_delete(Client* c, const Seperator *sep);
void bot_command_spell_settings_list(Client* c, const Seperator *sep);
void bot_command_spell_settings_toggle(Client* c, const Seperator *sep);
void bot_command_spell_settings_update(Client* c, const Seperator *sep);
void bot_command_spelltype_ids(Client* c, const Seperator* sep);
void bot_command_spelltype_names(Client* c, const Seperator* sep);
void bot_spell_info_dialogue_window(Client* c, const Seperator *sep);
void bot_command_enforce_spell_list(Client* c, const Seperator* sep);
void bot_command_suspend(Client *c, const Seperator *sep);
void bot_command_taunt(Client *c, const Seperator *sep);
void bot_command_timer(Client* c, const Seperator* sep);
void bot_command_track(Client *c, const Seperator *sep);
void bot_command_view_combos(Client *c, const Seperator *sep);

// Bot Subcommands
void bot_command_appearance(Client *c, const Seperator *sep);
void bot_command_beard_color(Client *c, const Seperator *sep);
void bot_command_beard_style(Client *c, const Seperator *sep);
void bot_command_camp(Client *c, const Seperator *sep);
void bot_command_clone(Client *c, const Seperator *sep);
void bot_command_create(Client *c, const Seperator *sep);
void bot_command_delete(Client *c, const Seperator *sep);
void bot_command_details(Client *c, const Seperator *sep);
void bot_command_dye_armor(Client *c, const Seperator *sep);
void bot_command_eyes(Client *c, const Seperator *sep);
void bot_command_face(Client *c, const Seperator *sep);
void bot_command_follow_distance(Client *c, const Seperator *sep);
void bot_command_hair_color(Client *c, const Seperator *sep);
void bot_command_hairstyle(Client *c, const Seperator *sep);
void bot_command_heritage(Client *c, const Seperator *sep);
void bot_command_inspect_message(Client *c, const Seperator *sep);
void bot_command_list_bots(Client *c, const Seperator *sep);
void bot_command_report(Client *c, const Seperator *sep);
void bot_command_set_assistee(Client* c, const Seperator* sep);
void bot_command_spawn(Client *c, const Seperator *sep);
void bot_command_stance(Client *c, const Seperator *sep);
void bot_command_stop_melee_level(Client *c, const Seperator *sep);
void bot_command_suffix(Client *c, const Seperator *sep);
void bot_command_summon(Client *c, const Seperator *sep);
void bot_command_surname(Client *c, const Seperator *sep);
void bot_command_tattoo(Client *c, const Seperator *sep);
void bot_command_title(Client *c, const Seperator *sep);
void bot_command_toggle_helm(Client *c, const Seperator *sep);
void bot_command_toggle_ranged(Client* c, const Seperator* sep);
void bot_command_update(Client *c, const Seperator *sep);
void bot_command_woad(Client *c, const Seperator *sep);

void bot_command_heal_rotation_adaptive_targeting(Client *c, const Seperator *sep);
void bot_command_heal_rotation_add_member(Client *c, const Seperator *sep);
void bot_command_heal_rotation_add_target(Client *c, const Seperator *sep);
void bot_command_heal_rotation_adjust_critical(Client *c, const Seperator *sep);
void bot_command_heal_rotation_adjust_safe(Client *c, const Seperator *sep);
void bot_command_heal_rotation_casting_override(Client *c, const Seperator *sep);
void bot_command_heal_rotation_change_interval(Client *c, const Seperator *sep);
void bot_command_heal_rotation_clear_hot(Client *c, const Seperator *sep);
void bot_command_heal_rotation_clear_targets(Client *c, const Seperator *sep);
void bot_command_heal_rotation_create(Client *c, const Seperator *sep);
void bot_command_heal_rotation_delete(Client *c, const Seperator *sep);
void bot_command_heal_rotation_fast_heals(Client *c, const Seperator *sep);
void bot_command_heal_rotation_list(Client *c, const Seperator *sep);
void bot_command_heal_rotation_remove_member(Client *c, const Seperator *sep);
void bot_command_heal_rotation_remove_target(Client *c, const Seperator *sep);
void bot_command_heal_rotation_reset_limits(Client *c, const Seperator *sep);
void bot_command_heal_rotation_save(Client *c, const Seperator *sep);
void bot_command_heal_rotation_set_hot(Client *c, const Seperator *sep);
void bot_command_heal_rotation_start(Client *c, const Seperator *sep);
void bot_command_heal_rotation_stop(Client *c, const Seperator *sep);
void bot_command_inventory_give(Client *c, const Seperator *sep);
void bot_command_inventory_list(Client *c, const Seperator *sep);
void bot_command_inventory_remove(Client *c, const Seperator *sep);
void bot_command_inventory_window(Client *c, const Seperator *sep);
void bot_command_pet_get_lost(Client *c, const Seperator *sep);
void bot_command_pet_remove(Client *c, const Seperator *sep);
void bot_command_pet_set_type(Client *c, const Seperator *sep);


// bot command helpers
bool helper_bot_appearance_fail(Client *bot_owner, Bot *my_bot, uint8 fail_type, const char* type_desc);
void helper_bot_appearance_form_final(Client *bot_owner, Bot *my_bot);
void helper_bot_appearance_form_update(Bot *my_bot);
uint32 helper_bot_create(Client *bot_owner, std::string bot_name, uint8 bot_class, uint16 bot_race, uint8 bot_gender);
int helper_bot_follow_option_chain(Client *bot_owner);
bool helper_command_disabled(Client *bot_owner, bool rule_value, const char *command);
bool helper_command_alias_fail(Client *bot_owner, const char* command_handler, const char *alias, const char *command);
bool helper_is_help_or_usage(const char* arg);
bool helper_no_available_bots(Client *bot_owner, Bot *my_bot = nullptr);
void helper_send_available_subcommands(Client *bot_owner, const char* command_simile, std::vector<const char*> subcommand_list);
void helper_send_usage_required_bots(Client *bot_owner, uint16 spell_type);
void SendSpellTypeWindow(Client* c, const Seperator* sep);
