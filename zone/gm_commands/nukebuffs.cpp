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

void command_nukebuffs(Client *c, const Seperator *sep)
{
	Mob* target = c;
	if (c->GetTarget()) {
		target = c->GetTarget();
	}

	std::string buff_identifier = Strings::ToLower(sep->arg[1]);
	std::string buff_type;
	bool is_beneficial = buff_identifier.find("beneficial") != std::string::npos;
	bool is_detrimental = buff_identifier.find("detrimental") != std::string::npos;
	bool is_help = buff_identifier.find("help") != std::string::npos;
	if (is_beneficial) {
		target->BuffFadeBeneficial();
		buff_type = " beneficial";
	} else if (is_detrimental) {
		target->BuffFadeDetrimental();
		buff_type = " detrimental";
	} else if (is_help) {
		c->Message(Chat::White, "Usage: #nukebuffs");
		c->Message(Chat::White, "Usage: #nukebuffs beneficial");
		c->Message(Chat::White, "Usage: #nukebuffs detrimental");
		return;
	} else {
		target->BuffFadeAll();
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Faded all{} buffs for {}.",
			buff_type,
			c->GetTargetDescription(target)
		).c_str()
	);
}

