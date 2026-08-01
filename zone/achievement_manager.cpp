/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.
*/
#include "achievement_manager.h"
#include "../common/data_bucket.h"

#include "common/seperator.h"
#include "common/skills.h"
#include "common/strings.h"
#include "zone/client.h"
#include "common/classes.h"   // GetPlayerClassBit (class_mask gating)
#include "zone/npc.h"
#include "zone/titles.h"
#include "zone/zone.h"
#include "zone/zonedb.h"

#include "fmt/format.h"

#include <algorithm>
#include <cstring>

AchievementManager achievement_manager;

namespace {
	constexpr uint32 RewardStatusPending = 0;
	constexpr uint32 RewardStatusClaimed = 1;
	constexpr uint32 RewardStatusFailed = 2;

	uint32 ToUInt(const char *value)
	{
		return value ? Strings::ToUnsignedInt(value) : 0;
	}

	uint64 ToUInt64(const char *value)
	{
		return value ? Strings::ToUnsignedBigInt(value) : 0;
	}

	std::string SqlString(const std::string &value)
	{
		return fmt::format("'{}'", Strings::Escape(value));
	}

	std::string ObjectiveDisplayName(const std::string &type, const std::string &target_name, uint32 required_count)
	{
		if (!target_name.empty()) {
			return target_name;
		}

		if (Strings::EqualFold(type, "level")) {
			return fmt::format("Reach level {}", required_count);
		}

		return type;
	}

	std::string ProtocolValue(std::string value, size_t maximum_length = 0)
	{
		for (auto &c : value) {
			if (c == '|' || c == '\r' || c == '\n' || c == '\t') {
				c = ' ';
			}
		}

		if (maximum_length && value.size() > maximum_length) {
			if (maximum_length > 3) {
				value.resize(maximum_length - 3);
				value.append("...");
			}
			else {
				value.resize(maximum_length);
			}
		}

		return value;
	}

	std::string RewardDisplayName(
		const std::string &type,
		const std::string &preview_text,
		const std::string &data_text,
		uint32 reward_id,
		uint32 amount
	)
	{
		if (!preview_text.empty()) {
			return preview_text;
		}

		if (Strings::EqualFold(type, "title_text")) {
			return fmt::format("Title: {}", data_text);
		}

		if (Strings::EqualFold(type, "title_suffix")) {
			return fmt::format("Suffix: {}", data_text);
		}

		if (Strings::EqualFold(type, "title_set")) {
			return fmt::format("Title set {}", reward_id);
		}

		if (Strings::EqualFold(type, "item")) {
			return fmt::format("Item {}", reward_id);
		}

		if (Strings::EqualFold(type, "currency")) {
			return fmt::format("{} currency {}", amount, reward_id);
		}

		if (Strings::EqualFold(type, "coin")) {
			return fmt::format("{} copper", amount);
		}

		if (Strings::EqualFold(type, "scribe_spell")) {
			return fmt::format("Aura spell {}", reward_id);
		}

		if (Strings::EqualFold(type, "live_item_request")) {
			return "Leveled item request";
		}

		return type;
	}
}

void AchievementManager::HandleCommand(Client *client, const Seperator *sep)
{
	if (!client || !sep) {
		return;
	}

	if (sep->argnum < 1) {
		SendNativeWindow(client);
		return;
	}

	if (!strcasecmp(sep->arg[1], "help")) {
		SendHelp(client);
		return;
	}

	if (
		!strcasecmp(sep->arg[1], "window") ||
		!strcasecmp(sep->arg[1], "ui") ||
		!strcasecmp(sep->arg[1], "open") ||
		!strcasecmp(sep->arg[1], "panel")
	) {
		SendNativeWindow(client);
		return;
	}

	if (!strcasecmp(sep->arg[1], "native")) {
		auto number_arg = [sep](int argument_index) -> uint32 {
			return sep->argnum >= argument_index && sep->IsNumber(argument_index) ?
				Strings::ToUnsignedInt(sep->arg[argument_index]) :
				0;
		};

		if (sep->argnum < 2) {
			SendNativeWindow(client);
			return;
		}

		if (
			!strcasecmp(sep->arg[2], "show") ||
			!strcasecmp(sep->arg[2], "open") ||
			!strcasecmp(sep->arg[2], "window") ||
			!strcasecmp(sep->arg[2], "status") ||
			!strcasecmp(sep->arg[2], "snapshot") ||
			!strcasecmp(sep->arg[2], "refresh")
		) {
			SendNativeWindow(client, number_arg(3), number_arg(4));
			return;
		}

		if (!strcasecmp(sep->arg[2], "category")) {
			if (sep->argnum < 3 || !sep->IsNumber(3)) {
				client->Message(Chat::White, "Usage: #ach native category [category_id]");
				return;
			}

			SendNativeCategory(client, Strings::ToUnsignedInt(sep->arg[3]), number_arg(4));
			return;
		}

		if (!strcasecmp(sep->arg[2], "detail")) {
			if (sep->argnum < 3 || !sep->IsNumber(3)) {
				client->Message(Chat::White, "Usage: #ach native detail [achievement_id]");
				return;
			}

			client->Message(Chat::White, "ACH|objectives|clear");
			client->Message(Chat::White, "ACH|rewards|clear");
			SendNativeDetail(client, Strings::ToUnsignedInt(sep->arg[3]));
			client->Message(Chat::White, "ACH|window|show");
			return;
		}

		if (!strcasecmp(sep->arg[2], "check")) {
			RecheckAutomatic(client);
			SendNativeWindow(client, number_arg(3), number_arg(4));
			return;
		}

		client->Message(Chat::White, "Usage: #ach native [show|refresh|category|detail|check]");
		return;
	}

	if (!strcasecmp(sep->arg[1], "status")) {
		SendStatus(client);
		return;
	}

	if (!strcasecmp(sep->arg[1], "categories") || !strcasecmp(sep->arg[1], "list")) {
		SendCategories(client);
		return;
	}

	if (!strcasecmp(sep->arg[1], "category")) {
		if (sep->argnum < 2 || !sep->IsNumber(2)) {
			client->Message(Chat::White, "Usage: #ach category [category_id]");
			return;
		}

		SendCategory(client, Strings::ToUnsignedInt(sep->arg[2]));
		return;
	}

	if (!strcasecmp(sep->arg[1], "detail")) {
		if (sep->argnum < 2 || !sep->IsNumber(2)) {
			client->Message(Chat::White, "Usage: #ach detail [achievement_id]");
			return;
		}

		SendDetail(client, Strings::ToUnsignedInt(sep->arg[2]));
		return;
	}

	if (!strcasecmp(sep->arg[1], "rewards") || !strcasecmp(sep->arg[1], "reward")) {
		SendRewardQueue(client);
		return;
	}

	if (!strcasecmp(sep->arg[1], "claim")) {
		if (sep->argnum >= 2 && sep->IsNumber(2)) {
			ClaimPendingRewards(client, Strings::ToUnsignedBigInt(sep->arg[2]));
			return;
		}

		ClaimPendingRewards(client);
		return;
	}

	if (!strcasecmp(sep->arg[1], "check")) {
		RecheckAutomatic(client);
		SendStatus(client);
		return;
	}

	SendHelp(client);
}

void AchievementManager::ProcessLevel(Client *client)
{
	if (!client) {
		return;
	}

	const uint32 character_id = client->CharacterID();
	const uint32 level = client->GetLevel();

	auto results = database.QueryDatabase(
		fmt::format(
			"SELECT `id`, `achievement_id`, `required_count` "
			"FROM `custom_achievement_objectives` "
			"WHERE `objective_type` = 'level' AND `required_count` <= {}",
			level
		)
	);

	if (!results.Success()) {
		return;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		const uint32 objective_id = ToUInt(row[0]);
		const uint32 achievement_id = ToUInt(row[1]);
		const uint32 required_count = ToUInt(row[2]);

		UpdateObjectiveProgress(character_id, objective_id, level, level >= required_count);
		TryCompleteAchievement(client, achievement_id);
	}
}

void AchievementManager::ProcessZoneVisit(Client *client)
{
	if (!client || !zone) {
		return;
	}

	ProcessMatchedObjectives(
		client,
		fmt::format(
			"o.`objective_type` = 'zone_visit' AND o.`zone_id` = {}",
			zone->GetZoneID()
		),
		1,
		true
	);
}

void AchievementManager::ProcessTaskComplete(Client *client, uint32 task_id)
{
	if (!client || !task_id) {
		return;
	}

	ProcessMatchedObjectives(
		client,
		fmt::format(
			"o.`objective_type` = 'task_complete' AND o.`target_id` = {}",
			task_id
		),
		1,
		true
	);
}

void AchievementManager::ProcessSkill(Client *client, uint32 skill_id, uint32 value)
{
	if (!client) {
		return;
	}

	ProcessMatchedObjectives(
		client,
		fmt::format(
			"o.`objective_type` = 'skill' AND o.`target_id` = {}",
			skill_id
		),
		value,
		true
	);
}

void AchievementManager::ProcessKill(Client *client, NPC *npc)
{
	if (!client || !npc || !zone) {
		return;
	}

	const std::string clean_name = Strings::Escape(Strings::ToLower(npc->GetCleanName()));
	ProcessMatchedObjectives(
		client,
		fmt::format(
			"("
			"(o.`objective_type` = 'any_kill') OR "
			"(o.`objective_type` = 'npc_kill' AND o.`target_id` = {}) OR "
			"(o.`objective_type` = 'npc_name_kill' AND o.`zone_id` = {} AND LOWER(o.`target_name`) = '{}') OR "
			"(o.`objective_type` = 'zone_kill' AND o.`zone_id` = {}) OR "
			"(o.`objective_type` = 'race_kill' AND o.`target_id` = {}) OR "
			"(o.`objective_type` = 'bodytype_kill' AND o.`target_id` = {})"
			")",
			npc->GetNPCTypeID(),
			zone->GetZoneID(),
			clean_name,
			zone->GetZoneID(),
			npc->GetRace(),
			npc->GetBodyType()
		),
		1,
		false
	);
}

void AchievementManager::ProcessItemReceive(Client *client, uint32 item_id)
{
	if (!client || !item_id) {
		return;
	}

	// item_receive completes when the target item enters the player's possession
	// (quest turn-in, loot, summon). Class-gating is handled by ProcessMatchedObjectives
	// via the objective's class_mask (so a class epic only credits that class).
	ProcessMatchedObjectives(
		client,
		fmt::format("(o.`objective_type` = 'item_receive' AND o.`target_id` = {})", item_id),
		1,
		false
	);
}

// AoTv4: a death AT THE LEVEL CAP. The roguelite's terminal event -- you took a run as far as it
// goes and then lost it -- and the trigger for the region-unlock achievement chain.
//
// ⚠️⚠️ EACH DEATH IS ONE POINT OF PROGRESS ON EVERY MATCHING OBJECTIVE, which is what makes a chain
// of achievements with goals 1,2,3,4,5 fire on the first, second, third... capped death without any
// per-achievement bookkeeping. The engine already tracks progress per objective per character, so
// the chain is expressed entirely in DATA (five rows) rather than in code.
// ⚠️ absolute_progress is FALSE: the count must accumulate across deaths. True would overwrite the
// stored progress with 1 every time and no achievement past the first would ever complete.
//
// ⚠️ The caller decides what "at max" means -- this is not the place to re-derive the level cap.
// era_system owns that (region ceiling vs hard cap vs era cap) and it is a Lua-side answer.
void AchievementManager::ProcessDeathAtMaxLevel(Client *client)
{
	if (!client) {
		return;
	}

	ProcessMatchedObjectives(
		client,
		"(o.`objective_type` = 'death_at_max')",
		1,
		false
	);
}

void AchievementManager::ProcessItemInventory(Client *client)
{
	if (!client) {
		return;
	}

	// Backstop for item_receive: credit any target item the character already owns
	// (e.g. an epic obtained before this feature, or via a path we don't hook live).
	// Only queries the small set of distinct item_receive target ids, then checks
	// possession — cheap enough to run alongside the other RecheckAutomatic passes.
	auto results = database.QueryDatabase(
		"SELECT DISTINCT o.`target_id` FROM `custom_achievement_objectives` o "
		"JOIN `custom_achievements` a ON a.`id` = o.`achievement_id` AND a.`enabled` = 1 "
		"WHERE o.`objective_type` = 'item_receive' AND o.`target_id` > 0"
	);
	if (!results.Success()) {
		return;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		const uint32 item_id = ToUInt(row[0]);
		if (item_id && client->GetInv().HasItem(item_id, 1, invWhereWorn | invWherePersonal | invWhereBank | invWhereCursor) != INVALID_INDEX) {
			ProcessItemReceive(client, item_id);
		}
	}
}

void AchievementManager::SendHelp(Client *client)
{
	client->Message(Chat::White, "Achievement commands:");
	client->Message(Chat::White, "#ach - Open the native achievement window.");
	client->Message(Chat::White, "#ach window - Reopen the native achievement window.");
	client->Message(Chat::White, "#ach status - Show your achievement totals.");
	client->Message(Chat::White, "#ach categories - List achievement categories.");
	client->Message(Chat::White, "#ach category [id] - List achievements in a category.");
	client->Message(Chat::White, "#ach detail [id] - Show achievement objectives.");
	client->Message(Chat::White, "#ach rewards - List pending achievement rewards.");
	client->Message(Chat::White, "#ach claim [reward_id|all] - Claim pending achievement rewards.");
	client->Message(Chat::White, "#ach check - Recheck automatic achievements for your character.");
}

void AchievementManager::RecheckAutomatic(Client *client)
{
	if (!client) {
		return;
	}

	ProcessLevel(client);
	ProcessZoneVisit(client);
	ProcessItemInventory(client);
	for (int skill_id = 0; skill_id <= EQ::skills::HIGHEST_SKILL; ++skill_id) {
		ProcessSkill(client, skill_id, client->GetRawSkill(static_cast<EQ::skills::SkillType>(skill_id)));
	}

	auto completed = database.QueryDatabase(
		fmt::format(
			"SELECT `achievement_id` FROM `custom_character_achievements` WHERE `character_id` = {}",
			client->CharacterID()
		)
	);

	if (!completed.Success()) {
		return;
	}

	for (auto row = completed.begin(); row != completed.end(); ++row) {
		const uint32 achievement_id = ToUInt(row[0]);
		if (!achievement_id) {
			continue;
		}

		ProcessMatchedObjectives(
			client,
			fmt::format(
				"o.`objective_type` = 'achievement_complete' AND o.`target_id` = {}",
				achievement_id
			),
			1,
			true
		);
	}
}

void AchievementManager::SendStatus(Client *client)
{
	const uint32 character_id = client->CharacterID();

	auto results = database.QueryDatabase(
		fmt::format(
			"SELECT "
			"(SELECT COUNT(*) FROM `custom_achievements` WHERE `enabled` = 1 AND `hidden` = 0), "
			"(SELECT COUNT(*) FROM `custom_character_achievements` ca "
			"JOIN `custom_achievements` a ON a.`id` = ca.`achievement_id` "
			"WHERE ca.`character_id` = {} AND a.`enabled` = 1), "
			"(SELECT COALESCE(SUM(a.`points`), 0) FROM `custom_character_achievements` ca "
			"JOIN `custom_achievements` a ON a.`id` = ca.`achievement_id` "
			"WHERE ca.`character_id` = {} AND a.`enabled` = 1), "
			"(SELECT COUNT(*) FROM `custom_achievement_categories` WHERE `enabled` = 1)",
			character_id,
			character_id
		)
	);

	if (!results.Success() || !results.RowCount()) {
		client->Message(Chat::White, "Achievements are not available. Check the custom achievement tables.");
		return;
	}

	auto row = results.begin();
	client->Message(
		Chat::White,
		fmt::format(
			"Achievements: {} / {} complete, {} points, {} categories.",
			ToUInt(row[1]),
			ToUInt(row[0]),
			ToUInt(row[2]),
			ToUInt(row[3])
		).c_str()
	);
}

void AchievementManager::SendCategories(Client *client)
{
	const auto categories = LoadCategorySummaries(client->CharacterID());
	if (categories.empty()) {
		client->Message(Chat::White, "No achievement categories are available.");
		return;
	}

	client->Message(Chat::White, "Achievement Categories:");
	for (const auto &category : categories) {
		client->Message(
			Chat::White,
			fmt::format(
				"[{}] {} - {} / {} complete, {} points",
				category.id,
				category.name,
				category.completed,
				category.total,
				category.points
			).c_str()
		);
	}
}

void AchievementManager::SendCategory(Client *client, uint32 category_id)
{
	const auto achievements = LoadAchievements(client->CharacterID(), category_id);
	if (achievements.empty()) {
		client->Message(Chat::White, fmt::format("No achievements found in category {}.", category_id).c_str());
		return;
	}

	client->Message(Chat::White, fmt::format("Achievements in category {}:", category_id).c_str());
	for (const auto &achievement : achievements) {
		client->Message(
			Chat::White,
			fmt::format(
				"[{}] [{}] {} ({} pts) - {}",
				achievement.id,
				achievement.completed ? "Complete" : "Open",
				achievement.name,
				achievement.points,
				achievement.description
			).c_str()
		);
	}
}

void AchievementManager::SendDetail(Client *client, uint32 achievement_id)
{
	const uint32 character_id = client->CharacterID();

	auto achievement_results = database.QueryDatabase(
		fmt::format(
			"SELECT a.`name`, a.`description`, a.`points`, COALESCE(ca.`completed_at`, 0) "
			"FROM `custom_achievements` a "
			"LEFT JOIN `custom_character_achievements` ca "
			"ON ca.`achievement_id` = a.`id` AND ca.`character_id` = {} "
			"WHERE a.`id` = {} AND a.`enabled` = 1 LIMIT 1",
			character_id,
			achievement_id
		)
	);

	if (!achievement_results.Success() || !achievement_results.RowCount()) {
		client->Message(Chat::White, fmt::format("Achievement {} was not found.", achievement_id).c_str());
		return;
	}

	auto achievement = achievement_results.begin();
	client->Message(
		Chat::White,
		fmt::format(
			"[{}] {} ({} pts) - {}",
			achievement_id,
			achievement[0] ? achievement[0] : "",
			ToUInt(achievement[2]),
			achievement[1] ? achievement[1] : ""
		).c_str()
	);

	if (ToUInt(achievement[3])) {
		client->Message(Chat::White, fmt::format("Completed at Unix time {}.", ToUInt(achievement[3])).c_str());
	}

	const auto objectives = LoadObjectives(character_id, achievement_id);
	for (const auto &objective : objectives) {
		client->Message(
			Chat::White,
			fmt::format(
				" - [{}] {} ({}/{})",
				objective.completed ? "Done" : "Open",
				ObjectiveDisplayName(objective.type, objective.target_name, objective.required_count),
				objective.current_count,
				objective.required_count
			).c_str()
		);
	}

	const auto rewards = LoadRewardPreview(achievement_id);
	if (!rewards.empty()) {
		client->Message(Chat::White, "Rewards:");
		for (const auto &reward : rewards) {
			client->Message(
				Chat::White,
				fmt::format(
					" - {}{}",
					RewardDisplayName(reward.type, reward.preview_text, reward.data_text, reward.reward_id, reward.amount),
					reward.auto_claim ? " (auto-claim)" : ""
				).c_str()
			);
		}
	}
}

void AchievementManager::SendRewardQueue(Client *client)
{
	if (!client) {
		return;
	}

	const auto rewards = LoadQueuedRewards(client->CharacterID());
	if (rewards.empty()) {
		client->Message(Chat::White, "You have no pending achievement rewards.");
		return;
	}

	client->Message(Chat::White, "Pending achievement rewards:");
	for (const auto &reward : rewards) {
		client->Message(
			Chat::White,
			fmt::format(
				"[{}] Achievement {} - {}",
				reward.queue_id,
				reward.achievement_id,
				RewardDisplayName(reward.type, reward.preview_text, reward.data_text, reward.reward_id, reward.amount)
			).c_str()
		);
	}

	client->Message(Chat::White, "Use #ach claim [reward_id] or #ach claim all.");
}

void AchievementManager::SendNativeWindow(Client *client, uint32 category_id, uint32 achievement_id)
{
	if (!client) {
		return;
	}

	const uint32 character_id = client->CharacterID();
	uint32 total = 0;
	uint32 completed = 0;
	uint32 points = 0;
	uint32 category_count = 0;

	auto status_results = database.QueryDatabase(
		fmt::format(
			"SELECT "
			"(SELECT COUNT(*) FROM `custom_achievements` WHERE `enabled` = 1 AND `hidden` = 0), "
			"(SELECT COUNT(*) FROM `custom_character_achievements` ca "
			"JOIN `custom_achievements` a ON a.`id` = ca.`achievement_id` "
			"WHERE ca.`character_id` = {} AND a.`enabled` = 1), "
			"(SELECT COALESCE(SUM(a.`points`), 0) FROM `custom_character_achievements` ca "
			"JOIN `custom_achievements` a ON a.`id` = ca.`achievement_id` "
			"WHERE ca.`character_id` = {} AND a.`enabled` = 1), "
			"(SELECT COUNT(*) FROM `custom_achievement_categories` WHERE `enabled` = 1)",
			character_id,
			character_id
		)
	);

	if (status_results.Success() && status_results.RowCount()) {
		auto row = status_results.begin();
		total = ToUInt(row[0]);
		completed = ToUInt(row[1]);
		points = ToUInt(row[2]);
		category_count = ToUInt(row[3]);
	}

	if (achievement_id && !category_id) {
		category_id = LoadAchievementCategory(achievement_id);
	}

	auto categories = LoadCategorySummaries(character_id);
	uint32 selected_category_id = 0;
	for (const auto &category : categories) {
		if (category.id == category_id) {
			selected_category_id = category.id;
			break;
		}
	}

	if (!selected_category_id) {
		for (const auto &category : categories) {
			if (category.total > 0) {
				selected_category_id = category.id;
				break;
			}
		}
	}

	if (!selected_category_id && !categories.empty()) {
		selected_category_id = categories.front().id;
	}

	auto achievements = selected_category_id ? LoadAchievements(character_id, selected_category_id) : std::vector<AchievementSummary>();
	uint32 selected_achievement_id = 0;
	if (achievement_id) {
		for (const auto &achievement : achievements) {
			if (achievement.id == achievement_id) {
				selected_achievement_id = achievement.id;
				break;
			}
		}
	}

	if (!selected_achievement_id && !achievements.empty()) {
		selected_achievement_id = achievements.front().id;
	}

	client->Message(Chat::White, "ACH|window|clear");
	client->Message(
		Chat::White,
		fmt::format(
			"ACH|status|completed={}|total={}|points={}|categories={}",
			completed,
			total,
			points,
			category_count
		).c_str()
	);

	for (const auto &category : categories) {
		client->Message(
			Chat::White,
			fmt::format(
				"ACH|category|id={}|parent={}|name={}|completed={}|total={}|points={}",
				category.id,
				category.parent_id,
				ProtocolValue(category.name, 80),
				category.completed,
				category.total,
				category.points
			).c_str()
		);
	}

	client->Message(
		Chat::White,
		fmt::format(
			"ACH|selection|category={}|achievement={}",
			selected_category_id,
			selected_achievement_id
		).c_str()
	);

	for (const auto &achievement : achievements) {
		client->Message(
			Chat::White,
			fmt::format(
				"ACH|achievement|id={}|category={}|completed={}|points={}|name={}|description={}",
				achievement.id,
				achievement.category_id,
				achievement.completed ? 1 : 0,
				achievement.points,
				ProtocolValue(achievement.name, 96),
				ProtocolValue(achievement.description, 160)
			).c_str()
		);
	}

	if (selected_achievement_id) {
		SendNativeDetail(client, selected_achievement_id);
	}
	else {
		client->Message(Chat::White, "ACH|detail|id=0|completed=0|points=0|name=No achievements|description=No achievements are available for this category.");
	}

	client->Message(Chat::White, "ACH|window|show");
}

// AoTv4: live refresh pushed on achievement completion. Sends only the updated status + category
// summaries + a single achievement_state flip -- NO window|clear / window|show. An OPEN window updates
// in place (dll upserts category counts by id, flips the one achievement row if in the current view);
// a CLOSED window is untouched (no window|show -> no create); the player's current selection is kept.
void AchievementManager::PushLiveUpdate(Client *client, uint32 completed_achievement_id)
{
	if (!client) {
		return;
	}

	const uint32 character_id = client->CharacterID();

	uint32 total = 0, completed = 0, points = 0, category_count = 0;
	auto status_results = database.QueryDatabase(
		fmt::format(
			"SELECT "
			"(SELECT COUNT(*) FROM `custom_achievements` WHERE `enabled` = 1 AND `hidden` = 0), "
			"(SELECT COUNT(*) FROM `custom_character_achievements` ca JOIN `custom_achievements` a ON a.`id` = ca.`achievement_id` WHERE ca.`character_id` = {} AND a.`enabled` = 1), "
			"(SELECT COALESCE(SUM(a.`points`), 0) FROM `custom_character_achievements` ca JOIN `custom_achievements` a ON a.`id` = ca.`achievement_id` WHERE ca.`character_id` = {} AND a.`enabled` = 1), "
			"(SELECT COUNT(*) FROM `custom_achievement_categories` WHERE `enabled` = 1)",
			character_id, character_id
		)
	);
	if (status_results.Success() && status_results.RowCount()) {
		auto row = status_results.begin();
		total = ToUInt(row[0]); completed = ToUInt(row[1]); points = ToUInt(row[2]); category_count = ToUInt(row[3]);
	}

	client->Message(
		Chat::White,
		fmt::format("ACH|status|completed={}|total={}|points={}|categories={}", completed, total, points, category_count).c_str()
	);

	// Send the row-flip FIRST, then only the affected leaf category + its parent header.
	// Re-pushing ALL ~40 categories per completion is a chat burst the RoF2 client can
	// drop/reorder, which loses the achievement_state line -> the "sometimes doesn't
	// update" symptom. Completing one achievement only changes its own category's and
	// its parent's counts, so 1 status + 1 state + <=2 category lines is enough and reliable.
	if (completed_achievement_id) {
		client->Message(
			Chat::White,
			fmt::format("ACH|achievement_state|id={}|completed=1", completed_achievement_id).c_str()
		);
	}

	const uint32 affected_cat = completed_achievement_id ? LoadAchievementCategory(completed_achievement_id) : 0;
	if (affected_cat) {
		// rolled-up summaries for just the affected leaf + its parent (mirrors LoadCategorySummaries)
		auto cat_results = database.QueryDatabase(
			fmt::format(
				"SELECT c.`id`, c.`parent_id`, c.`name`, COUNT(a.`id`), "
				"COALESCE(SUM(CASE WHEN ca.`achievement_id` IS NULL THEN 0 ELSE 1 END), 0), "
				"COALESCE(SUM(CASE WHEN ca.`achievement_id` IS NULL THEN 0 ELSE a.`points` END), 0) "
				"FROM `custom_achievement_categories` c "
				"LEFT JOIN `custom_achievements` a ON ("
				"    a.`category_id` = c.`id` "
				"    OR a.`category_id` IN (SELECT sc.`id` FROM `custom_achievement_categories` sc "
				"                           WHERE sc.`parent_id` = c.`id` AND sc.`enabled` = 1) "
				"  ) AND a.`enabled` = 1 AND a.`hidden` = 0 "
				"LEFT JOIN `custom_character_achievements` ca ON ca.`achievement_id` = a.`id` AND ca.`character_id` = {0} "
				"WHERE c.`enabled` = 1 AND (c.`id` = {1} "
				"  OR c.`id` = (SELECT `parent_id` FROM `custom_achievement_categories` WHERE `id` = {1})) "
				"GROUP BY c.`id`, c.`parent_id`, c.`name`",
				character_id, affected_cat
			)
		);
		if (cat_results.Success()) {
			for (auto row = cat_results.begin(); row != cat_results.end(); ++row) {
				client->Message(
					Chat::White,
					fmt::format(
						"ACH|category|id={}|parent={}|name={}|completed={}|total={}|points={}",
						ToUInt(row[0]), ToUInt(row[1]), ProtocolValue(row[2] ? row[2] : "", 80),
						ToUInt(row[4]), ToUInt(row[3]), ToUInt(row[5])
					).c_str()
				);
			}
		}
	}
}

void AchievementManager::SendNativeCategory(Client *client, uint32 category_id, uint32 achievement_id)
{
	if (!client) {
		return;
	}

	const uint32 character_id = client->CharacterID();
	const auto achievements = category_id ? LoadAchievements(character_id, category_id) : std::vector<AchievementSummary>();
	uint32 selected_achievement_id = 0;
	if (achievement_id) {
		for (const auto &achievement : achievements) {
			if (achievement.id == achievement_id) {
				selected_achievement_id = achievement.id;
				break;
			}
		}
	}

	if (!selected_achievement_id && !achievements.empty()) {
		selected_achievement_id = achievements.front().id;
	}

	client->Message(Chat::White, "ACH|achievements|clear");
	client->Message(
		Chat::White,
		fmt::format(
			"ACH|selection|category={}|achievement={}",
			category_id,
			selected_achievement_id
		).c_str()
	);

	for (const auto &achievement : achievements) {
		client->Message(
			Chat::White,
			fmt::format(
				"ACH|achievement|id={}|category={}|completed={}|points={}|name={}|description={}",
				achievement.id,
				achievement.category_id,
				achievement.completed ? 1 : 0,
				achievement.points,
				ProtocolValue(achievement.name, 96),
				ProtocolValue(achievement.description, 160)
			).c_str()
		);
	}

	if (selected_achievement_id) {
		SendNativeDetail(client, selected_achievement_id);
	}
	else {
		client->Message(Chat::White, "ACH|detail|id=0|completed=0|points=0|name=No achievements|description=No achievements are available for this category.");
	}

	client->Message(Chat::White, "ACH|window|show");
}

void AchievementManager::SendNativeDetail(Client *client, uint32 achievement_id)
{
	if (!client) {
		return;
	}

	const uint32 character_id = client->CharacterID();
	auto achievement_results = database.QueryDatabase(
		fmt::format(
			"SELECT a.`name`, a.`description`, a.`points`, COALESCE(ca.`completed_at`, 0) "
			"FROM `custom_achievements` a "
			"LEFT JOIN `custom_character_achievements` ca "
			"ON ca.`achievement_id` = a.`id` AND ca.`character_id` = {} "
			"WHERE a.`id` = {} AND a.`enabled` = 1 LIMIT 1",
			character_id,
			achievement_id
		)
	);

	if (!achievement_results.Success() || !achievement_results.RowCount()) {
		client->Message(
			Chat::White,
			fmt::format(
				"ACH|detail|id={}|completed=0|points=0|name=Achievement unavailable|description=Achievement {} was not found.",
				achievement_id,
				achievement_id
			).c_str()
		);
		return;
	}

	auto achievement = achievement_results.begin();
	const uint32 completed_at = ToUInt(achievement[3]);
	client->Message(Chat::White, "ACH|rewards|clear");
	client->Message(
		Chat::White,
		fmt::format(
			"ACH|detail|id={}|completed={}|completed_at={}|points={}|name={}|description={}",
			achievement_id,
			completed_at ? 1 : 0,
			completed_at,
			ToUInt(achievement[2]),
			ProtocolValue(achievement[0] ? achievement[0] : "", 96),
			ProtocolValue(achievement[1] ? achievement[1] : "", 260)
		).c_str()
	);

	const auto objectives = LoadObjectives(character_id, achievement_id);
	for (const auto &objective : objectives) {
		client->Message(
			Chat::White,
			fmt::format(
				"ACH|objective|id={}|completed={}|current={}|required={}|name={}",
				objective.id,
				objective.completed ? 1 : 0,
				objective.current_count,
				objective.required_count,
				ProtocolValue(ObjectiveDisplayName(objective.type, objective.target_name, objective.required_count), 120)
			).c_str()
		);
	}

	const auto rewards = LoadRewardPreview(achievement_id);
	for (const auto &reward : rewards) {
		client->Message(
			Chat::White,
			fmt::format(
				"ACH|reward|definition={}|type={}|id={}|amount={}|auto={}|tier={}|name={}",
				reward.definition_id,
				ProtocolValue(reward.type, 32),
				reward.reward_id,
				reward.amount,
				reward.auto_claim ? 1 : 0,
				ProtocolValue(reward.tier, 32),
				ProtocolValue(RewardDisplayName(reward.type, reward.preview_text, reward.data_text, reward.reward_id, reward.amount), 160)
			).c_str()
		);
	}
}

std::vector<AchievementManager::CategorySummary> AchievementManager::LoadCategorySummaries(uint32 character_id)
{
	std::vector<CategorySummary> categories;

	auto results = database.QueryDatabase(
		fmt::format(
			"SELECT c.`id`, c.`parent_id`, c.`name`, "
			"COUNT(a.`id`) AS total_count, "
			"COALESCE(SUM(CASE WHEN ca.`achievement_id` IS NULL THEN 0 ELSE 1 END), 0) AS completed_count, "
			"COALESCE(SUM(CASE WHEN ca.`achievement_id` IS NULL THEN 0 ELSE a.`points` END), 0) AS points "
			"FROM `custom_achievement_categories` c "
			// Roll child-category achievements up onto their parent so header categories
			// (Exploration/Hunter/Slayer/Progression/Tradeskill) show the sum of their leaves
			// instead of 0/0. Leaves have no children (subquery empty) so they stay direct-only.
			"LEFT JOIN `custom_achievements` a ON ("
			"    a.`category_id` = c.`id` "
			"    OR a.`category_id` IN (SELECT sc.`id` FROM `custom_achievement_categories` sc "
			"                           WHERE sc.`parent_id` = c.`id` AND sc.`enabled` = 1) "
			"  ) AND a.`enabled` = 1 AND a.`hidden` = 0 "
			"LEFT JOIN `custom_character_achievements` ca ON ca.`achievement_id` = a.`id` AND ca.`character_id` = {} "
			"WHERE c.`enabled` = 1 "
			"GROUP BY c.`id`, c.`parent_id`, c.`name`, c.`sort_order` "
			"ORDER BY c.`sort_order`, c.`id`",
			character_id
		)
	);

	if (!results.Success()) {
		return categories;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		CategorySummary summary;
		summary.id = ToUInt(row[0]);
		summary.parent_id = ToUInt(row[1]);
		summary.name = row[2] ? row[2] : "";
		summary.total = ToUInt(row[3]);
		summary.completed = ToUInt(row[4]);
		summary.points = ToUInt(row[5]);
		categories.push_back(summary);
	}

	return categories;
}

std::vector<AchievementManager::AchievementSummary> AchievementManager::LoadAchievements(uint32 character_id, uint32 category_id)
{
	std::vector<AchievementSummary> achievements;

	auto results = database.QueryDatabase(
		fmt::format(
			"SELECT a.`id`, a.`category_id`, a.`name`, a.`description`, a.`points`, COALESCE(ca.`completed_at`, 0) "
			"FROM `custom_achievements` a "
			"LEFT JOIN `custom_character_achievements` ca "
			"ON ca.`achievement_id` = a.`id` AND ca.`character_id` = {0} "
			// Selecting a parent header (Hunter/Exploration/Slayer/...) shows the union of its child
			// categories' achievements, so a parent is never an empty row. Leaves have no children
			// (subquery empty) so they stay direct-only. Grouped by category so children list together.
			"WHERE a.`enabled` = 1 AND a.`hidden` = 0 AND ("
			"    a.`category_id` = {1} "
			"    OR a.`category_id` IN (SELECT sc.`id` FROM `custom_achievement_categories` sc "
			"                           WHERE sc.`parent_id` = {1} AND sc.`enabled` = 1) "
			"  ) "
			"ORDER BY a.`category_id`, a.`sort_order`, a.`id`",
			character_id,
			category_id
		)
	);

	if (!results.Success()) {
		return achievements;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		AchievementSummary summary;
		summary.id = ToUInt(row[0]);
		summary.category_id = ToUInt(row[1]);
		summary.name = row[2] ? row[2] : "";
		summary.description = row[3] ? row[3] : "";
		summary.points = ToUInt(row[4]);
		summary.completed_at = ToUInt(row[5]);
		summary.completed = summary.completed_at != 0;
		achievements.push_back(summary);
	}

	return achievements;
}

std::vector<AchievementManager::ObjectiveSummary> AchievementManager::LoadObjectives(uint32 character_id, uint32 achievement_id)
{
	std::vector<ObjectiveSummary> objectives;

	auto results = database.QueryDatabase(
		fmt::format(
			"SELECT o.`id`, o.`objective_type`, o.`target_name`, o.`required_count`, "
			"COALESCE(p.`count`, 0), COALESCE(p.`completed_at`, 0) "
			"FROM `custom_achievement_objectives` o "
			"LEFT JOIN `custom_character_achievement_progress` p "
			"ON p.`objective_id` = o.`id` AND p.`character_id` = {} "
			"WHERE o.`achievement_id` = {} "
			"ORDER BY o.`objective_index`, o.`id`",
			character_id,
			achievement_id
		)
	);

	if (!results.Success()) {
		return objectives;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		ObjectiveSummary summary;
		summary.id = ToUInt(row[0]);
		summary.type = row[1] ? row[1] : "";
		summary.target_name = row[2] ? row[2] : "";
		summary.required_count = ToUInt(row[3]);
		summary.current_count = ToUInt(row[4]);
		summary.completed = ToUInt(row[5]) != 0;
		objectives.push_back(summary);
	}

	return objectives;
}

std::vector<AchievementManager::RewardSummary> AchievementManager::LoadRewardPreview(uint32 achievement_id)
{
	std::vector<RewardSummary> rewards;
	if (!achievement_id) {
		return rewards;
	}

	auto reward_results = database.QueryDatabase(
		fmt::format(
			"SELECT `id`, `reward_type`, `reward_id`, `amount`, `auto_claim`, `tier`, `preview_text`, `data_text` "
			"FROM `custom_achievement_rewards` "
			"WHERE `achievement_id` = {} AND `enabled` = 1 "
			"ORDER BY `sort_order`, `id`",
			achievement_id
		)
	);

	if (reward_results.Success()) {
		for (auto row = reward_results.begin(); row != reward_results.end(); ++row) {
			RewardSummary reward;
			reward.definition_id = ToUInt64(row[0]);
			reward.achievement_id = achievement_id;
			reward.type = row[1] ? row[1] : "";
			reward.reward_id = ToUInt(row[2]);
			reward.amount = ToUInt(row[3]);
			reward.auto_claim = ToUInt(row[4]) != 0;
			reward.tier = row[5] ? row[5] : "";
			reward.preview_text = row[6] ? row[6] : "";
			reward.data_text = row[7] ? row[7] : "";
			rewards.push_back(reward);
		}
	}

	auto legacy_results = database.QueryDatabase(
		fmt::format(
			"SELECT `reward_title_set`, `reward_item_id`, `reward_currency_id`, `reward_currency_amount` "
			"FROM `custom_achievements` WHERE `id` = {} AND `enabled` = 1 LIMIT 1",
			achievement_id
		)
	);

	if (legacy_results.Success() && legacy_results.RowCount()) {
		auto row = legacy_results.begin();
		const uint32 title_set = ToUInt(row[0]);
		const uint32 item_id = ToUInt(row[1]);
		const uint32 currency_id = ToUInt(row[2]);
		const uint32 currency_amount = ToUInt(row[3]);

		if (title_set) {
			RewardSummary reward;
			reward.achievement_id = achievement_id;
			reward.type = "title_set";
			reward.reward_id = title_set;
			reward.amount = 1;
			reward.auto_claim = true;
			reward.preview_text = fmt::format("Title set {}", title_set);
			rewards.push_back(reward);
		}

		if (item_id) {
			RewardSummary reward;
			reward.achievement_id = achievement_id;
			reward.type = "item";
			reward.reward_id = item_id;
			reward.amount = 1;
			reward.preview_text = fmt::format("Item {}", item_id);
			rewards.push_back(reward);
		}

		if (currency_id && currency_amount) {
			RewardSummary reward;
			reward.achievement_id = achievement_id;
			reward.type = "currency";
			reward.reward_id = currency_id;
			reward.amount = currency_amount;
			reward.auto_claim = true;
			reward.preview_text = fmt::format("{} currency {}", currency_amount, currency_id);
			rewards.push_back(reward);
		}
	}

	return rewards;
}

std::vector<AchievementManager::RewardSummary> AchievementManager::LoadQueuedRewards(
	uint32 character_id,
	uint64 queue_id,
	bool auto_claim_only
)
{
	std::vector<RewardSummary> rewards;
	if (!character_id) {
		return rewards;
	}

	std::string filter = fmt::format(
		"`character_id` = {} AND `status` = {}",
		character_id,
		RewardStatusPending
	);

	if (queue_id) {
		filter += fmt::format(" AND `id` = {}", queue_id);
	}

	if (auto_claim_only) {
		filter += " AND `auto_claim` = 1";
	}

	auto results = database.QueryDatabase(
		fmt::format(
			"SELECT `id`, `achievement_id`, `reward_definition_id`, `reward_type`, `reward_id`, "
			"`amount`, `auto_claim`, `tier`, `preview_text`, `data_text` "
			"FROM `custom_character_achievement_rewards` "
			"WHERE {} "
			"ORDER BY `created_at`, `id`",
			filter
		)
	);

	if (!results.Success()) {
		return rewards;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		RewardSummary reward;
		reward.queue_id = ToUInt64(row[0]);
		reward.achievement_id = ToUInt(row[1]);
		reward.definition_id = ToUInt64(row[2]);
		reward.type = row[3] ? row[3] : "";
		reward.reward_id = ToUInt(row[4]);
		reward.amount = ToUInt(row[5]);
		reward.auto_claim = ToUInt(row[6]) != 0;
		reward.tier = row[7] ? row[7] : "";
		reward.preview_text = row[8] ? row[8] : "";
		reward.data_text = row[9] ? row[9] : "";
		rewards.push_back(reward);
	}

	return rewards;
}

uint32 AchievementManager::LoadAchievementCategory(uint32 achievement_id)
{
	if (!achievement_id) {
		return 0;
	}

	auto results = database.QueryDatabase(
		fmt::format(
			"SELECT `category_id` FROM `custom_achievements` "
			"WHERE `id` = {} AND `enabled` = 1 LIMIT 1",
			achievement_id
		)
	);

	if (!results.Success() || !results.RowCount()) {
		return 0;
	}

	auto row = results.begin();
	return ToUInt(row[0]);
}

void AchievementManager::ProcessMatchedObjectives(Client *client, const std::string &match_sql, uint32 progress, bool absolute_progress)
{
	if (!client || match_sql.empty()) {
		return;
	}

	const uint32 character_id = client->CharacterID();
	// AoTv4: honor the objective's class_mask so per-class achievements only advance for that class
	// (0 = any class). Uses the standard class bit (GetPlayerClassBit, common/classes.h).
	const uint32 class_bit = GetPlayerClassBit(client->GetClass());
	auto results = database.QueryDatabase(
		fmt::format(
			"SELECT o.`id`, o.`achievement_id`, o.`required_count`, "
			"COALESCE(p.`count`, 0), COALESCE(p.`completed_at`, 0) "
			"FROM `custom_achievement_objectives` o "
			"JOIN `custom_achievements` a ON a.`id` = o.`achievement_id` AND a.`enabled` = 1 "
			"LEFT JOIN `custom_character_achievement_progress` p "
			"ON p.`objective_id` = o.`id` AND p.`character_id` = {} "
			"WHERE ({}) AND (o.`class_mask` = 0 OR (o.`class_mask` & {}) != 0)",
			character_id,
			match_sql,
			class_bit
		)
	);

	if (!results.Success()) {
		return;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		const uint32 objective_id = ToUInt(row[0]);
		const uint32 achievement_id = ToUInt(row[1]);
		const uint32 required_count = ToUInt(row[2]);
		const uint32 current_count = ToUInt(row[3]);
		const bool already_completed = ToUInt(row[4]) != 0;
		const uint32 new_count = absolute_progress ? progress : current_count + progress;

		if (!already_completed || new_count > current_count) {
			UpdateObjectiveProgress(character_id, objective_id, new_count, new_count >= required_count);
		}

		TryCompleteAchievement(client, achievement_id);
	}
}

void AchievementManager::UpdateObjectiveProgress(uint32 character_id, uint32 objective_id, uint32 count, bool completed)
{
	database.QueryDatabase(
		fmt::format(
			"INSERT INTO `custom_character_achievement_progress` "
			"(`character_id`, `objective_id`, `count`, `completed_at`, `updated_at`) "
			"VALUES ({}, {}, {}, {}, UNIX_TIMESTAMP()) "
			"ON DUPLICATE KEY UPDATE "
			"`count` = GREATEST(`count`, VALUES(`count`)), "
			"`completed_at` = IF(`completed_at` = 0 AND VALUES(`completed_at`) > 0, VALUES(`completed_at`), `completed_at`), "
			"`updated_at` = VALUES(`updated_at`)",
			character_id,
			objective_id,
			count,
			completed ? "UNIX_TIMESTAMP()" : "0"
		)
	);
}

void AchievementManager::TryCompleteAchievement(Client *client, uint32 achievement_id)
{
	const uint32 character_id = client->CharacterID();
	if (HasCompletedAchievement(character_id, achievement_id)) {
		return;
	}

	auto results = database.QueryDatabase(
		fmt::format(
			"SELECT "
			"COUNT(*) AS required_count, "
			"COALESCE(SUM(CASE WHEN p.`completed_at` > 0 THEN 1 ELSE 0 END), 0) AS completed_count "
			"FROM `custom_achievement_objectives` o "
			"LEFT JOIN `custom_character_achievement_progress` p "
			"ON p.`objective_id` = o.`id` AND p.`character_id` = {} "
			"WHERE o.`achievement_id` = {} AND o.`optional` = 0",
			character_id,
			achievement_id
		)
	);

	if (!results.Success() || !results.RowCount()) {
		return;
	}

	auto row = results.begin();
	const uint32 required_count = ToUInt(row[0]);
	const uint32 completed_count = ToUInt(row[1]);
	if (!required_count || completed_count < required_count) {
		return;
	}

	database.QueryDatabase(
		fmt::format(
			"INSERT INTO `custom_character_achievements` "
			"(`character_id`, `achievement_id`, `completed_at`, `awarded_at`, `completion_count`, `announced`) "
			"VALUES ({}, {}, UNIX_TIMESTAMP(), UNIX_TIMESTAMP(), 1, 0)",
			character_id,
			achievement_id
		)
	);

	auto achievement = database.QueryDatabase(
		fmt::format(
			"SELECT `name`, `description`, `points` FROM `custom_achievements` WHERE `id` = {} LIMIT 1",
			achievement_id
		)
	);

	if (achievement.Success() && achievement.RowCount()) {
		auto achievement_row = achievement.begin();
		client->Message(
			Chat::Yellow,
			fmt::format(
				"Achievement complete: {} ({} pts) - {}",
				achievement_row[0] ? achievement_row[0] : "",
				ToUInt(achievement_row[2]),
				achievement_row[1] ? achievement_row[1] : ""
			).c_str()
		);
	}

	QueueAchievementRewards(client, achievement_id);
	ClaimPendingRewards(client, 0, true);

	ProcessMatchedObjectives(
		client,
		fmt::format(
			"o.`objective_type` = 'achievement_complete' AND o.`target_id` = {}",
			achievement_id
		),
		1,
		true
	);

	// AoTv4: live-refresh an already-open achievement window so completion shows immediately.
	PushLiveUpdate(client, achievement_id);
}

void AchievementManager::QueueAchievementRewards(Client *client, uint32 achievement_id)
{
	if (!client || !achievement_id) {
		return;
	}

	const uint32 character_id = client->CharacterID();
	database.QueryDatabase(
		fmt::format(
			"INSERT INTO `custom_character_achievement_rewards` "
			"(`character_id`, `achievement_id`, `reward_definition_id`, `reward_type`, `reward_id`, "
			"`amount`, `auto_claim`, `tier`, `preview_text`, `data_text`, `status`, `completion_count`, `created_at`) "
			"SELECT {}, r.`achievement_id`, r.`id`, r.`reward_type`, r.`reward_id`, "
			"GREATEST(r.`amount`, 1), r.`auto_claim`, r.`tier`, r.`preview_text`, r.`data_text`, {}, "
			"ca.`completion_count`, UNIX_TIMESTAMP() "
			"FROM `custom_achievement_rewards` r "
			"JOIN `custom_character_achievements` ca "
			"ON ca.`achievement_id` = r.`achievement_id` AND ca.`character_id` = {} "
			"WHERE r.`achievement_id` = {} AND r.`enabled` = 1 "
			"AND (r.`chance` >= 10000 OR FLOOR(RAND() * 10000) < r.`chance`) "
			"ON DUPLICATE KEY UPDATE `reward_definition_id` = `reward_definition_id`",
			character_id,
			RewardStatusPending,
			character_id,
			achievement_id
		)
	);

	QueueLegacyAchievementRewards(client, achievement_id);
}

void AchievementManager::QueueLegacyAchievementRewards(Client *client, uint32 achievement_id)
{
	if (!client || !achievement_id) {
		return;
	}

	auto legacy_results = database.QueryDatabase(
		fmt::format(
			"SELECT a.`reward_title_set`, a.`reward_item_id`, a.`reward_currency_id`, a.`reward_currency_amount`, "
			"ca.`completion_count` "
			"FROM `custom_achievements` a "
			"JOIN `custom_character_achievements` ca "
			"ON ca.`achievement_id` = a.`id` AND ca.`character_id` = {} "
			"WHERE a.`id` = {} AND a.`enabled` = 1 LIMIT 1",
			client->CharacterID(),
			achievement_id
		)
	);

	if (!legacy_results.Success() || !legacy_results.RowCount()) {
		return;
	}

	auto row = legacy_results.begin();
	const uint32 title_set = ToUInt(row[0]);
	const uint32 item_id = ToUInt(row[1]);
	const uint32 currency_id = ToUInt(row[2]);
	const uint32 currency_amount = ToUInt(row[3]);
	const uint32 completion_count = ToUInt(row[4]) ? ToUInt(row[4]) : 1;

	auto queue_reward = [this, client, achievement_id, completion_count](
		const std::string &type,
		uint32 reward_id,
		uint32 amount,
		bool auto_claim,
		const std::string &preview_text
	) {
		database.QueryDatabase(
			fmt::format(
				"INSERT INTO `custom_character_achievement_rewards` "
				"(`character_id`, `achievement_id`, `reward_definition_id`, `reward_type`, `reward_id`, "
				"`amount`, `auto_claim`, `tier`, `preview_text`, `data_text`, `status`, `completion_count`, `created_at`) "
				"VALUES ({}, {}, 0, {}, {}, {}, {}, 'legacy', {}, '', {}, {}, UNIX_TIMESTAMP()) "
				"ON DUPLICATE KEY UPDATE `reward_definition_id` = `reward_definition_id`",
				client->CharacterID(),
				achievement_id,
				SqlString(type),
				reward_id,
				amount ? amount : 1,
				auto_claim ? 1 : 0,
				SqlString(preview_text),
				RewardStatusPending,
				completion_count
			)
		);
	};

	if (title_set) {
		queue_reward("title_set", title_set, 1, true, fmt::format("Title set {}", title_set));
	}

	if (item_id) {
		queue_reward("item", item_id, 1, false, fmt::format("Item {}", item_id));
	}

	if (currency_id && currency_amount) {
		queue_reward(
			"currency",
			currency_id,
			currency_amount,
			true,
			fmt::format("{} currency {}", currency_amount, currency_id)
		);
	}
}

void AchievementManager::ClaimPendingRewards(Client *client, uint64 queue_id, bool auto_claim_only)
{
	if (!client) {
		return;
	}

	const auto rewards = LoadQueuedRewards(client->CharacterID(), queue_id, auto_claim_only);
	if (rewards.empty()) {
		if (!auto_claim_only) {
			client->Message(Chat::White, "No pending achievement rewards were found.");
		}

		return;
	}

	uint32 claimed = 0;
	uint32 failed = 0;
	for (const auto &reward : rewards) {
		std::string result;
		const bool success = AwardQueuedReward(client, reward, result);
		MarkQueuedReward(reward.queue_id, success ? RewardStatusClaimed : RewardStatusFailed, result);

		if (success) {
			++claimed;
			client->Message(
				Chat::Yellow,
				fmt::format(
					"Achievement reward claimed: {}",
					RewardDisplayName(reward.type, reward.preview_text, reward.data_text, reward.reward_id, reward.amount)
				).c_str()
			);
		}
		else {
			++failed;
			client->Message(
				Chat::Red,
				fmt::format(
					"Achievement reward failed: {} ({})",
					RewardDisplayName(reward.type, reward.preview_text, reward.data_text, reward.reward_id, reward.amount),
					result
				).c_str()
			);
		}
	}

	if (!auto_claim_only) {
		client->Message(
			Chat::White,
			fmt::format("Achievement reward claim complete: {} claimed, {} failed.", claimed, failed).c_str()
		);
	}
}

bool AchievementManager::AwardQueuedReward(Client *client, const RewardSummary &reward, std::string &result)
{
	if (!client) {
		result = "No client.";
		return false;
	}

	const uint32 amount = reward.amount ? reward.amount : 1;

	if (Strings::EqualFold(reward.type, "title_text")) {
		const std::string title = !reward.data_text.empty() ? reward.data_text : reward.preview_text;
		if (title.empty()) {
			result = "Missing title text.";
			return false;
		}

		title_manager.CreateNewPlayerTitle(client, title);
		result = fmt::format("Granted title {}", title);
		return true;
	}

	if (Strings::EqualFold(reward.type, "title_suffix")) {
		const std::string suffix = !reward.data_text.empty() ? reward.data_text : reward.preview_text;
		if (suffix.empty()) {
			result = "Missing suffix text.";
			return false;
		}

		title_manager.CreateNewPlayerSuffix(client, suffix);
		result = fmt::format("Granted suffix {}", suffix);
		return true;
	}

	if (Strings::EqualFold(reward.type, "title_set")) {
		if (!reward.reward_id) {
			result = "Missing title set.";
			return false;
		}

		client->EnableTitle(reward.reward_id);
		result = fmt::format("Granted title set {}", reward.reward_id);
		return true;
	}

	if (Strings::EqualFold(reward.type, "item")) {
		if (!reward.reward_id) {
			result = "Missing item id.";
			return false;
		}

		const int16 charges = amount > 1 ? static_cast<int16>(std::min<uint32>(amount, 32767)) : static_cast<int16>(-1);
		const bool success = client->SummonItem(reward.reward_id, charges);
		result = success ? fmt::format("Granted item {}", reward.reward_id) : "Item could not be summoned.";
		return success;
	}

	if (Strings::EqualFold(reward.type, "currency")) {
		if (!reward.reward_id) {
			result = "Missing currency id.";
			return false;
		}

		const int new_value = client->AddAlternateCurrencyValue(reward.reward_id, static_cast<int>(amount), true);
		if (new_value <= 0) {
			result = "Currency id is unavailable.";
			return false;
		}

		result = fmt::format("Granted {} currency {}", amount, reward.reward_id);
		return true;
	}

	if (Strings::EqualFold(reward.type, "coin")) {
		client->AddMoneyToPP(static_cast<uint64>(amount), true);
		result = fmt::format("Granted {} copper.", amount);
		return true;
	}

	// AoTv4: grant one REGION CHOICE -- a credit the player spends on whichever region they want.
	//
	// ⚠️⚠️ IT DOES NOT UNLOCK A REGION ITSELF, and must not. The whole point of the chain is that each
	// capped death opens ANOTHER region OF THE PLAYER'S CHOOSING; hard-wiring a region id here would
	// make the order fixed and the five achievements interchangeable with a single counter.
	// The credit lands in the `region_credits_<charid>` bucket and Lua spends it (aotv4_regions.lua).
	// ⚠️ reward_id is IGNORED for this type. Deliberate -- there is no region to name yet.
	if (Strings::EqualFold(reward.type, "region_choice")) {
		const std::string key = fmt::format("region_credits_{}", client->CharacterID());
		const int have = Strings::ToInt(DataBucket::GetData(&database, key));
		DataBucket::SetData(&database, key, std::to_string(have + static_cast<int>(amount)));
		result = fmt::format("Region unlock available ({} unspent).", have + static_cast<int>(amount));
		return true;
	}

	// AoTv4: scribe a spell into the player's spellbook (used to grant class auras).
	// reward.reward_id = the spell id (the aura's SPA-351 "cast" spell). Idempotent.
	if (Strings::EqualFold(reward.type, "scribe_spell")) {
		if (!reward.reward_id) {
			result = "Missing spell id.";
			return false;
		}

		const uint16 spell_id = static_cast<uint16>(reward.reward_id);
		if (!IsValidSpell(spell_id)) {
			result = "Invalid spell id.";
			return false;
		}

		if (client->HasSpellScribed(spell_id)) {
			result = fmt::format("Spell {} already scribed.", spell_id);
			return true;
		}

		const int slot = client->GetNextAvailableSpellBookSlot();
		if (slot < 0) {
			result = "No free spellbook slot.";
			return false;
		}

		client->ScribeSpell(spell_id, slot);
		const std::string aura_name = !reward.preview_text.empty() ? reward.preview_text : fmt::format("spell {}", spell_id);
		client->Message(Chat::Yellow, "----------------------------------------------------------");
		client->Message(Chat::Yellow, fmt::format("  You have earned an AURA:  {} !", aura_name).c_str());
		client->Message(Chat::Yellow, "  Memorize it from your spellbook and cast it to buff your group.");
		client->Message(Chat::Yellow, "----------------------------------------------------------");
		result = fmt::format("Scribed aura {} ({})", aura_name, spell_id);
		return true;
	}

	if (Strings::EqualFold(reward.type, "live_item_request")) {
		auto request = database.QueryDatabase(
			fmt::format(
				"INSERT INTO `custom_achievement_live_item_requests` "
				"(`character_id`, `achievement_id`, `reward_queue_id`, `level_band`, `tier`, `item_slot`, `theme`, `status`, `created_at`) "
				"VALUES ({}, {}, {}, {}, {}, {}, {}, 'pending', UNIX_TIMESTAMP()) "
				"ON DUPLICATE KEY UPDATE `id` = LAST_INSERT_ID(`id`)",
				client->CharacterID(),
				reward.achievement_id,
				reward.queue_id,
				amount,
				SqlString(reward.tier),
				reward.reward_id,
				SqlString(reward.data_text)
			)
		);

		if (!request.Success()) {
			result = "Live item request could not be queued.";
			return false;
		}

		result = "Queued a leveled item request.";
		return true;
	}

	result = fmt::format("Unknown reward type {}", reward.type);
	return false;
}

void AchievementManager::MarkQueuedReward(uint64 queue_id, uint32 status, const std::string &result)
{
	if (!queue_id) {
		return;
	}

	database.QueryDatabase(
		fmt::format(
			"UPDATE `custom_character_achievement_rewards` "
			"SET `status` = {}, `claimed_at` = IF({} = {}, UNIX_TIMESTAMP(), `claimed_at`), `result_text` = {} "
			"WHERE `id` = {}",
			status,
			status,
			RewardStatusClaimed,
			SqlString(result),
			queue_id
		)
	);
}

bool AchievementManager::HasCompletedAchievement(uint32 character_id, uint32 achievement_id)
{
	auto results = database.QueryDatabase(
		fmt::format(
			"SELECT 1 FROM `custom_character_achievements` "
			"WHERE `character_id` = {} AND `achievement_id` = {} LIMIT 1",
			character_id,
			achievement_id
		)
	);

	return results.Success() && results.RowCount() > 0;
}
