#include "../../client.h"

void ShowSpells(Client *c, const Seperator *sep)
{
	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const auto is_disciplines = !strcasecmp(sep->arg[2], "disciplines");
	const auto is_spells      = !strcasecmp(sep->arg[2], "spells");
	if (
		!is_disciplines &&
		!is_spells
	) {
		c->Message(Chat::White, "Usages: #show spells disciplines - Show your or your target's learned disciplines");
		c->Message(Chat::White, "Usages: #show spells spells - Show your or your target's memorized spells");
		return;
	}

	ShowSpellType show_spell_type;

	if (is_disciplines) {
		show_spell_type = ShowSpellType::Disciplines;
	} else if (is_spells) {
		show_spell_type = ShowSpellType::Spells;
	}

	t->ShowSpells(c, show_spell_type);
}
