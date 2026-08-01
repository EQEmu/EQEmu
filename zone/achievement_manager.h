/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.
*/
#pragma once

#include "common/types.h"

#include <string>
#include <vector>

class Client;
class NPC;
class Seperator;

class AchievementManager {
public:
	void HandleCommand(Client *client, const Seperator *sep);
	void ProcessLevel(Client *client);
	void ProcessZoneVisit(Client *client);
	void ProcessTaskComplete(Client *client, uint32 task_id);
	void ProcessSkill(Client *client, uint32 skill_id, uint32 value);
	void ProcessKill(Client *client, NPC *npc);
	void ProcessItemReceive(Client *client, uint32 item_id);   // item_receive objective (e.g. class epics)
	void ProcessDeathAtMaxLevel(Client *client);               // death_at_max objective (region unlocks)

private:
	struct CategorySummary {
		uint32 id = 0;
		uint32 parent_id = 0;
		std::string name;
		uint32 total = 0;
		uint32 completed = 0;
		uint32 points = 0;
	};

	struct AchievementSummary {
		uint32 id = 0;
		uint32 category_id = 0;
		std::string name;
		std::string description;
		uint32 points = 0;
		bool completed = false;
		uint32 completed_at = 0;
	};

	struct ObjectiveSummary {
		uint32 id = 0;
		std::string type;
		std::string target_name;
		uint32 required_count = 0;
		uint32 current_count = 0;
		bool completed = false;
	};

	struct RewardSummary {
		uint64 queue_id = 0;
		uint64 definition_id = 0;
		uint32 achievement_id = 0;
		uint32 reward_id = 0;
		uint32 amount = 0;
		bool auto_claim = false;
		std::string type;
		std::string tier;
		std::string preview_text;
		std::string data_text;
	};

	void SendHelp(Client *client);
	void SendStatus(Client *client);
	void SendCategories(Client *client);
	void SendCategory(Client *client, uint32 category_id);
	void SendDetail(Client *client, uint32 achievement_id);
	void SendRewardQueue(Client *client);
	void SendNativeWindow(Client *client, uint32 category_id = 0, uint32 achievement_id = 0);
	void PushLiveUpdate(Client *client, uint32 completed_achievement_id);   // live refresh on completion (no clear/show)
	void SendNativeCategory(Client *client, uint32 category_id, uint32 achievement_id = 0);
	void SendNativeDetail(Client *client, uint32 achievement_id);
	void RecheckAutomatic(Client *client);

	std::vector<CategorySummary> LoadCategorySummaries(uint32 character_id);
	std::vector<AchievementSummary> LoadAchievements(uint32 character_id, uint32 category_id);
	std::vector<ObjectiveSummary> LoadObjectives(uint32 character_id, uint32 achievement_id);
	std::vector<RewardSummary> LoadRewardPreview(uint32 achievement_id);
	std::vector<RewardSummary> LoadQueuedRewards(uint32 character_id, uint64 queue_id = 0, bool auto_claim_only = false);
	uint32 LoadAchievementCategory(uint32 achievement_id);

	void ProcessItemInventory(Client *client);   // backstop: credit item_receive for items already owned
	void ProcessMatchedObjectives(Client *client, const std::string &match_sql, uint32 progress, bool absolute_progress);
	void UpdateObjectiveProgress(uint32 character_id, uint32 objective_id, uint32 count, bool completed);
	void TryCompleteAchievement(Client *client, uint32 achievement_id);
	void QueueAchievementRewards(Client *client, uint32 achievement_id);
	void QueueLegacyAchievementRewards(Client *client, uint32 achievement_id);
	void ClaimPendingRewards(Client *client, uint64 queue_id = 0, bool auto_claim_only = false);
	bool AwardQueuedReward(Client *client, const RewardSummary &reward, std::string &result);
	void MarkQueuedReward(uint64 queue_id, uint32 status, const std::string &result);
	bool HasCompletedAchievement(uint32 character_id, uint32 achievement_id);
};

extern AchievementManager achievement_manager;
