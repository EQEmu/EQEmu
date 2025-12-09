#include "../../client.h"

void FindSkill(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(2)) {
		const auto skill_id = Strings::ToInt(sep->arg[2]);
		if (EQ::ValueWithin(skill_id, EQ::skills::Skill1HBlunt, EQ::skills::SkillCount)) {
			for (const auto& s : EQ::skills::GetSkillTypeMap()) {
				if (skill_id == s.first) {
					c->Message(
						Chat::White,
						fmt::format(
							"Skill {} | {}",
							s.first,
							s.second
						).c_str()
					);
					break;
				}
			}

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Skill ID {} was not found.",
				skill_id
			).c_str()
		);

		return;
	}

	const auto& search_criteria = Strings::ToLower(sep->argplus[2]);

	auto found_count = 0;

	for (const auto& s : EQ::skills::GetSkillTypeMap()) {
		const auto& skill_name_lower = Strings::ToLower(s.second);
		if (!Strings::Contains(skill_name_lower, sep->argplus[2])) {
			continue;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Skill {} | {}",
				s.first,
				s.second
			).c_str()
		);

		found_count++;

		if (found_count == 50) {
			break;
		}
	}

	if (found_count == 50) {
		c->Message(
			Chat::White,
			fmt::format(
				"50 Skills were found matching '{}', max reached.",
				sep->argplus[2]
			).c_str()
		);

		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Skill{} found matching '{}'.",
			found_count,
			found_count != 1 ? "s" : "",
			sep->argplus[2]
		).c_str()
	);
}
