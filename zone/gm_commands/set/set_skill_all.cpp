#include "../../client.h"

void SetSkillAll(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set skill_all [Skill Level]");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const uint16 skill_level = Strings::ToUnsignedInt(sep->arg[2]);

	for (const auto& s : EQ::skills::GetSkillTypeMap()) {
		if (t->CanHaveSkill(s.first) && t->MaxSkill(s.first)) {
			if (c != t) {
				c->Message(
					Chat::White,
					fmt::format(
						"Setting {} ({}) to {} for {}.",
						s.second,
						s.first,
						skill_level > t->MaxSkill(s.first) ? t->MaxSkill(s.first) : skill_level,
						c->GetTargetDescription(t)
					).c_str()
				);
			}

			t->SetSkill(
				s.first,
				skill_level > t->MaxSkill(s.first) ? t->MaxSkill(s.first) : skill_level
			);
		}
	}
}
