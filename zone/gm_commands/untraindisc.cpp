#include "common/data_verification.h"
#include "zone/client.h"

void command_untraindisc(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #untraindisc [Spell ID] - Untrain your or your target's discipline by Spell ID");
		return;
	}

	auto target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM()) {
		target = c->GetTarget()->CastToClient();
	}

	uint16 spell_id = EQ::Clamp(Strings::ToInt(sep->arg[1]), 0, 65535);

	if (!IsValidSpell(spell_id)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Spell ID {} could not be found.",
				spell_id
			).c_str()
		);
		return;
	}

	auto spell_name = GetSpellName(spell_id);

	if (target->HasDisciplineLearned(spell_id)) {
		target->UntrainDiscBySpellID(spell_id);

		c->Message(
			Chat::White,
			fmt::format(
				"Untraining {} ({}) for {}.",
				spell_name,
				spell_id,
				c->GetTargetDescription(target)
			).c_str()
		);
	} else {
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} not have {} ({}) trained.",
				c->GetTargetDescription(target),
				c == target ? "do" : "does",
				spell_name,
				spell_id
			).c_str()
		);
	}
}

