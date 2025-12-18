#include "common/repositories/tradeskill_recipe_entries_repository.h"
#include "common/repositories/tradeskill_recipe_repository.h"
#include "zone/client.h"
#include "zone/command.h"

void ShowRecipe(Client *c, const Seperator *sep)
{
	if (!sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #show recipe [Recipe ID]");
		return;
	}

	const uint32 recipe_id = Strings::ToUnsignedInt(sep->arg[2]);

	const auto& re = TradeskillRecipeEntriesRepository::GetWhere(
		content_db,
		fmt::format("recipe_id = {} ORDER BY id ASC", recipe_id)
	);

	const auto& r = TradeskillRecipeRepository::GetWhere(
		content_db,
		fmt::format("id = {}", recipe_id)
	);

	if (re.empty() || r.empty()) {
		c->Message(
			Chat::White,
			fmt::format(
				"Recipe ID {} has no entries or could not be found.",
				recipe_id
			).c_str()
		);
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Recipe {} | {}",
			recipe_id,
			r[0].name
		).c_str()
	);

	uint32 entry_number = 1;
	const bool can_summon_items = c->Admin() >= GetCommandStatus("summonitem");

	for (const auto& e : re) {
		c->Message(
			Chat::White,
			fmt::format(
				"Entry {}{} | {}{}",
				entry_number,
				e.iscontainer > 0 ? " (Container)" : "",
				(
					e.item_id > 1000 ?
					database.CreateItemLink(static_cast<uint32>(e.item_id)) :
					ObjectType::GetName(static_cast<uint32>(e.item_id))
				),
				(
					can_summon_items && e.item_id > 1000 ?
					fmt::format(
						" | {}",
						Saylink::Silent(
							fmt::format("#si {}", e.item_id),
							"Summon"
						)
					) :
					""
				)
			).c_str()
		);

		std::vector<std::string> emv;
		bool has_message = false;

		if (e.componentcount) {
			emv.push_back(
				fmt::format(
					"Component: {}",
					e.componentcount
				)
			);

			has_message = true;
		}

		if (e.failcount) {
			emv.push_back(
				fmt::format(
					"Fail: {}",
					e.failcount
				)
			);

			has_message = true;
		}

		if (e.salvagecount) {
			emv.push_back(
				fmt::format(
					"Salvage: {}",
					e.salvagecount
				)
			);

			has_message = true;
		}

		if (e.successcount) {
			emv.push_back(
				fmt::format(
					"Success: {}",
					e.successcount
				)
			);

			has_message = true;
		}

		if (has_message) {
			c->Message(
				Chat::White,
				fmt::format(
					"Entry {} Counts | {}",
					entry_number,
					Strings::Implode(" | ", emv)
				).c_str()
			);
		}

		entry_number++;
	}
}
