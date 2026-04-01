/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
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
