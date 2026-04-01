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

void command_suspendmulti(Client *c, const Seperator *sep)
{
	auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #suspendmulti [Character Name|Character Name Two|etc] [Days] [Reason]");
		c->Message(Chat::White, "Note: Specify 0 days to lift a suspension");
		return;
	}

	const auto& n = Strings::Split(sep->arg[1], "|");
	std::vector<std::string> v;
	for (const auto& c : n) {
		v.emplace_back(fmt::format("'{}'", Strings::ToLower(c)));
	}

	auto days = Strings::ToUnsignedInt(sep->arg[2]);

	const std::string reason = sep->arg[3] ? sep->argplus[3] : "";

	auto l = AccountRepository::GetWhere(
		database,
		fmt::format(
			"LOWER(charname) IN ({})",
			Strings::Implode(", ", v)
		)
	);

	if (l.empty()) {
		c->Message(Chat::White, "No characters found.");
		return;
	}

	for (auto a : l) {
		a.status         = -1;
		a.suspendeduntil = std::time(nullptr) + (days * 86400);
		a.suspend_reason = reason;

		if (!AccountRepository::UpdateOne(database, a)) {
			c->Message(
				Chat::White,
				fmt::format(
					"Failed to suspend Account {} ({}).",
					a.name,
					a.id
				).c_str()
			);
			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Account {} ({}) {}.",
				a.name,
				a.id,
				(
					days ?
						fmt::format(
							"has been temporarily suspended for {} day{}.",
							days,
							days != 1 ? "s" : ""
						) :
						"is no longer suspended"
				)
			).c_str()
		);

		auto *b = entity_list.GetClientByAccID(a.id);

		if (b) {
			b->WorldKick();
			return;
		}

		auto pack = new ServerPacket(ServerOP_KickPlayer, sizeof(ServerKickPlayer_Struct));
		auto *k = (ServerKickPlayer_Struct *) pack->pBuffer;

		strn0cpy(k->adminname, c->GetName(), sizeof(k->adminname));
		k->account_id = a.id;
		k->adminrank = c->Admin();

		worldserver.SendPacket(pack);

		safe_delete(pack);
	}
}

