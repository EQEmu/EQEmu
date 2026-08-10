#pragma once

#include "common/achievement_mutations.h"

namespace AchievementMutations {

// A true result means the validated request was handed to world; member
// expansion and application are asynchronous. Advance functions set an
// absolute progress floor rather than adding value.
bool QueueGroupAdvance(
	uint32_t group_id,
	uint32_t achievement_id,
	ComponentType component_type,
	uint32_t component_id,
	uint32_t value
);
bool QueueGroupCompletion(uint32_t group_id, uint32_t achievement_id);

bool QueueRaidAdvance(
	int32_t raid_id,
	uint32_t achievement_id,
	ComponentType component_type,
	uint32_t component_id,
	uint32_t value
);
bool QueueRaidCompletion(int32_t raid_id, uint32_t achievement_id);

bool QueueDynamicZoneAdvance(
	uint32_t dynamic_zone_id,
	uint32_t achievement_id,
	ComponentType component_type,
	uint32_t component_id,
	uint32_t value
);
bool QueueDynamicZoneCompletion(
	uint32_t dynamic_zone_id,
	uint32_t achievement_id
);

bool QueueSharedTaskAdvance(
	int64_t shared_task_id,
	uint32_t achievement_id,
	ComponentType component_type,
	uint32_t component_id,
	uint32_t value
);
bool QueueSharedTaskCompletion(
	int64_t shared_task_id,
	uint32_t achievement_id
);

} // namespace AchievementMutations
