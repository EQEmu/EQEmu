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

void command_merchantshop(Client *c, const Seperator *sep)
{
	const auto m = c->GetTarget();
	if (
		!m ||
		!m->IsNPC() ||
		(
			m->GetClass() != Class::Merchant &&
			m->GetClass() != Class::DiscordMerchant &&
			m->GetClass() != Class::AdventureMerchant &&
			m->GetClass() != Class::NorrathsKeepersMerchant &&
			m->GetClass() != Class::DarkReignMerchant &&
			m->GetClass() != Class::AlternateCurrencyMerchant
		)
	) {
		c->Message(Chat::White, "You must target a merchant.");
		return;
	}

	const auto arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "#merchantshop close - Close your targeted merchant's shop");
		c->Message(Chat::White, "#merchantshop open - Open your targeted merchant's shop");
		return;
	}

	const bool is_close = !strcasecmp(sep->arg[1], "close");
	const bool is_open  = !strcasecmp(sep->arg[1], "open");
	if (!is_close && !is_open) {
		c->Message(Chat::White, "#merchantshop close - Close your targeted merchant's shop");
		c->Message(Chat::White, "#merchantshop open - Open your targeted merchant's shop");
		return;
	}

	if (is_close) {
		m->CastToNPC()->MerchantCloseShop();
	} else if (is_open) {
		m->CastToNPC()->MerchantOpenShop();
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} shop for {}.",
			is_close ? "Closed" : "Opened",
			c->GetTargetDescription(m)
		).c_str()
	);
}

