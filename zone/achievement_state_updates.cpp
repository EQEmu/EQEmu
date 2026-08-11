#include "zone/achievement_state_updates.h"

#include "common/eq_packet.h"
#include "common/rulesys.h"
#include "common/servertalk.h"
#include "zone/achievement_manager.h"
#include "zone/worldserver.h"

#include <cstring>

extern WorldServer worldserver;

namespace AchievementStateUpdates {

static bool Send(const Request &request)
{
	if (!IsValidRequest(request) || !worldserver.Connected()) {
		return false;
	}

	auto serialized = SerializeRequest(request);
	if (!serialized.size()) {
		return false;
	}

	ServerPacket packet(ServerOP_CZAchievementStateUpdateRequest, serialized.size());
	memcpy(packet.pBuffer, serialized.buffer(), serialized.size());
	return worldserver.SendPacket(&packet);
}

static bool QueueAdvance(
	TargetType target_type,
	uint64_t target_id,
	uint32_t achievement_id,
	ComponentType component_type,
	uint32_t component_id,
	uint32_t value
)
{
	const auto &manager = AchievementManager::Instance();
	if (
		!RuleB(Achievements, EnableAchievements) ||
		!manager.IsLoaded() ||
		!value
	) {
		return false;
	}

	const auto definition = manager.FindDefinition(achievement_id);
	if (
		!definition ||
		!manager.FindComponentIndex(
			achievement_id,
			static_cast<uint8_t>(component_type),
			component_id
		)
	) {
		return false;
	}

	return Send({
		.target_id = target_id,
		.achievement_id = achievement_id,
		.component_id = component_id,
		.value = value,
		.version = definition->version,
		.target_type = target_type,
		.operation = Operation::Advance,
		.component_type = component_type
	});
}

static bool QueueCompletion(
	TargetType target_type,
	uint64_t target_id,
	uint32_t achievement_id
)
{
	const auto &manager = AchievementManager::Instance();
	if (
		!RuleB(Achievements, EnableAchievements) ||
		!manager.IsLoaded()
	) {
		return false;
	}

	const auto definition = manager.FindDefinition(achievement_id);
	if (!definition) {
		return false;
	}

	return Send({
		.target_id = target_id,
		.achievement_id = achievement_id,
		.version = definition->version,
		.target_type = target_type,
		.operation = Operation::Complete
	});
}

bool QueueGroupAdvance(
	uint32_t group_id,
	uint32_t achievement_id,
	ComponentType component_type,
	uint32_t component_id,
	uint32_t value
)
{
	return
		IsValidGroupTarget(group_id) &&
		QueueAdvance(
			TargetType::Group,
			group_id,
			achievement_id,
			component_type,
			component_id,
			value
		);
}

bool QueueGroupCompletion(uint32_t group_id, uint32_t achievement_id)
{
	return
		IsValidGroupTarget(group_id) &&
		QueueCompletion(TargetType::Group, group_id, achievement_id);
}

bool QueueRaidAdvance(
	int32_t raid_id,
	uint32_t achievement_id,
	ComponentType component_type,
	uint32_t component_id,
	uint32_t value
)
{
	return
		IsValidRaidTarget(raid_id) &&
		QueueAdvance(
			TargetType::Raid,
			static_cast<uint64_t>(raid_id),
			achievement_id,
			component_type,
			component_id,
			value
		);
}

bool QueueRaidCompletion(int32_t raid_id, uint32_t achievement_id)
{
	return
		IsValidRaidTarget(raid_id) &&
		QueueCompletion(
			TargetType::Raid,
			static_cast<uint64_t>(raid_id),
			achievement_id
		);
}

bool QueueDynamicZoneAdvance(
	uint32_t dynamic_zone_id,
	uint32_t achievement_id,
	ComponentType component_type,
	uint32_t component_id,
	uint32_t value
)
{
	return
		IsValidDynamicZoneTarget(dynamic_zone_id) &&
		QueueAdvance(
			TargetType::DynamicZone,
			dynamic_zone_id,
			achievement_id,
			component_type,
			component_id,
			value
		);
}

bool QueueDynamicZoneCompletion(
	uint32_t dynamic_zone_id,
	uint32_t achievement_id
)
{
	return
		IsValidDynamicZoneTarget(dynamic_zone_id) &&
		QueueCompletion(
			TargetType::DynamicZone,
			dynamic_zone_id,
			achievement_id
		);
}

bool QueueSharedTaskAdvance(
	int64_t shared_task_id,
	uint32_t achievement_id,
	ComponentType component_type,
	uint32_t component_id,
	uint32_t value
)
{
	return
		IsValidSharedTaskTarget(shared_task_id) &&
		QueueAdvance(
			TargetType::SharedTask,
			static_cast<uint64_t>(shared_task_id),
			achievement_id,
			component_type,
			component_id,
			value
		);
}

bool QueueSharedTaskCompletion(
	int64_t shared_task_id,
	uint32_t achievement_id
)
{
	return
		IsValidSharedTaskTarget(shared_task_id) &&
		QueueCompletion(
			TargetType::SharedTask,
			static_cast<uint64_t>(shared_task_id),
			achievement_id
		);
}

} // namespace AchievementStateUpdates
