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
#include "common/repositories/account_repository.h"
#include "zone/client.h"
#include "zone/worldserver.h"

extern WorldServer worldserver;

void command_suspend(Client *c, const Seperator *sep)
{
	const uint16 arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #suspend [Character Name] [Days] [Reason]");
		c->Message(Chat::White, "Note: Specify 0 days to lift a suspension, reason is not required when removing a suspension");
		return;
	}

	const std::string& character_name = sep->arg[1];

	const auto& e = CharacterDataRepository::FindByName(database, character_name);

	if (!e.id) {
		c->Message(
			Chat::White,
			fmt::format(
				"Character '{}' does not exist.",
				character_name
			).c_str()
		);
		return;
	}

	auto a = AccountRepository::FindOne(database, e.account_id);

	if (!a.id) {
		c->Message(
			Chat::White,
			fmt::format(
				"Character '{}' is not attached to an account.",
				character_name
			).c_str()
		);
		return;
	}

	const uint32 days         = Strings::ToUnsignedInt(sep->arg[2]);
	const bool   is_suspend = days != 0;

	const std::string reason = sep->arg[3] ? sep->argplus[3] : "";

	a.status         = is_suspend ? -1 : 0;
	a.suspendeduntil = is_suspend ? std::time(nullptr) + (days * 86400) : 0;
	a.suspend_reason = is_suspend ? reason : "";

	if (!AccountRepository::UpdateOne(database, a)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to {}suspend {}.",
				is_suspend ? "" : "un",
				character_name
			).c_str()
		);
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Account {} ({}) with the character {} {}.",
			a.name,
			a.id,
			character_name,
			(
				is_suspend ?
				fmt::format(
					"has been suspended for {} day{}",
					days,
					days != 1 ? "s" : ""
				) :
				"is no longer suspended"
			)
		).c_str()
	);

	if (is_suspend) { // Only kick if we're suspending, otherwise there's no reason to kick someone who is already suspended
		Client* b = entity_list.GetClientByName(character_name.c_str());

		if (b) {
			b->WorldKick();
			return;
		}

		auto pack = new ServerPacket(ServerOP_KickPlayer, sizeof(ServerKickPlayer_Struct));
		auto* k = (ServerKickPlayer_Struct*) pack->pBuffer;

		strn0cpy(k->adminname, c->GetName(), sizeof(k->adminname));
		strn0cpy(k->name, character_name.c_str(), sizeof(k->name));
		k->adminrank = c->Admin();

		worldserver.SendPacket(pack);

		safe_delete(pack);
	}
}

