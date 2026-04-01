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
