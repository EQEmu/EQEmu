#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace EQ::RewardSelection {

enum class Source : uint8_t {
	Unknown     = 0,
	Achievement = 1,
	Task        = 2,
	General     = 3
};

enum class RewardType : uint8_t {
	Item                 = 0,
	Experience           = 1,
	AlternateAdvancement = 2,
	Copper               = 3,
	AlternateCurrency    = 4,
	Title                = 5
};

enum class ExperienceMode : uint32_t {
	Default    = 0,
	NormalOnly = 1
};

struct Reward {
	uint64_t    entry_id = 0;
	RewardType  type = RewardType::Item;
	uint32_t    data_id = 0;
	uint64_t    amount = 0;
	std::string description;
};

struct Option {
	uint32_t            option_id = 0;
	uint32_t            wire_option_id = 0;
	uint32_t            sequence = 0;
	std::string         label;
	bool                common_to_all = false;
	uint8_t             flags = 0;
	std::vector<Reward> rewards;
};

struct Set {
	uint32_t            reward_set_id = 0;
	std::string         title;
	std::vector<Option> options;
};

// Both RoF2 reward-manager opcodes use this action field.
enum class Action : uint32_t {
	List = 0,
	InspectItem = 1,
	Claim = 3,
	TaskView = 4,
	AchievementView = 5,
	Pending = 6,
	Bulk = 7
};

} // namespace EQ::RewardSelection
