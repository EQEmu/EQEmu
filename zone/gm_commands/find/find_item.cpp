/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include "zone/client.h"
#include "common/repositories/items_repository.h"

void FindItem(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(2)) {
		const auto  item_id = Strings::ToUnsignedInt(sep->arg[2]);
		const auto* item    = database.GetItem(item_id);
		if (item) {
			std::string summon_links = Saylink::Silent(
				fmt::format(
					"#si {}",
					item_id
				),
				"X"
			);

			if (item->Stackable && item->StackSize > 1) {
				summon_links += fmt::format(
					" | {}",
					Saylink::Silent(
						fmt::format(
							"#si {} {}",
							item_id,
							item->StackSize
						),
						std::to_string(item->StackSize)
					)
				);
			}

			c->Message(
				Chat::White,
				fmt::format(
					"{} | {}",
					summon_links,
					database.CreateItemLink(item_id)
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Item ID {} not found",
				item_id
			).c_str()
		);

		return;
	}

	const auto& search_criteria = Strings::ToLower(sep->argplus[2]);

	const auto& l = ItemsRepository::GetItemIDsBySearchCriteria(content_db, search_criteria, 50);

	if (l.empty()) {
		c->Message(
			Chat::White,
			fmt::format(
				"No items were found matching '{}'.",
				sep->argplus[2]
			).c_str()
		);

		return;
	}

	auto found_count = 0;

	for (const auto& e : l) {
		const auto item = database.GetItem(e);
		if (!item) {
			continue;
		}

		std::string summon_links = Saylink::Silent(
			fmt::format(
				"#si {}",
				e
			),
			"X"
		);

		if (item->Stackable && item->StackSize > 1) {
			summon_links += fmt::format(
				" | {}",
				Saylink::Silent(
					fmt::format(
						"#si {} {}",
						e,
						item->StackSize
					),
					std::to_string(item->StackSize)
				)
			);
		}

		c->Message(
			Chat::White,
			fmt::format(
				"{} | {} ({})",
				summon_links,
				database.CreateItemLink(e),
				item->ID
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
				"50 Items found matching '{}', max reached.",
				sep->argplus[2]
			).c_str()
		);

		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Item{} found matching '{}'.",
			found_count,
			found_count != 1 ? "s" :"",
			sep->argplus[2]
		).c_str()
	);
}

