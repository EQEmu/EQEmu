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
