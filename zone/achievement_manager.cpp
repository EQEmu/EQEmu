#include "achievement_manager.h"

#include "../common/classes.h"
#include "../common/eqemu_logsys.h"
#include "../common/rulesys.h"
#include "../common/skills.h"
#include "reward_selection.h"
#include "zonedb.h"

#include <algorithm>
#include <cstdlib>
#include <limits>
#include <map>
#include <set>
#include <tuple>
#include <unordered_set>

namespace
{

uint32_t ParseUInt32(const char *value)
{
	return value ? static_cast<uint32_t>(std::strtoul(value, nullptr, 10)) : 0;
}

uint64_t ParseUInt64(const char *value)
{
	return value ? static_cast<uint64_t>(std::strtoull(value, nullptr, 10)) : 0;
}

int64_t ParseInt64(const char *value)
{
	return value ? static_cast<int64_t>(std::strtoll(value, nullptr, 10)) : 0;
}

std::string Text(const char *value)
{
	return value ? value : "";
}

uint32_t RequiredCount(uint64_t value)
{
	return static_cast<uint32_t>(std::clamp<uint64_t>(
		value ? value : 1,
		1,
		std::numeric_limits<uint32_t>::max()
	));
}

} // namespace

AchievementManager &AchievementManager::Instance()
{
	static AchievementManager instance;
	return instance;
}

void AchievementManager::Clear()
{
	m_loaded = false;
	m_categories.clear();
	m_definitions.clear();
	m_criteria.clear();
	m_definition_packet = SerializeBuffer{};
	m_definition_indices.clear();
	m_criteria_by_event.clear();
	m_npc_name_kill_criteria.clear();
	m_criteria_by_achievement.clear();
	m_rewards.clear();
	m_reward_sets.clear();
	m_reward_set_achievements.clear();
	m_cast_restrictions.clear();
	m_required_classes.clear();
	m_reset_on_version_change.clear();
}

bool AchievementManager::Load()
{
	return ReplaceFromDatabase(false);
}

bool AchievementManager::Reload()
{
	return ReplaceFromDatabase(true);
}

bool AchievementManager::ReplaceFromDatabase(bool is_reload)
{
	AchievementManager staged;
	if (!staged.LoadSnapshot()) {
		if (is_reload && m_loaded) {
			LogError(
				"Achievement reload failed; the currently loaded achievement content "
				"remains active"
			);
		}
		return false;
	}

	Swap(staged);
	LogInfo(
		"{} [{}] achievement categories, [{}] definitions, and [{}] evaluation criteria",
		is_reload ? "Reloaded" : "Loaded",
		m_categories.size(),
		m_definitions.size(),
		m_criteria.size()
	);
	return true;
}

void AchievementManager::Swap(AchievementManager &other)
{
	using std::swap;

	// The indexes point into m_criteria. vector::swap preserves those addresses,
	// so the owning vectors and indexes must be swapped together.
	swap(m_loaded, other.m_loaded);
	swap(m_categories, other.m_categories);
	swap(m_definitions, other.m_definitions);
	swap(m_criteria, other.m_criteria);
	swap(m_definition_packet, other.m_definition_packet);
	swap(m_definition_indices, other.m_definition_indices);
	swap(m_criteria_by_event, other.m_criteria_by_event);
	swap(m_npc_name_kill_criteria, other.m_npc_name_kill_criteria);
	swap(m_criteria_by_achievement, other.m_criteria_by_achievement);
	swap(m_rewards, other.m_rewards);
	swap(m_reward_sets, other.m_reward_sets);
	swap(m_reward_set_achievements, other.m_reward_set_achievements);
	swap(m_cast_restrictions, other.m_cast_restrictions);
	swap(m_required_classes, other.m_required_classes);
	swap(m_reset_on_version_change, other.m_reset_on_version_change);
}

bool AchievementManager::LoadSnapshot()
{
	using namespace EQ::Achievements;

	Clear();
	if (!RuleB(Achievements, EnableAchievements)) {
		try {
			// Connected clients need an empty packet to clear the previous snapshot.
			auto definitions = SerializeDefinitions({}, {});
			m_definition_packet = CompressDefinitions(definitions);
		}
		catch (const std::exception &error) {
			LogError(
				"Failed to build disabled achievement definition packet: {}",
				error.what()
			);
			return false;
		}
		LogInfo("Achievement system is disabled");
		m_loaded = true;
		return true;
	}

	if (
		!content_db.DoesTableExist("achievements") ||
		!content_db.DoesTableExist("achievement_categories")
	) {
		LogError("Achievement content tables are missing; run the required database updates");
		return false;
	}

	auto category_results = content_db.QueryDatabase(
		"SELECT id, parent_id, sequence, name, description, icon "
		"FROM achievement_categories ORDER BY parent_id, sequence, id"
	);
	if (!category_results.Success()) {
		LogError("Failed to load achievement categories");
		return false;
	}

	std::unordered_map<uint32_t, size_t> category_indices;
	for (auto row : category_results) {
		Category category;
		category.category_id = ParseUInt32(row[0]);
		if (category_indices.contains(category.category_id)) {
			LogError("Duplicate achievement category ID [{}]", category.category_id);
			Clear();
			return false;
		}
		category.parent_category_id = ParseUInt32(row[1]);
		category.display_order = ParseUInt32(row[2]);
		category.name = Text(row[3]);
		category.description = Text(row[4]);
		category.icon = Text(row[5]);
		category_indices[category.category_id] = m_categories.size();
		m_categories.emplace_back(std::move(category));
	}

	auto definition_results = content_db.QueryDatabase(
		"SELECT id, name, description, icon_id, definition_version, "
		"points, reward_display, reset_on_version_change "
		"FROM achievements WHERE enabled = 1 ORDER BY id"
	);
	if (!definition_results.Success()) {
		LogError("Failed to load achievement definitions");
		Clear();
		return false;
	}

	for (auto row : definition_results) {
		Definition definition;
		definition.achievement_id = ParseUInt32(row[0]);
		if (m_definition_indices.contains(definition.achievement_id)) {
			LogError("Duplicate enabled achievement ID [{}]", definition.achievement_id);
			Clear();
			return false;
		}
		definition.name = Text(row[1]);
		definition.description = Text(row[2]);
		definition.icon_id = ParseUInt32(row[3]);
		definition.definition_version = ParseUInt32(row[4]);
		if (!definition.definition_version) {
			LogError(
				"Enabled achievement [{}] has definition version zero",
				definition.achievement_id
			);
			Clear();
			return false;
		}
		definition.points = ParseUInt32(row[5]);
		definition.reward_display = ParseUInt32(row[6]);
		m_reset_on_version_change[definition.achievement_id] = ParseUInt32(row[7]) != 0;
		m_definition_indices[definition.achievement_id] = m_definitions.size();
		m_definitions.emplace_back(std::move(definition));
	}

	auto association_results = content_db.QueryDatabase(
		"SELECT a.category_id, a.sequence, a.achievement_id, a.display_text "
		"FROM achievement_category_associations a "
		"INNER JOIN achievements d ON d.id = a.achievement_id AND d.enabled = 1 "
		"ORDER BY a.category_id, a.sequence, a.achievement_id"
	);
	if (!association_results.Success()) {
		LogError("Failed to load achievement category associations");
		Clear();
		return false;
	}

	std::unordered_set<uint32_t> associated_achievement_ids;
	for (auto row : association_results) {
		const auto category = category_indices.find(ParseUInt32(row[0]));
		if (category == category_indices.end()) {
			continue;
		}

		const auto achievement_id = ParseUInt32(row[2]);
		m_categories[category->second].associations.push_back({
			achievement_id,
			Text(row[3]),
			ParseUInt32(row[1])
		});
		associated_achievement_ids.insert(achievement_id);
	}

	for (const auto &definition : m_definitions) {
		if (!associated_achievement_ids.contains(definition.achievement_id)) {
			LogError(
				"Enabled achievement [{}] has no valid category association",
				definition.achievement_id
			);
			Clear();
			return false;
		}
	}

	try {
		m_categories = SelectActiveCategories(m_categories);
	}
	catch (const std::exception &error) {
		LogError("Invalid active achievement category hierarchy: {}", error.what());
		Clear();
		return false;
	}

	auto component_results = content_db.QueryDatabase(
		"SELECT c.achievement_id, c.component_type, c.sequence, c.component_id, "
		"c.description, c.description_2, COALESCE(n.required_count, 1) "
		"FROM achievement_components c "
		"INNER JOIN achievements d ON d.id = c.achievement_id AND d.enabled = 1 "
		"LEFT JOIN achievement_component_counts n ON n.component_id = c.component_id "
		"ORDER BY c.achievement_id, c.component_type, c.sequence, c.component_id"
	);
	if (!component_results.Success()) {
		LogError("Failed to load achievement components");
		Clear();
		return false;
	}

	for (auto row : component_results) {
		const auto achievement_id = ParseUInt32(row[0]);
		const auto definition = m_definition_indices.find(achievement_id);
		const auto component_type = ParseUInt32(row[1]);
		if (definition == m_definition_indices.end() || component_type > 3) {
			LogError(
				"Enabled achievement [{}] has an invalid component type [{}]",
				achievement_id,
				component_type
			);
			Clear();
			return false;
		}
		if (
			FindComponentIndex(
				achievement_id,
				static_cast<uint8_t>(component_type),
				ParseUInt32(row[3])
			)
		) {
			LogError(
				"Enabled achievement [{}] has duplicate component identity [{}/{}]",
				achievement_id,
				component_type,
				ParseUInt32(row[3])
			);
			Clear();
			return false;
		}

		Component component;
		component.component_type = static_cast<uint8_t>(component_type);
		component.sequence = ParseUInt32(row[2]);
		component.component_id = ParseUInt32(row[3]);
		component.description = Text(row[4]);
		component.description2 = Text(row[5]);
		component.required_count = RequiredCount(ParseUInt64(row[6]));
		component.display_order = static_cast<uint8_t>(std::min<uint32_t>(component.sequence, 255));
		m_definitions[definition->second].components[component_type].emplace_back(std::move(component));
	}

	using ComponentPolicyKey = std::tuple<uint32_t, uint8_t, uint32_t>;
	struct ComponentPolicy {
		CriterionBehavior behavior;
		uint32_t required_count;
		EventType event_type;
		ProgressMode progress_mode;
	};
	std::map<ComponentPolicyKey, ComponentPolicy> component_policies;
	std::set<
		std::tuple<uint32_t, uint8_t, uint32_t, uint8_t, uint32_t, uint32_t>
	> criterion_keys;

	auto criterion_results = content_db.QueryDatabase(
		"SELECT c.id, c.achievement_id, c.component_type, c.component_id, c.event_type, "
		"c.progress_mode, c.behavior, c.target_id, c.target_id2, c.target_value, "
		"c.required_count FROM achievement_criteria c "
		"INNER JOIN achievements a ON a.id = c.achievement_id AND a.enabled = 1 "
		"WHERE c.enabled = 1 ORDER BY c.id"
	);
	if (!criterion_results.Success()) {
		LogError("Failed to load achievement evaluation criteria");
		Clear();
		return false;
	}

	for (auto row : criterion_results) {
		const auto achievement_id = ParseUInt32(row[1]);
		const auto component_type = ParseUInt32(row[2]);
		const auto event_type = ParseUInt32(row[4]);
		const auto progress_mode = ParseUInt32(row[5]);
		const auto behavior = ParseUInt32(row[6]);
		const auto target_id = ParseUInt32(row[7]);
		const auto target_id2 = ParseUInt32(row[8]);
		const auto target_value = ParseInt64(row[9]);
		if (
			component_type > 2 ||
			event_type > static_cast<uint32_t>(EventType::SkillCap) ||
			progress_mode > static_cast<uint32_t>(ProgressMode::Boolean) ||
			behavior > static_cast<uint32_t>(CriterionBehavior::Blocker) ||
			target_value < 0
		) {
			LogError(
				"Invalid enabled achievement criterion [{}]; RoF2 component type 3 is presentation-only",
				ParseUInt64(row[0])
			);
			Clear();
			return false;
		}
		const auto effective_event_type = static_cast<EventType>(event_type);
		const auto effective_progress_mode = static_cast<ProgressMode>(progress_mode);
		if (
			target_id2 &&
			effective_event_type != EventType::NpcNameKill &&
			effective_event_type != EventType::OwnItem &&
			effective_event_type != EventType::SkillCap
		) {
			LogError(
				"Achievement criterion [{}] uses unsupported target_id2 [{}]",
				ParseUInt64(row[0]),
				target_id2
			);
			Clear();
			return false;
		}
		if (effective_event_type == EventType::NpcNameKill && !target_id) {
			LogError(
				"Achievement criterion [{}] has no usable NPC-name identity hash",
				ParseUInt64(row[0])
			);
			Clear();
			return false;
		}
		if (effective_event_type == EventType::TaskComplete && !target_id) {
			LogError(
				"Achievement criterion [{}] must target a specific task so its "
				"one-time completion can be reconciled",
				ParseUInt64(row[0])
			);
			Clear();
			return false;
		}
		if (
			effective_event_type == EventType::OwnItem &&
			target_id2 &&
			(
				target_id2 < Class::Warrior ||
				target_id2 > Class::Berserker
			)
		) {
			LogError(
				"Achievement criterion [{}] has invalid OwnItem class [{}]",
				ParseUInt64(row[0]),
				target_id2
			);
			Clear();
			return false;
		}
		if (
			effective_event_type == EventType::SkillCap &&
			(
				target_id > static_cast<uint32_t>(EQ::skills::HIGHEST_SKILL) ||
				target_id2 < Class::Warrior ||
				target_id2 > Class::Berserker ||
				target_value <= 0 ||
				target_value > std::numeric_limits<uint8_t>::max()
			)
		) {
			LogError(
				"Achievement criterion [{}] has invalid SkillCap skill [{}], "
				"class [{}], or milestone level [{}]",
				ParseUInt64(row[0]),
				target_id,
				target_id2,
				target_value
			);
			Clear();
			return false;
		}
		if (
			effective_event_type == EventType::SkillValue &&
			target_id != SkillWildcardTargetId &&
			target_id > static_cast<uint32_t>(EQ::skills::HIGHEST_SKILL)
		) {
			LogError(
				"Achievement criterion [{}] targets invalid skill ID [{}]; use "
				"[{}] for the SkillValue wildcard",
				ParseUInt64(row[0]),
				target_id,
				SkillWildcardTargetId
			);
			Clear();
			return false;
		}
		const auto absolute_fact_event =
			effective_event_type == EventType::Level ||
			effective_event_type == EventType::OwnItem ||
			effective_event_type == EventType::SkillValue ||
			effective_event_type == EventType::SkillCap ||
			effective_event_type == EventType::AlternateAdvancement;
		const auto replayed_specific_event =
			target_id && effective_event_type == EventType::TaskComplete;
		const auto replayed_achievement_event =
			effective_event_type == EventType::AchievementComplete;
		if (
			effective_progress_mode == ProgressMode::Increment &&
			(
				absolute_fact_event ||
				replayed_specific_event ||
				replayed_achievement_event
			)
		) {
			LogError(
				"Achievement criterion [{}] uses increment mode for a reconciled "
				"absolute or one-time event",
				ParseUInt64(row[0])
			);
			Clear();
			return false;
		}
		if (
			effective_progress_mode == ProgressMode::Boolean &&
			absolute_fact_event &&
			target_value <= 0
		) {
			LogError(
				"Achievement criterion [{}] must have a positive target value "
				"when Boolean mode evaluates an absolute fact",
				ParseUInt64(row[0])
			);
			Clear();
			return false;
		}

		const auto component_index = FindComponentIndex(
			achievement_id,
			static_cast<uint8_t>(component_type),
			ParseUInt32(row[3])
		);
		const auto definition_index = FindDefinitionIndex(achievement_id);
		if (!component_index || !definition_index) {
			LogError(
				"Enabled achievement criterion [{}] references missing component [{}/{}/{}]",
				ParseUInt64(row[0]),
				achievement_id,
				component_type,
				ParseUInt32(row[3])
			);
			Clear();
			return false;
		}

		auto &component = m_definitions[*definition_index].components[component_type][*component_index];
		const auto component_key = ComponentPolicyKey{
			achievement_id,
			static_cast<uint8_t>(component_type),
			component.component_id
		};
		if (
			!criterion_keys.emplace(
				achievement_id,
				static_cast<uint8_t>(component_type),
				component.component_id,
				static_cast<uint8_t>(effective_event_type),
				target_id,
				target_id2
			).second
		) {
			LogError(
				"Duplicate enabled achievement criterion identity [{}/{}/{}/{}/{}/{}]",
				achievement_id,
				component_type,
				component.component_id,
				event_type,
				target_id,
				target_id2
			);
			Clear();
			return false;
		}
		const auto required_override = ParseUInt64(row[10]);
		if (
			!required_override ||
			required_override > std::numeric_limits<uint32_t>::max()
		) {
			LogError(
				"Achievement criterion [{}] must have an explicit uint32 required count",
				ParseUInt64(row[0])
			);
			Clear();
			return false;
		}
		const auto effective_required_count = static_cast<uint32_t>(required_override);
		const auto effective_behavior = static_cast<CriterionBehavior>(behavior);
		const auto [policy, inserted] = component_policies.try_emplace(
			component_key,
			ComponentPolicy{
				effective_behavior,
				effective_required_count,
				effective_event_type,
				effective_progress_mode
			}
		);
		if (
			!inserted &&
			(
				policy->second.behavior != effective_behavior ||
				policy->second.required_count != effective_required_count ||
				policy->second.event_type != effective_event_type ||
				policy->second.progress_mode != effective_progress_mode
			)
		) {
			LogError(
				"Achievement criteria for component [{}/{}/{}] have conflicting "
				"behavior, required-count, event, or progress-mode policy",
				achievement_id,
				component_type,
				component.component_id
			);
			Clear();
			return false;
		}

		AchievementCriterion criterion;
		criterion.criterion_id = ParseUInt64(row[0]);
		criterion.achievement_id = achievement_id;
		criterion.component_type = static_cast<uint8_t>(component_type);
		criterion.component_id = ParseUInt32(row[3]);
		criterion.event_type = effective_event_type;
		criterion.progress_mode = effective_progress_mode;
		criterion.behavior = effective_behavior;
		criterion.target_id = target_id;
		criterion.target_id2 = target_id2;
		criterion.target_value = target_value;
		criterion.required_count = effective_required_count;
		m_criteria.emplace_back(std::move(criterion));

		component.required_count = effective_required_count;
	}

	// Build pointer indexes only after the criterion vector has stopped reallocating.
	for (const auto &criterion : m_criteria) {
		m_criteria_by_event[static_cast<uint8_t>(criterion.event_type)].push_back(&criterion);
		if (
			criterion.event_type == EventType::NpcNameKill &&
			criterion.target_id
		) {
			m_npc_name_kill_criteria[criterion.target_id].push_back(&criterion);
		}
		m_criteria_by_achievement[criterion.achievement_id].push_back(&criterion);
	}

	std::unordered_map<uint32_t, std::set<uint8_t>> required_classes;
	for (const auto &criterion : m_criteria) {
		const auto class_scoped_event =
			criterion.event_type == EventType::OwnItem ||
			criterion.event_type == EventType::SkillCap;
		const auto completion_policy =
			criterion.behavior != CriterionBehavior::Optional &&
			criterion.behavior != CriterionBehavior::DisplayOnly &&
			criterion.behavior != CriterionBehavior::Blocker;
		if (
			class_scoped_event &&
			completion_policy &&
			criterion.target_id2 >= Class::Warrior &&
			criterion.target_id2 <= Class::Berserker
		) {
			required_classes[criterion.achievement_id].insert(
				static_cast<uint8_t>(criterion.target_id2)
			);
		}
	}
	for (const auto &[achievement_id, classes] : required_classes) {
		if (classes.size() != 1) {
			LogError(
				"Achievement [{}] has contradictory required class criteria",
				achievement_id
			);
			Clear();
			return false;
		}
		m_required_classes[achievement_id] = *classes.begin();
	}

	auto reward_set_results = content_db.QueryDatabase(
		"SELECT reward_set_id, achievement_id, title "
		"FROM achievement_reward_sets WHERE enabled = 1 "
		"ORDER BY achievement_id, reward_set_id"
	);
	if (!reward_set_results.Success()) {
		LogError("Failed to load achievement reward sets");
		Clear();
		return false;
	}

	for (auto row : reward_set_results) {
		AchievementRewardSet reward_set;
		reward_set.reward_set_id = ParseUInt32(row[0]);
		reward_set.achievement_id = ParseUInt32(row[1]);
		reward_set.title = Text(row[2]);
		if (
			!reward_set.reward_set_id ||
			!m_definition_indices.contains(reward_set.achievement_id) ||
			m_reward_sets.contains(reward_set.achievement_id) ||
			!m_reward_set_achievements.emplace(
				reward_set.reward_set_id,
				reward_set.achievement_id
			).second
		) {
			LogError(
				"Enabled achievement reward set [{}/{}] is invalid or duplicated",
				reward_set.reward_set_id,
				reward_set.achievement_id
			);
			Clear();
			return false;
		}
		if (reward_set.title.empty()) {
			reward_set.title =
				m_definitions[m_definition_indices[reward_set.achievement_id]].name;
		}
		m_reward_sets.emplace(
			reward_set.achievement_id,
			std::move(reward_set)
		);
	}

	auto reward_option_results = content_db.QueryDatabase(
		"SELECT reward_set_id, option_id, sequence, label, common_to_all, flags "
		"FROM achievement_reward_options WHERE enabled = 1 "
		"ORDER BY reward_set_id, sequence, option_id"
	);
	if (!reward_option_results.Success()) {
		LogError("Failed to load achievement reward options");
		Clear();
		return false;
	}

	std::map<std::pair<uint32_t, uint32_t>, size_t> reward_option_indices;
	for (auto row : reward_option_results) {
		const auto reward_set_id = ParseUInt32(row[0]);
		const auto reward_set_achievement = m_reward_set_achievements.find(reward_set_id);
		if (reward_set_achievement == m_reward_set_achievements.end()) {
			continue;
		}

		auto &reward_set = m_reward_sets[reward_set_achievement->second];
		AchievementRewardOption option;
		option.option_id = ParseUInt32(row[1]);
		option.sequence = ParseUInt32(row[2]);
		option.label = Text(row[3]);
		option.common_to_all = ParseUInt32(row[4]) != 0;
		const auto flags = ParseUInt32(row[5]);
		if (
			!option.option_id ||
			flags > std::numeric_limits<uint8_t>::max() ||
			!reward_option_indices.emplace(
				std::make_pair(reward_set_id, option.option_id),
				reward_set.options.size()
			).second
		) {
			LogError(
				"Enabled achievement reward option [{}/{}] is invalid or duplicated",
				reward_set_id,
				option.option_id
			);
			Clear();
			return false;
		}
		option.flags = static_cast<uint8_t>(flags);
		reward_set.options.emplace_back(std::move(option));
	}

	auto reward_results = content_db.QueryDatabase(
		"SELECT reward_id, achievement_id, reward_type, reward_data_id, amount, description "
		"FROM achievement_rewards WHERE enabled = 1 "
		"ORDER BY achievement_id, sequence, reward_id"
	);
	if (!reward_results.Success()) {
		LogError("Failed to load achievement rewards");
		Clear();
		return false;
	}

	std::unordered_map<uint64_t, AchievementReward> reward_rows;
	for (auto row : reward_results) {
		const auto reward_row_id = ParseUInt64(row[0]);
		const auto achievement_id = ParseUInt32(row[1]);
		const auto reward_type = ParseUInt32(row[2]);
		if (!m_definition_indices.contains(achievement_id)) {
			continue;
		}
		if (reward_type > static_cast<uint32_t>(RewardType::Title)) {
			LogError(
				"Enabled achievement reward [{}] has invalid type [{}]",
				reward_row_id,
				reward_type
			);
			Clear();
			return false;
		}

		const auto reward_data_id = ParseUInt32(row[3]);
		const auto amount = ParseUInt64(row[4]);
		const auto effective_reward_type = static_cast<RewardType>(reward_type);
		const auto requires_data_id =
			effective_reward_type == RewardType::Item ||
			effective_reward_type == RewardType::AlternateCurrency ||
			effective_reward_type == RewardType::Title;
		const auto invalid_experience_mode =
			effective_reward_type == RewardType::Experience &&
			reward_data_id > static_cast<uint32_t>(
				RewardSelectionExperienceMode::NormalOnly
			);
		if (
			!reward_row_id ||
			reward_row_id > std::numeric_limits<uint32_t>::max() ||
			!amount ||
			(requires_data_id && !reward_data_id) ||
			invalid_experience_mode
		) {
			LogError(
				"Enabled achievement reward [{}] has an invalid RoF2 wire ID, "
				"data ID, or amount",
				reward_row_id
			);
			Clear();
			return false;
		}

		AchievementReward reward;
		reward.reward_row_id = reward_row_id;
		reward.achievement_id = achievement_id;
		reward.reward_type = effective_reward_type;
		reward.reward_id = reward_data_id;
		reward.amount = amount;
		reward.description = Text(row[5]);
		if (!reward_rows.emplace(reward_row_id, std::move(reward)).second) {
			LogError("Duplicate enabled achievement reward ID [{}]", reward_row_id);
			Clear();
			return false;
		}
	}

	auto reward_mapping_results = content_db.QueryDatabase(
		"SELECT reward_set_id, option_id, reward_id "
		"FROM achievement_reward_option_entries "
		"ORDER BY reward_set_id, option_id, reward_id"
	);
	if (!reward_mapping_results.Success()) {
		LogError("Failed to load achievement reward option entries");
		Clear();
		return false;
	}

	std::unordered_set<uint64_t> mapped_reward_ids;
	// A mapping keeps its reward out of automatic grants even when the set,
	// option, or reward row is disabled.
	for (auto row : reward_mapping_results) {
		const auto reward_set_id = ParseUInt32(row[0]);
		const auto option_id = ParseUInt32(row[1]);
		const auto reward_row_id = ParseUInt64(row[2]);
		if (!mapped_reward_ids.insert(reward_row_id).second) {
			LogError(
				"Achievement reward [{}] belongs to more than one selectable option",
				reward_row_id
			);
			Clear();
			return false;
		}

		const auto reward_row = reward_rows.find(reward_row_id);
		if (reward_row == reward_rows.end()) {
			continue;
		}
		const auto reward_set_achievement = m_reward_set_achievements.find(reward_set_id);
		const auto option_index = reward_option_indices.find(
			std::make_pair(reward_set_id, option_id)
		);
		if (
			reward_set_achievement == m_reward_set_achievements.end() ||
			option_index == reward_option_indices.end()
		) {
			continue;
		}
		if (reward_row->second.achievement_id != reward_set_achievement->second) {
			LogError(
				"Achievement reward [{}] does not belong to reward set [{}]'s achievement",
				reward_row_id,
				reward_set_id
			);
			Clear();
			return false;
		}

		m_reward_sets[reward_set_achievement->second]
			.options[option_index->second]
			.rewards.push_back(reward_row->second);
	}

	for (const auto &[reward_row_id, reward] : reward_rows) {
		if (!mapped_reward_ids.contains(reward_row_id)) {
			m_rewards[reward.achievement_id].push_back(reward);
		}
	}

	// Client resources flag rewards but do not define them. Show View Reward
	// only when the server loaded reward content.
	for (auto &definition : m_definitions) {
		definition.reward_display = 0;
	}
	for (auto &[achievement_id, reward_set] : m_reward_sets) {
		bool has_selectable_option = false;
		for (const auto &option : reward_set.options) {
			if (option.rewards.empty()) {
				LogError(
					"Enabled achievement reward option [{}/{}] has no enabled rewards",
					reward_set.reward_set_id,
					option.option_id
				);
				Clear();
				return false;
			}
			has_selectable_option = has_selectable_option || !option.common_to_all;
		}
		if (reward_set.options.empty() || !has_selectable_option) {
			LogError(
				"Enabled achievement reward set [{}] has no selectable option",
				reward_set.reward_set_id
			);
			Clear();
			return false;
		}
		m_definitions[m_definition_indices[achievement_id]].reward_display = 1;
	}
	for (const auto &[achievement_id, rewards] : m_rewards) {
		if (!rewards.empty()) {
			m_definitions[m_definition_indices[achievement_id]].reward_display = 1;
		}
	}

	auto restriction_results = content_db.QueryDatabase(
		"SELECT r.restriction_id, r.achievement_id, r.requires_completed "
		"FROM achievement_cast_restrictions r "
		"INNER JOIN achievements a ON a.id = r.achievement_id AND a.enabled = 1 "
		"ORDER BY r.restriction_id, r.achievement_id"
	);
	if (!restriction_results.Success()) {
		LogError("Failed to load achievement spell restrictions");
		Clear();
		return false;
	}

	std::set<std::pair<uint32_t, uint32_t>> restriction_keys;
	for (auto row : restriction_results) {
		const auto restriction_id = ParseUInt32(row[0]);
		const auto achievement_id = ParseUInt32(row[1]);
		if (!restriction_keys.emplace(restriction_id, achievement_id).second) {
			LogError(
				"Duplicate achievement cast restriction [{}/{}]",
				restriction_id,
				achievement_id
			);
			Clear();
			return false;
		}
		m_cast_restrictions[restriction_id].push_back({
			achievement_id,
			ParseUInt32(row[2]) != 0
		});
	}

	try {
		auto definitions = SerializeDefinitions(m_categories, m_definitions);
		m_definition_packet = CompressDefinitions(definitions);
	}
	catch (const std::exception &error) {
		LogError("Failed to build achievement definition packet: {}", error.what());
		Clear();
		return false;
	}

	m_loaded = true;
	return true;
}

const EQ::Achievements::Definition *AchievementManager::FindDefinition(uint32_t achievement_id) const
{
	const auto index = FindDefinitionIndex(achievement_id);
	return index ? &m_definitions[*index] : nullptr;
}

std::optional<size_t> AchievementManager::FindDefinitionIndex(uint32_t achievement_id) const
{
	const auto definition = m_definition_indices.find(achievement_id);
	if (definition == m_definition_indices.end()) {
		return std::nullopt;
	}
	return definition->second;
}

std::optional<size_t> AchievementManager::FindComponentIndex(
	uint32_t achievement_id,
	uint8_t component_type,
	uint32_t component_id
) const
{
	const auto definition_index = FindDefinitionIndex(achievement_id);
	if (!definition_index || component_type > 3) {
		return std::nullopt;
	}

	const auto &components = m_definitions[*definition_index].components[component_type];
	for (size_t component_index = 0; component_index < components.size(); ++component_index) {
		if (components[component_index].component_id == component_id) {
			return component_index;
		}
	}
	return std::nullopt;
}

const std::vector<const AchievementCriterion *> &AchievementManager::Criteria(
	EQ::Achievements::EventType event_type
) const
{
	static const std::vector<const AchievementCriterion *> empty;
	const auto criteria = m_criteria_by_event.find(static_cast<uint8_t>(event_type));
	return criteria != m_criteria_by_event.end() ? criteria->second : empty;
}

const std::vector<const AchievementCriterion *> &AchievementManager::NpcNameKillCriteria(
	uint32_t name_identity
) const
{
	static const std::vector<const AchievementCriterion *> empty;
	const auto criteria = m_npc_name_kill_criteria.find(name_identity);
	return criteria != m_npc_name_kill_criteria.end() ? criteria->second : empty;
}

const std::vector<const AchievementCriterion *> &AchievementManager::CriteriaForAchievement(
	uint32_t achievement_id
) const
{
	static const std::vector<const AchievementCriterion *> empty;
	const auto criteria = m_criteria_by_achievement.find(achievement_id);
	return criteria != m_criteria_by_achievement.end() ? criteria->second : empty;
}

const std::vector<AchievementReward> &AchievementManager::Rewards(uint32_t achievement_id) const
{
	static const std::vector<AchievementReward> empty;
	const auto rewards = m_rewards.find(achievement_id);
	return rewards != m_rewards.end() ? rewards->second : empty;
}

const AchievementRewardSet *AchievementManager::RewardSet(uint32_t achievement_id) const
{
	const auto reward_set = m_reward_sets.find(achievement_id);
	return reward_set != m_reward_sets.end() ? &reward_set->second : nullptr;
}

const AchievementRewardSet *AchievementManager::FindRewardSet(uint32_t reward_set_id) const
{
	const auto achievement = m_reward_set_achievements.find(reward_set_id);
	return achievement != m_reward_set_achievements.end()
		? RewardSet(achievement->second)
		: nullptr;
}

const std::vector<AchievementCastRestriction> &AchievementManager::CastRestrictions(
	uint32_t restriction_id
) const
{
	static const std::vector<AchievementCastRestriction> empty;
	const auto restrictions = m_cast_restrictions.find(restriction_id);
	return restrictions != m_cast_restrictions.end() ? restrictions->second : empty;
}

std::optional<uint8_t> AchievementManager::RequiredClass(
	uint32_t achievement_id
) const
{
	const auto required_class = m_required_classes.find(achievement_id);
	if (required_class == m_required_classes.end()) {
		return std::nullopt;
	}
	return required_class->second;
}

bool AchievementManager::ResetOnVersionChange(uint32_t achievement_id) const
{
	const auto reset = m_reset_on_version_change.find(achievement_id);
	return reset != m_reset_on_version_change.end() && reset->second;
}
