#include "common/achievements.h"

#include "common/compression.h"
#include "common/reward_selection.h"

#include <limits>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <zlib.h>

namespace EQ::Achievements
{
uint32_t NpcNameIdentityHash(std::string_view name)
{
	constexpr uint32_t fnv_offset_basis = 2166136261u;
	constexpr uint32_t fnv_prime = 16777619u;

	uint32_t hash = fnv_offset_basis;
	bool has_letter = false;
	bool separator_pending = false;

	const auto append = [&hash](uint8_t value) {
		hash ^= value;
		hash *= fnv_prime;
	};

	for (const auto character : name) {
		const auto byte = static_cast<uint8_t>(character);
		if (byte == static_cast<uint8_t>(' ') || byte == static_cast<uint8_t>('_')) {
			separator_pending = has_letter;
			continue;
		}

		auto normalized = byte;
		if (normalized >= static_cast<uint8_t>('A') && normalized <= static_cast<uint8_t>('Z')) {
			normalized = static_cast<uint8_t>(normalized + ('a' - 'A'));
		}
		if (normalized < static_cast<uint8_t>('a') || normalized > static_cast<uint8_t>('z')) {
			continue;
		}

		if (separator_pending) {
			append(static_cast<uint8_t>(' '));
			separator_pending = false;
		}
		append(normalized);
		has_letter = true;
	}

	return has_letter && hash ? hash : 0;
}

static uint16_t PackBitsetWord(
	const std::vector<uint8_t> &satisfied,
	size_t component_count,
	size_t word_index
)
{
	uint16_t word = 0;
	for (size_t bit_index = 0; bit_index < 16; ++bit_index) {
		const auto component_index = word_index * 16 + bit_index;
		if (
			component_index < component_count &&
			component_index < satisfied.size() &&
			satisfied[component_index]
		) {
			word |= static_cast<uint16_t>(1u << bit_index);
		}
	}
	return word;
}

template <typename T>
static void AppendLinkValue(std::string &out, T value)
{
	out.append(std::to_string(value));
	out.push_back('^');
}

static void SerializeCategory(SerializeBuffer &out, const Category &category)
{
	out.WriteUInt32(category.category_id);
	// Content stores a root parent as 0; RoF2 expects the signed sentinel -1.
	out.WriteUInt32(
		category.parent_category_id
			? category.parent_category_id
			: std::numeric_limits<uint32_t>::max()
	);
	out.WriteString(category.name);
	out.WriteString(category.description);
	out.WriteString(category.icon);
	out.WriteUInt32(category.display_order);

	out.WriteUInt32(static_cast<uint32_t>(category.associations.size()));
	for (const auto &association : category.associations) {
		out.WriteUInt32(association.achievement_id);
		out.WriteString(association.text);
		out.WriteUInt32(association.display_order);
	}

	out.WriteUInt32(static_cast<uint32_t>(category.child_category_ids.size()));
	for (const auto child_category_id : category.child_category_ids) {
		out.WriteUInt32(child_category_id);
	}
}

static void SerializeComponent(SerializeBuffer &out, const Component &component)
{
	out.WriteUInt32(component.component_id);
	out.WriteUInt8(component.component_type);
	out.WriteUInt32(component.required_count);
	out.WriteString(component.name);
	out.WriteString(component.description);
	out.WriteUInt8(component.display_order);
}

static void SerializeDefinition(SerializeBuffer &out, const Definition &definition)
{
	out.WriteUInt32(definition.achievement_id);
	out.WriteString(definition.name);
	out.WriteString(definition.description);
	out.WriteUInt32(definition.icon_id);
	out.WriteUInt8(1); // persistent
	out.WriteUInt32(definition.version);

	for (const auto &components : definition.components) {
		out.WriteUInt32(static_cast<uint32_t>(components.size()));
		for (const auto &component : components) {
			SerializeComponent(out, component);
		}
	}

	out.WriteUInt32(definition.points);
	out.WriteUInt32(definition.has_reward ? 1 : 0);
}

static void SerializeBitset(
	SerializeBuffer &out,
	const std::vector<uint8_t> &satisfied,
	size_t component_count
)
{
	const auto word_count = (component_count + 15) / 16;
	for (size_t word_index = 0; word_index < word_count; ++word_index) {
		out.WriteUInt16(PackBitsetWord(satisfied, component_count, word_index));
	}
}

static void SerializeCounts(
	SerializeBuffer &out,
	const std::vector<uint32_t> &counts,
	size_t component_count
)
{
	for (size_t component_index = 0; component_index < component_count; ++component_index) {
		out.WriteUInt32(component_index < counts.size() ? counts[component_index] : 0);
	}
}

SerializeBuffer SerializeDefinitions(
	const std::vector<Category> &categories,
	const std::vector<Definition> &definitions
)
{
	SerializeBuffer out;
	out.WriteUInt32(static_cast<uint32_t>(categories.size()));
	for (const auto &category : categories) {
		SerializeCategory(out, category);
	}

	out.WriteUInt32(static_cast<uint32_t>(definitions.size()));
	for (const auto &definition : definitions) {
		SerializeDefinition(out, definition);
	}

	return out;
}

std::vector<Category> SelectActiveCategories(const std::vector<Category> &categories)
{
	std::unordered_map<uint32_t, size_t> category_indices;
	for (size_t index = 0; index < categories.size(); ++index) {
		if (!categories[index].category_id) {
			throw std::invalid_argument("achievement category ID zero is reserved");
		}
		if (!category_indices.emplace(categories[index].category_id, index).second) {
			throw std::invalid_argument("achievement category IDs must be unique");
		}
	}

	std::unordered_set<uint32_t> retained_ids;
	for (const auto &category : categories) {
		if (category.associations.empty()) {
			continue;
		}

		std::unordered_set<uint32_t> lineage;
		auto category_id = category.category_id;
		while (category_id) {
			if (!lineage.insert(category_id).second) {
				throw std::invalid_argument(
					"active achievement category hierarchy contains a cycle"
				);
			}

			const auto category_index = category_indices.find(category_id);
			if (category_index == category_indices.end()) {
				throw std::invalid_argument(
					"active achievement category hierarchy references a missing parent"
				);
			}

			retained_ids.insert(category_id);
			category_id = categories[category_index->second].parent_category_id;
		}
	}

	std::vector<Category> selected;
	selected.reserve(retained_ids.size());
	for (const auto &category : categories) {
		if (!retained_ids.contains(category.category_id)) {
			continue;
		}

		selected.push_back(category);
		selected.back().child_category_ids.clear();
	}

	std::unordered_map<uint32_t, size_t> selected_indices;
	for (size_t index = 0; index < selected.size(); ++index) {
		selected_indices.emplace(selected[index].category_id, index);
	}
	for (const auto &category : selected) {
		if (!category.parent_category_id) {
			continue;
		}

		const auto parent = selected_indices.find(category.parent_category_id);
		if (parent == selected_indices.end()) {
			throw std::invalid_argument(
				"active achievement category hierarchy lost a required parent"
			);
		}
		selected[parent->second].child_category_ids.push_back(category.category_id);
	}

	return selected;
}

SerializeBuffer CompressDefinitions(const SerializeBuffer &definitions)
{
	if (definitions.size() > std::numeric_limits<uint32_t>::max()) {
		throw std::length_error("uncompressed achievement definitions exceed uint32");
	}

	const auto input_size = static_cast<uint32_t>(definitions.size());
	const auto output_capacity = static_cast<uint32_t>(compressBound(input_size));
	std::vector<char> compressed(output_capacity);

	const auto compressed_size = EQ::DeflateData(
		reinterpret_cast<const char *>(definitions.buffer()),
		input_size,
		compressed.data(),
		output_capacity
	);
	if (!compressed_size) {
		throw std::runtime_error("unable to compress achievement definitions");
	}

	SerializeBuffer out(sizeof(uint32_t) + compressed_size);
	out.WriteUInt32(input_size);
	for (uint32_t i = 0; i < compressed_size; ++i) {
		out.WriteUInt8(static_cast<uint8_t>(compressed[i]));
	}

	return out;
}

void SerializeState(
	SerializeBuffer &out,
	const Definition &definition,
	const State &state,
	bool include_counts
)
{
	out.WriteInt16(static_cast<int16_t>(state.status));

	// Runtime state order differs from the definition's 0..3 order.
	for (const auto component_type : {1u, 2u, 0u}) {
		SerializeBitset(
			out,
			state.satisfied[component_type],
			definition.components[component_type].size()
		);
	}

	if (state.status == Status::Completed) {
		out.WriteUInt32(state.completion_timestamp);
	}

	if (include_counts) {
		for (const auto component_type : {1u, 2u, 0u}) {
			SerializeCounts(
				out,
				state.counts[component_type],
				definition.components[component_type].size()
			);
		}
	}
}

SerializeBuffer SerializeSnapshot(
	const std::vector<Definition> &definitions,
	const std::vector<State> &states
)
{
	if (definitions.size() != states.size()) {
		throw std::invalid_argument("achievement snapshot must contain one state per definition");
	}

	SerializeBuffer out;
	for (size_t definition_index = 0; definition_index < definitions.size(); ++definition_index) {
		SerializeState(out, definitions[definition_index], states[definition_index]);
	}

	return out;
}

SerializeBuffer SerializeDenseUpdate(
	uint32_t serial,
	const std::vector<Definition> &definitions,
	const std::vector<State> &states
)
{
	if (definitions.size() != states.size()) {
		throw std::invalid_argument(
			"dense achievement state initialization must contain one state per definition"
		);
	}

	SerializeBuffer out;
	out.WriteUInt32(serial);
	out.WriteUInt8(1);
	out.WriteUInt32(static_cast<uint32_t>(states.size()));
	for (size_t definition_index = 0; definition_index < definitions.size(); ++definition_index) {
		SerializeState(out, definitions[definition_index], states[definition_index]);
	}

	return out;
}

SerializeBuffer SerializeIncremental(
	uint32_t serial,
	const std::vector<Definition> &definitions,
	const std::vector<StateUpdate> &updates,
	bool dense
)
{
	if (dense) {
		if (updates.size() > definitions.size()) {
			throw std::invalid_argument(
				"dense achievement state updates cannot exceed the definition count"
			);
		}
		for (size_t definition_index = 0; definition_index < updates.size(); ++definition_index) {
			if (updates[definition_index].definition_index != definition_index) {
				throw std::invalid_argument(
					"dense achievement state updates must be in definition order"
				);
			}
		}
	}

	SerializeBuffer out;
	out.WriteUInt32(serial);
	out.WriteUInt8(dense ? 1 : 0);
	out.WriteUInt32(static_cast<uint32_t>(updates.size()));

	for (const auto &update : updates) {
		if (update.definition_index >= definitions.size()) {
			throw std::out_of_range("achievement state update definition index is invalid");
		}

		if (!dense) {
			out.WriteUInt32(update.definition_index);
		}
		SerializeState(out, definitions[update.definition_index], update.state);
	}

	return out;
}

SerializeBuffer SerializeProgress(const std::vector<ProgressUpdate> &progress)
{
	for (const auto &entry : progress) {
		if (entry.requirement_type > 2) {
			throw std::invalid_argument(
				"RoF2 achievement progress supports component types 0 through 2"
			);
		}
	}

	SerializeBuffer out;
	out.WriteUInt32(static_cast<uint32_t>(progress.size()));
	for (const auto &entry : progress) {
		out.WriteUInt32(entry.achievement_id);
		out.WriteUInt32(entry.component_id);
		out.WriteUInt32(entry.requirement_id);
		out.WriteUInt32(entry.requirement_type);
		out.WriteUInt32(entry.current_count);
	}

	return out;
}

std::string SerializeLinkData(
	const std::string &player_name,
	const Definition &definition,
	const State &state
)
{
	std::string out;
	out.reserve(player_name.size() + 64);
	out.append(player_name);
	out.push_back('^');
	AppendLinkValue(out, definition.achievement_id);
	AppendLinkValue(out, static_cast<int16_t>(state.status));

	// Text links use the same component order as state packets.
	for (const auto component_type : {1u, 2u, 0u}) {
		const auto component_count = definition.components[component_type].size();
		const auto word_count = (component_count + 15) / 16;
		for (size_t word_index = 0; word_index < word_count; ++word_index) {
			const auto word = PackBitsetWord(
				state.satisfied[component_type],
				component_count,
				word_index
			);
			AppendLinkValue(out, static_cast<int16_t>(word));
		}
	}

	if (state.status == Status::Completed) {
		AppendLinkValue(out, state.completion_timestamp);
	}

	for (const auto component_type : {1u, 2u, 0u}) {
		const auto component_count = definition.components[component_type].size();
		for (size_t component_index = 0; component_index < component_count; ++component_index) {
			const auto &counts = state.counts[component_type];
			AppendLinkValue(
				out,
				component_index < counts.size() ? counts[component_index] : 0
			);
		}
	}

	return out;
}

SerializeBuffer SerializeEarnedNotification(
	uint32_t achiever_spawn_id,
	uint32_t achievement_id,
	uint32_t sound_id,
	const std::string &achievement_link_data
)
{
	if (
		achievement_link_data.empty() ||
		achievement_link_data.back() != '^' ||
		achievement_link_data.find('\0') != std::string::npos
	) {
		throw std::invalid_argument("RoF2 earned notification requires valid link metadata");
	}

	SerializeBuffer out;
	out.WriteUInt32(achiever_spawn_id);
	out.WriteUInt32(achievement_id);
	out.WriteUInt32(sound_id);
	out.WriteString(achievement_link_data);
	return out;
}

SerializeBuffer SerializeComparison(
	const std::string &player_name,
	uint32_t achievement_id,
	const Definition &definition,
	const State &state,
	uint8_t trailing_flag
)
{
	SerializeBuffer out;
	out.WriteString(player_name);
	out.WriteUInt32(achievement_id);
	SerializeState(out, definition, state, true);
	out.WriteUInt8(trailing_flag);
	return out;
}

size_t ComparisonPayloadSize(
	size_t player_name_length,
	const Definition &definition,
	Status status
)
{
	size_t component_count = 0;
	size_t bitset_word_count = 0;
	for (const auto component_type : {1u, 2u, 0u}) {
		const auto count = definition.components[component_type].size();
		component_count += count;
		bitset_word_count += (count + 15) / 16;
	}

	return
		player_name_length + 1 +
		sizeof(uint32_t) +
		sizeof(int16_t) +
		bitset_word_count * sizeof(uint16_t) +
		(status == Status::Completed ? sizeof(uint32_t) : 0) +
		component_count * sizeof(uint32_t) +
		sizeof(uint8_t);
}

static void SerializeRewardDisplaySet(
	SerializeBuffer &out,
	const RewardDisplaySet &reward_set
)
{
	if (
		!reward_set.pending_reward_id ||
		!reward_set.reward_set_id ||
		reward_set.subsets.size() >
			static_cast<size_t>(std::numeric_limits<int32_t>::max())
	) {
		throw std::invalid_argument("invalid RoF2 achievement reward set");
	}

	out.WriteUInt32(reward_set.pending_reward_id);
	out.WriteUInt32(reward_set.reward_set_id);
	out.WriteString(reward_set.title);
	out.WriteUInt32(reward_set.amount_multiplier_bits);

	out.WriteUInt32(reward_set.reward_set_id);
	out.WriteInt32(static_cast<int32_t>(reward_set.subsets.size()));

	std::unordered_set<uint32_t> subset_ids;
	for (const auto &subset : reward_set.subsets) {
		if (
			!subset.subset_id ||
			!subset_ids.insert(subset.subset_id).second ||
			subset.entries.size() > std::numeric_limits<uint32_t>::max()
		) {
			throw std::invalid_argument("invalid RoF2 achievement reward subset");
		}

		out.WriteUInt32(subset.subset_id);
		out.WriteUInt32(static_cast<uint32_t>(subset.entries.size()));
		out.WriteUInt8(subset.common_to_all ? 1 : 0);
		out.WriteUInt8(subset.flags);
		out.WriteString(subset.option_label);

		for (const auto &entry : subset.entries) {
			out.WriteUInt32(static_cast<uint32_t>(entry.wire_type));
			for (const auto field : entry.fields) {
				out.WriteUInt32(field);
			}
			out.WriteString(entry.description);

			switch (entry.wire_type) {
			case RewardWireType::Money:
				for (const auto value : entry.values) {
					out.WriteUInt32(value);
				}
				break;
			case RewardWireType::Item:
				out.WriteUInt32(static_cast<uint32_t>(entry.items.size()));
				for (const auto &item : entry.items) {
					out.WriteUInt32(item.field0);
					out.WriteUInt32(item.field1);
					out.WriteUInt8(item.field2);
					out.WriteUInt32(item.field3);
					out.WriteString(item.text);
				}
				break;
			case RewardWireType::AlternateAdvancementAbility:
				out.WriteUInt32(entry.values[0]);
				out.WriteUInt32(entry.values[1]);
				break;
			case RewardWireType::AlternateAdvancementPoints:
				out.WriteUInt32(entry.values[0]);
				out.WriteUInt8(entry.flag);
				break;
			case RewardWireType::GenericPoints:
				for (const auto value : entry.values) {
					out.WriteUInt32(value);
				}
				break;
			default:
				break;
			}
		}
	}
}

SerializeBuffer SerializeRewardDisplay(const RewardDisplaySet *reward_set)
{
	SerializeBuffer out;
	out.WriteUInt32(static_cast<uint32_t>(RewardSelection::Action::List));
	out.WriteUInt8(reward_set ? 1 : 0);
	if (reward_set) {
		SerializeRewardDisplaySet(out, *reward_set);
	}

	return out;
}

SerializeBuffer SerializeRewardDisplays(
	const std::vector<RewardDisplaySet> &reward_sets
)
{
	if (reward_sets.size() >
		static_cast<size_t>(std::numeric_limits<int32_t>::max())) {
		throw std::invalid_argument("too many RoF2 reward display sets");
	}

	SerializeBuffer out;
	out.WriteUInt32(static_cast<uint32_t>(RewardSelection::Action::Bulk));
	out.WriteInt32(static_cast<int32_t>(reward_sets.size()));
	out.WriteUInt8(0);
	for (const auto &reward_set : reward_sets) {
		SerializeRewardDisplaySet(out, reward_set);
	}
	return out;
}

SerializeBuffer SerializeRewardDisplayClear()
{
	return SerializeRewardDisplays({});
}

SerializeBuffer SerializeRewardClaimReply(
	uint32_t pending_reward_id,
	uint32_t reward_set_id,
	uint32_t selected_subset_id,
	bool success
)
{
	SerializeBuffer out;
	out.WriteUInt32(static_cast<uint32_t>(RewardSelection::Action::Claim));
	out.WriteUInt32(pending_reward_id);
	out.WriteUInt32(reward_set_id);
	out.WriteUInt32(selected_subset_id);
	out.WriteUInt8(success ? 1 : 0);
	out.WriteUInt8(0);
	out.WriteUInt8(0);
	out.WriteUInt8(0);
	return out;
}

} // namespace EQ::Achievements
