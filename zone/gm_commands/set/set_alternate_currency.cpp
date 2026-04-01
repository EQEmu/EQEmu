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

void SetAlternateCurrency(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 3 || !sep->IsNumber(2) || !sep->IsNumber(3)) {
		c->Message(Chat::White, "Usage: #set alternate_currency [Currency ID] [Amount]");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const uint32 currency_id      = Strings::ToUnsignedInt(sep->arg[2]);
	const uint32 currency_item_id = zone->GetCurrencyItemID(currency_id);
	const uint32 currency_amount  = Strings::ToUnsignedInt(sep->arg[3]);

	if (!currency_item_id) {
		c->Message(
			Chat::White,
			fmt::format(
				"Currency ID {} could not be found.",
				currency_id
			).c_str()
		);
		return;
	}

	t->SetAlternateCurrencyValue(currency_id, currency_amount);

	c->Message(
		Chat::White,
		fmt::format(
			"{} now {} {} {}.",
			c->GetTargetDescription(t, TargetDescriptionType::UCYou),
			c == t ? "have" : "has",
			Strings::Commify(currency_amount),
			database.CreateItemLink(currency_item_id)
		).c_str()
	);
}
