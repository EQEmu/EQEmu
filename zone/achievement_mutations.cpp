#include "achievement_mutations.h"

#include "../common/eq_packet.h"
#include "../common/rulesys.h"
#include "../common/servertalk.h"
#include "achievement_manager.h"
#include "worldserver.h"

#include <cstring>

extern WorldServer worldserver;

namespace AchievementMutations {
namespace {

bool Send(const Request &request)
{
	if (!IsValidRequest(request) || !worldserver.Connected()) {
		return false;
	}

	ServerPacket packet(ServerOP_CZAchievementMutationRequest, sizeof(Request));
	std::memcpy(packet.pBuffer, &request, sizeof(Request));
	return worldserver.SendPacket(&packet);
}

} // namespace

bool QueueAdvance(
	TargetType target_type,
	uint64_t target_id,
	uint32_t achievement_id,
	uint8_t component_type,
	uint32_t component_id,
	uint32_t value
)
{
	const auto &manager = AchievementManager::Instance();
	if (
		!RuleB(Achievements, EnableAchievements) ||
		!manager.IsLoaded() ||
		!IsValidTarget(target_type, target_id) ||
		component_type > 2 ||
		!value
	) {
		return false;
	}

	const auto definition = manager.FindDefinition(achievement_id);
	if (
		!definition ||
		!manager.FindComponentIndex(achievement_id, component_type, component_id)
	) {
		return false;
	}

	return Send({
		.target_id = target_id,
		.achievement_id = achievement_id,
		.component_id = component_id,
		.value = value,
		.definition_version = definition->definition_version,
		.target_type = target_type,
		.operation = Operation::Advance,
		.component_type = component_type
	});
}

bool QueueCompletion(
	TargetType target_type,
	uint64_t target_id,
	uint32_t achievement_id
)
{
	const auto &manager = AchievementManager::Instance();
	if (
		!RuleB(Achievements, EnableAchievements) ||
		!manager.IsLoaded() ||
		!IsValidTarget(target_type, target_id)
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
		.definition_version = definition->definition_version,
		.target_type = target_type,
		.operation = Operation::Complete
	});
}

} // namespace AchievementMutations
