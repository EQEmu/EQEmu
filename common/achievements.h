#pragma once

#include "common/serialize_buffer.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace EQ::Achievements
{

// Skill ID zero is 1H Blunt, so skill criteria use a separate wildcard value.
inline constexpr uint32_t SkillWildcardTargetId = 0xFFFFFFFFu;

// RoF2 maps this ID to sounds/Achievement.wav in soundassets.txt.
inline constexpr uint32_t RoF2AchievementSoundId = 3695;

enum class Status : int16_t {
	Completed = 0,
	Open      = 1,
	Locked    = 2,
	Hidden    = 3
};

enum class EventType : uint8_t {
	Manual                 = 0,
	Level                  = 1,
	NpcKill                = 2,
	NpcRaceKill            = 3,
	TaskComplete           = 4,
	ZoneEnter              = 5,
	LootItem               = 6,
	OwnItem                = 7,
	TradeskillSuccess      = 8,
	SkillValue             = 9,
	AlternateAdvancement   = 10,
	AchievementComplete    = 11,
	NpcNameKill            = 12,
	SkillCap               = 13
};

enum class ProgressMode : uint8_t {
	Increment = 0,
	Highest   = 1,
	Set       = 2,
	Boolean   = 3
};

enum class CriterionBehavior : uint8_t {
	Required    = 0,
	Optional    = 1,
	Unlock      = 2,
	Visibility  = 3,
	DisplayOnly = 4,
	Blocker     = 5
};

// RoF2 reward-window entry types. RewardSelection::RewardType describes delivery.
enum class RewardWireType : uint32_t {
	Text                         = 0,
	Money                        = 1,
	Item                         = 2,
	Experience                   = 3,
	AlternateAdvancementAbility = 4,
	AlternateAdvancementPoints  = 5,
	GenericPoints                = 11
};

struct RewardDisplayItem {
	uint32_t    field0 = 0;
	uint32_t    field1 = 0;
	uint8_t     field2 = 0;
	uint32_t    field3 = 0;
	std::string text;
};

struct RewardDisplayEntry {
	RewardWireType                 wire_type = RewardWireType::Text;
	std::array<uint32_t, 2>        fields{};
	std::string                    description;
	std::array<uint32_t, 4>        values{};
	uint8_t                        flag = 0;
	std::vector<RewardDisplayItem> items;
};

struct RewardDisplaySubset {
	uint32_t                        subset_id = 0;
	bool                            common_to_all = false;
	uint8_t                         flags = 0;
	std::string                     option_label;
	std::vector<RewardDisplayEntry> entries;
};

struct RewardDisplaySet {
	uint32_t                         pending_reward_id = 0;
	uint32_t                         reward_set_id = 0;
	std::string                      title;
	// IEEE-754 1.0f. The client multiplies numeric reward displays by this.
	uint32_t                         amount_multiplier_bits = 0x3f800000;
	std::vector<RewardDisplaySubset> subsets;
};

struct CategoryAssociation {
	uint32_t    achievement_id = 0;
	std::string text;
	uint32_t    display_order = 0;
};

struct Category {
	uint32_t                         category_id = 0;
	uint32_t                         parent_category_id = 0;
	std::string                      name;
	std::string                      description;
	std::string                      icon;
	uint32_t                         display_order = 0;
	std::vector<CategoryAssociation> associations;
	std::vector<uint32_t>            child_category_ids;
};

struct Component {
	uint32_t    component_id = 0;
	uint8_t     component_type = 0;
	uint32_t    sequence = 0; // Full presentation order; display_order is the RoF2 byte value.
	uint32_t    required_count = 1;
	std::string name;
	std::string description;
	uint8_t     display_order = 0;
};

struct Definition {
	uint32_t                              achievement_id = 0;
	std::string                           name;
	std::string                           description;
	uint32_t                              icon_id = 0;
	uint32_t                              version = 0; // Client definition and server persistence version.
	std::array<std::vector<Component>, 4> components;
	uint32_t                              points = 0;
	bool                                  has_reward = false;
};

struct State {
	Status                                  status = Status::Open;
	std::array<std::vector<uint8_t>, 4>      satisfied;
	std::array<std::vector<uint32_t>, 4>     counts;
	uint32_t                                completion_timestamp = 0;
};

struct StateUpdate {
	uint32_t definition_index = 0;
	State    state;
};

struct ProgressUpdate {
	uint32_t achievement_id = 0;
	uint32_t component_id = 0;
	uint32_t requirement_id = 0;
	uint32_t requirement_type = 0;
	uint32_t current_count = 0;
};

uint32_t NpcNameIdentityHash(std::string_view name);

// The client consumes this uncompressed stream after inflating the definition packet.
SerializeBuffer SerializeDefinitions(
	const std::vector<Category> &categories,
	const std::vector<Definition> &definitions
);

// Retains categories that contain active associations and their ancestors,
// preserving display order and rebuilding child lists for the retained tree.
// Association lists must already be limited to enabled definitions.
std::vector<Category> SelectActiveCategories(const std::vector<Category> &categories);

// Packet form: uncompressed byte count followed by a zlib stream.
SerializeBuffer CompressDefinitions(const SerializeBuffer &definitions);

// Serializes one state using the component vector sizes from its definition.
// Runtime bitset order is component type 1, type 2, then type 0.
void SerializeState(
	SerializeBuffer &out,
	const Definition &definition,
	const State &state,
	bool include_counts = false
);

// Comparison/summary snapshot: one state per definition, with no leading count.
SerializeBuffer SerializeSnapshot(
	const std::vector<Definition> &definitions,
	const std::vector<State> &states
);

// Full primary-state initialization on OP_AchievementUpdate. RoF2's dense
// form uses implicit definition indices 0..count-1.
SerializeBuffer SerializeDenseUpdate(
	uint32_t serial,
	const std::vector<Definition> &definitions,
	const std::vector<State> &states
);

// Incremental state packet. Sparse form is preferred because it does not require
// updates to be contiguous in definition order.
SerializeBuffer SerializeIncremental(
	uint32_t serial,
	const std::vector<Definition> &definitions,
	const std::vector<StateUpdate> &updates,
	bool dense = false
);

SerializeBuffer SerializeProgress(const std::vector<ProgressUpdate> &progress);

// Builds RoF2's caret-delimited achievement-link metadata. The client supplies
// the visible definition name when it wraps this metadata in a clickable link.
std::string SerializeLinkData(
	const std::string &player_name,
	const Definition &definition,
	const State &state
);

// RoF2's native completion notification. The trailing string is achievement
// link metadata, not display text. The client resolves the visible definition
// name and plays sound_id at the achiever's spawn.
SerializeBuffer SerializeEarnedNotification(
	uint32_t achiever_spawn_id,
	uint32_t achievement_id,
	uint32_t sound_id,
	const std::string &achievement_link_data
);

SerializeBuffer SerializeComparison(
	const std::string &player_name,
	uint32_t achievement_id,
	const Definition &definition,
	const State &state,
	uint8_t trailing_flag = 0
);

// Exact byte count for RoF2's binary per-achievement inspect payload:
// NUL name, achievement ID, state bitsets/counts, and trailing flag.
size_t ComparisonPayloadSize(
	size_t player_name_length,
	const Definition &definition,
	Status status
);

// RoF2 opcode 0x6411, action 0. A null display set emits the client-supported
// "no reward" response (action followed by has_reward=0).
SerializeBuffer SerializeRewardDisplay(const RewardDisplaySet *reward_set);

// The bulk action replaces the reward manager with one tab per display set.
// An empty collection clears the manager.
SerializeBuffer SerializeRewardDisplays(
	const std::vector<RewardDisplaySet> &reward_sets
);

SerializeBuffer SerializeRewardDisplayClear();

// RoF2 opcode 0x6411, action 3. The three request identity fields are echoed
// and the final byte tells the client whether to remove the pending reward.
SerializeBuffer SerializeRewardClaimReply(
	uint32_t pending_reward_id,
	uint32_t reward_set_id,
	uint32_t selected_subset_id,
	bool success
);

} // namespace EQ::Achievements
