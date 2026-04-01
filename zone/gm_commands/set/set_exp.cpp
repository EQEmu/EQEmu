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

void SetEXP(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 3 || !sep->IsNumber(3)) {
		c->Message(Chat::White, "Usage: #set exp [aa|exp] [Amount]");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const std::string& type = Strings::ToLower(sep->arg[2]);

	const bool is_aa  = Strings::EqualFold(type, "aa");
	const bool is_exp = Strings::EqualFold(type, "exp");
	if (!is_aa && !is_exp) {
		c->Message(Chat::White, "Usage: #set exp [aa|exp] [Amount]");
		return;
	}

	const uint32 amount = Strings::ToUnsignedInt(sep->arg[3]);

	if (is_aa) {
		t->SetEXP(
			ExpSource::GM,
			t->GetEXP(),
			amount
		);
	} else if (is_exp) {
		t->SetEXP(
			ExpSource::GM,
			amount,
			t->GetAAXP()
		);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} now {} {} {}experience.",
			c->GetTargetDescription(t, TargetDescriptionType::UCYou),
			c == t ? "have" : "has",
			Strings::Commify(amount),
			is_aa ? "AA " : ""
		).c_str()
	);
}
