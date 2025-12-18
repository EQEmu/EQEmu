#include "zone/client.h"

void command_castspell(Client *c, const Seperator *sep)
{
	if (SPDAT_RECORDS <= 0) {
		c->Message(Chat::White, "Spells not loaded.");
		return;
	}

	const auto arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(
			Chat::White,
			"Usage: #castspell [Spell ID]  [Instant (0 = False, 1 = True, Default is 1 if Unused)]"
		);
		return;
	}

	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	const uint16 spell_id = Strings::ToUnsignedInt(sep->arg[1]);

	if (spell_id >= SPDAT_RECORDS) {
		c->Message(Chat::White, "Invalid Spell ID.");
		return;
	}

	const bool   instant_cast = sep->IsNumber(2) ? Strings::ToBool(sep->arg[2]) : true;
	const uint16 target_id    = t->GetID();

	if (instant_cast) {
		c->SpellFinished(spell_id, t);
	} else {
		c->CastSpell(spell_id, t->GetID(), EQ::spells::CastingSlot::Item, spells[spell_id].cast_time);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Cast {} ({}) on {}{}.",
			GetSpellName(spell_id),
			spell_id,
			c->GetTargetDescription(t, TargetDescriptionType::LCSelf, target_id),
			instant_cast ? " instantly" : ""
		).c_str()
	);
}
