#include "../../client.h"

void FindClass(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(2)) {
		const auto class_id = Strings::ToInt(sep->arg[2]);
		if (EQ::ValueWithin(class_id, Class::Warrior, Class::Berserker)) {
			const std::string& class_name = GetClassIDName(class_id);
			c->Message(
				Chat::White,
				fmt::format(
					"Class {} | {}{}",
					class_id,
					class_name,
					(
						IsPlayerClass(class_id) ?
						fmt::format(
							" ({})",
							Strings::Commify(GetPlayerClassBit(class_id))
						) :
						""
					)
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Class ID {} was not found.",
				class_id
			).c_str()
		);

		return;
	}

	const auto& search_criteria = Strings::ToLower(sep->argplus[2]);

	auto found_count = 0;

	for (uint16 class_id = Class::Warrior; class_id <= Class::MercenaryLiaison; class_id++) {
		const std::string& class_name       = GetClassIDName(class_id);
		const auto&        class_name_lower = Strings::ToLower(class_name);
		if (!Strings::Contains(class_name_lower, search_criteria)) {
			continue;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Class {} | {}{}",
				class_id,
				class_name,
				(
					IsPlayerClass(class_id) ?
					fmt::format(
						" ({})",
						Strings::Commify(GetPlayerClassBit(class_id))
					) :
					""
				)
			).c_str()
		);

		found_count++;
	}

	c->Message(
		Chat::White,
		fmt::format(
		"{} Class{} found matching '{}'.",
			found_count,
			found_count != 1 ? "es" : "",
			sep->argplus[2]
		).c_str()
	);
}
