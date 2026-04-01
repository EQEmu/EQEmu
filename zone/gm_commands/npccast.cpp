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

void command_npccast(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	auto target = c->GetTarget()->CastToNPC();
	if (!sep->IsNumber(1) && sep->arg[1] && sep->IsNumber(2)) {
		std::string entity_name = sep->arg[1] ? sep->arg[1] : 0;
		auto spell_id = sep->arg[2] ? Strings::ToUnsignedInt(sep->arg[2]) : 0;
		auto spell_target = entity_list.GetMob(entity_name.c_str());
		if (spell_target && IsValidSpell(spell_id) && spell_id < SPDAT_RECORDS) {
			c->Message(
				Chat::White,
				fmt::format(
					"{} casting {} ({}) on {}.",
					c->GetTargetDescription(target),
					GetSpellName(static_cast<uint16>(spell_id)),
					spell_id,
					c->GetTargetDescription(spell_target)
				).c_str()
			);

			target->CastSpell(spell_id, spell_target->GetID());
		} else {
			if (!spell_target) {
				c->Message(
					Chat::White,
					fmt::format(
						"Entity {} was not found",
						entity_name
					).c_str()
				);
			} else if (!spell_id || !IsValidSpell(spell_id)) {
				c->Message(
					Chat::White,
					fmt::format(
						"Spell ID {} was not found",
						spell_id
					).c_str()
				);
			}
		}
	} else if (sep->IsNumber(1) && sep->IsNumber(2)) {
		uint16 entity_id = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[1]));
		auto spell_id = Strings::ToUnsignedInt(sep->arg[2]);
		auto spell_target = entity_list.GetMob(entity_id);
		if (spell_target && IsValidSpell(spell_id) && spell_id < SPDAT_RECORDS) {
			c->Message(
				Chat::White,
				fmt::format(
					"{} casting {} ({}) on {}.",
					c->GetTargetDescription(target),
					GetSpellName(static_cast<uint16>(spell_id)),
					spell_id,
					c->GetTargetDescription(spell_target)
				).c_str()
			);

			target->CastSpell(spell_id, spell_target->GetID());
		} else {
			if (!spell_target) {
				c->Message(
					Chat::White,
					fmt::format(
						"Entity ID {} was not found",
						entity_id
					).c_str()
				);
			} else if (!spell_id || !IsValidSpell(spell_id)) {
				c->Message(
					Chat::White,
					fmt::format(
						"Spell ID {} was not found",
						spell_id
					).c_str()
				);
			}
		}
	}
}

