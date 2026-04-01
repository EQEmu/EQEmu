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
#include "common/repositories/tradeskill_recipe_repository.h"
#include "zone/client.h"
#include "zone/command.h"

void FindRecipe(Client *c, const Seperator *sep)
{
	const auto can_view_recipes = c->Admin() >= GetCommandStatus("viewrecipe");

	if (sep->IsNumber(2)) {
		const auto recipe_id = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[2]));

		const auto& l = TradeskillRecipeRepository::GetWhere(
			content_db,
			fmt::format("id = {}", recipe_id)
		);

		if (l.empty() || !l[0].id) {
			c->Message(
				Chat::White,
				fmt::format(
					"Recipe ID {} could not be found.",
					recipe_id
				).c_str()
			);
			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Recipe {} | {}{}",
				recipe_id,
				l[0].name,
				(
					can_view_recipes ?
					fmt::format(
						" | {}",
						Saylink::Silent(
							fmt::format(
								"#viewrecipe {}",
								l[0].id
							),
							"View"
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

	const auto& l = TradeskillRecipeRepository::GetWhere(
		content_db,
		fmt::format(
			"LOWER(`name`) LIKE '%%{}%%' ORDER BY `id` ASC",
			search_criteria
		)
	);

	if (l.empty() || !l[0].id) {
		c->Message(
			Chat::White,
			fmt::format(
				"No recipes were found matching '{}'.",
				sep->argplus[2]
			).c_str()
		);

		return;
	}

	for (const auto& e : l) {
		c->Message(
			Chat::White,
			fmt::format(
				"Recipe {} | {}{}",
				Strings::Commify(e.id),
				e.name,
				(
					can_view_recipes ?
					fmt::format(
						" | {}",
						Saylink::Silent(
							fmt::format(
								"#viewrecipe {}",
								e.id
							),
							"View"
						)
					) :
					""
				)
			).c_str()
		);

		if (found_count == 50) {
			break;
		}

		found_count++;
	}

	if (found_count == 50) {
		c->Message(
			Chat::White,
			fmt::format(
				"50 Recipes found matching '{}', max reached.",
				sep->argplus[2]
			).c_str()
		);

		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Recipe{} found matching '{}'.",
			found_count,
			found_count != 1 ? "s" : "",
			sep->argplus[2]
		).c_str()
	);
}
