#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace AchievementMutations {

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
	uint32_t definition_version = 0;
	TargetType target_type = TargetType::Character;
	Operation operation = Operation::Advance;
	uint8_t component_type = 0;
	uint8_t reserved8 = 0;
	uint32_t reserved32 = 0;
};

static_assert(sizeof(Request) == 32);
static_assert(std::is_standard_layout_v<Request>);
static_assert(std::is_trivially_copyable_v<Request>);
static_assert(offsetof(Request, reserved32) == 28);

constexpr bool IsValidTarget(TargetType target_type, uint64_t target_id)
{
	if (!target_id || target_type > TargetType::SharedTask) {
		return false;
	}

	if (target_type == TargetType::SharedTask) {
		return target_id <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max());
	}
	if (target_type == TargetType::Raid) {
		return target_id <= static_cast<uint64_t>(std::numeric_limits<int32_t>::max());
	}

	return target_id <= std::numeric_limits<uint32_t>::max();
}

constexpr bool IsValidRequest(const Request &request)
{
	if (
		!IsValidTarget(request.target_type, request.target_id) ||
		!request.achievement_id ||
		!request.definition_version ||
		request.reserved8 ||
		request.reserved32
	) {
		return false;
	}

	switch (request.operation) {
	case Operation::Advance:
		return
			request.component_type <= 2 &&
			request.value;
	case Operation::Complete:
		return
			request.component_type == 0 &&
			request.component_id == 0 &&
			request.value == 0;
	}

	return false;
}

} // namespace AchievementMutations
