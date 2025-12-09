#include "../../client.h"

void FindAA(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(2)) {
		const auto  aa_id   = Strings::ToInt(sep->arg[2]);
		const auto& aa_name = zone->GetAAName(aa_id);
		if (!aa_name.empty()) {
			c->Message(
				Chat::White,
				fmt::format(
					"AA {} | {}",
					aa_id,
					aa_name
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"AA ID {} was not found.",
				aa_id
			).c_str()
		);

		return;
	}

	const auto& search_criteria = Strings::ToLower(sep->argplus[2]);

	auto found_count = 0;

	std::map<int, std::string> ordered_aas;

	for (const auto &a: zone->aa_abilities) {
		ordered_aas[a.second.get()->first->id] = a.second.get()->name;
	}

	for (const auto &a: ordered_aas) {
		const auto& aa_name = zone->GetAAName(a.first);
		if (!aa_name.empty()) {
			const auto& aa_name_lower = Strings::ToLower(aa_name);
			if (!Strings::Contains(aa_name_lower, search_criteria)) {
				continue;
			}

			c->Message(
				Chat::White,
				fmt::format(
					"AA {} | {}",
					a.first,
					aa_name
				).c_str()
			);

			found_count++;

			if (found_count == 50) {
				break;
			}
		}
	}

	if (found_count == 50) {
		c->Message(
			Chat::White,
			fmt::format(
				"50 AAs were found matching '{}', max reached.",
				sep->argplus[2]
			).c_str()
		);

		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} AA{} found matching '{}'.",
			found_count,
			found_count != 1 ? "s" : "",
			sep->argplus[2]
		).c_str()
	);
}
