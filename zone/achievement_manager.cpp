#include "zone/achievement_manager.h"

#include "common/classes.h"
#include "common/eqemu_logsys.h"
#include "common/repositories/achievement_associations_repository.h"
#include "common/repositories/achievement_cast_requirements_repository.h"
#include "common/repositories/achievement_categories_repository.h"
#include "common/repositories/achievement_category_associations_repository.h"
#include "common/repositories/achievement_components_repository.h"
#include "common/repositories/achievement_criteria_repository.h"
#include "common/repositories/achievements_repository.h"
#include "common/rulesys.h"
#include "common/skills.h"
#include "zone/reward_selection.h"
#include "zone/reward_selection_catalog.h"
#include "zone/zonedb.h"

#include <algorithm>
#include <limits>
#include <map>
#include <set>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

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
	m_cast_requirements.clear();
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
	swap(m_cast_requirements, other.m_cast_requirements);
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

	std::vector<AchievementCategoriesRepository::AchievementCategories> category_rows;
	if (!AchievementCategoriesRepository::GetAll(content_db, category_rows)) {
		LogError("Failed to load achievement categories");
		return false;
	}
	std::sort(category_rows.begin(), category_rows.end(), [](const auto &left, const auto &right) {
		return std::tie(left.parent_id, left.sequence, left.id) <
			std::tie(right.parent_id, right.sequence, right.id);
	});

	std::unordered_map<uint32_t, size_t> category_indices;
	for (const auto &row : category_rows) {
		Category category;
		category.category_id = row.id;
		if (category_indices.contains(category.category_id)) {
			LogError("Duplicate achievement category ID [{}]", category.category_id);
			Clear();
			return false;
		}
		category.parent_category_id = row.parent_id;
		category.display_order = row.sequence;
		category.name = row.name;
		category.description = row.description;
		category.icon = row.icon;
		category_indices[category.category_id] = m_categories.size();
		m_categories.emplace_back(std::move(category));
	}

	std::vector<AchievementsRepository::Achievements> definition_rows;
	if (!AchievementsRepository::GetAll(content_db, definition_rows)) {
		LogError("Failed to load achievement definitions");
		Clear();
		return false;
	}
	std::sort(definition_rows.begin(), definition_rows.end(), [](const auto &left, const auto &right) {
		return left.id < right.id;
	});

	for (const auto &row : definition_rows) {
		if (!row.enabled) {
			continue;
		}

		Definition definition;
		definition.achievement_id = row.id;
		if (m_definition_indices.contains(definition.achievement_id)) {
			LogError("Duplicate enabled achievement ID [{}]", definition.achievement_id);
			Clear();
			return false;
		}
		definition.name = row.name;
		definition.description = row.description;
		definition.icon_id = row.icon_id;
		definition.version = row.version;
		definition.points = row.points;
		definition.has_reward = row.has_reward != 0;
		m_reset_on_version_change[definition.achievement_id] = row.reset_on_version_change != 0;
		m_definition_indices[definition.achievement_id] = m_definitions.size();
		m_definitions.emplace_back(std::move(definition));
	}

	std::vector<AchievementCategoryAssociationsRepository::AchievementCategoryAssociations>
		association_rows;
	if (!AchievementCategoryAssociationsRepository::GetAll(content_db, association_rows)) {
		LogError("Failed to load achievement category associations");
		Clear();
		return false;
	}
	std::sort(association_rows.begin(), association_rows.end(), [](const auto &left, const auto &right) {
		return std::tie(left.category_id, left.sequence, left.achievement_id) <
			std::tie(right.category_id, right.sequence, right.achievement_id);
	});

	std::unordered_set<uint32_t> associated_achievement_ids;
	for (const auto &row : association_rows) {
		if (!m_definition_indices.contains(row.achievement_id)) {
			continue;
		}

		const auto category = category_indices.find(row.category_id);
		if (category == category_indices.end()) {
			continue;
		}

		m_categories[category->second].associations.push_back({
			row.achievement_id,
			row.display_text,
			row.sequence
		});
		associated_achievement_ids.insert(row.achievement_id);
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

	std::vector<AchievementComponentsRepository::AchievementComponents> component_rows;
	std::vector<AchievementAssociationsRepository::AchievementAssociations> component_count_rows;
	if (
		!AchievementComponentsRepository::GetAll(content_db, component_rows) ||
		!AchievementAssociationsRepository::GetAll(content_db, component_count_rows)
	) {
		LogError("Failed to load achievement components");
		Clear();
		return false;
	}
	std::sort(component_rows.begin(), component_rows.end(), [](const auto &left, const auto &right) {
		return std::tie(
			left.achievement_id,
			left.component_type,
			left.sequence,
			left.component_id
		) < std::tie(
			right.achievement_id,
			right.component_type,
			right.sequence,
			right.component_id
		);
	});

	std::unordered_map<uint32_t, uint32_t> component_counts;
	component_counts.reserve(component_count_rows.size());
	for (const auto &row : component_count_rows) {
		component_counts[row.component_id] = std::max<uint32_t>(row.required_count, 1);
	}

	for (const auto &row : component_rows) {
		const auto achievement_id = row.achievement_id;
		const auto definition = m_definition_indices.find(achievement_id);
		if (definition == m_definition_indices.end()) {
			continue;
		}

		const auto component_type = static_cast<uint32_t>(row.component_type);
		if (component_type > 3) {
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
				row.component_id
			)
		) {
			LogError(
				"Enabled achievement [{}] has duplicate component identity [{}/{}]",
				achievement_id,
				component_type,
				row.component_id
			);
			Clear();
			return false;
		}

		Component component;
		component.component_type = static_cast<uint8_t>(component_type);
		component.sequence = row.sequence;
		component.component_id = row.component_id;
		component.name = row.name;
		component.description = row.description;
		const auto required_count = component_counts.find(component.component_id);
		component.required_count = required_count != component_counts.end() ? required_count->second : 1;
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

	std::vector<AchievementCriteriaRepository::AchievementCriteria> criterion_rows;
	if (!AchievementCriteriaRepository::GetAll(content_db, criterion_rows)) {
		LogError("Failed to load achievement evaluation criteria");
		Clear();
		return false;
	}
	std::sort(criterion_rows.begin(), criterion_rows.end(), [](const auto &left, const auto &right) {
		return left.id < right.id;
	});

	for (const auto &row : criterion_rows) {
		if (!row.enabled || !m_definition_indices.contains(row.achievement_id)) {
			continue;
		}

		const auto achievement_id = row.achievement_id;
		const auto component_type = static_cast<uint32_t>(row.component_type);
		const auto event_type = static_cast<uint32_t>(row.event_type);
		const auto progress_mode = static_cast<uint32_t>(row.progress_mode);
		const auto behavior = static_cast<uint32_t>(row.behavior);
		const auto target_id = row.target_id;
		const auto target_id2 = row.target_id2;
		const auto target_value = row.target_value;
		if (
			component_type > 2 ||
			event_type > static_cast<uint32_t>(EventType::SkillCap) ||
			progress_mode > static_cast<uint32_t>(ProgressMode::Boolean) ||
			behavior > static_cast<uint32_t>(CriterionBehavior::Blocker) ||
			target_value < 0
		) {
			LogError(
				"Invalid enabled achievement criterion [{}]; RoF2 component type 3 is presentation-only",
				row.id
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
				row.id,
				target_id2
			);
			Clear();
			return false;
		}
		if (effective_event_type == EventType::NpcNameKill && !target_id) {
			LogError(
				"Achievement criterion [{}] has no usable NPC-name identity hash",
				row.id
			);
			Clear();
			return false;
		}
		if (effective_event_type == EventType::TaskComplete && !target_id) {
			LogError(
				"Achievement criterion [{}] must target a specific task so its "
				"one-time completion can be reconciled",
				row.id
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
				row.id,
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
				row.id,
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
				row.id,
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
		const auto replayed_zone_event =
			effective_event_type == EventType::ZoneEnter;
		if (
			effective_progress_mode == ProgressMode::Increment &&
			(
				absolute_fact_event ||
				replayed_specific_event ||
				replayed_achievement_event ||
				replayed_zone_event
			)
		) {
			LogError(
				"Achievement criterion [{}] uses increment mode for a reconciled "
				"absolute or one-time event",
				row.id
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
				row.id
			);
			Clear();
			return false;
		}

		const auto component_index = FindComponentIndex(
			achievement_id,
			static_cast<uint8_t>(component_type),
			row.component_id
		);
		const auto definition_index = FindDefinitionIndex(achievement_id);
		if (!component_index || !definition_index) {
			LogError(
				"Enabled achievement criterion [{}] references missing component [{}/{}/{}]",
				row.id,
				achievement_id,
				component_type,
				row.component_id
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
		const auto required_override = row.required_count;
		if (!required_override) {
			LogError(
				"Achievement criterion [{}] must have an explicit uint32 required count",
				row.id
			);
			Clear();
			return false;
		}
		const auto effective_required_count = required_override;
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
		criterion.criterion_id = row.id;
		criterion.achievement_id = achievement_id;
		criterion.component_type = static_cast<uint8_t>(component_type);
		criterion.component_id = row.component_id;
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

	std::unordered_set<uint64_t> active_reward_source_ids;
	active_reward_source_ids.reserve(m_definition_indices.size());
	for (const auto &definition : m_definitions) {
		active_reward_source_ids.insert(definition.achievement_id);
	}

	RewardSelectionCatalog reward_catalog;
	if (!LoadRewardSelectionCatalog(
		content_db,
		RewardSelectionSource::Achievement,
		active_reward_source_ids,
		reward_catalog
	)) {
		LogError("Failed to load the shared reward catalog for achievements");
		Clear();
		return false;
	}

	for (auto &[source_id, rewards] : reward_catalog.automatic) {
		if (
			source_id > std::numeric_limits<uint32_t>::max() ||
			!m_definition_indices.contains(static_cast<uint32_t>(source_id))
		) {
			LogError("Automatic rewards reference unknown achievement [{}]", source_id);
			Clear();
			return false;
		}
		m_rewards.emplace(static_cast<uint32_t>(source_id), std::move(rewards));
	}

	for (auto &[source_id, selection_set] : reward_catalog.selectable) {
		if (
			source_id > std::numeric_limits<uint32_t>::max() ||
			!m_definition_indices.contains(static_cast<uint32_t>(source_id))
		) {
			LogError("Selectable rewards reference unknown achievement [{}]", source_id);
			Clear();
			return false;
		}

		const auto achievement_id = static_cast<uint32_t>(source_id);
		if (selection_set.title.empty()) {
			selection_set.title =
				m_definitions[m_definition_indices[achievement_id]].name;
		}
		if (!m_reward_sets.emplace(achievement_id, std::move(selection_set)).second) {
			LogError("Achievement [{}] has more than one selectable reward set", achievement_id);
			Clear();
			return false;
		}
	}

	for (auto &definition : m_definitions) {
		definition.has_reward =
			m_rewards.contains(definition.achievement_id) ||
			m_reward_sets.contains(definition.achievement_id);
	}

	std::vector<AchievementCastRequirementsRepository::AchievementCastRequirements>
		restriction_rows;
	if (!AchievementCastRequirementsRepository::GetAll(content_db, restriction_rows)) {
		LogError("Failed to load achievement spell restrictions");
		Clear();
		return false;
	}
	std::sort(restriction_rows.begin(), restriction_rows.end(), [](const auto &left, const auto &right) {
		return std::tie(left.restriction_id, left.achievement_id) <
			std::tie(right.restriction_id, right.achievement_id);
	});

	std::set<std::pair<uint32_t, uint32_t>> restriction_keys;
	for (const auto &row : restriction_rows) {
		if (!m_definition_indices.contains(row.achievement_id)) {
			continue;
		}

		const auto restriction_id = row.restriction_id;
		const auto achievement_id = row.achievement_id;
		if (!restriction_keys.emplace(restriction_id, achievement_id).second) {
			LogError(
				"Duplicate achievement cast restriction [{}/{}]",
				restriction_id,
				achievement_id
			);
			Clear();
			return false;
		}
		m_cast_requirements[restriction_id].push_back({
			achievement_id,
			row.requires_completed != 0
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

const std::vector<RewardSelectionReward> &AchievementManager::Rewards(
	uint32_t achievement_id
) const
{
	static const std::vector<RewardSelectionReward> empty;
	const auto rewards = m_rewards.find(achievement_id);
	return rewards != m_rewards.end() ? rewards->second : empty;
}

const RewardSelectionSet *AchievementManager::RewardSet(
	uint32_t achievement_id
) const
{
	const auto reward_set = m_reward_sets.find(achievement_id);
	return reward_set != m_reward_sets.end() ? &reward_set->second : nullptr;
}

const std::vector<AchievementCastRequirement> &AchievementManager::CastRequirements(
	uint32_t restriction_id
) const
{
	static const std::vector<AchievementCastRequirement> empty;
	const auto requirements = m_cast_requirements.find(restriction_id);
	return requirements != m_cast_requirements.end() ? requirements->second : empty;
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
