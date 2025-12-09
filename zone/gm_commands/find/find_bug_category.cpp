#include "../../client.h"

void FindBugCategory(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(2)) {
		const uint32 category_id = Strings::ToUnsignedInt(sep->arg[2]);
		const std::string& category_name = Bug::GetName(category_id);
		if (Strings::EqualFold(category_name, "UNKNOWN BUG CATEGORY")) {
			c->Message(
				Chat::White,
				fmt::format(
					"Bug Category ID {} does not exist.",
					category_id
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Bug Category {} | {}",
				category_id,
				category_name
			).c_str()
		);

		return;
	}

	const std::string& search_criteria = Strings::ToLower(sep->argplus[2]);

	uint32 found_count = 0;

	for (const auto& e : bug_category_names) {
		const std::string& bug_category_name_lower = Strings::ToLower(e.second);
		if (!Strings::Contains(bug_category_name_lower, search_criteria)) {
			continue;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Bug Category {} | {}",
				e.first,
				e.second
			).c_str()
		);

		found_count++;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Bug Categor{} found matching '{}'.",
			found_count,
			found_count != 1 ? "ies" : "y",
			sep->argplus[2]
		).c_str()
	);
}
