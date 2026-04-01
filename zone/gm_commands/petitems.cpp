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
#include "zone/bot.h"
#include "zone/client.h"

void command_petitems(Client *c, const Seperator *sep)
{
	NPC* t = nullptr;

	if (c->GetPet()) {
		t = c->GetPet()->CastToNPC();
	}

	if (c->GetTarget() && c->GetTarget()->IsNPC()) {
		t = c->GetTarget()->CastToNPC();
	}

	if (!t || !t->IsPet()) {
		c->Message(Chat::White, "You must have a pet or target a bot's pet to use this command.");
		return;
	}	

	Mob* o = t->GetOwner();
	if (!o) {
		c->Message(Chat::White, "Invalid owner for pet.");
		return;
	}

	if (
		o->IsBot() &&
		o->CastToBot()->GetBotOwnerCharacterID() != c->CharacterID()
	) {
		c->Message(Chat::White, "You do not own the targeted pet.");
		return;
	}

	if (
		o->IsClient() &&
		o->GetID() != c->GetID()
	) {
		c->Message(Chat::White, "You do not own the targeted pet.");
		return;
	}

	const std::string& pet_owner = (
		o->IsClient() ?
		"Your" :
		fmt::format(
			"Your bot {}{}",
			o->GetCleanName(),
			Strings::EndsWith(o->GetCleanName(), "s") ? "'" : "'s"
		)
	);

	const auto& l = t->GetLootList();
	if (!l.empty()) {
		t->QueryLoot(c, true);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} pet has {} item{}.",
			pet_owner,
			l.size(),
			l.size() != 1 ? "s" : ""
		).c_str()
	);
}
