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

void SetCrystals(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(3)) {
		c->Message(Chat::White, "Usage: #setcrystals [ebon|radiant] [Amount]");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const std::string& crystal_type = Strings::ToLower(sep->arg[2]);
	const uint32 crystal_amount = Strings::ToUnsignedInt(sep->arg[3]);

	const bool is_ebon = Strings::EqualFold(crystal_type, "ebon");
	const bool is_radiant = Strings::EqualFold(crystal_type, "radiant");
	if (!is_ebon && !is_radiant) {
		c->Message(Chat::White, "Usage: #setcrystals [ebon|radiant] [Amount]");
		return;
	}

	const uint32 crystal_item_id = (
		is_ebon ?
		RuleI(Zone, EbonCrystalItemID) :
		RuleI(Zone, RadiantCrystalItemID)
	);

	if (is_radiant) {
		t->SetRadiantCrystals(crystal_amount);
	} else {
		t->SetEbonCrystals(crystal_amount);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} now {} {} {}.",
			c->GetTargetDescription(t, TargetDescriptionType::UCYou),
			c == t ? "have" : "has",
			Strings::Commify(crystal_amount),
			database.CreateItemLink(crystal_item_id)
		).c_str()
	);
}
