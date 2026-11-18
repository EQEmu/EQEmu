#include "common/achievement_state_updates.h"
#include "common/net/packet.h"

#include <cstddef>
#include <limits>

namespace AchievementStateUpdates {

static constexpr std::size_t kTargetIdOffset = 0;
static constexpr std::size_t kAchievementIdOffset = 8;
static constexpr std::size_t kComponentIdOffset = 12;
static constexpr std::size_t kValueOffset = 16;
static constexpr std::size_t kVersionOffset = 20;
static constexpr std::size_t kTargetTypeOffset = 24;
static constexpr std::size_t kOperationOffset = 25;
static constexpr std::size_t kComponentTypeOffset = 26;
static constexpr std::size_t kSerializedRequestSize =
	sizeof(uint64_t) +
	(4 * sizeof(uint32_t)) +
	(3 * sizeof(uint8_t));

static bool IsValidTarget(TargetType target_type, uint64_t target_id)
{
	switch (target_type) {
	case TargetType::Character:
		return
			target_id <= std::numeric_limits<uint32_t>::max() &&
			IsValidCharacterTarget(static_cast<uint32_t>(target_id));
	case TargetType::Group:
		return
			target_id <= std::numeric_limits<uint32_t>::max() &&
			IsValidGroupTarget(static_cast<uint32_t>(target_id));
	case TargetType::Raid:
		return
			target_id <= static_cast<uint64_t>(std::numeric_limits<int32_t>::max()) &&
			IsValidRaidTarget(static_cast<int32_t>(target_id));
	case TargetType::DynamicZone:
		return
			target_id <= std::numeric_limits<uint32_t>::max() &&
			IsValidDynamicZoneTarget(static_cast<uint32_t>(target_id));
	case TargetType::SharedTask:
		return
			target_id <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max()) &&
			IsValidSharedTaskTarget(static_cast<int64_t>(target_id));
	}

	return false;
}

bool IsValidCharacterTarget(uint32_t character_id)
{
	return character_id != 0;
}

bool IsValidGroupTarget(uint32_t group_id)
{
	return group_id != 0;
}

bool IsValidRaidTarget(int32_t raid_id)
{
	return raid_id > 0;
}

bool IsValidDynamicZoneTarget(uint32_t dynamic_zone_id)
{
	return dynamic_zone_id != 0;
}

bool IsValidSharedTaskTarget(int64_t shared_task_id)
{
	return shared_task_id > 0;
}

bool IsValidRequest(const Request &request)
{
	if (!IsValidTarget(request.target_type, request.target_id) || !request.achievement_id) {
		return false;
	}

	switch (request.operation) {
	case Operation::Advance:
		return
			request.component_type <= ComponentType::Optional &&
			request.value;
	case Operation::Complete:
		return
			request.component_type == ComponentType::Completion &&
			request.component_id == 0 &&
			request.value == 0;
	}

	return false;
}

SerializeBuffer SerializeRequest(const Request &request)
{
	if (!IsValidRequest(request)) {
		return {};
	}

	SerializeBuffer buffer(kSerializedRequestSize);
	buffer.WriteUInt64(request.target_id);
	buffer.WriteUInt32(request.achievement_id);
	buffer.WriteUInt32(request.component_id);
	buffer.WriteUInt32(request.value);
	buffer.WriteUInt32(request.version);
	buffer.WriteUInt8(static_cast<uint8_t>(request.target_type));
	buffer.WriteUInt8(static_cast<uint8_t>(request.operation));
	buffer.WriteUInt8(static_cast<uint8_t>(request.component_type));
	return buffer;
}

bool DeserializeRequest(const EQ::Net::Packet &packet, Request &request)
{
	if (packet.Length() != kSerializedRequestSize) {
		return false;
	}

	Request decoded;
	decoded.target_id = packet.GetUInt64(kTargetIdOffset);
	decoded.achievement_id = packet.GetUInt32(kAchievementIdOffset);
	decoded.component_id = packet.GetUInt32(kComponentIdOffset);
	decoded.value = packet.GetUInt32(kValueOffset);
	decoded.version = packet.GetUInt32(kVersionOffset);
	decoded.target_type = static_cast<TargetType>(packet.GetUInt8(kTargetTypeOffset));
	decoded.operation = static_cast<Operation>(packet.GetUInt8(kOperationOffset));
	decoded.component_type = static_cast<ComponentType>(packet.GetUInt8(kComponentTypeOffset));
	if (!IsValidRequest(decoded)) {
		return false;
	}

	request = decoded;
	return true;
}

} // namespace AchievementStateUpdates
