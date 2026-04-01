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

void command_scribespell(Client *c, const Seperator *sep)
{
	auto arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #scribespell [Spell ID]");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM()) {
		t = c->GetTarget()->CastToClient();
	}

	const auto spell_id = Strings::ToUnsignedInt(sep->arg[1]);

	if (IsValidSpell(spell_id)) {
		t->Message(
			Chat::White,
			fmt::format(
				"Scribing {} ({}) to spellbook.",
				spells[spell_id].name,
				spell_id
			).c_str()
		);

		if (t != c) {
			c->Message(
				Chat::White,
				fmt::format(
					"Scribing {} ({}) for {}.",
					spells[spell_id].name,
					spell_id,
					t->GetName()
				).c_str()
			);
		}

		LogInfo(
			"Scribe spell: [{}] ([{}]) request for [{}] from [{}]",
			spells[spell_id].name,
			spell_id,
			t->GetName(),
			c->GetName()
		);

		if (
			spells[spell_id].classes[Class::Warrior] != 0 &&
			spells[spell_id].skill != EQ::skills::SkillTigerClaw &&
			spells[spell_id].classes[t->GetPP().class_ - 1] > 0 &&
			!IsDiscipline(spell_id)
		) {
			auto book_slot = t->GetNextAvailableSpellBookSlot();

			if (book_slot >= 0 && t->FindSpellBookSlotBySpellID(spell_id) < 0) {
				t->ScribeSpell(spell_id, book_slot);
			} else {
				t->Message(
					Chat::White,
					fmt::format(
						"Unable to scribe {} ({}) to your spellbook.",
						spells[spell_id].name,
						spell_id
					).c_str()
				);

				if (t != c) {
					c->Message(
						Chat::White,
						fmt::format(
							"Unable to scribe {} ({}) for {}.",
							spells[spell_id].name,
							spell_id,
							t->GetName()
						).c_str()
					);
				}
			}
		} else {
			c->Message(Chat::White, "Your target cannot scribe this spell.");
		}
	} else {
		c->Message(
			Chat::White,
			fmt::format(
				"Spell ID {} is an unknown spell and cannot be scribed.",
				spell_id
			).c_str()
		);
	}
}

