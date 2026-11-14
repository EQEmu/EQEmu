#include "zone/client_achievements.h"

#include "common/achievement_state_updates.h"
#include "common/eq_packet.h"
#include "common/eqemu_logsys.h"
#include "common/rulesys.h"
#include "zone/achievement_manager.h"
#include "zone/client.h"
#include "zone/guild_mgr.h"
#include "zone/zone.h"
#include "zone/zonedb.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <fmt/format.h>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <tuple>
#include <unordered_map>
#include <utility>

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

class AchievementStateUpdateCharacterLock
{
public:
	explicit AchievementStateUpdateCharacterLock(uint32_t character_id)
		: m_character_id(character_id)
	{
	}

	~AchievementStateUpdateCharacterLock()
	{
		if (m_acquired) {
			database.QueryDatabase(fmt::format(
				"SELECT RELEASE_LOCK('eqemu_achievement_state_update_{}')",
				m_character_id
			));
		}
	}

	bool TryAcquire()
	{
		auto result = database.QueryDatabase(fmt::format(
			"SELECT GET_LOCK('eqemu_achievement_state_update_{}', 0)",
			m_character_id
		));
		if (!result.Success() || result.RowCount() != 1) {
			return false;
		}

		auto row = result.begin();
		m_acquired = ParseUInt32(row[0]) == 1;
		return m_acquired;
	}

	AchievementStateUpdateCharacterLock(const AchievementStateUpdateCharacterLock &) = delete;
	AchievementStateUpdateCharacterLock &operator=(const AchievementStateUpdateCharacterLock &) = delete;

private:
	uint32_t m_character_id;
	bool m_acquired = false;
};

uint32_t ClampCount(uint64_t value, uint32_t required_count)
{
	return static_cast<uint32_t>(std::min<uint64_t>(
		value,
		std::max<uint32_t>(required_count, 1)
	));
}

std::optional<RewardSelectionSession> BuildAchievementRewardSelectionSession(
	const EQ::Achievements::Definition &definition,
	const RewardSelectionSet &reward_set,
	RewardSelectionChannel channel,
	uint32_t selected_option_id = 0
)
{
	RewardSelectionSession session;
	session.source.source = RewardSelectionSource::Achievement;
	session.source.source_id = definition.achievement_id;
	session.channel = channel;
	session.pending_reward_id = definition.achievement_id;
	session.reward_set.reward_set_id = reward_set.reward_set_id;
	session.reward_set.title = reward_set.title;
	session.reward_set.options.reserve(reward_set.options.size());

	bool found_selected_option = selected_option_id == 0;
	for (const auto &option : reward_set.options) {
		if (
			selected_option_id &&
			!option.common_to_all &&
			option.option_id != selected_option_id
		) {
			continue;
		}

		RewardSelectionOption selection_option;
		selection_option.option_id = option.option_id;
		selection_option.sequence = option.sequence;
		selection_option.label = option.label;
		selection_option.common_to_all = option.common_to_all;
		selection_option.flags = option.flags;
		selection_option.rewards = option.rewards;
		session.reward_set.options.emplace_back(std::move(selection_option));

		if (!option.common_to_all && option.option_id == selected_option_id) {
			found_selected_option = true;
		}
	}

	if (!found_selected_option || session.reward_set.options.empty()) {
		return std::nullopt;
	}
	return session;
}

struct PersistedAchievementRewardSelection {
	uint32_t reward_set_id = 0;
	uint32_t selected_option_id = 0;
	uint32_t status = 0;
};

const PersistedAchievementRewardSelection *FindPersistedSelection(
	const std::vector<PersistedAchievementRewardSelection> &selections,
	uint32_t reward_set_id
)
{
	const auto found = std::find_if(
		selections.begin(),
		selections.end(),
		[reward_set_id](const PersistedAchievementRewardSelection &selection) {
			return selection.reward_set_id == reward_set_id;
		}
	);
	return found != selections.end() ? &*found : nullptr;
}

bool ResolvePendingOption(
	const PersistedAchievementRewardSelection *selection,
	uint32_t &selected_option_id
)
{
	selected_option_id = 0;
	if (!selection) {
		return true;
	}

	const bool unselected_pending =
		selection->status == 0 && !selection->selected_option_id;
	const bool retryable =
		selection->status == 2 && selection->selected_option_id;
	if (!unselected_pending && !retryable) {
		return false;
	}

	selected_option_id = retryable ? selection->selected_option_id : 0;
	return true;
}

static_assert(
	static_cast<uint8_t>(RewardSelectionDeliveryResult::Delivered) == 0 &&
	static_cast<uint8_t>(RewardSelectionDeliveryResult::RetryableFailure) == 1 &&
	static_cast<uint8_t>(RewardSelectionDeliveryResult::Ambiguous) == 2
);

bool GetOwnedItemCounts(
	Client &client,
	std::unordered_map<uint32_t, uint64_t> &counts
)
{
	struct DurableItemRow {
		bool shared_bank = false;
		int16_t slot_id = EQ::invslot::SLOT_INVALID;
		uint32_t item_id = 0;
		uint64_t charges = 0;
		std::array<uint32_t, 6> augment_ids{};
	};

	counts.clear();
	auto inventory_results = database.QueryDatabase(fmt::format(
		"SELECT slot_id, item_id, charges, augment_one, augment_two, augment_three, "
		"augment_four, augment_five, augment_six FROM inventory "
		"WHERE character_id = {} AND ("
		"slot_id BETWEEN {} AND {} OR slot_id BETWEEN {} AND {} OR "
		"slot_id BETWEEN {} AND {} OR slot_id BETWEEN {} AND {} OR "
		"slot_id BETWEEN {} AND {})",
		client.CharacterID(),
		EQ::invslot::POSSESSIONS_BEGIN,
		EQ::invslot::POSSESSIONS_END,
		EQ::invbag::GENERAL_BAGS_BEGIN,
		EQ::invbag::GENERAL_BAGS_END,
		EQ::invbag::CURSOR_BAG_BEGIN,
		EQ::invbag::CURSOR_BAG_END,
		EQ::invslot::BANK_BEGIN,
		EQ::invslot::BANK_END,
		EQ::invbag::BANK_BAGS_BEGIN,
		EQ::invbag::BANK_BAGS_END
	));
	auto shared_bank_results = database.QueryDatabase(fmt::format(
		"SELECT slot_id, item_id, charges, augment_one, augment_two, augment_three, "
		"augment_four, augment_five, augment_six FROM sharedbank "
		"WHERE account_id = {} AND (slot_id BETWEEN {} AND {} OR "
		"slot_id BETWEEN {} AND {})",
		client.AccountID(),
		EQ::invslot::SHARED_BANK_BEGIN,
		EQ::invslot::SHARED_BANK_END,
		EQ::invbag::SHARED_BANK_BAGS_BEGIN,
		EQ::invbag::SHARED_BANK_BAGS_END
	));
	auto keyring_results = database.QueryDatabase(fmt::format(
		"SELECT item_id FROM keyring WHERE char_id = {}",
		client.CharacterID()
	));
	if (
		!inventory_results.Success() ||
		!shared_bank_results.Success() ||
		!keyring_results.Success()
	) {
		LogError(
			"Could not reconcile durable owned-item achievements for character [{}]",
			client.CharacterID()
		);
		return false;
	}

	std::vector<DurableItemRow> rows;
	const auto append_rows = [&rows](auto &results, bool shared_bank) {
		for (auto row : results) {
			DurableItemRow item_row;
			item_row.shared_bank = shared_bank;
			item_row.slot_id = static_cast<int16_t>(ParseUInt32(row[0]));
			item_row.item_id = ParseUInt32(row[1]);
			item_row.charges = ParseUInt64(row[2]);
			for (size_t augment_index = 0; augment_index < item_row.augment_ids.size(); ++augment_index) {
				item_row.augment_ids[augment_index] = ParseUInt32(row[augment_index + 3]);
			}
			rows.emplace_back(item_row);
		}
	};
	append_rows(inventory_results, false);
	append_rows(shared_bank_results, true);

	std::unordered_map<int16_t, size_t> inventory_rows;
	std::unordered_map<int16_t, size_t> shared_bank_rows;
	for (size_t row_index = 0; row_index < rows.size(); ++row_index) {
		auto &slot_rows = rows[row_index].shared_bank ? shared_bank_rows : inventory_rows;
		slot_rows[rows[row_index].slot_id] = row_index;
	}

	const auto *inventory_lookup = client.GetInv().GetLookup();
	const auto is_valid_row = [
		&rows,
		&inventory_rows,
		&shared_bank_rows,
		inventory_lookup
	](const DurableItemRow &row) {
		if (
			EQ::ValueWithin(
				row.slot_id,
				EQ::invslot::POSSESSIONS_BEGIN,
				EQ::invslot::POSSESSIONS_END
			)
		) {
			return
				inventory_lookup &&
				((static_cast<uint64_t>(1) << row.slot_id) &
					inventory_lookup->PossessionsBitmask) != 0;
		}
		if (
			EQ::ValueWithin(
				row.slot_id,
				EQ::invslot::BANK_BEGIN,
				EQ::invslot::BANK_END
			)
		) {
			return
				inventory_lookup &&
				(row.slot_id - EQ::invslot::BANK_BEGIN) <
					inventory_lookup->InventoryTypeSize.Bank;
		}

		const auto general_bag = EQ::ValueWithin(
			row.slot_id,
			EQ::invbag::GENERAL_BAGS_BEGIN,
			EQ::invbag::GENERAL_BAGS_END
		);
		const auto bank_bag = EQ::ValueWithin(
			row.slot_id,
			EQ::invbag::BANK_BAGS_BEGIN,
			EQ::invbag::BANK_BAGS_END
		);
		const auto shared_bag = EQ::ValueWithin(
			row.slot_id,
			EQ::invbag::SHARED_BANK_BAGS_BEGIN,
			EQ::invbag::SHARED_BANK_BAGS_END
		);
		if (!general_bag && !bank_bag && !shared_bag) {
			// Cursor-range rows are durable cursor-queue entries in the loader.
			return true;
		}

		const auto parent_slot = EQ::InventoryProfile::CalcSlotId(row.slot_id);
		if (
			general_bag &&
			(
				!inventory_lookup ||
				((static_cast<uint64_t>(1) << parent_slot) &
					inventory_lookup->PossessionsBitmask) == 0
			)
		) {
			return false;
		}
		if (
			bank_bag &&
			(
				!inventory_lookup ||
				(parent_slot - EQ::invslot::BANK_BEGIN) >=
					inventory_lookup->InventoryTypeSize.Bank
			)
		) {
			return false;
		}

		const auto &slot_rows = row.shared_bank ? shared_bank_rows : inventory_rows;
		const auto parent = slot_rows.find(parent_slot);
		if (parent == slot_rows.end()) {
			return false;
		}
		const auto *parent_item = database.GetItem(rows[parent->second].item_id);
		return
			parent_item &&
			parent_item->IsClassBag() &&
			EQ::InventoryProfile::CalcBagIdx(row.slot_id) <
				std::min<uint8_t>(parent_item->BagSlots, EQ::invbag::SLOT_COUNT);
	};

	for (const auto &row : rows) {
		const auto *item = database.GetItem(row.item_id);
		if (!item || !is_valid_row(row)) {
			continue;
		}

		if (item->Stackable) {
			if (
				row.charges == 0 ||
				row.charges > static_cast<uint64_t>(std::numeric_limits<int16_t>::max())
			) {
				++counts[row.item_id];
			}
			else {
				counts[row.item_id] += row.charges;
			}
		}
		else {
			++counts[row.item_id];
		}

		if (!item->IsClassCommon()) {
			continue;
		}
		for (const auto augment_id : row.augment_ids) {
			if (augment_id && database.GetItem(augment_id)) {
				++counts[augment_id];
			}
		}
	}

	// A keyring entry counts as one retained copy, independent of inventory.
	for (auto row : keyring_results) {
		const auto item_id = ParseUInt32(row[0]);
		if (item_id && database.GetItem(item_id)) {
			counts[item_id] = std::max<uint64_t>(counts[item_id], 1);
		}
	}

	return true;
}

} // namespace

ClientAchievementState::ClientAchievementState(Client &client)
	: m_client(client)
{
}

void ClientAchievementState::InitializeStates()
{
	const auto &definitions = AchievementManager::Instance().Definitions();
	m_states.clear();
	m_states.resize(definitions.size());
	for (size_t definition_index = 0; definition_index < definitions.size(); ++definition_index) {
		for (uint8_t component_type = 0; component_type < 4; ++component_type) {
			const auto component_count = definitions[definition_index].components[component_type].size();
			m_states[definition_index].satisfied[component_type].resize(component_count);
			m_states[definition_index].counts[component_type].resize(component_count);
		}
	}
}

bool ClientAchievementState::Load(bool allow_disabled)
{
	auto &manager = AchievementManager::Instance();
	if (!manager.IsLoaded()) {
		return false;
	}

	if (!RuleB(Achievements, EnableAchievements)) {
		if (!allow_disabled) {
			return false;
		}

		InitializeStates();
		m_loaded = true;
		return true;
	}

	const auto character_id = m_client.CharacterID();
	AchievementStateUpdateCharacterLock state_update_lock(character_id);
	if (!state_update_lock.TryAcquire()) {
		m_client.NotifyAchievementStateUpdatePending();
		return false;
	}

	InitializeStates();
	std::unordered_set<uint32_t> stale_achievements;

	auto completion_results = database.QueryDatabase(fmt::format(
		"SELECT achievement_id, `version`, completed_at "
		"FROM character_achievements WHERE character_id = {}",
		character_id
	));
	if (!completion_results.Success()) {
		LogError("Failed to load achievements for character [{}]", character_id);
		return false;
	}

	for (auto row : completion_results) {
		const auto achievement_id = ParseUInt32(row[0]);
		const auto definition_index = manager.FindDefinitionIndex(achievement_id);
		if (!definition_index) {
			continue;
		}

		const auto &definition = manager.Definitions()[*definition_index];
		if (
			manager.ResetOnVersionChange(achievement_id) &&
			ParseUInt32(row[1]) != definition.version
		) {
			stale_achievements.insert(achievement_id);
			continue;
		}

		m_states[*definition_index].status = EQ::Achievements::Status::Completed;
		m_states[*definition_index].completion_timestamp = ParseUInt32(row[2]);
	}

	auto progress_results = database.QueryDatabase(fmt::format(
		"SELECT achievement_id, component_type, component_sequence, component_id, "
		"current_count, `version` "
		"FROM character_achievement_progress WHERE character_id = {}",
		character_id
	));
	if (!progress_results.Success()) {
		LogError("Failed to load achievement progress for character [{}]", character_id);
		return false;
	}

	for (auto row : progress_results) {
		const auto achievement_id = ParseUInt32(row[0]);
		const auto component_type = ParseUInt32(row[1]);
		if (component_type > 2) {
			// Component type 3 is presentation-only.
			continue;
		}
		const auto definition_index = manager.FindDefinitionIndex(achievement_id);
		const auto component_index = manager.FindComponentIndex(
			achievement_id,
			static_cast<uint8_t>(component_type),
			ParseUInt32(row[3])
		);
		if (!definition_index || !component_index) {
			continue;
		}

		const auto &definition = manager.Definitions()[*definition_index];
		if (
			manager.ResetOnVersionChange(achievement_id) &&
			ParseUInt32(row[5]) != definition.version
		) {
			stale_achievements.insert(achievement_id);
			continue;
		}

		const auto &component = definition.components[component_type][*component_index];
		// Do not attach an old row to a component ID changed without a version bump.
		if (ParseUInt32(row[3]) != component.component_id) {
			continue;
		}

		const auto count = ClampCount(ParseUInt64(row[4]), component.required_count);
		m_states[*definition_index].counts[component_type][*component_index] = count;
		m_states[*definition_index].satisfied[component_type][*component_index] =
			count >= component.required_count;
	}

	for (const auto achievement_id : stale_achievements) {
		// Delete completion last so a partial reset remains discoverable and can
		// be retried on the next zone entry.
		const auto selection_delete = database.QueryDatabase(fmt::format(
			"DELETE FROM character_achievement_reward_selections "
			"WHERE character_id = {} AND achievement_id = {}",
			character_id,
			achievement_id
		));
		if (!selection_delete.Success()) {
			LogError(
				"Failed to reset selectable reward ledger for stale achievement "
				"[{}], character [{}]",
				achievement_id,
				character_id
			);
			return false;
		}

		const auto reward_delete = database.QueryDatabase(fmt::format(
			"DELETE FROM character_achievement_rewards "
			"WHERE character_id = {} AND achievement_id = {}",
			character_id,
			achievement_id
		));
		if (!reward_delete.Success()) {
			LogError(
				"Failed to reset reward ledger for stale achievement [{}], character [{}]",
				achievement_id,
				character_id
			);
			return false;
		}

		const auto progress_delete = database.QueryDatabase(fmt::format(
			"DELETE FROM character_achievement_progress "
			"WHERE character_id = {} AND achievement_id = {}",
			character_id,
			achievement_id
		));
		if (!progress_delete.Success()) {
			LogError(
				"Failed to reset progress for stale achievement [{}], character [{}]",
				achievement_id,
				character_id
			);
			return false;
		}

		const auto completion_delete = database.QueryDatabase(fmt::format(
			"DELETE FROM character_achievements "
			"WHERE character_id = {} AND achievement_id = {}",
			character_id,
			achievement_id
		));
		if (!completion_delete.Success()) {
			LogError(
				"Failed to reset stale achievement [{}] for character [{}]",
				achievement_id,
				character_id
			);
			return false;
		}

		// A stale row can invalidate progress already loaded for this definition.
		// Reset memory to match the durable state before reconciliation.
		if (const auto definition_index = manager.FindDefinitionIndex(achievement_id)) {
			auto &state = m_states[*definition_index];
			state.status = EQ::Achievements::Status::Open;
			state.completion_timestamp = 0;
			for (uint8_t component_type = 0; component_type < 4; ++component_type) {
				std::fill(
					state.satisfied[component_type].begin(),
					state.satisfied[component_type].end(),
					0
				);
				std::fill(
					state.counts[component_type].begin(),
					state.counts[component_type].end(),
					0
				);
			}
		}
	}

	m_loaded = true;
	if (!DrainPendingStateUpdatesLocked(true)) {
		LogError(
			"Failed to drain one or more pending achievement state updates for character [{}]",
			character_id
		);
	}
	if (RuleB(Achievements, GrantRewards)) {
		for (size_t definition_index = 0; definition_index < m_states.size(); ++definition_index) {
			if (m_states[definition_index].status == EQ::Achievements::Status::Completed) {
				QueueRewards(manager.Definitions()[definition_index].achievement_id);
			}
		}
	}
	if (!Reconcile()) {
		LogError(
			"Failed to reconcile authoritative achievement facts for character [{}]",
			character_id
		);
		m_loaded = false;
		return false;
	}
	EvaluateAll(false, true);
	return true;
}

bool ClientAchievementState::DrainPendingStateUpdates(bool retry_blocked)
{
	if (!m_loaded) {
		return false;
	}
	if (!m_client.Connected() || m_client.IsLD() || m_client.IsZoning()) {
		m_client.NotifyAchievementStateUpdatePending();
		return true;
	}

	AchievementStateUpdateCharacterLock state_update_lock(m_client.CharacterID());
	if (!state_update_lock.TryAcquire()) {
		m_client.NotifyAchievementStateUpdatePending();
		return true;
	}

	return DrainPendingStateUpdatesLocked(retry_blocked);
}

bool ClientAchievementState::DrainPendingStateUpdatesLocked(bool retry_blocked)
{
	using AchievementStateUpdates::Operation;
	using AchievementStateUpdates::Status;

	if (!m_loaded) {
		return false;
	}

	const auto character_id = m_client.CharacterID();
	if (retry_blocked) {
		const auto reset = database.QueryDatabase(fmt::format(
			"UPDATE character_achievement_pending_updates "
			"SET status = {}, last_attempt_at = 0, last_error = '' "
			"WHERE character_id = {} AND status = {}",
			static_cast<uint32_t>(Status::Pending),
			character_id,
			static_cast<uint32_t>(Status::Blocked)
		));
		if (!reset.Success()) {
			return false;
		}
	}

	constexpr uint32_t batch_size = 64;
	auto results = database.QueryDatabase(fmt::format(
		"SELECT update_id, operation, achievement_id, component_type, "
		"component_id, requested_value, `version`, status, attempt_count "
		"FROM character_achievement_pending_updates "
		"WHERE character_id = {} AND (status = {} "
		"OR (status = {} AND last_attempt_at + {} <= UNIX_TIMESTAMP())) "
		"ORDER BY update_id LIMIT {}",
		character_id,
		static_cast<uint32_t>(Status::Pending),
		static_cast<uint32_t>(Status::Processing),
		AchievementStateUpdates::ProcessingLeaseSeconds,
		batch_size
	));
	if (!results.Success()) {
		return false;
	}
	if (results.RowCount() == batch_size) {
		m_client.NotifyAchievementStateUpdatePending();
	}

	auto set_status = [character_id](
		uint64_t update_id,
		Status status,
		const std::string &last_error,
		uint32_t claim_token
	) {
		const auto result = database.QueryDatabase(fmt::format(
			"UPDATE character_achievement_pending_updates "
			"SET status = {}, last_error = '{}' "
			"WHERE update_id = {} AND character_id = {} "
			"AND status = {} AND attempt_count = {}",
			static_cast<uint32_t>(status),
			database.Escape(last_error),
			update_id,
			character_id,
			static_cast<uint32_t>(Status::Processing),
			claim_token
		));
		return result.Success() && result.RowsAffected() == 1;
	};

	bool succeeded = true;
	auto &manager = AchievementManager::Instance();
	for (auto row : results) {
		const auto update_id = ParseUInt64(row[0]);
		const auto operation_value = ParseUInt32(row[1]);
		const auto achievement_id = ParseUInt32(row[2]);
		const auto component_type = ParseUInt32(row[3]);
		const auto component_id = ParseUInt32(row[4]);
		const auto requested_value = ParseUInt32(row[5]);
		const auto version = ParseUInt32(row[6]);
		const auto status_value = ParseUInt32(row[7]);
		const auto previous_attempt_count = ParseUInt32(row[8]);

		if (status_value > static_cast<uint32_t>(Status::Processing)) {
			succeeded = false;
			continue;
		}

		const auto claim_token = previous_attempt_count + 1;
		const auto attempt = database.QueryDatabase(fmt::format(
			"UPDATE character_achievement_pending_updates "
			"SET status = {}, attempt_count = attempt_count + 1, "
			"last_attempt_at = UNIX_TIMESTAMP(), last_error = '' "
			"WHERE update_id = {} AND character_id = {} "
			"AND status = {} AND attempt_count = {}{}",
			static_cast<uint32_t>(Status::Processing),
			update_id,
			character_id,
			status_value,
			previous_attempt_count,
			status_value == static_cast<uint32_t>(Status::Processing)
				? fmt::format(
					" AND last_attempt_at + {} <= UNIX_TIMESTAMP()",
					AchievementStateUpdates::ProcessingLeaseSeconds
				)
				: ""
		));
		if (!attempt.Success()) {
			succeeded = false;
			break;
		}
		if (attempt.RowsAffected() == 0) {
			continue;
		}

		if (operation_value > static_cast<uint32_t>(Operation::Complete)) {
			succeeded =
				set_status(
					update_id,
					Status::Blocked,
					"invalid operation",
					claim_token
				) &&
				succeeded;
			continue;
		}

		const auto definition_index = manager.FindDefinitionIndex(achievement_id);
		if (!definition_index) {
			succeeded =
				set_status(
					update_id,
					Status::Blocked,
					"achievement definition is disabled or unavailable",
					claim_token
				) &&
				succeeded;
			continue;
		}

		const auto &definition = manager.Definitions()[*definition_index];
		if (version != definition.version) {
			succeeded =
				set_status(
					update_id,
					Status::Blocked,
					fmt::format(
						"definition version {} does not match loaded version {}",
						version,
						definition.version
					),
					claim_token
				) &&
				succeeded;
			continue;
		}

		auto &state = m_states[*definition_index];
		auto completion_results = database.QueryDatabase(fmt::format(
			"SELECT `version`, completed_at "
			"FROM character_achievements "
			"WHERE character_id = {} AND achievement_id = {} LIMIT 1",
			character_id,
			achievement_id
		));
		if (!completion_results.Success()) {
			succeeded =
				set_status(
					update_id,
					Status::Pending,
					"failed to refresh durable achievement completion",
					claim_token
				) &&
				succeeded;
			continue;
		}

		bool durable_completed = false;
		if (completion_results.RowCount() == 1) {
			auto completion_row = completion_results.begin();
			const auto durable_version = ParseUInt32(completion_row[0]);
			if (
				manager.ResetOnVersionChange(achievement_id) &&
				durable_version != definition.version
			) {
				succeeded =
					set_status(
						update_id,
						Status::Blocked,
						fmt::format(
							"durable completion version {} does not match loaded version {}",
							durable_version,
							definition.version
						),
						claim_token
					) &&
					succeeded;
				continue;
			}

			durable_completed = true;
			const auto completed_at = ParseUInt32(completion_row[1]);
			const auto required_class = manager.RequiredClass(achievement_id);
			const auto refreshed_status =
				required_class && m_client.GetClass() != *required_class
					? EQ::Achievements::Status::Hidden
					: EQ::Achievements::Status::Completed;
			const auto completion_changed =
				state.status != refreshed_status ||
				state.completion_timestamp != completed_at;
			state.status = refreshed_status;
			state.completion_timestamp = completed_at;
			if (completion_changed && m_initial_sent) {
				SendStateUpdate(*definition_index);
			}
			if (
				completion_changed &&
				RuleB(Achievements, GrantRewards)
			) {
				QueueRewards(achievement_id);
			}
		}
		else if (
			state.status == EQ::Achievements::Status::Completed ||
			state.completion_timestamp
		) {
			const auto required_class = manager.RequiredClass(achievement_id);
			state.status =
				required_class && m_client.GetClass() != *required_class
					? EQ::Achievements::Status::Hidden
					: EQ::Achievements::Status::Open;
			state.completion_timestamp = 0;
			if (m_initial_sent) {
				SendStateUpdate(*definition_index);
			}
		}

		const auto operation = static_cast<Operation>(operation_value);
		bool applied = false;
		if (operation == Operation::Advance) {
			if (
				component_type > 2 ||
				!requested_value
			) {
				succeeded =
					set_status(
						update_id,
						Status::Blocked,
						"invalid progress component or requested value",
						claim_token
					) &&
					succeeded;
				continue;
			}

			const auto component_index = manager.FindComponentIndex(
				achievement_id,
				static_cast<uint8_t>(component_type),
				component_id
			);
			if (!component_index) {
				succeeded =
					set_status(
						update_id,
						Status::Blocked,
						"achievement component is unavailable",
						claim_token
					) &&
					succeeded;
				continue;
			}

			const auto &component =
				definition.components[component_type][*component_index];
			auto progress_results = database.QueryDatabase(fmt::format(
				"SELECT component_type, component_id, current_count, `version` "
				"FROM character_achievement_progress "
				"WHERE character_id = {} AND achievement_id = {}",
				character_id,
				achievement_id
			));
			if (!progress_results.Success()) {
				succeeded =
					set_status(
						update_id,
						Status::Pending,
						"failed to refresh durable achievement progress",
						claim_token
					) &&
					succeeded;
				continue;
			}

			const auto previous_counts = state.counts;
			auto refreshed_counts = state.counts;
			auto refreshed_satisfied = state.satisfied;
			for (uint8_t refresh_type = 0; refresh_type < 3; ++refresh_type) {
				std::fill(
					refreshed_counts[refresh_type].begin(),
					refreshed_counts[refresh_type].end(),
					0
				);
				std::fill(
					refreshed_satisfied[refresh_type].begin(),
					refreshed_satisfied[refresh_type].end(),
					0
				);
			}

			bool progress_is_compatible = true;
			for (auto progress_row : progress_results) {
				const auto row_component_type = ParseUInt32(progress_row[0]);
				if (row_component_type > 2) {
					continue;
				}

				const auto row_component_id = ParseUInt32(progress_row[1]);
				const auto row_component_index = manager.FindComponentIndex(
					achievement_id,
					static_cast<uint8_t>(row_component_type),
					row_component_id
				);
				if (!row_component_index) {
					continue;
				}

				const auto durable_version = ParseUInt32(progress_row[3]);
				if (
					manager.ResetOnVersionChange(achievement_id) &&
					durable_version != definition.version
				) {
					succeeded =
						set_status(
							update_id,
							Status::Blocked,
							fmt::format(
								"durable progress version {} does not match loaded version {}",
								durable_version,
								definition.version
							),
							claim_token
						) &&
						succeeded;
					progress_is_compatible = false;
					break;
				}

				const auto &row_component =
					definition.components[row_component_type][*row_component_index];
				const auto durable_count = ClampCount(
					ParseUInt64(progress_row[2]),
					row_component.required_count
				);
				refreshed_counts[row_component_type][*row_component_index] = durable_count;
				refreshed_satisfied[row_component_type][*row_component_index] =
					durable_count >= row_component.required_count;
			}
			if (!progress_is_compatible) {
				continue;
			}

			state.counts = std::move(refreshed_counts);
			state.satisfied = std::move(refreshed_satisfied);
			std::vector<EQ::Achievements::ProgressUpdate> refreshed_progress_updates;
			for (uint8_t refresh_type = 0; refresh_type < 3; ++refresh_type) {
				for (
					size_t refresh_index = 0;
					refresh_index < state.counts[refresh_type].size();
					++refresh_index
				) {
					if (
						previous_counts[refresh_type][refresh_index] ==
							state.counts[refresh_type][refresh_index] ||
						(
							refresh_type == component_type &&
							refresh_index == *component_index
						)
					) {
						continue;
					}

					const auto &refreshed_component =
						definition.components[refresh_type][refresh_index];
					refreshed_progress_updates.push_back({
						achievement_id,
						refreshed_component.component_id,
						refreshed_component.sequence,
						refresh_type,
						state.counts[refresh_type][refresh_index]
					});
				}
			}
			if (m_initial_sent && !refreshed_progress_updates.empty()) {
				SendProgressUpdates(refreshed_progress_updates);
			}

			const auto durable_count = state.counts[component_type][*component_index];
			const auto progress_changed =
				previous_counts[component_type][*component_index] != durable_count;

			applied = durable_count >= requested_value;
			if (applied) {
				if (progress_changed && m_initial_sent) {
					SendProgressUpdates({{
						achievement_id,
						component.component_id,
						component.sequence,
						static_cast<uint8_t>(component_type),
						durable_count
					}});
				}
				if (!durable_completed) {
					bool persistence_succeeded = true;
					const auto status_changed = EvaluateDefinition(
						*definition_index,
						true,
						&persistence_succeeded
					);
					if (
						status_changed &&
						m_initial_sent &&
						state.status != EQ::Achievements::Status::Completed
					) {
						SendStateUpdate(*definition_index);
					}
					applied = persistence_succeeded;
				}
			}
			else {
				applied = SetProgress(
					achievement_id,
					static_cast<uint8_t>(component_type),
					component_id,
					requested_value,
					false,
					true
				);
			}
		}
		else {
			if (component_type || component_id || requested_value) {
				succeeded =
					set_status(
						update_id,
						Status::Blocked,
						"completion state update contains progress fields",
						claim_token
					) &&
					succeeded;
				continue;
			}
			applied = durable_completed;
			if (!applied) {
				applied = Complete(achievement_id, true);
			}
		}

		if (!applied) {
			succeeded =
				set_status(
					update_id,
					Status::Pending,
					"achievement persistence did not complete",
					claim_token
				) &&
				succeeded;
			continue;
		}

		const auto removed = database.QueryDatabase(fmt::format(
			"DELETE FROM character_achievement_pending_updates "
			"WHERE update_id = {} AND character_id = {} "
			"AND status = {} AND attempt_count = {}",
			update_id,
			character_id,
			static_cast<uint32_t>(Status::Processing),
			claim_token
		));
		if (!removed.Success() || removed.RowsAffected() == 0) {
			succeeded = false;
		}
	}

	if (!succeeded) {
		m_client.NotifyAchievementStateUpdatePending();
	}
	return succeeded;
}

bool ClientAchievementState::Reconcile()
{
	using EQ::Achievements::EventType;

	bool succeeded = true;
	succeeded =
		ProcessEvent(EventType::Level, 0, 0, m_client.GetLevel(), false) &&
		succeeded;
	succeeded =
		ProcessEvent(EventType::ZoneEnter, m_client.GetZoneID(), 0, 1, false) &&
		succeeded;
	succeeded = ProcessEvent(
		EventType::AlternateAdvancement,
		0,
		0,
		m_client.GetAchievementAAPointsSpent(),
		false
	) && succeeded;

	std::set<uint32_t> tasks;
	std::set<uint32_t> dependencies;
	bool reconcile_any_dependency = false;
	for (const auto *criterion : AchievementManager::Instance().Criteria(EventType::TaskComplete)) {
		tasks.insert(criterion->target_id);
	}
	for (
		const auto *criterion :
		AchievementManager::Instance().Criteria(EventType::AchievementComplete)
	) {
		if (criterion->target_id) {
			dependencies.insert(criterion->target_id);
		}
		else {
			reconcile_any_dependency = true;
		}
	}

	for (const auto task_id : tasks) {
		if (task_id && m_client.IsTaskCompleted(static_cast<int>(task_id))) {
			succeeded =
				ProcessEvent(EventType::TaskComplete, task_id, 0, 1, false) &&
				succeeded;
		}
	}
	if (!AchievementManager::Instance().Criteria(EventType::SkillValue).empty()) {
		succeeded =
			ProcessEvent(EventType::SkillValue, 0, 0, 0, false) &&
			succeeded;
	}
	if (!AchievementManager::Instance().Criteria(EventType::SkillCap).empty()) {
		succeeded =
			ProcessEvent(
				EventType::SkillCap,
				EQ::Achievements::SkillWildcardTargetId,
				0,
				0,
				false
			) &&
			succeeded;
	}
	if (!AchievementManager::Instance().Criteria(EventType::OwnItem).empty()) {
		succeeded =
			ProcessEvent(EventType::OwnItem, 0, 0, 0, false) &&
			succeeded;
	}

	for (const auto achievement_id : dependencies) {
		if (achievement_id && HasCompleted(achievement_id)) {
			succeeded = ProcessEvent(
				EventType::AchievementComplete,
				achievement_id,
				0,
				1,
				false
			) && succeeded;
		}
	}
	if (reconcile_any_dependency) {
		const auto &definitions = AchievementManager::Instance().Definitions();
		for (size_t definition_index = 0; definition_index < m_states.size(); ++definition_index) {
			if (m_states[definition_index].status == EQ::Achievements::Status::Completed) {
				succeeded = ProcessEvent(
					EventType::AchievementComplete,
					definitions[definition_index].achievement_id,
					0,
					1,
					false
				) && succeeded;
			}
		}
	}
	return succeeded;
}

bool ClientAchievementState::SupportsPackets() const
{
	return m_client.ClientVersion() == EQ::versions::ClientVersion::RoF2;
}

void ClientAchievementState::SendInitial()
{
	using namespace EQ::Achievements;

	if (!m_loaded || !SupportsPackets()) {
		return;
	}

	const auto &manager = AchievementManager::Instance();
	const auto &definition_packet = manager.DefinitionPacket();
	auto definitions = new EQApplicationPacket(OP_AchievementDefinitions, definition_packet.size());
	std::memcpy(definitions->pBuffer, definition_packet.buffer(), definition_packet.size());
	m_client.FastQueuePacket(&definitions);

	// OP_AchievementState opens comparison mode. Initial and window-request
	// refreshes send only primary state; a validated Compare request sends the
	// comparison snapshot.
	auto primary_data = SerializeDenseUpdate(
		++m_serial,
		manager.Definitions(),
		m_states
	);
	auto primary = new EQApplicationPacket(OP_AchievementUpdate, std::move(primary_data));
	m_client.FastQueuePacket(&primary);

	std::vector<ProgressUpdate> progress;
	for (size_t definition_index = 0; definition_index < manager.Definitions().size(); ++definition_index) {
		const auto &definition = manager.Definitions()[definition_index];
		for (const auto component_type : {1u, 2u, 0u}) {
			for (
				size_t component_index = 0;
				component_index < definition.components[component_type].size();
				++component_index
			) {
				const auto count = m_states[definition_index].counts[component_type][component_index];
				if (!count) {
					continue;
				}
				const auto &component = definition.components[component_type][component_index];
				progress.push_back({
					definition.achievement_id,
					component.component_id,
					component.sequence,
					component_type,
					count
				});
			}
		}
	}
	// RoF2 initializes the progress model only after this third packet arrives.
	// Send a valid zero-entry packet when the character has no saved progress.
	SendProgressUpdates(progress);
	m_initial_sent = true;
	if (!m_pending_notifications.empty() && !m_notification_timer.Enabled()) {
		ArmCompletionNotificationTimer(true);
	}
	RestorePendingRewardSelection();
}

void ClientAchievementState::PreservePendingNotificationsFrom(
	ClientAchievementState &previous
)
{
	if (!m_loaded || !SupportsPackets()) {
		return;
	}

	std::deque<PendingCompletionNotification> merged;
	std::unordered_set<uint32_t> merged_ids;
	const auto append_if_pending = [this, &merged, &merged_ids](
		const PendingCompletionNotification &notification
	) -> bool {
		if (
			HasCompleted(notification.achievement_id) &&
			merged_ids.insert(notification.achievement_id).second
		) {
			merged.push_back(notification);
			return true;
		}
		return false;
	};

	// Keep queued notifications ahead of reload discoveries and deduplicate IDs.
	bool preserved_previous_notification = false;
	for (const auto &notification : previous.m_pending_notifications) {
		preserved_previous_notification =
			append_if_pending(notification) || preserved_previous_notification;
	}
	for (const auto &notification : m_pending_notifications) {
		append_if_pending(notification);
	}

	m_pending_notifications = std::move(merged);
	m_pending_notification_ids = std::move(merged_ids);

	if (preserved_previous_notification && previous.m_notification_timer.Enabled()) {
		const auto remaining = previous.m_notification_timer.GetRemainingTime();
		const auto interval = static_cast<uint32_t>(std::max(
			RuleI(Achievements, CompletionNotificationIntervalMS),
			1
		));
		m_notification_timer.Start(std::max<uint32_t>(remaining, 1));
		m_notification_timer.SetAtTrigger(interval);
		if (!remaining) {
			m_notification_timer.Trigger();
		}
	}
}

bool ClientAchievementState::HasCompleted(uint32_t achievement_id) const
{
	const auto status = GetStatus(achievement_id);
	return status && *status == EQ::Achievements::Status::Completed;
}

std::optional<EQ::Achievements::Status> ClientAchievementState::GetStatus(
	uint32_t achievement_id
) const
{
	if (!m_loaded) {
		return std::nullopt;
	}

	const auto definition_index = AchievementManager::Instance().FindDefinitionIndex(achievement_id);
	if (!definition_index || *definition_index >= m_states.size()) {
		return std::nullopt;
	}

	return m_states[*definition_index].status;
}

std::optional<uint32_t> ClientAchievementState::GetProgress(
	uint32_t achievement_id,
	uint8_t component_type,
	uint32_t component_id
) const
{
	if (!m_loaded || component_type > 2) {
		return std::nullopt;
	}

	const auto &manager = AchievementManager::Instance();
	const auto definition_index = manager.FindDefinitionIndex(achievement_id);
	const auto component_index = manager.FindComponentIndex(
		achievement_id,
		component_type,
		component_id
	);
	if (
		!definition_index ||
		!component_index ||
		*definition_index >= m_states.size() ||
		*component_index >= m_states[*definition_index].counts[component_type].size()
	) {
		return std::nullopt;
	}

	return m_states[*definition_index].counts[component_type][*component_index];
}

bool ClientAchievementState::NeedsOwnershipReconciliation() const
{
	if (!m_loaded) {
		return false;
	}

	auto &manager = AchievementManager::Instance();
	for (const auto *criterion : manager.Criteria(EQ::Achievements::EventType::OwnItem)) {
		if (
			criterion->target_id2 &&
			criterion->target_id2 != m_client.GetClass()
		) {
			continue;
		}
		const auto definition_index = manager.FindDefinitionIndex(criterion->achievement_id);
		if (
			definition_index &&
			*definition_index < m_states.size() &&
			m_states[*definition_index].status != EQ::Achievements::Status::Completed
		) {
			return true;
		}
	}
	return false;
}

bool ClientAchievementState::PassCastRestriction(uint32_t restriction_id) const
{
	const auto &requirements = AchievementManager::Instance().CastRequirements(restriction_id);
	for (const auto &requirement : requirements) {
		const auto completed = HasCompleted(requirement.achievement_id);
		if (
			(requirement.requires_completed && !completed) ||
			(!requirement.requires_completed && completed)
		) {
			return false;
		}
	}
	return true;
}

bool ClientAchievementState::ProcessEvent(
	EQ::Achievements::EventType event_type,
	uint32_t target_id,
	uint32_t target_id2,
	uint32_t value,
	bool send_packets
)
{
	using namespace EQ::Achievements;

	if (!m_loaded) {
		return false;
	}
	send_packets = send_packets && m_initial_sent;

	auto &manager = AchievementManager::Instance();
	const auto &event_criteria =
		event_type == EventType::NpcNameKill
			? manager.NpcNameKillCriteria(target_id)
			: manager.Criteria(event_type);
	if (event_criteria.empty()) {
		return true;
	}
	const auto absolute_event =
		event_type == EventType::Level ||
		event_type == EventType::OwnItem ||
		event_type == EventType::SkillValue ||
		event_type == EventType::SkillCap ||
		event_type == EventType::AlternateAdvancement;

	std::unordered_map<uint32_t, uint64_t> owned_item_counts;
	uint64_t wildcard_owned_item_count = 0;
	if (event_type == EventType::OwnItem) {
		if (!GetOwnedItemCounts(m_client, owned_item_counts)) {
			return false;
		}
		for (const auto &[item_id, count] : owned_item_counts) {
			(void) item_id;
			wildcard_owned_item_count = std::max(wildcard_owned_item_count, count);
		}
	}

	uint32_t wildcard_skill_value = 0;
	if (event_type == EventType::SkillValue) {
		for (uint32_t skill_id = 0; skill_id <= EQ::skills::HIGHEST_SKILL; ++skill_id) {
			wildcard_skill_value = std::max<uint32_t>(
				wildcard_skill_value,
				m_client.GetRawSkill(static_cast<EQ::skills::SkillType>(skill_id))
			);
		}
	}

	using ComponentLocation = std::tuple<size_t, uint8_t, size_t>;
	struct Candidate {
		uint32_t value = 0;
		const AchievementCriterion *criterion = nullptr;
	};
	std::map<ComponentLocation, Candidate> candidates;
	using CriterionComponentIdentity = std::tuple<uint32_t, uint8_t, uint32_t>;
	std::set<CriterionComponentIdentity> touched_skill_cap_components;
	if (
		event_type == EventType::SkillCap &&
		target_id != SkillWildcardTargetId
	) {
		for (const auto *criterion : event_criteria) {
			if (criterion->target_id == target_id) {
				touched_skill_cap_components.emplace(
					criterion->achievement_id,
					criterion->component_type,
					criterion->component_id
				);
			}
		}
	}

	for (const auto *criterion : event_criteria) {
		uint64_t observed_value = value;
		if (event_type == EventType::OwnItem) {
			const auto required_class =
				static_cast<uint8_t>(criterion->target_id2);
			if (required_class && m_client.GetClass() != required_class) {
				observed_value = 0;
			}
			else {
				observed_value = criterion->target_id
					? owned_item_counts[criterion->target_id]
					: wildcard_owned_item_count;
			}
		}
		else if (event_type == EventType::SkillValue) {
			if (criterion->target_id2) {
				continue;
			}
			if (criterion->target_id == SkillWildcardTargetId) {
				observed_value = wildcard_skill_value;
			}
			else {
				observed_value = m_client.GetRawSkill(
					static_cast<EQ::skills::SkillType>(criterion->target_id)
				);
			}
		}
		else if (event_type == EventType::SkillCap) {
			if (
				target_id != SkillWildcardTargetId &&
				!touched_skill_cap_components.contains({
					criterion->achievement_id,
					criterion->component_type,
					criterion->component_id
				})
			) {
				continue;
			}

			const auto milestone_level =
				static_cast<uint8_t>(criterion->target_value);
			const auto required_class =
				static_cast<uint8_t>(criterion->target_id2);
			const auto skill_id =
				static_cast<EQ::skills::SkillType>(criterion->target_id);
			const auto required_cap = m_client.MaxSkill(
				skill_id,
				required_class,
				milestone_level
			);
			const auto meets_cap =
				m_client.GetClass() == required_class &&
				m_client.GetLevel() >= milestone_level &&
				required_cap > 0 &&
				m_client.GetRawSkill(skill_id) >= required_cap;
			observed_value = meets_cap
				? static_cast<uint64_t>(criterion->target_value)
				: 0;
		}
		else if (
			(criterion->target_id && criterion->target_id != target_id) ||
			(criterion->target_id2 && criterion->target_id2 != target_id2)
		) {
			continue;
		}

		const auto below_target =
			criterion->target_value > 0 &&
			observed_value < static_cast<uint64_t>(criterion->target_value);
		if (below_target && !absolute_event) {
			continue;
		}
		const auto definition_index = manager.FindDefinitionIndex(criterion->achievement_id);
		const auto component_index = manager.FindComponentIndex(
			criterion->achievement_id,
			criterion->component_type,
			criterion->component_id
		);
		if (!definition_index || !component_index) {
			continue;
		}
		if (m_states[*definition_index].status == Status::Completed) {
			continue;
		}

		const auto &component =
			manager.Definitions()[*definition_index].components[criterion->component_type][*component_index];
		const auto current = m_states[*definition_index].counts[criterion->component_type][*component_index];
		uint64_t candidate = current;
		switch (criterion->progress_mode) {
		case ProgressMode::Increment:
			candidate = static_cast<uint64_t>(current) + observed_value;
			break;
		case ProgressMode::Highest:
			candidate = below_target
				? current
				: std::max<uint64_t>(current, observed_value);
			break;
		case ProgressMode::Set:
			candidate = below_target ? 0 : observed_value;
			break;
		case ProgressMode::Boolean:
			candidate = below_target ? 0 : component.required_count;
			break;
		}

		const auto location = ComponentLocation{
			*definition_index,
			criterion->component_type,
			*component_index
		};
		auto &entry = candidates[location];
		const auto clamped = ClampCount(candidate, component.required_count);
		if (!entry.criterion || clamped > entry.value) {
			entry.value = clamped;
			entry.criterion = criterion;
		}
	}

	bool persistence_succeeded = true;
	std::unordered_set<size_t> affected_definitions;
	std::set<size_t> evaluatable_definitions;
	std::vector<ProgressUpdate> progress_updates;
	for (const auto &[location, candidate] : candidates) {
		const auto [definition_index, component_type, component_index] = location;
		auto &state = m_states[definition_index];
		if (state.counts[component_type][component_index] == candidate.value) {
			// Durable progress can outlive a failed completion write; evaluate it
			// again when the same fact is observed.
			evaluatable_definitions.insert(definition_index);
			continue;
		}

		const auto &definition = manager.Definitions()[definition_index];
		const auto &component = definition.components[component_type][component_index];
		const auto previous_count = state.counts[component_type][component_index];
		const auto previous_satisfied = state.satisfied[component_type][component_index];
		state.counts[component_type][component_index] = candidate.value;
		state.satisfied[component_type][component_index] =
			candidate.value >= component.required_count;
		if (!PersistProgress(definition_index, component_type, component_index)) {
			state.counts[component_type][component_index] = previous_count;
			state.satisfied[component_type][component_index] = previous_satisfied;
			persistence_succeeded = false;
			continue;
		}

		affected_definitions.insert(definition_index);
		evaluatable_definitions.insert(definition_index);
		progress_updates.push_back({
			definition.achievement_id,
			component.component_id,
			component.sequence,
			component_type,
			candidate.value
		});
	}

	if (send_packets && !progress_updates.empty()) {
		SendProgressUpdates(progress_updates);
	}

	if (event_type == EventType::OwnItem && !persistence_succeeded) {
		// Do not evaluate ownership against a partially persisted refresh.
		return false;
	}

	bool ownership_is_fresh = event_type == EventType::OwnItem;
	bool ownership_refresh_failed = false;
	std::unordered_set<size_t> ownership_gated_definitions;
	if (!ownership_is_fresh) {
		for (const auto definition_index : evaluatable_definitions) {
			const auto achievement_id =
				manager.Definitions()[definition_index].achievement_id;
			for (const auto *criterion : manager.CriteriaForAchievement(achievement_id)) {
				if (
					criterion->event_type == EventType::OwnItem &&
					criterion->behavior != CriterionBehavior::Optional &&
					criterion->behavior != CriterionBehavior::DisplayOnly
				) {
					ownership_gated_definitions.insert(definition_index);
					break;
				}
			}
		}

		if (!ownership_gated_definitions.empty()) {
			if (!ProcessEvent(EventType::OwnItem, 0, 0, 0, send_packets)) {
				m_client.UpdateAchievementForOwnItem(0);
				ownership_refresh_failed = true;
			}
			else {
				ownership_is_fresh = true;
			}
		}
	}

	for (const auto definition_index : evaluatable_definitions) {
		if (
			ownership_refresh_failed &&
			ownership_gated_definitions.contains(definition_index)
		) {
			continue;
		}
		const auto status_changed = EvaluateDefinition(
			definition_index,
			send_packets,
			&persistence_succeeded,
			ownership_is_fresh
		);
		if (
			send_packets &&
			m_states[definition_index].status != Status::Completed &&
			(affected_definitions.contains(definition_index) || status_changed)
		) {
			SendStateUpdate(definition_index);
		}
	}
	return persistence_succeeded && !ownership_refresh_failed;
}

bool ClientAchievementState::SetProgress(
	uint32_t achievement_id,
	uint8_t component_type,
	uint32_t component_id,
	uint32_t value,
	bool additive,
	bool send_packets
)
{
	using namespace EQ::Achievements;

	if (!m_loaded || component_type > 2) {
		return false;
	}
	send_packets = send_packets && m_initial_sent;

	auto &manager = AchievementManager::Instance();
	const auto definition_index = manager.FindDefinitionIndex(achievement_id);
	const auto component_index = manager.FindComponentIndex(
		achievement_id,
		component_type,
		component_id
	);
	if (!definition_index || !component_index) {
		return false;
	}

	const auto &component =
		manager.Definitions()[*definition_index].components[component_type][*component_index];
	auto &state = m_states[*definition_index];
	const auto next = ClampCount(
		additive ? static_cast<uint64_t>(state.counts[component_type][*component_index]) + value : value,
		component.required_count
	);
	if (state.counts[component_type][*component_index] == next) {
		bool persistence_succeeded = true;
		EvaluateDefinition(
			*definition_index,
			send_packets,
			&persistence_succeeded
		);
		return persistence_succeeded;
	}

	const auto previous_count = state.counts[component_type][*component_index];
	const auto previous_satisfied = state.satisfied[component_type][*component_index];
	state.counts[component_type][*component_index] = next;
	state.satisfied[component_type][*component_index] = next >= component.required_count;
	if (!PersistProgress(*definition_index, component_type, *component_index)) {
		state.counts[component_type][*component_index] = previous_count;
		state.satisfied[component_type][*component_index] = previous_satisfied;
		return false;
	}

	if (send_packets) {
		SendProgressUpdates({{
			achievement_id,
			component.component_id,
			component.sequence,
			component_type,
			next
		}});
	}

	bool persistence_succeeded = true;
	EvaluateDefinition(
		*definition_index,
		send_packets,
		&persistence_succeeded
	);
	if (send_packets && state.status != Status::Completed) {
		SendStateUpdate(*definition_index);
	}
	return persistence_succeeded;
}

void ClientAchievementState::EvaluateAll(bool send_packets, bool ownership_is_fresh)
{
	for (size_t definition_index = 0; definition_index < m_states.size(); ++definition_index) {
		EvaluateDefinition(
			definition_index,
			send_packets,
			nullptr,
			ownership_is_fresh
		);
	}
}

bool ClientAchievementState::EvaluateDefinition(
	size_t definition_index,
	bool send_packets,
	bool *persistence_succeeded,
	bool ownership_is_fresh
)
{
	using namespace EQ::Achievements;

	auto &manager = AchievementManager::Instance();
	if (definition_index >= manager.Definitions().size()) {
		return false;
	}

	auto &state = m_states[definition_index];
	const auto &definition = manager.Definitions()[definition_index];
	if (const auto required_class = manager.RequiredClass(definition.achievement_id)) {
		if (m_client.GetClass() != *required_class) {
			const auto changed = state.status != Status::Hidden;
			state.status = Status::Hidden;
			return changed;
		}
	}
	if (state.status == Status::Completed) {
		return false;
	}

	std::map<std::pair<uint8_t, uint32_t>, CriterionBehavior> component_behaviors;
	bool has_completion_ownership_criterion = false;
	for (const auto *criterion : manager.CriteriaForAchievement(definition.achievement_id)) {
		component_behaviors.try_emplace(
			std::make_pair(criterion->component_type, criterion->component_id),
			criterion->behavior
		);
		if (
			criterion->event_type == EventType::OwnItem &&
			criterion->behavior != CriterionBehavior::Optional &&
			criterion->behavior != CriterionBehavior::DisplayOnly
		) {
			has_completion_ownership_criterion = true;
		}
	}

	if (has_completion_ownership_criterion && !ownership_is_fresh) {
		const auto previous_status = state.status;
		const auto refreshed = ProcessEvent(
			EventType::OwnItem,
			0,
			0,
			0,
			send_packets
		);
		if (!refreshed) {
			// The trigger is durable, but ownership must be refreshed before its
			// status or completion transition is evaluated.
			m_client.UpdateAchievementForOwnItem(0);
			if (persistence_succeeded) {
				*persistence_succeeded = false;
			}
			return false;
		}
		const auto reevaluated = EvaluateDefinition(
			definition_index,
			send_packets,
			persistence_succeeded,
			true
		);
		return reevaluated || state.status != previous_status;
	}

	size_t required_components = 0;
	bool all_required_satisfied = true;
	bool hidden = false;
	bool locked = false;
	for (const auto &[component_key, behavior] : component_behaviors) {
		const auto component_index = manager.FindComponentIndex(
			definition.achievement_id,
			component_key.first,
			component_key.second
		);
		if (!component_index) {
			continue;
		}
		const auto satisfied = state.satisfied[component_key.first][*component_index] != 0;
		switch (behavior) {
		case CriterionBehavior::Required:
			++required_components;
			all_required_satisfied = all_required_satisfied && satisfied;
			break;
		case CriterionBehavior::Unlock:
			locked = locked || !satisfied;
			break;
		case CriterionBehavior::Visibility:
			hidden = hidden || !satisfied;
			break;
		case CriterionBehavior::Blocker:
			locked = locked || satisfied;
			break;
		case CriterionBehavior::Optional:
		case CriterionBehavior::DisplayOnly:
			break;
		}
	}

	if (required_components && all_required_satisfied && !hidden && !locked) {
		const auto completed = Complete(definition.achievement_id, send_packets);
		if (
			!completed &&
			state.status != Status::Completed &&
			persistence_succeeded
		) {
			*persistence_succeeded = false;
		}
		return completed;
	}

	const auto next_status = hidden ? Status::Hidden : (locked ? Status::Locked : Status::Open);
	const auto changed = state.status != next_status;
	state.status = next_status;
	return changed;
}

bool ClientAchievementState::PersistProgress(
	size_t definition_index,
	uint8_t component_type,
	size_t component_index
)
{
	const auto &definition = AchievementManager::Instance().Definitions()[definition_index];
	const auto &component = definition.components[component_type][component_index];
	const auto &state = m_states[definition_index];
	const auto count = state.counts[component_type][component_index];
	const auto completed = state.satisfied[component_type][component_index] ? 1 : 0;
	const auto now = static_cast<uint32_t>(std::time(nullptr));

	const auto result = database.QueryDatabase(fmt::format(
		"INSERT INTO character_achievement_progress "
		"(character_id, achievement_id, component_type, component_sequence, component_id, "
		"current_count, completed, `version`, updated_at) "
		"VALUES ({}, {}, {}, {}, {}, {}, {}, {}, {}) "
		"ON DUPLICATE KEY UPDATE component_sequence = VALUES(component_sequence), "
		"component_id = VALUES(component_id), "
		"current_count = VALUES(current_count), completed = VALUES(completed), "
		"`version` = VALUES(`version`), updated_at = VALUES(updated_at)",
		m_client.CharacterID(),
		definition.achievement_id,
		component_type,
		component.sequence,
		component.component_id,
		count,
		completed,
		definition.version,
		now
	));
	if (!result.Success()) {
		LogError(
			"Failed to persist achievement progress for character [{}], achievement [{}]",
			m_client.CharacterID(),
			definition.achievement_id
		);
		return false;
	}
	return true;
}

bool ClientAchievementState::PersistCompletion(size_t definition_index, uint32_t completed_at)
{
	const auto &definition = AchievementManager::Instance().Definitions()[definition_index];
	const auto result = database.QueryDatabase(fmt::format(
		"INSERT INTO character_achievements "
		"(character_id, achievement_id, `version`, completed_at) "
		"VALUES ({}, {}, {}, {}) "
		"ON DUPLICATE KEY UPDATE `version` = VALUES(`version`), "
		"completed_at = VALUES(completed_at)",
		m_client.CharacterID(),
		definition.achievement_id,
		definition.version,
		completed_at
	));
	return result.Success();
}

bool ClientAchievementState::Complete(uint32_t achievement_id, bool send_packets)
{
	using namespace EQ::Achievements;

	if (!m_loaded || m_completion_stack.contains(achievement_id)) {
		return false;
	}
	send_packets = send_packets && m_initial_sent;

	const auto definition_index = AchievementManager::Instance().FindDefinitionIndex(achievement_id);
	if (!definition_index || m_states[*definition_index].status == Status::Completed) {
		return false;
	}

	m_completion_stack.insert(achievement_id);
	const auto completed_at = static_cast<uint32_t>(std::time(nullptr));
	if (!PersistCompletion(*definition_index, completed_at)) {
		m_completion_stack.erase(achievement_id);
		return false;
	}

	auto &state = m_states[*definition_index];
	state.status = Status::Completed;
	state.completion_timestamp = completed_at;
	if (send_packets) {
		SendStateUpdate(*definition_index);
	}
	QueueCompletionNotification(achievement_id);

	if (RuleB(Achievements, GrantRewards)) {
		QueueRewards(achievement_id);
	}

	ProcessEvent(EventType::AchievementComplete, achievement_id, 0, 1, send_packets);
	m_completion_stack.erase(achievement_id);
	if (send_packets) {
		RestorePendingRewardSelection();
	}
	return true;
}

bool ClientAchievementState::Reset(uint32_t achievement_id, bool reset_rewards)
{
	using namespace EQ::Achievements;

	if (!m_loaded) {
		return false;
	}

	auto &manager = AchievementManager::Instance();
	const auto definition_index = manager.FindDefinitionIndex(achievement_id);
	if (!definition_index) {
		return false;
	}

	AchievementStateUpdateCharacterLock state_update_lock(m_client.CharacterID());
	if (!state_update_lock.TryAcquire() || !database.TransactionBeginStrict().Success()) {
		return false;
	}

	const auto character_id = m_client.CharacterID();
	const auto remove_rows = [&](const std::string &table) {
		return database.QueryDatabase(fmt::format(
			"DELETE FROM {} WHERE character_id = {} AND achievement_id = {}",
			table,
			character_id,
			achievement_id
		)).Success();
	};

	bool succeeded =
		remove_rows("character_achievement_pending_updates") &&
		remove_rows("character_achievement_progress") &&
		remove_rows("character_achievements");
	if (succeeded && reset_rewards) {
		succeeded =
			remove_rows("character_achievement_reward_selections") &&
			remove_rows("character_achievement_rewards");
	}

	if (!succeeded || !database.TransactionCommitStrict().Success()) {
		database.TransactionRollbackStrict();
		return false;
	}

	const auto &definition = manager.Definitions()[*definition_index];
	State reset_state;
	for (uint8_t component_type = 0; component_type < 4; ++component_type) {
		const auto component_count = definition.components[component_type].size();
		reset_state.satisfied[component_type].resize(component_count);
		reset_state.counts[component_type].resize(component_count);
	}
	if (const auto required_class = manager.RequiredClass(achievement_id)) {
		if (m_client.GetClass() != *required_class) {
			reset_state.status = Status::Hidden;
		}
	}
	m_states[*definition_index] = std::move(reset_state);

	m_completion_stack.erase(achievement_id);
	m_pending_reward_ids.erase(achievement_id);
	std::erase(m_pending_rewards, achievement_id);
	m_pending_notification_ids.erase(achievement_id);
	std::erase_if(
		m_pending_notifications,
		[achievement_id](const PendingCompletionNotification &notification) {
			return notification.achievement_id == achievement_id;
		}
	);
	m_client.GetRewardSelection().ClearSource(
		RewardSelectionSource::Achievement,
		achievement_id,
		true
	);

	if (m_initial_sent) {
		std::vector<ProgressUpdate> cleared_progress;
		for (const auto component_type : {0u, 1u, 2u}) {
			for (const auto &component : definition.components[component_type]) {
				cleared_progress.push_back({
					achievement_id,
					component.component_id,
					component.sequence,
					component_type,
					0
				});
			}
		}
		SendProgressUpdates(cleared_progress);
		SendStateUpdate(*definition_index);
	}

	return true;
}

void ClientAchievementState::QueueCompletionNotification(uint32_t achievement_id)
{
	if (
		!SupportsPackets() ||
		!m_pending_notification_ids.insert(achievement_id).second
	) {
		return;
	}

	m_pending_notifications.push_back({achievement_id, m_client.GuildID()});
	if (m_initial_sent && !m_notification_timer.Enabled()) {
		ArmCompletionNotificationTimer(true);
	}
}

void ClientAchievementState::ArmCompletionNotificationTimer(bool immediate)
{
	const auto configured_interval = RuleI(Achievements, CompletionNotificationIntervalMS);
	const auto interval = static_cast<uint32_t>(std::max(configured_interval, 1));
	m_notification_timer.Start(interval);
	m_notification_timer.SetAtTrigger(interval);
	if (immediate) {
		m_notification_timer.Trigger();
	}
}

void ClientAchievementState::ProcessPendingNotifications()
{
	if (
		!m_loaded ||
		!m_initial_sent ||
		!SupportsPackets() ||
		!m_client.Connected()
	) {
		return;
	}
	if (m_pending_notifications.empty()) {
		m_notification_timer.Disable();
		return;
	}
	if (!m_notification_timer.Enabled()) {
		ArmCompletionNotificationTimer(true);
	}
	if (!m_notification_timer.Check()) {
		return;
	}

	const auto notification = m_pending_notifications.front();
	m_pending_notifications.pop_front();
	m_pending_notification_ids.erase(notification.achievement_id);
	SendCompletionNotification(notification);

	if (m_pending_notifications.empty()) {
		m_notification_timer.Disable();
	}
}

void ClientAchievementState::QueueRewards(uint32_t achievement_id)
{
	if (
		AchievementManager::Instance().Rewards(achievement_id).empty() ||
		!m_pending_reward_ids.insert(achievement_id).second
	) {
		return;
	}
	m_pending_rewards.push_back(achievement_id);
}

void ClientAchievementState::ProcessPendingRewards()
{
	if (!m_loaded) {
		return;
	}
	if (!RuleB(Achievements, GrantRewards)) {
		m_pending_rewards.clear();
		m_pending_reward_ids.clear();
		return;
	}

	// Rewards can complete more achievements. Drain a bounded number from
	// Client::Process after the triggering game event unwinds.
	const auto per_tick_budget = static_cast<size_t>(std::clamp(
		RuleI(Achievements, RewardGrantsPerTick),
		1,
		64
	));
	for (size_t processed = 0; processed < per_tick_budget && !m_pending_rewards.empty(); ++processed) {
		const auto achievement_id = m_pending_rewards.front();
		const auto &rewards = AchievementManager::Instance().Rewards(achievement_id);
		const auto item_count = static_cast<size_t>(std::count_if(
			rewards.begin(),
			rewards.end(),
			[](const RewardSelectionReward &reward) {
				return reward.type == RewardSelectionRewardType::Item;
			}
		));
		const auto cursor_size = static_cast<size_t>(m_client.GetInv().CursorSize());
		if (
			item_count > static_cast<size_t>(EQ::invbag::CURSOR_BAG_COUNT) ||
			cursor_size > static_cast<size_t>(EQ::invbag::CURSOR_BAG_COUNT) - item_count
		) {
			return;
		}

		m_pending_rewards.pop_front();
		m_pending_reward_ids.erase(achievement_id);
		GrantRewardBatch(achievement_id, rewards);
	}
}

ClientAchievementState::RewardGrantResult ClientAchievementState::GrantRewardBatch(
	uint32_t achievement_id,
	const std::vector<RewardSelectionReward> &rewards
)
{
	auto batch_result = RewardGrantResult::Delivered;
	for (const auto &reward : rewards) {
		const auto result = GrantTrackedReward(achievement_id, reward);
		if (result == RewardGrantResult::Ambiguous) {
			batch_result = RewardGrantResult::Ambiguous;
		}
		else if (
			result == RewardGrantResult::RetryableFailure &&
			batch_result == RewardGrantResult::Delivered
		) {
			batch_result = RewardGrantResult::RetryableFailure;
		}
	}
	return batch_result;
}

ClientAchievementState::RewardGrantResult ClientAchievementState::GrantTrackedReward(
	uint32_t achievement_id,
	const RewardSelectionReward &reward
)
{
	const auto now = static_cast<uint32_t>(std::time(nullptr));
	const auto claim = database.QueryDatabase(fmt::format(
		"INSERT IGNORE INTO character_achievement_rewards "
		"(character_id, achievement_id, reward_id, status, attempt_count, last_attempt_at) "
		"VALUES ({}, {}, {}, 0, 1, {})",
		m_client.CharacterID(),
		achievement_id,
		reward.entry_id,
		now
	));
	if (!claim.Success()) {
		LogError(
			"Failed to claim achievement reward [{}] for character [{}]",
			reward.entry_id,
			m_client.CharacterID()
		);
		return RewardGrantResult::RetryableFailure;
	}

	bool claimed = claim.RowsAffected() != 0;
	if (!claimed) {
		auto existing = database.QueryDatabase(fmt::format(
			"SELECT status FROM character_achievement_rewards "
			"WHERE character_id = {} AND achievement_id = {} AND reward_id = {} LIMIT 1",
			m_client.CharacterID(),
			achievement_id,
			reward.entry_id
		));
		if (!existing.Success() || existing.RowCount() != 1) {
			return RewardGrantResult::RetryableFailure;
		}

		auto row = existing.begin();
		const auto status = ParseUInt32(row[0]);
		if (status == 1) {
			return RewardGrantResult::Delivered;
		}
		if (status == 0) {
			// Delivery may have occurred before a status-zero row was finalized.
			return RewardGrantResult::Ambiguous;
		}
		if (status != 2) {
			return RewardGrantResult::RetryableFailure;
		}

		const auto retry = database.QueryDatabase(fmt::format(
			"UPDATE character_achievement_rewards "
			"SET status = 0, attempt_count = attempt_count + 1, "
			"last_attempt_at = {}, last_error = '' "
			"WHERE character_id = {} AND achievement_id = {} "
			"AND reward_id = {} AND status = 2",
			now,
			m_client.CharacterID(),
			achievement_id,
			reward.entry_id
		));
		if (!retry.Success()) {
			LogError(
				"Failed to reclaim achievement reward [{}] for character [{}]",
				reward.entry_id,
				m_client.CharacterID()
			);
			return RewardGrantResult::RetryableFailure;
		}
		claimed = retry.RowsAffected() != 0;
	}
	if (!claimed) {
		return RewardGrantResult::RetryableFailure;
	}

	const auto grant_result = static_cast<RewardGrantResult>(
		ClientRewardSelection::GrantReward(
			m_client,
			reward
		)
	);
	if (grant_result == RewardGrantResult::Delivered) {
		const auto granted = database.QueryDatabase(fmt::format(
			"UPDATE character_achievement_rewards "
			"SET status = 1, granted_at = {}, last_error = '' "
			"WHERE character_id = {} AND achievement_id = {} AND reward_id = {}",
			static_cast<uint32_t>(std::time(nullptr)),
			m_client.CharacterID(),
			achievement_id,
			reward.entry_id
		));
		if (!granted.Success() || granted.RowsAffected() == 0) {
			LogError(
				"Achievement reward [{}] was delivered to character [{}], "
				"but its ledger could not be finalized",
				reward.entry_id,
				m_client.CharacterID()
			);
			return RewardGrantResult::Ambiguous;
		}
		return RewardGrantResult::Delivered;
	}
	if (grant_result == RewardGrantResult::RetryableFailure) {
		const auto failed = database.QueryDatabase(fmt::format(
			"UPDATE character_achievement_rewards "
			"SET status = 2, last_error = 'delivery API reported failure' "
			"WHERE character_id = {} AND achievement_id = {} AND reward_id = {}",
			m_client.CharacterID(),
			achievement_id,
			reward.entry_id
		));
		if (!failed.Success()) {
			LogError(
				"Failed to record delivery failure for achievement reward [{}], character [{}]",
				reward.entry_id,
				m_client.CharacterID()
			);
			return RewardGrantResult::Ambiguous;
		}
		return RewardGrantResult::RetryableFailure;
	}

	// Keep status 0 because retrying an uncertain delivery could duplicate it.
	const auto ambiguous = database.QueryDatabase(fmt::format(
		"UPDATE character_achievement_rewards "
		"SET last_error = 'delivery persistence result was ambiguous' "
		"WHERE character_id = {} AND achievement_id = {} AND reward_id = {}",
		m_client.CharacterID(),
		achievement_id,
		reward.entry_id
	));
	if (!ambiguous.Success()) {
		LogError(
			"Failed to record ambiguous delivery for achievement reward [{}], character [{}]",
			reward.entry_id,
			m_client.CharacterID()
		);
	}
	LogError(
		"Achievement reward [{}] delivery was ambiguous for character [{}]; "
		"the pending claim will not be retried automatically",
		reward.entry_id,
		m_client.CharacterID()
	);
	return RewardGrantResult::Ambiguous;
}

void ClientAchievementState::SendStateUpdate(size_t definition_index)
{
	if (!SupportsPackets()) {
		return;
	}

	const auto &definitions = AchievementManager::Instance().Definitions();
	auto data = EQ::Achievements::SerializeIncremental(
		++m_serial,
		definitions,
		{{static_cast<uint32_t>(definition_index), m_states[definition_index]}}
	);
	auto packet = new EQApplicationPacket(OP_AchievementUpdate, std::move(data));
	m_client.FastQueuePacket(&packet);
}

void ClientAchievementState::SendCompletionNotification(
	const PendingCompletionNotification &notification
)
{
	const auto achievement_id = notification.achievement_id;
	const auto definition_index = AchievementManager::Instance().FindDefinitionIndex(achievement_id);
	if (
		!definition_index ||
		*definition_index >= m_states.size() ||
		m_states[*definition_index].status != EQ::Achievements::Status::Completed
	) {
		return;
	}

	const auto &definition = AchievementManager::Instance().Definitions()[*definition_index];
	const auto achievement_link_data = EQ::Achievements::SerializeLinkData(
		m_client.GetName(),
		definition,
		m_states[*definition_index]
	);
	auto data = EQ::Achievements::SerializeEarnedNotification(
		m_client.GetID(),
		achievement_id,
		EQ::Achievements::RoF2AchievementSoundId,
		achievement_link_data
	);
	auto packet = new EQApplicationPacket(OP_AchievementEarned, std::move(data));
	const auto nearby_distance = RuleI(Achievements, NearbyPlayerNotificationDistance);
	if (RuleB(Achievements, NearbyPlayerNotifications) && nearby_distance > 0) {
		const auto distance = static_cast<float>(nearby_distance);
		for (const auto &entry : m_client.GetCloseMobList(distance)) {
			auto mob = entry.second;
			if (!mob || !mob->IsClient()) {
				continue;
			}

			auto client = mob->CastToClient();
			if (
				client == &m_client ||
				!client->Connected() ||
				client->ClientVersion() != EQ::versions::ClientVersion::RoF2 ||
				m_client.CalculateDistance(client) >= distance
			) {
				continue;
			}

			client->QueuePacket(packet);
		}
	}
	m_client.FastQueuePacket(&packet);

	if (
		RuleB(Achievements, GuildMemberNotifications) &&
		notification.guild_id != GUILD_NONE
	) {
		guild_mgr.SendAchievementAnnouncement(
			notification.guild_id,
			achievement_id,
			m_client.GetName(),
			achievement_link_data
		);
	}
}

void ClientAchievementState::SendProgressUpdates(
	const std::vector<EQ::Achievements::ProgressUpdate> &updates
)
{
	if (!SupportsPackets()) {
		return;
	}
	auto data = EQ::Achievements::SerializeProgress(updates);
	auto packet = new EQApplicationPacket(OP_AchievementProgress, std::move(data));
	m_client.FastQueuePacket(&packet);
}

void ClientAchievementState::SendComparison(uint32_t achievement_id)
{
	SendComparisonTo(m_client, achievement_id);
}

void ClientAchievementState::SendComparisonTo(
	Client &recipient,
	uint32_t achievement_id
)
{
	if (
		!m_loaded ||
		recipient.ClientVersion() != EQ::versions::ClientVersion::RoF2
	) {
		return;
	}

	const auto definition_index = AchievementManager::Instance().FindDefinitionIndex(achievement_id);
	if (!definition_index) {
		return;
	}

	const auto &definition = AchievementManager::Instance().Definitions()[*definition_index];
	auto data = EQ::Achievements::SerializeComparison(
		m_client.GetName(),
		achievement_id,
		definition,
		m_states[*definition_index]
	);
	auto packet = new EQApplicationPacket(OP_AchievementComparisonReply, std::move(data));
	recipient.FastQueuePacket(&packet);
}

void ClientAchievementState::SendCompareSnapshotTo(Client &recipient) const
{
	if (
		!m_loaded ||
		recipient.ClientVersion() != EQ::versions::ClientVersion::RoF2
	) {
		return;
	}

	const auto &definitions = AchievementManager::Instance().Definitions();
	if (definitions.size() != m_states.size()) {
		return;
	}

	auto data = EQ::Achievements::SerializeSnapshot(definitions, m_states);
	auto packet = new EQApplicationPacket(OP_AchievementState, std::move(data));
	recipient.FastQueuePacket(&packet);
}

void ClientAchievementState::SendRewardDisplay(uint32_t definition_index)
{
	if (!SupportsPackets()) {
		return;
	}

	auto &manager = AchievementManager::Instance();
	if (
		definition_index >= manager.Definitions().size() ||
		definition_index >= m_states.size()
	) {
		return;
	}

	const auto &definition = manager.Definitions()[definition_index];
	auto &selection = m_client.GetRewardSelection();
	if (const auto active = selection.FindSession(
			RewardSelectionChannel::Claimable,
			RewardSelectionSource::Achievement,
			definition.achievement_id
		)) {
		const auto pending = *active;
		if (selection.Open(pending)) {
			return;
		}
	}

	const auto reward_set = manager.RewardSet(definition.achievement_id);
	if (
		reward_set &&
		m_states[definition_index].status ==
			EQ::Achievements::Status::Completed &&
		RuleB(Achievements, GrantRewards)
	) {
		std::vector<PersistedAchievementRewardSelection> persisted;
		auto rows = database.QueryDatabase(fmt::format(
			"SELECT reward_set_id, selected_option_id, status "
			"FROM character_achievement_reward_selections "
			"WHERE character_id = {} AND achievement_id = {}",
			m_client.CharacterID(),
			definition.achievement_id
		));
		if (!rows.Success()) {
			LogError(
				"Failed to inspect selectable achievement reward [{}] for "
				"character [{}]",
				definition.achievement_id,
				m_client.CharacterID()
			);
		}
		else {
			for (auto row : rows) {
				persisted.push_back({
					.reward_set_id = ParseUInt32(row[0]),
					.selected_option_id = ParseUInt32(row[1]),
					.status = ParseUInt32(row[2])
				});
			}

			const auto saved = FindPersistedSelection(
				persisted,
				reward_set->reward_set_id
			);
			if (!saved && !persisted.empty()) {
				LogError(
					"Selectable achievement reward [{}] for character [{}] "
					"does not match the active reward set; refusing to replace "
					"its durable selection",
					definition.achievement_id,
					m_client.CharacterID()
				);
			}
			else {
				uint32_t selected_option_id = 0;
				if (ResolvePendingOption(saved, selected_option_id)) {
					auto pending = BuildAchievementRewardSelectionSession(
						definition,
						*reward_set,
						RewardSelectionChannel::Claimable,
						selected_option_id
					);
					if (pending && selection.Open(*pending)) {
						return;
					}
					LogError(
						"Pending selectable achievement reward [{}] for "
						"character [{}] could not be presented",
						definition.achievement_id,
						m_client.CharacterID()
					);
				}
			}
		}
	}

	RewardSelectionSession session;
	session.source.source = RewardSelectionSource::Achievement;
	session.source.source_id = definition.achievement_id;
	session.channel = RewardSelectionChannel::Preview;
	session.pending_reward_id = definition.achievement_id;
	session.reward_set.title = definition.name;

	if (reward_set) {
		auto preview = BuildAchievementRewardSelectionSession(
			definition,
			*reward_set,
			RewardSelectionChannel::Preview
		);
		if (!preview) {
			m_client.GetRewardSelection().Clear(
				RewardSelectionChannel::Preview
			);
			return;
		}
		session = std::move(*preview);
	}
	else {
		const auto &rewards = manager.Rewards(definition.achievement_id);
		if (rewards.empty()) {
			m_client.GetRewardSelection().Clear(
				RewardSelectionChannel::Preview
			);
			return;
		}

		// Preview automatic rewards as one common, non-claimable bundle.
		session.reward_set.reward_set_id = definition.achievement_id;
		RewardSelectionOption common;
		common.option_id = 1;
		common.common_to_all = true;
		common.rewards.reserve(rewards.size());
		for (const auto &reward : rewards) {
			common.rewards.push_back(reward);
		}
		session.reward_set.options.emplace_back(std::move(common));
	}

	// No pending selection is claimable, so answer action 5 on the preview channel.
	if (!m_client.GetRewardSelection().Open(session)) {
		m_client.GetRewardSelection().Clear(
			RewardSelectionChannel::Preview
		);
	}
}

bool ClientAchievementState::RestorePendingRewardSelection()
{
	if (
		!m_loaded ||
		!m_initial_sent ||
		!SupportsPackets() ||
		!RuleB(Achievements, GrantRewards)
	) {
		return false;
	}

	auto &selection = m_client.GetRewardSelection();

	// A status-zero entry makes replay ambiguous. Otherwise, skip delivered rows
	// and retry explicit failures.
	auto interrupted_ambiguous = database.QueryDatabase(fmt::format(
		"UPDATE character_achievement_reward_selections AS selections "
		"SET status = 3, "
		"last_error = "
		"'interrupted delivery has an in-flight reward entry' "
		"WHERE selections.character_id = {} "
		"AND selections.status = 0 "
		"AND selections.selected_option_id <> 0 "
		"AND EXISTS ("
		"SELECT 1 FROM character_achievement_rewards AS entries "
		"WHERE entries.character_id = selections.character_id "
		"AND entries.achievement_id = selections.achievement_id "
		"AND entries.status = 0"
		")",
		m_client.CharacterID()
	));
	if (!interrupted_ambiguous.Success()) {
		LogError(
			"Failed to quarantine interrupted selectable achievement rewards "
			"for character [{}]",
			m_client.CharacterID()
		);
	}
	else {
		auto interrupted_retryable = database.QueryDatabase(fmt::format(
			"UPDATE character_achievement_reward_selections "
			"SET status = 2, "
			"last_error = 'resuming an interrupted ledger-safe delivery' "
			"WHERE character_id = {} AND status = 0 "
			"AND selected_option_id <> 0",
			m_client.CharacterID()
		));
		if (!interrupted_retryable.Success()) {
			LogError(
				"Failed to recover interrupted selectable achievement rewards "
				"for character [{}]",
				m_client.CharacterID()
			);
		}
	}

	std::unordered_map<
		uint32_t,
		std::vector<PersistedAchievementRewardSelection>
	> persisted;
	auto rows = database.QueryDatabase(fmt::format(
		"SELECT achievement_id, reward_set_id, selected_option_id, status "
		"FROM character_achievement_reward_selections "
		"WHERE character_id = {}",
		m_client.CharacterID()
	));
	if (!rows.Success()) {
		LogError(
			"Failed to restore selectable achievement rewards for character [{}]",
			m_client.CharacterID()
		);
		return false;
	}
	for (auto row : rows) {
		persisted[ParseUInt32(row[0])].push_back({
			.reward_set_id = ParseUInt32(row[1]),
			.selected_option_id = ParseUInt32(row[2]),
			.status = ParseUInt32(row[3])
		});
	}

	const auto &manager = AchievementManager::Instance();
	const auto &definitions = manager.Definitions();
	if (definitions.size() != m_states.size()) {
		return false;
	}

	std::vector<RewardSelectionSession> sessions;
	for (size_t definition_index = 0; definition_index < definitions.size(); ++definition_index) {
		if (
			m_states[definition_index].status !=
			EQ::Achievements::Status::Completed
		) {
			continue;
		}

		const auto &definition = definitions[definition_index];
		const auto reward_set = manager.RewardSet(definition.achievement_id);
		if (!reward_set) {
			continue;
		}

		uint32_t selected_option_id = 0;
		if (const auto found = persisted.find(definition.achievement_id);
			found != persisted.end()) {
			const auto saved = FindPersistedSelection(
				found->second,
				reward_set->reward_set_id
			);
			if (!saved) {
				LogError(
					"Selectable achievement reward [{}] for character [{}] "
					"does not match the active reward set; refusing to replace "
					"its durable selection",
					definition.achievement_id,
					m_client.CharacterID()
				);
				continue;
			}
			if (!ResolvePendingOption(saved, selected_option_id)) {
				continue;
			}
		}

		auto session = BuildAchievementRewardSelectionSession(
			definition,
			*reward_set,
			RewardSelectionChannel::Claimable,
			selected_option_id
		);
		if (session) {
			sessions.push_back(std::move(*session));
			continue;
		}

		LogError(
			"Pending selectable achievement reward [{}] for character [{}] "
			"could not be presented",
			definition.achievement_id,
			m_client.CharacterID()
		);
	}

	selection.ClearSource(
		RewardSelectionChannel::Claimable,
		RewardSelectionSource::Achievement,
		0,
		sessions.empty()
	);
	return !sessions.empty() && selection.Open(sessions);
}

RewardSelectionDeliveryResult ClientAchievementState::ClaimReward(
	uint32_t pending_reward_id,
	uint32_t reward_set_id,
	uint32_t selected_option_id
)
{
	if (
		!m_loaded ||
		!RuleB(Achievements, GrantRewards)
	) {
		return RewardSelectionDeliveryResult::RetryableFailure;
	}

	const auto achievement_id = pending_reward_id;
	const auto reward_set = AchievementManager::Instance().RewardSet(achievement_id);
	if (
		!reward_set ||
		reward_set_id != reward_set->reward_set_id ||
		!HasCompleted(achievement_id)
	) {
		return RewardSelectionDeliveryResult::RetryableFailure;
	}

	const RewardSelectionOption *selected_option = nullptr;
	std::vector<RewardSelectionReward> rewards;
	for (const auto &option : reward_set->options) {
		if (option.common_to_all) {
			rewards.insert(rewards.end(), option.rewards.begin(), option.rewards.end());
		}
		else if (option.option_id == selected_option_id) {
			selected_option = &option;
		}
	}
	if (!selected_option) {
		return RewardSelectionDeliveryResult::RetryableFailure;
	}
	rewards.insert(
		rewards.end(),
		selected_option->rewards.begin(),
		selected_option->rewards.end()
	);

	const auto now = static_cast<uint32_t>(std::time(nullptr));
	auto selection = database.QueryDatabase(fmt::format(
		"INSERT IGNORE INTO character_achievement_reward_selections "
		"(character_id, achievement_id, reward_set_id, selected_option_id, "
		"status, attempt_count, last_attempt_at) "
		"VALUES ({}, {}, {}, {}, 0, 1, {})",
		m_client.CharacterID(),
		achievement_id,
		reward_set->reward_set_id,
		selected_option_id,
		now
	));
	if (!selection.Success()) {
		return RewardSelectionDeliveryResult::RetryableFailure;
	}

	if (selection.RowsAffected() == 0) {
		auto existing = database.QueryDatabase(fmt::format(
			"SELECT selected_option_id, status "
			"FROM character_achievement_reward_selections "
			"WHERE character_id = {} AND achievement_id = {} "
			"AND reward_set_id = {} LIMIT 1",
			m_client.CharacterID(),
			achievement_id,
			reward_set->reward_set_id
		));
		if (!existing.Success() || existing.RowCount() != 1) {
			return RewardSelectionDeliveryResult::RetryableFailure;
		}

		auto row = existing.begin();
		const auto persisted_option_id = ParseUInt32(row[0]);
		const auto persisted_status = ParseUInt32(row[1]);
		if (persisted_status == 1) {
			return persisted_option_id == selected_option_id
				? RewardSelectionDeliveryResult::Delivered
				: RewardSelectionDeliveryResult::RetryableFailure;
		}
		if (persisted_status > 3) {
			return RewardSelectionDeliveryResult::RetryableFailure;
		}
		if (persisted_status == 3) {
			return RewardSelectionDeliveryResult::Ambiguous;
		}
		if (persisted_status == 0 && persisted_option_id) {
			// A locked status-zero selection may already have been delivered.
			return RewardSelectionDeliveryResult::Ambiguous;
		}
		if (
			persisted_option_id &&
			persisted_option_id != selected_option_id
		) {
			return RewardSelectionDeliveryResult::RetryableFailure;
		}

		selection = database.QueryDatabase(fmt::format(
			"UPDATE character_achievement_reward_selections "
			"SET selected_option_id = {}, status = 0, "
			"attempt_count = attempt_count + 1, "
			"last_attempt_at = {}, last_error = '' "
			"WHERE character_id = {} AND achievement_id = {} "
			"AND reward_set_id = {} AND "
			"((selected_option_id = 0 AND status = 0) OR "
			"(selected_option_id = {} AND status = 2))",
			selected_option_id,
			now,
			m_client.CharacterID(),
			achievement_id,
			reward_set->reward_set_id,
			selected_option_id
		));
		if (!selection.Success() || selection.RowsAffected() == 0) {
			return RewardSelectionDeliveryResult::RetryableFailure;
		}
	}

	const auto grant_result = GrantRewardBatch(achievement_id, rewards);
	uint32_t status = 1;
	const char *last_error = "";
	if (grant_result == RewardGrantResult::RetryableFailure) {
		status = 2;
		last_error = "one or more reward deliveries explicitly failed";
	}
	else if (grant_result == RewardGrantResult::Ambiguous) {
		status = 3;
		last_error = "one or more reward deliveries were ambiguous";
	}

	const auto finalized = database.QueryDatabase(fmt::format(
		"UPDATE character_achievement_reward_selections "
		"SET status = {}, claimed_at = {}, last_error = '{}' "
		"WHERE character_id = {} AND achievement_id = {} "
		"AND reward_set_id = {} AND selected_option_id = {} AND status = 0",
		status,
		status == 1 ? static_cast<uint32_t>(std::time(nullptr)) : 0,
		last_error,
		m_client.CharacterID(),
		achievement_id,
		reward_set->reward_set_id,
		selected_option_id
	));
	if (!finalized.Success() || finalized.RowsAffected() == 0) {
		return RewardSelectionDeliveryResult::Ambiguous;
	}
	return static_cast<RewardSelectionDeliveryResult>(grant_result);
}

bool Client::LoadAchievements()
{
	return ReplaceAchievementState(false, false);
}

bool Client::ReloadAchievements()
{
	const auto achievements_enabled = RuleB(Achievements, EnableAchievements);
	// Deferred events were observed against the previous definition snapshot.
	m_deferred_achievement_state_updates.clear();
	m_achievement_inventory_transaction_update_checkpoint = 0;
	if (!achievements_enabled && !m_achievement_inventory_transaction_depth) {
		m_achievement_inventory_update_pending = false;
		m_achievement_inventory_transaction_failed = false;
	}

	return ReplaceAchievementState(Connected(), !achievements_enabled);
}

bool Client::ReplaceAchievementState(bool send_initial, bool allow_disabled)
{
	m_achievement_ownership_reconcile_timer.Disable();
	if (m_reward_selection) {
		m_reward_selection->ClearSource(
			RewardSelectionSource::Achievement,
			0,
			send_initial
		);
	}
	auto replacement = std::make_unique<ClientAchievementState>(*this);
	if (!replacement->Load(allow_disabled)) {
		// The old state indexes refer to the previous manager snapshot.
		m_achievement_state.reset();
		if (RuleB(Achievements, EnableAchievements) || allow_disabled) {
			m_achievement_state_load_pending = true;
			m_achievement_state_load_send_initial = send_initial || Connected();
			m_achievement_state_load_allow_disabled = allow_disabled;
			m_achievement_state_load_retry_timer.Start(1000);
		}
		else {
			m_achievement_state_load_pending = false;
			m_achievement_state_load_retry_timer.Disable();
			m_deferred_achievement_state_updates.clear();
			m_achievement_inventory_transaction_update_checkpoint = 0;
			if (!m_achievement_inventory_transaction_depth) {
				m_achievement_inventory_update_pending = false;
				m_achievement_inventory_transaction_failed = false;
			}
		}
		return false;
	}

	m_achievement_state_load_pending = false;
	m_achievement_state_load_retry_timer.Disable();
	if (m_achievement_state) {
		replacement->PreservePendingNotificationsFrom(*m_achievement_state);
	}
	m_achievement_state = std::move(replacement);
	ConfigureAchievementOwnershipReconciliation();
	if (send_initial) {
		m_achievement_state->SendInitial();
	}
	return true;
}

void Client::ConfigureAchievementOwnershipReconciliation()
{
	const auto ownership_reconcile_interval =
		RuleI(Achievements, OwnershipReconcileIntervalMS);
	if (
		ownership_reconcile_interval > 0 &&
		m_achievement_state->NeedsOwnershipReconciliation()
	) {
		const auto interval = static_cast<uint32>(ownership_reconcile_interval);
		// Jitter the first background pass to spread shared-bank reads after zoning.
		const auto character_hash = CharacterID() * 2654435761u;
		const auto first_interval =
			std::max<uint32>(interval / 2, 1) + (character_hash % interval);
		m_achievement_ownership_reconcile_timer.Start(first_interval);
		m_achievement_ownership_reconcile_timer.SetAtTrigger(interval);
	}
}

void Client::SendAchievementPackets()
{
	if (m_achievement_state) {
		m_achievement_state->SendInitial();
	}
}

void Client::ProcessAchievementRewards()
{
	if (
		m_achievement_state_load_pending &&
		m_achievement_state_load_retry_timer.Check()
	) {
		const auto send_initial =
			m_achievement_state_load_send_initial || Connected();
		const auto allow_disabled = m_achievement_state_load_allow_disabled;
		if (!ReplaceAchievementState(send_initial, allow_disabled)) {
			return;
		}
	}

	if (!m_achievement_state) {
		return;
	}
	m_achievement_state->ProcessPendingNotifications();

	const auto periodic_ownership_due =
		Connected() && m_achievement_ownership_reconcile_timer.Check();
	const auto foreground_work_pending =
		m_achievement_inventory_update_pending ||
		!m_deferred_achievement_state_updates.empty();

	if (
		(m_achievement_inventory_update_pending || !m_deferred_achievement_state_updates.empty()) &&
		!FlushAchievementInventoryUpdate()
	) {
		return;
	}

	ProcessPendingAchievementStateUpdates();

	if (periodic_ownership_due && !foreground_work_pending) {
		if (m_achievement_state->NeedsOwnershipReconciliation()) {
			// Another online character can change the shared bank. On failure,
			// wait for the next interval instead of retrying every client tick.
			m_achievement_state->ProcessEvent(
				EQ::Achievements::EventType::OwnItem,
				0,
				0,
				0
			);
		}
		else {
			m_achievement_ownership_reconcile_timer.Disable();
		}
	}
	m_achievement_state->ProcessPendingRewards();
}

void Client::NotifyAchievementStateUpdatePending()
{
	m_achievement_pending_state_updates = true;
}

void Client::ProcessPendingAchievementStateUpdates()
{
	if (
		!m_achievement_pending_state_updates ||
		!m_achievement_state ||
		ShouldDeferAchievementStateUpdate() ||
		!RuleB(Achievements, EnableAchievements)
	) {
		return;
	}

	m_achievement_pending_state_updates = false;
	if (!m_achievement_state->DrainPendingStateUpdates(false)) {
		LogError(
			"Failed to process one or more pending achievement state updates for character [{}]",
			CharacterID()
		);
	}
}

bool Client::HasCompletedAchievement(uint32 achievement_id) const
{
	return m_achievement_state && m_achievement_state->HasCompleted(achievement_id);
}

int Client::GetAchievementStatus(uint32 achievement_id) const
{
	if (!m_achievement_state) {
		return -1;
	}

	const auto status = m_achievement_state->GetStatus(achievement_id);
	return status ? static_cast<int>(*status) : -1;
}

int64 Client::GetAchievementProgress(
	uint32 achievement_id,
	uint8 component_type,
	uint32 component_id
) const
{
	if (!m_achievement_state) {
		return -1;
	}

	const auto progress = m_achievement_state->GetProgress(
		achievement_id,
		component_type,
		component_id
	);
	return progress ? static_cast<int64>(*progress) : -1;
}

bool Client::PassAchievementCastRestriction(uint32 restriction_id) const
{
	const auto &requirements = AchievementManager::Instance().CastRequirements(restriction_id);
	if (requirements.empty()) {
		return true;
	}
	return m_achievement_state && m_achievement_state->PassCastRestriction(restriction_id);
}

void Client::SendAchievementComparison(uint32 definition_index)
{
	if (!m_achievement_state) {
		return;
	}

	const auto &definitions = AchievementManager::Instance().Definitions();
	if (definition_index >= definitions.size()) {
		return;
	}

	m_achievement_state->SendComparison(definitions[definition_index].achievement_id);
}

void Client::SendAchievementComparisonTo(
	Client &recipient,
	uint32 achievement_id
)
{
	if (m_achievement_state) {
		m_achievement_state->SendComparisonTo(recipient, achievement_id);
	}
}

void Client::SendAchievementCompareSnapshotTo(Client &recipient) const
{
	if (m_achievement_state) {
		m_achievement_state->SendCompareSnapshotTo(recipient);
	}
}

void Client::SendAchievementRewardDisplay(uint32 definition_index)
{
	if (m_achievement_state) {
		m_achievement_state->SendRewardDisplay(definition_index);
	}
}

bool Client::RestorePendingAchievementRewardSelection()
{
	return
		m_achievement_state &&
		m_achievement_state->RestorePendingRewardSelection();
}

RewardSelectionDeliveryResult Client::ClaimAchievementReward(
	uint32 pending_reward_id,
	uint32 reward_set_id,
	uint32 selected_option_id
)
{
	return m_achievement_state
		? m_achievement_state->ClaimReward(
			pending_reward_id,
			reward_set_id,
			selected_option_id
		)
		: RewardSelectionDeliveryResult::RetryableFailure;
}

bool Client::ShouldDeferAchievementStateUpdate() const
{
	return
		m_achievement_state_load_pending ||
		(
			m_achievement_state &&
			(
				m_achievement_inventory_transaction_depth ||
				m_achievement_inventory_update_pending
			)
		);
}

void Client::QueueAchievementStateUpdate(const DeferredAchievementStateUpdate &update)
{
	m_deferred_achievement_state_updates.push_back(update);
}

bool Client::FlushAchievementInventoryUpdate()
{
	if (!m_achievement_state || m_achievement_inventory_transaction_depth) {
		return false;
	}

	if (m_achievement_inventory_update_pending) {
		if (!m_achievement_state->ProcessEvent(
			EQ::Achievements::EventType::OwnItem,
			0,
			0,
			0
		)) {
			// Keep dependent state updates queued until the ownership read succeeds.
			return false;
		}
		m_achievement_inventory_update_pending = false;
	}

	ReplayDeferredAchievementStateUpdates();
	return true;
}

void Client::ReplayDeferredAchievementStateUpdates()
{
	if (ShouldDeferAchievementStateUpdate() || m_deferred_achievement_state_updates.empty()) {
		return;
	}

	auto updates = std::move(m_deferred_achievement_state_updates);
	m_deferred_achievement_state_updates.clear();
	for (const auto &update : updates) {
		switch (update.type) {
		case DeferredAchievementStateUpdateType::Kill:
			UpdateAchievementForKill(
				update.value1,
				update.value2,
				update.value3,
				update.value4
			);
			break;
		case DeferredAchievementStateUpdateType::Level:
			UpdateAchievementForLevel(update.value1);
			break;
		case DeferredAchievementStateUpdateType::Task:
			UpdateAchievementForTask(update.value1);
			break;
		case DeferredAchievementStateUpdateType::Zone:
			UpdateAchievementForZone(update.value1);
			break;
		case DeferredAchievementStateUpdateType::Loot:
			UpdateAchievementForLoot(update.value1, update.value2);
			break;
		case DeferredAchievementStateUpdateType::Tradeskill:
			UpdateAchievementForTradeskill(update.value1);
			break;
		case DeferredAchievementStateUpdateType::Skill:
			UpdateAchievementForSkill(update.value1, update.value2);
			break;
		case DeferredAchievementStateUpdateType::AlternateAdvancement:
			UpdateAchievementForAA(update.value1);
			break;
		case DeferredAchievementStateUpdateType::SetProgress:
			SetAchievementProgress(
				update.value1,
				static_cast<uint8>(update.value2),
				update.value3,
				update.value4,
				update.flag
			);
			break;
		case DeferredAchievementStateUpdateType::Complete:
			CompleteAchievement(update.value1);
			break;
		}
	}
}

void Client::UpdateAchievementForKill(
	uint32 npc_type_id,
	uint32 race_id,
	uint32 npc_name_identity,
	uint32 zone_id
)
{
	if (ShouldDeferAchievementStateUpdate()) {
		QueueAchievementStateUpdate({
			DeferredAchievementStateUpdateType::Kill,
			npc_type_id,
			race_id,
			npc_name_identity,
			zone_id
		});
		return;
	}
	if (!m_achievement_state) {
		return;
	}
	m_achievement_state->ProcessEvent(EQ::Achievements::EventType::NpcKill, npc_type_id);
	m_achievement_state->ProcessEvent(EQ::Achievements::EventType::NpcRaceKill, race_id);
	if (npc_name_identity) {
		m_achievement_state->ProcessEvent(
			EQ::Achievements::EventType::NpcNameKill,
			npc_name_identity,
			zone_id
		);
	}
}

void Client::UpdateAchievementForLevel(uint32 level)
{
	if (ShouldDeferAchievementStateUpdate()) {
		QueueAchievementStateUpdate({DeferredAchievementStateUpdateType::Level, level});
		return;
	}
	if (!m_achievement_state) {
		return;
	}
	m_achievement_state->ProcessEvent(EQ::Achievements::EventType::Level, 0, 0, level);
	m_achievement_state->ProcessEvent(
		EQ::Achievements::EventType::SkillCap,
		EQ::Achievements::SkillWildcardTargetId
	);
}

void Client::UpdateAchievementForTask(uint32 task_id)
{
	if (ShouldDeferAchievementStateUpdate()) {
		QueueAchievementStateUpdate({DeferredAchievementStateUpdateType::Task, task_id});
		return;
	}
	if (!m_achievement_state) {
		return;
	}
	m_achievement_state->ProcessEvent(EQ::Achievements::EventType::TaskComplete, task_id);
}

void Client::UpdateAchievementForZone(uint32 zone_id)
{
	if (ShouldDeferAchievementStateUpdate()) {
		QueueAchievementStateUpdate({DeferredAchievementStateUpdateType::Zone, zone_id});
		return;
	}
	if (!m_achievement_state) {
		return;
	}
	m_achievement_state->ProcessEvent(EQ::Achievements::EventType::ZoneEnter, zone_id);
}

void Client::UpdateAchievementForLoot(uint32 item_id, uint32 quantity)
{
	if (ShouldDeferAchievementStateUpdate()) {
		QueueAchievementStateUpdate({
			DeferredAchievementStateUpdateType::Loot,
			item_id,
			quantity
		});
		return;
	}
	if (!m_achievement_state) {
		return;
	}
	m_achievement_state->ProcessEvent(EQ::Achievements::EventType::LootItem, item_id, 0, quantity);
}

void Client::UpdateAchievementForOwnItem(uint32 item_id)
{
	(void) item_id;
	// Reconcile once the inventory operation reaches its durable final state.
	m_achievement_inventory_update_pending = true;
}

void Client::BeginAchievementInventoryTransaction()
{
	if (!m_achievement_inventory_transaction_depth) {
		m_achievement_inventory_transaction_update_checkpoint =
			m_deferred_achievement_state_updates.size();
		m_achievement_inventory_transaction_failed = false;
	}
	m_achievement_inventory_update_pending = true;
	++m_achievement_inventory_transaction_depth;
}

void Client::EndAchievementInventoryTransaction(bool committed)
{
	if (!m_achievement_inventory_transaction_depth) {
		return;
	}

	m_achievement_inventory_transaction_failed =
		m_achievement_inventory_transaction_failed || !committed;
	--m_achievement_inventory_transaction_depth;
	if (m_achievement_inventory_transaction_depth) {
		return;
	}

	if (
		m_achievement_inventory_transaction_failed &&
		m_achievement_inventory_transaction_update_checkpoint <
			m_deferred_achievement_state_updates.size()
	) {
		m_deferred_achievement_state_updates.erase(
			m_deferred_achievement_state_updates.begin() +
				m_achievement_inventory_transaction_update_checkpoint,
			m_deferred_achievement_state_updates.end()
		);
	}
	m_achievement_inventory_transaction_update_checkpoint = 0;
	m_achievement_inventory_transaction_failed = false;
}

void Client::UpdateAchievementForTradeskill(uint32 recipe_id)
{
	if (ShouldDeferAchievementStateUpdate()) {
		QueueAchievementStateUpdate({DeferredAchievementStateUpdateType::Tradeskill, recipe_id});
		return;
	}
	if (!m_achievement_state) {
		return;
	}
	m_achievement_state->ProcessEvent(EQ::Achievements::EventType::TradeskillSuccess, recipe_id);
}

void Client::UpdateAchievementForSkill(uint32 skill_id, uint32 value)
{
	if (ShouldDeferAchievementStateUpdate()) {
		QueueAchievementStateUpdate({
			DeferredAchievementStateUpdateType::Skill,
			skill_id,
			value
		});
		return;
	}
	if (!m_achievement_state) {
		return;
	}
	m_achievement_state->ProcessEvent(
		EQ::Achievements::EventType::SkillValue,
		skill_id,
		0,
		value
	);
	m_achievement_state->ProcessEvent(
		EQ::Achievements::EventType::SkillCap,
		skill_id
	);
}

void Client::UpdateAchievementForAA(uint32 spent_points)
{
	if (ShouldDeferAchievementStateUpdate()) {
		QueueAchievementStateUpdate({
			DeferredAchievementStateUpdateType::AlternateAdvancement,
			spent_points
		});
		return;
	}
	if (!m_achievement_state) {
		return;
	}
	m_achievement_state->ProcessEvent(
		EQ::Achievements::EventType::AlternateAdvancement,
		0,
		0,
		spent_points
	);
}

uint32 Client::GetAchievementAAPointsSpent()
{
	uint64_t spent_points = m_epp.expended_aa;
	for (const auto &[ability_id, rank_state] : aa_ranks) {
		auto *ability = zone->GetAlternateAdvancementAbility(ability_id);
		if (!ability || rank_state.first == 0) {
			continue;
		}
		const auto *rank = ability->GetRankByPointsSpent(rank_state.first);
		if (rank && rank->total_cost > 0) {
			spent_points += static_cast<uint64_t>(rank->total_cost);
		}
	}
	return static_cast<uint32>(std::min<uint64_t>(
		spent_points,
		std::numeric_limits<uint32>::max()
	));
}

bool Client::SetAchievementProgress(
	uint32 achievement_id,
	uint8 component_type,
	uint32 component_id,
	uint32 value,
	bool additive
)
{
	if (
		component_type > 2 ||
		!AchievementManager::Instance().FindDefinitionIndex(achievement_id) ||
		!AchievementManager::Instance().FindComponentIndex(
			achievement_id,
			component_type,
			component_id
		)
	) {
		return false;
	}
	if (ShouldDeferAchievementStateUpdate()) {
		QueueAchievementStateUpdate({
			DeferredAchievementStateUpdateType::SetProgress,
			achievement_id,
			component_type,
			component_id,
			value,
			additive
		});
		return true;
	}
	if (!m_achievement_state) {
		return false;
	}
	return m_achievement_state->SetProgress(
		achievement_id,
		component_type,
		component_id,
		value,
		additive
	);
}

bool Client::CompleteAchievement(uint32 achievement_id)
{
	if (!AchievementManager::Instance().FindDefinitionIndex(achievement_id)) {
		return false;
	}
	if (m_achievement_state && m_achievement_state->HasCompleted(achievement_id)) {
		return false;
	}
	if (ShouldDeferAchievementStateUpdate()) {
		QueueAchievementStateUpdate({
			DeferredAchievementStateUpdateType::Complete,
			achievement_id
		});
		return true;
	}
	if (!m_achievement_state) {
		return false;
	}
	return m_achievement_state->Complete(achievement_id);
}

bool Client::ResetAchievement(uint32 achievement_id, bool reset_rewards)
{
	return
		m_achievement_state &&
		m_achievement_state->Reset(achievement_id, reset_rewards);
}
