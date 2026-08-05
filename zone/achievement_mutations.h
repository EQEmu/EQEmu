#pragma once

#include "../common/achievement_mutations.h"

namespace AchievementMutations {

// A true result means the validated request was handed to world; member
// expansion and application are asynchronous.
// QueueAdvance sets an absolute progress floor rather than adding value.
bool QueueAdvance(
	TargetType target_type,
	uint64_t target_id,
	uint32_t achievement_id,
	uint8_t component_type,
	uint32_t component_id,
	uint32_t value
);

bool QueueCompletion(
	TargetType target_type,
	uint64_t target_id,
	uint32_t achievement_id
);

} // namespace AchievementMutations
