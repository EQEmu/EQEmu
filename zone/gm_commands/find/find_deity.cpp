#include "../../client.h"

void FindDeity(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(2)) {
		const uint32 deity_id = Strings::ToUnsignedInt(sep->arg[2]);
		const std::string& deity_name = Deity::GetName(deity_id);
		if (Strings::EqualFold(deity_name, "UNKNOWN DEITY")) {
			c->Message(
				Chat::White,
				fmt::format(
					"Deity ID {} does not exist.",
					deity_id
				).c_str()
			);
			return;
		}

		const uint32 deity_bitmask = Deity::GetBitmask(deity_id);

		c->Message(
			Chat::White,
			fmt::format(
				"Deity {} | {} ({})",
				deity_id,
				deity_name,
				Strings::Commify(deity_bitmask)
			).c_str()
		);

		return;
	}

	const std::string& search_criteria = Strings::ToLower(sep->argplus[2]);

	uint32 found_count = 0;

	for (const auto& d : deity_names) {
		const std::string& deity_name_lower = Strings::ToLower(d.second);
		if (!Strings::Contains(deity_name_lower, search_criteria)) {
			continue;
		}

		const uint32 deity_bitmask = Deity::GetBitmask(d.first);

		c->Message(
			Chat::White,
			fmt::format(
				"Deity {} | {} ({})",
				d.first,
				d.second,
				Strings::Commify(deity_bitmask)
			).c_str()
		);

		found_count++;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Deit{} found matching '{}'.",
			found_count,
			found_count != 1 ? "ies" : "y",
			sep->argplus[2]
		).c_str()
	);
}
