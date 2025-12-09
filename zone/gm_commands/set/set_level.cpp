#include "../../bot.h"
#include "../../client.h"

void SetLevel(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set level [Level]");
		return;
	}

	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	const uint8 max_level = RuleI(Character, MaxLevel);
	const uint8 level     = Strings::ToUnsignedInt(sep->arg[2]);

	if (c != t && c->Admin() < RuleI(GM, MinStatusToLevelTarget)) {
		c->Message(Chat::White, "Your status is not high enough to change another person's level.");
		return;
	}

	t->SetLevel(level, true);

	if (t->IsClient()) {
		for (const auto& s : EQ::skills::GetSkillTypeMap()) {
			const uint16 max_skill_value = t->CastToClient()->MaxSkill(s.first);
			if (t->GetSkill(s.first) > max_skill_value) {
				t->CastToClient()->SetSkill(s.first, max_skill_value);
			}
		}

		t->CastToClient()->SendLevelAppearance();

		if (RuleB(Bots, Enabled) && RuleB(Bots, BotLevelsWithOwner)) {
			Bot::LevelBotWithClient(t->CastToClient(), level, true);
		}
	}
}
