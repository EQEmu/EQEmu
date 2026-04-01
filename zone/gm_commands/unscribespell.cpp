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
#include "common/data_verification.h"
#include "zone/client.h"

void command_unscribespell(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #unscribespell [Spell ID] - Unscribe a spell from your or your target's spell book by Spell ID");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM()) {
		t = c->GetTarget()->CastToClient();
	}

	const uint16 spell_id = EQ::Clamp(Strings::ToInt(sep->arg[1]), 0, 65535);

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

	if (t->HasSpellScribed(spell_id)) {
		t->UnscribeSpellBySpellID(spell_id);

		c->Message(
			Chat::White,
			fmt::format(
				"Unscribing {} ({}) from {}.",
				spell_name,
				spell_id,
				c->GetTargetDescription(t, TargetDescriptionType::LCSelf)
			).c_str()
		);
	} else {
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} not have {} ({}) scribed.",
				c->GetTargetDescription(t, TargetDescriptionType::UCYou),
				c == t ? "do" : "does",
				spell_name,
				spell_id
			).c_str()
		);
	}
}

