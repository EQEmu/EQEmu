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
#include "zone/dialogue_window.h"

void ShowCurrencies(Client *c, const Seperator *sep)
{
	Client *t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const uint64 platinum = (
		t->GetMoney(MoneyTypes::Platinum, MoneySubtypes::Personal) +
		t->GetMoney(MoneyTypes::Platinum, MoneySubtypes::Bank) +
		t->GetMoney(MoneyTypes::Platinum, MoneySubtypes::Cursor) +
		t->GetMoney(MoneyTypes::Platinum, MoneySubtypes::SharedBank)
	);

	const uint64 gold = (
		t->GetMoney(MoneyTypes::Gold, MoneySubtypes::Personal) +
		t->GetMoney(MoneyTypes::Gold, MoneySubtypes::Bank) +
		t->GetMoney(MoneyTypes::Gold, MoneySubtypes::Cursor)
	);

	const uint64 silver = (
		t->GetMoney(MoneyTypes::Silver, MoneySubtypes::Personal) +
		t->GetMoney(MoneyTypes::Silver, MoneySubtypes::Bank) +
		t->GetMoney(MoneyTypes::Silver, MoneySubtypes::Cursor)
	);

	const uint64 copper = (
		t->GetMoney(MoneyTypes::Copper, MoneySubtypes::Personal) +
		t->GetMoney(MoneyTypes::Copper, MoneySubtypes::Bank) +
		t->GetMoney(MoneyTypes::Copper, MoneySubtypes::Cursor)
	);

	std::string currency_table;

	bool has_currency = false;

	currency_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Currency")  +
		DialogueWindow::TableCell("Amount")
	);

	if (
		platinum ||
		gold ||
		silver ||
		copper
	) {
		currency_table += DialogueWindow::TableRow(
			DialogueWindow::TableCell("Money") +
			DialogueWindow::TableCell(Strings::Money(platinum, gold, silver, copper))
		);

		has_currency = true;
	}

	const uint32 ebon_crystals = t->GetEbonCrystals();
	if (ebon_crystals) {
		currency_table += DialogueWindow::TableRow(
			DialogueWindow::TableCell("Ebon Crystals") +
			DialogueWindow::TableCell(Strings::Commify(ebon_crystals))
		);

		has_currency = true;
	}

	const uint32 radiant_crystals = t->GetRadiantCrystals();
	if (radiant_crystals) {
		currency_table += DialogueWindow::TableRow(
			DialogueWindow::TableCell("Radiant Crystals") +
			DialogueWindow::TableCell(Strings::Commify(radiant_crystals))
		);

		has_currency = true;
	}

	for (const auto& a : zone->AlternateCurrencies) {
		const uint32 currency_value = t->GetAlternateCurrencyValue(a.id);
		if (currency_value) {
			const auto *item = database.GetItem(a.item_id);
			currency_table += DialogueWindow::TableRow(
				DialogueWindow::TableCell(item->Name) +
				DialogueWindow::TableCell(Strings::Commify(currency_value))
			);

			has_currency = true;
		}
	}

	for (const auto& l : ldon_theme_names) {
		const uint32 ldon_currency_value = t->GetLDoNPointsTheme(l.first);
		if (ldon_currency_value) {
			currency_table += DialogueWindow::TableRow(
				DialogueWindow::TableCell(l.second.first) +
				DialogueWindow::TableCell(Strings::Commify(ldon_currency_value))
			);

			has_currency = true;
		}
	}

	const uint32 pvp_points = t->GetPVPPoints();
	if (pvp_points) {
		currency_table += DialogueWindow::TableRow(
			DialogueWindow::TableCell("PVP Points") +
			DialogueWindow::TableCell(Strings::Commify(pvp_points))
		);

		has_currency = true;
	}

	currency_table = DialogueWindow::Table(currency_table);

	if (!has_currency) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} not have any currencies.",
				c->GetTargetDescription(t, TargetDescriptionType::UCYou),
				c == t ? "do" : "does"
			).c_str()
		);

		return;
	}

	c->SendPopupToClient(
		fmt::format(
			"Currency for {}",
			c->GetTargetDescription(t, TargetDescriptionType::UCSelf)
		).c_str(),
		currency_table.c_str()
	);
}
