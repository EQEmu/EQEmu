#include "zone/reward_selection.h"

#include "common/achievements.h"
#include "common/eq_packet.h"
#include "common/rulesys.h"
#include "zone/client.h"
#include "zone/zone.h"
#include "zone/zonedb.h"

#include <algorithm>
#include <cstring>
#include <fmt/format.h>
#include <limits>
#include <memory>
#include <unordered_set>

static constexpr uint32_t kRewardRequestRateLimitMs = 500;
static constexpr uint32_t kRewardItemRequestRateLimitMs = 100;
static constexpr uint32_t kRoF2PlayerFlagsStringId = 3689;

static uint32_t RewardDisplayValue(uint64_t value)
{
	return static_cast<uint32_t>(std::min<uint64_t>(
		value,
		std::numeric_limits<uint32_t>::max()
	));
}

static EQ::Achievements::RewardDisplayEntry BuildDisplayEntry(
	const RewardSelectionReward &reward
)
{
	using namespace EQ::Achievements;

	RewardDisplayEntry entry;
	entry.fields[0] = static_cast<uint32_t>(reward.entry_id);
	entry.description = reward.description;

	switch (reward.type) {
	case RewardSelectionRewardType::Item: {
		entry.wire_type = RewardWireType::Item;
		const auto item = database.GetItem(reward.data_id);
		entry.items.push_back({
			reward.data_id,
			RewardDisplayValue(reward.amount),
			static_cast<uint8_t>(item && item->LoreFlag),
			item && item->LoreFlag ? static_cast<uint32_t>(item->LoreGroup) : 0,
			item ? item->Name : fmt::format("Item {}", reward.data_id)
		});
		if (entry.description.empty()) {
			entry.description = entry.items.front().text;
		}
		break;
	}
	case RewardSelectionRewardType::Experience:
		entry.wire_type = RewardWireType::Experience;
		entry.fields[1] = 3690;
		if (entry.description.empty()) {
			entry.description = fmt::format("{} Experience", reward.amount);
		}
		break;
	case RewardSelectionRewardType::AlternateAdvancement:
		entry.wire_type = RewardWireType::AlternateAdvancementPoints;
		entry.values[0] = RewardDisplayValue(reward.amount);
		if (entry.description.empty()) {
			entry.description = fmt::format(
				"{} Alternate Advancement Point{}",
				reward.amount,
				reward.amount == 1 ? "" : "s"
			);
		}
		break;
	case RewardSelectionRewardType::Copper:
		entry.wire_type = RewardWireType::Money;
		entry.values[0] = RewardDisplayValue(reward.amount / 1000);
		entry.values[1] = RewardDisplayValue((reward.amount % 1000) / 100);
		entry.values[2] = RewardDisplayValue((reward.amount % 100) / 10);
		entry.values[3] = RewardDisplayValue(reward.amount % 10);
		if (entry.description.empty()) {
			entry.description = "Coin";
		}
		break;
	case RewardSelectionRewardType::AlternateCurrency:
		entry.wire_type = RewardWireType::GenericPoints;
		entry.values[2] = RewardDisplayValue(reward.amount);
		entry.values[3] = reward.data_id;
		if (entry.description.empty()) {
			entry.description = fmt::format(
				"{} Alternate Currency {}",
				reward.amount,
				reward.data_id
			);
		}
		break;
	case RewardSelectionRewardType::Title:
		entry.wire_type = RewardWireType::Text;
		entry.fields[1] = kRoF2PlayerFlagsStringId;
		if (entry.description.empty()) {
			entry.description = fmt::format("Title {}", reward.data_id);
		}
		break;
	}

	return entry;
}

static EQ::Achievements::RewardDisplaySet BuildDisplaySet(
	const RewardSelectionSession &session
)
{
	EQ::Achievements::RewardDisplaySet display;
	display.pending_reward_id = session.pending_reward_id;
	display.reward_set_id = session.reward_set.reward_set_id;
	display.title = session.reward_set.title;
	display.subsets.reserve(session.reward_set.options.size());

	for (const auto &option : session.reward_set.options) {
		EQ::Achievements::RewardDisplaySubset subset;
		subset.subset_id = option.wire_option_id;
		subset.common_to_all = option.common_to_all;
		subset.flags = option.flags;
		subset.option_label = option.label;
		subset.entries.reserve(option.rewards.size());
		for (const auto &reward : option.rewards) {
			subset.entries.push_back(BuildDisplayEntry(reward));
		}
		display.subsets.emplace_back(std::move(subset));
	}

	return display;
}

static bool SupportsRewardSelection(const Client &client)
{
	return client.ClientVersion() == EQ::versions::ClientVersion::RoF2;
}

static EmuOpcode RewardSelectionOpcode(RewardSelectionChannel channel)
{
	return channel == RewardSelectionChannel::Claimable
		? OP_AchievementReward
		: OP_RewardSelection;
}

ClientRewardSelection::ClientRewardSelection(Client &client)
	: m_client(client)
{
}

bool ClientRewardSelection::ValidateSession(
	const RewardSelectionSession &session
)
{
	if (
		session.source.source == RewardSelectionSource::Unknown ||
		!session.source.source_id ||
		!session.pending_reward_id ||
		!session.reward_set.reward_set_id ||
		session.reward_set.options.empty()
	) {
		return false;
	}

	std::unordered_set<uint32_t> option_ids;
	std::unordered_set<uint64_t> entry_ids;
	for (const auto &option : session.reward_set.options) {
		if (
			!option.option_id ||
			!option_ids.insert(option.option_id).second ||
			option.rewards.empty()
		) {
			return false;
		}

		for (const auto &reward : option.rewards) {
			if (
				!reward.entry_id ||
				reward.entry_id > std::numeric_limits<uint32_t>::max() ||
				!entry_ids.insert(reward.entry_id).second ||
				!reward.amount
			) {
				return false;
			}
			if (
				(reward.type == RewardSelectionRewardType::Item ||
					reward.type == RewardSelectionRewardType::AlternateCurrency ||
					reward.type == RewardSelectionRewardType::Title) &&
				!reward.data_id
			) {
				return false;
			}
			if (
				reward.type == RewardSelectionRewardType::Experience &&
				reward.data_id > static_cast<uint32_t>(
					RewardSelectionExperienceMode::NormalOnly
				)
			) {
				return false;
			}
		}
	}

	return true;
}

bool ClientRewardSelection::SameSession(
	const RewardSelectionSession &left,
	const RewardSelectionSession &right
)
{
	return
		left.channel == right.channel &&
		left.source.source == right.source.source &&
		left.source.source_id == right.source.source_id &&
		left.source.source_instance_id == right.source.source_instance_id &&
		left.pending_reward_id == right.pending_reward_id &&
		left.reward_set.reward_set_id == right.reward_set.reward_set_id;
}

bool ClientRewardSelection::AssignWireOptionIds(ChannelState &state)
{
	uint64_t next_id = 1;
	for (auto &session : state.sessions) {
		for (auto &option : session.reward_set.options) {
			if (next_id > std::numeric_limits<uint32_t>::max()) {
				return false;
			}
			option.wire_option_id = static_cast<uint32_t>(next_id++);
		}
	}
	return true;
}

ClientRewardSelection::ChannelState &ClientRewardSelection::State(
	RewardSelectionChannel channel
)
{
	return channel == RewardSelectionChannel::Claimable
		? m_claimable_channel
		: m_preview_channel;
}

const ClientRewardSelection::ChannelState &ClientRewardSelection::State(
	RewardSelectionChannel channel
) const
{
	return channel == RewardSelectionChannel::Claimable
		? m_claimable_channel
		: m_preview_channel;
}

bool ClientRewardSelection::Open(const RewardSelectionSession &session)
{
	return Open(std::vector<RewardSelectionSession>{session});
}

bool ClientRewardSelection::Open(
	const std::vector<RewardSelectionSession> &sessions
)
{
	if (!SupportsRewardSelection(m_client) || sessions.empty()) {
		return false;
	}

	const auto channel = sessions.front().channel;
	for (const auto &session : sessions) {
		if (session.channel != channel || !ValidateSession(session)) {
			return false;
		}
		if (
			channel == RewardSelectionChannel::Claimable &&
			std::none_of(
				session.reward_set.options.begin(),
				session.reward_set.options.end(),
				[](const RewardSelectionOption &option) {
					return !option.common_to_all;
				}
			)
		) {
			return false;
		}
	}

	auto &state = State(channel);
	if (state.claim_in_flight) {
		return false;
	}

	if (channel == RewardSelectionChannel::Preview) {
		if (sessions.size() != 1) {
			return false;
		}
		state.sessions = sessions;
	}
	else {
		for (const auto &session : sessions) {
			const auto found = std::find_if(
				state.sessions.begin(),
				state.sessions.end(),
				[&session](const RewardSelectionSession &existing) {
					return SameSession(existing, session);
				}
			);
			if (found == state.sessions.end()) {
				state.sessions.push_back(session);
			}
			else {
				*found = session;
			}
		}
	}
	if (!AssignWireOptionIds(state)) {
		return false;
	}

	++state.session_generation;
	if (!state.session_generation) {
		++state.session_generation;
	}
	state.claim_in_flight = false;
	return SendSessions(channel);
}

bool ClientRewardSelection::SendSessions(RewardSelectionChannel channel)
{
	const auto &state = State(channel);
	SerializeBuffer data;
	if (state.sessions.empty()) {
		data = EQ::Achievements::SerializeRewardDisplayClear();
	}
	else if (channel == RewardSelectionChannel::Preview) {
		const auto display = BuildDisplaySet(state.sessions.front());
		data = EQ::Achievements::SerializeRewardDisplay(&display);
	}
	else {
		std::vector<EQ::Achievements::RewardDisplaySet> displays;
		displays.reserve(state.sessions.size());
		for (const auto &session : state.sessions) {
			displays.push_back(BuildDisplaySet(session));
		}
		data = EQ::Achievements::SerializeRewardDisplays(displays);
	}

	auto packet = new EQApplicationPacket(
		RewardSelectionOpcode(channel),
		std::move(data)
	);
	m_client.FastQueuePacket(&packet);
	return true;
}

void ClientRewardSelection::Clear(
	RewardSelectionChannel channel,
	bool notify_client
)
{
	auto &state = State(channel);
	state.sessions.clear();
	state.claim_in_flight = false;
	++state.session_generation;
	if (!state.session_generation) {
		++state.session_generation;
	}

	if (!notify_client || !SupportsRewardSelection(m_client)) {
		return;
	}

	SendSessions(channel);
}

void ClientRewardSelection::ClearSource(
	RewardSelectionChannel channel,
	RewardSelectionSource source,
	uint64_t source_id,
	bool notify_client
)
{
	auto &state = State(channel);
	const auto old_size = state.sessions.size();
	std::erase_if(
		state.sessions,
		[source, source_id](const RewardSelectionSession &session) {
			return
				session.source.source == source &&
				(!source_id || session.source.source_id == source_id);
		}
	);
	if (state.sessions.size() == old_size) {
		return;
	}
	state.claim_in_flight = false;
	++state.session_generation;
	if (!state.session_generation) {
		++state.session_generation;
	}
	if (notify_client && SupportsRewardSelection(m_client)) {
		SendSessions(channel);
	}
}

void ClientRewardSelection::ClearSource(
	RewardSelectionSource source,
	uint64_t source_id,
	bool notify_client
)
{
	for (const auto channel : {
		RewardSelectionChannel::Claimable,
		RewardSelectionChannel::Preview
	}) {
		ClearSource(channel, source, source_id, notify_client);
	}
}

void ClientRewardSelection::ClearAll(bool notify_client)
{
	Clear(RewardSelectionChannel::Claimable, notify_client);
	Clear(RewardSelectionChannel::Preview, notify_client);
}

bool ClientRewardSelection::HasActiveSession(
	RewardSelectionChannel channel
) const
{
	return !State(channel).sessions.empty();
}

const RewardSelectionSession *ClientRewardSelection::ActiveSession(
	RewardSelectionChannel channel
) const
{
	const auto &sessions = State(channel).sessions;
	return sessions.empty() ? nullptr : &sessions.front();
}

const RewardSelectionSession *ClientRewardSelection::FindSession(
	RewardSelectionChannel channel,
	RewardSelectionSource source,
	uint64_t source_id,
	uint64_t source_instance_id
) const
{
	const auto &sessions = State(channel).sessions;
	const auto found = std::find_if(
		sessions.begin(),
		sessions.end(),
		[source, source_id, source_instance_id](
			const RewardSelectionSession &session
		) {
			return
				session.source.source == source &&
				session.source.source_id == source_id &&
				(!source_instance_id ||
					session.source.source_instance_id == source_instance_id);
		}
	);
	return found == sessions.end() ? nullptr : &*found;
}

RewardSelectionPacketResult ClientRewardSelection::HandlePacket(
	const EQApplicationPacket &app,
	RewardSelectionChannel channel
)
{
	using EQ::RewardSelection::Action;

	RewardSelectionPacketResult result;
	if (!SupportsRewardSelection(m_client) || app.size < sizeof(uint32_t)) {
		return result;
	}

	Action action{};
	std::memcpy(&action, app.pBuffer, sizeof(action));
	if (
		(channel == RewardSelectionChannel::Claimable &&
			action == Action::TaskView) ||
		(channel == RewardSelectionChannel::Preview &&
			(action == Action::AchievementView ||
				action == Action::Claim))
	) {
		return result;
	}
	if (
		((action == Action::TaskView ||
			action == Action::AchievementView) &&
			app.size != sizeof(uint32_t) * 2) ||
		(action == Action::Pending &&
			app.size != sizeof(uint32_t)) ||
		(action == Action::InspectItem && app.size != sizeof(uint32_t) * 5) ||
		(action == Action::Claim && app.size != sizeof(uint32_t) * 5)
	) {
		return result;
	}

	if (
		action == Action::TaskView ||
		action == Action::AchievementView ||
		action == Action::Pending
	) {
		auto &state = State(channel);
		if (
			state.request_rate_limit.Enabled() &&
			!state.request_rate_limit.Check(false)
		) {
			result.type = RewardSelectionPacketResultType::Handled;
			return result;
		}
		state.request_rate_limit.Start(kRewardRequestRateLimitMs);

		if (action == Action::Pending) {
			result.requested_source = RewardSelectionSource::General;
			result.type = RewardSelectionPacketResultType::PendingRequested;
			return result;
		}

		std::memcpy(
			&result.requested_id,
			app.pBuffer + sizeof(action),
			sizeof(result.requested_id)
		);
		result.requested_source =
			action == Action::TaskView
				? RewardSelectionSource::Task
				: RewardSelectionSource::Achievement;
		result.type = RewardSelectionPacketResultType::ViewRequested;
		return result;
	}

	if (action == Action::InspectItem) {
		auto &state = State(channel);
		if (
			state.item_request_rate_limit.Enabled() &&
			!state.item_request_rate_limit.Check(false)
		) {
			result.type = RewardSelectionPacketResultType::Handled;
			return result;
		}
		state.item_request_rate_limit.Start(kRewardItemRequestRateLimitMs);

		uint32_t reward_set_id = 0;
		uint32_t option_id = 0;
		uint32_t reward_entry_id = 0;
		uint32_t item_id = 0;
		std::memcpy(
			&reward_set_id,
			app.pBuffer + sizeof(uint32_t),
			sizeof(reward_set_id)
		);
		std::memcpy(
			&option_id,
			app.pBuffer + sizeof(uint32_t) * 2,
			sizeof(option_id)
		);
		std::memcpy(
			&reward_entry_id,
			app.pBuffer + sizeof(uint32_t) * 3,
			sizeof(reward_entry_id)
		);
		std::memcpy(
			&item_id,
			app.pBuffer + sizeof(uint32_t) * 4,
			sizeof(item_id)
		);
		SendItemInspect(
			channel,
			reward_set_id,
			option_id,
			reward_entry_id,
			item_id
		);
		result.type = RewardSelectionPacketResultType::Handled;
		return result;
	}

	if (action != Action::Claim) {
		return result;
	}

	uint32_t pending_reward_id = 0;
	uint32_t reward_set_id = 0;
	uint32_t selected_wire_option_id = 0;
	std::memcpy(
		&pending_reward_id,
		app.pBuffer + sizeof(uint32_t),
		sizeof(pending_reward_id)
	);
	std::memcpy(
		&reward_set_id,
		app.pBuffer + sizeof(uint32_t) * 2,
		sizeof(reward_set_id)
	);
	std::memcpy(
		&selected_wire_option_id,
		app.pBuffer + sizeof(uint32_t) * 3,
		sizeof(selected_wire_option_id)
	);

	result.claim = ResolveClaim(
		channel,
		pending_reward_id,
		reward_set_id,
		selected_wire_option_id
	);
	if (!result.claim) {
		SendClaimReply(
			channel,
			pending_reward_id,
			reward_set_id,
			selected_wire_option_id,
			false
		);
		result.type = RewardSelectionPacketResultType::Handled;
		return result;
	}

	result.type = RewardSelectionPacketResultType::ClaimRequested;
	return result;
}

bool ClientRewardSelection::SendItemInspect(
	RewardSelectionChannel channel,
	uint32_t reward_set_id,
	uint32_t option_id,
	uint32_t reward_entry_id,
	uint32_t item_id
)
{
	auto &state = State(channel);
	if (
		state.sessions.empty() ||
		state.claim_in_flight ||
		!item_id
	) {
		return false;
	}

	const RewardSelectionReward *matched_reward = nullptr;
	for (const auto &session : state.sessions) {
		if (session.reward_set.reward_set_id != reward_set_id) {
			continue;
		}
		for (const auto &option : session.reward_set.options) {
			if (option.wire_option_id != option_id) {
				continue;
			}
			const auto reward = std::find_if(
				option.rewards.begin(),
				option.rewards.end(),
				[reward_entry_id, item_id](
					const RewardSelectionReward &candidate
				) {
					return
						candidate.entry_id == reward_entry_id &&
						candidate.type == RewardSelectionRewardType::Item &&
						candidate.data_id == item_id;
				}
			);
			if (reward != option.rewards.end()) {
				matched_reward = &*reward;
			}
			break;
		}
		if (matched_reward) {
			break;
		}
	}
	if (!matched_reward) {
		return false;
	}

	const auto charges = matched_reward->amount <=
		static_cast<uint64_t>(std::numeric_limits<int16_t>::max())
		? static_cast<int16_t>(matched_reward->amount)
		: 0;
	std::unique_ptr<EQ::ItemInstance> instance(
		database.CreateItem(item_id, charges)
	);
	if (!instance) {
		return false;
	}

	const auto internal_item = instance->Serialize(0);
	auto packet = new EQApplicationPacket(
		RewardSelectionOpcode(channel),
		sizeof(uint32_t) * 2 + internal_item.size()
	);
	const auto action = EQ::RewardSelection::Action::InspectItem;
	std::memcpy(packet->pBuffer, &action, sizeof(action));
	std::memcpy(
		packet->pBuffer + sizeof(action),
		&item_id,
		sizeof(item_id)
	);
	std::memcpy(
		packet->pBuffer + sizeof(action) + sizeof(item_id),
		internal_item.data(),
		internal_item.size()
	);
	m_client.FastQueuePacket(&packet);
	return true;
}

std::optional<ResolvedRewardSelectionClaim>
ClientRewardSelection::ResolveClaim(
	RewardSelectionChannel channel,
	uint32_t pending_reward_id,
	uint32_t reward_set_id,
	uint32_t selected_wire_option_id
)
{
	auto &state = State(channel);
	if (
		channel != RewardSelectionChannel::Claimable ||
		state.sessions.empty() ||
		state.claim_in_flight ||
		!pending_reward_id ||
		!reward_set_id
	) {
		return std::nullopt;
	}
	const auto session = std::find_if(
		state.sessions.begin(),
		state.sessions.end(),
		[pending_reward_id, reward_set_id, selected_wire_option_id](
			const RewardSelectionSession &candidate
		) {
			return
				candidate.channel == RewardSelectionChannel::Claimable &&
				candidate.pending_reward_id == pending_reward_id &&
				candidate.reward_set.reward_set_id == reward_set_id &&
				std::any_of(
					candidate.reward_set.options.begin(),
					candidate.reward_set.options.end(),
					[selected_wire_option_id](
						const RewardSelectionOption &option
					) {
						return
							!option.common_to_all &&
							option.wire_option_id == selected_wire_option_id;
					}
				);
		}
	);
	if (session == state.sessions.end()) {
		return std::nullopt;
	}

	const RewardSelectionOption *selected_option = nullptr;
	ResolvedRewardSelectionClaim resolution;
	resolution.session = *session;
	resolution.selected_wire_option_id = selected_wire_option_id;
	resolution.session_generation = state.session_generation;

	for (const auto &option : session->reward_set.options) {
		if (option.common_to_all) {
			resolution.rewards.insert(
				resolution.rewards.end(),
				option.rewards.begin(),
				option.rewards.end()
			);
		}
		else if (option.wire_option_id == selected_wire_option_id) {
			selected_option = &option;
			resolution.selected_option_id = option.option_id;
		}
	}
	if (!selected_option) {
		return std::nullopt;
	}

	resolution.rewards.insert(
		resolution.rewards.end(),
		selected_option->rewards.begin(),
		selected_option->rewards.end()
	);
	state.claim_in_flight = true;
	return resolution;
}

void ClientRewardSelection::CompleteClaim(
	const ResolvedRewardSelectionClaim &claim,
	RewardSelectionDeliveryResult result
)
{
	const auto channel = claim.session.channel;
	if (channel != RewardSelectionChannel::Claimable) {
		return;
	}
	auto &state = State(channel);
	const auto session = std::find_if(
		state.sessions.begin(),
		state.sessions.end(),
		[&claim](const RewardSelectionSession &candidate) {
			return SameSession(candidate, claim.session);
		}
	);
	const bool matches_active =
		session != state.sessions.end() &&
		state.claim_in_flight &&
		claim.session_generation == state.session_generation;
	const bool delivered =
		matches_active &&
		result == RewardSelectionDeliveryResult::Delivered;

	SendClaimReply(
		channel,
		claim.session.pending_reward_id,
		claim.session.reward_set.reward_set_id,
		claim.selected_wire_option_id,
		delivered
	);

	if (!matches_active) {
		return;
	}
	if (
		result == RewardSelectionDeliveryResult::Ambiguous ||
		result == RewardSelectionDeliveryResult::RetryableFailure ||
		delivered
	) {
		state.sessions.erase(session);
		++state.session_generation;
		if (!state.session_generation) {
			++state.session_generation;
		}
	}
	state.claim_in_flight = false;
	if (result == RewardSelectionDeliveryResult::Ambiguous) {
		SendSessions(channel);
	}
}

void ClientRewardSelection::SendClaimReply(
	RewardSelectionChannel channel,
	uint32_t pending_reward_id,
	uint32_t reward_set_id,
	uint32_t selected_option_id,
	bool success
)
{
	auto data = EQ::Achievements::SerializeRewardClaimReply(
		pending_reward_id,
		reward_set_id,
		selected_option_id,
		success
	);
	auto packet = new EQApplicationPacket(RewardSelectionOpcode(channel), std::move(data));
	m_client.FastQueuePacket(&packet);
}

RewardSelectionDeliveryResult ClientRewardSelection::GrantBatch(
	Client &client,
	const std::vector<RewardSelectionReward> &rewards,
	const RewardSelectionDeliveryPolicy &policy
)
{
	auto batch_result = RewardSelectionDeliveryResult::Delivered;
	for (const auto &reward : rewards) {
		const auto result = GrantReward(client, reward, policy);
		if (result == RewardSelectionDeliveryResult::Ambiguous) {
			batch_result = RewardSelectionDeliveryResult::Ambiguous;
		}
		else if (
			result == RewardSelectionDeliveryResult::RetryableFailure &&
			batch_result == RewardSelectionDeliveryResult::Delivered
		) {
			batch_result = RewardSelectionDeliveryResult::RetryableFailure;
		}
	}
	return batch_result;
}

RewardSelectionDeliveryResult ClientRewardSelection::GrantReward(
	Client &client,
	const RewardSelectionReward &reward,
	const RewardSelectionDeliveryPolicy &policy
)
{
	const auto amount = reward.amount;
	switch (reward.type) {
	case RewardSelectionRewardType::Item:
		if (
			!reward.data_id ||
			!amount ||
			amount > static_cast<uint64_t>(std::numeric_limits<int16_t>::max()) ||
			client.GetInv().CursorSize() >= EQ::invbag::CURSOR_BAG_COUNT
		) {
			return RewardSelectionDeliveryResult::RetryableFailure;
		}
		{
			bool persistence_succeeded = false;
			const auto summoned = client.SummonItem(
				reward.data_id,
				static_cast<int16_t>(amount),
				0,
				0,
				0,
				0,
				0,
				0,
				false,
				EQ::invslot::slotCursor,
				0,
				0,
				0,
				&persistence_succeeded
			);
			if (!summoned) {
				return RewardSelectionDeliveryResult::RetryableFailure;
			}
			return persistence_succeeded
				? RewardSelectionDeliveryResult::Delivered
				: RewardSelectionDeliveryResult::Ambiguous;
		}
	case RewardSelectionRewardType::Experience:
		if (
			!amount ||
			amount > static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()) ||
			reward.data_id > static_cast<uint32_t>(
				RewardSelectionExperienceMode::NormalOnly
			) ||
			(policy.require_experience_enabled && !client.IsEXPEnabled())
		) {
			return RewardSelectionDeliveryResult::RetryableFailure;
		}
		if (
			reward.data_id ==
			static_cast<uint32_t>(RewardSelectionExperienceMode::NormalOnly)
		) {
			client.SetEXP(
				policy.experience_source,
				static_cast<uint64_t>(client.GetEXP()) + amount,
				client.GetAAXP()
			);
		}
		else {
			client.AddEXP(policy.experience_source, amount);
		}
		return client.Save(2)
			? RewardSelectionDeliveryResult::Delivered
			: RewardSelectionDeliveryResult::Ambiguous;
	case RewardSelectionRewardType::AlternateAdvancement: {
		const auto current_points = client.GetAAPoints();
		const auto spent_points = client.GetSpentAA();
		if (current_points < 0 || spent_points < 0) {
			return RewardSelectionDeliveryResult::RetryableFailure;
		}
		const auto current_total =
			static_cast<uint64_t>(current_points) +
			static_cast<uint64_t>(spent_points);
		if (
			!amount ||
			current_total > static_cast<uint64_t>(std::numeric_limits<int>::max()) ||
			amount >
				static_cast<uint64_t>(std::numeric_limits<int>::max()) -
					current_total
		) {
			return RewardSelectionDeliveryResult::RetryableFailure;
		}
		client.AddAAPoints(static_cast<uint32_t>(amount));
		return client.Save(2)
			? RewardSelectionDeliveryResult::Delivered
			: RewardSelectionDeliveryResult::Ambiguous;
	}
	case RewardSelectionRewardType::Copper:
		if (!amount) {
			return RewardSelectionDeliveryResult::RetryableFailure;
		}
		{
			const auto max_currency =
				static_cast<uint64_t>(std::numeric_limits<int32_t>::max());
			const auto platinum = amount / 1000;
			const auto gold = (amount % 1000) / 100;
			const auto silver = (amount % 100) / 10;
			const auto copper = amount % 10;
			if (
				client.GetPlatinum() > max_currency ||
				client.GetGold() > max_currency ||
				client.GetSilver() > max_currency ||
				client.GetCopper() > max_currency ||
				platinum > max_currency - client.GetPlatinum() ||
				gold > max_currency - client.GetGold() ||
				silver > max_currency - client.GetSilver() ||
				copper > max_currency - client.GetCopper()
			) {
				return RewardSelectionDeliveryResult::RetryableFailure;
			}
		}
		client.AddMoneyToPP(amount, true);
		return client.SaveCurrency()
			? RewardSelectionDeliveryResult::Delivered
			: RewardSelectionDeliveryResult::Ambiguous;
	case RewardSelectionRewardType::AlternateCurrency:
		if (
			!reward.data_id ||
			!amount ||
			amount > static_cast<uint64_t>(std::numeric_limits<int>::max()) ||
			!zone ||
			!zone->DoesAlternateCurrencyExist(reward.data_id)
		) {
			return RewardSelectionDeliveryResult::RetryableFailure;
		}
		{
			const auto current = client.GetAlternateCurrencyValue(reward.data_id);
			if (
				current > static_cast<uint32_t>(std::numeric_limits<int>::max()) ||
				amount >
					static_cast<uint64_t>(std::numeric_limits<int>::max()) -
						static_cast<int>(current)
			) {
				return RewardSelectionDeliveryResult::RetryableFailure;
			}
			const auto expected =
				static_cast<int>(current) + static_cast<int>(amount);
			bool persistence_succeeded = false;
			const auto result = client.AddAlternateCurrencyValue(
				reward.data_id,
				static_cast<int>(amount),
				true,
				&persistence_succeeded
			);
			if (result != expected) {
				return client.GetAlternateCurrencyValue(reward.data_id) == current
					? RewardSelectionDeliveryResult::RetryableFailure
					: RewardSelectionDeliveryResult::Ambiguous;
			}
			return persistence_succeeded
				? RewardSelectionDeliveryResult::Delivered
				: RewardSelectionDeliveryResult::Ambiguous;
		}
	case RewardSelectionRewardType::Title:
		if (
			!reward.data_id ||
			reward.data_id >
				static_cast<uint32_t>(std::numeric_limits<int>::max())
		) {
			return RewardSelectionDeliveryResult::RetryableFailure;
		}
		client.EnableTitle(static_cast<int>(reward.data_id));
		return client.CheckTitle(static_cast<int>(reward.data_id))
			? RewardSelectionDeliveryResult::Delivered
			: RewardSelectionDeliveryResult::RetryableFailure;
	}

	return RewardSelectionDeliveryResult::RetryableFailure;
}
