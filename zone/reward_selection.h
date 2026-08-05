#pragma once

#include "../common/eq_constants.h"
#include "../common/timer.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

class Client;
class EQApplicationPacket;

// RoF2 shares Select Reward across providers, with separate claimable and
// read-only preview channels.
enum class RewardSelectionSource : uint8_t {
	Unknown     = 0,
	Achievement = 1,
	Task        = 2,
	General     = 3
};

enum class RewardSelectionChannel : uint8_t {
	Claimable,
	Preview
};

// RoF2 actions not already declared by the shared achievement wire serializer.
inline constexpr uint32_t RewardSelectionActionTaskView        = 4;
inline constexpr uint32_t RewardSelectionActionAchievementView = 5;
inline constexpr uint32_t RewardSelectionActionPending         = 6;

enum class RewardSelectionRewardType : uint8_t {
	Item                 = 0,
	Experience           = 1,
	AlternateAdvancement = 2,
	Copper               = 3,
	AlternateCurrency    = 4,
	Title                = 5
};

// Experience rewards use data_id to select normal-only or normal/AA allocation.
enum class RewardSelectionExperienceMode : uint32_t {
	Default    = 0,
	NormalOnly = 1
};

struct RewardSelectionReward {
	// RoF2 echoes only the low 32 bits; loaders reject wider entry IDs.
	uint64_t                  entry_id = 0;
	RewardSelectionRewardType type = RewardSelectionRewardType::Item;
	uint32_t                  data_id = 0;
	uint64_t                  amount = 0;
	std::string               description;
};

struct RewardSelectionOption {
	uint32_t                           option_id = 0;
	// RoF2 indexes option details across the entire reward manager. This ID is
	// assigned per displayed collection and mapped back to option_id on input.
	uint32_t                           wire_option_id = 0;
	uint32_t                           sequence = 0;
	std::string                        label;
	bool                               common_to_all = false;
	uint8_t                            flags = 0;
	std::vector<RewardSelectionReward> rewards;
};

struct RewardSelectionSet {
	uint32_t                           reward_set_id = 0;
	std::string                        title;
	std::vector<RewardSelectionOption> options;
};

struct RewardSelectionSourceKey {
	RewardSelectionSource source = RewardSelectionSource::Unknown;
	uint64_t              source_id = 0;
	// Distinguishes repeatable occurrences; achievements normally use zero.
	uint64_t              source_instance_id = 0;
};

struct RewardSelectionSession {
	RewardSelectionSourceKey source;
	RewardSelectionChannel   channel = RewardSelectionChannel::Preview;
	uint32_t                 pending_reward_id = 0;
	RewardSelectionSet       reward_set;
};

enum class RewardSelectionDeliveryResult : uint8_t {
	Delivered,
	RetryableFailure,
	Ambiguous
};

struct RewardSelectionDeliveryPolicy {
	ExpSource experience_source = ExpSource::Quest;
	bool      require_experience_enabled = true;
	bool      require_quest_experience_rule = true;
};

struct ResolvedRewardSelectionClaim {
	RewardSelectionSession              session;
	// Provider/database identity used for authorization and persistence.
	uint32_t                            selected_option_id = 0;
	// Client identity echoed in the action-3 response.
	uint32_t                            selected_wire_option_id = 0;
	std::vector<RewardSelectionReward>  rewards;

	// Prevents a delayed result from completing a newer session.
	uint64_t session_generation = 0;
};

enum class RewardSelectionPacketResultType : uint8_t {
	Ignored,
	Handled,
	ViewRequested,
	PendingRequested,
	ClaimRequested
};

struct RewardSelectionPacketResult {
	RewardSelectionPacketResultType             type =
		RewardSelectionPacketResultType::Ignored;
	RewardSelectionSource                       requested_source =
		RewardSelectionSource::Unknown;
	// Achievement action 5 carries a zero-based definition index. Task action
	// 4 carries the task ID directly.
	uint32_t                                    requested_id = 0;
	std::optional<ResolvedRewardSelectionClaim> claim;
};

class ClientRewardSelection
{
public:
	explicit ClientRewardSelection(Client &client);

	// Copy the provider snapshot so content reloads cannot leave dangling state.
	bool Open(const RewardSelectionSession &session);
	bool Open(const std::vector<RewardSelectionSession> &sessions);
	void Clear(RewardSelectionChannel channel, bool notify_client = true);
	void ClearSource(
		RewardSelectionChannel channel,
		RewardSelectionSource source,
		uint64_t source_id = 0,
		bool notify_client = true
	);
	void ClearSource(
		RewardSelectionSource source,
		uint64_t source_id = 0,
		bool notify_client = true
	);
	void ClearAll(bool notify_client = true);
	bool HasActiveSession(RewardSelectionChannel channel) const;
	const RewardSelectionSession *ActiveSession(
		RewardSelectionChannel channel
	) const;
	const RewardSelectionSession *FindSession(
		RewardSelectionChannel channel,
		RewardSelectionSource source,
		uint64_t source_id,
		uint64_t source_instance_id = 0
	) const;

	// Handles item inspection; provider-owned view, pending, and claim requests
	// are returned for authorization and durable ledger work.
	RewardSelectionPacketResult HandlePacket(
		const EQApplicationPacket &app,
		RewardSelectionChannel channel
	);

	std::optional<ResolvedRewardSelectionClaim> ResolveClaim(
		RewardSelectionChannel channel,
		uint32_t pending_reward_id,
		uint32_t reward_set_id,
		uint32_t selected_wire_option_id
	);

	// Retryable claims release the snapshot for reopening; ambiguous claims close
	// without automatic retry.
	void CompleteClaim(
		const ResolvedRewardSelectionClaim &claim,
		RewardSelectionDeliveryResult result
	);

	static RewardSelectionDeliveryResult GrantReward(
		Client &client,
		const RewardSelectionReward &reward,
		const RewardSelectionDeliveryPolicy &policy = {}
	);
	static RewardSelectionDeliveryResult GrantBatch(
		Client &client,
		const std::vector<RewardSelectionReward> &rewards,
		const RewardSelectionDeliveryPolicy &policy = {}
	);

private:
	struct ChannelState {
		std::vector<RewardSelectionSession>    sessions;
		uint64_t                              session_generation = 0;
		bool                                  claim_in_flight = false;
		Timer                                 request_rate_limit;
		Timer                                 item_request_rate_limit;
	};

	static bool ValidateSession(const RewardSelectionSession &session);
	static bool SameSession(
		const RewardSelectionSession &left,
		const RewardSelectionSession &right
	);
	static bool AssignWireOptionIds(ChannelState &state);
	ChannelState &State(RewardSelectionChannel channel);
	const ChannelState &State(RewardSelectionChannel channel) const;
	bool SendSessions(RewardSelectionChannel channel);
	bool SendItemInspect(
		RewardSelectionChannel channel,
		uint32_t reward_set_id,
		uint32_t option_id,
		uint32_t reward_entry_id,
		uint32_t item_id
	);
	void SendClaimReply(
		RewardSelectionChannel channel,
		uint32_t pending_reward_id,
		uint32_t reward_set_id,
		uint32_t selected_option_id,
		bool success
	);

	Client       &m_client;
	ChannelState  m_claimable_channel;
	ChannelState  m_preview_channel;
};
