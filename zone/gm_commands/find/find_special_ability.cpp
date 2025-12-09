#include "../../client.h"

void FindSpecialAbility(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(2)) {
		const int ability_id = Strings::ToInt(sep->arg[2]);
		const std::string& ability_name = SpecialAbility::GetName(ability_id);
		if (Strings::EqualFold(ability_name, "UNKNOWN SPECIAL ABILITY")) {
			c->Message(
				Chat::White,
				fmt::format(
					"Ability ID {} does not exist.",
					ability_id
				).c_str()
			);
			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Ability {} | {}",
				ability_id,
				ability_name
			).c_str()
		);

		return;
	}

	const std::string& search_criteria = Strings::ToLower(sep->argplus[2]);

	uint32 found_count = 0;

	for (const auto& e : special_ability_names) {
		const std::string& ability_name_lower = Strings::ToLower(e.second);
		if (!Strings::Contains(ability_name_lower, search_criteria)) {
			continue;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Ability {} | {}",
				e.first,
				e.second
			).c_str()
		);

		found_count++;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Abilit{} found matching '{}'.",
			found_count,
			found_count != 1 ? "ies" : "y",
			sep->argplus[2]
		).c_str()
	);
}
