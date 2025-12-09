#include "../../client.h"

void SetSkill(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 3 || !sep->IsNumber(2) || !sep->IsNumber(3)) {
		c->Message(Chat::White, "Usage: #set skill [Skill ID] [Skill Value]");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const uint16 skill_id    = Strings::ToUnsignedInt(sep->arg[2]);
	const uint16 skill_value = Strings::ToUnsignedInt(sep->arg[3]);

	if (!EQ::ValueWithin(skill_id, EQ::skills::Skill1HBlunt, EQ::skills::HIGHEST_SKILL)) {
		c->Message(Chat::White, "Usage: #set skill [Skill ID] [Skill Value]");
		c->Message(Chat::White, fmt::format("Skill ID: 0 to {}", EQ::skills::HIGHEST_SKILL).c_str());
		return;
	}

	const auto skill_type = static_cast<EQ::skills::SkillType>(skill_id);

	t->SetSkill(
		skill_type,
		skill_value > t->MaxSkill(skill_type) ? t->MaxSkill(skill_type) : skill_value
	);

	if (c != t) {
		c->Message(
			Chat::White,
			fmt::format(
				"Set {} ({}) to {} for {}.",
				EQ::skills::GetSkillName(skill_type),
				skill_id,
				skill_value > t->MaxSkill(skill_type) ? t->MaxSkill(skill_type) : skill_value,
				c->GetTargetDescription(t)
			).c_str()
		);
	}
}
