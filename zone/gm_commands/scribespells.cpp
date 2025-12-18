#include "zone/client.h"

void command_scribespells(Client *c, const Seperator *sep)
{
	auto arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #scribespells [Max Level] [Min Level]");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM()) {
		t = c->GetTarget()->CastToClient();
	}

	uint8 rule_max_level = RuleI(Character, MaxLevel);
	auto max_level = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[1]));
	uint8 min_level = (
		sep->IsNumber(2) ?
		static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[2])) :
		1
	); // Default to Level 1 if there isn't a 2nd argument

	if (!c->GetGM()) { // Default to Character:MaxLevel if we're not a GM and Level is higher than the max level
		if (max_level > rule_max_level) {
			max_level = rule_max_level;
		}

		if (min_level > rule_max_level) {
			min_level = rule_max_level;
		}
	}

	if (max_level < 1 || min_level < 1) {
		c->Message(Chat::White, "Maximum Level and Minimum Level must be greater than 0.");
		return;
	}

	if (min_level > max_level) {
		c->Message(Chat::White, "Maximum Level must be greater than Minimum Level.");
		return;
	}

	const auto scribed_spells = t->ScribeSpells(min_level, max_level);

	if (c != t) {
		const auto target_description = c->GetTargetDescription(t);

		if (!scribed_spells) {
			c->Message(
				Chat::White,
				fmt::format(
					"No new spells scribed for {}.",
					target_description
				).c_str()
			);
			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"{} New spell{} scribed for {}.",
				scribed_spells,
				scribed_spells != 1 ? "s" : "",
				target_description
			).c_str()
		);
	}
}

