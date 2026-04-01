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
#include "zone/worldserver.h"

extern WorldServer worldserver;

void command_revoke(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #revoke [Character Name] [0|1]");
		return;
	}

	std::string character_name = sep->arg[1];

	auto account_id = database.GetAccountIDByChar(character_name.c_str());
	if (!account_id) {
		c->Message(
			Chat::White,
			fmt::format(
				"Character {} does not exist.",
				character_name
			).c_str()
		);
		return;
	}

	bool revoked = Strings::ToInt(sep->arg[2]) ? true : false;

	auto query = fmt::format(
		"UPDATE account SET revoked = {} WHERE id = {}",
		revoked,
		account_id
	);
	auto results = database.QueryDatabase(query);

	c->Message(
		Chat::White,
		fmt::format(
			"{} character {} on account ID {}.",
			revoked ? "Revoking" : "Unrevoking",
			character_name,
			account_id
		).c_str()
	);

	auto revoke_client = entity_list.GetClientByAccID(account_id);
	if (revoke_client) {
		c->Message(
			Chat::White,
			fmt::format(
				"Found {} in this zone.",
				c->GetTargetDescription(revoke_client)
			).c_str()
		);
		revoke_client->SetRevoked(revoked);
		return;
	} else {
		c->Message(
			Chat::White,
			fmt::format(
				"Attempting to {} {}.",
				revoked ? "revoked" : "unrevoke",
				character_name
			).c_str()
		);

		auto pack = new ServerPacket(ServerOP_Revoke, sizeof(RevokeStruct));
		auto rs = (RevokeStruct *) pack->pBuffer;
		strn0cpy(rs->adminname, c->GetName(), sizeof(rs->adminname));
		strn0cpy(rs->name, character_name.c_str(), sizeof(rs->name));
		rs->toggle = revoked;
		worldserver.SendPacket(pack);
		safe_delete(pack);
	};
}

