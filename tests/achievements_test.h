#pragma once

#include "common/achievement_state_updates.h"
#include "common/achievements.h"
#include "common/compression.h"
#include "common/net/packet.h"
#include "common/reward_selection.h"
#include "common/rulesys.h"
#include "common/skills.h"
#include "common/types.h"
#include "cppunit/cpptest.h"

#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

class AchievementsTest : public Test::Suite
{
public:
	AchievementsTest()
	{
		TEST_ADD(AchievementsTest::DefinitionLayout);
		TEST_ADD(AchievementsTest::CategoryParentLayout);
		TEST_ADD(AchievementsTest::ActiveCategorySelection);
		TEST_ADD(AchievementsTest::CompressedEnvelope);
		TEST_ADD(AchievementsTest::PackedStateLayout);
		TEST_ADD(AchievementsTest::DenseInitialStateLayout);
		TEST_ADD(AchievementsTest::EmptyInitializationLayout);
		TEST_ADD(AchievementsTest::IncrementalAndProgressLayouts);
		TEST_ADD(AchievementsTest::LinkDataLayout);
		TEST_ADD(AchievementsTest::EarnedNotificationLayout);
		TEST_ADD(AchievementsTest::DenseIncrementalValidation);
		TEST_ADD(AchievementsTest::ComparisonCountsLayout);
		TEST_ADD(AchievementsTest::RewardDisplayLayout);
		TEST_ADD(AchievementsTest::RewardClaimReplyLayout);
		TEST_ADD(AchievementsTest::NpcNameIdentityHashLayout);
		TEST_ADD(AchievementsTest::SkillWildcardDoesNotAliasSkillZero);
		TEST_ADD(AchievementsTest::TypeThreeIsPresentationOnly);
		TEST_ADD(AchievementsTest::StateUpdateRequestValidation);
		TEST_ADD(AchievementsTest::GuildMemberNotificationRule);
		TEST_ADD(AchievementsTest::NearbyPlayerNotificationRules);
	}

private:
	void StateUpdateRequestValidation()
	{
		using namespace AchievementStateUpdates;

		TEST_ASSERT(static_cast<uint8_t>(Status::Pending) == 0);
		TEST_ASSERT(static_cast<uint8_t>(Status::Blocked) == 1);
		TEST_ASSERT(static_cast<uint8_t>(Status::Processing) == 2);
		TEST_ASSERT(ProcessingLeaseSeconds > 0);

		Request advance{
			.target_id = 42,
			.achievement_id = 100,
			.component_id = 7,
			.value = 3,
			.version = 0,
			.target_type = TargetType::Character,
			.operation = Operation::Advance,
			.component_type = ComponentType::Required
		};
		TEST_ASSERT(IsValidRequest(advance));
		TEST_ASSERT(advance.version == 0);
		advance.component_id = 0;
		TEST_ASSERT(IsValidRequest(advance));
		advance.component_id = 7;

		auto serialized = SerializeRequest(advance);
		TEST_ASSERT(serialized.size() != 0);
		EQ::Net::StaticPacket wire(
			const_cast<unsigned char *>(serialized.buffer()),
			serialized.size()
		);
		Request decoded;
		TEST_ASSERT(DeserializeRequest(wire, decoded));
		TEST_ASSERT(decoded.target_id == advance.target_id);
		TEST_ASSERT(decoded.component_type == ComponentType::Required);
		static_cast<uint8_t *>(wire.Data())[wire.Length() - 1] = 0xff;
		TEST_ASSERT(!DeserializeRequest(wire, decoded));
		EQ::Net::StaticPacket truncated(wire.Data(), wire.Length() - 1);
		TEST_ASSERT(!DeserializeRequest(truncated, decoded));

		advance.target_type = TargetType::SharedTask;
		advance.target_id = std::numeric_limits<uint64_t>::max();
		TEST_ASSERT(!IsValidRequest(advance));

		Request completion{
			.target_id = 9,
			.achievement_id = 100,
			.version = 0,
			.target_type = TargetType::Raid,
			.operation = Operation::Complete
		};
		TEST_ASSERT(IsValidRequest(completion));
		completion.value = 1;
		TEST_ASSERT(!IsValidRequest(completion));
	}

	void GuildMemberNotificationRule()
	{
		auto *rules = RuleManager::Instance();
		std::string original_value;
		TEST_ASSERT(rules->GetRule("Achievements:GuildMemberNotifications", original_value));

		struct RuleRestorer {
			RuleManager *rules;
			std::string value;

			~RuleRestorer()
			{
				rules->SetRule("Achievements:GuildMemberNotifications", value);
			}
		} rule_restorer{rules, original_value};

		TEST_ASSERT(rules->SetRule("Achievements:GuildMemberNotifications", "false"));
		TEST_ASSERT(!RuleB(Achievements, GuildMemberNotifications));
		TEST_ASSERT(rules->SetRule("Achievements:GuildMemberNotifications", "true"));
		TEST_ASSERT(RuleB(Achievements, GuildMemberNotifications));
	}

	void NearbyPlayerNotificationRules()
	{
		auto *rules = RuleManager::Instance();
		std::string original_enabled;
		std::string original_distance;
		TEST_ASSERT(rules->GetRule("Achievements:NearbyPlayerNotifications", original_enabled));
		TEST_ASSERT(rules->GetRule("Achievements:NearbyPlayerNotificationDistance", original_distance));

		struct RuleRestorer {
			RuleManager *rules;
			std::string enabled;
			std::string distance;

			~RuleRestorer()
			{
				rules->SetRule("Achievements:NearbyPlayerNotifications", enabled);
				rules->SetRule("Achievements:NearbyPlayerNotificationDistance", distance);
			}
		} rule_restorer{rules, original_enabled, original_distance};

		TEST_ASSERT(rules->SetRule("Achievements:NearbyPlayerNotifications", "false"));
		TEST_ASSERT(!RuleB(Achievements, NearbyPlayerNotifications));
		TEST_ASSERT(rules->SetRule("Achievements:NearbyPlayerNotifications", "true"));
		TEST_ASSERT(RuleB(Achievements, NearbyPlayerNotifications));
		TEST_ASSERT(rules->SetRule("Achievements:NearbyPlayerNotificationDistance", "375"));
		TEST_ASSERT(RuleI(Achievements, NearbyPlayerNotificationDistance) == 375);
	}

	struct Reader {
		const unsigned char *data;
		size_t size;
		size_t position = 0;

		template <typename T>
		T Read()
		{
			if (position + sizeof(T) > size) {
				throw std::out_of_range("achievement test packet read exceeds buffer");
			}
			T value{};
			std::memcpy(&value, data + position, sizeof(T));
			position += sizeof(T);
			return value;
		}

		std::string ReadString()
		{
			const auto start = position;
			while (position < size && data[position] != 0) {
				++position;
			}
			if (position >= size) {
				throw std::out_of_range("achievement test string is not NUL terminated");
			}
			std::string value(reinterpret_cast<const char *>(data + start), position - start);
			++position;
			return value;
		}
	};

	static EQ::Achievements::Definition TestDefinition()
	{
		using namespace EQ::Achievements;
		Definition definition;
		definition.achievement_id = 42;
		definition.name = "A";
		definition.description = "B";
		definition.icon_id = 77;
		definition.version = 2;
		definition.components[1].push_back({101, 1, 2, 5, "Reach five", "", 2});
		definition.components[3].push_back({303, 3, 9, 1, "Presentation only", "", 9});
		definition.points = 10;
		definition.has_reward = true;
		return definition;
	}

	void DefinitionLayout()
	{
		using namespace EQ::Achievements;
		Category category;
		category.category_id = 10;
		category.parent_category_id = 0;
		category.name = "General";
		category.description = "General achievements";
		category.icon = "Achievement";
		category.display_order = 4;
		category.associations.push_back({42, "", 7});
		category.child_category_ids.push_back(11);

		auto packet = SerializeDefinitions({category}, {TestDefinition()});
		Reader reader{packet.buffer(), packet.size()};

		TEST_ASSERT(reader.Read<uint32_t>() == 1);
		TEST_ASSERT(reader.Read<uint32_t>() == 10);
		TEST_ASSERT(reader.Read<int32_t>() == -1);
		TEST_ASSERT(reader.ReadString() == "General");
		TEST_ASSERT(reader.ReadString() == "General achievements");
		TEST_ASSERT(reader.ReadString() == "Achievement");
		TEST_ASSERT(reader.Read<uint32_t>() == 4);
		TEST_ASSERT(reader.Read<uint32_t>() == 1);
		TEST_ASSERT(reader.Read<uint32_t>() == 42);
		TEST_ASSERT(reader.ReadString().empty());
		TEST_ASSERT(reader.Read<uint32_t>() == 7);
		TEST_ASSERT(reader.Read<uint32_t>() == 1);
		TEST_ASSERT(reader.Read<uint32_t>() == 11);

		TEST_ASSERT(reader.Read<uint32_t>() == 1);
		TEST_ASSERT(reader.Read<uint32_t>() == 42);
		TEST_ASSERT(reader.ReadString() == "A");
		TEST_ASSERT(reader.ReadString() == "B");
		TEST_ASSERT(reader.Read<uint32_t>() == 77);
		TEST_ASSERT(reader.Read<uint8_t>() == 1);
		TEST_ASSERT(reader.Read<uint32_t>() == 2);
		TEST_ASSERT(reader.Read<uint32_t>() == 0);
		TEST_ASSERT(reader.Read<uint32_t>() == 1);
		TEST_ASSERT(reader.Read<uint32_t>() == 101);
		TEST_ASSERT(reader.Read<uint8_t>() == 1);
		TEST_ASSERT(reader.Read<uint32_t>() == 5);
		TEST_ASSERT(reader.ReadString() == "Reach five");
		TEST_ASSERT(reader.ReadString().empty());
		TEST_ASSERT(reader.Read<uint8_t>() == 2);
		TEST_ASSERT(reader.Read<uint32_t>() == 0);
		TEST_ASSERT(reader.Read<uint32_t>() == 1);
		TEST_ASSERT(reader.Read<uint32_t>() == 303);
		TEST_ASSERT(reader.Read<uint8_t>() == 3);
		TEST_ASSERT(reader.Read<uint32_t>() == 1);
		TEST_ASSERT(reader.ReadString() == "Presentation only");
		TEST_ASSERT(reader.ReadString().empty());
		TEST_ASSERT(reader.Read<uint8_t>() == 9);
		TEST_ASSERT(reader.Read<uint32_t>() == 10);
		TEST_ASSERT(reader.Read<uint32_t>() == 1);
		TEST_ASSERT(reader.position == reader.size);
	}

	void CategoryParentLayout()
	{
		using namespace EQ::Achievements;
		Category root;
		root.category_id = 10;
		root.child_category_ids.push_back(11);

		Category child;
		child.category_id = 11;
		child.parent_category_id = 10;

		auto packet = SerializeDefinitions({root, child}, {});
		Reader reader{packet.buffer(), packet.size()};

		TEST_ASSERT(reader.Read<uint32_t>() == 2);

		TEST_ASSERT(reader.Read<uint32_t>() == 10);
		TEST_ASSERT(reader.Read<int32_t>() == -1);
		TEST_ASSERT(reader.ReadString().empty());
		TEST_ASSERT(reader.ReadString().empty());
		TEST_ASSERT(reader.ReadString().empty());
		TEST_ASSERT(reader.Read<uint32_t>() == 0);
		TEST_ASSERT(reader.Read<uint32_t>() == 0);
		TEST_ASSERT(reader.Read<uint32_t>() == 1);
		TEST_ASSERT(reader.Read<uint32_t>() == 11);

		TEST_ASSERT(reader.Read<uint32_t>() == 11);
		TEST_ASSERT(reader.Read<int32_t>() == 10);
		TEST_ASSERT(reader.ReadString().empty());
		TEST_ASSERT(reader.ReadString().empty());
		TEST_ASSERT(reader.ReadString().empty());
		TEST_ASSERT(reader.Read<uint32_t>() == 0);
		TEST_ASSERT(reader.Read<uint32_t>() == 0);
		TEST_ASSERT(reader.Read<uint32_t>() == 0);

		TEST_ASSERT(reader.Read<uint32_t>() == 0);
		TEST_ASSERT(reader.position == reader.size);
	}

	void ActiveCategorySelection()
	{
		using namespace EQ::Achievements;
		Category root;
		root.category_id = 10;

		Category active_child;
		active_child.category_id = 11;
		active_child.parent_category_id = 10;
		active_child.associations.push_back({42, "", 0});

		Category empty_child;
		empty_child.category_id = 12;
		empty_child.parent_category_id = 10;

		Category empty_root;
		empty_root.category_id = 20;

		const auto selected = SelectActiveCategories({
			root,
			active_child,
			empty_child,
			empty_root
		});
		TEST_ASSERT(selected.size() == 2);
		TEST_ASSERT(selected[0].category_id == 10);
		TEST_ASSERT(selected[0].child_category_ids.size() == 1);
		TEST_ASSERT(selected[0].child_category_ids[0] == 11);
		TEST_ASSERT(selected[1].category_id == 11);
		TEST_ASSERT(selected[1].child_category_ids.empty());

		Category missing_parent;
		missing_parent.category_id = 30;
		missing_parent.parent_category_id = 999;
		missing_parent.associations.push_back({43, "", 0});
		const std::vector<Category> missing_parent_categories = {missing_parent};
		TEST_THROWS(
			SelectActiveCategories(missing_parent_categories),
			std::invalid_argument
		);

		Category cycle_a;
		cycle_a.category_id = 40;
		cycle_a.parent_category_id = 41;
		cycle_a.associations.push_back({44, "", 0});
		Category cycle_b;
		cycle_b.category_id = 41;
		cycle_b.parent_category_id = 40;
		const std::vector<Category> cycle_categories = {cycle_a, cycle_b};
		TEST_THROWS(
			SelectActiveCategories(cycle_categories),
			std::invalid_argument
		);

		Category reserved_id;
		reserved_id.associations.push_back({45, "", 0});
		const std::vector<Category> reserved_id_categories = {reserved_id};
		TEST_THROWS(
			SelectActiveCategories(reserved_id_categories),
			std::invalid_argument
		);
	}

	void CompressedEnvelope()
	{
		using namespace EQ::Achievements;
		auto definitions = SerializeDefinitions({}, {TestDefinition()});
		auto compressed = CompressDefinitions(definitions);
		Reader reader{compressed.buffer(), compressed.size()};
		const auto uncompressed_size = reader.Read<uint32_t>();
		TEST_ASSERT(uncompressed_size == definitions.size());

		std::vector<char> inflated(uncompressed_size);
		const auto inflated_size = EQ::InflateData(
			reinterpret_cast<const char *>(compressed.buffer() + reader.position),
			static_cast<uint32_t>(compressed.size() - reader.position),
			inflated.data(),
			uncompressed_size
		);
		TEST_ASSERT(inflated_size == definitions.size());
		TEST_ASSERT(std::memcmp(inflated.data(), definitions.buffer(), definitions.size()) == 0);
	}

	void PackedStateLayout()
	{
		using namespace EQ::Achievements;
		Definition definition;
		definition.components[1].resize(17);
		definition.components[2].resize(1);
		definition.components[0].resize(2);

		State state;
		state.status = Status::Completed;
		state.satisfied[1].resize(17);
		state.satisfied[1][0] = 1;
		state.satisfied[1][15] = 1;
		state.satisfied[1][16] = 1;
		state.satisfied[2] = {1};
		state.satisfied[0] = {0, 1};
		state.completion_timestamp = 0x12345678;

		SerializeBuffer packet;
		SerializeState(packet, definition, state);
		Reader reader{packet.buffer(), packet.size()};
		TEST_ASSERT(reader.Read<int16_t>() == 0);
		TEST_ASSERT(reader.Read<uint16_t>() == 0x8001);
		TEST_ASSERT(reader.Read<uint16_t>() == 0x0001);
		TEST_ASSERT(reader.Read<uint16_t>() == 0x0001);
		TEST_ASSERT(reader.Read<uint16_t>() == 0x0002);
		TEST_ASSERT(reader.Read<uint32_t>() == 0x12345678);
		TEST_ASSERT(reader.position == reader.size);
	}

	void IncrementalAndProgressLayouts()
	{
		using namespace EQ::Achievements;
		auto definition = TestDefinition();
		State state;
		state.satisfied[1] = {1};
		auto update = SerializeIncremental(17, {definition}, {{0, state}});
		Reader state_reader{update.buffer(), update.size()};
		TEST_ASSERT(state_reader.Read<uint32_t>() == 17);
		TEST_ASSERT(state_reader.Read<uint8_t>() == 0);
		TEST_ASSERT(state_reader.Read<uint32_t>() == 1);
		TEST_ASSERT(state_reader.Read<uint32_t>() == 0);
		TEST_ASSERT(state_reader.Read<int16_t>() == 1);
		TEST_ASSERT(state_reader.Read<uint16_t>() == 1);
		TEST_ASSERT(state_reader.position == state_reader.size);

		auto progress = SerializeProgress({{42, 101, 3, 1, 4}});
		Reader progress_reader{progress.buffer(), progress.size()};
		TEST_ASSERT(progress_reader.Read<uint32_t>() == 1);
		TEST_ASSERT(progress_reader.Read<uint32_t>() == 42);
		TEST_ASSERT(progress_reader.Read<uint32_t>() == 101);
		TEST_ASSERT(progress_reader.Read<uint32_t>() == 3);
		TEST_ASSERT(progress_reader.Read<uint32_t>() == 1);
		TEST_ASSERT(progress_reader.Read<uint32_t>() == 4);
		TEST_ASSERT(progress_reader.position == progress_reader.size);
	}

	void DenseInitialStateLayout()
	{
		using namespace EQ::Achievements;
		auto first = TestDefinition();
		Definition second;
		second.achievement_id = 43;

		State first_state;
		first_state.status = Status::Completed;
		first_state.satisfied[1] = {1};
		first_state.completion_timestamp = 99;
		State second_state;
		second_state.status = Status::Hidden;

		auto update = SerializeDenseUpdate(
			17,
			{first, second},
			{first_state, second_state}
		);
		Reader reader{update.buffer(), update.size()};
		TEST_ASSERT(reader.Read<uint32_t>() == 17);
		TEST_ASSERT(reader.Read<uint8_t>() == 1);
		TEST_ASSERT(reader.Read<uint32_t>() == 2);
		TEST_ASSERT(reader.Read<int16_t>() == 0);
		TEST_ASSERT(reader.Read<uint16_t>() == 1);
		TEST_ASSERT(reader.Read<uint32_t>() == 99);
		TEST_ASSERT(reader.Read<int16_t>() == 3);
		TEST_ASSERT(reader.position == reader.size);

		bool rejected = false;
		try {
			SerializeDenseUpdate(18, {first}, {});
		}
		catch (const std::invalid_argument &) {
			rejected = true;
		}
		TEST_ASSERT(rejected);
	}

	void DenseIncrementalValidation()
	{
		using namespace EQ::Achievements;
		auto definition = TestDefinition();
		State state;
		state.satisfied[1] = {1};

		auto dense = SerializeIncremental(18, {definition}, {{0, state}}, true);
		Reader reader{dense.buffer(), dense.size()};
		TEST_ASSERT(reader.Read<uint32_t>() == 18);
		TEST_ASSERT(reader.Read<uint8_t>() == 1);
		TEST_ASSERT(reader.Read<uint32_t>() == 1);
		TEST_ASSERT(reader.Read<int16_t>() == 1);
		TEST_ASSERT(reader.Read<uint16_t>() == 1);
		TEST_ASSERT(reader.position == reader.size);

		bool rejected = false;
		try {
			SerializeIncremental(19, {definition}, {{1, state}}, true);
		}
		catch (const std::invalid_argument &) {
			rejected = true;
		}
		TEST_ASSERT(rejected);
	}

	void LinkDataLayout()
	{
		using namespace EQ::Achievements;
		Definition definition;
		definition.achievement_id = 42;
		definition.components[1].resize(17);
		definition.components[2].resize(1);
		definition.components[0].resize(2);

		State state;
		state.status = Status::Completed;
		state.satisfied[1].resize(17);
		state.satisfied[1][0] = 1;
		state.satisfied[1][15] = 1;
		state.satisfied[1][16] = 1;
		state.satisfied[2] = {1};
		state.satisfied[0] = {0, 1};
		state.counts[1].resize(17);
		state.counts[1][0] = 5;
		state.counts[1][16] = 7;
		state.counts[2] = {8};
		state.counts[0] = {9, 10};
		state.completion_timestamp = 1700000000;

		TEST_ASSERT(
			SerializeLinkData("Alice", definition, state) ==
			"Alice^42^0^-32767^1^1^2^1700000000^"
			"5^0^0^0^0^0^0^0^0^0^0^0^0^0^0^0^7^8^9^10^"
		);

		Definition empty_definition;
		empty_definition.achievement_id = 7;
		State empty_state;
		empty_state.status = Status::Completed;
		empty_state.completion_timestamp = 99;
		TEST_ASSERT(SerializeLinkData("Bob", empty_definition, empty_state) == "Bob^7^0^99^");

		empty_state.status = Status::Open;
		TEST_ASSERT(SerializeLinkData("Bob", empty_definition, empty_state) == "Bob^7^1^");
		empty_state.status = Status::Locked;
		TEST_ASSERT(SerializeLinkData("Bob", empty_definition, empty_state) == "Bob^7^2^");
		empty_state.status = Status::Hidden;
		TEST_ASSERT(SerializeLinkData("Bob", empty_definition, empty_state) == "Bob^7^3^");
	}

	void EarnedNotificationLayout()
	{
		using namespace EQ::Achievements;
		auto earned = SerializeEarnedNotification(
			1234,
			5678,
			RoF2AchievementSoundId,
			"Bob^5678^0^99^"
		);
		Reader reader{earned.buffer(), earned.size()};
		TEST_ASSERT(reader.Read<uint32_t>() == 1234);
		TEST_ASSERT(reader.Read<uint32_t>() == 5678);
		TEST_ASSERT(reader.Read<uint32_t>() == RoF2AchievementSoundId);
		TEST_ASSERT(reader.ReadString() == "Bob^5678^0^99^");
		TEST_ASSERT(reader.position == reader.size);

		auto named = SerializeEarnedNotification(7, 42, 99, "Alice^42^0^99^");
		Reader named_reader{named.buffer(), named.size()};
		TEST_ASSERT(named_reader.Read<uint32_t>() == 7);
		TEST_ASSERT(named_reader.Read<uint32_t>() == 42);
		TEST_ASSERT(named_reader.Read<uint32_t>() == 99);
		TEST_ASSERT(named_reader.ReadString() == "Alice^42^0^99^");
		TEST_ASSERT(named_reader.position == named_reader.size);

		bool rejected = false;
		try {
			SerializeEarnedNotification(7, 42, 99, "");
		}
		catch (const std::invalid_argument &) {
			rejected = true;
		}
		TEST_ASSERT(rejected);
	}

	void ComparisonCountsLayout()
	{
		using namespace EQ::Achievements;
		auto definition = TestDefinition();
		State state;
		state.status = Status::Completed;
		state.satisfied[1] = {1};
		state.counts[1] = {5};
		state.completion_timestamp = 99;

		auto comparison = SerializeComparison("Alice", 42, definition, state, 7);
		TEST_ASSERT(
			comparison.size() ==
			ComparisonPayloadSize(
				std::string("Alice").size(),
				definition,
				Status::Completed
			)
		);
		Reader reader{comparison.buffer(), comparison.size()};
		TEST_ASSERT(reader.ReadString() == "Alice");
		TEST_ASSERT(reader.Read<uint32_t>() == 42);
		TEST_ASSERT(reader.Read<int16_t>() == 0);
		TEST_ASSERT(reader.Read<uint16_t>() == 1);
		TEST_ASSERT(reader.Read<uint32_t>() == 99);
		TEST_ASSERT(reader.Read<uint32_t>() == 5);
		TEST_ASSERT(reader.Read<uint8_t>() == 7);
		TEST_ASSERT(reader.position == reader.size);

		state.status = Status::Open;
		auto open_comparison =
			SerializeComparison("Alice", 42, definition, state, 0);
		TEST_ASSERT(
			open_comparison.size() ==
			ComparisonPayloadSize(
				std::string("Alice").size(),
				definition,
				Status::Open
			)
		);
	}

	void RewardDisplayLayout()
	{
		using namespace EQ::Achievements;

		auto empty = SerializeRewardDisplay(nullptr);
		Reader empty_reader{empty.buffer(), empty.size()};
		TEST_ASSERT(
			empty_reader.Read<uint32_t>() ==
				static_cast<uint32_t>(EQ::RewardSelection::Action::List)
		);
		TEST_ASSERT(empty_reader.Read<uint8_t>() == 0);
		TEST_ASSERT(empty_reader.position == empty_reader.size);
		auto clear = SerializeRewardDisplayClear();
		Reader clear_reader{clear.buffer(), clear.size()};
		TEST_ASSERT(
			clear_reader.Read<uint32_t>() ==
				static_cast<uint32_t>(EQ::RewardSelection::Action::Bulk)
		);
		TEST_ASSERT(clear_reader.Read<uint32_t>() == 0);
		TEST_ASSERT(clear_reader.Read<uint8_t>() == 0);
		TEST_ASSERT(clear_reader.position == clear_reader.size);

		RewardDisplaySet reward_set;
		reward_set.pending_reward_id = 42;
		reward_set.reward_set_id = 700;
		reward_set.title = "A Reward";
		reward_set.amount_multiplier_bits = 0x3f800000;

		RewardDisplaySubset common;
		common.subset_id = 701;
		common.common_to_all = true;
		common.option_label = "Always";
		RewardDisplayEntry money;
		money.wire_type = RewardWireType::Money;
		money.description = "Pocket money";
		money.values = {4, 3, 2, 1};
		common.entries.push_back(money);
		reward_set.subsets.push_back(common);

		RewardDisplaySubset choice;
		choice.subset_id = 702;
		choice.option_label = "Choose this";
		RewardDisplayEntry item;
		item.wire_type = RewardWireType::Item;
		item.fields = {10, 11};
		item.description = "An item";
		item.items.push_back({1001, 2, 1, 99, "Item link"});
		choice.entries.push_back(item);
		RewardDisplayEntry aa_points;
		aa_points.wire_type = RewardWireType::AlternateAdvancementPoints;
		aa_points.description = "AA";
		aa_points.values[0] = 5;
		aa_points.flag = 1;
		choice.entries.push_back(aa_points);
		reward_set.subsets.push_back(choice);

		auto packet = SerializeRewardDisplay(&reward_set);
		Reader reader{packet.buffer(), packet.size()};
		TEST_ASSERT(
			reader.Read<uint32_t>() ==
				static_cast<uint32_t>(EQ::RewardSelection::Action::List)
		);
		TEST_ASSERT(reader.Read<uint8_t>() == 1);
		TEST_ASSERT(reader.Read<uint32_t>() == 42);
		TEST_ASSERT(reader.Read<uint32_t>() == 700);
		TEST_ASSERT(reader.ReadString() == "A Reward");
		TEST_ASSERT(reader.Read<uint32_t>() == 0x3f800000);
		TEST_ASSERT(reader.Read<uint32_t>() == 700);
		TEST_ASSERT(reader.Read<int32_t>() == 2);

		TEST_ASSERT(reader.Read<uint32_t>() == 701);
		TEST_ASSERT(reader.Read<uint32_t>() == 1);
		TEST_ASSERT(reader.Read<uint8_t>() == 1);
		TEST_ASSERT(reader.Read<uint8_t>() == 0);
		TEST_ASSERT(reader.ReadString() == "Always");
		TEST_ASSERT(reader.Read<uint32_t>() == static_cast<uint32_t>(RewardWireType::Money));
		TEST_ASSERT(reader.Read<uint32_t>() == 0);
		TEST_ASSERT(reader.Read<uint32_t>() == 0);
		TEST_ASSERT(reader.ReadString() == "Pocket money");
		TEST_ASSERT(reader.Read<uint32_t>() == 4);
		TEST_ASSERT(reader.Read<uint32_t>() == 3);
		TEST_ASSERT(reader.Read<uint32_t>() == 2);
		TEST_ASSERT(reader.Read<uint32_t>() == 1);

		TEST_ASSERT(reader.Read<uint32_t>() == 702);
		TEST_ASSERT(reader.Read<uint32_t>() == 2);
		TEST_ASSERT(reader.Read<uint8_t>() == 0);
		TEST_ASSERT(reader.Read<uint8_t>() == 0);
		TEST_ASSERT(reader.ReadString() == "Choose this");
		TEST_ASSERT(reader.Read<uint32_t>() == static_cast<uint32_t>(RewardWireType::Item));
		TEST_ASSERT(reader.Read<uint32_t>() == 10);
		TEST_ASSERT(reader.Read<uint32_t>() == 11);
		TEST_ASSERT(reader.ReadString() == "An item");
		TEST_ASSERT(reader.Read<uint32_t>() == 1);
		TEST_ASSERT(reader.Read<uint32_t>() == 1001);
		TEST_ASSERT(reader.Read<uint32_t>() == 2);
		TEST_ASSERT(reader.Read<uint8_t>() == 1);
		TEST_ASSERT(reader.Read<uint32_t>() == 99);
		TEST_ASSERT(reader.ReadString() == "Item link");
		TEST_ASSERT(
			reader.Read<uint32_t>() ==
			static_cast<uint32_t>(RewardWireType::AlternateAdvancementPoints)
		);
		TEST_ASSERT(reader.Read<uint32_t>() == 0);
		TEST_ASSERT(reader.Read<uint32_t>() == 0);
		TEST_ASSERT(reader.ReadString() == "AA");
		TEST_ASSERT(reader.Read<uint32_t>() == 5);
		TEST_ASSERT(reader.Read<uint8_t>() == 1);
		TEST_ASSERT(reader.position == reader.size);

		auto second_reward_set = reward_set;
		second_reward_set.pending_reward_id = 43;
		second_reward_set.reward_set_id = 800;
		second_reward_set.title = "Another Reward";
		auto second_packet = SerializeRewardDisplay(&second_reward_set);
		auto bulk = SerializeRewardDisplays({reward_set, second_reward_set});
		Reader bulk_reader{bulk.buffer(), bulk.size()};
		TEST_ASSERT(
			bulk_reader.Read<uint32_t>() ==
				static_cast<uint32_t>(EQ::RewardSelection::Action::Bulk)
		);
		TEST_ASSERT(bulk_reader.Read<int32_t>() == 2);
		TEST_ASSERT(bulk_reader.Read<uint8_t>() == 0);
		constexpr size_t single_header_size = sizeof(uint32_t) + sizeof(uint8_t);
		TEST_ASSERT(
			bulk.size() ==
			sizeof(uint32_t) + sizeof(int32_t) + sizeof(uint8_t) +
			packet.size() - single_header_size +
			second_packet.size() - single_header_size
		);
		TEST_ASSERT(
			std::memcmp(
				bulk.buffer() + bulk_reader.position,
				packet.buffer() + single_header_size,
				packet.size() - single_header_size
			) == 0
		);
		TEST_ASSERT(
			std::memcmp(
				bulk.buffer() + bulk_reader.position +
					packet.size() - single_header_size,
				second_packet.buffer() + single_header_size,
				second_packet.size() - single_header_size
			) == 0
		);

		reward_set.subsets.push_back(choice);
		TEST_THROWS(SerializeRewardDisplay(&reward_set), std::invalid_argument);
	}

	void RewardClaimReplyLayout()
	{
		using namespace EQ::Achievements;
		auto packet = SerializeRewardClaimReply(42, 700, 702, true);
		Reader reader{packet.buffer(), packet.size()};
		TEST_ASSERT(
			reader.Read<uint32_t>() ==
				static_cast<uint32_t>(EQ::RewardSelection::Action::Claim)
		);
		TEST_ASSERT(reader.Read<uint32_t>() == 42);
		TEST_ASSERT(reader.Read<uint32_t>() == 700);
		TEST_ASSERT(reader.Read<uint32_t>() == 702);
		TEST_ASSERT(reader.Read<uint8_t>() == 1);
		TEST_ASSERT(reader.Read<uint8_t>() == 0);
		TEST_ASSERT(reader.Read<uint8_t>() == 0);
		TEST_ASSERT(reader.Read<uint8_t>() == 0);
		TEST_ASSERT(reader.position == reader.size);
	}

	void EmptyInitializationLayout()
	{
		using namespace EQ::Achievements;

		auto definitions = SerializeDefinitions({}, {});
		Reader definitions_reader{definitions.buffer(), definitions.size()};
		TEST_ASSERT(definitions_reader.Read<uint32_t>() == 0);
		TEST_ASSERT(definitions_reader.Read<uint32_t>() == 0);
		TEST_ASSERT(definitions_reader.position == definitions_reader.size);

		auto compressed = CompressDefinitions(definitions);
		Reader compressed_reader{compressed.buffer(), compressed.size()};
		TEST_ASSERT(compressed_reader.Read<uint32_t>() == definitions.size());
		std::vector<char> inflated(definitions.size());
		const auto inflated_size = EQ::InflateData(
			reinterpret_cast<const char *>(
				compressed.buffer() + compressed_reader.position
			),
			static_cast<uint32_t>(
				compressed.size() - compressed_reader.position
			),
			inflated.data(),
			static_cast<uint32_t>(inflated.size())
		);
		TEST_ASSERT(inflated_size == definitions.size());
		TEST_ASSERT(
			std::memcmp(inflated.data(), definitions.buffer(), definitions.size()) == 0
		);

		auto snapshot = SerializeSnapshot({}, {});
		TEST_ASSERT(snapshot.size() == 0);

		auto dense = SerializeDenseUpdate(1, {}, {});
		Reader dense_reader{dense.buffer(), dense.size()};
		TEST_ASSERT(dense_reader.Read<uint32_t>() == 1);
		TEST_ASSERT(dense_reader.Read<uint8_t>() == 1);
		TEST_ASSERT(dense_reader.Read<uint32_t>() == 0);
		TEST_ASSERT(dense_reader.position == dense_reader.size);

		auto progress = SerializeProgress({});
		Reader progress_reader{progress.buffer(), progress.size()};
		TEST_ASSERT(progress_reader.Read<uint32_t>() == 0);
		TEST_ASSERT(progress_reader.position == progress_reader.size);
	}

	void SkillWildcardDoesNotAliasSkillZero()
	{
		using namespace EQ::Achievements;
		TEST_ASSERT(static_cast<uint32_t>(EQ::skills::Skill1HBlunt) == 0);
		TEST_ASSERT(SkillWildcardTargetId != static_cast<uint32_t>(EQ::skills::Skill1HBlunt));
		TEST_ASSERT(SkillWildcardTargetId > static_cast<uint32_t>(EQ::skills::HIGHEST_SKILL));
	}

	void NpcNameIdentityHashLayout()
	{
		using namespace EQ::Achievements;

		TEST_ASSERT(static_cast<uint8_t>(EventType::NpcNameKill) == 12);
		TEST_ASSERT(static_cast<uint8_t>(EventType::SkillCap) == 13);
		TEST_ASSERT(NpcNameIdentityHash("Vishimtar_the_Fallen00") == 0x708BEE77u);
		TEST_ASSERT(
			NpcNameIdentityHash("  VISHIMTAR__the   Fallen 99 ") ==
			0x708BEE77u
		);
		TEST_ASSERT(NpcNameIdentityHash("Tunare`s_Guardian00") == 0xCF16724Eu);
		TEST_ASSERT(NpcNameIdentityHash("Tunare's Guardian") == 0xCF16724Eu);
		TEST_ASSERT(NpcNameIdentityHash("#A_Rat_01") == 0x40FF2A77u);
		TEST_ASSERT(NpcNameIdentityHash(" 123_#- ") == 0);
		TEST_ASSERT(NpcNameIdentityHash("\xC3\x89") == 0);
	}

	void TypeThreeIsPresentationOnly()
	{
		using namespace EQ::Achievements;
		bool rejected = false;
		try {
			SerializeProgress({{42, 303, 9, 3, 1}});
		}
		catch (const std::invalid_argument &) {
			rejected = true;
		}
		TEST_ASSERT(rejected);
	}
};
