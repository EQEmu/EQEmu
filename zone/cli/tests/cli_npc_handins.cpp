
#include "zone/zone_cli.h"

#include "zone/client.h"
#include "zone/zone.h"
#include "common/cli/eqemu_command_handler.h"
#include "common/eqemu_logsys.h"
#include "common/http/httplib.h"
#include "common/json/json.hpp"
#include "common/net/eqstream.h"
#include "common/platform.h"

extern Zone *zone;
using json = nlohmann::json;

void RunTest(const std::string& test_name, const std::string& expected, const std::string& actual);
void RunTest(const std::string& test_name, bool expected, bool actual);
void RunTest(const std::string& test_name, int expected, int actual);

struct HandinEntry {
	std::string            item_id            = "0";
	uint32                 count              = 0;
	const EQ::ItemInstance *item              = nullptr;
	bool                   is_multiquest_item = false; // state
};

struct HandinMoney {
	uint32 platinum = 0;
	uint32 gold     = 0;
	uint32 silver   = 0;
	uint32 copper   = 0;
};

struct Handin {
	std::vector<HandinEntry> items = {}; // items can be removed from this set as successful handins are made
	HandinMoney              money = {}; // money can be removed from this set as successful handins are made
};

struct TestCase {
	std::string description;
	Handin      hand_in;
	Handin      required;
	Handin      returned;
	bool        handin_check_result;
};

void RunSerializedTest(const std::string &test_name, const std::string &expected, const std::string &actual)
{
	if (expected == actual) {
		std::cout << "[✅] " << test_name << " PASSED\n";
	}
	else {
		std::cerr << "[❌] " << test_name << " FAILED\n";
		std::cerr << "   📌 Expected: " << expected << "\n";
		std::cerr << "   ❌ Got:      " << actual << "\n";
		std::exit(1);
	}
}

std::string SerializeHandin(const std::map<std::string, uint32> &items, const HandinMoney &money)
{
	json j;
	j["items"] = items;
	j["money"] = {
		{"platinum", money.platinum},
		{"gold",     money.gold},
		{"silver",   money.silver},
		{"copper",   money.copper}
	};
	return j.dump();
}

void ZoneCLI::TestNpcHandins(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	if (cmd[{"-h", "--help"}]) {
		return;
	}

	EQEmuLogSys::Instance()->SilenceConsoleLogging();

	Zone::Bootup(ZoneID("qrg"), 0, false);
	zone->StopShutdownTimer();

	entity_list.Process();
	entity_list.MobProcess();

	std::cout << "===========================================\n";
	std::cout << "⚙\uFE0F> Running Hand-in Tests...\n";
	std::cout << "===========================================\n\n";

	Client *c       = new Client();
	auto   npc_type = content_db.LoadNPCTypesData(754008);
	if (npc_type) {
		auto npc = new NPC(
			npc_type,
			nullptr,
			glm::vec4(0, 0, 0, 0),
			GravityBehavior::Water
		);

		entity_list.AddNPC(npc);

		std::vector<TestCase> test_cases = {
			TestCase{
				.description = "Test basic cloth-cap hand-in",
				.hand_in = {
					.items = {
						HandinEntry{.item_id = "1001", .count = 1},
					},
				},
				.required = {
					.items = {
						HandinEntry{.item_id = "1001", .count = 1},
					},
				},
				.returned = {},
				.handin_check_result = true,
			},
			TestCase{
				.description = "Test basic cloth-cap hand-in failure",
				.hand_in = {
					.items = {
						HandinEntry{.item_id = "9997", .count = 1},
					},
				},
				.required = {
					.items = {
						HandinEntry{.item_id = "1001", .count = 1},
					},
				},
				.returned = {
					.items = {
						HandinEntry{.item_id = "9997", .count = 1},
					},
				},
				.handin_check_result = false,
			},
			TestCase{
				.description = "Test basic cloth-cap hand-in failure from handing in too many",
				.hand_in = {
					.items = {
						HandinEntry{.item_id = "9997", .count = 1},
						HandinEntry{.item_id = "9997", .count = 1},
					},
				},
				.required = {
					.items = {
						HandinEntry{.item_id = "1001", .count = 1},
					},
				},
				.returned = {
					.items = {
						HandinEntry{.item_id = "9997", .count = 1},
						HandinEntry{.item_id = "9997", .count = 1},
					},
				},
				.handin_check_result = false,
			},
			TestCase{
				.description = "Test handing in money",
				.hand_in = {
					.items = {},
					.money = {.platinum = 1},
				},
				.required = {
					.items = {},
					.money = {.platinum = 1},
				},
				.returned = {},
				.handin_check_result = true,
			},
			TestCase{
				.description = "Test handing in money, but not enough",
				.hand_in = {
					.items = {},
					.money = {.platinum = 1},
				},
				.required = {
					.items = {},
					.money = {.platinum = 100},
				},
				.returned = {
					.items = {},
					.money = {.platinum = 1},
				},
				.handin_check_result = false,
			},
			TestCase{
				.description = "Test handing in money, but not enough of any type",
				.hand_in = {
					.items = {},
					.money = {.platinum = 1, .gold = 1, .silver = 1, .copper = 1},
				},
				.required = {
					.items = {},
					.money = {.platinum = 100, .gold = 100, .silver = 100, .copper = 100},
				},
				.returned = {
					.items = {},
					.money = {.platinum = 1, .gold = 1, .silver = 1, .copper = 1},
				},
				.handin_check_result = false,
			},
			TestCase{
				.description = "Test handing in money of all types",
				.hand_in = {
					.items = {},
					.money = {.platinum = 1, .gold = 1, .silver = 1, .copper = 1},
				},
				.required = {
					.items = {},
					.money = {.platinum = 1, .gold = 1, .silver = 1, .copper = 1},
				},
				.returned = {},
				.handin_check_result = true,
			},
			TestCase{
				.description = "Test handing in platinum with items with success",
				.hand_in = {
					.items = {
						HandinEntry{.item_id = "1001", .count = 1},
					},
					.money = {.platinum = 1},
				},
				.required = {
					.items = {
						HandinEntry{.item_id = "1001", .count = 1},
					},
					.money = {.platinum = 1},
				},
				.returned = {},
				.handin_check_result = true,
			},
			TestCase{
				.description = "Test handing in platinum with items with failure",
				.hand_in = {
					.items = {
						HandinEntry{.item_id = "1001", .count = 1},
					},
					.money = {.platinum = 1},
				},
				.required = {
					.items = {
						HandinEntry{.item_id = "1001", .count = 1},
					},
					.money = {.platinum = 100},
				},
				.returned = {
					.items = {
						HandinEntry{
							.item_id = "1001", .count = 0,
						},
					},
					.money = {.platinum = 1},
				},
				.handin_check_result = false,
			},
			TestCase{
				.description = "Test returning money and items",
				.hand_in = {
					.items = {
						HandinEntry{.item_id = "1007", .count = 1},
					},
					.money = {
						.platinum = 1,
						.gold    = 666,
						.silver  = 234,
						.copper  = 444,
					},
				},
				.required = {
					.items = {
						HandinEntry{.item_id = "1001", .count = 1},
					},
					.money = {.platinum = 100},
				},
				.returned = {
					.items = {
						HandinEntry{.item_id = "1007", .count = 1},
					},
					.money = {
						.platinum = 1,
						.gold    = 666,
						.silver  = 234,
						.copper  = 444,
					},
				},
				.handin_check_result = false,
			},
			TestCase{
				.description = "Test returning money",
				.hand_in = {
					.items = {},
					.money = {
						.platinum = 1,
						.gold    = 666,
						.silver  = 234,
						.copper  = 444,
					},
				},
				.required = {
					.items = {},
					.money = {.platinum = 100},
				},
				.returned = {
					.items = {
					},
					.money = {
						.platinum = 1,
						.gold    = 666,
						.silver  = 234,
						.copper  = 444,
					},
				},
				.handin_check_result = false,
			},
			TestCase{
				.description = "Test handing in many items of the same required item",
				.hand_in = {
					.items = {
						HandinEntry{.item_id = "1007", .count = 1},
						HandinEntry{.item_id = "1007", .count = 1},
						HandinEntry{.item_id = "1007", .count = 1},
						HandinEntry{.item_id = "1007", .count = 1},
					},
					.money = {},
				},
				.required = {
					.items = {
						HandinEntry{.item_id = "1007", .count = 1},
					},
					.money = {},
				},
				.returned = {
					.items = {
						HandinEntry{.item_id = "1007", .count = 1},
						HandinEntry{.item_id = "1007", .count = 1},
						HandinEntry{.item_id = "1007", .count = 1},
					},
					.money = {},
				},
				.handin_check_result = true,
			},
			TestCase{
				.description = "Test handing in item of a stack",
				.hand_in = {
					.items = {
						HandinEntry{.item_id = "13005", .count = 20},
					},
					.money = {},
				},
				.required = {
					.items = {
						HandinEntry{.item_id = "13005", .count = 20},
					},
					.money = {},
				},
				.returned = {
					.items = {},
					.money = {},
				},
				.handin_check_result = true,
			},
			TestCase{
				.description = "Test handing in item of a stack but not enough",
				.hand_in = {
					.items = {
						HandinEntry{.item_id = "13005", .count = 10},
					},
					.money = {},
				},
				.required = {
					.items = {
						HandinEntry{.item_id = "13005", .count = 20},
					},
					.money = {},
				},
				.returned = {
					.items = {
						HandinEntry{.item_id = "13005", .count = 10},
					},
					.money = {},
				},
				.handin_check_result = false,
			},
			TestCase{
				.description = "Test handing in 4 non-stacking helmets when 4 are required",
				.hand_in = {
					.items = {
						HandinEntry{.item_id = "29062", .count = 1},
						HandinEntry{.item_id = "29062", .count = 1},
						HandinEntry{.item_id = "29062", .count = 1},
						HandinEntry{.item_id = "29062", .count = 1},
					},
					.money = {},
				},
				.required = {
					.items = {
						HandinEntry{.item_id = "29062", .count = 4},
					},
					.money = {},
				},
				.returned = {
					.items = {
					},
					.money = {},
				},
				.handin_check_result = true,
			},
			TestCase{
				.description = "Test handing in Soulfire that has 5 charges and have it count as 1 item",
				.hand_in = {
					.items = {
						HandinEntry{.item_id = "5504", .count = 1},
					},
					.money = {},
				},
				.required = {
					.items = {
						HandinEntry{.item_id = "5504", .count = 1},
					},
					.money = {},
				},
				.returned = {
					.items = {
					},
					.money = {},
				},
				.handin_check_result = true,
			},
		};

		std::map<std::string, uint32>   hand_ins;
		std::map<std::string, uint32>   required;
		std::vector<EQ::ItemInstance *> items;

		EQEmuLogSys::Instance()->EnableConsoleLogging();

		// turn this on to see debugging output
		EQEmuLogSys::Instance()->log_settings[Logs::NpcHandin].log_to_console = std::getenv("DEBUG") ? 3 : 0;

		for (auto &test: test_cases) {
			hand_ins.clear();
			required.clear();
			items.clear();

			for (auto &hand_in: test.hand_in.items) {
				auto             item_id = Strings::ToInt(hand_in.item_id);
				EQ::ItemInstance *inst   = database.CreateItem(item_id);
				if (inst->IsStackable()) {
					inst->SetCharges(hand_in.count);
				}

				if (inst->GetItem()->MaxCharges > 0) {
					inst->SetCharges(inst->GetItem()->MaxCharges);
				}

				hand_ins[hand_in.item_id] = inst->GetCharges();
				items.push_back(inst);
			}

			// money
			if (test.hand_in.money.platinum > 0) {
				hand_ins["platinum"] = test.hand_in.money.platinum;
			}
			if (test.hand_in.money.gold > 0) {
				hand_ins["gold"] = test.hand_in.money.gold;
			}
			if (test.hand_in.money.silver > 0) {
				hand_ins["silver"] = test.hand_in.money.silver;
			}
			if (test.hand_in.money.copper > 0) {
				hand_ins["copper"] = test.hand_in.money.copper;
			}

			for (auto &req: test.required.items) {
				required[req.item_id] = req.count;
			}

			// money
			if (test.required.money.platinum > 0) {
				required["platinum"] = test.required.money.platinum;
			}
			if (test.required.money.gold > 0) {
				required["gold"] = test.required.money.gold;
			}
			if (test.required.money.silver > 0) {
				required["silver"] = test.required.money.silver;
			}
			if (test.required.money.copper > 0) {
				required["copper"] = test.required.money.copper;
			}

			auto result = npc->CheckHandin(c, hand_ins, required, items);

			RunTest(test.description, test.handin_check_result, result);

			auto returned = npc->ReturnHandinItems(c);

			std::map<std::string, uint32> returned_items;
			HandinMoney                   returned_money{};

			// Serialize returned items
			for (const auto &ret: returned.items) {
//				if (ret.item->IsStackable() && ret.item->GetCharges() != ret.count) {
//					ret.item->SetCharges(ret.count);
//				}
				returned_items[ret.item_id] += ret.count;
			}

			// Serialize returned money
			returned_money.platinum = returned.money.platinum;
			returned_money.gold     = returned.money.gold;
			returned_money.silver   = returned.money.silver;
			returned_money.copper   = returned.money.copper;

			// Serialize expected and actual return values for comparison
			std::map<std::string, uint32> expected_returned_items;
			for (const auto               &entry: test.returned.items) {
				expected_returned_items[entry.item_id] += entry.count;
			}

			std::string expected_serialized = SerializeHandin(
				expected_returned_items,
				test.returned.money
			);

			std::string actual_serialized = SerializeHandin(returned_items, returned_money);

			// Run serialization check test
			RunSerializedTest(test.description + " (Return Validation)", expected_serialized, actual_serialized);

			npc->ResetHandin();

			if (EQEmuLogSys::Instance()->log_settings[Logs::NpcHandin].log_to_console > 0) {
				std::cout << std::endl;
			}
		}
	}

	std::cout << "\n===========================================\n";
	std::cout << "✅ All NPC Hand-in Tests Completed!\n";
	std::cout << "===========================================\n";
}
