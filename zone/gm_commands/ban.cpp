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

void command_ban(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #ban [Character Name] [Reason]");
		return;
	}

	std::string character_name = sep->arg[1];
	if (character_name.empty()) {
		c->Message(Chat::White, "Usage: #ban [Character Name] [Reason]");
		return;
	}

	std::string reason = sep->argplus[2];
	if (reason.empty()) {
		c->Message(Chat::White, "Usage: #ban [Character Name] [Reason]");
		return;
	}

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

	auto query = fmt::format(
		"UPDATE account SET status = -2, ban_reason = '{}' WHERE id = {}",
		Strings::Escape(reason),
		account_id
	);
	auto results = database.QueryDatabase(query);

	c->Message(
		Chat::White,
		fmt::format(
			"Account ID {} with the character {} has been banned for the following reason: \"{}\"",
			account_id,
			character_name,
			reason
		).c_str()
	);

	ServerPacket flagUpdatePack(ServerOP_FlagUpdate, sizeof(ServerFlagUpdate_Struct));
	auto sfus = (ServerFlagUpdate_Struct *) flagUpdatePack.pBuffer;
	sfus->account_id = account_id;
	sfus->admin = -2;
	worldserver.SendPacket(&flagUpdatePack);

	auto client = entity_list.GetClientByName(character_name.c_str());
	if (client) {
		client->WorldKick();
		return;
	}

	ServerPacket kickPlayerPack(ServerOP_KickPlayer, sizeof(ServerKickPlayer_Struct));
	auto skp = (ServerKickPlayer_Struct *) kickPlayerPack.pBuffer;
	strcpy(skp->adminname, c->GetName());
	strcpy(skp->name, character_name.c_str());
	skp->adminrank = c->Admin();
	worldserver.SendPacket(&kickPlayerPack);
}

