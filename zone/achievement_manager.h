#pragma once

#include "common/achievements.h"
#include "zone/reward_selection.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

struct AchievementCriterion {
	uint64_t                            criterion_id = 0;
	uint32_t                            achievement_id = 0;
	uint8_t                             component_type = 0;
	uint32_t                            component_id = 0;
	EQ::Achievements::EventType         event_type = EQ::Achievements::EventType::Manual;
	uint32_t                            target_id = 0;
	uint32_t                            target_id2 = 0;
	EQ::Achievements::ProgressMode      progress_mode = EQ::Achievements::ProgressMode::Increment;
	EQ::Achievements::CriterionBehavior behavior = EQ::Achievements::CriterionBehavior::Required;
	int64_t                             target_value = 0;
	uint32_t                            required_count = 1;
};

struct AchievementCastRequirement {
	uint32_t achievement_id = 0;
	bool     requires_completed = true;
};

class AchievementManager
{
public:
	static AchievementManager &Instance();

	bool Load();
	bool Reload();

	bool IsLoaded() const { return m_loaded; }
	const std::vector<EQ::Achievements::Category> &Categories() const { return m_categories; }
	const std::vector<EQ::Achievements::Definition> &Definitions() const { return m_definitions; }
	const SerializeBuffer &DefinitionPacket() const { return m_definition_packet; }

	const EQ::Achievements::Definition *FindDefinition(uint32_t achievement_id) const;
	std::optional<size_t> FindDefinitionIndex(uint32_t achievement_id) const;
	std::optional<size_t> FindComponentIndex(
		uint32_t achievement_id,
		uint8_t component_type,
		uint32_t component_id
	) const;

	const std::vector<const AchievementCriterion *> &Criteria(EQ::Achievements::EventType event_type) const;
	const std::vector<const AchievementCriterion *> &NpcNameKillCriteria(
		uint32_t name_identity
	) const;
	const std::vector<const AchievementCriterion *> &CriteriaForAchievement(uint32_t achievement_id) const;
	// Rows not assigned to a selectable reward set are granted automatically.
	const std::vector<RewardSelectionReward> &Rewards(uint32_t achievement_id) const;
	const RewardSelectionSet *RewardSet(uint32_t achievement_id) const;
	const std::vector<AchievementCastRequirement> &CastRequirements(uint32_t restriction_id) const;
	std::optional<uint8_t> RequiredClass(uint32_t achievement_id) const;
	bool ResetOnVersionChange(uint32_t achievement_id) const;

private:
	AchievementManager() = default;

	void Clear();
	bool LoadSnapshot();
	bool ReplaceFromDatabase(bool is_reload);
	void Swap(AchievementManager &other);

	bool m_loaded = false;
	std::vector<EQ::Achievements::Category> m_categories;
	std::vector<EQ::Achievements::Definition> m_definitions;
	std::vector<AchievementCriterion> m_criteria;
	SerializeBuffer m_definition_packet;

	std::unordered_map<uint32_t, size_t> m_definition_indices;
	std::unordered_map<uint8_t, std::vector<const AchievementCriterion *>> m_criteria_by_event;
	std::unordered_map<uint32_t, std::vector<const AchievementCriterion *>>
		m_npc_name_kill_criteria;
	std::unordered_map<uint32_t, std::vector<const AchievementCriterion *>> m_criteria_by_achievement;
	std::unordered_map<uint32_t, std::vector<RewardSelectionReward>> m_rewards;
	std::unordered_map<uint32_t, RewardSelectionSet> m_reward_sets;
	std::unordered_map<uint32_t, std::vector<AchievementCastRequirement>> m_cast_requirements;
	std::unordered_map<uint32_t, uint8_t> m_required_classes;
	std::unordered_map<uint32_t, bool> m_reset_on_version_change;
};
