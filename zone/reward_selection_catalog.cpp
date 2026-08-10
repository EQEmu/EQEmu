#include "zone/reward_selection_catalog.h"

#include "common/database.h"
#include "common/eqemu_logsys.h"
#include "common/repositories/reward_option_entries_repository.h"
#include "common/repositories/reward_options_repository.h"
#include "common/repositories/reward_sets_repository.h"
#include "common/repositories/reward_source_entries_repository.h"
#include "common/repositories/reward_sources_repository.h"
#include "common/repositories/rewards_repository.h"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <utility>

struct StagedRewardSet {
	RewardSelectionSet                    set;
	std::unordered_map<uint32_t, size_t> option_indices;
	std::unordered_map<uint32_t, uint32_t> reward_sequences;
	std::unordered_set<uint32_t>          disabled_option_ids;
	std::unordered_set<uint32_t>          reward_ids;
};

static bool ValidReward(const RewardsRepository::Rewards &row)
{
	if (
		!row.reward_id ||
		!row.amount ||
		row.reward_type > static_cast<uint8_t>(RewardSelectionRewardType::Title)
	) {
		return false;
	}

	const auto type = static_cast<RewardSelectionRewardType>(row.reward_type);
	const auto requires_data_id =
		type == RewardSelectionRewardType::Item ||
		type == RewardSelectionRewardType::AlternateCurrency ||
		type == RewardSelectionRewardType::Title;
	const auto invalid_experience_mode =
		type == RewardSelectionRewardType::Experience &&
		row.reward_data_id > static_cast<uint32_t>(
			RewardSelectionExperienceMode::NormalOnly
		);
	return (!requires_data_id || row.reward_data_id) && !invalid_experience_mode;
}

static RewardSelectionReward ToReward(
	const RewardsRepository::Rewards &row
)
{
	return {
		.entry_id = row.reward_id,
		.type = static_cast<RewardSelectionRewardType>(row.reward_type),
		.data_id = row.reward_data_id,
		.amount = row.amount,
		.description = row.description
	};
}

bool LoadRewardSelectionCatalog(
	Database &database,
	RewardSelectionSource source,
	const std::unordered_set<uint64_t> &active_source_ids,
	RewardSelectionCatalog &catalog
)
{
	catalog = {};
	if (
		source == RewardSelectionSource::Unknown ||
		source > RewardSelectionSource::General
	) {
		return false;
	}

	for (const auto *table : {
		"reward_sets",
		"reward_options",
		"rewards",
		"reward_option_entries",
		"reward_sources",
		"reward_source_entries"
	}) {
		if (!database.DoesTableExist(table)) {
			LogError("Reward catalog table [{}] is missing", table);
			return false;
		}
	}

	std::vector<RewardSetsRepository::RewardSets> reward_set_rows;
	std::vector<RewardOptionsRepository::RewardOptions> reward_option_rows;
	std::vector<RewardsRepository::Rewards> reward_rows;
	std::vector<RewardOptionEntriesRepository::RewardOptionEntries>
		reward_option_entry_rows;
	std::vector<RewardSourcesRepository::RewardSources> reward_source_rows;
	std::vector<RewardSourceEntriesRepository::RewardSourceEntries>
		reward_source_entry_rows;
	if (
		!RewardSetsRepository::GetAll(database, reward_set_rows) ||
		!RewardOptionsRepository::GetAll(database, reward_option_rows) ||
		!RewardsRepository::GetAll(database, reward_rows) ||
		!RewardOptionEntriesRepository::GetAll(
			database,
			reward_option_entry_rows
		) ||
		!RewardSourcesRepository::GetAll(database, reward_source_rows) ||
		!RewardSourceEntriesRepository::GetAll(
			database,
			reward_source_entry_rows
		)
	) {
		LogError("Failed to read the shared reward catalog");
		return false;
	}

	const auto source_type = static_cast<uint8_t>(source);
	const bool recover_invalid_task_sets = source == RewardSelectionSource::Task;
	std::unordered_set<uint32_t> disabled_reward_ids;
	for (const auto &row : reward_rows) {
		if (!row.enabled && row.reward_id) {
			disabled_reward_ids.insert(row.reward_id);
		}
	}
	std::unordered_map<uint64_t, uint32_t> source_sets;
	std::unordered_set<uint32_t> active_set_ids;
	std::unordered_set<uint64_t> invalid_source_ids;
	for (const auto &row : reward_source_rows) {
		if (
			row.source_type != source_type ||
			!row.enabled ||
			!active_source_ids.contains(row.source_id)
		) {
			continue;
		}
		if (!row.source_id || !row.reward_set_id) {
			LogError(
				"Reward source [{}/{}] has an invalid or duplicate selectable set",
				static_cast<uint32_t>(source_type),
				row.source_id
			);
			if (!recover_invalid_task_sets) {
				return false;
			}
			invalid_source_ids.insert(row.source_id);
			source_sets.erase(row.source_id);
			continue;
		}
		if (
			invalid_source_ids.contains(row.source_id) ||
			!source_sets.emplace(row.source_id, row.reward_set_id).second
		) {
			LogError(
				"Reward source [{}/{}] has an invalid or duplicate selectable set",
				static_cast<uint32_t>(source_type),
				row.source_id
			);
			if (!recover_invalid_task_sets) {
				return false;
			}
			invalid_source_ids.insert(row.source_id);
			source_sets.erase(row.source_id);
			continue;
		}
	}
	for (const auto &source_set : source_sets) {
		active_set_ids.insert(source_set.second);
	}

	// Tasks already have their own automatic-reward fields. An effective direct
	// entry is a provider-level configuration error; mappings to explicitly
	// disabled rewards remain inert.
	if (source == RewardSelectionSource::Task) {
		for (const auto &row : reward_source_entry_rows) {
			if (
				row.source_type == source_type &&
				active_source_ids.contains(row.source_id) &&
				!disabled_reward_ids.contains(row.reward_id)
			) {
				LogError(
					"Task reward source [{}] has an unsupported automatic entry [{}]",
					row.source_id,
					row.reward_id
				);
				return false;
			}
		}
	}

	std::unordered_map<uint32_t, StagedRewardSet> staged_sets;
	std::unordered_set<uint32_t> found_set_ids;
	std::unordered_set<uint32_t> disabled_set_ids;
	std::unordered_set<uint32_t> invalid_set_ids;
	for (const auto &row : reward_set_rows) {
		if (!active_set_ids.contains(row.reward_set_id)) {
			continue;
		}
		if (!row.reward_set_id || !found_set_ids.insert(row.reward_set_id).second) {
			LogError("Reward set [{}] is invalid or duplicated", row.reward_set_id);
			if (!recover_invalid_task_sets) {
				return false;
			}
			invalid_set_ids.insert(row.reward_set_id);
			continue;
		}
		if (!row.enabled) {
			disabled_set_ids.insert(row.reward_set_id);
			continue;
		}
		StagedRewardSet staged;
		staged.set.reward_set_id = row.reward_set_id;
		staged.set.title = row.title;
		staged_sets.emplace(row.reward_set_id, std::move(staged));
	}
	for (const auto reward_set_id : active_set_ids) {
		if (!found_set_ids.contains(reward_set_id)) {
			LogError("Enabled reward source references unavailable set [{}]", reward_set_id);
			if (!recover_invalid_task_sets) {
				return false;
			}
			invalid_set_ids.insert(reward_set_id);
		}
	}
	for (auto source_set = source_sets.begin(); source_set != source_sets.end();) {
		if (
			disabled_set_ids.contains(source_set->second) ||
			invalid_set_ids.contains(source_set->second)
		) {
			source_set = source_sets.erase(source_set);
		} else {
			++source_set;
		}
	}

	for (const auto &row : reward_option_rows) {
		auto staged = staged_sets.find(row.reward_set_id);
		if (staged == staged_sets.end()) {
			continue;
		}
		if (!row.enabled) {
			if (row.option_id) {
				staged->second.disabled_option_ids.insert(row.option_id);
			}
			continue;
		}
		if (
			!row.option_id ||
			!staged->second.option_indices.emplace(
				row.option_id,
				staged->second.set.options.size()
			).second
		) {
			LogError(
				"Reward option [{}/{}] is invalid or duplicated",
				row.reward_set_id,
				row.option_id
			);
			if (!recover_invalid_task_sets) {
				return false;
			}
			invalid_set_ids.insert(row.reward_set_id);
			continue;
		}

		staged->second.set.options.push_back({
			.option_id = row.option_id,
			.sequence = row.sequence,
			.label = row.label,
			.common_to_all = row.common_to_all != 0,
			.flags = row.flags
		});
	}

	// Reward rows are shared by every provider. Validate only the graph reachable
	// from this provider's enabled options and direct source entries.
	std::unordered_set<uint32_t> referenced_reward_ids;
	std::unordered_map<uint32_t, std::unordered_set<uint32_t>>
		reward_set_references;
	for (const auto &row : reward_option_entry_rows) {
		const auto staged = staged_sets.find(row.reward_set_id);
		if (
			staged != staged_sets.end() &&
			!staged->second.disabled_option_ids.contains(row.option_id)
		) {
			referenced_reward_ids.insert(row.reward_id);
			reward_set_references[row.reward_id].insert(row.reward_set_id);
		}
	}
	for (const auto &row : reward_source_entry_rows) {
		if (
			row.source_type == source_type &&
			active_source_ids.contains(row.source_id)
		) {
			referenced_reward_ids.insert(row.reward_id);
		}
	}

	std::unordered_map<uint32_t, RewardSelectionReward> rewards;
	std::unordered_set<uint32_t> invalid_reward_ids;
	for (const auto &row : reward_rows) {
		if (!referenced_reward_ids.contains(row.reward_id)) {
			continue;
		}
		if (!row.enabled) {
			if (row.reward_id) {
				disabled_reward_ids.insert(row.reward_id);
			}
			continue;
		}
		if (!ValidReward(row)) {
			LogError("Enabled reward [{}] has invalid delivery data", row.reward_id);
			if (!recover_invalid_task_sets) {
				return false;
			}
			invalid_reward_ids.insert(row.reward_id);
			continue;
		}
		if (!rewards.emplace(row.reward_id, ToReward(row)).second) {
			LogError("Enabled reward ID [{}] is duplicated", row.reward_id);
			if (!recover_invalid_task_sets) {
				return false;
			}
			invalid_reward_ids.insert(row.reward_id);
			rewards.erase(row.reward_id);
		}
	}
	for (const auto reward_id : referenced_reward_ids) {
		if (disabled_reward_ids.contains(reward_id)) {
			continue;
		}
		if (
			!invalid_reward_ids.contains(reward_id) &&
			!rewards.contains(reward_id)
		) {
			LogError("Enabled reward entry references unavailable reward [{}]", reward_id);
			if (!recover_invalid_task_sets) {
				return false;
			}
		}
		if (
			rewards.contains(reward_id) &&
			!invalid_reward_ids.contains(reward_id)
		) {
			continue;
		}
		for (const auto reward_set_id : reward_set_references[reward_id]) {
			invalid_set_ids.insert(reward_set_id);
		}
	}
	for (const auto &row : reward_option_entry_rows) {
		auto staged = staged_sets.find(row.reward_set_id);
		if (
			staged == staged_sets.end() ||
			invalid_set_ids.contains(row.reward_set_id)
		) {
			continue;
		}
		if (
			staged->second.disabled_option_ids.contains(row.option_id) ||
			disabled_reward_ids.contains(row.reward_id)
		) {
			continue;
		}
		const auto option = staged->second.option_indices.find(row.option_id);
		const auto reward = rewards.find(row.reward_id);
		if (
			option == staged->second.option_indices.end() ||
			reward == rewards.end() ||
			!staged->second.reward_ids.insert(row.reward_id).second
		) {
			LogError(
				"Reward option entry [{}/{}/{}] is invalid or duplicated",
				row.reward_set_id,
				row.option_id,
				row.reward_id
			);
			if (!recover_invalid_task_sets) {
				return false;
			}
			invalid_set_ids.insert(row.reward_set_id);
			continue;
		}

		staged->second.reward_sequences.emplace(row.reward_id, row.sequence);
		staged->second.set.options[option->second].rewards.push_back(reward->second);
	}

	for (auto &[reward_set_id, staged] : staged_sets) {
		if (invalid_set_ids.contains(reward_set_id)) {
			continue;
		}
		bool has_selectable_option = false;
		for (auto &option : staged.set.options) {
			if (option.rewards.empty()) {
				LogError(
					"Enabled reward option [{}/{}] has no enabled entries",
					reward_set_id,
					option.option_id
				);
				if (!recover_invalid_task_sets) {
					return false;
				}
				invalid_set_ids.insert(reward_set_id);
				break;
			}
			std::sort(
				option.rewards.begin(),
				option.rewards.end(),
				[&staged](const auto &left, const auto &right) {
					const auto left_sequence = staged.reward_sequences.at(
						static_cast<uint32_t>(left.entry_id)
					);
					const auto right_sequence = staged.reward_sequences.at(
						static_cast<uint32_t>(right.entry_id)
					);
					return left_sequence != right_sequence
						? left_sequence < right_sequence
						: left.entry_id < right.entry_id;
				}
			);
			has_selectable_option = has_selectable_option || !option.common_to_all;
		}
		if (invalid_set_ids.contains(reward_set_id)) {
			continue;
		}
		std::sort(
			staged.set.options.begin(),
			staged.set.options.end(),
			[](const auto &left, const auto &right) {
				return left.sequence != right.sequence
					? left.sequence < right.sequence
					: left.option_id < right.option_id;
			}
		);
		if (staged.set.options.empty() || !has_selectable_option) {
			LogError("Enabled reward set [{}] has no selectable option", reward_set_id);
			if (!recover_invalid_task_sets) {
				return false;
			}
			invalid_set_ids.insert(reward_set_id);
		}
	}

	for (const auto reward_set_id : invalid_set_ids) {
		staged_sets.erase(reward_set_id);
	}
	for (auto source_set = source_sets.begin(); source_set != source_sets.end();) {
		if (invalid_set_ids.contains(source_set->second)) {
			source_set = source_sets.erase(source_set);
		} else {
			++source_set;
		}
	}

	for (const auto &[source_id, reward_set_id] : source_sets) {
		catalog.selectable.emplace(source_id, staged_sets.at(reward_set_id).set);
	}

	std::map<std::pair<uint64_t, uint32_t>, uint32_t> automatic_sequences;
	for (const auto &row : reward_source_entry_rows) {
		if (
			row.source_type != source_type ||
			!active_source_ids.contains(row.source_id)
		) {
			continue;
		}
		if (disabled_reward_ids.contains(row.reward_id)) {
			continue;
		}
		const auto reward = rewards.find(row.reward_id);
		const auto selected_set = catalog.selectable.find(row.source_id);
		if (
			!row.source_id ||
			reward == rewards.end() ||
			(selected_set != catalog.selectable.end() &&
				staged_sets.at(selected_set->second.reward_set_id)
					.reward_ids.contains(row.reward_id)) ||
			!automatic_sequences.emplace(
				std::make_pair(row.source_id, row.reward_id),
				row.sequence
			).second
		) {
			LogError(
				"Automatic reward source entry [{}/{}/{}] is invalid or duplicated",
				static_cast<uint32_t>(source_type),
				row.source_id,
				row.reward_id
			);
			return false;
		}
		catalog.automatic[row.source_id].push_back(reward->second);
	}

	for (auto &[source_id, source_rewards] : catalog.automatic) {
		std::sort(
			source_rewards.begin(),
			source_rewards.end(),
			[source_id, &automatic_sequences](const auto &left, const auto &right) {
				const auto left_sequence = automatic_sequences.at({
					source_id,
					static_cast<uint32_t>(left.entry_id)
				});
				const auto right_sequence = automatic_sequences.at({
					source_id,
					static_cast<uint32_t>(right.entry_id)
				});
				return left_sequence != right_sequence
					? left_sequence < right_sequence
					: left.entry_id < right.entry_id;
			}
		);
	}

	return true;
}
