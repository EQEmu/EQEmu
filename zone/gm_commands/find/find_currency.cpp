#include "../../client.h"

void FindCurrency(Client *c, const Seperator *sep)
{
	const auto can_summon_items = c->Admin() >= GetCommandStatus("summonitem");

	if (sep->IsNumber(2)) {
		const auto item_id     = Strings::ToUnsignedInt(sep->arg[2]);
		const auto currency_id = zone->GetCurrencyID(item_id);

		if (!currency_id) {
			c->Message(
				Chat::White,
				fmt::format(
					"There is no currency with an item ID of {}.",
					item_id
				).c_str()
			);

			return;
		}

		const auto item_data = database.GetItem(item_id);
		if (!item_data) {
			c->Message(
				Chat::White,
				fmt::format(
					"Item ID {} does not exist.",
					item_id
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Currency {} | {}{}",
				currency_id,
				database.CreateItemLink(item_id),
				(
					can_summon_items ?
					fmt::format(
						" | {}",
						Saylink::Silent(
							fmt::format(
								"#summonitem {} {}",
								item_id,
								item_data->StackSize
							),
							"Summon"
						)
					) :
					""
				)
			).c_str()
		);

		return;
	}

	const auto& search_criteria = Strings::ToLower(sep->argplus[2]);

	auto found_count = 0;

	for (const auto& e : zone->AlternateCurrencies) {
		const auto item_data = database.GetItem(e.item_id);
		if (!item_data) {
			continue;
		}

		const auto& item_name = Strings::ToLower(item_data->Name);

		if (!Strings::Contains(item_name, search_criteria)) {
			continue;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Currency {} | {}{}",
				e.id,
				database.CreateItemLink(e.item_id),
				(
					can_summon_items ?
					fmt::format(
						" | {}",
						Saylink::Silent(
							fmt::format(
								"#summonitem {} {}",
								e.item_id,
								item_data->StackSize
							),
							"Summon"
						)
					) :
					""
				)
			).c_str()
		);

		found_count++;

		if (found_count == 50) {
			break;
		}
	}

	if (found_count == 50) {
		c->Message(
			Chat::White,
			fmt::format(
				"50 Currencies found matching '{}', max reached.",
				sep->argplus[2]
			).c_str()
		);

		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Currenc{} found matching '{}'.",
			found_count,
			found_count != 1 ? "ies" : "y",
			sep->argplus[2]
		).c_str()
	);
}
