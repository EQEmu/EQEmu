#include "../../client.h"

void FindStance(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(2)) {
		const uint8 stance_id = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[2]));
		const std::string& stance_name = Stance::GetName(stance_id);
		if (Strings::EqualFold(stance_name, "UNKNOWN STANCE")) {
			c->Message(
				Chat::White,
				fmt::format(
					"Stance ID {} does not exist.",
					stance_id
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Stance {} | {}",
				stance_id,
				stance_name
			).c_str()
		);

		return;
	}

	const std::string& search_criteria = Strings::ToLower(sep->argplus[2]);

	uint32 found_count = 0;

	for (const auto& e : stance_names) {
		const std::string& stance_name_lower = Strings::ToLower(e.second);
		if (!Strings::Contains(stance_name_lower, search_criteria)) {
			continue;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Stance {} | {}",
				e.first,
				e.second
			).c_str()
		);

		found_count++;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Stance{} found matching '{}'.",
			found_count,
			found_count != 1 ? "s" : "",
			sep->argplus[2]
		).c_str()
	);
}
