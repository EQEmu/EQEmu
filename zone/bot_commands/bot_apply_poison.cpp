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
#include "zone/bot_command.h"

void bot_command_apply_poison(Client* c, const Seperator* sep)
{
	if (helper_command_disabled(c, RuleB(Bots, AllowApplyPoisonCommand), "applypoison")) {
		return;
	}
	if (helper_command_alias_fail(c, "bot_command_apply_poison", sep->arg[0], "applypoison")) {
		return;
	}
	if (helper_is_help_or_usage(sep->arg[1])) {

		c->Message(Chat::White, "usage: <rogue_bot_target> %s", sep->arg[0]);
		c->Message(Chat::White, "note: Applies cursor-held poison to a rogue bot's weapon");
		return;
	}

	Bot* my_rogue_bot = nullptr;
	auto t = c->GetTarget();

	if (
		t &&
		t->IsBot() &&
		t->CastToBot()->GetBotOwnerCharacterID() == c->CharacterID() &&
		t->GetClass() == Class::Rogue
		) {
		my_rogue_bot = t->CastToBot();
	}

	if (!my_rogue_bot) {

		c->Message(Chat::White, "You must target a rogue bot that you own to use this command!");
		return;
	}
	if (my_rogue_bot->GetLevel() < 18) {

		c->Message(
			Chat::White,
			"Your rogue bot must be level 18 before %s can apply poison!",
			(my_rogue_bot->GetGender() == Gender::Female ? "she" : "he"));
		return;
	}

	const auto poison_instance = c->GetInv().GetItem(EQ::invslot::slotCursor);
	if (!poison_instance) {

		c->Message(Chat::White, "No item found on cursor!");
		return;
	}

	auto poison_data = poison_instance->GetItem();
	if (!poison_data) {

		c->Message(Chat::White, "No data found for cursor item!");
		return;
	}

	if (poison_data->ItemType == EQ::item::ItemTypePoison) {

		if ((~poison_data->Races) & GetPlayerRaceBit(my_rogue_bot->GetRace())) {

			c->Message(Chat::White, "Invalid race for weapon poison!");
			return;
		}

		if (poison_data->Proc.Level2 > my_rogue_bot->GetLevel()) {

			c->Message(Chat::White, "This poison is too powerful for your intended target!");
			return;
		}

		// generalized from client ApplyPoison handler
		double ChanceRoll   = zone->random.Real(0, 1);
		uint16 poison_skill = 95 + ((my_rogue_bot->GetLevel() - 18) * 5);
		if (poison_skill > 200) {
			poison_skill = 200;
		}
		bool apply_poison_chance = (ChanceRoll < (.75 + poison_skill / 1000));

		if (apply_poison_chance && my_rogue_bot->AddProcToWeapon(
			poison_data->Proc.Effect,
			false,
			(my_rogue_bot->GetDEX() / 100) + 103,
			POISON_PROC
		)) {
			c->Message(
				Chat::White,
				"Successfully applied %s to %s's weapon.",
				poison_data->Name,
				my_rogue_bot->GetCleanName());
		} else {
			c->Message(
				Chat::White,
				"Failed to apply %s to %s's weapon.",
				poison_data->Name,
				my_rogue_bot->GetCleanName());
		}

		c->DeleteItemInInventory(EQ::invslot::slotCursor, 1, true);
	} else {

		c->Message(Chat::White, "Item on cursor is not a weapon poison!");
		return;
	}
}
