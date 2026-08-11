#include "common/achievement_state_updates.h"

#include <cstring>
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
static constexpr std::size_t kReservedOffset = 27;

template <typename T>
static void Write(uint8_t *buffer, std::size_t offset, T value)
{
	std::memcpy(buffer + offset, &value, sizeof(value));
}

template <typename T>
static T Read(const uint8_t *buffer, std::size_t offset)
{
	T value{};
	std::memcpy(&value, buffer + offset, sizeof(value));
	return value;
}

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

bool EncodeRequest(
	const Request &request,
	uint8_t *buffer,
	std::size_t buffer_size
)
{
	if (!buffer || buffer_size != RequestWireSize || !IsValidRequest(request)) {
		return false;
	}

	std::memset(buffer, 0, RequestWireSize);
	Write(buffer, kTargetIdOffset, request.target_id);
	Write(buffer, kAchievementIdOffset, request.achievement_id);
	Write(buffer, kComponentIdOffset, request.component_id);
	Write(buffer, kValueOffset, request.value);
	Write(buffer, kVersionOffset, request.version);
	Write(buffer, kTargetTypeOffset, request.target_type);
	Write(buffer, kOperationOffset, request.operation);
	Write(buffer, kComponentTypeOffset, request.component_type);
	return true;
}

bool DecodeRequest(
	const uint8_t *buffer,
	std::size_t buffer_size,
	Request &request
)
{
	if (!buffer || buffer_size != RequestWireSize) {
		return false;
	}

	for (std::size_t offset = kReservedOffset; offset < RequestWireSize; ++offset) {
		if (buffer[offset] != 0) {
			return false;
		}
	}

	Request decoded;
	decoded.target_id = Read<uint64_t>(buffer, kTargetIdOffset);
	decoded.achievement_id = Read<uint32_t>(buffer, kAchievementIdOffset);
	decoded.component_id = Read<uint32_t>(buffer, kComponentIdOffset);
	decoded.value = Read<uint32_t>(buffer, kValueOffset);
	decoded.version = Read<uint32_t>(buffer, kVersionOffset);
	decoded.target_type = Read<TargetType>(buffer, kTargetTypeOffset);
	decoded.operation = Read<Operation>(buffer, kOperationOffset);
	decoded.component_type = Read<ComponentType>(buffer, kComponentTypeOffset);
	if (!IsValidRequest(decoded)) {
		return false;
	}

	request = decoded;
	return true;
}

} // namespace AchievementStateUpdates
