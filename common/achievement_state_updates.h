#pragma once

#include "common/serialize_buffer.h"

#include <cstdint>

namespace EQ::Net {
class Packet;
}

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

bool IsValidCharacterTarget(uint32_t character_id);
bool IsValidGroupTarget(uint32_t group_id);
bool IsValidRaidTarget(int32_t raid_id);
bool IsValidDynamicZoneTarget(uint32_t dynamic_zone_id);
bool IsValidSharedTaskTarget(int64_t shared_task_id);
bool IsValidRequest(const Request &request);

SerializeBuffer SerializeRequest(const Request &request);
bool DeserializeRequest(const EQ::Net::Packet &packet, Request &request);

} // namespace AchievementStateUpdates
