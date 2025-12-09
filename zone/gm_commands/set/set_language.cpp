#include "../../client.h"
#include "../../../common/data_verification.h"

void SetLanguage(Client *c, const Seperator *sep)
{
	const int arguments = sep->argnum;
	if (arguments < 3 || !sep->IsNumber(2) || !sep->IsNumber(3)) {
		c->Message(Chat::White, "Usage: #set language [Language ID] [Language Skill]");
		c->Message(Chat::White, "Language ID = 0 to 27");
		c->Message(Chat::White, "Language Skill = 0 to 100");
		return;
	}

	Client* t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const uint8 language_id    = Strings::ToInt(sep->arg[2]);
	const uint8 language_skill = Strings::ToInt(sep->arg[3]);
	if (
		!EQ::ValueWithin(language_id, Language::CommonTongue, Language::Unknown27) ||
		!EQ::ValueWithin(language_skill, 0, Language::MaxValue)
	) {
		c->Message(Chat::White, "Usage: #set language [Language ID] [Language Skill]");
		c->Message(Chat::White, "Language ID = 0 to 27");
		c->Message(Chat::White, "Language Skill = 0 to 100");
		return;
	}

	LogInfo(
		"Set language request from [{}], Target: [{}] Language ID: [{}] Language Skill: [{}]",
		c->GetCleanName(),
		c->GetTargetDescription(t),
		language_id,
		language_skill
	);

	t->SetLanguageSkill(language_id, language_skill);

	if (c != t) {
		c->Message(
			Chat::White,
			fmt::format(
				"Set {} ({}) to {} for {}.",
				EQ::constants::GetLanguageName(language_id),
				language_id,
				language_skill,
				c->GetTargetDescription(t)
			).c_str()
		);
	}
}
