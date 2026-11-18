#pragma once

#include "common/achievements.h"
#include "common/timer.h"
#include "zone/reward_selection.h"

#include <cstddef>
#include <cstdint>
#include <deque>
#include <optional>
#include <unordered_set>
#include <vector>

class Client;

class ClientAchievementState
{
public:
	explicit ClientAchievementState(Client &client);

	bool Load(bool allow_disabled = false);
	void SendInitial();
	void PreservePendingNotificationsFrom(ClientAchievementState &previous);

	bool IsLoaded() const { return m_loaded; }
	bool HasCompleted(uint32_t achievement_id) const;
	std::optional<EQ::Achievements::Status> GetStatus(uint32_t achievement_id) const;
	std::optional<uint32_t> GetProgress(
		uint32_t achievement_id,
		uint8_t component_type,
		uint32_t component_id
	) const;
	bool NeedsOwnershipReconciliation() const;
	bool PassCastRestriction(uint32_t restriction_id) const;

	bool ProcessEvent(
		EQ::Achievements::EventType event_type,
		uint32_t target_id = 0,
		uint32_t target_id2 = 0,
		uint32_t value = 1,
		bool send_packets = true
	);

	bool SetProgress(
		uint32_t achievement_id,
		uint8_t component_type,
		uint32_t component_id,
		uint32_t value,
		bool additive = false,
		bool send_packets = true
	);

	bool Complete(uint32_t achievement_id, bool send_packets = true);
	bool Reset(uint32_t achievement_id, bool reset_rewards = false);
	void SendComparison(uint32_t achievement_id);
	void SendComparisonTo(Client &recipient, uint32_t achievement_id);
	void SendCompareSnapshotTo(Client &recipient) const;
	void SendRewardDisplay(uint32_t definition_index);
	bool RestorePendingRewardSelection();
	RewardSelectionDeliveryResult ClaimReward(
		uint32_t pending_reward_id,
		uint32_t reward_set_id,
		uint32_t selected_option_id
	);
	void ProcessPendingNotifications();
	void ProcessPendingRewards();
	bool DrainPendingStateUpdates(bool retry_blocked);

private:
	enum class RewardGrantResult : uint8_t {
		Delivered,
		RetryableFailure,
		Ambiguous
	};

	struct PendingCompletionNotification {
		uint32_t achievement_id;
		uint32_t guild_id;
	};

	void InitializeStates();
	bool DrainPendingStateUpdatesLocked(bool retry_blocked);
	bool Reconcile();
	void EvaluateAll(bool send_packets, bool ownership_is_fresh = false);
	bool EvaluateDefinition(
		size_t definition_index,
		bool send_packets,
		bool *persistence_succeeded = nullptr,
		bool ownership_is_fresh = false
	);
	bool PersistProgress(size_t definition_index, uint8_t component_type, size_t component_index);
	bool PersistCompletion(size_t definition_index, uint32_t completed_at);
	void QueueRewards(uint32_t achievement_id);
	void QueueCompletionNotification(uint32_t achievement_id);
	RewardGrantResult GrantRewardBatch(
		uint32_t achievement_id,
		const std::vector<RewardSelectionReward> &rewards
	);
	RewardGrantResult GrantTrackedReward(
		uint32_t achievement_id,
		const RewardSelectionReward &reward
	);
	void ArmCompletionNotificationTimer(bool immediate);
	void SendCompletionNotification(const PendingCompletionNotification &notification);
	void SendStateUpdate(size_t definition_index);
	void SendProgressUpdates(const std::vector<EQ::Achievements::ProgressUpdate> &updates);
	bool SupportsPackets() const;

	Client &m_client;
	bool m_loaded = false;
	bool m_initial_sent = false;
	uint32_t m_serial = 0;
	std::vector<EQ::Achievements::State> m_states;
	std::unordered_set<uint32_t> m_completion_stack;
	std::deque<PendingCompletionNotification> m_pending_notifications;
	std::unordered_set<uint32_t> m_pending_notification_ids;
	Timer m_notification_timer;
	std::deque<uint32_t> m_pending_rewards;
	std::unordered_set<uint32_t> m_pending_reward_ids;
};
