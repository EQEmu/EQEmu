#include "../../bot.h"
#include "../../client.h"

void SetHaste(Client *c, const Seperator *sep)
{
	const uint16 arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set haste [Percentage] [Save] - Set GM Bonus Haste (100 is 100% more Attack Speed) (Save is optional)");
		return;
	}

	Mob* t = c;
	if (c->GetGM() && c->GetTarget() && c->GetTarget()->IsOfClientBot()) {
		t = c->GetTarget();
	}

	const int  extra_haste  = Strings::ToInt(sep->arg[2]);
	const bool need_to_save = sep->arg[3] ? Strings::ToBool(sep->arg[3]) : false;

	t->SetExtraHaste(extra_haste, need_to_save);

	if (t->IsBot()) {
		t->CastToBot()->CalcBonuses();
	} else if (t->IsClient()) {
		t->CastToClient()->CalcBonuses();
	}

	t->SetAttackTimer();

	c->Message(
		Chat::White,
		fmt::format(
			"GM Haste Bonus set to {}%%{} for {}.",
			Strings::Commify(extra_haste),
			need_to_save ? " and saved" : "",
			c->GetTargetDescription(t)
		).c_str()
	);
}
