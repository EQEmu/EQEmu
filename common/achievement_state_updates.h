#pragma once

#include <cstddef>
#include <cstdint>

namespace AchievementStateUpdates {

enum class TargetType : uint8_t {
	Character = 0,
	Group,
	Raid,
	DynamicZone,
	SharedTask
};

enum class Operation : uint8_t {
	Advance = 0,
	Complete
};

enum class ComponentType : uint8_t {
	Completion = 0,
	Required,
	Optional
};

enum class Status : uint8_t {
	Pending = 0,
	Blocked,
	Processing
};

inline constexpr uint32_t ProcessingLeaseSeconds = 60;

struct Request {
	uint64_t target_id = 0;
	uint32_t achievement_id = 0;
	uint32_t component_id = 0;
	uint32_t value = 0;
	uint32_t version = 0;
	TargetType target_type = TargetType::Character;
	Operation operation = Operation::Advance;
	ComponentType component_type = ComponentType::Completion;
};

// Zone and world exchange a fixed-width representation rather than the
// compiler-dependent layout of Request.
inline constexpr std::size_t RequestWireSize = 32;

bool IsValidCharacterTarget(uint32_t character_id);
bool IsValidGroupTarget(uint32_t group_id);
bool IsValidRaidTarget(int32_t raid_id);
bool IsValidDynamicZoneTarget(uint32_t dynamic_zone_id);
bool IsValidSharedTaskTarget(int64_t shared_task_id);
bool IsValidRequest(const Request &request);

bool EncodeRequest(
	const Request &request,
	uint8_t *buffer,
	std::size_t buffer_size
);
bool DecodeRequest(
	const uint8_t *buffer,
	std::size_t buffer_size,
	Request &request
);

} // namespace AchievementStateUpdates
