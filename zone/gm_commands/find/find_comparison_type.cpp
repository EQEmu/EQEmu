#include "../../client.h"

void FindComparisonType(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(2)) {
		const uint8 type = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[2]));
		const std::string& type_name = ComparisonType::GetName(type);
		if (Strings::EqualFold(type_name, "UNKNOWN COMPARISON TYPE")) {
			c->Message(
				Chat::White,
				fmt::format(
					"Comparison Type {} does not exist.",
					type
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Comparison Type {} | {}",
				type,
				type_name
			).c_str()
		);

		return;
	}

	const std::string& search_criteria = Strings::ToLower(sep->argplus[2]);

	uint32 found_count = 0;

	for (const auto& e : comparison_types) {
		const std::string& type_name_lower = Strings::ToLower(e.second);
		if (!Strings::Contains(type_name_lower, search_criteria)) {
			continue;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Comparison Type {} | {}",
				e.first,
				e.second
			).c_str()
		);

		found_count++;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Comparison Type{} found matching '{}'.",
			found_count,
			found_count != 1 ? "s" : "",
			sep->argplus[2]
		).c_str()
	);
}
