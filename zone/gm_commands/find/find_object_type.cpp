#include "../../object.h"
#include "../../client.h"

void FindObjectType(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(2)) {
		const uint32 object_type = Strings::ToUnsignedInt(sep->arg[2]);
		const std::string& object_type_name = ObjectType::GetName(object_type);
		if (Strings::EqualFold(object_type_name, "UNKNOWN OBJECT TYPE")) {

			c->Message(
				Chat::White,
				fmt::format(
					"Object Type {} does not exist.",
					object_type
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Object Type {} | {}",
				object_type,
				ObjectType::GetName(object_type)
			).c_str()
		);

		return;
	}

	const std::string& search_criteria = Strings::ToLower(sep->argplus[2]);

	uint32 found_count = 0;

	for (const auto& e : object_types) {
		const std::string& object_type_name_lower = Strings::ToLower(e.second);
		if (!Strings::Contains(object_type_name_lower, search_criteria)) {
			continue;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Object Type {} | {}",
				e.first,
				e.second
			).c_str()
		);

		found_count++;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Object Type{} found matching '{}'.",
			found_count,
			found_count != 1 ? "s" : "",
			sep->argplus[2]
		).c_str()
	);
}

