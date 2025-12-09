#include "../../client.h"

void FindBodyType(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(2)) {
		const uint8 body_type_id = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[2]));
		const std::string& body_type_name = BodyType::GetName(body_type_id);
		if (Strings::EqualFold(body_type_name, "UNKNOWN BODY TYPE")) {
			c->Message(
				Chat::White,
				fmt::format(
					"Body Type {} does not exist.",
					body_type_id
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Body Type {} | {}",
				body_type_id,
				body_type_name
			).c_str()
		);
		return;
	}

	const std::string& search_criteria = Strings::ToLower(sep->argplus[2]);

	uint32 found_count = 0;

	for (const auto& e : body_type_names) {
		const std::string& body_type_name_lower = Strings::ToLower(e.second);
		if (!Strings::Contains(body_type_name_lower, search_criteria)) {
			continue;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Body Type {} | {}",
				e.first,
				e.second
			).c_str()
		);

		found_count++;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Body Type{} found matching '{}'.",
			found_count,
			found_count != 1 ? "s" : "",
			sep->argplus[2]
		).c_str()
	);
}
