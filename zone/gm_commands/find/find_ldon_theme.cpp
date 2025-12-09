#include "../../client.h"

void FindLDoNTheme(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(2)) {
		const uint32 theme_id = Strings::ToUnsignedInt(sep->arg[2]);
		if (LDoNTheme::IsValid(theme_id)) {
			c->Message(
				Chat::White,
				fmt::format(
					"Theme {} | {} ({})",
					theme_id,
					LDoNTheme::GetName(theme_id),
					LDoNTheme::GetBitmask(theme_id)
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Theme ID {} was not found.",
				theme_id
			).c_str()
		);

		return;
	}

	const std::string& search_criteria = Strings::ToLower(sep->argplus[2]);

	uint32 found_count = 0;

	for (const auto& l : ldon_theme_names) {
		const std::string& ldon_theme_name_lower = Strings::ToLower(l.second.first);
		if (!Strings::Contains(ldon_theme_name_lower, search_criteria)) {
			continue;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Theme {} | {} ({})",
				l.first,
				l.second.first,
				l.second.second
			).c_str()
		);

		found_count++;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Theme{} found matching '{}'.",
			found_count,
			found_count != 1 ? "s" : "",
			sep->argplus[2]
		).c_str()
	);
}

