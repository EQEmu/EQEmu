
#include "zone/zone_cli.h"

#include "zone/client.h"
#include "zone/zone.h"
#include "common/cli/eqemu_command_handler.h"
#include "common/eqemu_logsys.h"
#include "common/net/eqstream.h"
#include "common/platform.h"

extern Zone *zone;

void RunTest(const std::string& test_name, const std::string& expected, const std::string& actual);
void RunTest(const std::string& test_name, bool expected, bool actual);
void RunTest(const std::string& test_name, int expected, int actual);

void ZoneCLI::TestNpcHandinsMultiQuest(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	if (cmd[{"-h", "--help"}]) {
		return;
	}

	uint32 break_length = 50;
	int    failed_count = 0;

	EQEmuLogSys::Instance()->SilenceConsoleLogging();

	Zone::Bootup(ZoneID("qrg"), 0, false);
	zone->StopShutdownTimer();

	entity_list.Process();
	entity_list.MobProcess();

	std::cout << "===========================================\n";
	std::cout << "⚙\uFE0F> Running Hand-in Tests (Multi-Quest)...\n";
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
		npc->MultiQuestEnable();

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
			std::string description = "";
			Handin      hand_in;
			Handin      required;
			Handin      returned;
			bool        handin_check_result;
		};

		std::vector<TestCase> test_cases = {
			TestCase{
				.description = "Journeyman's Boots",
				.hand_in = {
					.items = {
						HandinEntry{.item_id = "12268", .count = 1},
						HandinEntry{.item_id = "7100", .count = 1},
					},
					.money = {.platinum = 325},
				},
				.required = {
					.items = {
						HandinEntry{.item_id = "12268", .count = 1},
						HandinEntry{.item_id = "7100", .count = 1},
					},
					.money = {.platinum = 325},
				},
				.returned = {},
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
			required.clear();

			for (auto &hand_in: test.hand_in.items) {
				hand_ins.clear();
				items.clear();

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

				npc->CheckHandin(c, hand_ins, required, items);
				npc->ResetHandin();
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

			npc->ResetHandin();

			if (EQEmuLogSys::Instance()->log_settings[Logs::NpcHandin].log_to_console > 0) {
				std::cout << std::endl;
			}
		}
	}

	std::cout << "\n===========================================\n";
	std::cout << "✅ All NPC Hand-in Tests Completed (Multi-Quest)!\n";
	std::cout << "===========================================\n";
}
