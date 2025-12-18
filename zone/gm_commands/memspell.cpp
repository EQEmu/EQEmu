#include "zone/client.h"

void command_memspell(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (
		!arguments ||
		!sep->IsNumber(1)
	) {
		c->Message(Chat::White, "Usage: #memspell [Spell ID] [Spell Gem]");
		return;
	}

	auto target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM()) {
		target = c->GetTarget()->CastToClient();
	}

	auto spell_id = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[1]));
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

	auto empty_slot = target->FindEmptyMemSlot();
	if (empty_slot == -1) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} not have a place to memorize {} ({}).",
				c->GetTargetDescription(target, TargetDescriptionType::UCYou),
				c == target ? "do" : "does",
				GetSpellName(spell_id),
				spell_id
			).c_str()
		);
		return;
	}

	auto spell_gem = sep->IsNumber(2) ? Strings::ToUnsignedInt(sep->arg[2]) : empty_slot;
	if (spell_gem > EQ::spells::SPELL_GEM_COUNT) {
		c->Message(
			Chat::White,
			fmt::format(
				"Spell Gems range from 0 to {}.",
				EQ::spells::SPELL_GEM_COUNT
			).c_str()
		);
		return;
	}

	target->MemSpell(spell_id, spell_gem);

	if (c != target) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} ({}) memorized to spell gem {} for {}.",
				GetSpellName(spell_id),
				spell_id,
				spell_gem,
				c->GetTargetDescription(target)
			).c_str()
		);
	}
}
