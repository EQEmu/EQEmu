#pragma once

#include "../common/achievement_mutations.h"
#include "../common/timer.h"

#include <cstdint>
#include <deque>
#include <vector>

class AchievementMutationManager
{
public:
	bool Queue(const AchievementMutations::Request &request);
	void Process();

private:
	enum class TargetResolution {
		Resolved,
		Missing,
		RetryableFailure
	};

	struct RetryRequest {
		AchievementMutations::Request request;
		std::vector<uint32_t> character_ids;
		bool targets_resolved = false;
	};

	TargetResolution ResolveTargets(
		AchievementMutations::TargetType target_type,
		uint64_t target_id,
		std::vector<uint32_t> &character_ids
	) const;
	static void NormalizeTargets(std::vector<uint32_t> &character_ids);
	bool RetainForRetry(
		const AchievementMutations::Request &request,
		std::vector<uint32_t> character_ids = {},
		bool targets_resolved = false
	);
	void ProcessRetries();
	bool Persist(
		const AchievementMutations::Request &request,
		const std::vector<uint32_t> &character_ids
	) const;
	void Wake(const std::vector<uint32_t> &character_ids) const;

	Timer m_retry_timer{5000};
	uint32_t m_retry_cursor = 0;
	std::deque<RetryRequest> m_retry_requests;
};
