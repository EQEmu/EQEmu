#include "common/content/world_content_service.h"
#include "common/http/httplib.h"
#include "zone/client.h"

#include "zone/bot.h"

void command_gearup(Client *c, const Seperator *sep)
{
	Mob* t = c;
	if (
		c->GetTarget() &&
		(
			(c->GetTarget()->IsClient() && c->GetGM()) ||
			c->GetTarget()->IsBot()
		)
	) {
		t = c->GetTarget();
	}

	const std::string tool_table_name = "tool_gearup_armor_sets";
	if (!database.DoesTableExist(tool_table_name)) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"Table [{}] does not exist. Downloading from Github and installing...",
				tool_table_name
			).c_str()
		);

		// http get request
		httplib::Client cli("https://raw.githubusercontent.com");
		cli.set_connection_timeout(0, 15000000); // 15 sec
		cli.set_read_timeout(15, 0); // 15 seconds
		cli.set_write_timeout(15, 0); // 15 seconds

		int         sourced_queries = 0;
		std::string url             = "/EQEmu/Server/master/utils/sql/git/optional/2020_07_20_tool_gearup_armor_sets.sql";

		if (auto res = cli.Get(url.c_str())) {
			if (res->status == 200) {
				for (auto &s: Strings::Split(res->body, ';')) {
					if (!Strings::Trim(s).empty()) {
						auto results = database.QueryDatabase(s);
						if (!results.ErrorMessage().empty()) {
							c->Message(
								Chat::Yellow,
								fmt::format(
									"Error sourcing SQL [{}]", results.ErrorMessage()
								).c_str()
							);
							return;
						}
						sourced_queries++;
					}
				}
			}
		} else {
			c->Message(
				Chat::Yellow,
				fmt::format(
					"Error retrieving URL [{}]",
					url
				).c_str()
			);
		}

		c->Message(
			Chat::Yellow,
			fmt::format(
				"Table [{}] installed. Sourced [{}] quer{}",
				tool_table_name,
				sourced_queries,
				sourced_queries != 1 ? "ies" : "y"
			).c_str()
		);
	}

	const std::string expansion_arg = sep->arg[1];
	std::string expansion_filter;
	if (!expansion_arg.empty()) {
		expansion_filter = fmt::format(" AND `expansion` = {}", expansion_arg);
	}

	auto query = fmt::format(
		"SELECT item_id, slot FROM {} WHERE "
		"`class` = {} AND `level` = {}{} "
		"ORDER BY score DESC, expansion DESC",
		tool_table_name,
		t->GetClass(),
		t->GetLevel(),
		expansion_filter
	);
	auto results = database.QueryDatabase(query);

	int           items_equipped     = 0;
	int           items_already_have = 0;
	std::set<int> equipped;

	for (auto row : results) {
		auto item_id = Strings::ToUnsignedInt(row[0]);
		auto slot_id = static_cast<uint16>(Strings::ToUnsignedInt(row[1]));

		if (equipped.find(slot_id) != equipped.end()) {
			if (slot_id == EQ::invslot::slotEar1) {
				slot_id = EQ::invslot::slotEar2;
			} else if (slot_id == EQ::invslot::slotFinger1) {
				slot_id = EQ::invslot::slotFinger2;
			} else if (slot_id == EQ::invslot::slotWrist1) {
				slot_id = EQ::invslot::slotWrist2;
			}
		}

		if (equipped.find(slot_id) == equipped.end()) {
			const auto *item = database.GetItem(item_id);
			bool has_item = false;
			if (t->IsClient()) {
				has_item = t->CastToClient()->GetInv().HasItem(item_id, 1, invWhereWorn) != INVALID_INDEX;
			} else if (t->IsBot()) {
				has_item = t->CastToBot()->HasBotItem(item_id) != INVALID_INDEX;
			}

			bool can_wear_item = false;
			if (t->IsClient()) {
				can_wear_item = !t->CastToClient()->CheckLoreConflict(item) && !has_item;
			} else if (t->IsBot()) {
				can_wear_item = !t->CastToBot()->CheckLoreConflict(item) && !has_item;
			}

			if (!can_wear_item) {
				items_already_have++;
			}

			if (
				can_wear_item &&
				t->CanClassEquipItem(item_id) &&
				(
					t->CanRaceEquipItem(item_id) ||
					(t->IsBot() && !t->CanRaceEquipItem(item_id) && RuleB(Bots, AllowBotEquipAnyRaceGear))
				)
			) {
				equipped.insert(slot_id);

				if (t->IsClient()) {
					t->CastToClient()->SummonItem(
						item_id,
						0, 0, 0, 0, 0, 0, 0, 0,
						slot_id
					);
				} else if (t->IsBot()) {
					t->CastToBot()->AddBotItem(slot_id, item_id);
				}

				items_equipped++;
			}
		}
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Equipped items [{}] already had [{}] items equipped",
			items_equipped,
			items_already_have
		).c_str()
	);

	if (expansion_arg.empty()) {
		query = fmt::format(
			"SELECT expansion FROM {} WHERE "
			"class = {} AND level = {} "
			"GROUP BY expansion",
			tool_table_name,
			t->GetClass(),
			t->GetLevel()
		);
		results = database.QueryDatabase(query);

		c->Message(Chat::White, "Choose Armor by Expansion:");
		std::string message;
		for (auto row : results) {
			const auto expansion = Strings::ToInt(row[0]);
			message += fmt::format(
				"[{}] ",
				Saylink::Silent(
					fmt::format("#gearup {}", expansion),
					Expansion::ExpansionName[expansion]
				)
			);

			if (message.length() > 2000) {
				c->Message(Chat::White, message.c_str());
				message = std::string();
			}
		}

		if (!message.empty()) {
			c->Message(Chat::White, message.c_str());
		}
	}
}
