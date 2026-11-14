#pragma once

#include "zone/reward_selection.h"

#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Database;

struct RewardSelectionCatalog {
	std::unordered_map<uint64_t, RewardSelectionSet> selectable;
	std::unordered_map<uint64_t, std::vector<RewardSelectionReward>> automatic;
};

bool LoadRewardSelectionCatalog(
	Database &database,
	RewardSelectionSource source,
	const std::unordered_set<uint64_t> &active_source_ids,
	RewardSelectionCatalog &catalog
);
